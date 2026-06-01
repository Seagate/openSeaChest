---
description: 'RAID passthrough support in opensea-transport — scan infrastructure, CSMI/Intel RST/CISS implementations, and kernel-verified driver reference for MegaRAID (megaraid_sas/mpi3mr), mpt3sas HBA, Marvell mvsas HBA, Adaptec, Highpoint, 3ware, Areca, VROC, AMD RAIDCORE'
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

### RAID Support Triage Checklist (Use This First)

When someone reports "RAID passthrough is broken" or asks why model/serial/SMART look wrong, run this checklist before debugging code:

1. Is the controller in explicit JBOD mode?
2. Are we talking to a logical volume path or a physical-drive passthrough path?
3. What exact controller family, firmware revision, and driver version are in use?
4. What exact command failed (CDB/ATA op), and what status/sense data came back?
5. Does a basic read-only passthrough command work on the same path?
6. Is this failure reproducible across reboot and with a second command class?

If (1) is no, assume the presented device is logical and all physical-drive identity/health fields are controller-mediated. If (1) is yes and passthrough still fails, assume firmware-level command filtering until proven otherwise.

### Commonly Blocked Command Classes Behind RAID

These blocks are controller/firmware policy decisions and are often undocumented:

| Command class | Typical behavior through passthrough | Notes |
|---------------|--------------------------------------|-------|
| Sanitize / secure erase | Often blocked | High-risk destructive path; many controllers forbid it |
| Security state changes | Often blocked | ATA Security / similar admin flows commonly filtered |
| Firmware download / activate | Often blocked | Controller may only allow vendor tool workflows |
| Vendor-unique diagnostics | Usually blocked | Unknown opcodes frequently rejected or normalized |
| SMART/log retrieval | Sometimes translated, sometimes hidden | Logical view may omit physical data entirely |
| Identify / inquiry variants | Usually allowed with rewritten fields | Model/serial/capacity may be synthesized |

---

## Philosophy and Architecture

RAID passthrough is fundamentally different from direct-attach storage. The goal is **transparent passthrough**: once a RAID-encapsulated physical drive is opened and a `tDevice` is constructed for it, the rest of opensea-transport and opensea-operations must not know or care that the device is behind a RAID controller. This transparency is achieved by replacing `tDevice->issue_io` with a RAID-specific function pointer at open time.

### The Open-Source Challenge

Vendors typically provide internal C libraries for RAID passthrough. These cannot be used in open-source projects. The only approach available for open-source implementations is:

1. Read the kernel driver source for the target RAID controller
2. Identify the IOCTL codes the driver exposes
3. Reconstruct the IOCTL buffer structures from driver source
4. Implement the passthrough using those structures directly

This is painstaking work. **Smartmontools** has completed this for several controllers and its source is a useful reference for understanding which IOCTL structures and constants a given driver uses — but it is GPL-licensed software. **openSeaChest is not GPL. GPL code cannot be copied into this codebase under any circumstances**, not even as a starting point that is later modified. Reading smartmontools source to understand the protocol is fine; copying its struct definitions, constants, or logic is not.

The correct primary source is always the kernel driver itself. Kernel driver header files define the IOCTL structures from first principles; smartmontools simply reconstructed those same structures independently. Reconstruct them independently here too, from the kernel source, not from smartmontools. Where smartmontools and the kernel driver disagree, the kernel driver is authoritative.

Additionally, some RAID support in smartmontools has been deliberately disabled due to discovered bugs and instability — review it critically as a cross-check, not as ground truth.

### CPU Architecture and Kernel Independence

There is a second, equally important reason to use the kernel IOCTL interface directly rather than vendor-provided management libraries: **vendor libraries are almost always x86-only binaries**. ARM is a rapidly growing deployment target for openSeaChest — both 64-bit ARM server (AArch64) and embedded configurations — and vendor SDKs simply do not ship ARM builds.

The kernel driver itself, however, is architecture-neutral. A kernel module written in C compiles and runs on any architecture the kernel supports. If a RAID controller vendor's kernel driver loads on ARM64, the IOCTL interface it exposes is available on ARM64 — no vendor library needed, no vendor approval required. By reconstructing the IOCTL structures from kernel source and calling them directly, openSeaChest can support RAID passthrough on ARM and any other kernel-supported architecture, **providing better platform coverage than the vendor's own tools**.

The same principle extends across kernels. The RAID controller IOCTL structures are driven by the firmware protocol, not by the operating system. When vendors port their drivers to illumos, FreeBSD, or other kernels, comparison of those drivers against the Linux equivalents consistently shows the same structures and the same constants — the differences are in how each kernel's IOCTL dispatch layer is called, not in the payload. The firmware on the controller does not know or care which OS kernel submitted the command.

This means that a correct implementation built from Linux kernel source should translate to illumos, FreeBSD, and other kernels with only the system call wrapper and device open path changing. The goal is: **if the driver works, openSeaChest should be able to talk to it** — regardless of which kernel is running and regardless of which CPU architecture the system uses.

This architectural independence is why the direct-IOCTL approach is preferred even for vendors who do provide user-space management libraries: the libraries are a ceiling, not a floor.

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

**Supported platforms**: Linux and FreeBSD. See the "Windows CISS Gap" subsection below for what is known about Windows support.

### Device Paths

- Linux: `/dev/ciss*` (older `cciss` driver) and `/dev/sg*` (`hpsa`/`smartpqi` drivers expose physical drives as sg nodes)
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

### SmartPQI — CISS Successor with CCISS_PASSTHRU Compatibility

SmartPQI is the modern HPE/Microsemi SmartArray controller driver, introduced to replace the older `cciss`/`hpsa` kernel drivers. Kernel source: `drivers/scsi/smartpqi/smartpqi_init.c` (verified against kernel 7.0).

**Key finding**: The `smartpqi` kernel driver's `pqi_ioctl()` function handles `CCISS_PASSTHRU` **directly**, calling `pqi_passthru_ioctl()` which accepts the **same `IOCTL_Command_struct`** used by the older `hpsa`/`cciss` drivers:

```c
// From smartpqi_init.c pqi_ioctl() — verified from kernel source:
case CCISS_PASSTHRU:
    rc = pqi_passthru_ioctl(ctrl_info, arg);   // arg is IOCTL_Command_struct __user *
    break;
```

`pqi_passthru_ioctl` reads `IOCTL_Command_struct iocommand` from userspace, builds a `pqi_raid_path_request` internally, submits it to the PQI hardware, then translates the PQI error response back to CISS error codes (`pqi_error_info_to_ciss`). The caller sees an unmodified `CCISS_PASSTHRU` IOCTL with `IOCTL_Command_struct` — **the existing CISS implementation is compatible with SmartPQI controllers without any structural changes**.

SmartPQI also handles these CISS management IOCTLs:

| IOCTL | SmartPQI handler | Purpose |
|-------|-----------------|---------|
| `CCISS_PASSTHRU` | `pqi_passthru_ioctl` | Physical drive SCSI passthrough |
| `CCISS_GETPCIINFO` | `pqi_getpciinfo_ioctl` | Returns `cciss_pci_info_struct` |
| `CCISS_GETDRIVVER` | `pqi_getdrivver_ioctl` | Driver version |
| `CCISS_DEREGDISK` / `CCISS_REGNEWDISK` / `CCISS_REGNEWD` | Triggers `pqi_scan_scsi_devices` | Device rescan |

**What differs with SmartPQI**:
- Device nodes: SmartPQI drives appear as `/dev/sg*` nodes (not `/dev/ciss*`). The `smartpqi_filter` scan function is needed to find them.
- Sense data limit: `pqi_passthru_ioctl` caps sense at `sizeof(iocommand.error_info.SenseInfo)` — same CISS limit applies.
- FreeBSD SmartPQI: The SDK (`cissio_freebsd.h`) defines a separate `SMARTPQI_PASS_THRU = _IOWR('M', 2, pqi_ioctl_passthruCmd_struct_t)` IOCTL. FreeBSD SmartPQI is **not** CCISS-compatible on FreeBSD — it uses `SMARTPQI_IOCTL_BASE='M'` rather than `CCISS_IOC_MAGIC='C'`. The existing `smartpqi_filter` FreeBSD function handles device enumeration; the passthrough call path needs the FreeBSD-specific IOCTL.

### Windows CISS Gap

**Current state**: The CISS implementation is guarded by `#if defined(__unix__)`, so Windows support does not exist. The Adaptec SDK ships `winciss.h` which defines CISS command structures and constants for Windows, confirming that Windows CISS passthrough is architecturally possible.

From `winciss.h` (Adaptec SDK 3.01):
- Standard CISS constants: `CMD_SUCCESS=0x0000` through `CMD_TIMEOUT=0x000B`; `XFER_NONE/WRITE/READ`; `TYPE_CMD=0x00/TYPE_MSG=0x01`
- I2O register offsets and `CFGTBL_*` configuration table constants are defined
- The `IOCTL_Command_struct` and `BIG_IOCTL_Command_struct` structures are present

**Windows CTL_CODE not found**: `winciss.h` contains only data structures and protocol constants — **no Windows `CTL_CODE` / `DeviceIoControl` IOCTL definitions were found**. The Linux ioctl numbering (`_IOWR('B', 11, IOCTL_Command_struct)` for `CCISS_PASSTHRU`, per the kernel uapi header `include/uapi/linux/cciss_ioctl.h`) uses a completely different encoding from Windows `CTL_CODE(DeviceType, Function, Method, Access)`. The Linux numeric value is meaningless on Windows.

**Magic number note**: The Linux kernel and the Adaptec SDK `linuxciss.h` both use `CCISS_IOC_MAGIC = 'B'`. The FreeBSD implementation in `cissio_freebsd.h` uses `'C'`. These produce different raw ioctl numbers on their respective platforms and are not interchangeable.

**Finding the Windows IOCTL code**: To identify the Windows `DeviceIoControl` code for CCISS passthrough, one of the following is needed:
- The Windows HP/Compaq CISS driver source (`cpqarray.sys`, `hpcissX.sys`, `cpqci.sys`, or equivalent)
- The HPE Windows Smart Array driver developer kit / DDK
- API monitoring against HP Array Configuration Utility (ACU) or HPE Smart Storage Administrator while it executes a passthrough command

The Windows device path is also unverified — it is likely `\\.\HpCissX` or `\\.\cpqxxx` but requires confirmation from an installed Windows driver. Removing the `__unix__` guard and adding a `DeviceIoControl` path is structurally straightforward; the blocking issue is the unknown Windows IOCTL code.

### CSMI Subset Note

HP's CISS/HPSA driver reports itself via CSMI's driver info interface (identified as `CSMI_DRIVER_HPCISS`). It implements a limited CSMI subset, believed to cover SMP (SAS Management Protocol) passthrough only. This subset is insufficient for physical drive command passthrough. Use the CISS interface, not CSMI, for HP controllers.

---

## Universal RAID Driver Patterns

After analyzing all supported RAID drivers in this project, a consistent set of architectural patterns repeats across every implementation. Understanding these patterns makes it far easier to read a new driver's kernel source and quickly identify what to implement.

### Pattern 1: Dedicated Management Interface Device

Every driver creates a separate access path for management operations — it is never the same as the "normal" SCSI device that the OS uses for I/O:

| Driver Family | Management Path |
|---|---|
| MegaRAID (`megaraid_sas`, `mpi3mr`) | Character device `/dev/megaraid_sas_ioctl_node`, `/dev/mpi3mr<N>ioctl` |
| mpt3sas HBA | Character device `/dev/mpt3ctl`, `/dev/mpt3ctl_host<N>` |
| Adaptec ASR (`aacraid`) | Character device `/dev/aac<N>` |
| HP CISS (`hpsa`) | Character device `/dev/cciss/c<N>d0` (`ioctl`) |
| 3ware Gen 2 (`3w-9xxx`) | Character device `/dev/twe<N>` |
| 3ware Gen 3 (`3w-sas`) | Character device `/dev/twl<N>` |
| Areca (`arcmsr`) | Character device `/dev/arcmsr<N>` |
| PMC MaxRAID (`pmcraid`) | Character device `/dev/pmcsas<N>` |
| Highpoint (`hptiop`) | Character device `/dev/hptiop<N>` or request via any SG node |
| ATTO (`esas2r`) | **No separate char device** — `ioctl()` on any `/dev/sg*` on the esas2r host |

The ATTO exception is significant: `esas2r` registers `ioctl` in `scsi_host_template`, so the management commands flow through the SCSI generic layer on an arbitrary `/dev/sg*` rather than a dedicated node.

### Pattern 2: IOCTL Envelope Structure

Every driver's IOCTL has the same conceptual shape, regardless of how the fields are named:

```
[ signature / magic / driver-name header ]
    → identifies this IOCTL as belonging to this driver (sanity check)
[ command code / sub-command ]
    → selects the operation (get config, list drives, passthrough, etc.)
[ buffer length / data length ]
    → how many bytes of payload follow
[ payload union ]
    → the actual data for this command (input and/or output)
```

Concrete examples:
- **MegaRAID**: `megasas_iocpacket { sense_len, sge_count, frame }` + `megasas_header.cmd_status`
- **CSMI**: every command has `IOCTL_HEADER { IOControlCode, ReturnCode, Length }` + command payload
- **CISS**: `IOCTL_Command_struct { LUN_info, Request, err_info, buf_size, buf }`
- **3ware Gen 1**: `TW_Passthru { request_id, unit, opcode, sgl_offset, sgl_entries, ... }`
- **ATTO**: `atto_express_ioctl { header.signature="Express", header.channel, ... data union }`
- **PMC**: `pmcraid_passthru_ioctl_buffer { signature="PMCRAID", ... }` with `pmcraid_ioarcb`

The **signature/magic bytes** are the primary driver-identification fingerprint used by scanner code.

### Pattern 3: Three-Phase Discovery Sequence

All implementations follow the same three-phase discovery flow before issuing a passthrough command:

```
Phase A — Controller Enumeration
    Scan for management device nodes (or SG nodes for esas2r)
    For each found node: send the driver-specific "get info" IOCTL
    Extract: controller model, firmware version, number of ports

Phase B — Physical Drive Enumeration
    For each controller: send "list physical drives" IOCTL
    Get back: channel/target/lun or flat target_id + lun for each drive
    Note: these drives are hidden from the OS block layer
    Optional: get SAS/WWN address for each drive (persistent identifier)

Phase C — Passthrough Setup
    For a specific drive (identified by its address from Phase B):
    Send SCSI CDB passthrough IOCTL
    Collect: scsi_status + sense data + request/firmware status
```

### Pattern 4: Physical Drive Addressing

Every driver uses one of two addressing models for targeting a specific physical drive in a passthrough command:

**Model A — Channel / Target / LUN** (most common, SAS/SCSI heritage):

```c
u8  channel;    /* HBA port / bus / channel */
u8  target;     /* Drive target ID on that channel */
u32 lun;        /* LUN (usually 0) */
```

Used by: MegaRAID, mpt3sas (via `btag`), Adaptec ASR, HP CISS, 3ware.

**Model B — Flat target_id + LUN** (newer, simpler):

```c
u32 target_id;  /* Controller-wide flat index — no channel concept */
u8  lun[8];     /* Full 8-byte LUN structure */
```

Used by: ATTO (`atto_hba_scsi_pass_thru.target_id`).

### Pattern 5: Controller Identity Query

Every driver has a "get controller info" command that serves two purposes: sanity-check that communication works, and retrieve metadata (model string, firmware version, serial number, product family):

| Driver | Command | Key Output |
|---|---|---|
| MegaRAID | `MFI_CMD_DCMD` + `MR_DCMD_CTRL_GET_INFO` | `ctrl_info` struct with model, firmware, serial |
| mpt3sas | `MPT3IOCINFO` ioctl | adapter_type, ioc_number, pci_ids |
| Adaptec ASR | `FSACTL_MINIPORT_REV_CHECK` → `FSACTL_GET_ADAPTER_FIB` + `RequestAdapterInfo` | adapter model, bus_count, target_count |
| HP CISS | `CISS_IOC_GETDRIVVER` | driver version, CISS interface level |
| 3ware | `TW_IOCTL_GET_UNIT_DESCRIPTOR` | unit info per slot |
| Highpoint | `IOP_REQUEST_TYPE_GET_PROPERTIES` | adapter firmware version, model |
| ATTO | `EXPRESS_IOCTL_HBA` + `ATTO_FUNC_GET_ADAP_INFO` | `tunnel_flags`, `adap_type`, firmware version |
| PMC MaxRAID | `PMCRAID_DRIVER_IOCTL_VERSION` | driver version, `pmcraid_hcam_hdr` |

### Pattern 6: Hidden Physical Drive Listing

Physical drives behind a RAID controller are hidden from the OS block layer. The only way to enumerate them is through the controller's management interface. Every driver provides this:

| Driver | Discovery Command |
|---|---|
| MegaRAID | `MR_DCMD_PD_LIST_QUERY` → `MR_PD_INFO` per drive (enclosure_id, slot, wwn, interface_type) |
| Adaptec ASR | `GetContainerCountResponse` then `QueryDisk` FIB for each slot |
| HP CISS | `CISS_IOC_*` for LUN enumeration of raw drives |
| 3ware | `TW_IOCTL_GET_UNIT_DESCRIPTOR` for each slot (3w-xxxx), `getdevinfo` (3w-9xxx/3w-sas) |
| Highpoint | `IOP_REQUEST_TYPE_GET_DEVICE_INFO` per IOP device index |
| ATTO | `EXPRESS_IOCTL_HBA` + `ATTO_FUNC_GET_DEV_INFO` per `target_id` (0, 1, 2, ... until error) |
| PMC MaxRAID | `PMCRAID_IOCTL_PASSTHROUGH` with `pmcraid_ioarcb` GSCSI resource type |

### Pattern 7: Logical-to-Physical Membership Mapping

For RAID-class controllers (not pure HBAs), every driver can report which physical drives belong to which RAID volume. This allows openSeaChest to decide whether to address a drive as "this drive is a member of RAID volume 0" or as a standalone physical target:

- MegaRAID: `MR_DCMD_LD_GET_LIST` → `MR_LD_INFO.span_0.pd[]` maps LDs to physical drive IDs
- Adaptec ASR: `GetContainerCountResponse` + `QueryDisk FIB` → `DiskDeviceType` distinguishes bare vs array-member
- 3ware: `TW_IOCTL_GET_UNIT_DESCRIPTOR.status` — units with `UNT_STATUS_DEGRADED` or multi-disk units are arrays
- ATTO: `ATTO_FUNC_GET_DEV_INFO.dev_type` — `ATTO_SDI_DT_END_DEVICE` = physical drive; `EXPANDER` = expander

### Pattern 8: SCSI CDB Passthrough — Universal but Constrained

Every driver supports SCSI CDB passthrough. CDB length limits differ:

| Driver | Max CDB | Sense buffer | Notes |
|---|---|---|---|
| Mylex DAC960 V1 (`myrb`) | 12 bytes | 64 bytes | Very old; 12-byte limit is a hard firmware constraint |
| Adaptec ASR (`aacraid`) | 16 bytes | 64 bytes | Standard |
| MegaRAID (`megaraid_sas`) | 16 bytes | 64 bytes | Standard |
| HP CISS (`hpsa`) | 16 bytes | 32 bytes | Shorter sense — handle truncation |
| 3ware Gen 1 (`3w-xxxx`) | 12 bytes | limited | ATA passthrough preferred for this gen |
| 3ware Gen 3 (`3w-sas`) | 16 bytes | 64 bytes | Standard |
| Highpoint (`hptiop`) | 16 bytes | 64 bytes | Standard |
| ATTO (`esas2r`) | **32 bytes** | **252 bytes** | Best-in-class; `atto_hba_scsi_pass_thru.cdb[32]`, `sense_data[0xFC]` |
| PMC MaxRAID (`pmcraid`) | 16 bytes | 64 bytes | Standard |

For ATA commands, use SAT PASS-THROUGH(16) (`0x85`) when only SCSI passthrough is available. 32-byte CDB path (ATTO) can carry extended SAT variants.

### Pattern 9: Three-Layer Status Checking

Every driver passthrough result requires checking three independent status layers:

```
Layer 1 — OS ioctl() return value
    0 = ioctl delivered to driver; non-zero (errno) = OS/driver rejection before any firmware action

Layer 2 — Driver / firmware status
    Driver-specific code in the response struct (e.g., MegaRAID cmd_status, ATTO req_status, CISS CommandStatus)
    0 or driver-specific SUCCESS = firmware accepted and completed the command
    Non-zero = firmware error (device not found, timeout, internal error)

Layer 3 — SCSI status + sense data
    scsi_status byte from response (0x00 = GOOD, 0x02 = CHECK CONDITION, 0x08 = BUSY, etc.)
    If 0x02: read sense_data[] for ASC/ASCQ and additional detail
    If 0x00 with residual: data underrun — account for it
```

Do not short-circuit: a zero ioctl return with a non-zero firmware status is a failure. A zero ioctl return, a zero firmware status, AND `scsi_status = 0x02` is also a failure (CHECK CONDITION) that requires sense parsing.

### Pattern 10: Signature-Based Driver Identification

Scanners identify which driver is present by signature strings embedded in IOCTL structures. Never rely on device node name alone — paths can be shared or aliased:

| Driver | Signature location | Value |
|---|---|---|
| ATTO `esas2r` | `atto_express_ioctl_header.signature` | `"Express"` (8 bytes) |
| PMC MaxRAID | `pmcraid_passthru_ioctl_buffer.signature` | `"PMCRAID"` (7 bytes + `\0`) |
| HP CISS | `CSMI_SAS_DRIVER_INFO.szName` via CSMI | `"hpsa"` / `"HPCISS"` |
| CSMI (all) | `IOCTL_HEADER.Signature` | per-command (e.g., `"CSMISIG_"`) |
| 3ware Gen 1 | `TW_Passthru.request_id` magic pattern | driver version IOCTL has no signature — identify by char device path |
| MegaRAID | `megasas_iocpacket.sge_count` heuristic + char device path | no explicit signature |

### Pattern 11: Windows Portability by Design

On Windows, virtually all RAID miniport drivers use **the same delivery mechanism** regardless of vendor:

1. Open `\\.\SCSI<X>:` where `X` is the SCSI port number (0, 1, 2, …). This is the controller's port device object. Enumerate by trying `\\.\SCSI0:`, `\\.\SCSI1:`, etc., or derive `X` from `IOCTL_STORAGE_GET_DEVICE_NUMBER` on a `\\.\PhysicalDrive<N>` or `\\.\Scsi<X>:` handle.

2. Issue `DeviceIoControl` with `IOCTL_SCSI_MINIPORT` (`0x0004D008`) as the control code.

3. The input/output buffer starts with `SRB_IO_CONTROL`, followed immediately by the driver-specific payload:

```c
typedef struct _SRB_IO_CONTROL {
    ULONG HeaderLength;   /* sizeof(SRB_IO_CONTROL) */
    UCHAR Signature[8];   /* Driver identifier — same as the Linux IOCTL signature field */
    ULONG Timeout;        /* Seconds */
    ULONG ControlCode;    /* Driver-specific sub-command — the Linux ioctl() cmd value */
    ULONG ReturnCode;     /* Driver return status (out) */
    ULONG Length;         /* Byte length of payload following this header */
} SRB_IO_CONTROL;

/* Layout: [ SRB_IO_CONTROL ][ driver payload ] — same buffer for in and out */
DeviceIoControl(hScsiPort, IOCTL_SCSI_MINIPORT,
    pBuf, sizeof(SRB_IO_CONTROL) + payloadSize,
    pBuf, sizeof(SRB_IO_CONTROL) + payloadSize,
    &dwReturned, NULL);
```

The `Signature` field maps directly to each driver's IOCTL signature string. The `ControlCode` field carries the driver-specific IOCTL sub-code — the same value used as the Linux `ioctl()` `cmd` argument:

| Driver | `Signature[8]` | `ControlCode` examples |
|---|---|---|
| CSMI (all vendors) | `"CSMISIG_"` (confirmed) | per-command CSMI code |
| ATTO `esas2r` | `"Express"` (confirmed) | `EXPRESS_IOCTL_HBA = 0x450C` |
| MegaRAID | **unverified** — likely `"MR_IOCTL"` or `"MEGARAID"`; confirm via `strings megasas.sys` or smartmontools Windows source | MFI DCMD / passthrough opcode |
| Adaptec ASR | **unverified** — likely `"AACRAID"` or `"AACAPI"`; confirm via `strings adpu320.sys` / `archdflop.sys` or smartmontools | AAC FIB command codes |
| 3ware | **unverified** — likely `"3ware   "` (space-padded to 8); confirm via `strings 3wareDrv.sys` | generation-specific passthrough code |
| Highpoint | **unverified** — confirm via `strings hptiop.sys` or Highpoint RAID management source | IOP request type |

The struct layouts and field offsets are **identical** between Linux and Windows — only the device open path (`\\.\Scsi<X>:` vs `/dev/twe<N>` etc.) and the `DeviceIoControl`/`ioctl` wrapper differ. This means a well-factored implementation can share almost all struct definitions and logic between platforms.

CSMI was designed for Windows first and later adopted on Linux, which is why the Windows path for CSMI is already implemented in openSeaChest. All other drivers follow the exact same `IOCTL_SCSI_MINIPORT` delivery mechanism.

### Pattern 12: CSMI Reuse Spectrum

Different drivers have different relationships to CSMI:

| Relationship | Drivers | Implication |
|---|---|---|
| **Full CSMI implementation** | ATTO (`esas2r`) — has `EXPRESS_CSMI = 0x450B`, Intel RST | Full `CSMI_SAS_*` command set available; openSeaChest CSMI may already work on Windows |
| **CSMI-adjacent / partial** | AMD rcraid, Intel VROC | Partial CSMI; detection only; not suitable for passthrough |
| **CSMI-identified only** | HP HPSA/CISS | Reports via CSMI driver info but passthrough is CISS-only |
| **No CSMI whatsoever** | 3ware, Mylex DAC960, PMC MaxRAID, Highpoint, Areca (own protocol), IBM IPR | Must use driver-native IOCTL; CSMI probe will fail or be ignored |

---

## Future RAID Implementations

These RAID types have placeholder bits in `raidTypeHint` (or will need them added) but have no implementation yet. The approach for all of them is the same: read kernel driver source, identify IOCTL structures, and implement accordingly.

### SCSI-Only vs. SCSI + ATA Passthrough

When implementing passthrough for any RAID controller, determine what passthrough modes the driver actually exposes:

- **SCSI passthrough only** — the most common case. The RAID driver accepts SCSI CDBs directed at physical drives. ATA commands must be tunneled using SAT ATA PASS-THROUGH CDBs (A1h / 85h). See the ATA instruction file for full SAT CDB byte layouts and field mapping, plus the "Known SAT Translator Bugs and Quirks" section for real-world translator failures.
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

### Next-Phase Implementation Order

For the next development phase, implement in this order:

1. **MegaRAID first** — `megaraid_sas` driver; see the "MegaRAID Driver Family" section below for complete IOCTL reference.
2. **Adaptec ASR second** — `aacraid` driver (Adaptec/PMC/Microsemi/Microchip ASR-series controllers). **Not Areca.** See the "Adaptec ASR Series" section below for the kernel-verified IOCTL reference.
3. **Areca ARC third** — `arcmsr` driver (Areca ARC-series controllers). A completely separate vendor from Adaptec. See the "Areca RAID" section below.

---

## MegaRAID Driver Family (Broadcom/Avago/LSI)

The MegaRAID product line spans three kernel driver generations. Each targets a distinct product era and exposes a different IOCTL interface.

### Generation 1 — Legacy mailbox (`megaraid` driver) — Obsolete

**Kernel source**: `drivers/scsi/megaraid/megaraid_ioctl.h`, `megaraid.h`

Very old hardware: PERC3, PERC4, pre-2004 cards. Uses a mailbox-based command model.

- **IOCTL**: `MEGAIOCCMD = _IOWR('m', 0, mimd_t)` — takes a `mimd_t` structure
- **Extended IOCTL**: `uioc_t` with `signature = "$$_EXTD_IOCTL_$$"`, `opcode` field selects subcommand
- **SCSI passthrough**: `mega_passthru` struct — `channel` (u8), `target` (u8), `cdb[10]`, `reqsensearea[0x20]`, `dataxferaddr` / `dataxferlen`
- **Extended CDB passthrough**: `mega_ext_passthru` for CDB > 10 bytes
- **Status**: Do not implement. Hardware is entirely end-of-life. No production use.

---

### Generation 2 — MFI-based (`megaraid_sas` driver) — Primary MegaRAID Target

**Kernel source**: `drivers/scsi/megaraid/megaraid_sas.h`

Covers all SCSI-era and modern MegaRAID: PERC5 through PERC11, all SAS-generation cards including Aero-series. This is the driver to implement.

#### Device Access

The driver registers a character device; the `host_no` field in `megasas_iocpacket` selects which controller to target. An application opens the character device and issues `MEGASAS_IOC_FIRMWARE`.

#### Top-level IOCTLs

| Constant | Value | Purpose |
|----------|-------|---------|
| `MEGASAS_IOC_FIRMWARE` | `_IOWR('M', 1, struct megasas_iocpacket)` | Submit a firmware command frame |
| `MEGASAS_IOC_GET_AEN` | `_IOW('M', 3, struct megasas_aen)` | Register for async event notification |

#### `megasas_iocpacket` — The Top-Level Wrapper

```c
#define MAX_IOCTL_SGE  16

struct megasas_iocpacket {
    u16 host_no;          // controller index (0-based)
    u16 __pad1;
    u32 sgl_off;          // byte offset in this struct to the SGL array
    u32 sge_count;        // number of entries in sgl[]
    u32 sense_off;        // byte offset in this struct to the sense buffer
    u32 sense_len;        // length of sense buffer
    union {
        u8                  raw[128];  // 2×64-byte MFI frame
        struct megasas_header hdr;    // frame header
    } frame;
    struct iovec sgl[MAX_IOCTL_SGE]; // scatter-gather list from user space
} __packed;
```

#### MFI Command Opcodes (`enum MFI_CMD_OP`)

| Opcode | Value | Purpose |
|--------|-------|---------|
| `MFI_CMD_PD_SCSI_IO` | `0x4` | **Physical drive SCSI passthrough** — the main path |
| `MFI_CMD_DCMD` | `0x5` | Driver Control Command — firmware management queries |
| `MFI_CMD_SMP` | `0x7` | SMP passthrough |
| `MFI_CMD_STP` | `0x8` | STP (SATA) passthrough — ATA register FIS directly |
| `MFI_CMD_NVME` | `0x9` | NVMe passthrough (requires `support_nvme_passthru` capability flag) |

Set `cmd` in `megasas_pthru_frame.cmd` or `megasas_dcmd_frame.cmd` to one of these values.

**Current and planned implementation paths**:

- `MFI_CMD_PD_SCSI_IO` (0x4) — used for SAS physical drives (SSP passthrough); this is the primary SCSI path.
- `MFI_CMD_STP` (0x8) — used for SATA physical drives; carries ATA register FIS directly without SAT translation. This is the current internal ATA passthrough path in storelib. Preferred over wrapping ATA commands in a SAT CDB because it avoids the translation layer and gives direct access to RTFRs.
- `MFI_CMD_NVME` (0x9) — not yet implemented. Requires checking `adapter_operations4.support_nvme_passthru` first (Aero-series only). Future work.
- `MFI_CMD_SMP` (0x7) — SAS Management Protocol for communicating with SAS expanders. Not currently implemented. Future work: SMP is a cross-interface capability (MFI, mpt3sas, CSMI, direct SAS) and should be considered for integration across all passthrough paths where the transport supports it. SMP allows discovering topology, reading expander route tables, and configuring expander PHYs.

#### `megasas_pthru_frame` — SCSI Passthrough to Physical Drive

```c
struct megasas_pthru_frame {
    u8    cmd;             // 0x04 = MFI_CMD_PD_SCSI_IO
    u8    sense_len;       // max sense length (e.g., 64)
    u8    cmd_status;      // filled by FW: MFI_STAT_OK (0x00) on success
    u8    scsi_status;     // filled by FW: raw SCSI status byte
    u8    target_id;       // physical drive target — from MR_PD_ADDRESS.deviceId
    u8    lun;             // usually 0
    u8    cdb_len;         // 6, 10, 12, or 16
    u8    sge_count;
    __le32 context;
    __le32 pad_0;
    __le16 flags;          // MFI_FRAME_DIR_READ/WRITE/NONE
    __le16 timeout;        // seconds; 0 = driver default
    __le32 data_xfer_len;
    __le32 sense_buf_phys_addr_lo;
    __le32 sense_buf_phys_addr_hi;
    u8    cdb[16];         // CDB bytes — up to 16 bytes
    union megasas_sgl sgl; // scatter-gather list
} __packed;
```

**Key flags** (set in `megasas_pthru_frame.flags`):
- `MFI_FRAME_DIR_NONE = 0x0000` — no data transfer
- `MFI_FRAME_DIR_WRITE = 0x0008`
- `MFI_FRAME_DIR_READ = 0x0010`

#### `megasas_dcmd_frame` — Firmware Control Commands

```c
struct megasas_dcmd_frame {
    u8    cmd;        // 0x05 = MFI_CMD_DCMD
    u8    reserved_0;
    u8    cmd_status; // MFI_STAT_* on return
    u8    reserved_1[4];
    u8    sge_count;
    __le32 context;
    __le32 pad_0;
    __le16 flags;
    __le16 timeout;
    __le32 data_xfer_len;
    __le32 opcode;    // MR_DCMD_* constant
    union {
        u8    b[12]; // mailbox bytes (DCMD-specific parameters)
        __le16 s[6];
        __le32 w[3];
    } mbox;
    union megasas_sgl sgl;
} __packed;
```

#### Key DCMD Opcodes for Drive Discovery

| Constant | Value | Returns | Purpose |
|----------|-------|---------|----------|
| `MR_DCMD_CTRL_GET_INFO` | `0x01010000` | `megasas_ctrl_info` | Controller capabilities, PD/LD counts, firmware version |
| `MR_DCMD_PD_LIST_QUERY` | `0x02010100` | `MR_PD_LIST` | List of all physical drives with addresses |
| `MR_DCMD_PD_GET_INFO` | `0x02020000` | `MR_PD_INFO` | Full info for one physical drive (inquiry, state, interface type) |
| `MR_DCMD_LD_GET_LIST` | `0x03010000` | `MR_LD_LIST` | List of all logical drives |

#### Physical Drive Discovery: `MR_PD_LIST` → `MR_PD_ADDRESS`

Issue `MR_DCMD_PD_LIST_QUERY` DCMD. The response buffer is `MR_PD_LIST`:

```c
struct MR_PD_ADDRESS {
    __le16 deviceId;          // use as target_id in pthru_frame
    u16    enclDeviceId;
    union {
        struct { u8 enclIndex; u8 slotNumber; } mrPdAddress;
        struct { u8 enclPosition; u8 enclConnectorIndex; } mrEnclAddress;
    };
    u8     scsiDevType;       // SAS_PD=2, SATA_PD=3, NVME_PD=5 (from MR_PD_TYPE)
    union  { u8 connectedPortBitmap; u8 connectedPortNumbers; };
    u64    sasAddr[2];        // SAS address(es)
} __packed;

struct MR_PD_LIST {
    __le32 size;
    __le32 count;
    struct MR_PD_ADDRESS addr[1]; // count entries follow
} __packed;
```

`MR_PD_TYPE` enum: `UNKNOWN_DRIVE=0`, `PARALLEL_SCSI=1`, `SAS_PD=2`, `SATA_PD=3`, `FC_PD=4`, `NVME_PD=5`.

After discovery, iterate `addr[]`, use `deviceId` as `target_id` in each `megasas_pthru_frame` to address that physical drive.

For detailed info on a specific drive, issue `MR_DCMD_PD_GET_INFO` with `mbox.s[0] = deviceId`. Returns `MR_PD_INFO` (512 bytes) containing inquiry data, VPD page 83, power state, interface type, SAS address, and enclosure/slot.

#### MFI Status Codes (`enum MFI_STAT`)

| Code | Value | Meaning |
|------|-------|---------|
| `MFI_STAT_OK` | `0x00` | Command succeeded |
| `MFI_STAT_INVALID_CMD` | `0x01` | Bad command opcode |
| `MFI_STAT_DEVICE_NOT_FOUND` | `0x0c` | Physical drive not present |
| `MFI_STAT_SCSI_DONE_WITH_ERROR` | `0x2d` | SCSI error — check `scsi_status` and sense data |
| `MFI_STAT_SCSI_IO_FAILED` | `0x2e` | I/O-level failure (no sense data) |
| `MFI_STAT_SCSI_RESERVATION_CONFLICT` | `0x2f` | SCSI reservation conflict |
| `MFI_STAT_INVALID_STATUS` | `0xFF` | Firmware internal error |

#### Adapter Series (for context)

From `enum MR_ADAPTER_TYPE` in `megaraid_sas.h`:

| Enum | Series | Typical Products |
|------|--------|------------------|
| `MFI_SERIES` | Gen 1 SAS | PERC5, PERC6, SAS1064E |
| `THUNDERBOLT_SERIES` | Gen 2 | PERC7, SAS2108 (Fusion) |
| `INVADER_SERIES` | Gen 3 | PERC8/H710, SAS2308 |
| `VENTURA_SERIES` | Gen 4 | PERC9/10, SAS3416 |
| `AERO_SERIES` | Gen 5 | PERC11, SAS3916, 9540-8i |

#### NVMe Passthrough Capability

Check `megasas_ctrl_info.adapter_operations4.support_nvme_passthru` before attempting `MFI_CMD_NVME`. This bit was added for Aero-series controllers. Use `MR_DCMD_CTRL_GET_INFO` to fetch `megasas_ctrl_info` first.

#### Passthrough Limitations

- Frame size is 64 bytes (`MEGAMFI_FRAME_SIZE`). The `megasas_pthru_frame` fits exactly in one frame.
- Maximum 16 CDB bytes (`cdb[16]`).
- Sense buffer is caller-allocated; physical address must be supplied in `sense_buf_phys_addr_lo/hi`. In userspace, the driver handles the translation for IOCTL callers.
- `sge_count` maximum depends on whether the 64-byte or extended frame format is in use — typically 16 SGEs via the IOCTL path.

---

### Generation 3 — MPI3 (`mpi3mr` driver) — Newest

**Kernel source**: `drivers/scsi/mpi3mr/mpi3mr.h`, `mpi3mr_app.c`, `mpi/mpi30_sas.h`
**UAPI interface**: `<uapi/scsi/scsi_bsg_mpi3mr.h>`

Covers 9600-series controllers and later (PERC12+, Aero-Plus). This is fundamentally different from the MFI family.

- **No classic ioctl**. The `mpi3mr` driver exposes a BSG (Block SCSI Generic) endpoint.
- **Device node**: `/dev/bsg/mpi3mr<N>` where N is the controller index.
- **User-space interface**: Send BSG commands via the BSG protocol on the `bsg` device. The UAPI structures are in `<uapi/scsi/scsi_bsg_mpi3mr.h>`.
- **MPI3 SMP passthrough** frame structure (`mpi30_sas.h`):
  ```c
  struct mpi3_smp_passthrough_request {
      __le16 host_tag;
      u8     ioc_use_only02;
      u8     function;           // function code
      __le16 ioc_use_only04;
      u8     ioc_use_only06;
      u8     msg_flags;
      __le16 change_count;
      u8     reserved0a;
      u8     io_unit_port;
      __le32 reserved0c[3];
      __le64 sas_address;
      struct mpi3_sge_common request_sge;
      struct mpi3_sge_common response_sge;
  };
  ```
- **SAS device info flags** (from `mpi30_sas.h`, different bit positions than MPI2):
  - `MPI3_SAS_DEVICE_INFO_SSP_TARGET = 0x00000100`
  - `MPI3_SAS_DEVICE_INFO_STP_SATA_TARGET = 0x00000080`
  - `MPI3_SAS_DEVICE_INFO_SSP_INITIATOR = 0x00000020`
- **Implementation approach**: Construct BSG requests targeting the `/dev/bsg/mpi3mr<N>` node. Refer to `scsi_bsg_mpi3mr.h` for the BSG message subtypes (admin passthrough, IO unit control, etc.).
- **Status**: Not yet implemented in openSeaChest. Requires BSG infrastructure that may need adding to the scan layer.

---

## MPT Fusion HBA / IR-RAID Passthrough (`mpt3sas` / `mpt2sas` drivers)

**Kernel source**: `drivers/scsi/mpt3sas/mpt3sas_ctl.h`, `mpi/mpi2_sas.h`

The `mpt3sas` family covers Fusion MPT SAS controllers flashed with one of two distinct firmware personalities that fundamentally change how physical drives are accessed:

- **IT (Initiator Target) mode** — the controller acts as a plain HBA. Physical drives appear directly as `/dev/sdX` / `/dev/sgX`. Standard `SG_IO` already reaches them without any special handling. The custom IOCTLs are primarily useful for controller enumeration and handle-to-device mapping. Examples: 9200-8i (IT), 9300-8i, 9400-16i, 9500-16i.
- **IR (Integrated RAID) mode** — the controller runs onboard RAID firmware. Only the RAID logical volume(s) appear as `/dev/sgX`; **physical drives are completely hidden from the OS SCSI layer**. To reach physical drives you must use `MPT3COMMAND` with an MPI2 SCSI IO Request frame targeting each physical device's firmware handle directly. This mode is less common in the typical customer environment but does come up and warrants support.

#### Device Nodes

| Driver | Device node |
|--------|-------------|
| `mpt3sas` | `/dev/mpt3ctl` |
| `mpt2sas` (older, SAS2) | `/dev/mpt2ctl` |

#### IOCTLs

| Constant | Value | Purpose |
|----------|-------|---------|
| `MPT3COMMAND` | `_IOWR('L', 20, struct mpt3_ioctl_command)` | Submit MPI2 firmware message frame |
| `MPT3IOCINFO` | `_IOWR('L', 17, struct mpt3_ioctl_iocinfo)` | Query IOC identity |
| `MPT3BTDHMAPPING` | `_IOWR('L', 31, struct mpt3_ioctl_btdh_mapping)` | Translate bus/target ↔ device handle |
| `MPT3HARDRESET` | `_IOWR('L', 24, struct mpt3_ioctl_diag_reset)` | Hard reset |
| `MPT3DIAGREGISTER` | `_IOWR('L', 26, struct mpt3_diag_register)` | Register trace buffer |

Default timeout: `MPT3_IOCTL_DEFAULT_TIMEOUT = 10` seconds.

#### `mpt3_ioctl_command` — Generic MPI2 Frame Submission

```c
struct mpt3_ioctl_header {
    uint32_t ioc_number;    // controller index (0-based)
    uint32_t port_number;
    uint32_t max_data_size; // max bytes to transfer on read
};

struct mpt3_ioctl_command {
    struct mpt3_ioctl_header hdr;
    uint32_t  timeout;              // seconds; 0 = driver default
    void __user *reply_frame_buf_ptr;  // caller allocates reply buffer
    void __user *data_in_buf_ptr;      // destination for read data
    void __user *data_out_buf_ptr;     // source for write data
    void __user *sense_data_ptr;       // sense data destination
    uint32_t  max_reply_bytes;
    uint32_t  data_in_size;
    uint32_t  data_out_size;
    uint32_t  max_sense_bytes;
    uint32_t  data_sge_offset;   // word offset in mf[] to first SGL element
    uint8_t   mf[1];             // variable-length MPI2 request frame follows
};
```

The `mf[]` flexible array holds the actual MPI2 request frame (e.g., `MPI2_SCSI_IO_REQUEST`, `MPI2_SAS_IOUNIT_CONTROL_REQUEST`). Set `data_sge_offset` to the word offset within `mf[]` where the driver should embed the scatter-gather list it constructs from `data_in_buf_ptr` / `data_out_buf_ptr`.

#### Handle ↔ Bus/Target Mapping: `mpt3_ioctl_btdh_mapping`

```c
struct mpt3_ioctl_btdh_mapping {
    struct mpt3_ioctl_header hdr;
    uint32_t id;      // SCSI target ID
    uint32_t bus;     // SCSI bus
    uint16_t handle;  // firmware device handle
    uint16_t rsvd;
};
```

To get bus/target from handle: set `handle` to the known handle, set `id` and `bus` to `0xFFFF`, issue `MPT3BTDHMAPPING`. The driver fills in `id` and `bus`.

To get handle from bus/target: set `handle = 0xFFFF`, set `id`/`bus` to known values.

#### MPI2 SAS Device Info Flags (from `mpi2_sas.h`)

| Constant | Value | Meaning |
|----------|-------|---------|
| `MPI2_SAS_DEVICE_INFO_SSP_TARGET` | `0x00000400` | SAS SSP target (standard SCSI SAS) |
| `MPI2_SAS_DEVICE_INFO_STP_TARGET` | `0x00000200` | STP target (SATA behind SAS expander) |
| `MPI2_SAS_DEVICE_INFO_SATA_DEVICE` | `0x00000080` | Direct-attached SATA |
| `MPI2_SAS_DEVICE_INFO_END_DEVICE` | `0x00000001` | End device (not expander) |

#### MPI2 SAS Status Codes (from `mpi2_sas.h`)

| Constant | Value | Meaning |
|----------|-------|---------|
| `MPI2_SASSTATUS_SUCCESS` | `0x00` | Success |
| `MPI2_SASSTATUS_UNKNOWN_ERROR` | `0x01` | Unknown error |
| `MPI2_SASSTATUS_UTC_STP_RESOURCES_BUSY` | `0x08` | STP resources busy |
| `MPI2_SASSTATUS_DATA_OFFSET_ERROR` | `0x11` | Data offset error |
| `MPI2_SASSTATUS_INITIATOR_RESPONSE_TIMEOUT` | `0x14` | Response timeout |

#### Implementation Notes

**IT mode**: For openSeaChest with IT-mode controllers, the mpt3sas IOCTLs are primarily useful for **controller-level enumeration** and **handle-to-device mapping**. Physical drives appear as `/dev/sgX` and are reached via standard `SG_IO`. Exception: SATA drives behind a SAS expander may not receive an `/dev/sgX` entry; for those, `MPT3COMMAND` with an MPI2 STP passthrough frame targeting the physical device handle may be the only route.

**IR mode (Integrated RAID)**: When the controller is in IR mode, physical drives are hidden — only RAID logical volumes appear in the OS SCSI layer. To reach physical drives:

1. **Enumerate physical drive handles** — send a Config Page request via `MPT3COMMAND` to discover physical device handles. Use an `MPI2_CONFIG_REQUEST` frame with `ExtPageType = MPI2_CONFIG_EXTPAGETYPE_SAS_DEVICE` to iterate the SAS device table. Refer to `mpi2_cnfg.h` for the full structure (not yet read from kernel source; investigation needed before implementation).
2. **Issue SCSI IO to a physical handle** — place an `MPI2_SCSI_IO_REQUEST` frame in `mf[]`. Set `DevHandle` to the physical drive's device handle obtained in step 1. Set `data_sge_offset` to the word offset of the first SGE within the frame. Refer to `mpi2_scsi.h` for the full request and reply structures (not yet read; investigation needed).
3. **Check the MPI2 reply** — the driver fills `reply_frame_buf_ptr` with an `MPI2_SCSI_IO_REPLY`. Check `IOCStatus == MPI2_IOCSTATUS_SUCCESS (0x0000)` and `SCSIStatus == 0` for a clean completion. Non-zero `SCSIStatus` means SCSI error; check sense data in `sense_data_ptr`.

**SMP passthrough in IR mode**: The same `MPT3COMMAND` path can carry an `MPI2_SMP_PASSTHROUGH_REQUEST` frame to communicate with SAS expanders. This enables topology discovery and expander configuration. Future work — relevant across IT and IR modes.

**Priority**: Implement IT-mode enumeration first (low risk, drives already visible). IR-mode physical drive passthrough is the scenario that requires `MPT3COMMAND` for actual drive access; implement after MegaRAID support is stable.

---

## Marvell SAS HBA (`mvsas` driver) — Not a RAID Controller

**Kernel source**: `drivers/scsi/mvsas/mv_sas.h`, `mv_sas.c`

The `mvsas` driver covers Marvell 88SE64xx and 88SE94xx SAS/SATA host bus adapters. This is a libsas-based HBA driver, **not a RAID controller**.

#### Key Facts

- Physical drives appear directly as `/dev/sdX` and `/dev/sgX` via the standard SCSI mid-layer. No custom IOCTL is required for passthrough.
- Standard `SG_IO` already reaches physical drives without any special handling.
- **The mvsas driver is not an obstacle** — it is transparent. For openSeaChest purposes, drives behind this HBA are no different from drives behind any other HBA.
- The driver uses libsas internals (`sas_task`, `ata_task`, `ssp_task`) to dispatch commands. From user space there is no visibility into these structures; everything goes through the SCSI and ATA mid-layers.
- **Do not add a `raidTypeHint` bit** for this driver — it is not RAID and requires no special scan path.
- If a future scenario requires SMP passthrough to an expander connected to an mvsas HBA, use the standard libsas SMP passthrough interface (`/dev/smpX` nodes on Linux), not a custom driver IOCTL.

#### Marvell Products Covered

| Chip | Typical Use |
|------|-------------|
| 88SE6440 | PCIe 1.0 4-port SAS/SATA |
| 88SE6445 | PCIe 1.0 8-port SAS/SATA |
| 88SE9480 | PCIe 2.0 8-port SAS/SATA |
| 88SE9485 | PCIe 2.0 8-port SAS/SATA (6 Gbps) |

#### Action for openSeaChest

No action needed. Drives behind `mvsas` are already enumerated and accessible via the existing Linux SCSI/ATA passthrough infrastructure. Confirm this by verifying that `/dev/sg<N>` nodes appear for drives attached to this HBA and that standard `SG_IO` succeeds.

---

## Summary: Linux RAID/HBA Driver Quick Reference

| Driver | Type | IOCTL / Interface | Device Node | Passthrough Target |
|--------|------|-------------------|-------------|--------------------|
| `megaraid` | Legacy RAID | `MEGAIOCCMD` `_IOWR('m',0,mimd_t)` | — | `mega_passthru` to phys. drive |
| `megaraid_sas` | RAID (MFI) | `MEGASAS_IOC_FIRMWARE` `_IOWR('M',1,megasas_iocpacket)` | char dev | `megasas_pthru_frame` (cmd=0x4) |
| `mpi3mr` | RAID (MPI3) | BSG protocol | `/dev/bsg/mpi3mrN` | BSG SMP/IO passthrough |
| `mpt3sas` (IT mode) | HBA | `SG_IO` direct | `/dev/sgX` | No custom IOCTL needed |
| `mpt3sas` (IR mode) | RAID | `MPT3COMMAND` `_IOWR('L',20,mpt3_ioctl_command)` | `/dev/mpt3ctl` | `MPI2_SCSI_IO_REQUEST` in `mf[]` targeting physical `DevHandle` |
| `mpt2sas` (IT mode) | HBA | `SG_IO` direct | `/dev/sgX` | No custom IOCTL needed |
| `mpt2sas` (IR mode) | RAID | Same as mpt3sas IR | `/dev/mpt2ctl` | Same |
| `mvsas` | HBA (libsas) | Standard `SG_IO` | `/dev/sgX` | No custom IOCTL needed |

Why this order:

- MegaRAID has broad enterprise prevalence, a mature Linux driver (`megaraid_sas`), and established reference behavior in smartctl.
- Adaptec ARC/ASR can follow once the generic RAID passthrough scaffolding is proven with MegaRAID.
- Current Microchip SmartRAID controllers continue the HPE-era lineage and typically follow CISS-style management patterns conceptually, even when transport details differ by generation/driver. Treat ARC/ASR/SmartRAID as one family for staged implementation planning.

### 3ware (AMCC/LSI) — Three Generations

3ware is an older SAS/SATA RAID controller brand (originally AMCC, later LSI). **Old but doable — three kernel drivers cover three distinct hardware generations.** A `raidTypeHint` bit does not yet exist and must be added.

Smartctl has good 3ware support in all three generations; its source (`os_linux.cpp`, `dev_3ware.cpp`) is the best reference for the character device paths and exact ioctl call structure.

#### Generation 1 — `3w-xxxx` (ATA RAID: 6xxx/7xxx/8xxx series)

**Kernel source**: `drivers/scsi/3w-xxxx.h`

- **Character device**: `/dev/tw0` (Linux), `/dev/twe0` (older naming)
- **IOCTL magic**: `TW_IOCTL = 0x80` — embedded as the ioctl subcommand code
- **Passthrough structure**: `TW_Passthru` — carries raw ATA task-file registers, not a SCSI CDB. This generation was pure ATA RAID (no SCSI device support).

```c
/* Command header for ATA pass-thru (3w-xxxx) */
typedef struct TAG_TW_Passthru {
    unsigned char opcode__sgloffset;
    unsigned char size;
    unsigned char request_id;
    unsigned char aport__hostid;
    unsigned char status;
    unsigned char flags;
    unsigned short param;
    unsigned short features;       /* ATA Features register */
    unsigned short sector_count;   /* ATA Sector Count TFR */
    unsigned short sector_num;     /* ATA Sector Number TFR */
    unsigned short cylinder_lo;    /* ATA Cylinder Low TFR */
    unsigned short cylinder_hi;    /* ATA Cylinder High TFR */
    unsigned char drive_head;      /* ATA Device/Head TFR */
    unsigned char command;         /* ATA Command TFR */
    TW_SG_Entry sg_list[TW_ATA_PASS_SGL_MAX];
    unsigned char padding[12];
} TW_Passthru;
```

- **Also**: `TW_New_Ioctl` provides an updated format with a `TW_Command firmware_command` field and `char data_buffer[]` flexible array for data.
- **Note**: This is ATA-register passthrough only. No SCSI CDB support. Relevant for ATA-specific operations (SMART, Identify, SCT) but not for SCSI/SAS drives.

#### Generation 2 — `3w-9xxx` (3ware 9000 series: SATA RAID)

**Kernel source**: `drivers/scsi/3w-9xxx.h`

- **Character device**: `/dev/twa0`
- **Passthrough control code**: `TW_IOCTL_FIRMWARE_PASS_THROUGH = 0x108`
- **IOCTL structure**: `TW_Ioctl_Buf_Apache` (identical layout to 3w-sas below; see Generation 3 for the complete struct — the 9xxx and sas versions share this layout).
- **CDB limit**: 16 bytes via `TW_Command_Apache.cdb[16]`
- **Data transfer**: variable-length `data_buffer[]` appended after the fixed structure

#### Generation 3 — `3w-sas` (3ware 9750 SAS/SATA-RAID)

**Kernel source**: `drivers/scsi/3w-sas.h`

- **Character device**: `/dev/twl0`
- **Passthrough control code**: `TW_IOCTL_FIRMWARE_PASS_THROUGH = 0x108` (comment in kernel source: "Used by smartmontools")

```c
/* 3w-sas / 3w-9xxx: IOCTL driver command header */
typedef struct TAG_TW_Ioctl_Driver_Command {
    unsigned int control_code;   /* TW_IOCTL_FIRMWARE_PASS_THROUGH (0x108) for passthrough */
    unsigned int status;
    unsigned int unique_id;
    unsigned int sequence_id;
    unsigned int os_specific;
    unsigned int buffer_length;  /* byte length of data_buffer[] that follows */
} TW_Ioctl_Driver_Command;

/* Top-level IOCTL buffer */
typedef struct TAG_TW_Ioctl_Apache {
    TW_Ioctl_Driver_Command driver_command;
    char padding[488];
    TW_Command_Full firmware_command;  /* header + old/new command union */
    char data_buffer[];                /* data follows immediately after */
} TW_Ioctl_Buf_Apache;

/* New command packet — used for SCSI passthrough */
typedef struct TAG_TW_Command_Apache {
    unsigned char opcode__reserved;
    unsigned char unit;
    unsigned short request_id__lunl;
    unsigned char status;
    unsigned char sgl_offset;
    unsigned short sgl_entries__lunh;
    unsigned char cdb[16];             /* SCSI CDB, up to 16 bytes */
    TW_SG_Entry_ISO sg_list[TW_LIBERATOR_MAX_SGL_LENGTH];
    unsigned char padding[TW_PADDING_LENGTH_LIBERATOR];
} TW_Command_Apache;
```

- **SENSE data location**: returned in `TW_Command_Apache_Header.sense_data[TW_SENSE_DATA_LENGTH]` (18 bytes)
- **Error decode**: `TW_Command_Apache_Header.status_block.error` plus `severity__reserved` field
- **Max CDB**: 16 bytes
- **Windows portability**: `TW_IOCTL_FIRMWARE_PASS_THROUGH = 0x108` is a plain integer constant (not a `_IOWR`-encoded number), analogous to aacraid's `FSACTL_*` approach. A Windows 3ware driver using the same constant would be directly portable with only the device-open path changed.

#### Implementation Notes

- All three generations use a **dedicated character device** (not the SCSI generic `/dev/sg` path). Open `/dev/twl0` (3w-sas), `/dev/twa0` (3w-9xxx), or `/dev/tw0` (3w-xxxx) and issue `ioctl()`.
- **Detection**: check for the character device nodes at startup; the driver generation is implied by which node exists.
- **Generation 1 only supports ATA passthrough** — no SCSI CDB. If a drive needs SCSI, this generation is limited to SAT-tunneled ATA or not supported.
- Generations 2 and 3 both expose `cdb[16]` SCSI passthrough. Use SAT ATA PASS-THROUGH for ATA commands when needed.
- `TW_MAX_CDB_LEN = 16` is defined in `3w-sas.h`; do not attempt to send longer CDBs.

### MegaRAID (LSI / Avago / Broadcom)

`megaRAID` bit already exists in `raidTypeHint`. MegaRAID is the enterprise SAS/SATA/NVMe RAID line now owned by Broadcom.

- Internal transport model is message-frame based (MFI, historically tied to Message Passing Interface-style mailbox/message semantics). It is a good design but hard to reason about unless you are already familiar with request/response frame queues and firmware mailboxes.
- Linux driver: `megaraid_sas`
- FreeBSD driver: `mrsas`
- Device nodes: `/dev/megaraid_sas_ioctl_node` (Linux dedicated ioctl node), `/dev/sda`-style for logical drives
- Physical drive passthrough: via `MEGASAS_IOC_FIRMWARE` ioctl with `MFI_CMD_PD_SCSI_IO` or `MFI_CMD_DCMD`
- MegaRAID uses a Management Frame (MFI) or Fusion MPT protocol depending on generation. For broad product coverage, expect to account for MPI, MPI2, and MPI3-era lower-layer behaviors.
- Driver history matters: Broadcom has converged toward a more unified driver model across classic MegaRAID and some non-MegaRAID HBAs, while older IT/IR firmware generations were often surfaced through separate `mfiX`/`mptX`-style stacks. In practice, internal libraries may present a MegaRAID-style API surface and translate to lower-layer controller-specific interfaces.
- Smartctl has MegaRAID support; review `smartctl/dev_areca.cpp` and `megaraid.cpp` for structural guidance
- CrystalDiskInfo has MegaRAID support on Windows and is another useful behavioral reference for cross-checking IOCTL assumptions
- Windows: internal and external tooling often use the same high-level flow on Linux and Windows, suggesting interface compatibility, but treat this as an empirical pattern rather than a guaranteed ABI contract.
- Known command filtering:
    - Trusted Send / Trusted Receive (TCG security flows) are blocked.
    - On 96xx MegaRAID, vendor-unique commands are blocked.

#### Open-Source Reconstruction Path (Theoretical)

For public/open-source implementation work, follow this vendor-neutral MegaRAID reconstruction flow from kernel driver behavior and public headers only:

1. Enumerate controller handles from the OS-visible MegaRAID interface.
2. Use the public MegaRAID firmware ioctl path (`MEGASAS_IOC_FIRMWARE`) with controller-management/DCMD requests to query physical-drive inventory (count/list).
3. For each physical-drive entry, collect addressing fields needed for passthrough (for example persistent device identifier, enclosure/slot/path metadata as exposed by the driver).
4. Cache those fields in an internal per-drive structure used by `tDevice->raid_device`.
5. Issue passthrough requests through the same public firmware ioctl channel:
   - SCSI passthrough path for general command transport.
   - ATA transport via SAT/STP encapsulation when required by controller/driver behavior.

Operational guidance:

- Treat SCSI passthrough as the baseline transport path.
- Do not assume blocked commands return useful drive sense; controller-level policy failures may return controller status/error codes instead.
- Build explicit controller-status-to-openSeaChest error translation, not just sense-data parsing.

### Adaptec ASR Series (PMC-Sierra / Microsemi / Microchip) — `aacraid` driver

> **Vendor disambiguation**: Adaptec controllers use the kernel driver `aacraid` (AAC chipset) and carry the **ASR** (Adaptec SAS RAID) product designation. The `arcmsr` driver is for **Areca** (ARC-series) — a completely different vendor. These two drivers have different IOCTL interfaces and are not interchangeable.

`adaptecRAID` bit already exists. Covers ASR enterprise controllers sold under the Adaptec, PMC-Sierra, Microsemi, and Microchip SmartRAID brand names.

**Kernel source**: `drivers/scsi/aacraid/aacraid.h` (verified against kernel 7.0)

#### Device Access

- **Linux device node**: `/dev/aac0`, `/dev/aac1`, … — one character device per controller, registered via `register_chrdev(0, "aac", &aac_cfg_fops)`.
- **Modern Linux (3.x+)**: The `aacraid` driver began exposing `/dev/sg` nodes for physical drives directly around kernel 3.x. On these kernels, physical drives may appear as standard SG nodes and standard `SG_IO` passthrough works without custom IOCTLs. Confirm by probing the kernel version and checking whether sg nodes for physical drives appear. If they do, prefer the sg path.
- **Pre-3.x Linux**: Physical drive passthrough requires custom `aacraid` IOCTLs through `/dev/aac*`.
- **FreeBSD**: `aac` driver.
- **SCSI ioctl path**: The IOCTL can also be invoked through the SCSI host ioctl path (via `aac_ioctl` registered in `Scsi_Host::hostt->ioctl`).

#### Top-Level IOCTLs

The IOCTL encoding uses a non-standard macro (back-compat with Windows):

```c
#define CTL_CODE(function, method) ((4 << 16) | ((function) << 2) | (method))
#define METHOD_BUFFERED  0
```

| Constant | Value | Purpose |
|----------|-------|---------|
| `FSACTL_SENDFIB` | `CTL_CODE(2050, METHOD_BUFFERED)` | Send a generic FIB (Firmware Interface Block) to the controller |
| `FSACTL_SEND_RAW_SRB` | `CTL_CODE(2067, METHOD_BUFFERED)` | **SCSI passthrough** to a physical drive — primary passthrough path |
| `FSACTL_SEND_LARGE_FIB` | `CTL_CODE(2138, METHOD_BUFFERED)` | Send a large FIB (for commands needing more buffer space) |
| `FSACTL_GET_PCI_INFO` | `CTL_CODE(2119, METHOD_BUFFERED)` | Controller PCI info (vendor/device ID) |
| `FSACTL_GET_HBA_INFO` | `CTL_CODE(2150, METHOD_BUFFERED)` | Adapter info (maps to `aac_hba_info`) |
| `FSACTL_QUERY_DISK` | `0x173` | Query logical disk info |
| `FSACTL_GET_CONTAINERS` | `2131` | Get container list |

#### `user_aac_srb` — SCSI Passthrough to Physical Drive

Sent via `FSACTL_SEND_RAW_SRB`. This is the userspace form (host byte order); the kernel converts to the firmware (`aac_srb`) form internally. The structure ends with a variable-length scatter-gather list.

```c
struct user_aac_srb
{
    u32     function;     // SRBF_ExecuteScsi = 0x0000 for SCSI passthrough
    u32     channel;      // Bus/channel number on controller
    u32     id;           // Target ID on that channel
    u32     lun;          // LUN
    u32     timeout;      // Timeout in seconds
    u32     flags;        // SRB_DataIn=0x0040, SRB_DataOut=0x0080, SRB_NoDataXfer=0x0000
    u32     count;        // Data transfer length in bytes
    u32     retry_limit;  // Retry count; set to 0 for no retry
    u32     cdb_size;     // CDB length (1–16 bytes)
    u8      cdb[16];      // SCSI Command Descriptor Block
    struct user_sgmap sg; // Variable-length scatter-gather list; count + entries[]
};
```

**SRB flags**:

| Constant | Value | Meaning |
|----------|-------|---------|
| `SRB_NoDataXfer` | `0x0000` | No data transfer (e.g., format without data) |
| `SRB_DisableAutosense` | `0x0020` | Do not request auto-sense data |
| `SRB_DataIn` | `0x0040` | Data transfer from device to host (READ) |
| `SRB_DataOut` | `0x0080` | Data transfer from host to device (WRITE) |

**SRB function codes** (set in `function` field):

| Constant | Value | Meaning |
|----------|-------|---------|
| `SRBF_ExecuteScsi` | `0x0000` | **Issue a SCSI CDB to the physical drive** — the only one needed for passthrough |
| `SRBF_AbortCommand` | `0x0010` | Abort an outstanding command |
| `SRBF_ResetBus` | `0x0012` | Reset the SCSI bus |
| `SRBF_ResetDevice` | `0x0013` | Reset a specific device |

#### `aac_srb_reply` — Response Structure

Returned in-place after the IOCTL completes (the IOCTL writes a composite `aac_srb_unit` back to the user buffer — `aac_srb_reply` first, then `aac_srb`).

```c
struct aac_srb_reply
{
    __le32  status;           // Overall status from controller
    __le32  srb_status;       // SRB-level status (see SRB_STATUS_* below)
    __le32  scsi_status;      // SCSI status byte (0 = GOOD, 2 = CHECK CONDITION)
    __le32  data_xfer_length; // Actual bytes transferred
    __le32  sense_data_size;  // Valid bytes in sense_data[]
    u8      sense_data[30];   // AAC_SENSE_BUFFERSIZE = 30; sense buffer is SHORT
};
```

> **Critical**: `AAC_SENSE_BUFFERSIZE = 30` bytes. This is the maximum sense data returned through the SRB path. Do not rely on full descriptor-format sense (which can exceed 30 bytes). Standard fixed-format sense (18 bytes) fits; extended ATA RTFRs in descriptor format may be truncated.

**Selected `srb_status` codes**:

| Constant | Value | Meaning |
|----------|-------|---------|
| `SRB_STATUS_SUCCESS` | `0x01` | Command completed successfully |
| `SRB_STATUS_ABORTED` | `0x02` | Command was aborted |
| `SRB_STATUS_ERROR` | `0x04` | General error |
| `SRB_STATUS_BUSY` | `0x05` | Device busy |
| `SRB_STATUS_NO_DEVICE` | `0x08` | No device at channel/id/lun |
| `SRB_STATUS_TIMEOUT` | `0x09` | Command timed out |
| `SRB_STATUS_COMMAND_TIMEOUT` | `0x0B` | Drive-level command timeout |
| `SRB_STATUS_DATA_OVERRUN` | `0x12` | Data overrun |
| `SRB_STATUS_INVALID_LUN` | `0x20` | Invalid LUN |
| `SRB_STATUS_INVALID_TARGET_ID` | `0x21` | Invalid target ID (drive not present) |
| `SRB_STATUS_BAD_FUNCTION` | `0x22` | Bad `function` field value |

#### Physical Drive Discovery

Two complementary paths exist for enumerating physical drives behind the `aacraid` controller:

**Path A — SCSI REPORT PHYSICAL LUNS (via SRB)**: Send opcode `0xC3` (`CISS_REPORT_PHYSICAL_LUNS`) as a SCSI command through `FSACTL_SEND_RAW_SRB` directed at the controller's virtual target. Returns `aac_ciss_phys_luns_resp` — a list of physical LUN entries with `tid[3]` (target ID), `bus`, and `node_ident[16]` (16-byte identifier).

```c
// CISS opcodes known to aacraid (defined in aacraid.h)
#define CISS_REPORT_PHYSICAL_LUNS    0xC3  // SCSI opcode; returns phys LUN list
#define CISS_IDENTIFY_PHYSICAL_DEVICE 0x15  // BMIC opcode; returns per-drive details
#define BMIC_IN                      0x26  // BMIC read transport opcode
#define BMIC_OUT                     0x27  // BMIC write transport opcode
```

**Path B — BMIC IDENTIFY PHYSICAL DEVICE**: Send BMIC opcode `0x15` (`CISS_IDENTIFY_PHYSICAL_DEVICE`) via the `BMIC_IN` (0x26) transport. Returns `aac_ciss_identify_pd`, which contains:
- `scsi_bus`, `scsi_id`, `scsi_lun` — how to address the drive in subsequent SRB passthrough
- `model[40]`, `serial_number[40]`, `firmware_revision[8]`
- `device_type` — drive type (HDD, SSD, etc.)
- `sata_version` — indicates SATA drives when non-zero
- `big_total_block_count` — 64-bit LBA count
- `wwid[20]` — world-wide identifier
- `current_temperature_degreesC`, `temperature_threshold_degreesC`
- `SanitizeSecureEraseSupport`, `DriveKeyFlags`

> **Verification required — CISS-hybrid timing unknown**: The `CISS_REPORT_PHYSICAL_LUNS (0xC3)` and `BMIC_IN/BMIC_OUT (0x26/0x27)` definitions are present in `aacraid.h` (kernel 7.0), but the kernel version in which this CISS-compatible enumeration path was introduced into the aacraid driver is unknown without `git log` analysis (`git log --follow drivers/scsi/aacraid/aacraid.h`). Older firmware and older kernel/driver versions may not respond to these opcodes. **Do not assume this path is universally supported.** Verify on a real Adaptec ASR controller before depending on it in production. Fall back to the standard SRB target enumeration path if BMIC commands are not acknowledged.

#### SATA / ATA Passthrough

The `aacraid` SDK (Adaptec SDK 3.01, `SoulAPIHeaders/scsifib.h`) defines `HOST_SATA_REQUEST_BLOCK` — a structure that packs ATA register fields (command, features, sector/cylinder/device registers, Ext variants, sector counts) plus a `SgMap64`. The SDK comment states:

> "SataPassThrough utilizes the same scsi structures to send a SATA command to a CHIM with a Scsi interface. The command is received by firmware in the HOST_SCSI_REQUEST_BLOCK->Cdb fields."

This means **ATA registers are packed into the `cdb[]` field of `user_aac_srb`** in a vendor-defined format, not as a standard SAT ATA PASS-THROUGH CDB. The firmware on the controller interprets these packed registers natively. This is the Adaptec ATA fast-path; it bypasses SAT translation and carries raw ATA TFR registers directly.

**Practical approach**: For the initial implementation, use **SAT ATA PASS-THROUGH (0x85/0xA1)** CDB via `SRBF_ExecuteScsi` — the controller firmware likely includes a SAT translator. Verify with a simple IDENTIFY DEVICE command (0xEC). If the controller SAT path works, this avoids reverse-engineering the vendor ATA packing format. Document the ATA register packing format later when direct ATA is confirmed needed.

#### CSMI Subset

The `aacraid` driver is identified in openSeaChest as `CSMI_DRIVER_ADAPTEC` with driver name `arcsas`. It implements a limited CSMI subset believed to cover SMP (SAS Management Protocol) passthrough IOCTLs only. **Do not use CSMI for physical drive SCSI passthrough on Adaptec** — use `FSACTL_SEND_RAW_SRB` instead.

#### Windows Cross-Platform Compatibility — The Primary Implementation Gap

This is the **most important architectural note** for `aacraid` implementation. The `aacraid.h` kernel header contains:

```c
// From drivers/scsi/aacraid/aacraid.h — comment is verbatim from the kernel:
//   "Ugly - non Linux like ioctl coding for back compat."
#define CTL_CODE(function, method) ( \
    (4 << 16) | ((function) << 2) | (method) \
)
#define METHOD_BUFFERED  0
```

This is a **deliberate replica of the Windows `CTL_CODE` macro** with `DeviceType=4` and `Access=0` hardcoded. The standard Windows form (from `winioctl.h`) is:

```c
// Windows winioctl.h:
#define CTL_CODE(DeviceType, Function, Method, Access) \
    (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
```

Setting `DeviceType=4, Access=0` in the Windows form produces **exactly the same result** as the Linux 2-parameter form:

```
FSACTL_SEND_RAW_SRB = CTL_CODE(2067, 0)         // Linux
                    = (4 << 16) | (2067 << 2) | 0  = 0x4204C

FSACTL_SEND_RAW_SRB = CTL_CODE(4, 2067, 0, 0)   // Windows equivalent
                    = (4 << 16) | (0 << 14) | (2067 << 2) | 0 = 0x4204C
```

**The IOCTL codes are numerically identical on Linux and Windows.** The `user_aac_srb` structure uses platform-independent integer widths (`u32`, `u8`). The driver was explicitly designed for cross-platform IOCTL compatibility.

##### Windows Call Path

| Aspect | Linux | Windows |
|--------|-------|---------|
| Open device | `open("/dev/aac0", O_RDWR)` | `CreateFile("\\\\.\\Aac0", GENERIC_READ\|GENERIC_WRITE, FILE_SHARE_READ\|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)` |
| IOCTL call | `ioctl(fd, FSACTL_SEND_RAW_SRB, &srb)` | `DeviceIoControl(hDev, FSACTL_SEND_RAW_SRB, pSrb, srbSize, pReply, replySize, &bytesReturned, NULL)` |
| IOCTL code value | `0x4204C` | `0x4204C` (identical) |
| Structure layout | `user_aac_srb` + `user_sgmap` | Same structure; METHOD_BUFFERED means kernel copies data — SG list used differently |
| Response | Returned in-place via `ioctl` output | Returned in `lpOutBuffer` of `DeviceIoControl` |

> **Device path on Windows**: The Adaptec Windows driver exposes a named device. The path is most likely `\\.\Aac0`, `\\.\Aac1`, etc. (mirroring the Linux `/dev/aac0` convention), but verify against the installed driver by querying `\\.\` device names or checking the driver's `IoCreateDevice` / `IoCreateSymbolicLink` calls. An alternative path is through `\\.\ScsiPort0` or `\\.\PhysicalDriveN` via SCSI ioctl — check both if direct open fails.

##### Windows Scatter-Gather Difference

On Linux, `user_sgmap.sg[].addr` contains a direct user-space virtual address. On Windows with `METHOD_BUFFERED`, the kernel driver handles all memory mapping automatically:

- Pass the SRB as `lpInBuffer` to `DeviceIoControl`.
- The data buffer (IDENTIFY response, SMART data, etc.) is returned in `lpOutBuffer`, appended after `aac_srb_unit`, or the driver may use an embedded buffer pointer in the SRB structure.
- Do **not** fill in scatter-gather user addresses on Windows; the METHOD_BUFFERED contract eliminates the need. Set `sg.count = 0` or embed the data buffer reference as the driver's Windows interface expects.

Verify the exact data-return convention by testing with a simple read command and inspecting what the driver places in the output buffer.

##### Implementation Recommendation

Given the IOCTL code identity, implement `aacraid` passthrough as a **single shared code path** with an OS abstraction only at the device open/close and IOCTL dispatch layers:

```c
// Shared logic — works on both platforms with the same IOCTL code
#if defined(_WIN32)
    BOOL ok = DeviceIoControl(hDev, FSACTL_SEND_RAW_SRB,
                              pSrb, srbSize, pReply, replySize, &bytes, NULL);
#else
    int rc = ioctl(fd, FSACTL_SEND_RAW_SRB, pSrb);
#endif
```

This is a **lower porting burden than any other RAID driver** — no separate Windows header, no separate Windows structure, no IOCTL code translation needed.

#### Management SDK

Adaptec's official C++ SDK (`arcSDK::StorLibSDK`, distributed as a pre-built library) wraps `aacraid`, CISS, CSMI, SmartPQI, and BMIC into a unified object model. The SDK is internally wrapped in C in `arclib_helper.cpp` (with `ENABLE_ARC_SUPPORT` compile guard). The openSeaChest approach for open-source builds is to use the kernel IOCTL interface directly rather than the proprietary SDK.

#### Smartctl Reference

Smartctl has Adaptec support (look for `dev_aacraid.cpp` or the aacraid handling in `dev_interface.cpp`). Use this for behavioral reference on physical drive enumeration ordering and error translation. Do not assume the IOCTL calling convention is identical — smartctl may use a wrapper around the proprietary SDK on some platforms.

#### Legacy Adaptec Parallel SCSI HBAs — aic7xxx / aic79xx

The **AIC-7xxx / AIC-79xx** family (Adaptec Ultra-160 / Ultra-320 Parallel SCSI chips) predates RAID entirely. These are pure HBAs — there is no RAID volume; drives behind them appear directly as standard SCSI devices via the kernel SCSI mid-layer.

Key facts confirmed from `drivers/scsi/aic7xxx/aic79xx_osm.c` (kernel 7.0):
- Registers a plain `scsi_host_template` with **no `unlocked_ioctl` handler** — zero custom IOCTL surface.
- Drives are exposed as standard `/dev/sdX` block devices and as `/dev/sgX` raw SCSI nodes.
- **Standard `SG_IO` passthrough already works** — no RAID-specific code needed in openSeaChest.

The Adaptec arclib / StorLib SDK historically wrapped aic7xxx for controller management (NVRAM, SEEPROM configuration, microcode download). It does not affect drive access. The open-source kernel driver source provides the complete management interface without needing the proprietary SDK.

**Action**: If an aic7xxx / aic79xx device appears in the field, it should already work through openSeaChest's standard Linux SG passthrough path. No `raidTypeHint` bit is needed. If controller-level management (NVRAM queries, HBA diagnostics) is ever required, the aic79xx proc interface (`aic79xx_proc.c`) documents the internal state exposure, and `aic79xx_osm.h` / `aic7xxx_osm.h` document the host data structures — sufficient to implement direct management without the proprietary library.

### Highpoint RAID (`hptiop`)

`highpointRAID` bit already exists.

**Kernel source**: `drivers/scsi/hptiop.h` (covers RR3xxx/4xxx series — HighPoint's queue-based SAS/SATA RAID generation)

The `hptiop` driver uses an IOP (I/O Protocol) firmware architecture with shared-memory queue submission rather than a simple `ioctl()` pass-through. The kernel driver handles the queue internally; user-space tools access physical drives via a character device.

#### Firmware Request Types (from `hptiop.h`)

```c
enum hpt_iop_request_type {
    IOP_REQUEST_TYPE_GET_CONFIG     = 0,   /* Query IOP configuration */
    IOP_REQUEST_TYPE_SET_CONFIG     = 1,   /* Set IOP configuration */
    IOP_REQUEST_TYPE_BLOCK_COMMAND  = 2,   /* High-level block read/write */
    IOP_REQUEST_TYPE_SCSI_COMMAND   = 3,   /* SCSI command passthrough */
    IOP_REQUEST_TYPE_IOCTL_COMMAND  = 4,   /* Firmware management IOCTL */
};
```

#### SCSI Passthrough Structure

`IOP_REQUEST_TYPE_SCSI_COMMAND` carries raw SCSI CDBs to physical drives:

```c
struct hpt_iop_request_scsi_command {
    struct hpt_iop_request_header header;
    u8     channel;           /* Physical channel (0-based) */
    u8     target;            /* Physical target ID */
    u8     lun;               /* LUN */
    u8     pad1;
    u8     cdb[16];           /* SCSI CDB, up to 16 bytes */
    __le32 dataxfer_length;   /* Data transfer byte count */
    struct hpt_iopsg sg_list[]; /* Scatter-gather list (eot field marks end) */
};
```

#### Firmware Management IOCTL

`IOP_REQUEST_TYPE_IOCTL_COMMAND` issues management commands to the firmware (not SCSI passthrough):

```c
struct hpt_iop_request_ioctl_command {
    struct hpt_iop_request_header header;
    __le32 ioctl_code;       /* Firmware-defined management opcode */
    __le32 inbuf_size;       /* Input buffer size in bytes */
    __le32 outbuf_size;      /* Output buffer size in bytes */
    __le32 bytes_returned;   /* Actual bytes returned in out buffer */
    u8     buf[];            /* Input buffer; out data at buf[(inbuf_size+3)&~3] */
};
```

Result codes: `IOP_RESULT_SUCCESS`, `IOP_RESULT_FAIL`, `IOP_RESULT_BUSY`, `IOP_RESULT_INVALID_REQUEST`, `IOP_RESULT_BAD_TARGET`, `IOP_RESULT_CHECK_CONDITION`.

#### Hardware Variants

The driver supports three IOP hardware variants (different register layouts in the IOPMU):

- **ITL** — original IOP memory unit with `inbound_head`, `inbound_tail`, `outbound_head`, `outbound_tail` queue pointers
- **MV** (Marvell-based) — `inbound_base`, `inbound_base_high`, `outbound_read_index`, `inbound_write_index`
- **MVFREY** — Marvell Frey-based, additional `f_index` and `u_index` ring management

#### Implementation Notes

- Linux driver: `hptiop`, `hpt366`, `hpt37x` (older generations with different architecture)
- FreeBSD driver: `hptiop`
- The SCSI passthrough uses `channel`/`target`/`lun` to address physical drives. Physical drives are NOT exposed as SCSI generic (`/dev/sg`) nodes via the RAID volume interface — they are hidden behind RAID volumes and reachable only via the char device.
- Smartctl has HighPoint support — review `dev_areca.cpp` in smartctl (HighPoint and Areca historically shared code paths in some smartctl versions) and `os_linux.cpp` for the char device open/ioctl paths.
- **CDB limit**: 16 bytes
- **Windows portability**: HighPoint ships Windows drivers; the IOP firmware interface is proprietary but the vendor likely provides a Windows SDK or IOCTL header. Treat as likely portable but requires Windows driver investigation.

### Areca RAID

`areccaRAID` bit already exists (note the spelling in the code is `areccaRAID`).

- Linux driver: `arcmsr`
- FreeBSD: `arcmsr`
- Device nodes: `/dev/arcmsr*`
- Smartctl has Areca support via `dev_areca.cpp` — a well-documented reference

---

### ATTO Technology ExpressSAS (`esas2r`) — **HIGH PRIORITY**

**No `raidTypeHint` bit yet — one must be added.** Covers both HBA (ESASHBA) and RAID (ESASRAID) products from ATTO Technology.

**Kernel sources**: `drivers/scsi/esas2r/atioctl.h` (IOCTL header), `drivers/scsi/esas2r/esas2r_ioctl.c` (dispatch implementation)

The driver has a clean, hierarchical IOCTL system. The **HBA path** (`EXPRESS_IOCTL_HBA`) is the right primary implementation target — it is the direct firmware interface. The CSMI path (`EXPRESS_CSMI`) is also available but just wraps the same firmware calls; it is not needed for a new implementation.

#### Linux Device Access

`esas2r_ioctl()` is registered as the `scsi_host_template.ioctl` callback. This means the ATTO IOCTLs are accessible through **any `/dev/sg*` device that belongs to the esas2r SCSI host**. Open any SG node on this HBA and call `ioctl(fd, EXPRESS_IOCTL_*, &buf)` directly. The `cmd` argument to `ioctl()` IS the `EXPRESS_IOCTL_*` constant (range `0x4500`–`0x450F`). All IOCTLs require `atto_express_ioctl_header.signature = "Express"` (8 bytes, no null).

There is no dedicated char device — the SCSI generic path IS the management path.

#### Adapter Enumeration (Step 0)

Start with `EXPRESS_IOCTL_GET_CHANNELS (0x4504)` on any SG node. This returns the count of all esas2r adapters in the system and their **channel indices** without needing to know the adapter in advance:

```c
/* Fill header, send ioctl(sg_fd, 0x4504, &express_buf) */
/* Result: express_buf.data.chanlist.num_channels + .channel[] array of adapter indices */
```

All subsequent commands use `atto_express_ioctl_header.channel` to select the target adapter. Setting `channel = 0xFF` means "use the adapter this sg device belongs to."

#### IOCTL Function Map

```
EXPRESS_IOCTL_GET_CHANNELS = 0x4504  — enumerate all adapters → chanlist.num_channels / .channel[]
EXPRESS_IOCTL_CHAN_INFO     = 0x4505  — adapter identity (PCI IDs, IRQ, host_no, driver rev)
EXPRESS_IOCTL_HBA           = 0x450C  — HBA management functions (ATTO_FUNC_*) ← PRIMARY PATH
EXPRESS_IOCTL_VDA           = 0x450D  — VDA commands (SCSI to virtual targets, SMP, flash, cfg)
EXPRESS_IOCTL_SMP           = 0x450A  — SMP passthrough (direct SAS Management Protocol frames)
EXPRESS_CSMI                = 0x450B  — CSMI wrapper (calls handle_csmi_ioctl → delegates to HBA)
EXPRESS_IOCTL_GET_ID        = 0x450E  — adapter identification
EXPRESS_RW_MEMORY           = 0x4508  — memory read/write (debug/firmware)
```

#### Primary Path: `EXPRESS_IOCTL_HBA` (`0x450C`)

Send `ioctl(sg_fd, 0x450C, &express_buf)` with `express_buf.data.ioctl_hba` populated. The `atto_ioctl.function` byte selects the sub-operation:

```c
struct atto_ioctl {
    u8 version;
    u8 function;    /* ATTO_FUNC_XXX — selects operation */
    u8 status;      /* ATTO_STS_SUCCESS/FAILED/INV_VERSION/INV_FUNC/UNSUPPORTED/etc. */
    u8 flags;       /* HBAF_TUNNEL = 0x01 — tunnels to firmware */
    u32 data_length;
    u8 reserved2[56];
    union {
        struct atto_hba_get_adapter_info  get_adap_info;   /* function 0x00 */
        struct atto_hba_get_adapter_address get_adap_addr; /* function 0x01 */
        struct atto_hba_scsi_pass_thru    scsi_pass_thru;  /* function 0x04 */
        struct atto_hba_get_device_address get_dev_addr;   /* function 0x05 */
        struct atto_hba_adap_ctrl         adap_ctrl;       /* function 0x0E */
        struct atto_hba_get_device_info   get_dev_info;    /* function 0x0F */
        /* ... */
    } data;
};
```

`ATTO_STS_*` codes: `SUCCESS=0x00`, `FAILED=0x01`, `INV_VERSION=0x02`, `OUT_OF_RSRC=0x03`, `INV_FUNC=0x04`, `UNSUPPORTED=0x05`, `INV_ADAPTER=0x06`, `INV_DRVR_VER=0x07`, `INV_PARAM=0x08`, `TIMEOUT=0x09`, `NOT_APPL=0x0A`, `DEGRADED=0x0B`.

#### Step 1 — Adapter Capability Check (`ATTO_FUNC_GET_ADAP_INFO = 0x00`)

```c
/* Check that physical drive passthrough is supported */
#define ATTO_GAI_TF_SCSI_PASS_THRU  0x00000004  /* bit in get_adap_info.tunnel_flags */

/* Adapter class (HBA vs RAID, gen 1 vs gen 2) */
#define ATTO_GAI_AT_ESASRAID   0   /* RAID class — may have RAID volumes AND physical drives */
#define ATTO_GAI_AT_ESASHBA    1   /* Pure SAS HBA */
#define ATTO_GAI_AT_ESASRAID2  2
#define ATTO_GAI_AT_ESASHBA2   3
/* ... CELERITY, CELERITY8, CELERITY16, FASTFRAME, TLSASHBA, ESASHBA3, ESASHBA4 */
```

If `tunnel_flags & ATTO_GAI_TF_SCSI_PASS_THRU` is set, physical drive passthrough is available.

#### Step 2 — Physical Drive Discovery (`ATTO_FUNC_GET_DEV_INFO = 0x0F`)

Enumerate `target_id` starting at 0 and incrementing until the status is not `ATTO_STS_SUCCESS`. Each valid target returns `atto_hba_get_device_info`:

```c
struct atto_hba_get_device_info {
    u32 target_id;
    u8  info_type;  /* ATTO_GDI_IT_SAS=1 / FC=2 / FCOE=3 / UNKNOWN=0 */
    u8  reserved[11];
    union atto_hba_device_info dev_info;  /* sas_dev_info for SAS */
};

struct atto_hba_sas_device_info {
    u8  phy_id[16];       /* PHY IDs of parent expander/adapter (16 PHY wide port) */
    u32 exp_target_id;    /* Parent expander target_id (if behind expander) */
    u32 sas_port_mask;
    u8  sas_level;        /* 0 = direct, higher = expander hops */
    u8  slot_num;         /* Physical slot number if available */
    u8  dev_type;         /* ATTO_SDI_DT_END_DEVICE=0, EXPANDER=1, PORT_MULT=2 */
    u8  ini_flags;
    u8  tgt_flags;
    u8  link_rate;        /* SMP_RATE_XXX */
    u8  loc_flags;        /* ATTO_SDI_LF_DIRECT/EXPANDER/PORT_MULT */
    u8  pm_port;
};
```

Filter for `dev_type == ATTO_SDI_DT_END_DEVICE` — those are the physical drives.

#### Step 3 — Get Drive Address (`ATTO_FUNC_GET_DEV_ADDR = 0x05`)

After identifying end devices, retrieve the SAS node address for each `target_id`:

```c
struct atto_hba_get_device_address {
    u8  addr_type;   /* ATTO_GDA_AT_PORT=0 / NODE=1 / MAC=2 / PORTID=3 / UNIQUE=4 */
    u8  reserved;
    u16 addr_len;    /* Filled in by driver on return */
    u32 target_id;   /* Set by caller */
    u8  address[256]; /* SAS address (8 bytes for NODE type), or other format */
};
```

Use `ATTO_GDA_AT_NODE` to get the 8-byte SAS World Wide Name. This is the persistent address for the typed handle.

#### Step 4 — SCSI Passthrough (`ATTO_FUNC_SCSI_PASS_THRU = 0x04`)

```c
struct atto_hba_scsi_pass_thru {
    u8  cdb[32];          /* Up to 32-byte CDB — more than any other RAID controller */
    u8  cdb_length;
    u8  req_status;       /* ATTO_SPT_RS_SUCCESS/FAILED/OVERRUN/NO_DEVICE/NO_LUN/IOERR/etc. */
    u8  scsi_status;      /* Standard SCSI status */
    u8  sense_length;
    u32 flags;            /* ATTO_SPTF_DATA_IN=1 / DATA_OUT=2 / SIMPLE_Q=4 / HEAD_OF_Q=8 / ORDERED_Q=0x10 */
    u32 timeout;          /* Seconds */
    u32 target_id;        /* From enumeration step */
    u8  lun[8];           /* 8-byte LUN structure */
    u32 residual_length;
    u8  sense_data[0xFC]; /* 252 bytes of sense */
    u8  reserved[0x28];
};
```

`req_status`: `SUCCESS=0x00`, `FAILED=0x01`, `OVERRUN=0x02`, `UNDERRUN=0x03`, `NO_DEVICE=0x04`, `NO_LUN=0x05`, `TIMEOUT=0x06`, `BUS_RESET=0x07`, `ABORTED=0x08`, `BUSY=0x09`, `DEGRADED=0x0A`.

#### Typed Handle Format

With `channel` (adapter index from GET_CHANNELS) and `target_id` (from GET_DEV_INFO enumeration):

```
atto:<channel>:<target_id>:<lun>
Example: atto:0:7:0
```

#### SMP Passthrough (`EXPRESS_IOCTL_SMP = 0x450A`)

Direct SAS Management Protocol frames for expander management, via `atto_ioctl_smp`. Also available via `EXPRESS_IOCTL_VDA + atto_ioctl_vda_smp_cmd` (which uses a u64 `dest` SAS address rather than a target_id).

#### About CSMI on esas2r

`EXPRESS_CSMI (0x450B)` routes to `handle_csmi_ioctl()`, which translates CSMI structures into the same firmware requests that `handle_hba_ioctl()` submits directly. **Do not implement the CSMI path for Linux** — it adds translation overhead for no benefit. The HBA path is simpler and gives access to features CSMI doesn't expose (e.g., 32-byte CDB, direct target enumeration).

On **Windows**, the story may differ: openSeaChest already issues CSMI IOCTLs via `IOCTL_SCSI_MINIPORT`. If ATTO's Windows driver also exposes CSMI through `IOCTL_SCSI_MINIPORT` (the standard Windows CSMI delivery mechanism), existing openSeaChest code **may already work on Windows for ATTO adapters without any new code**. Investigate whether the Windows ATTO miniport driver responds to `IOCTL_SCSI_MINIPORT` with CSMI sub-codes.

#### Platform Notes

- **Linux**: Via `ioctl(sg_fd, 0x450C, &buf)` on any `/dev/sg*` belonging to the esas2r host. No dedicated char device needed.
- **Windows**: Open `\\.\SCSI<X>:` for the esas2r port, then `DeviceIoControl(h, IOCTL_SCSI_MINIPORT, pBuf, ...)` with `SRB_IO_CONTROL.Signature = "Express"` and `SRB_IO_CONTROL.ControlCode = EXPRESS_IOCTL_HBA` (or whichever ATTO sub-command). The payload struct layout is the same as Linux. The CSMI path (`ControlCode = EXPRESS_CSMI`) is also available on Windows and may already work with openSeaChest's existing CSMI infrastructure through this same `IOCTL_SCSI_MINIPORT` channel.
- **FreeBSD**: `esas2r` is Linux-only upstream — verify whether ATTO ships a FreeBSD driver separately.

---

### Mylex/IBM DAC960 (`myrb` / `myrs`)

**No `raidTypeHint` bit exists yet — one must be added if implementing.** Mylex was acquired by IBM in 1999; these controllers shipped as IBM DAC960, AcceleRAID, and eXtremeRAID brands.

**Kernel sources**: `drivers/block/myrb.h` (V1 firmware), `drivers/block/myrs.h` (V2 firmware)

> **Priority note**: This is very old hardware (late 1990s–early 2000s). The Linux kernel still carries the driver, but encountering DAC960 hardware in production is rare. Implement only if a specific customer need arises.

#### V1 Firmware — `myrb` (DCDB: Direct CDB)

Passthrough via "DCDB" (Direct CDB) commands — `MYRB_CMD_DCDB = 0x04`:

```c
struct myrb_dcdb {
    unsigned target:4;                /* Physical SCSI target (0-15) */
    unsigned channel:4;               /* SCSI channel (0-15) */
    enum myrb_dcdb_xfer data_xfer:2; /* NONE / DEVICE_TO_SYSTEM / SYSTEM_TO_DEVICE */
    unsigned early_status:1;
    unsigned rsvd1:1;
    enum myrb_dcdb_tmo timeout:2;    /* TEN_SECS / SIXTY_SECS / TEN_MINS / TWENTY_FOUR_HRS */
    unsigned no_autosense:1;
    unsigned allow_disconnect:1;
    unsigned short xfer_len_lo;       /* Transfer length (low 16 bits) */
    u32 dma_addr;                     /* DMA address (physical — user-space fills virtual) */
    unsigned char cdb_len:4;          /* CDB length (4-bit field, so max = 15 — effective max 12) */
    unsigned char xfer_len_hi4:4;     /* Transfer length high nibble */
    unsigned char sense_len;          /* Sense buffer length to return */
    unsigned char cdb[12];            /* SCSI CDB — **hard limit of 12 bytes** in V1 firmware */
    unsigned char sense[64];          /* 64 bytes sense data */
    unsigned char status;             /* SCSI status byte */
    unsigned char rsvd2;
};
```

- `MYRB_CMD_DCDB_SG = 0x84` — scatter-gather variant of DCDB
- **12-byte CDB maximum** — ATA PASS-THROUGH (16) will NOT work. Only 6/10/12-byte SCSI CDBs are available. Consider using INQUIRY, READ CAPACITY (10), and shorter READ/WRITE (10) only.
- IOCTL submission: via mailbox command (`union myrb_cmd_mbox`) to the controller register window.

Status codes: `MYRB_STATUS_DEVICE_BUSY=0x0008`, `MYRB_STATUS_DEVICE_NONRESPONSIVE=0x000E`.

#### V2 Firmware — `myrs` (Enhanced DCDB)

- **`MYRS_DCDB_SIZE = 16`** — V2 raises the CDB limit to 16 bytes, enabling ATA PASS-THROUGH (16)
- **`MYRS_SENSE_SIZE = 14`** — slightly shorter sense buffer than V1
- Uses a V2 command mailbox (`union myrs_cmd_mbox`) with a separate V2 opcode set
- Same channel/target/lun addressing model as V1

#### Implementation Notes

- **Character device**: `/dev/rd/c0d0` (old Red Hat style) — check kernel source or smartctl for current paths
- The kernel driver still exists in modern kernels; hardware is extremely rare
- No CSMI, no SAS (pure parallel SCSI channel model)
- Smartctl historically had DAC960 support; review smartctl history for the exact ioctl call path

---

### PMC Sierra MaxRAID (`pmcraid`)

**No `raidTypeHint` bit exists yet — one must be added if implementing.**

**Kernel source**: `drivers/scsi/pmcraid.h`

> **Platform note**: PMC MaxRAID is primarily used in IBM pSeries/Power Systems servers (POWER7+, IBM i, AIX workloads). This driver is relevant for Linux on Power/IBM POWER hardware. x86 usefulness is low unless the hardware appears in x86 servers.

#### IOCTL Interface

```c
#define PMCRAID_DRIVER_IOCTL  'D'
#define DRV_IOCTL(n, size) \
    _IOC(_IOC_READ | _IOC_WRITE, PMCRAID_DRIVER_IOCTL, (n), (size))

/* All PMC passthrough IOCTLs begin with this header */
struct pmcraid_ioctl_header {
    u8  signature[8];    /* Must be "PMCRAID\0" */
    u32 reserved;
    u32 buffer_length;   /* Byte length of data following this header */
};

#define PMCRAID_IOCTL_SIGNATURE  "PMCRAID"
#define PMCRAID_IOCTL_RESET_ADAPTER  DRV_IOCTL(5, sizeof(struct pmcraid_ioctl_header))
```

- **Device file**: `/dev/pmcsas` (created by the driver)
- **Resource types**: `PMCRAID_RES_TYPE_GSCSI` (Generic SCSI = physical drives behind the RAID), `PMCRAID_RES_TYPE_VSET` (virtual RAID volume), `PMCRAID_RES_TYPE_AF_DASD` (array function disk), `PMCRAID_RES_TYPE_IOA_FP` (IOA functional partition)
- **Max CDB**: `PMCRAID_MAX_CDB_LEN` (defined in header; exact value is 16 for standard IOARCB commands)
- **Passthrough IOCTL**: IOCTLs use `DRV_IOCTL` with the `'D'` magic — standard Linux `_IOC` format; not directly Windows-portable, but IOCTL encoding can be reconstructed from the `n` and `size` parameters
- **DMA list**: `pmcraid_sglist` is used for data transfer in passthrough IOCTLs; user-space provides virtual addresses and the driver maps them

#### Implementation Notes

- Physical drives are accessed via the `GSCSI` resource type — list resources, identify GSCSI entries, issue passthrough to those
- IBM Power-centric hardware; SPARC/x86 occurrence is rare
- FreeBSD: unlikely — no upstream FreeBSD `pmcraid` driver

---

### IBM IPR RAID (`ipr`)

**No `raidTypeHint` bit.** The `ipr` (IBM Power RAID) driver version `2.6.4` is an IBM-proprietary RAID adapter used in IBM Power Systems and IBM i (formerly OS/400) server hardware.

**Kernel source**: `drivers/scsi/ipr.h`

- **Platform**: Linux on IBM POWER (pSeries). Not relevant for x86 or other architectures.
- The driver has a char device interface for management, but passthrough details are not documented in the public kernel header in a straightforward way.
- **Priority**: Very low — IBM Power-only, no x86 relevance.
- If implementing: contact IBM or review IBM's open-source SDK for AIX-to-Linux driver equivalents.

---

### IBM/Adaptec ServeRAID (`ips`)

**No `raidTypeHint` bit.** IBM ServeRAID (based on Adaptec RAID technology, OEM'd and sold under the IBM brand 1999–2003).

**Kernel source**: `drivers/scsi/ips.h`

> **Do not implement** — the `ips` driver was **removed from the Linux kernel** around kernel 5.x (the hardware was end-of-life and the driver unmaintained). Any system still running this hardware uses a very old kernel. Implementing support in openSeaChest provides no practical value.

- Character device was historically `/dev/ips0`
- Hardware: ISA/PCI-era IBM ServeRAID models (ServeRAID-1, 2, 3, 4, 5, 6, 7 in the IBM product line)
- If encountered: recommend upgrading hardware

---

### Legacy SAS/SCSI HBAs — No RAID Passthrough Needed

The following controllers are **pure SCSI HBAs with no RAID function**. Drives attached to them appear as standard SCSI block devices accessible via `/dev/sg*` using the standard `SG_IO` ioctl. No proprietary RAID passthrough is required — openSeaChest's existing SCSI/SAS transport path handles them transparently.

| Driver | Hardware | Notes |
|--------|----------|-------|
| `aha152x` | Adaptec AHA-152x (ISA Parallel SCSI HBA) | Port I/O via `SETPORT`/`GETPORT` (inb/outb); drives visible as `/dev/sg*`; same as aic7xxx |
| `a100u2w` | Initio INI-A100U2W (PCI Ultra2 Wide SCSI HBA) | Orchid Host Command Set; pure HBA; drives on standard SCSI path |
| `qlogicpti` | QLogic ISP SBUS (SPARC SBUS bus only) | SBUS-specific; irrelevant for x86/ARM; SCSI HBA, no RAID |

These join `aic7xxx` and `aic79xx` (noted in an earlier section) as HBAs where the standard SG_IO path is the correct approach — no new code is needed in openSeaChest.

---

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
| CISS/HP SmartArray (cciss/hpsa) | Yes — `hpsa` is the modern replacement for `cciss`; both maintain `CCISS_PASSTHRU` ioctl compatibility; `hpsa_cmd.h` defines current command structures | Yes | Likely | Likely | Possible — `winciss.h` confirms structures; `__unix__` guard must be lifted; device path unverified | Unlikely |
| SmartPQI (Linux) | Yes — `CCISS_PASSTHRU` compat; existing CISS code works | FreeBSD uses separate `SMARTPQI_PASS_THRU` IOCTL (`'M'` magic, not `'C'`) | Likely | Likely | Unknown | Unlikely |
| MegaRAID | Planned | Planned | Likely same as Linux | Follows Linux | Likely portable | Unlikely |
| Adaptec ASR (aacraid) | Planned — sg nodes on 3.x+ kernels; custom IOCTL for pre-3.x | Planned | Likely | Follows Linux | **High potential** — `FSACTL_*` codes are identical to Windows CTL_CODE values by design; same `user_aac_srb` structure; only device path + `DeviceIoControl` wrapper needed | Unlikely |
| 3ware Gen 1 (3w-xxxx) | Planned — `/dev/tw0`; `TW_IOCTL=0x80`; ATA register passthrough only (no SCSI CDB) | Historically supported | Unlikely | Unlikely | Likely portable — `0x80` is a plain integer code; ATA-register mode limits operations | Unlikely |
| 3ware Gen 2 (3w-9xxx) | Planned — `/dev/twa0`; `TW_IOCTL_FIRMWARE_PASS_THROUGH=0x108`; 16-byte SCSI CDB | Likely | Unlikely | Unlikely | Likely portable — plain integer control code; same `TW_Ioctl_Buf_Apache` layout | Unlikely |
| 3ware Gen 3 (3w-sas) | Planned — `/dev/twl0`; same `0x108` control code (kernel comment: "Used by smartmontools"); 16-byte SCSI CDB | Likely | Unlikely | Unlikely | Likely portable — plain integer control code; same layout as Gen 2 | Unlikely |
| Highpoint (`hptiop`) | Planned — queue-based IOP; char device; `IOP_REQUEST_TYPE_SCSI_COMMAND`; `cdb[16]`; channel/target/lun addressing | `hptiop` driver exists | Unlikely | Unlikely | Investigate HighPoint Windows SDK — IOP firmware interface may be identical | Unlikely |
| ATTO ExpressSAS (`esas2r`) | **High priority** — CSMI path may already work via existing openSeaChest CSMI code; native path adds `cdb[32]` + 252-byte sense | Unlikely (Linux-only upstream) | Unlikely | Unlikely | Likely portable via CSMI path (CSMI designed for Windows); native path needs ATTO Windows driver headers | Unlikely |
| Areca (`arcmsr`) | Planned — `/dev/arcmsr*` | Yes | Unlikely | Unlikely | Investigate Areca Windows drivers | Unlikely |
| Mylex DAC960 V1 (`myrb`) | Low priority — DCDB passthrough; **12-byte CDB limit** (no ATA16!); very old hardware | Unlikely | Unlikely | Unlikely | Driver likely not available for Windows; hardware end-of-life | Unlikely |
| Mylex DAC960 V2 (`myrs`) | Low priority — 16-byte DCDB; same old hardware family | Unlikely | Unlikely | Unlikely | Driver not available; end-of-life | Unlikely |
| PMC MaxRAID (`pmcraid`) | Low priority — IBM POWER-centric; `/dev/pmcsas`; GSCSI resource type for physical drives | Unlikely | Unlikely | Unlikely | Unlikely (IBM Power hardware) | Unlikely |
| IBM IPR (`ipr`) | Very low priority — IBM POWER-only | No | No | No | No | No |
| IBM ServeRAID (`ips`) | **Do not implement** — driver removed from kernel; hardware end-of-life | No | No | No | No | No |
| Intel VROC | No | No | No | No | Yes (NVMe RAID via VROC IOCTL) | No |
| AMD RAIDCORE (`rcraid`) | Planned (CSMI-adjacent) | No | No | No | Yes (Windows driver; proprietary IOCTL; legal barriers remain) | No |
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
