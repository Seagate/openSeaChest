---
description: 'Generate a complete Doxygen comment block for a C function, struct, or enum'
agent: 'ask'
tools: ['search/codebase', 'edit/editFiles', 'read/readFile']
---

# Add Doxygen Documentation to a C Symbol

## Mission

Generate a complete, correctly formatted Doxygen comment block for a C function,
struct, or enum, following the project's Qt-style `//!` conventions. Insert the
block immediately above the declaration in the header file (or above the definition
if the symbol is static). Follow all conventions in
`.github/instructions/doxygen.instructions.md`.

## Scope & Preconditions

- Read the Doxygen instruction file before generating any output:
  `.github/instructions/doxygen.instructions.md`
- Target: the function, struct, or enum the user has selected or named.
- Prefer the header file declaration as the documentation site for public symbols.
  Static symbols are documented at their definition in the `.c` file.
- Do not add comments that merely restate the code — every sentence must add
  information not obvious from the signature alone.

## Inputs

| Input | How to obtain |
|-------|---------------|
| Symbol to document | `${selection}` (selected code) or `${input:symbol:function/struct/enum name}` |
| File containing the symbol | Current file, or `${input:filePath:relative path}` |

If neither `${selection}` nor an explicit name is provided, ask the user which
symbol to document.

## Comment Style Rules (summary — read the full instruction file)

- Use `//!` for all comment lines (Qt style).
- Use `//!<` for trailing member documentation (struct/enum fields only).
- Use backslash (`\`) for all Doxygen commands — never `@`.
- Every public function **must** have: `\brief`, `\param[in/out]`, `\retval`, `\code{.c}`.
- Annotations: `\pre` / `\post` for ordering constraints, `\warning` for destructive
  ops, `\attention` for platform/USB quirks, `\par Platform:` for OS-specific
  behaviour, `\deprecated` with a migration note, `\todo` for known gaps.
- Use `\details` to introduce extended descriptions — always explicitly separate from `\brief`.
- Use `\important` for notable highlights that are not warnings or caveats.
- Use Markdown pipe tables inside `\details` for register/byte-field maps.
- Use `\mermaid` inside `\details` for flowcharts and command sequence diagrams.

## Workflow

### 1. Analyse the symbol

Examine the signature or declaration:
- **Function**: identify return type, all parameters (name, type, direction), and
  what the function does based on its name, body, and any existing comments.
- **Struct**: identify each field's type, purpose, and whether it can be null.
- **Enum**: identify what the enum represents and the meaning of each enumerator.

### 2. Determine parameter directions
For each function parameter:
- `\param[in]` — read-only input; the function does not write through this pointer.
- `\param[out]` — the function writes the result here; the initial value is irrelevant.
- `\param[in,out]` — the function reads and modifies the value.
- Non-pointer parameters are always `\param[in]`.

### 3. Enumerate return values
List every distinct value the function can return:
```
\retval SUCCESS          Operation completed successfully.
\retval MEMORY_FAILURE   Failed to allocate an intermediate buffer.
\retval NOT_SUPPORTED    Device or interface does not support this command.
\retval BAD_PARAMETER    A required pointer argument was M_NULLPTR.
```
Use the project's `eReturnValues` constant names, not raw integers.

### 4. Write the \brief
One sentence, imperative mood, no period at the end if it fits on one line:
```
\brief Read a single log page from the device using the ATA READ LOG EXT command
```

### 5. Generate the full block

#### Function example
```c
//! \brief Brief one-line description of what the function does.
//!
//! Extended description (optional). Explain WHY or any non-obvious behaviour.
//! Omit if the brief is already self-explanatory.
//!
//! \param[in]  device      Handle to the open device.
//! \param[in]  logAddress  Log address to read (ATA Log Address byte).
//! \param[in]  pageNumber  Page number within the log (0-based).
//! \param[out] buffer      Caller-allocated buffer to receive the log data.
//! \param[in]  bufferSize  Size of \p buffer in bytes; must be a multiple of 512.
//!
//! \retval SUCCESS        Log page read successfully into \p buffer.
//! \retval BAD_PARAMETER  \p device or \p buffer is M_NULLPTR, or \p bufferSize is 0.
//! \retval NOT_SUPPORTED  Device does not support READ LOG EXT.
//! \retval FAILURE        Command was issued but the device returned an error.
//!
//! \code{.c}
//! uint8_t logData[512];
//! eReturnValues ret = ata_Read_Log_Ext(device, ATA_LOG_DIRECTORY, 0,
//!                                      logData, sizeof(logData));
//! if (ret == SUCCESS)
//! {
//!     // process logData
//! }
//! \endcode
```

#### Struct example
```c
//! \brief Brief description of the struct's purpose.
typedef struct s_myStruct
{
    uint32_t count;       //!< Number of valid entries in \p entries.
    bool     valid;       //!< Set to \c true when \p count has been populated.
    uint8_t* entries;     //!< M_NULLABLE. Pointer to the entry array; M_NULLPTR if none.
} myStruct;
```

#### Enum example
```c
//! \brief Describes the operating state of the device.
typedef enum e_deviceState
{
    DEVICE_STATE_UNKNOWN = 0, //!< State has not been determined yet.
    DEVICE_STATE_ACTIVE,      //!< Device is fully powered and ready.
    DEVICE_STATE_STANDBY,     //!< Device is in standby; spin-up required before I/O.
    DEVICE_STATE_SLEEP,       //!< Device is in sleep; reset required to wake.
} eDeviceState;
```

### 6. Add conditional tags where applicable
- Add `\pre` if the function requires locks held, a prior call, or an open device handle.
- Add `\post` if the function changes device state in a way callers must know.
- Add `\warning` if the operation is destructive (e.g., erase, format, secure overwrite).
- Add `\attention` for USB bridge or platform-specific behaviour.
- Add `\par Platform:` block for OS-specific restrictions (e.g., "28-bit only on NetBSD/OpenBSD").
- Add `\deprecated` with a `\sa` referencing the replacement if the symbol is deprecated.
- **For ATA commands in a specific feature set**, add a `\note` naming the feature set and
  the spec version that introduced it. This allows Doxygen to group commands by feature.
  Common ATA feature sets: NCQ (ATA8-ACS), SMART (ATA-3), DCO (ATA/ATAPI-6),
  HPA (ATA/ATAPI-4, obsoleted by AMAC), AMAC (ACS-3), TCG/ATA Security, DSM/TRIM (ACS-2),
  Sanitize (ACS-2), Streaming (ATA/ATAPI-7), Hybrid/AUX (ACS-2).
- Add `\details` before any extended description to clearly separate it from `\brief`.
- Add `\important` for notable information that is not a warning or caveat.
- **For register or byte-field maps** (ATA/SCSI/NVMe commands): add a Markdown pipe table
  inside `\details` with columns for bit/offset, field name, and description.
- **For complex command flows or state machines**: add a `\mermaid` ... `\endhermaid` block
  inside `\details` with a flowchart or sequence diagram.
- **For mathematical relationships** (e.g., sector count, transfer rate, error thresholds):
  use `\f$ ... \f$` for inline or `\f[ ... \f]` for block formulas (requires `USE_MATHJAX = YES`).
- **For standard or specification references**: use `\cite key` with the BibTeX key for the
  relevant document (e.g., C11 Annex K, CERT C, ACS-4, SPC-5).
- **For diagrams or screenshots**: use `\image html img.png "Caption"` with the image stored
  under `docs/images/`.

### 7. Insert the block
Place the complete block immediately above the declaration in the header file.
For static functions, place it above the definition in the `.c` file.
Do not duplicate the comment in both locations.

### 8. Verify the \file block exists
If the target file lacks a `\file` block at the top, add one:
```c
//! \file filename.h
//! \brief One-sentence description of the file's purpose.
```

## Output Expectations

- A complete `//!`-style Doxygen block above the target symbol.
- All required tags present: `\brief`, `\param[in/out]` for every parameter,
  `\retval` for every distinct return code, `\code{.c}` example.
- Conditional tags (`\warning`, `\pre`, etc.) added only where genuinely applicable.
- No comments that merely restate the signature.
- `\file` block added to the file if missing.

## Quality Assurance

- [ ] All comment lines use `//!` (not `/**` or `//`)
- [ ] All commands use backslash (`\brief`, not `@brief`)
- [ ] `\brief` is one sentence, imperative, meaningful
- [ ] Every parameter has `\param[in]`, `\param[out]`, or `\param[in,out]`
- [ ] Every distinct return code has its own `\retval` line
- [ ] `\code{.c}` / `\endcode` example compiles if extracted
- [ ] Struct/enum members use `//!<` trailing comment style
- [ ] Destructive operations have `\warning`
- [ ] Platform restrictions documented with `\par Platform:` or `\attention`
- [ ] `\file` block present in the target file
- [ ] `\details` used when description extends meaningfully beyond `\brief`
- [ ] Markdown pipe table added inside `\details` for any register or byte-field map
- [ ] `\mermaid` diagram added inside `\details` where a flowchart clarifies behaviour
- [ ] `\important` used for notable non-warning highlights where appropriate
- [ ] `\cite` used when referencing standards, coding rules, or specifications
- [ ] `\image` used for raster diagrams, with file stored under `docs/images/` and a caption
