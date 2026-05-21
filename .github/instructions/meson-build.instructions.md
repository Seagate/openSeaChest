---
description: 'Meson build system conventions for openSeaChest — subprojects, compiler detection, warning flags, options, and cross-compilation'
applyTo: '**/meson.build, **/meson_options.txt, **/meson.format'
---

# Meson Build System — openSeaChest Conventions

## Build System
This project uses **[Meson](https://mesonbuild.com/)** (minimum version `>=0.56.0`) as its primary build system. Do NOT suggest CMake, autotools, or vcpkg. All dependency management goes through Meson subprojects or system `dependency()` lookups.

Reference: [mesonbuild.com/Reference-manual.html](https://mesonbuild.com/Reference-manual.html) — use Context7 or fetch this URL directly when you need authoritative API details.

## Project Structure

```
meson.build              ← top-level project file
meson_options.txt        ← user-visible build options (get_option())
subprojects/
    opensea-common/      ← subproject, dep: opensea_common_dep
    opensea-transport/   ← subproject, dep: opensea_transport_dep
    opensea-operations/  ← subproject, dep: opensea_operations_dep
    opensea-jsonformat/  ← subproject (optional, json_outputformat option)
    json-c/              ← subproject (optional, used by jsonformat)
    wingetopt/           ← subproject (getopt_long for all platforms)
```

## Subprojects

Always use `subproject()` + `get_variable()` pattern:
```meson
opensea_common = subproject('opensea-common', default_options: 'default_library=static')
opensea_common_dep = opensea_common.get_variable('opensea_common_dep')
```

Never use `dependency()` for the opensea-* libraries — they are not system packages.

For system libraries with a subproject fallback (e.g., `json-c`), use the built-in `fallback` parameter on `dependency()` — cleaner than a manual `if not found()` block:
```meson
jsonc_dep = dependency(
    'json-c',
    required: true,
    fallback: ['json-c', 'jsonc_dep'],
    default_options: [
        'default_library=static',
        'build_apps=false',
    ],
)
```

Useful `dependency()` kwargs:
- `fallback: ['subproj', 'var']` — automatic wrap fallback (since 0.55.0; auto-detected since 0.56.0 if wrap provides it).
- `static: true` — also sets `default_library=static` on the fallback subproject (since 0.60.0), so `default_options` is usually redundant when using this.
- Multiple names: `dependency('png', 'libpng')` tries names in order (since 0.60.0).
- `not_found_message: 'Install json-c'` — prints a helpful hint when the dep is not found (since 0.50.0).
- `disabler: true` — returns a disabler object instead of a not-found dep, propagating "disabled" state to any target that uses it (since 0.49.0).
- `allow_fallback: false` — equivalent to `fallback: []`; always prefer system over subproject.

Only use manual `subproject()` + `if` control when you need platform-specific fallback logic. For example, on Solaris the system json-c is absent on non-illumos kernels and must be forced to the wrap:
```meson
force_wrap = false
if host_machine.system() == 'sunos'
    # meson >= 1.2.0 exposes target_machine.kernel() for illumos detection
    if meson.version().version_compare('>=1.2.0')
        if target_machine.kernel() != 'illumos'
            force_wrap = true
        endif
    else
        force_wrap = true  # Can't detect illumos on older Meson; default to wrap
    endif
endif

if force_wrap
    jsonc_proj = subproject(
        'json-c',
        default_options: ['default_library=static', 'build_apps=false'],
    )
    jsonc_dep = jsonc_proj.get_variable('jsonc_dep')
else
    jsonc_dep = dependency(
        'json-c',
        required: true,
        fallback: ['json-c', 'jsonc_dep'],
        default_options: ['default_library=static', 'build_apps=false'],
    )
endif
```

## Compiler Detection

Always use `c.get_id()` and `c.version()` — never hardcode compiler names in preprocessor form from the build file side:
```meson
c = meson.get_compiler('c')

if c.get_id().contains('gcc') or c.get_id().contains('clang')
    # GCC or Clang (includes clang-cl, clang on Windows)
elif c.get_id().contains('msvc')
    # MSVC (cl.exe)
elif c.get_id().contains('xlc')
    # IBM XLC
endif
```

## Adding Warning Flags

ALWAYS use `c.get_supported_arguments()` — never add flags unconditionally. Flags that are unsupported on a compiler are silently dropped:
```meson
warning_flags = ['-Wshadow=compatible-local', '-Wvla', ...]
add_project_arguments(c.get_supported_arguments(warning_flags), language: 'c')
```

For linker flags:
```meson
linker_flags = ['-Wl,-z,relro', '-Wl,-z,now', ...]
add_project_link_arguments(c.get_supported_link_arguments(linker_flags), language: 'c')
```

## Compiler-Specific Flag Reference

### GCC / Clang Warning Flags (active in this project)
Security hardening (from OpenSSF Compiler Hardening Guide):
- `-fstack-protector-strong` (not on Solaris)
- `-fstack-clash-protection` (not on Windows GCC)
- `-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3`
- `-fno-delete-null-pointer-checks`, `-fno-strict-overflow`, `-fno-strict-aliasing`
- `-ftrivial-auto-var-init=zero`, `-fzero-init-padding-bits=all`
- `-fvisibility=hidden`

Architecture-specific:
- x86_64: `-fcf-protection=full`
- aarch64: `-mbranch-protection=standard`
- ppc64: `-Wno-psabi`

Linker hardening:
- `-Wl,-z,nodlopen`, `-Wl,-z,noexecstack`, `-Wl,-z,relro`, `-Wl,-z,now`

C standard: GCC <5 needs explicit `-std=gnu11` or `-std=gnu99`.

### MSVC Warning Flags (active in this project)
Security: `/GS`, `/sdl`, `/Qspectre`, `/guard:cf`, `/d2guard4`
Linker: `/guard:cf`, `/SafeSEH`, `/NXCOMPAT`, `/dynamicbase`
Warnings off: `/wd4214`, `/wd4201`, `/wd4668`, `/wd4820`, `/wd4710`, `/wd5045`
Warnings promoted: `/w14062`, `/w14101`, `/w14189`, `/w15262` (implicit fallthrough)
Errors: `/we4431`, `/we4905`, `/we4906`, `/we4837`, `/we4464`
x86/x64 only: `/QIntel-jcc-erratum`
Always add: `/std:c17` if `c.has_argument('/std:c17')` is true.

## Build Options

The options file is `meson.options` (since Meson 1.1.0) or `meson_options.txt` on older versions — both names are accepted. Access values with `get_option('name')`. Common options in this project:
- `debug` → adds `-D_DEBUG`
- `libc_musl` → adds `-DUSING_MUSL_LIBC=1`
- `json_outputformat` → enables JSON output, pulls in json-c and opensea-jsonformat
- `use_system_json` → prefer system json-c over subproject
- `tcg` → TCG/Opal security support (`feature` type, use `.enabled()`)
- `atasecsetpass` → ATA Security Set Password support
- `cc-suggest-attribute` → enables `-Wsuggest-attribute=*` flags

Feature options use `.enabled()` / `.disabled()` / `.auto()`:
```meson
if not get_option('tcg').enabled()
    add_project_arguments('-DDISABLE_TCG_SUPPORT', language: 'c')
endif
```

Boolean options use direct truthy check:
```meson
if get_option('debug')
    add_project_arguments('-D_DEBUG', language: 'c')
endif
```

Other option features to know:
- **Built-in options**: `get_option('prefix')`, `get_option('bindir')`, etc. give install paths.
- **`yield: true`** in an option definition lets a subproject inherit the superproject's value rather than using its own default — useful for `debug`, `default_library`, etc.
- **Deprecating options**: Set `deprecated: true` (hide from `meson configure`), `deprecated: 'new_option_name'` (rename redirect), or `deprecated: {'old_val': 'new_val'}` (value mapping).
- **Per-subproject overrides**: `meson configure -Dsubproject:option=val` sets an option only for that subproject.
- **Changing options after setup**: `meson configure builddir -Doption=newval` — no need to wipe the build directory.

## Platform Detection

Use `target_machine.system()` for OS, `target_machine.cpu_family()` for arch:
```meson
if target_machine.system() == 'windows'   # Windows
if target_machine.system() == 'linux'     # Linux
if target_machine.system() == 'sunos'     # Solaris/Illumos
if target_machine.system() == 'freebsd'   # FreeBSD

if target_machine.cpu_family() == 'x86_64'
if target_machine.cpu_family() == 'aarch64'
if target_machine.cpu_family() == 'ppc64'
```

Windows resources (RC files):
```meson
windows = import('windows')
resources = windows.compile_resources('openSeaChest.rc')
```

## Cross-Compilation

Cross-compile files live in `meson_crosscompile/`. Use them via:
```
meson setup builddir --cross-file meson_crosscompile/linux-musl-aarch64.ini
```

When writing build logic, prefer `target_machine.*` over `host_machine.*` for portability with cross-compile setups.

Key cross-compilation APIs:
```meson
# Detect whether a cross build is active
if meson.is_cross_build()
    # We are cross-compiling
endif

# Can we execute host binaries? (e.g. true on macOS→Linux with Rosetta-style binfmt)
if meson.can_run_host_binaries()
    # Can run target programs during build
endif

# Get the BUILD-machine compiler (for code-gen tools that must run on the build host)
build_c = meson.get_compiler('c', native: true)

# Build-machine executable (tool run during build, not installed)
gen_tool = executable('gen_tool', 'gen_tool.c',
    native: true,   # Built for the build machine, not the host machine
    dependencies: [],
)
```

**Machine objects** (`build_machine`, `host_machine`, `target_machine`):
- `.system()` — OS string: `'windows'`, `'linux'`, `'sunos'`, `'freebsd'`, etc.
- `.cpu_family()` — `'x86_64'`, `'aarch64'`, `'ppc64'`, etc.
- `.cpu()` — specific CPU (less portable, prefer `cpu_family()`).
- `.endian()` — `'little'` or `'big'`.
- `.subsystem()` — subsystem string, e.g. `'efi'` (since Meson 1.2.0).
- `.kernel()` — kernel string, e.g. `'linux'`, `'nt'`, `'illumos'`, `'xnu'` (since Meson 1.2.0).

Cross-file `[host_machine]` section can specify `kernel = 'nt'` (Windows NT kernel) or `kernel = 'illumos'` for Illumos, which is how `target_machine.kernel() != 'illumos'` detection works (see json-c Solaris workaround above).

For build-only programs, always set `native: true` — otherwise Meson may try to cross-compile them for the host, which breaks when they need to run during the build.

## Reproducible Builds

This project follows the [Reproducible Builds](https://reproducible-builds.org/) specification. `SOURCE_DATE_EPOCH` is the canonical timestamp variable defined by the spec — Meson supports it natively. Detection order in the build:
1. `SOURCE_DATE_EPOCH` env var (via `sh`, `bash`, `pwsh`, or `cmd.exe`)
2. `git log -1 --pretty=%ct`
3. Falls back to compiler's `__DATE__`

Do not use `__DATE__`/`__TIME__` directly in source — use the `BUILD_TIMESTAMP` macro instead, which is set from `SOURCE_DATE_EPOCH`.

## Small-Code Optimization Flags

On GCC/Clang (non-Windows):
```meson
small_code_cc_flags = ['-ffunction-sections', '-fdata-sections']
small_code_link_flags = ['-Wl,--gc-sections']
```
These are added globally, not per-target.

## Common Patterns

### Adding a new executable target
```meson
executable('openSeaChest_MyTool',
    sources: common_sources + ['utils/C/openSeaChest/openSeaChest_MyTool.c'],
    dependencies: [opensea_common_dep, opensea_transport_dep, opensea_operations_dep] + os_deps,
    install: true
)
```

### Checking compiler feature support before using it
```meson
if c.has_argument('-Wsome-flag')
    add_project_arguments('-Wsome-flag', language: 'c')
endif

if c.has_header('some_header.h')
    add_project_arguments('-DHAVE_SOME_HEADER', language: 'c')
endif
```

### Running a helper program at build time
```meson
python = find_program('python3', 'python', required: false)
if python.found()
    result = run_command(python, '-c', 'print("hello")', check: false)
endif
```
