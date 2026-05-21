---
description: 'OpenSSF Compiler Annotations guidance — mapping code_attributes.h macros to the OpenSSF Compiler Annotations for C and C++ guide'
applyTo: '**/*.c, **/*.h, **/*.cpp, **/*.hpp, **/*.cc, **/*.cxx'
---

# OpenSSF Compiler Annotations — openSeaChest

## Overview

`subprojects/opensea-common/include/code_attributes.h` is the project's comprehensive portable annotation system. It covers **everything in** the [OpenSSF Compiler Annotations for C and C++](https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Annotations-for-C-and-C++.html) guide and goes significantly beyond it, tracking attributes from both GCC and Clang that the guide has not yet codified.

**Always use the macros from `code_attributes.h` instead of raw `__attribute__` or `[[…]]` syntax.** The macros are portable across GCC, Clang, MSVC, and old compilers via `DETECT_GNU_ATTR`/`DETECT_STD_ATTR`/`DETECT_STD_ATTR_QUAL` detection wrappers.

> **Note:** Compiler annotations are a rapidly evolving space. GCC and Clang regularly introduce new attributes beyond what standards bodies and best-practice guides have codified. The project proactively tracks these additions in `code_attributes.h`. When looking for an annotation not listed here, consult the [GCC Common Function Attributes](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html) and [Clang Attribute Reference](https://clang.llvm.org/docs/AttributeReference.html) directly, and add a new macro following the established detection pattern if needed.

---

## Complete Macro Reference

### Memory Allocation

| Macro | Underlying Attribute | OpenSSF Guide |
|-------|---------------------|:---:|
| `M_FUNC_ATTR_MALLOC` | `malloc` (no-alias return) | §2.1 |
| `M_ALLOC_DEALLOC(fn, pos)` | `malloc(deallocator, ptr-index)` (GCC 11+) | §2.1 |
| `M_MALLOC_SIZE(sz)` | `alloc_size(sz)` (GCC 2.95+ / Clang 4+) | §2.2 |
| `M_CALLOC_SIZE(n, sz)` | `alloc_size(n, sz)` — two-argument form | §2.2 |
| `M_ALLOC_ALIGN(pos)` | `alloc_align(pos)` (GCC 4.9+ / Clang 3.7+) | — |

Use `M_FUNC_ATTR_MALLOC` on all `malloc`/`calloc`-style functions. Do **not** apply to `realloc`-style functions (they may alias existing storage).

`M_MALLOC_SIZE`/`M_CALLOC_SIZE` improves `_FORTIFY_SOURCE=3` accuracy because `__builtin_object_size` uses this information.

Use `M_ALLOC_DEALLOC` to associate the paired deallocation function, enabling GCC's static analyzer to warn on mismatched deallocation, double-free, and memory leaks:

```c
M_NODISCARD M_FUNC_ATTR_MALLOC M_MALLOC_SIZE(1) M_ALLOC_DEALLOC(safe_free_impl, 1)
void* safe_malloc(size_t size);

M_NODISCARD M_FUNC_ATTR_MALLOC M_CALLOC_SIZE(1, 2) M_ALLOC_DEALLOC(safe_free_impl, 1)
void* safe_calloc(size_t count, size_t size);
```

### Parameter Access Modes

| Macro | Underlying Attribute | OpenSSF Guide |
|-------|---------------------|:---:|
| `M_PARAM_RO(arg)` | `access(read_only, arg)` (GCC 11+) | §2.3 |
| `M_PARAM_WO(arg)` | `access(write_only, arg)` (GCC 11+) | §2.3 |
| `M_PARAM_RW(arg)` | `access(read_write, arg)` (GCC 11+) | §2.3 |
| `M_PARAM_RO_SIZE(arg, sz)` | `access(read_only, arg, sz)` | §2.3 |
| `M_PARAM_WO_SIZE(arg, sz)` | `access(write_only, arg, sz)` | §2.3 |
| `M_PARAM_RW_SIZE(arg, sz)` | `access(read_write, arg, sz)` | §2.3 |

```c
M_PARAM_RO_SIZE(2, 3) M_PARAM_WO_SIZE(1, 3)
errno_t safe_memcpy(void* dest, size_t destsz, const void* src, size_t count);
```

### File Descriptor Safety

| Macro | Underlying Attribute | OpenSSF Guide |
|-------|---------------------|:---:|
| `M_FILE_DESCRIPTOR(n)` | `fd_arg(n)` (GCC 13+) | §2.5 |
| `M_FILE_DESCRIPTOR_R(n)` | `fd_arg_read(n)` (GCC 13+) | §2.5 |
| `M_FILE_DESCRIPTOR_W(n)` | `fd_arg_write(n)` (GCC 13+) | §2.5 |

Enables GCC `-fanalyzer` to catch double-close, use-after-close, fd leaks, and access-mode mismatches.

```c
M_FILE_DESCRIPTOR_R(1) ssize_t read_from_fd(int fd, void* buf, size_t n);
```

### Nullability

| Macro | Underlying Attribute | Notes |
|-------|---------------------|-------|
| `M_NULLABLE` | `_Nullable` (Clang) | Pointer may be null |
| `M_NONNULL` | `_Nonnull` (Clang) | Pointer must not be null |
| `M_NULL_UNSPECIFIED` | `_Null_unspecified` | Unspecified nullability |
| `M_NONNULL_ARRAY` | `static` (C99 array qualifier) | Non-null, at-least-N-elements |
| `M_ALL_PARAMS_NONNULL` | `nonnull` | All pointer params non-null |
| `M_NONNULL_PARAM_LIST(...)` | `nonnull(idx...)` | Named params non-null |
| `M_NONNULL_IF_NONZERO_SIZE(p, sz)` | `nonnull_if_nonzero` | Conditional: non-null when size ≠ 0 |
| `M_NONNULL_IF_NONZERO_SIZE_COUNT(p, sz, n)` | `nonnull_if_nonzero` (two-arg) | Conditional: size × count |
| `M_RETURNS_NONNULL` | `returns_nonnull` | Return is never null |
| `M_NULL_TERM_STRING(arg)` | `null_terminated_string_arg` | Param is a null-terminated string |

> **Warning:** `M_ALL_PARAMS_NONNULL` and `M_NONNULL_PARAM_LIST` let the optimizer remove null checks. Use only for internal functions where null is provably impossible. For public APIs, prefer `M_NONNULL` as a pointer type decorator instead.

### Flexible Array Members and Pointer Bounds

| Macro | Underlying Attribute | OpenSSF Guide |
|-------|---------------------|:---:|
| `M_COUNTED_BY(member)` | `counted_by(member)` (GCC 15+ / Clang 18+) | §2.9 |
| `M_COUNTED_BY_OR_NULL(member)` | `counted_by_or_null(member)` (Clang 19+) | §2.9 |
| `M_SIZED_BY(member)` | `sized_by(member)` (Clang 19+) | §2.9 |
| `M_SIZED_BY_OR_NULL(member)` | `sized_by_or_null(member)` (Clang 19+) | §2.9 |
| `M_STRICT_FLEX_ARRAY(level)` | `strict_flex_array(level)` (GCC / Clang) | — |
| `M_STRICT_FLEX_ARRAY_AUTO` | Auto-selects level from `FLEX_LEVEL` | — |
| `FLEX_ARRAY` | `[]` / `[0]` / `[1]` per standard and compiler | — |

`M_COUNTED_BY` improves `-fsanitize=bounds` and `__builtin_dynamic_object_size` accuracy. Always combine with `M_STRICT_FLEX_ARRAY_AUTO` and `FLEX_ARRAY`:

```c
struct DataBuffer
{
    size_t count;
    M_COUNTED_BY(count) M_STRICT_FLEX_ARRAY_AUTO uint8_t data[FLEX_ARRAY];
};
```

### Format String Safety

| Macro | Underlying Attribute |
|-------|---------------------|
| `FUNC_ATTR_PRINTF(fmt, va)` | `format(printf, fmt, va)` |
| `FUNC_ATTR_SCANF(fmt, va)` | `format(scanf, fmt, va)` |
| `FUNC_ATTR_SCANF_S(fmt, va)` | `format(scanf, fmt, va)` |
| `M_NONSTRING` | `nonstring` — char array may not be null-terminated |

Apply `FUNC_ATTR_PRINTF` to any function with a printf-style format string. Set the variadic position to 0 when the function takes a `va_list`. Use `M_NONSTRING` on fixed-length character buffers that may not be null-terminated to suppress false warnings from string functions.

### Control Flow

| Macro | Underlying Attribute |
|-------|---------------------|
| `M_NORETURN` | `noreturn` / `[[noreturn]]` / `_Noreturn` |
| `M_UNREACHABLE()` | `__builtin_unreachable()` / `__assume(0)` |
| `M_FALLTHROUGH` | `fallthrough` / `__fallthrough__` |
| `M_ASSUME(condition)` | `[[assume(condition)]]` (C++23) / `__builtin_assume` |

### Function Purity / Side-Effect Hierarchy

These four attributes form a strictness hierarchy, from most to least restrictive:

| Macro | Attribute | Reads globals? | Modifies memory? | Placement |
|-------|-----------|:-:|:-:|-----------|
| `M_CONST_FUNC` | `const` | ✗ | ✗ | **Front** (before return type) |
| `M_PURE_FUNC` | `pure` | ✓ | ✗ | **Front** (before return type) |
| `M_REPRODUCIBLE` | `reproducible` (C23) | ✓ | ✗ | **End** (after parameter list) |
| `M_UNSEQUENCED` | `unsequenced` (C23) | ✗ | ✗ | **End** (after parameter list) |

Apply the most restrictive attribute that fits. Do not use `M_CONST_FUNC` with `M_FUNC_ATTR_MALLOC`. Do not use `M_PURE_FUNC` on any function with output-pointer parameters or `void` return.

> **Placement warning:** `M_CONST_FUNC`/`M_PURE_FUNC` and `M_REPRODUCIBLE`/`M_UNSEQUENCED` have **opposite** placement requirements despite being in the same semantic family. See the Placement Rules section below.

### Branch Prediction Hints

| Macro | Underlying Attribute |
|-------|---------------------|
| `M_LIKELY` | `[[likely]]` / `__attribute__((likely))` |
| `M_UNLIKELY` | `[[unlikely]]` / `__attribute__((unlikely))` |
| `M_HOT_FUNC` | `hot` — frequently called function |
| `M_COLD_FUNC` | `cold` — infrequently called function |

### Result Discarding

| Macro | Underlying Attribute |
|-------|---------------------|
| `M_NODISCARD` | `nodiscard` / `warn_unused_result` |
| `M_NODISCARD_REASON(msg)` | `nodiscard(msg)` (C++20/C23) |

Apply `M_NODISCARD` to all functions whose return value encodes an error code or represents an owned resource.

### Taint Tracking (Attack Surface Annotation)

| Macro | Underlying Attribute | OpenSSF Guide |
|-------|---------------------|:---:|
| `M_TAINTED_ARGS` | `tainted_args` (GCC 12+) | §2.7 |

Marks functions whose arguments come from untrusted external sources (the attack surface). GCC's `-fanalyzer` then tracks tainted values through the call graph to detect unsanitized use in sensitive operations (allocation sizes, array indices, divisors, pointer offsets).

```c
// Function directly accepting user/network/device data
M_TAINTED_ARGS int process_user_command(const uint8_t* cmd, size_t len);
```

### Deprecation

| Macro | Use |
|-------|-----|
| `M_DEPRECATED` | Mark deprecated function/variable |
| `M_DEPRECATED_REASON(msg)` | With explanatory message |
| `M_ENUM_DEPRECATED` | Deprecated enum value |
| `M_ENUM_DEPRECATED_REASON(msg)` | With explanatory message |

### Enum Safety (Clang)

| Macro | Underlying Attribute |
|-------|---------------------|
| `M_FLAG_ENUM` | `flag_enum` — values can be OR'd safely |
| `M_CLOSED_ENUM` | `enum_extensibility(closed)` |
| `M_OPEN_ENUM` | `enum_extensibility(open)` |

Use `M_FLAG_ENUM` on bitmask enumerations. Combine with `M_CLOSED_ENUM` when external code should never use values outside the declared set.

### Alignment

| Macro | Underlying Attribute |
|-------|---------------------|
| `M_ALIGNOF(x)` | `alignof` / `_Alignof` / `__alignof__` |
| `M_ALIGNAS(x)` | `alignas` / `_Alignas` / `__attribute__((aligned(x)))` |
| `M_WARN_IF_NOT_ALIGNED(n)` | `warn_if_not_aligned(n)` |

### Compile-time Diagnostics (Clang)

```c
// Warn at call site if condition is true at compile time
M_DIAG_WARNING(!ptr, "ptr must not be null")
void process(void* ptr);

// Error at call site
M_DIAG_ERROR(len > MAX_LEN, "length exceeds buffer")
void set_string(const char* s, size_t len);
```

### Visibility

| Macro | Underlying Attribute |
|-------|---------------------|
| `DLL_EXPORT` | `visibility("default")` / `__declspec(dllexport)` |
| `DLL_IMPORT` | `visibility("default")` / `__declspec(dllimport)` |

### Other

| Macro | Underlying Attribute | Notes |
|-------|---------------------|-------|
| `M_RESTRICT` | `restrict` / `__restrict` / `__restrict__` | Pointer no-alias hint |
| `M_INLINE` / `M_NOINLINE` / `M_FORCEINLINE` | `inline` / `noinline` / `always_inline` | Inlining control |
| `M_MS_STRUCT` | `ms_struct` | MSVC-compatible struct layout |

---

## Not Yet Implemented

The only attributes not yet in `code_attributes.h` are the **Clang static analyzer ownership attributes** — a TODO comment already exists in the file:

| Clang Attribute | Purpose |
|----------------|---------|
| `ownership_returns(type)` | Associate returned pointer with allocation type |
| `ownership_takes(type, pos)` | Mark function as deallocator for allocation type |
| `ownership_holds(type, pos)` | Mark function as taking ownership (will free later) |

These are the Clang-side equivalent of `M_ALLOC_DEALLOC` (which covers GCC's `malloc(deallocator, pos)` form). They interact with the Clang static analyzer to catch mismatched deallocation, double-free, and memory leaks.

---

## Attribute Placement Rules

Placement of annotations varies by compiler and attribute family. This project standardizes on a consistent scheme to maintain compatibility across GCC, Clang, and MSVC simultaneously.

### General Rule: Place Before the Return Type

The default placement for all macros is **before the return type**, at the very start of the declaration:

```c
M_NODISCARD M_FUNC_ATTR_MALLOC M_MALLOC_SIZE(1) M_ALLOC_DEALLOC(safe_free_impl, 1)
void* safe_malloc(size_t size);
```

This position is required by MSVC's `__declspec(...)` equivalents (`__declspec(noreturn)`, `__declspec(restrict)`, `__declspec(noinline)`, etc.), which must always appear before the return type. Since the macros may expand to either `__attribute__((...))` or `__declspec(...)` depending on the compiler, the front position is the safe universal location.

### Exception: End-of-Declaration Attributes

Some attributes are **positionally fixed** and must appear **after the closing parenthesis of the parameter list**. This is a property of the specific attribute, not of the syntax used — both `[[...]]` and `__attribute__((...))` forms behave the same way:

- `M_DIAG_WARNING` / `M_DIAG_ERROR` (`diagnose_if`) — Clang requires these after the parameter list.
- `M_REPRODUCIBLE` (`reproducible`) — GCC errors if placed before the return type; must come after the parameters regardless of whether `[[reproducible]]` or `__attribute__((reproducible))` is used.
- `M_UNSEQUENCED` (`unsequenced`) — Same requirement as `M_REPRODUCIBLE`.

```c
// M_DIAG_ERROR, M_REPRODUCIBLE, M_UNSEQUENCED always go after the parameters
void process(const char* M_NONNULL input, size_t len)
    M_DIAG_ERROR(len == 0, "len must be non-zero");

int get_cached_value(int key) M_REPRODUCIBLE;

int pure_transform(int x) M_UNSEQUENCED;
```

### Exception: GNU `const` and `pure` Must Be at the Front

`M_CONST_FUNC` and `M_PURE_FUNC` expand to `__attribute__((const))` and `__attribute__((pure))`. GCC only recognizes these when placed **before the return type**; placing them after the parameter list (even using `[[gnu::const]]` syntax) produces an error or is silently ignored:

```c
// Correct for GCC: const/pure before return type
M_CONST_FUNC M_NODISCARD int byte_swap_32(uint32_t value);

// Do NOT place after parameters — GCC rejects or ignores it
int byte_swap_32(uint32_t value) __attribute__((const)); // Avoid
```

> **Note:** `const`/`pure` (front-required) and `reproducible`/`unsequenced` (end-required) are all in the same purity hierarchy (see the table above), but they have **opposite** placement requirements. This is a known GCC-ism — do not assume that attributes in the same semantic category share the same placement rules.

### Repeat Attributes in Both .h and .c Files

Unlike GCC/Clang which only need attributes on the declaration (`.h`), **MSVC `__declspec` attributes must also appear on the definition** (`.c`/`.cpp`) to take effect. Therefore, repeat all macros that expand to `__declspec` equivalents on both the header declaration and the source definition:

```c
// opensea-something.h
M_NODISCARD M_NONNULL M_FUNC_ATTR_MALLOC M_MALLOC_SIZE(1)
void* safe_malloc(size_t size);

// opensea-something.c — repeat the same macros on the definition
M_NODISCARD M_NONNULL M_FUNC_ATTR_MALLOC M_MALLOC_SIZE(1)
void* safe_malloc(size_t size)
{
    // ...
}
```

Attributes that are purely GCC/Clang analyzer hints with no MSVC equivalent (e.g., `M_TAINTED_ARGS`, `M_FILE_DESCRIPTOR_*`, `M_COUNTED_BY`) only need to appear on the declaration, but repeating them on the definition is harmless and preferred for consistency.

---

## Evolving Attribute Landscape

Compiler annotations are actively evolving. Neither the OpenSSF guide nor this file should be treated as exhaustive. Check these sources when implementing new safe abstractions:

- [GCC Common Function Attributes](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html)
- [GCC Common Variable Attributes](https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html)
- [Clang Attribute Reference](https://clang.llvm.org/docs/AttributeReference.html)
- [cppreference C attributes](https://en.cppreference.com/w/c/language/attributes)
- [OpenSSF Compiler Annotations Guide](https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Annotations-for-C-and-C++.html)

When adding a new attribute macro to `code_attributes.h`, follow the established pattern: detect with `DETECT_STD_ATTR_QUAL` first, then `DETECT_GNU_ATTR`, then provide a comment-only fallback. Document the macro with its minimum compiler version in the Doxygen header.

> **TODO (documentation phase):** Every new attribute macro added to `code_attributes.h` must include explicit placement requirements in its Doxygen `\brief` or a dedicated `\note`. Specifically: whether it must appear before the return type, after the parameter list, or in both places. The existing macros for `M_REPRODUCIBLE`, `M_UNSEQUENCED`, `M_CONST_FUNC`, `M_PURE_FUNC`, and `M_DIAGNOSE_IF` are known to be missing this documentation today and should be updated in the documentation phase.
