# Slapback Expansion Plan

## Intent

Extend slapback behavior for specific close-range scenarios without turning it into another heavy world-scan system.

## Design Goals

- keep slapback local and cheap
- limit per-shot slapback count
- distinguish hard wall/alley reflections from trench-like close-side reflections
- preserve stable playback through existing helper-emitter architecture

## Current Baseline

- slapback path already exists in runtime
- helper emitters and ACP assets already exist for:
  - normal slapback
  - silenced slapback
  - trench slapback
- current tail system is stable enough that slapback can be extended without reopening the whole echo architecture

## Planned Feature Buckets

### 1. Hard-Wall / Alley Slapback

- keep fixed close raycasts
- prefer hard nearby surfaces
- spatialize one or two short-delay reflections

### 2. Trench / Earth-Wall Slapback

- detect close bilateral wall geometry
- emit a distinct trench-like reflection character
- use dedicated trench emitter/ACP where configured, otherwise fall back safely

### 3. Safe Local Fallbacks

- if direct slap rays fail, allow bounded local fallback from strong nearby hard reflectors
- do not reuse far city tails as fake slapback

## Constraints

- no broad SoundMap planner rewrite for slapback
- no global heavy query pass
- no removal of existing tail logic
- keep budget behavior bounded and visible

## Validation Targets

- close wall
- alley / between two walls
- trench / ditch
- open field should produce no false slapback
- burst fire should respect limiter behavior

## Not In Scope

- large retune of the main tail planner
- spawn-order experiments from `M-06`
- trace-filter rewrite from `M-07`
