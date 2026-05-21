---
description: 'RAID passthrough support in opensea-transport — scan infrastructure, CSMI/Intel RST/CISS implementations, and institutional knowledge for future RAID types (MegaRAID, Adaptec, Highpoint, 3ware, Areca, VROC, AMD RAIDCORE)'
applyTo: 'subprojects/opensea-transport/**/*.c, subprojects/opensea-transport/**/*.h'
---

# RAID Passthrough in opensea-transport

## The Most Important Thing About RAID

> **A RAID controller is not a drive. A logical volume is not a physical drive. This distinction is the single most common source of confusion when working with RAID storage.**

When a RAID controller is operating in any mode other than explicit JBOD (Just a Bunch of Disks) passthrough, **every device the OS sees is a logical presentation from the controller**. This is true even for a "single drive RAID" — a RAID-0 array of one disk, or a passthrough logical drive that maps 1:1 to a physical disk. The controller still stands between the OS and the physical media.

Consequences of talking to a logical RAID volume instead of a physical drive:

- **ATA passthrough does not work.** The logical device speaks SCSI (or a SCSI-like protocol); the controller does not forward ATA commands to the physical drive unless JBOD mode is active.
- **Vendor-unique data is invisible.** SMART attributes, vendor-specific log pages, and proprietary NVMe commands are intercepted or stripped by the controller. What you read is whatever the controller chooses to expose, not what the physical drive would report.
- **Model number and serial number are wrong.** The controller reports its own logical volume identifiers, not the drive's true model and serial number. On some controllers the serial number field is a RAID set UUID. On others it is simply blank or zero-filled.
- **Capacity may differ.** Some controllers reserve drive space for metadata, parity, or hot-spare regions. The reported capacity of the logical volume does not equal the physical drive capacity.
- **Physical drive health is not visible at all.** The logical device surface does not expose SMART attributes, drive temperature, error counts, or any per-drive health data. The controller presents only RAID-level health through controller-specific means (there is a SCSI-level specification for RAID controller health reporting, but it covers array status, not individual physical drive health). To read physical drive health, you must use the controller's physical device passthrough.

**The only way to get real, trustworthy data from a physical drive behind a RAID controller is to use a controller-specific passthrough mechanism** — CSMI STP/SSP passthrough, CISS, MegaRAID physical device IOCTLs, or equivalent. This is exactly what the code in this file implements.

**Even with a working passthrough, the controller firmware can silently block individual commands.** A passthrough channel that accepts arbitrary CDBs in principle may still reject specific commands at the firmware level — sanitize, security, firmware download, and certain diagnostic commands are common targets. This blocking is rarely documented anywhere. It is discovered through trial and error: the command goes in, the controller drops it or returns a generic error, and no explanation is given. Do not assume that a working passthrough for one command means all commands work.

When JBOD mode is explicitly set on the controller, physical drives appear as direct-attach devices and need no special passthrough — but verify JBOD mode is actually active; do not assume it based on the array configuration UI.

---

## Philosophy and Architecture

RAID passthrough is fundamentally different from direct-attach storage. The goal is **transparent passthrough**: once a RAID-encapsulated physical drive is opened and a `tDevice` is constructed for it, the rest of opensea-transport and opensea-operations must not know or care that the device is behind a RAID controller. This transparency is achieved by replacing `tDevice->issue_io` with a RAID-specific function pointer at open time.

### The Open-Source Challenge

Vendors typically provide internal C libraries for RAID passthrough. These cannot be used in open-source projects. The only approach available for open-source implementations is:

1. Read the kernel driver source for the target RAID controller
2. Identify the IOCTL codes the driver exposes
3. Reconstruct the IOCTL buffer structures from driver source
4. Implement the passthrough using those structures directly

This is painstaking work. **Smartctl** has completed this for several controllers and is the best public reference implementation, but its focus is narrower than openSeaChest's (monitoring only, not reconfiguration or firmware), and some RAID support has been deliberately disabled in smartctl due to discovered bugs and instability. Review smartctl source critically — do not copy bugs.

### No Universal Standard

CSMI (Common Storage Management Interface) was proposed by HP and taken to the T10 standards committee, where it was drafted under the name **SDI** (the exact expansion of the acronym is not recalled). The proposal failed to achieve adoption. The opposition was not purely technical — a kernel mailing list discussion at the time included the criticism: *"This will make us just like Windows with too many fucking IOCTLs."* The irony is that Linux storage drivers have since accumulated far more vendor-specific IOCTLs in aggregate than CSMI ever defined. Every RAID vendor together probably contributes more IOCTLs than CSMI's entire specification contained.

CSMI had other genuine technical problems, but the IOCTL proliferation concern was the most prominent opposition found in archived discussions. What CSMI was attempting — a common passthrough library across RAID vendors — is analogous to what the T11 committee achieved for Fibre Channel, which succeeded. CSMI simply never reached the adoption critical mass to follow the same path.

In practice today:
- **Intel** is the most common implementor encountered in the field. This does not mean their implementation is correct or complete — Intel's CSMI implementation has bugs, missing functionality, and conflicts with definitions that were added to the spec later. Different Intel chipset driver versions have subtly different behavior, so what works on one driver version may not work on another. The code in `csmi_helper.c` has been tuned to Intel's behavior because that is what was available to test against, not because Intel's interpretation is authoritative.
- One old LSI driver for the 92xx-series MegaRAID cards (encountered in some Dell configurations) is the only other implementation with any confirmed working support, and even that is uncertain in detail.
- Almost every other RAID driver implements *some* CSMI subset — usually enough to identify the controller — but never enough to rely on for physical drive discovery and command passthrough. Treat CSMI as a partial hint, not a complete solution.

The pattern across all RAID vendors is instead:
- Each vendor defines custom IOCTL codes specific to their driver.
- Most IOCTL definitions mirror their Windows counterparts, so a Linux implementation has a strong chance of working on Windows against the same driver, and vice versa. This is how CSMI, OpenFabrics, and Intel RST drivers all behave.
- Almost all vendor IOCTLs are limited to **16-byte CDBs** and approximately **64 bytes of sense data**. Design accordingly — do not assume more.

---

## Relevant Files

| File | Purpose |
|------|---------|
| `include/raid_scan_helper.h` | `raidTypeHint` bitfield and `raidHandleToScan` linked-list types; list management function declarations |
| `src/raid_scan_helper.c` | Implementation of the 4 list management functions |
| `include/csmi_helper_func.h` | All public CSMI function declarations; requires `ENABLE_CSMI` |
| `src/csmi_helper.c` | Full CSMI implementation (~6800 lines); includes Intel RST dispatch |
| `include/csmi_helper.h` | CSMI struct and constant definitions (internal types) |
| `include/intel_rst_helper.h` | Intel RST public API; requires `ENABLE_INTEL_RST` |
| `src/intel_rst_helper.c` | Intel RST implementation; conditionally includes `csmi_helper.h` |
| `include/intel_rst_defs.h` | Intel RST IOCTL constant definitions |
| `include/ciss_helper_func.h` | CISS/HP SmartArray public API; requires `ENABLE_CISS` and `__unix__` |
| `src/ciss_helper.c` | CISS implementation (Linux and FreeBSD) |
| `include/ciss_helper.h` | CISS struct and constant definitions |
| `src/csmi_legacy_pt_cdb_helper.c` | Legacy CSMI CDB passthrough helpers |

---

## RAID Scan Infrastructure

### `raidTypeHint` — Compile-Time Allocation

The `raidTypeHint` bitfield (defined in `raid_scan_helper.h`) marks which RAID type(s) a handle is suspected to be. OS device scan code sets hints when adding handles; RAID implementations consume handles from the list.

```c
typedef struct s_raidTypeHint {
    bool unknownRAID : 1;   // Try all RAID libs on this handle
    bool csmiRAID    : 1;   // CSMI (Intel/AMD/HP via CSMI)     — IMPLEMENTED
    bool cissRAID    : 1;   // HP CISS (SmartArray/HPSA)        — IMPLEMENTED
    bool megaRAID    : 1;   // LSI/Avago/Broadcom MegaRAID      — NOT YET
    bool adaptecRAID : 1;   // Adaptec/PMC/Microsemi ASR series — NOT YET
    bool highpointRAID : 1; // Highpoint                        — NOT YET
    bool areccaRAID  : 1;   // Areca                            — NOT YET
    bool intelVROC   : 1;   // Intel VROC NVMe RAID             — NOT YET
    // NOTE: 3ware and AMD RAIDCORE have no bits yet — add bits when implementing
} raidTypeHint;
```

When adding a new RAID type, add a bit here first.

### `raidHandleToScan` — Singly-Linked List

```c
#define RAID_HANDLE_STRING_MAX_LEN 32

typedef struct s_raidHandleToScan {
    struct s_raidHandleToScan* M_NULLABLE next;
    char         handle[RAID_HANDLE_STRING_MAX_LEN];
    raidTypeHint raidHint;
    eSCSIPeripheralDeviceType systemDeviceType;
    adapterInfo  adapter_info;
    driverInfo   driver_info;
} raidHandleToScan, *ptrRaidHandleToScan;
```

### List Management Functions

| Function | Behaviour |
|----------|-----------|
| `add_RAID_Handle(currentPtr, handle, hint)` | Always appends to `currentPtr->next`; if `currentPtr == NULL`, allocates a new head |
| `add_RAID_Handle_If_Not_In_List(listBegin, currentPtr, handle, hint)` | Walks from `listBegin`, deduplicates by `strcmp(handle)`; calls `add_RAID_Handle` only if not found |
| `remove_RAID_Handle(toRemove, previous)` | Fixes `previous->next`; frees `toRemove`; returns `toRemove->next` |
| `delete_RAID_List(listBegin)` | Frees all nodes |

**Why deduplication matters**: Some HBAs expose both RAID volumes and JBOD passthrough on the same OS handle. The OS scan code may encounter the same handle via multiple code paths. `add_RAID_Handle_If_Not_In_List` prevents double-scanning the same controller.

### OS→RAID-Lib Handoff Pattern

The pattern is the same for every RAID type:

1. OS scan code calls `add_RAID_Handle_If_Not_In_List` with a suspected handle and appropriate `raidTypeHint` bits set.
2. The matching RAID library's `get_*_RAID_Device_Count` function receives the list, walks it, and attempts to open each handle whose hint matches.
3. Successfully opened handles are consumed (removed from the list via `remove_RAID_Handle`); handles that do not match the RAID type are left alone.
4. `get_*_RAID_Device_List` then enumerates the physical drives reachable through each confirmed controller.

---

## CSMI — Common Storage Management Interface

### What CSMI Is

CSMI is a SAS HBA management IOCTL standard from SNIA / the SAS consortium era. It was intended to provide a uniform way to identify controllers, query topology, and issue SATA/SAS passthrough commands. In practice, it was never fully adopted.

**Primary use**: Intel chipset motherboard RAID controllers on **Windows** (drivers `iaStorAC`, `iaStorVD`, `iaStor`). The code is compiled for Linux as well and can theoretically work there, but Linux CSMI is not actively used or well-tested.

**Compile guard**: `#if defined(ENABLE_CSMI)` wraps all CSMI code.

### Platform Dispatch

| Platform | Mechanism |
|----------|-----------|
| Windows | `DeviceIoControl(handle, IOCTL_SCSI_MINIPORT, buffer, size, buffer, size, &bytesReturned, &overlapped)` with overlapped I/O |
| Linux | `ioctl(fd, ioctlCode, ioctlBuffer)` |

The Windows path uses a single in/out buffer (same pointer for both `lpInBuffer` and `lpOutBuffer`). Overlapped I/O is used so the call can be made from threads.

### `IOCTL_HEADER` Layout Differences

The CSMI spec's `IOCTL_HEADER` (`SRB_IO_CONTROL` on Windows) is filled differently per platform:

| Field | Windows | Linux |
|-------|---------|-------|
| `Signature[8]` | Required; matches driver signature | Not used |
| `ControlCode` | Set to the CSMI IOCTL code | Not used |
| `HeaderLength` | `sizeof(SRB_IO_CONTROL)` | Not used |
| `IOControllerNumber` | Not used | Set to `controllerNumber` param |
| `Direction` | Not used | Set to `CSMI_SAS_DATA_READ` or `WRITE` |

The `controllerNumber` parameter present on all public CSMI functions is **Linux-only**. Windows ignores it. It is kept in the API for future cross-platform consistency.

### Internal I/O Structures

```c
// Input to issue_CSMI_IO() — not exposed publicly
typedef struct s_csmiIOin {
    CSMI_HANDLE deviceHandle;
    uint32_t    ioctlBufferSize;
    M_SIZED_BY(ioctlBufferSize) void* M_NONNULL ioctlBuffer;
    uint32_t    ioctlCode;
    char        ioctlSignature[8];   // fixed 8 bytes per CSMI spec — never 9
    uint32_t    timeoutInSeconds;
    uint32_t    dataLength;          // sizeof(struct) - sizeof(IOCTL_HEADER)
    uint32_t    controllerNumber;    // Linux only
    const tDevice* M_NULLABLE device;
    eVerbosityLevels csmiVerbosity;  // LEGACY — still set alongside device
    uint16_t    ioctlDirection;      // CSMI_SAS_DATA_READ or CSMI_SAS_DATA_WRITE
} csmiIOin;
```

> **Legacy field note**: `csmiIOin.csmiVerbosity` remains in the struct even after the public API was refactored to use `const tDevice*`. Both fields are still set in every CSMI function body:
> ```c
> ioIn.device         = device;
> ioIn.csmiVerbosity  = get_Device_Verbosity(device);
> ```
> New code should use `ioIn.device`; `csmiVerbosity` is retained for the internal dispatch layer.

### Known CSMI Driver Types

The `eKnownCSMIDriver` enum and `get_Known_CSMI_Driver_Type()` identify the underlying driver from `csmi_Get_Driver_Info` output:

| Enum constant | Driver name | Controller |
|---------------|-------------|-----------|
| `CSMI_DRIVER_INTEL_RST` | `iaStorAC` | Intel RST (Skylake+) |
| `CSMI_DRIVER_INTEL_VROC` | `iaStorE` | Intel VROC NVMe RAID |
| `CSMI_DRIVER_INTEL_RST_VD` | `iaStorVD` | Intel RST Volume Driver |
| `CSMI_DRIVER_INTEL_GENERIC` | `iaStor` | Intel generic |
| `CSMI_DRIVER_HPCISS` | `HpCISSS3` | HP SmartArray (CISS) |
| `CSMI_DRIVER_ADAPTEC` | `arcsas` | Adaptec |
| `CSMI_DRIVER_AMD_RCRAID` | `rcraid` | AMD RAIDCORE |

#### AMD RAIDCORE / `rcraid`

AMD motherboards use "RAIDCORE" for RAID. The driver is `rcraid`. Some CSMI passthrough exists within it (hence its presence in `eKnownCSMIDriver`), but the full IOCTL interface was a licensed technology — legal barriers prevented Seagate from obtaining full access to the specification. If implementing AMD RAIDCORE support in the future, kernel driver source analysis will be required.

#### HP CISS via CSMI

The HP `HpCISSS3` driver appears in `eKnownCSMIDriver` because it reports itself via CSMI's driver info interface. However, CISS and Adaptec drivers only implement a limited CSMI subset — believed to be SMP (SAS Management Protocol) passthrough related functions — not the full CSMI physical drive passthrough. Do not assume CSMI alone is sufficient to access physical drives behind an HP SmartArray; use the CISS interface instead.

### Standard IOCTL Dispatch Pattern

Every public CSMI function follows the same boilerplate:

```c
eReturnValues csmi_Some_Function(CSMI_HANDLE deviceHandle, uint32_t controllerNumber,
                                  PCSMI_SAS_SOME_BUFFER buffer, ..., const tDevice* M_NULLABLE device)
{
    eReturnValues ret = SUCCESS;
    csmiIOin      ioIn;
    csmiIOout     ioOut;
    safe_memset(&ioIn, 0, sizeof(csmiIOin));
    safe_memset(&ioOut, 0, sizeof(csmiIOout));

    ioIn.deviceHandle    = deviceHandle;
    ioIn.ioctlCode       = CC_CSMI_SAS_SOME_CODE;
    safe_strcpy(ioIn.ioctlSignature, sizeof(ioIn.ioctlSignature), CSMI_SAS_SIGNATURE, ...);
    ioIn.ioctlBuffer     = buffer;
    ioIn.ioctlBufferSize = sizeof(CSMI_SAS_SOME_BUFFER);
    ioIn.dataLength      = sizeof(CSMI_SAS_SOME_BUFFER) - sizeof(IOCTL_HEADER);
    ioIn.timeoutInSeconds = 15;
    ioIn.controllerNumber = controllerNumber;
    ioIn.device          = device;
    ioIn.csmiVerbosity   = get_Device_Verbosity(device);
    ioIn.ioctlDirection  = CSMI_SAS_DATA_READ;

    ret = issue_CSMI_IO(&ioIn, &ioOut);

    if (ioOut.sysIoctlReturn == CSMI_SYSTEM_IOCTL_SUCCESS)
    {
        ret = csmi_Return_To_OpenSea_Result(buffer->IoctlHeader.ReturnCode);
    }
    else
    {
        ret = OS_PASSTHROUGH_FAILURE;
    }
    return ret;
}
```

All 13+ public CSMI functions follow this exact pattern. When implementing a new CSMI function, copy this boilerplate exactly.

### Public CSMI API Summary

All functions take `CSMI_HANDLE deviceHandle, uint32_t controllerNumber, PCSMI_SAS_*_BUFFER buffer, ..., const tDevice* M_NULLABLE device`.

| Function | Purpose |
|----------|---------|
| `csmi_Get_Driver_Info` | Driver name, version; used to identify known driver type |
| `csmi_Get_Controller_Configuration` | Controller model, firmware, SAS address |
| `csmi_Get_Controller_Status` | Status flags |
| `csmi_Controller_Firmware_Download` | Firmware update for the HBA itself |
| `csmi_Get_RAID_Info` | Number of RAID sets |
| `csmi_Get_RAID_Config` | Config for a specific RAID set |
| `csmi_Get_RAID_Features` | **Not universally supported** — many controllers return unsupported |
| `csmi_Get_Phy_Info` | Phy topology: attached addresses, rates |
| `csmi_Set_Phy_Info` | Configure phy parameters |
| `csmi_Get_Link_Errors` | Per-phy error counters; optional reset |
| `csmi_Get_SATA_Signature` | SATA FIS signature for a phy |
| `csmi_Get_SCSI_Address` | Resolve SAS address + LUN → SCSI address |
| `csmi_Get_Device_Address` | Reverse: host/path/target/lun → device info |
| `csmi_Get_Connector_Info` | Physical connector information |
| `send_CSMI_IO(ScsiIoCtx*)` | Set as `tDevice->issue_io` for CSMI devices |
| `handle_Supports_CSMI_IO(CSMI_HANDLE, device)` | Returns bool; calls `csmi_Get_Basic_Info` |
| `device_Supports_CSMI_With_RST(device)` | Returns bool; Windows only; requires RST firmware support |
| `jbod_Setup_CSMI_Info(CSMI_HANDLE, tDevice*, ...)` | Set up CSMI JBOD device: allocates `csmiDeviceData`, identifies driver, builds SAS address |
| `get_CSMI_RAID_Device_Count(...)` | Enumerate controllers; populate list |
| `get_CSMI_RAID_Device_List(...)` | Enumerate physical drives; populate `tDevice` array |

### JBOD vs RAID Mode

**JBOD mode** (`jbod_Setup_CSMI_Info`): Used when the OS already presents the physical drive directly (e.g., pass-through mode on the HBA). Sets up `device->os_info.csmiDeviceData` with addressing info. On Windows uses `device->os_info.scsiSRBHandle`. Calls `csmi_Get_Basic_Info` (driver info + controller config + status), then attempts `csmi_Get_Device_Address` — **SKIPPED for `CSMI_DRIVER_HPCISS`** due to a known hang bug (the HP driver does not respond to this IOCTL and the call hangs indefinitely). Falls back to scanning RAID config for the SAS address.

**RAID mode** (`get_CSMI_RAID_Device_Count` / `get_CSMI_RAID_Device_List`): Used when enumerating physical drives hidden behind RAID volumes. Walks controllers, queries RAID topology, constructs `tDevice` for each physical member.

### SAS (SSP) vs SATA (STP) Passthrough

`send_CSMI_IO` dispatches based on the device's interface type:

- **SSP passthrough** (`csmi_SSP_Passthrough`): For SAS-attached physical drives. Uses `CSMI_SAS_SSP_PASSTHROUGH` IOCTL. Constructs a SCSI CDB-based request.
- **STP passthrough** (`csmi_STP_Passthrough`): For SATA-attached physical drives. Uses `CSMI_SAS_STP_PASSTHROUGH` IOCTL. Works at the FIS (Frame Information Structure) level.

If STP passthrough is not supported by the controller, `csmi_STP_Passthrough` sets a retry flag. `send_CSMI_IO` then retries as SSP with a SAT-style CDB (A1h or 85h opcode). If *that* returns invalid opcode, it falls back to `ATA_PASSTHROUGH_CSMI` legacy passthrough mode.

### STP RTFRs — ATA Return Registers via Descriptor Sense

After an STP command completes, the FIS response (D2H or PIO Setup) is packed into descriptor-format sense data in `scsiIoCtx->psense`. This is so the ATA layer above can read RTFRs from the standard location:

```
psense[0]  = SCSI_SENSE_CUR_INFO_DESC (0x72)
psense[1]  = 0x01 (recovered error / check condition)
psense[2]  = 0x00 (ASC)
psense[3]  = 0x1D (ASCQ = ATA Passthrough Information Available)
psense[7]  = 0x0E (additional sense length)
psense[8]  = 0x09 (ATA Return descriptor code)
psense[9]  = 0x0C (additional descriptor length)
psense[10] = extend bit (set for 48-bit commands)
psense[11] = Error register
psense[12] = Sector Count Ext
psense[13] = Sector Count
psense[14] = LBA Low Ext
psense[15] = LBA Low
psense[16] = LBA Mid Ext
psense[17] = LBA Mid
psense[18] = LBA Hi Ext
psense[19] = LBA Hi
psense[20] = Device/Head
psense[21] = Status
```

This is the standard ATA descriptor return format (SAT spec). The caller layer does not need to know the response came from a FIS.

---

## Intel RST — NVMe Tunneling via CSMI

### What It Is

Intel RST (Rapid Storage Technology) is a Windows-only driver layer that runs on top of Intel chipset CSMI. It adds an NVMe passthrough mechanism, allowing NVMe Admin and I/O commands to be tunneled through the RAID driver to NVMe devices.

**Compile guard**: `#if defined(ENABLE_INTEL_RST)` — independent of `ENABLE_CSMI` but tightly coupled:
- `csmi_helper.c` includes `intel_rst_helper.h`
- `intel_rst_helper.c` conditionally includes `csmi_helper.h` when `ENABLE_CSMI`

**Windows only**. There is no Linux Intel RST implementation.

### Public API

| Function | Purpose |
|----------|---------|
| `send_Intel_NVM_Command(nvmeCmdCtx*)` | Tunnel NVMe Admin/IO commands; driver may filter some commands |
| `send_Intel_NVM_Firmware_Download(nvmeCmdCtx*)` | NVMe firmware download via RST IOCTL |
| `send_Intel_NVM_SCSI_Command(ScsiIoCtx*)` | SCSI→NVMe translation internally (SNTL-style); receives SCSI, converts to NVMe |
| `supports_Intel_Firmware_Download(device)` | Capability check; returns bool |
| `send_Intel_Firmware_Download(ScsiIoCtx*)` | ATA or SCSI firmware download via RST IOCTL |

### NVMe JBOD Gap

`jbod_Setup_CSMI_Info` explicitly does **not** handle Intel NVMe devices in JBOD mode. The code contains a comment stating "these devices will be handled separately." If implementing Intel NVMe JBOD via RST, a new setup path is needed.

### Detection

`device_Supports_CSMI_With_RST(device)` combines:
1. `handle_Supports_CSMI_IO` — confirms CSMI is working
2. `supports_Intel_Firmware_Download` — confirms RST NVMe firmware IOCTL is available

Both must succeed.

---

## CISS — HP SmartArray / CCISS / HPSA / SmartPQI

### What It Is

CISS is HP's RAID controller passthrough interface, used by controllers sold under the names SmartArray, CCISS, HPSA, and SmartPQI. The interface exposes physical drives behind the RAID controller for direct command passthrough.

**Compile guard**: `#if defined(ENABLE_CISS)` + `#if defined(__unix__)`

**Supported platforms**: Linux and FreeBSD. Windows support for CISS passthrough **does not exist** in the current implementation. The mechanism for Windows CISS passthrough is unknown and would need to be researched from driver source.

### Device Paths

- Linux: `/dev/ciss*` (older cciss driver) and `/dev/sg*` (hpsa/smartpqi drivers with sg nodes)
- FreeBSD: `/dev/smartpqi*` (SmartPQI driver)

### Public API

| Function | Purpose |
|----------|---------|
| `ciss_filter(const struct dirent*)` | `scandir` filter for `/dev/ciss*` |
| `smartpqi_filter(const struct dirent*)` | FreeBSD only; returns 0 on other OSes |
| `is_Supported_ciss_Dev(const char* devName)` | Validates device name string |
| `issue_io_ciss_Dev(ScsiIoCtx*)` | **Replaces `tDevice->issue_io`** for CISS devices |
| `get_CISS_RAID_Device(filename, tDevice*)` | Open a specific CISS device |
| `close_CISS_RAID_Device(tDevice*)` | Close a CISS device |
| `get_CISS_RAID_Device_Count(uint32_t*, flags, ptrRaidHandleToScan**)` | Enumerate controllers |
| `get_CISS_RAID_Device_List(tDevice*, size, versionBlock, flags, ptrRaidHandleToScan**)` | Enumerate physical drives |

### CSMI Subset Note

HP's CISS/HPSA driver reports itself via CSMI's driver info interface (identified as `CSMI_DRIVER_HPCISS`). It implements a limited CSMI subset, believed to cover SMP (SAS Management Protocol) passthrough only. This subset is insufficient for physical drive command passthrough. Use the CISS interface, not CSMI, for HP controllers.

---

## Future RAID Implementations

These RAID types have placeholder bits in `raidTypeHint` (or will need them added) but have no implementation yet. The approach for all of them is the same: read kernel driver source, identify IOCTL structures, and implement accordingly.

### SCSI-Only vs. SCSI + ATA Passthrough

When implementing passthrough for any RAID controller, determine what passthrough modes the driver actually exposes:

- **SCSI passthrough only** — the most common case. The RAID driver accepts SCSI CDBs directed at physical drives. ATA commands must be tunneled using SAT ATA PASS-THROUGH CDBs (A1h / 85h). See the ATA instruction file for full SAT CDB byte layouts and field mapping.
- **Separate ATA passthrough IOCTL** — some drivers provide a dedicated ATA-level IOCTL in addition to SCSI passthrough. This path can carry raw ATA TFR registers directly without SAT CDB encapsulation, analogous to CSMI's STP passthrough. Check the kernel driver source for both SCSI and ATA IOCTL codes before assuming only SCSI is available.

When only SCSI passthrough is available, ATA passthrough via SAT still works for most commands but has constraints: sense data is limited to 64 bytes (see [16B CDB / 64B sense limits above](#common-implementation-approach)), which means 48-bit RTFR data may be truncated in fixed-format sense responses. For full 48-bit RTFRs through SCSI-only RAID passthrough, use descriptor-format sense (requires `CK_COND=1` in the SAT CDB) and verify the RAID driver honors it.

### Common Implementation Approach

When implementing any new RAID type:

1. **Read the kernel driver source** — this is the only public source of IOCTL structure definitions for open-source work. Linux kernel drivers are in `drivers/scsi/`, FreeBSD drivers in `sys/dev/`.
2. **Mirror the IOCTL structures** — define structs that match the driver's expectations. Be careful with alignment and packing (`#pragma pack` or `__attribute__((packed))`).
3. **Almost all are limited to 16-byte CDBs and ~64 bytes of sense data** — do not assume more capacity; design around this constraint.
4. **Check for a CSMI subset** — the driver may respond to some CSMI IOCTLs for controller identification, even if not for passthrough. Use it for detection; do not rely on it for passthrough.
5. **Check smartctl source** — `smartmontools/` is a useful reference but treat it critically. Some RAID support was disabled due to discovered bugs.
6. **Windows portability**: Most RAID vendors define their IOCTL codes to mirror their Linux counterparts. A well-factored implementation should work on Windows against the same driver with minimal changes.
7. **Add a `raidTypeHint` bit** if one does not already exist, then add device count and device list functions following the CSMI/CISS pattern.

### 3ware (AMCC/LSI)

3ware is an older SAS/SATA RAID controller brand (originally AMCC, acquired by LSI). **Old but doable for a lot of operations.** 3ware does not have a `raidTypeHint` bit yet — one must be added.

- Linux driver: `3w-xxxx` (ATA RAID), `3w-9xxx` (SAS)
- FreeBSD: historically supported
- Device nodes: `/dev/twe*`, `/dev/twa*`, `/dev/tws*` on Linux; `/dev/tws*` on FreeBSD
- The controller exposes physical drives via proprietary IOCTLs
- Smartctl has historically good 3ware support — a reasonable starting reference

### MegaRAID (LSI / Avago / Broadcom)

`megaRAID` bit already exists in `raidTypeHint`. MegaRAID is the enterprise SAS/SATA/NVMe RAID line now owned by Broadcom.

- Linux driver: `megaraid_sas`
- FreeBSD driver: `mrsas`
- Device nodes: `/dev/megaraid_sas_ioctl_node` (Linux dedicated ioctl node), `/dev/sda`-style for logical drives
- Physical drive passthrough: via `MEGASAS_IOC_FIRMWARE` ioctl with `MFI_CMD_PD_SCSI_IO` or `MFI_CMD_DCMD`
- MegaRAID uses a Management Frame (MFI) or Fusion MPT protocol depending on generation
- Smartctl has MegaRAID support; review `smartctl/dev_areca.cpp` and `megaraid.cpp` for structural guidance
- Windows: MegaRAID Windows driver uses the same ioctl framework, so a Linux implementation is very likely portable

### Adaptec ASR Series (PMC-Sierra / Microsemi / Microchip)

`adaptecRAID` bit already exists. This covers the ASR (Adaptec SAS RAID) enterprise controllers.

- Linux driver: `aacraid`
- **Linux 3.x kernel milestone**: Starting around kernel 3.x, the Adaptec `aacraid` driver began exposing `/dev/sg` nodes for physical drives directly. On these kernels, passthrough discovery may not require custom IOCTLs — the device already appears as a passable sg node.
- Pre-3.x Linux: physical drive passthrough requires custom `aacraid` IOCTLs for discovery
- FreeBSD: `aac` driver
- The Adaptec driver implements some CSMI functions (identified as `CSMI_DRIVER_ADAPTEC` / driver name `arcsas`) but only a limited subset (believed to cover SMP-related IOCTLs)
- Smartctl has Adaptec support

### Highpoint RAID

`highpointRAID` bit already exists.

- Linux driver: `hptiop`, `hpt366`, `hpt37x` (older)
- FreeBSD: `hptiop`
- Highpoint exposes physical drives via proprietary IOCTLs
- Smartctl has some Highpoint support

### Areca RAID

`areccaRAID` bit already exists (note the spelling in the code is `areccaRAID`).

- Linux driver: `arcmsr`
- FreeBSD: `arcmsr`
- Device nodes: `/dev/arcmsr*`
- Smartctl has Areca support via `dev_areca.cpp` — a well-documented reference

### Intel VROC (NVMe RAID)

`intelVROC` bit already exists. Intel VROC (Virtual RAID On CPU) is Intel's NVMe RAID solution.

- The `iaStorE` driver is identified as `CSMI_DRIVER_INTEL_VROC` in `get_Known_CSMI_Driver_Type`
- VROC is fundamentally different from Intel RST: RST handles SATA/SAS, VROC handles NVMe RAID volumes
- NVMe RAID volume discovery via VROC is **not yet implemented** despite driver detection being present
- This is separate from `send_Intel_NVM_Command` (RST) — VROC uses different IOCTLs

### AMD RAIDCORE (`rcraid`)

No `raidTypeHint` bit exists yet. The AMD motherboard RAID driver is named `rcraid`.

- `rcraid` is already identified in `eKnownCSMIDriver` as `CSMI_DRIVER_AMD_RCRAID` because it partially reports via CSMI
- Some passthrough IOCTL exists in the Windows driver, but the full specification was a licensed/proprietary technology — legal barriers prevented Seagate from obtaining access
- Implementing support requires kernel driver source analysis
- Would need a new `raidTypeHint` bit when implementing

---

## Platform Support Matrix

| RAID Type | Linux | FreeBSD | Solaris/Illumos | VMware | Windows | OpenBSD/NetBSD |
|-----------|-------|---------|-----------------|--------|---------|---------------|
| CSMI | Compiled, not actively used | No | No | Follows Linux | Yes (primary) | No |
| Intel RST | No | No | No | No | Yes | No |
| CISS/HP SmartArray | Yes | Yes | Likely | Likely | Unknown passthrough | Unlikely |
| MegaRAID | Planned | Planned | Likely same as Linux | Follows Linux | Likely portable | Unlikely |
| Adaptec ASR | Planned | Planned | Likely | Follows Linux | Likely portable | Unlikely |
| Highpoint | Planned | Planned | Likely | Follows Linux | Likely portable | Unlikely |
| 3ware | Planned | Planned | Likely | Follows Linux | Likely portable | Unlikely |
| Areca | Planned | Planned | Likely | Follows Linux | Likely portable | Unlikely |
| Intel VROC | Planned | Unknown | Unknown | Unknown | Planned | No |
| AMD RAIDCORE | Unknown | Unknown | Unknown | Unknown | Some exists | No |

### Platform Notes

**FreeBSD**: The CAM (Common Access Method) subsystem exposes many physical drives at `/dev/passX` directly, including many behind RAID controllers. This significantly reduces RAID-specific work compared to Linux — CAM's passthrough infrastructure handles discovery for those devices. However, CAM does **not** surface every physical drive behind every controller; some RAID controllers do not expose physical members through CAM and will require vendor-specific IOCTL discovery. Check whether a specific controller's physical drives appear as `/dev/passX` nodes before assuming CAM is sufficient.

**Linux**: Most RAID passthrough is via driver-specific IOCTLs on `/dev/sg*`, `/dev/sd*`, or dedicated device nodes (e.g., `/dev/megaraid_sas_ioctl_node`). Adaptec is an exception on modern kernels (3.x+) where `/dev/sg` nodes appear for physical drives automatically.

**Solaris / Illumos**: Generally mirrors FreeBSD/Linux behaviour for RAID. Expect Linux IOCTLs to work on Illumos with the same or equivalent drivers.

**VMware**: Reuses Linux drivers extensively. Linux RAID implementations are very likely to work on VMware with the same drivers. **Notable exception**: NVMe support on VMware differs from Linux — do not assume NVMe RAID passthrough from Linux applies to VMware.

**OpenBSD / NetBSD**: These OSes use significantly different driver architectures. It is not clear that RAID passthrough will work at all on these platforms. They are low priority for RAID support.

**Windows**: Most vendor IOCTL structures mirror their Linux counterparts by design. A well-written Linux RAID implementation should be portable to Windows with the same vendor driver. This pattern is confirmed in CSMI, OpenFabrics, and Intel RST implementations.

---

## Common Patterns and Pitfalls

### Compile Guard Discipline

**Always** gate RAID code with the appropriate guard:

```c
#if defined(ENABLE_CSMI)
    // CSMI-specific code
#endif

#if defined(ENABLE_INTEL_RST)
    // RST-specific code
#endif

#if defined(ENABLE_CISS)
    // CISS-specific code
#endif
```

Never reference RAID-specific types, functions, or fields outside their guard. The guards are build-time options; RAID support is optional.

### Heap-Allocated Device Data

CSMI allocates per-device state on the heap and stores it in `device->os_info.csmiDeviceData`. This must be freed in the corresponding close/cleanup function. CISS follows the same pattern. When implementing new RAID types, follow this pattern — do not store large structs on the stack or as file-scope statics.

### The HPCISS `csmi_Get_Device_Address` Hang Bug

When the known driver type is `CSMI_DRIVER_HPCISS`, `jbod_Setup_CSMI_Info` explicitly skips the call to `csmi_Get_Device_Address`. The HP driver does not respond to this IOCTL and the call hangs indefinitely. This is a known, documented driver bug — do not remove the guard.

When implementing other RAID types, assume any IOCTL call could hang on a specific driver variant. Implement timeouts where the OS allows it, and document any driver-specific skips clearly.

### CSMI Signature Is Always 8 Bytes

The `ioctlSignature` field in `csmiIOin` is always exactly 8 bytes per the CSMI specification. Do not use 9 bytes (no null terminator space). Use `safe_strncpy` with count 8, not `safe_strcpy` which expects null termination.

### 16-Byte CDB and 64-Byte Sense Limits

Almost all RAID passthrough IOCTLs cap CDB length at 16 bytes and sense data at ~64 bytes. Never send a 32-byte CDB through a RAID interface. Always verify sense buffer sizes before filling `scsiIoCtx->psense`. This is not a soft limit — drivers will reject or truncate data silently.

### `send_CSMI_IO` Is the `issue_io` Hook

`send_CSMI_IO` is set as `tDevice->issue_io` for CSMI devices. When implementing new RAID types, create an equivalent `issue_io_yourraid_Dev(ScsiIoCtx*)` function and set it as the hook during device open. The function must satisfy the `ScsiIoCtx*` contract: fill `psense`, `returnStatus`, and return an `eReturnValues` code.

### CSMI Is a Partial Hint, Not a Complete Solution

If a controller's driver reports via CSMI (e.g., identifies itself in `csmi_Get_Driver_Info`), do not assume full CSMI support. Always verify the specific IOCTLs you need by attempting them and checking return codes. Fall back gracefully to vendor-specific IOCTLs when CSMI fails.

### Smartctl Source as Reference

Smartctl's `smartmontools/` source (particularly `dev_areca.cpp`, `megaraid.cpp`, `dev_interface.cpp`) provides reference implementations for several RAID types. Key caveats:
- Smartctl's focus is read-only health monitoring; it does not test write operations, firmware download, or reconfiguration
- Some RAID support has been intentionally disabled in smartctl due to discovered bugs — read the git history for any disabled controller before relying on that implementation
- The IOCTL structures are generally accurate; the higher-level logic may not translate directly to openSeaChest's architecture
