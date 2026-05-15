# Subaudit - Driver and explosion bridge

## Scope

`Scripts/Game/BS5_EchoDriverComponent.c` and `Scripts/Game/BS5_ExplosionBridge.c`, with supporting call-path checks in runtime and shared types.

## Files inspected

- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_ExplosionBridge.c`
- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_EchoTypes.c`

## MAP context used

The map marks the driver as the main config/cache/limiter surface and the explosion bridge as multiple hook fan-in into runtime explosion dispatch.

## Enfusion MCP checks

Subagent and main-agent checks found no confirmed override signature mismatch for muzzle, trigger, or projectile effect hooks. Super-order behavior was not elevated without base method body evidence.

## Functional summary

The driver owns BS5 authoring attributes, resource resolution, shot and explosion entry handling, cache state, limiter/burst guard logic, and active emitter budget counters. The bridge routes weapon blast, explosive trigger, pressure trigger, and projectile effect paths into runtime explosion dispatch.

## Findings

### A-001: Analysis cache can reuse stale or wrong context

- Severity: Medium-High
- Category: lifecycle / cache correctness
- Evidence: `Scripts/Game/BS5_EchoDriverComponent.c:2128-2151` checks explosion flag, suppressed flag, origin tolerance, and heading dot. The `owner` parameter is accepted but not used. `StoreCachedResult()` at `2165-2183` only schedules time-based invalidation.
- Why this is AI-slop / risk: analysis depends on active technical preset and owner context, but the cache key does not include preset/settings generation or owner/root identity.
- API/BIKI/base-game verification: local cache logic, not API mismatch.
- Behavior risk if changed: medium. Cache saves hot-path work; invalidating too broadly may increase cost.
- Cleanup direction: add owner/root identity and settings generation to the cache key, or invalidate driver caches on relevant preset/settings changes.

### A-002: Explosion duplicate suppression is handled downstream and is too broad

- Severity: Medium
- Category: duplicate suppression / risky side effect
- Evidence: `Scripts/Game/BS5_ExplosionBridge.c:1-37` has multiple explosion ingress paths; `Scripts/Game/BS5_EchoRuntime.c:120-140` suppresses a second dispatch within two frames and `DistanceSq < 4.0` using only the last origin.
- Why this is AI-slop / risk: suppression is necessary, but origin/frame alone can drop distinct nearby explosions.
- API/BIKI/base-game verification: `BaseWorld.GetFrameNumber()` exists; heuristic is local.
- Behavior risk if changed: medium-high. Must still collapse duplicate hook fan-in for one explosion.
- Cleanup direction: key recent dispatches by source identity, instigator/damage source, source tag, and origin bucket.

### A-003: Explosion intensity multiplier is unclamped

- Severity: Medium
- Category: formula / authoring guard
- Evidence: `Scripts/Game/BS5_EchoDriverComponent.c:952` returns `m_fExplosionIntensityMultiplier` raw; runtime applies it in `Scripts/Game/BS5_EchoRuntime.c:361-363`.
- Why this is AI-slop / risk: neighboring getters clamp authoring values, but this one can mute or saturate explosion analysis silently.
- API/BIKI/base-game verification: local formula logic.
- Behavior risk if changed: low if default remains unchanged.
- Cleanup direction: clamp/floor like neighboring getters and log out-of-range authoring under debug validation.

### A-004: Debug validation skips explosion-specific resources

- Severity: Low-Medium
- Category: diagnostics / config drift
- Evidence: `DebugValidateConfiguration()` around `Scripts/Game/BS5_EchoDriverComponent.c:2349+` checks normal tail/slapback paths, while explosion ACP and emitter getters live around `772+` and `788+`.
- Why this is AI-slop / risk: explosion authoring can fall back silently instead of surfacing during debug startup.
- API/BIKI/base-game verification: resource paths were not exhaustively validated here; finding is about local debug coverage.
- Behavior risk if changed: low.
- Cleanup direction: add explosion ACP, explosion slapback ACP, explosion emitter, and explosion slapback emitter to debug validation.

## Duplicate/overlapping logic

- Explosion fan-in is intentionally duplicated across hooks and collapsed downstream.
- Tail sector cache clone-on-write/read is defensive but not cheap.
- Several limiter priority and kill-score distance bands are duplicated in runtime.

## Dead or unreachable code candidates

- `ResolveCachedResult(IEntity owner, ...)` does not use `owner`.
- `HandleExplosionFire(... projectileEntity)` does not use `projectileEntity` after driver lookup.

## Performance hotspots

- Cache clone-on-hit in tail sector cache.
- Burst limiter schedules a reset for each accepted shot; not confirmed harmful, but worth logging under sustained MG fire.

## API uncertainty / required follow-up

Super-order behavior for explosion hooks was not judged as a bug without reading base method bodies or runtime testing.

## Sanity notes

`OnDelete()` cleanup for queued callbacks and limiter/cache generation counters appears coherent.
