# Workbench script compiler crash notes

Date: 2026-04-20

## Symptom

Workbench 1.6.0.119 crashed while loading `G:\BettersMods\BetterSounds5\addon.gproj`.

Observed crash:

- `SEH exception thrown. Exception code: 0xc0000374`
- Crash happened during `Compiling Game scripts`
- No normal EnforceScript compile error was printed
- `mod validate` could still report `scripts` as passed, so the validator alone was not enough to catch this failure

## Trigger found

During the BS5 echo/slapback tuning pass, Workbench crashed when the patch added either:

- new large top-level `ref array<string>` hint lists for prefab classification, or
- "safer" split rewrites of existing dense score/condition formulas in hot classifier/runtime code.

The crash was native Workbench/compiler heap corruption, not a normal script syntax error.

Problematic pattern examples:

- adding a new global string hint array for hard reflectors
- replacing existing compact score expressions with several intermediate additive assignments
- expanding existing well-tested one-line conditions around facade/slapback candidate scoring

## Safe workaround

Keep this code style for Workbench 1.6:

- Do not add new large global `ref array<string>` hint tables unless absolutely necessary.
- Prefer reusing existing hint arrays and helper functions.
- If a new prefab class must be recognized, first try a small local check in an existing function and immediately test Workbench startup.
- Avoid mechanically splitting existing scoring formulas if they already compile.
- Test with a real Workbench project load, not only `mod validate`.

Valid startup test used:

```powershell
$exe = 'C:\Program Files (x86)\Steam\steamapps\common\Arma Reforger Tools\Workbench\ArmaReforgerWorkbenchSteamDiag.exe'
$wd = 'C:\Program Files (x86)\Steam\steamapps\common\Arma Reforger'
$p = Start-Process -FilePath $exe -WorkingDirectory $wd -ArgumentList '-gproj','G:\BettersMods\BetterSounds5\addon.gproj' -PassThru
Start-Sleep -Seconds 15
Get-Process -Id $p.Id -ErrorAction SilentlyContinue
```

Important: launching from the wrong working directory can produce a false-positive "alive" process because base addon dependencies are not loaded correctly.

## Fix used in the final patch

The final minimal patch avoided new global arrays and avoided formula rewrites.

It used only small local classification checks:

- `prefabName.IndexOf("structures") != -1` in facade/tail entity classification
- `prefabName.IndexOf("structures") != -1` in slapback entity classification
- rescue forward facade scan only when no forward facade was confirmed

This kept Workbench startup stable and still allowed `Prefabs/Structures/...` assets such as military blast covers, bunkers, and concrete shelters to participate in urban tail and slapback fallback logic.

## Validation status

After the final patch:

- `wb_reload scripts` completed successfully.
- Workbench connected through MCP.
- `mod validate` passed `scripts` and `prefabs`.
- Remaining warnings were existing custom config/API-index warnings, not compile errors.

