# Project map - BetterSounds5

Generated: 2026-05-15
Mod root: `G:\BettersMods\BettersMods\BetterSounds5_Echo_Module`
Branch observed: `codex-close-reflection-plan`

## Executive summary

BetterSounds5 is an Arma Reforger audio extension focused on weapon echoes, reflection tails, slapbacks, close-space reflections, and related audio settings. Runtime behavior is script-heavy: weapon and blast hooks feed a central echo driver, which analyzes environment geometry and terrain, chooses tail/slapback candidates, and emits audio through spawned emitter prefabs, `SoundComponent`, `SignalsManagerComponent`, `AudioSystem`, and `SCR_SoundManagerModule`.

The mod is not a small resource pack. Its main risk surface is the hot shot path: `SCR_MuzzleEffectComponent.OnFired` -> `BS5_EchoDriverComponent.HandleWeaponFire` -> `BS5_EchoRuntime` / `BS5_EnvironmentAudioClassifier` / `BS5_CloseReflectionPlanner` -> `BS5_EchoEmissionService` -> `BS5_SpatialSoundEmitterComponent`.

Current high-value audit targets are:

- Resource reference drift in ACP/prefab paths after recent cleanup.
- Trace/entity-query cost in environment classification and close-reflection rescue logic.
- Large mutable state in driver/runtime caches and playback limiter.
- Procedural audio settings UI lifecycle and save/flush behavior.
- Publish hygiene around `Scripts/WorkbenchGame/EnfusionMCP`.

## Current git/worktree state

At mapping time `git status --short --untracked-files=all` was clean before writing this map.

Recent history:

- `e3894b6 Add project map for BetterSounds5 Echo Module`
- `9543c6c Add close-reflection system and assets`
- `bc4615e Tune BS5 weapon audio and slapback assets`
- `23da175 docs(bs5): normalize repo docs layout`

`.agent-work/` was absent before this run. This file is local scratch by policy and should not be treated as publishable mod content unless explicitly requested.

## Addon identity and dependencies

Manifest: `addon.gproj`

```text
ID "BetterSounds5"
GUID "6717325A0F4513E2"
TITLE "BetterSounds5"
Dependencies {
 "58D0FB3206B6F859"
}
```

Observed top-level folders:

- `Assets/` - wave source assets, mainly gunpowder tails, slapbacks, city/forest/field reflections, bullet sounds.
- `Configs/` - BS5 sound and technical preset config resources.
- `Prefabs/` - weapon base overlays and audio emitter prefabs.
- `Scripts/` - game scripts plus Workbench MCP handler scripts.
- `Sounds/` - ACP audio projects and signal resources.
- `tools/` - PowerShell helper scripts for Workbench/repo environment checks.

Missing by current tree scan: `Worlds/`, `Missions/`, `Language/`, `Layouts/`, root `UI/`, `docs/`, `UserMaps.desc`.

## Functional overview

BetterSounds5 attaches echo-related components to weapon base prefabs, hooks muzzle and blast firing events, analyzes the local environment around a shot, and plays additional audio tails/slapbacks using configured audio projects.

Functional systems:

- Weapon fire echo routing: `BS5_EchoDriverComponent` and modded `SCR_MuzzleEffectComponent`.
- Explosion/blast echo reuse path: `BS5_ExplosionBridge` modded `SCR_WeaponBlastComponent`.
- Environment analysis: `BS5_EchoEnvironmentAnalyzer`, `BS5_EnvironmentAudioClassifier`, `BS5_HybridTailPlanner`, `BS5_SoundMapAnchorPlanner`.
- Close-space slapbacks: `BS5_CloseReflectionPlanner` plus `BS5_CloseReflectionSettingsComponent`.
- Emission and voice limiting: `BS5_EchoEmissionService`.
- Spawned sound emitters: `BS5_SpatialSoundEmitterComponent` on `Prefabs/Props/BS5_*Emitter*.et`.
- Player settings and presets: `BS5_PlayerAudioSettings`, `BS5_PresetRegistry`, `BS5_AudioSettingsSubMenu`.
- Debug routing: `BS5_AudioDebugSettingsComponent`, `BS5_DebugLog`.

## Entry points and runtime flow

Primary weapon shot flow:

1. `Scripts/Game/BS5_EchoDriverComponent.c` mods `SCR_MuzzleEffectComponent.OnFired`.
2. The hook resolves a `BS5_EchoDriverComponent` from the effect entity, muzzle owner, or related weapon hierarchy.
3. `BS5_EchoDriverComponent.HandleWeaponFire` resolves muzzle suppression, duplicate dispatch guards, cache state, owner/effect origin, and active prefabs/settings.
4. `BS5_EchoRuntime.AnalyzeShot` delegates to `BS5_EchoEnvironmentAnalyzer.Analyze`.
5. Environment analysis calls classifier/planner logic to build tail and slapback candidates from traces, entity queries, terrain, and close-space evidence.
6. `BS5_EchoRuntime.EmitShot` / `BS5_EchoEmissionService.Emit` applies user volumes, preset limits, voice budgets, start gates, and candidate priority.
7. Emission either uses managed `SCR_SoundManagerModule` audio sources or spawns configured `BS5_*Emitter*.et` prefabs.
8. `BS5_SpatialSoundEmitterComponent.Play` sets audio signals and triggers the ACP event.

Explosion/blast flow:

1. `Scripts/Game/BS5_ExplosionBridge.c` mods `SCR_WeaponBlastComponent.OnWeaponFired`.
2. The hook calls `BS5_EchoRuntime.FindExplosionDriver`.
3. `BS5_EchoDriverComponent.HandleExplosionFire` reuses the analysis/emission path with explosion-specific radii and event/project settings where enabled.

Settings flow:

1. `BS5_AudioSettingsSubMenu` mods `SCR_AudioSettingsSubMenu` and creates procedural rows on tab show.
2. UI changes write to `BS5_PlayerAudioSettings`.
3. `BS5_PlayerAudioSettings` persists via `BS5_GameAudioSettings : ModuleGameSettings`.
4. `BS5_PresetRegistry` loads config presets from `Configs/BS5/Presets`.
5. Active preset values are read by `BS5_EchoDriverComponent.GetActiveTechnicalPreset()` and its numerous accessor methods.

## Script map

| File | Responsibility | Key classes/functions | Dependencies | Risk notes |
| --- | --- | --- | --- | --- |
| `Scripts/Game/BS5_EchoDriverComponent.c` | Main weapon echo controller and prefab-tunable settings source. | `BS5_EchoDriverComponent`, `HandleWeaponFire`, `HandleExplosionFire`, ACP/event/prefab resolvers, cache/limiter accessors, modded `SCR_MuzzleEffectComponent.OnFired`. | Weapon prefabs, muzzle components, runtime analyzer/emitter, preset registry, debug/settings components. | Very large monolith; many tunables; duplicate dispatch/caches/limiter state need focused audit. |
| `Scripts/Game/BS5_EchoRuntime.c` | Static runtime facade, environment analyzer, emission service, queueing, emitter spawn/cleanup. | `FindDriver`, `AnalyzeShot`, `EmitShot`, `BS5_EchoEnvironmentAnalyzer.Analyze`, `BS5_EchoEmissionService.Emit`, `QueueEmission`, `EmitPending`, `TryPlayManagedAudioSource`, `SpawnEmitterEntity`. | `BaseWorld`, `TraceParam`, `ChimeraCharacter.TraceMoveWithoutCharacters`, `SCR_SoundManagerModule`, emitter prefabs. | Hot path; delayed calls; entity spawning; voice limiter and cleanup correctness. |
| `Scripts/Game/BS5_EnvironmentAudioClassifier.c` | Builds terrain/environment snapshots and sound-map tail candidates. | `BS5_EnvironmentAudioClassifier.BuildSnapshot`, `BS5_HybridTailPlanner.BuildCandidates`, `BS5_SoundMapAnchorPlanner.TryBuildCandidates`. | World terrain APIs, entity queries, trace APIs, driver tunables, echo result/candidate types. | Largest trace/query surface; likely first performance audit target. |
| `Scripts/Game/BS5_CloseReflectionPlanner.c` | Close-space slapback evidence, acceptance, roof/wall rescue, candidate synthesis. | `Evaluate`, `TryAcceptDirect`, `TryAcceptRoofRescue`, `TryAcceptWallRescue`, `TraceProbeHit`, `BuildCloseCandidate`. | Echo analysis candidates, close settings, trace exclusions, driver tunables. | Math drift and extra rescue traces can affect both feel and cost. |
| `Scripts/Game/BS5_CloseReflectionSettingsComponent.c` | Prefab component exposing close-reflection tuning. | `IsEnabled`, ACP/prefab/event resolvers, score/threshold accessors. | `Weapon_Base.et`, close slapback emitter ACP/prefab. | Defaults strongly shape indoor/close behavior; audit against real test goals. |
| `Scripts/Game/BS5_SpatialSoundEmitterComponent.c` | Plays configured audio on spawned emitter entities and writes signal values. | `EOnInit`, `Play`, `TryPlayAudioSystemProject`, `BuildAudioSystemSignals`, `ResolveComponents`, `SetSignalValue`. | `SoundComponent`, `SignalsManagerComponent`, `AudioSystem`, `SCR_AudioSource`, ACP resources. | Resource invalid cache and fallback behavior can hide broken ACP references. |
| `Scripts/Game/BS5_PresetRegistry.c` | Loads sound/technical preset configs and applies presets to settings. | `BS5_SoundPresetRegistryConfig`, `BS5_TechnicalPresetRegistryConfig`, `BS5_PresetRegistry`, `ApplySoundPreset`, `ApplyTechnicalPreset`. | `Configs/BS5/Presets/*.conf`, `BS5_PlayerAudioSettings`, driver accessor pattern. | Fallback/default IDs and config drift are important audit targets. |
| `Scripts/Game/BS5_PlayerAudioSettings.c` | User-facing persisted audio settings. | `BS5_GameAudioSettings : ModuleGameSettings`, `BS5_PlayerAudioSettings`, volume/preset getters/setters, `Save`, `OnUserSettingsChanged`. | Engine game settings module, preset registry, audio settings UI. | Save timing and preset-custom marking should be checked with UI behavior. |
| `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c` | Procedural extension of audio settings UI. | `modded SCR_AudioSettingsSubMenu`, `BuildBs5AudioSettingsRows`, row builders, click/change handlers, flush/refresh methods. | UI widget classes, player settings, preset registry. | Large procedural UI file; lifecycle, handler cleanup, and text/style cloning are fragile. |
| `Scripts/Game/BS5_AudioDebugSettingsComponent.c` | Prefab debug channel toggles and debug level gating. | `BS5_AudioDebugSettingsComponent`, `IsAnyDebugEnabled`, `GetDebugLevel`, `Allows`. | `Weapon_Base.et`, debug log utility, driver. | Debug level 2 and analysis/slapback/close flags are enabled in `Weapon_Base.et`; can add runtime/editor cost. |
| `Scripts/Game/BS5_DebugLog.c` | Central debug print helpers. | `BS5_DebugLog.IsEnabled`, `Channel`, `Line`, `ChannelName`, `BoolText`. | Driver debug component and channels. | Audit for high-frequency log spam in shot path. |
| `Scripts/Game/BS5_EchoTypes.c` | Shared enums, candidate/result/context data, math helpers. | `BS5_EchoEnvironmentType`, `BS5_TailProfileType`, `BS5_EchoReflectorCandidate`, `BS5_EchoAnalysisResult`, `BS5_PendingEmissionContext`, `BS5_EchoMath`. | All core runtime scripts. | Many mutable fields; reset/default invariants matter. |
| `Scripts/Game/BS5_ExplosionBridge.c` | Small blast hook into echo driver. | `modded SCR_WeaponBlastComponent.OnWeaponFired`. | `SCR_WeaponBlastComponent`, runtime driver lookup. | MCP found the class but did not list `OnWeaponFired`; compile validation is needed before treating the callback as verified-current. |

## Script details

### `Scripts/Game/BS5_EchoDriverComponent.c`

**Does:** Owns the prefab-facing configuration and the runtime shot/explosion dispatch for weapon echoes.

**Classes:**

- `BS5_EchoDriverComponentClass : ScriptComponentClass`
- `BS5_EchoDriverComponent : ScriptComponent`
- `BS5_WeaponEchoSettingsComponentClass : ScriptComponentClass`
- `BS5_WeaponEchoSettingsComponent : ScriptComponent`
- `BS5_WeaponEchoRplCharacterComponentClass : ScriptComponentClass`
- `BS5_WeaponEchoRplCharacterComponent : ScriptComponent`
- `modded class SCR_MuzzleEffectComponent`

**Important functions:**

- `EOnInit(IEntity owner)`: caches component state and initializes runtime-facing driver data.
- `OnDelete(IEntity owner)`: cleanup path, including runtime limiter/context cancellation.
- `HandleWeaponFire(...)`: main shot entry; resolves owner, suppression, cache/dispatch guard, analysis, and emission.
- `HandleExplosionFire(...)`: explosion-specific entry using the shared analysis/emission system.
- `ResolveMasterAcp`, `ResolveSlapbackAcp`, `ResolveExplosionAcp`: select audio projects.
- `ResolveMasterEmitterPrefab`, `ResolveSlapbackEmitterPrefab`: select emitter prefab resources.
- `GetActiveTechnicalPreset` and many `Get*` accessors: merge prefab defaults with selected technical preset.
- `TryGetTailSectorCache`, `StoreTailSectorCache`, `TryGetForwardFacadeNegativeCache`: shot-analysis caching.
- `TryAcquireActiveEmitterBudget`, `ReleaseActiveEmitterBudget`: per-driver emitter budget.
- `ShouldSuppressDuplicateDispatch`, `ActivateDispatchGuard`, `ClearDispatchGuard`: duplicate event protection.
- `DebugValidateConfiguration`: validates obvious config issues when debug is enabled.

**Depends on:** `BS5_EchoRuntime`, `BS5_PresetRegistry`, `BS5_DebugLog`, `BS5_AudioDebugSettingsComponent`, `BS5_CloseReflectionSettingsComponent`, `BaseMuzzleComponent`, weapon prefabs, ACP/prefab resources.

**Used by:** Modded `SCR_MuzzleEffectComponent.OnFired`, `BS5_ExplosionBridge`, and prefab-attached weapon components.

**API/BIKI context checked:** `ScriptComponent`, `BaseMuzzleComponent`, `SoundComponent` and related sound APIs through MCP.

**Risks / audit targets:** The file is very large and mixes configuration, runtime gating, caches, duplicate suppression, and debug validation. Audit should focus on hot-path branch cost and stale resource paths before style cleanup.

### `Scripts/Game/BS5_EchoRuntime.c`

**Does:** Provides static runtime orchestration for analyzing shots/explosions and emitting audio.

**Classes:**

- `BS5_EchoRuntime`
- `BS5_EchoEnvironmentAnalyzer`
- `BS5_EchoEmissionService`

**Important functions:**

- `FindDriver`, `FindExplosionDriver`: locate driver component from effect/muzzle/projectile context.
- `AnalyzeShot`, `AnalyzeExplosion`: public analysis entry points.
- `EmitShot`, `EmitExplosion`: public emission entry points.
- `BS5_EchoEnvironmentAnalyzer.Analyze`: traces and classifies the environment.
- `IsSlapbackWallTraceAccepted`, `IsSelfHierarchyHit`: trace filtering helpers.
- `BS5_EchoEmissionService.Emit`: converts analysis candidates to pending emissions.
- `QueueEmission`, `EmitPending`, `EmitOnEmitter`: delayed and immediate playback flow.
- `TryAdmitPlaybackVoice`, `StealPlaybackVoice`, `RegisterActiveVoice`: global limiter behavior.
- `TryPlayManagedAudioSource`: managed sound manager playback.
- `SpawnEmitterEntity`, `CleanupEmitter`, `ReleaseAndCleanupEmitter`: fallback spawned emitter lifecycle.

**Depends on:** Driver tunables, echo types, classifier/planner scripts, `SCR_SoundManagerModule`, `Resource`, `BaseWorld`, `TraceParam`, emitter prefabs, ACP resources.

**Used by:** `BS5_EchoDriverComponent` and `BS5_ExplosionBridge`.

**API/BIKI context checked:** MCP confirmed `SCR_SoundManagerModule` creation/play API, `BaseWorld.TraceMove`, and `ChimeraCharacter.TraceMoveWithoutCharacters`.

**Risks / audit targets:** This is the core performance-sensitive path. Delayed calls, global static arrays, limiter world-state resets, stolen voices, and emitter cleanup should be tested under high fire rates and world transitions.

### `Scripts/Game/BS5_EnvironmentAudioClassifier.c`

**Does:** Builds an environment snapshot and plans far/medium tail candidates using terrain, city/forest/meadow scoring, sound-map samples, and path validation.

**Classes:**

- `BS5_EnvironmentAudioClassifier`
- `BS5_HybridTailPlanner`
- `BS5_SoundMapAnchorSample`
- `BS5_SoundMapAnchorPlanner : BS5_HybridTailPlanner`

**Important functions:**

- `BuildSnapshot`: collects terrain/environment scoring around a shot.
- `ResolveTerrainHeight`, `ResolveTerrainNormal`: terrain probes.
- `BuildCandidates`: creates tail candidates from the snapshot and trace context.
- `TryBuildCandidates`: sound-map anchor planner entry.

**Depends on:** `BS5_EchoDriverComponent` technical settings, `BS5_EchoAnalysisResult`, `BS5_EchoReflectorCandidate`, terrain/world APIs, trace/entity query APIs.

**Used by:** `BS5_EchoEnvironmentAnalyzer.Analyze`.

**API/BIKI context checked:** MCP confirmed `BaseWorld.TraceMove` family and `ChimeraCharacter.TraceMoveWithoutCharacters`; query APIs were identified from code but not fully MCP-expanded in this map.

**Risks / audit targets:** The file is the largest script and likely contains the most expensive per-shot operations. Audit exact trace/query counts by preset (`default`, `light`, `dynamic`) and check cache effectiveness.

### `Scripts/Game/BS5_CloseReflectionPlanner.c`

**Does:** Decides whether near-wall/room geometry supports close slapback and builds close reflection candidates.

**Classes:**

- `BS5_CloseReflectionProbeHit`
- `BS5_CloseReflectionPlannerResult`
- `BS5_CloseReflectionSupportPoint`
- `BS5_CloseReflectionEvidence`
- `BS5_CloseReflectionPlanner`

**Important functions:**

- `Evaluate`: main close-reflection planner.
- `CollectEvidence`: groups wall candidates into directional evidence.
- `TryAcceptDirect`: accepts strong side/front/back evidence directly.
- `TryAcceptRoofRescue`, `TryAcceptWallRescue`: runs extra probes to rescue ambiguous close spaces.
- `TraceProbeHit`: trace helper using `ChimeraCharacter.TraceMoveWithoutCharacters`.
- `BuildCloseCandidate`, `CreateCloseCandidate`: candidate creation and delay/intensity math.

**Depends on:** Driver settings, close reflection settings component, echo result/candidates, trace exclusions.

**Used by:** `BS5_EchoEnvironmentAnalyzer.Analyze`.

**API/BIKI context checked:** MCP confirmed `ChimeraCharacter.TraceMoveWithoutCharacters` as optimized character-filtered trace.

**Risks / audit targets:** The planner mixes heuristic scoring and extra trace probes. Good audit targets are roof/wall rescue thresholds, symmetry math, trench override interaction, and trace count under automatic fire.

### `Scripts/Game/BS5_CloseReflectionSettingsComponent.c`

**Does:** Exposes close-reflection prefab settings and fallbacks to scripts.

**Classes:**

- `BS5_CloseReflectionSettingsComponentClass : ScriptComponentClass`
- `BS5_CloseReflectionSettingsComponent : ScriptComponent`

**Important functions:** `IsEnabled`, `ResolveCloseSlapbackAcp`, `ResolveCloseSlapbackEmitterPrefab`, `ResolveCloseSlapbackEventName`, and threshold/boost getters from `GetMaxCloseDistanceMeters` through `GetSurfaceHardnessFloor`.

**Depends on:** `Prefabs/Weapons/Core/Weapon_Base.et`, close slapback ACP and emitter prefab.

**Used by:** Driver and close reflection planner.

**API/BIKI context checked:** MCP confirmed `ScriptComponent` behavior.

**Risks / audit targets:** Prefab defaults are active and materially shape close-space output. Tune only with audio test evidence.

### `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`

**Does:** Plays an audio project/event from an emitter entity and pushes runtime signal values.

**Classes:**

- `BS5_SpatialSoundEmitterComponentClass : ScriptComponentClass`
- `BS5_SpatialSoundEmitterComponent : ScriptComponent`

**Important functions:**

- `EOnInit`: resolves `SoundComponent` and `SignalsManagerComponent`.
- `Play`: applies transform, resolves event/project, sets signals, and starts playback.
- `TryPlayAudioSystemProject`: tries direct audio system playback.
- `BuildAudioSystemSignals`: creates signal name/value arrays from context.
- `ResolveComponents`, `SetSignalValue`: component/signal helpers.

**Depends on:** Emitter prefabs, `SoundComponent`, `SignalsManagerComponent`, ACP project resources, `BS5_PendingEmissionContext`.

**Used by:** `BS5_EchoEmissionService.EmitOnEmitter`.

**API/BIKI context checked:** MCP confirmed `SoundComponent.SoundEvent`, base sound signal methods, and `SignalsManagerComponent` signal APIs.

**Risks / audit targets:** Invalid project caching and fallback order may mask broken resources. Resource validation should precede logic refactors.

### `Scripts/Game/BS5_PresetRegistry.c`

**Does:** Defines config entry types and loads sound/technical presets into script-side structures.

**Classes:** `BS5_SoundPresetRegistryConfig`, `BS5_SoundPresetConfigEntry`, `BS5_TechnicalPresetRegistryConfig`, `BS5_TechnicalPresetConfigEntry`, `BS5_SoundPreset`, `BS5_TechnicalPreset`, `BS5_PresetRegistry`.

**Important functions:** `GetDefaultSoundPresetId`, `GetDefaultTechnicalPresetId`, `GetSoundPresetCount`, `GetTechnicalPresetCount`, `GetActiveTechnicalPreset`, `ApplySoundPreset`, `ApplyTechnicalPreset`, config loaders and fallback fillers.

**Depends on:** `Configs/BS5/Presets/BS5_SoundPresets.conf`, `Configs/BS5/Presets/BS5_TechnicalPresets.conf`, `BS5_PlayerAudioSettings`.

**Used by:** Driver accessors and audio settings UI.

**API/BIKI context checked:** Config resource parsing was not deeply MCP-verified in this map; file existence and class names were verified by project scan.

**Risks / audit targets:** Config and script defaults must stay in sync. A missing config should degrade predictably, but fallback behavior needs compile/runtime validation.

### `Scripts/Game/BS5_PlayerAudioSettings.c`

**Does:** Stores user-facing BS5 audio settings and selected presets.

**Classes:**

- `BS5_GameAudioSettings : ModuleGameSettings`
- `BS5_PlayerAudioSettings`

**Important functions:** `Get/SetEchoVolume`, `Get/SetSlapbackVolume`, `Get/SetSlapbackCloseVolume`, `Get/SetSlapbackEnabled`, `Get/SetTechnicalPresetId`, `Get/SetSoundPresetId`, `Save`, `EnsureInitialized`, `OnUserSettingsChanged`, `LoadFromUserSettings`.

**Depends on:** Engine `ModuleGameSettings`, preset registry, settings UI.

**Used by:** UI and emission code.

**API/BIKI context checked:** MCP confirmed `ModuleGameSettings` exists as an Enfusion settings module base.

**Risks / audit targets:** Verify that save-on-hide and external settings reload do not overwrite recent slider/button changes.

### `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`

**Does:** Adds BS5 controls to the vanilla audio settings submenu without a separate layout file.

**Classes:** `modded class SCR_AudioSettingsSubMenu`.

**Important functions:** `HandlerAttached`, `OnTabShow`, `BuildBs5AudioSettingsRows`, `OnTabHide`, `OnMenuHide`, `OnTabRemove`, `OnChange`, `OnClick`, mouse down/up handlers, row creation methods, `RemoveExistingBs5ProceduralRows`, flush/refresh methods, preset stepping, style cloning, script handler cleanup.

**Depends on:** UI widget classes, vanilla `SCR_AudioSettingsSubMenu`, player settings, preset registry.

**Used by:** Game settings UI.

**API/BIKI context checked:** MCP confirmed `SCR_AudioSettingsSubMenu` methods including `OnTabShow`, `OnTabHide`, `OnMenuHide`, and `OnTabRemove`.

**Risks / audit targets:** Procedural UI construction is long and stateful. Audit handler cleanup, duplicate row prevention, style clone assumptions, and whether all dirty settings flush in every close path.

### `Scripts/Game/BS5_AudioDebugSettingsComponent.c`

**Does:** Provides prefab-level debug channels and debug level gating.

**Classes:** `BS5_AudioDebugSettingsComponentClass`, `BS5_AudioDebugSettingsComponent`.

**Important functions:** `IsAnyDebugEnabled`, `GetDebugLevel`, `Allows`.

**Depends on:** `BS5_DebugChannel`, `BS5_DebugLevel`, `Weapon_Base.et`.

**Used by:** Driver and debug log helper.

**Risks / audit targets:** Current `Weapon_Base.et` sets `m_iDebugLevel 2`, `m_bDebugAnalysis 1`, `m_bDebugSlapback 1`, and `m_bDebugClose 1`. This is useful for testing but can be noisy and costly if shipped unintentionally.

### `Scripts/Game/BS5_DebugLog.c`

**Does:** Centralizes debug print decisions and channel labels.

**Classes:** `BS5_DebugLog`.

**Important functions:** `IsEnabled`, `Channel`, `Line`, `LineEnabled`, `ChannelEnabled`, `BoolText`, `ChannelName`.

**Depends on:** Driver/debug component.

**Risks / audit targets:** Check high-frequency debug paths before runtime performance conclusions.

### `Scripts/Game/BS5_EchoTypes.c`

**Does:** Defines shared enums, mutable context/result/candidate classes, and math helper functions.

**Classes/enums:** `BS5_EchoEnvironmentType`, `BS5_TailProfileType`, `BS5_VegetationClass`, `BS5_EchoCandidateSourceType`, `BS5_DebugChannel`, `BS5_DebugLevel`, `BS5_EchoReflectorCandidate`, `BS5_EchoAnalysisResult`, `BS5_EnvironmentSnapshot`, `BS5_PendingEmissionContext`, `BS5_ActiveEchoVoice`, `BS5_EchoMath`.

**Important functions:** constructors/defaults, `Reset`, `Clamp01`, `Clamp`, `MaxFloat`, `MinFloat`, enum-to-name helpers, `CloneCandidate`.

**Depends on:** Used by all core runtime scripts.

**Risks / audit targets:** Mutable data classes must have reliable reset/default behavior because they flow through caches, pending queues, and limiter arrays.

### `Scripts/Game/BS5_ExplosionBridge.c`

**Does:** Bridges weapon blast events into the BS5 echo driver.

**Classes:** `modded class SCR_WeaponBlastComponent`.

**Important functions:** `OnWeaponFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)`.

**Depends on:** `SCR_WeaponBlastComponent`, `BS5_EchoRuntime.FindExplosionDriver`, driver explosion handler.

**API/BIKI context checked:** MCP confirmed `SCR_WeaponBlastComponent` exists, but the search result did not expose `OnWeaponFired` in the public method list. Treat this callback signature as requiring compile/reload validation.

**Risks / audit targets:** Small file, but the override surface needs validation after any engine update.

## Prefabs/configs/layouts/resources

### Weapon prefabs

- `Prefabs/Weapons/Core/Weapon_Base.et`: main injection point. Adds `BS5_AudioDebugSettingsComponent`, `BS5_CloseReflectionSettingsComponent`, `BS5_EchoDriverComponent`, `BS5_WeaponEchoRplCharacterComponent`, and `BS5_WeaponEchoSettingsComponent`.
- `Prefabs/Weapons/Core/MachineGun_Base.et`: inherits `Weapon_Base.et`, switches to MG ACP/emitter resources, and tightens limiter/lifetime settings for automatic fire.
- `Prefabs/Weapons/Core/Handgun_Base.et`: inherits `Weapon_Base.et`.
- `Prefabs/Weapons/Core/LongRangeRifle_Base.et`: inherits a rifle base resource outside this mod path.
- `Prefabs/Weapons/MachineGuns/M60/MG_M60_base.et`: inherits `MachineGun_Base.et`.
- `Prefabs/Characters/Core/Character_Base.et`: present but minimal; no major BS5 script surface observed in this map.

### Emitter prefabs

Each emitter prefab is a `GenericEntity` with `BS5_SpatialSoundEmitterComponent`, `SoundComponent`, and `SignalsManagerComponent`.

- `Prefabs/Props/BS5_TailEmitter.et`: rifle/master tail ACP.
- `Prefabs/Props/BS5_TailEmitter_MG.et`: machine-gun tail ACP.
- `Prefabs/Props/BS5_TailEmitter_Silenced.et`: suppressed tail ACP.
- `Prefabs/Props/BS5_SlapbackEmitter.et`: normal slapback ACP.
- `Prefabs/Props/BS5_SlapbackEmitter_Close.et`: close slapback ACP.
- `Prefabs/Props/BS5_SlapbackEmitter_Silenced.et`: suppressed slapback ACP.
- `Prefabs/Props/BS5_SlapbackEmitter_Trench.et`: trench slapback ACP.
- `Prefabs/Props/BS5_CloseReverbEmitter.et`: close reverb emitter using the close slapback ACP.

### Configs

- `Configs/BS5/Presets/BS5_SoundPresets.conf`: sound presets `vanilla`, `bettersounds_v4`, `bettersounds_v5`, `lunacy_audio`.
- `Configs/BS5/Presets/BS5_TechnicalPresets.conf`: technical presets `default`, `light`, `dynamic`. These control scan radii, trace counts, candidate counts, active/pending voice budgets, limiter thresholds, sound-map planner settings, and path validation settings.

### Audio projects and signals

- ACPs live under `Sounds/Weapons/Rifles/BS5/`.
- BS5-specific signals live under `Sounds/_SharedData/Signals/BS5/`:
  - `BS5_Intensity.sig`
  - `BS5_UserSlapbackVolume.sig`
  - `BS5_UserSlapbackCloseVolume.sig`
- ACPs also reference many base-game shared weapon/reverb/occlusion signals and local wav assets under `Assets/Gunpowder/`.

### Tools and generated/support files

- `tools/*.ps1` contains local helper scripts for checking Workbench/repo/MCP state.
- `Scripts/WorkbenchGame/EnfusionMCP/` contains 20 Workbench MCP handler scripts. This may be intentional for live MCP support, but it is a publish/release hygiene risk and should be cleaned with `wb_cleanup` before release if these handlers were injected.
- `resourceDatabase.rdb`, `console.log`, and `error.log` are present at root.

No `Layouts/`, root `UI/`, `Language/`, `Worlds/`, or `Missions/` surfaces were present.

## Dependency graph

```text
addon.gproj
  -> base-game dependency 58D0FB3206B6F859

Prefabs/Weapons/Core/Weapon_Base.et
  -> BS5_AudioDebugSettingsComponent
  -> BS5_CloseReflectionSettingsComponent
  -> BS5_EchoDriverComponent
  -> BS5_WeaponEchoRplCharacterComponent
  -> BS5_WeaponEchoSettingsComponent
  -> ACP resources under Sounds/Weapons/Rifles/BS5
  -> emitter prefabs under Prefabs/Props

SCR_MuzzleEffectComponent.OnFired
  -> BS5_EchoDriverComponent.HandleWeaponFire
  -> BS5_EchoRuntime.AnalyzeShot
  -> BS5_EchoEnvironmentAnalyzer.Analyze
  -> BS5_EnvironmentAudioClassifier / BS5_HybridTailPlanner / BS5_SoundMapAnchorPlanner
  -> BS5_CloseReflectionPlanner
  -> BS5_EchoEmissionService.Emit / QueueEmission
  -> SCR_SoundManagerModule or spawned BS5_*Emitter prefab
  -> BS5_SpatialSoundEmitterComponent.Play
  -> SoundComponent / SignalsManagerComponent / AudioSystem

SCR_WeaponBlastComponent.OnWeaponFired
  -> BS5_EchoRuntime.FindExplosionDriver
  -> BS5_EchoDriverComponent.HandleExplosionFire
  -> shared analysis/emission path

SCR_AudioSettingsSubMenu
  -> BS5_AudioSettingsSubMenu procedural rows
  -> BS5_PlayerAudioSettings
  -> BS5_PresetRegistry
  -> Configs/BS5/Presets/*.conf
  -> BS5_EchoDriverComponent active technical preset accessors
```

## Enfusion API / BIKI / vanilla context checked

Checked through the active Enfusion MCP tools in this session:

- `ScriptComponent`: parent for script-created components; exposes owner/component/event-mask methods.
- `SCR_WeaponBlastComponent`: class exists; MCP result listed lifecycle methods but not `OnWeaponFired`, so the modded callback needs compile validation.
- `SCR_AudioSettingsSubMenu`: MCP confirmed menu/tab lifecycle methods used by `BS5_AudioSettingsSubMenu`.
- `ModuleGameSettings`: engine settings module base used by `BS5_GameAudioSettings`.
- `SCR_SoundManagerModule`: MCP confirmed `GetInstance`, `CreateAudioSource`, `PlayAudioSource`, and termination APIs.
- `SoundComponent`: MCP confirmed `SoundEvent` and transform/offset event methods.
- `SignalsManagerComponent`: MCP confirmed `AddOrFindSignal`, `FindSignal`, `SetSignalValue`, and MP signal APIs.
- `BaseMuzzleComponent`: MCP confirmed suppression and fire mode/muzzle methods.
- `BaseWorld.TraceMove`, `World.TraceMove`, `BaseWorld.AsyncTraceMove`, `ChimeraCharacter.TraceMoveWithoutCharacters`: MCP confirmed trace surfaces; `TraceMoveWithoutCharacters` is described as an optimization when filtering out `ChimeraCharacter` entities.

Not fully verified in this map:

- Exact `SCR_MuzzleEffectComponent.OnFired` docs were not expanded in the main pass, though the code and subagent evidence identify it as the current shot hook.
- Exact entity query APIs used in `BS5_EnvironmentAudioClassifier.c` were not individually MCP-expanded.
- ACP graph validity was checked by text reference scan, not by Workbench audio validation.

## Performance-sensitive surfaces

Most sensitive runtime files:

- `BS5_EchoRuntime.c`: per-shot analysis, delayed queues, global limiter arrays, managed audio sources, emitter spawning/cleanup.
- `BS5_EnvironmentAudioClassifier.c`: terrain and environment classification, sound-map sampling, entity queries, trace/path validation.
- `BS5_CloseReflectionPlanner.c`: close-space evidence scoring and rescue traces.
- `BS5_EchoDriverComponent.c`: cache lookup/store, dispatch guard, debug validation, preset accessor fan-out.
- `BS5_SpatialSoundEmitterComponent.c`: audio project readiness, signal writes, playback fallback.
- `BS5_AudioSettingsSubMenu.c`: not shot-hot, but lifecycle-sensitive and large enough to break settings UI.

Preset-driven cost levers:

- `m_iMaxTraceCount`
- `m_iForwardAnchorTraceCount`
- `m_iLateralAnchorTraceCount`
- `m_iSoundMapForwardRayCount`
- `m_iSoundMapForwardSampleCount`
- `m_iSoundMapOmniDirectionCount`
- `m_iSoundMapUrbanMicroMaxEntities`
- `m_iSoundMapPathSampleCount`
- `m_iMaxTailEmittersPerShot`
- `m_iMaxSlapbackEmittersPerShot`
- limiter global/pending/owner voice caps

The `light` technical preset sharply reduces these counts and is useful for performance comparison.

## AI-slop risk targets for audit

1. Resource drift:
   - ACPs reference missing `Sounds/FinalMix.afm` with GUID `{B764D803219C775E}`.
   - `Sounds/FinalMix.afm` and `.meta` were deleted in recent history.
   - `Prefabs/Weapons/Core/MachineGun_Base.et` and `Prefabs/Props/BS5_TailEmitter_MG.et` reference `{13CBB6A34DC9700D}Sounds/Weapons/Rifles/BS5/Weapons_MG_EchoMaster.acp`, while the current file is `Weapons_MG_EchoMaster.acp.acp` with GUID `{1C6E4055FD8532F6}`.
   - `Prefabs/Weapons/Core/Weapon_Base.et` legacy settings and `Prefabs/Props/BS5_TailEmitter_Silenced.et` reference `{5B784125E2E54AA0}Sounds/Weapons/Rifles/BS5/Weapons_Silinced_EchoMaster.acp`, while the current file is `Weapons_Silinced_EchoMaster.acp.acp` with GUID `{064282CFD84AFC4E}`. The active `m_sSuppressedMasterAcp` field already points to the `.acp.acp` GUID/path.
2. Debug defaults:
   - `Weapon_Base.et` currently enables debug level 2 with analysis/slapback/close channels enabled. Verify whether this is intentional for current testing before performance conclusions.
3. Hot-path complexity:
   - Driver/runtime/classifier/planner are large and trace-heavy. Avoid broad refactors; start with measured resource correctness and preset-specific trace budgets.
4. Callback validity:
   - `SCR_WeaponBlastComponent.OnWeaponFired` signature needs compile validation because MCP did not list it in the public method result for that class.
5. Workbench handler publish hygiene:
   - `Scripts/WorkbenchGame/EnfusionMCP/` should not be silently published if it is only injected tooling.
6. UI lifecycle:
   - `BS5_AudioSettingsSubMenu.c` manually builds widgets and clears handlers. Audit duplicate row prevention and save/flush order.

## Open questions / unverified assumptions

- Is `Sounds/FinalMix.afm` intentionally removed, and should ACPs be rebuilt to remove or replace the reference?
- Are `.acp.acp` filenames intentional output names, or did ACP resources get double-extension drift?
- Are legacy components `BS5_WeaponEchoRplCharacterComponent` and `BS5_WeaponEchoSettingsComponent` still required by current runtime, or only retained for prefab compatibility?
- Should debug defaults in `Weapon_Base.et` stay enabled in development branches?
- Is the explosion bridge currently compiling with `SCR_WeaponBlastComponent.OnWeaponFired`, despite MCP not listing the callback?
- Are Workbench MCP handler scripts intentionally committed in this mod, or should they be cleaned before publish/release?

## Recommended next skill

Use `v2-bhe-audit` next for a read-only audit before deslop or fixes. Suggested audit focus:

1. Resource/reference integrity: ACPs, `.meta` GUIDs, emitter prefabs, deleted `FinalMix.afm`, `.acp.acp` paths.
2. Compile/API validation for `SCR_WeaponBlastComponent.OnWeaponFired` and `SCR_MuzzleEffectComponent.OnFired`.
3. Performance review of trace/query counts in `BS5_EchoRuntime.c`, `BS5_EnvironmentAudioClassifier.c`, and `BS5_CloseReflectionPlanner.c`.
4. Debug-default and publish-hygiene review, including `Scripts/WorkbenchGame/EnfusionMCP`.

Subagent evidence:

- api_researcher / native equivalent: not used; no separate API-only subagent was necessary after direct MCP checks.
- code_researcher / native equivalent: used; purpose was script inventory and hot-path mapping; returned key facts on driver/runtime/classifier/planner/settings/debug files and API docs used.
- repo_sentinel / native equivalent: used via native explorer-style resource mapper; purpose was repo/resource inventory; returned addon identity, clean status, missing `.agent-work`, resource counts, prefab/config/audio risks, and Workbench handler hygiene risk.
- heavy_advisor / native equivalent: not used; no repeated failure, crash loop, or architecture deadlock occurred.
- files changed by subagents: none.
