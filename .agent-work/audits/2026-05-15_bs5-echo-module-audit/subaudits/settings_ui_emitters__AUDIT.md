# Subaudit - Settings, UI, and emitters

## Scope

Read-only audit of settings, preset registry, emitter playback, debug settings, and procedural audio settings UI.

## Files inspected

- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
- `Scripts/Game/BS5_CloseReflectionSettingsComponent.c`
- `Scripts/Game/BS5_AudioDebugSettingsComponent.c`
- `Scripts/Game/BS5_DebugLog.c`
- `Scripts/Game/BS5_PlayerAudioSettings.c`
- `Scripts/Game/BS5_PresetRegistry.c`
- `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`
- `Configs/BS5/Presets/BS5_SoundPresets.conf`
- `Configs/BS5/Presets/BS5_TechnicalPresets.conf`
- `Prefabs/Weapons/Core/Weapon_Base.et`
- `Prefabs/Props/BS5_SlapbackEmitter_Close.et`

## MAP context used

The map identifies `BS5_SpatialSoundEmitterComponent` as the spawned emitter playback layer, `BS5_PlayerAudioSettings` and `BS5_PresetRegistry` as settings/preset owners, and `BS5_AudioSettingsSubMenu` as the procedural UI extension.

## Enfusion MCP checks

- `ModuleGameSettings` exists and supports the user settings pattern used by the mod.
- `SCR_AudioSettingsSubMenu` exposes `OnTabShow`, `OnTabHide`, `OnMenuHide`, and `OnTabRemove`.
- `SoundComponent`, `SignalsManagerComponent`, and `AudioSystem` APIs used by the emitter component are real.
- BI audio docs confirm the event, signal, ACP, and `SoundComponent` playback model.

## Functional summary

The settings and emitter code is mostly engine-real, but several pieces duplicate the same data in multiple places: preset defaults, fallback constants, signal payloads, and close-hook flush behavior. The invalid audio-resource caches also have no recovery path.

## Findings

### A-001: Preset defaults are split across config, script fallback, and player bootstrap

- Severity: Medium
- Category: duplicated state / data drift
- Evidence: `BS5_PlayerAudioSettings` repeats `0.65 / 0.4 / 0.4` in attributes, static defaults, and load fallback. `BS5_PresetRegistry.AddFallbackSoundPresets` repeats those values for `vanilla`. The live `BS5_SoundPresets.conf` sets `vanilla` to `0.75 / 0.6 / 0.8`.
- Why this is AI-slop / risk: One concept has multiple sources of truth and they already diverge.
- API/BIKI/base-game verification: `ModuleGameSettings` usage is valid; the risk is local ownership drift.
- Behavior risk if changed: Removing fallbacks too aggressively can make missing config failures harder to tolerate.
- Cleanup direction: Make config the canonical data source, keep emergency fallback explicit, and avoid pretending stale fallback values are the current preset.

### A-002: Emitter signal payload is duplicated between playback backends

- Severity: Medium
- Category: double computation / performance / architecture
- Evidence: `BS5_SpatialSoundEmitterComponent.Play` writes signals directly through `SetSignalValue`, while `BuildAudioSystemSignals` separately reconstructs the same signal map for `AudioSystem.PlayEvent`. Per-signal writes also call `SignalsManagerComponent.AddOrFindSignal` and `SoundComponent.SetSignalValueStr`.
- Why this is AI-slop / risk: A future signal rename or new signal can drift between `SoundComponent` and `AudioSystem` playback paths.
- API/BIKI/base-game verification: MCP confirmed both signal APIs and `AudioSystem.PlayEvent`; the issue is duplication, not API correctness.
- Behavior risk if changed: Shared payload generation must preserve existing signal names and order for both backends.
- Cleanup direction: Build one signal payload and reuse it for both direct audio and component playback; consider cached indices where safe.

### A-003: Static invalid ACP/event caches can hide a fixed resource until restart

- Severity: Medium
- Category: risky side effect / invalid cache
- Evidence: `EnsureAudioProjectReady`, `IsAudioProjectEventKnownInvalid`, and `MarkAudioProjectEventInvalid` maintain static negative caches. A failed `PlayEvent` immediately marks a project/event invalid for the rest of the session.
- Why this is AI-slop / risk: Temporary missing resources or hot-reloaded ACP fixes stay suppressed.
- API/BIKI/base-game verification: `AudioSystem.PlayEventInitialize` and `PlayEvent` are valid.
- Behavior risk if changed: Retrying too often could spam bad resources.
- Cleanup direction: Use time-limited negative caching, add explicit invalidation on settings/resource reload, or add a debug cache-clear path.

### A-004: Procedural settings submenu has cleanup stubs and repeated close-path logic

- Severity: Low
- Category: lifecycle / dead code
- Evidence: `TryCloneBs5EchoVolumeRow` always returns `false`; `ClearBs5ScriptHandlers` exists but is not called; `OnTabHide`, `OnMenuHide`, and `OnTabRemove` repeat the same flush sequence.
- Why this is AI-slop / risk: Dead helpers imply cleanup that is not actually wired, which makes future UI lifecycle edits fragile.
- API/BIKI/base-game verification: The submenu lifecycle overrides are MCP-verified.
- Behavior risk if changed: UI teardown changes need a smoke test because procedural widgets are stateful.
- Cleanup direction: Either wire cleanup into tab removal or remove the unused helpers; collapse repeated flush logic into one local helper.

## Duplicate/overlapping logic

- Player setting defaults repeat in attributes, static fields, config fallbacks, and live config.
- Emitter signal payload is built in two different methods.
- UI close flushing is repeated in three overrides.

## Dead or unreachable code candidates

- `TryCloneBs5EchoVolumeRow`.
- `ClearBs5ScriptHandlers`.
- No-op `HandlerAttached` override.

## Performance hotspots

- Per-emission signal payload assembly and `AddOrFindSignal` calls.
- `game.UserSettingsChanged()` on every slider change.
- Debug printing if the master debug switch is enabled.

## API uncertainty / required follow-up

Widget handler removal was not fully expanded through MCP. Treat any UI cleanup implementation as requiring Workbench/UI smoke validation.

## Sanity notes

The inspected `Weapon_Base.et` keeps the debug component disabled, so verbose debug flags are not active by default in that prefab.
