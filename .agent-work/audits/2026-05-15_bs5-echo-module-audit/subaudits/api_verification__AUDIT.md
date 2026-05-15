# Subaudit - API verification

## Scope

Read-only verification of the main Enfusion/API assumptions used by BetterSounds5.

## Files inspected

- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_ExplosionBridge.c`
- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
- `Scripts/Game/BS5_PlayerAudioSettings.c`
- `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`

## MAP context used

The project map flagged `SCR_MuzzleEffectComponent.OnFired`, `SCR_WeaponBlastComponent.OnWeaponFired`, audio playback APIs, settings APIs, and trace/query APIs as API surfaces needing verification.

## Enfusion MCP checks

- `SCR_MuzzleEffectComponent` exposes `override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)`.
- `SCR_WeaponBlastComponent` exposes `void OnWeaponFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)`.
- `SCR_SoundManagerModule` exposes `GetInstance`, `CreateAudioSource`, and `PlayAudioSource`.
- `AudioSystem` exposes `PlayEventInitialize`, `PlayEvent`, `IsSoundPlayed`, and termination APIs.
- `SoundComponent`/base sound surfaces expose event and signal methods.
- `SignalsManagerComponent` / signal components expose signal lookup and value writes.
- `ModuleGameSettings` is the correct base class for game settings modules.
- `SCR_AudioSettingsSubMenu` exposes the lifecycle overrides used by the mod.
- `BaseWorld.QueryEntitiesBySphere` and `ChimeraCharacter.TraceMoveWithoutCharacters` exist.

## Functional summary

The audit did not find a confirmed wrong API call in the major entry points. Earlier map uncertainty around `SCR_WeaponBlastComponent.OnWeaponFired` is resolved: the method exists in MCP method search even though the class summary is sparse.

## Findings

### A-001: No confirmed wrong callback signatures in the main fire/blast hooks

- Severity: Informational
- Category: API verification
- Evidence: MCP confirms both `SCR_MuzzleEffectComponent.OnFired` and `SCR_WeaponBlastComponent.OnWeaponFired` with the same three-parameter signature used by the mod.
- Why this is AI-slop / risk: This rejects a previous suspicion rather than adding a cleanup task.
- API/BIKI/base-game verification: MCP API search.
- Behavior risk if changed: Do not refactor these signatures as part of deslop unless compile output proves a problem.
- Cleanup direction: Keep as-is; run compile/reload after later implementation changes.

### A-002: Audio and settings APIs are real, but local fallback behavior still needs cleanup

- Severity: Informational
- Category: API verification
- Evidence: MCP confirms `SCR_SoundManagerModule`, `AudioSystem`, `SoundComponent`, signal APIs, `ModuleGameSettings`, and `SCR_AudioSettingsSubMenu` surfaces.
- Why this is AI-slop / risk: Local slop findings should not be framed as guessed engine behavior.
- API/BIKI/base-game verification: MCP API search and BI audio/settings docs.
- Behavior risk if changed: Treat settings/emitter cleanup as behavior-preserving local cleanup, not API migration.
- Cleanup direction: Focus on duplicate local data, resource integrity, invalid caches, and UI lifecycle.

## Duplicate/overlapping logic

No API-level duplicates found; duplication is local code/data duplication.

## Dead or unreachable code candidates

None from API verification alone.

## Performance hotspots

Trace/query APIs are real and performance-sensitive; see runtime subaudit for local hotspots.

## API uncertainty / required follow-up

Compile/reload validation was not run. Base-game script bodies for the guessed UI paths were not available through `game_read`.

## Sanity notes

Do not list `OnWeaponFired` as unverified in future reports for this session; MCP method search found it.
