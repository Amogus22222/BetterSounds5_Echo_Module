# BS5 Slapback Rework Plan

## Current State
- Slapback path lives in `BS5_EchoRuntime.CollectSlapbackCandidates`.
- Current scan is 5 rays: front, front-left, front-right, right, left.
- Current base config has slapback disabled and uses one wall slapback ACP/prefab.
- Current per-shot cap in prefab is 4, but target behavior is max 2.
- No distinction exists between hard wall/alley/car reflection and trench/earth-wall boom.

## Target Behavior
- Slapback remains cheap and local: fixed raycasts only, no SoundMap, no broad entity query.
- Max 2 slapbacks per shot.
- Wall/alley mode detects hard nearby surfaces up to 25 m and emits spatial hard reflection.
- Trench mode detects close left/right earth walls and emits muffled trench boom.
- Trench has higher priority than wall when confidence is high.
- If a dedicated trench ACP/prefab is not configured yet, trench falls back to the normal wall slapback path so runtime stays safe.

## Implementation Plan
- Add candidate sources `SLAPBACK_WALL` and `SLAPBACK_TRENCH`.
- Add raycast-miss fail-safe: if no close ray hits but already-ranked close city/facade anchors exist within `NearSlapbackRadius`, spawn one wall slapback from the nearest strong anchor. This fixes SoundMap-visible buildings whose physics trace does not answer reliably without turning 50m city tails into slapbacks.
- Add close hard-object entity fallback: if direct slap rays miss, run a bounded sphere/beveled-line query within 25 m, filter hard prefabs (house/wall/fence/vehicle/container/rock), and build the slapback point from the entity AABB closest wall. This is still local and cheap, and does not use 50 m SoundMap city anchors.
- Fix slapback resource wiring: `BS5_SlapbackEmitter.et` and `Weapon_Base.et` use current `Weapons_Slapbacks_Master.acp` GUID, not removed `Weapons_Rifles_SlapBacksMaster.acp`.
- Fix event wiring: slapback default event is `SOUND_SHOT`; helper also falls back from old `BS5_TAIL_SLAPBACK` to `SOUND_SHOT` if prefab ACP lacks the old event.
- Replace the old 5-ray collector with a smart fixed ray set:
  - front, front +/- 25 deg, front +/- 55 deg, left, right, rear-left, rear-right.
  - collect valid hits, reject self hierarchy and floor/ceiling-like normals.
  - score by distance, direction, wall-facing normal, and lateral support.
- Build wall clusters from the best hard-surface hits and keep at most 2.
- Build one trench candidate when both left and right close side hits are present:
  - side distance under configurable max, default 4.5 m.
  - left/right distance symmetry and wall normal confidence decide trench score.
  - emitter position is centered between side walls, slightly forward.
- Resolve slapback ACP/event/emitter by candidate source:
  - wall uses existing slapback ACP/prefab/event.
  - trench uses new optional trench ACP/prefab/event attributes, with fallback to wall.
- Add debug summary fields:
  - `slapMode`, `slapRays`, `slapHits`, `wallScore`, `trenchScore`.
- Add slapback distance controls/signals:
  - runtime gain uses `SlapbackDistanceFarGain` + `SlapbackDistanceGainCurvePower`.
  - ACP can read `BS5_SlapbackDistanceMeters`, `BS5_SlapbackDistanceNorm`, `BS5_SlapbackDistanceGain`.

## Test Plan
- Compile/reload Game scripts without BS5 errors.
- Wall test: fire near a house wall/fence/car, get `source=slapback_wall`, 1-2 close reflections.
- Alley test: fire between two hard walls, get up to 2 wall slapbacks, not 4.
- Trench test: fire in trench/ditch with close left/right earth walls, get `source=slapback_trench`.
- Open field: no nearby hit, no slapback candidates.
- Burst test: slapbacks stay under global limiter and do not starve tail pool.

## Defaults
- Slapback enabled in base weapon config.
- Max slapback emitters per shot: 2.
- Wall scan radius: 25 m.
- Anchor fallback max distance: 25 m and always capped by wall scan radius.
- Trench side max distance: 4.5 m.
- Trench min confidence: 0.62.
- Wall delay clamp: 0.006-0.055 s.
- Trench delay clamp: 0.004-0.030 s.

## Implementation Notes
- Wall mode is active through the existing slapback prefab/ACP.
- Trench mode is script-ready and source-tagged as `slapback_trench`.
- Until a dedicated trench helper prefab/ACP is assigned, trench falls back to the normal slapback helper to avoid broken playback.
- ACP graph can read `BS5_SlapbackMode`: `1 = wall`, `2 = trench`.
- ACP graph can read slapback distance signals for extra volume/filter attenuation.
