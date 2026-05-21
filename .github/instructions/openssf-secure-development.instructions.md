---
description: 'OpenSSF secure development practices — Concise Guide for Developing More Secure Software and Memory Safety Continuum, applied to openSeaChest'
applyTo: '**'
---

# OpenSSF Secure Development Practices — openSeaChest

Sources:
- [Concise Guide for Developing More Secure Software](https://best.openssf.org/Concise-Guide-for-Developing-More-Secure-Software)
- [Memory Safety Continuum](https://best.openssf.org/Memory-Safety/Memory-Safety-Continuum)

---

## Access and Identity

- All privileged project contributors (write access to the repository) must use multi-factor authentication (MFA/2FA). This is enforced at the GitHub Organization level.
- Do not share accounts. Use individual GitHub accounts for all contributions.

## Dependencies

- **Evaluate before adding** any new dependency:
  - Confirm the dependency name exactly matches the intended package (typosquatting defense).
  - Verify the correct repository/owner — not a fork or lookalike.
  - Check for known vulnerabilities: `npm audit`, `pip-audit`, or the Dependabot security updates already configured for this repository.
- Prefer dependencies with a clear security disclosure process, frequent releases, and an active maintainer community.
- **Minimize dependencies.** Low-dependency counts reduce attack surface. For small utilities, consider vendoring or reimplementing rather than pulling in a large library.
- **Do not add** generated files (e.g., autoconf output, baked build artefacts) to the source tree. The build system generates these.

## Secrets and Credentials

- **Never hardcode** passwords, API keys, tokens, certificates, or private keys in source code or configuration files.
- The `secrets-scanner` hook (`.github/hooks/secrets-scanner/`) scans modified files at session end. Do not bypass it.
- Store secrets in environment variables or a secrets management service. If a secret is accidentally committed, **rotate it immediately** — git history is public.

## Code Review

- All changes must go through pull request review before merging to `develop` or `main`.
- At least one reviewer (ideally two) must approve before merge.
- Copilot code review is a supplement, not a substitute for human review.
- Reviewers must check for security issues, not just correctness.

## Automated Testing

- New code must be accompanied by tests covering both positive (expected behavior) and **negative** (invalid input, boundary conditions, error paths) scenarios.
- CI runs on every PR: Windows (MSVC x64/x86/ARM64, GCC, Clang), Linux (GCC/Clang), MUSL cross-compile (8 architectures), FreeBSD (Cirrus CI), OpenBSD/NetBSD/DragonFlyBSD/Solaris/Illumos (VMActions).
- Do not disable or skip CI jobs without documented justification.

## Static Analysis and Tooling

- **CodeQL**: Configured for C/C++ scanning. Do not suppress CodeQL findings without documented rationale.
- **Dependabot**: Configured for submodule and package-ecosystem updates. Review and merge Dependabot PRs promptly.
- **Compiler warnings as errors**: All build configurations use `-Werror` (GCC/Clang) or `/WX` (MSVC). Do not suppress warnings with `#pragma` or `-Wno-*` without discussion.
- **Sanitizers**: Use `builddir` / `build_test*` configurations for local testing with AddressSanitizer (`-fsanitize=address`) and UndefinedBehaviorSanitizer (`-fsanitize=undefined`). Sanitizer builds are a development tool, not a release artifact.
- The `tool-guardian` hook (`.github/hooks/tool-guardian/`) blocks dangerous shell patterns (e.g., `rm -rf /`, `git push --force`). Do not work around it.

## Vulnerability Reporting

`SECURITY.md` documents the project's vulnerability disclosure policy. When a security vulnerability is found:
1. **Do not open a public GitHub issue.**
2. Follow the process in `SECURITY.md` (private disclosure to maintainers).
3. Allow a reasonable remediation window before public disclosure.

## Release Integrity

- Important releases should be signed. Consider [sigstore/cosign](https://github.com/sigstore/cosign) for release artifact signing.
- The `nfpm.yaml` packaging configuration should be reviewed for each release.
- Document any breaking changes clearly in release notes.

## OpenSSF Scorecard and Best Practices Badge

- Review the [OpenSSF Scorecard](https://scorecard.dev/) results for this repository periodically. Address findings that are practical to fix.
- Consider pursuing the [OpenSSF Best Practices Badge](https://www.bestpractices.dev/). Many criteria are already met (CI, code review, security disclosure, SPDX license).

---

## Memory Safety — C-Specific Guidance

openSeaChest is written in C — a non-memory-safe-by-default language. The project uses a layered strategy consistent with the OpenSSF Memory Safety Continuum.

### Layer 1: Compiler Hardening (already active)

All hardening flags documented in `openssf-compiler-hardening.instructions.md` are enabled by default in `meson.build`. These include:
- `-D_FORTIFY_SOURCE=3` (GCC) / `-D_FORTIFY_SOURCE=2` (Clang)
- `-fstack-protector-strong`
- `-fcf-protection=full` (x86 CET)
- `-mbranch-protection=standard` (AArch64 BTI/PAC)
- `-fPIE` + `-Wl,-z,relro,-z,now`
- Full warning sets (`-Wall -Wextra -Wshadow -Wformat=2 -Wundef …`)

Do not disable these flags. If a specific flag causes a false positive, suppress it narrowly with `#pragma GCC diagnostic push/pop` or add the suppression to a specific file's `meson.build` entry, not globally.

### Layer 2: Safe Abstraction Library (opensea-common)

The `safe_*` functions in `subprojects/opensea-common/include/` provide bounds-checked, C11-Annex-K-style wrappers for all standard C memory, string, I/O, and conversion functions.

**Always use these instead of the raw C standard library equivalents:**

| Standard | Use instead |
|----------|------------|
| `malloc` / `calloc` / `realloc` / `free` | `safe_malloc` / `safe_calloc` / `safe_realloc` / `safe_free(&ptr)` |
| `strcpy` / `strcat` / `strtok` | `safe_strcpy` / `safe_strcat` / `safe_strtok` |
| `memset` / `memcpy` / `memmove` | `safe_memset` / `safe_memcpy` / `safe_memmove` |
| `strtol` / `atoi` / `atof` | `safe_strtol` / `safe_atoi` / `safe_atof` |
| `fopen` / `tmpfile` | `safe_fopen` / `safe_tmpfile` |

For zeroing sensitive data (keys, passwords), use `explicit_zeroes()` — it is not optimized away by the compiler.

### Layer 3: Compiler Annotations (code_attributes.h)

Use the macros documented in `openssf-compiler-annotations.instructions.md` to communicate buffer sizes, nullability, access modes, and taint sources to the compiler and static analyzers. These annotations amplify the effectiveness of `-fsanitize=*` and GCC `-fanalyzer`.

### Layer 4: Isolation

- Code that processes untrusted external data (device command responses, USB data, user-supplied command-line arguments) should be isolated from code that performs direct memory management.
- Validate all lengths and offsets derived from external data **before** using them as allocation sizes or array indices.
- Never use externally-supplied data directly as a `printf` format string.

### Layer 5: Sanitizers in CI / Development

- Run sanitizer builds periodically, especially after adding new code that handles device data or user input.
- AddressSanitizer catches: heap/stack/global buffer overflows, use-after-free, double-free, memory leaks.
- UndefinedBehaviorSanitizer catches: signed integer overflow, null pointer dereference, misaligned access, invalid enum values.
- LeakSanitizer is included with ASan on Linux; check for leaks in long-running test sequences.

### What Not To Do

- Do not use `VLAs` (variable-length arrays) — they are stack-allocated with no overflow protection and can silently corrupt the stack. The project's warning flags include `-Wvla` and `-Wvla-parameter`.
- Do not use `gets()` or `sprintf()` — always use the `safe_*` equivalents.
- Do not cast away `const` to modify data.
- Do not perform arithmetic on `void*` pointers — cast to `uint8_t*` first.
- Do not use `reinterpret_cast`-style tricks to bypass strict aliasing. Use `safe_memcpy` to copy between different pointer types.
