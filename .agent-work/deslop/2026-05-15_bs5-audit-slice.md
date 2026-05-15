# Deslop - BS5 audit slice

Generated: 2026-05-15 19:35 +03:00
Approved by user: "de slop this project using 2026-05-15_bettersounds5-anti-slop-audit.md and map (already created in agents work), spawn sub agents if needed, do not turn off debug yet - mod still in development"

## Scope

- Active mod: `G:\BettersMods\BettersMods\BetterSounds5_Echo_Module`
- Project file: `addon.gproj`
- Cleanup batch from `.agent-work/audits/2026-05-15_bettersounds5-anti-slop-audit.md`
- Implemented script-side slice only:
  - A-2 explosion duplicate dispatch identity.
  - A-6 settings broadcast/cache churn.
  - A-8 raw debug prints in touched runtime paths.

## Non-goals

- Did not disable prefab debug defaults. User explicitly said debug must stay on.
- Did not delete Workbench helper files, `resourceDatabase.rdb`, `UserMaps.desc`, or `.agent-work`.
- Did not retune presets, SoundMap settings, audio resources, or prefab coverage.
- Did not add mine/charge leaf prefab coverage.

## Evidence from audit

- Audit A-2 identified frame/origin-only explosion dedupe as risky for clustered detonations.
- Audit A-6 identified setter-driven `UserSettingsChanged()` and audio-project cache clearing churn.
- Audit A-8 identified raw debug prints in runtime/debug paths despite existing `BS5_DebugLog`.
- Project map confirmed lifecycle split:
  - ordinary shot path via `SCR_MuzzleEffectComponent.OnFired`
  - fire-time blast callback via `SCR_WeaponBlastComponent.OnWeaponFired`
  - impact/detonation via `BS5_ExplosionEchoEffect.OnEffect` and trigger hooks

## MCP/API/resource verification

- `workshop_info` confirmed addon ID/GUID/title: `BetterSounds5` / `6717325A0F4513E2` / `BetterSounds5`.
- `api_search` confirmed `BaseTriggerComponent.TriggeredInSafetyDistance(...)` and inherited explosive/pressure trigger signatures.
- `api_search` confirmed `IEntity.GetRootParent()` exists.
- `mod validate` passed structure, gproj, scripts, and prefabs.
- `prefab(action=inspect)` confirmed debug remains enabled on:
  - `Prefabs/Weapons/Core/Weapon_Base.et`
  - `Prefabs/Props/BS5_ExplosionDriver.et`

## Files touched

- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_PlayerAudioSettings.c`
- `Scripts/Game/BS5_PresetRegistry.c`
- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`

Backup:
- `.agent-work/backups/2026-05-15_bs5-deslop-scripts-before.zip`

## Change log

- Replaced single last-explosion frame/origin dedupe with a bounded recent marker list keyed by nearby origin plus owner/damage/hit/instigator root identity and source tag.
- Kept weapon-fire blast lifecycle separate; `BS5_ExplosionBridge.c` was not changed.
- Added no-op guards to BS5 player audio setting setters so unchanged values do not rebroadcast settings.
- Added `BeginSettingsBatch()` / `EndSettingsBatch()` and batched preset apply writes to one settings notification/save.
- Limited audio-project cache clearing to technical or sound preset ID changes instead of every user setting change.
- Routed raw debug prints in touched debug-gated runtime paths through `BS5_DebugLog.Channel` or `ChannelEnabled`.
- Left `EXPLOSION_DIAGNOSTIC_LOG = false` unchanged and left prefab debug defaults enabled.

## Validation

- `git diff --check`: no whitespace errors; Git reported standard LF-to-CRLF warnings on touched scripts.
- `rg` raw print check on touched files: only `BS5_EchoRuntime.ExplosionDiag()` remains, hard-gated by `EXPLOSION_DIAGNOSTIC_LOG = false`.
- MCP `mod validate`: passed structure, gproj, scripts, prefabs; warnings remained for config classes missing from API index and local class prefix info.
- MCP prefab inspect confirmed debug-on state preserved for weapon base and explosion driver prefabs.
- Workbench connection check succeeded, but Workbench was in play/game mode; script reload was not forced.

## Final scout result

Verdict: no-finding.

Evidence:
- Final scout found no blocker in settings batching, dedupe identity, or debug-log routing.
- It noted runtime/compile reload and real explosion duplicate tests remain unverified.

## Risks / follow-up

- Runtime edge case remains: same-origin near-simultaneous detonations with the same strong owner/damage source may still dedupe. Needs live duplicate-path logs if this scenario matters.
- Settings UI live slider behavior was preserved; no in-game menu smoke test was run.
- Workbench script reload was skipped because Workbench is currently in play mode.
- Static mine/charge prefab coverage, publish hygiene, and debug-off publish prep remain separate batches.

## Rollback instructions

- Restore touched scripts from `.agent-work/backups/2026-05-15_bs5-deslop-scripts-before.zip`, or use Git to revert only the five touched script files if these changes are not wanted.
