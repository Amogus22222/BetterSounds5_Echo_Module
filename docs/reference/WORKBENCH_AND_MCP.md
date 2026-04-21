# Workbench And MCP Notes

## Scope

This is the only maintained Workbench/MCP reference note for the repo.

## MCP Bridge

The MCP bridge under `Scripts/WorkbenchGame/EnfusionMCP/` is intentionally kept in the repo.

Keep in mind:

- bridge issues are not automatically gameplay issues
- Workbench editor readiness must be checked before touching `WorldEditorAPI`
- if startup/reload crashes point into `EMCP_WB_*`, inspect bridge lifecycle first

## Known Stability Rules

### 1. Workbench startup must be validated with real project load

Static validation alone is not enough. Always prefer:

- real `addon.gproj` launch
- `Module: Game` compile confirmation
- `Game successfully created`

### 2. Be conservative with script-shape changes in hot classifier/runtime code

Past Workbench/compiler instability was associated with:

- adding new large top-level string hint arrays
- mechanically splitting stable scoring formulas into many temporary expressions

Working rule:

- prefer localized, minimal logic changes
- avoid broad stylistic rewrites in the hot classifier/runtime path

### 3. Separate environment issues from repo issues

Workbench may fail for reasons unrelated to the repo, for example:

- missing base `addons/data` resolution
- broken local Workbench install state
- MCP/editor readiness timing

Do not treat these as mod regressions unless the evidence points into repo code.

## Local Environment Note

On this machine, Workbench needed base `addons/data` resolution restored so the dependency `58D0FB3206B6F859` could load correctly.

That fix is local environment state, not a repo-tracked project change.

## Practical Rule

If a new patch causes suspicious Workbench behavior:

1. check whether the crash happens before or after `Compiling Game scripts`
2. confirm base game dependency resolution
3. inspect `EMCP_WB_*` bridge scripts if the stack points there
4. only then blame new BS5 gameplay/runtime code
