# Subaudit - Runtime hot path

## Scope

Read-only audit of the weapon and explosion echo runtime path.

## Files inspected

- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- `Scripts/Game/BS5_CloseReflectionPlanner.c`
- `Scripts/Game/BS5_EchoTypes.c`
- `Scripts/Game/BS5_ExplosionBridge.c`
- `Scripts/Game/BS5_AudioDebugSettingsComponent.c`
- `Scripts/Game/BS5_DebugLog.c`

## MAP context used

`SCR_MuzzleEffectComponent.OnFired` and `SCR_WeaponBlastComponent.OnWeaponFired` both route into `BS5_EchoDriverComponent`. The driver calls `BS5_EchoRuntime`, which analyzes the shot through environment/classifier/close-reflection code and then emits tails and slapbacks.

## Enfusion MCP checks

- `SCR_MuzzleEffectComponent.OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)` exists and matches the modded override.
- `SCR_WeaponBlastComponent.OnWeaponFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)` exists and matches `BS5_ExplosionBridge`.
- `ChimeraCharacter.TraceMoveWithoutCharacters(BaseWorld, inout TraceParam)` exists and is documented as an optimized trace variant for filtering characters.
- `BaseWorld.QueryEntitiesBySphere` / `World.QueryEntitiesBySphere` exist.
- `SCR_SoundManagerModule.CreateAudioSource`, `SCR_AudioSourceConfiguration.IsValid`, `SCR_AudioSource.Terminate`, and `AudioSystem.TerminateSoundFadeOut` exist.
- `wb_knowledge` identifies `TraceMove` and `QueryEntitiesBySphere` as performance-sensitive operations.

## Functional summary

The runtime is functional and API-grounded, but it concentrates a lot of local state in one driver: cache provenance, duplicate dispatch suppression, preset accessors, limiter state, and debug validation. The shot analysis path also performs overlapping environment classification before running several expensive terrain, trace, and entity query passes.

## Findings

### A-001: Top-level analysis cache does not key on `explosionLike`

- Severity: High
- Category: lifecycle / cache correctness
- Evidence: `BS5_EchoDriverComponent.ResolveCachedResult(..., bool explosionLike, bool suppressed)` only uses `explosionLike` to block reuse when explosion reuse is disabled, while `StoreCachedResult` stores only `suppressed`; `HandleExplosionFire` stores explosion analysis into the same cache used by `HandleWeaponFire`.
- Why this is AI-slop / risk: Lower-level tail-sector and forward-facade caches already track explosion provenance, so the top-level omission is inconsistent and can reuse blast analysis for later weapon shots or vice versa.
- API/BIKI/base-game verification: The involved shot and blast callbacks are MCP-verified; this is local cache logic, not an API uncertainty.
- Behavior risk if changed: Properly separating shot and explosion caches may reduce reuse in alternating blast/shot sequences and increase analysis work slightly.
- Cleanup direction: Add a stored `m_bLastExplosionLike` field and compare it in `ResolveCachedResult`, or split shot and explosion caches.

### A-002: Duplicate dispatch guard is shot-only and uses a brittle 10 ms timer

- Severity: Medium
- Category: lifecycle / duplicate event path
- Evidence: `HandleWeaponFire` calls `ShouldSuppressDuplicateDispatch` and later `ActivateDispatchGuard`; `HandleExplosionFire` does not use the guard. `ActivateDispatchGuard` clears via `CallLater(ClearDispatchGuard, 10, false)`.
- Why this is AI-slop / risk: The same owner/origin can arrive through two verified engine callbacks, but only one path participates in suppression. A hardcoded 10 ms guard is narrower than a full frame at low FPS and is not a stable event-generation boundary.
- API/BIKI/base-game verification: Both callbacks are MCP-verified.
- Behavior risk if changed: A wider or shared guard can suppress some edge-case rapid events, so it should be keyed to event provenance and reuse windows rather than blindly enlarged.
- Cleanup direction: Share duplicate suppression between weapon and explosion dispatch paths and key it on a stable generation/timestamp window.

### A-003: Environment analysis recomputes similar state before expensive query fan-out

- Severity: Medium
- Category: double computation / performance
- Evidence: `BS5_EchoEnvironmentAnalyzer.Analyze` computes open/indoor/forest/hill/water scores, then calls `BS5_EnvironmentAudioClassifier.BuildSnapshot`, which recomputes overlapping weights and terrain bias. The classifier then performs terrain samples, `QueryEntitiesBySphere`, `QueryEntitiesByBeveledLine`, and `TraceMoveWithoutCharacters` in follow-up paths.
- Why this is AI-slop / risk: The same shot environment is described in multiple systems before being blended back together. This raises per-shot cost and makes tuning drift likely.
- API/BIKI/base-game verification: The trace/query APIs are valid; `wb_knowledge` marks them performance-sensitive.
- Behavior risk if changed: Consolidation can shift audio feel because several heuristics blend overlapping scores.
- Cleanup direction: Build one per-shot environment snapshot and have both the analyzer and planner consume it.

## Duplicate/overlapping logic

- `BS5_EchoRuntime.Analyze` and `BS5_EnvironmentAudioClassifier.BuildSnapshot` both classify the same environment.
- `ComputeDistanceGain` and `ComputeSlapbackDistanceGain` use parallel attenuation shapes.
- `ResolvePairScore` and `ResolveRescuePairScore` in the close planner share a similar weighted reducer shape.

## Dead or unreachable code candidates

- `BS5_EchoMath.EnvironmentId(...)` has no call sites in the inspected tree.
- `BS5_CloseReflectionPlannerResult.m_fCornerPairScore` and `m_iRescueRayCount` appear diagnostic-only.

## Performance hotspots

- Near-trace fan-out in `BS5_EchoEnvironmentAnalyzer.Analyze`.
- Terrain/signal sampling in `BS5_EnvironmentAudioClassifier.BuildSnapshot`.
- Forward facade and slapback entity query passes.
- Close-reflection rescue traces.
- Emission queue start-gate and voice-admission paths.

## API uncertainty / required follow-up

No suspicious API misuse was found in this sector. Base-game script bodies were not available through guessed `game_read` paths, so compile/reload validation remains useful after implementation changes.

## Sanity notes

No files were mutated. Debug spam is gated by `m_bEnableDebug`, which defaults off in `BS5_AudioDebugSettingsComponent`; verbose internal flags only matter when that master switch is enabled.
