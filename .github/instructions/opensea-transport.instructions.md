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
| NetBSD / OpenBSD | `netbsd_openbsd_helper.h` | SCSI passthrough is the primary method (handles most commands via SCSI translation). Legacy ATA passthrough is limited to 28-bit commands on these platforms. |
| Solaris / Illumos | `uscsi_helper.h` | USCSI |
| VMware | `vm_helper.h` | NVMe IOCTL via kernel module |
| UEFI | `uefi_helper.h` | EFI passthrough protocols |

**Rule**: Never `#include` a specific OS helper directly. Include `platform_helper.h` or use the abstract functions in `ata_helper_func.h`, `scsi_helper_func.h`, and `nvme_helper_func.h`.

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

### The `passthrough_hacks` Structure

The primary mechanism for adapting to bridge-specific behavior is `device->drive_info.passThroughHacks`. This is populated from a VID/PID lookup table during device enumeration. It controls which passthrough variant to attempt and what limitations to apply. Key principles:

- **Hacks change default behavior only when explicitly set.** If a flag is not set, do not apply the corresponding quirk.
- The default **SAT passthrough** CDB size is **12-byte** (`ATA PASS-THROUGH 12`), not 16-byte. Testing across unknown USB VID/PID combinations has shown that 12-byte SAT CDBs succeed more often out-of-the-box with unknown bridges than 16-byte. 16-byte SAT CDBs are used when specifically required (e.g., 48-bit ATA commands that cannot fit in a 12-byte CDB). This 12-byte default applies to SAT only and was established after broad USB compatibility testing.
- For **native SCSI read/write CDBs** (used for all USB read/write traffic), the CDB size selection follows capacity: if the device capacity exceeds 2 TB, 16-byte READ/WRITE commands are supported and required (10-byte CDBs cannot address beyond 2 TB). For devices at or below 2 TB, use 10-byte CDBs. 12-byte SCSI read/write CDBs are rarely supported by USB bridges and should not be used.
- As VID/PID data accumulates and trends become clear, default behavior may be updated. This is how the SAT 12-byte default was established.

### Legacy Vendor-Specific CDBs

Legacy vendor passthrough CDBs (Cypress, JMicron, SunPlus, and similar) use opcode ranges that are **vendor-unique and undefined for other devices**. Issuing them to an unknown device can cause unpredictable behavior, including bricking USB flash drives. Rules:

- **Never issue a legacy vendor CDB unless another mechanism has already confirmed the bridge vendor.** VID/PID lookup is the primary gate. Do not guess.
- `usb_hacks.h` is being gradually deprecated as the internal `passthrough_hacks` infrastructure matures. Do not extend it with new vendor entries.
- The enum for legacy passthrough type is set via `passthrough_hacks`; once set to a legacy type, the normal SAT hacks are irrelevant since that spec defines its own capability advertisement.

### NVMe over USB

Some USB docks expose NVMe drives via vendor-specific tunneling. This behaves like legacy ATA passthrough: only use it when confirmed supported via VID/PID lookup or capability detection. The existing enumeration code already handles automatic retry and fallback. Do not extend this path without testing on actual hardware.

### Automatic SCSI CDB Size Fallback

The transport layer already tracks per-device CDB size capability for mode pages, log pages, reads, and writes. If a command returns an ILLEGAL REQUEST / INVALID COMMAND OPERATION CODE sense response, the layer automatically retries with a smaller CDB and records the limit for that device so subsequent commands do not repeat the probe. This mechanism applies to native SAS devices as well as USB.

When adding a new SCSI command that has both a 10-byte and a 16-byte variant (e.g., a new mode sense or log sense path), structure the implementation so it participates in this fallback tracking rather than hardcoding a size. Check whether the relevant `passThroughHacks` flag (or equivalent device-capability field) is already populated before attempting the larger CDB.

### SCSI Version Pre-filter (INQUIRY Version Byte)

The INQUIRY standard response byte 2 (`VERSION`) reports which SCSI standard the device conforms to. The transport layer checks this during enumeration:

- SCSI-2 and earlier — 16-byte CDBs did not exist in that revision. Do not attempt them.
- SPC-3 (version ≥ 5) — 16-byte CDBs are expected to be supported.

When writing code that selects between CDB sizes, respect this cached version value rather than blindly attempting 16-byte first. Future work may extend this to check the INQUIRY version descriptor list (bytes 58–73) for finer-grained capability detection, but that is not yet implemented.

### ATA Passthrough Autosense Limitation (SAT)

When the OS issues an ATA passthrough command, the OS autosense subsystem **always requests fixed-format sense data** (not descriptor format). This matters because the ATA Return Task File Registers (RTFRs) are embedded in the sense data response, and fixed-format sense data has limited space compared to descriptor format.

The practical consequence: for ATA commands that return large LBA values in their RTFRs (e.g., `READ NATIVE MAX ADDRESS EXT`, capacity-reporting commands), fixed-format sense data may not convey the full 48-bit result correctly when the drive capacity falls in the ambiguous range between 28-bit max (128 GiB) and 2 TB. Some host systems allow inhibiting autosense to work around this, but opensea-transport does not currently implement that mechanism. Alternative approaches are used with mixed results. Be aware of this limitation when writing code that depends on RTFR values returned over a SAT connection.
