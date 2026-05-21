---
description: 'Rules for correctly and securely using regular expressions in C and C++ code'
applyTo: '**/*.c, **/*.h, **/*.cpp, **/*.hpp, **/*.cc, **/*.cxx'
---

# Regex Security — openSeaChest

Source: [OpenSSF — Correctly Using Regular Expressions for Secure Input Validation](https://best.openssf.org/Correctly-Using-Regular-Expressions-for-Secure-Input-Validation)

> Regular expressions are not currently a primary pattern in openSeaChest. This guidance applies when regex is introduced for device model matching, firmware version parsing, or any user-supplied pattern matching.

---

## C Regex API

C provides POSIX BRE/ERE via `<regex.h>`:

```c
regex_t  re;
int      rc;
regmatch_t m[1];

// Always compile with REG_EXTENDED for ERE (simpler, less surprising)
rc = regcomp(&re, "^[0-9]{1,4}$", REG_EXTENDED | REG_NOSUB);
if (rc != 0)
{
    // Handle compile error — do not proceed
}

rc = regexec(&re, input, 0, NULL, 0);
regfree(&re);

if (rc == 0) { /* matched */ }
else if (rc == REG_NOMATCH) { /* did not match */ }
else { /* error */ }
```

---

## Anchoring — The Most Common Error

**Always anchor patterns** that are intended to match the full string. Without anchors, the regex engine finds a match anywhere within the string.

```c
// WRONG: matches "abc" anywhere — "prefix_abc_suffix" would pass
regcomp(&re, "abc", REG_EXTENDED);

// CORRECT: matches only the exact string "abc"
regcomp(&re, "^abc$", REG_EXTENDED);
```

In POSIX regex, `$` matches the end of the string (or end of a line with `REG_NEWLINE`). Without `REG_NEWLINE`:
- `^` matches start of the entire string
- `$` matches end of the entire string

This differs from some other languages (e.g., Python's `re` module where `$` can match before a trailing newline). Be explicit.

### Alternation Grouping

`|` has very low precedence. Always group alternatives explicitly:

```c
// WRONG: matches "^foo" OR "bar$" — not what was intended
regcomp(&re, "^foo|bar$", REG_EXTENDED);

// CORRECT: matches "foo" or "bar" as the full string
regcomp(&re, "^(foo|bar)$", REG_EXTENDED);
```

---

## ReDoS (Regular Expression Denial of Service)

Catastrophic backtracking can cause near-infinite execution time on crafted inputs. In a storage tool context this matters when regex patterns are applied to externally-supplied strings (device model numbers, firmware versions, command-line arguments).

### Patterns to avoid

| Pattern | Problem |
|---------|---------|
| `(a+)+` | Nested quantifiers — exponential backtracking |
| `(a*)*` | Same issue |
| `(a+b?)+` | Polynomial backtracking on non-matching input |
| `(a|aa)+` | Overlapping alternatives with quantifiers |

### Mitigations

1. **Bound repetition**: use `{0,64}` instead of `*` or `+` when the input length is known to be bounded.
2. **Limit input length** before calling `regexec()`. Reject strings longer than the maximum valid value before applying any pattern.
3. **Prefer literal matching** for exact strings. Use `strcmp`/`strncmp`/`safe_strcpy` comparisons instead of regex when the set of valid values is fixed and small.
4. **Avoid nested quantifiers**. Write `a{1,10}b{1,10}` rather than `(a+b)+`.
5. **Test with long adversarial inputs** when adding new patterns.

```c
// Limit input before regex
if (safe_strnlen(input, MAX_INPUT_LEN + 1) > MAX_INPUT_LEN)
{
    return FAILURE; // Reject before even compiling the regex
}
```

---

## Validation vs. Extraction

Use regex for **input validation** (does this string conform to the expected format?), not as the primary extraction mechanism for safety-critical fields. When extraction is needed, use the POSIX capture group (`regmatch_t`) carefully:

```c
regmatch_t matches[2]; // [0] = whole match, [1] = first capture group
rc = regexec(&re, input, 2, matches, 0);
if (rc == 0 && matches[1].rm_so != -1)
{
    size_t len = (size_t)(matches[1].rm_eo - matches[1].rm_so);
    // len is safe — rm_so and rm_eo are within the input string
    safe_strncpy(dest, destsz, input + matches[1].rm_so, len);
}
```

Always validate that `rm_so != -1` (group did not participate in the match) and that derived lengths are within the expected bounds before using them.

---

## Character Classes

- `[0-9]` is portable. `\d` is **not** in POSIX BRE/ERE — use `[[:digit:]]` for locale-aware digit matching or `[0-9]` for ASCII digits only.
- `[a-zA-Z]` is portable for ASCII. `[[:alpha:]]` is locale-aware.
- Prefer explicit ASCII ranges for parsing device data (model strings, firmware version tokens) to avoid locale-dependent behavior.

---

## PCRE2 (if added as a dependency)

If complex patterns are required and PCRE2 is added:
- Enable `PCRE2_ANCHORED` or use explicit `\A` / `\z` anchors — `^` and `$` are not anchors in multiline mode.
- Set a match limit (`pcre2_match_data_create_from_pattern`) to bound backtracking.
- Prefer `pcre2_match` over `pcre2_dfa_match` for most use cases (DFA does not support captures but avoids backtracking).
- Compile patterns once and reuse (`pcre2_compile` is expensive; call it at startup, not per-input).

---

## Summary Checklist

When adding a regex to this codebase:

- [ ] Pattern is anchored with `^` and `$` (or `\A` / `\z` in PCRE2)
- [ ] Alternation groups are parenthesized: `^(foo|bar)$`
- [ ] No nested quantifiers: `(a+)+`, `(a*)*`, `(a+b?)+`
- [ ] Input length is validated before `regexec()` / `pcre2_match()`
- [ ] `REG_EXTENDED` flag is set (for POSIX ERE)
- [ ] `regfree()` is called for every successful `regcomp()`
- [ ] Capture group indices are bounds-checked before use
- [ ] Consider replacing with `strcmp`/`strncmp` if the valid set is small and fixed
