---
description: 'opensea-transport library — tDevice architecture, OS passthrough model, verbosity, command dispatch, and conventions for adding new protocol commands'
applyTo: 'subprojects/opensea-transport/**/*.c, subprojects/opensea-transport/**/*.h'
---

# opensea-transport — Architecture and Conventions

## Purpose

opensea-transport is the command transport layer. It owns:

- The `tDevice` structure — the single opaque handle used by all layers to issue commands to any supported device
- Command-context structures (`ataPassthroughCommand`, `ScsiIoCtx`, `nvmeCmdCtx`) — filled in by protocol helpers and passed down to OS passthrough
- OS-specific passthrough implementations (`win_helper.c`, `sg_helper.c`, `cam_helper.c`, `netbsd_openbsd_helper.c`, `uscsi_helper.c`, etc.)
- Protocol-level command builders and helpers (ATA, SCSI, NVMe, SAT/SNTL, USB bridge quirks)
- A thin cross-protocol command layer (`cmds.h`) for operations whose logic is shared across ATA/SCSI/NVMe

---

## Design Philosophy — "Everything is SCSI"

The foundational design principle of opensea-transport is that **every device can be treated as a SCSI device**. A SCSI CDB can be issued to any device handle — ATA, NVMe, or SCSI — and the library will either pass it to a translator below it or translate it internally. The caller does not need to know the underlying protocol to issue well-known SCSI commands.

This is not a theoretical abstraction. It reflects the dominant real-world architecture of storage stacks:

- **OS kernel drivers** (AHCI on Windows and Linux, CAM on FreeBSD) internally translate SCSI commands to ATA before issuing them to a SATA drive. The OS exposes a SCSI interface upward regardless of whether the physical device speaks ATA.
- **HBAs and SAS expanders** with SATA backplanes run SAT (SCSI-ATA Translation) in firmware to bridge between the SCSI initiator model and the ATA device model.
- **USB bridge chips** always speak SCSI toward the host regardless of what is behind them — SATA, NVMe, or SD. SAT is used when ATA passthrough is needed; otherwise all I/O is SCSI.
- **NVMe**: AHCI-to-NVMe transitions still involve an SNTL (SCSI-to-NVMe Translation Layer) in many situations — see `sntl_helper.c` and the NVMe instruction file.

The consequence for this codebase: the `ScsiIoCtx` path (`scsi_Send_Cdb`) is the most widely exercised path in the library and the one with the most real-world translation coverage. Basic I/O — reads and writes — can almost always travel the SCSI path with confidence. Configuration operations (power state timers, write cache policy, error recovery settings, protocol-specific feature bits) typically require native ATA or NVMe commands because the SCSI equivalents either do not map cleanly or the translator may not implement them. The limit is always translator coverage — translations are not complete, and some operations (ATA SMART sub-commands, NVMe-specific log pages, ATA security features) have no SCSI equivalent and must use the native path. The philosophy is useful context for understanding why the library is structured the way it is, not a rule about which command variant to choose for a given operation.

---

## The `tDevice` Structure

`tDevice` (defined in `common_public.h`) is the central device handle passed to every function in this library and in opensea-operations. Never embed protocol-specific handles directly in application code — always go through `tDevice`.

Key fields:

| Field | Type | Purpose |
|-------|------|---------|
| `os_info` | `OSDriveInfo` | OS-level handle (fd, HANDLE, cam union, etc.), device name, friendly name, interface type, drive type |
| `drive_info` | `driveInfo` | Identify data, serial number, model, firmware, sector size, max LBA, supported features |
| `deviceVerbosity` | `eVerbosityLevels` | Controls debug output; see verbosity levels below |
| `verboseOutputStream` | `FILE*` | Destination for verbose output — can be stdout, stderr, a file, or a pipe |
| `delay_io` | `uint32_t` | Milliseconds to sleep between commands — a CLI-level option that lets customers limit the tool’s IO impact on other workloads running concurrently. Not USB-specific; applies globally to every command issued through this device handle. |
| `issue_io` | function pointer | Override passthrough for RAID / custom drivers |
| `issue_nvme_io` | function pointer | Override passthrough for NVMe RAID / custom drivers |
| `dFlags` | `uint64_t` | Device-level flags (capabilities, quirks) |

**Rules**:
- Always pass `const tDevice*` when a function does not modify device state.
- Pass `tDevice*` (non-const) only when the function may update `drive_info`, `os_info`, or the device handle itself (e.g., `get_Device`, `close_Device`, `fill_Drive_Info_Data`).
- Never access `os_info.fd`, `os_info.fd_secondary`, or other OS fields directly from opensea-operations — call through the passthrough functions in this layer.

---

## Verbosity Levels

`eVerbosityLevels` controls how much debug output is generated:

| Level | Constant | Typical use |
|-------|----------|-------------|
| 0 | `VERBOSITY_QUIET` | No output at all |
| 1 | `VERBOSITY_DEFAULT` | Normal user-facing output |
| 2 | `VERBOSITY_COMMAND_NAMES` | Print command names as they are issued |
| 3 | `VERBOSITY_COMMAND_VERBOSE` | Print full command context (registers, sense data, etc.) |
| 4 | `VERBOSITY_BUFFERS` | Print raw data buffer hex dumps |

Use the `print_tDevice_*` family of functions — do not call `printf` or `fprintf` directly in transport layer code:

```c
// Simple message
print_tDevice_Verbose_String(device, VERBOSITY_COMMAND_NAMES, "Issuing ATA IDENTIFY\n");

// Formatted message
print_tDevice_Verbose_Formatted_String(device, VERBOSITY_COMMAND_VERBOSE,
    "LBA: %" PRIu64 " sectors: %" PRIu32 "\n", lba, sectorCount);

// Flush buffered output (see note below)
flush_tDevice_Verbose_Stream(device);

// Print raw buffer
print_tDevice_Data_Buffer(device, VERBOSITY_BUFFERS, dataBuf, dataLen, true);

// Print command elapsed time
print_Command_Time_Verbose(device, VERBOSITY_COMMAND_VERBOSE, elapsedNanoSeconds);
```

`flush_tDevice_Verbose_Stream` is not required after every print call, but call it when:
- About to issue an IOCTL that may take a significant amount of time — ensures all debug output is visible before the kernel blocks
- About to perform a potentially dangerous or destructive operation — ensures all context is flushed before the OS may become unresponsive

---

## OS Passthrough Selection

`platform_helper.h` includes the correct OS passthrough header at compile time:

| OS | Header | Mechanism |
|----|--------|-----------|
| Linux | `sg_helper.h` | `SG_IO` ioctl (SCSI Generic v3) |
| Windows | `win_helper.h` | `SCSI_PASS_THROUGH_DIRECT`, `STORAGE_PROTOCOL_COMMAND`, and various `STORAGE_`/`DISK_` IOCTLs. `IOCTL_ATA_PASS_THROUGH` is documented and used for legacy ATA passthrough. Older IDE variants (pre-`IOCTL_ATA_PASS_THROUGH`) were never formally documented by Microsoft. `SMART_SEND_DRIVE_COMMAND` / `SMART_RCV_DRIVE_DATA` IOCTLs are also supported as a last-resort fallback for old devices or drivers where generic passthrough is not available. |
| FreeBSD / DragonFlyBSD | `cam_helper.h` | CAM (Common Access Method) |
| NetBSD / OpenBSD | `netbsd_openbsd_helper.h` | Two separate device node families: `/dev/rsd*` (raw SCSI, `scsireq_t` + `SCIOCCOMMAND`) and `/dev/rwd*` (raw ATA via the `wd` driver, `atareq_t` + `ATAIOCCOMMAND`). ATA passthrough is 28-bit only — any nonzero 48-bit TFR field is rejected at the OS layer. NVMe is not yet supported. Infinite timeout is not supported. Minor API divergence between the two: `ATACMD_LBA` flag exists in NetBSD but is absent in OpenBSD (guarded by `#ifdef ATACMD_LBA`). |
| Solaris / Illumos | `uscsi_helper.h` | USCSI |
| VMware | `vm_helper.h` | SG v3 compatible, but **not all SG IOCTLs are available** — `SG_GET_VERSION_NUM` is known to fail. Device handles are enumerated differently: each device is identified via the T10 device identification VPD page (SAT SCSI string, WWN, etc.) rather than by OS device node. Linux RAID/HBA driver IOCTLs may work if the same kernel module is loaded, but availability is not guaranteed. |
| UEFI | `uefi_helper.h` | Four EFI protocols, chosen at runtime based on what the firmware exposes: `EFI_SCSI_PASS_THRU_PROTOCOL` (pre-UEFI / EFI 1.x era — predates full standardization, supported for older systems), `EFI_EXT_SCSI_PASS_THRU_PROTOCOL` (modern SCSI/SAS/SATA), `EFI_ATA_PASS_THRU_PROTOCOL` (native ATA), `EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL` (NVMe). |

**Rule**: Never `#include` a specific OS helper directly. Include `platform_helper.h` or use the abstract functions in `ata_helper_func.h`, `scsi_helper_func.h`, and `nvme_helper_func.h`.

### Key IOCTL Structures Per OS

These are the OS-level structures that the helper files fill in. Understanding them is useful when debugging at the OS boundary or when porting to a new OS.

#### Linux — `sg_io_hdr_t` (`SG_IO` ioctl, `<scsi/sg.h>`)

```c
sg_io_hdr_t io_hdr = {0};
io_hdr.interface_id    = 'S';                 /* mandatory magic — always 'S' */
io_hdr.cmd_len         = scsiIoCtx->cdbLength;
io_hdr.cmdp            = scsiIoCtx->cdb;      /* pointer to CDB bytes */
io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;  /* or TO_DEV / NONE / UNKNOWN */
io_hdr.dxfer_len       = scsiIoCtx->dataLength;
io_hdr.dxferp          = scsiIoCtx->pdata;   /* data buffer */
io_hdr.mx_sb_len       = scsiIoCtx->senseDataSize; /* capped at UINT8_MAX */
io_hdr.sbp             = scsiIoCtx->psense;  /* sense buffer */
io_hdr.timeout         = timeoutMs;           /* milliseconds; UINT32_MAX = no timeout */
ret = ioctl(fd, SG_IO, &io_hdr);
/* Success: io_hdr.masked_status == 0 (SAM GOOD); masked_status = (io_hdr.status >> 1) & 0x1f  */
/* Always check masked_status, not the raw status byte — raw status includes reserved bits.       */
/* Also check: io_hdr.host_status == 0 (no adapter/DMA error), io_hdr.driver_status == 0.       */
/* Sense data is valid when masked_status != 0 and io_hdr.sb_len_wr > 0.                        */
```

#### Windows — `SCSI_PASS_THROUGH_DIRECT` (SCSI commands)

The codebase implements both `SCSI_PASS_THROUGH_DIRECT` (`IOCTL_SCSI_PASS_THROUGH_DIRECT`) and the non-DIRECT
variant (`SCSI_PASS_THROUGH` / `IOCTL_SCSI_PASS_THROUGH`). **Always use DIRECT.** The non-DIRECT variant causes
the OS to copy the data through an intermediate kernel buffer (double-buffering), and for read commands the driver
may satisfy the request from its internal cache rather than issuing a real command to the device. ATA IDENTIFY
via non-DIRECT passthrough, for example, commonly returns the miniport's cached copy of the identify data, not a
freshly executed command result. DIRECT bypasses both the double-buffer and the cache — you get exactly what the
drive returns. The same rationale applies to `ATA_PASS_THROUGH_EX` vs. `ATA_PASS_THROUGH` (non-DIRECT).

```c
/* Buffer layout: [SCSI_PASS_THROUGH_DIRECT][sense data][data buffer (if needed)] */
SCSI_PASS_THROUGH_DIRECT sptd = {0};
sptd.Length             = sizeof(SCSI_PASS_THROUGH_DIRECT);
sptd.CdbLength          = cdbLength;           /* 1–16 */
sptd.SenseInfoLength    = senseLength;         /* typically 32 or 64 */
sptd.DataIn             = SCSI_IOCTL_DATA_IN;  /* or DATA_OUT / UNSPECIFIED */
sptd.DataTransferLength = dataLength;
sptd.TimeOutValue       = timeoutSeconds;
sptd.DataBuffer         = pdata;               /* direct pointer — no offset needed */
sptd.SenseInfoOffset    = offsetof(sptd_with_buf, sense); /* byte offset from the start of the DeviceIoControl
                                                              input buffer (SPTD is always at byte 0, so this equals
                                                              the offset from the struct itself; sptd_with_buf is a
                                                              local composite type, not a Windows SDK type) */
memcpy(sptd.Cdb, cdb, cdbLength);
DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
    &sptd, sizeof(sptd), &sptd, sizeof(sptd), &returned, NULL);
```

#### Windows — `ATA_PASS_THROUGH_EX` (ATA commands, `IOCTL_ATA_PASS_THROUGH`)

```c
/* PreviousTaskFile = high/extended bytes (written first to shadow register)   */
/* CurrentTaskFile  = low/current bytes  (written second, trigger on Command)  */
ATA_PASS_THROUGH_EX aptex = {0};
aptex.Length              = sizeof(ATA_PASS_THROUGH_EX);
aptex.AtaFlags            = ATA_FLAGS_DATA_IN | ATA_FLAGS_DRDY_REQUIRED; /* or DATA_OUT / NO_MULTIPLE */
aptex.DataTransferLength  = dataLength;
aptex.TimeOutValue        = timeoutSeconds;
aptex.DataBufferOffset    = sizeof(ATA_PASS_THROUGH_EX); /* offset from struct start */
/* High (extended/48-bit) bytes: */
aptex.PreviousTaskFile[0] = tfr.ErrorFeature48;  /* Feature high */
aptex.PreviousTaskFile[1] = tfr.SectorCount48;
aptex.PreviousTaskFile[2] = tfr.LbaLow48;
aptex.PreviousTaskFile[3] = tfr.LbaMid48;
aptex.PreviousTaskFile[4] = tfr.LbaHi48;
/* Low (current) bytes: */
aptex.CurrentTaskFile[0]  = tfr.ErrorFeature;    /* Feature low / opcode sub-command */
aptex.CurrentTaskFile[1]  = tfr.SectorCount;
aptex.CurrentTaskFile[2]  = tfr.LbaLow;
aptex.CurrentTaskFile[3]  = tfr.LbaMid;
aptex.CurrentTaskFile[4]  = tfr.LbaHi;
aptex.CurrentTaskFile[5]  = tfr.DeviceHead;
aptex.CurrentTaskFile[6]  = tfr.CommandStatus;   /* command opcode — triggers execution */
DeviceIoControl(hDevice, IOCTL_ATA_PASS_THROUGH, &aptex, totalSize, &aptex, totalSize, &returned, NULL);
/* RTFRs on return are in aptex.CurrentTaskFile[6:0] and aptex.PreviousTaskFile[4:0] */
```

#### FreeBSD / DragonFlyBSD — CAM (`<cam/cam.h>`, `<cam/scsi/scsi_all.h>`)

```c
union ccb ccb;
memset(&ccb, 0, sizeof(ccb));
cam_fill_csio(
    &ccb.csio,
    /* retries */       1,
    /* cbfcnp */        NULL,
    /* flags */         CAM_DIR_IN,          /* or CAM_DIR_OUT / CAM_DIR_NONE */
    /* tag_action */    MSG_SIMPLE_Q_TAG,
    /* data_ptr */      pdata,
    /* dxfer_len */     dataLength,
    /* sense_len */     SSD_FULL_SIZE,
    /* cdb_len */       cdbLength,
    /* timeout */       timeoutMs   /* milliseconds — same unit as Linux SG_IO; Windows uses seconds */
);
memcpy(ccb.csio.cdb_io.cdb_bytes, cdb, cdbLength);
ioctl(camfd, CAMIOCOMMAND, &ccb);
/* Status: ccb.ccb_h.status & CAM_STATUS_MASK == CAM_REQ_CMP for success */
/* Sense data in ccb.csio.sense_data when (ccb.ccb_h.status & CAM_AUTOSNS_VALID) is set */
```

#### Solaris / Illumos — USCSI (`<sys/scsi/impl/uscsi.h>`)

```c
struct uscsi_cmd cmd = {0};
cmd.uscsi_cdb     = (caddr_t)cdb;
cmd.uscsi_cdblen  = cdbLength;
cmd.uscsi_bufaddr = (caddr_t)pdata;
cmd.uscsi_buflen  = dataLength;
cmd.uscsi_rqbuf   = (caddr_t)senseBuf;
cmd.uscsi_rqlen   = senseLength;
cmd.uscsi_flags   = USCSI_READ | USCSI_RQENABLE | USCSI_SILENT; /* or USCSI_WRITE */
cmd.uscsi_timeout = timeoutSeconds;
ioctl(fd, USCSICMD, &cmd);
/* Status: cmd.uscsi_status (SCSI status byte); sense in rqbuf if uscsi_rqresid < rqlen */
```

#### NetBSD / OpenBSD — SCSI path (`scsireq_t`, `SCIOCCOMMAND`, `<sys/scsiio.h>`)

Use `/dev/rsd*` device nodes. SCSI address can be queried with `SCIOCIDENTIFY`.

```c
scsireq_t scsicmd;
M_INITIALIZE_STRUCTURE(&scsicmd, sizeof(scsireq_t));
scsicmd.flags    = SCCMD_READ;               /* or SCCMD_WRITE / SCCMD_ESCAPE */
scsicmd.cmdlen   = cdbLength;
safe_memcpy(scsicmd.cmd, sizeof(scsicmd.cmd), cdb, cdbLength);
scsicmd.databuf  = pdata;
scsicmd.datalen  = dataLength;
scsicmd.senselen = senseLength;
scsicmd.timeout  = timeoutMs;               /* milliseconds */
ioctl(fd, SCIOCCOMMAND, &scsicmd);
/* Status: scsicmd.retsts (SCCMD_OK / SCCMD_SENSE / SCCMD_BUSY / SCCMD_TIMEOUT) */
/* Sense: scsicmd.sense[0..senselen-1] when retsts == SCCMD_SENSE               */
```

#### NetBSD / OpenBSD — ATA path (`atareq_t`, `ATAIOCCOMMAND`, `<sys/ataio.h>`)

Use `/dev/rwd*` device nodes (the `wd` driver). **28-bit only** — any nonzero 48-bit TFR
field (`SectorCount48`, `Feature48`, `LbaLow48`, `LbaMid48`, `LbaHi48`, `aux*`) causes the
passthrough layer to return `OS_COMMAND_NOT_AVAILABLE` before the ioctl is issued.

```c
atareq_t atacmd;
M_INITIALIZE_STRUCTURE(&atacmd, sizeof(atareq_t));
attacmd.flags    = ATACMD_READREG;           /* always request RTFRs */
attacmd.flags   |= ATACMD_READ;             /* or ATACMD_WRITE; omit for non-data */
#if defined(ATACMD_LBA)                     /* NetBSD only — flag absent on OpenBSD */
if (tfr.DeviceHead & LBA_MODE_BIT)
    atacmd.flags |= ATACMD_LBA;
#endif
attacmd.command  = tfr.CommandStatus;
attacmd.features = tfr.ErrorFeature;
attacmd.sec_count = tfr.SectorCount;
attacmd.sec_num  = tfr.SectorNumber;        /* LBA low 8 bits in LBA mode */
attacmd.head     = tfr.DeviceHead;
attacmd.cylinder = M_BytesTo2ByteValue(tfr.CylinderHigh, tfr.CylinderLow);
attacmd.databuf  = pdata;
attacmd.datalen  = dataLength;
attacmd.timeout  = timeoutMs;              /* milliseconds; INT_MAX = no timeout */
ioctl(fd, ATAIOCCOMMAND, &atacmd);
/* RTFRs on return are in atacmd.{command,features,sec_count,sec_num,head,cylinder} */
```

These structures are all superficially different but map to the same five fields: **CDB**, **data direction + buffer**, **sense buffer**, **timeout**, and **status out**. Any new OS port follows the same pattern: find the OS IOCTL that accepts these five inputs and adapt the helper accordingly. The RAID-specific Windows pattern (`IOCTL_SCSI_MINIPORT` + `SRB_IO_CONTROL`) follows the same concept but wraps a vendor-specific sub-command inside — see the RAID instruction file for details.

---

## Cross-Protocol Dispatch (`cmds.h`)

`cmds.h` defines commands that are shared across ATA, SCSI, and NVMe because their logic is identical at the intent level even if the wire encoding differs:

- `fill_Drive_Info_Data(device)` — identify device; dispatches to ATA IDENTIFY, SCSI INQUIRY/READ CAPACITY, or NVMe IDENTIFY CONTROLLER depending on `device->drive_info.drive_type`
- `send_Sanitize_Block_Erase`, `send_Sanitize_Crypto_Erase`, `send_Sanitize_Overwrite_Erase` — sanitize dispatched by protocol
- `spin_down_drive` — ATA STANDBY IMMEDIATE or SCSI START STOP UNIT

A command in `cmds.h` does not require support from every protocol. The threshold is **at least two interfaces or protocols** sharing the concept at the intent level. For example, pseudo-uncorrectable sectors exist on SATA and SAS but have no equivalent on NVMe — a `cmds.h` wrapper that dispatches to ATA and SCSI only is still appropriate because it provides a single callable location rather than duplicating the dispatch logic.

Separate protocol-specific subtleties into the per-protocol helpers. Do not combine different commands into a `cmds.h` wrapper just because they happen to have similar names across protocols.

---

## Adding a New Command to opensea-transport

1. **Identify which protocol layer** the command belongs to:
   - ATA-specific → `ata_cmds.c` + declare in `ata_helper_func.h`
   - SCSI-specific → `scsi_cmds.c` + declare in `scsi_helper_func.h`
   - NVMe-specific → `nvme_cmds.c` + declare in `nvme_helper_func.h`
   - Cross-protocol abstraction → `cmds.c` + declare in `cmds.h`

2. **Fill the command context structure** with all required fields. Zero-initialize before setting individual fields — the structures contain `uint8_t*` members that must be either valid or `M_NULLPTR`. Two equivalent options:
   - `safe_memset(&ctx, sizeof(ctx), 0, sizeof(ctx))` — bounds-checked (check the return value)
   - `explicit_zeroes(&ctx, sizeof(ctx))` or `M_INITIALIZE_STRUCTURE(&ctx, sizeof(ctx))` — no return code check required

3. **Always validate inputs** before filling the context. Return `BAD_PARAMETER` from `eReturnValues` for null or out-of-range arguments. Do not silently truncate or ignore bad inputs.

4. **Set a timeout**. Use `DEFAULT_COMMAND_TIMEOUT` or `0` — the platform passthrough layer promotes `0` to the default automatically. Use a larger explicit value only when the command can legitimately take longer (e.g., a full overwrite pass, a firmware download chunk). Pass the value through the `timeout` field in the command context.

5. **Call the appropriate dispatch function** (`ata_Passthrough_Command`, `scsi_Send_Cdb`, `nvme_Cmd`) — do not call OS passthrough helpers directly from command builders.

6. **Check the return value** of every dispatch call and propagate errors. Do not return `SUCCESS` if the transport layer returned `FAILURE` or `OS_PASSTHROUGH_FAILURE`. There are a small number of legitimate exceptions where ignoring a return is intentional — for example, in opensea-operations after writing a pseudo-uncorrectable sector, the subsequent read used to confirm the entry was added to the pending defect list can have its return ignored because the real goal (triggering the reallocate mechanism) has already been achieved. Document any such intentional ignore with a comment explaining why.

7. **Document the command spec reference** in a Doxygen comment. Include:
   - The version of the standard that **introduced** the command and, when relevant, the version that **deprecated or obsoleted** key sub-features (e.g., SMART was introduced in ATA-3; by ACS-3 many mandatory sub-functions were obsoleted in the spec, though firmware may still implement them).
   - The acronym used in code alongside its full name (e.g., `HPA` = Host Protected Area, `AMAC` = Accessible Max Address Configuration — the replacement for HPA). Place acronym expansions in the `\brief` or `\note` tag so they are visible in generated documentation.

---

## USB Bridge Quirks

USB bridge chips sit between the host OS and the ATA/SCSI/NVMe protocol. The host OS **always communicates with a USB storage device using the SCSI command set** — read and write requests are routed through SCSI CDBs. Only use ATA or NVMe passthrough for capabilities that are not available via native SCSI (e.g., ATA IDENTIFY, SET FEATURES, SMART sub-commands). This approach reuses the translations that bridge firmware is required to implement and produces better compatibility than trying to passthrough raw read/write commands.

### USB Device Detection

Device-type detection proceeds through a layered hierarchy before the VID/PID lookup is attempted:

1. **OS-level interface type** (most reliable, used where available):
   - **Windows**: the device enumeration API reports the interface type and VID/PID directly.
   - **Linux**: `readlink /sys/class/scsi_generic/sgX` reveals the bus in the resolved path — a USB device's path contains `usb` and a USB port address; an ATA/SATA device's path contains `ata`; a FireWire device's path contains `fw`; a pure SCSI device has no interface prefix.
   - **FreeBSD**: partially detectable; not always available.
   - **Solaris/Illumos, OpenBSD, NetBSD, DragonFlyBSD**: OS-level USB interface detection and VID/PID lookup are **not yet implemented**. These platforms fall back entirely to live probing. Implementing VID/PID detection here would be the largest single improvement to USB support on those platforms.

2. **Handle type screen**: on OSes that issue separate ATA and SCSI device handles, USB devices never receive an ATA handle — they always appear as SCSI. This screens out a miscategorized USB device before it reaches the ATA passthrough path.

3. **SCSI INQUIRY version descriptors** (last resort): after the INQUIRY response is parsed, the transport layer checks for a USB version descriptor in bytes 58–73 of the standard response. Its presence confirms USB even when the OS-level interface type is unknown. Additionally, some USB bridge firmware programs the SCSI VERSION byte (INQUIRY byte 2) to values that satisfy Windows or Apple storage certification test suites — the version byte is therefore a useful secondary signal about which compatibility profile the firmware was designed to meet.

### The `passthrough_hacks` Structure

The primary mechanism for adapting to bridge-specific behavior is `device->drive_info.passThroughHacks`. Although most fields were created to handle USB bridge quirks, the structure is **not USB-exclusive** — the same mechanism handles HBA firmware quirks, OS driver limitations, and any other host-side adapter that requires behavioral workarounds. It is populated from a VID/PID lookup table during device enumeration. It controls which passthrough variant to attempt and what limitations to apply. Key principles:

- **Hacks change default behavior only when explicitly set.** If a flag is not set, do not apply the corresponding quirk.
- The default **SAT passthrough** CDB size is **12-byte** (`ATA PASS-THROUGH 12`), not 16-byte. Testing across unknown USB VID/PID combinations has shown that 12-byte SAT CDBs succeed more often out-of-the-box with unknown bridges than 16-byte. 16-byte SAT CDBs are used when specifically required (e.g., 48-bit ATA commands that cannot fit in a 12-byte CDB). This 12-byte default applies to SAT only and was established after broad USB compatibility testing.
- For **native SCSI read/write CDBs** (used for all USB read/write traffic), the CDB size selection follows capacity: if the device capacity exceeds 2 TB, 16-byte READ/WRITE commands are supported and required (10-byte CDBs cannot address beyond 2 TB). For devices at or below 2 TB, use 10-byte CDBs. 12-byte SCSI read/write CDBs are rarely supported by USB bridges and should not be used.
- As VID/PID data accumulates and trends become clear, default behavior may be updated. This is how the SAT 12-byte default was established.
- **VID/PID table internals**: the table is statically compiled into `common_public.c`, organized as a per-VID dispatch — one sub-function per vendor ID containing switch/case statements for that vendor's product IDs. `setup_Passthrough_Hacks_By_ID()` dispatches to the appropriate bus-type function (`set_USB_Passthrough_Hacks_By_PID_and_VID()`, `set_IEEE1394_Passthrough_Hacks_By_PID_and_VID()`, or `set_PCI_Passthrough_Hacks_By_PCI_ID()`). When a match is found, `hacksSetByReportedID` is set. Modifying the table requires a source change and rebuild. It is common for the same bridge chip and firmware to appear under multiple PIDs — or even multiple VIDs — because a storage vendor typically works directly with a bridge chip manufacturer to design each USB product, and the same chip board sometimes ships in different housings or under different product names. Entries for different PIDs may therefore be identical or nearly identical; this is intentional. Official documentation for chip-to-product mappings is not consistently available, which is a primary reason the PassthroughTest tool exists.
- **Adding a new device**: run `openSeaChest_PassthroughTest --runPTTest --ptDriveHint ata --ptTypeHint sat` and capture the output. The tool proceeds sequentially through known problem areas in an order that generally resolves them in one pass. The output prints compact short-name notation for each detected capability or limitation (e.g., `UNA RW10 NMP NLP SCTSM RS A1`). The short-names map directly to `passthroughHacks` fields; the mapping is documented in the struct comments in `common_public.h`. Submit the output together with the VID/PID to the project; a developer translates it and adds an entry to the correct VID sub-function.

  After the main test completes, run `--enableHangCmdsTest all` as a follow-up. This tests commands known to hang certain bridges (zero-length reads, SCT GPL page reads, READ REMOTE TDIR read-direction variants, and similar). These tests are kept separate because they can lock up a device that does not handle them gracefully and should only be run after the normal test completes. Results usually do not differ from the first pass, but occasionally reveal additional limitations. In rare cases a device still requires manual step-by-step testing with power cycles between commands to get reliable results, though this has become increasingly uncommon.

  A behavioral fingerprint approach — identifying the bridge chip from observed command responses alone, without a VID/PID match — has been investigated repeatedly but no reliable universal pattern has been found; each candidate fingerprint is eventually broken by a new device. The VID/PID table is what holds, and it is the approach that essentially every other USB passthrough software also relies on.
- **VPD page 89h (ATA Information) for HBA identification**: VPD page 89h is almost always available on SAS HBAs and SATA host adapters; it is almost never supported on USB devices. The page contains SATL identification strings that can indicate the adapter manufacturer and is used for broad adapter-family workaround selection. Its usefulness has a hard limit: two physically identical HBAs running different firmware versions can produce identical identification strings on page 89h while exhibiting different behavior — the page does not encode firmware version in a way that enables fine-grained workaround targeting. VPD 89h is therefore used only where family-level identification is sufficient. A practical example: Adaptec/PMC/Microchip HBAs require all ATA passthrough DMA requests to use the DMA protocol field rather than UDMA IN/OUT; LSI/Avago/Broadcom HBAs accept UDMA mode, which is the same protocol variant commonly used with USB adapters. VPD 89h is sufficient to distinguish these two families reliably, but cannot resolve firmware-version-level differences within a family.
- **Unknown device retry strategy**: when a device is not in the VID/PID table, the code does not fail outright. Where it makes sense, commands are attempted and, on failure, retried with an adjusted variant (different CDB size, DMA instead of PIO, alternative command opcode, smaller transfer). A successful retry populates the relevant `passthroughHacks` fields for the remainder of the session, so subsequent commands go directly to the working approach without re-probing. A VID/PID table entry bypasses this probing entirely and goes straight to the known-good configuration — faster, fewer command attempts, no uncertainty. The retry mechanism is why unlisted devices often "just work"; the table is why listed devices are immediately efficient. When implementing a new passthrough command, always check whether the relevant `passthroughHacks` flag is already set before probing, and structure fallback to participate in this framework rather than hardcoding a single approach.
- **RTFR retrieval levels**: return task file registers carry ATA command status back through sense data. Three capability levels exist, in priority order:
  - `returnResponseInfoSupported = true` — preferred. The SAT "Return Response Information" protocol reliably retrieves full RTFRs regardless of sense data format or truncation.
  - `partialRTFRs = true` — only 28-bit registers are returned. Commands requiring 48-bit RTFRs (e.g., READ NATIVE MAX ADDRESS EXT) do not work correctly. Treat this as a diagnostic signal rather than something with a comprehensive code workaround.
  - `noRTFRsPossible = true` — no task file registers can be retrieved at all. Skip RTFR-dependent commands. Where workarounds exist, use them: SMART health assessment falls back to reading the attribute and threshold pages manually and comparing them rather than relying on SMART RETURN STATUS; READ NATIVE MAX and SCT feature queries may become unavailable.
- **TURF — Test Unit Ready after Failure**: some bridge chips enter an error-throttling state after consecutive unsupported commands, with response latency growing exponentially until the device appears hung. Setting `testUnitReadyAfterAnyCommandFailure` injects a TEST UNIT READY command after each failure to clear the bridge's internal error state. `turfValue` quantifies how many multiples longer than normal these devices take to respond, calibrating the recovery timing library-wide.
- **Multi-sector PIO as a chip-family fingerprint**: whether a bridge correctly handles a READ LOG EXT request that transfers more than 512 bytes in a single PIO command (e.g., reading Device Statistics log 04h entirely in one shot) is a reliable indirect indicator of bridge chip vendor and firmware generation. Most consumer-grade chips handle single-sector PIO correctly but silently truncate or abort multi-sector PIO log reads. This fingerprint distinguishes chip families even when VID/PID lookup fails or is unavailable. Note that this is distinct from the `multiSectorPIOWithMultipleMode` hack, which governs a different multi-sector transfer mechanism.
- **Passthrough transfer length limits**: most modern USB bridges allow large data transfers per passthrough command, but some impose hard limits — 512 bytes per request is the most restrictive, with 8 KiB being another common cap. These per-bridge limits interact with OS-level ceilings that are independent of the hardware: on Windows, all ATA passthrough commands — including those issued through the SCSI IOCTLs — are capped at **64 KB** per request by the OS; SCSI passthrough hits the same ceiling. These caps can prevent certain passthrough operations from working correctly even when the bridge hardware itself would support the transfer. Where a command has both a DMA and a PIO mode variant, prefer DMA — it does not require the host to manage the transfer sector-by-sector and is more tolerant of OS transfer size constraints — but DMA does not eliminate all transfer length problems. Some operations have no viable workaround when the required transfer size exceeds the applicable cap.
- **`someHacksSetByOSDiscovery`**: a flag set primarily on Windows, where OS-level device enumeration reveals the interface type and bus protocol through APIs unavailable on other platforms. When set, it forces 16-byte SAT CDBs (rather than the library's 12-byte default for unknown devices) and applies a small number of additional Windows-specific adjustments. Once set, these adjustments are protected from being overridden by subsequent probing operations. It is less thorough than a VID/PID table match — it establishes the correct SAT passthrough CDB format but does not populate bridge-specific quirk flags the way a full table entry does.
- **`alwaysUseTPSIUForSATPassthrough` / TPSIU**: some USB bridge chips cannot handle the standard SAT `T_LENGTH` field encoding (sector count or features register) and require TPSIU ("Transport Protocol Specific Information Unit") mode instead, which instructs the bridge to read the transfer length from the transport packet directly rather than from any TFR field. This flag is set during the `initial_Identify_Device()` retry sequence (see below) when INVALID FIELD IN CDB errors persist after disabling check condition and switching to the 16-byte SAT CDB. See the ATA instruction file's TPSIU section for the full T_LENGTH value table and all three TPSIU use cases.

### `initial_Identify_Device()` — The Canonical SAT Compatibility Retry Function

`initial_Identify_Device()` in `ata_helper.c` (static, lines 1908–2007) implements the full retry-with-workaround strategy for the initial ATA Identify Device command. Its comment reads: *"This function attempts numerous workarounds to get working identify data (to work around SAT issues)."* It is the primary reference when investigating USB bridge compatibility and the first function to read when a new device fails ATA Identify.

**Retry sequence** (applied only when `hacksSetByReportedID` is false, i.e. the device is not in the VID/PID table):

*On INVALID FIELD IN CDB* (opcode accepted, a field rejected) — three corrections in order:
1. Disable check condition (`alwaysCheckConditionAvailable = false`) → retry
2. Switch to 16-byte 85h SAT CDB (`a1NeverSupported = true`) → retry
3. Enable TPSIU encoding (`alwaysUseTPSIUForSATPassthrough = true`) → retry

*On INVALID COMMAND OPERATION CODE* (opcode itself rejected):
1. Switch to 16-byte 85h SAT CDB (`a1NeverSupported = true`) → retry
2. If USB interface: disable check condition → retry
3. If `retryWithJMicronPT` hint set during enumeration: switch to `ATA_PASSTHROUGH_JMICRON` → retry

`scsi_Test_Unit_Ready()` is issued between every retry (except on IDE interface) to clear error-throttling state in the adapter. **SAS HBAs** succeed on the first call with no retries — these paths are a USB/SATL compatibility mechanism only. When `hacksSetByReportedID` is true, only the JMicron PT retry path is considered; all other hacks are pre-populated correctly from the VID/PID table.

### Legacy Vendor-Specific CDBs

Legacy vendor passthrough CDBs (Cypress, JMicron, SunPlus, and similar) use opcode ranges that are **vendor-unique and undefined for other devices**. Issuing them to an unknown device can cause unpredictable behavior, including bricking USB flash drives. Rules:

- **Never issue a legacy vendor CDB unless another mechanism has already confirmed the bridge vendor.** VID/PID lookup is the primary gate. Do not guess.
- `usb_hacks.h` is being gradually deprecated as the internal `passthrough_hacks` infrastructure matures. Do not extend it with new vendor entries.
- The enum for legacy passthrough type is set via `passthrough_hacks`; once set to a legacy type, the normal SAT hacks are irrelevant since that spec defines its own capability advertisement.

### NVMe over USB

Some USB docks expose NVMe drives via vendor-specific tunneling. This behaves like legacy ATA passthrough: only use it when confirmed supported via VID/PID lookup or capability detection. The existing enumeration code already handles automatic retry and fallback. Do not extend this path without testing on actual hardware.

### Automatic SCSI CDB Size Fallback

CDB size fallback is one specific instance of the broader retry-and-record strategy described above. The transport layer tracks per-device CDB size capability for mode pages, log pages, reads, and writes. If a command returns an ILLEGAL REQUEST / INVALID COMMAND OPERATION CODE sense response, the layer automatically retries with a smaller CDB and records the limit for that device so subsequent commands do not repeat the probe. This mechanism applies to native SAS devices as well as USB.

When adding a new SCSI command that has both a 10-byte and a 16-byte variant (e.g., a new mode sense or log sense path), structure the implementation so it participates in this fallback tracking rather than hardcoding a size. Check whether the relevant `passThroughHacks` flag (or equivalent device-capability field) is already populated before attempting the larger CDB.

### SCSI Version Pre-filter (INQUIRY Version Byte)

The INQUIRY standard response byte 2 (`VERSION`) reports which SCSI standard the device conforms to. The transport layer checks this during enumeration:

- SCSI-2 and earlier — 16-byte CDBs did not exist in that revision. Do not attempt them.
- SPC-3 (version ≥ 5) — 16-byte CDBs are expected to be supported.

When writing code that selects between CDB sizes, respect this cached version value rather than blindly attempting 16-byte first. Future work may extend this to check the INQUIRY version descriptor list (bytes 58–73) for finer-grained capability detection, but that is not yet implemented.

### ATA Passthrough Autosense Limitation (SAT)

When the OS issues an ATA passthrough command, the OS autosense subsystem **always requests fixed-format sense data** (not descriptor format). This matters because the ATA Return Task File Registers (RTFRs) are embedded in the sense data response, and fixed-format sense data has limited space compared to descriptor format.

The practical consequence: for ATA commands that return large LBA values in their RTFRs (e.g., `READ NATIVE MAX ADDRESS EXT`, capacity-reporting commands), fixed-format sense data may not convey the full 48-bit result correctly when the drive capacity falls in the ambiguous range between 28-bit max (128 GiB) and 2 TB. Some host systems allow inhibiting autosense to work around this, but opensea-transport does not currently implement that mechanism. Alternative approaches are used with mixed results. Be aware of this limitation when writing code that depends on RTFR values returned over a SAT connection.
