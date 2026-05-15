# Subaudit - Environment and close planner

## Scope

`Scripts/Game/BS5_EnvironmentAudioClassifier.c` and `Scripts/Game/BS5_CloseReflectionPlanner.c`.

## Files inspected

- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- `Scripts/Game/BS5_CloseReflectionPlanner.c`
- Supporting reference: `Scripts/Game/BS5_CloseReflectionSettingsComponent.c`

## MAP context used

The map identifies this sector as trace/entity-query/terrain/signal heavy. The audit focused on duplicated work, expensive query fan-out, close reflection formula correctness, and trench/close decision drift.

## Enfusion MCP checks

MCP confirmed the trace/query APIs used here: `QueryEntitiesBySphere`, `QueryEntitiesByBeveledLine`, `SoundWorld.GetMapValuesAtPos`, `SCR_TerrainHelper.GetHeightAboveTerrain`, and `ChimeraCharacter.TraceMoveWithoutCharacters`.

## Functional summary

The classifier builds environmental snapshots and reflector candidates from SoundWorld samples, terrain samples, entity queries, and traces. The close planner takes wall/trench candidates and decides whether to emit close-space slapback, trench slapback, or no close reflection.

## Findings

### A-001: Ray-density score likely uses integer division

- Severity: High
- Category: math / wrong formula
- Evidence: `Scripts/Game/BS5_CloseReflectionPlanner.c:259-265` computes `wallHitCount / rayCount` with integer parameters.
- Why this is AI-slop / risk: the intended density is continuous, but integer division can collapse values such as `1/2` or `2/3` to `0` before `Clamp01`.
- API/BIKI/base-game verification: no engine API issue; this is local Enforce math risk.
- Behavior risk if changed: low. The intended output is already a float; the cleanup should make the formula match its declared type.
- Cleanup direction: cast one operand to float, then validate with logs for `wallHitCount`, `rayCount`, density, close score, trench score, and final mode.

### A-002: Close/trench policy is split and partly hardcoded

- Severity: Medium
- Category: formula / threshold drift
- Evidence: `Scripts/Game/BS5_CloseReflectionPlanner.c:287-305` pre-rejects trench-like cases with hardcoded `0.58`, `0.60`, and `0.50` thresholds. `Scripts/Game/BS5_EchoRuntime.c:1036-1040` later uses `GetTrenchOverrideMargin()`.
- Why this is AI-slop / risk: two separate layers decide trench dominance. Runtime respects authored override margin, but planner can reject close reflection before that margin participates.
- API/BIKI/base-game verification: local tuning logic, not an API mismatch.
- Behavior risk if changed: medium. Trench and close-space modes are audible behavior, so changes need scenario logs.
- Cleanup direction: make the pre-reject path use the same authored margin/policy as the runtime dominance check, or document and log why it must be stricter.

### A-003: Environment analysis repeats expensive terrain/query/trace work

- Severity: Medium
- Category: performance / duplicated work
- Evidence: `Scripts/Game/BS5_EnvironmentAudioClassifier.c:330-384`, `617-680`, `2650-2745`, `2748-2780`, and `2782+` build multiple candidate/query passes from the same origin/view basis.
- Why this is AI-slop / risk: the hot shot path stacks terrain samples, SoundWorld samples, entity queries, and traces in several phases. Some overlap is intentional, but the shape is additive and hard to reason about.
- API/BIKI/base-game verification: MCP confirmed the query APIs are valid; the risk is cost and repeated work, not invalid API.
- Behavior risk if changed: high if rewritten broadly. Candidate selection is core audio behavior.
- Cleanup direction: do not rewrite first. Add counters and staged gates, then collapse only proven duplicate passes.

### A-004: Rescue traces are capped below the configured close radius

- Severity: Medium-Low
- Category: config drift / formula
- Evidence: `TryAcceptRoofRescue` and `TryAcceptWallRescue` are rooted at `Scripts/Game/BS5_CloseReflectionPlanner.c:414` and `466`; the subaudit flagged rescue trace caps around `Clamp(maxCloseDistance * 1.05, 1.2, 4.8)` while close settings can allow a larger effective radius.
- Why this is AI-slop / risk: authored max close distance and rescue trace reach can diverge, making upper-radius close pockets impossible to confirm through rescue.
- API/BIKI/base-game verification: local formula issue.
- Behavior risk if changed: medium; larger rescue traces can increase false positives.
- Cleanup direction: log actual max close distance and rescue trace distance before changing caps.

## Duplicate/overlapping logic

- Terrain basis, terrain profile validation, and path plausibility repeat related terrain sampling.
- Forward facade, forward fallback, urban micro, and SoundMap candidates overlap around the same view basis.
- Close direct acceptance, roof rescue, and wall rescue recompute from shared evidence.

## Dead or unreachable code candidates

No confirmed dead planner methods. `GetTrenchOverrideMargin()` is not dead; it is used in runtime dominance logic. The issue is split policy, not unused config.

## Performance hotspots

- Forward facade entity queries.
- SoundMap forward sampling loops.
- Per-candidate terrain/path plausibility checks.
- Close rescue trace branches.

## API uncertainty / required follow-up

No confirmed wrong API call. Main follow-up is behavioral validation around trace exclusion consistency and rescue caps.

## Sanity notes

The most actionable low-risk fix in this sector is float division in `ResolveRayDensityScore()`.
