# Subaudit - Runtime and emission

## Scope

`Scripts/Game/BS5_EchoRuntime.c` and direct emitter interaction with `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`.

## Files inspected

- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
- Supporting context: `Scripts/Game/BS5_ExplosionBridge.c`, `Scripts/Game/BS5_EnvironmentAudioClassifier.c`, `Scripts/Game/BS5_EchoTypes.c`, `Scripts/Game/BS5_EchoDriverComponent.c`

## MAP context used

The map identifies runtime/emission as the main hot path: driver lookup, shot/explosion analysis, emission service, limiter/voice stealing, resource/emitter fallback, and duplicate explosion suppression.

## Enfusion MCP checks

MCP confirmed `AudioSystem`, `SoundComponent`, `SignalsManagerComponent`, `SCR_SoundManagerModule`, `ScriptCallQueue`, `Game.SpawnEntityPrefab`, `ChimeraGame.SpawnEntityPrefabLocal`, and world entity APIs used by this sector.

## Functional summary

Runtime dispatches shot and explosion events into environment analysis, then queues emission contexts. The emission service reserves pending and playback voices, applies start gates, tries managed audio for tails, spawns emitter prefabs when needed, and cleans up active voices and emitters.

## Findings

### A-001: Explosion dedupe is broader than the hook fan-in requires

- Severity: Medium
- Category: duplicate suppression / risky side effect
- Evidence: `Scripts/Game/BS5_EchoRuntime.c:30-49` calls suppression before dispatch; `120-140` tracks only last frame and origin. `Scripts/Game/BS5_ExplosionBridge.c:15-36` has multiple hook sources that justify dedupe but not source-blind dedupe.
- Why this is AI-slop / risk: source-blind origin/frame suppression can confuse duplicate hook fan-in with distinct nearby explosions.
- API/BIKI/base-game verification: `GetFrameNumber()` exists; finding is local heuristic risk.
- Behavior risk if changed: medium-high.
- Cleanup direction: maintain a small recent-dispatch list keyed by source identity plus origin bucket.

### A-002: `Analyze()` uses `GetGame().GetWorld()` without a guard

- Severity: Low-Medium
- Category: lifecycle hardening
- Evidence: `Scripts/Game/BS5_EchoRuntime.c:322-328` dereferences `GetGame()` directly, while other runtime paths guard `GetGame()` before use.
- Why this is AI-slop / risk: most runtime callbacks likely have `GetGame()`, so this is not proven crashy, but the guard style is inconsistent.
- API/BIKI/base-game verification: Game/world APIs exist; guarantee of non-null in this exact path was not proven.
- Behavior risk if changed: low.
- Cleanup direction: guard `Game game = GetGame()` before `game.GetWorld()`.

### A-003: Emitter invalid-resource cache has no reset path

- Severity: Medium-Low
- Category: resource cache / Workbench iteration risk
- Evidence: `Scripts/Game/BS5_EchoRuntime.c:3531-3571` stores failed emitter prefab names in `s_aInvalidEmitterResourceNames`; no reset path was found. Audio project caches can be cleared by `BS5_PlayerAudioSettings.OnUserSettingsChanged()`, but emitter resource caches cannot.
- Why this is AI-slop / risk: a transient Workbench/hot-reload/resource rebuild failure can blackhole a prefab for the session.
- API/BIKI/base-game verification: `Resource.Load()` and `Resource.IsValid()` are valid.
- Behavior risk if changed: low if reset is explicit.
- Cleanup direction: add an explicit cache reset/debug command or time-bound invalid entries.

### A-004: Duplicate signal writes are a profiler candidate, not a proven bug

- Severity: Low
- Category: performance / duplicate work
- Evidence: `Scripts/Game/BS5_SpatialSoundEmitterComponent.c:294-320` writes each signal to `SignalsManagerComponent` and `SoundComponent.SetSignalValueStr()`. The code comment at `316-317` says direct write is for same-frame visibility.
- Why this is AI-slop / risk: duplicated work exists, but it may be an intentional engine workaround.
- API/BIKI/base-game verification: both APIs exist; signal wiki implies manager signals feed sound components, but same-frame timing was not proven.
- Behavior risk if changed: medium for audio regression.
- Cleanup direction: do not remove in first pass. Add profiling/debug toggle or test same-frame behavior first.

## Duplicate/overlapping logic

- `ComputeVoicePriority()` and `ComputeVoiceKillScore()` repeat distance/intensity judgement.
- Managed audio and prefab emitter paths both build/push the same signal payload.
- Distance gain functions use similar curves for tail and slapback.

## Dead or unreachable code candidates

- Local `right` in runtime analysis is computed but unused.
- Some limiter fields are written mainly for logging/debug visibility rather than control flow.
- `GetSlapbackModeSignal()` has a branch returning the same value as default for wall slapbacks.

## Performance hotspots

- Per-analysis arrays and `TraceParam` allocations.
- Per-emission `SCR_AudioSourceConfiguration` and signal arrays.
- Linear scans of active voice, pending voice, and resource cache arrays.
- Debug string construction when debug channels are enabled.

## API uncertainty / required follow-up

Runtime validation is needed before changing explosion dedupe or signal write paths.

## Sanity notes

Prefab topology from the map and subagent inspection matches expected runtime structure: emitter prefabs have `BS5_SpatialSoundEmitterComponent`, `SoundComponent`, and `SignalsManagerComponent`.
