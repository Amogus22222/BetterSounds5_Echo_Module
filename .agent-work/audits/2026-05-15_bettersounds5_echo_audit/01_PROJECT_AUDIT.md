# Project anti-slop audit - BetterSounds5 Echo Module

Generated: 2026-05-15
Mod root: `G:\BettersMods\BettersMods\BetterSounds5_Echo_Module`
Based on map: `.agent-work/project-map.md`
Audit mode: read-only, except audit markdown artifacts

## Executive summary

`BetterSounds5_Echo_Module` is functionally coherent, but the runtime carries classic additive AI-slop: broad dedupe heuristics, stale caches without settings generation, repeated global settings invalidation, and expensive analysis passes stacked around every shot.

No confirmed wrong core Enfusion API call was found in the main audio/query/trace paths. The highest-confidence bug is local math: close reflection ray density likely uses integer division. The highest-value cleanup is settings batching, because one sound preset action can trigger multiple global `UserSettingsChanged()` calls and repeated audio cache clears.

This audit does not recommend a broad rewrite. The safest next cleanup is a few narrow behavior-preserving slices with debug proof: batch settings notifications, fix the ray-density formula, tighten explosion dedupe keys, and add cache generation/owner context.

## Scope and files covered

- Runtime and emission: `Scripts/Game/BS5_EchoRuntime.c`, `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
- Driver and hooks: `Scripts/Game/BS5_EchoDriverComponent.c`, `Scripts/Game/BS5_ExplosionBridge.c`
- Environment and close planner: `Scripts/Game/BS5_EnvironmentAudioClassifier.c`, `Scripts/Game/BS5_CloseReflectionPlanner.c`
- Settings and UI: `Scripts/Game/BS5_PlayerAudioSettings.c`, `Scripts/Game/BS5_PresetRegistry.c`, `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`
- Shared small files: `BS5_EchoTypes.c`, `BS5_DebugLog.c`, `BS5_AudioDebugSettingsComponent.c`, `BS5_CloseReflectionSettingsComponent.c`
- Prefab/config context from `.agent-work/project-map.md` and targeted representative checks

## Subagent/advisor evidence

Subagent evidence:
- api_researcher / native equivalent: not used as separate agent; main agent performed MCP checks for API surfaces.
- code_researcher / native equivalent: used; four read-only sector subaudits returned successfully and covered all meaningful script sectors.
- repo_sentinel / native equivalent: not used; main agent checked status/diff.
- heavy_advisor / native equivalent: used; read-only synthesis returned successfully and downgraded weak findings.
- files changed by subagents: none.

Advisor downgraded these points:

- Duplicate signal writes are not automatically wrong because the code explicitly comments that direct `SoundComponent` writes support same-frame visibility.
- Invalid resource caches are mostly Workbench/hot-reload risk unless runtime resource availability is changing.
- Preset fallback duplication is currently coherent; stale ID canonicalization is the concrete issue.
- Tail sector cache cloning is plausible cost but should not be removed until candidate mutation is audited.
- `GetTrenchOverrideMargin()` is not dead; the issue is that planner pre-reject logic bypasses the authored margin used later by runtime.

## Severity index

| ID | Severity | Category | Short title |
| --- | --- | --- | --- |
| F-001 | High | lifecycle / duplicated work | Sound preset/settings changes cause repeated global notifications and audio cache clears |
| F-002 | High | math / wrong formula | Close reflection ray-density formula likely collapses to integer division |
| F-003 | Medium-High | cache correctness | Analysis cache can reuse stale context across preset/settings changes |
| F-004 | Medium | duplicate suppression | Explosion dedupe is keyed too broadly |
| F-005 | Medium | performance | Environment analysis repeats expensive terrain/query/trace work |
| F-006 | Medium | formula / threshold drift | Close/trench policy is split between runtime and planner |
| F-007 | Medium-Low | resource cache | Emitter/audio invalid caches have weak invalidation semantics |
| F-008 | Low-Medium | lifecycle / diagnostics | Guard and debug-validation gaps around runtime and explosion resources |
| F-009 | Low-Medium | dead code / UI contract | Procedural settings UI is brittle and has one unused helper |
| F-010 | Low | performance | Duplicate signal writes need profiling before cleanup |

## Top cleanup opportunities

1. Batch settings writes and reduce cache invalidation.
2. Fix close ray density float math and add short debug proof.
3. Tighten explosion dedupe keys while preserving multi-hook dedupe for one event.
4. Add cache generation/owner identity to analysis cache validity.
5. Add low-risk guards and debug validation for explosion-specific resources.

## Findings

### F-001: Sound preset/settings changes cause repeated global notifications and audio cache clears

- Severity: High
- Category: lifecycle / duplicated work
- Files/classes/functions: `BS5_PresetRegistry.ApplySoundPreset`, `BS5_PlayerAudioSettings.Set*`, `BS5_PlayerAudioSettings.OnUserSettingsChanged`, `BS5_AudioSettingsSubMenu` slider handlers
- Evidence: `Scripts/Game/BS5_PresetRegistry.c:448-459` calls five setters for one preset. Setters call `game.UserSettingsChanged()` at `Scripts/Game/BS5_PlayerAudioSettings.c:78`, `113`, `148`, `189`, `245`, and `278`. The settings-change callback clears audio project caches at `Scripts/Game/BS5_PlayerAudioSettings.c:306-310`. Slider handlers call setters while changing values at `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c:1046-1207`.
- MCP/API/BIKI/base-game verification: MCP confirmed the settings APIs exist. The issue is call frequency and invalidation scope, not invalid API.
- Why this is AI-slop: individual setters are reused as a batch API, so one user action fans out into repeated global notifications and repeated cache invalidations.
- Runtime/performance/behavior impact: unnecessary work during menu interaction and preset changes; possible audio cache churn while the player is adjusting volumes.
- Recommended cleanup direction: implement a batch settings write path with one notification and one optional save. Do not clear ACP validity caches for pure volume changes unless runtime testing proves it is needed.
- Dependencies / risk: verify whether live slider preview relies on immediate global settings writes.
- Related subaudits: `settings_ui_playback__AUDIT.md`

### F-002: Close reflection ray-density formula likely collapses to integer division

- Severity: High
- Category: math / wrong formula
- Files/classes/functions: `BS5_CloseReflectionPlanner.ResolveRayDensityScore`
- Evidence: `Scripts/Game/BS5_CloseReflectionPlanner.c:259-265` returns `BS5_EchoMath.Clamp01(wallHitCount / rayCount)` with both operands declared as `int`.
- MCP/API/BIKI/base-game verification: local Enforce math; no engine API dependency.
- Why this is AI-slop: the function returns `float` and feeds continuous scoring, but the operands make the expression easy to accidentally evaluate as integer division.
- Runtime/performance/behavior impact: `m_fRayDensityScore` can become binary, weakening close reflection acceptance for partial hit coverage.
- Recommended cleanup direction: cast to float and validate with representative ratios in debug output.
- Dependencies / risk: low code risk; audible behavior should still be checked in close/urban/trench scenes.
- Related subaudits: `environment_close_planner__AUDIT.md`

### F-003: Analysis cache can reuse stale context across preset/settings changes

- Severity: Medium-High
- Category: cache correctness / lifecycle
- Files/classes/functions: `BS5_EchoDriverComponent.ResolveCachedResult`, `StoreCachedResult`, technical preset getters
- Evidence: `Scripts/Game/BS5_EchoDriverComponent.c:2128-2151` validates cache by origin, heading, suppressed flag, and explosion flag. The `owner` parameter is unused. `StoreCachedResult()` only schedules time-based invalidation at `2165-2183`. Active technical preset values are read dynamically by driver getters, for example around `905+` and `1090+`.
- MCP/API/BIKI/base-game verification: local cache policy, not API mismatch.
- Why this is AI-slop: derived analysis state can survive a settings/preset transition because the cache has no settings generation or owner identity.
- Runtime/performance/behavior impact: shots/explosions shortly after preset changes can reuse analysis based on previous scan radius, trace counts, or environment planner settings.
- Recommended cleanup direction: key cache validity on owner/root and a BS5 settings generation; invalidate driver caches when technical preset changes.
- Dependencies / risk: too much invalidation can increase hot-path cost. Add cache-hit logs before changing behavior broadly.
- Related subaudits: `driver_explosion__AUDIT.md`

### F-004: Explosion duplicate suppression is keyed too broadly

- Severity: Medium
- Category: duplicate suppression / risky side effect
- Files/classes/functions: `BS5_EchoRuntime.DispatchExplosionEffect`, `ShouldSuppressExplosionDispatch`, `BS5_ExplosionBridge`
- Evidence: `Scripts/Game/BS5_EchoRuntime.c:120-140` suppresses dispatches within two frames and `DistanceSq < 4.0` using only one last origin/frame. `Scripts/Game/BS5_ExplosionBridge.c:1-37` has multiple hook sources that make dedupe necessary.
- MCP/API/BIKI/base-game verification: `BaseWorld.GetFrameNumber()` exists. The risk is the heuristic key.
- Why this is AI-slop: the dedupe is downstream and source-blind, even though the dispatch function receives owner, hit entity, damage source, instigator, and source tag context.
- Runtime/performance/behavior impact: legitimate nearby chain detonations can lose echo output.
- Recommended cleanup direction: use a short recent-dispatch list keyed by source identity, instigator/damage source, source tag, and origin bucket.
- Dependencies / risk: must still suppress duplicate hook fan-in for a single explosion.
- Related subaudits: `runtime_emission__AUDIT.md`, `driver_explosion__AUDIT.md`

### F-005: Environment analysis repeats expensive terrain/query/trace work

- Severity: Medium
- Category: performance / duplicated work
- Files/classes/functions: `BS5_EnvironmentAudioClassifier`, `BS5_HybridTailPlanner`, SoundMap planners
- Evidence: `Scripts/Game/BS5_EnvironmentAudioClassifier.c:330-384`, `617-680`, `2650-2745`, `2748-2780`, and later SoundMap/fallback paths all perform staged work from the same origin/view basis.
- MCP/API/BIKI/base-game verification: query and trace APIs are valid. MCP did not find an API misuse here.
- Why this is AI-slop: the planner layers multiple fallback and confirmation systems without enough staged gating or shared cached terrain basis.
- Runtime/performance/behavior impact: possible shot-time hitches in dense urban scenes or when SoundMap and facade passes both run.
- Recommended cleanup direction: first add counters and debug timing; only collapse passes proven redundant.
- Dependencies / risk: high behavior risk if rewritten broadly.
- Related subaudits: `environment_close_planner__AUDIT.md`

### F-006: Close/trench policy is split between runtime and planner

- Severity: Medium
- Category: formula / threshold drift
- Files/classes/functions: `BS5_CloseReflectionPlanner.ShouldRejectForTrench`, `BS5_EchoRuntime` close/trench dominance block, `BS5_CloseReflectionSettingsComponent.GetTrenchOverrideMargin`
- Evidence: planner pre-reject thresholds are hardcoded at `Scripts/Game/BS5_CloseReflectionPlanner.c:287-305`. Runtime later uses `GetTrenchOverrideMargin()` at `Scripts/Game/BS5_EchoRuntime.c:1036-1040`.
- MCP/API/BIKI/base-game verification: local audio mode policy.
- Why this is AI-slop: two separate systems decide trench dominance; one respects authored margin and one bypasses it.
- Runtime/performance/behavior impact: prefab tuning cannot fully control close-vs-trench behavior.
- Recommended cleanup direction: align planner pre-reject with the authored margin or explicitly document it as a stricter early gate.
- Dependencies / risk: medium. Needs audio validation across trench, indoor, and close wall cases.
- Related subaudits: `environment_close_planner__AUDIT.md`

### F-007: Emitter and audio invalid caches have weak invalidation semantics

- Severity: Medium-Low
- Category: resource cache / lifecycle
- Files/classes/functions: `BS5_EchoEmissionService.ResolveEmitterPrefabResource`, `BS5_SpatialSoundEmitterComponent.TryPlayAudioSystemProject`
- Evidence: emitter resource failures are cached in `Scripts/Game/BS5_EchoRuntime.c:3531-3571` with no reset path. Audio project/event failures are cached in `Scripts/Game/BS5_SpatialSoundEmitterComponent.c:87-160` and cleared by settings changes.
- MCP/API/BIKI/base-game verification: `Resource.Load`, `Resource.IsValid`, and `AudioSystem.PlayEvent` are valid APIs.
- Why this is AI-slop: failure caches are useful, but their lifetime does not distinguish permanent bad resource paths from temporary Workbench/hot-reload failures.
- Runtime/performance/behavior impact: mostly Workbench iteration risk; runtime resources are normally static.
- Recommended cleanup direction: add explicit debug/cache reset or expiry. Do not treat every volume setting change as audio resource validity change.
- Dependencies / risk: low if reset is explicit.
- Related subaudits: `runtime_emission__AUDIT.md`, `settings_ui_playback__AUDIT.md`

### F-008: Guard and debug-validation gaps around runtime and explosion resources

- Severity: Low-Medium
- Category: lifecycle / diagnostics
- Files/classes/functions: `BS5_EchoEnvironmentAnalyzer.Analyze`, `BS5_EchoDriverComponent.GetExplosionIntensityMultiplier`, `DebugValidateConfiguration`
- Evidence: `Scripts/Game/BS5_EchoRuntime.c:322-328` calls `GetGame().GetWorld()` without guarding `GetGame()`. `Scripts/Game/BS5_EchoDriverComponent.c:952` returns raw explosion intensity multiplier. Debug validation covers normal tail/slapback resources but not all explosion-specific ACP/prefab/event fields.
- MCP/API/BIKI/base-game verification: Game/world APIs and audio resource APIs exist; null guarantee in this exact callback path was not proven.
- Why this is AI-slop: neighboring code is more defensive and neighboring getters clamp authoring values.
- Runtime/performance/behavior impact: low in normal runtime; useful hardening and diagnostics for Workbench and bad authoring.
- Recommended cleanup direction: guard `GetGame()` before world/callqueue use, clamp explosion intensity like other multipliers, and include explosion resource fields in debug validation.
- Dependencies / risk: low.
- Related subaudits: `runtime_emission__AUDIT.md`, `driver_explosion__AUDIT.md`

### F-009: Procedural settings UI is brittle and has one unused helper

- Severity: Low-Medium
- Category: dead code / UI contract
- Files/classes/functions: `BS5_AudioSettingsSubMenu`
- Evidence: `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c:214-216` defines `HandleBs5ClickAction()` but actual click flow uses `ExecuteBs5ClickAction()` directly. Widget discovery relies on `Content`, first slider row, and text scanning.
- MCP/API/BIKI/base-game verification: lifecycle hooks were treated as valid; vanilla widget internals were not fully inspected.
- Why this is AI-slop: leftover wrapper is clear dead scaffolding; broader UI anchor is heuristic.
- Runtime/performance/behavior impact: menu fragility, not gameplay hot-path risk.
- Recommended cleanup direction: remove unused wrapper in a narrow cleanup; defer broader UI anchoring until UI work is in scope.
- Dependencies / risk: low for wrapper removal, medium for broad UI changes.
- Related subaudits: `settings_ui_playback__AUDIT.md`

### F-010: Duplicate signal writes need profiling before cleanup

- Severity: Low
- Category: performance / duplicate work
- Files/classes/functions: `BS5_SpatialSoundEmitterComponent.BuildAudioSystemSignals`, `ApplyAudioSignals`, `SetSignalValue`
- Evidence: `Scripts/Game/BS5_SpatialSoundEmitterComponent.c:294-320` writes signal values through `SignalsManagerComponent` and directly through `SoundComponent.SetSignalValueStr()`.
- MCP/API/BIKI/base-game verification: both signal APIs exist; docs imply manager signals feed sound components, but same-frame propagation timing was not proven.
- Why this is AI-slop: duplicate work is present, but the local comment says the direct write is intentional for same-frame playback.
- Runtime/performance/behavior impact: possible per-emission overhead.
- Recommended cleanup direction: keep as-is until profiler/audio tests prove it is safe to reduce.
- Dependencies / risk: medium audio regression risk.
- Related subaudits: `runtime_emission__AUDIT.md`, `settings_ui_playback__AUDIT.md`

## Duplicate computations and overlapping systems

- Preset apply duplicates individual setter side effects rather than using a batch path.
- Explosion fan-in duplicates event paths and relies on broad downstream suppression.
- Environment analysis stacks SoundWorld, terrain, facade, fallback, and path validation passes.
- Close planner and runtime both participate in trench-vs-close dominance.
- Runtime voice priority and kill-score logic contain overlapping distance/intensity judgement.

## Dead/unreachable/stale code candidates

- `BS5_AudioSettingsSubMenu.HandleBs5ClickAction()` is unused.
- `BS5_EchoDriverComponent.ResolveCachedResult(IEntity owner, ...)` does not use `owner`.
- `BS5_EchoDriverComponent.HandleExplosionFire(... projectileEntity)` does not use `projectileEntity` after lookup.
- A few runtime fields/branches are used mostly for logging/debug and should be reviewed only after functional cleanup.

## Incorrect or suspicious API usage

No confirmed wrong core API signatures were found for the checked query, trace, audio, settings, trigger, and projectile effect paths.

Suspicious or unverified:

- `AudioSystem.PlayEvent()` `-1` permanent failure semantics are not proven enough to justify session-permanent invalid-event caches without a reset path.
- `GetGame()` non-null is assumed in some runtime helpers.
- UI widget hierarchy assumptions were not verified against vanilla source in this audit.

## Lifecycle/event/timer issues

- Settings changes and slider drags trigger global settings notifications too often.
- Analysis caches lack settings generation and owner identity.
- `OnDelete()` cleanup in `BS5_EchoDriverComponent` appears coherent and was not flagged as leaking timers.
- Resource invalid caches lack explicit reset paths.

## Network/replication/authority issues

No concrete server/client authority bug was proven. The mod appears primarily client-local audio behavior, but explosion and muzzle hook entry points should be runtime-tested in multiplayer before changing dedupe semantics.

## Performance hotspots

- Repeated `UserSettingsChanged()` and audio cache clears during UI/settings work.
- Environment analysis query/terrain/trace fan-out.
- Per-emission signal array construction and writes.
- Managed audio source configuration allocation.
- Linear active voice and cache array scans.
- Debug string construction when debug channels are enabled.

## Formula/math/multiplier anomalies

- `ResolveRayDensityScore()` likely integer-divides.
- `GetExplosionIntensityMultiplier()` is raw/unclamped unlike neighboring getters.
- Close/trench thresholds are split between hardcoded planner pre-reject and authored runtime override margin.
- Rescue trace caps should be checked against configured close radius.

## Architecture simplification candidates

- Batch settings writer API for preset changes and UI flushes.
- Recent explosion dispatch key instead of one global last-origin pair.
- Settings generation value used by driver analysis cache.
- Shared or logged terrain basis for planner validation instead of blind repeated sampling.

## Findings rejected or downgraded

- Duplicate signal writes: downgraded to profiling candidate because same-frame sound graph visibility may be intentional.
- Tail sector cache cloning: not treated as a cleanup target until downstream candidate mutation is audited.
- Preset fallback duplication: not a bug while IDs/values remain coherent; stale ID canonicalization is the actionable piece.
- `GetTrenchOverrideMargin()` dead config: rejected. It is used by runtime; the issue is planner pre-reject drift.
- Hook super ordering: not elevated without base method body or runtime evidence.

## Open questions

- Does slider movement intentionally preview audio volume live, or should it only persist on final change?
- Can one real explosion enter through multiple hook paths with different source tags but same origin in adjacent frames?
- Is `SoundComponent.SetSignalValueStr()` required for same-frame signal visibility before `PlayStr()`?
- Should Workbench/resource reload clear emitter prefab invalid caches?
- What runtime logs are acceptable during close/trench debug without causing audio-frame cost?

## Recommended `v2-bhe-deslop` plan seed

1. First slice: settings batching and cache invalidation scope.
   - Add a batch writer or suppression flag in `BS5_PlayerAudioSettings`.
   - Make `ApplySoundPreset()` notify once.
   - Stop clearing audio project caches for pure volume changes unless verified necessary.
   - Validation: count `UserSettingsChanged()` and `ClearAudioProjectCaches()` during preset apply and slider drag.

2. Second slice: close planner correctness.
   - Fix ray-density float division.
   - Add temporary concise debug for ray count, density, trench score, close score, and final mode.
   - Align planner trench pre-reject with authored margin or document it as stricter.
   - Validation: urban, indoor, trench, open test shots.

3. Third slice: runtime cache/dedupe hardening.
   - Add owner/root and settings generation to analysis cache.
   - Replace one-entry explosion dedupe with a small recent-key list.
   - Add debug proof for duplicate hook collapse versus nearby distinct explosions.
   - Validation: one explosive triggering multiple hooks still dedupes; two nearby blasts both emit.

4. Fourth slice: low-risk hardening.
   - Guard `GetGame()` before world/callqueue dereferences.
   - Clamp explosion intensity.
   - Extend debug validation to explosion resources.
   - Leave dual signal writes and tail-sector clone behavior unchanged unless profiling proves them harmful.
