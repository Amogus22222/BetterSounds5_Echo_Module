# Workbench MCP crash notes

Date: 2026-04-01

Related technical notes:

- [TECH_HINTS.md](TECH_HINTS.md)
- [Workbench script compiler crash notes, 2026-04-20](WORKBENCH_SCRIPT_COMPILER_CRASH_NOTES_2026-04-20.md)

## What happened

Workbench crashed while loading `G:\BetterSounds5\addon.gproj` with an access violation:

- `Access violation. Illegal read by ... at 0x18`
- The crash report stack pointed to `Scripts/WorkbenchGame/EnfusionMCP/EMCP_WB_GetState.c:98`
- The failure happened inside `EMCP_WB_GetState::GetResponse()`

## Root cause

`EMCP_WB_GetState` was calling `WorldEditorAPI` methods too early, during a transition state where the `WorldEditor` module existed but the editor scene was not ready yet.

The dangerous calls were:

- `api.GetEditorEntityCount()`
- `api.GetSelectedEntitiesCount()`
- `api.GetCurrentSubScene()`
- `api.GetSelectedEntity(i)`

This was not a BetterSounds5 gameplay crash. It was a Workbench bridge lifecycle crash.

## Fix already applied

Two readiness guards were added:

- `Scripts/WorkbenchGame/EnfusionMCP/EMCP_WB_GetState.c`
  - now returns `mode = "loading"` if `SCR_WorldEditorToolHelper.IsWorldLoaded()` is false
  - avoids touching `WorldEditorAPI` until the editor is actually ready
- `Scripts/WorkbenchGame/EnfusionMCP/EMCP_WB_Ping.c`
  - uses the same helper-based guard so health checks do not force a bad editor-state read

## What to remember next time

1. If Workbench crashes around startup, reload, or mode switching, check the bridge handlers first.
2. If the stack points at `EMCP_WB_GetState.c`, suspect `WorldEditorAPI` readiness, not mod content.
3. Before calling any editor enumeration API, use `SCR_WorldEditorToolHelper.IsWorldLoaded()` as the readiness gate.
4. If the bridge is being reinstalled, remember the handler scripts live under:
   - `Scripts/WorkbenchGame/EnfusionMCP/`
5. Before publishing the mod, remove the temporary bridge handlers with `wb_cleanup`.

## Validation status

- `wb_state` now returns successfully after the guard fix.
- Workbench bridge is stable again in edit mode.
- Current Workbench startup log confirms `GameProject load` with `G:/BetterSounds5/addon.gproj` in the loaded addons list.
- No engine/runtime validation has been done for this specific fix beyond the bridge reconnect and state query.
