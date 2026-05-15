# Project map - BetterSounds5

Generated: 2026-05-15 19:11 +03:00
Mod root: G:\BettersMods\BettersMods\BetterSounds5_Echo_Module
.gproj: G:\BettersMods\BettersMods\BetterSounds5_Echo_Module\addon.gproj

## Executive summary

BetterSounds5 is an Arma Reforger audio extension that adds scripted weapon-tail, slapback, close-reflection, and explosion echo routing. The main runtime path attaches `BS5_EchoDriverComponent` to weapon/explosion-driver prefabs, intercepts muzzle fire via a `modded SCR_MuzzleEffectComponent`, and intercepts detonation/impact cases through `BS5_ExplosionBridge.c` plus `BS5_ExplosionEchoEffect : BaseProjectileEffect`.

The system is script-heavy. The main risk surface is not asset volume; it is runtime analysis, SoundMap/terrain/entity probing, delayed emitter scheduling, limiter state, and prefab coverage for projectile/effect containers.

## Current repo state

Checked with `git status --short` before writing this map.

- Gameplay files: no non-`.agent-work` git diff was present at bootstrap.
- `.agent-work`: existing tracked audit/goal/map files were already deleted before this run; this map recreates `.agent-work/project-map.md`.
- Final hygiene status also shows unrelated untracked weapon prefab files under `Prefabs/Weapons/Cannons/` and `Prefabs/Weapons/HeavyWeapons/`. They were not part of this mapping pass and were left untouched.
- Workbench helper scripts are present under `Scripts/WorkbenchGame/EnfusionMCP/`. They are useful locally but are not publishable mod content and should be cleaned with `wb_cleanup` before Workshop publication.
- MCP merged-prefab inspection currently shows `BS5_AudioDebugSettingsComponent` enabled with debug level 2 on both `Prefabs/Weapons/Core/Weapon_Base.et` and `Prefabs/Props/BS5_ExplosionDriver.et`. This is live current state, not an old memory claim.

## Addon identity and dependencies

From `addon.gproj` and MCP `workshop_info`:

- ID: `BetterSounds5`
- GUID: `6717325A0F4513E2`
- Title: `BetterSounds5`
- Dependency: `58D0FB3206B6F859` (Arma Reforger)
- Project file: `G:\BettersMods\BettersMods\BetterSounds5_Echo_Module\addon.gproj`

## Functional overview

Core behavior:

- Adds echo-tail and slapback emitters to weapon fire.
- Supports ordinary, suppressed, MG/launcher-style, trench, close-reflection, and explosion-like routing.
- Uses environment and SoundMap analysis to choose reflector anchors and emitted tail profiles.
- Persists user audio settings and presets through settings menu integration.
- Uses prefab-side components for configuration and emitter ACP routing.

Important lifecycle split:

- Ordinary shot path: `SCR_MuzzleEffectComponent.OnFired` -> `BS5_EchoDriverComponent.HandleWeaponFire`.
- Fire-time blast callback: `SCR_WeaponBlastComponent.OnWeaponFired` -> `HandleExplosionFire`; must not be mistaken for impact detonation coverage.
- Projectile impact/detonation path: `BS5_ExplosionEchoEffect.OnEffect` and trigger-component hooks -> `BS5_EchoRuntime.DispatchExplosionEffect`.

## Entry points and runtime flow

Primary shot flow:

1. `Scripts/Game/BS5_EchoDriverComponent.c`
   - `modded class SCR_MuzzleEffectComponent`
   - `override OnFired(...)`
   - Finds `BS5_EchoDriverComponent` and calls `HandleWeaponFire(...)`.
2. `BS5_EchoDriverComponent.HandleWeaponFire(...)`
   - Detects suppressed state and launcher/ammo path.
   - Applies playback limiter admission.
   - Calls `BS5_EchoRuntime.AnalyzeShot(...)`.
   - Calls `BS5_EchoRuntime.EmitShot(...)`.
3. `Scripts/Game/BS5_EchoRuntime.c`
   - `BS5_EchoEnvironmentAnalyzer.Analyze(...)` builds near confinement, slapback, tail, SoundMap, and environment signals.
   - `BS5_EchoEmissionService.Emit(...)` queues tail/slapback emission contexts.
   - `QueueEmission(...)`, `EmitPending(...)`, and `EmitOnEmitter(...)` schedule playback and cleanup.
4. `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
   - Plays via `SoundComponent`/audio project routing and writes runtime signals to `SignalsManagerComponent`.

Explosion flow:

1. `Scripts/Game/BS5_ExplosionBridge.c`
   - `modded SCR_WeaponBlastComponent.OnWeaponFired(...)`
   - `modded SCR_ExplosiveTriggerComponent.TriggeredInSafetyDistance(...)`
   - `modded SCR_PressureTriggerComponent.TriggeredInSafetyDistance(...)`
   - `BS5_ExplosionEchoEffect.OnEffect(...)`
2. `BS5_EchoRuntime.DispatchExplosionEffect(...)`
   - Resolves the explosion driver, owner/origin/forward, and calls explosion analysis/emission.
3. `Prefabs/Props/BS5_ExplosionDriver.et`
   - Provides standalone `BS5_EchoDriverComponent`, close-reflection settings, debug settings, explosion ACPs, and explosion emitter prefab refs.

Settings/UI flow:

1. `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`
   - `modded SCR_AudioSettingsSubMenu`
   - Adds/updates BS5 echo/slapback/close/explosion controls and preset widgets.
2. `Scripts/Game/BS5_PlayerAudioSettings.c`
   - `BS5_GameAudioSettings : ModuleGameSettings`
   - Static `BS5_PlayerAudioSettings` getters/setters/save/load.
3. `Scripts/Game/BS5_PresetRegistry.c`
   - Loads sound and technical presets from `Configs/BS5/Presets/*.conf`.

## Script map

`Scripts/Game/BS5_EchoTypes.c`

- Data model and math utilities.
- Defines `BS5_EchoEnvironmentType`, `BS5_TailProfileType`, `BS5_VegetationClass`, `BS5_EchoCandidateSourceType`, `BS5_DebugChannel`, and `BS5_DebugLevel`.
- Owns `BS5_EchoReflectorCandidate`, `BS5_EchoAnalysisResult`, `BS5_EnvironmentSnapshot`, `BS5_PendingEmissionContext`, `BS5_ActiveEchoVoice`, and `BS5_EchoMath`.

`Scripts/Game/BS5_EchoDriverComponent.c`

- Main per-weapon/per-driver config component.
- Classes: `BS5_EchoDriverComponentClass`, `BS5_EchoDriverComponent`, `BS5_WeaponEchoSettingsComponent`, `BS5_WeaponEchoRplCharacterComponent`, `modded SCR_MuzzleEffectComponent`.
- Responsibilities: attribute defaults, resource resolution, shot dispatch, explosion dispatch, playback limiter config, analysis cache, tail-sector cache, forward-facade negative cache, active emitter budget, debug channel bridge.
- Key symbols from code scout: `BS5_EchoDriverComponent : ScriptComponent` at line 6, `EOnInit` line 514, `OnDelete` line 556, `HandleWeaponFire` line 581, `HandleExplosionAt` line 686, launcher/ammo detection around lines 2294-2315, duplicate-shot guard around lines 2345-2354, limiter cadence around lines 2356-2383, `modded SCR_MuzzleEffectComponent.OnFired` around lines 2636-2645.

`Scripts/Game/BS5_EchoRuntime.c`

- Main analysis and emission runtime.
- Classes: `BS5_EchoRuntime`, `BS5_EchoEnvironmentAnalyzer`, `BS5_EchoEmissionService`.
- Responsibilities: driver lookup, explosion dispatch, near tracing, slapback collection, tail candidate generation, emitter queueing, SoundManager fallback, limiter active voice bookkeeping, delayed cleanup.
- Key symbols from code scout: `FindDriver` lines 9-19, `FindExplosionDriver` lines 21-28, `DispatchExplosionEffect` lines 30-82, frame/origin dedupe around lines 120-139, global driver spawn fallback around lines 142-187, `BS5_EchoEmissionService` starts line 1983, emission queue starts line 2237, start gate around lines 3256-3344, SoundManager fallback around lines 3346-3432.

`Scripts/Game/BS5_EnvironmentAudioClassifier.c`

- Environment snapshot and long-tail anchor planning.
- Classes: `BS5_EnvironmentAudioClassifier`, `BS5_HybridTailPlanner`, `BS5_SoundMapAnchorSample`, `BS5_SoundMapAnchorPlanner`.
- Responsibilities: global/entity audio signal reads, terrain height/normal sampling, SoundMap forward/omni samples, urban micro-scan, facade/entity candidates, path plausibility, terrain front-slope validation.
- Key symbols from code scout: `BuildSnapshot` lines 26-83, terrain helpers around lines 144-220, `BS5_HybridTailPlanner.BuildCandidates` around lines 330-436, forward facade entity scan around lines 617-760, `BS5_SoundMapAnchorPlanner.TryBuildCandidates` starts around line 2464, SoundMap path validation around lines 3176-3372, terrain-profile validation around lines 3393-3491.

`Scripts/Game/BS5_CloseReflectionPlanner.c`

- Dedicated close slapback/reflection planner.
- Classes: `BS5_CloseReflectionProbeHit`, `BS5_CloseReflectionPlannerResult`, `BS5_CloseReflectionSupportPoint`, `BS5_CloseReflectionEvidence`, `BS5_CloseReflectionPlanner`.
- Responsibilities: close pocket evidence, side/front-back/corner pair scoring, roof/wall rescue traces, candidate synthesis.
- Key symbols from code scout: planner entry `Evaluate` line 97, direct acceptance around line 372, roof rescue around line 414, wall rescue around line 466, rescue trace `TraceProbeHit` around line 540.

`Scripts/Game/BS5_CloseReflectionSettingsComponent.c`

- Component for close-reflection tuning and resource overrides.
- Resolves close slapback ACP, emitter prefab, event name, thresholds, rescue permissions, intensity/reverb/tail-width/surface-hardness shaping.

`Scripts/Game/BS5_SpatialSoundEmitterComponent.c`

- Component placed on emitter prefabs.
- Uses `EOnInit`, resolves `SoundComponent` and `SignalsManagerComponent`, applies BS5 signals, plays requested event, and maintains audio-project invalid/initialized caches.
- Key symbols from code scout: `EOnInit` lines 14-18, `Play` lines 25-85, `TryPlayAudioSystemProject` lines 87-104, signal builder lines 173-254, component binding lines 279-292, signal writes lines 294-320.

`Scripts/Game/BS5_ExplosionBridge.c`

- Explosion coverage bridge.
- Modded classes: `SCR_WeaponBlastComponent`, `SCR_ExplosiveTriggerComponent`, `SCR_PressureTriggerComponent`.
- Projectile effect class: `BS5_ExplosionEchoEffect : BaseProjectileEffect`.
- Dispatches fire-time blast and actual projectile/trigger detonation paths into `BS5_EchoRuntime`.
- Code scout confirmed the lifecycle split: `SCR_WeaponBlastComponent.OnWeaponFired` lines 1-11 is the fire-time blast path, trigger hooks at lines 13-29 call the detonation/impact dispatch, and `BS5_ExplosionEchoEffect.OnEffect` lines 31-38 is the projectile-effect path.

`Scripts/Game/BS5_AudioDebugSettingsComponent.c`

- Channelized prefab debug component.
- Current script defaults have master debug off, but live merged `Weapon_Base.et` and `BS5_ExplosionDriver.et` override it on.

`Scripts/Game/BS5_DebugLog.c`

- Small logging facade for channel/level checks and bool text.

`Scripts/Game/BS5_PlayerAudioSettings.c`

- User-facing setting persistence.
- `BS5_GameAudioSettings : ModuleGameSettings` stores echo/slapback/close/explosion volume, slapback enabled, technical preset id, and sound preset id.
- Key symbols from code scout: settings module lines 1-23, getters/setters lines 48-281, initialization/hook/load path lines 292-371.

`Scripts/Game/BS5_PresetRegistry.c`

- Config-backed preset registry with fallback preset generation.
- Important because technical limits exist both in code/fallbacks and `Configs/BS5/Presets/BS5_TechnicalPresets.conf`.
- Key symbols from code scout: config refs around lines 321-322, lookup/apply methods lines 331-482, load/fallback path starts around line 484.

`Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`

- Large UI mod of `SCR_AudioSettingsSubMenu`.
- Adds BS5 widgets, handles slider/click/change events, syncs menu controls to `BS5_PlayerAudioSettings`, and applies presets through `BS5_PresetRegistry`.
- Key symbols from code scout: `OnTabShow` lines 82-86, `OnTabHide` lines 117-121, `OnMenuHide` lines 123-127, `OnTabRemove` lines 129-133, input handlers lines 135-211, row builders around lines 283-940, refresh/writeback lines 1031-1227, preset steppers lines 1256-1307.

## Script details

Resource resolution order is layered:

- `BS5_EchoDriverComponent.ResolveMasterAcp`, `ResolveSlapbackAcp`, `ResolveExplosionAcp`, and emitter-prefab counterparts first read driver fields.
- Weapon-level `BS5_WeaponEchoSettingsComponent` and character `BS5_WeaponEchoRplCharacterComponent` provide legacy/compat hooks.
- Emitter prefabs provide final `SoundComponent` ACP fallback when direct audio project playback fails.

Limiter and queue state lives in:

- Driver getters and per-owner active budget in `BS5_EchoDriverComponent.c`.
- Global voice/start-gate bookkeeping in `BS5_EchoEmissionService` inside `BS5_EchoRuntime.c`.
- Important controls: `m_iLimiterMaxTailStartsPer100Ms`, `m_iLimiterMaxSlapbackStartsPer100Ms`, `m_iLimiterGlobalMaxTailVoices`, `m_iLimiterGlobalMaxSlapbackVoices`, pending tail cap, owner cap, burst cadence.

Environment planning has two generations:

- Current preferred path: `BS5_SoundMapAnchorPlanner` when `m_bUseSoundMapAnchorPlanner` is enabled.
- Legacy/fallback path: `BS5_HybridTailPlanner` and obstacle/terrain trace sectors. `m_bAllowLegacyAnchorFallback` controls fallback.

Debug path:

- `BS5_AudioDebugSettingsComponent` is the intended channel gate.
- `BS5_DebugLog` centralizes print calls.
- Old raw `Print()` searches can be noisy; map/debug work should prefer the explicit debug component, `EXPLOSION_DIAGNOSTIC_LOG` if present in code, and `BS5_DebugChannel`.

## Prefabs/configs/layouts/resources

Key weapon/base prefabs:

- `Prefabs/Weapons/Core/Weapon_Base.et`
  - Adds `BS5_AudioDebugSettingsComponent`, `BS5_CloseReflectionSettingsComponent`, `BS5_EchoDriverComponent`, `BS5_WeaponEchoRplCharacterComponent`, and `BS5_WeaponEchoSettingsComponent`.
  - Routes rifle, suppressed, slapback, trench, close, and explosion ACP/emitter resources.
  - MCP merged view confirms limiter gate values `m_iLimiterMaxTailStartsPer100Ms 12`, `m_iLimiterMaxSlapbackStartsPer100Ms 12`, and `m_iLimiterGlobalMaxSlapbackVoices 12`.
- `Prefabs/Weapons/Core/MachineGun_Base.et`
  - Overrides driver to MG ACP and MG tail emitter; tighter per-owner/start cadence.
- `Prefabs/Weapons/Core/Launcher_Base.et`
  - Overrides to MG-style tail ACP/emitter and includes `SCR_WeaponBlastComponent`.
- `Prefabs/Weapons/Core/Ammo_GrenadeLauncher_Base.et`
  - Has `ProjectileEffects` and a `BS5_ExplosionEchoEffect`.
- `Prefabs/Weapons/Core/Grenade_Base.et`
  - Has `ProjectileEffects` and a `BS5_ExplosionEchoEffect`.
- `Prefabs/Weapons/Core/Explosives_base.et`
  - Small base surface; needs merged prefab inspection before assuming child mine/charge coverage.
- `Prefabs/Weapons/Core/Launcher_Base.et` and `Prefabs/Weapons/Core/MachineGun_Base.et` inherit `Weapon_Base.et` and swap master/tail resources to the MG ACP/emitter variant. They also tighten cadence/per-owner/start-gate knobs.

Projectile/grenade leaf prefabs with explicit BS5 explosion effects:

- `Prefabs/Weapons/Ammo/Ammo_Grenade_HEDP_M433.et`
- `Prefabs/Weapons/Ammo/Ammo_Rocket_M72A3.et`
- `Prefabs/Weapons/Ammo/Ammo_Rocket_PG22.et`
- `Prefabs/Weapons/Ammo/Ammo_Rocket_PG7VL.et`
- `Prefabs/Weapons/Ammo/Ammo_Rocket_PG7VM.et`
- `Prefabs/Weapons/Ammo/Ammo_Rocket_PG7VR.et`
- `Prefabs/Weapons/Ammo/Ammo_Rocket_RPG75.et`
- `Prefabs/Weapons/Grenades/Grenade_M67.et`
- `Prefabs/Weapons/Grenades/Grenade_RGD5.et`

Prefab scout confirmed this pattern matters: these leaf prefabs own local `CollisionTriggerComponent.PROJECTILE_EFFECTS` or `TimerTriggerComponent.PROJECTILE_EFFECTS`, so base-only hook claims are not enough. `Grenade_Base.et` and `Ammo_GrenadeLauncher_Base.et` contain base effect entries, but `Grenade_RGD5.et`, `Grenade_M67.et`, and several rocket leaf prefabs override or add local containers.

Emitter/driver prefabs:

- `Prefabs/Props/BS5_TailEmitter.et` -> rifle echo ACP.
- `Prefabs/Props/BS5_TailEmitter_MG.et` -> MG echo ACP.
- `Prefabs/Props/BS5_TailEmitter_Silenced.et` -> suppressed echo ACP.
- `Prefabs/Props/BS5_SlapbackEmitter.et` -> standard slapback ACP.
- `Prefabs/Props/BS5_SlapbackEmitter_Close.et` -> close slapback ACP.
- `Prefabs/Props/BS5_SlapbackEmitter_Silenced.et` -> suppressed slapback ACP.
- `Prefabs/Props/BS5_SlapbackEmitter_Trench.et` -> trench slapback ACP.
- `Prefabs/Props/BS5_ExplosionEmitter.et` -> explosion echo ACP.
- `Prefabs/Props/BS5_ExplosionSlapbackEmitter.et` -> explosion slapback ACP.
- `Prefabs/Props/BS5_ExplosionDriver.et` -> standalone explosion driver used by runtime fallback/spawn path.

Prefab scout classified the BS5 emitter prefabs as bare emitter shells with `SoundComponent` plus `SignalsManagerComponent` and `BS5_SpatialSoundEmitterComponent`. The authored ACPs wired through these shells are:

- `Weapons_Rifles_EchoMaster.acp`
- `Weapons_MG_EchoMaster.acp.acp`
- `Weapons_Silinced_EchoMaster.acp.acp`
- `Weapons_Slapbacks_Master.acp`
- `Weapons_Slapbacks_Close_Master.acp`
- `Weapons_Slapbacks_Trench_Master.acp`
- `Weapons_Slapbacks_Silinced_Master.acp`
- `Weapons_Explosions_EchoMaster.acp`
- `Weapons_Explosions_Slapbacks_Master.acp`

Configs:

- `Configs/BS5/Presets/BS5_SoundPresets.conf`
  - Presets: `vanilla`, `bettersounds_v4`, `bettersounds_v5`, `lunacy_audio`.
  - Controls user-facing echo/slapback/close/explosion volume defaults.
- `Configs/BS5/Presets/BS5_TechnicalPresets.conf`
  - Presets: `default`, `light`, `dynamic`.
  - Controls runtime budgets and trace/SoundMap parameters.
  - Default and dynamic are high-coverage; light reduces trace counts, active voices, starts-per-window, and SoundMap samples.

Audio/signals:

- `Sounds/Weapons/Rifles/BS5/*.acp` are the authored master/slapback/explosion audio projects.
- `Sounds/_SharedData/Signals/BS5/*.sig` define exposed user/intensity signals.
- Important signal resources observed by prefab scout: `BS5_Intensity.sig`, `BS5_UserExplosionVolume.sig`, `BS5_UserSlapbackCloseVolume.sig`, `BS5_UserSlapbackVolume.sig`.
- `Assets/Gunpowder/**`, `Assets/BulletSounds/**`, and `Assets/explosions/**` hold source wav assets.

Layouts/localization/worlds:

- No `.layout`, localization string table, or world/mission resources were found in the local file inventory.
- `UserMaps.desc` exists but is not a primary runtime surface for this map.

## Dependency graph

High-level code dependencies:

- `SCR_MuzzleEffectComponent.OnFired`
  -> `BS5_EchoDriverComponent.HandleWeaponFire`
  -> `BS5_EchoRuntime.AnalyzeShot`
  -> `BS5_EchoEnvironmentAnalyzer.Analyze`
  -> `BS5_EnvironmentAudioClassifier.BuildSnapshot`
  -> `BS5_SoundMapAnchorPlanner` or `BS5_HybridTailPlanner`
  -> `BS5_EchoEmissionService.Emit`
  -> `BS5_SpatialSoundEmitterComponent.Play`

- `BS5_ExplosionEchoEffect.OnEffect` / trigger hooks
  -> `BS5_EchoRuntime.DispatchExplosionEffect`
  -> `BS5_EchoDriverComponent.HandleExplosionAt`
  -> `BS5_EchoRuntime.AnalyzeExplosion`
  -> `BS5_EchoEmissionService.Emit`

- `SCR_AudioSettingsSubMenu`
  -> `BS5_AudioSettingsSubMenu`
  -> `BS5_PlayerAudioSettings`
  -> `BS5_PresetRegistry`
  -> `Configs/BS5/Presets/*.conf`
  -> driver getter overrides at runtime.

Prefab/resource dependencies:

- Weapon base / launcher / MG prefabs own the driver config.
- Ammo/grenade leaf prefabs own active `PROJECTILE_EFFECTS`/`ProjectileEffects` containers.
- Emitter prefabs own `SoundComponent` ACP fallback routing.
- ACPs consume BS5 signals set by `BS5_SpatialSoundEmitterComponent`.

## API / BIKI / base-game context checked

Checked through Enfusion MCP/API and prefab tools:

- `SCR_MuzzleEffectComponent`
  - API confirms `OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)` and `GetOnWeaponFired()`.
  - This matches the modded shot hook in `BS5_EchoDriverComponent.c`.
- `BaseProjectileEffect`
  - API confirms `OnEffect(IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, Instigator instigator, string colliderName, float speed)`.
  - This matches `BS5_ExplosionEchoEffect.OnEffect`.
- `SCR_WeaponBlastComponent`
  - API surface exists and provides weapon blast component lifecycle; `OnWeaponFired` is used by the modded class.
- `SCR_ExplosiveTriggerComponent`
  - API confirms explosive trigger behavior and public trigger methods; modded trigger hook is plausible but should be runtime-log verified per explosive type.
- `SCR_PressureTriggerComponent`
  - API confirms pressure trigger component and contact/activation lifecycle; mine/static coverage still needs leaf prefab verification.
- `SCR_AudioSettingsSubMenu`
  - API confirms settings menu lifecycle methods including `OnTabShow`, `OnTabHide`, and menu callbacks.
- `ChimeraCharacter.TraceMoveWithoutCharacters`
  - API confirms it is an optimized trace variant that filters out ChimeraCharacter entities. This is heavily used in analysis/planning.
- `SCR_SoundManagerModule.CreateAudioSource`
  - API confirms overloads for custom `SCR_AudioSourceConfiguration` and world position, matching the fallback/managed playback path.
- MCP `prefab(action=inspect)` was used for `Prefabs/Weapons/Core/Weapon_Base.et` and `Prefabs/Props/BS5_ExplosionDriver.et`.
- MCP `workshop_info` was used for addon identity/dependency.

## Performance-sensitive surfaces

Highest-risk runtime surfaces:

- `BS5_EchoRuntime.c`
  - Near/slapback traces: `ChimeraCharacter.TraceMoveWithoutCharacters` in near probe and slapback collection.
  - Tail candidate traces: sector x band x distance sample x height sample loops.
  - `BS5_EchoEmissionService` delayed queues, voice stealing, start gates, and cleanup scheduling.
  - Code scout highlighted heavy scans in pending/reserved/active voice lists, queue admit/steal paths, candidate emission loops, and cleanup sweeps.
- `BS5_EnvironmentAudioClassifier.c`
  - SoundMap forward/omni scans.
  - Urban micro-scan entity queries and facade confirmation traces.
  - Path plausibility sampling and optional geometry raycasts.
  - Terrain height/normal sampling loops.
  - Code scout highlighted nested candidate sorting, repeated path-profile samples, SoundMap path raycast gates, and urban candidate pressure passes as the biggest heuristic-cost surface.
- `BS5_CloseReflectionPlanner.c`
  - Extra roof/wall rescue traces on close-reflection candidates.
- `BS5_AudioSettingsSubMenu.c`
  - Large UI modded class; lower runtime audio risk, but high maintenance/conflict risk against upstream settings menu changes.
- Prefab/config tuning
  - Technical presets can radically alter trace count, sample count, candidate count, and limiter gates without script changes.

## AI-slop risk targets

Audit/deslop should focus here first:

- Lifecycle separation: ordinary shot echo, fire-time blast callback, and actual detonation impact must remain separate. Do not prove one lifecycle and claim all three.
- Prefab coverage: base-prefab additions do not guarantee children with their own `PROJECTILE_EFFECTS`, `ProjectileEffects`, `CollisionTriggerComponent`, `TimerTriggerComponent`, or warhead/effect containers are covered.
- Resolver consistency: ACP/emitter fallback order is layered. Do not claim MG-style, suppressed, slapback, close, or explosion routing is changed unless the exact branch/resource is verified.
- Debug publish hygiene: live merged prefabs currently show debug enabled at `Weapon_Base.et` and `BS5_ExplosionDriver.et`; do not publish until this is intentionally reviewed.
- Limiter tuning drift: code defaults, prefab defaults, and technical preset config can diverge.
- SoundMap/entity-query cost: broad city/urban settings can look good in isolated tests but spike in dense scenes.
- UI monolith: `BS5_AudioSettingsSubMenu.c` is large and modded; future changes should be surgical and tested in menu lifecycle.
- Workbench helper files: `Scripts/WorkbenchGame/EnfusionMCP/*` should not be left in a publish build.
- Resource names with typos such as `Silinced` and doubled `.acp.acp` appear to be existing resource names; do not rename casually without resource GUID/workbench verification.

## Open questions / unverified assumptions

- Runtime logs were not generated during this mapping pass, so active in-game behavior is inferred from scripts/prefabs/MCP, not freshly replayed.
- Leaf prefab coverage was scanned locally, but not every grenade/rocket/mine/explosive descendant was MCP-merged inspected.
- Workbench was not launched; script compile/reload was not run as part of this read-only map.
- `resourceDatabase.rdb` exists; publish/build state was not verified in this pass.
- Subagent scout results were folded into this map after the initial draft. Both scouts were read-only and made no file changes.

## Recommended next skill

- Use `bhe-audit` next if the goal is anti-slop/deslop preparation.
- Use `bhe-repo` next if the goal is Workshop publish hygiene, especially debug-off and `Scripts/WorkbenchGame/EnfusionMCP` cleanup.
- Use `bhe-goal` next for one concrete gameplay/audio bug or feature slice.
