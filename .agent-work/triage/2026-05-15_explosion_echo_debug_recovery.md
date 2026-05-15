# 2026-05-15 Explosion Echo Debug Recovery

## Symptom

- No `[BS5]` debug lines were visible for explosion echo/slapbacks.
- M67 and M72 did not produce explosion echo/slapbacks.
- M433 produced a one-time preset registry error with `{0000000000000000}` config GUIDs.

## Confirmed

- `BS5_AudioDebugSettingsComponent` requires `m_bEnableDebug 1`; channel flags alone are not enough.
- The current `BS5_PresetRegistry.c` uses GUID-qualified config resource names.
- `Configs/BS5/Presets/*.conf.meta` GUIDs match the current source constants.
- The `{0000000000000000}` preset error does not exist in the current source tree and points to stale compiled/runtime state.
- Base-game prefab replacements must keep the original resource GUID; same-path files with new GUIDs are separate resources.
- `BaseProjectileEffect` in `TimerTriggerComponent` / `CollisionTriggerComponent` `PROJECTILE_EFFECTS` is the correct vanilla hook shape for M67/M72-style explosions.
- The vanilla M72 launcher points to `Ammo_Rocket_M72A3.et` as `{79FA751EEBE25DDE}`.
- The vanilla M67 resource is `{E8F00BF730225B00}`.

## Applied Fixes

- Enabled debug master flag and channels on `Prefabs/Props/BS5_ExplosionDriver.et`.
- Enabled debug master flag and channels on `Prefabs/Weapons/Core/Weapon_Base.et`.
- Made preset registry config constants GUID-qualified.
- Added direct `[BS5][explosion_hook]` diagnostics around explosion hook dispatch and global driver load/spawn failure paths.
- Changed the new M67 and M72 override `.meta` files to use the vanilla resource GUIDs.

## Remaining Runtime Checks

After a full Workbench restart/recompile:

1. Test M433 HEDP.
2. Test M67.
3. Test M72 LAW.
4. Capture `[BS5][explosion_hook]`, `[BS5][driver]`, `[BS5][emit]`, and any `Wrong GUID` lines.

Expected interpretation:

- No `[BS5][explosion_hook]` for M67/M72 means the prefab replacement is still not being used by the running resource DB/cache.
- `[BS5][explosion_hook]` plus `driver prefab load failed` means the global driver resource is not resolving in runtime.
- `[BS5][explosion_hook]` plus normal driver/emit logs but no sound means the issue moved to ACP/emitter/settings.
- Any remaining `{0000000000000000}` preset error after restart means Workbench/runtime is still executing stale compiled code or a stale resource cache.
