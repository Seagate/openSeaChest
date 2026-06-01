---
description: 'Doxygen documentation conventions for openSeaChest — comment style, tags, structure, grouping, and Doxyfile settings'
applyTo: '**/*.c, **/*.h, **/*.cpp, **/*.hpp, **/*.cc, **/*.cxx'
---

# Doxygen Documentation Conventions

## Philosophy

Documentation is part of the code, not an afterthought. The goal is for someone reading a header for the first time to understand:

1. **What** the function does (the brief line)
2. **Why** they might call it and what to watch out for
3. **How** to call it correctly (parameter types, direction, preconditions)
4. **What comes back** (every distinct return code)
5. **A working example** they can copy directly

Use `\` (backslash) for all Doxygen commands, not `@`. The project standardises on backslash throughout.

---

## Comment Style

### Use `//!` throughout

`//!` is the Qt-style single-line Doxygen comment. It is compatible with all compilers the project supports (C++ line comments are universally accepted), works with clang-format without needing `clang-format off/on` guards, and is the style already established in opensea-common.

```c
//! \brief Allocates memory with bounds checking.
//!
//! This function allocates \a size bytes of memory, guarding against zero-size
//! allocations which have implementation-defined behaviour.
//!
//! \param[in] size Number of bytes to allocate. Must be > 0.
//! \return Pointer to the allocated block, or \c M_NULLPTR on failure.
void* safe_malloc(size_t size);
```

Blank `//!` lines separate logical paragraphs inside a comment block. Always add one between the description and the first tag, and between tag groups.

### Trailing member documentation — `//!<`

For struct, union, and enum members, place the comment **after** the member on the same line using `//!<`. This is the only correct way to attach documentation to a member without Doxygen misattributing it:

```c
typedef struct
{
    uint32_t startLBA;    //!< Starting logical block address (host byte order after conversion)
    uint32_t blockCount;  //!< Number of blocks to transfer
    uint8_t  direction;   //!< Transfer direction: \c XFER_DATA_IN or \c XFER_DATA_OUT
    bool     isValid;     //!< Set to \c true when the structure has been fully initialised
} tMyCommand;
```

### Do not use `/** */` or `/*! */` blocks

Mixing styles causes inconsistency and makes search/replace harder during mass documentation updates. Use `//!` exclusively. For multi-line detail sections just continue with `//!` lines.

### Abbreviations, acronyms, and storage-industry terms

The storage industry uses a large number of acronyms (SMART, DST, LBA, AMAC, SED, TCG, etc.). Readers — including AI models working with this code in future sessions — cannot safely infer their meaning from context alone.

**Rule: spell out every abbreviation the first time it appears in each Doxygen block** (file block, function block, struct block, enum block). After the first definition in that block the short form may be used freely. Format: **Full Form (ABBR)** on first use.

```c
//! \brief Returns the Accessible Max Address Configuration (AMAC) setting.
//!
//! AMAC defines the highest Logical Block Address (LBA) the host may access on the device.
//! Query AMAC before and after a Set Max Address command to verify the change took effect.
```

Common storage-industry abbreviations that must be expanded on first use per block:

| Abbreviation | Full form |
|-------------|-----------|
| LBA | Logical Block Address |
| CDB | Command Descriptor Block |
| SMART | Self-Monitoring, Analysis and Reporting Technology |
| DST | Drive Self-Test |
| AMAC | Accessible Max Address Configuration |
| HPA | Host Protected Area |
| SED | Self-Encrypting Drive |
| TCG | Trusted Computing Group |
| ATA | AT Attachment |
| SATA | Serial ATA (Serial AT Attachment) |
| SCSI | Small Computer System Interface |
| SAS | Serial Attached SCSI (Serial Attached Small Computer System Interface) |
| NVMe | Non-Volatile Memory Express |
| USB | Universal Serial Bus |
| 512n | 512-byte native sector size |
| 512e | 512-byte emulated (4096-byte physical, 512-byte logical sector) |
| 4Kn | 4K native sector size (4096-byte logical block) |
| ZBD | Zoned Block Device |
| HDD | Hard Disk Drive |
| SSD | Solid-State Drive |
| FPDMA | First Party DMA (Native Command Queuing transport) |
| NCQ | Native Command Queuing |
| PUIS | Power-Up In Standby |
| EPC | Extended Power Conditions |
| DCO | Device Configuration Overlay |

This list is not exhaustive. When in doubt, spell it out. Acronyms that appear in a function or type name itself (e.g., `get_AMAC_value`) still require expansion in the `\brief` line.

### Storage sanitization terminology (IEEE 2883-2022)

When documenting any function, command, enum value, or operation that erases, sanitizes, or overwrites user data on storage media, use the IEEE 2883-2022 sanitization level terms precisely. The project tracks four levels internally via `ERASE_SANITIZATION_*` enum values. Use the corresponding term in `\brief`, `\details`, and `\warning` text whenever the level is known. The definitions below are the authoritative project definitions (from `print_Supported_Erase_Methods` in `operations.c`):

| Internal constant | Term to use in docs | Definition |
|------------------|---------------------|------------|
| `ERASE_SANITIZATION_CLEAR` | **clear** | Logical techniques are applied to all addressable storage locations, protecting against simple, non-invasive data recovery techniques. |
| `ERASE_SANITIZATION_POSSIBLE_PURGE` | **clear, possible purge** | Cryptographic erase is a purge if the vendor implementation meets the requirements in IEEE 2883-2022. |
| `ERASE_SANITIZATION_PURGE` | **purge** | Logical techniques that target user data, overprovisioning, unused space, and bad blocks rendering data recovery infeasible even with state-of-the-art laboratory techniques. |
| `ERASE_SANITIZATION_UNKNOWN` | unknown sanitization level | The sanitization level cannot be determined from available information. |

**Recommendation — include in `\attention` for any destructive storage operation:**

> Restore the device's maximum Logical Block Address (LBA) prior to any erase to ensure all user-addressable sectors are included. For ATA devices, restore the Host Protected Area (HPA) and Device Configuration Overlay (DCO) / Accessible Max Address Configuration (AMAC) before erasing. Restoring the MaxLBA also allows full post-erase verification without a lower MaxLBA masking unverified sectors.

**Common operations and their levels:**

| Operation | Level |
|-----------|-------|
| ATA Security Erase Unit — normal mode | clear |
| ATA Security Erase Unit — enhanced mode | purge |
| ATA Sanitize — Overwrite | purge |
| ATA Sanitize — Block Erase | purge |
| ATA Sanitize — Cryptographic Scramble | purge |
| NVMe Sanitize — Overwrite | purge |
| NVMe Sanitize — Block Erase | purge |
| NVMe Sanitize — Crypto Erase | purge |
| SCSI Sanitize — Overwrite | purge |
| SCSI Sanitize — Block Erase | purge |
| SCSI Sanitize — Cryptographic Erase | purge |
| TCG Revert / Revert SP — encrypting SSCs (Enterprise, Opal, Opalite, Ruby) | purge |
| TCG Revert / Revert SP — Pyrite SSC (no encryption) | clear |
| NVMe Format NVM — Cryptographic Secure Erase | clear, possible purge |
| NVMe Format NVM — User Data Erase | clear |
| Write Same / Write all LBAs | clear |
| Overwrite (software write to all LBAs) | clear |
| SCSI Format Unit — native SCSI/SAS | clear |
| SCSI Format Unit — SAT-translated device (USB, SAS-to-SATA bridges, etc.) | translator-dependent; may be a no-op |

**In `\brief`:** name the level:
- `\brief Performs a **clear** of the host-addressable Logical Block Address (LBA) space.`
- `\brief Performs a **purge** — all user data including reallocated sectors and reserved areas is rendered irrecoverable.`
- `\brief Performs a cryptographic erase (**clear, possible purge**) — classified as a purge only if the vendor implementation meets IEEE 2883-2022.`

**In `\warning`:** use the project definitions verbatim or rephrase closely. When a single function can perform multiple levels depending on a parameter, add one `\warning` per level:

```c
//! \warning (Normal erase — IEEE 2883 clear): Logical techniques are applied to all
//!          addressable storage locations. Simple non-invasive data recovery is prevented.
//!          This operation is irreversible. Obtain explicit user confirmation before invoking.
//! \warning (Enhanced erase — IEEE 2883 purge): Logical techniques target user data,
//!          overprovisioning, unused space, and bad blocks. Data recovery is infeasible
//!          even with state-of-the-art laboratory techniques. This operation is irreversible.
//!          Obtain explicit user confirmation before invoking.
```

---

## File-Level Block

Every `.h` and `.c` file must begin with a file block immediately after the SPDX comment. Use the exact copyright text from the existing file. Do not invent or abbreviate it.

```c
// SPDX-License-Identifier: MPL-2.0

//! \file my_module.h
//! \brief One-line summary of what this file provides.
//!
//! Longer description if needed: what subsystem this belongs to, what
//! problem it solves, and any important usage constraints.
//!
//! \copyright
//! Do NOT modify or remove this copyright and license
//!
//! Copyright (c) 2024-2026 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//!
//! This software is subject to the terms of the Mozilla Public License, v. 2.0.
//! If a copy of the MPL was not distributed with this file, You can obtain one at
//! http://mozilla.org/MPL/2.0/.
```

For `.c` implementation files, the file block is shorter — a brief summary and the copyright. Document the public API in the `.h` file, not the `.c` file.

---

## Function Documentation — Complete Template

Place the comment block **directly above** the function declaration in the header file. The `\fn` tag is optional when the comment is directly attached to the declaration (Doxygen auto-attaches), but include it for clarity and for `static M_INLINE` functions where the full signature makes intent unambiguous.

```c
//! \fn eReturnValues perform_ata_security_erase(tDevice* device, eAtaSecurityEraseType eraseType, const char* password)
//! \brief Performs an ATA Security Erase Unit command (IEEE 2883-2022 clear or purge
//!   depending on \a eraseType).
//!
//! \details Issues the ATA Security Erase Unit command sequence. The sanitization
//!   level depends on \a eraseType:
//!   - \c ATA_SECURITY_ERASE_NORMAL (IEEE 2883 **clear**): logical techniques applied to
//!     all host-addressable Logical Block Addresses (LBAs), protecting against simple,
//!     non-invasive data recovery techniques. Reallocated sectors and reserved areas are
//!     not guaranteed to be overwritten. On most drives this takes hours.
//!   - \c ATA_SECURITY_ERASE_ENHANCED (IEEE 2883 **purge**): logical techniques target
//!     user data, overprovisioning, unused space, and bad blocks, rendering data recovery
//!     infeasible even with state-of-the-art laboratory techniques. On Self-Encrypting
//!     Drives (SEDs) this is often implemented as a cryptographic erase and may be
//!     near-instant.
//!
//! \attention Restore the device's maximum LBA prior to erasing to ensure all
//!   user-addressable sectors are included. For ATA devices, restore the Host Protected
//!   Area (HPA) and Device Configuration Overlay (DCO) / Accessible Max Address
//!   Configuration (AMAC) before issuing this command. Restoring the MaxLBA also allows
//!   full post-erase verification without a lower MaxLBA masking unverified sectors.
//!
//! \param[in] device      Pointer to an initialised and opened \c tDevice structure.
//! \param[in] eraseType   Selects normal (\c ATA_SECURITY_ERASE_NORMAL, IEEE 2883 clear)
//!                        or enhanced (\c ATA_SECURITY_ERASE_ENHANCED, IEEE 2883 purge)
//!                        erase mode.
//! \param[in] password    Null-terminated ASCII password string. Maximum 32 bytes
//!                        (bytes 33–512 are ignored by the drive). Use \c M_NULLPTR
//!                        to attempt with an empty password.
//!
//! \pre  The ATA Security Feature Set must be enabled on the device (check
//!       \c device->drive_info.ata_Options.securitySupported and
//!       \c device->drive_info.ata_Options.securityEnabled before calling).
//! \pre  The caller must have issued \c ata_Security_Erase_Prepare() immediately
//!       before calling this function. The drive will abort the erase if prepare
//!       was not issued or if too much time elapsed between prepare and erase.
//!
//! \post On \c SUCCESS with \c ATA_SECURITY_ERASE_NORMAL (clear): all host-addressable
//!       LBAs read as zero. Reallocated sectors may retain data. The security password
//!       is cleared.
//! \post On \c SUCCESS with \c ATA_SECURITY_ERASE_ENHANCED (purge): all user data
//!       including reallocated sectors, reserved blocks, and non-volatile caches is
//!       permanently destroyed. The security password is cleared.
//!
//! \retval SUCCESS           The erase completed successfully.
//! \retval FAILURE           The drive reported a command error.
//! \retval NOT_SUPPORTED     The device does not support the ATA Security Feature Set,
//!                           or the interface does not allow this command (e.g., many
//!                           USB bridges block ATA security commands).
//! \retval PERMISSION_DENIED The drive is in the security locked state. Unlock before erasing.
//! \retval BAD_PARAMETER     \a device is \c M_NULLPTR.
//!
//! \warning (Normal erase — IEEE 2883 clear): Logical techniques are applied to all
//!          addressable storage locations. Simple non-invasive data recovery is prevented.
//!          This operation is irreversible. Obtain explicit user confirmation before invoking.
//! \warning (Enhanced erase — IEEE 2883 purge): Logical techniques target user data,
//!          overprovisioning, unused space, and bad blocks. Data recovery is infeasible
//!          even with state-of-the-art laboratory techniques. This operation is irreversible.
//!          Obtain explicit user confirmation before invoking.
//!
//! \par Platform:
//!   - Windows: requires administrator privileges; libATA on Linux may block this command
//!     unless the kernel parameter \c libata.allow_tpm=1 is set (the flag name is misleading).
//!   - USB: support depends on the USB bridge chip's ATA Security command translation. Use \ref openSeaChest_PassthroughTest to validate a specific device.
//!
//! \code{.c}
//!   // Restore MaxLBA first, then run enhanced (purge) erase with empty password
//!   if (device->drive_info.ata_Options.securityEnabled)
//!   {
//!       eReturnValues prepRet = ata_Security_Erase_Prepare(device);
//!       if (prepRet == SUCCESS)
//!       {
//!           eReturnValues ret = perform_ata_security_erase(
//!               device, ATA_SECURITY_ERASE_ENHANCED, M_NULLPTR);
//!           if (ret != SUCCESS)
//!           {
//!               printf("Erase failed: %d\n", ret);
//!           }
//!       }
//!   }
//! \endcode
//!
//! \sa ata_Security_Erase_Prepare(), ata_Security_Disable_Password(), get_ATA_Security_Info()
eReturnValues perform_ata_security_erase(tDevice*            device,
                                         eAtaSecurityEraseType eraseType,
                                         const char*          password);
```

---

## Protocol-Specific Parameter Encoding

Storage protocols define several categories of non-obvious numeric encodings for command fields. The type and variable name alone cannot convey these semantics — they must appear in the `\param` or `\details` text. The patterns below recur throughout the ATA, SCSI, and NVMe command sets.

### ATA sector count: zero means maximum

For ATA data transfer commands, a sector count of **0 is not "zero sectors"** — it encodes the maximum supported transfer:

- 28-bit commands (READ SECTORS, WRITE SECTORS, READ MULTIPLE, WRITE MULTIPLE, etc.): `0` = **256 sectors**
- 48-bit commands (READ SECTORS EXT, WRITE SECTORS EXT, etc.): `0` = **65536 sectors**

This rule applies to data transfer commands specifically. Not all ATA commands follow it — check the ATA Command Set (ACS) specification for each command individually.

```c
//! \param[in] sectorCount  Sectors to transfer. A value of 0 is a special ATA encoding
//!   meaning maximum transfer size: 256 sectors for 28-bit commands, or 65536 sectors
//!   for 48-bit commands. Do not pass 0 with the intent of transferring nothing.
```

### SCSI transfer length: READ(6) vs READ(10/12/16/32)

SCSI uses two distinct zero-length conventions depending on the READ command variant:

| Command | Transfer length = 0 means |
|---------|--------------------------|
| READ(6) | **256 logical blocks** — matches the ATA zero-means-maximum convention |
| READ(10), READ(12), READ(16), READ(32) | **No data transfer** — the drive validates all CDB fields (LBA, length, flags) but transfers no data; a standard way to probe whether a command and address are valid |

**SATL and bridge caveat:** ATA-to-SCSI Translation Layer (SATL) implementations — USB bridges, RAID controllers, and software SATLs — frequently do not implement the zero-length READ(10/12/16/32) NOP behavior correctly. Add an `\attention` when the function may be called in a SATL context:

```c
//! \attention On SATL (ATA-to-SCSI Translation Layer) devices such as USB bridges,
//!   a zero-length READ may not behave as the SCSI specification defines.
//!   Validate behavior on such devices before relying on zero-length transfer semantics.
```

### NVMe: zero-indexed block count (NLB field)

NVMe read and write commands use a **zero-indexed Number of Logical Blocks (NLB)** field. The wire value is one less than the actual transfer count:

| NLB field value | Logical blocks transferred |
|-----------------|--------------------------|
| 0 | 1 |
| 1 | 2 |
| N | N + 1 |

This is the opposite of the ATA and SCSI conventions. When a function accepts a human-readable block count and constructs an NVMe command internally, document the conversion:

```c
//! \param[in] blockCount  Number of logical blocks to transfer (1 or more). Converted
//!   internally to the NVMe NLB field (NLB = blockCount − 1) using \c NVME_0_BASED_ADJUST().
//!   Passing 0 is a programming error and returns \c BAD_PARAMETER.
//!
//! \sa NVME_0_BASED_ADJUST
```

When a function accepts a raw NLB field value (a low-level command builder), document the zero-indexed encoding directly instead.

Use `NVME_0_BASED_ADJUST(count)` at the call site rather than writing `count - 1` inline — it makes the intent clear to the reader and is the project-standard way to express this conversion. When documenting such call sites, a brief inline `\sa NVME_0_BASED_ADJUST` in the `\param` keeps the cross-reference close to where the reader needs it.

### ATA standby timer: lookup table, not a formula

The ATA IDLE and STANDBY commands accept a **timer count value** whose encoding is a non-linear lookup table defined in the ACS specification — not a simple formula. Functions that accept or return a raw timer byte must include the complete table in `\details`:

```c
//! \param[in] timerValue  ATA standby timer count (raw byte), encoded per ACS:
//!
//!   | Value   | Standby interval                                            |
//!   |---------|-------------------------------------------------------------|
//!   | 0       | Timer disabled — standby never entered automatically        |
//!   | 1–240   | \f$ \text{value} \times 5 \f$ seconds (5 s to 20 min)      |
//!   | 241–251 | \f$ (\text{value} - 240) \times 30 \f$ minutes (30–330 min)|
//!   | 252     | Vendor-defined (commonly 21 minutes on shipping drives)     |
//!   | 253     | Vendor-defined (between 8 and 12 hours)                     |
//!   | 254     | Reserved — do not use                                       |
//!   | 255     | 21 minutes 15 seconds                                       |
//!
//!   The SAT (SCSI-ATA Translation) approximation maps a SCSI Standby Condition Timer
//!   (in milliseconds) to this table using the formula
//!   \f$ \text{timerValue} = \lceil T_{ms} / 5000 \rceil \f$ for values 1–240.
//!   Use \ref set_Standby_Timer() when a millisecond-based interface is preferred —
//!   it performs this conversion automatically.
```

Always link to the project's higher-level helper (`\ref set_Standby_Timer()`, `\ref set_EPC_Timer()`, etc.) when a raw-encoding parameter has a convenience wrapper, so callers know the simpler path exists.

### General guidance: ask about special numeric inputs

When starting documentation for any function whose parameters are passed directly or nearly-directly to a protocol command field, ask the human reviewer:

> "Does this parameter use a standard linear encoding (bytes, seconds, a plain count), or does the protocol define special values — such as zero-means-maximum, zero-means-NOP, zero-indexing, or a lookup table? If so, what are the special values, and what is the valid input range?"

This question is especially important for:

- **Sector and block counts** in ATA, SCSI, and NVMe read/write commands — all three protocols differ
- **Timer values** in ATA (standby, idle, EPC timers), SCSI (timer mode pages), and NVMe (feature fields)
- **LBA and address fields** that may have alignment requirements or maximum-address conventions
- **Command-specific fields** where 0, the maximum value, or reserved values have protocol-defined semantics

When the encoding has three or more distinct cases, include the complete table in `\details`. When there are only one or two special values, note them inline in the `\param` text. When the function wraps a higher-level helper that hides these encodings from the caller, document only the helper's interface — but link to the raw-level function so readers can follow the data path.

---

## Protocol-Defined Feature States and Command Acceptance

Storage protocol specifications define feature sets whose commands are only valid in specific device states. ACS defines state machines with numbered states, explicit transition conditions, and command acceptance tables. When a function wraps a command that participates in such a state machine, rich documentation requires three components:

1. **A feature-level overview page** — the state machine diagram and command acceptance table, placed once in the primary feature header and never duplicated per-function.
2. **Per-command state annotations** — `\pre` entries stating which states accept or block the specific command, with a `\sa` cross-reference to the feature overview.
3. **Sequential prerequisite chains** — where the spec requires one command to immediately precede another (e.g., ERASE PREPARE → ERASE UNIT), document this as a `\pre` on the dependent command.

This pattern applies whenever the spec defines named states, a state diagram, and a table describing which states allow or block each command. ATA Security is the canonical reference for this pattern.

### ATA Security feature set: canonical reference (ACS §4.18)

ACS §4.18 defines seven named states (SEC0–SEC6) distinguished by three observable bits: Security Enabled, Security Locked, and Security Frozen. The SECURITY COUNT EXPIRED bit tracks the password attempt counter separately.

**Security states (ACS Table 10):**

| State | Power | Enabled | Locked | Frozen | Notes |
|-------|-------|---------|--------|--------|-------|
| SEC0  | off   | 0       | N/A    | N/A    | Powered down, security disabled |
| SEC1  | on    | 0       | 0      | 0      | Disabled / not locked / not frozen |
| SEC2  | on    | 0       | 0      | 1      | Disabled / not locked / **frozen** |
| SEC3  | off   | 1       | 1      | N/A    | Powered down, security enabled |
| SEC4  | on    | 1       | 1      | 0      | Enabled / **locked** / not frozen |
| SEC5  | on    | 1       | 0      | 0      | Enabled / not locked / not frozen |
| SEC6  | on    | 1       | 0      | 1      | Enabled / not locked / **frozen** |

SEC0 and SEC3 are powered-down states and are not observable via IDENTIFY DEVICE. The SECURITY ENABLED, SECURITY LOCKED, SECURITY FROZEN, and SECURITY COUNT EXPIRED bits in the Security page of the IDENTIFY DEVICE data log (ACS §A.11.8) report the live state for powered-on states.

**ACS Table 11 groups command acceptance into exactly three categories:**

- **Locked** = SEC4 only (Security Enabled / Locked / Not Frozen)
- **Unlocked or Disabled** = SEC1 or SEC5
- **Frozen** = SEC2 or SEC6

### Feature-level overview page template

Place the state machine diagram and command acceptance table in a `\defgroup` comment block in the primary feature header file. Every per-command function then cross-references this block with `\sa` rather than repeating the diagram. The `\mermaid` / `\endmermaid` tag pair (Doxygen ≥ 1.9.4) renders the diagram inline in HTML and PDF output.

```c
//! \defgroup ata_security ATA Security feature set
//! \ingroup ata_features
//!
//! \brief Password-based access control for user data and security configuration commands
//!   (ACS §4.18).
//!
//! \details
//!   The ATA Security feature set provides a two-password system (User and Master) that
//!   locks user data after every power-on until the correct password is supplied. Setting
//!   a User password transitions the device from SEC1 to SEC5; subsequent power-on resets
//!   move the device to SEC4 (Locked), where data access commands are aborted until
//!   SECURITY UNLOCK succeeds. Security is disabled when there is no active User password.
//!
//!   A factory-installed Master password exists before any User password is set. Setting
//!   the Master password alone does not enable security (does not lock after power-on).
//!
//!   ## State machine (ACS Figure 10)
//!
//! \mermaid
//! stateDiagram-v2
//!     SEC0 : SEC0 / Powered down / Disabled
//!     SEC1 : SEC1 / Disabled / Not Locked / Not Frozen
//!     SEC2 : SEC2 / Disabled / Not Locked / Frozen
//!     SEC3 : SEC3 / Powered down / Enabled / Locked
//!     SEC4 : SEC4 / Enabled / Locked / Not Frozen
//!     SEC5 : SEC5 / Enabled / Not Locked / Not Frozen
//!     SEC6 : SEC6 / Enabled / Not Locked / Frozen
//!
//!     SEC0 --> SEC1 : power-on reset / hardware reset
//!     SEC1 --> SEC0 : power-down
//!     SEC1 --> SEC1 : hardware reset\nSET PASSWORD (master)
//!     SEC1 --> SEC2 : FREEZE LOCK
//!     SEC1 --> SEC5 : SET PASSWORD (user)
//!     SEC2 --> SEC0 : power-down
//!     SEC2 --> SEC1 : hardware reset
//!     SEC3 --> SEC4 : power-on reset
//!     SEC4 --> SEC3 : power-down
//!     SEC4 --> SEC4 : hardware reset\nERASE PREPARE\nfailed UNLOCK
//!     SEC4 --> SEC1 : ERASE UNIT (success)
//!     SEC4 --> SEC5 : UNLOCK (success)
//!     SEC5 --> SEC3 : power-down
//!     SEC5 --> SEC4 : hardware reset
//!     SEC5 --> SEC5 : SET PASSWORD\nERASE PREPARE\nUNLOCK (while unlocked)
//!     SEC5 --> SEC1 : DISABLE PASSWORD / ERASE UNIT
//!     SEC5 --> SEC6 : FREEZE LOCK
//!     SEC6 --> SEC3 : power-down
//!     SEC6 --> SEC4 : hardware reset
//! \endmermaid
//!
//!   ## Command acceptance by security state (ACS Table 11)
//!
//!   | Command                    | Locked (SEC4) | Unlocked/Disabled (SEC1, SEC5) | Frozen (SEC2, SEC6) |
//!   |----------------------------|---------------|-------------------------------|---------------------|
//!   | SECURITY SET PASSWORD      | Aborted       | Executable                    | Aborted             |
//!   | SECURITY UNLOCK            | Executable    | Executable                    | Aborted             |
//!   | SECURITY ERASE PREPARE     | Executable    | Executable                    | Aborted             |
//!   | SECURITY ERASE UNIT        | Executable    | Executable                    | Aborted             |
//!   | SECURITY FREEZE LOCK       | Aborted       | Executable                    | Executable          |
//!   | SECURITY DISABLE PASSWORD  | Aborted       | Executable                    | Aborted             |
//!   | IDENTIFY DEVICE            | Executable    | Executable                    | Executable          |
//!   | READ / WRITE data commands | Aborted       | Executable                    | Executable          |
//!   | IDLE, STANDBY, SLEEP       | Executable    | Executable                    | Executable          |
//!   | READ LOG EXT / DMA EXT     | Executable    | Executable                    | Executable          |
//!   | SET FEATURES               | Executable    | Executable                    | Executable          |
//!   | DOWNLOAD MICROCODE         | Vendor-specific | Vendor-specific             | Vendor-specific     |
//!
//!   Commands not listed in ACS Table 11 are not addressed by the Security feature set.
//!   See ACS §4.18.11.2 for the exhaustive ~80-command list.
//!
//! \attention Restore the device's maximum LBA (HPA and DCO/AMAC for ATA devices) before
//!   invoking any erase command. A reduced MaxLBA leaves sectors above the limit unerased
//!   and unverifiable after the erase completes.
//!
//! \sa ata_Security_Set_Password(), ata_Security_Unlock(), ata_Security_Erase_Prepare(),
//!     ata_Security_Erase_Unit(), ata_Security_Freeze_Lock(), ata_Security_Disable_Password(),
//!     ata_Get_ATA_Security_Info()
```

### Per-command annotation patterns

Once the feature-level overview page exists, annotate each per-command function with its security state constraints. Use `\pre` for hard preconditions (the command returns aborted if not met) and `\par ATA Security State:` for an informational summary. Always include a `\sa` pointing to the `ata_security` defgroup.

**Pattern A — command aborted when Locked (most data-access and configuration commands):**

```c
//! \pre Device must not be in the Locked state (SEC4: Security Enabled/Locked/Not Frozen).
//!   This command returns command aborted when locked. Issue \ref ata_Security_Unlock()
//!   before calling this function. See \ref ata_security for the full state machine.
```

**Pattern B — command aborted when Locked and when Frozen (SECURITY SET PASSWORD, SECURITY DISABLE PASSWORD):**

These commands are blocked in two of the three acceptance categories. The only valid state is Unlocked or Disabled (SEC1 or SEC5).

```c
//! \pre Device must be in an Unlocked or Disabled state (SEC1 or SEC5). This command
//!   returns command aborted if the device is Locked (SEC4) or Frozen (SEC2 or SEC6).
//!   A SECURITY FREEZE LOCK command cannot be undone without a power cycle or hardware
//!   reset. See \ref ata_security for the state machine.
```

**Pattern C — always accepted in all security states (IDENTIFY DEVICE, IDLE, STANDBY, etc.):**

Add a `\note` only when the function appears near security-sensitive code, where the contrast with "most commands are blocked when locked" is relevant:

```c
//! \note Accepted in all ATA security states (Locked, Unlocked/Disabled, and Frozen).
//!   Use \ref ata_Get_ATA_Security_Info() to read the current security state bits from
//!   the IDENTIFY DEVICE data log at any time. See \ref ata_security.
```

**Pattern D — security command accepted in the Locked state (SECURITY UNLOCK, ERASE PREPARE, ERASE UNIT):**

These commands are the mechanism to exit the Locked state. The thing that blocks them is the Frozen state — the opposite of most commands. State this explicitly so it is not assumed to follow the "aborted when locked" pattern.

```c
//! \pre Device must be in the Locked (SEC4) or Unlocked/Not Locked (SEC5) state.
//!   This command returns command aborted in the Frozen states (SEC2 and SEC6).
//!   A power cycle or hardware reset is required to exit the Frozen state.
//! \pre The SECURITY COUNT EXPIRED bit must be zero (ACS §4.18.9). If the password
//!   attempt counter has reached zero, the device aborts all SECURITY UNLOCK and
//!   SECURITY ERASE UNIT commands until the next power-on or hardware reset.
```

### Sequential prerequisite pattern: ERASE PREPARE → ERASE UNIT

ACS §7.38 requires SECURITY ERASE PREPARE to be the **immediately preceding command** before SECURITY ERASE UNIT. Any command between them — including IDENTIFY DEVICE — clears the prepare condition and causes ERASE UNIT to return command aborted. Document this as a strict ordering `\pre` on the ERASE UNIT function:

```c
//! \pre \ref ata_Security_Erase_Prepare() must have been the immediately preceding command
//!   issued to the device. Any intervening command, including IDENTIFY DEVICE, clears the
//!   prepare condition and causes this command to return command aborted. Do not issue any
//!   diagnostic or status command between ERASE PREPARE and ERASE UNIT.
//! \pre Device must be in the Locked (SEC4) or Unlocked (SEC5) state. Returns command
//!   aborted in Frozen states (SEC2, SEC6). See \ref ata_security.
//! \pre ACS §4.18.8 exception: SECURITY ERASE UNIT accepts either the User password or
//!   the Master password regardless of the MASTER PASSWORD CAPABILITY setting (High or
//!   Maximum). This differs from SECURITY UNLOCK and SECURITY DISABLE PASSWORD, which
//!   reject the Master password when MASTER PASSWORD CAPABILITY is set to Maximum.
```

The MASTER PASSWORD CAPABILITY exception (ACS §4.18.8) is a genuine surprise for callers who expect ERASE UNIT to follow the same Master password rules as UNLOCK. Always document it.

### Password attempt counter

ACS §4.18.9 defines a 5-attempt password counter reset only by power-on or hardware reset (not by a successful unlock). When the counter reaches zero, the SECURITY COUNT EXPIRED bit is set and the device aborts all SECURITY UNLOCK and SECURITY ERASE UNIT commands until the next reset. For any function that performs an unlock or erase-unit operation, add an `\attention`:

```c
//! \attention ACS §4.18.9 password attempt counter: the device allows at most 5 failed
//!   SECURITY UNLOCK attempts in the Locked state (SEC4) before setting the SECURITY COUNT
//!   EXPIRED bit and aborting all further SECURITY UNLOCK and SECURITY ERASE UNIT commands.
//!   A power-on reset or hardware reset resets the counter to 5 and clears the SECURITY
//!   COUNT EXPIRED bit. A successful unlock does not reset the counter.
```

### Other ACS feature sets with state machines

Apply the same two-level structure (feature overview page + per-command annotations) to every ACS feature set that defines named states or command acceptance rules:

**Enhanced Power Conditions (EPC, ACS §4.8):** Defines states Active, Idle_a, Idle_b, Idle_c, Standby_z, Standby_y, and Sleep, each with a configurable inactivity timer. The IDLE (ACS §7.14) and STANDBY (ACS §7.44) commands trigger transitions and accept the non-linear lookup-table timer encoding documented in the Protocol-Specific Parameter Encoding section. Create a `\defgroup ata_power_management` overview with the EPC state diagram. Link timer-encoding functions back to it with `\sa`.

**Sanitize (ACS-4+):** Defines a four-state machine: SANITIZE_IDLE → SANITIZE_OPERATION_IN_PROGRESS → SANITIZE_OPERATION_COMPLETE or SANITIZE_OPERATION_FAILED. SANITIZE FREEZE LOCK and ANTIFREEZE LOCK parallel the Security freeze mechanism. Create a `\defgroup ata_sanitize` overview page using the same pattern.

**Obsoleted features — HPA and DCO (defined in ACS-2, obsoleted in ACS-3):** Host Protected Area (HPA) and Device Configuration Overlay (DCO) were blocked in the Security Locked state. For openSeaChest functions implementing these features, add `\deprecated` citing the spec revision and naming the replacement (Accessible Max Address Configuration / AMAC, introduced in ACS-3), and document the security state interaction from the ACS-2 command acceptance table in the function's `\pre`.

---

## Tag Reference

### Always-Required Tags

| Tag | Applies to | Usage |
|-----|-----------|-------|
| `\brief` | everything | One sentence, no period required. Appears in summary tables. |
| `\param[in]` | functions, macros | Input-only parameter. |
| `\param[out]` | functions, macros | Output-only — value written through the pointer; initial value not read. |
| `\param[in,out]` | functions, macros | Value is read AND written (e.g., a counter that is incremented). |
| `\retval <value> <description>` | functions | One entry per distinct return code. More thorough than `\return` prose. |

### Description Structure

| Tag | Usage |
|-----|-------|
| `\file` | File name — required at the top of every file. |
| `\fn` | Full function signature — optional when comment is directly above declaration; use for clarity on complex signatures. |
| `\def` | Macro name — required for function-like macros. |
| `\struct` / `\union` / `\enum` | Type name — required when the comment appears above a `typedef struct {}` block. |
| `\typedef` | Typedef name — required when documenting a `typedef`. |
| `\var` | Variable or struct member (alternative to `//!<` on the same line). |
| `\details` | Extended description body that follows `\brief`. Always use `\details` when the description is longer than one or two sentences so it is clearly separated from the brief in generated output. |

### Condition and Safety Tags (use as appropriate)

| Tag | When to use |
|-----|-------------|
| `\pre` | Document what **must be true before** calling the function. Use for ordering requirements (e.g., prepare before erase), initialisation state, locked/unlocked states. |
| `\post` | Document what **will be true after** the function returns successfully. Use when the call changes device or object state in a way the caller must account for. |
| `\warning` | Destructive, irreversible, or security-sensitive operations (erase, format, ATA security, TCG commands). Rendered as a prominent box. |
| `\attention` | Non-destructive but important caveats: USB bridge limitations, platform-specific surprises, timing constraints. Rendered differently from `\warning`. |
| `\note` | Informational aside: implementation choice explanations, standard references, non-obvious behaviour that is correct. |
| `\remark` | Implementation notes that do not affect the caller (why a particular algorithm was chosen, performance characteristics). |
| `\deprecated` | Functions that should no longer be used. Always include a migration note: what to use instead. See section below. |
| `\todo` | Known gaps: unimplemented cases, planned improvements, known missing error handling. |
| `\important` | Notable point that is neither a warning nor a caution. Rendered as a distinct callout box. Use when something must be understood but is not dangerous or destructive. |
| `\cite key` | Bibliographic reference via a BibTeX `.bib` file. Use to cite standards, specifications, or coding rules (e.g., C11 Annex K, CERT C, ISO/IEC TS 17961, ACS-4). |

### Custom Paragraph: `\par Platform:`

For any function whose behaviour differs by OS or interface, add a `\par Platform:` section. The colon and space after the name are required by Doxygen:

```c
//! \par Platform:
//!   - Windows: requires STORAGE_PROTOCOL_COMMAND; not available on Windows 7/8.1 without driver workarounds.
//!   - Linux: requires kernel 4.4+ for NVMe passthrough via /dev/nvme*; earlier kernels use /dev/sg*.
//!   - FreeBSD: uses CAM; NVMe passthrough available from FreeBSD 12+.
//!   - USB: not applicable — this command requires direct ATA or NVMe passthrough.
```

### `\important` and `\remark`: Informational Callouts

`\important` renders as a distinct callout box — lighter than `\attention` but more prominent than `\note`. Use it when something must be understood but is not dangerous or destructive:

```c
//! \important This function performs the NLB adjustment internally. Pass a
//!   human-readable block count ≥ 1; do not pre-subtract 1.
```

`\remark` is for secondary implementation observations that do not affect the caller — algorithm choices, performance characteristics, historical context:

```c
//! \remark On most Self-Encrypting Drives the enhanced erase is implemented as a
//!   cryptographic key rotation and completes in under one second regardless of
//!   drive capacity.
```

**Callout hierarchy** (most to least severe):

| Tag | Severity | Typical content |
|-----|----------|----------------|
| `\warning` | Critical | Destructive, irreversible operation — requires user confirmation |
| `\attention` | High | Important non-destructive caveat: platform quirk, timing constraint |
| `\important` | Medium | Notable point that must be understood but is not dangerous |
| `\note` | Low | Informational aside, standard reference, non-obvious-but-correct behaviour |
| `\remark` | Informational | Implementation detail that does not affect how the caller uses the function |

### ACS Feature Lifecycle: Spec Additions, Mutual Exclusions, and Supersessions

#### When a feature was added or made obsolete

Use `\note` with a bold spec label. **Do not use `\deprecated`** — that marks the *C function* as deprecated for callers, adds the symbol to Doxygen's Deprecated List, and triggers IDE warnings. It must not be used to indicate that a *storage-protocol feature* was retired in a spec revision:

```c
//! \note \b Added in ACS-2 (2011): This feature was introduced in ACS-2. It is not
//!   available on drives conforming only to ATA-6 or earlier revisions.

//! \note \b Obsolete in ACS-7 (2024): Read Look-Ahead is specified as obsolete.
//!   It remains functional on drives implementing earlier ACS revisions, but new
//!   device designs are not required to implement it.
```

#### Mutually exclusive features

When two features cannot be active simultaneously, document the constraint on both sides:

```c
//! \note \b EPC/APM mutual exclusion (ACS-3 §4.8): Extended Power Conditions (EPC)
//!   and Advanced Power Management (APM) cannot both be enabled at the same time.
//!   Enabling EPC automatically disables APM, and vice versa. Check
//!   \c device->drive_info.ata_Options.EPCEnabled and
//!   \c device->drive_info.ata_Options.APMEnabled before calling either feature's
//!   configuration functions.
//!
//! \sa set_EPC_settings(), set_APM_level()
```

#### Superseded features — cross-reference both directions

When one ATA feature supersedes another, document the relationship in both directions so readers navigating either API find the connection:

```c
// In the HPA functions — pointing forward to AMAC:
//! \note \b Superseded by AMAC (ACS-3): Host Protected Area (HPA) was superseded by
//!   Accessible Max Address Configuration (AMAC) in ACS-3. AMAC provides equivalent
//!   MaxLBA control without the limitations of the HPA register model. A device
//!   supports either HPA or AMAC, not both; check identify data before calling.
//!
//! \sa set_Max_Address_Ext(), get_HPA_info()

// In the AMAC functions — pointing back to HPA:
//! \note \b Replaces HPA (ACS-3): AMAC supersedes the Host Protected Area (HPA)
//!   feature set introduced in ATA-4. A device supports either HPA or AMAC, not
//!   both. Use identify data to determine which feature set is present before calling.
//!
//! \sa get_MaxLBA(), get_HPA_info()
```

#### `\since` — project release versioning only

`\since` tracks *when this function was added to the project's own API*, not ACS spec revision history. Use `\note \b ACS-x:` for spec revision notes:

```c
//! \since opensea-operations 3.0.0 — replaces the legacy do_secure_erase() interface.
```

---

## Inline Text Markup

Use these inside description text to format symbols correctly:

| Markup | Output | Use for |
|--------|--------|---------|
| `\a name` | *name* (italic) | Parameter names when referenced in description text |
| `\p name` | `name` (monospace) | Parameter names (alternative to `\a`; prefer `\a` for consistency) |
| `\c symbol` | `symbol` (monospace) | Constants, enum values, macro names, type names |
| `\e word` | *word* (italic) | Emphasis |
| `\b word` | **word** (bold) | Strong emphasis (use sparingly) |
| `\n` | newline | Force line break within a tag's text |

The existing codebase uses `\a` for parameter references in prose. Continue this pattern:

```c
//! Allocates \a count elements of \a size bytes each, returning \c M_NULLPTR on failure.
```

---

## Macro Documentation

Function-like macros use `\def` instead of `\fn`. When the macro is a thin wrapper around a typed inline (the preferred project pattern), document the macro — not the inline — as the public API:

```c
//! \def M_BytesTo4ByteValue(b3, b2, b1, b0)
//! \brief Assembles four bytes into a \c uint32_t with \a b3 as the most significant byte.
//!
//! Arguments are named in MSB-first order regardless of the host byte order.
//! The result is in host byte order. Use this when parsing a big-endian (SCSI) buffer:
//!
//! \code{.c}
//!   uint32_t lba = M_BytesTo4ByteValue(buf[0], buf[1], buf[2], buf[3]);
//! \endcode
//!
//! For a little-endian (ATA/NVMe) buffer, reverse the byte offsets:
//!
//! \code{.c}
//!   uint32_t sector = M_BytesTo4ByteValue(buf[3], buf[2], buf[1], buf[0]);
//! \endcode
//!
//! \param[in] b3 Most significant byte (bits 31:24).
//! \param[in] b2 Bits 23:16.
//! \param[in] b1 Bits 15:8.
//! \param[in] b0 Least significant byte (bits 7:0).
//! \return Assembled \c uint32_t in host byte order.
#define M_BytesTo4ByteValue(b3, b2, b1, b0) bytes_To_Uint32(b3, b2, b1, b0)
```

---

## Type Documentation

### Structs and Unions

Place the `\struct` comment directly above the `typedef struct` or `struct` declaration. Document every member with `//!<`:

```c
//! \struct tScsiCommandOptions
//! \brief Options controlling how a SCSI command is constructed and issued.
//!
//! Initialise with \c safe_memset(&opts, sizeof(opts), 0, sizeof(opts)) before
//! setting fields; unset fields default to zero which is the safe value for all members.
typedef struct
{
    uint8_t   cdb[32];          //!< Command descriptor block (CDB). Bytes past \c cdbLength are ignored.
    uint8_t   cdbLength;        //!< Valid length of \a cdb in bytes. Must be 6, 10, 12, 16, or 32.
    uint8_t*  dataBuffer;       //!< Pointer to the data transfer buffer. \c M_NULLPTR for no-data commands.
    uint32_t  dataLength;       //!< Length of \a dataBuffer in bytes. Must be 0 when \a dataBuffer is \c M_NULLPTR.
    eDataTransferDirection direction; //!< Direction of the data transfer relative to the host.
    uint32_t  timeout;          //!< Command timeout in seconds. Use 0 for the device default.
} tScsiCommandOptions;
```

### Enumerations

Document the enum type and every enumerator:

```c
//! \enum eAtaSecurityEraseType
//! \brief Selects the ATA Security Erase Unit erase mode.
typedef enum
{
    ATA_SECURITY_ERASE_NORMAL   = 0, //!< Normal erase: overwrites user data; may take hours on HDDs.
    ATA_SECURITY_ERASE_ENHANCED = 1, //!< Enhanced erase: uses drive-defined method (e.g., crypto-erase on SEDs); may be near-instant.
} eAtaSecurityEraseType;
```

### Enumerations Wrapped in Macros (`M_DECLARE_ENUM` / `M_DECLARE_ENUM_TYPE`)

The project declares enums through `M_DECLARE_ENUM(name, ...)` and `M_DECLARE_ENUM_TYPE(name, type, ...)` to
portably generate `typedef enum`, `enum class`, or a typed enum depending on the active C/C++ standard.

**Without Doxyfile preprocessing**, Doxygen treats `M_DECLARE_ENUM(...)` as an opaque macro call. The `\enum`
tag placed above the call IS processed — the enum type appears in the docs — but the individual enum
values are **not documented** because Doxygen never sees the expanded body.

**Comment style inside macro arguments**: `//!<` terminates at the newline and would cut the macro
call short. Use `/*!< description. */` block-style trailing comments inside macro argument lists —
this is the **only** valid Doxygen member-doc syntax that fits inside a multi-line macro invocation.
This is a deliberate exception to the project's general `//!<` preference.

```c
//! \enum eAllowedUnitInput
//! \brief Enum specifying which units are allowed at the end of user input.
//!
//! \details This enum controls which suffix strings the parser accepts after a numeric value.
//! Accepted suffix strings are matched case-sensitively. Any unrecognised suffix must be
//! treated as a parse error.
M_DECLARE_ENUM(eAllowedUnitInput,
               /*!< No unit suffix is permitted; the value must be a plain integer. */
               ALLOW_UNIT_NONE,
               /*!< Data-size unit suffixes. Each accepted string and its meaning:
                *   - \c BLOCKS or \c SECTORS — one logical block (sector). The byte count
                *     depends on the drive geometry: 512 bytes for 512n (512-byte native sector)
                *     drives, 4096 bytes for 4Kn (4K native, 4096-byte sector) drives, or the
                *     device's reported Logical Block Length for other configurations.
                *   - \c B — bytes (octets).
                *   - \c KB — kilobytes: 1\,000 bytes (SI, International System of Units prefix).
                *   - \c KiB — kibibytes: 1\,024 bytes (IEC 80000-13 binary prefix).
                *   - \c MB — megabytes: 1\,000\,000 bytes (SI prefix).
                *   - \c MiB — mebibytes: 1\,048\,576 bytes (IEC binary prefix).
                *   - \c GB — gigabytes: 1\,000\,000\,000 bytes (SI prefix).
                *   - \c GiB — gibibytes: 1\,073\,741\,824 bytes (IEC binary prefix).
                *   - \c TB — terabytes: 10^12 bytes (SI prefix).
                *   - \c TiB — tebibytes: 2^40 bytes (IEC binary prefix).
                */
               ALLOW_UNIT_DATASIZE,
               /*!< Temperature unit suffixes. Each accepted string and its meaning:
                *   - \c f — degrees Fahrenheit.
                *   - \c c — degrees Celsius (Centigrade).
                *   - \c k — kelvin (K; absolute temperature scale; no degree symbol).
                */
               ALLOW_UNIT_TEMPERATURE);
```

The `/*!< ... */` comments only attach to the enum values in the generated output when the Doxyfile
preprocessing settings described in the Doxyfile section below are active. Without them, enum value
docs are silently lost.

### Typedefs

When a `typedef` has its own line (not a `typedef struct`), use `\typedef`:

```c
//! \typedef eReturnValues
//! \brief Common return code type used throughout the opensea library stack.
//!
//! Functions return \c SUCCESS (0) on success and a non-zero code on failure.
//! Always compare against the named constants — never test for specific integer values.
typedef int eReturnValues;
```

### Bit-fields and Anonymous Structs/Unions

ATA, SCSI, and NVMe structures use bit-fields, anonymous unions, and nested anonymous structs heavily. Three rules cover the common patterns.

#### Bit-fields

Document every named bit-field with `//!<`. Include the bit range in `[hi:lo]` notation so readers don't have to count widths from the declaration. Name valid constant values with `\c`:

```c
typedef struct
{
    uint8_t lbaLow    : 4;  //!< LBA bits [27:24]. Ignored when \c lba is 0 (CHS mode).
    uint8_t device    : 1;  //!< Device select: 0 = device 0, 1 = device 1.
    uint8_t lba       : 1;  //!< Address mode: 1 = LBA, 0 = CHS. Must be 1 for 48-bit commands.
    uint8_t reserved  : 1;  //!< Reserved. Must be 0.
    uint8_t obsolete  : 1;  //!< Obsolete (ATA-1 DRQ type). Set to 0.
} ataDeviceRegister;
```

Reserved and padding fields get a one-phrase comment — nothing more:

```c
uint8_t reserved  : 3;  //!< Reserved. Must be 0.
uint8_t _pad;           //!< Padding; not transmitted.
```

#### Anonymous union with a raw-byte overlay

This is the most common ATA pattern: a union that exposes the same register as either a raw byte (for bulk copy) or a structured set of bit-fields. Put `//!<` on the `union {` line for a brief label; document each member inside normally. Doxygen promotes anonymous union/struct members into the parent struct's member list in the generated output:

```c
typedef struct
{
    union               //!< Device/Head register — byte or structured bit-field access.
    {
        uint8_t raw;    //!< Full register byte for bulk copy and compare operations.
        struct
        {
            uint8_t lbaLow   : 4;  //!< LBA bits [27:24].
            uint8_t device   : 1;  //!< Device select: 0 = device 0, 1 = device 1.
            uint8_t lba      : 1;  //!< Address mode: 1 = LBA, 0 = CHS.
            uint8_t reserved : 2;  //!< Reserved. Must be 0.
        };
    };
} ataDeviceRegister;
```

#### Deeply nested or repeated register pairs — use `\name` groups

When the same physical register is split across a current/previous pair (HOB-style 48-bit ATA), an anonymous `//!<` on `struct {` would be ambiguous. Use a `\name` group instead:

```c
//! \name 48-bit LBA low register (current / previous)
//! \brief HOB-aware access to LBA bits [7:0] and [31:24].
//!@{
uint8_t lbaLowCurrent;   //!< LBA bits [7:0]  — HOB = 0 (current value).
uint8_t lbaLowPrevious;  //!< LBA bits [31:24] — HOB = 1 (previous value, 48-bit commands only).
//!@}
```

---

## Inferring Documentation from Compiler Attribute Macros

The attribute macros in `code_attributes.h` carry machine-readable semantics that map directly to Doxygen. When attributes are present, use them as authoritative source of truth for `\param` directions, `\pre` conditions, `\return`/`\retval` wording, and struct member descriptions. Do not write documentation that contradicts the attribute — the attribute is what the compiler and static analyzers enforce.

> **`code_attributes.h` is actively maintained.** New attributes are added and existing ones evolve over time. The quick-reference table below is a snapshot — it may not list every attribute present in the current header. Before writing documentation for a function decorated with an attribute that does not appear in this table, **read `code_attributes.h` directly**. The `\def` Doxygen block on each macro is the authoritative description of its semantics, parameter positions, and correct usage. The table is a convenience guide, not a complete specification.

### Attribute → Doxygen quick-reference table

| Macro(s) | Applied to | Doxygen implication |
|----------|-----------|---------------------|
| `M_PARAM_RO(n)` | function | param `n` is input-only → `\param[in]`; caller must have data ready before call |
| `M_PARAM_WO(n)` | function | param `n` is output-only → `\param[out]`; no pre-initialization required by caller |
| `M_PARAM_RW(n)` | function | param `n` is read and written → `\param[in,out]`; caller must pre-initialize |
| `M_PARAM_RO_SIZE(n, sz)` | function | param `n` read-only; `sz` is its byte size — note both in `\param` descriptions |
| `M_PARAM_WO_SIZE(n, sz)` | function | param `n` write-only; `sz` is its byte size — note both in `\param` descriptions |
| `M_PARAM_RW_SIZE(n, sz)` | function | param `n` read-write; `sz` is its byte size — note both in `\param` descriptions |
| `M_NONNULL` | pointer param | caller must pass non-null — add inline note or `\pre` |
| `M_NULLABLE` | pointer param | pointer may be null — document what null means (ignored, empty, error, etc.) |
| `M_NULL_UNSPECIFIED` | pointer param | nullability unspecified — document usage explicitly in `\param` |
| `M_NONNULL_IF_NONZERO_SIZE(p, sz)` | function | `p` may be null only when `sz` == 0 — document the conditional |
| `M_NONNULL_IF_NONZERO_SIZE_COUNT(p, sz, n)` | function | `p` may be null only when `sz` × `n` == 0 — document the conditional |
| `M_RETURNS_NONNULL` | function | return value is never null — say "never returns `\c M_NULLPTR`" |
| `M_NULL_TERM_STRING(n)` | function | param `n` must be null-terminated — note in `\param[in]` |
| `M_NONSTRING` | struct member | char array may not be null-terminated — note field length and warn against string functions |
| `M_FUNC_ATTR_MALLOC` | function | returns heap memory caller must free — document ownership in `\return` |
| `M_ALLOC_DEALLOC(fn, pos)` | function | caller must use `fn` to free — name the deallocation function explicitly |
| `M_MALLOC_SIZE(n)` | function | allocation size comes from param `n` — reference in `\return` or `\param` |
| `M_CALLOC_SIZE(n, m)` | function | allocation size = param `n` × param `m` — document the relationship |
| `FUNC_ATTR_PRINTF(fmt, va)` | function | param `fmt` is a printf format string — note literal-string security requirement |
| `FUNC_ATTR_SCANF(fmt, va)` | function | param `fmt` is a scanf format string — note literal-string security requirement |
| `M_FILE_DESCRIPTOR(n)` | function | param `n` is an open file descriptor, not a plain `int` — note in `\param[in]` |
| `M_FILE_DESCRIPTOR_R(n)` | function | fd must be open for reading — add `\pre` or note in `\param[in]` |
| `M_FILE_DESCRIPTOR_W(n)` | function | fd must be open for writing — add `\pre` or note in `\param[in]` |
| `M_DIAG_WARN(cond, msg)` | function | Clang warns when `cond` is true at compile time — translate the `cond` expression into an `\attention` paragraph (never copy the C expression verbatim) |
| `M_DIAG_ERROR(cond, msg)` | function | Clang errors when `cond` is true at compile time — translate the `cond` expression into a `\pre` entry; use the condition-to-prose guide |
| `M_COUNTED_BY(member)` | struct member | flex array element count controlled by `member` — cross-reference both `//!<` docs |
| `M_COUNTED_BY_OR_NULL(member)` | struct member | as above but array may be null — note the null case |
| `M_SIZED_BY(member)` | struct member | pointer byte size controlled by `member` — cross-reference both `//!<` docs |
| `M_SIZED_BY_OR_NULL(member)` | struct member | as above but pointer may be null — note the null case |
| `M_CONST_FUNC` | function | no global reads, no side effects — note thread-safety and determinism |
| `M_PURE_FUNC` | function | no side effects; may read globals — note result may differ if global state changes |
| `M_TAINTED_ARGS` | function | arguments from untrusted input — note validation requirement in `\attention` |
| `M_ALL_PARAMS_NONNULL` | internal function | optimizer removes null checks — all pointer params proven non-null at every call site |
| `M_NONNULL_PARAM_LIST(...)` | internal function | same as above for specific named params only |

---

### Parameter access mode → `\param` direction

`M_PARAM_RO`, `M_PARAM_WO`, and `M_PARAM_RW` are the most direct attribute → Doxygen mapping:

| Attribute | `\param` direction | Pre-initialization required? |
|-----------|-------------------|-----------------------------|
| `M_PARAM_RO(n)` | `\param[in]` | Yes — caller must provide valid data |
| `M_PARAM_WO(n)` | `\param[out]` | No — function writes the result |
| `M_PARAM_RW(n)` | `\param[in,out]` | Yes — function reads and modifies in place |

For `M_PARAM_WO`, MSVC analysis benefits from caller zero-initialization even though it is not logically required. Note this if the caller is expected to initialize:

```c
//! \param[out] outputInteger Pointer to store the converted value. The caller need not
//!   initialize this before the call; the result is written on return.
```

The `_SIZE` variants link two parameters. When `M_PARAM_RO_SIZE(2, 3)` appears, document both parameters' relationship explicitly:

```c
M_PARAM_RO_SIZE(2, 3) M_PARAM_WO_SIZE(1, 3)
errno_t safe_memcpy(void* dest, size_t destsz, const void* src, size_t count);

//! \param[out] dest    Destination buffer. Must be at least \a count bytes.
//! \param[in]  destsz  Byte capacity of \a dest. The copy is aborted if \a count exceeds this.
//! \param[in]  src     Source buffer. Read for \a count bytes.
//! \param[in]  count   Number of bytes to copy. Must be \<= \a destsz.
```

Always document both ends: the pointer `\param` mentions the size parameter, and the size `\param` mentions which buffer it guards.

---

### Nullability

**`M_NONNULL`** — caller must pass a non-null pointer. For most cases, add an inline note to `\param`:

```c
//! \param[in] device Pointer to an initialised \c tDevice structure. Must not be \c M_NULLPTR.
```

For parameters where null causes undefined behavior (not just an error return), use `\pre`:

```c
//! \pre \a device must not be \c M_NULLPTR. Behavior is undefined if null is passed.
```

**`M_NULLABLE`** — pointer may be null. Document *what happens* when null is passed — do not leave this implicit:

```c
//! \param[out] unit Pointer to receive the unit string. Pass \c M_NULLPTR to ignore the unit.
```

**`M_NULL_UNSPECIFIED`** — nullability is unspecified (legacy/compatibility). Document the actual usage clearly in `\param`; the attribute alone is not enough.

**`M_NONNULL_IF_NONZERO_SIZE(arg, sizearg)`** — the pointer at index `arg` may be null only when the parameter at index `sizearg` is zero. Document the conditional on both parameters:

```c
M_NONNULL_IF_NONZERO_SIZE(2, 1)
errno_t write_if_sized(size_t count, uint8_t* buffer);

//! \param[in]  count  Number of bytes. When zero, the function is a no-op and returns immediately.
//! \param[out] buffer Destination buffer. May be \c M_NULLPTR only when \a count is zero.
//!   Must point to at least \a count bytes when \a count is non-zero.
```

**`M_NONNULL_IF_NONZERO_SIZE_COUNT(arg, sizearg, countarg)`** — same pattern with a two-component size (`size × count`). Note that the pointer must be non-null when *either* `sizearg` or `countarg` is non-zero.

**`M_RETURNS_NONNULL`** — return value is never null. Say so directly:

```c
//! \return Pointer to the result buffer. Never returns \c M_NULLPTR.
```

For `M_NODISCARD` functions, combine with `\retval`:

```c
//! \retval (non-null)  Always succeeds; the returned pointer must be freed with \ref safe_free().
```

**`M_ALL_PARAMS_NONNULL` / `M_NONNULL_PARAM_LIST`** — these appear **only on internal functions** (the optimizer removes null checks when these are present). In Doxygen, add a `\note` making the internal nature explicit:

```c
//! \note Internal function. All pointer parameters are proven non-null at every call site.
//!   Do not call with null pointers — null checks have been eliminated by the optimizer.
```

---

### String parameters

**`M_NULL_TERM_STRING(n)`** — parameter `n` must be a null-terminated string. Note this in `\param[in]`:

```c
//! \param[in] strToConvert Null-terminated string to parse. Must not be \c M_NULLPTR.
//!   Hex prefixes (0x, AEh) are accepted.
```

**`M_NONSTRING`** on a struct member — the character array is NOT null-terminated. Note the exact byte length and warn against passing directly to string functions:

```c
typedef struct
{
    M_NONSTRING char serialNumber[20]; //!< Serial number: 20 bytes of ASCII, NOT null-terminated.
                                       //!< Do not pass directly to printf / strlen / strcpy.
    M_NONSTRING char modelNumber[40];  //!< Model number: 40 bytes of ASCII, NOT null-terminated.
} tDriveIdentify;
```

---

### Memory allocation and deallocation

**`M_FUNC_ATTR_MALLOC`** — the function returns newly allocated heap memory. The caller owns it and must free it. Document this in `\return`:

```c
//! \return Pointer to the allocated buffer. Caller must free with \ref safe_free().
//! \retval M_NULLPTR  Allocation failed.
```

**`M_ALLOC_DEALLOC(fn, pos)`** — name the required deallocation function. This is critical when the paired free function is non-obvious (e.g., `safe_free_aligned` vs. `safe_free`):

```c
M_FUNC_ATTR_MALLOC M_MALLOC_SIZE(1) M_ALLOC_DEALLOC(safe_free_aligned, 1)
void* safe_malloc_aligned(size_t size, size_t alignment);

//! \return Pointer to the aligned buffer. Must be freed with \ref safe_free_aligned(),
//!   not \ref safe_free() — using the wrong deallocator crashes on Windows.
//! \retval M_NULLPTR  Allocation failed.
```

**`M_MALLOC_SIZE(n)`** / **`M_CALLOC_SIZE(n, m)`** — the allocation size is driven by one or two parameters. Reflect this in the `\return` or linked `\param` so callers know the buffer extent:

```c
//! \return Pointer to a buffer of exactly \a size bytes. Returns \c M_NULLPTR on failure.
//! \return Pointer to a zero-initialised buffer of \a count × \a size bytes. Returns \c M_NULLPTR on failure.
```

---

### File descriptors

**`M_FILE_DESCRIPTOR(n)`** — param `n` is an integer file descriptor, not a general-purpose `int`. Note this in `\param[in]` and use `\pre` for the pre-open requirement:

```c
//! \param[in] fd     Open file descriptor. Must be a valid, open descriptor.
//! \pre \a fd must be open and valid. Behavior is undefined on an invalid or closed descriptor.
```

**`M_FILE_DESCRIPTOR_R(n)`** — must have been opened for reading:

```c
//! \param[in] fd  File descriptor opened for reading (e.g., \c O_RDONLY or \c O_RDWR).
```

**`M_FILE_DESCRIPTOR_W(n)`** — must have been opened for writing:

```c
//! \param[in] fd  File descriptor opened for writing (e.g., \c O_WRONLY or \c O_RDWR).
```

---

### Format strings (`FUNC_ATTR_PRINTF`, `FUNC_ATTR_SCANF`)

When `FUNC_ATTR_PRINTF(fmt, va)` is present, the parameter at position `fmt` is a printf-format string. Document it and add the literal-string security note:

```c
//! \param[in] format  printf-style format string. Must be a compile-time string literal —
//!   never a runtime-constructed string — to prevent format-string injection vulnerabilities.
//!   Followed by variadic arguments matching the conversion specifiers.
```

For `FUNC_ATTR_SCANF` or `FUNC_ATTR_SCANF_S`, apply the same literal-string requirement.

---

### Compile-time diagnostics (`M_DIAG_WARN`, `M_DIAG_ERROR`)

Both macros take two arguments: `(cond, msg)`.

- `cond` — a C boolean expression that Clang evaluates at compile time. **This is the authoritative specification of exactly when the constraint fires.** Read it literally and translate it into the Doxygen prose — do not just use the message string.
- `msg` — a human-readable string emitted in the compiler diagnostic. Use it as a starting point, but write natural English that references the parameter names by `\a name`.

#### Condition-to-prose translation guide

Most `cond` expressions follow a small set of patterns. Recognise the pattern first, then write the corresponding prose:

| Condition expression | Plain-English translation for Doxygen |
|---------------------|---------------------------------------|
| `param == 0` | "\a param must be non-zero." |
| `param > MAX / other` | "The product \a param × \a other must not overflow `size_t` (\a param must be ≤ `SIZE_MAX` / \a other)." |
| `param > RSIZE_MAX` | "Values of \a param exceeding `RSIZE_MAX` may fail at runtime." |
| `param == 0 \|\| (param & (param - 1)) != 0` | "\a param must be a non-zero power of two (e.g., 4, 8, 512, 4096)." |
| `ptr == M_NULLPTR && size != 0` | "\a ptr must not be `\c M_NULLPTR` when \a size is non-zero." |
| `a > b` | "\a a must be ≤ \a b." |
| `!condition` | "The condition described by `condition` must hold." |

**Never** copy a raw bitwise expression (e.g., `(alignment & (alignment - 1)) != 0`) verbatim into Doxygen prose — translate it to the human-readable equivalent ("non-zero power of two").

#### `M_DIAG_ERROR` → `\pre`

`M_DIAG_ERROR(cond, msg)` is a hard constraint; calling with `cond` true is a programming error. Each occurrence becomes a separate `\pre` entry derived from its `cond`, in the order they appear on the declaration.

#### `M_DIAG_WARN` → `\attention`

`M_DIAG_WARN(cond, msg)` is a soft constraint; calling with `cond` true produces a compiler warning. Each occurrence becomes a separate `\attention` paragraph, again translated from its `cond`.

#### Multiple diagnostics on one function — complete example

```c
// Source declaration (from impl_memory_safety.h):
M_NODISCARD M_FUNC_ATTR_MALLOC M_CALLOC_SIZE(1, 2) M_ALLOC_ALIGN(3) void* M_NULLABLE
    safe_calloc_aligned_impl(size_t count, size_t size, size_t alignment, ...)
    M_DIAG_ERROR(count == 0,                               "safe_calloc_aligned with count of zero is not allowed")
    M_DIAG_ERROR(size == 0,                                "safe_calloc_aligned with size of zero is not allowed")
    M_DIAG_ERROR(count > (SIZE_MAX / size),                "safe_calloc_aligned size * count overflows")
    M_DIAG_WARN((count * size) > RSIZE_MAX,                "allocating more than RSIZE_MAX bytes may fail")
    M_DIAG_WARN(alignment == 0 || (alignment & (alignment - 1)) != 0,
                                                           "alignment should be a non-zero power of two")
    ;
```

Reading each `cond` from top to bottom and applying the translation guide produces these tags, in this order — `M_DIAG_ERROR` conditions become `\pre`, `M_DIAG_WARN` conditions become `\attention`:

```c
//! \fn void* safe_calloc_aligned_impl(size_t count, size_t size, size_t alignment, ...)
//! \brief Allocates aligned memory for an array with bounds checking.
//!
//! \param[in] count     Number of elements to allocate. Must be non-zero.
//! \param[in] size      Size of each element in bytes. Must be non-zero.
//! \param[in] alignment Memory alignment in bytes. Must be a non-zero power of two.
//!
//! \pre \a count must be non-zero. Passing zero is a compile-time error on constant arguments.
//! \pre \a size must be non-zero. Passing zero is a compile-time error on constant arguments.
//! \pre \a count × \a size must not overflow \c size_t (i.e., \a count must be ≤
//!   \c SIZE_MAX / \a size). Violating this is a compile-time error on constant arguments
//!   and produces undefined behavior at runtime.
//!
//! \attention Values of \a count × \a size exceeding \c RSIZE_MAX may fail at runtime even
//!   when they do not overflow — allocations larger than \c RSIZE_MAX are rejected by the
//!   bounds-checking layer.
//! \attention \a alignment must be a non-zero power of two (e.g., 4, 8, 512, 4096).
//!   Passing zero or a non-power-of-two value produces a compile-time warning on constant
//!   arguments and undefined behavior on some platforms at runtime.
//!
//! \return Pointer to the allocated memory block. Caller must free with \ref safe_free_aligned().
//!   Never returns \c M_NULLPTR on success.
//! \retval M_NULLPTR  Allocation failed (out of memory, or a constraint was violated at runtime).
```

#### Rules summary

- One `\pre` per `M_DIAG_ERROR`, one `\attention` per `M_DIAG_WARN` — in declaration order.
- When a condition spans two parameters (e.g., `count > SIZE_MAX / size`), mention both parameters and explain the mathematical relationship in plain English.
- When the condition is `X == 0 || (X & (X - 1)) != 0`, always write "must be a non-zero power of two" — never quote the bitwise expression.
- `\pre` entries come before `\attention` entries in the tag block. Both come before `\return`/`\retval`.
- Mirror the `msg` string in the `\pre`/`\attention` text only when it adds useful information beyond the translated condition; omit it when it would be redundant.

---

### Struct member bounds tracking

**`M_COUNTED_BY(member)`** — the flexible array's element count is tracked by `member`. Document both the count member and the array member, cross-referencing each other:

```c
typedef struct
{
    size_t   entryCount;                                               //!< Number of valid entries in \c entries.
    M_COUNTED_BY(entryCount) M_STRICT_FLEX_ARRAY_AUTO
    LogEntry entries[FLEX_ARRAY];  //!< Flexible array of \c entryCount \c LogEntry elements.
} LogPage;
```

**`M_COUNTED_BY_OR_NULL(member)`** — the pointer may also be null; note when null is valid:

```c
    M_COUNTED_BY_OR_NULL(dataLen) uint8_t* data; //!< Data buffer of \c dataLen bytes,
                                                  //!< or \c M_NULLPTR when no data is attached (\c dataLen must then be 0).
```

**`M_SIZED_BY(member)`** — like `M_COUNTED_BY` but `member` is the byte size, not element count. Note which unit is being used:

```c
    size_t   bufferBytes;              //!< Byte size of \c buffer.
    M_SIZED_BY(bufferBytes) uint8_t* buffer; //!< Data buffer of exactly \c bufferBytes bytes.
```

---

### Function purity and thread safety

**`M_CONST_FUNC`** — reads no global state, has no side effects. For the same inputs, always returns the same output. Add a thread-safety note when this is non-obvious:

```c
//! \note This function is pure computation with no side effects and reads no global state.
//!   It is safe to call concurrently from any thread without synchronization.
```

**`M_PURE_FUNC`** — no side effects, but may read global state. Result may differ across calls if globals change. Note this distinction:

```c
//! \note This function has no side effects. Its result depends on the input parameters
//!   and on global state, which may change between calls.
```

---

### Tainted arguments (`M_TAINTED_ARGS`)

When a function is marked `M_TAINTED_ARGS`, its arguments originate from untrusted external input (user command line, network, device response data). Add an `\attention` block:

```c
//! \attention All arguments to this function originate from untrusted external input.
//!   Callers must validate all values (range, alignment, non-zero) before passing them in.
//!   The static analyzer tracks tainted values from this function call forward.
```

---

### Checklist for attribute-driven documentation

When scanning a function declaration to write or review its Doxygen block:

1. **`M_PARAM_RO/WO/RW[_SIZE]`** → set `\param[in]`, `\param[out]`, `\param[in,out]`; link size parameters.
2. **`M_NONNULL/NULLABLE` on params** → note null constraints inline or as `\pre`.
3. **`M_NONNULL_IF_NONZERO_SIZE`** → document the conditional null relationship on both params.
4. **`M_NULL_TERM_STRING`** → note null-termination requirement in `\param[in]`.
5. **`M_RETURNS_NONNULL`** → add "never returns `M_NULLPTR`" to `\return` or `\retval`.
6. **`M_FUNC_ATTR_MALLOC` + `M_ALLOC_DEALLOC`** → document ownership and the exact deallocation function.
7. **`FUNC_ATTR_PRINTF/SCANF`** → note format-string nature and literal-string security requirement.
8. **`M_FILE_DESCRIPTOR_R/W`** → note fd pre-open requirement in `\param` and `\pre`.
9. **`M_DIAG_ERROR/WARN`** → translate each condition into `\pre` or `\attention`.
10. **`M_COUNTED_BY/SIZED_BY` on struct members** → cross-reference the count/size member in both `//!<` comments.
11. **`M_CONST_FUNC/M_PURE_FUNC`** → add thread-safety / purity note when non-obvious.
12. **`M_TAINTED_ARGS`** → add `\attention` flagging untrusted input.
13. **`M_NONSTRING`** on struct members → note non-null-termination and warn against string functions.

---

## Deprecated Functions

### Deriving `\deprecated` from the source macro

Two macros mark a function deprecated at the compiler level. When either is present on a function, the Doxygen block **must** also contain a `\deprecated` tag — Doxygen builds a dedicated deprecated-symbol index from these entries.

#### `M_DEPRECATED_REASON("reason string")` — copy verbatim

The string argument is the authoritative migration message. Copy it into `\deprecated` exactly as written. If the reason names or implies replacement functions, add `\sa` references to them:

```c
// In source:
M_DEPRECATED_REASON("use the bit width specific versions instead!")
M_NODISCARD bool get_And_Validate_Integer_Input(...);

// In Doxygen block — reason copied verbatim, \sa added for the implied replacements:
//! \deprecated Use the bit width specific versions instead.
//!   Prefer \ref get_And_Validate_Integer_Input_Uint64(),
//!   \ref get_And_Validate_Integer_Input_Uint32(),
//!   \ref get_And_Validate_Integer_Input_Uint16(), or
//!   \ref get_And_Validate_Integer_Input_Uint8() depending on the expected output range.
```

**When the reason string is vague** (names no specific function or call site), flag it to the human and propose a more informative replacement for both the macro string and the `\deprecated` tag. The goal is to give callers the exact call they need to migrate without reading the source:

- Identify the closest replacement function in the same file by matching return type and parameter types.
- Map old parameter names to the new function's parameters.
- For new optional parameters (e.g., an output `char** unit` pointer or an `eAllowedUnitInput` filter), use `M_NULLPTR` and the "none" enum value as safe defaults.
- Propose an improved macro string: `"use get_And_Validate_Integer_Input_Uint64(strToConvert, M_NULLPTR, ALLOW_UNIT_NONE, outputInteger) instead"`.
- **Ask the human to confirm before updating the macro string** — it is compiler-visible and changing it is a code change, not just a documentation change.

#### `M_DEPRECATED` (no reason string) — infer or ask

When no reason is given, use this strategy in order:

1. **Scan the same file** for a newer function with a similar name — common patterns: `_v2` suffix, bit-width-specific variants (`_Uint32`, `_Int64`), a `safe_` prefix, or a renamed form with clearer semantics.
2. **Check nearby comments or `\todo` entries** for migration notes left by the original author.
3. If a clear replacement is found: construct the most specific `\deprecated` message possible (following the "vague reason" guidance above) and **ask the human to confirm** before writing it.
4. If no replacement can be determined: **ask the human** — do not write a generic placeholder or `\todo`. A vague deprecation notice tells callers nothing useful.

Never leave `\deprecated` with an empty body.

### Tag placement

Place `\deprecated` immediately **after** `\brief` (and `\details` if present), before `\param` entries, so it is impossible to miss without scrolling:

```c
//! \brief One-sentence summary.
//!
//! \deprecated Use \ref replacement() instead.
//!
//! \param[in] ...
//! \return ...
//! \sa replacement()
```

### Complete example (`io_utils.h` — `M_DEPRECATED_REASON`)

Starting point — reason copied verbatim from the macro, `\sa` added for the replacement family:

```c
//! \fn bool get_And_Validate_Integer_Input(const char* strToConvert, uint64_t* outputInteger)
//! \brief Validates and converts a string to an unsigned integer.
//!
//! \deprecated Use the bit width specific versions instead.
//!   Prefer \ref get_And_Validate_Integer_Input_Uint64(),
//!   \ref get_And_Validate_Integer_Input_Uint32(),
//!   \ref get_And_Validate_Integer_Input_Uint16(), or
//!   \ref get_And_Validate_Integer_Input_Uint8() depending on the expected output range.
//!
//! \param[in] strToConvert The buffer to convert to an integer.
//! \param[out] outputInteger Pointer to store the converted value.
//! \return true if the string was a valid integer; false otherwise.
//!
//! \sa get_And_Validate_Integer_Input_Uint64(), get_And_Validate_Integer_Input_Uint32()
M_DEPRECATED_REASON("use the bit width specific versions instead!")
```

Improved — specific drop-in call included after human confirmation:

```c
//! \fn bool get_And_Validate_Integer_Input(const char* strToConvert, uint64_t* outputInteger)
//! \brief Validates and converts a string to an unsigned integer.
//!
//! \deprecated Use get_And_Validate_Integer_Input_Uint64(strToConvert, M_NULLPTR, ALLOW_UNIT_NONE, outputInteger)
//!   as a direct drop-in. For narrower output types see \ref get_And_Validate_Integer_Input_Uint32(),
//!   \ref get_And_Validate_Integer_Input_Uint16(), or \ref get_And_Validate_Integer_Input_Uint8().
//!
//! \param[in] strToConvert The buffer to convert to an integer.
//! \param[out] outputInteger Pointer to store the converted value. Valid only when this function returns \c true.
//!
//! \attention The return value must be checked. When this function returns \c false,
//!   \a outputInteger does not contain a valid result.
//!
//! \retval true  The string was successfully converted; \a outputInteger contains the result.
//! \retval false The string format is invalid; \a outputInteger is indeterminate.
//!
//! \sa get_And_Validate_Integer_Input_Uint64(), get_And_Validate_Integer_Input_Uint32()
M_DEPRECATED_REASON("use get_And_Validate_Integer_Input_Uint64(strToConvert, M_NULLPTR, ALLOW_UNIT_NONE, outputInteger) instead")
M_PARAM_RO(1)
M_NULL_TERM_STRING(1)
M_PARAM_RW(2)
M_NODISCARD_REASON("You must check this return value; if false, outputInteger does not contain a valid result")
bool get_And_Validate_Integer_Input(const char* M_NONNULL strToConvert,
                                    uint64_t* M_NONNULL   outputInteger);
```

---

## Nodiscard Documentation

`M_NODISCARD` and `M_NODISCARD_REASON("reason")` both generate compiler warnings when the return value is discarded. When either is present, the Doxygen block must reflect the consequence of ignoring the return value so the two sources of information stay consistent.

### `M_NODISCARD` — use `\retval` and document output-parameter validity

When a function is marked `M_NODISCARD`, replace any prose `\return` with `\retval` entries for each distinct return code. For each entry, state what output parameters contain:

```c
//! \retval true  Conversion succeeded; \a outputInteger contains the result.
//! \retval false String format is invalid; \a outputInteger is indeterminate.
```

If the `\param[out]` line does not already mention when the parameter is valid, add it inline:

```c
//! \param[out] outputInteger Pointer to store the result. Valid only when this function returns \c true.
```

### `M_NODISCARD_REASON("reason string")` — mirror verbatim in `\attention`

When the more specific macro is present, additionally mirror the reason string in an `\attention` tag placed immediately before the `\retval` entries. The `\attention` text is what a reader sees in the header; the compiler emits the reason string as a warning — both should convey the same message:

```c
//! \attention The return value must be checked. When this function returns \c false,
//!   \a outputInteger does not contain a valid result.
//!
//! \retval true  Conversion succeeded; \a outputInteger contains the result.
//! \retval false String format is invalid; \a outputInteger is indeterminate.
```

### Suggesting upgrades from `M_NODISCARD` to `M_NODISCARD_REASON`

When reviewing a function with bare `M_NODISCARD` and the consequence of discarding the return value is clear from the signature or the `\retval` documentation being written, AI may suggest upgrading to `M_NODISCARD_REASON("reason")`. This is **not** a hard requirement — ask the human to confirm the reason string before changing the macro. Make the reason string actionable: it will appear verbatim in the compiler warning:

```c
M_NODISCARD_REASON("You must check this return value; if false, outputInteger does not contain a valid result")
bool get_And_Validate_Integer_Input(const char* M_NONNULL strToConvert,
                                    uint64_t* M_NONNULL   outputInteger);
```

### Summary: keeping compiler output and Doxygen in sync

| Macro | Doxygen treatment |
|-------|------------------|
| `M_NODISCARD` | `\retval` for each distinct return value; `\param[out]` validity note |
| `M_NODISCARD_REASON("reason")` | `\attention` mirroring the reason string + `\retval` entries + `\param[out]` validity note |

---

## Code Examples

Every public API function must have at least one `\code{.c}...\endcode` example showing correct usage. Guidelines:

- Show the **full calling context**: declare variables, check return values, clean up.
- Use project types (`size_t`, `uint32_t`, `eReturnValues`) — not `int` for everything.
- Use `M_NULLPTR` (not `NULL`).
- Use `safe_free(&ptr)` (not `free(ptr)`).
- Show the error check — never leave out the `if (ret != SUCCESS)` case.

```c
//! \code{.c}
//!   size_t     bufSz = UINT32_C(512);
//!   uint8_t*   buf   = C_CAST(uint8_t*, safe_calloc(bufSz, sizeof(uint8_t)));
//!   if (buf == M_NULLPTR)
//!   {
//!       return MEMORY_FAILURE;
//!   }
//!
//!   eReturnValues ret = my_read_function(device, buf, bufSz);
//!   if (ret != SUCCESS)
//!   {
//!       safe_free(&buf);
//!       return ret;
//!   }
//!
//!   // process buf ...
//!   safe_free(&buf);
//! \endcode
```

---

## Module Grouping (`\defgroup` / `\ingroup`)

Groups organise the HTML output into logical sections. Each subsystem defines its group in its main header using `\defgroup`, then all other headers in that subsystem add themselves with `\ingroup`.

### Defining a group

Place this near the top of the primary header for each subsystem, below the file block:

```c
//! \defgroup MemorySafety Memory Safety Utilities
//! \brief Bounds-checked memory allocation, deallocation, and manipulation functions.
//!
//! All functions in this group are thin wrappers around the standard C allocation
//! functions that add zero-size guards, NULL-pointer checks, and atomic free-and-null
//! semantics. Always use these instead of the raw C standard library equivalents.
```

### Adding a file to a group

Place `\ingroup` inside the file's `\file` block, or at the top of each function/macro comment:

```c
//! \file safe_str.h
//! \ingroup StringOperations
//! \brief Bounds-checked string functions.
```

### Per-library group guidance

**opensea-common** — group by subsystem:
- `MemorySafety` — `memory_safety.h`, aligned/page allocation
- `StringOperations` — `safe_str.h`, string copy, concat, tokenise, dup
- `BitManipulation` — `bit_manip.h`, byte assembly, endianness conversion, byte/word extraction
- `TypeConversion` — `safe_io_utils.h`, `strtol`/`strtoull` family
- `PlatformDetect` — `predef_env_detect.h`, endianness macros, POSIX version macros
- `CodeAttributes` — `code_attributes.h`, portability macros

**opensea-transport** — group by protocol/layer:
- `ATATransport` — ATA command construction and passthrough
- `SCSITransport` — SCSI CDB construction, sense data parsing
- `NVMeTransport` — NVMe admin and I/O command sets
- `OSPassthrough` — OS-specific passthrough implementations (Windows, Linux, BSD, Solaris)
- `USBBridge` — USB bridge handling and quirk workarounds

**opensea-operations** — group by feature domain:
- `SMARTOperations`, `FirmwareOperations`, `EraseOperations`, `FormatOperations`, etc.

---

## Markdown, Tables & Advanced Doxygen Features

Doxygen supports Markdown (including extended Markdown features) and several advanced commands
that produce rich HTML output. Use these to document protocol register maps, command value
tables, state machines, and mathematical relationships that appear throughout ATA/SCSI/NVMe
specifications. Key references:

- Markdown extras: https://www.doxygen.nl/manual/markdown.html#markdown_extra
- Tables: https://www.doxygen.nl/manual/tables.html
- Images: https://www.doxygen.nl/manual/markdown.html#md_images
- Formulas: https://www.doxygen.nl/manual/formulas.html
- Autolinks: https://www.doxygen.nl/manual/autolink.html
- All commands: https://www.doxygen.nl/manual/commands.html

### Tables

Use Markdown pipe tables inside `\details` for register maps, byte-field breakdowns, and
command value descriptions. Prefer this over prose lists for tabular data:

```c
//! \brief Returns the status byte for the given device.
//!
//! \details
//! Status register bit definitions (ACS-4 Table 55):
//!
//! | Bit | Name | Description                                      |
//! |-----|------|--------------------------------------------------|
//! |  7  | BSY  | Device busy — ignore all other bits when set     |
//! |  6  | DRDY | Device ready to accept commands                  |
//! |  4  | DSC  | Seek complete (ATA-1; obsolete from ATA-3)       |
//! |  3  | DRQ  | Data transfer requested                          |
//! |  0  | ERR  | Previous command ended in an error               |
```

For complex tables requiring colspan, rowspan, or precise column widths, use raw HTML
inside `\htmlonly` / `\endhtmlonly`.

### Images

Use `\image html img.png "Caption"` for HTML output, or Markdown `![alt](path)`. Store
images under `docs/images/` with relative paths so the Doxygen site generator includes them:

```c
//! \image html ata_identify_layout.png "ATA IDENTIFY DEVICE word layout (ACS-3 Figure 2)"
```

### Formulas

Use `\f$ ... \f$` for inline math and `\f[ ... \f]` for displayed (block) equations.
Requires `USE_MATHJAX = YES` in the Doxyfile:

```c
//! \details
//! Transfer rate in bytes per second:
//! \f[ R = \frac{N \times 512}{t} \f]
//! where \f$ N \f$ is the sector count and \f$ t \f$ is transfer time in seconds.
```

### Flowcharts and Diagrams (`\mermaid` / `\endhermaid`)

Use `\mermaid` blocks for state machines, decision trees, and command sequences. Requires
Mermaid support in the Doxyfile. Particularly useful for documenting multi-step ATA/SCSI/NVMe
command flows that are difficult to express in prose:

```c
//! \details
//! Security erase command sequence:
//!
//! \mermaid
//! graph TD;
//!   A["Check securityEnabled"] -->|yes| B["Issue SECURITY ERASE PREPARE"];
//!   A -->|no| Z["Return NOT_SUPPORTED"];
//!   B --> C["Issue SECURITY ERASE UNIT"];
//!   C --> D["Read IDENTIFY DEVICE to verify"];
//! \endhermaid
```

### Autolinks and Cross-References

- Prefer `\ref symbol` to link to documented functions, types, and groups in prose text.
- Use `\sa` at the end of a comment block to list related symbols.
- Bare URLs in description text are made clickable automatically when `AUTOLINK_SUPPORT = YES`.

### Citations (`\cite`)

Use `\cite key` (with a configured BibTeX `.bib` file) to reference standards, coding rules,
or specifications inline. Especially useful when documenting `safe_*` functions that
correspond to C11 Annex K or ISO/IEC TS 17961 behaviours:

```c
//! \details
//! Implements the bounds-checked string copy specified in \cite C11AnnexK and enforces the
//! errno-zeroing rule from \cite ISOTS17961 before calling the underlying conversion.
```

### General Guidelines

- Always use `\details` to introduce extended descriptions — it unambiguously separates
  the detail body from `\brief` in the generated output. Never put long text directly
  after `\brief` without `\details`.
- Use `\important` to highlight notable points that are neither warnings nor caveats.
  Rendered as a distinct callout box separate from `\note` and `\warning`.
- Put tables, diagrams, and images inside `\details` to keep summaries and index pages compact.
- When documenting ATA/SCSI/NVMe register fields or command parameters, a table with
  columns for bit range/offset, field name, and description is the clearest format.
- Test formula and Mermaid rendering locally with the project Doxyfile before committing.
- Keep image files small and always provide a caption or `alt` text for accessibility.

---

## Cross-References

### `\sa` (see also)

List related functions at the end of a comment block. Use bare function names with parentheses:

```c
//! \sa safe_calloc(), safe_realloc(), safe_free()
```

### `\ref`

Inline link to another documented symbol within prose text:

```c
//! Call \ref ata_Security_Erase_Prepare() before invoking this function.
```

### `\copydoc`

When two functions or macros are documented identically (e.g., a macro wrapper and its underlying inline), copy the documentation from one to the other rather than duplicating it:

```c
//! \copydoc bytes_To_Uint32
#define M_BytesTo4ByteValue(b3, b2, b1, b0) bytes_To_Uint32(b3, b2, b1, b0)
```

---

## What Not To Do

| Anti-pattern | Why it's wrong | Correct approach |
|-------------|----------------|------------------|
| `\brief` longer than one line | It appears in summary tables — truncated badly | Split: one sentence in `\brief`, details in the following paragraph |
| `\return` prose listing every code | Hard to scan; codes buried in text | Use `\retval` for each distinct code |
| `\param name` with no direction | Reader cannot tell if the pointer is in or out | Always use `\param[in]`, `\param[out]`, or `\param[in,out]` |
| Documenting only in the `.c` file | Doxygen by default only processes headers | Document public API in `.h`; internal details in `.c` are supplementary |
| `//!<` on the line *above* the member | Doxygen attaches it to the *previous* symbol | `//!<` must be on the **same line** as the member |
| Leaving `\todo` without any description | Useless in the generated TODO list | `\todo` must state what is missing and ideally why it was deferred |
| Using `NULL` in examples | Inconsistent with project conventions | Use `M_NULLPTR` in all Doxygen code examples |
| No `\code` example for public API | Readers must guess correct usage | Every public API function needs at least one working example |
| `//!<` inside a macro argument list | The `//` comment terminates at the newline, cutting the macro call short | Use `/*!< description. */` block comments inside `M_DECLARE_ENUM` argument lists |
| `M_DECLARE_ENUM` enum with no Doxyfile preprocessing | Individual enum values are invisible to Doxygen; `/*!< ... */` comments are orphaned | Add `MACRO_EXPANSION = YES`, `EXPAND_ONLY_PREDEF = YES`, and `PREDEFINED` entries — see Doxyfile section below |
| `M_DEPRECATED_REASON("...")` present but no `\deprecated` tag | Function appears in the deprecated index with no explanation | Add `\deprecated` copying the macro string verbatim, then add `\sa` for implied replacements |
| Vague `M_DEPRECATED_REASON` when a specific call can be constructed from context | Callers must read the source to find the exact replacement call | Flag to the human; propose an improved macro string with the exact call (e.g., `"use foo(a, M_NULLPTR, ALLOW_UNIT_NONE, b) instead"`); confirm before changing |
| `M_DEPRECATED` present but no `\deprecated` tag | Same as above — and the reason must be inferred | Scan the file for replacement functions; write the most specific `\deprecated` message possible; if the replacement is unknown, ask the human |
| `M_NODISCARD` present but `\return` prose used instead of `\retval` | Output-parameter validity per return value is unclear to the reader | Replace `\return` with `\retval` for each distinct return code; add validity note to `\param[out]` |
| `M_NODISCARD_REASON("reason")` present but no `\attention` in Doxygen | Compiler warning and header docs convey different levels of detail | Add `\attention` mirroring the reason string verbatim before the `\retval` entries |
| `M_PARAM_RO(n)` present but `\param[in,out]` or `\param[out]` used | Direction tag contradicts the access-mode attribute | Fix tag to `\param[in]`; remove any out-parameter validity note from this parameter |
| `M_PARAM_WO(n)` present but `\param[in]` used | Caller-initialization requirement implied by `\param[in]` is incorrect | Fix to `\param[out]`; note that pre-initialization is not required |
| `M_PARAM_RW(n)` present but `\param[in]` or `\param[out]` used | Missing the in-place modification semantics | Fix to `\param[in,out]`; document what the function reads and what it overwrites |
| `M_NONNULL` on param but no non-null note in `\param` or `\pre` | Callers may not know null is forbidden | Add inline note "Must not be `\c M_NULLPTR`" or a `\pre` condition |
| `M_NULLABLE` on param but no note about what null means | Callers don't know whether null is ignored, returns an error, or triggers a no-op | Add "Pass `\c M_NULLPTR` to …" to the `\param` description |
| `M_NONNULL_IF_NONZERO_SIZE` present but conditional null not documented | The dependency between the pointer and size parameters is invisible to readers | Document both params: the pointer notes the size param, and the size param notes the pointer's null condition |
| `M_RETURNS_NONNULL` present but `\return`/`\retval` implies null is possible | Doxygen says null can be returned; attribute says it cannot | Remove the null-return case or rephrase; add "Never returns `\c M_NULLPTR`" |
| `M_FUNC_ATTR_MALLOC` present but `\return` doesn't mention caller must free | Callers may not know they own the allocation | Add "Caller must free with \ref safe_free()" (or the paired deallocation function) |
| `M_ALLOC_DEALLOC(fn, pos)` present but specific deallocation function not named in `\return` | Callers may use the wrong deallocation function, which crashes on Windows | Name the exact deallocation function in `\return`: "Must be freed with \ref fn()" |
| `FUNC_ATTR_PRINTF` present but format param not noted as a printf-format string | Callers don't know the security requirement to use string literals | Add "printf-style format string. Must be a string literal" to the `\param[in]` |
| `M_NULL_TERM_STRING(n)` present but param not noted as null-terminated | Callers may pass a non-terminated buffer without realizing | Add "Null-terminated string" to the `\param[in]` description |
| `M_NONSTRING` on struct member but member docs don't note non-null-termination | Code that passes the field to a string function compiles without warning | Add "NOT null-terminated; do not use with string functions" to the `//!<` comment |
| `M_FILE_DESCRIPTOR_R` or `M_FILE_DESCRIPTOR_W` present but access requirement not stated | Callers may pass a write-only or read-only descriptor and get confusing failures | Add the access-mode requirement to `\param[in]` and/or `\pre` |
| `M_DIAG_ERROR(cond, msg)` condition not reflected anywhere in Doxygen | The compile-time constraint is invisible until the compiler fires | Translate each `cond` expression into a `\pre` entry; use the condition-to-prose guide (e.g., `X == 0 || (X & (X-1)) != 0` → "must be a non-zero power of two") |
| `M_DIAG_WARN(cond, msg)` condition not reflected anywhere in Doxygen | Callers don't know what combination of arguments triggers the warning | Translate the `cond` expression into an `\attention` paragraph in the same way |
| Raw bitwise or arithmetic condition copied verbatim into `\pre`/`\attention` prose | Readers can't parse C expressions in documentation text | Translate to plain English: `param == 0 \|\| (param & (param-1)) != 0` → "must be a non-zero power of two" |
| `M_COUNTED_BY(member)` on flex array but neither member documents the relationship | The sizing invariant is invisible to readers | Cross-reference both members in their `//!<` comments |
| `M_TAINTED_ARGS` present but no `\attention` about untrusted input | Callers don't know they must validate inputs before forwarding | Add `\attention` noting that arguments come from untrusted input and must be validated |

---

## Recommended Doxyfile Settings

A `Doxyfile` exists at the repository root. The settings below summarise the most important options and explain the reasoning. Do not change them without understanding the impact.

```ini
OPTIMIZE_OUTPUT_FOR_C  = YES   # treats unions/structs/enums correctly for C; disables C++ class docs
TYPEDEF_HIDES_STRUCT   = YES   # typedef struct tFoo {...} tFoo; shows up as tFoo, not struct tFoo

EXTRACT_ALL            = NO    # only document symbols that have Doxygen comments — enforces completeness
EXTRACT_STATIC         = YES   # static M_INLINE functions in headers must be documented
EXTRACT_PRIVATE        = NO    # internal implementation details stay internal

WARN_IF_UNDOCUMENTED   = YES   # warn when a public symbol has no comment
WARN_NO_PARAMDOC       = YES   # warn when a documented function is missing a \param entry
WARN_LOGFILE           = doxygen_warnings.txt  # CI reads this file
EXIT_ON_FAIL_ON_WARNINGS = NO  # set to YES once all existing functions are fully documented

# Preprocessing — required for M_DECLARE_ENUM / M_DECLARE_ENUM_TYPE to be visible
# Without these settings Doxygen treats every M_DECLARE_ENUM call as an opaque macro and
# enum values plus their /*!< ... */ documentation comments are silently lost.
ENABLE_PREPROCESSING   = YES           # Doxygen default; listed here for clarity
MACRO_EXPANSION        = YES           # expand macros so enum bodies become visible
EXPAND_ONLY_PREDEF     = YES           # only expand the macros listed in PREDEFINED — safer than global expansion
PREDEFINED             = "M_DECLARE_ENUM(name,...)=typedef enum { __VA_ARGS__ } name" \
                         "M_DECLARE_ENUM_TYPE(name,type,...)=typedef enum { __VA_ARGS__ } name"
# Note: the simplified expansion omits the e_##name tag-paste from the real macro because
# Doxygen resolves the type name via the \enum tag placed above the M_DECLARE_ENUM call.
# TYPEDEF_HIDES_STRUCT = YES handles the typedef-vs-struct-name difference automatically.
```

**Graduation path**: once all existing public functions have complete `\param` and `\retval` documentation, flip `EXIT_ON_FAIL_ON_WARNINGS = YES`. After that, any PR that adds a public function without documentation will fail the Doxygen CI job — the same philosophy as `-Werror` in the build.
