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

---

## Protocols (`commadProtocol` field)

Pick the protocol that matches the actual data transfer mechanism the command uses:

| Protocol | Constant | When to use |
|----------|---------|-------------|
| No data | `ATA_PROTOCOL_NO_DATA` | Commands that transfer no data (SET FEATURES, FLUSH CACHE, etc.) |
| PIO in | `ATA_PROTOCOL_PIO` | PIO data-in (IDENTIFY DEVICE, PIO READ SECTORS, SMART READ DATA) |
| PIO out | `ATA_PROTOCOL_PIO` | PIO data-out (SMART WRITE LOG, DOWNLOAD MICROCODE via PIO) |
| DMA | `ATA_PROTOCOL_DMA` | DMA data transfer (READ DMA EXT, WRITE DMA EXT) |
| DMA queued | `ATA_PROTOCOL_DMA_QUE` | TCQ (legacy) |
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

**Standards bodies — T13 and SATA-IO**: The ATA command set (ACS-x) is maintained by the **T13** technical committee under INCITS. The physical SATA interface is maintained separately by **SATA-IO** (Serial ATA International Organization). SATA-IO may define new IDENTIFY fields, features, and capabilities that are specific to the SATA transport layer. These additions can take years to appear in an equivalent ACS revision from T13, if they are adopted at all. When working with SATA-specific IDENTIFY data or encountering a field that is not in the current ACS spec, check the SATA-IO specification — the feature may be defined there first.

**AHCI (Advanced Host Controller Interface)**: AHCI is a **driver-level interface specification**, not a storage device standard. T13 and SATA-IO define what ATA/SATA devices do; AHCI defines how a SATA Host Bus Adapter (HBA) exposes itself to the OS and its driver. Intel developed AHCI and published the specification to provide a single, common programming interface for SATA HBAs — enabling a single OS driver to work with any AHCI-compliant HBA from any manufacturer. Before AHCI achieved widespread adoption, each silicon vendor shipped its own proprietary register-level interface: Silicon Image had their own programming interface for their SATA controller chips, NVIDIA had a different interface for the SATA controllers integrated into their chipsets, and so on. Each required a separate driver. AHCI eliminated that fragmentation. The register layout, command list structures, FIS (Frame Information Structure) memory organization, and interrupt model that AHCI defines are the reason ATA passthrough on SATA host systems looks the way it does in the OS passthrough layer.

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
