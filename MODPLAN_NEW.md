# BetterSounds5: Self-Contained Technical Specification

## 1. Назначение документа

Этот файл нужен как стартовый технический контекст для новой сессии работы над модом `BetterSounds5` без опоры на историю текущего чата.

После прочтения этого файла новый инженер/агент должен понимать:

- что именно делает мод;
- какие подсистемы уже существуют;
- какие файлы и ресурсы являются ключевыми;
- какие поведенческие цели считаются правильными;
- какие проблемы уже известны;
- что нужно проверять в первую очередь;
- по каким принципам вносить новые изменения.

Это не дневник изменений и не просто список идей. Это рабочее ТЗ и оперативная карта проекта.

### Current Status: technical presets

Live logs from `2026-04-20` showed that preset IDs switch correctly (`tech=default` / `tech=dynamic`), but several runtime values were still taken from hardcoded fallback presets in `Scripts/Game/BS5_PresetRegistry.c`, not from the tuned `.conf` table. Evidence: tail lifetime stayed `3.5`, dynamic micro-scan query cap stayed `24`, and SoundMap samples stayed near `9*7`.

Required invariant: `Configs/BS5/Presets/BS5_TechnicalPresets.conf`, `Prefabs/Weapons/Core/Weapon_Base.et`, and fallback functions `FillDefaultTechnicalPreset`, `FillLightTechnicalPreset`, `FillDynamicTechnicalPreset` must stay synchronized. When testing after reload, expected logs are:

- Default/Dynamic tail `lifetime=3.9`.
- Dynamic `soundMapSamples` can approach `11*8`.
- Dynamic `urbanMicroQueries` can approach `32`.
- Light `soundMapSamples` should be around `4*4`, with low ray/query pressure.

---

## 2. Идентичность проекта

- Проект: `BetterSounds5`
- Корень проекта: `G:\BettersMods\BetterSounds5`
- Основная задача мода: добавить в Arma Reforger управляемую, событийную, позиционную систему отражённых оружейных хвостов/эхо (`tails`), опционального `slapback`, а также переиспользуемый акустический хвост для взрывов.

Мод не должен превращаться в тяжёлую постоянную симуляцию акустики. Целевая архитектура:

- event-driven;
- максимально дешёвая в runtime;
- с кэшированием;
- с минимальным количеством spawn'ов;
- с правдоподобным выбором отражающих поверхностей;
- с приоритетом фасадов/города в релевантных сценах;
- с fallback-путями там, где явный фасад не найден.

---

## 3. Продуктовая цель мода

### 3.1. Что должен давать мод игроку

При выстреле или взрыве игрок должен слышать не просто сухой tail preset, а позиционно размещённые отражения, характер которых зависит от окружающей среды:

- открытое поле;
- холмистая местность;
- лес/лесополоса;
- населённый пункт / edge-of-settlement;
- частные дома / сараи / гаражи / магазины / другие крупные отражающие строения;
- indoor-like / room-edge ситуации.

### 3.2. Как это должно ощущаться

- В городе и у домов система должна преимущественно находить фасадные отражения, а не подменять всё terrain fallback'ом.
- На дистанциях примерно `25–300 м` при стрельбе в сторону города/домов городские отражения должны срабатывать практически всегда, если впереди действительно есть крупные постройки.
- Деревья, кусты, лесной clutter и мелкие props не должны ложно продвигаться как `facade_hit`.
- На дальних дистанциях хвосты допустимы вплоть до `800 м`, но это потолок, а не обязательная дистанция срабатывания.
- Дальние эмиттеры должны быть тише ближних.
- Глушитель должен уменьшать дистанцию и энергию хвостов, но не ломать систему полностью.
- Fallback должен быть страховкой, а не доминирующим характером сцены.

### 3.3. Что не является текущим приоритетом

- `Slapback` сейчас намеренно выключен в оружейном префабе и не является текущей активной целью доводки.
- Это не означает, что архитектура slapback не нужна. Она должна оставаться совместимой, но сейчас её не нужно расширять, если это не требуется для исправления побочного бага.

---

## 4. Основные принципы реализации

### 4.1. Архитектурный принцип

Система должна быть:

- событийной, а не `per-frame`;
- опирающейся на уже доступные сигналы движка и геометрию мира;
- устойчивой к плотной стрельбе;
- совместимой с multiplayer runtime;
- минимально инвазивной относительно vanilla weapon flow.

### 4.2. Источник правды

При дальнейшей работе источник истины такой:

1. текущие файлы проекта;
2. Enfusion API / BIKI / vanilla references;
3. затем инженерные выводы.

Нельзя выдумывать:

- сигналы ACP;
- поведение `SoundComponent`;
- контракт пользовательских настроек;
- логику Workbench / World Editor;
- API методов и событий.

---

## 5. Текущее runtime-устройство системы

## 5.1. Главный runtime-компонент оружия

Ключевой файл:

- `Scripts/Game/BS5_EchoDriverComponent.c`

Это основной контроллер, навешанный на оружейный prefab. Он:

- хранит конфиг tails/slapback/suppressor/indoor/cache/tuning;
- принимает события огня;
- строит или использует кэшированный анализ среды;
- вызывает runtime analyzer;
- рассчитывает бюджет эмиттеров;
- подготавливает pending emission contexts;
- инициирует spawn и playback.

Важные публичные зоны ответственности:

- обработка weapon fire;
- обработка explosion fire;
- хранение и чтение tuning-настроек из prefab;
- управление кэшем;
- budget / caps;
- debug summary.

## 5.2. Входные gameplay hooks

Текущие точки входа:

- `modded class SCR_MuzzleEffectComponent`
- `modded class SCR_WeaponBlastComponent`

Смысл:

- при выстреле через muzzle hook находится `BS5_EchoDriverComponent` и вызывается обработка выстрела;
- через explosion bridge возможна отдельная обработка акустического хвоста взрывов.

Текущее важное состояние:

- в этих hooks уже добавлены `#ifdef WORKBENCH` ранние выходы, чтобы Workbench не заходил в gameplay fire path;
- это сделано именно как изоляция Workbench-crash риска;
- эти guards нельзя бездумно удалять, пока проблема Workbench не локализована и не подтверждено, что startup стабилен.

## 5.3. Runtime фасад анализа и эмиссии

Ключевой файл:

- `Scripts/Game/BS5_EchoRuntime.c`

Основные обязанности:

- собрать `BS5_EchoAnalysisResult`;
- оценить окружение;
- выбрать кандидаты отражений;
- построить delayed emission contexts;
- создать и запустить spatial emitters.

Внутри runtime есть как минимум два логических слоя:

1. `BS5_EchoEnvironmentAnalyzer`
2. `BS5_EchoEmissionService`

### 5.3.1. Что делает environment analyzer

- читает world/environment signals;
- строит `BS5_EnvironmentSnapshot`;
- классифицирует профиль сцены;
- зовёт `BS5_HybridTailPlanner.BuildCandidates(...)`;
- формирует `BS5_EchoAnalysisResult` со списком кандидатов и debug summary.

### 5.3.2. Что делает emission service

- читает локальную пользовательскую громкость эха;
- режет число эмиттеров по budget;
- конвертирует кандидатов в `BS5_PendingEmissionContext`;
- откладывает playback через `ScriptCallQueue.CallLater`;
- спавнит helper prefab emitters;
- проставляет runtime signals в `SoundComponent`;
- запускает событие `PlayStr(...)` / `SoundEventTransform(...)`.

---

## 6. Анализ среды и выбор отражателей

Ключевой файл:

- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`

Это сердце логики выбора отражающих точек.

## 6.1. Что система должна учитывать

- open / forest / water / indoor / trench / hill contribution;
- terrain bias и slope bias;
- близкую confinement-структуру;
- urban forward query;
- sector-field anchors;
- confirm-traces по геометрии;
- fallback distance bands из prefab.

## 6.2. Целевой активный путь выбора кандидатов

Правильная рабочая последовательность:

1. снять snapshot среды;
2. запустить секторное/anchor-планирование;
3. если профиль или геометрия намекают на settlement/structures, провести узкий urban facade pass;
4. использовать prefab-hints и geometry confirm для отбора реальных строений;
5. только при необходимости добрать `profile_fallback`;
6. сбалансировать ближние/средние/дальние кандидаты;
7. отдать в emission service.

## 6.3. Анти-ложные срабатывания

Система уже содержит несколько классов фильтрации:

- vegetation hints;
- building hints;
- structure hints;
- facade reject hints;
- urban clutter reject hints;
- house-like hints.

Критически важно сохранить и развивать следующий принцип:

- дерево, куст, hedge, grass, branch, trunk и подобные prefab hints не должны продвигаться в `facade_hit`;
- мелкие props и мусорные объекты не должны забивать urban ranking;
- `facade_hit` должен быть подтверждён геометрией или безопасным house-like fallback только там, где это оправдано.

## 6.4. Текущие подтверждённые debug-поля в результате анализа

В логах могут и должны встречаться:

- `mode=engine_env`
- `candidates=...`
- `envProfile=...`
- `anchorMode=sector_field`
- `sectors=...`
- `heights=...`
- `sectorCacheHit=...`
- `anchorHits=...`
- `queryEntities=...`
- `queryBuildings=...`
- `queryFacades=...`
- `queryTop=...`
- `confirmTop=...`
- `anchorFallback=...`
- `slapbacks=...`

Эти поля считаются важными диагностическими сигналами, а не декоративным логом.

---

## 7. Runtime-типы и контракт данных

Ключевой файл:

- `Scripts/Game/BS5_EchoTypes.c`

### 7.1. Важные enum'ы

- `BS5_EchoEnvironmentType`
- `BS5_TailProfileType`
- `BS5_VegetationClass`
- `BS5_EchoCandidateSourceType`

### 7.2. Важные runtime-структуры

- `BS5_EchoReflectorCandidate`
- `BS5_EchoAnalysisResult`
- `BS5_EnvironmentSnapshot`
- `BS5_PendingEmissionContext`

### 7.3. Что должно жить в `BS5_EchoAnalysisResult`

Это не просто временный контейнер. Это контракт между анализом среды и эмиссией.

Минимально важные поля:

- итоговый environment/profile;
- suppressed flag;
- open/urban/forest/hill/water/hard/indoor/trench scores;
- room size / confidence;
- master/slapback delays;
- intensity;
- confinement values;
- anchor/query/facade counters;
- cache hit / fallback flags;
- `m_sTailForwardTopPrefabs`
- `m_sTailForwardConfirmTopPrefabs`
- массивы кандидатов tail/slapback;
- `m_sDebugSummary`

---

## 8. Контракт helper emitter-ов и звуковых ресурсов

## 8.1. Helper prefab emitters

Текущие runtime prefab-ресурсы:

- `Prefabs/Props/BS5_TailEmitter.et`
- `Prefabs/Props/BS5_TailEmitter_PBS.et`
- `Prefabs/Props/BS5_SlapbackEmitter.et`
- `Prefabs/Props/BS5_TailEmitter_MG.et`

Runtime-код спавнит именно helper emitters, а не играет tail напрямую на оружии.

## 8.2. ACP master-графы

Текущие ключевые ACP:

- `Sounds/Weapons/Rifles/BS5/Weapons_Rifles_EchoMaster.acp`
- `Sounds/Weapons/Rifles/BS5/BS5_EchoTailsPBSMaster.acp`
- `Sounds/Weapons/Rifles/BS5/Weapons_Rifles_SlapBacksMaster.acp`

### 8.2.1. Обязательный контракт ACP

ACP-графы должны принимать runtime signal'ы из скрипта. Для этого уже используется custom signal path.

Критически важный подтверждённый путь:

- скрипт ставит signal в `SoundComponent.SetSignalValueStr(...)`;
- ACP читает его через custom signal graph;
- сигнал `BS5_Intensity` должен реально быть заведён в `Volume` path ACP.

Это уже было отдельно подтверждено практическим тестом:

- `0%` отключает эхо;
- промежуточные значения громкости после исправления custom signal меняют громкость tail emitter-ов.

### 8.2.2. Чего нельзя делать

- нельзя снова сломать структуру ACP-графа неправильным размещением `SignalClass` или `FilterClass`;
- suppressed ACP и slapback ACP уже однажды ломались после ручной вставки сигнала;
- любые правки ACP нужно делать структурно и проверять очень аккуратно.

---

## 9. Контракт runtime signal-ов эмиттера

Ключевой файл:

- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`

Текущий emitter script перед playback проставляет как минимум следующие сигналы:

- `BS5_Intensity`
- `BS5_DelaySeconds`
- `BS5_IsSlapback`
- `BS5_IsExplosion`
- `BS5_IsSuppressed`
- `BS5_EnvironmentId`
- `BS5_MasterDelaySeconds`
- `BS5_SlapbackDelaySeconds`
- `BS5_CandidateRank`
- `BS5_DistanceNorm`
- `BS5_DistanceGain`
- `BS5_UserEchoVolume`
- `BS5_PanBias`
- `BS5_DirectionSupport`
- `BS5_ReverbSend`
- `BS5_TailWidth`
- `BS5_TailBrightness`
- `BS5_SurfaceHardness`
- `Interior`
- `RoomSize`
- `BS5_IndoorBias`
- `BS5_RoomSizeNorm`
- `BS5_EnvOpen`
- `BS5_EnvUrban`
- `BS5_EnvForest`
- `BS5_EnvHill`
- `BS5_EnvHard`
- `BS5_EnvIndoor`
- `BS5_EnvTrench`
- `BS5_EnvFront`
- `BS5_EnvBack`
- `BS5_EnvLeft`
- `BS5_EnvRight`
- `BS5_Confidence`
- `BS5_ReflectorCount`

Эти сигналы составляют runtime audio contract. Если меняется ACP, нельзя забывать о соответствии этому контракту.

---

## 10. Пользовательская настройка громкости эха

Ключевые файлы:

- `Scripts/Game/BS5_PlayerAudioSettings.c`
- `Scripts/Game/UI/Menu/SettingsMenu/BS5_AudioSettingsSubMenu.c`

## 10.1. Что уже должно работать

- пользовательский слайдер громкости эха;
- default значение `0.5` (50%);
- сохранение в `UserSettings`;
- чтение значения runtime-кодом;
- применение именно к громкости tail emitter-ов, а не к их количеству.

## 10.2. Важный уже подтверждённый принцип

Ползунок громкости не должен:

- менять выбор кандидатов;
- менять число эмиттеров как основной эффект;
- ломать структуру tail pattern.

Он должен:

- масштабировать именно громкость / intensity.

---

## 11. Текущее состояние `Weapon_Base.et`

Ключевой файл:

- `Prefabs/Weapons/Core/Weapon_Base.et`

В нём лежит текущий tuning `BS5_EchoDriverComponent`.

### 11.1. Текущие важные resource refs

- `m_sTailMasterProject = {6717325A0F4513E2}Sounds/Weapons/Rifles/BS5/Weapons_Rifles_EchoMaster.acp`
- `m_sSlapbackProject = {92DF373C3ABDC7AB}Sounds/Weapons/Rifles/BS5/Weapons_Rifles_SlapBacksMaster.acp`
- `m_sSuppressedTailProject = {7E2D4A4C9F1B6D23}Sounds/Weapons/Rifles/BS5/BS5_EchoTailsPBSMaster.acp`
- `m_sTailEventName = SOUND_SHOT`
- `m_sSlapbackEventName = BS5_TAIL_SLAPBACK`
- `m_sExplosionSlapbackEventName = BS5_EXPLOSION_SLAPBACK`
- `m_sTailEmitterPrefab = {995F66906C1D9EDC}Prefabs/Props/BS5_TailEmitter.et`
- `m_sSlapbackEmitterPrefab = {29D823F0744A8637}Prefabs/Props/BS5_SlapbackEmitter.et`
- `m_sSuppressedTailEmitterPrefab = {7D95E2C84D9A4F31}Prefabs/Props/BS5_TailEmitter_PBS.et`

### 11.2. Текущие важные поведенческие значения

- `m_bEnableTails = 1`
- `m_bEnableSlapback = 0`
- `m_bEnableExplosionReuse = 1`
- `m_fScanRadius = 800`
- `m_iMaxCandidateCount = 10`
- `m_iMaxTailEmittersPerShot = 4`
- `m_iMaxExplosionEmittersPerShot = 4`
- `m_iMaxActiveTailEmitters = 20`
- `m_iMaxActiveSlapbackEmitters = 8`
- `m_fCachePositionToleranceMeters = 0.4`
- `m_fCacheHeadingDotTolerance = 0.99`
- `m_fTailForwardConeDegrees = 108`
- `m_fSettlementMaxDistanceMeters = 300`
- `m_fTerrainMaxDistanceMeters = 800`
- `m_fOpenTailMinDistanceMeters = 90`
- `m_fOpenTailMaxDistanceMeters = 800`
- `m_iOpenTailFallbackCount = 3`
- `m_iSettlementTailTargetCount = 6`
- `m_iTerrainTailTargetCount = 4`
- `m_iMaxSuppressedTailEmittersPerShot = 3`
- `m_fSuppressedDistanceMultiplier = 0.72`
- `m_fSuppressedIntensityMultiplier = 0.72`
- `m_fIndoorCloseDistanceMeters = 4`
- `m_fIndoorMaxDistanceMeters = 100`
- `m_fIndoorIntensityMultiplier = 0.55`
- `m_iTailSectorCount = 11`
- `m_iTailHeightSampleCount = 2`
- `m_fTailScanHeightLowMeters = 2`
- `m_fTailScanHeightHighMeters = 9.5`
- `m_fTailClusterDistanceMeters = 10`
- `m_fTailNormalMergeDot = 0.78`
- `m_fTailGroundNormalMaxY = 0.9`
- `m_fTailForwardSectorWeight = 1`
- primary band `25–90`
- secondary band `90–220`
- tertiary band `220–800`
- `m_fPrimaryCandidateScore = 0.96`
- `m_fTertiaryCandidateScore = 0.48`
- `m_fFacadeDelayScale = 0.52`
- `m_fDistanceGainNearMeters = 90`
- `m_fDistanceGainFarMeters = 800`
- `m_fDistanceGainFarVolume = 0.28`
- `m_fDistanceGainCurvePower = 1.15`
- `m_bDebug = 0`

### 11.3. Важное замечание по prefab-tuning

Этот tuning уже был синхронизирован под такие цели:

- потолок дальнего spawn до `800 м`;
- более уверенная работа settlement/houses на `25–300 м`;
- уменьшение вклада слишком близкого fallback;
- ослабление ложных flat/terrain-like отражателей;
- снижение loudness дальних хвостов.

Но этот tuning не следует считать окончательно подтверждённым, пока не завершена стабилизация Workbench и не проведена повторная серия игровых тестов.

---

## 12. Подтверждённая текущая игровая функциональность

По предыдущим игровым логам было подтверждено:

- urban path действительно может давать `facade_hit`;
- `envProfile=settlement_edge` реально активируется;
- cache может переиспользовать успешный facade-result;
- suppressed path в игре работает отдельно через PBS emitter/ACP;
- пользовательская громкость после фикса ACP-сигнала реально влияет на tail volume;
- дальние хвосты тише ближних.

Примеры наблюдавшихся корректных симптомов:

- `queryFacades > 0`
- `source=facade_hit`
- `anchorHits > 0`
- успешные delay/intensity для ближних фасадов и более тихие дальние terrain/profile fallback кандидаты.

---

## 13. Текущие известные проблемы и риски

## 13.1. Главная нерешённая проблема: Workbench startup crash

Текущее состояние:

- Workbench стабильно крашится при открытии проекта `BetterSounds5`;
- crash код: `SEH exception 0xc0000374`;
- это `STATUS_HEAP_CORRUPTION`;
- crash происходит в фазе `Workbench Create Game` после compile/create path;
- это native heap corruption, а не обычная Enforce compile error.

### 13.1.1. Что уже НЕ следует считать достаточным объяснением

- active `EnfusionMCP` bridge scripts как единственная причина;
- настройки `Weapon_Base.et` как наиболее вероятный root cause сами по себе;
- `BAR_echo` wav как единственный источник бага.

Почему:

- `Scripts/WorkbenchGame` уже был очищен, а crash остался;
- `BAR_echo` wav были удалены, а crash остался;
- числовой tuning prefab чаще ломает поведение в игре, а не Workbench startup.

### 13.1.2. Что сейчас вероятнее всего

Наиболее правдоподобная рабочая гипотеза:

- один или несколько runtime script-файлов в `Scripts/Game` триггерят engine/Workbench bug в startup/create path;
- это script-side проблема, но проявляется как native heap corruption.

Наиболее подозрительные файлы:

- `Scripts/Game/BS5_EchoRuntime.c`
- `Scripts/Game/BS5_EnvironmentAudioClassifier.c`
- `Scripts/Game/BS5_SpatialSoundEmitterComponent.c`
- `Scripts/Game/BS5_EchoDriverComponent.c`
- `Scripts/Game/BS5_ExplosionBridge.c`

## 13.2. `Slapback` сейчас intentionally disabled

- `m_bEnableSlapback = 0`
- трогать его без явного запроса не нужно

## 13.3. `BAR_echo` wav были удалены

Удалённые файлы:

- `Assets/Gunpowder/SlapBacks/BAR_echo_3p_close_stereo_02.wav`
- `Assets/Gunpowder/SlapBacks/BAR_echo_3p_close_stereo_03.wav`
- `Assets/Gunpowder/SlapBacks/BAR_echo_3p_close_stereo_05.wav`

Важно:

- в `resourceDatabase.rdb` могут оставаться stale refs;
- это не нужно автоматически считать root cause, но при расследовании Workbench crash это нужно помнить.

## 13.4. Workbench-специфические guards уже добавлены

В fire hooks уже есть `#ifdef WORKBENCH` ранний выход:

- в `BS5_EchoDriverComponent.c` для `SCR_MuzzleEffectComponent.OnFired`
- в `BS5_ExplosionBridge.c` для `SCR_WeaponBlastComponent.OnWeaponFired`

Эти guards были добавлены как минимальная попытка изоляции Workbench path и пока не должны удаляться без доказательств.

---

## 14. Что считается правильным поведением мода

## 14.1. Правильное поведение в игре

1. При выстреле tail system должна анализировать окружающую среду и выбирать ограниченное число кандидатов.
2. В городе и около домов при выстреле вперёд фасады/строения должны выигрывать у terrain fallback.
3. При отсутствии убедительных строений система должна использовать fallback, но не спамить лишними точками.
4. При выстреле с глушителем хвосты должны быть ближе и мягче.
5. Дальние отражения должны быть тише.
6. UI volume должен регулировать именно громкость tail emitter-ов.
7. Система должна оставаться дешёвой при плотной стрельбе.

## 14.2. Правильное поведение в логах

В хороших settlement/city сценариях желательно видеть:

- `envProfile=settlement_edge`
- `anchorHits > 0`
- `queryBuildings > 0`
- `queryFacades > 0`
- `source=facade_hit` у лучших кандидатов
- `anchorFallback=false` в удачных кейсах

В лесу и открытой местности нормально видеть:

- `terrain_primary`
- `profile_fallback`

Но в городе это не должно полностью вытеснять фасады.

## 14.3. Что считается плохим поведением

- дерево или куст становятся `facade_hit`;
- `queryTop` забит мусорными props;
- city/settlement direction почти всегда скатывается в open/terrain fallback;
- при 50% пользовательской громкости tail structure разваливается или пропадает вместо плавного ослабления;
- runtime budget раздувается без контроля.

---

## 15. Что уже обязательно учитывать при новой работе

### 15.1. Не переписывать всё с нуля без причины

В моде уже есть:

- рабочая общая архитектура;
- рабочий signal contract;
- helper emitters;
- suppressor path;
- user volume;
- debug summary;
- urban facade path;
- fallback system;
- cache.

Это не сырой прототип. Это система, которую нужно стабилизировать и довести, а не уничтожить и переписать без доказательств.

### 15.2. Изменения делать минимально

Следующий инженер должен придерживаться:

- минимального diff;
- доказуемого engine-level пути;
- строгой валидации;
- разделения game-runtime проблем и Workbench-only проблем.

### 15.3. Не терять различие между двумя задачами

Есть две разные задачи:

1. качество акустики/детекта в игре;
2. стабильность открытия проекта в Workbench.

Их нельзя снова смешивать в одну гипотезу без доказательств.

---

## 16. Приоритеты дальнейшей работы

## 16.1. Приоритет №1: стабилизировать открытие Workbench

Это самый высокий приоритет, потому что без него затруднены дальнейшие редакторские проверки.

Правильный следующий подход:

1. не гадать по очередному `0xc0000374`;
2. делать жёсткую изоляцию источника;
3. сначала исключать script groups, а не бесконечно крутить prefab tuning.

Рекомендуемый метод:

1. binary isolation по `Scripts/Game`;
2. начать с наиболее рискованных BS5 runtime файлов;
3. добиться первого успешного открытия Workbench;
4. возвращать файлы обратно по одному / малыми группами;
5. зафиксировать точный culprit.

## 16.2. Приоритет №2: после стабилизации Workbench перепроверить urban quality

После починки Workbench:

- перепроверить актуальный behavior в игре;
- убедиться, что tuning `Weapon_Base.et` не потерян;
- решить, нужен ли ещё один pass по:
  - urban prefab ranking;
  - detached house confirm;
  - anti-tree / anti-clutter filtering.

## 16.3. Приоритет №3: потом уже cleanup/release polish

Только после этого:

- чистить лишний debug;
- приводить release package в порядок;
- решать вопрос final publish readiness.

---

## 17. Валидация, которую обязан делать новый инженер

После нетривиального изменения по возможности запускать:

1. `mod_validate`
2. `mod_build` если он даёт полезный результат
3. Workbench open / reload / resource validation при доступности
4. live in-game check

### 17.1. Что считать обязательным для gameplay-изменений

- `mod_validate`
- хотя бы один реальный игровой тест
- анализ `BS5 shot analysis` и `BS5 candidate[...]`

### 17.2. Что считать обязательным для ACP/audio signal правок

- проверить, что ACP не ломается структурно;
- проверить, что user volume реально меняет громкость;
- проверить, что `0%` отключает tail path;
- проверить, что suppressed ACP не падает.

### 17.3. Что считать обязательным для Workbench-fix

- Workbench должен открыть `addon.gproj` без startup crash;
- только после этого можно делать вывод, что culprit локализован.

---

## 18. Практический чеклист для нового чата

Если начинать работу с этого файла с нуля, порядок должен быть такой:

1. Проверить текущие файлы проекта и убедиться, что `Weapon_Base.et`, runtime scripts и ACP лежат на ожидаемых местах.
2. Не трогать slapback.
3. Подтвердить, что user volume и `BS5_Intensity` path сохранены.
4. Сначала расследовать Workbench crash через изоляцию script groups.
5. После стабилизации Workbench перепроверить:
   - unsuppressed tail
   - suppressed tail
   - settlement edge
   - detached houses
   - tree false positives
6. Только потом делать следующий tuning или release cleanup.

---

## 19. Итог

`BetterSounds5` уже представляет собой сложную, событийную систему динамических отражений оружейного звука с:

- runtime анализом среды;
- urban facade acquisition;
- suppressor-aware path;
- explosion bridge;
- helper emitters;
- signal-driven ACP;
- пользовательской громкостью.

Главное, что нельзя потерять при новой сессии:

- это не задача “сделать tails вообще”;
- это задача **сохранить существующую архитектуру, стабилизировать Workbench, не сломать рабочие игровые path'ы и довести urban quality без регресса**.

Именно так должен читать этот проект новый инженер после открытия этого файла.
