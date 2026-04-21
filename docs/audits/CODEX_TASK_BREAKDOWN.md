# CODEX Task Breakdown

## BS5-AUD-001

- Current status: implemented, compile-validated
- Title: Use read-only signal lookup in entity signal reads
- Source finding(s): `M-02`
- Files involved: `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- Goal: Stop creating missing signals during read-only environment classification
- Proposed change: Replace `SignalsManagerComponent.AddOrFindSignal()` with `FindSignal()` in `ReadEntitySignalComponent`
- Why safe / why risky: Safe. It aligns code with read intent and documented API semantics.
- Validation steps:
  - Script compile
  - Check entity/global signal reads in runtime
  - Confirm no missing classification data for existing signals
- Rollback concern: Minimal. Revert single API call if any unexpected compatibility issue appears.
- Suitable for Codex now: Yes

## BS5-AUD-002

- Current status: implemented, compile-validated
- Title: Tighten emitter prefab suffix detection
- Source finding(s): `L-04`
- Files involved: `Scripts/Game/BS5_EchoDriverComponent.c`
- Goal: Match only actual `.et` prefab paths
- Proposed change: Replace substring check with `EndsWith(".et")`
- Why safe / why risky: Safe. This is a direct correctness fix and narrows false positives.
- Validation steps:
  - Script compile
  - Resolve several configured emitter prefab paths
  - Ensure non-prefab strings do not pass the check
- Rollback concern: Minimal
- Suitable for Codex now: Yes

## BS5-AUD-003

- Current status: implemented, compile-validated
- Title: Remove queued driver callbacks on deletion
- Source finding(s): `L-06`
- Files involved: `Scripts/Game/BS5_EchoDriverComponent.c`
- Goal: Avoid queued instance method execution after component teardown
- Proposed change:
  - Remove queued `InvalidateCache`
  - Remove queued `ResetPlaybackLimiterBurst`
  - Remove queued `ClearDispatchGuard`
  - Invalidate local cache/dispatch state before `super.OnDelete`
- Why safe / why risky: Safe. The methods are driver-owned delayed maintenance callbacks.
- Validation steps:
  - Script compile
  - Spawn and remove a weapon/entity using the driver
  - Confirm no delayed callback errors after destruction
- Rollback concern: Low. Revert `OnDelete` block if lifecycle side effects appear.
- Suitable for Codex now: Yes

## BS5-AUD-004

- Current status: implemented, compile-validated
- Title: Gate hot debug log string construction
- Source finding(s): `L-02`
- Files involved: `Scripts/Game/BS5_EchoRuntime.c`
- Goal: Avoid unnecessary string allocation when debug output is disabled
- Proposed change:
  - Guard slapback collection debug strings
  - Guard top-level emit-path debug strings
- Why safe / why risky: Safe. Logging output remains the same when debug is enabled.
- Validation steps:
  - Script compile
  - Run with debug disabled and enabled
  - Confirm debug output still appears when enabled
- Rollback concern: Low
- Suitable for Codex now: Yes

## BS5-AUD-005

- Current status: implemented in guarded form, compile-validated, runtime smoke still incomplete
- Title: Re-resolve delayed emission owner by entity ID
- Source finding(s): `H-03`
- Files involved:
  - `Scripts/Game/BS5_EchoTypes.c`
  - `Scripts/Game/BS5_EchoRuntime.c`
- Goal: Reduce stale-owner risk in delayed playback paths
- Proposed change:
  - Store owner `EntityID` in `BS5_PendingEmissionContext`
  - Resolve owner from `BaseWorld.FindEntityByID()` before delayed component lookups
  - Keep direct owner reference only as a fallback when no ID is available
- Why safe / why risky: Moderate. Safer than trusting a stale `IEntity`, but it changes delayed-path behavior when owners are deleted between queue and playback.
- Validation steps:
  - Script compile
  - Queue delayed emission, then remove owner before callback
  - Verify cleanup paths still release voice/budget state
- Rollback concern: Medium. A bad resolution path could skip legitimate playback.
- Suitable for Codex now: Yes, but only with runtime smoke validation

## BS5-AUD-006

- Current status: implemented, compile-validated
- Title: Cap invalid emitter resource cache
- Source finding(s): `H-04`
- Files involved: `Scripts/Game/BS5_EchoRuntime.c`
- Goal: Prevent unbounded growth of cached invalid emitter prefab names
- Proposed change: Apply a small fixed cap to `s_aInvalidEmitterResourceNames`; keep valid resource cache behavior unchanged
- Why safe / why risky: Safe. This does not alter valid resource loading behavior.
- Validation steps:
  - Script compile
  - Exercise invalid prefab path handling if practical
  - Confirm repeated invalid lookups still short-circuit
- Rollback concern: Low
- Suitable for Codex now: Yes

## BS5-AUD-007

- Current status: implemented in minimal guarded form, compile-validated, runtime smoke still incomplete
- Title: Add reentrancy guards around shared query scratch state
- Source finding(s): `H-02`
- Files involved:
  - `Scripts/Game/BS5_EchoRuntime.c`
  - `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- Goal: Make shared mutable query state fail-closed instead of silently reusing globals
- Proposed change:
  - Add active-query guard flag for slapback entity query path
  - Add active-query guard flag for forward-facade query path
  - Reject callback participation when query state is inactive
- Why safe / why risky: Low-Medium. Small code change, but it relies on a conservative "drop nested query" behavior if reentry occurs.
- Validation steps:
  - Script compile
  - Heavy repeated-fire smoke test
  - Verify no broken classification or obvious query starvation
- Rollback concern: Medium. If the engine legitimately nests these queries, dropping nested work could reduce candidates.
- Suitable for Codex now: Yes, but keep the change minimal

## BS5-AUD-008

- Current status: not implemented
- Title: Validate local-first emitter spawn behavior
- Source finding(s): `M-06`
- Files involved: `Scripts/Game/BS5_EchoRuntime.c`
- Goal: Determine whether spawn-order rewrite is actually correct for this mod in MP/client contexts
- Proposed change: No code change yet. Run a focused runtime experiment comparing current fallback order with local-first behavior.
- Why safe / why risky: Risky if implemented blindly. Spawn semantics are engine- and network-context-dependent.
- Validation steps:
  - MP client/server test
  - Observe spawned emitter lifetime and `SoundComponent` readiness
  - Confirm no unwanted replication side effects
- Rollback concern: High if changed without evidence
- Suitable for Codex now: No, investigation first
