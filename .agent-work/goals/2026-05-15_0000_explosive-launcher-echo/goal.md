# Goal - explosive launcher echo timing

Generated: 2026-05-15 00:00
Mod root: G:\BettersMods\BettersMods\BetterSounds5_Echo_Module
.gproj: G:\BettersMods\BettersMods\BetterSounds5_Echo_Module\addon.gproj

## User request

Fix explosive echo for launcher/explosive cases where explosion echo is emitted at shot time, especially RPG22/RPG7/M72/RPG75 families, while keeping global explosion coverage as broad as practical. Shoulder/rocket launcher shots must still emit ordinary shot echo/slapbacks with launcher shot tails routed through MG-style tail ACP/emitter settings. Underbarrel grenade launcher shots must remain rifle-like for ordinary shot echo/slapbacks. Impact must separately emit explosion echo/slapbacks at the explosion position.

## Non-goals

- Do not retune slapback/close reflection values.
- Do not disable normal weapon-shot echo for launchers or underbarrel launchers.
- Do not revert existing debug/test prefab changes.

## Likely files/surfaces

- `Scripts/Game/BS5_ExplosionBridge.c`
- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_EchoTypes.c`
- `Prefabs/Weapons/Core/Launcher_Base.et`
- `Prefabs/Weapons/Ammo/Ammo_Rocket_*.et`
- existing base coverage in `Prefabs/Weapons/Core/*.et`

## MCP/API questions

- Does `SCR_WeaponBlastComponent.OnWeaponFired()` run on shot lifecycle rather than projectile impact?
- Which rocket ammo children override `CollisionTriggerComponent.PROJECTILE_EFFECTS` and therefore need same-path BS5 effect insertion?

## Implementation plan

1. Guard the weapon-blast bridge so projectile weapon fire does not emit explosion echo at muzzle time.
2. Add same-path replacements for rocket ammo children whose collision trigger effects currently lack `BS5_ExplosionEchoEffect`.
3. Route rocket/launcher shot tail echo through MG-style master ACP/emitter while leaving underbarrel GL shots rifle-like and leaving normal/close slapback routing intact.
4. Validate scripts, prefabs, configs/references, and inspect the patched prefab chains.

## Evidence ledger

- User RPG22 log shows explosion path owner is `Launcher_RPG22.et`, not `BS5_ExplosionDriver.et`, at shot frame.
- M433/M67 logs show `explosion dispatch source=projectile_effect localDriver=0` and owner `BS5_ExplosionDriver.et`, which is the intended impact-time path.
- MCP prefab inspect: `Ammo_GrenadeLauncher_Base.et` and local `Ammo_Rocket_M72A3.et` include `BS5_ExplosionEchoEffect`.
- MCP prefab inspect: base-game `Ammo_Rocket_PG22.et`, `Ammo_Rocket_PG7VM.et`, `Ammo_Rocket_PG7VL.et`, `Ammo_Rocket_PG7VR.et`, `Ammo_Rocket_RPG75.et`, and `Ammo_Grenade_HEDP_M433.et` collision trigger effects needed same-path `BS5_ExplosionEchoEffect` coverage.
- User follow-up RPG7/RPG22 logs show the fire-time explosion path now logs `explosion blast skip weapon fire-time`, but ordinary shot path still used `Weapons_Rifles_EchoMaster.acp`; launcher shot routing therefore needed a separate MG-style shot tail branch.
- MCP prefab inspect: `Prefabs/Weapons/Core/Launcher_Base.et` now inherits `Weapon_Base.et` and overrides `BS5_EchoDriverComponent`, `BS5_WeaponEchoRplCharacterComponent`, and `BS5_WeaponEchoSettingsComponent` to MG master ACP/emitter settings.
- MCP prefab inspect: `Prefabs/Weapons/Core/Ammo_GrenadeLauncher_Base.et` carries `BS5_ExplosionEchoEffect` on `ShellMoveComponent.ProjectileEffects`, but `Ammo_Grenade_HEDP_M433.et` also owns `CollisionTriggerComponent.PROJECTILE_EFFECTS`; the M433 leaf therefore still needs same-path coverage for the actual explosion trigger.
- MCP prefab inspect: AT mine bases such as `Mine_TM62M_base.et` and `Mine_M15AT_base.et` use `BaseTriggerComponent.PROJECTILE_EFFECTS` plus pressure trigger components. BS5 currently covers those through script hooks on `SCR_PressureTriggerComponent` / `SCR_ExplosiveTriggerComponent`; no mine prefab override was added in this slice to avoid double-dispatch without runtime evidence.

## Change log

- `SCR_WeaponBlastComponent.OnWeaponFired()` no longer calls `HandleExplosionFire()` for BS5. It leaves the vanilla `super` behavior intact and only logs a BS5 debug skip when driver debug is enabled.
- `HandleWeaponFire()` now tags rocket ammo and launcher-prefab shots as `launcherShot`; cached shot analysis is not reused across launcher/non-launcher mismatch. Underbarrel grenade ammo is deliberately not tagged, so M433 shot echo stays rifle-like through the host weapon.
- Non-explosion launcher shots now use `ResolveLauncherMasterAcp()` and `ResolveLauncherMasterEmitterPrefab()` for tail echo. Unsuppressed launcher shots route to the MG master ACP/emitter constants; suppressed shots keep suppressed routing.
- Added `m_bLauncherShot` to `BS5_EchoAnalysisResult` and included `launcherShot=` in driver/emit debug logs for runtime proof.
- `Launcher_Base.et` override now applies MG master tail ACP/emitter and MG limiter tail settings to vanilla launchers inheriting from that base.
- `Launcher_Base.et` was rebuilt as a full same-path launcher base override: it preserves vanilla launcher components (`SCR_WeaponBlastComponent`, `WeaponComponent` with `WeaponType RocketLauncher`, `SCR_MuzzleInMagComponent`, launcher gamepad effects) and only adds BS5 MG-style overrides on top.
- Added same-path vanilla leaf replacements for `Ammo_Rocket_PG22`, `Ammo_Rocket_PG7VM`, `Ammo_Rocket_PG7VL`, `Ammo_Rocket_PG7VR`, `Ammo_Rocket_RPG75`, and `Ammo_Grenade_HEDP_M433`, preserving vanilla fields and adding only `BS5_ExplosionEchoEffect` under `CollisionTriggerComponent.PROJECTILE_EFFECTS`.
- Added missing `.et.meta` files for the new same-path ammo overrides using the vanilla resource GUIDs from existing local Shrapnel/BHE metadata, so Workbench/resource DB can resolve them as replacements for the intended base-game resources.
- MCP `prefab inspect` confirmed the new BS5 effect entries on the patched leaves.
- `mod validate` passed `structure`, `gproj`, `scripts`, and `prefabs`; remaining warnings are existing BS5 API-index warnings.
- `git diff --check` passed with only CRLF warnings.

## Remaining risks

- Runtime confirmation still needs fresh in-game logs for RPG22/RPG7/RPG75 after Workbench/resource reload.
- Slapback ACP remains the existing normal/close slapback ACP because the mod currently has MG-specific master tail ACP/emitter, not a separate MG slapback ACP resource.
