---
description: 'clang-format coding style conventions for openSeaChest — formatting rules derived from the project .clang-format file'
applyTo: '**/*.c, **/*.h, **/*.cpp, **/*.hpp, **/*.cc, **/*.cxx'
---

# clang-format Conventions — openSeaChest

## Overview

All C/C++ source in this project is formatted with **clang-format** using the `.clang-format` file at the repository root. Every subproject ships an identical copy. Run `clang-format -i <file>` or configure your editor to format on save.

Reference: [clang.llvm.org/docs/ClangFormatStyleOptions.html](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)

## Base Style

```yaml
BasedOnStyle: Microsoft
```

The project starts from the **Microsoft** style and applies targeted overrides. When in doubt about an unspecified rule, the Microsoft baseline applies.

## Brace Style — Allman

```yaml
BreakBeforeBraces: Allman
BraceWrapping:
  AfterControlStatement: true
```

Opening braces always go on their **own line** — including after `if`, `else`, `for`, `while`, `do`, `switch`, and function definitions:

```c
// ✅ CORRECT
if (condition)
{
    do_something();
}
else
{
    do_other();
}

// ❌ WRONG — K&R / opening brace on same line
if (condition) {
    do_something();
}
```

## No Short Forms

```yaml
AllowShortBlocksOnASingleLine: Never
AllowShortIfStatementsOnASingleLine: Never
```

Never collapse blocks or `if` bodies onto a single line:

```c
// ✅ CORRECT
if (err)
{
    return FAILURE;
}

// ❌ WRONG
if (err) { return FAILURE; }
if (err) return FAILURE;
```

## Pointer Alignment — Left

```yaml
PointerAlignment: Left
```

The `*` and `&` attach to the **type**, not the variable name:

```c
// ✅ CORRECT
uint8_t* buffer;
tDevice* device;
const char* str;

// ❌ WRONG
uint8_t *buffer;
uint8_t * buffer;
```

## Alignment Rules

```yaml
AlignConsecutiveAssignments: true
AlignConsecutiveDeclarations: true
AlignTrailingComments: true
AlignConsecutiveMacros: AcrossEmptyLines
AlignAfterOpenBracket: true
```

Align `=` signs in consecutive assignments, types/names in consecutive declarations, trailing `//` comments, and `#define` values — even across empty lines:

```c
// ✅ CORRECT — aligned assignments
uint32_t transferLength = 512;
bool     supportedCmd   = false;
int      retCode        = SUCCESS;

// ✅ CORRECT — aligned macros (AcrossEmptyLines)
#define MAX_RETRIES     3
#define TIMEOUT_MS      5000

#define BUFFER_SIZE     4096
```

## Parameter Formatting

```yaml
BinPackParameters: false
AllowAllParametersOfDeclarationOnNextLine: false
```

When a function signature doesn't fit on one line, each parameter goes on its **own line**, aligned under the opening parenthesis — never all dumped to the next line as a block:

```c
// ✅ CORRECT — each param on its own aligned line
int perform_operation(tDevice* device,
                      uint8_t* buffer,
                      uint32_t bufferLength,
                      bool     verify);

// ❌ WRONG — bin-packed
int perform_operation(tDevice* device, uint8_t* buffer,
                      uint32_t bufferLength, bool verify);

// ❌ WRONG — all on next line
int perform_operation(
    tDevice* device, uint8_t* buffer, uint32_t bufferLength, bool verify);
```

## Preprocessor Directive Indentation

```yaml
IndentPPDirectives: AfterHash
```

Nested preprocessor directives are indented **after** the `#`:

```c
// ✅ CORRECT
#if defined(_WIN32)
#  include <windows.h>
#  if defined(_WIN64)
#    define PLATFORM_64BIT
#  endif
#endif

// ❌ WRONG — no indent
#if defined(_WIN32)
#include <windows.h>
#endif
```

## Applying the Format

```bash
# Format a single file in-place
clang-format -i src/myfile.c

# Check without modifying (CI use)
clang-format --dry-run --Werror src/myfile.c

# Format all C/H files recursively (bash)
find . -name '*.c' -o -name '*.h' | xargs clang-format -i
```

Do NOT add or suggest clang-format disable comments (`// clang-format off`) without a compelling reason — prefer restructuring the code instead.
