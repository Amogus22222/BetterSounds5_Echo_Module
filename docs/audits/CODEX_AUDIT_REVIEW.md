# CODEX Audit Review

## Summary

This review re-checks the external forensic audit against the actual `Scripts/Game` sources and documented Enfusion / Arma Reforger API behavior.

Main conclusion:

- The audit found several real issues.
- Severity was overstated in multiple places.
- Some proposed fixes are safe and worth doing.
- Some proposed fixes depend on engine/runtime assumptions that are not proven from this codebase alone.
- One finding (`M-03`) is misleading and should not be implemented as written.

Safe findings to take into work immediately:

- `M-02` read-only signal lookup should use `FindSignal`
- `L-02` hot debug string building should be gated
- `L-04` `.et` path check should use `EndsWith`
- `L-06` `OnDelete` should remove pending instance `CallLater` callbacks

Implementation snapshot after current pass:

- Implemented and compile-validated: `M-02`, `L-02`, `L-04`, `L-06`, `H-04`
- Implemented in guarded form and compile-validated: `H-02`, `H-03`
- Not implemented: `H-01`, `M-06`, `M-07`, `L-05`
- Intentionally not touched: `M-03`, `M-04`, `M-05`, `M-09`, `L-01`, `L-03`

H-01 measurement result after runtime sampling:

- Repeated live tests with temporary timing-only telemetry showed `profile analyze` around `3-5 ms` and `profile shot` around `4-5 ms` on cache misses.
- Measured time concentrated in planner and slapback work, not in any proven GC spike.
- Candidate/cache behavior looked stable; no evidence justified pooling or structural allocation rewrites.
- Conclusion: keep `H-01` as a real code characteristic, but defer optimization. Do not add pooling or object reuse complexity now.

Guarded but acceptable after minimal validation:

- `H-03` deferred owner liveness via `EntityID` re-resolve
- `H-04` cap invalid emitter resource cache growth
- `H-02` lightweight reentrancy guards around shared query scratch state

Do not implement from the original audit as-is:

- `M-03` `ToLower()` assignment rewrite
- `M-06` local-first spawn rewrite without runtime verification
- `M-07` trace-layer/filter rewrite without experiment
- `H-01` allocation pooling without measurement

## Findings Table

| ID | Status | Revised Severity | Repro Likelihood | Fix Risk | Effort | Needs Engine Assumption | Safe Now | Notes |
|---|---|---|---|---|---|---|---|---|
| H-01 | CONFIRMED | Medium | Medium | Medium | Medium | No for existence, yes for payoff | No | Hot-path allocations exist, but current runtime timing samples did not justify pooling or structural optimization. Treat as measured-and-deferred unless future profiling shows spikes. |
| H-02 | PARTIALLY_CONFIRMED | Low | Low | Low-Medium | Low | Yes | Guarded | Shared mutable scratch arrays exist. The race/reentrancy claim is not proven. A small guard is reasonable; a context rewrite is not justified. |
| H-03 | PARTIALLY_CONFIRMED | Medium | Medium | Low-Medium | Low-Medium | Partial | Guarded | Deferred contexts hold `IEntity` references. A guaranteed crash is not proven, but resolving owner by `EntityID` is a defensible safety guard. |
| H-04 | PARTIALLY_CONFIRMED | Low | Low | Low | Low | Partial | Yes | Linear invalid-cache growth exists, but emitter prefab cardinality appears low and `Resource.Load` is engine-cached. Cap invalid names only. |
| M-01 | CONFIRMED | Low | High | Low | Low | No | Not priority | Duplicate terrain helper logic is real but mostly maintenance debt. |
| M-02 | CONFIRMED | Medium | High | Low | Low | No | Yes | `AddOrFindSignal` on read path can create signals and pollute state. `FindSignal` is the correct read-only API. |
| M-03 | MISLEADING | None | N/A | High | Low | No | No | Official docs indicate `string.ToLower()` mutates and returns length. Reassigning the result would be wrong or dangerous. |
| M-04 | MISLEADING | None | N/A | Medium | Low | No | No | Burst-counter reset behavior is intentional. Overflow is not a realistic concern here. |
| M-05 | CONFIRMED | Low | High | Medium | Medium | No | No | Small-`N` quadratic behavior exists, but candidate counts are tiny and ordering changes are risky. |
| M-06 | PARTIALLY_CONFIRMED | Medium | Medium | Medium-High | Medium | Yes | No | `SpawnEntityPrefabLocal` is local-only by docs, but replacing spawn order in this mod still needs live MP validation. |
| M-07 | UNCONFIRMED | Low | Low | Medium | Medium | Yes | No | Default trace/query flags exist, but "wrong layers" affecting acoustics is not proven in this project. |
| M-08 | PARTIALLY_CONFIRMED | Low | Medium | Low | Low | No | Optional | Raw IDs/constants can be cleaned up, but this is maintenance, not a functional defect. |
| M-09 | CONFIRMED | Low | High | Medium | Medium | No | No | Linear entity search exists, but the cap is small and the current behavior is predictable. |
| L-01 | CONFIRMED | Low | High | High | Medium | No | No | Magic numbers are mostly acoustic tuning. Rewriting them without regression baselines would be reckless. |
| L-02 | CONFIRMED | Low | High | Low | Low | No | Yes | Several debug logs build strings on hot paths before debug gating. |
| L-03 | MISLEADING | None | N/A | High | Low | No | No | "Legacy" components are still referenced by prefabs and compatibility paths. Removal would break content. |
| L-04 | CONFIRMED | Low | High | Low | Low | No | Yes | `IndexOf(".et")` matches non-suffix cases. `EndsWith(".et")` is the correct intent. |
| L-05 | UNCONFIRMED | Low | Low | Medium | Medium | Yes | No | Time-vs-frame dispatch semantics are not proven wrong in current behavior. |
| L-06 | CONFIRMED | Medium | Medium | Low | Low | No | Yes | `OnDelete` exists but does not remove queued instance callbacks, leaving stale delayed work risk. |

## Reassessed Takeaways

### Confirmed and worth fixing

- `M-02`
- `L-02`
- `L-04`
- `L-06`

### Confirmed but not worth rewriting now

- `M-05`
- `M-09`
- `L-01`

### Partially confirmed and should stay guarded

- `H-02`
- `H-03`
- `H-04`
- `M-06`
- `M-08`

### Downgraded or rejected

- `M-03`
- `M-04`
- `L-03`
- `L-05`
- `M-07`

## What Is Safe To Implement

Safe now:

- Replace read-only `AddOrFindSignal` with `FindSignal`
- Gate hot debug-string building
- Tighten `.et` prefab suffix check
- Remove driver instance `CallLater` callbacks in `OnDelete`
- Cap invalid emitter resource cache

Safe only with narrow guardrails:

- Store owner `EntityID` in pending emission context and re-resolve on delayed execution
- Add debug-oriented reentrancy guards around shared global query scratch state

Not safe to take blindly from the external audit:

- Any `ToLower()` assignment change
- Any spawn-order rewrite around `SpawnEntityPrefab` vs `SpawnEntityPrefabLocal`
- Any trace flag/layer rewrite
- Any pool/preallocation rewrite aimed at GC pressure without runtime data

## Implementation Buckets

### 1. Safe To Implement Now

- `M-02`
- `L-02`
- `L-04`
- `L-06`
- `H-04`
- `H-03` as a narrow owner re-resolve guard
- `H-02` only as a minimal reentrancy guard, not a structural rewrite

### 2. Needs Measurement / Profiling First

- None required for current roadmap execution

### 3. Measured And Deferred

- `H-01`

### 4. Needs Engine / API Verification First

- `M-06`
- `M-07`
- `L-05`
- Any deeper rewrite implied by `H-02`

### 5. Probably Not Worth Fixing Now

- `M-01`
- `M-05`
- `M-08`
- `M-09`

### 6. Should Be Documented, Not Rewritten

- `M-03`
- `M-04`
- `L-01`
- `L-03`
