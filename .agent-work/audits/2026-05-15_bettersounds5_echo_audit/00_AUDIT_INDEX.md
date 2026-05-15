# Audit index - BetterSounds5 Echo Module

Generated: 2026-05-15
Mod root: `G:\BettersMods\BettersMods\BetterSounds5_Echo_Module`
Based on map: `.agent-work/project-map.md`
Audit mode: read-only, except audit markdown artifacts

## Files produced

- `00_AUDIT_INDEX.md`
- `01_PROJECT_AUDIT.md`
- `subaudits/runtime_emission__AUDIT.md`
- `subaudits/driver_explosion__AUDIT.md`
- `subaudits/environment_close_planner__AUDIT.md`
- `subaudits/settings_ui_playback__AUDIT.md`
- `evidence/mcp_checks.md`

## Coverage table

| Sector | Files covered | Status |
| --- | --- | --- |
| Runtime and emission | `Scripts/Game/BS5_EchoRuntime.c`, `Scripts/Game/BS5_SpatialSoundEmitterComponent.c` | Covered by subagent and main-agent cross-check |
| Driver and explosion bridge | `Scripts/Game/BS5_EchoDriverComponent.c`, `Scripts/Game/BS5_ExplosionBridge.c` | Covered by subagent and main-agent cross-check |
| Environment and close planner | `Scripts/Game/BS5_EnvironmentAudioClassifier.c`, `Scripts/Game/BS5_CloseReflectionPlanner.c` | Covered by subagent and main-agent cross-check |
| Settings, presets, UI, playback helpers | `Scripts/Game/BS5_PlayerAudioSettings.c`, `Scripts/Game/BS5_PresetRegistry.c`, `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`, `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`, small shared settings/debug/type files | Covered by subagent and main-agent cross-check |
| Prefab/config/resource context | Representative BS5 weapon/driver/emitter prefabs from map plus local config references | Covered through existing map and targeted MCP/API checks; no prefab edits |

## Subagents attempted

Subagent evidence:
- api_researcher / native equivalent: not used as a separate agent; main agent performed targeted MCP API checks for query, trace, audio, trigger, projectile effect, and settings APIs.
- code_researcher / native equivalent: used; four read-only sector subaudits returned successfully; key facts were incorporated and challenged in `01_PROJECT_AUDIT.md`.
- repo_sentinel / native equivalent: not used; main agent checked git status before and after artifact creation.
- heavy_advisor / native equivalent: used; read-only synthesis returned successfully and downgraded weak findings around signal writes, invalid-resource caches, preset fallback duplication, and tail-cache cloning.
- files changed by subagents: none.

No subagent timed out. No subagent was asked to mutate files or control Workbench.

## Severity index

| ID | Severity | Short title |
| --- | --- | --- |
| F-001 | High | Sound preset/settings changes cause repeated global notifications and audio cache clears |
| F-002 | High | Close reflection ray-density formula likely collapses to integer division |
| F-003 | Medium-High | Analysis cache can reuse stale context across preset/settings changes |
| F-004 | Medium | Explosion duplicate suppression is keyed too broadly |
| F-005 | Medium | Environment analysis repeats expensive terrain/query/trace work |
| F-006 | Medium | Close/trench policy is split between runtime and planner with hardcoded pre-reject thresholds |
| F-007 | Medium | Emitter and audio invalid caches have weak invalidation semantics |
| F-008 | Low-Medium | Guard and debug-validation gaps around runtime and explosion resources |
| F-009 | Low-Medium | Procedural settings UI is brittle and has one unused helper |
| F-010 | Low | Duplicate signal writes are a profiling candidate, not a first-pass bug |

## How to continue with `v2-bhe-deslop`

Recommended first slice:

1. Batch `BS5_PlayerAudioSettings` writes so sound preset changes and slider drags do not repeatedly fire `UserSettingsChanged()` and clear audio project caches.
2. Fix `BS5_CloseReflectionPlanner.ResolveRayDensityScore()` to use float division and add short debug proof for density, trench score, close score, and selected mode.
3. Tighten explosion dedupe and analysis-cache keys with runtime debug output before changing broader planner logic.

Keep dual signal writes and tail-sector cache cloning out of the first cleanup unless profiling or audio regression testing proves they are harmful.
