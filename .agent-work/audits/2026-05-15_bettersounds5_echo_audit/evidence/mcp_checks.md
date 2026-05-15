# MCP/API evidence notes

Generated: 2026-05-15

## Confirmed API surfaces

- `BaseWorld.QueryEntitiesBySphere(vector center, float radius, QueryEntitiesCallback addEntity, QueryEntitiesCallback filterEntity = null, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL)` exists.
- `BaseWorld.QueryEntitiesByBeveledLine(vector from, vector to, vector bevelMins, vector bevelMaxs, QueryEntitiesCallback addEntity, QueryEntitiesCallback filterEntity = null, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL)` exists.
- `ChimeraCharacter.TraceMoveWithoutCharacters(BaseWorld world, inout TraceParam param)` exists and is documented as an optimized TraceMove variant that filters out characters.
- `AudioSystem` exposes `PlayEventInitialize`, `PlayEvent`, `IsSoundPlayed`, and `TerminateSoundFadeOut`.
- `SCR_SoundManagerModule` exposes `CreateAudioSource(...)` and `PlayAudioSource(...)` paths.
- `Game.GetGameUserSettings()`, `Game.UserSettingsChanged()`, `ArmaReforgerScripted.OnUserSettingsChangedInvoker()`, and settings module access exist.
- `BaseProjectileEffect.OnEffect(...)` signature matches the modded projectile effect usage.
- `SCR_ExplosiveTriggerComponent.TriggeredInSafetyDistance(...)` and `SCR_PressureTriggerComponent.TriggeredInSafetyDistance(...)` signatures matched MCP search results.

## API uncertainty kept in the audit

- `SCR_AudioSettingsSubMenu` lifecycle hooks were treated as valid based on available API results and existing script compile context, but vanilla widget layout internals were not read from base-game source in this run.
- `AudioSystem.PlayEvent(...)` returning `-1` is used as permanent invalid event evidence by local caches; the exact failure semantics were not proven, so this is kept as a medium/low Workbench/hot-reload risk.
- `SignalsManagerComponent` plus `SoundComponent.SetSignalValueStr(...)` are both valid APIs. The audit downgrades duplicate signal writes because the code comment says same-frame sound graph visibility may be intentional.

## Validation not run

Workbench was not launched and no mod build/reload was run. The user requested read-only audit mode, and this audit changed only markdown artifacts.
