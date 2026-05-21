---
description: 'Prompt for documenting a C function, macro, struct, or enum with complete Doxygen comments following the openSeaChest conventions'
mode: 'ask'
---

Document the following symbol with complete Doxygen comments following the openSeaChest conventions defined in `.github/instructions/doxygen.instructions.md`.

## Symbol to Document

```c
${selection}
```

## Documentation Requirements

Generate a complete `//!`-style Doxygen comment block placed directly above the declaration. Include:

1. **`\fn` line** — full function signature (or `\def` for macros, `\struct`/`\enum`/`\typedef` for types)
2. **`\brief`** — one sentence, no period needed, written to be useful in a summary table
3. **Extended description** — one or more paragraphs explaining purpose, behaviour, and any non-obvious details. Reference parameter names with `\a name`.
4. **`\param[in]` / `\param[out]` / `\param[in,out]`** — every parameter, with direction, type context, valid range, and what `M_NULLPTR` means (if a pointer)
5. **`\retval`** — one entry for every distinct return code the function can produce
6. **`\pre`** — any ordering or state requirements the caller must satisfy before calling (e.g., "device must be opened", "prepare command must have been issued")
7. **`\post`** — any state changes the caller must account for after a successful call
8. **`\warning`** — if the operation is destructive, irreversible, or security-sensitive
9. **`\attention`** — if there are USB bridge limitations, platform surprises, or timing constraints
10. **`\par Platform:`** — if behaviour differs by OS or transport interface
11. **`\deprecated`** — if this symbol is being replaced; include migration note with `\c replacement_name()`
12. **`\todo`** — any known gaps in the implementation
13. **`\code{.c}...\endcode`** — a realistic, self-contained usage example showing variable declaration, the call itself, error checking, and cleanup. Use `M_NULLPTR` not `NULL`, `safe_free(&ptr)` not `free(ptr)`, `eReturnValues` for return types.
14. **`\sa`** — related functions the caller should also know about

For **structs and enums**: add `//!<` trailing comments on every member on the same line.

## Style Rules

- Use `\` (backslash) for all commands — never `@`
- Blank `//!` lines between tag groups
- Use `\c symbol` for constants, enum values, type names, and macro names
- Use `\a name` when referencing a parameter name inside description text
- Use `M_NULLPTR` (not `NULL`) in all prose and examples
- Do not use `\return` — use `\retval` for each distinct return code
- If the symbol is straightforward and has no platform quirks, omit `\warning`, `\attention`, and `\par Platform:` — do not add empty or generic placeholders

## Output Format

Output **only** the completed Doxygen comment block followed by the original declaration unchanged. Do not add any surrounding markdown fences or explanatory text outside the comment.
