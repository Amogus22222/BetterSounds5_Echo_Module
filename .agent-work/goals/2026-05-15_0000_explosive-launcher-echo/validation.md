# Validation

- `prefab inspect` confirmed `BS5_ExplosionEchoEffect` under `CollisionTriggerComponent.PROJECTILE_EFFECTS` for:
  - `Prefabs/Weapons/Ammo/Ammo_Rocket_PG22.et`
  - `Prefabs/Weapons/Ammo/Ammo_Rocket_PG7VM.et`
  - `Prefabs/Weapons/Ammo/Ammo_Rocket_PG7VL.et`
  - `Prefabs/Weapons/Ammo/Ammo_Rocket_PG7VR.et`
  - `Prefabs/Weapons/Ammo/Ammo_Rocket_RPG75.et`
  - `Prefabs/Weapons/Ammo/Ammo_Grenade_HEDP_M433.et`
- `mod validate` passed `structure`, `gproj`, `scripts`, and `prefabs`.
- `git diff --check` passed with only CRLF warnings.
- MCP `prefab inspect Prefabs/Weapons/Core/Launcher_Base.et` confirmed merged MG-style launcher shot tail settings:
  - `BS5_EchoDriverComponent.m_sMasterAcp = {1C6E4055FD8532F6}Sounds/Weapons/Rifles/BS5/Weapons_MG_EchoMaster.acp.acp`
  - `BS5_EchoDriverComponent.m_sMasterEmitterPrefab = {4CB3F211A7DF906E}Prefabs/Props/BS5_TailEmitter_MG.et`
  - `BS5_WeaponEchoRplCharacterComponent.m_DefaultEchoAcp` and `BS5_WeaponEchoSettingsComponent.m_EchoProject` also point to the MG master ACP.
- MCP `prefab inspect Prefabs/Weapons/Core/Launcher_Base.et` confirmed the same-path override also preserves vanilla launcher components, including `SCR_WeaponBlastComponent`, `WeaponComponent`, `SCR_MuzzleInMagComponent`, `WeaponType RocketLauncher`, and launcher gamepad trigger effects.
- MCP `prefab inspect Prefabs/Weapons/Ammo/Ammo_Rocket_PG7VM.et` confirmed `BS5_ExplosionEchoEffect` remains in the merged collision trigger projectile effects.
- MCP `prefab inspect Prefabs/Weapons/Core/Ammo_GrenadeLauncher_Base.et` confirmed base GL projectile effects carry `BS5_ExplosionEchoEffect`.
- MCP `prefab inspect Prefabs/Weapons/Ammo/Ammo_Grenade_HEDP_M433.et` confirmed M433 inherits the base GL `ShellMoveComponent` BS5 effect and also has local `CollisionTriggerComponent.PROJECTILE_EFFECTS` with local `BS5_ExplosionEchoEffect`.
- MCP `prefab inspect Prefabs/Weapons/Explosives/Mine_TM62M/Mine_TM62M_base.et` and `Mine_M15AT_base.et` confirmed mine explosion effects live under `BaseTriggerComponent.PROJECTILE_EFFECTS`; this slice relies on existing `SCR_PressureTriggerComponent` / `SCR_ExplosiveTriggerComponent` script hooks for mines.
- MCP `mod validate --checks structure,gproj,scripts,prefabs,references` passed after launcher shot MG routing changes and meta additions. Warning: `BS5_EnvironmentAudioClassifier.c` extends `BS5_HybridTailPlanner`, which MCP API index does not know; this is pre-existing local class/index limitation.

Remaining runtime validation:

- Reload/restart Workbench/resource database if needed.
- Fire RPG22/RPG7/RPG75 once and verify the shot frame no longer logs `emit enter explosion=1` with launcher owner.
- Verify rocket launcher shot frame logs `dispatch shot ... launcherShot=1`, `emit enter explosion=0 launcherShot=1`, and tail queues use `Weapons_MG_EchoMaster.acp.acp`.
- Verify M433/UGL shot frame logs ordinary rifle-like shot path (`launcherShot=0`) while impact still logs explosion path.
- Verify impact still logs `explosion dispatch source=projectile_effect`, `emit enter explosion=1`, and plays explosion echo/slapbacks from explosion prefabs.
- For mines, verify runtime log source is `pressure_trigger` or `trigger` and there is no duplicate `projectile_effect` at the same origin/frame.
