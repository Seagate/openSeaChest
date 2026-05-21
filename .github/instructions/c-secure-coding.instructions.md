---
description: 'Project-specific C secure coding rules for openSeaChest — common mistakes, safe_ function patterns, cast conventions, memory management, integer safety, platform portability, and nullability annotations'
applyTo: '**/*.c, **/*.h, **/*.cpp, **/*.hpp, **/*.cc, **/*.cxx'
---

# C Secure Coding — openSeaChest

This file documents the most common coding mistakes in this codebase and how to avoid them. It supplements the safe_ function reference in the main `copilot-instructions.md` with project-specific patterns and anti-patterns derived from real bugs and audits.

---

## 1. Safe Function Return Values

### Always Check or Explicitly Discard

All `safe_*` functions whose return value matters are annotated with `CONSTRAINT_NO_DISCARD` (a condition-dependent alias for `M_NODISCARD`). Silently discarding the return value produces a warning. Three macros exist for intentional discards — choose the one that matches the return type:

| Macro | For return type | Assert condition |
|-------|----------------|-----------------|
| `M_IGNORE_SAFE_ERRNO_CALL(expr, reason)` | `errno_t` (0 on success) | `result == 0` |
| `M_IGNORE_SAFE_INT_CALL(expr, reason)` | `int` (≥ 0 on success) | `result >= 0` |
| `M_IGNORE_SAFE_PTR_CALL(expr, reason)` | pointer (non-null on success) | `result != NULL` |

The `reason` string is required — it is passed to `assert()` in debug builds, so if your assumption is wrong you can read the justification in the crash dump.

```c
// WRONG: bare discard
safe_strcpy(dest, sizeof(dest), src);

// WRONG: void cast or M_USE_UNUSED — skips the assert
(void)safe_strcpy(dest, sizeof(dest), src);

// CORRECT: when the result genuinely cannot fail here
M_IGNORE_SAFE_ERRNO_CALL(
    safe_memset(&myStruct, sizeof(myStruct), 0, sizeof(myStruct)),
    "zeroing a local struct of matching size cannot fail"
);

// CORRECT: when the result matters — check it
errno_t err = safe_strcpy(dest, destsz, src);
if (err != 0)
{
    return FAILURE;
}
```

### Safe Function Parameter Differences from Standard Counterparts

When migrating from standard C functions, be aware of these differences:

| Standard | Safe equivalent | Key difference |
|----------|----------------|---------------|
| `char* strdup(src)` → returns ptr | `safe_strdup(char** dup, src)` → output parameter | Return is `errno_t`; pointer is written through first arg |
| `ssize_t getline(...)` | `safe_getline(...)` → returns `errno_t` | Always calloc's; reading uninit memory was a real bug with the standard version |
| `memcpy(dst, src, n)` | `safe_memcpy(dst, dstsz, src, n)` | Internally calls `memmove` — overlapping buffers are safe by default |
| `memcpy` (no-overlap, perf-critical) | `safe_memcpy_no_overlap(dst, dstsz, src, n)` | Uses actual `memcpy`; only when overlap is provably impossible |

**`safe_strdup` is the most common AI mistake**: the return value is `errno_t`, and the allocated string is written through the `char**` output parameter:

```c
// WRONG — treating safe_strdup like POSIX strdup
char* copy = safe_strdup(original);  // does not compile

// CORRECT
char*   copy = M_NULLPTR;
errno_t err  = safe_strdup(&copy, original);
if (err != 0 || copy == M_NULLPTR)
{
    return MEMORY_FAILURE;
}
// ... use copy ...
safe_free(&copy);
```

### Always Provide the Destination Buffer Size

A common AI mistake is omitting the destination size argument. If the size is available in scope, always pass it — never guess or approximate:

```c
char buf[256];

// WRONG: missing destsz
safe_strcpy(buf, src);  // won't compile, but the mistake is easy in code review

// CORRECT
safe_strcpy(buf, sizeof(buf), src);
```

---

## 2. Memory Allocation and Deallocation

### `safe_free` Requires the Address of the Pointer

`safe_free` takes `void**` so it can NULL the pointer after freeing, preventing double-free and dangling pointer dereferences. Forgetting the `&` is the single most common mistake:

```c
uint8_t* buf = safe_malloc(size);

// WRONG: passes the pointer value, not its address — pointer not nulled
safe_free(buf);

// CORRECT
safe_free(&buf);  // buf is NULL after this line
```

For struct pointers, write a typed free helper and use `M_REINTERPRET_CAST`:

```c
void free_my_context(MyContext** ctx)
{
    if (ctx != M_NULLPTR && *ctx != M_NULLPTR)
    {
        // free internal allocations first
        safe_free(&(*ctx)->name);
        safe_free(M_REINTERPRET_CAST(void**, ctx));
    }
}
```

### Aligned Allocations Must Use Matching Free Functions

Windows `_aligned_malloc` **cannot** be freed with the standard `free`. Mismatching on Windows crashes immediately. Always pair:

| Allocator | Free |
|-----------|------|
| `safe_malloc_aligned` / `safe_calloc_aligned` | `safe_free_aligned(&ptr)` |
| `safe_malloc_page_aligned` / `safe_calloc_page_aligned` | `safe_free_page_aligned(&ptr)` |
| `safe_malloc` / `safe_calloc` / `safe_realloc` | `safe_free(&ptr)` |

Never mix these across the boundary.

### Always Free on Every Error Path

In functions that allocate multiple resources, add a cleanup label or free in each early-exit branch:

```c
uint8_t* buf = C_CAST(uint8_t*, safe_malloc(size));
if (buf == M_NULLPTR)
{
    return MEMORY_FAILURE;
}

eReturnValues ret = do_operation(buf, size);
if (ret != SUCCESS)
{
    safe_free(&buf);  // MUST free before returning — not just at the bottom
    return ret;
}

safe_free(&buf);
return SUCCESS;
```

---

## 3. Integer Overflow and Allocation Size Calculations

### Calculate Allocation Sizes into a Variable First

Passing arithmetic directly into a size argument — especially mixing `uint32_t` with `size_t` — is a source of overflow vulnerabilities. Calculate into a checked variable first:

```c
uint32_t count   = get_count_from_device();  // untrusted!
uint32_t itemsz  = sizeof(MyRecord);

// WRONG: overflow of uint32_t multiplication invisible at call site
void* buf = safe_malloc(count * itemsz);

// CORRECT: calculate first, then check
size_t allocSize = SIZE_T_C(0);
if (count > SIZE_T_C(0) && itemsz <= (SIZE_MAX / count))
{
    allocSize = (size_t)count * (size_t)itemsz;
}
if (allocSize == SIZE_T_C(0))
{
    return BAD_PARAMETER;
}
void* buf = safe_malloc(allocSize);
```

There is currently no checked integer math library in this codebase — overflow must be validated manually. This is a known weakness. When reviewing allocation sites, always check for multiplication or addition of values that could be attacker-influenced.

### `strtol` / `strtoull` family: `errno` must be zero before the call

This is a requirement of ISO/IEC TS 17961 (C Secure Coding Rules) rule INT07-C. The `strtol` / `strtoul` / `strtoll` / `strtoull` / `strtod` family sets `errno` to `ERANGE` on overflow but **does not clear it first**. If `errno` was already non-zero from a prior operation, the check after the call is meaningless.

The `safe_strtol`, `safe_strtoull`, `safe_strtod`, and all related safe conversion wrappers set `errno = 0` internally before calling the underlying function. This is one of the primary reasons to always use the safe wrappers:

```c
// WRONG: errno may already be set from a previous unrelated call
long val = strtol(str, &end, 10);
if (errno == ERANGE) { /* unreliable */ }

// WRONG: safe_ wrapper called, but return value ignored — ERANGE goes undetected
M_IGNORE_SAFE_ERRNO_CALL(safe_strtol(&val, str, &end, 10),
    "assuming valid input"); // if str is out of range, val is indeterminate

// CORRECT: check the return value — the wrapper already zeroed errno
long val  = 0;
errno_t e = safe_strtol(&val, str, &end, 10);
if (e != 0)
{
    // ERANGE = overflow/underflow; EINVAL = no digits or bad base
    return BAD_PARAMETER;
}
if (end == str)
{
    return BAD_PARAMETER;  // no digits consumed
}
```

Never call raw `strtol` / `strtoull` / `strtod` directly — use the `safe_*` equivalents which handle the `errno` reset and return a proper `errno_t`.

### Never Trust Drive-Reported Sizes Directly

Sizes and counts reported by drives (from IDENTIFY data, log pages, VPD pages, NVMe Identify Namespace, etc.) are **untrusted external data**. Treat them like user input:

- Check that the reported size is non-zero before dividing.
- Check that the reported size is within a realistic range before allocating.
- Check that the computed allocation does not overflow `size_t`.
- Check that any offset derived from the size does not exceed the buffer you already allocated.

```c
uint32_t reported_count = get_log_entry_count_from_drive(device);

// WRONG: trusting it directly
void* log_buf = safe_malloc(reported_count * sizeof(LogEntry));

// CORRECT
if (reported_count == 0 || reported_count > MAX_REALISTIC_LOG_ENTRIES)
{
    return BAD_PARAMETER;
}
size_t alloc = (size_t)reported_count * sizeof(LogEntry);
if (alloc / sizeof(LogEntry) != (size_t)reported_count)  // overflow check
{
    return BAD_PARAMETER;
}
void* log_buf = safe_malloc(alloc);
```

### Division Safety

Always check for zero before dividing. The most common case is computing logical-sectors-per-physical-sector:

```c
// WRONG
uint32_t ratio = device->physSectorSize / device->logSectorSize;

// CORRECT: use the helper which never returns zero
uint32_t ratio = get_Logical_Sectors_Per_Physical_Sector(device);
```

When writing any division, ask: "can the denominator be zero, and if so, where does that value come from?"

---

## 4. Cast Macros

C-style casts are **discouraged** in this project (and **forbidden** when compiling in C++ mode). Use the project macros so that casts are searchable and type-safe in C++ builds:

| Macro | C equivalent | When to use |
|-------|-------------|-------------|
| `C_CAST(type, expr)` | `(type)(expr)` | Most numeric and pointer casts |
| `M_STATIC_CAST(type, expr)` | `(type)(expr)` | Explicit widening/narrowing with intent |
| `M_REINTERPRET_CAST(type, expr)` | `(type)(expr)` | Type-punning (e.g., `void**` conversions) |
| `M_CONST_CAST(type, expr)` | `(type)(expr)` | Removing `const` — must be rare and justified |

### Narrowing Must Be Preceded by a Range Check

The most common bad cast is narrowing without verifying the value fits:

```c
uint32_t val = get_large_value();

// WRONG: silent truncation if val > UINT16_MAX
uint16_t small = C_CAST(uint16_t, val);

// CORRECT
if (val > UINT16_MAX)
{
    return BAD_PARAMETER;
}
uint16_t small = C_CAST(uint16_t, val);
```

### `M_CONST_CAST` Must Have a Comment

Removing `const` is occasionally necessary but always suspicious. Leave a justification:

```c
// justified: the API was designed before const-correctness; we own the memory
char* mutable_ptr = M_CONST_CAST(char*, const_string);
```

---

## 5. Output and Verbosity

### Transport and Operations Layer: Prefer `tDevice`-Aware Prints

The codebase is actively migrating away from `printf` in library code. When adding output in `opensea-transport` or `opensea-operations`:

- Use `print_tDevice_Verbose_String(device, level, "message")` for fixed strings.
- Use `print_tDevice_Verbose_Formatted_String(device, level, format, ...)` for formatted output.
- Use `print_str("message")` (not `puts`) when you need plain output without automatic newlines.
- Wrap debug-only output in `#if defined(_DEBUG)` — do not leave debug `printf` in release paths.

```c
// WRONG in library code
printf("Sending command 0x%02X\n", cmd);

// CORRECT
print_tDevice_Verbose_Formatted_String(device, VERBOSITY_COMMAND_VERBOSE,
    "Sending command 0x%02X\n", cmd);
```

### Verbosity Levels

| Level | Constant | Content |
|-------|----------|---------|
| 0 | `VERBOSITY_QUIET` | No output |
| 1 | `VERBOSITY_DEFAULT` | Normal user-facing messages |
| 2 | `VERBOSITY_COMMAND_NAMES` | Command names being issued (largely historical) |
| 3 | `VERBOSITY_COMMAND_VERBOSE` | CDB, task file registers, status |
| 4 | `VERBOSITY_BUFFERS` | Raw data buffers to/from the drive |

CLI utilities (`openSeaChest_*`) may use `printf` directly for their user-facing output — the verbosity pattern applies to the library layers below them.

---

## 6. `memcpy` vs `memmove`

Do **not** call `safe_memcpy` when the intent is `memmove`. This is a moot point in this project because `safe_memcpy` already internally calls `safe_memmove` for safety. However, if overlap is provably impossible and performance matters, you may call `safe_memcpy_no_overlap` explicitly:

```c
// Always safe — internally uses memmove
safe_memcpy(dst, dstsz, src, count);

// Only when you are CERTAIN src and dst regions do not overlap
safe_memcpy_no_overlap(dst, dstsz, src, count);
```

Never call the raw `memcpy` or `memmove` directly — use the `safe_*` equivalents.

---

## 7. Switch Completeness

The right strategy depends on what the enum represents. Use the `M_CLOSED_ENUM` and `M_OPEN_ENUM` attributes on the enum declaration as a machine-readable signal:

### `M_OPEN_ENUM` — prefer a `default` case

Use `M_OPEN_ENUM` for enumerations whose values come from hardware specifications, storage standards (ATA, SCSI, NVMe, ZAC/ZBC, etc.), or any external authority where **future reserved values may appear in real commands**. A `default` case that returns `BAD_PARAMETER` (or an equivalent protocol error) is correct here — it avoids undefined behavior when the library receives an unrecognized command code and gives the caller actionable feedback.

```c
enum M_OPEN_ENUM zac_action_code
{
    ZAC_ACTION_REPORT_ZONES = 0x00,
    ZAC_ACTION_CLOSE_ZONE   = 0x01,
    ZAC_ACTION_FINISH_ZONE  = 0x02,
    ZAC_ACTION_OPEN_ZONE    = 0x03,
    ZAC_ACTION_RESET_ZONE   = 0x04,
    // 0x05–0xFF: reserved by T13 — future commands may be added
};

// In the switch:
switch (actionCode)
{
case ZAC_ACTION_REPORT_ZONES:
    return zac_report_zones(device, ...);
case ZAC_ACTION_CLOSE_ZONE:
    return zac_close_zone(device, ...);
// ... all currently known values ...
default:
    // Caller passed a reserved/unknown action. Return an error — do not call M_UNREACHABLE().
    // The specification may define this code in a future revision.
    return BAD_PARAMETER;
}
```

### `M_CLOSED_ENUM` — prefer exhaustive cases with no `default`

Use `M_CLOSED_ENUM` for purely software-defined enumerations that the project controls entirely (e.g., internal state machines, option flags, error codes). With no `default`, `-Wswitch` / `-Wswitch-enum` fires at compile time whenever the enum gains a new value and this switch is not updated — exactly what you want.

```c
enum M_CLOSED_ENUM transfer_direction
{
    XFER_READ,
    XFER_WRITE,
    XFER_NONE,
};

// In the switch — no default; compiler warns on missing cases:
switch (dir)
{
case XFER_READ:  return issue_read(device, buf, len);
case XFER_WRITE: return issue_write(device, buf, len);
case XFER_NONE:  return SUCCESS;
}
```

### Review guidance

During code review, when a switch on an enum lacks a `default` or does not cover all values, ask:

> *Does this enum represent a hardware/protocol concept where future reserved values could arrive in real commands (`M_OPEN_ENUM`)? If so, a `default` returning an error is correct. Or is it a purely software enum this project fully controls (`M_CLOSED_ENUM`)? If so, enumerate all cases explicitly and rely on the compiler to catch missed additions.*
>
> *If there are a very large number of cases, a `default` can be pragmatic even for software enums — just make sure the default returns an error, not silent success.*

Never add `default: break;` or `default: return SUCCESS;` to silence a `-Wswitch` warning on a `M_CLOSED_ENUM` switch — that hides future maintenance bugs. Fix the missing cases instead.

### `M_UNREACHABLE()` in a `default` — when it is legitimate

`M_UNREACHABLE()` (from `code_attributes.h`) tells the compiler that a code path can provably never be reached at runtime, enabling better optimization and suppressing "control reaches end of non-void function" warnings. It expands to `std::unreachable()` (C++23), `unreachable()` (C23), `__builtin_unreachable()` (GCC/Clang), or `__assume(0)` (MSVC).

**It is legitimate** in a `default` case when an outer guard has already mathematically eliminated every value that could fall through to it. The key requirement is that the guard must be a real invariant — not just an assumption:

```c
// Caller has already checked that commandType < CMD_ZONE_BOUNDARY.
// The switch handles all values in [0, CMD_ZONE_BOUNDARY).
// The default can never be reached — M_UNREACHABLE() is correct here.
if (commandType < CMD_ZONE_BOUNDARY)
{
    switch (commandType)
    {
    case CMD_REPORT_ZONES:
        return handle_report_zones(device, buf);
    case CMD_CLOSE_ZONE:
        return handle_close_zone(device, lba);
    case CMD_FINISH_ZONE:
        return handle_finish_zone(device, lba);
    case CMD_OPEN_ZONE:
        return handle_open_zone(device, lba);
    case CMD_RESET_ZONE:
        return handle_reset_zone(device, lba);
    default:
        // Logically unreachable: the if-guard above eliminates all values
        // outside [CMD_REPORT_ZONES, CMD_RESET_ZONE]. The compiler can use
        // this to optimize the branch table and suppress missing-return warnings.
        M_UNREACHABLE();
    }
}
else
{
    // Handle values >= CMD_ZONE_BOUNDARY here
}
```

**When `M_UNREACHABLE()` in a `default` is NOT legitimate:**

- The outer guard is just "I think only valid values are passed" — not a proven invariant
- The enum is `M_OPEN_ENUM` (hardware/protocol values): a future specification may define the "unreachable" code — use `return BAD_PARAMETER` instead
- There is no outer guard and the switch is simply missing cases — the correct fix is to add the cases, not `M_UNREACHABLE()`
- The code path is unreachable today but could become reachable as the codebase evolves without another guard being updated

**Important**: note the macro is called as `M_UNREACHABLE()` — with parentheses — not `M_UNREACHABLE` (no parentheses). This is different from `M_FALLTHROUGH` which has no parentheses.

> **Review rule**: When you see `default: M_UNREACHABLE();`, verify that a real guard exists upstream that provably prevents every enum value not covered by explicit `case` labels from entering the switch. If no such guard exists, flag it as 🔴 Critical — `M_UNREACHABLE()` on a reachable path is undefined behavior.

---

## 8. Reentrant and Thread-Safe Function Variants

### Always prefer the reentrant (`_r`) version

Many classic C and POSIX functions operate on hidden global or static state — `strtok`, `gmtime`, `localtime`, `ctime`, `asctime`, `rand`, `strerror` — making them unsafe in multithreaded code and producing race conditions even in single-threaded code when called from interrupt handlers or from within callbacks. The `_r` reentrant variants pass caller-owned state instead:

| Non-reentrant (do not use) | Reentrant variant | opensea-common wrapper |
|---------------------------|-------------------|----------------------|
| `strtok` | `strtok_r` (POSIX) / `strtok_s` (Windows) | `safe_strtok` |
| `gmtime` | `gmtime_r` (POSIX) / `gmtime_s` (Windows) | `safe_gmtime` |
| `localtime` | `localtime_r` (POSIX) / `localtime_s` (Windows) | `safe_localtime` |
| `ctime` | `ctime_r` (POSIX) | `safe_ctime` |
| `asctime` | `asctime_r` (POSIX) | `safe_asctime` |
| `strerror` | `strerror_r` (POSIX — two incompatible variants!) | `get_strerror` |
| `rand` | `rand_r` (POSIX) | — |

Even if a specific libc documents that the non-reentrant version is safe (e.g., glibc's `gmtime` returning thread-local storage on Linux), this guarantee does not extend to uclibc, musl, Newlib, or other embedded C libraries the project cross-compiles against. **Always use the wrapper or the `_r` variant.**

### `strtok`, `basename`, and `dirname` modify their input string

`strtok` (and `strtok_r` / `safe_strtok`) inserts null terminators into the string it tokenizes. `basename` and `dirname` may modify their argument in place depending on the implementation. If the original string is needed after calling any of these, **duplicate it first with `safe_strdup` and pass the copy**:

```c
char* pathCopy = M_NULLPTR;
errno_t err = safe_strdup(&pathCopy, originalPath);
if (err != 0 || pathCopy == M_NULLPTR)
{
    return MEMORY_FAILURE;
}

// safe_strtok will modify pathCopy, not originalPath
char* saveptr = M_NULLPTR;
char* token   = safe_strtok(pathCopy, sizeof(pathCopy), "/", &saveptr);
while (token != M_NULLPTR)
{
    // process token...
    token = safe_strtok(M_NULLPTR, sizeof(pathCopy), "/", &saveptr);
}

safe_free(&pathCopy);
// originalPath is still intact here
```

The same pattern applies to `basename` and `dirname` — check the platform documentation: POSIX permits them to return a pointer into the argument or to a static buffer; the GNU version of `basename` does not modify its argument but the POSIX version may. When in doubt, duplicate the string and use the result before freeing the copy.

### General rule: a non-`const char*` parameter may modify your string

Any function that accepts a `char*` (not `const char*`) reserves the right to modify the content. If the string must survive the call unchanged, pass a duplicate. Functions to watch — beyond the reentrant variants already listed:

- `mktemp` / `mkstemp` / `mkdtemp` — modify the template in place.
- `realpath(path, NULL)` — does not modify `path`, but platform implementations vary.
- Any vendor-supplied callback or plugin API that takes `char*`.

The rule of thumb: if you are not certain a function is documented to treat its `char*` argument as read-only, `safe_strdup` first.

### Use opensea-common wrappers, not the raw `_r` functions

The `_r` variants themselves have cross-platform incompatibilities:

- `strerror_r` has **two different signatures** — POSIX and GNU — that both compile silently but behave differently (see Section 9).
- `gmtime_s` / `localtime_s` on Windows swap the argument order from POSIX's `gmtime_r` / `localtime_r`.

The opensea-common wrappers (`safe_gmtime`, `safe_localtime`, `safe_strtok`, etc.) resolve these differences internally. Use the wrapper and do not call the underlying `_r` / `_s` variants directly.

### Checking POSIX version availability with `predef_env_detect.h`

Reentrant variants were introduced at specific POSIX revisions. When adding new code that needs a POSIX API, check whether the platform's POSIX level supports it using the convenience macros from `predef_env_detect.h`:

| Macro | POSIX version | Notable reentrant APIs introduced |
|-------|--------------|----------------------------------|
| `POSIX_1996` | POSIX.1c (1996) | `strtok_r`, `gmtime_r`, `localtime_r`, `asctime_r`, `ctime_r`, `rand_r` |
| `POSIX_2001` | POSIX.1-2001 | `strerror_r`, `readdir_r` |
| `POSIX_2008` | POSIX.1-2008 | Various deprecations of non-reentrant forms |

```c
#if defined(POSIX_1996)
    // safe to call strtok_r directly (but prefer safe_strtok)
#else
    // fall back to platform-specific alternative
#endif
```

These macros are defined in `predef_env_detect.h` and are safe to use in any opensea-common or opensea-transport code. Do not test `_POSIX_VERSION` directly — use the project's named macros instead for clarity and consistency.

### Locale-sensitive functions

Some standard functions change behavior based on the process locale (`LC_ALL`, `LC_NUMERIC`, `LC_CTYPE`, etc.). This is subtle and cross-platform:

- `strtod` / `strtof` / `printf` with `%f` use the decimal separator from `LC_NUMERIC`. On a French locale this is `,` not `.`, which breaks device data parsing.
- `toupper` / `tolower` / `isalpha` etc. use `LC_CTYPE` — behavior can differ for bytes 128–255.
- `strcoll` and `strxfrm` are locale-dependent; `strcmp` is not.

**In library code (`opensea-transport`, `opensea-operations`, `opensea-common`)**: do not call locale-sensitive variants of functions when parsing or generating device data that must be byte-exact. Use explicit locale-independent functions where available (`strtol` / `safe_strtol` are locale-independent; `strtod_l` with `LC_CLASSIC` if a locale-independent float parse is needed). The library must produce correct output regardless of the user's locale.

```c
// RISKY in library code: LC_NUMERIC affects the decimal separator
double val = strtod(str, &end);

// PREFERRED: safe_strtod wraps strtod; document clearly if locale sensitivity matters
// For float values from device data, consider whether float parsing is needed at all.
```

---

## 9. Undefined, Unspecified, and Platform-Specific Behavior

A significant class of bugs in this codebase comes from accidentally depending on behavior that is undefined in the C standard, platform-specific extensions, or POSIX extensions that differ across systems. These bugs are invisible on one platform and crash (or silently corrupt) on another.

### CERT-C is the baseline

The clang-tidy `cert-*` check family enforces a large portion of the CERT-C Coding Standard rules automatically. Beyond what the toolchain catches, the following patterns require human attention.

### POSIX extensions vs. GNU/Linux extensions

Some APIs have a POSIX-standard version and a GNU-extended version that **share a name but differ in signature or behavior**. The canonical example is `strerror_r`:

- **POSIX version**: `int strerror_r(int errnum, char* buf, size_t buflen)` — writes into the provided buffer, returns an error code.
- **GNU version**: `char* strerror_r(int errnum, char* buf, size_t buflen)` — may return a pointer to a static string, ignoring the buffer.

Code that uses the POSIX version correctly will compile and run on Linux with `_GNU_SOURCE` defined but produce incorrect output because the GNU version silently ignores the buffer. opensea-common provides a wrapper that ensures consistent behavior; **use the wrapper, not `strerror_r` directly**.

When you encounter any function that exists in both a POSIX and a GNU/platform-extended form:
1. Do not use the function directly if opensea-common already wraps it.
2. If no wrapper exists and you need platform-specific behavior, guard it:

```c
#if defined(__linux__) && defined(_GNU_SOURCE)
    // GNU strerror_r behavior
#else
    // POSIX strerror_r behavior
#endif
```

### Zero-size `memset` / `memcpy` was historically undefined

Calling `memset(ptr, 0, 0)` or `memcpy(dst, src, 0)` has been undefined behavior for most of C's history and is only defined in recent standard revisions. Do not rely on it being a no-op on Linux (where glibc documents it) or Windows (where it is observed but not guaranteed). The `safe_memset` / `safe_memcpy` wrappers include a size guard — one more reason to always use them.

```c
// WRONG: if count is 0 this is UB on older standards
memset(buf, 0, count * sizeof(MyType));

// CORRECT: guard the call, or use safe_memset which guards internally
if (count > 0)
{
    M_IGNORE_SAFE_ERRNO_CALL(
        safe_memset(buf, bufSz, 0, count * sizeof(MyType)),
        "size already validated non-zero and within buf"
    );
}
```

### Platform-specific behavior requires a guard

If you intentionally use behavior that is only defined on one platform, guard it with the appropriate preprocessor check. Using it unguarded is a latent cross-platform bug:

```c
// WRONG: epoll is Linux-specific; compiles nowhere else
int fd = epoll_create1(EPOLL_CLOEXEC);

// CORRECT
#if defined(__linux__)
int fd = epoll_create1(EPOLL_CLOEXEC);
#endif
```

The same rule applies to POSIX APIs that are missing or behave differently on Windows, BSD, or Solaris. If you are not sure whether an API is universally available, check the platform support matrix in `README.md` and the platform guards in the existing opensea-transport passthrough code.

### Compiler warnings across multiple environments catch many of these issues

A function that compiles cleanly under GCC on Linux may emit warnings or errors under:
- MSVC (different defaults for sign/type mismatches, different `__declspec` requirements)
- Clang (stricter `-Wnullability`, `-Wshadow`, `-Wcast-align`)
- GCC on a 32-bit target (where `size_t` is 32 bits and `long` is 32 bits)

When possible, build and test under at least two of the three compilers (MSVC, GCC, Clang) before submitting. The CI matrix does this automatically, but running locally first catches issues sooner.

---

## 10. Nullability Annotations and `M_NONNULL_PARAM_LIST` Safety

### Prefer `M_NONNULL` / `M_NULLABLE` on API boundaries

For any function declared in a public header, use the Clang nullability qualifiers from `code_attributes.h` on each pointer parameter:

```c
// Public API
eReturnValues do_operation(const tDevice* M_NONNULL device,
                           uint8_t* M_NULLABLE      outBuf,
                           size_t                   outBufSz);
```

These qualifiers:
- Generate Clang warnings at every **call site** that passes a potentially-null pointer to a `M_NONNULL` parameter.
- Do **not** cause the optimizer to remove null pointer checks inside the function body — the function can still defensively check `device != M_NULLPTR`.
- Are no-ops on GCC and MSVC, so they do not affect those builds.

### `M_NONNULL_PARAM_LIST` / `M_ALL_PARAMS_NONNULL` — only for internal functions

`M_NONNULL_PARAM_LIST(n, ...)` maps to GCC/Clang `__attribute__((nonnull(...)))`. This attribute tells the **optimizer** the parameters are never null, which causes it to **eliminate null pointer checks** inside the function body. On an API boundary this is dangerous: if a caller passes `NULL`, the optimizer has already removed the check that would catch it, leading to a crash or silent memory corruption.

**Rule**: Use `M_NONNULL_PARAM_LIST` / `M_ALL_PARAMS_NONNULL` only on `static` or file-scope functions where all call sites are visible and null is provably impossible.

```c
// WRONG on a public API — optimizer removes the NULL guard inside
M_ALL_PARAMS_NONNULL
eReturnValues process_command(tDevice* device, CommandBlock* cmd);

// CORRECT on a public API — Clang warns callers, no null-check elimination
eReturnValues process_command(tDevice* M_NONNULL device, CommandBlock* M_NONNULL cmd);

// CORRECT on an internal/static function — all callers visible, null proven impossible
M_ALL_PARAMS_NONNULL
static eReturnValues internal_helper(tDevice* device, CommandBlock* cmd);
```

### Annotations improve sanitizer and static analysis coverage

Beyond null safety, the annotations in `code_attributes.h` (`M_PARAM_RO_SIZE`, `M_PARAM_WO_SIZE`, `M_FUNC_ATTR_MALLOC`, `M_TAINTED_ARGS`, etc.) generate richer diagnostics from `-fsanitize=address`, `-fsanitize=undefined`, GCC `-fanalyzer`, and Clang's static analyzer. Annotating functions correctly amplifies the value of the existing sanitizer CI jobs without requiring additional tooling.

---

## Quick Reference: Common Mistakes

| Mistake | Correct pattern |
|---------|----------------|
| `strtol` / `strtoull` / `strtod` called directly | use `safe_strtol` / `safe_strtoull` / `safe_strtod` — they zero `errno` first |
| `strtok` / `gmtime` / `localtime` / `ctime` / `asctime` / `strerror` directly | use `safe_strtok` / `safe_gmtime` / `safe_localtime` / `safe_ctime` / `safe_asctime` / `get_strerror` |
| `strtok` / `basename` / `dirname` on string needed afterward | `safe_strdup` a copy first; pass the copy to the modifying function |
| non-`const char*` parameter — unsure if function modifies it | `safe_strdup` a copy first when the original must survive |
| `_POSIX_VERSION` tested directly | use `POSIX_1996` / `POSIX_2001` / `POSIX_2008` macros from `predef_env_detect.h` |
| `strtod` in library code parsing device data | document locale sensitivity; prefer locale-independent parse paths |
| `safe_free(ptr)` | `safe_free(&ptr)` |
| `safe_aligned_free(&ptr)` without matching allocator | `safe_free_aligned(&ptr)` |
| Bare discard of `CONSTRAINT_NO_DISCARD` function | `M_IGNORE_SAFE_ERRNO_CALL(expr, "reason")` |
| `char* s = safe_strdup(src)` | `errno_t e = safe_strdup(&s, src)` |
| `safe_strcpy(buf, src)` — missing size | `safe_strcpy(buf, sizeof(buf), src)` |
| `malloc(count * itemsz)` with external `count` | calculate and overflow-check first |
| `(uint16_t)val` without range check | check `val <= UINT16_MAX` first |
| `physSz / logSz` without zero check | use `get_Logical_Sectors_Per_Physical_Sector(device)` |
| `printf(...)` in library code | `print_tDevice_Verbose_Formatted_String(device, ...)` |
| `memcpy` / `memmove` directly | `safe_memcpy` / `safe_memcpy_no_overlap` |
| Trusting drive-reported sizes for allocation | validate range and overflow before using |
| `default: break;` to silence `-Wswitch` on `M_CLOSED_ENUM` | fix the missing cases instead |
| `default: M_UNREACHABLE()` with no upstream guard | only valid when a real invariant provably eliminates all uncovered enum values before the switch |
| `M_UNREACHABLE` without parentheses | must be called as `M_UNREACHABLE()` |
| `M_ALL_PARAMS_NONNULL` on a public API function | use `M_NONNULL` qualifier on each parameter instead |
| `strerror_r` called directly | use the opensea-common wrapper |
| `memset(ptr, 0, 0)` / `memcpy(d, s, 0)` | guard with `if (count > 0)` or use `safe_memset` |
| struct cast over raw drive buffer without byte-swap | read fields with `le32_to_host` / `be32_to_host` or `M_BytesTo4ByteValue` |
| `byte_Swap_32(&field)` on SCSI big-endian data on LE host | use `be32_to_host(field)` — self-documenting and no-op on BE hosts |
| `M_Byte0(wireVal)` before converting to host byte order | convert first: `M_Byte0(be32_to_host(wireVal))` or assemble via `M_BytesTo4ByteValue` |
| `goto` in new code | restructure to early returns, helper functions, or explicit cleanup on each path |
| intentional switch fall-through without `M_FALLTHROUGH` | add `M_FALLTHROUGH` before the next `case` — suppresses `-Wimplicit-fallthrough` correctly |
| bracketless `if`/`for`/`while` body | always add braces — prevents merge-induced silent logic errors |
| `NULL` or `0` used as null pointer constant | use `M_NULLPTR` from `common_types.h` |
| `__FUNCTION__` predefined identifier | use C99 standard `__func__` instead |
| VLA declaration (`type arr[runtimeSize]`) | use `safe_malloc` with an explicit size variable |
| `alloca(size)` | use `safe_malloc(size)` — non-standard and no overflow protection |
| new function-like macro with computation/comparison | write a `static M_INLINE` function instead — gets type checking and debugger visibility |
| `sizeof(heap_ptr)` to get buffer element count | track size in a companion variable; never derive from `sizeof(ptr)` |
| `sizeof(arr) / sizeof(arr[0])` for stack array count | use `SIZE_OF_STACK_ARRAY(arr)` from `memory_safety.h` |

---

## 11. Array Size: `sizeof` on a Pointer Is Always Wrong

### `sizeof` on a heap-allocated array returns the pointer size, not the allocation size

When an array is allocated on the heap (`safe_malloc`, `safe_calloc`, etc.), the variable holding it is a **pointer**. Applying `sizeof` to that pointer returns `sizeof(void*)` — 4 or 8 bytes depending on the target — regardless of how large the allocation is. This is one of the most silent and common bugs:

```c
uint8_t* buf = C_CAST(uint8_t*, safe_malloc(512));

// WRONG: always 4 or 8 — the size of the pointer, not the buffer
size_t n = sizeof(buf);

// CORRECT: track the size in a separate variable
size_t bufSz = UINT16_C(512);
uint8_t* buf = C_CAST(uint8_t*, safe_malloc(bufSz));
// ... pass bufSz alongside buf everywhere it is needed
```

**Rule**: Always carry the allocation size in a companion variable. Never derive it from `sizeof(ptr)` after the fact.

### For stack arrays, use `SIZE_OF_STACK_ARRAY()` — not bare `sizeof(arr)/sizeof(arr[0])`

For arrays declared on the stack, the size *can* be obtained from `sizeof`, but the idiomatic divisor form is error-prone. Use the project macro from `memory_safety.h` instead:

```c
uint32_t codes[16];  // stack array

// WRONG: sizeof(codes) is the byte count, not the element count
for (size_t i = 0; i < sizeof(codes); ++i) { ... }

// WRONG: correct math but fragile — easy to mistype the element type
size_t n = sizeof(codes) / sizeof(uint32_t);

// CORRECT: use the macro — backed by _Countof (C23), _countof (MSVC),
//          or sizeof(arr)/sizeof(*arr) as fallback
for (size_t i = 0; i < SIZE_OF_STACK_ARRAY(codes); ++i) { ... }
```

`SIZE_OF_STACK_ARRAY` carries a compiler warning when used on a pointer — it will not silently produce a wrong answer the way the bare `sizeof` idiom does.

**Never use `SIZE_OF_STACK_ARRAY` on a heap pointer** — the macro's own documentation warns against this and on some compilers it will produce a diagnostic.

---

## 12. Endianness and Drive Wire Formats

### The host is not always little-endian

The project cross-compiles for ARM, MIPS, PowerPC, RISC-V, and other architectures where the host may be big-endian. **Never assume the host is little-endian.** Use `ENV_LITTLE_ENDIAN` / `ENV_BIG_ENDIAN` from `predef_env_detect.h` when a conditional compile is needed, and prefer the conversion functions below for portable byte-order handling.

### Drive wire formats by interface

| Interface | Wire byte order | Data entering host must use |
|-----------|----------------|-----------------------------|
| SCSI / SAS / iSCSI | Big-endian (network order) | `be16_to_host` / `be32_to_host` / `be64_to_host` |
| ATA / SATA (data fields) | Little-endian | `le16_to_host` / `le32_to_host` / `le64_to_host` |
| ATA IDENTIFY string fields | Byte-swapped words | special word-swap handling — see existing `ata_helper.c` patterns |
| NVMe | Little-endian | `le16_to_host` / `le32_to_host` / `le64_to_host` |

### Never cast a struct directly over a raw byte buffer

Overlaying a struct on a raw response buffer is wrong for two reasons:
1. **Byte order**: multi-byte fields in the buffer are in wire order, not host order.
2. **Alignment and padding**: the compiler may insert padding that does not exist in the wire format.

```c
// WRONG: UB on misaligned access; wrong values on BE host; padding mismatch
const MyScsiPage* page = C_CAST(const MyScsiPage*, responseBuffer);
uint32_t length = page->length;  // still in big-endian on a LE host!

// CORRECT option A: copy then convert each multi-byte field
MyScsiPage page;
M_IGNORE_SAFE_ERRNO_CALL(
    safe_memcpy(&page, sizeof(page), responseBuffer, sizeof(page)),
    "buffer validated to be at least sizeof(MyScsiPage)"
);
uint32_t length = be32_to_host(page.length);

// CORRECT option B: assemble from individual bytes (avoids alignment entirely)
uint32_t length = M_BytesTo4ByteValue(
    responseBuffer[0], responseBuffer[1],
    responseBuffer[2], responseBuffer[3]
);
```

### Endianness conversion API (`bit_manip.h`)

All functions are `M_NODISCARD` — always use the return value.

**Reading wire data into host values:**

```c
uint16_t val16 = be16_to_host(raw16);   // SCSI big-endian field → host
uint32_t val32 = le32_to_host(raw32);   // ATA/NVMe little-endian field → host
uint64_t val64 = be64_to_host(raw64);   // SCSI 8-byte field → host
```

**Writing host values into wire format before sending a command:**

```c
// host → SCSI big-endian: convert once, then byte-extract into CDB
uint32_t wireVal = host_to_be32(count);
cmdBuf[0] = M_Byte3(wireVal);  // MSB
cmdBuf[1] = M_Byte2(wireVal);
cmdBuf[2] = M_Byte1(wireVal);
cmdBuf[3] = M_Byte0(wireVal);  // LSB
```

**Assembling a value from raw bytes — `M_BytesTo2ByteValue` / `M_BytesTo4ByteValue` / `M_BytesTo8ByteValue`:**

These macros take individual bytes and assemble them into a host-endian integer. Because you name each byte offset explicitly, they are inherently correct regardless of the host's native byte order. Arguments are always **MSB first**:

```c
// Big-endian wire (SCSI): MSB is at the lowest buffer offset
uint32_t lba = M_BytesTo4ByteValue(buf[0], buf[1], buf[2], buf[3]);

// Little-endian wire (ATA/NVMe): LSB is at the lowest buffer offset, so reverse the order
uint32_t lba = M_BytesTo4ByteValue(buf[3], buf[2], buf[1], buf[0]);

// 8-byte SCSI value spanning buf[0]..buf[7]
uint64_t val = M_BytesTo8ByteValue(
    buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]
);
```

The result is already in host byte order — no further conversion is needed.

**In-place byte swap (use sparingly — prefer the conversion functions):**

```c
uint32_t val = *C_CAST(uint32_t*, buf);
byte_Swap_32(&val);  // swap after reading — but prefer be32_to_host(val) instead
```

### `M_Byte0` / `M_Byte1` / `M_Byte2` / `M_Byte3` — byte extraction from host-endian values

`M_Byte0(v)` extracts bits 7:0 of `v`, `M_Byte1(v)` extracts bits 15:8, and so on. These operate on the **numeric value** — which must already be in host byte order. If you are extracting bytes from a value that came off the wire, convert to host byte order first:

```c
// Reading a 32-bit value from a SCSI (big-endian) buffer, then pulling individual bytes:
uint32_t hostVal = be32_to_host(wireVal);   // convert once
uint8_t lo = M_Byte0(hostVal);              // bits 7:0 of the host value
uint8_t hi = M_Byte3(hostVal);              // bits 31:24 of the host value

// Or assemble with M_BytesTo4ByteValue, then extract:
uint64_t val = M_BytesTo8ByteValue(b7, b6, b5, b4, b3, b2, b1, b0);
uint8_t lsb = M_Byte0(val);   // correct — val is already in host byte order
```

**Writing bytes into a command buffer** (going to the drive) typically reverses this: convert the host value to wire order first, then use `M_Byte0`/`M_Byte1`/... to place each byte at the correct buffer offset. Interface-specific patterns (ATA CDB construction, SCSI CDB construction, NVMe submission queue entries) are covered in the per-interface instruction documents.

```c
// Sending a 32-bit count in a SCSI CDB (big-endian on wire): MSB goes first
uint32_t wireVal = host_to_be32(count);
cmdBuf[0] = M_Byte3(wireVal);  // MSB
cmdBuf[1] = M_Byte2(wireVal);
cmdBuf[2] = M_Byte1(wireVal);
cmdBuf[3] = M_Byte0(wireVal);  // LSB
```

### Key rule

> For every multi-byte field read from a drive response buffer or written to a command buffer, ask: "What byte order does this interface use, and am I converting it correctly for the host?"

If the answer is "I don't know" or "it works on x86 so it must be fine" — that is a bug waiting to manifest on a BE host or a future architecture.

---

## 13. `goto` — Treat Every Occurrence as a Red Flag

The project follows a strict **no-`goto`** rule. There is one known exception: a FreeBSD-derived function in `opensea-common` that was ported as-is and kept because rewriting it would risk introducing bugs in otherwise proven logic.

Every new use of `goto` must be treated as a review flag and justified explicitly. There is almost always a cleaner alternative:

| Situation | Preferred alternative |
|-----------|----------------------|
| Error cleanup at function exit | Reverse-order `safe_free` / `close_Device` calls; or refactor into a helper |
| Breaking out of nested loops | Refactor the inner logic into a function and `return` |
| Shared teardown path | Early `return` with a wrapper that owns the resource lifetime |

The kernel-style "jump to a single cleanup label at the end of the function" pattern is the least-bad use of `goto`, but it is still avoided here in favour of explicit cleanup on every return path. If you encounter this pattern in a review, flag it as 🟡 Important and ask whether the function can be restructured.

```c
// WRONG: jumping forward over initialisation
if (condition)
    goto skip;
int x = compute();  // initialization skipped — undefined behavior under skip
skip:
use(x);

// WRONG: jumping backward to retry — convert to a loop instead
retry:
if (!try_operation())
    goto retry;

// ACCEPTABLE (existing ported code only, not new code):
// goto cleanup;
// cleanup:
//     safe_free(&buf);
//     return ret;
```

---

## 14. Always Use Braces — No Bracketless Control Flow

Every `if`, `else`, `for`, `while`, and `do` body **must** be enclosed in braces, regardless of how simple the body is. This prevents a class of merge-introduced bugs where a second statement is added at the same indentation level but only the first is controlled by the condition — code that looks correct at a glance but behaves differently.

```c
// WRONG: bracketless — a later merge adding a second statement silently breaks logic
if (condition)
    doSomething();

// WRONG: same problem in a loop
for (size_t i = 0; i < n; ++i)
    process(i);

// CORRECT:
if (condition)
{
    doSomething();
}

for (size_t i = 0; i < n; ++i)
{
    process(i);
}
```

### Switch-case is the one exception

`case` labels do **not** need braces unless a local variable is declared within that case (to limit its scope). An intentional fall-through between cases must use the `M_FALLTHROUGH` macro from `code_attributes.h` — never rely on silent fall-through.

```c
switch (thing)
{
case 1:
    setFlagThenFallthrough = true;
    M_FALLTHROUGH  // intentional — documented fall-through to case 2
case 2:
    doCaseTwoStuff(setFlagThenFallthrough);
    break;
case 3:
{   // braces required here because of the local variable
    uint32_t localVal = compute();
    process(localVal);
    break;
}
default:
    break;
}
```

`M_FALLTHROUGH` expands to `[[fallthrough]]` (C23), `[[clang::fallthrough]]`, `[[gcc::fallthrough]]`, `__fallthrough` (SAL), or a `/*FALLTHRU*/` comment depending on the compiler — it is always safe to use and suppresses `-Wimplicit-fallthrough` correctly on all supported toolchains.

---

## 15. Null Pointer Constant and Predefined Identifier Conventions

### Always use `M_NULLPTR` for null pointers

The project uses `M_NULLPTR` from `common_types.h` instead of `NULL`, `0`, or (in C++) `nullptr`. The macro expands to:
- `nullptr` in C23 and C++11+
- `__nullptr` where available (Clang extension)
- `NULL` or `((void*)0)` otherwise

This ensures correct type behaviour across all supported language versions and compilers, and aligns with C23's introduction of `nullptr` as a typed null pointer constant.

```c
// WRONG: ambiguous or version-specific
char* p = NULL;
char* q = 0;

// CORRECT:
char* p = M_NULLPTR;
if (p == M_NULLPTR) { ... }
```

### Always use `__func__` — never `__FUNCTION__`

`__func__` is the C99/C11 standard predefined identifier for the current function name. `__FUNCTION__` is an old MSVC extension that predates C99 support. While MSVC does accept `__FUNCTION__` for compatibility, using it signals pre-C99 code and is inconsistent across static analysis tools.

The codebase targets C99 minimum (effectively C11 due to MSVC's C11 support level). Use `__func__` exclusively:

```c
// WRONG: MSVC pre-C99 extension
printf("%s: operation failed\n", __FUNCTION__);

// CORRECT:
printf("%s: operation failed\n", __func__);
```

---

## 16. Banned Stack Allocation — VLAs and `alloca`

### Variable-length arrays (VLAs) are banned

VLAs (`int buf[n]` where `n` is a runtime value) are banned for two reasons:

1. **MSVC does not support them.** Enforcing the ban everywhere is simpler than conditionally allowing them in Linux/FreeBSD-specific files. This keeps the code portable across all supported toolchains.
2. **They are dangerous.** VLAs allocate on the stack with no overflow protection. A large or attacker-influenced `n` silently corrupts the stack. The compiler warning flags `-Wvla` and `-Wvla-parameter` are active in all build configurations to catch these.

```c
// WRONG: VLA — stack overflow if n is large; not supported on MSVC
void process(size_t n)
{
    uint8_t buf[n];  // -Wvla catches this
    ...
}

// CORRECT: heap allocation with explicit size tracking
void process(size_t n)
{
    uint8_t* buf = C_CAST(uint8_t*, safe_malloc(n));
    if (buf == M_NULLPTR)
    {
        return;
    }
    ...
    safe_free(&buf);
}
```

### `alloca` is also banned

`alloca` is a non-standard function that allocates memory on the stack at runtime. It shares the same risks as VLAs (no overflow protection, stack corruption on large sizes) and is not available on all supported platforms in a consistent form. Use `safe_malloc` / `safe_calloc` instead.

```c
// WRONG: non-standard, no overflow protection, not portable
void* tmp = alloca(size);

// CORRECT:
void* tmp = safe_malloc(size);
if (tmp == M_NULLPTR) { return; }
// ... use tmp ...
safe_free(&tmp);
```

---

## 17. Prefer `static M_INLINE` Functions Over Function-Like Macros

### Why inline functions are safer

Function-like macros have no type checking. A macro will silently accept the wrong type, double-evaluate arguments with side effects, and can produce confusing diagnostics when they go wrong. Inline functions:

- Get full compiler type checking — mismatched argument types become hard errors
- Show up correctly in debugger stack traces and can have breakpoints set on them
- Evaluate each argument exactly once
- Can carry `M_NODISCARD`, `M_CONST_FUNC`, nullability annotations, and other attributes
- Can be inspected by static analysis tools with full semantic context

The conversion of `M_BytesTo4ByteValue` from a raw bitshift macro to a call into `bytes_To_Uint32` (a typed `static M_INLINE`) found real bugs: callers were passing values of the wrong width that the macro accepted silently but the typed function rejected.

### The preferred pattern: typed inline + thin macro front-end

When a macro name must be preserved for API compatibility, make the macro a thin wrapper that calls a typed inline:

```c
// Implementation: typed, debuggable, annotated
M_CONST_FUNC static M_INLINE uint32_t bytes_To_Uint32(
    uint8_t msb, uint8_t byte2, uint8_t byte1, uint8_t lsb) M_UNSEQUENCED
{
    return (M_STATIC_CAST(uint32_t, msb)  << 24U)
         | (M_STATIC_CAST(uint32_t, byte2) << 16U)
         | (M_STATIC_CAST(uint32_t, byte1) <<  8U)
         |  M_STATIC_CAST(uint32_t, lsb);
}

// Public name: macro delegates to the typed inline — no logic in the macro itself
#define M_BytesTo4ByteValue(b3, b2, b1, b0) bytes_To_Uint32(b3, b2, b1, b0)
```

### Multi-type dispatch: `_Generic` + typed inlines + macro

When the same logical operation needs to work on multiple input types (C11+), use `_Generic` selection backed by typed inline functions, then expose a single macro name:

```c
// Two typed implementations
M_CONST_FUNC static M_INLINE uint16_t get_Word0_uint32(uint32_t value) M_UNSEQUENCED
{ return M_STATIC_CAST(uint16_t, value & UINT32_C(0x0000FFFF)); }

M_CONST_FUNC static M_INLINE uint16_t get_Word0_uint64(uint64_t value) M_UNSEQUENCED
{ return M_STATIC_CAST(uint16_t, value & UINT64_C(0x000000000000FFFF)); }

// Single name dispatches by type — wrong types become compile errors
#define M_Word0(value) \
    _Generic((value), uint32_t: get_Word0_uint32, uint64_t: get_Word0_uint64)(value)
```

### When macros are still appropriate

| Situation | Reason macros are the right tool |
|-----------|----------------------------------|
| Token pasting (`##`) or stringification (`#`) | Cannot be done in a function |
| `_Generic` dispatch front-end | Macro is the dispatch layer; the logic lives in typed inlines |
| Compile-time integer constants (`#define SECTOR_SIZE UINT32_C(512)`) | `static const` or `enum` preferred, but macros are acceptable |
| `safe_free(&ptr)` — type-generic pointer-to-pointer | Uses `_Generic` internally; no equivalent as a plain function |
| Conditional compilation guards | No function equivalent |

For anything involving computation, comparison, or argument evaluation, write a typed `static M_INLINE` function instead.

---

## Standards Reference

The rules in this file are grounded in the following standards and guidelines. Consult these when a pattern is unclear or when adding new library code:

| Standard | Focus |
|----------|-------|
| **ISO/IEC TS 17961:2013** (C Secure Coding Rules — WG14 N1718) | Authoritative rule set for C secure coding; INT07-C mandates zeroing `errno` before `strtol` family; also covers string handling, memory, type safety |
| **CERT C Coding Standard** (SEI) | Practical per-rule guidance derived from ISO/IEC TS 17961; opensea-common functions (e.g., `secure_file.h`, `secured_env_vars.h`) explicitly cite CERT-C rules |
| **OpenSSF Concise Guide for Developing More Secure Software** | Dependency evaluation, MFA, automated scanning, memory safety strategy |
| **OpenSSF Memory Safety Continuum** | Layered strategy: compiler hardening → safe abstractions → annotations → isolation → sanitizers |
| **OWASP Secure Coding Practices** | Input validation, output encoding, error handling, data protection |
| **clang-tidy `cert-*` checks** | Machine-enforceable subset of CERT-C rules; enabled in `.clang-tidy` |
| **clang-tidy `bugprone-*`, `clang-analyzer-*`** | Common bug patterns: use-after-move, suspicious string operations, null dereference |
