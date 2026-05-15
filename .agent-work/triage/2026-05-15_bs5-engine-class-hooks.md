# Triage - BS5 engine-class hook compile failure

## Symptoms

- Workbench script reload failed:
  - `BS5_EchoDriverComponent.c,2661`: engine class `MuzzleEffectComponent` cannot be modded.
  - `BS5_ExplosionBridge.c,13`: engine class `BaseTriggerComponent` cannot be modded.

## Attempts already made

- Added broad `modded class MuzzleEffectComponent` and `modded class BaseTriggerComponent` during deslop pass to improve launcher/static explosive coverage.
- MCP `mod validate` did not catch this because it is not a full Workbench script compile.

## Evidence gathered

- Workbench `script.log` at 20:06 showed both engine-class mod errors and `Can't compile "Game" script module`.
- Current source scan shows only legal script hooks remain:
  - `modded class SCR_MuzzleEffectComponent`
  - `modded class SCR_ExplosiveTriggerComponent`
  - `modded class SCR_PressureTriggerComponent`
  - `BS5_ExplosionEchoEffect : BaseProjectileEffect`
- Workbench `script.log` at 20:10 shows `Game` and `WorkbenchGame` modules compiled after removing the engine-class hooks.

## Assumptions challenged

- Wrong: API-visible engine class means script can safely `modded` it.
- Wrong: `SCR_BaseTriggerComponentClass` is a runtime interception point.
- Corrected: `*ComponentClass` types are component metadata/class types; runtime callback interception belongs to component instances such as `SCR_BaseTriggerComponent` and subclasses.

## MCP/API/BIKI/base-game checks

- MCP API search confirmed `SCR_BaseTriggerComponent` inherits `BaseTriggerComponent` and exposes `TriggeredInSafetyDistance(...)`.
- MCP API search confirmed `SCR_BaseTriggerComponentClass` inherits `BaseTriggerComponentClass`; it is not the runtime callback owner.
- MCP prefab inspect confirmed `Mine_TM62M_base.et` active detonation effect container is `BaseTriggerComponent.PROJECTILE_EFFECTS`.
- MCP prefab inspect confirmed `Explosives_base.et` has no useful active effect container to hook globally.

## Subagent evidence

- Previous prefab scout found UGL GP25/M203 use `SCR_MuzzleEffectComponent`, so underbarrel launch coverage should not need engine `MuzzleEffectComponent`.
- Previous prefab scout found TM62M uses engine `BaseTriggerComponent.PROJECTILE_EFFECTS`, requiring prefab-effect coverage if script pressure hook is insufficient.

## Root cause candidate

- Engine classes are not script-moddable in Workbench. Coverage must be implemented either through legal SCR script subclasses or through prefab effect containers that instantiate `BS5_ExplosionEchoEffect`.

## Recovery path

- Remove engine-class `modded` hooks.
- Keep legal SCR hooks.
- For underbarrel launch shots, improve driver resolution by walking parent entities from UGL attachment to rifle owner.
- For TM62M, add a same-path prefab override on `Mine_TM62M_base.et` with exactly one `BS5_ExplosionEchoEffect` in `BaseTriggerComponent.PROJECTILE_EFFECTS`.

## Change applied, if any

- Removed attempted `MuzzleEffectComponent` and `BaseTriggerComponent` hooks.
- Added parent-chain driver lookup in `BS5_EchoRuntime.TryFindDriverOnEntity`.
- Added `Prefabs/Weapons/Explosives/Mine_TM62M/Mine_TM62M_base.et` same-path override preserving vanilla explosion effect and adding `BS5_ExplosionEchoEffect`.

## Validation result

- `git diff --check`: passed with LF-to-CRLF warnings only.
- MCP `mod validate` passed structure/gproj/scripts/prefabs with existing custom config API-index warnings.
- Workbench `script.log` after reload shows no BS5 script compile errors.
- MCP merged inspect shows `Mine_TM62M_base.et` has vanilla `ExplosionEffect` plus one `BS5_ExplosionEchoEffect`.

## Follow-up / handoff

- Runtime test TM62M pressure detonation. Expected markers:
  - `explosion dispatch source=projectile_effect` or `pressure_trigger`
  - `explosion analysis ... explosion=1`
  - `emit enter explosion=1`
- Runtime test M112/TSh400g. If `SCR_ExplosiveTriggerComponent` hook does not fire, add verified same-path `PROJECTILE_EFFECTS` prefab overrides for those demo block bases.
