# BetterSounds5 anti-slop audit

Date: 2026-05-15
Scope: `G:\BettersMods\BettersMods\BetterSounds5_Echo_Module`
Mode: read-only gameplay audit; no gameplay scripts, prefabs, configs, or resources were edited.
Input map: `.agent-work/project-map.md`

## Executive verdict

Verdict: fix-before-ship.

The project is functional in shape, but it is not clean enough for publish or deslop handoff without a few targeted cleanup batches. Highest risk is not missing generic audio plumbing; it is debug-enabled prefabs, explosion lifecycle dedupe that can hide real clustered detonations, unproven static explosive coverage, and heavy SoundMap/urban scan stacks that need measurable guardrails before more tuning.

## Bootstrap evidence

- Active mod root: `G:\BettersMods\BettersMods\BetterSounds5_Echo_Module`
- Project file: `addon.gproj`
- Addon ID/GUID/title: `BetterSounds5` / `6717325A0F4513E2` / `BetterSounds5`
- Dependency: `58D0FB3206B6F859`
- Existing map used: `.agent-work/project-map.md`
- Gameplay edit status: no gameplay files changed by this audit.

## Findings

### A-1: Debug is enabled on weapon and explosion driver prefabs

Severity: high
Confidence: high
Cleanup risk: low
Surface: prefab / publish hygiene

Evidence:
- `Prefabs/Weapons/Core/Weapon_Base.et:5-7` has `Enabled 1`, `m_bEnableDebug 1`, `m_iDebugLevel 2`.
- `Prefabs/Props/BS5_ExplosionDriver.et:5-7` has `Enabled 1`, `m_bEnableDebug 1`, `m_iDebugLevel 2`.
- MCP merged prefab inspect confirmed both components are active and verbose in the current merged view.

MCP/API/base-game verification:
- Verified with `prefab(action=inspect)` for `Prefabs/Weapons/Core/Weapon_Base.et`.
- Verified with `prefab(action=inspect)` for `Prefabs/Props/BS5_ExplosionDriver.et`.

Why it matters:
- This can ship verbose driver/analysis/slapback/close/emitter/limiter/SoundMap logs on normal weapon and explosion paths.
- Several raw `PrintFormat()` calls are still inside debug-channel-gated sections, so enabled debug can become runtime noise under actual gameplay.

Suggested cleanup:
- Native prefab-attribute cleanup only: turn debug off in both prefabs, preserve the debug component and channels.
- Keep one explicit local/debug preset path if needed, but do not publish verbose defaults.

Validation needed:
- MCP merged prefab inspect after edit.
- `mod validate` with prefab/script checks.
- One runtime smoke test to confirm no normal `[BS5]` spam with default published prefabs.

### A-2: Explosion dedupe keys only on frame and origin

Severity: high
Confidence: medium-high
Cleanup risk: medium
Surface: scripts / runtime lifecycle

Evidence:
- `Scripts/Game/BS5_EchoRuntime.c:30-82` fans all explosion-effect dispatch into one `DispatchExplosionEffect(...)`.
- `Scripts/Game/BS5_EchoRuntime.c:120-139` suppresses duplicate explosions when frame delta is <= 2 and origin distance squared is < 4.
- `Scripts/Game/BS5_ExplosionBridge.c:13-38` has multiple dispatch sources: explosive trigger, pressure trigger, and `BS5_ExplosionEchoEffect.OnEffect(...)`.

MCP/API/base-game verification:
- MCP verified `BaseProjectileEffect.OnEffect(IEntity, inout vector[3], IEntity, Instigator, string, float)`.
- MCP verified `BaseTriggerComponent.TriggeredInSafetyDistance(...)` inherited by explosive/pressure trigger components.
- MCP verified the current bridge hook signatures are real.

Why it matters:
- The dedupe probably protects against double fan-in from trigger/effect paths, but it cannot distinguish a duplicate callback from two legitimate detonations close together in the same 2-frame window.
- This is especially risky for mines, charges, cluster-like cases, and rapid explosive chains.

Suggested cleanup:
- Tighten dedupe to include source identity where available: source tag, owner/damage source/entity identity, and maybe effect component/container source.
- Keep lifecycle separation: ordinary shot echo, fire-time launcher blast callback, and actual detonation/impact explosion must remain separate.

Validation needed:
- Runtime logs for at least three cases: rocket impact, grenade timer detonation, and two near-simultaneous explosions.
- Confirm duplicate callbacks are still suppressed while independent nearby detonations are not.

### A-3: Static mine/charge coverage is not proven

Severity: medium
Confidence: medium
Cleanup risk: medium
Surface: prefab / resource coverage

Evidence:
- Local mod has same-path replacements for core weapon/launcher/grenade/ammo paths and leaf rocket/grenade prefabs with `BS5_ExplosionEchoEffect`.
- Local scan found all local `ProjectileEffects` / `PROJECTILE_EFFECTS` owners include `BS5_ExplosionEchoEffect`.
- Prefab scout found vanilla `Mine_base.et` and `ExplosiveCharge_base.et` use trigger effect containers, but this mod does not currently include same-path mine/charge leaf overrides.

MCP/API/base-game verification:
- MCP/API verified trigger lifecycle exists.
- Prefab coverage was partially verified by local scans and selected MCP merged prefab inspect.
- Mine/charge leaf coverage is not fully verified.

Why it matters:
- Base-prefab hooks do not automatically cover children that own or override active effect containers.
- Mines/static explosives are a separate lifecycle from grenade/rocket projectile impacts, so current rocket/grenade coverage does not prove static explosive coverage.

Suggested cleanup:
- Map the actual vanilla mine/charge prefabs instantiated at runtime.
- Use MCP merged inspect on their active trigger/effect containers before deciding whether to add same-path replacements.

Validation needed:
- MCP merged prefab inspect for selected mine and charge leaf prefabs.
- Runtime detonation logs for a mine and a placed charge.

### A-4: Prefab defaults and technical presets duplicate major tuning knobs

Severity: medium
Confidence: high
Cleanup risk: medium
Surface: config / prefab / architecture

Evidence:
- `Prefabs/Weapons/Core/Weapon_Base.et` has baked tuning for scan radius, candidate counts, SoundMap ray/sample counts, urban scan, limiter caps, and explosion/slapback caps.
- `Configs/BS5/Presets/BS5_TechnicalPresets.conf` repeats the same categories for `default`, `light`, and `dynamic`.
- Examples of drift: prefab scan radius `700` vs preset default `875`; prefab max candidates `9` vs preset default `13`; prefab max traces `9` vs preset default `13`; prefab slapback emitters `4` vs preset default `5` and dynamic `6`.
- `Scripts/Game/BS5_EchoDriverComponent.c:1070-1207`, `1288-1310`, and `1708-1880` prefer active technical preset values when available.

MCP/API/base-game verification:
- Not an API issue; verified by local config/prefab/script reads and MCP merged prefab inspect.

Why it matters:
- There are two sources of truth. Defaults visible in Workbench can differ from runtime values once presets load.
- Debugging performance or sound shape becomes confusing because "prefab value" may not be the effective value.

Suggested cleanup:
- Pick one source as canonical for runtime tuning. Prefer config presets for live tuning and keep prefab defaults as conservative fallback-only values.
- Add debug output only for effective values when diagnosing, not raw prefab values.

Validation needed:
- Check effective values in runtime logs after selecting each technical preset.
- Run `mod validate` after any config/prefab normalization.

### A-5: SoundMap and close-reflection analysis can stack several costly passes

Severity: medium
Confidence: medium-high
Cleanup risk: medium
Surface: scripts / performance

Evidence:
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c:2462-2560` runs SoundMap forward sampling, optional urban micro pass, sample-to-candidate append, fallback anchors, omni anchors, and urban rescue.
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c:3176-3358` path plausibility can sample terrain and SoundMap values along candidate paths and may raycast suspicious paths.
- `Scripts/Game/BS5_CloseReflectionPlanner.c` roof/wall rescue paths add extra traces on close-reflection cases.
- Technical presets allow high default/dynamic values: forward rays 12/14, forward samples 9/10, urban micro entities 35/40, path samples 8/9.

MCP/API/base-game verification:
- Engine trace/SoundMap behavior was not runtime-profiled here.
- The finding is call-path evidence, not a measured perf report.

Why it matters:
- Dense urban scenes can make a single shot/explosion analysis much heavier than the visible emitter count suggests.
- More visual/audio "coverage" settings can increase analysis cost even if limiter caps later drop emissions.

Suggested cleanup:
- Add cheap instrumentation around analysis pass totals before tuning further.
- Budget by phase: SoundMap samples, urban entity scan, path plausibility, raycasts, close-rescue traces.
- Consider preset-specific hard ceilings for explosion-like events, because explosions already emit more slapbacks.

Validation needed:
- One live dense-urban trace-count/log capture per preset.
- Compare `light`, `default`, and `dynamic` on the same scene.

### A-6: Settings slider writes can broadcast and clear audio caches too often

Severity: medium
Confidence: medium
Cleanup risk: low-medium
Surface: UI / state ownership / performance

Evidence:
- Code mapper found `BS5_PlayerAudioSettings` setters call `UserSettingsChanged()` and optional save immediately.
- `BS5_SpatialSoundEmitterComponent` has session-level invalid audio project/event caches that are cleared by settings changes.
- `BS5_AudioSettingsSubMenu.c` has multiple slider flows and pending settings handling.

MCP/API/base-game verification:
- Not MCP-verified in this audit; finding is static call-path evidence.

Why it matters:
- Dragging sliders may create unnecessary global settings broadcasts and cache invalidation churn.
- Cache clears are most valuable when committed settings actually change, not on every transient UI input tick.

Suggested cleanup:
- Gate `UserSettingsChanged()` and `ClearAudioProjectCaches()` to commit/apply transitions or changed-value boundaries.
- Keep UI behavior unchanged for users.

Validation needed:
- Menu smoke test: move sliders, apply, close/reopen menu, confirm persistence and no repeated cache-clear spam.

### A-7: Slapback fallback resolution can hide missing config

Severity: medium
Confidence: medium-high
Cleanup risk: low-medium
Surface: scripts / resource config

Evidence:
- Code mapper found fallback branches in `BS5_EchoDriverComponent.ResolveSlapbackAcp`, `ResolveSlapbackEventName`, and emitter resolver paths.
- `BS5_SpatialSoundEmitterComponent.Play(...)` can fall back from direct project playback to prefab `SoundComponent`.
- `Scripts/Game/BS5_EchoRuntime.c:2566-2579` uses SoundManager for non-slapback but prefab emitter path for slapbacks.

MCP/API/base-game verification:
- MCP verified `SCR_SoundManagerModule` supports `CreateAudioSource(...)` and world-position playback.
- Existing fallback model is plausible; the risk is silent config drift, not invalid API.

Why it matters:
- A missing or mistyped slapback ACP/event/prefab can still produce some audio, making resource regressions hard to notice.
- This can make "working but wrong sound" bugs hard to diagnose.

Suggested cleanup:
- Keep fallbacks, but add a one-time warning or diagnostic counter for fallback-to-default cases.
- Do not rename typo-looking resources such as `Silinced` or `.acp.acp` without Workbench/resource GUID verification.

Validation needed:
- Resource reference validation.
- One smoke test for normal, suppressed, MG, close slapback, trench slapback, and explosion slapback routes.

### A-8: Debug logging model is split between `BS5_DebugLog` and raw prints

Severity: low
Confidence: high
Cleanup risk: low
Surface: scripts / maintainability

Evidence:
- `Scripts/Game/BS5_DebugLog.c` centralizes channel gating.
- `Scripts/Game/BS5_EchoRuntime.c:2498-2506`, `2633-2668`, `2912`, `3550`, and `3588` still contain raw `PrintFormat()` / `Print()` calls, usually inside debug-enabled blocks.
- `Scripts/Game/BS5_EchoDriverComponent.c:2478-2556` contains raw sanity/config prints.

MCP/API/base-game verification:
- Not an API issue.

Why it matters:
- Debug channels cannot consistently shape or suppress logs when direct prints bypass the logging facade.
- This increases noise and makes publish-prep debug sweeps harder.

Suggested cleanup:
- Route raw debug prints through `BS5_DebugLog`.
- Keep unconditional warnings only for true configuration errors.

Validation needed:
- Search for `Print(` and `PrintFormat(` after cleanup.
- Runtime smoke with debug off and debug on.

### A-9: Workbench and local generated files are tracked in the publish surface

Severity: medium
Confidence: high
Cleanup risk: low-medium
Surface: repo / publish hygiene

Evidence:
- `git ls-files` includes `Scripts/WorkbenchGame/EnfusionMCP/*`.
- `git ls-files` includes `resourceDatabase.rdb`.
- `git ls-files` includes `UserMaps.desc`, whose content is just `UserMapDescClass {}`.
- `.agent-work/project-map.md` is tracked. This is useful as a local durable artifact, but it is not publishable mod content by default.

MCP/API/base-game verification:
- Repo sentinel checked this read-only; not an engine API issue.

Why it matters:
- Workbench MCP handlers and local resource DB/user-map metadata can pollute publish/checkpoint diffs.
- `.agent-work` should be intentionally excluded from commits unless the user wants to version audit artifacts.

Suggested cleanup:
- For publish prep, run `wb_cleanup` and verify filesystem/git status afterward.
- Decide repo policy for `resourceDatabase.rdb`, `UserMaps.desc`, and `.agent-work` before committing.

Validation needed:
- `git status --short --untracked-files=all`.
- Filesystem check for `Scripts/WorkbenchGame/EnfusionMCP`.
- `mod validate`; full `mod build` only as a separate long-running release gate.

### A-10: UI settings menu is a large modded monolith

Severity: low
Confidence: medium-high
Cleanup risk: medium
Surface: UI / maintainability

Evidence:
- `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c` is a large modded `SCR_AudioSettingsSubMenu` extension.
- Code mapper found repeated slider flows and manual widget traversal around settings row build/refresh/writeback paths.

MCP/API/base-game verification:
- Project map previously verified relevant settings-menu lifecycle methods exist.
- This audit did not re-run a menu-specific MCP deep dive.

Why it matters:
- Upstream menu changes can break this file in subtle ways.
- Repeated row plumbing increases risk when adding a new setting.

Suggested cleanup:
- Keep behavior, but extract small local helpers for repeated slider/stepper patterns only when doing a UI-focused batch.
- Do not bundle this with explosion/audio runtime fixes.

Validation needed:
- In-game settings menu smoke test after UI cleanup.

## Non-findings / already-good evidence

- Local projectile/grenade/rocket prefab coverage is better than base-only: local effect containers scanned in this mod all include `BS5_ExplosionEchoEffect`.
- MCP verified main hook signatures for `SCR_MuzzleEffectComponent.OnFired`, `BaseProjectileEffect.OnEffect`, `SCR_WeaponBlastComponent.OnWeaponFired`, and trigger safety-distance callbacks.
- `EXPLOSION_DIAGNOSTIC_LOG` is currently `false` in `Scripts/Game/BS5_EchoRuntime.c:4`.
- `modded SCR_WeaponBlastComponent.OnWeaponFired(...)` currently only logs a fire-time blast skip. That looks intentional under the lifecycle policy unless the user decides muzzle blast audio should be added separately.

## Recommended deslop batches

### Batch 1: Publish hygiene and debug-off

Risk: low
Targets:
- `Prefabs/Weapons/Core/Weapon_Base.et`
- `Prefabs/Props/BS5_ExplosionDriver.et`
- `Scripts/WorkbenchGame/EnfusionMCP/*`
- `resourceDatabase.rdb`
- `UserMaps.desc`

Work:
- Disable debug defaults in native prefab attributes.
- Run `wb_cleanup` before publication and verify handler removal.
- Decide whether generated/root metadata stays tracked.

Validation:
- MCP merged prefab inspect.
- `mod validate`.
- `git status --short --untracked-files=all`.

### Batch 2: Explosion lifecycle safety

Risk: medium
Targets:
- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_ExplosionBridge.c`
- selected explosive prefabs after MCP inspect

Work:
- Replace origin-only explosion dedupe with a key that distinguishes callback duplicates from separate detonations.
- Verify mine/charge/static explosive coverage.
- Preserve separation between ordinary shot, fire-time blast callback, and actual impact/detonation.

Validation:
- Runtime logs for launcher shot, rocket impact, grenade timer detonation, mine/charge detonation, and near-simultaneous explosions.

### Batch 3: Effective tuning source and performance evidence

Risk: medium
Targets:
- `Configs/BS5/Presets/BS5_TechnicalPresets.conf`
- `Prefabs/Weapons/Core/Weapon_Base.et`
- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- `Scripts/Game/BS5_CloseReflectionPlanner.c`

Work:
- Make config presets clearly canonical for runtime tuning.
- Add compact effective-value and trace-count instrumentation where needed.
- Do not retune values until current pass cost is measured.

Validation:
- Dense urban live log comparison for `light`, `default`, and `dynamic`.

### Batch 4: Settings/cache and debug facade cleanup

Risk: low-medium
Targets:
- `Scripts/Game/BS5_PlayerAudioSettings.c`
- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_DebugLog.c`
- optionally `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`

Work:
- Reduce settings-change/cache-clear churn.
- Move raw debug prints behind `BS5_DebugLog`.
- Leave UI restructuring as a separate optional step unless actively touching settings UX.

Validation:
- Settings menu smoke test.
- Runtime debug-off and debug-on smoke tests.

## Subagent summary

- `bhe_repo_sentinel`: fix-before-ship; tracked Workbench handlers, `resourceDatabase.rdb`, `UserMaps.desc`, and `.agent-work/project-map.md`; clean status at its checkpoint.
- `bhe_api_researcher`: fix-before-ship; API signatures verified; warned that `HandleExplosionFire(...)` is dead code, but this audit treats it as lifecycle clarity rather than an automatic muzzle-emission bug.
- `bhe_code_mapper`: fix-before-ship; strongest script findings were origin-only explosion dedupe, settings/cache churn, hidden slapback fallbacks, stacked SoundMap passes, raw debug prints, and UI monolith risk.
- `bhe_prefab_researcher`: fix-before-ship; strongest prefab findings were debug-on prefabs, duplicate tuning sources, unproven mine/charge coverage, tracked root artifacts, and possible dead `BS5_CloseReverbEmitter.et`.

## Validation run by this audit

- Read-only file inventory via `rg --files`.
- Git tracked-file/status checks.
- MCP `api_search` for core hook/lifecycle classes and methods.
- MCP `prefab(action=inspect)` for `Weapon_Base.et`, `BS5_ExplosionDriver.et`, `Grenade_RGD5.et`, and `Launcher_Base.et`.

Not run:
- No Workbench reload.
- No live runtime replay.
- No full build.
- No gameplay file mutation.

## Stop condition

The audit has produced a prioritized cleanup list and batch plan. Gameplay files were not mutated.
