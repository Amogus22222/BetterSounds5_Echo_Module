# CODEX Fix Implementation Plan

## Goal

Implement only the fixes that are defensible from source review and documented API behavior, while isolating engine-assumption-dependent work behind validation gates.

## Order Of Work

1. Wave 0: no-risk correctness fixes
2. Wave 1: low-risk performance and maintainability fixes
3. Wave 2: guarded structural fixes
4. Wave 3: risky engine-assumption-dependent changes
5. Deferred / do-not-touch-yet

## Current Status

- Wave 0: implemented, Workbench script compile validated
- Wave 1 safe fixes: implemented, Workbench script compile validated
- Wave 2 guarded fixes: implemented in narrow form, Workbench script compile validated
- Wave 3: not started
- Deferred / do-not-touch-yet: unchanged
- H-01 measurement pass: completed with timing-only runtime sampling; optimization not justified at current evidence level

## Wave 0 - No-Risk Correctness Fixes

Status: implemented

Tasks:

- `M-02` switch read-only signal lookup from `AddOrFindSignal` to `FindSignal`
- `L-04` replace `.IndexOf(".et") != -1` with `.EndsWith(".et")`
- `L-06` remove queued driver instance callbacks in `OnDelete` and invalidate local state

Why here:

- These change intent-preserving code paths.
- They do not alter acoustic tuning or spawning model.
- They directly correct mismatches between code intent and API usage.

Expected effect:

- No accidental signal creation during reads
- More accurate emitter-prefab path validation
- Better destruction safety for driver component teardown

Risk:

- Low

Dependencies:

- None beyond script compile

Validation:

- Workbench script compile
- Load a weapon prefab using the driver
- Fire one unsuppressed shot and one suppressed shot
- Verify no obvious regression in tail/slapback dispatch

## Wave 1 - Low-Risk Performance / Maintainability Fixes

Status: implemented for `L-02` and `H-04`; optional `M-01` / `M-08` cleanup intentionally skipped

Tasks:

- `L-02` gate hot debug-string building behind debug checks
- `H-04` cap invalid emitter resource cache growth
- Optional maintenance-only cleanup from `M-01` / `M-08` if adjacent edits justify it

Why here:

- These are low-risk and localized.
- They improve behavior under repeated runtime use without changing the main acoustic model.

Expected effect:

- Less avoidable string allocation on hot debug-disabled paths
- Invalid prefab cache cannot grow without bound
- Slightly clearer maintenance surface

Risk:

- Low

Dependencies:

- Wave 0 complete

Validation:

- Debug on/off smoke test
- Repeated fire test to ensure no missing logs when debug is enabled
- Invalid emitter prefab path test if possible

## Wave 2 - Guarded Structural Fixes

Status: implemented in guarded/minimal form

Tasks:

- `H-03` store owner `EntityID` in pending emission context and resolve owner at delayed execution time
- `H-02` add lightweight guards around shared global query scratch state

Why here:

- These improve safety around deferred callbacks and shared mutable globals.
- They are reasonable, but they touch lifecycle and control-flow assumptions.

Expected effect:

- Less reliance on stale `IEntity` references in delayed emit paths
- Reentrant query reuse becomes explicit instead of silent

Risk:

- Low-Medium

Dependencies:

- Wave 0 and Wave 1 complete
- Basic smoke validation in runtime

Validation:

- Despawn owner during delayed tail/slapback timing window
- Repeated close-quarters firing near surfaces
- Verify no stuck active voice counters or emitter-budget counters

## Wave 3 - Risky / Engine-Assumption-Dependent Changes

Status: not started

Tasks:

- `M-06` evaluate local-first emitter spawning
- `M-07` verify trace/query filter changes with real map/runtime evidence

Why here:

- These depend on engine behavior not proven by static reading alone.
- A wrong change here can alter multiplayer behavior, emitter readiness, or acoustic classification quality.

Expected effect:

- Unknown until validated

Risk:

- Medium to High

Dependencies:

- Manual runtime verification
- Preferably multiplayer test coverage for spawn behavior

Validation:

- MP client/server emitter spawn observation
- Instrumented trace hit comparison before/after filter changes

## H-01 Measurement Outcome

Status: measured, deferred

What was checked:

- Temporary timing-only telemetry on live runtime scenarios
- Repeated single-shot and repeated-fire cases in open, facade, and slapback-capable spaces

Observed result:

- `profile analyze` stayed around `3-5 ms`
- `profile shot` stayed around `4-5 ms` on cache misses and `0 ms` on cached repeats at timer resolution
- Measured cost clustered in planner/slapback work, not in any proven GC-related stall
- No evidence justified object pooling or structural allocation rewrites

Decision:

- Do not implement H-01 optimization work now
- Do not add pooling for candidate, result, or pending-emission objects
- Revisit only if a future profiler capture shows a real allocation/GC problem

## Deferred / Do-Not-Touch-Yet

Status: unchanged by implementation pass

Do not rewrite now:

- `M-03` `ToLower()` handling
- `M-04` burst counter logic
- `M-05` candidate ordering/data-structure rewrite
- `M-09` linear search rewrite
- `L-01` acoustic constants
- `L-03` "legacy" component removal
- `L-05` time-vs-frame dispatch rewrite

Reason:

- Either the finding is misleading, or the change risk is higher than the defect, or the payoff is too small for this codebase.

## Dependency Notes

- Wave 0 is independent.
- Wave 1 should follow Wave 0 mainly for review clarity.
- Wave 2 should not start until Wave 0 and Wave 1 pass smoke validation.
- Wave 3 should not start without new evidence.

## Validation Strategy

Compile:

- Run Workbench script compile after each wave

Runtime smoke:

- Unsuppressed shot
- Suppressed shot
- Slapback near a close wall
- Longer tail case in open terrain
- Despawn owner during delayed playback window
- Debug on/off comparisons

Regression checks:

- No missing tail/slapback audio events
- No obviously duplicated emitters
- No stuck driver counters
- No silent failure from guard code

## What Not To Fix Right Now

- Anything that rewrites acoustic scoring
- Anything that changes spawn semantics without runtime proof
- Anything that changes trace filters without evidence
- Any "micro-optimization" whose value has not been measured
- Any H-01 pooling or reuse rewrite based only on theoretical allocation counts
