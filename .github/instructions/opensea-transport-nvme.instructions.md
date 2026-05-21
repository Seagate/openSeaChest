---
description: 'NVMe command building patterns for opensea-transport — nvmeCmdCtx, admin vs. I/O queue commands, completion status, and NVMe-specific conventions'
applyTo: 'subprojects/opensea-transport/**/*.c, subprojects/opensea-transport/**/*.h'
---

# NVMe Command Building — opensea-transport

## Relevant Files

| File | Purpose |
|------|---------|
| `include/nvme_helper.h` | Controller registers, identify structures, SMART log, `nvmeCmdCtx`, `nvmeCommands`, `eNvmeCmdType`, status-field helpers |
| `include/nvme_helper_func.h` | Public NVMe command dispatch functions |
| `src/nvme_cmds.c` | NVMe command implementations |
| `src/nvme_helper.c` | Status parsing, verbose output, misc helpers |
| `include/of_nvme_helper.h` / `src/of_nvme_helper.c` | OpenFirmware NVMe passthrough |
| `include/jmicron_nvme_helper.h` | JMicron USB-to-NVMe bridge passthrough |
| `include/asmedia_nvme_helper.h` | ASMedia USB-to-NVMe bridge passthrough |
| `include/realtek_nvme_helper.h` | Realtek USB-to-NVMe bridge passthrough |
| `src/sntl_helper.c` | SCSI-to-NVMe Translation (SNTL) — translates incoming SCSI commands to NVMe equivalents on all platforms except Windows (which has its own built-in translation layer). This is a useful reference for SCSI-to-NVMe command equivalence, but it is **not a complete SNTL**. Not all possible SCSI-to-NVMe translations have been implemented. |

---

## NVMe Overview and Design Philosophy

### Origins and Design Goals

NVMe (Non-Volatile Memory Express) emerged in the 2010s as a command interface purpose-built for PCIe-attached non-volatile storage. ATA and SCSI both trace their origins to the 1980s and were designed around synchronous, single-queue models on shared buses. Asynchronous operation was shoehorned into both later through Native Command Queuing (NCQ/ATA) and Command Task Queuing (SCSI), carrying significant scheduling complexity. Commands that cannot be queued still exist in both protocols today.

NVMe was explicitly designed to address the limitations of AHCI (Advanced Host Controller Interface), the Intel-developed driver-level interface specification for SATA HBAs. The original design goals relative to AHCI were:

- Remove uncacheable reads from the command issue and completion path
- Minimize MMIO (memory-mapped I/O) writes in the command issue and completion path
- Support deep command queues and simplify command decoding and processing
- Support MSI-X and flexible interrupt aggregation
- Support for many-core systems (NUMA-aware, per-core queues with no cross-core locking)
- Support enterprise features
- Comprehensive statistics, health status reporting, and robust error reporting and handling

> **SATA vs. AHCI — a common confusion**: SATA and AHCI are frequently treated as synonymous, but they are distinct specifications at different layers. SATA (maintained by SATA-IO) defines the physical transport and electrical interface. AHCI (developed by Intel) defines how a SATA HBA exposes itself to a driver. ATA/ACS (maintained by T13) defines the device command protocol. A SATA drive involves all three. The "Why NVMe?" answer is therefore a replacement of all three simultaneously: PCIe replaces SATA as the transport, the NVMe controller register interface replaces AHCI as the host controller model, and the NVMe command set replaces ATA/ACS as the device protocol. When the NVMe design goals above reference AHCI, they are specifically targeting the host controller interface layer — but the full motivation encompasses the entire SATA/AHCI/ATA stack.

NVMe attaches directly to PCIe, bypassing the HBA model of SATA/SAS. In practice, many systems still use storage controllers and enclosures, but the command protocol itself has no intermediary bus abstraction.

### NVMe Queue Architecture

NVMe queues are **messaging queues, not command queues** — an important conceptual distinction from SCSI and ATA. They are circular, fixed-element-size queues implemented in PCIe memory (typically host DRAM), each consisting of a contiguous physical memory block or an optional non-contiguous PRP (Physical Region Page) list.

**Queue types:**

| Type | Count per controller | Max depth | Purpose |
|------|---------------------|-----------|--------|
| Admin Submission Queue (ASQ) | 1 | Up to 4096 entries | Create/delete I/O queues; controller and feature management |
| Admin Completion Queue (ACQ) | 1 | Up to 4096 entries | Receive admin command completions |
| I/O Submission Queue (SQ) | Up to ~64K | Up to ~64K entries | Submit I/O commands to the controller |
| I/O Completion Queue (CQ) | Up to ~64K | Up to ~64K entries | Receive I/O command completions from the controller |

Queue size (usable entries) = declared size − 1; minimum 2. A queue is **empty** when Head == Tail; **full** when Head == (Tail + 1) mod queue size.

**SQ/CQ relationship**: Each SQ maps to exactly one CQ, defined at SQ creation time. Multiple SQs may map to a single CQ (n:1). This enables NUMA-optimized layouts where each CPU core has one or more SQs feeding a per-core CQ and MSI-X interrupt with no cross-core locking.

**Command execution flow (8 steps via PCIe TLPs):**
1. Host writes command entry into the SQ
2. Host rings the SQ doorbell (writes new tail pointer via MMIO)
3. Controller fetches the command(s) from host memory
4. Controller processes the command(s)
5. Controller writes completion entry into the CQ
6. Controller generates an MSI-X interrupt
7. Host processes the completion entry
8. Host rings the CQ doorbell (writes new head pointer via MMIO)

**Command arbitration**: All controllers support round-robin arbitration across submission queues. Controllers may optionally support weighted round-robin (WRR) with an urgent priority class. WRR uses 8-bit weights and configurable arbitration bursts (1, 2, 4, 8, 16, 32, 64, or no limit).

**NVMe Subsystem and controller model**: An NVMe Subsystem consists of one or more controllers, one or more namespaces, one or more PCIe ports, and the non-volatile storage medium. Multiple controllers in a subsystem can share namespaces. The same physical namespace may have a different NSID in each controller that exposes it. Multi-path configurations (multiple controllers and/or PCIe ports reaching the same storage) are supported by the architecture.

The controller reports the number of I/O queues it supports and the maximum depth via Identify Controller fields: `MQES` (Maximum Queue Entries Supported) and the allocated queue counts. Verify exact field names and limits against the current NVMe base specification before implementing queue management.

### NVMe as a Hybrid of ATA and SCSI

NVMe draws from both of its predecessors:

- **Feature detection**: Get Features / Set Features uses explicit bitfields for current, default, saved, and changeable values — the same four selection values as SCSI mode pages. There is no SCSI-style “just try it” probing; features are discovered via Identify Controller fields.
- **Command style**: Commands resemble ATA’s register-based model more than SCSI’s CDB model.
- **Error reporting**: Far richer than ATA, and roughly comparable to SCSI, but better organized. Error status codes are grouped into typed categories with specific meanings, rather than ATA’s flat error register or SCSI’s sense key / ASC/ASCQ lookup.
- **Transfer direction**: NVMe opcodes encode the data transfer direction directly. In SCSI and ATA the direction is more implicit and can be ambiguous for certain commands.
- **Data size granularity**: NVMe does not require all transfers to be multiples of 512 bytes. The minimum granularity is a DWORD (4 bytes), which is much more flexible. However, all DWORDs fields in the spec are still little-endian, as in ATA.
- **128-bit fields**: NVMe uses 128-bit fields in more places than ATA or SCSI, though they are still not common. Work carefully when parsing them.

### What NVMe Changed

- **Zero transfer length**: NVMe has no ambiguous zero. A transfer length of 0 in the raw NLB field means exactly 1 logical block. There is no ATA-style “0 = maximum” and no SCSI-style “0 = do nothing”. See [Transfer Length Conventions](#transfer-length-conventions).
- **SMART is standardized**: The SMART/Health Information Log (log ID 0x02) has all fields fully defined by the specification. There are no vendor-unique fields within the standardized log entries (unlike ATA SMART attributes).
- **Storage medium**: NVMe 1.x through NVMe 2.0 technically supports any non-volatile storage medium, but all early devices and most current devices are NAND SSDs. NVMe 2.0 explicitly adds support for rotational media (HDDs).
- **Host RAM access**: Some NVMe features directly access host DRAM — the Persistent Memory Region and Host Memory Buffer features. This affects how certain operations (e.g., Sanitize) must behave when host memory is involved, and can be surprising to programmers familiar only with ATA/SCSI semantics.
- **Kernel enforcement**: Because NVMe attaches directly to PCIe, kernels are more protective about passthrough operations than they are for SCSI/ATA. Windows in particular enforces restrictions on which commands can be sent through user-space passthrough IOCTLs.

### NVMe 2.0 and Zoned Namespaces (ZNS)

NVMe 2.0 introduced the Zoned Namespace (ZNS) command set, which is the NVMe equivalent of ZBC (SCSI Zoned Block Commands) and ZAC (ATA Zoned Device ATA Commands). ZNS support has **not yet been implemented** in this codebase. Before adding ZNS support, review the NVMe 2.0 base specification and the ZNS command set addendum alongside the existing `openSeaChest_ZBD` utility (which currently handles ZBC/ZAC).

### NVMe Key-Value Command Set

NVMe 2.0 also introduced a Key-Value (KV) command set, which allows the host to store and retrieve data by an opaque key rather than by LBA. This is conceptually related to object storage — accessing named objects rather than fixed-sector addresses. SCSI has a comparable T10 standard called OSD (Object-Based Storage Device commands), and Seagate’s Kinetic drives deployed a proprietary key-value protocol over Ethernet that pursued a similar concept at the application level. Neither the Kinetic approach nor SCSI OSD achieved wide industry adoption.

**NVMe KV support is not implemented in this codebase.** If KV support is needed in the future, study the NVMe KV command set specification and verify OS passthrough support, as KV namespaces require a different access model from traditional block namespaces.

### Fused Commands

NVMe defines fused operations — pairs of commands that execute atomically (e.g., Compare and Write). To the best of current knowledge, **passthrough fused commands are not supported** through any of the OS passthrough interfaces used in this codebase. This must be investigated before any fused command implementation is attempted.

### NVMe Fabrics

NVMe Fabrics (NVMe-oF) is the network transport extension of NVMe, allowing NVMe commands to be carried over network fabrics such as RDMA (RoCE, iWARP), Fibre Channel, and TCP. It is the rough functional equivalent of SCSI Enclosure Services’ role in tying together distributed storage infrastructure — though the two are not directly equivalent in architecture or scope.

**NVMe Fabrics is not implemented in this codebase.** Implementing fabric discovery, connection management, and fabric-specific command routing would be a significant capability addition. Before attempting any Fabrics work, study the NVMe-oF specification alongside the host-side fabric driver interfaces (RDMA, FC-NVMe, TCP) for each target OS.
### Historical Context: SCSI-to-NVMe Translation

SCSI-to-NVMe translation originated as a white paper published by the NVM Express organization (nvmexpress.org). It was written as a transition aid for OS driver layers that needed to communicate with NVMe devices before native NVMe drivers were widely available — the idea being that an existing SCSI driver stack could be reused by inserting a translation shim below it.

The OS ecosystem adopted native NVMe drivers far more quickly than anticipated, making the white paper largely obsolete as a practical driver strategy before it saw wide deployment. Nevertheless, it established the conceptual mapping between SCSI commands and NVMe equivalents that still underlies most translation implementations today — including `sntl_helper.c` in this codebase.

The white paper also appears to be an influencing factor in the formal **SNT (SCSI-NVMe Translation)** standard that T10 is currently developing, which would standardize the translation at the committee level.

### OCP (Open Compute Project) Requirements

The Open Compute Project (OCP) is a consortium of hyperscale cloud operators and storage vendors (Meta, Microsoft, Google, and others) that publishes specification requirements for data center hardware. OCP specifications do not replace or supersede the NVMe, ATA, or SCSI standards — they layer additional mandatory requirements on top of them for devices intended for cloud data center use.

For **NVMe cloud SSDs**, OCP defines:
- Extended telemetry and health reporting well beyond what the SMART/Health log (log ID 0x02) provides — including latency histograms, error injection logs, and various per-device statistics
- Specific features from the NVMe standard that must be implemented (not merely optional)
- Additional vendor-defined log pages and Identify fields that OCP-compliant devices must expose

OCP also publishes specifications for SATA and SAS drives with equivalent requirements for those interfaces, though the NVMe cloud SSD specification is the most active and most extensive.

**Generic OCP support is not yet implemented in this codebase.** The goal is to add autodiscovered OCP support across all applicable interfaces — detecting OCP-compliant devices via their VPD or log page declarations and exposing the additional capabilities they provide. This is a known improvement area.

## The `nvmeCmdCtx` Structure

All NVMe commands go through `nvme_Cmd`. Fill `nvmeCmdCtx` and pass it in:

```c
nvmeCmdCtx cmdCtx;
safe_memset(&cmdCtx, sizeof(cmdCtx), 0, sizeof(cmdCtx));

cmdCtx.device         = device;
cmdCtx.commandType    = NVM_ADMIN_CMD;       // NVM_ADMIN_CMD or NVM_CMD (I/O queue)
cmdCtx.commandDirection = XFER_DATA_IN;      // or XFER_DATA_OUT, XFER_NO_DATA
cmdCtx.timeout        = 15;                  // seconds

// Fill the specific command union:
cmdCtx.cmd.adminCmd.opcode = NVME_ADMIN_IDENTIFY;
cmdCtx.cmd.adminCmd.nsid   = 0;             // namespace ID (0 for controller identify)
cmdCtx.cmd.adminCmd.cdw10  = 1;             // CNS = 1 → Identify Controller

cmdCtx.ptrData  = identifyBuf;
cmdCtx.dataSize = NVME_IDENTIFY_DATA_LEN;   // 4096 bytes

eReturnValues ret = nvme_Cmd(device, &cmdCtx);
```

---

## Admin vs. I/O Queue Commands

NVMe has two command types that go to different submission queues:

| `eNvmeCmdType` | Constant | Destination | Examples |
|----------------|---------|------------|---------|
| Admin | `NVM_ADMIN_CMD` | Admin Submission Queue | Identify, Get/Set Features, Get Log Page, Format NVM, Firmware Image Download, Sanitize |
| I/O (NVM) | `NVM_CMD` | I/O Submission Queue | Read, Write, Dataset Management (TRIM), Write Uncorrectable, Compare, Verify |

Always set `commandType` correctly — the OS passthrough layer uses it to route to the right queue. On Windows, routing to the wrong queue causes `IOCTL_STORAGE_PROTOCOL_COMMAND` to fail.

The third value `NVM_UNKNOWN_CMD_SET` exists for future NVMe command set extensions. Use `NVM_ADMIN_CMD` or `NVM_CMD` for all currently defined commands.

---

## Command DWORDs

NVMe commands are 64-byte Submission Queue Entries with 16 DWORDs (CDW0–CDW15). The union `nvmeCommands` provides typed sub-structures for admin and I/O commands. Key fields:

```c
// Admin command fields (cmdCtx.cmd.adminCmd):
cmdCtx.cmd.adminCmd.opcode;  // CDW0[7:0]
cmdCtx.cmd.adminCmd.nsid;    // CDW1: namespace ID
cmdCtx.cmd.adminCmd.cdw10;   // CDW10: command-specific
cmdCtx.cmd.adminCmd.cdw11;   // CDW11: command-specific
cmdCtx.cmd.adminCmd.cdw12;   // CDW12: command-specific
cmdCtx.cmd.adminCmd.cdw13;   // CDW13: command-specific
cmdCtx.cmd.adminCmd.cdw14;   // CDW14: command-specific
cmdCtx.cmd.adminCmd.cdw15;   // CDW15: command-specific

// I/O command fields (cmdCtx.cmd.nvmCmd):
cmdCtx.cmd.nvmCmd.opcode;
cmdCtx.cmd.nvmCmd.nsid;      // namespace ID (M_NULLPTR = 0xFFFFFFFF for all namespaces)
cmdCtx.cmd.nvmCmd.slba;      // CDW10+CDW11: Starting LBA (64-bit)
cmdCtx.cmd.nvmCmd.nlb;       // CDW12[15:0]: Number of Logical Blocks (0's based)
```

Use the `NVME_0_BASED(n)` macro when converting a count to a 0's-based NVMe value, and `NVME_0_BASED_ADJUST(n)` for the reverse:

```c
// NVMe Get Log Page: number of dwords is 0's based
cmdCtx.cmd.adminCmd.cdw10 = M_STATIC_CAST(uint32_t, NVME_0_BASED_ADJUST(numDwords) & UINT32_C(0x0FFF));
```

---

## Completion Queue Entry

After `nvme_Cmd` returns, `cmdCtx.commandCompletionData` holds the Completion Queue Entry:

```c
// Check command status
uint32_t statusDWord = cmdCtx.commandCompletionData.statusAndCID;

bool    doNotRetry   = false;
bool    more         = false;
uint8_t statusType   = 0;
uint8_t statusCode   = 0;
get_NVMe_Status_Fields_From_DWord(statusDWord, &doNotRetry, &more, &statusType, &statusCode);

// Or use the simplified check:
eReturnValues ret = check_NVMe_Status(statusDWord);
```

`check_NVMe_Status` maps NVMe status codes to `eReturnValues`:
- Status 0 (Success) → `SUCCESS`
- Invalid Command Opcode / Invalid Field → `NOT_SUPPORTED`
- Command Abort → `ABORTED`
- Other errors → `FAILURE`

---

## Common Admin Commands

Use the existing wrapper functions in `nvme_helper_func.h` instead of building raw `nvmeCmdCtx` structures for standard commands:

```c
// Get Log Page (e.g., SMART Health Information Log)
nvmeGetLogPageCmdOpts logOpts;
safe_memset(&logOpts, sizeof(logOpts), 0, sizeof(logOpts));
logOpts.nsid    = NVME_ALL_NAMESPACES;    // 0xFFFFFFFF
logOpts.lid     = 0x02;                  // SMART / Health Log
logOpts.addr    = logBuf;
logOpts.dataLen = NVME_SMART_HEALTH_LOG_LEN;

eReturnValues ret = nvme_Get_Log_Page(device, &logOpts);

// Get Features
nvmeFeaturesCmdOpt featOpts;
safe_memset(&featOpts, sizeof(featOpts), 0, sizeof(featOpts));
featOpts.nsid    = 0;
featOpts.fid     = NVME_FEAT_VOLATILE_WC_;  // Volatile Write Cache feature ID
featOpts.sel     = 0;                      // Current value
ret = nvme_Get_Features(device, &featOpts);

// Identify Controller
uint8_t idData[NVME_IDENTIFY_DATA_LEN];
safe_memset(idData, sizeof(idData), 0, sizeof(idData));
ret = nvme_Identify(device, idData, 0, NVME_IDENTIFY_CTRL);
```

---

## Namespace IDs

- `0` — reserved / invalid in most admin commands (means "controller scope" for Identify Namespace, but check the spec)
- `1` — first namespace (NVMe starts namespaces at 1, not 0)
- `NVME_ALL_NAMESPACES` (`0xFFFFFFFF`) — applies to all namespaces (Get Log Page, Sanitize, etc.)

Check whether a command operates at controller scope or namespace scope before setting `nsid`.

---

## Verbose Output for NVMe Commands

```c
// Print command before sending:
print_tDevice_Verbose_NVMe_Cmd(device, VERBOSITY_COMMAND_VERBOSE, &cmdCtx);

// After the call, print completion:
print_tDevice_Verbose_NVMe_Cmd_Result(device, VERBOSITY_COMMAND_VERBOSE, &cmdCtx);

// Flush:
flush_tDevice_Verbose_Stream(device);
```

---

## Transfer Length Conventions

NVMe eliminates the ambiguous-zero problem that exists in both ATA and SCSI:

| Protocol | Transfer length = 0 meaning |
|----------|-----------------------------|
| SCSI (10/12/16-byte) | No data transferred (used for probing) |
| ATA (28-bit) | Transfer 256 sectors (maximum for the field) |
| ATA (48-bit) | Transfer 65536 sectors (maximum for the field) |
| **NVMe** | **Transfer exactly 1 logical block** (raw field value is 0's based; add 1 to get count) |

The `nlb` (Number of Logical Blocks) field in NVMe I/O commands is 0's based: a raw value of 0 = 1 block, 1 = 2 blocks, etc. Always use `NVME_0_BASED(n)` to encode and `NVME_0_BASED_ADJUST(n)` to decode. There is no special case for zero.

---

## Feature Detection and Set Features

NVMe does not have a SCSI-style “just try it” probing model. Feature support is declared explicitly in the Identify Controller data structure (4096 bytes, retrieved with `nvme_Identify`). Check the relevant byte/bit in the Identify data before attempting to use a feature.

Get Features and Set Features (`nvme_Get_Features` / `nvme_Set_Features`) use a **sel** (select) field that maps directly to the four SCSI mode page selection values:

| sel value | Meaning |
|-----------|--------|
| 0 | Current value |
| 1 | Default value |
| 2 | Saved value |
| 3 | Supported capabilities |

For most features, the value format uses bitfields similar to ATA IDENTIFY DEVICE fields. Unlike SCSI mode pages, there is no concept of reading the full page and writing it back — each feature ID is set independently.

---

## SMART / Health Information Log

NVMe does not have a separate SMART feature like ATA. Instead, the SMART/Health Information Log (log ID 0x02) is a standardized, always-present log page. All fields are defined by the NVMe specification — there are no vendor-unique attributes or opaque counters as there are in ATA SMART.

The log is 512 bytes and is retrieved via `nvme_Get_Log_Page` with `logOpts.lid = 0x02`. Key fields include critical warning bits, composite temperature, available spare, percentage used, and media / data integrity error counts.

---

## Block Size and Format

NVMe reports a single logical block size per namespace, selected at format time via Format NVM. Unlike ATA and SCSI, there is no separate physical block size concept at the command interface level. Instead, NVMe provides additional fields for alignment, optimal transfer size, and granularity guidance, allowing the device to communicate its performance characteristics without conflating logical and physical block size.

Supported LBA format configurations are listed in the Identify Namespace data structure (`nvme_Identify` with CNS = 0, namespace ID = target namespace). The formatted LBA size field indicates which format is currently active.

---

## Security Protocol

NVMe’s security command set (SECURITY SEND / SECURITY RECEIVE) references the SCSI standard for protocol definitions. The list of security protocol codes and all discovery/communication with TCG-compliant drives (Opal, Enterprise, Ruby, Pyrite, Opalite) use big-endian byte ordering in their payloads, even though NVMe itself is little-endian. Parse security protocol payloads using big-endian accessors.

Some NVMe devices implement ATA-style password security via SAT (SCSI ATA Translation) security protocol mapping. This is **not standardized** in the NVMe specification — the NVMe spec refers all security features to TCG documentation. Treat SAT-via-NVMe ATA security as a vendor-specific extension.

---

## Platform Availability

NVMe passthrough availability varies significantly by OS:

| OS | Admin commands | I/O commands | Notes |
|----|---------------|-------------|-------|
| Windows 10+ | `STORAGE_PROTOCOL_COMMAND` | `STORAGE_PROTOCOL_COMMAND` | Full admin + I/O; restrictions apply (see below) |
| Windows 8.1 / 7 | Vendor IOCTLs | Not standard | Limited; vendor-specific only |
| Linux | Kernel NVMe IOCTL | IOCTL or io_uring | Full access via `/dev/nvme*` |
| FreeBSD | nvme IOCTL | Not directly | Admin only |
| Other BSDs | Varies | Varies | Check individual OS helper files |
| VMware | vm_nvme kernel module | via module | Custom IOCTLs |

### Windows 10 Passthrough Restrictions

Windows 10 applies significant restrictions to NVMe passthrough:

- **Commands Supported and Effects log**: Before Windows will allow a vendor-unique admin command through `STORAGE_PROTOCOL_COMMAND`, the device must report that command in its Commands Supported and Effects log (log ID 0x05). Without this declaration, Windows blocks the command.
- **Security commands**: Windows does not pass SECURITY SEND / SECURITY RECEIVE directly through `STORAGE_PROTOCOL_COMMAND`. Instead, these must be issued as SCSI SECURITY PROTOCOL IN / SECURITY PROTOCOL OUT commands (using the SCSI passthrough IOCTL), which the Windows storage stack then re-translates to NVMe SECURITY SEND / RECEIVE internally.
- **Reservations**: NVMe reservation commands are only beginning to be allowed on Windows 11. As of the current codebase, full reservation passthrough support on Windows is not complete.

When adding an NVMe command, always check `device->drive_info.passThroughHacks.nvmePTHacks` and `device->drive_info.adapter_info` for platform-specific limitations. Return `NOT_SUPPORTED` gracefully if the OS passthrough cannot route the command.

---

## USB-to-NVMe Bridges

Some USB docks expose NVMe drives through vendor-specific tunneling protocols:

| Vendor | Helper |
|--------|--------|
| JMicron | `jmicron_nvme_helper.*` |
| ASMedia | `asmedia_nvme_helper.*` |
| Realtek | `realtek_nvme_helper.*` |

These bridges only support a subset of NVMe admin commands (typically Identify and Get Log Page). Do not assume a USB-connected NVMe device can receive any arbitrary admin command — these bridges may silently drop or corrupt unsupported commands.

---

## Common Pitfalls

- **All data sizes in `nvmeCmdCtx.dataSize` are in bytes.** Many NVMe spec fields are in DWORDs (4 bytes). The `nvme_Get_Log_Page` function internally converts to DWORDs when building the CDB.
- **NVMe LBA counts are 0's based.** A `nlb` value of 0 reads one block, not zero blocks and not the maximum. Use `NVME_0_BASED_ADJUST` when converting a user-facing block count. See [Transfer Length Conventions](#transfer-length-conventions).
- **`NVME_ALL_NAMESPACES` (0xFFFFFFFF) is correct for broadcast.** Some operations use 0 for “applies to controller only” which is distinct from “applies to all namespaces.”
- **Firmware slot numbering starts at 1**, not 0. `NVME_MAX_FW_SLOTS` is 7.
- **Do not access the controller BAR registers directly** (`nvmeBarCtrlRegisters`). These are only used by the kernel-mode driver. User-space passthrough IOCTLs do not expose register-level access.
- **Host Memory Buffer and Persistent Memory Region**: Some NVMe devices can directly access host DRAM through these features. This changes the expected behavior of commands like Sanitize (which must account for whether host-accessible memory regions need to be cleared). Do not assume ATA/SCSI sanitize semantics apply directly.
- **Security payload byte order**: NVMe is little-endian throughout, but SECURITY SEND / RECEIVE payloads follow SCSI protocol conventions, which are big-endian. Use big-endian accessors when parsing discovery or TCG communication packets.
- **SNTL coverage is incomplete**: `sntl_helper.c` does not translate every possible SCSI command to NVMe. Verify that the specific SCSI command you need is actually handled before assuming it will work through the software translation layer.
