# BetterSounds5 Active Mod Plan

## Scope

This file is the single active roadmap for the repo. It replaces the older duplicated root-level planning notes.

## Current State

- Core echo/tail runtime is working and Workbench-compilable.
- Audit remediation Waves 0-2 are complete.
- `H-01` was measured with runtime telemetry and is now deferred.
- `M-06`, `M-07`, and `L-05` remain intentionally unimplemented until separate validation exists.
- Repo cleanup/documentation normalization is complete.

## Completed

### Audit and Safety

- `M-02`: read-only signal lookup now uses `FindSignal`
- `L-02`: hot debug string building gated
- `L-04`: prefab suffix check uses `EndsWith`
- `L-06`: driver `OnDelete` now cleans delayed callbacks
- `H-03`: delayed owner liveness guarded through `EntityID` re-resolve
- `H-04`: invalid emitter resource cache capped
- `H-02`: minimal shared-scratch reentrancy guards added

### Runtime Stability

- stale limiter state cleanup added for deleted owners / world changes
- Workbench compile path revalidated after each guarded fix

### H-01 Decision

- temporary runtime telemetry was added, tested, and removed
- measured result did not justify pooling or structural allocation optimization
- current decision: do not optimize H-01 now

### Repository Cleanup

- duplicate root planning/context notes removed
- active docs moved under `docs/reference`, `docs/plans`, and `docs/audits`
- one canonical `README.md` and one canonical `MODPLAN.md` retained

## Active Workstreams

### 1. Slapback Expansion

Goal:

- extend slapback behavior for specific near-wall / trench / constrained scenarios
- keep cost local and bounded
- avoid broad rewrites of tail planning while improving the close reflection layer

Primary design reference:

- [SLAPBACK_EXPANSION_PLAN.md](docs/plans/SLAPBACK_EXPANSION_PLAN.md)

### 2. Deferred Investigation

Do not start without fresh validation:

- `M-06` spawn-order behavior
- `M-07` trace/query filter rewrites
- `L-05` dispatch timing semantics

## Non-Goals Right Now

- no pooling of candidate/result/pending context objects
- no broad acoustic retuning rewrite
- no removal of MCP bridge scripts
- no prefab/config/audio churn unrelated to the next planned workstream

## Validation Standard

- Workbench startup on `addon.gproj`
- `Module: Game` compile success
- no new BS5 `SCRIPT (E)` lines
- runtime smoke only for the specific subsystem being changed
