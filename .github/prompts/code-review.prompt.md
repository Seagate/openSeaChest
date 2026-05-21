---
mode: ask
description: 'Structured code review for openSeaChest — checks safe_ usage, memory, integer safety, casts, verbosity, and platform guards'
---

Review the selected code (or the file in the editor if nothing is selected) against the openSeaChest secure coding standards. Work through each section below in order and report findings grouped by severity: 🔴 Critical, 🟡 Important, 🟢 Suggestion.

---

## 1. Safe Function Usage

- Are all `safe_*` calls present? Check for any raw use of: `malloc`, `calloc`, `realloc`, `free`, `strcpy`, `strncpy`, `strcat`, `strncat`, `strtok`, `memset`, `memcpy`, `memmove`, `fopen`, `tmpfile`, `gets`, `getline`, `strdup`, `strtol`, `atoi`, `atof`, `sprintf`, `vsprintf`.
- Is every `CONSTRAINT_NO_DISCARD` / `M_NODISCARD` return value either checked or wrapped in `M_IGNORE_SAFE_ERRNO_CALL` / `M_IGNORE_SAFE_INT_CALL` / `M_IGNORE_SAFE_PTR_CALL` with a non-empty justification string? Flag any bare discards, `(void)` casts, or `M_USE_UNUSED` used instead of the `M_IGNORE_SAFE_*` macros.
- Is the destination buffer size provided to every `safe_strcpy` / `safe_strcat` / `safe_memcpy` / `safe_memset` call? Flag any call missing the size argument.
- Is `safe_strdup` called correctly? Its signature is `errno_t safe_strdup(char** dup, const char* src)` — the allocated pointer comes out through the first argument, not the return value.
- Is `safe_getline` used instead of `getline`? The safe version returns `errno_t` and always calloc's the buffer.

## 2. Memory Management

- Does every `safe_free` call use `safe_free(&ptr)` (address of pointer)? Flag any `safe_free(ptr)` (missing `&`).
- Are aligned allocations freed with the matching free function?
  - `safe_malloc_aligned` / `safe_calloc_aligned` → `safe_free_aligned(&ptr)`
  - `safe_malloc_page_aligned` / `safe_calloc_page_aligned` → `safe_free_page_aligned(&ptr)`
  - Standard `safe_malloc` / `safe_calloc` → `safe_free(&ptr)`
  - Mismatched pairs crash on Windows.
- Does every allocation have a corresponding free on **every** exit path, including early error returns? Check for leaked allocations on error paths.
- For struct pointer types, is there a typed free helper that uses `safe_free(M_REINTERPRET_CAST(void**, &ptr))`?

## 3. Integer Overflow and Allocation Size Safety

- Is any allocation size computed by multiplying or adding values that originate from external sources (drive responses, user input, command-line arguments, network data)? If so:
  - Is the result calculated into a named variable before being passed to the allocator?
  - Is there an overflow check (e.g., `if (b != 0 && a > SIZE_MAX / b)`) before the multiplication?
  - Is the result checked to be non-zero and within a realistic range?
- Is any `uint32_t` value passed directly into a `size_t` parameter in an arithmetic expression? Flag these — they can silently overflow on 64-bit builds when the value is large.
- After calling `safe_strtol`, `safe_strtoul`, `safe_atoi`, etc., is `errno` (or the returned `errno_t`) checked for `ERANGE` before using the converted value?

## 4. Drive-Reported Data (Untrusted Input)

- Is any buffer allocated or indexed using a size, count, or offset that came directly from a drive response (IDENTIFY data, log pages, VPD pages, NVMe Identify structures, etc.)?
  - Is the value validated to be non-zero before use as a divisor?
  - Is the value validated to be within a realistic maximum before use as an allocation size or array index?
  - Is there an overflow check before multiplying it with `sizeof(struct)` or any other factor?
- Does any code divide by a value derived from drive geometry (logical sector size, physical sector size, sectors per track) without first checking for zero? Use `get_Logical_Sectors_Per_Physical_Sector(device)` for sector ratios — it never returns zero.

## 5. Cast Safety

- Are C-style casts `(type)(expr)` present? They should be replaced with `C_CAST`, `M_STATIC_CAST`, `M_REINTERPRET_CAST`, or `M_CONST_CAST`.
- Does any narrowing cast (wider type → narrower type) lack a preceding range check? E.g., casting `uint32_t` to `uint16_t` without first checking `val <= UINT16_MAX`.
- Is `M_CONST_CAST` used? If so, is there a comment justifying why `const` is being removed?

## 6. Output and Verbosity

- In `opensea-transport` or `opensea-operations` code: is `printf` used for non-debug output? It should be replaced with `print_tDevice_Verbose_String` or `print_tDevice_Verbose_Formatted_String`.
- Is any debug `printf` present outside of a `#if defined(_DEBUG)` guard?
- Is `puts` used anywhere? It should be `print_str` (which does not add a newline automatically).
- Is the verbosity level correct for the content?
  - `VERBOSITY_DEFAULT` — user-facing status messages
  - `VERBOSITY_COMMAND_VERBOSE` — CDB, task file register dumps
  - `VERBOSITY_BUFFERS` — raw data buffers to/from the drive

## 7. Error Handling and Control Flow

- Does every function that returns `eReturnValues` check the return of every operation that can fail, and propagate or handle the error?
- Is `SUCCESS` the only value tested with `== SUCCESS`? Avoid `!= FAILURE` — there are many non-SUCCESS, non-FAILURE return codes.
- For every `switch` on an `enum`, assess which pattern is correct based on the enum's annotation:
  - `M_OPEN_ENUM` (hardware/protocol command codes, ATA/SCSI/NVMe/ZAC operation codes, spec-reserved values): a `default` case returning `BAD_PARAMETER` is correct — future spec revisions may introduce codes this library doesn't know yet. Do **not** use `M_UNREACHABLE()` here.
  - `M_CLOSED_ENUM` (software-only, fully project-controlled): prefer exhaustive cases with no `default` so that `-Wswitch` fires automatically when the enum is extended. If a `default` is added for pragmatic reasons (many cases), it must return an error, never success.
  - Never add `default: break;` or `default: return SUCCESS;` just to silence a compiler warning — fix the missing cases instead.
  - `default: M_UNREACHABLE();` is legitimate **only** when a real outer guard (an `if`/`else` or range check) provably prevents every uncovered enum value from entering the switch. Verify the guard exists. If not, flag as 🔴 Critical — `M_UNREACHABLE()` on a reachable path is undefined behavior. Also check the macro is called with parentheses: `M_UNREACHABLE()`, not `M_UNREACHABLE`.
- Are there any `goto`, early returns, or `break`/`continue` paths that skip a necessary cleanup (free, close, unlock)?

## 8. Undefined, Unspecified, and Platform-Specific Behavior

- Is any function called that has a GNU-extended variant with a different signature or behavior from its POSIX counterpart (e.g., `strerror_r`, `basename`, `dirname`, `getline`)? These must use the opensea-common wrapper or be guarded with `#if defined(_GNU_SOURCE)` / `#if defined(__linux__)`.
- Is `memset`, `memcpy`, or `memmove` called with a size that could be zero? Zero-size calls were historically undefined. Use `safe_memset` / `safe_memcpy` which include an internal guard, or explicitly check `count > 0` before calling.
- Is any code calling a raw `strerror_r`, `strtok_r`, or similar function that opensea-common already wraps? Flag it and suggest the safe wrapper.
- Is any code depending on behavior that is only documented by one platform (e.g., Linux glibc behavior, Win32 behavior, FreeBSD behavior) without a corresponding compile-time guard?

## 9. Platform and Compiler Guards

- Is any OS-specific code properly wrapped in the correct preprocessor guard?
  - Windows: `#if defined(_WIN32)`
  - Linux: `#if defined(__linux__)`
  - FreeBSD: `#if defined(__FreeBSD__)`
  - Solaris/Illumos: `#if defined(__sun)`
- Is any code using POSIX-only APIs (e.g., `_aligned_malloc` vs `posix_memalign`) without a platform guard?
- Is any new dependency on a compiler extension guarded with `__has_attribute` / `__has_builtin` / `__has_c_attribute`? Never use GCC/Clang extensions without a fallback.

## 10. Endianness and Drive Wire Formats

- Is any struct cast directly over a raw drive response buffer (e.g., `(MyStruct*)responseBuffer`)? This is wrong: wire byte order differs from host byte order, and struct padding may not match the wire layout. Flag every such cast.
- Is any multi-byte field read from a SCSI/SAS response buffer without `be16_to_host` / `be32_to_host` / `be64_to_host`?
- Is any multi-byte field read from an ATA or NVMe response buffer without `le16_to_host` / `le32_to_host` / `le64_to_host`?
- Is any multi-byte field written into a SCSI/SAS command buffer without `host_to_be16` / `host_to_be32` / `host_to_be64`?
- Is any multi-byte field written into an ATA or NVMe command buffer without `host_to_le16` / `host_to_le32` / `host_to_le64`?
- Is `ENV_BIG_ENDIAN` / `ENV_LITTLE_ENDIAN` (from `predef_env_detect.h`) used for endian detection, rather than bare `__BYTE_ORDER`, `BYTE_ORDER`, or `__BIG_ENDIAN`?
- Is `byte_Swap_32` / `byte_Swap_16` / `byte_Swap_64` used where `be32_to_host` / `le32_to_host` would be more expressive and self-documenting?
- Are `M_BytesTo2ByteValue` / `M_BytesTo4ByteValue` / `M_BytesTo8ByteValue` used correctly — MSB first in the argument list for big-endian data, LSB last (i.e., reversed offset order) for little-endian data?
- Is `M_Byte0` / `M_Byte1` / `M_Byte2` / `M_Byte3` applied directly to a raw wire value without a preceding `be32_to_host` / `le32_to_host` conversion? These macros extract from a host-endian integer — using them on unconverted wire data produces wrong results on big-endian hosts.

## 11. Array Size and `sizeof` Misuse

- Is `sizeof(ptr)` used to determine the number of elements or byte count of a heap-allocated array? This always returns `sizeof(void*)` (4 or 8 bytes), not the allocation size. Flag any use of `sizeof` on a variable that holds a heap pointer.
- Is the raw `sizeof(arr) / sizeof(arr[0])` idiom used on a stack array instead of `SIZE_OF_STACK_ARRAY(arr)` from `memory_safety.h`? The macro is backed by `_Countof` / `_countof` where available and produces a compiler diagnostic if accidentally applied to a pointer.
- Is `SIZE_OF_STACK_ARRAY` applied to a heap pointer? Flag this — the macro's own documentation warns it must not be used on heap-allocated memory.

## 12. `goto` and Control Flow Structure

- Does the code contain any `goto` statement? Flag every occurrence as 🟡 Important unless it is in the one known ported FreeBSD function in `opensea-common`. Ask whether the function can be restructured to eliminate it.
- Is `goto` jumping forward over a variable initialisation? Flag as 🔴 Critical — skipping initialisation is undefined behavior in C.
- Is `goto` jumping backward to retry a block? Flag as 🟡 Important — convert to an explicit loop.
- Is every `if`, `else`, `for`, `while`, and `do` body enclosed in braces? A bracketless single-statement body is 🟡 Important — it is a merge-hazard even when syntactically correct. Exception: `case` labels in a `switch` do not require braces unless a local variable is declared within the case.
- Is there an intentional fall-through between `case` labels without `M_FALLTHROUGH`? Flag as 🟡 Important — silent fall-through suppresses `-Wimplicit-fallthrough` inconsistently across compilers and is easy to overlook during maintenance.

## 13. Null Pointer, Identifiers, and Banned Constructs

- Is `NULL`, `0`, or (in C++) `nullptr` used as a null pointer constant instead of `M_NULLPTR`? Flag all occurrences — `M_NULLPTR` from `common_types.h` is required for correct semantics across C99/C11/C23 and all supported compilers.
- Is `__FUNCTION__` used instead of `__func__`? `__FUNCTION__` is a pre-C99 MSVC extension. Use the C99 standard `__func__` identifier.
- Is there a VLA (variable-length array) declaration, i.e., `type arr[runtimeVar]`? Flag as 🔴 Critical — VLAs are banned: MSVC does not support them, and they silently corrupt the stack on overflow. `-Wvla` is active in all build configurations.
- Is `alloca()` called? Flag as 🔴 Critical — banned for the same reasons as VLAs: non-standard, no overflow protection, not consistently available across supported platforms.

## 14. Function-Like Macros vs Inline Functions

- Is new code introducing a function-like macro that performs computation, comparison, or argument evaluation? Flag as 🟡 Important — it should be a `static M_INLINE` function instead. Typed inline functions get compiler type checking, debugger visibility, and `M_NODISCARD` / `M_CONST_FUNC` annotations; macros get none of these.
- If the macro name must be kept for API compatibility, is it a thin wrapper that delegates to a typed inline (the `M_BytesTo4ByteValue` → `bytes_To_Uint32` pattern)? If not, suggest the refactor.
- If multi-type dispatch is needed, is `_Generic` (C11) used with typed inline functions behind a single macro name (the `M_Word0` pattern)? Bare `_Generic` on raw expressions without typed inline helpers is harder to maintain and loses argument evaluation safety.
- Are macros still used for compile-time constants, token pasting, stringification, or `_Generic` dispatch fronts? These are appropriate uses.

## 15. Code Attributes and Annotations

- Do new public functions in headers have appropriate annotations from `code_attributes.h`?
  - `M_NODISCARD` if the return value encodes an error code or an owned resource
  - `M_NONNULL` / `M_NULLABLE` on pointer parameters — use these (Clang nullability) on API boundaries, not `M_ALL_PARAMS_NONNULL` / `M_NONNULL_PARAM_LIST` which cause the optimizer to remove null pointer checks inside the function body
  - `M_PARAM_RO_SIZE` / `M_PARAM_WO_SIZE` on buffer parameters where size is a separate argument
  - `M_FUNC_ATTR_MALLOC` + `M_MALLOC_SIZE` on allocation functions
  - `M_TAINTED_ARGS` if the function directly accepts drive or user data
- Is `M_ALL_PARAMS_NONNULL` or `M_NONNULL_PARAM_LIST` applied to a public (non-static) function? Flag this — it should be `M_NONNULL` qualifiers on each parameter instead.
- Are attributes repeated on both the declaration (`.h`) and the definition (`.c`) for MSVC compatibility?

## 16. Final Summary

List all findings with file and line reference. For each 🔴 Critical and 🟡 Important finding, provide the corrected code. For 🟢 Suggestions, describe the improvement without requiring immediate action.
