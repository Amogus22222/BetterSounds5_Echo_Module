# CODEX Open Questions

## Purpose

These points cannot be closed honestly from static source review alone. They need either official API clarification, runtime observation, or profiling.

## Engine / API Questions

### 1. Query callback reentrancy and shared scratch state

- Related findings: `H-02`
- Current evidence:
  - Shared mutable global arrays are real.
  - No proof was found that these specific query callbacks can re-enter concurrently.
- What needs verification:
  - Can `QueryEntitiesBySphere` / `QueryEntitiesByBeveledLine` lead to nested callback execution that reuses the same static scratch state?
  - Is callback dispatch strictly single-threaded and non-reentrant for these calls?
- How to verify:
  - Runtime instrumentation in debug build
  - Artificial nested-query test if possible
- Until verified:
  - Keep only a minimal guard, not a structural rewrite

### 2. Delayed callback owner liveness

- Related findings: `H-03`, `L-06`
- Current evidence:
  - Delayed playback stores owner references in pending context.
  - Driver instance methods are scheduled with `CallLater`.
- What needs verification:
  - How stale `IEntity` references behave after owner deletion in this exact runtime path
  - Whether delayed static callbacks can still observe partially torn-down component state
- How to verify:
  - Queue delayed emission
  - Destroy owner before callback executes
  - Observe whether callbacks safely no-op or touch invalid state
- Until verified:
  - Prefer `EntityID` re-resolve and explicit cleanup over broader lifecycle changes

### 3. Spawn order: `SpawnEntityPrefab` vs `SpawnEntityPrefabLocal`

- Related finding: `M-06`
- Current evidence:
  - Docs confirm `SpawnEntityPrefabLocal` is local-only.
  - Current implementation tries replicated spawn first, local spawn second.
- What needs verification:
  - In this mod's actual client/runtime context, does local-first improve reliability or break expected ownership/visibility behavior?
  - Does `SoundComponent` initialization timing differ between the two paths?
- How to verify:
  - Client/server runtime test
  - Observe emitted helper entity existence, readiness, and any replication artifacts
- Until verified:
  - Do not change spawn order

### 4. Trace/query filter defaults

- Related finding: `M-07`
- Current evidence:
  - The code uses default query/trace behavior in several places.
  - No direct evidence shows that wrong entity classes are materially biasing acoustic results.
- What needs verification:
  - Which false-positive entities are actually being collected on representative maps
  - Whether changing flags would improve classification without removing valid reflectors
- How to verify:
  - Add temporary debug collection output
  - Compare hit sets before and after filter changes on several maps/surfaces
- Until verified:
  - Do not rewrite filters

### 5. Real cost of hot-path allocation pressure

- Related finding: `H-01`
- Current evidence:
  - Hot-path array/string/object allocation exists.
  - Timing-only runtime sampling showed analyze cost around `3-5 ms`, with most visible time in planner/slapback work.
  - No current evidence shows a payoff for pooling or structural allocation rewrites.
- What still remains open:
  - Whether a future profiler capture would reveal rare GC spikes that coarse timer sampling cannot show
- How to verify if this becomes important again:
  - Dedicated profiler capture on repeated-fire scenarios
  - Compare default, light, and dynamic presets
- Until then:
  - Treat H-01 as measured-and-deferred
  - Do not introduce pooling complexity

### 6. Dispatch timing semantics

- Related finding: `L-05`
- Current evidence:
  - Delays are time-based, not frame-count-based.
  - No user-visible defect is proven by the current implementation.
- What needs verification:
  - Whether frame-varying time slices actually produce unacceptable timing behavior in practice
- How to verify:
  - Runtime capture at different frame rates
  - Compare perceived timing and event ordering
- Until verified:
  - Leave timing model unchanged

## Questions Already Closed

These were checked against official docs and should not remain open:

- `M-03`: `string.ToLower()` mutates the string and returns length; do not assign the return value
- `M-02`: `AddOrFindSignal` creates missing signals; `FindSignal` is the correct read path
- `L-04`: suffix intent is better expressed with `EndsWith(".et")`

## What Still Cannot Be Claimed Honestly

- That query callbacks are actually racing in this mod
- That stale owner references are currently causing crashes rather than latent risk
- That local-first emitter spawning is superior in multiplayer/client contexts
- That allocation pooling will produce meaningful gains without regressions
