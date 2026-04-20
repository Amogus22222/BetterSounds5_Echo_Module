# BetterSounds 5 BS5 Echo/Tails Plan

## Status
- Phase 1: discovered existing `BHE_*` weapon/character prefab hooks and confirmed the project is runtime-scripts empty.
- Phase 2: verified the relevant engine hooks and audio APIs.
- Phase 3: implemented `BS5_` runtime components and shared echo core.
- Phase 4: prefab migration completed; script validation confirmed through Workbench logs; buildData CLI path still needs the active Workbench session / module path to be wired through the MCP bridge.
- Phase 5: runtime audio contract clarified. Existing vanilla-derived `Weapons_BS_Echo_1.acp` is not suitable as a self-contained spawned reflection ACP, so helper emitter prefabs and a local `BS5` signal contract were added.
- Phase 6: helper-emitter playback is now verified in live logs (`BS5 emitter play: handle=...`). Forward-cone candidate ranking, editor-facing tuning descriptions, emitter-budget guards, and a dedicated near-wall slapback candidate path are in place; current focus is empirical tuning and future emitter pooling if large-scale firefights demand it.
- Phase 7: project-side `EnfusionMCP` handlers were restored for local development. `EMCP_WB_GetState` now runs in ultra-safe mode after repeated Workbench crashes from editor-side API calls.
- Phase 8: the old synthetic tail template path was replaced by an obstacle-driven env-map scan. Tail candidates now come from real world traces against terrain/buildings using a flattened heading basis, sector/ring sampling, and reflector clustering. The old line-like fallback remains only as an emergency path when the scan finds nothing. Unused template-era driver settings/getters were removed to reduce dead tuning surface before the next live-fire pass.
- Phase 9: tail generation was refactored again around an ambient-driven hybrid planner. The old obstacle-grid path remains only as compatibility code; the active runtime path now builds an environment snapshot from global ambient/audio signals and terrain context, then generates view-biased anchors with capped pitch influence and profile-driven fallback clouds instead of a single fixed-distance synthetic emitter.
- Phase 10: the hybrid planner was corrected so the primary path no longer treats terrain-projected points as confirmed reflectors in open profiles. A dedicated settlement/facade acquisition pass was added ahead of the generic seed loop, open-scene fallback distances were reset to sane defaults, and the stale legacy character ACP ref was replaced with the BS5 master ACP.
- Phase 11: production pass in progress. Suppressor-aware tails now resolve through `BaseMuzzleComponent.IsMuzzleSuppressed()` and can switch to a dedicated suppressed emitter/ACP path. Indoor tuning fields and planner branches are wired in, explosion bridge lookup now checks a broader owner/root path, and tail emission exposes suppressor state through the spawned emitter signal contract. Live gameplay verification for suppressors, indoor spaces and explosion-prefab coverage is still pending.
- Phase 12: client-local player audio settings were added for BS5 echo volume. A new `ModuleGameSettings` module and runtime cache helper now drive a custom `Echo Volume` slider injected into the vanilla `Audio` submenu, and the final spawned-emitter intensity path now scales tails/slapbacks/explosion echoes by the player preference while also exposing `BS5_UserEchoVolume` to ACP.
- Phase 13: the hybrid tail planner now uses a coarse cached sector-field as the primary runtime path. Instead of dense forward/lateral seed scans and a dedicated settlement pre-pass every shot, each weapon driver now reuses a per-weapon sector cache keyed by coarse position/orientation/profile and synthesizes tails from a small obstacle/terrain sector field. `profile_fallback` remains only as the emergency fill path when the sector field does not produce enough useful anchors, and open-scene sector probes are now intentionally decoupled from the much longer open fallback distances so the scan can still discover medium-range reflectors without collapsing the fallback character.
- Phase 14: urban forward reflections are now supplemented by a very small forward-facade micro-pass that runs only on sector-cache misses for open/hill profiles when no facade hits were found by the main sector field. This keeps the normal runtime path cheap while giving shots fired toward buildings at roughly 65-130 m a chance to promote into `facade_hit` / `SETTLEMENT_EDGE` without reintroducing the old heavy facade scan.
- Phase 15: the urban forward path now also has an entity-aware fallback. On cache misses with no facade hits, the classifier queries a narrow forward corridor using `QueryEntitiesByBeveledLine`, filters large building-like entities via `GetWorldBounds()`, and synthesizes facade anchors from their forward-facing bounds instead of relying purely on geometry traces. This is intended to be more stable on Arland-style relief where forward traces often hit terrain before houses.
- Phase 16: facade-hit delays are now scaled separately from terrain/fallback delays via a dedicated driver setting so early urban reflections do not feel late simply because the emitter is spatialized on the building. The entity-aware forward query is also hard-capped to a small number of entities to keep cache-miss cost predictable.
- Phase 17: the urban/entity facade path now requires confirmatory geometry hits on the same queried entity instead of trusting bounds alone, which is intended to stop trees/vegetation from being promoted as `facade_hit`. The cache was relaxed slightly to improve reuse under short movement/aim changes, and the active long-tail runtime now uses the existing tertiary distance band so open/hill fallback and long probes can reach out toward 600 m without inventing new tuning fields.
- Phase 18: the urban/entity path is now split into a wider raw-query stage and a ranked confirm stage. The raw query follows terrain height better on downhill city shots, the entity cap was raised so detached houses do not get crowded out by early clutter hits, prefab-path hints now reject obvious vegetation and boost likely buildings, and expensive confirm-traces now run only on the best-ranked subset instead of every queried entity.
- Phase 19: the active tuning surface was synced with the real runtime planner. `Weapon_Base.et` now drives the intended 25-300 m settlement window and up-to-800 m long-tail ceiling more directly, while the urban facade query rejects more non-building clutter (`Props/Agriculture`, `Construction`, low-footprint junk, unknown props) and no longer relies on the earlier hardcoded 40/58/76/92/280 style distances.
- Phase 20: technical preset live logs showed that preset IDs switched (`tech=default/dynamic`) but many values still came from `BS5_PresetRegistry.c` fallback defaults (`lifetime=3.5`, dynamic `urbanMicroQueries=24`, `soundMapSamples=63`). The current fix is to keep `Configs/BS5/Presets/BS5_TechnicalPresets.conf`, `Weapon_Base.et`, and hardcoded `FillDefault/FillLight/FillDynamicTechnicalPreset()` values synchronized. Next live-log check should show default/dynamic tail `lifetime=3.9`, dynamic sample budget near `11*8`, dynamic urban micro query cap up to `32`, and light sample budget near `4*4`.

## Current Verified Facts
- `addon.gproj`:
  - `ID/TITLE = BetterSounds5`
  - `GUID = 6717325A0F4513E2`
- Existing prefab hooks:
  - `Prefabs/Weapons/Core/Weapon_Base.et`
  - `Prefabs/Characters/Core/Character_Base.et`
- Existing resource refs:
  - `Sounds/Weapons/Rifles/AK-74/Weapons_Rifles_EchoMaster.acp`
  - `Prefabs/Props/BS5_TailEmitter.et`
  - `Prefabs/Props/BS5_TailEmitter_PBS.et`
  - `Prefabs/Props/BS5_SlapbackEmitter.et`
- Confirmed hooks/APIs:
  - `SCR_MuzzleEffectComponent.OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)`
  - `SCR_WeaponBlastComponent.OnWeaponFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)`
  - `SoundComponent.SoundEventTransform(...)`
- `SignalsManagerComponent.AddOrFindSignal(...)`
- `SignalsManagerComponent.SetSignalValue(...)`
- `Game.SpawnEntityPrefab(...)`
- `Game.GetGameUserSettings()`
- `Game.UserSettingsChanged()`
- `Game.SaveUserSettings()`
- `ArmaReforgerScripted.OnUserSettingsChangedInvoker()`
- `SCR_TerrainHelper.GetHeightAboveTerrain(...)`
- `IEntity.GetTransform(out vector mat[4])`
- `ChimeraCharacter.TraceMoveWithoutCharacters(BaseWorld world, inout TraceParam param)`
- `BaseMuzzleComponent.IsMuzzleSuppressed()`
- `WorkspaceWidget.CreateWidgets(...)`
- `SliderWidget.SetRange/SetStep/SetCurrent/GetCurrent()`
- `BaseMuzzleComponent.IsMuzzleSuppressed()`
- existing driver-side event cache/reuse hooks are sufficient to hold a coarse per-weapon sector cache without introducing replicated gameplay state

## Implementation Targets
1. `Scripts/Game/BS5_EchoTypes.c`
2. `Scripts/Game/BS5_EchoRuntime.c`
3. `Scripts/Game/BS5_EchoDriverComponent.c`
4. `Scripts/Game/BS5_ExplosionBridge.c`
5. Prefab class-name migration from `BHE_*` to `BS5_*`

## Acceptance Criteria
- Weapon prefab gets an attachable `BS5_EchoDriverComponent`.
- ACP paths and tunables are exposed in editor-visible component properties.
- Runtime uses event-driven analysis, not per-frame scanning.
- Burst reuse is present for automatic fire.
- Slapback and tails are separated in logic and event routing.
- Explosion path shares the same core and is wired through a verified hook.
- Helper emitter playback is verified through runtime handles in Workbench logs.
- Candidate selection stays spatially stable for repeated shots from the same location/heading and changes only when the shooter position/orientation meaningfully changes.
- Majority of emitted tails should rank from a forward muzzle-aligned cone while still retaining a smaller set of wide/off-axis returns.
- Slapback no longer depends on far tail candidates; it is now generated from dedicated near-wall probe hits and emitted through the slapback helper prefab/event path.
- ACP-side modifiers are fed via `BS5_*` signals instead of script-side DSP hacks.
- `mod_validate` and `mod_build` pass.
- Workbench reload/play/stop verification is completed.
- Tails are no longer generated from deterministic angle templates in normal scenes; reflected emitters should preferentially sit on world obstacles and terrain faces discovered by the env-map scan.
- Vertical aim no longer strongly rotates the tail spread because tail azimuth sampling is driven from flattened forward/right vectors.
- Suppressor tails can switch to a dedicated emitter prefab / ACP without introducing a separate event name by default.
- Explosion-tail coverage is still expected to reuse the shared runtime core; the current production pass only broadens driver lookup and aligns event wiring, it does not introduce a separate explosion subsystem.
- Players can adjust BS5 echo loudness locally from the in-game `Audio` menu without changing vanilla dry-shot/music/UI/VOIP loudness.
- Primary tail generation should now report `anchorMode=sector_field`, `sectors=...`, `heights=...`, and `sectorCacheHit=...` in live logs so cache reuse versus fresh sector refresh is visible during tuning.
- In open scenes, true sector hits may now emit more than the old `m_iOpenTailFallbackCount` if the sector field actually found useful reflectors; the fallback count remains the emergency open-scene fill budget, not the hard cap for all open-scene tails.
- Runtime preset source is currently treated as fallback-first for safety: if external `.conf` preset loading is unavailable or stale in Workbench, `BS5_PresetRegistry.c` fallback preset values must still match the intended preset table.

## Risks
- Tails currently use per-emission helper entity spawn/cleanup. This is functionally verified and now protected by per-weapon active-emitter caps, but it remains the primary scalability hotspot for 32+ player automatic-fire scenarios until emitter reuse/pooling is added.
- Environment classification is intentionally limited; tails rely on engine environment at playback time, while script only contributes deterministic point placement and near-space confinement metrics.
- `CallLater`-based delay scheduling and helper retries must stay bounded and must not accumulate unbounded pending emissions.
- The dedicated slapback path is structurally ready through `BS5_SlapbackEmitter.et`, but its final audible quality still depends on the user-supplied slapback ACP graph.
- The repository-level shell can compile scripts through Workbench logs, but the `buildData` path did not produce output when launched standalone from shell after the Workbench process was closed.
- The new obstacle-driven tail scan still needs live-fire validation in Workbench to confirm that emitters consistently land on visible houses/slopes instead of synthetic rows in open coastal scenes.
- Workbench NET API is reachable again for `wb_connect` and `wb_reload`, but live-fire confirmation of the new settlement/facade acquisition pass is still pending.
- `mod_build` can still time out through MCP in this workspace, so the most reliable fast validation loop remains `wb_reload` + `mod_validate` until a full build is re-confirmed.
- The new `Echo Volume` slider path is script-valid and Workbench-reloaded, but its live menu placement/persistence still needs a manual smoke test in-game because the menu interaction itself is not directly automatable through MCP.
- The new sector-field planner is script-valid and Workbench-reloaded, but its perceptual quality still needs live-fire confirmation with fresh logs showing `source=...` and `sectorCacheHit=...`. The current CLI `mod_build` path still fails with a Workbench/SteamDiag exit code before producing a trustworthy full-build confirmation.
