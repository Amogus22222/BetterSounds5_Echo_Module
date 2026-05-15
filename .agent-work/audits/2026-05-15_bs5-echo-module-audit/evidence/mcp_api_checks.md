# MCP API checks

Generated: 2026-05-15

## Confirmed API surfaces

- `SCR_MuzzleEffectComponent.override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)`
- `SCR_WeaponBlastComponent.void OnWeaponFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)`
- `SCR_SoundManagerModule.GetInstance(World world)`
- `SCR_SoundManagerModule.CreateAudioSource(...)`
- `SCR_SoundManagerModule.PlayAudioSource(SCR_AudioSource audioSource)`
- `SCR_AudioSource.SetSignalValue(string name, float value)`
- `SCR_AudioSource.Terminate(bool fadeOut=true)`
- `AudioSystem.PlayEventInitialize(string resourceName)`
- `AudioSystem.PlayEvent(string resourceName, string eventName, vector transf[], array<string> names=null, array<float> values=null)`
- `ModuleGameSettings`
- `SCR_AudioSettingsSubMenu.OnTabShow`, `OnTabHide`, `OnMenuHide`, `OnTabRemove`
- `BaseWorld.QueryEntitiesBySphere(...)`
- `ChimeraCharacter.TraceMoveWithoutCharacters(BaseWorld world, inout TraceParam param)`

## Downgraded suspicions

- `SCR_WeaponBlastComponent.OnWeaponFired` is real in MCP method search even though the class summary result is sparse.
- The major audio and settings APIs used by BS5 are real; local issues are data drift, fallback/caching behavior, and lifecycle cleanup rather than guessed engine syntax.

## Unverified

- No Workbench compile/reload validation was run during this read-only audit.
- Exact base-game UI script bodies were not available from guessed `game_read` paths.
