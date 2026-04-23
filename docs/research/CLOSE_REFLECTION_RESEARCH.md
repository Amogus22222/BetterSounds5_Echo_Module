# Close Reflection / Close Reverb Research

## Scope

Цель исследования: спроектировать отдельный модуль close reflection / close slapback-reverb для `bettersounds5`, не создавая вторую параллельную систему анализа, кэшей, emitter flow или raycast-пайплайн. Требование проекта: новый сценарий должен максимально переиспользовать текущую slapback-архитектуру, а не дублировать ее.

Ниже выводы привязаны к реальным файлам и символам репозитория.

## Studied Sources

### Repository

- `README.md`
- `MODPLAN.md`
- `docs/reference/PROJECT_CONTEXT.md`
- `docs/reference/WORKBENCH_AND_MCP.md`
- `docs/plans/SLAPBACK_EXPANSION_PLAN.md`
- `docs/audits/CODEX_AUDIT_REVIEW.md`
- `docs/audits/CODEX_FIX_IMPLEMENTATION_PLAN.md`
- `docs/audits/CODEX_OPEN_QUESTIONS.md`
- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_EchoTypes.c`
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
- `Scripts/Game/BS5_PresetRegistry.c`
- `Configs/BS5/Presets/BS5_TechnicalPresets.conf`
- `Prefabs/Weapons/Core/Weapon_Base.et`
- `Prefabs/Props/BS5_SlapbackEmitter.et`
- `Prefabs/Props/BS5_SlapbackEmitter_Silenced.et`
- `Prefabs/Props/BS5_SlapbackEmitter_Trench.et`
- `Sounds/_SharedData/Signals/BS5/BS5_UserSlapbackVolume.sig`
- existing slapback ACP variants in `Sounds/Weapons/Rifles/BS5/`

### Enfusion MCP / knowledge base basis

Проверены актуальные материалы Enfusion MCP по темам:

- `Audio System`
- `World Environment`
- `Scripting Best Practices`
- `Resource Usage`
- `Entity Lifecycle`

Также сверены wiki/API-ориентиры по `SoundComponent`, signal-driven ACP playback, `QueryEntitiesBySphere`, resource lifetime и spatial audio flow.

### Practical API conclusions

- Текущий путь через `SoundComponent` + `SignalsManagerComponent` + `.acp` на helper-emitter prefab является правильным и совместимым с движком для динамических spatial one-shot reflection событий.
- `QueryEntitiesBySphere` и raycast-сканы надо держать дешевыми и строго gated; отдельный второй entity-query контур для close reflection будет плохим решением.
- Кэш `Resource` для emitter prefab уже реализован корректно в runtime и должен переиспользоваться, а не дублироваться.
- Любое новое close reflection решение должно жить внутри текущего analysis/emission pipeline, чтобы автоматически получать benefit от уже существующих limiter/cooldown/cache механизмов.

## Current Slapback Architecture

### Entry and analysis flow

Текущий путь выстрела:

1. `SCR_MuzzleEffectComponent.OnFired`
2. `BS5_EchoRuntime.FindDriver(...)`
3. `BS5_EchoDriverComponent.HandleWeaponFire(...)`
4. `BS5_EchoRuntime.AnalyzeShot(...)`
5. `BS5_EchoRuntime.EmitShot(...)`

Ключевые точки:

- `Scripts/Game/BS5_EchoDriverComponent.c`
  - `HandleWeaponFire(...)`
  - `ResolveCachedResult(...)`
  - `StoreCachedResult(...)`
- `Scripts/Game/BS5_EchoRuntime.c`
  - `BS5_EchoEnvironmentAnalyzer.Analyze(...)`
  - `CollectSlapbackCandidates(...)`
  - `BS5_EchoEmissionService.Emit(...)`
  - `QueueEmission(...)`
  - `EmitPending(...)`

Это уже единый пайплайн: driver -> analyze -> ranked candidates -> shared emit/limiter flow.

### Current analysis result object

`Scripts/Game/BS5_EchoTypes.c`:

- `BS5_EchoAnalysisResult`
- `BS5_EchoReflectorCandidate`
- `BS5_EchoCandidateSourceType`

Уже существуют нужные для reuse поля:

- `m_fNearConfinement`
- `m_fVerticalConfinement`
- `m_fFrontConfinement`
- `m_fBackConfinement`
- `m_fLeftConfinement`
- `m_fRightConfinement`
- `m_fIndoorScore`
- `m_fHardSurfaceScore`
- `m_fRoomSize`
- `m_sSlapbackMode`
- `m_aSlapbackCandidates`

Вывод: close reflection не требует отдельного result-контейнера. Его надо встроить в уже существующий `BS5_EchoAnalysisResult`.

### Current near slapback collection

Основная логика находится в `BS5_EchoRuntime.c -> CollectSlapbackCandidates(...)`.

Что уже делает текущая система:

- строит отдельный near-field trace fan для slapback;
- собирает wall candidates;
- собирает entity-based slapback candidates через `CollectSlapbackEntityCandidates(...)`;
- умеет различать wall и trench;
- умеет строить trench candidate;
- умеет fallback-нуться к anchor-based варианту;
- ранжирует кандидаты и пишет итог в `result.m_aSlapbackCandidates`.

Отдельно важны helper-функции:

- `BuildSlapbackEntityCandidate(...)`
- `ResolveSlapbackEntityPrefabScore(...)`
- `IsSlapbackEntityShapeAccepted(...)`
- `BuildSlapbackCandidate(...)`
- `BuildTrenchSlapbackCandidate(...)`

### Existing emitter / ACP routing

`BS5_EchoDriverComponent.c` уже умеет source-type-based routing:

- `ResolveSlapbackAcp(...)`
- `ResolveSlapbackEmitterPrefab(...)`
- `ResolveSlapbackEventName(...)`

Текущие `sourceType`-ветки:

- `SLAPBACK_WALL`
- `SLAPBACK_TRENCH`

Вывод: close reflection логично встраивается как новый `sourceType`, а не как отдельный emission subsystem.

### Existing emission, anti-spam and resource control

`BS5_EchoRuntime.c -> BS5_EchoEmissionService` уже содержит:

- pending reservation;
- playback admission;
- rolling start gate;
- global voice caps;
- per-owner fairness caps;
- active emitter budgets;
- emitter prefab resource cache.

Ключевые функции:

- `ReservePendingVoice(...)`
- `TryAdmitPlaybackVoice(...)`
- `TryEnterStartGate(...)`
- `ResolveEmitterPrefabResource(...)`
- `EnsureEmitterResourceCache(...)`

Вывод: новый close reflection mode должен проходить ровно через этот же поток. Добавлять новый cooldown, новый pending queue, новый emitter budget manager или новый resource cache нельзя.

### Existing spatial signal contract

`Scripts/Game/BS5_SpatialSoundEmitterComponent.c -> Play(...)` уже отправляет в ACP большой набор сигналов, включая:

- `BS5_IsSlapback`
- `BS5_SlapbackMode`
- `BS5_DelaySeconds`
- `BS5_SlapbackDelaySeconds`
- `BS5_SlapbackDistanceMeters`
- `BS5_SlapbackDistanceNorm`
- `BS5_SlapbackDistanceGain`
- `BS5_ReverbSend`
- `BS5_TailWidth`
- `BS5_TailBrightness`
- `BS5_SurfaceHardness`
- `Interior`
- `RoomSize`
- `BS5_IndoorBias`
- directional confinement/env signals

`GetSlapbackModeSignal(...)` уже различает `wall` и `trench`.

Вывод: новый close ACP можно повесить на уже существующий signal contract. Новая plumbing-система сигналов не нужна.

## Reusable Points

### 1. Full analysis cache in driver

`BS5_EchoDriverComponent.ResolveCachedResult(...)` / `StoreCachedResult(...)` кэшируют весь `BS5_EchoAnalysisResult`.

Это главный reusable point. Если close reflection живет внутри текущего analysis path, он автоматически получает cache reuse. Если делать отдельный модуль с отдельным cache key, это будет уже вторая система.

### 2. Existing slapback wall and entity evidence

Текущая slapback-система уже собирает большую часть нужных close-space сигналов:

- очень близкие стены;
- близкие гаражи;
- контейнеры;
- машины;
- `vehicle`, `car`, `truck`, `m998`, `apc`, `btr`, `wreck`;
- узкие боковые поверхности;
- prefab/shape-based hard-surface filtering.

То есть примеры из задачи уже во многом покрываются текущими данными. Нужно не писать новый детектор с нуля, а правильно переиспользовать уже собранные `wallCandidates`.

### 3. Existing source-type-dependent asset routing

Добавление нового close mode через новый `BS5_EchoCandidateSourceType` позволит:

- использовать отдельный ACP;
- использовать отдельный emitter prefab;
- не трогать общий emit flow;
- не плодить отдельный spawn API.

### 4. Existing limiter/start-gate/active-emitter protections

Для anti-overtrigger уже есть готовый foundation:

- `m_iLimiterMaxSlapbackStartsPer100Ms`
- global slapback voice cap
- per-owner active slapback emitter cap
- pending queue cap

Close reflection должен участвовать в тех же лимитах, а не получать собственную независимую частотную политику.

## Important Constraints and Gaps Found

### 1. Current vertical confinement is effectively blind

В `BS5_EchoRuntime.c -> Analyze(...)` массив `directions` полностью горизонтальный:

- `flatForward`
- `back`
- `flatRight`
- `left`
- горизонтальные диагонали

Дальше код считает `verticalWeight = abs(direction[1])`, но у этих направлений `y = 0`, поэтому:

- `ceilingHits` фактически не набирается;
- `m_fVerticalConfinement` практически всегда слабый;
- текущий near analysis плохо видит навесы, потолки и очень близкий overhead cover.

Это прямой аргумент против варианта "только использовать уже имеющиеся aggregate scores". Для close reflection нужен хотя бы минимальный rescue path для overhead detection.

### 2. Current slapback collector internally clamps emitters to 2

В `CollectSlapbackCandidates(...)`:

- `maxCandidates = settings.GetMaxSlapbackEmittersPerShot();`
- затем `Math.Clamp(maxCandidates, 1, 2);`

При этом:

- `Weapon_Base.et` ставит `m_iMaxSlapbackEmittersPerShot 3`
- technical presets местами задают `4` и `5`

Практический вывод: runtime сейчас все равно работает как максимум `2` slapback-кандидата. Новый close mode надо проектировать под это ограничение, а не рассчитывать на большое число параллельных close emitters.

### 3. Authored wall delay max is wider than runtime-effective clamp

`Weapon_Base.et` задает:

- `m_fSlapbackWallDelayMaxSeconds 0.3`

Но `BS5_EchoDriverComponent.GetSlapbackWallDelayMaxSeconds()` режет это до `0.12`.

Вывод: для close reflection не нужен отдельный длинный delay-path. Наоборот, очень близкий mode должен оставаться коротким и может спокойно жить на текущей slapback delay math, если ограничить effective distance.

### 4. No separate slapback cache should be added

Отдельного slapback-specific cache поверх `BS5_EchoAnalysisResult` сейчас нет. И это хорошо. Новый модуль не должен приносить:

- новый cache map;
- новый last-result snapshot;
- новый per-shot reuse store;
- новый entity query cache.

### 5. Current wall normal gate is intentionally hostile to ceilings

Текущий wall collector отбрасывает hit, если `abs(hitNormal[1]) > GetSlapbackWallNormalMaxY()`.

Это правильно для wall slapback, но плохо для overhead close reflection. Значит:

- reuse acceptance helper-функций нужен;
- reuse wall-normal filter нужен не полностью;
- close reflection planner должен иметь отдельную roof/overhang normal heuristic.

Это еще один аргумент за отдельный planner module, а не за размазывание логики по текущему wall collector.

## Option Set

### Option A. Existing-result-only close detection

Идея:

- использовать только `m_fNearConfinement`, `m_fIndoorScore`, `m_fHardSurfaceScore`, `m_fFront/Back/Left/RightConfinement` и уже собранные slapback candidates;
- без новых raycast.

Плюсы:

- самый дешевый по CPU;
- нулевой новый trace budget;
- идеально reuse-ит текущий cache.

Минусы:

- плохо видит навесы/потолки/overhead cover;
- хуже отличает "одна рядом стоящая машина" от "узкое полузамкнутое пространство";
- дает больше false negative в полу-закрытых местах.

Вердикт:

- годится как base gate;
- не годится как единственный close-space detector.

### Option B. Adapt current tail / SoundMap signals for close reverb

Идея:

- пытаться вывести close reflection из tail planner, SoundMap и env snapshot.

Плюсы:

- reuse существующей дальней акустической модели;
- почти не добавляет trace cost.

Минусы:

- tail planner проектировался под десятки и сотни метров, а не под 0.5-2 м;
- плохо покрывает тесные пространства между объектами и под навесом;
- высокий риск странной слуховой логики: дальние signals не обязаны означать сверхблизкий reflection pocket.

Вердикт:

- как secondary bias допустимо;
- как основа close reverb - плохой fit.

### Option C. Fully separate near-field scanner and emitter subsystem

Идея:

- отдельный компонент/модуль со своим mini-scan;
- свои 3-6 rays;
- своя candidate list;
- свой cache;
- свой spawn routing.

Плюсы:

- максимальная свобода настройки close mode;
- можно добиться хорошей локальной детекции.

Минусы:

- это именно "вторая система";
- дублирует близкие геометрические расчеты;
- дублирует data access и часть filtering logic;
- дублирует performance cost;
- повышает риск расхождения поведения wall slapback и close reverb;
- выше риск регрессий в emit flow.

Вердикт:

- архитектурно не рекомендован.

### Option D. Hybrid reuse-first planner with conditional ultra-short rescue scan

Идея:

- новый отдельный planner module;
- сначала использовать уже собранные slapback wall/entity candidates и текущий `BS5_EchoAnalysisResult`;
- только если данных почти хватает, но не хватает подтверждения close-space pocket, делать микроскан 1-4 очень короткими лучами;
- использовать существующий emit path, cache и limiter, а не создавать новые.

Плюсы:

- лучший баланс качества и CPU;
- reuse текущей slapback-системы максимальный;
- overhead/under-canopy кейсы закрываются rescue scan;
- не нужна вторая entity query система;
- не нужна вторая emitter/queue/cache система.

Минусы:

- чуть сложнее, чем "просто использовать result";
- нужно аккуратно формализовать trigger gates, чтобы не получить overtrigger рядом с одним объектом.

Вердикт:

- это рекомендуемый вариант.

## Option Comparison

| Option | Performance | Stability | Sound quality | Regression risk | Integration volume | Reuse level |
| --- | --- | --- | --- | --- | --- | --- |
| A. Existing-result-only | Very high | High | Medium | Low | Low | High |
| B. Tail/SoundMap adaptation | High | Medium | Low/Medium | Medium | Medium | Medium |
| C. Separate second system | Low/Medium | Medium/Low | High potential, but inconsistent | High | High | Low |
| D. Hybrid reuse + rescue scan | High | High | High | Low/Medium | Medium | Very high |

## Recommended Architecture

### Recommendation

Рекомендован `Option D`: отдельный planner module, встроенный в текущий slapback analysis path.

### Proposed module shape

Новый файл:

- `Scripts/Game/BS5_CloseReflectionPlanner.c`

Его ответственность:

- не запускать отдельный fire hook;
- не иметь свой cache;
- не иметь свой emitter spawn flow;
- не иметь свой entity query pipeline;
- оценивать close-space scenario поверх уже существующих slapback данных;
- при необходимости делать только маленький rescue scan;
- возвращать максимум один synthetic close candidate.

### Why the user's initial ray idea is not ideal as a primary system

Идея "3-6 коротких лучей 0.5-2 м" сама по себе не плохая, но как primary solution она слишком изолированная:

- она не переиспользует уже собранные slapback wall/entity hits;
- она превращается во вторую маленькую систему;
- она начнет дублировать то, что slapback collector уже делает.

Правильнее использовать эти короткие rays не как основу, а как conditional rescue scan поверх уже имеющихся данных.

### Recommended integration point

Лучшее место интеграции: `BS5_EchoRuntime.c -> CollectSlapbackCandidates(...)`.

Причина:

- здесь уже собраны `wallCandidates`;
- здесь уже известны `trenchCandidate`, `bestWallScore`, `entityHits`;
- здесь еще не завершен final winner selection;
- здесь можно подменить или вставить close candidate без нового pipeline.

### Recommended data flow

1. Текущий slapback collector строит обычные wall/entity candidates.
2. Текущий trench logic строит trench candidate.
3. Новый `BS5_CloseReflectionPlanner` оценивает, есть ли close-space pocket.
4. Если strong close mode подтвержден:
   - он создает один `SLAPBACK_CLOSE_SPACE` candidate;
   - этот candidate становится dominant slapback result;
   - обычный wall slapback не дублируется поверх него в том же выстреле.
5. Если close mode не подтвержден:
   - текущая slapback логика продолжает работать без изменений.

### Why close mode should be dominant, not additive

Если close reverb поверх сильного wall slapback еще и добавлять обычный wall slapback, высокий риск:

- лишней "двойной мокроты";
- размывания transient;
- спама в burst fire;
- конфликта с current voice caps.

Поэтому в первом дизайне close mode должен быть exclusive dominant mode: максимум один close candidate, без параллельной второй wall-пары.

## Recommended Detection Logic

### Stage 1. Reuse current evidence first

Planner сначала должен использовать уже имеющееся:

- `result.m_fNearConfinement`
- `result.m_fIndoorScore`
- `result.m_fHardSurfaceScore`
- `result.m_fFrontConfinement`
- `result.m_fBackConfinement`
- `result.m_fLeftConfinement`
- `result.m_fRightConfinement`
- `result.m_aSlapbackCandidates`
- локальный `wallCandidates` список до финального merge

Особенно ценно:

- пара близких hard candidates слева и справа;
- очень близкий front/back pair;
- близкие entity hits от машин/БТР/гаражей;
- высокая near confinement при hard-surface signal, но без trench characteristics.

### Stage 2. Use a tiny rescue scan only when needed

Rescue scan нужен не всегда, а только если есть "almost enough evidence".

Рекомендуемый порядок:

1. Upward ray
2. Optional left/right ray only if one бок уже подтвержден и нужен парный cue
3. Optional back ray only для very narrow passage/corridor cases

Практический смысл:

- `upward` ray закрывает навесы, потолки, низкие крыши, верх машины, перекрытие узкого прохода;
- `left/right` ray добирает симметрию, если обычный collector поймал только одну сторону;
- дополнительный `front` ray не нужен как default, потому что forward evidence уже есть в существующем slapback fan.

Важно:

- rescue path не должен запускать `QueryEntitiesBySphere`;
- rescue path должен reuse-ить текущий trace exclude root/array;
- для rescue path можно reuse-ить `IsSlapbackWallTraceAccepted(...)` как prefab/shape filter;
- но normal heuristic для roof hits должна быть отдельной, потому что current wall collector режет потолочные нормали намеренно.

### Trigger philosophy

Close mode должен требовать минимум два независимых cues, а не один.

Хорошие trigger patterns:

- left + right close hard reflectors;
- left/right pair + overhead cover;
- front/back pair в очень узком зазоре;
- very close hard reflector + overhead cover + high near confinement.

Плохой trigger pattern:

- просто один близкий объект справа в чистом поле;
- просто один гараж впереди;
- только `m_fIndoorScore` без physical local evidence;
- trench-like geometry.

### Candidate synthesis

Close reflection candidate лучше делать synthetic, а не просто переиспользовать лучший wall hit как есть.

Причина:

- close reverb в полузамкнутом pocket чаще воспринимается как centered local wash, а не как один точечный hard wall slap;
- midpoint/average по contributing hits звучит стабильнее;
- так проще сделать одну dominant close-emission без лишней панорамной дерготни.

Рекомендуемая логика:

- при left/right pair брать midpoint между contributing hit positions;
- при wall + roof брать усредненную позицию с небольшим forward bias;
- `panBias` держать ближе к `0`, если pocket симметричен;
- delay считать через текущую slapback delay math, но на очень короткой effective distance.

### Interaction with trench mode

Trench не должен ломаться.

Правило:

- если trench candidate уверенный и геометрия явно trench/ditch/earthwork-like, trench имеет приоритет;
- close mode работает только для hard/vehicle/semi-enclosed pocket cases;
- close mode не должен подменять trench ambience на hard close reverb.

## New Runtime Shape

### New source type

Рекомендовано добавить новый enum entry в `BS5_EchoTypes.c`:

- `SLAPBACK_CLOSE_SPACE`

Зачем:

- reuse current `m_aSlapbackCandidates`;
- reuse current emit loop;
- получить source-based ACP/emitter routing;
- отличать mode в debug и signal contract.

### New debug/result fields

Рекомендуется добавить в `BS5_EchoAnalysisResult` минимум:

- `m_fSlapbackCloseScore`
- `m_iCloseReflectionRescueRayCount`

Это не обязательные gameplay fields, но полезные для:

- tuning;
- regression checks;
- понимания, когда close mode сработал за счет reuse-only path, а когда за счет rescue ray.

### ACP and emitter path

Новые ресурсы:

- `Prefabs/Props/BS5_CloseReverbEmitter.et`
- `Sounds/Weapons/Rifles/BS5/Weapons_Slapbacks_Close_Master.acp`

Маршрутизация должна идти через существующие driver методы, а не через новый spawn API.

## Configuration Strategy

### What should be configurable now

В первом проходе достаточно добавить в `BS5_EchoDriverComponent`:

- отдельный ACP path для close mode;
- отдельный emitter prefab path для close mode;
- небольшой набор tuning values только если они реально нужны для контентной настройки.

Минимально полезный tuning set:

- `CloseReflectionMaxDistanceMeters`
- `CloseReflectionMinScore`

### What should not be added in phase 1

Не стоит сразу добавлять новый большой preset/schema блок с десятком close-specific полей.

Причина:

- performance budget уже регулируется существующими preset-полями;
- rescue scan можно делать adaptive от текущего `GetMaxTraceCount()`;
- иначе close mode начнет раздувать config surface раньше, чем появится проверенная слуховая модель.

## Risks and Mitigations

### Risk: false positives near a single object

Сценарий:

- рядом одна машина или одна стена, но пространство открытое.

Mitigation:

- минимум 2 independent cues;
- single reflector сам по себе не должен активировать close mode;
- indoor score использовать только как booster, не как trigger.

### Risk: under-canopy false positives on foliage

Сценарий:

- overhead ray бьет в дерево/куст/листву.

Mitigation:

- reuse prefab reject hints через existing acceptance helper;
- vegetation/tree hits не должны подтверждать close hard reflection.

### Risk: trench regression

Сценарий:

- боковые стенки окопа будут ошибочно классифицироваться как close hard pocket.

Mitigation:

- trench candidate evaluation выполнять до close acceptance;
- trench score/earthwork hints иметь priority reject для close mode.

### Risk: overtrigger in full-auto

Сценарий:

- close mode в тесном проходе срабатывает на каждый shot.

Mitigation:

- reuse current slapback limiter;
- не добавлять второй emit path;
- close mode делать single dominant candidate;
- не плодить дополнительную per-shot layer stack.

### Risk: duplicated scans and wasted CPU

Сценарий:

- новый модуль повторно читает те же объекты и делает еще одну sphere query.

Mitigation:

- reuse existing `wallCandidates`;
- rescue scan only if ambiguity remains;
- no new `QueryEntitiesBySphere`;
- no new cache/store layer.

## Edge Cases to Cover in Implementation

- между двумя близкими стенами;
- между гаражами;
- между двумя машинами/БТР;
- под навесом;
- узкий проход;
- маленькая полу-закрытая техническая зона;
- открытое поле рядом с одним объектом;
- trench/ditch;
- уже сильный indoor state с room signal;
- burst fire under limiter pressure;
- light preset.

## Final Recommendation

Лучший путь для этого репозитория:

- не делать вторую slapback-систему;
- не строить отдельный entity query subsystem;
- не строить отдельный cache/emitter flow;
- сделать новый `BS5_CloseReflectionPlanner` как отдельный модуль;
- встроить его внутрь текущего `CollectSlapbackCandidates(...)`;
- сначала reuse-ить уже собранные slapback данные;
- только потом, по условию, добирать 1-4 ultra-short rescue rays;
- возвращать максимум один `SLAPBACK_CLOSE_SPACE` candidate;
- прогонять его через существующий slapback emission/limiter/resource path;
- маршрутизировать на новый `BS5_CloseReverbEmitter.et` и отдельный close ACP.

Это дает лучший баланс:

- производительность;
- архитектурная чистота;
- качество на слух;
- низкий риск поломки существующего slapback flow.
