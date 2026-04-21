# BetterSounds5 Project Context

## Purpose

BetterSounds5 adds event-driven reflected weapon tails, echo emitters, slapback, and shared explosion-tail reuse for Arma Reforger.

The system is not meant to be a full acoustic simulation. The design goal is:

- event-driven runtime
- spatial helper emitters
- environment-informed reflector selection
- bounded pending/active emitter budgets
- ACP-driven playback through runtime signals

## Main Runtime Files

- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- `Scripts/Game/BS5_EchoTypes.c`
- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
- `Scripts/Game/BS5_PlayerAudioSettings.c`

## Main Content Files

- `Prefabs/Weapons/Core/Weapon_Base.et`
- `Prefabs/Props/BS5_TailEmitter.et`
- `Prefabs/Props/BS5_TailEmitter_Silenced.et`
- `Prefabs/Props/BS5_TailEmitter_MG.et`
- `Prefabs/Props/BS5_SlapbackEmitter.et`
- `Prefabs/Props/BS5_SlapbackEmitter_Silenced.et`
- `Prefabs/Props/BS5_SlapbackEmitter_Trench.et`
- `Sounds/Weapons/Rifles/BS5/Weapons_Rifles_EchoMaster.acp`
- `Sounds/Weapons/Rifles/BS5/Weapons_Silinced_EchoMaster.acp`
- `Sounds/Weapons/Rifles/BS5/Weapons_Slapbacks_Master.acp`
- `Sounds/Weapons/Rifles/BS5/Weapons_Slapbacks_Silinced_Master.acp`
- `Sounds/Weapons/Rifles/BS5/Weapons_Slapbacks_Trench_Master.acp`

## Runtime Contract

At a high level:

1. a weapon fire event reaches `BS5_EchoDriverComponent`
2. the driver resolves or computes `BS5_EchoAnalysisResult`
3. runtime picks tail/slapback candidates
4. pending emission contexts are queued with bounded limits
5. helper emitters spawn and receive runtime signals
6. ACP graphs handle actual playback character

## Current Stable Decisions

- Audit remediation Waves 0-2 are already implemented.
- Delayed owner liveness and stale limiter cleanup are already guarded.
- MCP bridge support stays in repo.
- `H-01` optimization is deferred after measurement.

## Current Deferred Decisions

Do not touch without separate validation:

- `M-06` spawn-order rewrite
- `M-07` trace/query filter rewrite
- `L-05` timing-model rewrite

## Working Constraints

- Prefer narrow changes over structural rewrites.
- Treat `Weapon_Base.et`, ACP graphs, and signal contracts as a coupled system.
- Do not blindly retune acoustics and code in the same patch.
- Validate meaningful script work with real Workbench startup.

## Next Planned Feature Area

- targeted slapback expansion for specific scenarios
- keep it local, cheap, and bounded
- avoid regressing the already-stable tail system
