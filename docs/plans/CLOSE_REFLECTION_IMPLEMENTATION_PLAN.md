# Close Reflection Implementation Plan

Source basis: `docs/research/CLOSE_REFLECTION_RESEARCH.md`

## Goal

Внедрить отдельный close reflection / close reverb модуль как часть существующей slapback-системы, без создания второй системы анализа, кэшей, emitter flow или limiter flow.

## Non-goals

- не переписывать текущий tail planner;
- не делать отдельный weapon-side driver component;
- не добавлять новый entity query subsystem;
- не плодить отдельный per-mode cache;
- не менять общую архитектуру emission service;
- не трогать trench mode, кроме необходимой защиты от конфликтов.

## Design Decisions Locked Before Coding

### 1. Close reflection lives inside the current slapback pipeline

Точка входа остается прежней:

- `BS5_EchoDriverComponent.HandleWeaponFire(...)`
- `BS5_EchoRuntime.AnalyzeShot(...)`
- `BS5_EchoRuntime.EmitShot(...)`

Новый модуль встраивается в `CollectSlapbackCandidates(...)`, а не в отдельный fire pipeline.

### 2. Close reflection is a new slapback source type, not a new candidate list

Нужен новый enum:

- `SLAPBACK_CLOSE_SPACE`

Новый mode использует тот же `result.m_aSlapbackCandidates`, а не отдельный `m_aCloseCandidates`.

### 3. Close mode is dominant and single-emitter in phase 1

Если close reflection подтвержден, он:

- формирует максимум один candidate;
- не наслаивается поверх обычного wall slapback в том же shot;
- проходит через existing slapback emit caps.

### 4. Rescue scan is conditional, not default-primary

Короткие rays нужны только как rescue path, а не как первичная новая система.

### 5. Suppressed fallback stays on existing suppressed slapback assets in phase 1

В первом проходе:

- unsuppressed close mode получает отдельный ACP/prefab;
- suppressed close mode fallback-ится в existing suppressed slapback routing, если не появится явная необходимость в отдельном suppressed-close asset set.

Это уменьшает объем интеграции и не раздувает asset matrix раньше времени.

## Files and Planned Changes

### New file

- `Scripts/Game/BS5_CloseReflectionPlanner.c`

Responsibility:

- evaluate close-space scenario on top of current slapback evidence;
- optionally run tiny rescue scan;
- synthesize one `SLAPBACK_CLOSE_SPACE` candidate;
- return accept/reject plus debug data;
- own no cache and no spawn logic.

### Existing files to change

- `Scripts/Game/BS5_EchoTypes.c`
  - add `SLAPBACK_CLOSE_SPACE`
  - extend candidate source name mapping
  - add minimal debug/result fields for close mode

- `Scripts/Game/BS5_EchoRuntime.c`
  - call the new planner from inside `CollectSlapbackCandidates(...)`
  - reuse existing wall/entity/trench data
  - insert or replace final slapback candidate selection when close mode wins
  - keep all existing limiter/emission code paths unchanged

- `Scripts/Game/BS5_EchoDriverComponent.c`
  - add close-mode ACP path attribute
  - add close-mode emitter prefab attribute
  - extend `ResolveSlapbackAcp(...)`
  - extend `ResolveSlapbackEmitterPrefab(...)`
  - keep generic event routing unless ACP authoring proves a dedicated event is required

- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
  - extend `GetSlapbackModeSignal(...)` so close mode gets a distinct numeric value

- `Prefabs/Weapons/Core/Weapon_Base.et`
  - wire the new close ACP/prefab attributes on the driver

- `Prefabs/Props/BS5_CloseReverbEmitter.et`
  - new emitter prefab with `BS5_SpatialSoundEmitterComponent`, `SoundComponent`, `SignalsManagerComponent`

- `Sounds/Weapons/Rifles/BS5/Weapons_Slapbacks_Close_Master.acp`
  - new close reverb / close reflection ACP

### Optional files only if tuning proves necessary after first pass

- `Scripts/Game/BS5_PresetRegistry.c`
- `Configs/BS5/Presets/BS5_TechnicalPresets.conf`

По умолчанию эти файлы лучше не трогать в первой реализации. Rescue scan budget должен адаптироваться от уже существующих preset-driven параметров, прежде всего от `GetMaxTraceCount()`.

## Planned Runtime Shape

### New source routing

В `BS5_EchoTypes.c`:

- добавить `SLAPBACK_CLOSE_SPACE`;
- обновить source name helper, чтобы debug показывал `slapback_close`.

### New result/debug fields

Добавить в `BS5_EchoAnalysisResult`:

- `m_fSlapbackCloseScore`
- `m_iCloseReflectionRescueRayCount`

При необходимости также:

- `m_bCloseReflectionAccepted`

Но это поле не обязательно, если достаточно `m_sSlapbackMode == "close"`.

### New driver attributes

Минимальный phase-1 набор:

- `m_sCloseSlapbackAcp`
- `m_sCloseSlapbackEmitterPrefab`

Опциональный content-tuning набор, только если без него неудобно настраивать:

- `m_fCloseReflectionMaxDistanceMeters`
- `m_fCloseReflectionMinScore`

Что не добавлять сейчас:

- отдельный close cooldown timer;
- отдельный close cache;
- большой close-only preset schema.

## Implementation Steps

### Step 1. Extend types and routing foundation

Files:

- `Scripts/Game/BS5_EchoTypes.c`
- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`

Tasks:

1. Добавить `SLAPBACK_CLOSE_SPACE`.
2. Добавить source-name mapping для debug.
3. Добавить driver-side ACP/prefab routing для close source type.
4. Добавить distinct `BS5_SlapbackMode` signal value для close source.
5. Не трогать emit queue/limiter architecture.

Done criteria:

- codebase знает новый source type;
- driver умеет вернуть close ACP/prefab;
- emitter signal contract различает close mode.

### Step 2. Add the close planner module

File:

- `Scripts/Game/BS5_CloseReflectionPlanner.c`

Tasks:

1. Создать отдельный planner class/module.
2. Вынести туда close-space evaluation, чтобы не раздувать `BS5_EchoRuntime.c`.
3. Сконструировать API planner так, чтобы он принимал уже существующие analysis/slapback inputs:
   - `BS5_EchoDriverComponent settings`
   - `BS5_EchoAnalysisResult result`
   - `origin`
   - `probeOrigin`
   - `flatForward`
   - `flatRight`
   - current `wallCandidates`
   - `trenchCandidate`
   - `traceExcludeArray` / `traceExcludeRoot`

4. Planner не должен сам:
   - запускать emit;
   - писать в cache store;
   - делать sphere query.

Done criteria:

- planner автономен как модуль;
- planner не зависит от нового внешнего pipeline;
- planner работает как helper inside existing runtime.

### Step 3. Implement reuse-first close evidence evaluation

Primary inputs:

- `wallCandidates`
- `result.m_fNearConfinement`
- `result.m_fIndoorScore`
- `result.m_fHardSurfaceScore`
- `result.m_fFrontConfinement`
- `result.m_fBackConfinement`
- `result.m_fLeftConfinement`
- `result.m_fRightConfinement`

Rules:

1. Сначала искать pair-based enclosure evidence:
   - left/right pair;
   - front/back pair;
   - very close side pair between hard objects.
2. Single close reflector сам по себе close mode не активирует.
3. Indoor score и room signals только усиливают решение, но не заменяют physical evidence.
4. Если trench score уверенный или геометрия earthwork-like, close mode reject.

Done criteria:

- planner умеет принять решение без rescue rays в очевидных cases типа "между двумя машинами" или "между двумя стенами";
- planner не срабатывает просто рядом с одним объектом.

### Step 4. Add the conditional rescue scan

Rescue scan policy:

- запускать только если есть near-close evidence, но не хватает второго подтверждения;
- использовать 1 обязательный upward ray;
- добирать 0-3 дополнительных rays только по ситуации;
- количество rescue rays адаптировать от текущего trace budget, а не от нового большого config surface.

Rules:

1. No `QueryEntitiesBySphere`.
2. Reuse current trace exclusion.
3. Reuse existing prefab/shape acceptance helper.
4. Для roof/overhang accept path использовать отдельную normal heuristic.
5. Rescue scan не должен запускаться в каждом shot по умолчанию.

Done criteria:

- навесы и low-overhead semi-closed cases начинают определяться;
- открытое пространство не получает массовых false positive.

### Step 5. Build the synthetic close candidate

Implementation target:

- один synthetic `SLAPBACK_CLOSE_SPACE` candidate

Rules:

1. Позицию строить из contributing hits, а не просто брать лучший wall hit.
2. В симметричном pocket держать `panBias` ближе к центру.
3. Delay считать через текущую slapback distance/path math.
4. Candidate должен проходить через существующий `QueueEmission(...)` и limiter path без особой ветки.

Done criteria:

- close mode звучит как local pocket reflection, а не как еще одна обычная wall slapback-точка;
- в emit loop нет отдельного close-only spawn path.

### Step 6. Integrate planner into `CollectSlapbackCandidates(...)`

File:

- `Scripts/Game/BS5_EchoRuntime.c`

Integration order:

1. current wall trace pass
2. current entity candidate pass
3. current trench candidate build
4. close planner evaluation
5. final winner selection

Selection rule:

- if close planner accepts:
  - close candidate becomes dominant slapback result;
  - regular wall candidates do not stack on top in the same shot;
  - trench still wins if trench criteria clearly dominate.

Debug/result updates:

- `m_sSlapbackMode = "close"` when accepted;
- update `m_fSlapbackCloseScore`;
- include rescue-ray count in result/debug.

Done criteria:

- current non-close slapback behavior stays unchanged when planner rejects;
- close mode only changes final candidate selection when it actually wins.

### Step 7. Add assets and wire prefabs

Files:

- `Prefabs/Props/BS5_CloseReverbEmitter.et`
- `Sounds/Weapons/Rifles/BS5/Weapons_Slapbacks_Close_Master.acp`
- `Prefabs/Weapons/Core/Weapon_Base.et`

Emitter prefab shape:

- `BS5_SpatialSoundEmitterComponent`
- `SoundComponent`
- `SignalsManagerComponent`

ACP expectations:

- reuse existing BS5 slapback signal contract;
- consume `BS5_UserSlapbackVolume.sig`;
- respond to `BS5_SlapbackMode`, `BS5_DelaySeconds`, `BS5_ReverbSend`, `BS5_SurfaceHardness`, indoor/room signals;
- voice should read as ultra-close reflection/reverb pocket, not as distant tail.

Wiring rules:

- weapon prefab points close source type to the new emitter/acp;
- generic slapback remains fallback if close resources are absent.

Done criteria:

- close source type resolves to the new assets;
- normal wall/trench assets remain untouched.

### Step 8. Verification and regression pass

Verification areas:

1. Functional
2. Acoustic
3. Performance
4. Regression

Functional matrix:

- between two close walls
- between garages
- between two vehicles / BTR
- under canopy
- narrow passage
- open field near one wall
- trench / ditch
- small indoor-like zone
- suppressed shot fallback

Performance checks:

- compare `profile analyze` and `profile shot` before/after in representative scenes;
- confirm no extra sphere query path exists;
- confirm rescue scan count stays low and conditional;
- confirm cached repeat shots still reuse the same analysis result.

Regression checks:

- ordinary wall slapback unchanged in open urban edge cases;
- trench mode still routes to trench assets;
- active slapback emitter cap still respected;
- start gate and pending queue still govern close mode because it rides the same path.

Done criteria:

- no new systemic spike in analyze cost;
- no duplicate emitters from mixed wall + close mode on one shot;
- no trench breakage;
- no obvious false-positive close wash in open space.

## Acceptance Criteria

Реализация считается готовой, когда выполнены все условия ниже.

### Architecture

- close reflection сделан отдельным модулем, а не распуханием main slapback logic по всему runtime;
- нет новой параллельной cache/emission system;
- нет новой sphere-query subsystem.

### Reuse

- используются текущие slapback candidate data;
- используется текущий `BS5_EchoAnalysisResult`;
- используется текущий emit/limiter/resource-cache pipeline;
- используется текущий spatial signal contract.

### Behavior

- close mode срабатывает в тесных semi-enclosed/hard-object случаях;
- не срабатывает от одного случайного объекта в поле;
- не ломает trench;
- не ломает suppressed fallback path.

### Performance

- additional ray cost небольшой и conditional;
- no duplicate broad queries;
- cache reuse retained.

### Content

- существует `BS5_CloseReverbEmitter.et`;
- существует отдельный close ACP;
- weapon driver может быть явно настроен на close assets.

## Recommended Coding Order

Практически безопасный порядок следующего шага:

1. `BS5_EchoTypes.c`
2. `BS5_EchoDriverComponent.c`
3. `BS5_SpatialSoundEmitterComponent.c`
4. `BS5_CloseReflectionPlanner.c`
5. `BS5_EchoRuntime.c`
6. `BS5_CloseReverbEmitter.et`
7. `Weapons_Slapbacks_Close_Master.acp`
8. `Weapon_Base.et`
9. verification

## Notes for the Implementation Pass

- Не менять current cached-result contract.
- Не дублировать `CollectSlapbackEntityCandidates(...)`.
- Не делать "еще один mini-driver".
- Не уводить close mode в tail planner.
- Если в процессе coding выяснится, что нужен отдельный close event name, добавить его только после проверки, что generic `SOUND_SHOT` действительно недостаточен.
- Если tuning станет неудобным без extra fields, сначала поднять только 1-2 самых нужных параметра, а не сразу расширять весь preset schema.

## Ready State for Next Step

После этого плана следующий шаг может идти уже в implementation mode без повторного большого исследования, потому что зафиксированы:

- реальные точки интеграции;
- reusable части текущей slapback-системы;
- ограничения текущего runtime;
- выбранный рекомендуемый вариант;
- состав файлов;
- порядок внедрения;
- критерии проверки и готовности.
