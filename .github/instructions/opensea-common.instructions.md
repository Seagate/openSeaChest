---
description: 'opensea-common library reference — header map, when-to-use guidance, and conventions for adding new cross-platform utilities'
applyTo: '**/*.c, **/*.h, **/*.cpp, **/*.hpp'
---

# opensea-common — Library Reference and Coding Conventions

## Purpose

opensea-common is the foundation layer of the opensea library stack. Its job is to provide the primitives that most modern languages (Python, Go, Rust) build in, but that portable C must recreate manually for every supported platform.

The library currently targets:

- **Compilers**: GCC ≥ 4.4, Clang ≥ 3.0, MSVC ≥ VS2013 (VS2015+ preferred — `printf`/`scanf` format macros for `<inttypes.h>` types do not expand correctly in TCHAR-family functions in VS2013), IAR, proprietary embedded toolchains
- **OS**: Windows Vista+; Linux; FreeBSD, NetBSD, OpenBSD, DragonFlyBSD; Solaris/Illumos; UEFI; AIX; HP-UX. macOS is supported by opensea-common only — opensea-transport requires OS-level passthrough IOCTLs that are not available on macOS without a kernel driver.
- **Architectures**: x86, x86-64, ARM (ARMv5+), ARM64/AArch64, PowerPC, SPARC, MIPS, RISC-V, and others. IA-64 and SystemZ are theoretically compatible but not regularly tested.
- **C standards**: C99 through C23 (falling back gracefully when new features are unavailable)

Before writing any cross-platform helper from scratch, check opensea-common first. The library exists precisely to avoid duplicating this effort.

---

## Module Map

Headers are grouped by functional area. Each entry describes the header, what it provides, and when to reach for it.

### 1. Foundation and Build Infrastructure

#### `code_attributes.h`

Portable attribute macros that map to the best available compiler extension, standard attribute, or SAL annotation. Include order: this header is pulled in by almost everything else; rarely include it directly.

Apply these attributes to every function wherever possible — the more annotations the compiler and static analyzers receive, the more they can verify for you:

| Macro | Purpose |
|-------|---------|
| `M_NULLABLE` / `M_NONNULL` | Nullability on **pointer** parameters only — do not apply to non-pointer parameters |
| `M_NODISCARD` | Warn when return value is discarded |
| `M_INLINE` / `static M_INLINE` | Inline hint without `__inline__` / `__forceinline` fragility |
| `M_FALLTHROUGH` | Intentional fall-through in a `switch` (no parentheses) |
| `M_UNREACHABLE()` | Mark provably unreachable paths (parentheses required) |
| `M_CLOSED_ENUM` / `M_OPEN_ENUM` | Signal software-controlled vs. hardware/protocol enums |
| `M_FUNC_ATTR_MALLOC` | Mark allocation functions for analyzer |
| `M_MALLOC_SIZE(n)` / `M_CALLOC_SIZE(n,m)` | Tell the compiler the allocated size |
| `M_PARAM_RO(n)` / `M_PARAM_WO(n)` / `M_PARAM_RW(n)` | Access-mode annotations for **pointer** parameters only — do not apply to non-pointer (value) parameters |
| `M_PARAM_RO_SIZE(ptr, sz)` / `M_PARAM_WO_SIZE(ptr, sz)` | Buffer pointer + associated size parameter |
| `M_NULL_TERM_STRING(n)` | Parameter must be a null-terminated string |
| `M_NONNULL_IF_NONZERO_SIZE(arg, sizearg)` | Parameter is non-null only when the paired size argument is non-zero |
| `M_COUNTED_BY(member)` | Flexible array member is sized by another struct member (element count) — **use on all flexible array members in passthrough structures** |
| `M_SIZED_BY(member)` | Pointer member is byte-sized by another struct member — **use on all pointer+size pairs in passthrough structures such as `ScsiIoCtx` and `nvmeCmdCtx`** |
| `M_RESTRICT` | Cross-compiler `restrict` |
| `M_CONST_FUNC` / `M_UNSEQUENCED` | Pure / no-side-effect function annotations |
| `M_DEPRECATED_REASON(msg)` | Mark deprecated APIs with a migration message |
| `M_DIAG_ERROR(cond, msg)` / `M_DIAG_WARN(cond, msg)` | Compile-time diagnostic when condition is true |
| `DISABLE_WARNING_*` / `RESTORE_WARNING_*` | Bracket unavoidable warning suppression |

**Priority annotations** — these have the highest impact on what tools can verify:
1. `M_NONNULL` / `M_NULLABLE` on every **pointer** parameter
2. `M_PARAM_RO` / `M_PARAM_WO` / `M_PARAM_RW` on every **pointer** parameter — do not apply to non-pointer (by-value) parameters such as `int`, `uint32_t`, `bool`, `size_t`, etc.
3. `M_COUNTED_BY` / `M_SIZED_BY` on every struct member that describes an array or buffer size

#### `predef_env_detect.h`

Compile-time platform detection macros. Prefer these over raw `#if defined(__linux__)` chains. Provides:
- `USING_C99`, `USING_C11`, `USING_C23`, `USING_CPP98`, etc.
- `POSIX_1990`, `POSIX_2001`, `POSIX_2008`
- `SYSTEM_WINDOWS`, `BSD4_2`, `SYSTEM_LINUX`
- `IS_GCC_VERSION(major, minor)`, `IS_CLANG_VERSION(major, minor)`, `IS_MSVC_VERSION(ver)`

**Use instead of**: raw compiler/OS macros scattered through code.

#### `env_detect.h`

Runtime (and compile-time) architecture and endianness detection:
- `get_Compiled_Architecture()` → `eArchitecture`
- `get_Compiled_Endianness()` → `eEndianness` (values: `OPENSEA_LITTLE_ENDIAN`, `OPENSEA_BIG_ENDIAN`, etc.)

**Use when**: you need to make a runtime decision based on architecture or endianness (e.g., picking a different code path for SPARC vs. x86-64).

**Note on endianness**: The macros in `env_detect.h` are intended for "under the hood" use and for informational queries. For actually converting device data between wire endianness and host endianness, **always use the functions in `bit_manip.h`** (`be16_to_host`, `le32_to_host`, etc.) rather than raw `#if` checks on these values.

#### `common_types.h`

The universal type include. Pulls in `<stdint.h>`, `<stdbool.h>` (with C99 fallback), `<inttypes.h>`, `<sys/types.h>`, and platform compatibility shims:
- `bool`, `true`, `false` — portable across C99–C23
- `uid_t`, `gid_t` — defined for Windows so code can use POSIX idioms
- `M_NULLPTR` — expands to `nullptr` (C23/C++11), `__nullptr` (Clang), or `((void*)0)`
- `_FILE_OFFSET_BITS 64` — large-file support on 32-bit hosts, always enforced
- `eReturnValues` — the universal error/return code enum (`SUCCESS`, `FAILURE`, `NOT_SUPPORTED`, `BAD_PARAMETER`, `MEMORY_FAILURE`, `PERMISSION_DENIED`, etc.)
- `rsize_t` — bounds-checked size type used by safe_ functions

**Include order**: include early; most other headers pull it in transitively.

#### `warning_ctl.h`

Macros for cross-compiler push/pop warning suppression: `DISABLE_WARNING_4255`, `RESTORE_WARNING_4255`, `DISABLE_WARNING_CONVERSION_DATA_LOSS`, etc. Use these instead of raw `#pragma warning` or `_Pragma` strings so the same code compiles cleanly on MSVC, GCC, and Clang.

**Rule**: Use these **only when no other solution exists**. They exist for cases where a particular compiler/libc combination differs just enough that a conversion or type mismatch warning appears even though the code is correct and safe on all platforms. For example, `ioctl` in musl libc has a slightly different argument type than glibc, causing a conversion warning even when the value being passed is identical and correct — suppressing that specific warning for that call site is the only viable fix.

**Always pair** a DISABLE with its matching RESTORE immediately around the one problematic line. Never suppress a warning file-wide.

#### `constraint_handling.h`

The constraint-handler subsystem that **most** (but not all) `safe_*` functions call when they detect a violation. Provides `set_Constraint_Handler()` with `ERR_ABORT` (default, most secure), `ERR_WARN`, and `ERR_IGNORE`. The handler receives `constraintEnvInfo` containing `__FILE__`, `__func__`, `__LINE__`, and the expression string, enabling precise error location reporting.

Note: some functions deliberately do not invoke the constraint handler — for example `safe_isascii` simply returns a defined result without triggering a handler. This is intentional for functions where a missing or out-of-range value is a normal condition rather than a programming error.

**Rule**: Do not change the constraint handler in library code. Only `main()` / application entry points should configure it.

**Exception**: `secured_env_vars.h` temporarily changes and then restores the constraint handler inside `get_Environment_Variable` to handle the case where an environment variable might not exist (e.g., `SUDO_USER` is only set when running under `sudo`, not as a regular user or root). Rather than crashing via `ERR_ABORT` simply because the variable is absent, the function locally switches to `ERR_IGNORE`, performs the lookup, and restores the original handler before returning.

---

### 2. Memory Management

#### `memory_safety.h`

All allocation and memory operations go through here. See `c-secure-coding.instructions.md` for the complete list and rationale.

| Pattern | Use |
|---------|-----|
| `safe_malloc(size)` | Single object allocation |
| `safe_calloc(count, size)` | Array allocation, zero-initialized, overflow-checked |
| `safe_realloc(ptr, size)` | Resize (leaves `ptr` unchanged on failure) |
| `safe_reallocf(&ptr, size)` | Resize (NULLs `ptr` on failure — use when the old ptr is no longer needed) |
| `safe_free(&ptr)` | Free + NULL in one call; prevents double-free |
| `safe_malloc_aligned(size, alignment)` | Power-of-2 aligned allocation (e.g., 512-byte aligned for passthrough buffers) |
| `safe_calloc_aligned(count, size, alignment)` | Power-of-2 aligned array allocation, zero-initialized |
| `safe_malloc_page_aligned(size)` | OS page-aligned allocation |
| `safe_calloc_page_aligned(count, size)` | OS page-aligned array allocation, zero-initialized |
| `safe_memset(dest, destsz, val, count)` | Bounds-checked memset; guards against zero-size |
| `safe_memcpy(dest, destsz, src, count)` | Bounds-checked memcpy (overlap-tolerant, like `memmove`) |
| `safe_memcpy_no_overlap(dest, destsz, src, count)` | Faster bounds-checked memcpy — use only when you can **guarantee** no pointer overlap |
| `safe_memmove(dest, destsz, src, count)` | Bounds-checked memmove (always overlap-safe) |
| `safe_memcmp(s1, s1max, s2, s2max, n)` | Bounds-checked memcmp |
| `explicit_zeroes(dest, count)` | Zero-fill with compiler optimization barrier — use for passwords, keys, sensitive data; cannot be elided by the optimizer |
| `SIZE_OF_STACK_ARRAY(arr)` | Element count of a stack array — backed by `_Countof`/`sizeof/sizeof*` |

**Critical rules**:
- `safe_free` takes a **pointer to pointer** (`&ptr`) and sets the pointer to `M_NULLPTR` after freeing, preventing double-free and use-after-free. This looks like a dead store to some static analyzers — it is intentional and follows CERT-C guidance. For non-built-in types (structs, typedefs that are not plain ints/chars/etc.), you must write a type-specific wrapper that casts to `void**` to avoid compiler errors; see existing `safe_free_*` wrappers in the codebase for examples.
- Never call `malloc`, `calloc`, `realloc`, or `free` directly.
- `explicit_zeroes(ptr, size)` instead of `memset` when the data is sensitive — the optimizer cannot elide it.
- `safe_realloc` / `safe_reallocf` with a size of zero will free the memory for you (defined behavior in the safe_ versions, unlike the undefined behavior in the standard `realloc`). The aligned realloc variant (`safe_realloc_aligned(block, originalSize, size, alignment)`) requires you to supply the **original allocation size** in addition to the new size, because the implementation may allocate fresh memory, copy, and free — it cannot infer the original size without your help. No page-aligned realloc variant exists.
- Use `safe_malloc_aligned` / `safe_calloc_aligned` for I/O buffers that the OS passthrough layer requires to be physically aligned (e.g., Windows `DeviceIoControl` direct buffers). The naming pattern throughout is `safe_<verb>_aligned` — never `safe_aligned_<verb>`.
- Use `safe_memcpy_no_overlap` instead of `safe_memcpy` when you can guarantee the source and destination buffers do not overlap — the compiler can then emit faster non-`memmove` code.

---

### 3. Type Conversions and Casts

#### `type_conversion.h`

Searchable cast macros that make type coercions visible in code review and static analysis:

| Macro | Equivalent | When to use |
|-------|-----------|-------------|
| `C_CAST(type, val)` | `(type)(val)` | C-only code where C++ compat is not a concern; avoid in headers shared with C++ |
| `M_STATIC_CAST(type, val)` | `static_cast<T>` in C++, C cast in C | Widening/narrowing where types are compatible — **prefer over `C_CAST`** |
| `M_REINTERPRET_CAST(type, ptr)` | `reinterpret_cast<T>` in C++, C cast in C | Pointer type aliasing (e.g., `uint8_t*` → `uint32_t*`) — use sparingly |
| `M_CONST_CAST(type, val)` | `const_cast<T>` in C++, uintptr_t round-trip in C | Removing `const` / `volatile` — justify in a comment |
| `M_ToBool(expr)` | `(expr) > 0 ? true : false` | Silence C++ conversion warnings on bool assignment |

**Prefer `M_STATIC_CAST` and `M_REINTERPRET_CAST` over `C_CAST`** in any code that might be compiled by a C++ compiler. Some projects ban C-style casts at the C++ level and `C_CAST` would violate that constraint. The typed variants are also more descriptive of intent during code review.

Safe widening functions for use in size/index calculations where signed-to-unsigned conversion would warn:

- `uint8_to_sizet`, `uint16_to_sizet`, `uint32_to_sizet`, `int8_to_sizet`, `int16_to_sizet`, `int32_to_sizet`, etc.
- These produce compile-time `M_DIAG_WARN` if the value might truncate and set `errno = ERANGE` if it does at runtime.

**Rule**: Never write a raw C cast without a cast macro. These macros are specifically designed to be `grep`-able during security audits.

---

### 4. Strings and Characters

#### `string_utils.h`

All string operations go through here. See `c-secure-coding.instructions.md` for the complete list.

| Function | Purpose |
|----------|---------|
| `safe_strlen(s)` / `safe_strnlen(s, n)` | Null-safe length |
| `safe_strcpy(dst, dstsz, src)` | Bounds-checked copy (routes to `safe_strmove` by default) |
| `safe_strncpy(dst, dstsz, src, n)` | Bounds-checked n-copy |
| `safe_strcat(dst, dstsz, src)` | Bounds-checked concatenation |
| `safe_strncat(dst, dstsz, src, n)` | Bounds-checked n-concatenation |
| `safe_strtok(str, strmax, delim, &saveptr)` | Reentrant tokenization (no hidden global state) |
| `safe_strdup(&dup, src)` | Allocating duplicate |
| `safe_strndup(&dup, src, len)` | Allocating n-duplicate |
| `safe_is*(c)` | Character classification: `safe_isalpha`, `safe_isdigit`, `safe_isspace`, etc. — guard against out-of-range `c` |
| `safe_tolower(c)` / `safe_toupper(c)` | Case conversion with range check |
| `strcasecmp(s1, s2)` | Case-insensitive compare — maps to `_stricmp` on Windows, `strcasecmp` elsewhere |
| `strncasecmp(s1, s2, n)` | n-char case-insensitive compare |

**Important flag**: By default `safe_strcpy`/`safe_strcat` route through `safe_strmove`/`safe_strcatmove` which tolerates overlapping source and destination buffers (like `memmove`). Define `STRCPY_IS_STRCPY_NOT_STRMOVE` to get strict non-overlapping semantics (like C11 Annex K). Use `safe_strcpy_no_overlap` / `safe_strcat_no_overlap` when you know buffers do not overlap and want maximum performance.

---

### 5. I/O, File Access, and Environment

#### `io_utils.h`

Printf wrappers and integer/unit parsing for command-line tools. Key functions:

- `get_And_Validate_Integer_Input_Uint64(str, &unit, unittype, &out)` — parse decimal/hex user input with optional unit suffix (bytes, sectors, time, power, temperature). Prefer this over `sscanf` or `atoi` for CLI argument parsing.
- `get_And_Validate_Integer_Input_Uint32` / `Uint16` / `Uint8` — bit-width-specific versions.
- `eAllowedUnitInput` — controls which unit suffixes are accepted: `ALLOW_UNIT_NONE`, `ALLOW_UNIT_DATASIZE`, `ALLOW_UNIT_TIME`, `ALLOW_UNIT_TEMPERATURE`, etc.
- `get_And_Validate_Integer_Input()` is deprecated — use the bit-width-specific versions.

**Use when**: processing user-supplied integer arguments, especially those with data-size or time units.

#### `secure_file.h`

CERT-C compliant file operations. Provides `safe_fopen`, `safe_freopen`, `safe_tmpfile`, plus `S_ISREG`/`S_ISDIR`/`S_ISCHR` macros for Windows (which lacks them). Always use `safe_fopen` instead of `fopen` to validate the path resolves to a regular file and is not a device or symlink pointing outside the intended directory.

#### `secured_env_vars.h`

CERT-C compliant environment variable access:
- `get_Environment_Variable(name, &value)` → `eEnvVarResult` — detects tampered environments (`ENV_VAR_TAMPERED_ENV_DETECTED`), allocates a copy that must be freed.

**Rule**: Never call `getenv()` directly. Always use `get_Environment_Variable`.

#### `error_translation.h`

Translate OS error codes to human-readable strings:
- `get_strerror(errno)` — cross-platform `strerror_r`/`strerror_s` wrapper (allocates)
- `print_Errno_To_Screen(errno)` — prints formatted error
- `get_windows_error_str(err)` — Windows `GetLastError()` translation (Windows only)
- `print_Windows_Error_To_Screen(err)` — prints Windows error

**Rule**: Never call `strerror()` directly — it is not thread-safe. Use `get_strerror()`.

---

### 6. Bit Manipulation

#### `bit_manip.h`

Drive data is almost entirely byte-field oriented. This header provides the standard bit/byte extraction and assembly macros used throughout the transport layer.

**Byte extraction (use for parsing device responses):**

| Macro / Function | Returns |
|----------------|---------|
| `get_Byte0(val)` / `M_Byte0(val)` | bits [7:0] |
| `get_Byte1(val)` / `M_Byte1(val)` | bits [15:8] |
| `get_Byte2(val)` / `M_Byte2(val)` | bits [23:16] |
| `get_Byte3(val)` / `M_Byte3(val)` | bits [31:24] |
| `get_Byte4(val)` … `get_Byte7(val)` | bits [39:32] … [63:56] |
| `get_DWord0(val)` / `M_DoubleWord0(val)` | lower 32 bits of a 64-bit value |
| `get_DWord1(val)` / `M_DoubleWord1(val)` | upper 32 bits of a 64-bit value |
| `get_Word0(val)` / `M_Word0(val)` | bits [15:0] |
| `get_Word1(val)` / `M_Word1(val)` | bits [31:16] |

**Multi-byte assembly (use for building command fields):**

| Macro | Purpose |
|-------|---------|
| `M_BytesTo2ByteValue(b1, b0)` | Assemble a `uint16_t` from two bytes (b1 = MSB) |
| `M_BytesTo4ByteValue(b3, b2, b1, b0)` | Assemble a `uint32_t` from four bytes |
| `M_BytesTo8ByteValue(b7, b6, b5, b4, b3, b2, b1, b0)` | Assemble a `uint64_t` from eight bytes |
| `M_WordsTo4ByteValue(w1, w0)` | Assemble a `uint32_t` from two `uint16_t` words |
| `M_DWordsTo8ByteValue(d1, d0)` | Assemble a `uint64_t` from two `uint32_t` double-words |

**Byte-swap (endianness correction):**
- `byte_Swap_16(v)`, `byte_Swap_32(v)`, `byte_Swap_64(v)` — unconditional reversal
- `M_ByteSwap_Uint16_BE_To_Host(v)` etc. — conditional swap from a specific wire endianness to host (use for ATA/SCSI response parsing)

**Important rule**: When a value from a device response is stored in a `uint16_t` or larger field, **always convert first** (apply `byte_Swap_*` or `M_BytesTo*`), then extract the byte sub-fields. Do not pass a multi-byte value directly to `M_Byte0` etc. — see Section 12 of `c-secure-coding.instructions.md`.

---

### 7. Mathematics

#### `math_utils.h`

Basic math utilities that are surprisingly tricky to do portably in C:

| Macro / Function | Purpose |
|----------------|---------|
| `M_Min(a, b)` / `M_Max(a, b)` | Branchless min/max (expression-safe macro) |
| `M_2sCOMPLEMENT(val)` | Two's complement negation (type-preserving with `typeof` when available) |
| `uint8_round_up_generic(v, n)` | Round `v` up to the nearest multiple of `n` (generic, all bit widths available) |
| `uint8_round_up_power2(v, n)` | Round up to power-of-2 multiple (faster, requires `n` is a power of 2) |

All round-up functions are available for `uint8`, `uint16`, `uint32`, `uint64`, `int8`, `int16`, `int32`, `int64`. Choose the matching bit width to avoid implicit conversion warnings.

---

### 8. Sorting and Searching

#### `sort_and_search.h`

Bounds-checked replacements for `qsort`, `bsearch`, `lsearch`, `lfind`. All standard functions have known weaknesses (zero-count misuse, null-pointer UB on empty arrays). Use these instead:

```c
// Sort a list of device handles by serial number
safe_qsort(handles, handleCount, sizeof(handles[0]), compare_by_serial);

// Find a drive record by LBA range
const driveRecord* found = M_REINTERPRET_CAST(const driveRecord*,
    safe_bsearch(&key, table, tableCount, sizeof(table[0]), compare_record));
```

Context variants (`safe_qsort_context`, `safe_bsearch_context`) accept a `void* context` parameter that is passed to the comparison function — use these to avoid global state.

`comparefn` and `ctxcomparefn` typedefs are provided; use them rather than raw function-pointer types.

---

### 9. Buffer Patterns

#### `pattern_utils.h`

Fills a `uint8_t` buffer with a repeating pattern — used heavily in drive test operations (write-pattern, then read-back and compare):

| Function | Pattern |
|----------|---------|
| `fill_Random_Pattern_In_Buffer(buf, len)` | Pseudo-random bytes |
| `fill_Hex_Pattern_In_Buffer(pattern, buf, len)` | Repeating 32-bit hex value |
| `fill_Incrementing_Pattern_In_Buffer(start, buf, len)` | 0x00–0xFF repeating from `start` |
| `fill_ASCII_Pattern_In_Buffer(str, slen, buf, len)` | Repeating ASCII string |
| `fill_Pattern_Buffer_Into_Another_Buffer(src, srclen, dst, dstlen)` | Repeat a smaller buffer to fill a larger one |

---

### 10. Timing

#### `precision_timer.h`

High-resolution cross-platform timer. Always use this to measure operation durations:

```c
DECLARE_SEATIMER(cmdTimer);   // stack-allocated, zero-initialized
start_Timer(&cmdTimer);
// ... issue command ...
stop_Timer(&cmdTimer);
double elapsed_ms = get_Milli_Seconds(cmdTimer);
```

- `get_Nano_Seconds(t)` → `uint64_t`
- `get_Micro_Seconds(t)` → `double`
- `get_Milli_Seconds(t)` → `double`
- `get_Seconds(t)` → `double`
- `DECLARE_SEATIMER(name)` — stack allocation (preferred)
- `NEW_SEATIMER(name)` + `safe_free_seatimer(&name)` — heap allocation

#### `sleep.h`

Cross-platform delay functions:

| Function | Precision | Notes |
|----------|-----------|-------|
| `sleepns(ns)` | ~100 ns on Windows, ns on POSIX | Returns `ENOSYS` if not available |
| `sleepus(us)` | microseconds | |
| `sleepms(ms)` | milliseconds | |
| `delay_Milliseconds(ms)` | deprecated alias for `sleepms` | |

**Rule**: Never call `Sleep()` (Windows), `usleep()`, or `nanosleep()` directly.

#### `time_utils.h`

Safe wrappers for `gmtime`, `localtime`, `ctime`, `asctime` — the reentrant versions wrapped cross-platform:
- `safe_gmtime(timer, buf)` / `get_UTCtime(timer, buf)` — UTC time in a caller-provided `struct tm`
- `safe_localtime(timer, buf)` — local time
- `safe_ctime(timer, buf, bufsz)` — formatted time string
- `get_current_timestamp()` — sets the global `CURRENT_TIME` and `CURRENT_TIME_STRING` (call once at program start)
- `get_Milliseconds_Since_Unix_Epoch()` → `uint64_t` — milliseconds since the Unix epoch; this is the standard time base for HDD time-programming commands (power condition timers, background scan intervals, scheduled operations, etc.)
- `milliseconds_Since_Unix_Epoch_To_Struct_TM(milliseconds, &tm)` — convert an epoch-millisecond value to a `struct tm` for display

**Rule**: Never call `gmtime()`, `localtime()`, `ctime()`, or `asctime()` directly — they use non-reentrant global state.

---

### 11. Unit Conversion

#### `unit_conversion.h`

Human-readable drive capacity and physical measurement display:

```c
double bytes = M_STATIC_CAST(double, device.drive_info.deviceMaxLba) * logicalSectorSize;
char unit[UNIT_STRING_LENGTH];
metric_Unit_Convert(&bytes, &unit);          // "7.28 TB" style (SI, power of 10)
// or:
capacity_Unit_Convert(&bytes, &unit);        // "6.82 TiB" style (IEC, power of 2)
printf("Capacity: %.2f %s\n", bytes, unit);
```

Temperature conversion: `celsius_To_Fahrenheit()`, `fahrenheit_To_celsius()`, `celsius_To_Kelvin()`, `kelvin_To_Celsius()`, `kelvin_To_Fahrenheit()`, `fahrenheit_To_Kelvin()`.

---

### 12. Pseudo-Random Number Generation

#### `prng.h`

XOR-shift+ based PRNG — suitable for test pattern generation and fuzz testing, **not** for cryptographic purposes:

```c
seed_32(M_STATIC_CAST(uint32_t, time(M_NULLPTR)));
uint32_t val = random_Range_32(0, UINT32_C(0xFFFF));   // [0, 65535]
```

- `seed_32` / `seed_64` — seed independently; call once at startup
- `xorshiftplus32()` / `xorshiftplus64()` — raw next value
- `random_Range_32(min, max)` / `random_Range_64(min, max)` — bounded output

---

### 13. Version Sorting

#### `version_sort.h`

GNU `versionsort`-compatible comparison function for `scandir`, portable across BSDs where the signature differs between releases:

```c
// Sort device entries with version-aware order
scandir("/dev/disk/by-id", &entries, filter, version_sort);
```

Abstracts the `const struct dirent**` vs. `const void*` prototype difference between old and new BSD `scandir` implementations via the `NEED_OLD_SCANDIR_CMP_FUNC_TYPE` guard.

---

## Adding a New Utility to opensea-common

Before adding a new function:

1. **Verify it does not already exist.** Search for the concept across all headers. The library is comprehensive and a near-identical function may already be there under a slightly different name.

2. **Confirm it is genuinely cross-platform.** If the function only matters on Windows, it belongs in `win_helper.c` in opensea-transport, not here. If it only matters on one OS, add it conditionally inside an existing file.

3. **Choose the right header.** Add to an existing themed header rather than creating a new one. Create a new header only when the functionality is distinct enough to form its own coherent group (e.g., a new protocol's helper functions) and the group will have at least 3–5 related functions.

4. **Follow the dual `impl_` pattern.** Public header goes in `include/foo.h`. Implementation details and internal helpers that do not need to be part of the public API go in `include/impl_foo.h` (included by `foo.h`). See `impl_memory_safety.h`, `impl_string_utils.h`, `impl_io_utils.h` for the pattern. This keeps public headers clean and avoids exposing implementation types.

5. **Add `DEV_ENVIRONMENT` duality** if the function is a `safe_*` variant. In DEV builds the function is an `M_INLINE` that captures `__FILE__`, `__func__`, `__LINE__` for constraint-handler messages. In release builds it is a macro that does the same via `##`. See how `safe_malloc` / `safe_memset` are defined in `memory_safety.h`.

6. **Add full Doxygen documentation** following `doxygen.instructions.md`. Every public symbol needs `\file`, `\brief`, `\param[in/out]`, `\return`/`\retval`, and a `\code{.c}` example.

7. **Add `M_DIAG_ERROR`/`M_DIAG_WARN` annotations** for pre-conditions. If the function has a constraint (e.g., count must be non-zero, pointers must be non-null), use `M_DIAG_ERROR` or `M_DIAG_WARN` on the declaration so the compiler or static analyzer can flag violations at the call site. See `math_utils.h` for examples.

8. **Invoke the constraint handler** (not `assert`) inside the function body for runtime-detectable violations. Call `invoke_Constraint_Handler(...)` from `constraint_handling.h`.

9. **Do not add generated build artifacts** (autoconf output, baked tables) to the source tree.

---

## What NOT to Add to opensea-common

- **Protocol-specific types or parsing** (ATA registers, SCSI CDBs, NVMe command structures) — those belong in opensea-transport
- **Device-operation logic** (DST, firmware download, sanitize workflows) — those belong in opensea-operations
- **Anything that requires linking a protocol or OS passthrough library** — opensea-common has no such dependencies
- **Global mutable state**, except for well-justified cases like `CURRENT_TIME` / `CURRENT_TIME_STRING` and the PRNG seed arrays (which are explicitly documented as non-thread-safe)
- **Cryptographic primitives** — use an established library; `prng.h` is for test data only
