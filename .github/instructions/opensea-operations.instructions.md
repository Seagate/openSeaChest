---
description: 'opensea-operations library — implementing new operations, protocol dispatch, feature detection, progress reporting, and operations by functional area'
applyTo: 'subprojects/opensea-operations/**/*.c, subprojects/opensea-operations/**/*.h'
---

# opensea-operations — Architecture and Conventions

## Purpose

opensea-operations is the highest-level library in the stack. It owns:

- Cross-protocol operation implementations that abstract ATA vs. SCSI vs. NVMe differences
- Feature detection (what the device can do) and operation dispatch (which command to use)
- Progress reporting for long-running operations (DST, sanitize, firmware download, full erase)
- High-level data structures that summarize drive state in a protocol-agnostic way

Operations code should **never need to know** which passthrough IOCTL is used — that belongs in opensea-transport. Operations code knows about ATA/SCSI/NVMe protocols (command opcodes and data structures), but isolates the caller from having to pick the right protocol.

---

## Root Include

```c
#include "operations_Common.h"
```

This single include pulls in all of `ata_helper.h`, `scsi_helper.h`, `nvme_helper.h`, all their `_func.h` partners, `cmds.h`, and `common_public.h`. Do not repeat these includes individually in operations files.

Mark public API functions with `OPENSEA_OPERATIONS_API` so they export correctly when the library is compiled as a DLL.

---

## How to Implement a New Operation

### 1. Determine the scope

- **Protocol-specific** (only makes sense on one command set): ATA Security, NVMe Sanitize NODMMAS, etc. → implement in the dedicated per-protocol header/source and gate it with a drive type check at call sites.
- **Cross-protocol** (same concept, different wire encoding): DST, Sanitize, Firmware Download → implement a single public function that dispatches internally.

### 2. Check feature support before issuing the command

Always query `device->drive_info` fields and/or issue a capability-check command before sending a destructive or optional command. Return `NOT_SUPPORTED` if the feature is absent — never send a command that the drive might not understand and rely on the command being harmlessly rejected.

```c
// ATA example: check 48-bit support before using 48-bit command
// NOTE: Direct field access shown here reflects current practice.
// Helper functions (e.g., is_ATA_48bit_Supported(device)) are being added
// to abstract these accesses. Use a helper if one exists for the check you need.
// Helpers already exist for drive type, interface type, logical/physical sector
// sizes, and more — check common_public.h before accessing drive_info directly.
if (!device->drive_info.ata_Options.fourtyEightBitAddressFeatureSetSupported)
{
    return NOT_SUPPORTED;
}

// SCSI example: check write-protect before attempting a write operation
if (device->drive_info.isWriteProtected)
{
    return PERMISSION_DENIED;
}
```

### 3. Dispatch by drive type

Use `device->drive_info.drive_type` to dispatch to the correct protocol implementation:

```c
eReturnValues do_My_Operation(const tDevice* M_NONNULL device, ...)
{
    switch (device->drive_info.drive_type)
    {
    case ATA_DRIVE:
        return my_ata_implementation(device, ...);
    case NVME_DRIVE:
        return my_nvme_implementation(device, ...);
    default:
        // For operations that have a SCSI equivalent, the default case is
        // typically the SCSI path — consistent with the "Everything is SCSI"
        // philosophy (see opensea-transport instructions). Non-ATA/NVMe devices
        // (SAS, USB bridges, unrecognized types) present as SCSI and the
        // translator below handles the rest. If a SCSI path doesn't make sense
        // for this operation, return NOT_SUPPORTED here instead.
        return my_scsi_implementation(device, ...);
    }
}
```

Common `drive_type` values:

| Constant | When used |
|----------|-----------|
| `ATA_DRIVE` | ATA/SATA direct attach or through SAT bridge |
| `SCSI_DRIVE` | SCSI/SAS or ATAPI devices reporting as SCSI |
| `NVME_DRIVE` | NVMe direct attach or USB-to-NVMe bridge |
| `ATAPI_DRIVE` | ATAPI (optical drives, legacy tape, etc.) |
| `FLASH_DRIVE` | USB mass storage — **not a reliable detection**. USB flash drives present as SCSI block devices, identical to USB-attached HDDs and SSDs. Some (e.g., certain SanDisk devices) even expose limited ATA passthrough. There is often not enough information in the SCSI command set to reliably distinguish a flash drive from a spinning disk or SSD over USB. `FLASH_DRIVE` is a best-effort classification, not a guarantee. |
| `UNKNOWN_DRIVE` | Device type could not be determined |

### 4. Allocate aligned buffers, free with `safe_free_aligned`

Data buffers used for passthrough commands must be aligned to the device's DMA alignment requirement. Use `safe_calloc_aligned` with the alignment obtained from the tDevice helper in `common_public.h`. This avoids an extra kernel-side buffer copy that would otherwise occur when an unaligned buffer is passed through the IOCTL.

```c
size_t   alignment = get_Device_IO_Minimum_Alignment(device); // from common_public.h
uint8_t* buf       = M_REINTERPRET_CAST(uint8_t*,
    safe_calloc_aligned(bufLen, sizeof(uint8_t), alignment));
if (buf == M_NULLPTR)
{
    return MEMORY_FAILURE;
}

// ... use buf ...

safe_free_aligned(&buf);
```

### 5. Time long operations (aspirational)

Wrapping operations with elapsed-time measurement is a long-standing improvement goal. Currently only firmware download does this consistently. Adding timing to new operations is encouraged — it enables reporting "operation X completed in N seconds" to the user and to diagnostic output. The codebase does not have full async support and NCQ commands are not currently used; OS passthrough for ATA in particular (and RAID passthrough even more so) does not allow performant command execution anyway, so timing here is for diagnostic reporting, not performance benchmarking.

```c
DECLARE_SEATIMER(opTimer);
start_Timer(&opTimer);

eReturnValues ret = dispatch_the_command(device, ...);

stop_Timer(&opTimer);
print_Command_Time_Verbose(device, VERBOSITY_COMMAND_VERBOSE, get_Nano_Seconds(opTimer));
```

### 6. Return the correct `eReturnValues` code

| Situation | Return value |
|-----------|-------------|
| Everything worked | `SUCCESS` |
| Device rejected or doesn't support the command | `NOT_SUPPORTED` |
| Command completed with a media error | `FAILURE` |
| Command parameter was invalid | `BAD_PARAMETER` |
| Memory allocation failed | `MEMORY_FAILURE` |
| Privilege / access denied | `PERMISSION_DENIED` |
| Operation is still in progress (background test) | `IN_PROGRESS` |
| Command was aborted | `ABORTED` |
| OS passthrough IOCTL failed | `OS_PASSTHROUGH_FAILURE` |

---

## Progress Reporting

A newline-delimited JSON progress reporting framework is **currently in progress** and not yet complete. Until it is fully wired up, long operations behave one of two ways: pass `pollForProgress = false` and poll status yourself with the per-operation `get_*_Progress` function, or pass `pollForProgress = true` and let the library print progress to stdout. The JSON callback mechanism described below reflects the intended final design:

```c
// Intended callback interface (in-progress, not yet fully wired):
static void my_progress_callback(const op_json_message* M_NONNULL msg, void* M_NULLABLE context)
{
    if (msg->has_percent)
    {
        printf("Progress: %.1f%%\n", msg->percent_complete);
    }
    if (msg->has_estimated_time_seconds)
    {
        printf("Time remaining: %" PRIu64 "s\n", msg->estimated_time_seconds);
    }
}
```

When implementing a new operation that reports progress, use `op_emit_json_callback` (from `operations.h`) to emit structured JSON messages. Schema version is `OP_JSON_SCHEMA_VERSION ("1.0")`. **Helping complete this wiring across all operations is a high-value contribution** — the framework exists but many operations have not been connected to it yet.

---

## Operations by Functional Area

### Drive Information (`drive_info.h`)

- `get_Drive_Information(device, driveInfoData*)` — collects all available drive info into a protocol-agnostic `driveInformation` structure (serial number, model, firmware, capacity, sector size, interface speed, temperatures, etc.)
- `print_Drive_Information(driveInfoData*, bool, bool)` — formats and prints to stdout
- `print_Device_Information(device)` — quick one-liner summary

Use the returned structure fields, not raw `device->drive_info` members, for display — the returned structure translates protocol-specific values into common types.

### Device Self Test (`dst.h`)

- `run_DST(device, DSTType, pollForProgress, captiveForeground, ignoreMaxTime)` — runs DST in background or foreground mode
- `send_DST(device, DSTType, captiveForeground, timeout)` — sends the DST command without polling
- `abort_DST(device)` — aborts an active DST
- `get_DST_Progress(device, &percentComplete, &status)` — polls DST status
- `print_DST_Progress(device)` — human-readable progress output

DST types: `DST_TYPE_SHORT`, `DST_TYPE_LONG`, `DST_TYPE_CONVEYANCE` (ATA only).

The `captiveForeground` mode (foreground/captive DST) exists in both the ATA and SCSI specifications, but **in practice it is only reliably supported on ATA devices**. SCSI devices may accept the command but behavior varies widely.

### SMART (`smart.h`)

- `get_SMART_Attributes(device, smartLogData*)` — retrieves SMART attributes for ATA devices. Technically has some NVMe handling, but NVMe health data is structured differently and does not map cleanly to "attributes". For NVMe devices, use the NVMe-specific SMART/Health log functions instead of this one.
- `get_Attribute_Name(device, attrNum, name)` — look up human-readable attribute name (ATA only)
- SMART attribute IDs are **not standardized** between vendors — treat them as vendor-specific unless the drive reports a known standard attribute (POH = 9, Reallocated Sector Count = 5, etc.)
- For NVMe: use the dedicated NVMe SMART/Health log retrieval functions. NVMe health data is a fixed-format log (log ID 0x02) with fully standardized fields — distinct from the attribute/threshold model of ATA SMART.

### Sanitize (`sanitize.h`)

- `get_SCSI_Sanitize_Supported_Features`, `get_ATA_Sanitize_Device_Features`, `get_NVMe_Sanitize_Supported_Features` — feature detection before issuing sanitize
- `run_Sanitize_Operation(device, sanitizeOperation, ...)` — block erase / crypto erase / overwrite dispatch
- Maximum passes: `ATA_NVME_MAX_SANITIZE_OVERWRITE_PASSES` = 16, `SCSI_MAX_SANITIZE_OVERWRITE_PASSES` = 31

Always check `sanitizeFeaturesSupported.sanitizeCmdEnabled` before any sanitize command.

### Firmware Download (`firmware_download.h`)

- `firmware_Download(device, firmwareUpdateData*)` — handles segmented / deferred / immediate / activate modes
- Fill `firmwareUpdateData` carefully:
  - `size` must be `sizeof(firmwareUpdateData)` (versioned struct)
  - `version` must be `FIRMWARE_UPDATE_DATA_VERSION`
  - `dlMode = FWDL_UPDATE_MODE_AUTOMATIC` lets the library pick the best method for the device
  - `segmentSize = FIRMWARE_UPDATE_SEGMENT_SIZE_AUTO` lets the library choose the optimal segment size

The library handles ATA `DOWNLOAD MICROCODE`, SCSI `WRITE BUFFER`, and NVMe `Firmware Image Download` + `Firmware Commit` internally.

### Format (`format.h`)

Drive-level format operations. SCSI FORMAT UNIT, NVMe Format NVM. ATA does not have a user-visible format command.

### Host Erase and Secure Erase (`host_erase.h`)

- `erase_Range(device, eraseStartLba, eraseEndLba, ...)` — writes patterns across an LBA range
- `erase_Drive(device, ...)` — full drive erase

**IEEE 2883 terminology — Clear vs. Purge**: Host erase operations are a *Clear* in IEEE 2883 terms: they overwrite only the visible LBA space accessible by the host. A *Purge* reaches all areas where data may reside — reallocated sectors, reserved blocks, write caches, and any other media that may have held user data in the past, present, or future. When in doubt about what an erase method covers, assume it is only a Clear.

| Method | IEEE 2883 | What it covers |
|--------|-----------|----------------|
| Host overwrite (this module) | Clear | Visible LBA space only |
| Normal ATA Security Erase | Clear | Visible LBA space (equivalent to overwrite) |
| Enhanced ATA Security Erase | Purge | All media including reallocated/reserved |
| Sanitize (any variant) | Purge | All media including reallocated/reserved |

### Power Control (`power_control.h`)

- `enable_Disable_EPC_Feature(device, feature)`, `enable_Disable_APM_Feature(device, enable)` — EPC and APM power management
- `set_APM_Level`, `get_APM_Level`, `get_EPC_Settings`, etc.

### ATA Security (`ata_Security.h`)

ATA Security feature set (password-based encryption lock). Primarily an ATA feature, but NVMe devices that comply with the SAT specification for Security Protocol may also support ATA Security commands issued via Security Send/Receive. Use the support-detection function in this header to determine capability rather than gating solely on `drive_type == ATA_DRIVE`.

- `set_ATA_Security_Password`, `run_Freeze_ATA_Security`, `disable_ATA_Security_Password`
- `start_ATA_Security_Erase` (pass `ATA_SECURITY_ERASE_ENHANCED_ERASE` or `ATA_SECURITY_ERASE_STANDARD_ERASE` as `eraseType`)

### Logs (`logs.h`)

Protocol-agnostic log retrieval:
- `get_ATA_Log_Size(device, logAddress, &logSize, &maxPages, &gpl)` — determine available log page size
- `get_SCSI_Log_Size(device, logPage, logSubPage, &logSize)` — SCSI equivalent
- `pull_ATA_Log(device, logAddress, logSubPage, buf, bufLen)` / `pull_SCSI_Log(...)` / `pull_NVMe_Log(...)` — retrieve log data

### Trim / Unmap (`trim_unmap.h`)

- `trim_Unmap_Range(device, startLba, endLba)` — sends ATA DSM TRIM, SCSI UNMAP, or NVMe Dataset Management to the device
- Use the support-detection function provided in this header to check whether TRIM/UNMAP is available before calling. Do not access `drive_info` fields directly for this check — the function encapsulates the per-protocol detection.

### Write Same (`writesame.h`)

- `write_Same(device, startLba, numLbas, pattern, patternLen)` — writes a repeating pattern or zeros across an LBA range using the drive's built-in command. Dispatches to:
  - **ATA**: SCT Write Same — supported on drives that implement the SCT feature set (not all do; check support before calling)
  - **ATA**: ATA Write Zeroes command — may or may not be wired up yet
  - **SCSI**: WRITE SAME
  - **NVMe**: Write Zeroes — NVMe has no write-same command, only write-zeroes; may or may not be wired up yet
- **ATA WRITE UNCORRECTABLE is not part of this module** — it is in `defect.h` and serves a different purpose (injecting pseudo-uncorrectable errors for testing).

### Zoned Operations (`zoned_operations.h`)

ZAC/ZBC zone management commands (Report Zones, Open/Close/Finish/Reset Zone):
- Check `device->drive_info.zonedType` for `ZONED_TYPE_HOST_MANAGED` before using
- `ZONED_TYPE_HOST_AWARE` is effectively obsolete — it was a short-lived hybrid mode where a drive could be accessed as both a conventional block device and a host-managed zoned device. It did not see widespread adoption. Current deployments are either conventional (device-managed) or host-managed; treat HOST_AWARE the same as host-managed if encountered.

### NVMe-Specific Operations (`nvme_operations.h`)

NVMe features not available in other protocols:
- Namespace management
- Controller registers
- Telemetry log retrieval
- NVMe Error log parsing

---

## Do Not Duplicate Protocol Logic

If a command already exists in opensea-transport (`ata_helper_func.h`, `scsi_helper_func.h`, `nvme_helper_func.h`, or `cmds.h`), call it — do not rebuild it in operations. The rule is:

- **opensea-transport** owns: command construction, CDB building, sense-data parsing, RTFRs retrieval, passthrough dispatch
- **opensea-operations** owns: protocol decision (ATA vs. SCSI vs. NVMe), feature detection, result interpretation, progress reporting, user-facing data structures

In practice, some logic that belongs in transport has accumulated in operations over time and has not been moved yet — a known technical debt. Follow the correct boundary for new code. Note that the dependency flows *upward*: operations calls transport, so a fix in transport automatically benefits operations. Building raw CDBs inside operations is a maintenance hazard because the command's transport-layer details are now duplicated and the two copies can drift.
