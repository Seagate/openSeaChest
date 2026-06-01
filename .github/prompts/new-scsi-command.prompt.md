---
description: 'Add a new SCSI command to opensea-transport following project conventions'
agent: 'agent'
tools: ['search/codebase', 'edit/editFiles', 'execute/runInTerminal', 'read/readFile']
---

# Add a New SCSI Command to opensea-transport

## Mission

Implement a new SCSI command in the opensea-transport layer, correctly building
the CDB, filling `ScsiIoCtx`, attaching sense and data buffers, and declaring
the function in the header with a complete Doxygen block. Follow all conventions
in `.github/instructions/opensea-transport-scsi.instructions.md`.

## Scope & Preconditions

- Target files: `subprojects/opensea-transport/src/scsi_cmds.c` (implementation)
  and `subprojects/opensea-transport/include/scsi_helper_func.h` (declaration).
- If the command is a cross-protocol abstraction shared with ATA or NVMe, use
  `cmds.c` / `cmds.h` instead and confirm with the user.
- Read the SCSI instruction file before proceeding:
  `.github/instructions/opensea-transport-scsi.instructions.md`
- All SCSI commands must be issued through `scsi_Send_Cdb()`. Do not call OS
  passthrough helpers directly.

## Inputs

| Input | How to obtain |
|-------|---------------|
| Command name | `${input:commandName:e.g. scsi_Read_Capacity_16}` |
| SPC/SBC/etc. spec reference | `${input:specReference:e.g. SBC-4 section 5.16}` |
| CDB size (bytes) | `${input:cdbSize:6 / 10 / 12 / 16 / 32}` |
| Data direction | `${input:dataDirection:XFER_NO_DATA / XFER_DATA_IN / XFER_DATA_OUT / XFER_DATA_IN_OUT / XFER_DATA_OUT_IN}` |
| Expected data length | `${input:dataLength:e.g. 32 bytes fixed / variable}` |

If any input is missing, ask the user before proceeding.

## Workflow

### 1. Validate inputs
- Confirm the command does not already exist in `scsi_cmds.c` or `cmds.c`.
- Confirm the correct service action value if the opcode shares a family
  (e.g., `0x9E` SERVICE ACTION IN(16) covers READ CAPACITY 16 and others).
- SCSI supports bidirectional transfers for some commands. `XFER_DATA_IN_OUT`
  and `XFER_DATA_OUT_IN` are valid, but uncommon and may depend on transport/OS
  passthrough support. Use exactly what the spec requires.

### 2. Build the CDB
Declare a zero-initialised CDB array of the correct length:
```c
DECLARE_ZERO_INIT_ARRAY(uint8_t, cdb, CDB_LEN_16); /* use the appropriate CDB_LEN_* constant */
cdb[CDB_OPERATION_CODE] = SCSI_OPCODE; /* named constant from scsi_helper.h */
cdb[CDB_1]              = serviceAction & UINT8_C(0x1F);
cdb[CDB_10]             = M_Byte3(allocationLength);
cdb[CDB_11]             = M_Byte2(allocationLength);
cdb[CDB_12]             = M_Byte1(allocationLength);
cdb[CDB_13]             = M_Byte0(allocationLength);
cdb[CDB16_CONTROL]      = set_Control_Field(false, false, false);
```
Use the common `eCDBOffsets` values from `scsi_helper.h` for all CDB byte access
instead of creating per-command local offset enums.
Whenever a command uses a Control byte, set/read it through the correct named
control offset for that CDB size (`CDB6_CONTROL`, `CDB10_CONTROL`,
`CDB12_CONTROL`, `CDB16_CONTROL`, or `CDB32_CONTROL`) rather than a raw index.
Use `set_Control_Field()` to compose the Control byte even when all inputs are
currently false. This keeps code paths ready for future NACA/FLAG/LINK support.
This avoids clang-tidy magic-number warnings and keeps byte mappings consistent.
Use `M_ByteN()` macros for multi-byte fields. Never hand-calculate byte shifts.
Use named constants (`SCSI_READ_CAPACITY_16_CMD`, `SCSI_SERVICE_ACTION_IN_16`, etc.)
from `scsi_helper.h` rather than raw opcodes.

### 3. Allocate and zero the data buffer (if applicable)
```c
uint8_t* data = C_CAST(uint8_t*, safe_calloc(dataLength, sizeof(uint8_t)));
if (!data)
{
    return MEMORY_FAILURE;
}
```
For fixed-length responses, prefer stack allocation with `DECLARE_ZERO_INIT_ARRAY`.

### 4. Fill `ScsiIoCtx` and issue the command
```c
eReturnValues ret = scsi_Send_Cdb(device, cdb, sizeof(cdb),
                                  data, dataLength,
                                  XFER_DATA_IN,        /* match dataDirection; IN/OUT or bidirectional as required */
                                  device->drive_info.lastCommandSenseData,
                                  SPC3_SENSE_LEN,
                                  DEFAULT_COMMAND_TIMEOUT);
```

### 5. Check return value and sense data
```c
if (ret != SUCCESS)
{
    safe_free(&data);
    return ret;
}
/* Parse sense data only when the spec requires distinguishing error types */
```

### 6. Parse the response
Use `M_BytesTo*Value()` macros to reassemble multi-byte fields:
```c
uint64_t returnedLBA = M_BytesTo8ByteValue(data[0], data[1], data[2], data[3],
                                           data[4], data[5], data[6], data[7]);
```
Do not cast a `uint8_t*` buffer pointer directly to a struct — always extract
field-by-field to avoid alignment and endianness issues.

### 7. Free dynamic allocations
```c
safe_free(&data);
```
`safe_free` takes a pointer-to-pointer and NULLs it automatically.

### 8. Declare in header
Add to the appropriate group in `scsi_helper_func.h`:
```c
OPENSEA_TRANSPORT_API eReturnValues scsi_Command_Name(tDevice* device, /* params */);
```

### 9. Add Doxygen comment
Use the `add-doxygen` prompt or follow `.github/instructions/doxygen.instructions.md`.
Required tags: `\brief`, `\param[in/out]` for every parameter, `\retval` for every
distinct return code, `\code{.c}` usage example, `\note` with the spec reference.

## Output Expectations

- One new function in `scsi_cmds.c` that compiles cleanly.
- One matching declaration in `scsi_helper_func.h`.
- A complete Doxygen block above the declaration.
- No raw byte casts from buffer to struct.
- No direct calls to OS passthrough helpers.
- All multi-byte field assembly uses `M_BytesTo*Value()` / `M_ByteN()` macros.

## Quality Assurance

- [ ] CDB array zero-initialised before any field is set
- [ ] Named opcode constants used (not raw hex literals in CDB[0])
- [ ] CDB field offsets use shared `eCDBOffsets` names (no raw magic indexes)
- [ ] Control byte uses the correct named control offset (`CDB6_CONTROL`/`CDB10_CONTROL`/`CDB12_CONTROL`/`CDB16_CONTROL`/`CDB32_CONTROL`)
- [ ] Control byte value is built with `set_Control_Field(...)` (use all-false input when no flags are needed)
- [ ] `M_ByteN()` used for field placement; `M_BytesTo*Value()` for field extraction
- [ ] `scsi_Send_Cdb()` used, not OS helpers directly
- [ ] Return value checked and propagated
- [ ] Dynamic buffer freed with `safe_free(&ptr)` on all exit paths
- [ ] No struct-pointer cast from raw `uint8_t*` buffer
- [ ] Sense buffer passed correctly (use `device->drive_info.lastCommandSenseData`)
- [ ] Doxygen block complete with spec reference
- [ ] Build passes (`meson compile -C builddir` or equivalent)
