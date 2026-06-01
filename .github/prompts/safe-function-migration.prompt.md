---
description: 'Migrate unsafe C standard library calls to safe_ equivalents from opensea-common'
agent: 'agent'
tools: ['search/codebase', 'edit/editFiles', 'execute/runInTerminal', 'read/readFile']
---

# Migrate Unsafe C Library Calls to `safe_*` Equivalents

## Mission

Find all uses of unsafe C standard library functions in a target file or function
and replace them with the bounds-checked `safe_*` equivalents from opensea-common.
Follow all conventions in `.github/instructions/c-secure-coding.instructions.md`
and `.github/instructions/opensea-common.instructions.md`.

## Scope & Preconditions

- Read the C secure coding instruction file before proceeding:
  `.github/instructions/c-secure-coding.instructions.md`
- Read the opensea-common instruction file for the full `safe_*` function reference:
  `.github/instructions/opensea-common.instructions.md`
- This prompt targets `.c` and `.h` files in the openSeaChest subprojects.
- Prefer fixing the entire file over fixing individual call sites — a half-migrated
  file is harder to audit than a complete one.

## Required Headers for safe_* APIs

Include the relevant header(s) from opensea-common at the top of each migrated file:

- `memory_safety.h` — memory allocation, memset/memcpy/memmove, SIZE_OF_STACK_ARRAY, explicit_zeroes
- `string_utils.h` — safe_strcpy, safe_strncpy, safe_strcat, safe_strncat, safe_strdup, safe_strtok, etc.
- `io_utils.h` — safe_fopen, safe_freopen, safe_tmpfile, safe_getline, safe_getdelim, etc.
- `sort_and_search.h` — safe_qsort, safe_bsearch, safe_lfind, safe_lsearch, etc.
- `time_utils.h` — safe_gmtime, safe_localtime, safe_asctime, safe_ctime, etc.

If you are unsure which header provides a function, check `.github/instructions/opensea-common.instructions.md` or the opensea-common/include directory.

## Inputs

| Input | How to obtain |
|-------|---------------|
| Target file or folder | `${input:target:relative path, e.g. subprojects/opensea-transport/src/ata_cmds.c}` |

If the target is a folder, process each `.c` file it contains.
If the input is missing, ask the user for the target before proceeding.

## Migration Map

Apply these substitutions. For each, update call sites to handle the return value
(`errno_t` in most cases) and pass the correct destination size.

### Memory allocation
| Unsafe | Safe replacement | Notes |
|--------|-----------------|-------|
| `malloc(n)` | `safe_malloc(n)` | Check return for `M_NULLPTR` |
| `calloc(count, size)` | `safe_calloc(count, size)` | Check return for `M_NULLPTR` |
| `realloc(ptr, n)` | `safe_realloc(ptr, n)` | Check return; original pointer is unchanged on failure |
| `realloc(ptr, n)` with free-on-failure pattern | `safe_reallocf(&ptr, n)` | Frees original block on failure and NULLs pointer |
| `free(ptr)` | `safe_free(&ptr)` | Takes pointer-to-pointer; NULLs it automatically |

Aligned allocation variants (when required by API/device/OS constraints):
- `safe_malloc_aligned(size, alignment)`
- `safe_calloc_aligned(count, size, alignment)`
- `safe_realloc_aligned(block, originalSize, size, alignment)`
- `safe_reallocf_aligned(&block, originalSize, size, alignment)`
- `safe_free_aligned(&ptr)`

Page-aligned allocation variants:
- `safe_malloc_page_aligned(size)`
- `safe_calloc_page_aligned(count, size)`
- `safe_free_page_aligned(&ptr)`

Parameter accuracy note:
- Use exact signatures from opensea-common headers.
- `safe_realloc` takes `(block, size)`.
- `originalSize` is required by `safe_realloc_aligned` / `safe_reallocf_aligned` for preserving existing data.

### Memory operations
| Unsafe | Safe replacement |
|--------|-----------------|
| `memset(dst, val, n)` | `safe_memset(dst, destsz, val, n)` |
| `memcpy(dst, src, n)` | `safe_memcpy(dst, destsz, src, n)` |
| `memmove(dst, src, n)` | `safe_memmove(dst, destsz, src, n)` |
| `memccpy(dst, src, c, n)` | `safe_memccpy(dst, destsz, src, c, n)` |
| `memcmove(dst, src, c, n)` | `safe_memcmove(dst, destsz, src, c, n)` |

Set `destsz` as follows:
- Stack arrays: `SIZE_OF_STACK_ARRAY(array)` (from `memory_safety.h`).
- Heap buffers: the tracked allocation-size variable/parameter.
- Never use `sizeof(pointer)` as `destsz` for heap memory; it yields pointer width, not allocation size.

For zeroing sensitive data (keys, passwords), use `explicit_zeroes()` — the
compiler cannot optimise it away.

### String operations
| Unsafe | Safe replacement | Notes |
|--------|-----------------|-------|
| `strcpy(dst, src)` | `safe_strcpy(dst, destsz, src)` | Returns `errno_t` |
| `strcpy(dst, src)` (overlap possible) | `safe_strmove(dst, destsz, src)` | Use when source/destination ranges may overlap |
| `strncpy(dst, src, n)` | `safe_strncpy(dst, destsz, src, n)` | Returns `errno_t` |
| `strcat(dst, src)` | `safe_strcat(dst, destsz, src)` | Returns `errno_t` |
| `strncat(dst, src, n)` | `safe_strncat(dst, destsz, src, n)` | Returns `errno_t` |
| `strtok(str, delim)` | `safe_strtok(str, strmax, delim, &saveptr)` | Thread-safe |
| `strdup(src)` | `safe_strdup(&dup, src)` | Returns `errno_t`; caller frees dup |
| `strndup(src, n)` | `safe_strndup(&dup, src, n)` | Returns `errno_t`; caller frees dup |
| `strlen(s)` | `safe_strlen(s)` | Safe version |
| `strnlen(s, n)` | `safe_strnlen(s, n)` | Bounded-length lookup |

### String-to-number conversion
| Unsafe | Safe replacement | Notes |
|--------|-----------------|-------|
| `atoi(s)` | `safe_atoi(&value, s)` | Output parameter; check `errno_t` |
| `atol(s)` | `safe_atol(&value, s)` | Output parameter |
| `atoll(s)` | `safe_atoll(&value, s)` | Output parameter |
| `strtol(s, &end, base)` | `safe_strtol(&value, s, &end, base)` | Output parameter |
| `strtoul(s, &end, base)` | `safe_strtoul(&value, s, &end, base)` | Output parameter |
| `strtoll(s, &end, base)` | `safe_strtoll(&value, s, &end, base)` | Output parameter |
| `strtoull(s, &end, base)` | `safe_strtoull(&value, s, &end, base)` | Output parameter |
| `strtoimax(s, &end, base)` | `safe_strtoimax(&value, s, &end, base)` | Output parameter |
| `strtoumax(s, &end, base)` | `safe_strtoumax(&value, s, &end, base)` | Output parameter |
| `atof(s)` | `safe_atof(&value, s)` | Output parameter |
| `strtof(s, &end)` | `safe_strtof(&value, s, &end)` | Output parameter |
| `strtod(s, &end)` | `safe_strtod(&value, s, &end)` | Output parameter |
| `strtold(s, &end)` | `safe_strtold(&value, s, &end)` | Output parameter |

### File I/O
| Unsafe | Safe replacement | Notes |
|--------|-----------------|-------|
| `fopen(name, mode)` | `safe_fopen(&stream, name, mode)` | Returns `errno_t` |
| `freopen(name, mode, stream)` | `safe_freopen(&stream, name, mode, old)` | Returns `errno_t` |
| `tmpfile()` | `safe_tmpfile(&stream)` | Returns `errno_t` |
| `tmpnam(buf)` | `safe_tmpnam(buf, bufsz)` (discouraged) | Returns `errno_t` |
| `gets(buf)` | `safe_gets(buf, bufsz)` | Returns `errno_t` |
| `getline(&line, &allocSize, stream)` | `safe_getline(&line, &allocSize, &charsRead, stream)` | Returns `errno_t` |
| `getdelim(&line, &allocSize, stream)` | `safe_getdelim(&line, &allocSize, &charsRead, delim, stream)` | Returns `errno_t` |

`safe_tmpnam` is available only with `WANT_SAFE_TMPNAM` and should generally be avoided.
Prefer `safe_tmpfile` or an API with explicit file creation semantics.

`safe_getline` / `safe_getdelim` use caller-managed pointers with allocation tracking
and return `errno_t` (with `charsRead` out-parameter), so update call-site control
flow accordingly.

### Formatted output
| Unsafe | Safe replacement | Notes |
|--------|-----------------|-------|
| `snprintf(buf, bufsz, fmt, ...)` | `snprintf_err_handle(buf, bufsz, fmt, ...)` | Error-handled wrapper; always null-terminates |
| `vsnprintf(buf, bufsz, fmt, args)` | `vsnprintf_err_handle(buf, bufsz, fmt, args)` | Error-handled wrapper |

### Sorting and searching
| Unsafe | Safe replacement |
|--------|-----------------|
| `qsort(base, n, size, cmp)` | `safe_qsort(base, n, size, cmp)` |
| `bsearch(key, base, n, size, cmp)` | `safe_bsearch(key, base, n, size, cmp)` |
| comparator needs external state/context | `safe_qsort_context` / `safe_bsearch_context` | ask user if context-aware comparator is needed |
| linear context-aware lookup/insert | `safe_lfind_context` / `safe_lsearch_context` | optional, based on call-site behavior |

Context-aware sort/search functions are not always direct conversions.
If comparator state is currently global or closure-like, ask the user whether to
migrate to `_context` variants during the same change.

### Time conversion helpers
| Unsafe | Safe replacement | Notes |
|--------|-----------------|-------|
| `gmtime(timer)` | `safe_gmtime(timer, buf)` | Returns `errno_t` |
| `localtime(timer)` | `safe_localtime(timer, buf)` | Returns `errno_t` |
| `asctime(time_ptr)` | `safe_asctime(buf, size, time_ptr)` | Returns `errno_t` |
| `ctime(timer)` | `safe_ctime(buf, size, timer)` | Returns `errno_t` |

These time helpers are reentrant/thread-safer because callers provide the output
buffers/structures instead of relying on shared static internal storage.

## Workflow

### 1. Scan the target file
Search for all occurrences of the unsafe functions listed in the Migration Map.
Produce a list of line numbers and call sites before making any changes.

### 2. Classify each call site
For each unsafe call found:
- Identify the destination buffer size (static array, dynamic allocation, or unknown).
- Determine whether the return value is currently used or discarded.
- Note whether error handling will need to be added.

### 3. Apply substitutions
Replace each call site following the Migration Map. For each replacement:
- Pass the destination buffer size as the `destsz` / `rsize_t` argument.
  - For stack arrays: `SIZE_OF_STACK_ARRAY(array)`.
  - For heap buffers: the allocation size variable/parameter.
  - Never pass `sizeof(ptr)` for heap allocations.
- Convert `free(ptr)` → `safe_free(&ptr)` (pass address of pointer).
- For string-to-number conversions: introduce a local variable to hold the output.
- For `safe_getline` / `safe_getdelim`: adapt to out-parameters (`lineptr`, allocated
  size, chars read) and `errno_t`-based return handling.
- For `tmpnam`: do not introduce new usage; prefer replacing with `safe_tmpfile`
  unless user explicitly requires path-only behavior.
- For realloc migration patterns that free on failure, prefer `safe_reallocf(&ptr, size)`
  over open-coded `realloc` + `safe_free` sequences.
- If alignment/page-alignment is required, use the aligned/page-aligned `safe_*`
  allocators and matching free functions.
- For sort/search with comparator state: ask the user whether `_context` variants
  should be adopted in the same migration.
- Check the `errno_t` return where failure matters; add error handling if absent.

### 4. Verify no unsafe calls remain
After substitution, re-scan the file to confirm no unsafe calls from the Migration
Map are still present.

### 5. Build verification
Run the build to confirm the file compiles without errors or new warnings:
```
meson compile -C builddir
```
Fix any type mismatches introduced by the output-parameter pattern of `safe_*`
conversion functions.

## Important Note: Do NOT Mix Up `safe_free` Variants

There are multiple `safe_free` functions in opensea-common, each designed for a specific allocation type:

- `safe_free(&ptr)` — for standard heap allocations from `safe_malloc`, `safe_calloc`, or `safe_realloc`.
- `safe_free_aligned(&ptr)` — for memory allocated with `safe_malloc_aligned`, `safe_calloc_aligned`, or `safe_realloc_aligned`.
- `safe_free_page_aligned(&ptr)` — for memory allocated with `safe_malloc_page_aligned` or `safe_calloc_page_aligned`.

**You must use the correct `safe_free` variant for the corresponding allocation function.**

Mixing up these functions (e.g., freeing aligned memory with plain `safe_free`, or vice versa) can cause memory corruption, crashes, or leaks. Always match the free function to the allocation method used.

If unsure, trace the allocation site and confirm which variant was used.

## Output Expectations

- All targeted unsafe calls replaced with `safe_*` equivalents.
- Return values of `safe_*` functions checked where failure is actionable.
- `safe_free(&ptr)` replaces every `free(ptr)` — pointer is NULLed after free.
- No regression in build output (no new warnings or errors).
- No functional behaviour change — this is a pure safety migration.

## Quality Assurance

- [ ] All `malloc`/`calloc`/`realloc`/`free` replaced
- [ ] `safe_reallocf(&ptr, size)` used when legacy logic frees original pointer on realloc failure
- [ ] Aligned/page-aligned allocators used when alignment constraints exist
- [ ] All `memset`/`memcpy`/`memmove` replaced
- [ ] `memccpy`/`memcmove` migrated to `safe_memccpy`/`safe_memcmove` where present
- [ ] All `strcpy`/`strcat`/`strtok` replaced
- [ ] All `atoi`/`strtol` family replaced with output-parameter variants
- [ ] All `fopen`/`tmpfile` replaced
- [ ] `safe_free(&ptr)` used (not `safe_free(ptr)`)
- [ ] `SIZE_OF_STACK_ARRAY(array)` used for stack-array `destsz` arguments
- [ ] No `sizeof(ptr)` used as `destsz` for heap buffers
- [ ] `snprintf`/`vsnprintf` migrations use `snprintf_err_handle` / `vsnprintf_err_handle`
- [ ] `tmpnam` is not newly introduced (or explicitly justified)
- [ ] `_context` sort/search variants considered when comparator state exists
- [ ] `errno_t` returns checked where error handling is meaningful
- [ ] `explicit_zeroes()` used instead of `memset(..., 0, ...)` for sensitive data
- [ ] Build passes cleanly after migration
