# BS5 deslop pass 2026-05-15

## Scope

- Debug stays enabled. No publish cleanup/debug-off changes in this pass.
- Implemented after runtime log evidence: RPG impact works as `explosion=1`; ordinary launcher shot path still emits `explosion=0`; underbarrel launcher shot path needed launch-time routing; static mines/demo blocks needed broader trigger coverage; heavy weapons/cannon needed explosion-like routing with close reflection disabled.

## Changes

- `Scripts/Game/BS5_ExplosionBridge.c`
  - Kept legal script hooks only: `SCR_ExplosiveTriggerComponent`, `SCR_PressureTriggerComponent`, and `BS5_ExplosionEchoEffect`.
  - Removed attempted `BaseTriggerComponent` modded hook after Workbench proved engine classes cannot be modded.

- `Scripts/Game/BS5_EchoDriverComponent.c`
  - Added underbarrel grenade launcher detection from projectile prefab path.
  - Underbarrel launch shots now use suppressed-like normal shot routing instead of RPG/MG launcher routing.
  - Removed attempted generic `MuzzleEffectComponent.OnFired` hook after Workbench proved engine classes cannot be modded.

- `Scripts/Game/BS5_EchoRuntime.c`
  - Driver lookup now walks the parent chain before root fallback, so underbarrel attachment muzzle effects can find the rifle-owned `BS5_EchoDriverComponent`.

- `Prefabs/Weapons/Explosives/Mine_TM62M/Mine_TM62M_base.et`
  - Added same-path override of the active `BaseTriggerComponent.PROJECTILE_EFFECTS` owner.
  - Preserved vanilla `ExplosionEffect` and added one `BS5_ExplosionEchoEffect` entry.
  - This replaces the impossible engine-class trigger hook for TM62M coverage.

- `Scripts/Game/BS5_DebugLog.c`, `BS5_EchoRuntime.c`, `BS5_SpatialSoundEmitterComponent.c`
  - Added one-time debug helper for fallback diagnostics.
  - Converted fallback-to-prefab/project/event invalid logs to once-per-key.
  - Added SoundMap `perf counts` and `perf effective` debug logs under the `soundmap` channel.

- Heavy/cannon prefabs:
  - `Prefabs/Weapons/HeavyWeapons/HMG_KPVT.et`
  - `Prefabs/Weapons/HeavyWeapons/HMG_M2HB.et`
  - `Prefabs/Weapons/HeavyWeapons/NSV/HMG_NSV_base.et`
  - `Prefabs/Weapons/Cannons/Cannon_M242/Cannon_M242_LAV25_base.et`
  - Set close reflection disabled.
  - Routed master/slapback ACPs and emitters to explosion-like BS5 resources.
  - Added BS5 runtime driver components to M242 local cannon prefab because it does not inherit `Weapon_Base.et`; did not add a new debug component there.

## Evidence

- MCP API verified `BaseTriggerComponent.TriggeredInSafetyDistance(...)`, `SCR_BaseTriggerComponent`, `SCR_ExplosiveTriggerComponent`, `SCR_PressureTriggerComponent`, and `MuzzleEffectComponent.OnFired(...)` signatures.
- Workbench script reload proved `BaseTriggerComponent` and `MuzzleEffectComponent` are engine classes and cannot be `modded`.
- MCP API confirmed `SCR_BaseTriggerComponentClass` is a component class/metadata type; it is not the runtime callback hook.
- MCP prefab inspect before changes showed:
  - `Mine_TM62M.et` owns `BaseTriggerComponent.PROJECTILE_EFFECTS` without BS5 hook.
  - `DemoBlock_M112_base.et` and `DemoBlock_TSh400g_base.et` own `SCR_ExplosiveTriggerComponent.PROJECTILE_EFFECTS` without BS5 hook.
  - `Explosives_base.et` has no useful active effect container.
  - Heavy weapons inherited BS5 but had close reflection enabled.
  - M242 did not have BS5 components before this pass.
- MCP prefab inspect after changes confirmed heavy weapons/cannon merged `m_bEnableCloseReflection 0` and explosion-like ACP/emitter values.
- MCP prefab inspect after changes confirmed `Mine_TM62M_base.et` merged `BaseTriggerComponent.PROJECTILE_EFFECTS` contains vanilla `ExplosionEffect` plus exactly one `BS5_ExplosionEchoEffect`.

## Validation

- `git diff --check`: passed; Git printed existing LF-to-CRLF warnings.
- MCP `mod validate` with `structure,gproj,scripts,prefabs,configs,references`: passed structure/gproj/scripts/prefabs; warnings only for BS5 custom config/script classes missing from API index.
- Workbench `wb_reload target=scripts`: triggered in edit mode.
- Workbench `script.log` at 20:10 confirmed `Game` and `WorkbenchGame` compiled after removing engine-class hooks. Remaining script messages were vanilla warnings, not BS5 compile errors.

## Runtime acceptance still needed

- Underbarrel GP-25/M203 launch shot:
  - expect `[BS5][driver] dispatch shot ... underbarrel=1 launcherShot=0 suppressed=1`
  - expect shot echo/slapbacks/close slapbacks through silenced-like routing.
- RPG/M72/RPG75 launch shot:
  - expect `launcherShot=1 underbarrel=0` and MG-like tail/slapback routing.
- Static explosives:
  - TM62M should emit `explosion dispatch source=projectile_effect|pressure_trigger` and `emit enter explosion=1`.
  - M112/TSh400g should emit `explosion dispatch source=trigger` and `emit enter explosion=1`; add prefab-effect override only if runtime proves the script trigger hook is not firing.
  - Duplicate callback may be suppressed; independent nearby detonations must not be swallowed.
- Heavy weapons/cannon:
  - HMG_KPVT, HMG_M2HB, HMG_NSV, M242 should emit explosion-like master/slapbacks and no close reflection acceptance.
- SoundMap instrumentation:
  - capture `perf counts` and `perf effective` for `light`, `default`, and `dynamic`.
