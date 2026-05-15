# Deslop implementation plan - BetterSounds5 Echo Module

Generated: 2026-05-15
Mod root: `G:\BettersMods\BettersMods\BetterSounds5_Echo_Module`

Based on:
- `.agent-work/project-map.md`
- `.agent-work/audits/2026-05-15_bs5-echo-module-audit/01_PROJECT_AUDIT.md`
- `.agent-work/audits/2026-05-15_bs5-echo-module-audit/subaudits/runtime_hot_path__AUDIT.md`
- `.agent-work/audits/2026-05-15_bs5-echo-module-audit/subaudits/settings_ui_emitters__AUDIT.md`
- `.agent-work/audits/2026-05-15_bs5-echo-module-audit/subaudits/repo_resource_hygiene__AUDIT.md`
- `.agent-work/audits/2026-05-15_bs5-echo-module-audit/subaudits/api_verification__AUDIT.md`

## Goal

Implement the safest audit-backed cleanup slices without feature drift: prevent the top-level echo analysis cache from reusing weapon-shot analysis for explosion playback, and remove duplicated emitter signal payload construction.

## Non-goals

- Do not rebuild or rename ACP resources in this batch.
- Do not remove tracked Workbench MCP handlers.
- Do not rewrite the environment classifier or close-reflection planner.
- Do not change gameplay tuning values, audio gains, preset values, or emitter resource paths.

## Safety constraints

- Keep edits scoped to `Scripts/Game/BS5_EchoDriverComponent.c` and `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`.
- Preserve existing cache position, heading, suppression, TTL, and reuse-window behavior.
- Use current MCP-verified callback signatures as-is.
- Run local validation before considering heavier Workbench actions.

## Backup plan

Before editing any script, create:

`.agent-work/backups/2026-05-15_bs5-echo-cache-cleanup_scripts_backup.zip`

The archive must include all project `.c` scripts, `.agent-work/project-map.md`, and the audit files used for this plan.

## Validation plan

- Static reference check for cache fields and cache store/resolve call sites.
- `git diff --check`.
- `mod validate` through Enfusion MCP for structure/scripts/prefabs/configs/references/naming.
- Inspect final `git diff --stat` and `git status --short --untracked-files=all`.

## Findings selected

- F-003 / runtime A-001: top-level analysis cache does not key on `explosionLike`.
- F-007 / settings-emitter A-002: emitter signal payload is duplicated between playback backends.

## Findings rejected/deferred

- F-001 / F-002 resource drift: deferred until Workbench canonical ACP identity is verified. The audit explicitly warns against blind bulk replacement.
- F-004 duplicate dispatch guard: deferred because it changes event suppression semantics and needs launcher/blast smoke testing.
- F-005 environment recomputation: deferred as a larger performance refactor with audio-feel risk.
- F-006 preset defaults: implemented in Batch 3 after user requested this follow-up slice.
- F-008 invalid audio negative cache: implemented in Batch 3 with user-settings-triggered cache clear.
- F-009 Workbench MCP handlers: deferred pending release/publish policy.
- F-010 UI cleanup stubs: implemented in Batch 3; live UI smoke test remains follow-up validation.

## Batch plan

### Batch 1 - Cache provenance for shot vs explosion analysis

- Audit findings: F-003, runtime subaudit A-001.
- Files/functions: `Scripts/Game/BS5_EchoDriverComponent.c`; cache fields, `EOnInit`, `HandleWeaponFire`, `HandleExplosionFire`, `ResolveCachedResult`, `StoreCachedResult`.
- Problem: the cache stores result, origin, forward, and suppressed state, but not whether the result came from explosion analysis.
- MCP/API/BIKI checks completed: current MCP confirms `SCR_MuzzleEffectComponent.OnFired(IEntity, BaseMuzzleComponent, IEntity)` and `SCR_WeaponBlastComponent.OnWeaponFired(IEntity, BaseMuzzleComponent, IEntity)`.
- Intended edits: add `m_bLastExplosionLike`, initialize/reset it, compare it in `ResolveCachedResult`, and pass the current context into `StoreCachedResult`.
- Behavior to preserve: regular shot reuse stays available for nearby regular shots; explosion reuse stays governed by `m_bEnableExplosionReuse`; suppressed/non-suppressed shot separation stays unchanged.
- Validation: static grep, `git diff --check`, Enfusion MCP `mod validate`.
- Rollback notes: restore the backup zip or revert only `Scripts/Game/BS5_EchoDriverComponent.c`.
- Risk: alternating shot/blast sequences may analyze more often, which is the intended correctness tradeoff.

### Batch 2 - Shared emitter signal payload

- Audit findings: F-007, settings/emitter subaudit A-002.
- Files/functions: `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`; `Play`, `TryPlayAudioSystemProject`, `BuildAudioSystemSignals`, `SetSignalValue`.
- Problem: the same signal names and values are manually written to `SoundComponent` and rebuilt separately for direct `AudioSystem` playback.
- MCP/API/BIKI checks completed: active audit/MCP evidence confirms `SoundComponent`, `SignalsManagerComponent`, and `AudioSystem.PlayEvent` signal APIs.
- Intended edits: build one signal name/value payload per emission, apply it to the component path, and pass the same payload to direct project playback.
- Behavior to preserve: exact signal names, values, and ordering used by existing ACP graphs.
- Validation: static grep, `git diff --check`, Enfusion MCP `mod validate`, Workbench script reload.
- Rollback notes: restore the backup zip or revert only `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`.
- Risk: compile/runtime mismatch if Enforce parameter passing differs for arrays; validation must catch this before continuing.

### Batch 3 - Settings defaults, invalid audio cache, and UI close cleanup

- Audit findings: F-006, F-008, F-010.
- Files/functions: `BS5_PlayerAudioSettings`, `BS5_PresetRegistry`, `BS5_SpatialSoundEmitterComponent`, `BS5_AudioSettingsSubMenu`.
- Problem: sound defaults drifted from live config, invalid ACP/event caches had no reset path, and UI close hooks repeated the same flush logic while carrying unused helpers.
- MCP/API/BIKI checks completed: `ModuleGameSettings`, `SCR_AudioSettingsSubMenu`, `AudioSystem.PlayEventInitialize`, and `AudioSystem.PlayEvent` verified.
- Intended edits: align script defaults/fallbacks with current config values, clear audio project caches on user settings changes, consolidate UI close flushes, remove unused UI stubs.
- Behavior to preserve: no audio resource path changes; same close-hook flush order; same menu lifecycle hooks.
- Validation: static grep, `git diff --check`, Enfusion MCP `mod validate`, Workbench script reload.
- Rollback notes: restore `.agent-work/backups/2026-05-15_bs5-settings-emitter-ui_fix_scripts_backup.zip` or revert only the Batch 3 files.
- Risk: actual settings menu persistence still needs a live UI smoke test.

## Execution checklist

- [x] Audit/map evidence loaded.
- [x] MCP callback signatures checked.
- [x] Backup archive created and verified.
- [x] Batch 1 implemented.
- [x] Batch 2 implemented.
- [x] Validation completed.
- [x] Implementation log written.
