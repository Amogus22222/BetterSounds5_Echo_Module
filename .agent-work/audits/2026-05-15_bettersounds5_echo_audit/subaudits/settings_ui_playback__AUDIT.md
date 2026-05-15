# Subaudit - Settings, UI, and playback helpers

## Scope

`BS5_PlayerAudioSettings.c`, `BS5_PresetRegistry.c`, `BS5_SpatialSoundEmitterComponent.c`, `BS5_AudioSettingsSubMenu.c`, and small shared settings/debug/type files.

## Files inspected

- `Scripts/Game/BS5_PlayerAudioSettings.c`
- `Scripts/Game/BS5_PresetRegistry.c`
- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
- `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`
- `Scripts/Game/BS5_EchoTypes.c`
- `Scripts/Game/BS5_DebugLog.c`
- `Scripts/Game/BS5_AudioDebugSettingsComponent.c`
- `Scripts/Game/BS5_CloseReflectionSettingsComponent.c`

## MAP context used

The map identified settings persistence, preset fallback, audio caches, signal pushing, procedural UI injection, and debug gating as this sector's main risks.

## Enfusion MCP checks

MCP confirmed settings APIs, `AudioSystem` APIs, `SignalsManagerComponent` APIs, and sound component signal/playback APIs. UI source layout internals were not fully verified from vanilla source.

## Functional summary

Player settings persist in `BS5_GameAudioSettings`. Presets load from config with script fallback. The settings menu injects BS5 controls procedurally. Emitter playback pushes signals to audio paths and can use direct ACP playback or prefab `SoundComponent` fallback.

## Findings

### A-001: Sound preset apply triggers repeated settings notifications and cache clears

- Severity: High
- Category: duplicated work / lifecycle
- Evidence: `Scripts/Game/BS5_PresetRegistry.c:448-459` calls five player-setting setters. Those setters call `game.UserSettingsChanged()` in `Scripts/Game/BS5_PlayerAudioSettings.c:78`, `113`, `148`, `189`, `245`, and `278`. `OnUserSettingsChanged()` clears audio project caches at `Scripts/Game/BS5_PlayerAudioSettings.c:306-310`.
- Why this is AI-slop / risk: one preset step fans out into repeated global settings notifications, repeated reloads, and repeated audio cache invalidation.
- API/BIKI/base-game verification: `UserSettingsChanged()` is a real engine notification path; batching writes before notification is the engine-idiomatic pattern.
- Behavior risk if changed: low-medium. Batching should preserve final values, but settings UI needs verification.
- Cleanup direction: add batch setters or a notification suppression scope so preset changes notify once and save once.

### A-002: Slider drags write live settings and clear audio caches

- Severity: Medium
- Category: lifecycle / performance
- Evidence: slider handlers such as `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c:1046-1207` call `BS5_PlayerAudioSettings.Set*` while dragging. Those setters trigger `UserSettingsChanged()` and cache clears before final flush.
- Why this is AI-slop / risk: the UI has dirty/flush state, but still mutates global settings on each movement.
- API/BIKI/base-game verification: settings APIs are valid; problem is call frequency and invalidation scope.
- Behavior risk if changed: medium. Live preview may depend on immediate writes.
- Cleanup direction: decide whether sliders need live audio preview. If yes, separate local preview values from persisted settings and avoid clearing ACP validity caches for pure volume changes.

### A-003: Preset fallback has duplicated truth and stale IDs are masked

- Severity: Medium
- Category: stale config / fallback behavior
- Evidence: fallback IDs and values are defined in `Scripts/Game/BS5_PresetRegistry.c:527-834` and config files under `Configs/BS5/Presets/`. `LoadFromUserSettings()` accepts any non-empty preset ID at `Scripts/Game/BS5_PlayerAudioSettings.c:365-370`.
- Why this is AI-slop / risk: duplicated fallback is currently coherent, but stale saved IDs can remain stored and be displayed as fallback labels rather than canonicalized.
- API/BIKI/base-game verification: settings containers allow raw string storage; validation is local responsibility.
- Behavior risk if changed: low. Canonicalizing unknown IDs should move invalid state to known defaults.
- Cleanup direction: validate stored preset IDs against registry on load and optionally write back canonical defaults.

### A-004: Procedural settings UI is brittle and contains a dead wrapper

- Severity: Low-Medium
- Category: dead code / UI contract
- Evidence: `HandleBs5ClickAction()` at `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c:214-216` is not called; the file uses heuristic widget discovery around `Content`, first slider row, and text widgets.
- Why this is AI-slop / risk: one unused helper is clear dead scaffolding, and the rest of the UI injection depends on vanilla widget hierarchy assumptions.
- API/BIKI/base-game verification: lifecycle hooks are valid; widget hierarchy was not confirmed from vanilla source.
- Behavior risk if changed: medium. UI changes are fragile.
- Cleanup direction: remove the unused wrapper only in a narrow cleanup. Defer broader UI anchoring until UI behavior is in scope.

## Duplicate/overlapping logic

- Preset application duplicates individual setting setter behavior.
- Spatial emitter sends the same signal set to direct ACP payloads and prefab playback paths.
- Four slider rows repeat near-identical code.

## Dead or unreachable code candidates

- `HandleBs5ClickAction()` is unused.

## Performance hotspots

- Repeated `UserSettingsChanged()` and `ClearAudioProjectCaches()` during preset apply and slider drag.
- Per-emission signal packing/writing in `BS5_SpatialSoundEmitterComponent`.

## API uncertainty / required follow-up

`AudioSystem.PlayEvent()` `-1` failure semantics were not proven enough to treat permanent invalid-event caching as a gameplay bug.

## Sanity notes

Dual signal writes are not first-pass cleanup. The code explicitly comments that direct `SoundComponent.SetSignalValueStr()` supports same-frame visibility.
