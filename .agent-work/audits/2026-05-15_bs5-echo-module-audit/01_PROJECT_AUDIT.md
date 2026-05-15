# Project anti-slop audit - BetterSounds5 Echo Module

Generated: 2026-05-15
Mod root: `G:\BettersMods\BettersMods\BetterSounds5_Echo_Module`
Based on map: `.agent-work/project-map.md`
Audit mode: read-only, except writing this audit artifact

## Executive summary

BetterSounds5 is API-grounded in the major places checked: the muzzle/blast hooks, audio playback APIs, settings module, UI lifecycle hooks, and trace/query APIs all exist in the active Enfusion MCP index. The main cleanup risk is therefore not guessed API syntax. It is stale resource references, cache provenance gaps, duplicated tuning/default data, and expensive overlapping shot-analysis work.

Top risks:

1. Missing and drifted ACP resources can break or mask playback.
2. The top-level analysis cache can reuse explosion analysis for regular shots, or regular shot analysis for explosions.
3. The shot environment is classified in overlapping passes before expensive trace/entity-query fan-out.
4. Preset/default values and emitter signal payloads have multiple sources of truth.
5. Tracked Workbench MCP handler scripts and local scratch files need publish policy.

## Scope and files covered

Scripts covered:

- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- `Scripts/Game/BS5_CloseReflectionPlanner.c`
- `Scripts/Game/BS5_EchoTypes.c`
- `Scripts/Game/BS5_ExplosionBridge.c`
- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
- `Scripts/Game/BS5_CloseReflectionSettingsComponent.c`
- `Scripts/Game/BS5_AudioDebugSettingsComponent.c`
- `Scripts/Game/BS5_DebugLog.c`
- `Scripts/Game/BS5_PlayerAudioSettings.c`
- `Scripts/Game/BS5_PresetRegistry.c`
- `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`

Resource/config/prefab surfaces sampled:

- `addon.gproj`
- `Configs/BS5/Presets/*.conf`
- `Prefabs/Weapons/Core/Weapon_Base.et`
- `Prefabs/Weapons/Core/MachineGun_Base.et`
- `Prefabs/Props/BS5_*Emitter*.et`
- `Sounds/Weapons/Rifles/BS5/*.acp*`
- `Scripts/WorkbenchGame/EnfusionMCP/*`

## Subagent/advisor evidence

Subagent evidence:

- api_researcher / native equivalent: used; verified fire/blast callbacks, audio APIs, settings APIs, UI lifecycle methods, and trace/query APIs; no major API misuse found.
- code_researcher / native equivalent: used; runtime sector found cache provenance, duplicate-dispatch, and overlapping analysis/query cost issues.
- code_researcher / native equivalent: used; settings/UI/emitter sector found duplicated defaults, duplicated signal payloads, permanent invalid caches, and UI cleanup stubs.
- repo_sentinel / native equivalent: used; found ACP/resource drift, missing `FinalMix.afm`, tracked Workbench MCP handlers, and scratch/log hygiene issues.
- heavy_advisor / native equivalent: not used; no repeated failure, crash loop, or architecture deadlock required it.
- files changed by subagents: none.

Advisor synthesis: main-agent synthesis only, using the returned subaudits and direct source/MCP checks.

## Severity index

| ID | Severity | Area | Title |
| --- | --- | --- | --- |
| F-001 | High | Resources | Missing `Sounds/FinalMix.afm` remains referenced by ACP graphs |
| F-002 | High | Resources | `.acp` / `.acp.acp` drift in machinegun and silenced echo paths |
| F-003 | High | Runtime cache | Top-level analysis cache does not key on `explosionLike` |
| F-004 | Medium | Runtime lifecycle | Dispatch suppression is shot-only and time-brittle |
| F-005 | Medium | Performance | Environment analysis recomputes overlapping data before expensive queries |
| F-006 | Medium | Data ownership | Preset/default values are split across config and script fallbacks |
| F-007 | Medium | Emitter playback | Signal payload is duplicated between playback backends |
| F-008 | Medium | Emitter recovery | Static invalid ACP/event caches can hide fixed resources until restart |
| F-009 | Medium | Publish hygiene | Tracked Workbench MCP handler scripts are release risk |
| F-010 | Low | UI lifecycle | Procedural settings UI has cleanup stubs and repeated close logic |

## Top cleanup opportunities

1. Fix resource identity and missing mix dependencies first. This is the least speculative and can unblock meaningful runtime tests.
2. Add `explosionLike` provenance to top-level cached analysis.
3. Make duplicate dispatch suppression consistent between weapon and explosion paths.
4. Collapse environment snapshot ownership enough to avoid recomputing the same shot context.
5. Centralize preset defaults and emitter signal payload construction.

## Findings

### F-001: Missing `Sounds/FinalMix.afm` remains referenced by ACP graphs

- Severity: High
- Category: resource drift / stale reference
- Files/classes/functions: `Sounds/Weapons/Rifles/BS5/*.acp*`
- Evidence: `Sounds/FinalMix.afm` is absent from the mod tree, while ACP files still reference `{B764D803219C775E}Sounds/FinalMix.afm`. Confirmed in rifle, MG, silenced, close, trench, normal, and silenced slapback ACPs.
- MCP/API/BIKI/base-game verification: BI audio docs confirm ACP graphs depend on valid referenced resources.
- Why this is AI-slop: The graph contains deleted/unregistered resource references that were not reconciled after asset changes.
- Runtime/performance/behavior impact: Audio project initialization/playback can fail or trigger fallback paths.
- Recommended cleanup direction: Restore/register the mix resource or rebuild/rewire every ACP graph that references it.
- Dependencies / risk: Requires Workbench/audio validation; changing final mix routing can affect sound output.
- Related subaudits: `repo_resource_hygiene__AUDIT.md`

### F-002: `.acp` / `.acp.acp` drift in machinegun and silenced echo paths

- Severity: High
- Category: resource drift / config mismatch
- Files/classes/functions: `Prefabs/Weapons/Core/MachineGun_Base.et`, `Prefabs/Props/BS5_TailEmitter_MG.et`, `Prefabs/Props/BS5_TailEmitter_Silenced.et`, `Prefabs/Weapons/Core/Weapon_Base.et`, `BS5_EchoDriverComponent.MACHINEGUN_MASTER_ACP`
- Evidence: Machinegun prefabs/script constants point at `Weapons_MG_EchoMaster.acp`, while the checked-in resource meta names `Weapons_MG_EchoMaster.acp.acp`. Silenced fields mix `Weapons_Silinced_EchoMaster.acp.acp` and stale `.acp` paths.
- MCP/API/BIKI/base-game verification: Runtime audio APIs require valid resource names; `.meta` files show current registered resource names.
- Why this is AI-slop: One logical asset has multiple path identities across script constants, prefab fields, and emitter sound components.
- Runtime/performance/behavior impact: Some weapon variants may fail direct playback, fall back to prefab `SoundComponent`, or be permanently blacklisted by invalid caches.
- Recommended cleanup direction: Decide canonical resource identities in Workbench, then update references file by file.
- Dependencies / risk: Do not bulk replace until Workbench resource GUID/path truth is confirmed.
- Related subaudits: `repo_resource_hygiene__AUDIT.md`, `settings_ui_emitters__AUDIT.md`

### F-003: Top-level analysis cache does not key on `explosionLike`

- Severity: High
- Category: lifecycle / cache correctness
- Files/classes/functions: `BS5_EchoDriverComponent.ResolveCachedResult`, `StoreCachedResult`, `HandleWeaponFire`, `HandleExplosionFire`
- Evidence: The cache stores suppressed state, origin, forward, and result, but not whether the result came from explosion analysis. `HandleExplosionFire` stores into the same cache that `HandleWeaponFire` can later reuse.
- MCP/API/BIKI/base-game verification: Fire and blast callbacks are MCP-confirmed; this is local cache logic.
- Why this is AI-slop: Lower-level caches already track explosion provenance, so the top-level cache is inconsistent with the surrounding design.
- Runtime/performance/behavior impact: Explosion candidate analysis can bias later shot emissions, or shot analysis can bias explosion playback.
- Recommended cleanup direction: Store and compare an `explosionLike` cache field, or split shot and explosion caches.
- Dependencies / risk: Slightly more analysis work in alternating shot/blast sequences.
- Related subaudits: `runtime_hot_path__AUDIT.md`

### F-004: Dispatch suppression is shot-only and time-brittle

- Severity: Medium
- Category: lifecycle / duplicate event path
- Files/classes/functions: `BS5_EchoDriverComponent.HandleWeaponFire`, `HandleExplosionFire`, `ActivateDispatchGuard`
- Evidence: Weapon fire checks `ShouldSuppressDuplicateDispatch`; explosion fire does not. Guard reset is a hardcoded 10 ms `CallLater`.
- MCP/API/BIKI/base-game verification: `SCR_MuzzleEffectComponent.OnFired` and `SCR_WeaponBlastComponent.OnWeaponFired` are both MCP-confirmed.
- Why this is AI-slop: Two event paths converge on one driver, but duplicate suppression only covers one path.
- Runtime/performance/behavior impact: Duplicate or asymmetric echo playback can occur depending on callback order and frame timing.
- Recommended cleanup direction: Share suppression between shot and explosion paths and key it on a stable event/reuse window.
- Dependencies / risk: Need live test for launcher/blast behavior.
- Related subaudits: `runtime_hot_path__AUDIT.md`

### F-005: Environment analysis recomputes overlapping data before expensive queries

- Severity: Medium
- Category: double computation / performance
- Files/classes/functions: `BS5_EchoEnvironmentAnalyzer.Analyze`, `BS5_EnvironmentAudioClassifier.BuildSnapshot`, `CollectForwardFacadeEntityCandidates`, `EvaluateSoundMapPathPlausibility`
- Evidence: Analyzer computes environment scores and then calls the classifier snapshot, which recomputes similar weights and terrain bias. Follow-up paths use terrain samples, sphere/beveled-line entity queries, and trace validation.
- MCP/API/BIKI/base-game verification: Query/trace APIs are real and `wb_knowledge` flags them as performance-sensitive.
- Why this is AI-slop: The same environment semantics are owned in two places and blended together after redundant work.
- Runtime/performance/behavior impact: High per-shot cost under automatic fire; tuning drift between overlapping classifiers.
- Recommended cleanup direction: Create one per-shot environment snapshot and feed both analysis and candidate planning from it.
- Dependencies / risk: Needs before/after audio feel and performance testing.
- Related subaudits: `runtime_hot_path__AUDIT.md`

### F-006: Preset/default values are split across config and script fallbacks

- Severity: Medium
- Category: duplicated state / data drift
- Files/classes/functions: `BS5_PlayerAudioSettings`, `BS5_PresetRegistry`, `Configs/BS5/Presets/*.conf`
- Evidence: Player defaults repeat `0.65 / 0.4 / 0.4`; fallback `vanilla` preset repeats the same; live config `vanilla` uses `0.75 / 0.6 / 0.8`.
- MCP/API/BIKI/base-game verification: `ModuleGameSettings` usage is valid.
- Why this is AI-slop: Multiple sources of truth already disagree.
- Runtime/performance/behavior impact: Missing config or bootstrap state can make UI values disagree with selected preset labels.
- Recommended cleanup direction: Make config canonical and make fallback values explicit emergency fallback only.
- Dependencies / risk: Settings migration behavior should be checked.
- Related subaudits: `settings_ui_emitters__AUDIT.md`

### F-007: Signal payload is duplicated between playback backends

- Severity: Medium
- Category: double computation / architecture
- Files/classes/functions: `BS5_SpatialSoundEmitterComponent.Play`, `BuildAudioSystemSignals`, `SetSignalValue`
- Evidence: The component writes signals directly for `SoundComponent` playback and separately reconstructs the same payload for `AudioSystem.PlayEvent`.
- MCP/API/BIKI/base-game verification: Signal and audio APIs are real.
- Why this is AI-slop: Same semantic payload exists in two code paths and can drift.
- Runtime/performance/behavior impact: Extra per-emission work and possible backend mismatch.
- Recommended cleanup direction: Build one payload and reuse it for both paths; cache stable signal lookups where appropriate.
- Dependencies / risk: Must preserve exact signal names used by ACP graphs.
- Related subaudits: `settings_ui_emitters__AUDIT.md`

### F-008: Static invalid ACP/event caches can hide fixed resources until restart

- Severity: Medium
- Category: risky side effect / invalid cache
- Files/classes/functions: `BS5_SpatialSoundEmitterComponent.EnsureAudioProjectReady`, `IsAudioProjectEventKnownInvalid`, `MarkAudioProjectEventInvalid`
- Evidence: Failed project initialization or event playback is stored in static invalid arrays with no invalidation path.
- MCP/API/BIKI/base-game verification: `AudioSystem.PlayEventInitialize` and `PlayEvent` are MCP-confirmed.
- Why this is AI-slop: Permanent negative caching makes Workbench/audio iteration misleading.
- Runtime/performance/behavior impact: A resource fixed during the same session may still fail until restart.
- Recommended cleanup direction: Add cache clear on reload/settings refresh, time-limit negative cache entries, or add debug cache clear.
- Dependencies / risk: Avoid retry spam for truly broken resources.
- Related subaudits: `settings_ui_emitters__AUDIT.md`

### F-009: Tracked Workbench MCP handler scripts are release risk

- Severity: Medium
- Category: repo hygiene / publish hygiene
- Files/classes/functions: `Scripts/WorkbenchGame/EnfusionMCP/*`
- Evidence: `git ls-files` shows 20 tracked Workbench MCP handler scripts in the mod.
- MCP/API/BIKI/base-game verification: Local AGENTS policy says injected handler scripts should be cleaned before publish/release hygiene when not intended as shipped content.
- Why this is AI-slop: Tooling support files are mixed into publishable mod script tree.
- Runtime/performance/behavior impact: May ship local tooling and increase script surface.
- Recommended cleanup direction: Decide whether these are intentionally versioned; if not, remove from publish scope and use `wb_cleanup` after Workbench work.
- Dependencies / risk: Removing them can break current MCP Workbench control until reinjected.
- Related subaudits: `repo_resource_hygiene__AUDIT.md`

### F-010: Procedural settings UI has cleanup stubs and repeated close logic

- Severity: Low
- Category: lifecycle / dead code
- Files/classes/functions: `BS5_AudioSettingsSubMenu.TryCloneBs5EchoVolumeRow`, `ClearBs5ScriptHandlers`, `OnTabHide`, `OnMenuHide`, `OnTabRemove`
- Evidence: `TryCloneBs5EchoVolumeRow` always returns false, `ClearBs5ScriptHandlers` is not called, and the same flush sequence repeats in three close hooks.
- MCP/API/BIKI/base-game verification: UI lifecycle overrides are MCP-confirmed.
- Why this is AI-slop: Dead helpers imply cleanup that does not happen and complicate future UI work.
- Runtime/performance/behavior impact: Mostly maintenance risk; possible stale handler/ref issues if the menu is recreated.
- Recommended cleanup direction: Wire or delete cleanup helpers, and consolidate close flushing into one helper.
- Dependencies / risk: Requires UI smoke test.
- Related subaudits: `settings_ui_emitters__AUDIT.md`

## Duplicate computations and overlapping systems

- Runtime environment classification is split between `BS5_EchoEnvironmentAnalyzer` and `BS5_EnvironmentAudioClassifier`.
- Preset values exist in config resources and script fallback builders.
- Player setting defaults exist in attributes, static fields, and load fallback.
- Emitter signal payloads exist in direct `SoundComponent` and `AudioSystem` paths.
- UI close flushing is repeated across three overrides.

## Dead/unreachable/stale code candidates

- `BS5_EchoMath.EnvironmentId(...)` appears uncalled.
- `TryCloneBs5EchoVolumeRow()` is a stub.
- `ClearBs5ScriptHandlers()` is defined but not called.
- No-op `HandlerAttached()` override.
- `BS5_CloseReflectionPlannerResult.m_fCornerPairScore` and `m_iRescueRayCount` appear diagnostic-only.

## Incorrect or suspicious API usage

No confirmed wrong API usage in the checked major surfaces. The previous `SCR_WeaponBlastComponent.OnWeaponFired` uncertainty is downgraded because MCP method search confirmed the callback.

## Lifecycle/event/timer issues

- Top-level cache lacks `explosionLike` provenance.
- Dispatch guard is shot-only and hardcoded to 10 ms.
- Static invalid audio caches have no invalidation path.
- Procedural UI cleanup helpers are not wired.

## Network/replication/authority issues

No confirmed network or replication bug found in this audit. The legacy `BS5_WeaponEchoRplCharacterComponent` acts as a resource holder in inspected code, not an active replication system.

## Performance hotspots

- Near-trace fan-out in shot analysis.
- Terrain and sound-map sampling in classifier snapshot.
- Forward facade and slapback entity queries.
- Path plausibility raycasts.
- Close-reflection rescue traces.
- Per-emission signal payload assembly and signal lookup/writes.
- Debug logs when the master debug switch is enabled.

## Formula/math/multiplier anomalies

- No confirmed double-applied audio gain formula was found.
- Attenuation and pair-score helpers have duplicated shapes and may be cleanup candidates after resource/caching fixes.
- The `light` technical preset uses `m_fSoundMapPathRaycastDistanceMeters 999`, which effectively shifts raycast gating behavior. This may be intentional as a cheap preset lever, but it should be documented during deslop.

## Architecture simplification candidates

- Extract one canonical shot environment snapshot.
- Extract one signal payload builder for all playback backends.
- Reduce or isolate fallback preset tables.
- Separate resource identity resolution from runtime emission logic.
- Keep UI lifecycle cleanup explicit and minimal.

## Findings rejected or downgraded

- `SCR_MuzzleEffectComponent.OnFired` signature: verified correct.
- `SCR_WeaponBlastComponent.OnWeaponFired` signature: verified correct despite sparse class summary.
- Main audio playback APIs: verified real.
- `ModuleGameSettings` and `SCR_AudioSettingsSubMenu` lifecycle APIs: verified real.
- Debug defaults: internal debug flags are set in `Weapon_Base.et`, but `m_bEnableDebug` defaults false, so debug spam is not active unless explicitly enabled.

## Open questions

- Should `.acp.acp` filenames be canonical, or are they accidental double-extension resources that should be renamed in Workbench?
- Should `Sounds/FinalMix.afm` be restored, or should ACPs be rebuilt without it?
- Are `Scripts/WorkbenchGame/EnfusionMCP/*` intentionally versioned for this mod, or only local tool injection?
- Should explosion echo reuse share the regular shot cache at all, or should it have a separate cache?
- Does UI tab recreation actually leave stale handlers in live Workbench/game UI?

## Recommended `v2-bhe-deslop` plan seed

1. Resource integrity slice:
   - Verify canonical ACP resources in Workbench.
   - Fix `FinalMix.afm` references.
   - Normalize MG and silenced ACP paths across scripts, prefabs, and emitter `SoundComponent`s.
   - Validate with Workbench/resource/audio smoke tests.

2. Runtime correctness slice:
   - Add `explosionLike` to top-level analysis cache provenance.
   - Share or explicitly separate duplicate dispatch suppression for weapon and explosion paths.
   - Compile/reload and test weapon fire plus blast behavior.

3. Performance/cleanup slice:
   - Consolidate environment snapshot ownership.
   - Reduce duplicated entity/query work where behavior can be preserved.
   - Add instrumentation or debug counters before changing tuning.

4. Settings/emitter/UI slice:
   - Centralize preset/default values.
   - Share emitter signal payload construction.
   - Add invalid audio cache invalidation.
   - Remove or wire UI cleanup stubs.

5. Repo hygiene slice:
   - Decide policy for tracked Workbench MCP handlers and root logs.
   - Keep `.agent-work` artifacts out of publish commits unless explicitly requested.
