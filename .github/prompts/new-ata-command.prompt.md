---
description: 'Add a new ATA command to opensea-transport following project conventions'
agent: 'agent'
tools: ['search/codebase', 'edit/editFiles', 'execute/runInTerminal', 'read/readFile']
---

# Add a New ATA Command to opensea-transport

## Mission

Implement a new ATA command in the opensea-transport layer, correctly filling
`ataPassthroughCommand`, setting all task file registers, selecting the right
protocol, declaring the function in the header, and adding a complete Doxygen
comment block. Follow all conventions in
`.github/instructions/opensea-transport-ata.instructions.md`.

## Scope & Preconditions

- Target files: `subprojects/opensea-transport/src/ata_cmds.c` (implementation)
  and `subprojects/opensea-transport/include/ata_helper_func.h` (declaration).
- If the command belongs to a cross-protocol abstraction shared with SCSI or NVMe,
  use `cmds.c` / `cmds.h` instead and ask the user to confirm.
- Read the ATA instruction file before proceeding:
  `.github/instructions/opensea-transport-ata.instructions.md`
- Do **not** call OS passthrough helpers (`sg_helper`, `win_helper`, etc.) directly.
  All ATA commands must go through `ata_Passthrough_Command()`.

## Inputs

| Input | How to obtain |
|-------|---------------|
| Command name | `${input:commandName:e.g. ata_Read_Log_Ext}` |
| ACS/ATA spec reference | `${input:specReference:e.g. ACS-4 section 7.43}` |
| Data direction | `${input:dataDirection:XFER_NO_DATA / XFER_DATA_IN / XFER_DATA_OUT}` |
| LBA mode | `${input:lbaMode:28-bit / 48-bit}` |
| Transfer protocol | `${input:protocol:NON_DATA / PIO_DATA_IN / PIO_DATA_OUT / DMA / DMA_QUEUED / PACKET / DMA_FPDMA_QUEUED}` |

If any input is missing, ask the user before proceeding.

## Workflow

### 1. Validate inputs
- Confirm the command does not already exist in `ata_cmds.c` or `cmds.c`.
- Confirm the spec reference is accurate (ask the user if unsure).
- **Reject bidirectional direction requests immediately.** If the user specifies
  `XFER_DATA_IN_OUT` or `XFER_DATA_OUT_IN`, stop and explain that ATA/SATA does not
  support bidirectional data transfer in a single command — this is a fundamental protocol
  constraint, not a feature gap. Do not attempt to implement it. Return `BAD_PARAMETER`.

### 2. Initialise the struct using factory functions (preferred)

Use the `create_ata_*_cmd()` inline helpers from `ata_helper.h` as the first choice.
They fully initialise the struct, set common defaults, and call `set_ata_pt_device_bits()`
automatically:

```c
/* PIO data-in (most common — READ LOG EXT, IDENTIFY, etc.): */
ataPassthroughCommand cmd = create_ata_pio_in_cmd(device, ATA_READ_LOG_EXT,
                                                   ATA_CMD_TYPE_EXTENDED_TASKFILE,
                                                   sectorCount, buffer, dataSize);
/* PIO data-out: */
ataPassthroughCommand cmd = create_ata_pio_out_cmd(device, opcode, ext,
                                                    sectorCount, buffer, dataSize);
/* PIO with LBA set in the same call: */
ataPassthroughCommand cmd = create_ata_pio_lba_cmd(device, opcode, ext, XFER_DATA_IN,
                                                    sectorCount, lba, buffer, dataSize);
/* DMA variants: create_ata_dma_in_cmd / create_ata_dma_out_cmd / create_ata_dma_lba_cmd  */
/* Non-data:     create_ata_nodata_cmd / create_ata_nodata_lba_cmd                         */
```

After calling a factory function, set any additional TFR fields directly (e.g.,
`cmd.tfr.ErrorFeature`). If no factory function fits the command's protocol or addressing
mode, fall back to zero-initialisation followed by the setter helpers:

```c
ataPassthroughCommand cmd;
M_INITIALIZE_STRUCTURE(&cmd, sizeof(cmd));
/* then use set_ata_pt_* helpers and set remaining fields manually */
```

### 3. Set remaining TFRs using helper functions

Use the `set_ata_pt_*()` helpers from `ata_helper.h`. Direct `cmd.tfr.*` field access is
the fallback only for fields that have no helper (`ErrorFeature`, `SectorCount`,
`CommandStatus` when not set by a factory):

```c
/* LBA addressing: */
set_ata_pt_LBA_28(&cmd, lba);          /* 28-bit; sets LBA mode bit automatically     */
set_ata_pt_LBA_48(&cmd, lba);          /* 48-bit; all 6 LBA bytes + LBA mode bit      */
set_ata_pt_LBA_28_sig(&cmd, sig);      /* 28-bit signature regs, no LBA mode bit      */
set_ata_pt_LBA_48_sig(&cmd, sig);      /* 48-bit signature regs, no LBA mode bit      */

/* Device head legacy bits + device-select from tDevice:                                 */
set_ata_pt_device_bits(&cmd, device);  /* called automatically by create_* functions  */

/* NCQ SEND/RECEIVE FPDMA priority and sub-command byte:                                 */
set_ata_pt_prio_subcmd(&cmd, prio, subcommand);

/* AUX and ICC — see note below:                                                         */
set_ata_pt_aux_icc(&cmd, aux, icc);   /* promotes to COMPLETE_TASKFILE automatically  */

/* PIO MULTIPLE block count:                                                             */
set_ata_pt_multipleCount(&cmd, device);
```

When manual field assignment is required, use named constants/macros and helper APIs only:
- Use `M_ByteN()` for byte extraction and `M_BytesTo*Value()` for byte assembly.
- Use named bit constants (`BIT0`...`BIT7`, `LBA_MODE_BIT`,
  `DEVICE_REG_BACKWARDS_COMPATIBLE_BITS`) instead of raw bit literals.
- If building SAT ATA PASS-THROUGH CDBs, use common CDB offset names from
  `scsi_helper.h` (`CDB_OPERATION_CODE`, `CDB_1`, `CDB12_CONTROL`, `CDB16_CONTROL`, etc.)
  instead of raw numeric indexes.
- Do not create per-command local offset enums when shared project offsets already exist.

**AUX, ICC, and the complete taskfile requirement:**
`set_ata_pt_aux_icc()` automatically upgrades `commandType` to
`ATA_CMD_TYPE_COMPLETE_TASKFILE` whenever any AUX or ICC byte is nonzero — you do not
need to set this manually. The `COMPLETE_TASKFILE` type is the only way to pass AUX/ICC
to the drive; `TASKFILE` and `EXTENDED_TASKFILE` cannot carry those fields.

The project does **not** currently use AUX fields. The ATA spec defines AUX bytes on some
READ/WRITE commands for hybrid drive media hints, but those hints are unimplemented here.
If you are implementing such a command and intentionally omitting AUX, add a `\todo` to
the Doxygen comment noting that AUX hybrid-hint support is not implemented.

### 4. Set command type and protocol
```c
ataCommandOptions.commandType      = ATA_CMD_TYPE_TASKFILE;   /* or EXTENDED_TASKFILE for 48-bit */
ataCommandOptions.commandDirection = XFER_DATA_IN;            /* match dataDirection input */
ataCommandOptions.commadProtocol   = ATA_PROTOCOL_PIO;        /* match protocol input */
```
Use `ATA_CMD_TYPE_EXTENDED_TASKFILE` for any 48-bit command.

### 5. Attach data buffer (if applicable)
```c
ataCommandOptions.ptrData    = buffer;
ataCommandOptions.dataSize   = bufferSize;
```
Leave `ptrData = M_NULLPTR` and `dataSize = 0` for non-data commands.

### 6. Set timeout
```c
ataCommandOptions.timeout = sataCommandTimeoutSeconds; /* use DEFAULT_COMMAND_TIMEOUT or 0 */
```

### 7. Issue the command
```c
eReturnValues ret = ata_Passthrough_Command(device, &ataCommandOptions);
```

### 8. Check return value
Always propagate errors:
```c
if (ret != SUCCESS)
{
    return ret;
}
```

### 9. Read RTFRs (if needed)
RTFRs are populated in `ataCommandOptions.rtfr` after a successful call:
```c
uint8_t status  = ataCommandOptions.rtfr.status;
uint8_t error   = ataCommandOptions.rtfr.error;
uint8_t lbaLow  = ataCommandOptions.rtfr.lbaLow;
/* 48-bit extended RTFRs are in ataCommandOptions.rtfr.*48 fields */
```

### 10. Declare in header
Add a declaration to `ata_helper_func.h` under the appropriate group:
```c
OPENSEA_TRANSPORT_API eReturnValues ata_Command_Name(tDevice* device, /* params */);
```

### 11. Add Doxygen comment
Use the `add-doxygen` prompt or follow `.github/instructions/doxygen.instructions.md`.
Required tags: `\brief`, `\param[in/out]` for every parameter, `\retval` for every
distinct return code, `\code{.c}` usage example.

When the command belongs to a specific ATA feature set, add a `\note` tag naming the
feature set and the spec version that introduced it. This enables Doxygen grouping by
feature. Common feature sets:

| Feature set | Abbreviation | Spec origin |
|-------------|--------------|-------------|
| Native Command Queuing | NCQ | ATA8-ACS |
| Self-Monitoring, Analysis and Reporting | SMART | ATA-3 |
| Device Configuration Overlay | DCO | ATA/ATAPI-6 |
| Host Protected Area | HPA | ATA/ATAPI-4; obsoleted by AMAC in ACS-3 |
| Accessible Max Address Configuration | AMAC | ACS-3 (HPA successor) |
| Trusted Computing Group security | TCG / ATA Security | ACS security feature set |
| Data Set Management (TRIM) | DSM | ACS-2 |
| Sanitize | — | ACS-2 |
| Streaming | — | ATA/ATAPI-7 |
| Hybrid Information | Hybrid | ACS-2; AUX field — not used in this codebase |

## Output Expectations

- One new function in `ata_cmds.c` that compiles cleanly.
- One matching declaration in `ata_helper_func.h`.
- A complete Doxygen block above the declaration.
- No direct calls to OS passthrough helpers.
- No magic numbers — use shared named constants/macros from `ata_helper.h` and
  `scsi_helper.h` where applicable.

## Quality Assurance

- [ ] `M_INITIALIZE_STRUCTURE` used (not partial initialisation)
- [ ] Shared helper APIs/constants used instead of raw numeric indexes/bit literals
- [ ] Correct `commandType` for 28 vs 48-bit
- [ ] All 48-bit TFR `*48` fields set when applicable
- [ ] `M_ByteN()` / `M_BytesTo*Value()` used for byte packing/unpacking
- [ ] `ata_Passthrough_Command()` used, not OS helpers directly
- [ ] Return value checked and propagated
- [ ] RTFRs read only when the spec requires them
- [ ] `safe_*` functions used for any memory operations
- [ ] Doxygen block complete with spec reference in `\note` or `\brief`
- [ ] Build passes (`meson compile -C builddir` or equivalent)
