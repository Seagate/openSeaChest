---
description: 'ATA command building patterns for opensea-transport — ataPassthroughCommand structure, TFR setup, LBA modes, RTFRs, and common ATA command conventions'
applyTo: 'subprojects/opensea-transport/**/*.c, subprojects/opensea-transport/**/*.h'
---

# ATA Command Building — opensea-transport

## ATA Specification Keywords

The ATA specification (ACS-6 §3.3) uses precise keywords that carry exact normative meaning. When reading spec text, do not interpret these as ordinary English words:

| Keyword | Meaning |
|---------|--------|
| **shall** | Mandatory requirement — must be implemented for conformance |
| **should** | Strongly recommended; equivalent to "it is recommended" |
| **may** | Permitted but not required; no implied preference |
| **expected** | Describes design-model behavior; other models may also be implemented |
| **mandatory** | Must be implemented as defined |
| **optional** | Not required; but if implemented, must follow the standard exactly |
| **prohibited** | Must not be supported |
| **reserved** | Set aside for future use; host **must not check** reserved fields; device **shall** return command aborted if a reserved code value is received in a defined field |
| **retired** | Was defined in a previous standard; undefined now; **may** be reclaimed in future standards. If used before reclamation, shall retain its previous meaning |
| **obsolete** | Was defined in a previous standard; undefined now; **shall not** be reclaimed. Obsolete commands may return command aborted; if they do not return abort, they shall return completion |
| **N/A** | Field has no defined value and shall not be checked by host or device |
| **vendor specific** | Not defined by the standard; determined by the device vendor |

The distinction between **retired** and **obsolete** is particularly important: retired bits/fields can be recycled; obsolete ones cannot.

## ATA Specification Conventions

### Naming (§3.4.1)

| Style | Meaning |
|-------|---------|
| `ALL UPPERCASE` | Command name or signal name (e.g., `IDENTIFY DEVICE`, `READ LOG EXT`) |
| `SMALL UPPERCASE` | Field name (e.g., `DRAT SUPPORTED`, `LBA MID`) |
| Normal case | The *contents* or *value* of a field being discussed |
| "the NAME bit" | A single-bit field; used instead of "the NAME field" |

### Conflict Precedence (§3.4.2)

When spec text, figures, and tables disagree: **tables take precedence over figures, figures over text**. Always trust the table.

### Number Notation (§3.4.4)

| Suffix | Base | Example |
|--------|------|---------|
| `b` (lowercase) | Binary | `0101b`, `0_0101_1010b` |
| `h` (lowercase) | Hexadecimal | `FA23h`, `B_FD8C_FA23h` |
| (none) | Decimal | `25`, `1 323 462.95` (space as thousands separator) |

Underscores and spaces within a number are cosmetic separators only — they carry no meaning. Decimal numbers in the spec use a **space** as the thousands separator (not a comma), which looks unusual in English.

### Bit Range Notation (§3.4.5)

`Feature (7:0)` — `n:m` where **n > m** denotes a bit range, MSB first. This is the format used in spec tables for multi-bit fields. A single bit is called "the NAME bit", not "the NAME field".

### Word/Page Range Notation (§3.4.6)

`p..q` (double-dot, p < q) denotes an **inclusive** range. Example: "words 100..103" means words 100, 101, 102, and 103.

### Data Unit Sizes and Byte Order (§3.4.8)

ATA defines these data unit widths relative to a 16-bit word (word n = byte offset 2n):

| Unit | Width | Bytes at word offset n |
|------|-------|-----------------------|
| Word | 16 bit | 2n, 2n+1 |
| DWord | 32 bit | 2n … 2n+3 (words n, n+1) |
| QWord | 64 bit | 2n … 2n+7 (words n … n+3) |
| DQWord | 128 bit | 2n … 2n+15 (words n … n+7) |

**All multi-byte fields are little-endian unless explicitly noted otherwise**: the byte containing the LSB is at the lowest offset; the byte containing the MSB is at the highest offset.

Concrete examples (directly from §3.4.8):

- **Word 0 = `0007h`** → byte 0 = `07h`, byte 1 = `00h`
- **DWord at words 60..61 = `8001_0203h`** → byte 120 = `03h`, byte 121 = `02h`, byte 122 = `01h`, byte 123 = `80h`
- **QWord at words 2..5 = `0000_0504_0302_0100h`** → byte 4 = `00h`, byte 5 = `01h`, byte 6 = `02h`, byte 7 = `03h`, byte 8 = `04h`, byte 9 = `05h`, byte 10 = `00h`, byte 11 = `00h`

> Even with deep ATA familiarity, it is easy to misread byte ordering from a spec table. When in doubt, rederive from these examples.

**Exceptions to little-endian multi-byte interpretation:**
- ATA strings are byte arrays (not multi-byte integers); see §3.4.9 below.
- The IDENTIFY DEVICE **World Wide Name** field is four separate 16-bit word fields, not a single QWord.
- Data in `TRUSTED SEND`/`TRUSTED RECEIVE` commands is formatted by the security protocol, not by ATA byte ordering.

---

## Relevant Files

| File | Purpose |
|------|---------|
| `include/ata_helper.h` | Status/error bit constants, `ataPassthroughCommand` structure, `ataTFRBlock`, `ataReturnTFRs`, inline LBA helpers |
| `include/ata_helper_func.h` | Public ATA command dispatch functions |
| `src/ata_cmds.c` | Implementations of all ATA commands |
| `src/ata_helper.c` | Identify parsing, TFR debug printing, misc helpers |
| `src/ata_legacy_cmds.c` | Legacy / obsolete ATA command implementations |
| `include/sata_types.h` | SATA-specific type definitions (FIS structures) |
| `include/sata_helper_func.h` | FIS building and SATA-specific helpers |

---

## The `ataPassthroughCommand` Structure

All ATA commands are built by filling an `ataPassthroughCommand` context and passing it to `ata_Passthrough_Command`. Never call OS passthrough directly.

### Prefer Command Builder Helper Functions

`ata_helper.h` provides inline builder functions that zero-initialize the structure, select the correct protocol, conditionally apply `DEVICE_REG_BACKWARDS_COMPATIBLE_BITS` based on the device, and set `LBA_MODE_BIT` only when appropriate. **Use these instead of filling the structure manually wherever possible.** This ensures that future protocol-level changes (e.g., DMA sub-protocol values evolving across SAT revisions as PIO is progressively obsoleted) only need to be updated in one place rather than hunting down every command.

| Helper | Use for |
|--------|---------|
| `create_ata_nondata_cmd(device, opcode, ext, needRTFRs)` | Commands with no data transfer (SET FEATURES, FLUSH CACHE, etc.) |
| `create_ata_pio_in_cmd(device, opcode, ext, sectorCount, buf, len)` | PIO data-in, no LBA addressing (SMART READ DATA, IDENTIFY, log reads) |
| `create_ata_pio_out_cmd(device, opcode, ext, sectorCount, buf, len)` | PIO data-out, no LBA addressing (SMART WRITE LOG, DOWNLOAD MICROCODE PIO) |
| `create_ata_pio_read_lba_cmd(device, opcode, ext, sectorCount, lba, buf, len)` | PIO data-in accessing user LBA space |
| `create_ata_pio_write_lba_cmd(device, opcode, ext, sectorCount, lba, buf, len)` | PIO data-out accessing user LBA space |
| `create_ata_dma_in_cmd(device, opcode, ext, sectorCount, buf, len)` | DMA data-in, no LBA addressing |
| `create_ata_dma_out_cmd(device, opcode, ext, sectorCount, buf, len)` | DMA data-out, no LBA addressing |
| `create_ata_dma_read_lba_cmd(device, opcode, ext, sectorCount, lba, buf, len)` | DMA data-in accessing user LBA space |
| `create_ata_dma_write_lba_cmd(device, opcode, ext, sectorCount, lba, buf, len)` | DMA data-out accessing user LBA space |

After creating the base command, set the remaining fields (Feature, specific TFR values, `needRTFRs`, etc.) and call `ata_Passthrough_Command`:

```c
// Example: SMART READ DATA (PIO-in, no LBA access, 1 sector = 512B)
ataPassthroughCommand ataCmd = create_ata_pio_in_cmd(device, ATA_SMART, ATA_CMD_TYPE_TASKFILE,
                                                     1, dataBuf, 512);
ataCmd.tfr.LbaMid       = ATA_SMART_SIG_MID;
ataCmd.tfr.LbaHi        = ATA_SMART_SIG_HI;
ataCmd.tfr.ErrorFeature = ATA_SMART_READ_DATA;  // sub-command in Feature register
ataCmd.needRTFRs        = true;

eReturnValues ret = ata_Passthrough_Command(device, &ataCmd);
```

For commands where no helper applies, filling the structure manually is an option — see the Common Patterns and Pitfalls section for the rules governing `DeviceHead`, `LBA_MODE_BIT`, and `ataTransferBlocks`.

---

## 28-bit vs. 48-bit LBA

ATA has two register widths. Always pick the correct `commandType`:

| Register width | `commandType` | SAT CDB built | Max LBA | Max sector count |
|---------------|--------------|--------------|---------|-----------------|
| 28-bit | `ATA_CMD_TYPE_TASKFILE` | 12-byte or 16-byte (extend=0) | 28-bit (up to ~137 GB) | 256 (0 = 256) |
| 48-bit | `ATA_CMD_TYPE_EXTENDED_TASKFILE` | 16-byte (extend=1) | 48-bit | 65536 (0 = 65536) |
| ICC / AUX registers | `ATA_CMD_TYPE_COMPLETE_TASKFILE` | 32-byte | 48-bit | 65536 (0 = 65536) |

`ATA_CMD_TYPE_COMPLETE_TASKFILE` is only needed when the `icc` or `aux` register fields are non-zero. If both are zero, `ATA_CMD_TYPE_EXTENDED_TASKFILE` is correct and sufficient — the controller zeroes those registers regardless. 32-byte SAT CDBs are rarely supported and many hosts reject CDBs larger than 16 bytes, so `COMPLETE_TASKFILE` is treated as a last resort.

For 48-bit commands, the TFR block has `LbaLow48`, `LbaMid48`, `LbaHi48`, and `SectorCount48` fields for the high bytes:

```c
ataCmd.commandType       = ATA_CMD_TYPE_EXTENDED_TASKFILE;
ataCmd.tfr.LbaLow        = M_Byte0(lba);
ataCmd.tfr.LbaMid        = M_Byte1(lba);
ataCmd.tfr.LbaHi         = M_Byte2(lba);
ataCmd.tfr.LbaLow48      = M_Byte3(lba);
ataCmd.tfr.LbaMid48      = M_Byte4(lba);
ataCmd.tfr.LbaHi48       = M_Byte5(lba);
ataCmd.tfr.SectorCount   = M_Byte0(sectorCount);
ataCmd.tfr.SectorCount48 = M_Byte1(sectorCount);
ataCmd.tfr.DeviceHead    = DEVICE_REG_BACKWARDS_COMPATIBLE_BITS | LBA_MODE_BIT;
// Do NOT use M_Nibble6 for the LBA high bits in 48-bit mode — the top nibble goes in the extended registers.
```

Use the inline helpers when available:
- `set_ata_pt_LBA_28(cmd, lba)` — sets LbaLow/Mid/Hi and DeviceHead nibble for 28-bit commands
- `set_ata_pt_LBA_28_sig(cmd, signature)` — same but without setting the LBA mode bit (for SMART signatures)

### Why the `*48` Fields Are "Previous" / "High" Bytes — PATA Shadow Registers

The 48-bit register mechanism has its roots in PATA/IDE hardware architecture. Each ATA task file register occupies a single 8-bit I/O port address. To supply 16-bit values to the drive without adding new port addresses, PATA controllers implement a **shadow register** for each port:

- **First write** to a register port → stored in the "previous content" shadow (the high byte)
- **Second write** to the same port → stored in the "current content" latch (the low byte)

The drive controller does not begin processing a command until the **Command register** (opcode register) is written. At that point it reads both the current and previous content of every register simultaneously, assembling the full 16-bit values. This let ATA extend to 48-bit LBA using exactly the same port map that existed for 28-bit commands — no new hardware I/O addresses were required.

This is why the `*48` fields in `ataTFRBlock` (the "extended" or high bytes) are written **first** when building a 48-bit command: they map to the "previous" shadow registers. The non-`*48` fields (the "current" or low bytes) are written second.

**Windows formalizes this naming directly** in the `ATA_PASS_THROUGH_EX` structure:

```c
typedef struct _ATA_PASS_THROUGH_EX {
    // ...
    UCHAR PreviousTaskFile[8];  // High (extended) bytes — the *48 fields
    UCHAR CurrentTaskFile[8];   // Low (current) bytes  — the plain fields
} ATA_PASS_THROUGH_EX;
```

When reading Windows ATA passthrough documentation or reverse-engineering Windows driver code, `PreviousTaskFile` = the extended/high registers (`LbaLow48`, `LbaMid48`, `LbaHi48`, `SectorCount48`, `Feature48`), and `CurrentTaskFile` = the current/low registers (`LbaLow`, `LbaMid`, `LbaHi`, `SectorCount`, `ErrorFeature`, `DeviceHead`, `CommandStatus`). The SAT ATA PASS-THROUGH CDB `EXTEND` bit (bit 2 of byte 1) signals that both previous and current content are valid and should be issued as a 48-bit command.

---

## Protocols (`commadProtocol` field)

Pick the protocol that matches the actual data transfer mechanism the command uses:

| Protocol | Constant | When to use |
|----------|---------|-------------|
| No data | `ATA_PROTOCOL_NO_DATA` | Commands that transfer no data (SET FEATURES, FLUSH CACHE, etc.) |
| PIO in | `ATA_PROTOCOL_PIO` | PIO data-in (IDENTIFY DEVICE, PIO READ SECTORS, SMART READ DATA) |
| PIO out | `ATA_PROTOCOL_PIO` | PIO data-out (SMART WRITE LOG, DOWNLOAD MICROCODE via PIO) |
| DMA | `ATA_PROTOCOL_DMA` | DMA data transfer (READ DMA EXT, WRITE DMA EXT) |
| DMA queued | `ATA_PROTOCOL_DMA_QUE` | TCQ (legacy) — defined in early SAT revisions but later changed to reserved because PATA TCQ saw virtually no real-world adoption. Only IBM is believed to have ever shipped a device that used this protocol. Do not use `ATA_PROTOCOL_DMA_QUE` in new code. |
| FPDMA / NCQ | `ATA_PROTOCOL_DMA_FPDMA` | All NCQ commands — READ/WRITE FPDMA QUEUED **and** ncq-nondata variants. SAT uses the term "NCQ" for this protocol. Some specs label new NCQ commands as "DMA" — verify the protocol before adding new NCQ commands. Transfer length is in the Feature register (not SectorCount) due to a historical workaround: early PATA controllers ignored the extended SectorCount field but honored Feature, so NCQ reused Feature to stay compatible with those controllers. |
| Packet | `ATA_PROTOCOL_PACKET` | ATAPI commands |
| Hard reset | `ATA_PROTOCOL_HARD_RESET` | Hardware reset |
| Soft reset | `ATA_PROTOCOL_SOFT_RESET` | Software reset |
| Return response | `ATA_PROTOCOL_RET_INFO` | Retrieves completion registers without issuing a new command. Used for NCQ error recovery, for ≤2TB drives where fixed-format sense data truncates RTFRs, and for USB bridges that return empty check condition responses. Not commonly needed outside these specific situations. |

---

## Return Task File Registers (RTFRs)

Many ATA commands return status information in the task file registers after completion (SMART, SCT, READ NATIVE MAX ADDRESS, SET MAX, etc.). Set `needRTFRs = true` so the passthrough layer retrieves them:

```c
ataCmd.needRTFRs = true;

eReturnValues ret = ata_Passthrough_Command(device, &ataCmd);

// After the call, RTFRs are in ataCmd.rtfr:
if (ataCmd.rtfr.status & ATA_STATUS_BIT_ERROR)
{
    if (ataCmd.rtfr.error & ATA_ERROR_BIT_ABORT)
    {
        return NOT_SUPPORTED;
    }
    return FAILURE;
}
uint64_t maxLba = M_BytesTo8ByteValue(0, 0,
    ataCmd.rtfr.lbaHi48,   ataCmd.rtfr.lbaMid48,  ataCmd.rtfr.lbaLow48,
    ataCmd.rtfr.lbaHi,     ataCmd.rtfr.lbaMid,     ataCmd.rtfr.lbaLow);
```

Only set `needRTFRs = true` when the command returns meaningful data in the TFRs beyond a simple pass/fail — for example READ NATIVE MAX ADDRESS, SCT status, SMART RETURN STATUS. For commands where only pass/fail matters (SET FEATURES, ZAC non-data, etc.), omit it: an abort error triggers a check condition that the transport layer handles without RTFRs.

**Status bit constants** (from `ata_helper.h`):

| Bit constant | Meaning |
|-------------|---------|
| `ATA_STATUS_BIT_BUSY` | Device is busy — all other bits invalid |
| `ATA_STATUS_BIT_READY` | Device ready |
| `ATA_STATUS_BIT_DEVICE_FAULT` | Device fault (unrecoverable) |
| `ATA_STATUS_BIT_DATA_REQUEST` | PIO data phase ready — **never seen in passthrough**; only relevant to low-level PIO drivers managing the data phase directly. Do not check this bit in command-builder code. |
| `ATA_STATUS_BIT_ERROR` | Error occurred — check error register |
| `ATA_STATUS_BIT_SENSE_DATA_AVAILABLE` | Sense data available for this command |

**Error bit constants**:

| Bit constant | Meaning |
|-------------|---------|
| `ATA_ERROR_BIT_INTERFACE_CRC` | Interface CRC error |
| `ATA_ERROR_BIT_UNCORRECTABLE_DATA` | Uncorrectable read error |
| `ATA_ERROR_BIT_ID_NOT_FOUND` | Sector ID not found |
| `ATA_ERROR_BIT_ABORT` | Command aborted (most common rejection signal) |

Some status and error bits are feature-set specific and should not be interpreted as general indicators. For example, the ACS-2 hybrid non-volatile cache feature (a small NAND cache embedded in the drive, used primarily to cache the Windows OS — unrelated to the NVMe protocol) repurposes bit fields, and sense data reporting in newer drives adds additional status semantics. Do not enable sense data reporting manually — let the OS or HBA manage it.

---

## SAT / SCSI-ATA Translation (`sat_helper.*`)

When an ATA device is connected through a SCSI bridge (SAS-to-SATA expander, USB-to-SATA bridge that supports SAT), ATA commands are tunneled inside SCSI CDBs using the ATA PASS-THROUGH CDB (op 0x85 / 0xA1).

`sat_helper.c` handles:
- Building SAT CDBs from an `ataPassthroughCommand`
- Retrieving RTFRs from the SCSI Passthrough Results Log (log page 0x10), descriptor-format sense data (ATA Return Descriptor), or fixed-format sense data
- Choosing 12-byte vs 16-byte vs 32-byte SAT CDB based on which registers are needed

`sat_helper.c` also contains a **software SCSI-to-ATA translator** — it accepts incoming SCSI commands and translates them to ATA commands per the SAT specification. This translator is used on FreeBSD, Windows ATA passthrough paths, and UEFI, where the OS presents an ATA device but issues only SCSI commands. When looking for the ATA equivalent of a SCSI command, `sat_helper.c` is the canonical reference.

**Note**: SCSI-to-NVMe translation (SNTL) is handled by `sntl_helper.c` and is documented in the NVMe-specific instruction file.

**Rule**: Do not call SAT helper functions directly from command-builder code. `ata_Passthrough_Command` dispatches to SAT automatically when the device interface requires it.

### SAT T_LENGTH Field and TPSIU

The SAT CDB contains a `T_LENGTH` field (bits [1:0]) that tells the SATL how to determine the data transfer length. Understanding these values matters when debugging unexpected SAT CDB construction or when adding a command that uses a non-standard transfer size encoding:

| T_LENGTH | Meaning | Typical commands |
|----------|---------|----------------|
| 0 | Non-data — nothing to transfer | Non-data commands (SET FEATURES, FLUSH CACHE, etc.) |
| 1 | Sector count register — length is (sector count) × logical sector size, or × 512 for non-LBA commands | Most PIO and DMA commands |
| 2 | Features register — length is (Features value) × 512 | NCQ/FPDMA commands; also TRUSTED SEND/RECEIVE and DOWNLOAD MICROCODE in extended mode |
| 3 | **TPSIU** ("Transport Protocol Specific Information Unit") — find the length in a transport-layer-specific location | Some USB bridges; SECURITY SEND/RECEIVE; DOWNLOAD MICROCODE; READ LONG/WRITE LONG |

**TPSIU** is an extremely generic encoding: it instructs the bridge to read the transfer length from the transport packet itself rather than deriving it from any TFR field. Only three scenarios require TPSIU in this codebase:

1. **Some USB bridge chips** cannot handle standard T_LENGTH=1/2 encoding and require TPSIU to function correctly. The `alwaysUseTPSIUForSATPassthrough` flag in `passThroughHacks` enables this globally for a device. It is set during the `initial_Identify_Device()` retry sequence (see below) when all other corrections have failed.
2. **SECURITY SEND / SECURITY RECEIVE** (ATA TRUSTED SEND / TRUSTED RECEIVE) — TCG security protocol commands use TPSIU because the transfer length is encoded in the security protocol payload, not in the standard TFR registers.
3. **DOWNLOAD MICROCODE** — ATA firmware download uses TPSIU for analogous reasons.

**28-bit sector count overflow caveat** (cases 2 and 3): in 28-bit mode the sector count register is only 8 bits wide. If the transfer length exceeds 255 sectors (128 KiB), the overflow must be read from a separate LBA register instead. This is a non-obvious quirk unique to these two commands. Keeping transfer lengths below 256 sectors avoids the issue entirely; real-world TRUSTED SEND/RECEIVE and DOWNLOAD MICROCODE transactions virtually never exceed this limit.

READ LONG / WRITE LONG use their own special T_LENGTH mode distinct from the four table entries above — they transfer exactly one logical block plus ECC bytes, a size not derivable from any sector count field. Special handling for these commands is already implemented.

### `initial_Identify_Device()` — Baseline of SAT Hacks

`initial_Identify_Device()` in `ata_helper.c` (static, lines 1908–2007) is the canonical implementation of the retry-with-workaround strategy for initial ATA Identify Device. Its comment reads: *"This function attempts numerous workarounds to get working identify data (to work around SAT issues)."* It is the primary reference point when investigating USB passthrough compatibility for a new or unknown device.

**Setup**: for ATAPI, optical, and tape devices where no hacks have been set by VID/PID lookup, `a1NeverSupported = true` is forced before the first attempt to prevent the A1h 12-byte CDB from being issued to those devices.

**Retry sequence** (applied only when `hacksSetByReportedID` is false, i.e. the device is not in the VID/PID table):

*On INVALID FIELD IN CDB* (opcode accepted, but a field in the CDB was rejected) — three corrections tried in order:
1. Disable check condition (`alwaysCheckConditionAvailable = false`) → retry
2. Set `a1NeverSupported = true` — switches from 12-byte A1h to 16-byte 85h SAT CDB → retry
3. Set `alwaysUseTPSIUForSATPassthrough = true` — enables TPSIU T_LENGTH encoding → retry

*On INVALID COMMAND OPERATION CODE* (the opcode itself was rejected):
1. Set `a1NeverSupported = true` (A1h → 85h) → retry
2. If USB interface: disable check condition → retry
3. If `retryWithJMicronPT` was set during enumeration: switch to `ATA_PASSTHROUGH_JMICRON` → retry

`scsi_Test_Unit_Ready()` is issued between every retry attempt (except on IDE interface) to clear error-throttling state in the adapter (cf. TURF). **SAS HBAs** succeed on the very first `get_Identify_Data()` call with no retries — the retry paths are a USB/SATL compatibility mechanism only. When `hacksSetByReportedID` is true (VID/PID table hit), only the JMicron PT retry path is considered; all other hacks are pre-populated correctly from the table.

---

## Common Patterns and Pitfalls

### Zero-initialize before filling

```c
// Always do this before setting individual fields
ataPassthroughCommand ataCmd;
safe_memset(&ataCmd, sizeof(ataCmd), 0, sizeof(ataCmd));
```

### DeviceHead register

Two of the most common mistakes when filling this field manually:

**`DEVICE_REG_BACKWARDS_COMPATIBLE_BITS` (0xA0)**: These bits are a legacy compatibility requirement from pre-ATA controllers that used them to encode drive sector size. They are **not applicable to SATA devices** and must not be set unconditionally. The `set_ata_pt_device_bits()` helper (called internally by every `create_ata_*` function) applies them conditionally based on `device->drive_info.ata_Options.noNeedLegacyDeviceHeadCompatBits`. Do not set them manually.

**`LBA_MODE_BIT`**: This bit must **not** be set for every command — only for commands that access user-LBA space. ATA commands fall into two categories:

- **LBA access commands** (READ, WRITE, VERIFY, READ NATIVE MAX ADDRESS, etc.) — set `LBA_MODE_BIT`. For 28-bit commands, the upper 4 bits of the LBA go into the lower nibble of DeviceHead (`M_Nibble6(lba)`), a leftover from CHS geometry naming. CHS mode is always a logical construct in modern ATA; the code uses LBA by default but retains software CHS translation for ancient CHS-only devices.
- **Non-LBA commands** (SET FEATURES, SMART, FLUSH CACHE, sanitize-signature commands, etc.) — do **not** set `LBA_MODE_BIT`.

The `create_ata_*_lba_cmd` helpers set `LBA_MODE_BIT` automatically. The non-LBA helpers (`create_ata_pio_in_cmd`, `create_ata_nondata_cmd`, etc.) do not. Use these helpers to avoid having to make this decision manually.

**Device control vs. DeviceHead**: The `DeviceControl` field is rarely available in passthrough and should always remain zero. It is easy to confuse with `DeviceHead` — double-check you are writing to the correct field.

### SMART commands use a fixed signature

All SMART commands require the signature `0xC24F` in LBAmid:LBAlo (0xC2 in LBAmid, 0x4F in LBAlo) on input. On successful completion the device echoes back the same value. SMART RETURN STATUS specifically indicates a drive health failure by swapping the response to `0x2CF4` (0x2C in LBAmid, 0xF4 in LBAlo). Do not rely on checking these response values to detect failure — some USB bridges return zeroes for all RTFR fields regardless of outcome. Use the abort bit in the error register as the primary failure indicator.

SMART commands set the signature bytes directly into the TFR registers without setting `LBA_MODE_BIT` (which the spec requires these commands to omit). The `_sig` helper `set_ata_pt_LBA_28_sig` omits `LBA_MODE_BIT` and is used for other commands that require a signature value in the LBA registers:

```c
ataPassthroughCommand ataCmd = create_ata_pio_in_cmd(device, ATA_SMART, ATA_CMD_TYPE_TASKFILE,
                                                     1, dataBuf, 512);
ataCmd.tfr.LbaMid       = ATA_SMART_SIG_MID;
ataCmd.tfr.LbaHi        = ATA_SMART_SIG_HI;
ataCmd.tfr.ErrorFeature = ATA_SMART_READ_DATA;  // sub-command in Feature register
ataCmd.needRTFRs        = true;
```

### Never send 48-bit commands to a device that does not support them

Check `device->drive_info.ata_Options.fourtyEightBitAddressFeatureSetSupported` before issuing 48-bit variants. Return `NOT_SUPPORTED` if the flag is not set.

### `ataTransferBlocks` — 512B vs. logical sector size

This field controls whether the SAT CDB uses a 512-byte sector as the unit for `T_LENGTH` (legacy default) or the device's reported logical sector size (needed for 4K-native / 4Kn devices). Set `ataTransferBlocks = XFER_NO_DATA` for non-data commands. For data-bearing commands on 4Kn devices, the transport layer sets this automatically when it knows the device's logical sector size — do not override it in command-builder code unless you have a specific reason.

---

## SAT ATA PASS-THROUGH CDB Reference (SAT-5 §12.2.2)

The following tables are from SAT-5 (INCITS 557 Revision 10, January 2022). This is the authoritative specification for how ATA commands are tunneled through SCSI interfaces. Three CDB variants exist:

- **A1h** — ATA PASS-THROUGH (12): 28-bit commands only
- **85h** — ATA PASS-THROUGH (16): 28-bit or 48-bit (controlled by EXTEND bit)
- **7Fh / service action 1FF0h** — ATA PASS-THROUGH (32): 48-bit with ICC and AUXILIARY registers

### ATA PASS-THROUGH (12) — Opcode A1h

```
Byte  7      6      5      4      3      2      1      0
  0   OPERATION CODE (A1h)
  1   Obs.   ┌── PROTOCOL (4 bits) ──┐   Reserved
  2   OFF_LINE(3)  CK_COND  T_TYPE  T_DIR  BYTE_BLOCK  T_LENGTH(2)
  3   FEATURES (7:0)
  4   COUNT (7:0)
  5   LBA (7:0)
  6   LBA (15:8)
  7   LBA (23:16)
  8   DEVICE
  9   COMMAND
 10   Reserved
 11   CONTROL
```

Limitations: 28-bit LBA only; no EXTEND bit; no FEATURES/COUNT high bytes; no ICC/AUXILIARY.

### ATA PASS-THROUGH (16) — Opcode 85h

```
Byte  7      6      5      4      3      2      1      0
  0   OPERATION CODE (85h)
  1   Obs.   ┌── PROTOCOL (4 bits) ──┐   EXTEND
  2   OFF_LINE(3)  CK_COND  T_TYPE  T_DIR  BYTE_BLOCK  T_LENGTH(2)
  3   FEATURES (15:8)           ← high byte; ignored when EXTEND=0
  4   FEATURES (7:0)
  5   COUNT (15:8)              ← high byte; ignored when EXTEND=0
  6   COUNT (7:0)
  7   LBA (31:24)               ← ignored when EXTEND=0
  8   LBA (7:0)
  9   LBA (39:32)               ← ignored when EXTEND=0
 10   LBA (15:8)
 11   LBA (47:40)               ← ignored when EXTEND=0
 12   LBA (23:16)
 13   DEVICE
 14   COMMAND
 15   CONTROL
```

**EXTEND=0**: Behaves identically to ATA PASS-THROUGH (12); high bytes and extended LBA fields are ignored.  
**EXTEND=1**: 48-bit extended taskfile; high bytes of FEATURES, COUNT, and all 48 bits of LBA are valid.

> **Byte ordering note**: LBA bytes are interleaved in an unusual pattern — bytes 7/8 are LBA[31:24]/LBA[7:0], bytes 9/10 are LBA[39:32]/LBA[15:8], bytes 11/12 are LBA[47:40]/LBA[23:16]. This non-sequential interleaving is intentional in the spec and must be reproduced exactly.

### ATA PASS-THROUGH (32) — Opcode 7Fh, Service Action 1FF0h

```
Byte  7      6      5      4      3      2      1      0
  0   OPERATION CODE (7Fh)
  1   CONTROL
  2 - 6   Reserved
  7   ADDITIONAL CDB LENGTH (18h)
  8   SERVICE ACTION (MSB = 1Fh)
  9   SERVICE ACTION (LSB = F0h)  ← combined: 1FF0h
 10   Reserved  ┌── PROTOCOL (4) ──┐   EXTEND
 11   OFF_LINE(3)  CK_COND  T_TYPE  T_DIR  BYTE_BLOCK  T_LENGTH(2)
 12   Reserved
 13   Reserved
 14   LBA (47:40)
 15   LBA (39:32)
 16   LBA (31:24)
 17   LBA (23:16)
 18   LBA (15:8)
 19   LBA (7:0)
 20   FEATURES (15:8)
 21   FEATURES (7:0)
 22   COUNT (15:8)
 23   COUNT (7:0)
 24   DEVICE
 25   COMMAND
 26   Reserved
 27   Reserved
 28   ICC (7:0)
 29   AUXILIARY (31:24)
 30   AUXILIARY (23:16)
 31   AUXILIARY (15:8)
    --- (32nd byte completes the 32-byte CDB)
    AUXILIARY (7:0)
```

Unlike (16), LBA bytes are in descending order (47:40 first, 7:0 last) — sequential and easier to fill. ICC and AUXILIARY registers are only present here; ATA PASS-THROUGH (16) zeros them. Only issue (32) when `ATA_CMD_TYPE_COMPLETE_TASKFILE` is needed.

### PROTOCOL Field Values (shared by all three CDB variants)

| Code | Protocol | Notes |
|------|----------|-------|
| 0h | Hardware Reset | COMRESET (SATA) or RST- assert (PATA); only PROTOCOL and OFF_LINE valid |
| 1h | Software Reset | ATA software reset; only PROTOCOL and OFF_LINE valid |
| 2h | Reserved | — |
| 3h | Non-Data | No data transfer |
| 4h | PIO Data-In | Device→host; verify T_DIR=1 |
| 5h | PIO Data-Out | Host→device; verify T_DIR=0 |
| 6h | DMA | |
| 7h | Reserved | — |
| 8h | Execute Device Diagnostic | |
| 9h | Device Reset | Non-data; device reset |
| Ah | UDMA Data In | |
| Bh | UDMA Data Out | |
| Ch | NCQ / FPDMA | NCQ commands (READ/WRITE FPDMA QUEUED, NCQ non-data) |
| Dh-Eh | Reserved | — |
| Fh | Return Response Information | Read current shadow TFRs without issuing a command; returns ATA Status Return Descriptor |

Protocols 0h–Bh match ATA8-AAM definitions. If PROTOCOL does not match the command type, the SATL may lose communication with the device — the spec does not define recovery behavior.

### Control/Status Fields (shared by all three variants)

| Field | Width | Meaning |
|-------|-------|---------|
| `OFF_LINE` | 3 bits | Seconds the ATA STATUS may be invalid after command: 0→0s, 1→2s, 2→6s, 3→14s. PATA-specific; needed for commands that put the bus in an indeterminate state. Set to 0 for all SATA commands. |
| `CK_COND` | 1 bit | If set, SATL always returns CHECK CONDITION with ATA Status Return Descriptor on completion, even on success. Used by the RTFR retrieval path. |
| `T_TYPE` | 1 bit | 0 = transfer count is in 512B blocks; 1 = transfer count is in logical sector size blocks. Irrelevant when T_LENGTH=0. |
| `T_DIR` | 1 bit | 0 = host→device (write); 1 = device→host (read). Ignored when T_LENGTH=0. For PIO, SATL validates this matches PROTOCOL direction. |
| `BYTE_BLOCK` | 1 bit | 0 = T_LENGTH specifies bytes; 1 = T_LENGTH specifies blocks (sized by T_TYPE). |
| `T_LENGTH` | 2 bits | Which field carries the transfer count: 0=no data, 1=FEATURES register, 2=COUNT register, 3=TPSIU. |
| `EXTEND` | 1 bit | (16) and (32) only. 1 = 48-bit command; 0 = 28-bit (high bytes ignored). |

### ATA Status Return Sense Data Descriptor (Descriptor Code 09h)

This descriptor is returned in **descriptor-format sense data** (sense key 01h = RECOVERED ERROR, ASC 00h, ASCQ 1Dh = ATA PASS-THROUGH INFORMATION AVAILABLE) after every successful ATA PASS-THROUGH when `CK_COND=1`, and after every error.

```
Byte  7      6      5      4      3      2      1      0
  0   DESCRIPTOR CODE (09h)
  1   ADDITIONAL DESCRIPTOR LENGTH (0Ch)
  2   Reserved                                   EXTEND
  3   ERROR
  4   COUNT (15:8)      ← set to 0 when EXTEND=0
  5   COUNT (7:0)
  6   LBA (31:24)       ← high nybble: 0 when EXTEND=0
  7   LBA (7:0)
  8   LBA (39:32)       ← 0 when EXTEND=0
  9   LBA (15:8)
 10   LBA (47:40)       ← 0 when EXTEND=0
 11   LBA (23:16)
 12   DEVICE
 13   STATUS
```

Total size: 14 bytes (2 header + 12 additional). **The CSMI STP passthrough code packs RTFRs into sense data at bytes [8..21] of the full sense buffer** — bytes [8..9] are the descriptor header (09h / 0Ch), then bytes [10..21] correspond to descriptor bytes [2..13] above.

**EXTEND=1**: All 48 bits of LBA and COUNT are valid.  
**EXTEND=0**: Bits [7:4] of byte 6 (LBA[31:24] high nybble), bytes 8 and 10 are zeroed. LBA bits [3:0] of byte 6 carry LBA[27:24] for the 28-bit DEVICE nibble. COUNT(15:8) is zeroed.

### Fixed Format Sense Data for ATA PASS-THROUGH

Some bridges return fixed-format sense data instead of descriptor format. RTFRs appear in the INFORMATION and COMMAND-SPECIFIC INFORMATION fields:

**INFORMATION field** (bytes 3–6 of fixed sense):
```
Byte 0: ERROR
Byte 1: STATUS
Byte 2: DEVICE
Byte 3: COUNT (7:0)
```

**COMMAND-SPECIFIC INFORMATION field** (bytes 8–11 of fixed sense):
```
Byte 0: EXTEND | COUNT_UPPER_NONZERO | LBA_UPPER_NONZERO | Reserved(4) | LOG_INDEX(4)
Byte 1: LBA (7:0)
Byte 2: LBA (15:8)
Byte 3: LBA (23:16)
```

Fixed format cannot return 48-bit registers fully: upper LBA bytes and COUNT(15:8) are lost. `COUNT_UPPER_NONZERO` and `LBA_UPPER_NONZERO` bits signal truncation. When these bits are set, the SATL may have logged the full descriptor-format sense in log page 10h (ATA PASS-THROUGH Results), retrievable with PROTOCOL=Fh (Return Response Information). `LOG_INDEX` nonzero indicates the parameter code in that log page (LOG_INDEX minus one).

> **Real-world note**: LOG_INDEX has **never** been observed as non-zero on any real hardware — not on USB adapters, not on SAS HBAs. The only implementation that sets it is the opensea-transport software SATL. Do not rely on LOG_INDEX being populated by hardware translators; treat it as always zero for practical purposes.

---

## Known SAT Translator Bugs and Quirks

This section documents deviations from the SAT specification that have been observed on real hardware and are explicitly handled in the codebase. These are not theoretical — they affect interoperability and must be understood when debugging RTFR parsing or extending passthrough support.

### Return Response Information (PROTOCOL=Fh) — Always Set T_DIR=1

The SAT spec says `T_DIR` is irrelevant when PROTOCOL=Fh (Return Response Information) because no data transfer occurs — the response is returned via sense data. However, some adapters (primarily USB bridges) still examine `T_DIR` and behave incorrectly when it is zero. The workaround is to **always set T_DIR=1** (device→host direction) for PROTOCOL=Fh commands, even though no data transfer happens. This matches the intent of the operation (reading back ATA register state) and avoids triggering adapter DMA-direction validation bugs.

This is applied unconditionally in the SAT CDB builder — do not remove it.

### USB Adapter EXTEND Bit Unreliability in ATA Status Return Descriptor

Some USB adapters do not correctly set the EXTEND bit (byte 2, bit 0) of the ATA Status Return Descriptor (descriptor code 09h). The bit may be returned as zero even for 48-bit commands that set `EXTEND=1` in the CDB. On these adapters, the EXTEND bit in the returned sense data cannot be trusted to indicate whether 48-bit RTFRs are available.

The workaround is to **ignore the returned EXTEND bit** on adapters with this known quirk. Instead, the code infers whether 48-bit registers are valid from the command that was issued (which the code already knows), not from what the adapter reports back. This is controlled by a passthrough hack flag set during device enumeration (`ignoreExtendBitInSenseData` or similar — check `passThroughHacks` in `ata_helper.h`).

### Fixed Format Sense Data — LBA Byte Ordering Bug

The LBA bytes in the COMMAND-SPECIFIC INFORMATION field of fixed-format sense data were defined differently in older SAT revisions vs. SAT-3 and later. Some translators implement the old byte order, some implement the new, and the bytes end up **reversed** compared to what the current spec specifies.

**Critically**: this bug is not reliably correlated with the adapter's reported SAT compliance level. An adapter that reports SAT-5 compliance (e.g., a modern Broadcom HBA) may still return LBA bytes in the old reversed order. Do not trust the compliance string to predict whether this bug is present.

The codebase detects this bug at device open time by issuing a **NOP command** with carefully chosen, non-palindromic COUNT and LBA values, then inspecting the returned RTFRs. The NOP command does nothing to the device but returns the command registers unmodified (on a conforming device), so mismatched byte order is immediately detectable. The test values in `eNOPCntTests` and `eNOPLBATests` are specifically chosen to avoid values that contain the digit patterns `2` or `F` in hex, which could mask byte-swap detection through coincidental symmetry.

This detection runs inside `fill_ata_drive_info()` in `ata_helper.c` and also catches other RTFR reporting anomalies. Once the byte-order bug is detected, the passthrough hack flag is set and the RTFR parser applies the correction for the lifetime of that device handle.

### Fixed Format LOG_INDEX — Never Populated by Hardware

As noted above, the LOG_INDEX field in the COMMAND-SPECIFIC INFORMATION byte has never been observed as non-zero on any real hardware translator, including both USB bridges and enterprise SAS HBAs. Do not write code that depends on hardware populating this field. It is implemented in the opensea-transport software SATL only.

### CDB Field → ATA Register Mapping (SAT-5 Table 207)

| CDB field | 48-bit ATA field (EXTEND=1) | 28-bit ATA field (EXTEND=0) |
|-----------|----------------------------|-----------------------------|
| FEATURES (15:8) | FEATURE (15:8) | — (ignored) |
| FEATURES (7:0) | FEATURE (7:0) | FEATURE (7:0) |
| COUNT (15:8) | COUNT (15:8) | — (ignored) |
| COUNT (7:0) | COUNT (7:0) | COUNT (7:0) |
| LBA (47:40) | LBA (47:40) | — (ignored) |
| LBA (39:32) | LBA (39:32) | — (ignored) |
| LBA (31:24) | LBA (31:24) | — (ignored) |
| LBA (23:16) | LBA (23:16) | LBA (23:16) |
| LBA (15:8) | LBA (15:8) | LBA (15:8) |
| LBA (7:0) | LBA (7:0) | LBA (7:0) |
| DEVICE (7:4) | DEVICE (7:4) | DEVICE (7:4) |
| DEVICE (3:0) | DEVICE (3:0) | LBA (27:24) ← 28-bit high nibble |
| COMMAND | COMMAND | COMMAND |
| AUXILIARY (31:0) | AUXILIARY (31:0) | — (32-byte CDB only) |
| ICC | ICC | — (32-byte CDB only) |

In 28-bit mode, the lower nibble of the DEVICE field carries LBA bits [27:24]. This is the CHS legacy: DEVICE[3:0] was originally the "head" number; in LBA mode it holds the top LBA nibble. **Do not confuse DEVICE(3:0) with DeviceControl** — they are different registers.

### ATA PASS-THROUGH Status Results (SAT-5 Table 209)

| CK_COND | ERROR bit | DEVICE FAULT bit | Result |
|---------|-----------|-----------------|--------|
| 0 | 0 | 0 | GOOD — no error |
| 0 | 0 | 1 | CHECK CONDITION, RECOVERED ERROR, ASC 00h/ASCQ 1Dh (ATA status return descriptor included) |
| 0 | 1 | any | CHECK CONDITION, error-mapped sense key per SAT-5 §11 (ATA status return descriptor included) |
| 1 | any | any | CHECK CONDITION always; ATA status return descriptor included |

When `CK_COND=1` and PROTOCOL is PIO Data-In or NCQ, sense data format depends on the device's D_SENSE bit (Control mode page): D_SENSE=1 returns descriptor format; D_SENSE=0 returns fixed format with INFORMATION and COMMAND-SPECIFIC INFORMATION fields zeroed for PIO Data-In, or the full RTFRs for other protocols.

The `ataTransferBlocks` field tells the SAT layer what unit the sector count represents in the CDB:

| Value | Meaning | Use for |
|-------|---------|--------|
| `ATA_PT_512B_BLOCKS` | Sector count is in 512-byte units | Non-LBA commands: SMART READ DATA, log reads, IDENTIFY, DOWNLOAD MICROCODE, etc. |
| `ATA_PT_LOGICAL_SECTOR_SIZE` | Sector count is in logical sectors | LBA access commands: READ, WRITE, VERIFY — required for 4Kn drives where each logical sector is 4096 bytes |

The `create_ata_*_lba_cmd` helpers automatically set `ATA_PT_LOGICAL_SECTOR_SIZE`. The non-LBA helpers set `ATA_PT_512B_BLOCKS`. Choosing the wrong value for a 4Kn drive does not silently transfer the wrong amount — it typically causes the OS or controller to hang (one side expects more data than the other sends), or results in a SATL crash, driver error, or hardware reset triggered by a timeout in the kernel error-handling path.

**Sector count for "N/A" commands**: A small number of 28-bit PIO commands (IDENTIFY DEVICE, DCO IDENTIFY, and a few others) list the sector count as "N/A" in the spec. Setting sector count to 0 confuses some SAT translators. Prefer `sectorCount = 1` for these commands — they always transfer exactly one 512-byte block. The low-level SAT CDB builder handles the underlying adjustment automatically, but passing the correct count is clearer.

**Exception — READ LONG / WRITE LONG**: These commands transfer one logical block plus ECC bytes, which is not a standard sector size. They use a dedicated SAT transfer mode ("number of bytes" rather than sectors) and do not use either `ATA_PT_512B_BLOCKS` or `ATA_PT_LOGICAL_SECTOR_SIZE`. Special handling is already implemented for these commands.

**SAT zero-length transfer (sector count = 256 or 65536)**: In 8-bit and 16-bit sector count fields, a value of 0 encodes a maximum-size transfer (256 sectors for 8-bit, 65536 for 16-bit). Most SAT translators handle this incorrectly. The code can signal TPSIU (Transport-specific Information Unit) to tell the translator that the actual transfer length is encoded in the transport layer (e.g., the USB CBW length field in the USB packet) rather than in the SCSI CDB. Some USB bridges require TPSIU and work correctly with it; others reject it entirely. For this reason, zero-encoded transfer sizes should be avoided in general use. The TPSIU path is available as a last resort when there is no other way to make a particular device work.

### Firmware download segment flags

Set `fwdlFirstSegment = true` on the first transfer and `fwdlLastSegment = true` on the last — some Windows IOCTLs need this to manage the firmware download state machine.

### Windows: stale RTFRs after a failed PIO-in command

On Windows, there is a driver bug affecting ATA/SATA devices whether accessed via SCSI passthrough or ATA passthrough IOCTLs. When a command fails and the next command issued is PIO data-in, the driver replays the registers from the previously-failed command into the data buffer — producing a false-positive result (e.g., stale IDENTIFY data with error bits set).

The current workaround: the transport layer automatically issues a `CHECK POWER MODE` command before the PIO-in command to flush the stale state. `CHECK POWER MODE` reliably clears the condition without side effects.

Potential optimization: the unconditional pre-flush adds an extra command round-trip every time. It may be worth making this conditional — only issuing the flush when the immediately preceding command is known to have failed **and** the command about to be issued is PIO data-in. This would reduce unnecessary command traffic and improve performance slightly. This optimization has not yet been implemented.

---

## Historical Context

ATA evolved from Western Digital controller designs originally built for ESDI (Enhanced Small Disk Interface) and ST-506/412 drives, primarily ESDI. The first bytes of the IDENTIFY DEVICE response are inherited directly from the ESDI specification; model number, serial number, and firmware revision fields are additions from early IDE controller designers.

The Feature register was originally the "Write Precompensation" register — older drive media required different write signal levels for inner (higher-density) tracks, and the host managed this timing. The ATA command set essentially passed ESDI signals through to the host bus, and many bit field layouts reflect this legacy directly.

IDE (Integrated Drive Electronics) integrated the controller onto the drive itself and used PATA (Parallel ATA) as its host bus — a low-cost adaptation of ISA designed specifically for disk drives, intended to undercut SCSI/SASI on price. This history is the reason ATA has many unusual bit field layouts and backwards-compatibility requirements that still surface today.

**NCQ/TCQ and the Feature register**: NCQ (Native Command Queuing) and its older predecessor TCQ (Tagged Command Queuing) carry the transfer length in the Feature register rather than SectorCount. This was a deliberate workaround: some early PATA controllers ignored the extended SectorCount48 field under the assumption that no command would ever need it, but they correctly passed the Feature register through to the drive. Reusing Feature as the transfer length field allowed NCQ to scale without breaking those controllers. This is why `ATA_PROTOCOL_DMA_FPDMA` commands populate Feature instead of SectorCount for the transfer length.

**SATA NCQ queue depth**: The ATA specification defines a 5-bit tag field for NCQ, giving a maximum queue depth of 32 (tags 0–31). The supported queue depth is reported in IDENTIFY DEVICE word 75 (Queue Depth field). A drive can report any value from 1 to 32; in practice, modern SATA drives almost universally report 32. Values of 4, 8, or 16 may appear on older or lower-end devices. SATA provides only a single queue — there is no multi-queue model. Older PATA TCQ implementations used a different tagging mechanism and may have had different queue depth limits.

**CHS mode**: Cylinder/Head/Sector addressing is always a logical construct in modern ATA — it has not reflected physical drive geometry in a very long time. The code uses LBA mode by default but retains software CHS support for ancient CHS-only devices, translating LBA to/from CHS in software. These devices are rare enough that the software translation approach is simpler than any hardware-level alternative.

**ATA register naming across eras**: ATA register names reflect the addressing model in use at the time and have changed twice:

- **CHS era**: `CylL` (cylinder low), `CylH` (cylinder high), `Head`, `Sector Number`, and `Sector Count`. These names came from physical disk geometry and appear in very old documentation and some legacy code comments.
- **28-bit LBA era**: the same register positions were repurposed. `Sector Number` became LBA bits 7:0 (`LBALo`), `CylL` became bits 15:8 (`LBAMid`), `CylH` became bits 23:16 (`LBAHi`), and bit 4 of the `Device/Head` register carried LBA bit 27. `Sector Count` remained the transfer length.
- **48-bit LBA era**: a shadow register bank was added — each LBA and Sector Count register has a "previous content" slot and a "current content" slot, loaded by two successive writes to the same port address. Modern ACS specifications express this as `lba7:0`, `lba15:8`, `lba23:16`, `lba31:24`, `lba39:32`, `lba47:40`, documenting exactly which LBA bits each write carries. When reading a current ACS specification this is the notation used; be aware of it when cross-referencing older documentation that still uses `CylL`/`CylH` or `LBALo`/`LBAMid`/`LBAHi`.

**Standards bodies — T13 and SATA-IO**: The ATA command set (ACS-x) is maintained by the **T13** technical committee under INCITS. The physical SATA interface is maintained separately by **SATA-IO** (Serial ATA International Organization). SATA-IO may define new IDENTIFY fields, features, and capabilities that are specific to the SATA transport layer. These additions can take years to appear in an equivalent ACS revision from T13, if they are adopted at all. When working with SATA-specific IDENTIFY data or encountering a field that is not in the current ACS spec, check the SATA-IO specification — the feature may be defined there first.

**AHCI (Advanced Host Controller Interface)**: AHCI is a **driver-level interface specification**, not a storage device standard. T13 and SATA-IO define what ATA/SATA devices do; AHCI defines how a SATA Host Bus Adapter (HBA) exposes itself to the OS and its driver. Intel developed AHCI and published the specification to provide a single, common programming interface for SATA HBAs — enabling a single OS driver to work with any AHCI-compliant HBA from any manufacturer. Before AHCI achieved widespread adoption, each silicon vendor shipped its own proprietary register-level interface: Silicon Image had their own programming interface for their SATA controller chips, NVIDIA had a different interface for the SATA controllers integrated into their chipsets, and so on. Each required a separate driver. AHCI eliminated that fragmentation. The register layout, command list structures, FIS (Frame Information Structure) memory organization, and interrupt model that AHCI defines are the reason ATA passthrough on SATA host systems looks the way it does in the OS passthrough layer.

---

## ATA Log Address Directory

Log addresses are used with READ LOG EXT (GPL, 48-bit), READ LOG (SMART, 28-bit), WRITE LOG EXT, and WRITE LOG commands. The `eATALog` enum in `ata_helper.h` defines all known addresses. Addresses correspond to the table below (ACS-6, April 2025).

**Access modes**: GPL = General Purpose Logging (READ/WRITE LOG EXT, 48-bit commands); SL = SMART Logging (SMART READ/WRITE LOG, 28-bit commands). GPL-only logs return command aborted to SMART commands and vice versa. Mandatory (M) logs must be supported; Optional (O) logs may be absent; Feature-mandatory (F) logs must be supported if the named feature set is supported.

| Address | Enum constant | Log name | Feature | Access |
|---------|--------------|----------|---------|--------|
| 00h | `ATA_LOG_DIRECTORY` | Log Directory | — | M, GPL+SL |
| 01h | `ATA_LOG_SUMMARY_SMART_ERROR_LOG` | Summary SMART Error log | SMART | O, SL only |
| 02h | `ATA_LOG_COMPREHENSIVE_SMART_ERROR_LOG` | Comprehensive SMART Error log | SMART | O, SL only |
| 03h | `ATA_LOG_EXTENDED_COMPREHENSIVE_SMART_ERROR_LOG` | Extended Comprehensive SMART Error log | SMART | O, GPL only |
| 04h | `ATA_LOG_DEVICE_STATISTICS` | Device Statistics | — | O, GPL+SL |
| 05h | — | Reserved for CFA | — | — |
| 06h | `ATA_LOG_SMART_SELF_TEST_LOG` | **Obsolete** (was: SMART Self-Test log / DST log) | SMART | — |
| 07h | `ATA_LOG_EXTENDED_SMART_SELF_TEST_LOG` | **Obsolete** (was: Extended SMART Self-Test log / DST log) | SMART | — |
| 08h | `ATA_LOG_POWER_CONDITIONS` | Power Conditions | EPC | F, GPL only |
| 09h | `ATA_LOG_SELECTIVE_SELF_TEST_LOG` | Selective Self-Test log | SMART | O, SL only |
| 0Ah | `ATA_LOG_DEVICE_STATISTICS_NOTIFICATION` | Device Statistics Notification | DSN | F, GPL only |
| 0Bh | — | Reserved for CFA | — | — |
| 0Ch | `ATA_LOG_PENDING_DEFECTS_LOG` | Pending Defects log | — | O, GPL only |
| 0Dh | `ATA_LOG_LPS_MISALIGNMENT_LOG` | LPS Mis-alignment log | LPS | F, GPL+SL |
| 0Eh | — | Reserved for ZAC-2 | — | — |
| 0Fh | `ATA_LOG_SENSE_DATA_FOR_SUCCESSFUL_NCQ_COMMANDS` | Sense Data for Successful NCQ Commands | NCQ | F, GPL only |
| 10h | `ATA_LOG_NCQ_COMMAND_ERROR_LOG` | NCQ Command Error log | NCQ | F, GPL only |
| 11h | `ATA_LOG_SATA_PHY_EVENT_COUNTERS_LOG` | SATA Phy Event Counters log | NCQ | F, GPL only |
| 12h | `ATA_LOG_SATA_NCQ_QUEUE_MANAGEMENT_LOG` | SATA NCQ Non-Data (Queue Mgmt) log | NCQ | F, GPL only |
| 13h | `ATA_LOG_SATA_NCQ_SEND_AND_RECEIVE_LOG` | SATA NCQ Send and Receive log | NCQ | F, GPL only |
| 14h | `ATA_LOG_HYBRID_INFORMATION` | Hybrid Information log | Hybrid Info | F, GPL only |
| 15h | `ATA_LOG_REBUILD_ASSIST` | Rebuild Assist log | Rebuild Assist | F, GPL only |
| 16h | `ATA_LOG_OUT_OF_BAND_MANAGEMENT_CONTROL_LOG` | Out Of Band Management Control log | OOB Mgmt | F, GPL only |
| 17h | — | Reserved for Serial ATA | — | — |
| 18h | `ATA_LOG_COMMAND_DURATION_LIMITS_LOG` | Command Duration Limits log | CDL | F, GPL only |
| 19h | `ATA_LOG_LBA_STATUS` | LBA Status log | — | O, GPL only |
| 1Ah–1Fh | — | Reserved | — | — |
| 20h | `ATA_LOG_STREAMING_PERFORMANCE` | **Obsolete** (was: Streaming Performance log) | Streaming | — |
| 21h | `ATA_LOG_WRITE_STREAM_ERROR_LOG` | Write Stream Error log | Streaming | F, GPL only |
| 22h | `ATA_LOG_READ_STREAM_ERROR_LOG` | Read Stream Error log | Streaming | F, GPL only |
| 23h | `ATA_LOG_DELAYED_LBA_LOG` | **Obsolete** (was: Delayed LBA log, streaming) | Streaming | — |
| 24h | `ATA_LOG_CURRENT_DEVICE_INTERNAL_STATUS_DATA_LOG` | Current Device Internal Status Data log | — | O, GPL only |
| 25h | `ATA_LOG_SAVED_DEVICE_INTERNAL_STATUS_DATA_LOG` | Saved Device Internal Status Data log | — | O, GPL only |
| 26h–2Eh | — | Reserved | — | — |
| 2Fh | `ATA_LOG_SECTOR_CONFIGURATION_LOG` | Set Sector Configuration log | — | F, GPL only |
| 30h | `ATA_LOG_IDENTIFY_DEVICE_DATA` | IDENTIFY DEVICE data log | — | M, GPL+SL |
| 31h–41h | — | Reserved | — | — |
| 42h | `ATA_LOG_MUTATE_CONFIGURATIONS` | Mutate Configurations log | User Data Init | O, GPL only |
| 43h–46h | — | Reserved | — | — |
| 47h | `ATA_LOG_CONCURRENT_POSITIONING_RANGES` | Concurrent Positioning Ranges log | — | O, GPL only |
| 48h–52h | — | Reserved | — | — |
| 53h | `ATA_LOG_SENSE_DATA` | Sense Data log | Sense Data Reporting | F, GPL only |
| 54h–58h | — | Reserved | — | — |
| 59h | `ATA_LOG_POWER_CONSUMPTION_CONTROL_LOG` | Power Consumption Control log | Power Consumption | F, GPL only |
| 5Ah–60h | — | Reserved | — | — |
| 61h | `ATA_LOG_CAPACITY_MODELNUMBER_MAPPING` | Capacity/Model Number Mapping log | — | F, GPL only |
| 62h–7Fh | — | Reserved | — | — |
| 80h–9Fh | `ATA_LOG_HOST_SPECIFIC_80H`..`9FH` | Host Specific logs | SMART | M, GPL+SL |
| A0h–DFh | — | Device Vendor Specific | SMART | O, GPL+SL |
| E0h | `ATA_SCT_COMMAND_STATUS` | SCT Command/Status | SCT | F, GPL+SL |
| E1h | `ATA_SCT_DATA_TRANSFER` | SCT Data Transfer | SCT | F, GPL+SL |
| E2h–FFh | — | Reserved | — | — |

### Notes on Obsolete Log Addresses

Several log addresses are **Obsolete** in ACS-6 but retain enum values in the code because older drives (pre-ACS-4/5) still implement them:

- **06h / 07h** (`ATA_LOG_SMART_SELF_TEST_LOG` / `ATA_LOG_EXTENDED_SMART_SELF_TEST_LOG`): These were the original DST (Drive Self-Test) result logs. In ACS-6 the DST results are accessed via the Device Statistics log (04h) and the IDENTIFY DEVICE data log (30h). Older drives only have 06h/07h.
- **20h** (`ATA_LOG_STREAMING_PERFORMANCE`): Was the streaming performance log. Obsoleted; streaming error logs at 21h/22h remain valid.
- **23h** (`ATA_LOG_DELAYED_LBA_LOG`): Was used by the streaming feature set for delayed LBA reporting. Obsoleted.

When reading these logs from a device that supports them, the log directory (00h) is authoritative — check it before assuming a log is present. A zero entry in the directory means the log is absent or unsupported on that device.

**The long-term trajectory — from blackbox to transparent device**: ATA was traditionally a blackbox model. The host sent a command and got a result; what happened inside the drive was opaque. That model has been eroding steadily, driven largely by cloud and hyperscale operators who require deeper visibility and control over large drive populations.

Concrete examples of this shift:
- **Device Statistics** (log 04h) replaced vendor-unique SMART attributes with a standardized set of counters, making drive health data comparable across manufacturers and model families.
- **Extended Power Conditions (EPC)** replaced the APM (Advanced Power Management) feature set, which exposed only a coarse "slider" value and a standby timer. EPC provides precise, individually configurable timers for multiple idle and standby states.
- **Sense Data Reporting** allows the drive to report SCSI-level sense codes natively for ATA command errors. Instead of a lossy translation through fixed-format sense data (which can only carry a small subset of ATA error information), the drive can describe the error in the richer SCSI vocabulary. This makes SAT translation more reliable and gives SAS host adapters much better diagnostic data when attached to SATA drives.
- **Seagate FARM** (Field Accessible Reliability Metrics) was open-sourced to expose a rich set of per-drive reliability telemetry that goes far beyond what the standards require — a voluntary extension in the same direction.
- **Asynchronous READ LOG EXT** and expanding log address spaces continue to add new structured log pages that are inaccessible through the old SMART channel.

The practical consequence for code: treat ATA as a protocol in active evolution. Features that were vendor-unique five years ago may now be standardized; fields that the spec marks as reserved may be populated by newer firmware. The direction of travel is toward SCSI-level data richness and diagnostic capability, but carried on ATA's historical command structure and backward-compatibility constraints rather than a clean SCSI command set.

**OCP (Open Compute Project) and SATA**: The Open Compute Project is a consortium of hyperscale cloud operators and storage vendors that publishes additional mandatory requirements on top of the existing storage standards for data center use. OCP has a SATA specification that layers requirements on top of ACS/SATA-IO for cloud SATA drives — mandating certain features, telemetry, and log pages. Generic OCP support across all interfaces (including SATA) is a known future improvement goal for this codebase.

---

## ATA Data Format Conventions

### Endianness

All ATA data structures are **little-endian** by default. Some fields are explicitly marked with a different byte order in the specification, but the default assumption is always little-endian. LBA space data (user read/write content) follows whatever the host decides, not ATA's own byte order.

`ata_cmds` passes raw data to/from the drive without interpretation. Callers are responsible for byte-swapping:
- **Before sending an output buffer**: swap multi-byte fields from host order to ATA little-endian.
- **After receiving an input buffer**: swap multi-byte fields from ATA little-endian to host order before use.

**Known vendor bug — security protocol list**: Many drive manufacturers incorrectly encode the supported security protocols list length in big-endian rather than little-endian. This is a firmware bug and does not match the ATA/SCSI specification. Code that reads this field must handle both orderings defensively.

### ATA Strings — §3.4.9 (Model Number, Serial Number, Firmware Revision)

ATA strings are defined as sequences of ASCII graphic characters in the range `20h`–`7Eh`. Values in the range `00h`–`1Fh` or `7Fh`–`FFh` are not permitted (though old or buggy firmware may produce them).

ATA strings have three properties that differ from every C string convention:

1. **Never null-terminated** — the full field width is always used.
2. **Space-padded to full length** — trailing unused characters are filled with ASCII space (`20h`), not null.
3. **Byte-swapped within each word** — within every adjacent pair of bytes, the two bytes are swapped. The first character of the string is stored in byte 1 (the high byte of word 0); the second character is in byte 0 (the low byte of word 0).

**Worked example — firmware revision `"abcdefg "`** (7 characters + 1 padding space, from ACS-6 §3.4.9):

| Word offset | Stored bytes (hex) | Characters stored |
|-------------|-------------------|-------------------|
| 0 | `62h 61h` | `'b'`, `'a'` → read as `"ba"` |
| 1 | `64h 63h` | `'d'`, `'c'` → read as `"dc"` |
| 2 | `66h 65h` | `'f'`, `'e'` → read as `"fe"` |
| 3 | `20h 67h` | `' '`, `'g'` → read as `" g"` |

To recover the correct string `"abcdefg "`, swap every adjacent byte pair: `"ab"`, `"cd"`, `"ef"`, `"g "`.

Always pass ATA strings through a byte-swap and space-strip routine before treating them as readable text. The `ata_helper.c` parsing code handles this for the standard IDENTIFY fields.

> This is one of the easiest conventions to misread even with long ATA experience. When reading IDENTIFY data directly from a buffer, always verify via the worked example above rather than trusting intuition.

**Exception — ancient controllers**: On very old hardware, the Model Number, Serial Number, and Firmware Revision fields may be entirely `00h` bytes rather than space-padded. All-zero means the field is not supported, not that the string is empty.

### ATA Security Password Encoding

The ATA security password is a raw **32-byte opaque array**. The ATA specification places no requirements on the encoding or content of those bytes — it is treated as a binary blob. Valid choices include ASCII, UTF-8, ATA-byteswapped strings, a hash of the user-visible password, or any other byte sequence the implementer chooses.

This creates a real interoperability hazard: **a BIOS and a software tool can both implement the ATA security feature correctly while using incompatible password encodings**. If a drive is locked by a BIOS that uses one encoding and an unlock attempt is made by software that uses a different encoding (or vice versa), the passwords will not match even if the user types the same text. The drive will reject the unlock and may count toward the attempt limit before freezing or requiring a power cycle.

When implementing or debugging ATA security operations, always verify what encoding the locking agent used. Do not assume the password bytes follow any particular scheme.

---

## SMART Feature Detection Nuances

The IDENTIFY DEVICE response contains two SMART-related capability bits that are often misread:

**SMART error logging supported** (word 84/87 bit): indicates that `SMART READ LOG` and `SMART WRITE LOG` commands are supported. The set of supported log pages is also confirmed by `SMART READ DATA` (the attributes/health data response) — that response contains a supported log pages field that gates what the host can actually request.

**SMART self-test supported** (word 84/87 bit): indicates that `SMART EXECUTE OFF-LINE IMMEDIATE` is supported with the offline data collection sub-command. It does **not** guarantee that short DST or long DST sub-commands are available. On old drives (e.g., ATA/ATAPI-4 era), the drive may silently substitute the offline data collection routine when short or long DST is requested, returning completion rather than an error. To determine whether short/long DST is actually supported, check the dedicated capability field inside the `SMART READ DATA` response — do not rely solely on the IDENTIFY bit.

---

## ATA Log Access — SMART vs. Read Log Ext

### Overview

Modern log access uses **READ LOG EXT** (GP log space). SMART is a legacy log mechanism that is no longer expanding — no new log definitions are being added to SMART. When both mechanisms can read a log, prefer Read Log Ext.

**SMART-only logs** (no Read Log Ext equivalent):
- Summary Error Log (log address 01h)
- Comprehensive Error Log (log address 02h)
- Self-Test (DST) Log (log address 06h)

**Extended equivalents** exist for both the comprehensive error log and the DST log in the GP log space and should be preferred on any device that supports Read Log Ext.

### SMART Read Log vs. Read Log Ext: Subpage Access

`SMART READ LOG` / `SMART WRITE LOG`: the entire log must be read or written in a **single command**; there is no subpage field.

`READ LOG EXT` / `WRITE LOG EXT`: supports a **subpage** parameter, allowing individual subpages of a log address to be accessed independently. Logs that exist at the same address in both spaces (e.g., 04h Device Statistics, 30h Identify Device Data) can only be accessed at the subpage level via Read Log Ext.

### Critical: Do Not Read Beyond a Log's Reported Size

**Reading beyond a log's maximum size does NOT return zero-padded data — it returns a command abort.** This is one of the most common misunderstandings about ATA log access:
- Some logs are defined as a fixed 512-byte size (GP log directory page 0, SMART Summary Error Log, SMART DST Log).
- Most Read Log Ext logs are **variable size** depending on what the drive supports and how much data it has collected.
- Always query the number of supported pages (sectors) for a log before reading. Never assume a log is larger than what the directory reports.

### Supported Pages Log (Page 0)

**Read Log Ext page 0** (GP log directory): should list only pages accessible via `READ LOG EXT`. Some old Maxtor devices incorrectly include SMART-only log addresses in this directory — treat those entries defensively.

**SMART read log page 0** (SMART log directory): lists pages accessible via `SMART READ LOG`. No known firmware violations of this rule when page 0 is supported.

**SMART log directory page 0 is not required on very old drives** if all logs the device supports are exactly 512 bytes in length. Only when the device has at least one log page larger than 512 bytes is it required to support the SMART log directory. `READ LOG EXT` page 0 should always be present on any device that supports Read Log Ext (firmware bugs notwithstanding).

### Logs with Subpage Directories (Log Addresses 04h and 30h)

Log address **04h** (Device Statistics) and **30h** (Identify Device Data Log) both follow a newer pattern: subpage 00h of each log lists all supported subpages for that log address. This first-page-as-directory pattern may expand to other log addresses in the future, but as of the current ACS revision only these two use it.

Both of these logs also use **QWORD (8-byte) fields** for most of their data entries. The one known exception is the World Wide Name (WWN), which is stored as a **DQWORD (16 bytes / 128 bits)** to accommodate the full field width along with the validity/support flags the log format adds.

### Identify Device Data Log (Log 30h) — Preferred Source for Modern Feature Detection

Log 30h is the **Identify Device Data Log** (IDDL). Standard IDENTIFY DEVICE is no longer receiving new word assignments in ACS revisions; new features are defined as additional IDDL subpages instead. For any new feature introduced in recent ACS revisions, check the IDDL first.

- **Subpage 00h** is a directory listing which subpages this device supports (same pattern as Device Statistics log 04h).
- **Subpage 01h** is a **verbatim, byte-for-byte copy** of the 512-byte IDENTIFY DEVICE response. It is accessible via READ LOG EXT asynchronously — the host can read it without halting the drive or waiting for pending I/O to drain. This is a meaningful advantage for background health monitoring.
- **Subpages 02h and above** carry structured data for features not described in the 512-byte Identify response. Each subpage uses QWORD fields with explicit validity and support flags rather than the bare bit fields in standard IDENTIFY.

**Availability caveat**: IDDL requires GP Log (General Purpose Logging) support and is absent on pre-GPL drives. USB bridge chips that do not implement GPL passthrough — or that silently truncate multi-sector PIO log reads — will not return IDDL data. Always confirm Read Log Ext availability before accessing IDDL, and maintain a fallback path using the standard IDENTIFY DEVICE response. When GPL availability is uncertain, read GP log directory page 0 first.
