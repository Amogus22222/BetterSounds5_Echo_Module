# BS5 resource/classifier optimization fork

Date: 2026-04-19
Root: G:\BettersMods\BetterSounds5

## Baseline

This is a local working-folder fork note. The folder is not detected as a Git repository, so this file records the change scope against the current on-disk state.

Known audit points being addressed here:
- repeated `Resource.Load(context.m_sEmitterPrefab)` during every tail/slapback emission;
- expensive forward facade classifier work on cache misses, especially in open/hill profiles with no confirmed facades.

## Goals

1. Keep behavior stable.
2. Avoid broad rewrite of the hybrid tail planner.
3. Add low-risk caching only around hot paths.
4. Preserve debug visibility so tuning logs show when cache shortcuts are active.

## Planned changes

### Emitter resource cache

Add a small script-level cache inside `BS5_EchoEmissionService`.

Behavior:
- key by emitter prefab `ResourceName`;
- call `Resource.Load` only on first use for that prefab;
- reuse valid cached `Resource`;
- remember invalid prefabs to avoid repeated failed loads and repeated log spam.

Expected effect:
- fewer repeated resource lookup calls during burst fire;
- no audio behavior change if prefab paths are valid.

### Forward facade negative cache

Add a small negative cache on `BS5_EchoDriverComponent`.

Behavior:
- when the expensive forward facade passes run and confirm zero facade hits, remember that local cell/heading/profile;
- on nearby later shots with similar heading/profile, skip only the expensive forward facade entity/micro passes;
- still run the cheaper sector field planner so terrain/open tails remain fresh.

Expected effect:
- open fields and hill/open profiles avoid repeated building/facade searches while moving slightly;
- settlement behavior remains conservative because any confirmed facade clears the negative cache.

## Out of scope

- no large refactor of `BS5_EnvironmentAudioClassifier.c`;
- no deletion of legacy tail code;
- no explosion hook/API fix;
- no prefab tuning changes such as `m_fSoundSpeedMetersPerSecond`.

## Validation plan

- static text scan for changed symbols;
- inspect modified call sites;
- Workbench compile still needed after this patch because this folder is outside the configured MCP project directory.

## Implemented in this fork

Files changed:
- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_EchoTypes.c`

Implemented behavior:
- `BS5_EchoEmissionService` now resolves emitter prefab resources through a script-level cache.
- Only the resolver calls `Resource.Load`; normal spawn path uses cached `Resource`.
- Invalid emitter prefab names are cached after the first failed load attempt.
- `BS5_EchoDriverComponent` now stores a forward-facade negative cache for nearby open/hill shots with similar heading.
- `BS5_HybridTailPlanner` skips only expensive forward facade entity/micro passes when that negative cache hits.
- Sector field candidate collection still runs on cache miss, so terrain/open tails remain fresh.
- Debug summary now includes `forwardNegativeCacheHit`.

Validation performed:
- static symbol scan confirms only one remaining `Resource.Load`, inside the cache resolver;
- Workbench bridge is connected in edit mode;
- script reload was requested through MCP, but MCP does not expose compiler diagnostics here;
- `mod validate` still cannot run because this folder is outside the configured Enfusion MCP project directory.
