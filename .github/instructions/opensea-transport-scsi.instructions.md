---
description: 'SCSI command building patterns for opensea-transport — ScsiIoCtx, CDB construction, sense data parsing, and SCSI-specific conventions'
applyTo: 'subprojects/opensea-transport/**/*.c, subprojects/opensea-transport/**/*.h'
---

# SCSI Command Building — opensea-transport

## Relevant Files

| File | Purpose |
|------|---------|
| `include/scsi_helper.h` | CDB length enum, sense format/key/ASC constants, inquiry response structures, `ScsiIoCtx`, operation code CDB-length helper |
| `include/scsi_helper_func.h` | Public SCSI dispatch and helper functions |
| `src/scsi_cmds.c` | SCSI command implementations |
| `src/scsi_helper.c` | Sense data parsing, verbose output, inquiry/log page helpers |
| `include/sat_helper.h` | SAT constants (for ATA passthrough via SCSI CDB) |
| `include/sntl_helper.h` | SCSI-to-NVMe translation (SNTL) constants |

---

## The `ScsiIoCtx` Structure

All SCSI commands go through `scsi_Send_Cdb`. For most use cases you do not build `ScsiIoCtx` directly — call `scsi_Send_Cdb` which builds it internally.

**Prefer `DECLARE_ZERO_INIT_ARRAY` from `opensea-common`** for all stack-allocated CDB and data buffers. It zero-initializes at the point of declaration and is compatible across all supported compilers:

```c
DECLARE_ZERO_INIT_ARRAY(uint8_t, cdb, CDB_LEN_10);
cdb[CDB_OPERATION_CODE]  = 0x25;   // READ CAPACITY(10) operation code
// ... fill remaining CDB bytes ...

DECLARE_ZERO_INIT_ARRAY(uint8_t, dataBuf, 8);

eReturnValues ret = scsi_Send_Cdb(
    device,
    cdb,
    CDB_LEN_10,
    dataBuf,
    sizeof(dataBuf),
    XFER_DATA_IN,
    M_NULLPTR,          // senseData: M_NULLPTR → use device's last command sense buffer
    0,                  // senseDataLen: 0 → use device's last command sense buffer
    DEFAULT_COMMAND_TIMEOUT
);
```

When you need direct `ScsiIoCtx` access (for custom sense buffer management or ATA-via-SCSI commands), zero-initialize first:

```c
ScsiIoCtx scsiIoCtx;
safe_memset(&scsiIoCtx, sizeof(scsiIoCtx), 0, sizeof(scsiIoCtx));
scsiIoCtx.device      = device;    // M_NONNULL
scsiIoCtx.cdbLength   = CDB_LEN_16;
scsiIoCtx.direction   = XFER_DATA_IN;
scsiIoCtx.pdata       = dataBuf;
scsiIoCtx.dataLength  = dataLen;
scsiIoCtx.psense      = senseBuffer;
scsiIoCtx.senseDataSize = sizeof(senseBuffer);
scsiIoCtx.timeout     = DEFAULT_COMMAND_TIMEOUT;
safe_memset(scsiIoCtx.cdb, CDB_LEN_16, 0, CDB_LEN_16);
scsiIoCtx.cdb[CDB_OPERATION_CODE] = 0x88;   // READ(16)
// ... fill remaining CDB bytes ...
```

---

## CDB Length Selection

SCSI defines CDB lengths based on the operation code ranges (SPC specification):

| Operation code range | CDB length | `eCDBLen` constant |
|--------------------|-----------|------------------|
| 0x00–0x1F | 6 bytes | `CDB_LEN_6` |
| 0x20–0x5F | 10 bytes | `CDB_LEN_10` |
| 0x80–0x9F | 12 bytes | `CDB_LEN_12` |
| 0xA0–0xBF | 16 bytes | `CDB_LEN_16` |
| 0x7E–0x7F | Technically variable-length (up to 255 bytes per spec); **implemented as 32 bytes** in this codebase. The `eCDBLen` enum covers common lengths; occasionally a specific command requires a non-standard length. | `CDB_LEN_32` (common case) |
| 0xC0–0xFF | Vendor unique; no standard length | — |

Most SCSI command implementations in `scsi_cmds.c` hard-code the CDB length for the specific command being built.

Always use the `eCDBOffsets` enum constants for CDB byte indices — primarily required to silence clang-tidy warnings, and makes byte intent explicit:
```c
cdb[CDB_OPERATION_CODE] = 0x25;    // not cdb[0]
cdb[CDB10_CONTROL]      = 0x00;    // not cdb[9]
```

### Zero-Length Transfers and Command Probing

Transfer length semantics differ by CDB size:

- **6-byte READ/WRITE commands**: a transfer length of 0 means **256 sectors** — the same convention as ATA's 8-bit sector count.
- **10-, 12-, and 16-byte READ/WRITE commands**: a transfer length of 0 is specified as a no-data probe — the device validates the CDB and returns status without transferring data. This was intended to let hosts check command support cleanly.
- **In practice**: most SAT translators (and many real drives) do not correctly implement the zero-length probe. They attempt a full transfer instead of a no-op, producing a timeout or hardware reset. Do not rely on zero-length probing through a SATL, SNTL, or USB bridge. See the Command Support Detection section.

---

## Sense Data

### What sense data is

After a SCSI command completes with a non-good status, the initiator requests sense data (SCSI protocol) or reads it from the device (REQUEST SENSE). Sense data describes the error in a structured format.

### Sense response formats

| Format | `eSenseFormat` constant | When used |
|--------|----------------------|-----------|
| Current fixed | `SCSI_SENSE_CUR_INFO_FIXED` (0x70) | Traditional format; widely supported |
| Deferred fixed | `SCSI_SENSE_DEFER_ERR_FIXED` (0x71) | Error from a command that returned initial status before completing. This occurs with true asynchronous commands, or with commands that have an IMMED bit (e.g., SYNCHRONIZE CACHE, FORMAT UNIT): the device returns status after CDB validation; if an error occurs during the subsequent background operation it is reported as a deferred error. |
| Current descriptor | `SCSI_SENSE_CUR_INFO_DESC` (0x72) | SPC-3+ preferred format; supports ATA Return Descriptor |
| Deferred descriptor | `SCSI_SENSE_DEFER_ERR_DESC` (0x73) | Deferred descriptor format |

Always check the response code byte before parsing. The low 7 bits carry the format value.

**Fixed-format VALID bit (bit 7 of byte 0)**: Bit 7 of byte 0 in fixed-format sense data is the VALID flag for the INFORMATION field. When set, the response code byte appears as `0xF0` or `0xF1` rather than `0x70` or `0x71`. Always mask off bit 7 when comparing the response code. Some SAT translators do not set VALID correctly in response to ATA PASS-THROUGH CDBs, but the INFORMATION field may still contain useful data and should be checked regardless.

### Sense key values

| Key | Constant | Typical meaning |
|-----|----------|----------------|
| 0x00 | `SENSE_KEY_NO_ERROR` | Command completed (check condition bits may still be set) |
| 0x01 | `SENSE_KEY_RECOVERED_ERROR` | Data transferred but with a recovered error |
| 0x02 | `SENSE_KEY_NOT_READY` | Device not ready (spinning up, no media, etc.) |
| 0x03 | `SENSE_KEY_MEDIUM_ERROR` | Unrecoverable read/write error |
| 0x04 | `SENSE_KEY_HARDWARE_ERROR` | Hardware failure |
| 0x05 | `SENSE_KEY_ILLEGAL_REQUEST` | Invalid CDB / invalid field in CDB (command not supported) |
| 0x06 | `SENSE_KEY_UNIT_ATTENTION` | State change detected (reset, media change, etc.) |
| 0x07 | `SENSE_KEY_DATA_PROTECT` | Write protected media |
| 0x0B | `SENSE_KEY_ABORTED_COMMAND` | Command aborted by the device |

### Using `check_Sense_Key_ASC_ASCQ_And_FRU`

```c
eReturnValues ret = scsi_Send_Cdb(device, cdb, cdbLen, dataBuf, dataLen, XFER_DATA_IN,
                                  M_NULLPTR, 0, DEFAULT_COMMAND_TIMEOUT);

if (ret != SUCCESS)
{
    // Pull sense fields from device's last-command sense buffer:
    senseDataFields senseFields;
    safe_memset(&senseFields, sizeof(senseFields), 0, sizeof(senseFields));
    get_Sense_Data_Fields(device->drive_info.lastCommandSenseData,
                          SPC3_SENSE_LEN,
                          &senseFields);

    return check_Sense_Key_ASC_ASCQ_And_FRU(device,
                                             senseFields.senseKey,
                                             senseFields.additionalSenseCode,
                                             senseFields.additionalSenseCodeQualifier,
                                             senseFields.fieldReplaceableUnitCode);
}
```

**Mapping guideline**: `SENSE_KEY_ILLEGAL_REQUEST` with ASC 0x20 (INVALID COMMAND OPERATION CODE) or 0x24 (INVALID FIELD IN CDB) → return `NOT_SUPPORTED`. `SENSE_KEY_MEDIUM_ERROR` → return `FAILURE`. `SENSE_KEY_UNIT_ATTENTION` → the condition may clear on retry, but always inspect the ASC/ASCQ pair first. Some UA conditions are triggered by MRIE (Method of Reporting Informational Exceptions) mode page configuration and represent informational exception events rather than true bus or device state changes — these may recur without a real error and may require manual intervention rather than a simple retry.

**FRU codes** are always vendor-unique. No FRU code value is standardized by SPC — do not attempt to interpret them portably.

**ASC/ASCQ standardization**: Values below `80h` for both ASC and ASCQ are standardized by SPC. Values `80h` and above are vendor-specific — treat them as opaque identifiers.

---

## Inquiry and VPD Pages

SCSI INQUIRY is the primary device identification mechanism. VPD pages extend the basic inquiry with additional structured device information.

Response length has evolved across SCSI generations:
- **Pre-SCSI-2**: no fixed length; SCSI and CCS formats existed with different field layouts. These devices are essentially never encountered today.
- **SCSI-2**: minimum 36 bytes.
- **Modern (SPC/SCSI-3 and later, block devices)**: minimum 96 bytes.

Some device-type-specific standards require different minimums. RBC (Reduced Block Commands) requires 6 bytes. Tape and other media types follow their own standards. For HDDs and SSDs, 96 bytes is the correct minimum.

```c
// Standard inquiry (96 bytes for block devices)
DECLARE_ZERO_INIT_ARRAY(uint8_t, inquiryData, INQ_RETURN_DATA_LENGTH);

DECLARE_ZERO_INIT_ARRAY(uint8_t, cdb, CDB_LEN_6);
cdb[CDB_OPERATION_CODE] = 0x12;   // INQUIRY
cdb[4]                  = INQ_RETURN_DATA_LENGTH;

eReturnValues ret = scsi_Send_Cdb(device, cdb, CDB_LEN_6,
                                  inquiryData, INQ_RETURN_DATA_LENGTH,
                                  XFER_DATA_IN, M_NULLPTR, 0, DEFAULT_COMMAND_TIMEOUT);

// For VPD pages, set EVPD bit and page code:
cdb[1] = 0x01;    // EVPD = 1
cdb[2] = 0x80;    // PAGE CODE (0x80 = Unit Serial Number)
```

Use the `INQ_DATA_T10_VENDOR_ID_LEN` (8), `INQ_DATA_PRODUCT_ID_LEN` (16), and `INQ_DATA_PRODUCT_REV_LEN` (4) constants when parsing the inquiry response.

---

## Log Pages and Mode Pages

Log pages carry operational counters, error history, and self-test results. Mode pages carry drive configuration parameters.

CDB patterns are already implemented in `scsi_cmds.c` — use the existing wrapper functions rather than building raw CDBs:
- `scsi_Log_Sense` — retrieve a log page
- `scsi_Mode_Sense_10` / `scsi_Mode_Sense_6` — retrieve mode page data
- `scsi_Mode_Select_10` / `scsi_Mode_Select_6` — write mode page data

Always use `MODE_SENSE_10` over `MODE_SENSE_6` unless the device is known to be an old SCSI-2 device without 10-byte command support.

### Log Page Internal Structure

SCSI log pages use a multi-level flexible array format:
- **Page header**: page code, subpage code, and a page length field describing the remaining bytes.
- **Parameters**: each parameter has a 2-byte parameter code, a control byte, a 1-byte length, and then the value bytes.

Never assume a fixed offset for any parameter — always walk the parameter list using the declared lengths. Do not read past the end of the page's declared length.

**Vendor-unique parameter codes**: Most log pages reserve codes `8000h` and above for vendor-unique data. The Informational Exceptions log page is an exception — vendor-unique codes start at `0001h` in that page. Check the page-specific spec table before assuming a parameter code is standardized.

**Size validation**: Always validate that a reported page or parameter length is consistent with the buffer you allocated. A length value larger than the allocation is either a firmware bug or an under-allocation — never read beyond the allocated buffer.

### MODE SELECT and LOG SELECT: Read Before Write

SCSI explicitly requires that you **read a mode page or log page before issuing MODE SELECT or LOG SELECT** to modify it. This is not a convention — it is a protocol requirement designed to prevent unintended modifications to fields you are not changing. The correct sequence:

1. Issue MODE SENSE / LOG SENSE and save the full response.
2. Modify only the specific field(s) you intend to change.
3. Issue MODE SELECT / LOG SELECT with the modified data.

Skipping the read step risks overwriting drive configuration fields with stale or zero values. There is a limited exception for LOG SELECT operations that only set a reset bit (some pages allow resetting counters without a prior read), but the read-first rule applies in general.

Note also that when switching between MODE SENSE 6 and MODE SENSE 10, the mode parameter header is a different length — the 6-byte header is 4 bytes and the 10-byte header is 8 bytes. This shifts the offset of all mode page data that follows. The same header-length difference applies in MODE SELECT. If you change the CDB size, you must also adjust the header and all offsets into the parameter data accordingly.

---

## CDB Byte-Field Conventions

SCSI CDB fields are big-endian. Use `M_Byte1(val)`, `M_Byte0(val)` in descending byte order to fill multi-byte fields:

```c
// Fill a 4-byte LBA field at CDB[2..5] with big-endian ordering:
uint32_t lba = ...;
cdb[2] = M_Byte3(lba);    // most significant
cdb[3] = M_Byte2(lba);
cdb[4] = M_Byte1(lba);
cdb[5] = M_Byte0(lba);    // least significant

// Fill a 2-byte transfer length at CDB[7..8]:
uint16_t transferLen = ...;
cdb[7] = M_Byte1(transferLen);
cdb[8] = M_Byte0(transferLen);
```

This reflects the big-endian wire format mandated by the SCSI standards. Never use `memcpy` of a host-endian integer directly into a CDB byte — it will silently produce wrong results on little-endian hosts (x86, ARM).

---

## Logical Block Guard (PI)

When a drive is formatted with Protection Information (T10 DIF), each logical block has an 8-byte descriptor appended (Logical Block Guard, Application Tag, Reference Tag). PI is designed for end-to-end data integrity checking from the application layer through the hardware stack.

- `calculate_Logical_Block_Guard(buf, userDataLen, totalBufLen)` — computes the CRC-16 guard value for one sector.
- The total buffer size is `sectorSize + 8` per sector.
- Check `device->drive_info.currentProtectionType` before assuming PI is active.

**PROTECT field**: Each read/write CDB contains a RDPROTECT or WRPROTECT field that controls whether the drive and/or host checks the Logical Block Guard and Reference Tag fields. Different values enable or disable guard and tag checking independently. Refer to the SBC specification for the full PROTECT field semantics.

PI is rarely used in practice, even in cloud and enterprise deployments. It requires coordinated support through the HBA, OS driver, and application, and adds 8 bytes of overhead per sector. Most deployments rely on the drive’s internal ECC and transport-layer CRC alone.

---

## Common Pitfalls

- **Do not hardcode CDB byte indices** with magic numbers. Use `CDB_OPERATION_CODE`, `CDB10_CONTROL`, `CDB16_CONTROL`, etc.
- **Check the SCSI version** before using SPC-4/5 features. Some targets only support SPC-3.
- **Unit Attention handling is not always a simple retry**: The ASC/ASCQ pair identifies the cause. Some conditions clear automatically; others (e.g., MRIE-triggered informational exceptions) may recur or require manual intervention.
- **Sense data is only valid when the status is CHECK CONDITION**. Do not parse sense data on a command that completed with GOOD status.
- **Variable-length CDBs** (7Fh) and vendor-unique CDBs (C0h–FFh) have no standardized length — document your CDB structure clearly.
- **SAM status gap**: The transport layer currently does not propagate the SAM (SCSI Architecture Model) status byte from the OS up the call stack. When sense data is empty, there may be no additional error information available. This is a known gap to be addressed.
- **SCSI sizes are almost always in bytes**, with some exceptions (descriptor counts). Always validate a reported length or count against your buffer allocation before using it as an offset or loop bound.

---

## SCSI Architecture and Standards History

### Origins: SASI → SCSI

SCSI (Small Computer System Interface) evolved from SASI (Shugart Associates System Interface). The oldest known SASI references appear in controller specifications and documents from around 1978. SASI was largely compatible with early SCSI but had differences, particularly in sense data format.

Real SASI devices will essentially never be encountered in practice today. However, the fundamental concepts SASI introduced have been present in every SCSI generation since: logical block addressing, a request/response protocol, and a separate sense mechanism for error detail. CHS (cylinder/head/sector) addressing was never part of the SASI/SCSI access model — it appears only in certain mode pages and defect list formats (e.g., head landing zones, grown defect lists on old HDDs), never as a direct block access mechanism.

### Standards Organization

SCSI originally combined physical bus and command set in a single monolithic specification. With the SCSI-3/SPC family, the standard was restructured into device-type-specific companion specs:

| Spec | Device type |
|------|-------------|
| SPC (SCSI Primary Commands) | Common commands for all device types |
| SBC (SCSI Block Commands) | Block devices: HDDs, SSDs |
| SSC (SCSI Stream Commands) | Tape |
| SMC (SCSI Medium Changer Commands) | Tape libraries |
| SES (SCSI Enclosure Services) | Storage enclosures |

For block storage: consult SPC for common commands (INQUIRY, REQUEST SENSE, MODE SENSE/SELECT, LOG SENSE, REPORT LUNS) and SBC for block-access commands (READ, WRITE, FORMAT UNIT, etc.).

**SCSI Enclosure Services (SES)**: SES allows the host to communicate with a storage enclosure’s management processor to control and monitor components such as fans, power supplies, temperature sensors, and drive slot LEDs. SES commands use the SEND DIAGNOSTIC / RECEIVE DIAGNOSTIC RESULTS CDB pair with SES-specific diagnostic page codes. **SES support is largely unimplemented in this codebase.** Implementing SES would be a significant capability addition for tools that manage enclosures or JBODs, and is a known gap.

**SCSI Object-Based Storage (OSD)**: T10 standardized an Object-Based Storage Device command set (OSD) that allows accessing storage by named objects rather than by LBA — conceptually similar to a key-value store. Seagate’s Kinetic drives pursued a related concept using a proprietary key-value protocol over Ethernet (not standard SCSI OSD). Neither OSD nor Kinetic achieved significant industry adoption and both have faded. NVMe 2.0 introduces a Key-Value (KV) command set that explores the same space. None of these are implemented in this codebase.

### Service Actions

Modern SCSI CDBs use a two-level addressing scheme. The operation code selects the command class; a **service action** field (typically 16 bits) selects the specific operation within that class. Example: READ CAPACITY(10) uses opcode 0x25 with no service action; READ CAPACITY(16) uses opcode 0x9E with service action 0x10. Always check whether a command uses a service action before assuming the operation code alone identifies it uniquely.

### Market Context

SCSI was historically more expensive and more reliable than ATA, targeting enterprises requiring high uptime: banks, financial institutions, applications that could not tolerate downtime. The physical transport has evolved (parallel SCSI → Fibre Channel → SAS) but the philosophy of rich error reporting, multiple logical units, and dual-port connectivity has remained.

Today, nearline SAS HDDs (7200 RPM) are common in cloud and enterprise deployments. The 10K–15K RPM “mission critical” SAS HDDs have been largely displaced by SSDs. Some cloud operators buy SATA for cost; others specify SAS for its reliability features and dual-port capability.

### Command Queuing Depth

SCSI does not have a standardized field that declares a target’s maximum queue depth. The SAM (SCSI Architecture Model) defines command queuing conceptually, but the maximum number of commands a target can hold outstanding at once is a device-specific property, typically documented in the product’s product manual or returned implicitly when the host exceeds it (the target will respond with TASK SET FULL status). The highest queue depth practically observed in SAS/SCSI targets is 256.

### OCP (Open Compute Project) and SAS/SCSI

The Open Compute Project is a consortium of hyperscale cloud operators and storage vendors that publishes additional mandatory requirements layered on top of existing storage standards for data center use. OCP has specifications covering SAS drives as well as SATA and NVMe, mandating certain feature support, telemetry reporting, and log pages beyond what the base SCSI/SAS standards require. Generic OCP support across all interfaces (including SAS) is a known future improvement goal for this codebase.

---

## Command Support Detection

### The Classic SCSI Approach: “Just Try It”

SCSI has no single authoritative feature support table equivalent to ATA IDENTIFY DEVICE. The traditional practice is to issue a command and observe whether the device returns ILLEGAL REQUEST or succeeds.

Zero-length transfers on 10/12/16-byte commands were defined to enable this probing without moving data, but they are not reliable through SATLs, hardware USB bridges, or NVMe translators. A better approach for native SCSI/SAS targets is to **use a reasonable transfer length** (512 bytes or one logical sector) rather than zero — this avoids zero-length semantics while still issuing the smallest meaningful transfer.

Another pattern used in this codebase is **CDB-size fallback**: attempt a command with the larger CDB (e.g., 16-byte READ), and if it returns ILLEGAL REQUEST, retry with the next smaller CDB size (e.g., 10-byte READ). This is implemented for READ, WRITE, VERIFY, SYNCHRONIZE CACHE, WRITE SAME, and similar commands. When changing CDB size affects the header format as well as the opcode (e.g., MODE SENSE 6 vs. 10), you must also adjust parameter data offsets — the headers are different lengths and the page data shifts accordingly. In general, if a 10-byte READ is supported, a 10-byte WRITE will be too. If 10-byte MODE SENSE is supported, 10-byte MODE SELECT is too.

This approach has caused friction — OS drivers may log failed probe attempts as errors even when the behavior is expected. Newer SCSI standards are increasingly adding explicit capability bitfields (mirroring ATA's approach).

### Security Protocol Commands and the Inc512 Bit

SECURITY PROTOCOL IN / OUT (opcode 0xA2 / 0xB5) carry TCG (Opal, Enterprise) and other security payloads. The CDB contains an **INC_512** bit that changes how the transfer length field is interpreted:

- **INC_512 = 0**: transfer length is in bytes.
- **INC_512 = 1**: transfer length is in 512-byte blocks.

The 512-block mode is generally well supported, including through SAT translators, and is the preferred mode for most uses. The exception is certain NVMe translators (SNTL), where Inc512 support is more variable.

When using the bytes mode for TCG, be aware that SAS drives often still expect the transfer to be padded to a 512-byte boundary, even if the actual TCG packet is smaller or not a multiple of 512 bytes. Allocating a 512-byte-aligned buffer and issuing the full 512-byte transfer avoids this mismatch.

### Methods for Determining Command Support

| Method | When it works | Limitations |
|--------|--------------|-------------|
| Issue and observe (with reasonable transfer size) | Native SCSI/SAS targets | Unreliable through SATL/SNTL/USB; SAT translators often mishandle zero-length commands |
| CDB-size fallback (try 16B → 10B → 6B) | Any target | Mode sense/select require header offset adjustment when switching sizes |
| REPORT SUPPORTED OPERATION CODES (0xA3 / service action 0x0C) | Native SCSI/SAS; also in the software translator in `sat_helper.c` | Rarely available on USB bridges or hardware SATLs |
| Extended INQUIRY VPD page (0x86) | Many modern drives | Contains specific bitfields (e.g., supported firmware download modes) |
| SCSI Feature Sets VPD page | Very new devices | Lists supported feature sets by code; not yet widely used in this codebase |
| INQUIRY `cmdDT` field | Theoretically defined in older SPC | Never observed to be implemented in real hardware; effectively defunct |

---

## Multiple Logical Units, Dual Port, and Reservations

### OS Device Handles per Logical Unit

In every OS supported by this codebase, the storage stack assigns **one device handle per logical unit**. There is no shared handle for a multi-LU target — each LU appears as a completely independent device node.

This surprises people in two common situations:

- **Dual-port SAS drive**: the drive exposes the same single LU through two physical ports. The OS may enumerate it twice, once per port, producing two device handles pointing at the same underlying LU. The user sees the drive listed twice with the same capacity.
- **Dual-actuator SAS drive**: each actuator is a separate LU. The OS enumerates two device handles with roughly half the capacity each. The user sees two drives and wonders why their capacity is halved.

When discovering devices, be aware that the same physical drive can produce multiple handles, and that filtering or deduplication may be needed depending on the use case.

### Multiple Logical Units

SCSI supports multiple logical units (LUs) per target. A SATA target always has exactly one LU; SAS targets can expose multiple.

A common real-world use case is a **dual-actuator SAS HDD**: each physical actuator is exposed as a separate LU, allowing the host to address both halves of the drive independently. This is the standard configuration for dual-actuator SAS drives.

SATA dual-actuator drives cannot expose multiple LUs, so they use the **Concurrent Positioning Ranges** log page to describe each actuator's LBA range. SCSI later added an equivalent VPD page for the same purpose on SAS.

### Dual Port

SAS targets support two physical ports, each of which can connect to a different initiator host. A SAS drive may be accessed concurrently by two separate host systems — a capability absent from SATA.

A theoretical dual-actuator dual-port drive would produce four OS device handles (two actuator LUs × two ports). No such drive exists today, but the enumeration model follows directly from these two independent features.

### Reservations

SCSI reservations (PERSISTENT RESERVE OUT / PERSISTENT RESERVE IN, and the older RESERVE/RELEASE) allow a host to claim exclusive or restricted access to a LU, preventing another initiator connected through the other port from conflicting. Both hosts must participate cooperatively.

NVMe has an analogous concept: multiple NVMe controllers can attach to the same namespace, and NVMe Reservations provide the equivalent coordination mechanism.
