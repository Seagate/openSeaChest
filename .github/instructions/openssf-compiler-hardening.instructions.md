---
description: 'OpenSSF compiler hardening flags for openSeaChest — mapping the OpenSSF Compiler Options Hardening Guide to flags already active in meson.build'
applyTo: '**/meson.build, **/meson_options.txt'
---

# OpenSSF Compiler Hardening — openSeaChest

## Overview

This project follows the [OpenSSF Compiler Options Hardening Guide for C and C++](https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++.html). All hardening flags are applied in `meson.build` via `warning_flags` and `linker_flags` arrays. Meson's `b_pie=true` default option enables PIE for all targets.

When adding new flags, check the guide first — it documents which flags apply to which compiler/linker version, and whether there are performance or compatibility caveats.

## Status: What Is Already Enabled

The following OpenSSF-recommended flags are **active in `meson.build`** for GCC/Clang builds:

### Compile-time warning flags (Table 1)

| Flag | Status | Notes |
|------|--------|-------|
| `-Wformat` / `-Wformat=2` | ✅ Active | Both specified for GCC+Clang compat |
| `-Werror=format-security` | ✅ Active | Treats unsafe format strings as errors |
| `-Wimplicit-fallthrough` | ✅ Active | Use `M_FALLTHROUGH` macro to annotate intentional cases |
| `-Wtrampolines` | ✅ Active | GCC only; detects nested functions needing executable stack |
| `-Werror=implicit` | ✅ Active | Obsolete C implicit declarations |
| `-Werror=incompatible-pointer-types` | ✅ Active | |
| `-Werror=int-conversion` | ✅ Active | |
| `-Wbidi-chars` | ❌ Not set | Consider adding `-Wbidi-chars=any` (GCC 12+) for Trojan Source defence |

### Run-time protection flags (Table 2)

| Flag | Status | Notes |
|------|--------|-------|
| `-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3` | ✅ Active | Probed at configure time; falls back gracefully |
| `-D_GLIBCXX_ASSERTIONS` | ✅ Active | C++ standard library precondition checks |
| `-fstrict-flex-arrays=3` | ✅ Active | Use `[]` for flexible array members, not `[0]` or `[1]` |
| `-fstack-clash-protection` | ✅ Active | Disabled on Solaris; conditional on illumos |
| `-fstack-protector-strong` | ✅ Active | Disabled on Solaris; conditional on illumos |
| `-fcf-protection=full` | ✅ Active | x86_64 only |
| `-mbranch-protection=standard` | ✅ Active | AArch64 only |
| `-fno-delete-null-pointer-checks` | ✅ Active | Retains null checks the compiler might otherwise remove |
| `-fno-strict-overflow` | ✅ Active | Defines signed overflow as two's-complement wrap |
| `-fno-strict-aliasing` | ✅ Active | Prevents removal of type-punning-safe code |
| `-ftrivial-auto-var-init=zero` | ✅ Active | Zero-initializes uninitialized stack variables |
| `-fzero-init-padding-bits=all` | ✅ Active | GCC 15+; zeros padding bits in struct/union initializers |
| `-fPIE -pie` | ✅ Active | Via Meson `b_pie=true` |

### Linker flags

| Flag | Status | Notes |
|------|--------|-------|
| `-Wl,-z,nodlopen` | ✅ Active | Restricts `dlopen(3)` on shared objects |
| `-Wl,-z,noexecstack` | ✅ Active | Non-executable stack |
| `-Wl,-z,relro` | ✅ Active | Partial RELRO |
| `-Wl,-z,now` | ✅ Active | Full RELRO (resolves GOT at startup) |
| `-Wl,--as-needed` | ❌ Not set | Consider adding to reduce attack surface |
| `-Wl,--no-copy-dt-needed-entries` | ❌ Not set | Already the linker default since Binutils 2.22 |

### MSVC (Windows)

| Feature | Flag | Status |
|---------|------|--------|
| Stack cookie | `/GS` | ✅ Active |
| SDL checks | `/sdl` | ✅ Active |
| Spectre mitigations | `/Qspectre` | ✅ Active |
| Control Flow Guard | `/guard:cf` | ✅ Active (compiler + linker) |
| DEP / NX | `/NXCOMPAT` | ✅ Active |
| ASLR | `/dynamicbase` | ✅ Active |
| Safe SEH | `/SafeSEH` | ✅ Active (x86 only; no-op on x64) |

## Flags NOT Yet Enabled (Candidates)

These are in the OpenSSF guide but not yet in `meson.build`:

| Flag | Guide Section | Why Not Yet / Caution |
|------|---------------|----------------------|
| `-Wconversion` / `-Wsign-conversion` | §3.3 | Very noisy on existing brown-field code; `-Wsign-conversion` is enabled for GCC ≥ 10 only |
| `-Wbidi-chars=any` | §3.6 | Add if all comments/strings are expected to be LTR |
| `-fexceptions` | §3.24 | Only relevant for multi-threaded C code using glibc pthreads |
| `-fhardened` | §3.25 | GCC 14+ umbrella option; would need to audit for conflicts with explicit flags |
| `-Wl,--as-needed` | §3.26 | Low risk; worth adding |

## Key Rules When Modifying Build Configuration

1. **Always check `_FORTIFY_SOURCE` compatibility** — the build already probes whether the compiler supports `_FORTIFY_SOURCE=3` using a `compiles()` test and falls back gracefully. Never hard-code `_FORTIFY_SOURCE` without the undefine-first pattern: `-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3`.

2. **Architecture-gated flags** — `fcf-protection=full` is x86_64 only; `mbranch-protection=standard` is AArch64 only. New arch-specific flags must be wrapped in `target_machine.cpu_family()` checks.

3. **OS-gated flags** — stack protector flags are suppressed on `sunos` (non-illumos Solaris). Any new flags that depend on Linux-specific linker behaviour need similar guards.

4. **Sanitizers conflict with `_FORTIFY_SOURCE`** — do not combine `_FORTIFY_SOURCE` with AddressSanitizer, ThreadSanitizer, or MemorySanitizer builds. The guide explicitly notes this incompatibility.

5. **`-Werror` policy** — selective `-Werror=<flag>` forms (already in use) are safe to distribute. Blanket `-Werror` is intentionally absent from the build config per OpenSSF guidance.

## Adding a New Hardening Flag

```python
# In meson.build warning_flags array (GCC/Clang block):
if c.has_argument('-Wsome-new-flag')
    warning_flags += '-Wsome-new-flag'
endif

# OR for linker flags:
if c.has_link_argument('-Wl,-z,some-option')
    linker_flags += '-Wl,-z,some-option'
endif
```

Always probe with `has_argument()` / `has_link_argument()` for flags that may not be available on all supported compiler versions.
