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
//! \brief Performs an ATA Security Erase operation on the specified device.
//!
//! Issues the ATA SECURITY ERASE UNIT command sequence. This is a destructive
//! and irreversible operation that permanently removes all user data. On self-encrypting
//! drives this is near-instant; on conventional drives this takes hours.
//!
//! \param[in] device      Pointer to an initialised and opened \c tDevice structure.
//! \param[in] eraseType   Selects normal (\c ATA_SECURITY_ERASE_NORMAL) or enhanced
//!                        (\c ATA_SECURITY_ERASE_ENHANCED) erase mode.
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
//! \post On \c SUCCESS, all user data LBAs read as zero or are cryptographically
//!       inaccessible. The security password is cleared.
//!
//! \retval SUCCESS          The erase completed successfully.
//! \retval FAILURE          The drive reported a command error.
//! \retval NOT_SUPPORTED    The device does not support the ATA Security Feature Set,
//!                          or the interface does not allow this command (e.g., many
//!                          USB bridges block ATA security commands).
//! \retval PERMISSION_DENIED The drive is in the security locked state. Unlock before erasing.
//! \retval BAD_PARAMETER    \a device is \c M_NULLPTR.
//!
//! \warning This operation is irreversible. All user data will be permanently destroyed.
//!          Ensure the caller has obtained explicit user confirmation before invoking.
//!
//! \attention Some USB bridges silently drop ATA security commands or return success
//!            without actually executing them. Verify the result by reading the security
//!            status from IDENTIFY DEVICE after the operation completes.
//!
//! \par Platform:
//!   - Windows: requires administrator privileges; libATA on Linux may block this command
//!     unless the kernel parameter \c libata.allow_tpm=1 is set (the flag name is misleading).
//!   - USB: unreliable — many bridge chips do not pass through ATA security commands.
//!
//! \code{.c}
//!   // Check prerequisites, then erase with empty password
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

### Custom Paragraph: `\par Platform:`

For any function whose behaviour differs by OS or interface, add a `\par Platform:` section. The colon and space after the name are required by Doxygen:

```c
//! \par Platform:
//!   - Windows: requires STORAGE_PROTOCOL_COMMAND; not available on Windows 7/8.1 without driver workarounds.
//!   - Linux: requires kernel 4.4+ for NVMe passthrough via /dev/nvme*; earlier kernels use /dev/sg*.
//!   - FreeBSD: uses CAM; NVMe passthrough available from FreeBSD 12+.
//!   - USB: not applicable — this command requires direct ATA or NVMe passthrough.
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

---

## Deprecated Functions

Always include what to use instead and when the function was deprecated:

```c
//! \deprecated Since opensea-common 2.3.0.
//!   Use \c safe_strcpy(dest, destsz, src) instead, which performs bounds checking
//!   and returns an error code rather than silently truncating.
//!
//! \brief Copies \a src into \a dest without bounds checking.
//! ...
```

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
```

**Graduation path**: once all existing public functions have complete `\param` and `\retval` documentation, flip `EXIT_ON_FAIL_ON_WARNINGS = YES`. After that, any PR that adds a public function without documentation will fail the Doxygen CI job — the same philosophy as `-Werror` in the build.
