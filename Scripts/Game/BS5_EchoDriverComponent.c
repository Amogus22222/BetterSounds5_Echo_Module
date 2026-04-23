[ComponentEditorProps(category: "BS5 Audio", description: "Main BS5 weapon-attached echo and tails controller.")]
class BS5_EchoDriverComponentClass : ScriptComponentClass
{
}

class BS5_EchoDriverComponent : ScriptComponent
{
	protected static const ResourceName DEFAULT_MASTER_EMITTER_PREFAB = "{995F66906C1D9EDC}Prefabs/Props/BS5_TailEmitter.et";
	protected static const ResourceName DEFAULT_MACHINEGUN_MASTER_EMITTER_PREFAB = "{4CB3F211A7DF906E}Prefabs/Props/BS5_TailEmitter_MG.et";
	protected static const ResourceName DEFAULT_SUPPRESSED_MASTER_EMITTER_PREFAB = "{A59D3E1092B44A6C}Prefabs/Props/BS5_TailEmitter_Silenced.et";
	protected static const ResourceName DEFAULT_SLAPBACK_EMITTER_PREFAB = "{29D823F0744A8637}Prefabs/Props/BS5_SlapbackEmitter.et";
	protected static const ResourceName DEFAULT_SUPPRESSED_SLAPBACK_EMITTER_PREFAB = "{D07C4B82E65C4F31}Prefabs/Props/BS5_SlapbackEmitter_Silenced.et";
	protected static const ResourceName MACHINEGUN_MASTER_ACP = "{13CBB6A34DC9700D}Sounds/Weapons/Rifles/BS5/Weapons_MG_EchoMaster.acp";

	[Attribute(defvalue: "", desc: "Optional ACP override for long tail playback. Direct ACP playback is tried first; if the engine rejects it, playback falls back to the emitter prefab SoundComponent.")]
	protected ResourceName m_sMasterAcp;

	[Attribute(defvalue: "", desc: "Optional ACP override for slapback playback. Direct ACP playback is tried first; if the engine rejects it, playback falls back to the emitter prefab SoundComponent.")]
	protected ResourceName m_sSlapbackAcp;

	[Attribute(defvalue: "", desc: "Optional ACP override for trench slapback playback. Leave empty to fall back to the normal slapback ACP until the trench ACP is authored.")]
	protected ResourceName m_sTrenchSlapbackAcp;

	[Attribute(defvalue: "", desc: "Optional ACP override for suppressed tail playback. Direct ACP playback is tried first; if the engine rejects it, playback falls back to the emitter prefab SoundComponent.")]
	protected ResourceName m_sSuppressedMasterAcp;

	[Attribute(defvalue: "", desc: "Optional ACP override for suppressed slapback playback. Direct ACP playback is tried first; if the engine rejects it, playback falls back to the suppressed slapback emitter prefab.")]
	protected ResourceName m_sSuppressedSlapbackAcp;

	[Attribute(defvalue: "SOUND_SHOT", desc: "Audio event triggered on the master tail emitter prefab.")]
	protected string m_sMasterEventName;

	[Attribute(defvalue: "SOUND_SHOT", desc: "Audio event triggered on the slapback emitter prefab.")]
	protected string m_sSlapbackEventName;

	[Attribute(defvalue: "BS5_TRENCH_SLAPBACK", desc: "Audio event triggered on the trench slapback emitter prefab.")]
	protected string m_sTrenchSlapbackEventName;

	[Attribute(defvalue: "", desc: "Optional dedicated audio event used for explosion-like long tails. Leave empty to reuse the main tail event.")]
	protected string m_sExplosionEventName;

	[Attribute(defvalue: "BS5_EXPLOSION_SLAPBACK", desc: "Audio event used for explosion-like near slapback reflections.")]
	protected string m_sExplosionSlapbackEventName;

	[Attribute(defvalue: "{995F66906C1D9EDC}Prefabs/Props/BS5_TailEmitter.et", desc: "Emitter prefab spawned for long tail playback. Its SoundComponent defines the actual master ACP.")]
	protected ResourceName m_sMasterEmitterPrefab;

	[Attribute(defvalue: "{29D823F0744A8637}Prefabs/Props/BS5_SlapbackEmitter.et", desc: "Emitter prefab spawned for slapback playback. Its SoundComponent defines the actual slapback ACP.")]
	protected ResourceName m_sSlapbackEmitterPrefab;

	[Attribute(defvalue: "", desc: "Emitter prefab spawned for trench slapback playback. Leave empty to fall back to normal slapback until the trench ACP/prefab is authored.")]
	protected ResourceName m_sTrenchSlapbackEmitterPrefab;

	[Attribute(defvalue: "{A59D3E1092B44A6C}Prefabs/Props/BS5_TailEmitter_Silenced.et", desc: "Emitter prefab spawned for suppressed muzzle tails. Its SoundComponent defines the suppressed tail ACP variant.")]
	protected ResourceName m_sSuppressedMasterEmitterPrefab;

	[Attribute(defvalue: "{D07C4B82E65C4F31}Prefabs/Props/BS5_SlapbackEmitter_Silenced.et", desc: "Emitter prefab spawned for suppressed slapbacks. Its SoundComponent defines the suppressed slapback ACP variant.")]
	protected ResourceName m_sSuppressedSlapbackEmitterPrefab;

	[Attribute(defvalue: "1", desc: "Enables spawned long tail reflections for weapon shots.")]
	protected bool m_bEnableTails;

	[Attribute(defvalue: "1", desc: "Enables near slapback reflections when the local confinement test allows them.")]
	protected bool m_bEnableSlapback;

	[Attribute(defvalue: "1", desc: "Allows explosion-like events to reuse the same echo analysis core.")]
	protected bool m_bEnableExplosionReuse;

	[Attribute(defvalue: "1", desc: "Uses SoundWorld/SoundMap plus terrain relief as the primary long-tail anchor planner.")]
	protected bool m_bUseSoundMapAnchorPlanner;

	[Attribute(defvalue: "1", desc: "Falls back to the legacy geometry-sector planner when SoundWorld/SoundMap is unavailable.")]
	protected bool m_bAllowLegacyAnchorFallback;

	[Attribute(defvalue: "555", desc: "Maximum search distance in meters used to build tail candidate anchors.")]
	protected float m_fScanRadius;

	[Attribute(defvalue: "25", desc: "Maximum wall distance in meters that is eligible for close slapback anchors.")]
	protected float m_fNearSlapbackRadius;

	[Attribute(defvalue: "0.9", desc: "Forward offset in meters applied to the slapback probe origin so the traces start closer to the muzzle and avoid self-hits.")]
	protected float m_fNearProbeForwardOffsetMeters;

	[Attribute(defvalue: "0.12", desc: "Vertical offset in meters applied to the slapback probe origin.")]
	protected float m_fNearProbeVerticalOffsetMeters;

	[Attribute(defvalue: "28.0", desc: "Half-forward side angle in degrees used by dedicated slapback wall traces. Lower values favor walls closer to the shot direction.")]
	protected float m_fSlapbackForwardProbeAngleDegrees;

	[Attribute(defvalue: "78.0", desc: "Wide side angle in degrees used to detect alley-like slapbacks from left and right walls.")]
	protected float m_fSlapbackSideProbeAngleDegrees;

	[Attribute(defvalue: "4.5", desc: "Maximum side distance in meters for trench left/right wall detection.")]
	protected float m_fTrenchSlapbackMaxSideDistanceMeters;

	[Attribute(defvalue: "0.78", desc: "Minimum confidence required to emit the trench slapback mode.")]
	protected float m_fTrenchSlapbackMinConfidence;

	[Attribute(defvalue: "0.88", desc: "Maximum absolute Y normal accepted as a slapback wall. Higher values allow more sloped trench/ditch walls.")]
	protected float m_fSlapbackWallNormalMaxY;

	[Attribute(defvalue: "0", desc: "Debug fallback only. If close slapback rays miss, allows a cheap fallback from already-ranked close city/facade tail anchors.")]
	protected bool m_bAllowSlapbackAnchorFallback;

	[Attribute(defvalue: "25", desc: "Maximum distance in meters for the city/facade anchor fallback used by slapback. Runtime also caps this to NearSlapbackRadius.")]
	protected float m_fSlapbackAnchorFallbackMaxDistanceMeters;

	[Attribute(defvalue: "0.18", desc: "Minimum gain multiplier applied at the far edge of the slapback radius.")]
	protected float m_fSlapbackDistanceFarGain;

	[Attribute(defvalue: "1.7", desc: "Curve exponent for slapback distance attenuation. Values above 1 keep near walls punchy and fade the far edge faster.")]
	protected float m_fSlapbackDistanceGainCurvePower;

	[Attribute(defvalue: "0.006", desc: "Minimum slapback delay in seconds.")]
	protected float m_fSlapbackDelayMinSeconds;

	[Attribute(defvalue: "0.055", desc: "Maximum hard wall slapback delay in seconds.")]
	protected float m_fSlapbackWallDelayMaxSeconds;

	[Attribute(defvalue: "0.030", desc: "Maximum trench slapback delay in seconds.")]
	protected float m_fSlapbackTrenchDelayMaxSeconds;

	[Attribute(defvalue: "12", desc: "Maximum number of ranked candidate anchors stored after analysis.")]
	protected int m_iMaxCandidateCount;

	[Attribute(defvalue: "8", desc: "Maximum number of cheap near probes traced per analysis pass.")]
	protected int m_iMaxTraceCount;

	[Attribute(defvalue: "0.18", desc: "Time window in seconds where repeated shots may reuse the previous analysis result.")]
	protected float m_fBurstReuseWindowSeconds;

	[Attribute(defvalue: "1", desc: "Enables the global BS5 ACP playback limiter, burst decimation, pending queue cap, and low-priority voice stealing.")]
	protected bool m_bEnablePlaybackLimiter;

	[Attribute(defvalue: "16", desc: "Global weighted cap of simultaneously playing BS5 tail native source voices across all weapons. Older far/quiet voices may be stolen above this cap.")]
	protected int m_iLimiterGlobalMaxTailVoices;

	[Attribute(defvalue: "6", desc: "Global weighted cap of simultaneously playing BS5 slapback native source voices across all weapons.")]
	protected int m_iLimiterGlobalMaxSlapbackVoices;

	[Attribute(defvalue: "8", desc: "Maximum delayed weighted tail source starts waiting for playback. Extra low-priority delayed tails are dropped before spawning emitters.")]
	protected int m_iLimiterMaxPendingTailVoices;

	[Attribute(defvalue: "4", desc: "Fairness cap of simultaneously playing weighted tail source voices owned by one weapon.")]
	protected int m_iLimiterMaxTailVoicesPerOwner;

	[Attribute(defvalue: "4", desc: "Normal automatic-fire echo cadence. After the first shot, one shot out of this many keeps tails.")]
	protected int m_iLimiterBurstCadenceNormal;

	[Attribute(defvalue: "7", desc: "High-pressure automatic-fire echo cadence. Used when the global tail voice pool is close to full.")]
	protected int m_iLimiterBurstCadenceHighPressure;

	[Attribute(defvalue: "1", desc: "Maximum tail emitters emitted by an accepted automatic-fire burst shot.")]
	protected int m_iLimiterBurstTailEmittersPerShot;

	[Attribute(defvalue: "1", desc: "Maximum tail emitters emitted per shot while the global tail pool is under high pressure.")]
	protected int m_iLimiterHighPressureTailEmittersPerShot;

	[Attribute(defvalue: "0.80", desc: "Global tail voice pressure where the limiter starts shortening tails and using high-pressure burst cadence.")]
	protected float m_fLimiterHighPressureThreshold;

	[Attribute(defvalue: "0.94", desc: "Global tail voice pressure where the limiter may steal old far/quiet voices aggressively.")]
	protected float m_fLimiterCriticalPressureThreshold;

	[Attribute(defvalue: "4", desc: "Maximum weighted tail source starts allowed per rolling 100 ms gate before defer/drop.")]
	protected int m_iLimiterMaxTailStartsPer100Ms;

	[Attribute(defvalue: "4", desc: "Maximum weighted slapback source starts allowed per rolling 100 ms gate before defer/drop.")]
	protected int m_iLimiterMaxSlapbackStartsPer100Ms;

	[Attribute(defvalue: "0.08", desc: "Fade-out time in seconds used when the limiter steals or prunes BS5 playback.")]
	protected float m_fLimiterStealFadeSeconds;

	[Attribute(defvalue: "2", desc: "Estimated native ACP source voices consumed by one tail emission.")]
	protected int m_iLimiterEstimatedSourcesPerTail;

	[Attribute(defvalue: "1", desc: "Estimated native ACP source voices consumed by one slapback emission.")]
	protected int m_iLimiterEstimatedSourcesPerSlapback;

	[Attribute(defvalue: "1.0", desc: "Maximum lifetime of the cached analysis result in seconds.")]
	protected float m_fCacheTtlSeconds;

	[Attribute(defvalue: "686.0", desc: "Speed of sound used to convert anchor distance to delay seconds.")]
	protected float m_fSoundSpeedMetersPerSecond;

	[Attribute(defvalue: "2", desc: "Maximum number of tail emitters actually spawned for a single weapon shot.")]
	protected int m_iMaxTailEmittersPerShot;

	[Attribute(defvalue: "1", desc: "Maximum number of tail emitters spawned for a single explosion-like event.")]
	protected int m_iMaxExplosionEmittersPerShot;

	[Attribute(defvalue: "2", desc: "Maximum number of slapback emitters spawned for a single weapon shot.")]
	protected int m_iMaxSlapbackEmittersPerShot;

	[Attribute(defvalue: "16", desc: "Hard cap of simultaneously alive tail emitter entities for this weapon. Extra tails are skipped to protect performance.")]
	protected int m_iMaxActiveTailEmitters;

	[Attribute(defvalue: "6", desc: "Hard cap of simultaneously alive slapback emitter entities for this weapon.")]
	protected int m_iMaxActiveSlapbackEmitters;

	[Attribute(defvalue: "2.5", desc: "Maximum movement in meters that still reuses the cached analysis result.")]
	protected float m_fCachePositionToleranceMeters;

	[Attribute(defvalue: "0.97", desc: "Minimum forward-vector dot product that still reuses the cached analysis result.")]
	protected float m_fCacheHeadingDotTolerance;

	[Attribute(defvalue: "100.0", desc: "Full cone width in degrees that receives forward-priority scoring for tail candidates.")]
	protected float m_fTailForwardConeDegrees;

	[Attribute(defvalue: "12.0", desc: "Additional degrees pushed beyond the forward cone for wide/outside-FOV tail candidates.")]
	protected float m_fOutsideConeOffsetDegrees;

	[Attribute(defvalue: "180.0", desc: "Radius in meters used by the ambient/environment classifier when building a fresh tail snapshot.")]
	protected float m_fEnvQueryRadiusMeters;

	[Attribute(defvalue: "12", desc: "Number of forward/FOV-biased anchor seeds traced on a fresh tail analysis.")]
	protected int m_iForwardAnchorTraceCount;

	[Attribute(defvalue: "6", desc: "Number of lateral/wide anchor seeds traced on a fresh tail analysis.")]
	protected int m_iLateralAnchorTraceCount;

	[Attribute(defvalue: "10.0", desc: "Maximum absolute pitch in degrees that may influence distant terrain anchors. Horizontal spread always follows flattened forward.")]
	protected float m_fAnchorPitchClampDegrees;

	[Attribute(defvalue: "16.0", desc: "Distance in meters used to merge close tail anchors from the new hybrid planner.")]
	protected float m_fAnchorMergeDistanceMeters;

	[Attribute(defvalue: "140.0", desc: "Maximum distance in meters considered settlement-relevant for facade-biased tail anchors.")]
	protected float m_fSettlementMaxDistanceMeters;

	[Attribute(defvalue: "260.0", desc: "Maximum distance in meters considered terrain-relevant for hill/open tail anchors.")]
	protected float m_fTerrainMaxDistanceMeters;

	[Attribute(defvalue: "120.0", desc: "Minimum distance in meters for open-scene fallback tail anchors.")]
	protected float m_fOpenTailMinDistanceMeters;

	[Attribute(defvalue: "280.0", desc: "Maximum distance in meters for open-scene fallback tail anchors.")]
	protected float m_fOpenTailMaxDistanceMeters;

	[Attribute(defvalue: "2", desc: "How many fallback anchors may be emitted in open scenes when no visible reflector geometry is confirmed.")]
	protected int m_iOpenTailFallbackCount;

	[Attribute(defvalue: "4", desc: "Target number of settlement/facade-biased tail anchors before emission caps are applied.")]
	protected int m_iSettlementTailTargetCount;

	[Attribute(defvalue: "3", desc: "Target number of terrain/hill-biased tail anchors before emission caps are applied.")]
	protected int m_iTerrainTailTargetCount;

	[Attribute(defvalue: "1", desc: "Maximum number of emitted tail anchors when the active muzzle is suppressed.")]
	protected int m_iMaxSuppressedTailEmittersPerShot;

	[Attribute(defvalue: "0.78", desc: "Distance multiplier applied to suppressor tails so reflected anchors stay closer and softer than unsuppressed shots.")]
	protected float m_fSuppressedDistanceMultiplier;

	[Attribute(defvalue: "0.72", desc: "Final intensity multiplier applied to suppressor tails after candidate scoring and distance gain.")]
	protected float m_fSuppressedIntensityMultiplier;

	[Attribute(defvalue: "3", desc: "Target number of short tail anchors used by the indoor planner before emission caps are applied.")]
	protected int m_iIndoorTailTargetCount;

	[Attribute(defvalue: "52.0", desc: "Maximum distance in meters used by the indoor planner. Outdoor-style distant anchors are clamped away above this range.")]
	protected float m_fIndoorMaxDistanceMeters;

	[Attribute(defvalue: "0.55", desc: "Lateral spread multiplier applied by the indoor planner so indoor tails stay mostly in front with a smaller side component.")]
	protected float m_fIndoorLateralSpreadScale;

	[Attribute(defvalue: "7", desc: "Number of azimuth sectors used by the obstacle-driven tail reflector scan. Higher values improve coverage but add more traces per fresh analysis.")]
	protected int m_iTailSectorCount;

	[Attribute(defvalue: "2", desc: "Number of vertical height samples used per azimuth sector and distance band. Use 2 for low/high facade coverage or 3 to add a mid-height sample.")]
	protected int m_iTailHeightSampleCount;

	[Attribute(defvalue: "2.4", desc: "Lowest world-space height above terrain used when scanning for tail reflectors. This helps catch low walls, slopes, and ground-adjacent facades.")]
	protected float m_fTailScanHeightLowMeters;

	[Attribute(defvalue: "7.5", desc: "Highest world-space height above terrain used when scanning for tail reflectors. This helps catch building facades and elevated terrain features without following the player's pitch.")]
	protected float m_fTailScanHeightHighMeters;

	[Attribute(defvalue: "12.0", desc: "Cluster merge distance in meters for obstacle-driven tail hits. Multiple traces that hit the same house or terrain face collapse into one reflector candidate.")]
	protected float m_fTailClusterDistanceMeters;

	[Attribute(defvalue: "0.55", desc: "Minimum normal dot used to merge two tail hits into the same reflector cluster. Lower values merge more aggressively across curved or uneven surfaces.")]
	protected float m_fTailNormalMergeDot;

	[Attribute(defvalue: "0.88", desc: "Maximum absolute Y value allowed for tail reflector normals. Hits flatter than this are treated as ground/roof-like and rejected.")]
	protected float m_fTailGroundNormalMaxY;

	[Attribute(defvalue: "1.0", desc: "Relative sector weight used for forward-facing obstacle hits inside the main forward cone.")]
	protected float m_fTailForwardSectorWeight;

	[Attribute(defvalue: "0.58", desc: "Relative sector weight used for side and wide obstacle hits outside the main forward cone.")]
	protected float m_fTailSideSectorWeight;

	[Attribute(defvalue: "70.0", desc: "Minimum distance in meters for the closest forward-priority tail candidate.")]
	protected float m_fPrimaryTailDistanceMin;

	[Attribute(defvalue: "95.0", desc: "Maximum distance in meters for the closest forward-priority tail candidate.")]
	protected float m_fPrimaryTailDistanceMax;

	[Attribute(defvalue: "120.0", desc: "Minimum distance in meters for mid-distance tail candidates.")]
	protected float m_fSecondaryTailDistanceMin;

	[Attribute(defvalue: "170.0", desc: "Maximum distance in meters for mid-distance tail candidates.")]
	protected float m_fSecondaryTailDistanceMax;

	[Attribute(defvalue: "180.0", desc: "Minimum distance in meters for wide/outside-cone tail candidates.")]
	protected float m_fTertiaryTailDistanceMin;

	[Attribute(defvalue: "260.0", desc: "Maximum distance in meters for wide/outside-cone tail candidates.")]
	protected float m_fTertiaryTailDistanceMax;

	[Attribute(defvalue: "0.95", desc: "Base score of the strongest forward-priority tail template.")]
	protected float m_fPrimaryCandidateScore;

	[Attribute(defvalue: "0.72", desc: "Base score of the medium-priority tail templates.")]
	protected float m_fSecondaryCandidateScore;

	[Attribute(defvalue: "0.56", desc: "Base score of the wide/outside-cone tail templates.")]
	protected float m_fTertiaryCandidateScore;

	[Attribute(defvalue: "3.5", desc: "How long a spawned tail emitter is kept alive before cleanup.")]
	protected float m_fTailEmitterLifetimeSeconds;

	[Attribute(defvalue: "2.4", desc: "Tail emitter lifetime cap used while the global ACP pool is under high pressure.")]
	protected float m_fTailEmitterHighPressureLifetimeSeconds;

	[Attribute(defvalue: "1.2", desc: "Minimum tail emitter lifetime used when the limiter has to shorten distant or quiet tails.")]
	protected float m_fTailEmitterMinManagedLifetimeSeconds;

	[Attribute(defvalue: "1.0", desc: "How long a spawned slapback emitter is kept alive before cleanup.")]
	protected float m_fSlapbackEmitterLifetimeSeconds;

	[Attribute(defvalue: "0.45", desc: "Minimum wall distance in meters before a trace is accepted as a slapback anchor. Lower values help catch walls that are very close to the muzzle without turning self-noise into a reflection.")]
	protected float m_fSlapbackMinDistanceMeters;

	[Attribute(defvalue: "1.5", desc: "Distance in meters used to merge multiple traces that hit the same nearby wall into a single slapback source.")]
	protected float m_fSlapbackMergeDistanceMeters;

	[Attribute(defvalue: "0.08", desc: "Small offset applied away from the wall normal so the slapback emitter is not embedded inside geometry.")]
	protected float m_fSlapbackWallOffsetMeters;

	[Attribute(defvalue: "2.0", desc: "Path-length multiplier used to convert wall distance into slapback delay. 2.0 approximates out-and-back travel.")]
	protected float m_fSlapbackPathLengthScale;

	[Attribute(defvalue: "0.56", desc: "Delay multiplier applied to facade-hit tails. Lower values help compensate the perceived extra travel time of spatial emitters placed on buildings.")]
	protected float m_fFacadeDelayScale;

	[Attribute(defvalue: "1.0", desc: "Additional intensity multiplier applied only to slapback emissions.")]
	protected float m_fSlapbackIntensityScale;

	[Attribute(defvalue: "1.0", desc: "Global intensity multiplier applied to explosion-like events before candidate scoring.")]
	protected float m_fExplosionIntensityMultiplier;

	[Attribute(defvalue: "1.0", desc: "Multiplier exposed to ACP as BS5_ReverbSend.")]
	protected float m_fReverbSendScale;

	[Attribute(defvalue: "1.0", desc: "Multiplier exposed to ACP as BS5_TailWidth.")]
	protected float m_fTailWidthScale;

	[Attribute(defvalue: "1.0", desc: "Multiplier exposed to ACP as BS5_TailBrightness.")]
	protected float m_fTailBrightnessScale;

	[Attribute(defvalue: "1.0", desc: "Multiplier exposed to ACP as BS5_SurfaceHardness.")]
	protected float m_fSurfaceHardnessScale;

	[Attribute(defvalue: "120.0", desc: "Forward cone width in degrees used by the SoundMap anchor planner.")]
	protected float m_fSoundMapForwardConeDegrees;

	[Attribute(defvalue: "700.0", desc: "Maximum forward sample distance in meters used by the SoundMap anchor planner.")]
	protected float m_fSoundMapForwardMaxDistanceMeters;

	[Attribute(defvalue: "9", desc: "Number of azimuth rays sampled inside the SoundMap forward cone.")]
	protected int m_iSoundMapForwardRayCount;

	[Attribute(defvalue: "7", desc: "Number of fixed forward distance bands sampled by the SoundMap planner.")]
	protected int m_iSoundMapForwardSampleCount;

	[Attribute(defvalue: "450.0", desc: "Maximum radius in meters for the SoundMap 360 context scan.")]
	protected float m_fSoundMapOmniRadiusMeters;

	[Attribute(defvalue: "8", desc: "Number of directions used by the SoundMap 360 context scan.")]
	protected int m_iSoundMapOmniDirectionCount;

	[Attribute(defvalue: "2", desc: "Maximum number of 360 context anchors allowed to supplement forward anchors.")]
	protected int m_iSoundMapOmniAnchorCount;

	[Attribute(defvalue: "0.045", desc: "Minimum SoundMap city value that guarantees an urban anchor candidate.")]
	protected float m_fSoundMapCityThreshold;

	[Attribute(defvalue: "0.16", desc: "Minimum SoundMap forest value used for forest echo anchor candidates.")]
	protected float m_fSoundMapForestThreshold;

	[Attribute(defvalue: "0.32", desc: "Minimum SoundMap meadow value used for open meadow echo candidates.")]
	protected float m_fSoundMapMeadowThreshold;

	[Attribute(defvalue: "0.20", desc: "Minimum terrain relief score required to create a hill/ridge echo anchor.")]
	protected float m_fSoundMapHillReliefThreshold;

	[Attribute(defvalue: "1", desc: "Validate terrain SoundMap anchors against the height profile between the shooter and anchor, snapping behind-hill anchors to the front slope or crest.")]
	protected bool m_bSoundMapTerrainFrontSlopeValidation;

	[Attribute(defvalue: "9", desc: "Number of terrain height samples used when validating a SoundMap terrain anchor.")]
	protected int m_iSoundMapTerrainProfileSampleCount;

	[Attribute(defvalue: "2.8", desc: "Height drop in meters after a crest that marks a terrain anchor as being on the back side of a hill.")]
	protected float m_fSoundMapTerrainBacksideDropMeters;

	[Attribute(defvalue: "1", desc: "Run the cheap forward entity/facade micro scan even when SoundMap city signal misses small settlements.")]
	protected bool m_bSoundMapUrbanMicroScan;

	[Attribute(defvalue: "180.0", desc: "Maximum radius in meters used by the SoundMap urban micro scan.")]
	protected float m_fSoundMapUrbanMicroScanRadiusMeters;

	[Attribute(defvalue: "28", desc: "Maximum number of entities inspected by the SoundMap urban micro scan.")]
	protected int m_iSoundMapUrbanMicroMaxEntities;

	[Attribute(defvalue: "0.18", desc: "Score boost applied to confirmed small-urban facade candidates.")]
	protected float m_fSoundMapUrbanScoreBoost;

	[Attribute(defvalue: "0.16", desc: "Deterministic per-analysis jitter applied to SoundMap forward distance bands.")]
	protected float m_fSoundMapDistanceJitter;

	[Attribute(defvalue: "1", desc: "Validate SoundMap tail anchors against a cheap terrain and sound-zone path profile from shooter to anchor.")]
	protected bool m_bSoundMapPathPlausibilityValidation;

	[Attribute(defvalue: "6", desc: "Maximum number of terrain/soundmap samples used by the cheap SoundMap path plausibility gate.")]
	protected int m_iSoundMapPathSampleCount;

	[Attribute(defvalue: "2.2", desc: "Terrain clearance in meters above the shooter-to-anchor LOS before path terrain is considered blocking.")]
	protected float m_fSoundMapPathTerrainClearanceMeters;

	[Attribute(defvalue: "420.0", desc: "Soft distance in meters after which SoundMap tail anchors need a clean path profile.")]
	protected float m_fSoundMapFarTailSoftLimitMeters;

	[Attribute(defvalue: "560.0", desc: "Hard distance in meters after which SoundMap tail anchors are rejected unless explicitly allowed by a clean crest/front-slope profile.")]
	protected float m_fSoundMapFarTailHardLimitMeters;

	[Attribute(defvalue: "180.0", desc: "Maximum distance in meters for close urban/facade tail preference.")]
	protected float m_fSoundMapNearUrbanTailMaxDistanceMeters;

	[Attribute(defvalue: "220.0", desc: "Distance in meters after which suspicious SoundMap tail anchors may get one cheap geometry raycast.")]
	protected float m_fSoundMapPathRaycastDistanceMeters;

	[Attribute(defvalue: "80.0", desc: "Distance in meters where scripted distance-based tail attenuation begins. Emitters closer than this keep full gain.")]
	protected float m_fDistanceGainNearMeters;

	[Attribute(defvalue: "200.0", desc: "Distance in meters where scripted distance-based tail attenuation reaches its far gain target.")]
	protected float m_fDistanceGainFarMeters;

	[Attribute(defvalue: "0.5", desc: "Minimum gain multiplier applied to distant tail emitters once DistanceGainFarMeters is reached.")]
	protected float m_fDistanceGainFarVolume;

	[Attribute(defvalue: "1.0", desc: "Curve exponent for scripted distance-based tail attenuation. Values above 1 keep more gain near the shooter and fall off later.")]
	protected float m_fDistanceGainCurvePower;

	[Attribute(defvalue: "0", desc: "Legacy master debug switch. Prefer BS5_AudioDebugSettingsComponent for channel-based debug.")]
	protected bool m_bDebug;

	protected vector m_vLastOrigin;
	protected vector m_vLastForward;
	protected ref BS5_EchoAnalysisResult m_LastResult;
	protected BS5_CloseReflectionSettingsComponent m_CloseReflectionSettings;
	protected BS5_AudioDebugSettingsComponent m_DebugSettings;
	protected int m_iCacheGeneration;
	protected bool m_bCacheValid;
	protected bool m_bLastSuppressed;
	protected vector m_vTailSectorCacheOrigin;
	protected vector m_vTailSectorCacheForward;
	protected BS5_TailProfileType m_eTailSectorCacheProfile;
	protected bool m_bTailSectorCacheExplosionLike;
	protected int m_iTailSectorCacheAnchorHits;
	protected int m_iTailSectorCacheFacadeHits;
	protected bool m_bTailSectorCacheValid;
	protected ref array<ref BS5_EchoReflectorCandidate> m_aTailSectorCacheCandidates;
	protected vector m_vForwardFacadeNegativeCacheOrigin;
	protected vector m_vForwardFacadeNegativeCacheForward;
	protected BS5_TailProfileType m_eForwardFacadeNegativeCacheProfile;
	protected bool m_bForwardFacadeNegativeCacheExplosionLike;
	protected bool m_bForwardFacadeNegativeCacheValid;
	protected bool m_bDispatchGuardActive;
	protected vector m_vDispatchGuardOrigin;
	protected vector m_vDispatchGuardForward;
	protected int m_iActiveTailEmitters;
	protected int m_iActiveSlapbackEmitters;
	protected int m_iLimiterBurstShotCount;
	protected int m_iLimiterBurstGeneration;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_vLastOrigin = vector.Zero;
		m_vLastForward = "0 0 1";
		m_LastResult = null;
		m_iCacheGeneration = 0;
		m_bCacheValid = false;
		m_bLastSuppressed = false;
		m_vTailSectorCacheOrigin = vector.Zero;
		m_vTailSectorCacheForward = "0 0 1";
		m_eTailSectorCacheProfile = BS5_TailProfileType.OPEN_MEADOW;
		m_bTailSectorCacheExplosionLike = false;
		m_iTailSectorCacheAnchorHits = 0;
		m_iTailSectorCacheFacadeHits = 0;
		m_bTailSectorCacheValid = false;
		m_aTailSectorCacheCandidates = new array<ref BS5_EchoReflectorCandidate>();
		m_vForwardFacadeNegativeCacheOrigin = vector.Zero;
		m_vForwardFacadeNegativeCacheForward = "0 0 1";
		m_eForwardFacadeNegativeCacheProfile = BS5_TailProfileType.OPEN_MEADOW;
		m_bForwardFacadeNegativeCacheExplosionLike = false;
		m_bForwardFacadeNegativeCacheValid = false;
		m_bDispatchGuardActive = false;
		m_vDispatchGuardOrigin = vector.Zero;
		m_vDispatchGuardForward = "0 0 1";
		m_iActiveTailEmitters = 0;
		m_iActiveSlapbackEmitters = 0;
		m_iLimiterBurstShotCount = 0;
		m_iLimiterBurstGeneration = 0;
		DebugValidateConfiguration(owner);
		string initFlagsLog = "driver init";
		initFlagsLog += " tails=" + BS5_DebugLog.BoolText(m_bEnableTails);
		initFlagsLog += " slapback=" + BS5_DebugLog.BoolText(m_bEnableSlapback);
		initFlagsLog += " limiter=" + BS5_DebugLog.BoolText(m_bEnablePlaybackLimiter);
		BS5_DebugLog.Channel(this, BS5_DebugChannel.DRIVER, initFlagsLog);
		BS5_DebugLog.Channel(this, BS5_DebugChannel.DRIVER, "driver slapEvent=" + m_sSlapbackEventName);
	}

	override void OnDelete(IEntity owner)
	{
		BS5_EchoEmissionService.CancelOwnerContexts(owner);

		ScriptCallQueue callQueue = null;
		if (GetGame())
			callQueue = GetGame().GetCallqueue();
		if (callQueue)
		{
			callQueue.RemoveByName(this, "InvalidateCache");
			callQueue.RemoveByName(this, "ResetPlaybackLimiterBurst");
			callQueue.RemoveByName(this, "ClearDispatchGuard");
		}

		m_iCacheGeneration++;
		m_iLimiterBurstGeneration++;
		m_bCacheValid = false;
		m_LastResult = null;
		m_bDispatchGuardActive = false;
		m_iLimiterBurstShotCount = 0;

		super.OnDelete(owner);
	}

	void HandleWeaponFire(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		IEntity owner = GetOwner();
		if (!owner)
			owner = GetOwnerEntity(effectEntity, muzzle);
		if (!owner)
			return;

		vector transform[4];
		owner.GetTransform(transform);
		vector origin = transform[3];
		vector forward = transform[2];
		vector planarForward = FlattenHeading(forward);
		bool suppressed = IsSuppressedMuzzle(muzzle);

		if (ShouldSuppressDuplicateDispatch(origin, planarForward))
		{
			BS5_DebugLog.Channel(this, BS5_DebugChannel.DRIVER, "dispatch skip duplicate same-frame shot");
			return;
		}

		bool allowTailEmit = ShouldEmitShotForPlaybackLimiter(suppressed);
		string dispatchLog = "dispatch shot";
		dispatchLog += " suppressed=" + BS5_DebugLog.BoolText(suppressed);
		dispatchLog += " allowTail=" + BS5_DebugLog.BoolText(allowTailEmit);
		dispatchLog += " driverTail=" + BS5_DebugLog.BoolText(m_bEnableTails);
		dispatchLog += " driverSlap=" + BS5_DebugLog.BoolText(m_bEnableSlapback);
		dispatchLog += " globalSlap=" + BS5_DebugLog.BoolText(IsPlayerSlapbackEnabled());
		BS5_DebugLog.Channel(this, BS5_DebugChannel.DRIVER, dispatchLog);
		string volumeLog = "dispatch volumes";
		volumeLog += " echoVol=" + BS5_PlayerAudioSettings.GetEchoVolume();
		volumeLog += " slapVol=" + BS5_PlayerAudioSettings.GetSlapbackVolume();
		volumeLog += " slapCloseVol=" + BS5_PlayerAudioSettings.GetSlapbackCloseVolume();
		volumeLog += " tech=" + BS5_PlayerAudioSettings.GetTechnicalPresetId();
		BS5_DebugLog.Channel(this, BS5_DebugChannel.DRIVER, volumeLog);
		if (!allowTailEmit && !IsSlapbackEnabled())
		{
			BS5_DebugLog.Channel(this, BS5_DebugChannel.DRIVER, "dispatch skip no tail by cadence and slapback disabled");
			m_vLastOrigin = origin;
			m_vLastForward = planarForward;
			m_bLastSuppressed = suppressed;
			return;
		}

		BS5_EchoAnalysisResult result = ResolveCachedResult(owner, origin, planarForward, false, suppressed);
		if (!result)
			result = BS5_EchoRuntime.AnalyzeShot(this, owner, origin, planarForward, suppressed);

		if (!result)
			return;

		StoreCachedResult(result, suppressed);

		BS5_DebugLog.Channel(this, BS5_DebugChannel.ANALYSIS, "shot analysis " + result.m_sDebugSummary);
		if (result.m_sSlapbackDebugSummary != string.Empty)
			BS5_DebugLog.Channel(this, BS5_DebugChannel.SLAPBACK, result.m_sSlapbackDebugSummary);
		if (result.m_sCloseDebugSummary != string.Empty)
			BS5_DebugLog.Channel(this, BS5_DebugChannel.CLOSE, result.m_sCloseDebugSummary);

		if (m_bEnableTails || m_bEnableSlapback)
		{
			string emitLog = "dispatch emit";
			emitLog += " tails=" + BS5_DebugLog.BoolText(m_bEnableTails && allowTailEmit);
			emitLog += " slapback=" + BS5_DebugLog.BoolText(m_bEnableSlapback);
			emitLog += " tailCandidates=" + result.m_aCandidates.Count();
			emitLog += " slapCandidates=" + result.m_aSlapbackCandidates.Count();
			emitLog += " slapMode=" + result.m_sSlapbackMode;
			BS5_DebugLog.Channel(this, BS5_DebugChannel.DRIVER, emitLog);
			ActivateDispatchGuard(origin, planarForward);
			BS5_EchoRuntime.EmitShot(this, owner, result, allowTailEmit);
		}
		m_vLastOrigin = origin;
		m_vLastForward = planarForward;
		m_bLastSuppressed = suppressed;
	}

	void HandleExplosionFire(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		IEntity owner = GetOwner();
		if (!owner)
			owner = GetOwnerEntity(effectEntity, muzzle);
		if (!owner)
			return;

		vector transform[4];
		owner.GetTransform(transform);
		vector origin = transform[3];
		vector forward = transform[2];
		vector planarForward = FlattenHeading(forward);
		BS5_EchoAnalysisResult result = ResolveCachedResult(owner, origin, planarForward, true, false);
		if (!result)
			result = BS5_EchoRuntime.AnalyzeExplosion(this, owner, origin, planarForward);

		if (!result)
			return;

		StoreCachedResult(result, false);

		if (IsDebugChannelEnabled(BS5_DebugChannel.ANALYSIS))
			BS5_DebugLog.Channel(this, BS5_DebugChannel.ANALYSIS, "explosion analysis " + result.m_sDebugSummary);

		if (m_bEnableExplosionReuse)
			BS5_EchoRuntime.EmitExplosion(this, owner, result);

		m_vLastOrigin = origin;
		m_vLastForward = planarForward;
		m_bLastSuppressed = false;
	}

	ResourceName ResolveMasterAcp(bool suppressed = false)
	{
		if (suppressed && m_sSuppressedMasterAcp != "")
			return m_sSuppressedMasterAcp;

		if (m_sMasterAcp != "")
			return m_sMasterAcp;

		BS5_WeaponEchoSettingsComponent settings = GetSettingsComponent();
		if (settings)
		{
			if (suppressed)
			{
				ResourceName suppressedEchoProject = settings.GetSuppressedEchoProject();
				if (suppressedEchoProject != "")
					return suppressedEchoProject;
			}

			ResourceName echoProject = settings.GetEchoProject();
			if (echoProject != "")
				return echoProject;
		}

		BS5_WeaponEchoRplCharacterComponent characterComponent = GetCharacterComponent();
		if (characterComponent)
		{
			ResourceName defaultEcho = characterComponent.GetDefaultEchoAcp();
			if (defaultEcho != "")
				return defaultEcho;
		}

		return "";
	}

	ResourceName ResolveSlapbackAcp(BS5_EchoCandidateSourceType sourceType = BS5_EchoCandidateSourceType.UNKNOWN, bool suppressed = false)
	{
		if (suppressed && m_sSuppressedSlapbackAcp != "")
			return m_sSuppressedSlapbackAcp;

		if (sourceType == BS5_EchoCandidateSourceType.SLAPBACK_CLOSE_SPACE)
		{
			BS5_CloseReflectionSettingsComponent closeSettings = GetCloseReflectionSettingsComponent();
			if (closeSettings && closeSettings.IsEnabled())
				return closeSettings.ResolveCloseSlapbackAcp();
		}

		if (sourceType == BS5_EchoCandidateSourceType.SLAPBACK_TRENCH && m_sTrenchSlapbackAcp != "")
			return m_sTrenchSlapbackAcp;

		if (m_sSlapbackAcp != "")
			return m_sSlapbackAcp;

		BS5_WeaponEchoSettingsComponent settings = GetSettingsComponent();
		if (settings)
		{
			if (suppressed)
			{
				ResourceName suppressedSlapbackProject = settings.GetSuppressedSlapbackProject();
				if (suppressedSlapbackProject != "")
					return suppressedSlapbackProject;
			}

			ResourceName slapbackProject = settings.GetSlapbackProject();
			if (slapbackProject != "")
				return slapbackProject;
		}

		return ResolveMasterAcp(false);
	}

	ResourceName ResolveExplosionAcp()
	{
		return ResolveMasterAcp(false);
	}

	ResourceName ResolveMasterEmitterPrefab(bool suppressed = false)
	{
		if (suppressed)
		{
			if (IsEmitterPrefabPath(m_sSuppressedMasterEmitterPrefab))
				return m_sSuppressedMasterEmitterPrefab;

			return DEFAULT_SUPPRESSED_MASTER_EMITTER_PREFAB;
		}

		if (IsEmitterPrefabPath(m_sMasterEmitterPrefab) && m_sMasterEmitterPrefab != DEFAULT_MASTER_EMITTER_PREFAB)
			return m_sMasterEmitterPrefab;

		if (ResolveMasterAcp(false) == MACHINEGUN_MASTER_ACP)
			return DEFAULT_MACHINEGUN_MASTER_EMITTER_PREFAB;

		if (IsEmitterPrefabPath(m_sMasterEmitterPrefab))
			return m_sMasterEmitterPrefab;

		return DEFAULT_MASTER_EMITTER_PREFAB;
	}

	ResourceName ResolveSlapbackEmitterPrefab(BS5_EchoCandidateSourceType sourceType = BS5_EchoCandidateSourceType.UNKNOWN, bool suppressed = false)
	{
		if (suppressed)
		{
			if (IsEmitterPrefabPath(m_sSuppressedSlapbackEmitterPrefab))
				return m_sSuppressedSlapbackEmitterPrefab;

			return DEFAULT_SUPPRESSED_SLAPBACK_EMITTER_PREFAB;
		}

		if (sourceType == BS5_EchoCandidateSourceType.SLAPBACK_CLOSE_SPACE)
		{
			BS5_CloseReflectionSettingsComponent closeSettings = GetCloseReflectionSettingsComponent();
			if (closeSettings && closeSettings.IsEnabled() && IsEmitterPrefabPath(closeSettings.ResolveCloseSlapbackEmitterPrefab()))
				return closeSettings.ResolveCloseSlapbackEmitterPrefab();
		}

		if (sourceType == BS5_EchoCandidateSourceType.SLAPBACK_TRENCH)
		{
			if (IsEmitterPrefabPath(m_sTrenchSlapbackEmitterPrefab))
				return m_sTrenchSlapbackEmitterPrefab;
		}

		if (IsEmitterPrefabPath(m_sSlapbackEmitterPrefab))
			return m_sSlapbackEmitterPrefab;

		return DEFAULT_SLAPBACK_EMITTER_PREFAB;
	}

	string ResolveMasterEventName()
	{
		if (m_sMasterEventName != string.Empty)
			return m_sMasterEventName;

		BS5_WeaponEchoSettingsComponent settings = GetSettingsComponent();
		if (settings)
		{
			string echoEventName = settings.GetEchoEventName();
			if (echoEventName != string.Empty)
				return echoEventName;
		}

		return "SOUND_SHOT";
	}

	string ResolveSlapbackEventName(BS5_EchoCandidateSourceType sourceType = BS5_EchoCandidateSourceType.UNKNOWN)
	{
		if (sourceType == BS5_EchoCandidateSourceType.SLAPBACK_CLOSE_SPACE)
		{
			BS5_CloseReflectionSettingsComponent closeSettings = GetCloseReflectionSettingsComponent();
			if (closeSettings && closeSettings.IsEnabled())
				return closeSettings.ResolveCloseSlapbackEventName();
		}

		if (sourceType == BS5_EchoCandidateSourceType.SLAPBACK_TRENCH && m_sTrenchSlapbackEventName != string.Empty && IsEmitterPrefabPath(m_sTrenchSlapbackEmitterPrefab))
			return m_sTrenchSlapbackEventName;

		if (m_sSlapbackEventName != string.Empty)
			return m_sSlapbackEventName;

		BS5_WeaponEchoSettingsComponent settings = GetSettingsComponent();
		if (settings)
		{
			string slapbackEventName = settings.GetSlapbackEventName();
			if (slapbackEventName != string.Empty)
				return slapbackEventName;
		}

		return "SOUND_SHOT";
	}

	string ResolveExplosionEventName()
	{
		if (m_sExplosionEventName != string.Empty)
			return m_sExplosionEventName;

		return ResolveMasterEventName();
	}

	string ResolveExplosionSlapbackEventName()
	{
		if (m_sExplosionSlapbackEventName != string.Empty)
			return m_sExplosionSlapbackEventName;

		return "BS5_EXPLOSION_SLAPBACK";
	}

	protected BS5_TechnicalPreset GetActiveTechnicalPreset()
	{
		return BS5_PresetRegistry.GetActiveTechnicalPreset();
	}

	float GetScanRadius()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fScanRadius;
		if (preset)
			value = preset.m_fScanRadius;
		if (value > 0.0)
			return value;
		return 1.0;
	}

	float GetNearSlapbackRadius()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fNearSlapbackRadius;
		if (preset)
			value = preset.m_fNearSlapbackRadius;
		if (value > 0.0)
			return value;
		return 1.0;
	}

	float GetNearProbeForwardOffsetMeters()
	{
		return BS5_EchoMath.MaxFloat(0.0, m_fNearProbeForwardOffsetMeters);
	}

	float GetNearProbeVerticalOffsetMeters()
	{
		return BS5_EchoMath.MaxFloat(0.0, m_fNearProbeVerticalOffsetMeters);
	}

	float GetExplosionScanRadius()
	{
		return GetScanRadius() * 1.2;
	}

	float GetExplosionNearRadius()
	{
		return GetNearSlapbackRadius() * 0.75;
	}

	float GetExplosionIntensityMultiplier()
	{
		return m_fExplosionIntensityMultiplier;
	}

	int GetMaxTailEmittersPerShot()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iMaxTailEmittersPerShot;
		if (preset)
			value = preset.m_iMaxTailEmittersPerShot;
		if (value > 0)
			return value;
		return 1;
	}

	int GetMaxExplosionEmittersPerShot()
	{
		if (m_iMaxExplosionEmittersPerShot > 0)
			return m_iMaxExplosionEmittersPerShot;
		return 1;
	}

	int GetMaxSlapbackEmittersPerShot()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iMaxSlapbackEmittersPerShot;
		if (preset)
			value = preset.m_iMaxSlapbackEmittersPerShot;
		return Math.Clamp(value, 1, 2);
	}

	int GetMaxActiveTailEmitters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iMaxActiveTailEmitters;
		if (preset)
			value = preset.m_iMaxActiveTailEmitters;
		if (value > 0)
			return value;
		return 0;
	}

	int GetMaxActiveSlapbackEmitters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iMaxActiveSlapbackEmitters;
		if (preset)
			value = preset.m_iMaxActiveSlapbackEmitters;
		if (value > 0)
			return value;
		return 0;
	}

	bool IsPlaybackLimiterEnabled()
	{
		return m_bEnablePlaybackLimiter;
	}

	int GetGlobalMaxTailVoices()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterGlobalMaxTailVoices;
		if (preset)
			value = preset.m_iLimiterGlobalMaxTailVoices;
		return Math.Clamp(value, 4, 64);
	}

	int GetGlobalMaxSlapbackVoices()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterGlobalMaxSlapbackVoices;
		if (preset)
			value = preset.m_iLimiterGlobalMaxSlapbackVoices;
		return Math.Clamp(value, 1, 24);
	}

	int GetMaxPendingTailVoices()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterMaxPendingTailVoices;
		if (preset)
			value = preset.m_iLimiterMaxPendingTailVoices;
		return Math.Clamp(value, 4, 96);
	}

	int GetMaxTailVoicesPerOwner()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterMaxTailVoicesPerOwner;
		if (preset)
			value = preset.m_iLimiterMaxTailVoicesPerOwner;
		return Math.Clamp(value, 1, GetGlobalMaxTailVoices());
	}

	int GetBurstTailCadenceNormal()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterBurstCadenceNormal;
		if (preset)
			value = preset.m_iLimiterBurstCadenceNormal;
		return Math.Clamp(value, 2, 8);
	}

	int GetBurstTailCadenceHighPressure()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterBurstCadenceHighPressure;
		if (preset)
			value = preset.m_iLimiterBurstCadenceHighPressure;
		return Math.Clamp(value, GetBurstTailCadenceNormal(), 12);
	}

	int GetBurstTailEmittersPerShot()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterBurstTailEmittersPerShot;
		if (preset)
			value = preset.m_iLimiterBurstTailEmittersPerShot;
		return Math.Clamp(value, 1, GetMaxTailEmittersPerShot());
	}

	int GetHighPressureTailEmittersPerShot()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterHighPressureTailEmittersPerShot;
		if (preset)
			value = preset.m_iLimiterHighPressureTailEmittersPerShot;
		return Math.Clamp(value, 1, GetMaxTailEmittersPerShot());
	}

	float GetLimiterHighPressureThreshold()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fLimiterHighPressureThreshold;
		if (preset)
			value = preset.m_fLimiterHighPressureThreshold;
		string presetId = BS5_PlayerAudioSettings.GetTechnicalPresetId();
		if (presetId == "light")
			value = BS5_EchoMath.MaxFloat(value, 0.72);
		else if (presetId == "dynamic")
			value = BS5_EchoMath.MaxFloat(value, 0.78);
		else
			value = BS5_EchoMath.MaxFloat(value, 0.80);
		return BS5_EchoMath.Clamp(value, 0.25, 0.98);
	}

	float GetLimiterCriticalPressureThreshold()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fLimiterCriticalPressureThreshold;
		if (preset)
			value = preset.m_fLimiterCriticalPressureThreshold;
		string presetId = BS5_PlayerAudioSettings.GetTechnicalPresetId();
		if (presetId == "light")
			value = BS5_EchoMath.MaxFloat(value, 0.90);
		else if (presetId == "dynamic")
			value = BS5_EchoMath.MaxFloat(value, 0.92);
		else
			value = BS5_EchoMath.MaxFloat(value, 0.94);
		return BS5_EchoMath.Clamp(value, GetLimiterHighPressureThreshold(), 1.0);
	}

	int GetLimiterMaxTailStartsPer100Ms()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterMaxTailStartsPer100Ms;
		if (preset)
			value = preset.m_iLimiterMaxTailStartsPer100Ms;
		return Math.Clamp(value, 1, GetGlobalMaxTailVoices());
	}

	int GetLimiterMaxSlapbackStartsPer100Ms()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterMaxSlapbackStartsPer100Ms;
		if (preset)
			value = preset.m_iLimiterMaxSlapbackStartsPer100Ms;
		return Math.Clamp(value, 1, GetGlobalMaxSlapbackVoices());
	}

	float GetLimiterStealFadeSeconds()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fLimiterStealFadeSeconds;
		if (preset)
			value = preset.m_fLimiterStealFadeSeconds;
		return BS5_EchoMath.Clamp(value, 0.0, 0.25);
	}

	int GetLimiterEstimatedSourcesPerTail()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterEstimatedSourcesPerTail;
		if (preset)
			value = preset.m_iLimiterEstimatedSourcesPerTail;
		return Math.Clamp(value, 1, 8);
	}

	int GetLimiterEstimatedSourcesPerSlapback()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLimiterEstimatedSourcesPerSlapback;
		if (preset)
			value = preset.m_iLimiterEstimatedSourcesPerSlapback;
		return Math.Clamp(value, 1, 4);
	}

	int GetLimiterBurstShotCount()
	{
		return m_iLimiterBurstShotCount;
	}

	bool IsTailsEnabled()
	{
		return m_bEnableTails;
	}

	bool IsSlapbackEnabled()
	{
		return m_bEnableSlapback && BS5_PlayerAudioSettings.IsSlapbackEnabled();
	}

	bool IsPlayerSlapbackEnabled()
	{
		return BS5_PlayerAudioSettings.IsSlapbackEnabled();
	}

	bool IsDebugEnabled()
	{
		if (m_bDebug)
			return true;

		BS5_AudioDebugSettingsComponent debugSettings = GetDebugSettingsComponent();
		if (!debugSettings)
			return false;

		return debugSettings.IsAnyDebugEnabled();
	}

	bool IsLegacyDebugEnabled()
	{
		return m_bDebug;
	}

	bool IsDebugChannelEnabled(BS5_DebugChannel channel, BS5_DebugLevel level = BS5_DebugLevel.BASIC)
	{
		if (m_bDebug)
			return true;

		BS5_AudioDebugSettingsComponent debugSettings = GetDebugSettingsComponent();
		if (!debugSettings)
			return false;

		return debugSettings.Allows(channel, level);
	}

	bool IsSoundMapAnchorPlannerEnabled()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		if (preset)
			return preset.m_bUseSoundMapAnchorPlanner;
		return m_bUseSoundMapAnchorPlanner;
	}

	bool AllowLegacyAnchorFallback()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		if (preset)
			return preset.m_bAllowLegacyAnchorFallback;
		return m_bAllowLegacyAnchorFallback;
	}

	float GetSoundSpeedMetersPerSecond()
	{
		if (m_fSoundSpeedMetersPerSecond > 0.0)
			return m_fSoundSpeedMetersPerSecond;
		return BS5_EchoMath.SPEED_OF_SOUND_MPS;
	}

	float GetFacadeDelayScale()
	{
		return BS5_EchoMath.Clamp(m_fFacadeDelayScale, 0.25, 1.0);
	}

	float GetCachePositionToleranceMeters()
	{
		if (m_fCachePositionToleranceMeters > 0.0)
			return m_fCachePositionToleranceMeters;
		return 0.25;
	}

	float GetCacheHeadingDotTolerance()
	{
		float value = m_fCacheHeadingDotTolerance;
		if (value < 0.0)
			return 0.0;
		if (value > 1.0)
			return 1.0;
		return value;
	}

	float GetTailForwardConeDegrees()
	{
		float coneDegrees = m_fTailForwardConeDegrees;
		if (coneDegrees < 20.0)
			return 20.0;
		if (coneDegrees > 170.0)
			return 170.0;
		return coneDegrees;
	}

	float GetOutsideConeOffsetDegrees()
	{
		return BS5_EchoMath.MaxFloat(0.0, m_fOutsideConeOffsetDegrees);
	}

	float GetEnvQueryRadiusMeters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fEnvQueryRadiusMeters;
		if (preset)
			value = preset.m_fEnvQueryRadiusMeters;
		return BS5_EchoMath.MaxFloat(20.0, value);
	}

	int GetForwardAnchorTraceCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iForwardAnchorTraceCount;
		if (preset)
			value = preset.m_iForwardAnchorTraceCount;
		return Math.Clamp(value, 4, 24);
	}

	int GetLateralAnchorTraceCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iLateralAnchorTraceCount;
		if (preset)
			value = preset.m_iLateralAnchorTraceCount;
		return Math.Clamp(value, 0, 16);
	}

	float GetAnchorPitchClampDegrees()
	{
		return BS5_EchoMath.Clamp(m_fAnchorPitchClampDegrees, 0.0, 25.0);
	}

	float GetAnchorMergeDistanceMeters()
	{
		return BS5_EchoMath.MaxFloat(1.0, m_fAnchorMergeDistanceMeters);
	}

	float GetSettlementMaxDistanceMeters()
	{
		return BS5_EchoMath.MaxFloat(GetPrimaryTailDistanceMin(), m_fSettlementMaxDistanceMeters);
	}

	float GetTerrainMaxDistanceMeters()
	{
		return BS5_EchoMath.MaxFloat(GetSettlementMaxDistanceMeters(), m_fTerrainMaxDistanceMeters);
	}

	float GetOpenTailMinDistanceMeters()
	{
		return BS5_EchoMath.MaxFloat(GetPrimaryTailDistanceMin(), m_fOpenTailMinDistanceMeters);
	}

	float GetOpenTailMaxDistanceMeters()
	{
		return BS5_EchoMath.MaxFloat(GetOpenTailMinDistanceMeters(), m_fOpenTailMaxDistanceMeters);
	}

	int GetOpenTailFallbackCount()
	{
		return Math.Clamp(m_iOpenTailFallbackCount, 1, 4);
	}

	int GetSettlementTailTargetCount()
	{
		return Math.Clamp(m_iSettlementTailTargetCount, 1, 6);
	}

	int GetTerrainTailTargetCount()
	{
		return Math.Clamp(m_iTerrainTailTargetCount, 1, 5);
	}

	int GetIndoorTailTargetCount()
	{
		return Math.Clamp(m_iIndoorTailTargetCount, 1, 4);
	}

	int GetMaxSuppressedTailEmittersPerShot()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iMaxSuppressedTailEmittersPerShot;
		if (preset)
			value = preset.m_iMaxSuppressedTailEmittersPerShot;
		return Math.Clamp(value, 1, 6);
	}

	float GetSuppressedDistanceMultiplier()
	{
		return BS5_EchoMath.Clamp(m_fSuppressedDistanceMultiplier, 0.35, 1.0);
	}

	float GetSuppressedIntensityMultiplier()
	{
		return BS5_EchoMath.Clamp(m_fSuppressedIntensityMultiplier, 0.1, 1.0);
	}

	float GetIndoorMaxDistanceMeters()
	{
		return BS5_EchoMath.MaxFloat(12.0, m_fIndoorMaxDistanceMeters);
	}

	float GetIndoorLateralSpreadScale()
	{
		return BS5_EchoMath.Clamp(m_fIndoorLateralSpreadScale, 0.15, 1.0);
	}

	float GetPrimaryTailDistanceMin()
	{
		return BS5_EchoMath.MaxFloat(1.0, m_fPrimaryTailDistanceMin);
	}

	float GetPrimaryTailDistanceMax()
	{
		return BS5_EchoMath.MaxFloat(GetPrimaryTailDistanceMin(), m_fPrimaryTailDistanceMax);
	}

	float GetSecondaryTailDistanceMin()
	{
		float clampedMin = BS5_EchoMath.MaxFloat(GetPrimaryTailDistanceMin(), m_fSecondaryTailDistanceMin);
		return BS5_EchoMath.MinFloat(GetPrimaryTailDistanceMax(), clampedMin);
	}

	float GetSecondaryTailDistanceMax()
	{
		return BS5_EchoMath.MaxFloat(GetSecondaryTailDistanceMin(), m_fSecondaryTailDistanceMax);
	}

	float GetTertiaryTailDistanceMin()
	{
		float clampedMin = BS5_EchoMath.MaxFloat(GetSecondaryTailDistanceMin(), m_fTertiaryTailDistanceMin);
		return BS5_EchoMath.MinFloat(GetSecondaryTailDistanceMax(), clampedMin);
	}

	float GetTertiaryTailDistanceMax()
	{
		return BS5_EchoMath.MaxFloat(GetTertiaryTailDistanceMin(), m_fTertiaryTailDistanceMax);
	}

	float GetPrimaryCandidateScore()
	{
		return BS5_EchoMath.Clamp01(m_fPrimaryCandidateScore);
	}

	float GetSecondaryCandidateScore()
	{
		return BS5_EchoMath.Clamp01(m_fSecondaryCandidateScore);
	}

	float GetTertiaryCandidateScore()
	{
		return BS5_EchoMath.Clamp01(m_fTertiaryCandidateScore);
	}

	float GetEmitterLifetimeSeconds(bool slapback)
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		if (slapback)
		{
			float slapbackValue = m_fSlapbackEmitterLifetimeSeconds;
			if (preset)
				slapbackValue = preset.m_fSlapbackEmitterLifetimeSeconds;
			return BS5_EchoMath.MaxFloat(1.0, slapbackValue);
		}

		float tailValue = m_fTailEmitterLifetimeSeconds;
		if (preset)
			tailValue = preset.m_fTailEmitterLifetimeSeconds;
		float tailFloor = 3.5;
		if (BS5_PlayerAudioSettings.GetTechnicalPresetId() == "light")
			tailFloor = 2.4;
		return BS5_EchoMath.MaxFloat(tailFloor, tailValue);
	}

	float GetTailEmitterHighPressureLifetimeSeconds()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fTailEmitterHighPressureLifetimeSeconds;
		if (preset)
			value = preset.m_fTailEmitterHighPressureLifetimeSeconds;
		string presetId = BS5_PlayerAudioSettings.GetTechnicalPresetId();
		if (presetId == "light")
			value = BS5_EchoMath.MaxFloat(value, 1.8);
		else if (presetId == "dynamic")
			value = BS5_EchoMath.MaxFloat(value, 2.2);
		else
			value = BS5_EchoMath.MaxFloat(value, 2.4);
		return BS5_EchoMath.Clamp(value, GetTailEmitterMinManagedLifetimeSeconds(), GetEmitterLifetimeSeconds(false));
	}

	float GetTailEmitterMinManagedLifetimeSeconds()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fTailEmitterMinManagedLifetimeSeconds;
		if (preset)
			value = preset.m_fTailEmitterMinManagedLifetimeSeconds;
		string presetId = BS5_PlayerAudioSettings.GetTechnicalPresetId();
		if (presetId == "light")
			value = BS5_EchoMath.MaxFloat(value, 0.9);
		else if (presetId == "dynamic")
			value = BS5_EchoMath.MaxFloat(value, 1.1);
		else
			value = BS5_EchoMath.MaxFloat(value, 1.2);
		return BS5_EchoMath.Clamp(value, 0.25, GetEmitterLifetimeSeconds(false));
	}

	float GetSlapbackMinDistanceMeters()
	{
		return BS5_EchoMath.MaxFloat(0.25, m_fSlapbackMinDistanceMeters);
	}

	float GetSlapbackForwardProbeAngleDegrees()
	{
		return BS5_EchoMath.Clamp(m_fSlapbackForwardProbeAngleDegrees, 0.0, 85.0);
	}

	float GetSlapbackSideProbeAngleDegrees()
	{
		return BS5_EchoMath.Clamp(m_fSlapbackSideProbeAngleDegrees, GetSlapbackForwardProbeAngleDegrees(), 89.0);
	}

	float GetTrenchSlapbackMaxSideDistanceMeters()
	{
		return BS5_EchoMath.Clamp(m_fTrenchSlapbackMaxSideDistanceMeters, 1.0, 8.0);
	}

	float GetTrenchSlapbackMinConfidence()
	{
		return BS5_EchoMath.Clamp(m_fTrenchSlapbackMinConfidence, 0.25, 0.95);
	}

	float GetSlapbackWallNormalMaxY()
	{
		return BS5_EchoMath.Clamp(m_fSlapbackWallNormalMaxY, 0.25, 0.98);
	}

	bool AllowSlapbackAnchorFallback()
	{
		return m_bAllowSlapbackAnchorFallback;
	}

	float GetSlapbackAnchorFallbackMaxDistanceMeters()
	{
		return BS5_EchoMath.Clamp(m_fSlapbackAnchorFallbackMaxDistanceMeters, 1.0, 90.0);
	}

	float GetSlapbackDistanceFarGain()
	{
		return BS5_EchoMath.Clamp(m_fSlapbackDistanceFarGain, 0.0, 1.0);
	}

	float GetSlapbackDistanceGainCurvePower()
	{
		return BS5_EchoMath.Clamp(m_fSlapbackDistanceGainCurvePower, 0.25, 4.0);
	}

	float GetSlapbackDelayMinSeconds()
	{
		if (m_fSlapbackDelayMinSeconds <= 0.001)
			return 0.006;

		return BS5_EchoMath.Clamp(m_fSlapbackDelayMinSeconds, 0.0, 0.04);
	}

	float GetSlapbackWallDelayMaxSeconds()
	{
		if (m_fSlapbackWallDelayMaxSeconds <= 0.001)
			return 0.055;

		return BS5_EchoMath.Clamp(m_fSlapbackWallDelayMaxSeconds, GetSlapbackDelayMinSeconds(), 0.12);
	}

	float GetSlapbackTrenchDelayMaxSeconds()
	{
		if (m_fSlapbackTrenchDelayMaxSeconds <= 0.001)
			return 0.030;

		return BS5_EchoMath.Clamp(m_fSlapbackTrenchDelayMaxSeconds, GetSlapbackDelayMinSeconds(), 0.08);
	}

	float GetSlapbackMergeDistanceMeters()
	{
		return BS5_EchoMath.MaxFloat(0.25, m_fSlapbackMergeDistanceMeters);
	}

	float GetSlapbackWallOffsetMeters()
	{
		return BS5_EchoMath.MaxFloat(0.0, m_fSlapbackWallOffsetMeters);
	}

	float GetSlapbackPathLengthScale()
	{
		return BS5_EchoMath.MaxFloat(0.1, m_fSlapbackPathLengthScale);
	}

	float GetSlapbackIntensityScale()
	{
		return BS5_EchoMath.MaxFloat(0.0, m_fSlapbackIntensityScale);
	}

	int GetMaxCandidateCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iMaxCandidateCount;
		if (preset)
			value = preset.m_iMaxCandidateCount;
		if (value > 0)
			return value;
		return 1;
	}

	int GetMaxTraceCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iMaxTraceCount;
		if (preset)
			value = preset.m_iMaxTraceCount;
		if (value > 0)
			return value;
		return 1;
	}

	float GetReverbSendScale()
	{
		return BS5_EchoMath.MaxFloat(0.0, m_fReverbSendScale);
	}

	float GetTailWidthScale()
	{
		return BS5_EchoMath.MaxFloat(0.0, m_fTailWidthScale);
	}

	float GetTailBrightnessScale()
	{
		return BS5_EchoMath.MaxFloat(0.0, m_fTailBrightnessScale);
	}

	float GetSurfaceHardnessScale()
	{
		return BS5_EchoMath.MaxFloat(0.0, m_fSurfaceHardnessScale);
	}

	float GetSoundMapForwardConeDegrees()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapForwardConeDegrees;
		if (preset)
			value = preset.m_fSoundMapForwardConeDegrees;
		return BS5_EchoMath.Clamp(value, 30.0, 170.0);
	}

	float GetSoundMapForwardMaxDistanceMeters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapForwardMaxDistanceMeters;
		if (preset)
			value = preset.m_fSoundMapForwardMaxDistanceMeters;
		return BS5_EchoMath.Clamp(value, 100.0, 1000.0);
	}

	int GetSoundMapForwardRayCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iSoundMapForwardRayCount;
		if (preset)
			value = preset.m_iSoundMapForwardRayCount;
		return Math.Clamp(value, 3, 15);
	}

	int GetSoundMapForwardSampleCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iSoundMapForwardSampleCount;
		if (preset)
			value = preset.m_iSoundMapForwardSampleCount;
		return Math.Clamp(value, 3, 8);
	}

	float GetSoundMapOmniRadiusMeters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapOmniRadiusMeters;
		if (preset)
			value = preset.m_fSoundMapOmniRadiusMeters;
		return BS5_EchoMath.Clamp(value, 100.0, 700.0);
	}

	int GetSoundMapOmniDirectionCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iSoundMapOmniDirectionCount;
		if (preset)
			value = preset.m_iSoundMapOmniDirectionCount;
		return Math.Clamp(value, 4, 12);
	}

	int GetSoundMapOmniAnchorCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iSoundMapOmniAnchorCount;
		if (preset)
			value = preset.m_iSoundMapOmniAnchorCount;
		return Math.Clamp(value, 0, 4);
	}

	float GetSoundMapCityThreshold()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapCityThreshold;
		if (preset)
			value = preset.m_fSoundMapCityThreshold;
		return BS5_EchoMath.Clamp(value, 0.01, 1.0);
	}

	float GetSoundMapForestThreshold()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapForestThreshold;
		if (preset)
			value = preset.m_fSoundMapForestThreshold;
		return BS5_EchoMath.Clamp(value, 0.01, 1.0);
	}

	float GetSoundMapMeadowThreshold()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapMeadowThreshold;
		if (preset)
			value = preset.m_fSoundMapMeadowThreshold;
		return BS5_EchoMath.Clamp(value, 0.01, 1.0);
	}

	float GetSoundMapHillReliefThreshold()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapHillReliefThreshold;
		if (preset)
			value = preset.m_fSoundMapHillReliefThreshold;
		return BS5_EchoMath.Clamp(value, 0.01, 1.0);
	}

	bool IsSoundMapTerrainFrontSlopeValidationEnabled()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		if (preset)
			return preset.m_bSoundMapTerrainFrontSlopeValidation;
		return m_bSoundMapTerrainFrontSlopeValidation;
	}

	int GetSoundMapTerrainProfileSampleCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iSoundMapTerrainProfileSampleCount;
		if (preset)
			value = preset.m_iSoundMapTerrainProfileSampleCount;
		return Math.Clamp(value, 3, 13);
	}

	float GetSoundMapTerrainBacksideDropMeters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapTerrainBacksideDropMeters;
		if (preset)
			value = preset.m_fSoundMapTerrainBacksideDropMeters;
		return BS5_EchoMath.Clamp(value, 0.5, 12.0);
	}

	bool IsSoundMapUrbanMicroScanEnabled()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		if (preset)
			return preset.m_bSoundMapUrbanMicroScan;
		return m_bSoundMapUrbanMicroScan;
	}

	float GetSoundMapUrbanMicroScanRadiusMeters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapUrbanMicroScanRadiusMeters;
		if (preset)
			value = preset.m_fSoundMapUrbanMicroScanRadiusMeters;
		return BS5_EchoMath.Clamp(value, 40.0, 320.0);
	}

	int GetSoundMapUrbanMicroMaxEntities()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iSoundMapUrbanMicroMaxEntities;
		if (preset)
			value = preset.m_iSoundMapUrbanMicroMaxEntities;
		return Math.Clamp(value, 0, 48);
	}

	float GetSoundMapUrbanScoreBoost()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapUrbanScoreBoost;
		if (preset)
			value = preset.m_fSoundMapUrbanScoreBoost;
		return BS5_EchoMath.Clamp(value, 0.0, 0.35);
	}

	float GetSoundMapDistanceJitter()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapDistanceJitter;
		if (preset)
			value = preset.m_fSoundMapDistanceJitter;
		return BS5_EchoMath.Clamp(value, 0.0, 0.30);
	}

	bool IsSoundMapPathPlausibilityValidationEnabled()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		if (preset)
			return preset.m_bSoundMapPathPlausibilityValidation;
		return m_bSoundMapPathPlausibilityValidation;
	}

	int GetSoundMapPathSampleCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iSoundMapPathSampleCount;
		if (preset)
			value = preset.m_iSoundMapPathSampleCount;
		return Math.Clamp(value, 2, 8);
	}

	float GetSoundMapPathTerrainClearanceMeters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapPathTerrainClearanceMeters;
		if (preset)
			value = preset.m_fSoundMapPathTerrainClearanceMeters;
		return BS5_EchoMath.Clamp(value, 0.5, 6.0);
	}

	float GetSoundMapFarTailSoftLimitMeters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapFarTailSoftLimitMeters;
		if (preset)
			value = preset.m_fSoundMapFarTailSoftLimitMeters;
		return BS5_EchoMath.Clamp(value, 180.0, 900.0);
	}

	float GetSoundMapFarTailHardLimitMeters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapFarTailHardLimitMeters;
		if (preset)
			value = preset.m_fSoundMapFarTailHardLimitMeters;
		return BS5_EchoMath.Clamp(value, GetSoundMapFarTailSoftLimitMeters(), 1000.0);
	}

	float GetSoundMapNearUrbanTailMaxDistanceMeters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapNearUrbanTailMaxDistanceMeters;
		if (preset)
			value = preset.m_fSoundMapNearUrbanTailMaxDistanceMeters;
		return BS5_EchoMath.Clamp(value, 60.0, 260.0);
	}

	float GetSoundMapPathRaycastDistanceMeters()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		float value = m_fSoundMapPathRaycastDistanceMeters;
		if (preset)
			value = preset.m_fSoundMapPathRaycastDistanceMeters;
		return BS5_EchoMath.Clamp(value, 80.0, 500.0);
	}

	float GetDistanceGainNearMeters()
	{
		return BS5_EchoMath.MaxFloat(0.0, m_fDistanceGainNearMeters);
	}

	float GetDistanceGainFarMeters()
	{
		return BS5_EchoMath.MaxFloat(GetDistanceGainNearMeters(), m_fDistanceGainFarMeters);
	}

	float GetDistanceGainFarVolume()
	{
		return BS5_EchoMath.Clamp(m_fDistanceGainFarVolume, 0.0, 1.0);
	}

	float GetDistanceGainCurvePower()
	{
		return BS5_EchoMath.Clamp(m_fDistanceGainCurvePower, 0.1, 4.0);
	}

	int GetTailSectorCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iTailSectorCount;
		if (preset)
			value = preset.m_iTailSectorCount;
		if (value < 3)
			return 3;
		if (value > 15)
			return 15;
		return value;
	}

	int GetTailHeightSampleCount()
	{
		BS5_TechnicalPreset preset = GetActiveTechnicalPreset();
		int value = m_iTailHeightSampleCount;
		if (preset)
			value = preset.m_iTailHeightSampleCount;
		if (value < 1)
			return 1;
		if (value > 4)
			return 4;
		return value;
	}

	float GetTailScanHeightLowMeters()
	{
		return BS5_EchoMath.MaxFloat(0.5, m_fTailScanHeightLowMeters);
	}

	float GetTailScanHeightHighMeters()
	{
		return BS5_EchoMath.MaxFloat(GetTailScanHeightLowMeters(), m_fTailScanHeightHighMeters);
	}

	float GetTailClusterDistanceMeters()
	{
		return BS5_EchoMath.MaxFloat(1.0, m_fTailClusterDistanceMeters);
	}

	float GetTailNormalMergeDot()
	{
		return BS5_EchoMath.Clamp(m_fTailNormalMergeDot, 0.0, 0.999);
	}

	float GetTailGroundNormalMaxY()
	{
		return BS5_EchoMath.Clamp(m_fTailGroundNormalMaxY, 0.1, 0.999);
	}

	float GetTailForwardSectorWeight()
	{
		return BS5_EchoMath.MaxFloat(0.05, m_fTailForwardSectorWeight);
	}

	float GetTailSideSectorWeight()
	{
		return BS5_EchoMath.MaxFloat(0.01, m_fTailSideSectorWeight);
	}

	bool TryGetTailSectorCache(vector origin, vector forward, BS5_TailProfileType profile, bool explosionLike, out array<ref BS5_EchoReflectorCandidate> candidates, out int anchorHits, out int facadeHits)
	{
		anchorHits = 0;
		facadeHits = 0;

		if (!m_bTailSectorCacheValid || !m_aTailSectorCacheCandidates)
			return false;

		if (m_eTailSectorCacheProfile != profile)
			return false;

		if (m_bTailSectorCacheExplosionLike != explosionLike)
			return false;

		float cacheCellSizeMeters = GetTailSectorCacheCellSizeMeters();
		float headingDotTolerance = GetTailSectorCacheHeadingDotTolerance();
		if (m_iTailSectorCacheFacadeHits < 1)
		{
			cacheCellSizeMeters *= 0.45;
			headingDotTolerance = BS5_EchoMath.MaxFloat(headingDotTolerance, 0.992);
		}

		if (vector.DistanceSq(origin, m_vTailSectorCacheOrigin) > cacheCellSizeMeters * cacheCellSizeMeters)
			return false;

		float headingDot = vector.Dot(forward.Normalized(), m_vTailSectorCacheForward.Normalized());
		if (headingDot < headingDotTolerance)
			return false;

		if (candidates)
		{
			candidates.Clear();
			foreach (BS5_EchoReflectorCandidate cachedCandidate : m_aTailSectorCacheCandidates)
			{
				BS5_EchoReflectorCandidate cloneCandidate = BS5_EchoMath.CloneCandidate(cachedCandidate);
				if (cloneCandidate)
					candidates.Insert(cloneCandidate);
			}
		}

		anchorHits = m_iTailSectorCacheAnchorHits;
		facadeHits = m_iTailSectorCacheFacadeHits;
		return true;
	}

	void StoreTailSectorCache(vector origin, vector forward, BS5_TailProfileType profile, bool explosionLike, array<ref BS5_EchoReflectorCandidate> candidates, int anchorHits, int facadeHits)
	{
		if (!m_aTailSectorCacheCandidates)
			m_aTailSectorCacheCandidates = new array<ref BS5_EchoReflectorCandidate>();

		m_aTailSectorCacheCandidates.Clear();
		if (candidates)
		{
			foreach (BS5_EchoReflectorCandidate candidate : candidates)
			{
				BS5_EchoReflectorCandidate cloneCandidate = BS5_EchoMath.CloneCandidate(candidate);
				if (cloneCandidate)
					m_aTailSectorCacheCandidates.Insert(cloneCandidate);
			}
		}

		m_vTailSectorCacheOrigin = origin;
		m_vTailSectorCacheForward = forward;
		m_eTailSectorCacheProfile = profile;
		m_bTailSectorCacheExplosionLike = explosionLike;
		m_iTailSectorCacheAnchorHits = anchorHits;
		m_iTailSectorCacheFacadeHits = facadeHits;
		m_bTailSectorCacheValid = true;
	}

	bool TryGetForwardFacadeNegativeCache(vector origin, vector forward, BS5_TailProfileType profile, bool explosionLike)
	{
		if (!m_bForwardFacadeNegativeCacheValid)
			return false;

		if (m_eForwardFacadeNegativeCacheProfile != profile)
			return false;

		if (m_bForwardFacadeNegativeCacheExplosionLike != explosionLike)
			return false;

		float cacheCellSizeMeters = GetForwardFacadeNegativeCacheCellSizeMeters();
		if (vector.DistanceSq(origin, m_vForwardFacadeNegativeCacheOrigin) > cacheCellSizeMeters * cacheCellSizeMeters)
			return false;

		float headingDot = vector.Dot(forward.Normalized(), m_vForwardFacadeNegativeCacheForward.Normalized());
		return headingDot >= GetForwardFacadeNegativeCacheHeadingDotTolerance();
	}

	void StoreForwardFacadeNegativeCache(vector origin, vector forward, BS5_TailProfileType profile, bool explosionLike)
	{
		m_vForwardFacadeNegativeCacheOrigin = origin;
		m_vForwardFacadeNegativeCacheForward = forward;
		m_eForwardFacadeNegativeCacheProfile = profile;
		m_bForwardFacadeNegativeCacheExplosionLike = explosionLike;
		m_bForwardFacadeNegativeCacheValid = true;
	}

	void ClearForwardFacadeNegativeCache()
	{
		m_bForwardFacadeNegativeCacheValid = false;
	}

	void ClearTailSectorCache()
	{
		m_bTailSectorCacheValid = false;
		m_iTailSectorCacheAnchorHits = 0;
		m_iTailSectorCacheFacadeHits = 0;
		if (m_aTailSectorCacheCandidates)
			m_aTailSectorCacheCandidates.Clear();
	}

	int GetEmissionCount(BS5_EchoAnalysisResult result, bool explosionLike)
	{
		if (!result)
			return 0;

		if (explosionLike)
		{
			int explosionCount = GetMaxExplosionEmittersPerShot();
			if (explosionCount > result.m_aCandidates.Count())
				explosionCount = result.m_aCandidates.Count();
			return explosionCount;
		}

		int count = GetMaxTailEmittersPerShot();
		if (result.m_bSuppressedShot)
		{
			int suppressedCount = GetMaxSuppressedTailEmittersPerShot();
			if (suppressedCount < count)
				count = suppressedCount;
		}

		if (IsPlaybackLimiterEnabled())
		{
			if (m_iLimiterBurstShotCount > 1)
			{
				int burstCount = GetBurstTailEmittersPerShot();
				if (burstCount < count)
					count = burstCount;
			}

			float tailPressure = BS5_EchoEmissionService.GetTailVoicePressure(this);
			if (tailPressure >= GetLimiterHighPressureThreshold())
			{
				int highPressureCount = GetHighPressureTailEmittersPerShot();
				if (highPressureCount < count)
					count = highPressureCount;
			}
		}

		if (count > result.m_aCandidates.Count())
			count = result.m_aCandidates.Count();
		return count;
	}

	bool ShouldAcceptCandidate(float score)
	{
		return score > 0.03;
	}

	bool TryAcquireActiveEmitterBudget(bool slapback)
	{
		if (slapback)
		{
			int slapbackLimit = GetMaxActiveSlapbackEmitters();
			if (slapbackLimit > 0 && m_iActiveSlapbackEmitters >= slapbackLimit)
				return false;

			m_iActiveSlapbackEmitters++;
			return true;
		}

		int tailLimit = GetMaxActiveTailEmitters();
		if (tailLimit > 0 && m_iActiveTailEmitters >= tailLimit)
			return false;

		m_iActiveTailEmitters++;
		return true;
	}

	void ReleaseActiveEmitterBudget(bool slapback)
	{
		if (slapback)
		{
			if (m_iActiveSlapbackEmitters > 0)
				m_iActiveSlapbackEmitters--;
			return;
		}

		if (m_iActiveTailEmitters > 0)
			m_iActiveTailEmitters--;
	}

	protected BS5_EchoAnalysisResult ResolveCachedResult(IEntity owner, vector origin, vector forward, bool explosionLike, bool suppressed)
	{
		if (!m_bCacheValid || !m_LastResult)
			return null;

		if (explosionLike && !m_bEnableExplosionReuse)
			return null;

		float positionTolerance = GetCachePositionToleranceMeters();
		if (vector.DistanceSq(origin, m_vLastOrigin) > positionTolerance * positionTolerance)
			return null;

		float headingTolerance = GetCacheHeadingDotTolerance();
		float headingDot = vector.Dot(forward.Normalized(), m_vLastForward.Normalized());
		if (headingDot < headingTolerance)
			return null;

		if (!explosionLike && suppressed != m_bLastSuppressed)
			return null;

		return m_LastResult;
	}

	protected vector FlattenHeading(vector forward)
	{
		vector planarForward = forward;
		planarForward[1] = 0.0;
		if (planarForward.LengthSq() < 0.0001)
			return "0 0 1";

		planarForward.Normalize();
		return planarForward;
	}

	protected void StoreCachedResult(BS5_EchoAnalysisResult result, bool suppressed)
	{
		m_iCacheGeneration++;
		m_LastResult = result;
		m_bCacheValid = true;
		m_bLastSuppressed = suppressed;

		float cacheLifetime = m_fCacheTtlSeconds;
		if (m_fBurstReuseWindowSeconds > cacheLifetime)
			cacheLifetime = m_fBurstReuseWindowSeconds;
		if (cacheLifetime <= 0.0)
			cacheLifetime = 0.05;

		int cacheGeneration = m_iCacheGeneration;
		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		if (callQueue)
			callQueue.CallLater(InvalidateCache, (int)(cacheLifetime * 1000.0), false, cacheGeneration);
	}

	void InvalidateCache(int cacheGeneration)
	{
		if (cacheGeneration != m_iCacheGeneration)
			return;

		m_bCacheValid = false;
		m_LastResult = null;
	}

	protected float GetTailSectorCacheCellSizeMeters()
	{
		return BS5_EchoMath.MaxFloat(32.0, GetTailClusterDistanceMeters() * 3.0);
	}

	protected float GetTailSectorCacheHeadingDotTolerance()
	{
		return 0.88;
	}

	protected float GetForwardFacadeNegativeCacheCellSizeMeters()
	{
		return BS5_EchoMath.Clamp(GetTailClusterDistanceMeters() * 2.5, 20.0, 36.0);
	}

	protected float GetForwardFacadeNegativeCacheHeadingDotTolerance()
	{
		return 0.965;
	}

	protected IEntity GetOwnerEntity(IEntity effectEntity, BaseMuzzleComponent muzzle)
	{
		if (muzzle)
			return muzzle.GetOwner();

		if (effectEntity)
			return effectEntity;

		return GetOwner();
	}

	protected bool IsEmitterPrefabPath(ResourceName resourceName)
	{
		if (resourceName == string.Empty)
			return false;

		return resourceName.EndsWith(".et");
	}

	protected bool ShouldSuppressDuplicateDispatch(vector origin, vector forward)
	{
		if (!m_bDispatchGuardActive)
			return false;

		if (vector.DistanceSq(origin, m_vDispatchGuardOrigin) > 0.05 * 0.05)
			return false;

		return vector.Dot(forward.Normalized(), m_vDispatchGuardForward.Normalized()) > 0.999;
	}

	protected bool ShouldEmitShotForPlaybackLimiter(bool suppressed)
	{
		if (!IsPlaybackLimiterEnabled())
			return true;

		m_iLimiterBurstGeneration++;
		m_iLimiterBurstShotCount++;

		int burstGeneration = m_iLimiterBurstGeneration;
		int resetMs = (int)(BS5_EchoMath.Clamp(m_fBurstReuseWindowSeconds, 0.12, 0.85) * 1000.0);
		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		if (callQueue)
			callQueue.CallLater(ResetPlaybackLimiterBurst, resetMs, false, burstGeneration);

		if (m_iLimiterBurstShotCount <= 1)
			return true;

		float tailPressure = BS5_EchoEmissionService.GetTailVoicePressure(this);
		int cadence = GetBurstTailCadenceNormal();
		if (tailPressure >= GetLimiterHighPressureThreshold())
			cadence = GetBurstTailCadenceHighPressure();

		bool acceptShot = ((m_iLimiterBurstShotCount - 1) % cadence) == 0;
		if (!acceptShot && IsDebugChannelEnabled(BS5_DebugChannel.LIMITER))
			BS5_DebugLog.Channel(this, BS5_DebugChannel.LIMITER, "dispatch skipped burstLimiter shot=" + m_iLimiterBurstShotCount + " cadence=" + cadence + " pressure=" + tailPressure + " suppressed=" + suppressed);

		return acceptShot;
	}

	void ResetPlaybackLimiterBurst(int burstGeneration)
	{
		if (burstGeneration != m_iLimiterBurstGeneration)
			return;

		m_iLimiterBurstShotCount = 0;
	}

	protected void ActivateDispatchGuard(vector origin, vector forward)
	{
		m_bDispatchGuardActive = true;
		m_vDispatchGuardOrigin = origin;
		m_vDispatchGuardForward = forward;

		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		if (callQueue)
			callQueue.CallLater(ClearDispatchGuard, 10, false);
	}

	void ClearDispatchGuard()
	{
		m_bDispatchGuardActive = false;
	}

	protected BS5_WeaponEchoSettingsComponent GetSettingsComponent()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;

		return BS5_WeaponEchoSettingsComponent.Cast(owner.FindComponent(BS5_WeaponEchoSettingsComponent));
	}

	BS5_CloseReflectionSettingsComponent GetCloseReflectionSettingsComponent()
	{
		if (m_CloseReflectionSettings)
			return m_CloseReflectionSettings;

		IEntity owner = GetOwner();
		if (!owner)
			return null;

		m_CloseReflectionSettings = BS5_CloseReflectionSettingsComponent.Cast(owner.FindComponent(BS5_CloseReflectionSettingsComponent));
		return m_CloseReflectionSettings;
	}

	BS5_AudioDebugSettingsComponent GetDebugSettingsComponent()
	{
		if (m_DebugSettings)
			return m_DebugSettings;

		IEntity owner = GetOwner();
		if (!owner)
			return null;

		m_DebugSettings = BS5_AudioDebugSettingsComponent.Cast(owner.FindComponent(BS5_AudioDebugSettingsComponent));
		return m_DebugSettings;
	}

	protected BS5_WeaponEchoRplCharacterComponent GetCharacterComponent()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;

		return BS5_WeaponEchoRplCharacterComponent.Cast(owner.FindComponent(BS5_WeaponEchoRplCharacterComponent));
	}

	protected bool IsSuppressedMuzzle(BaseMuzzleComponent muzzle)
	{
		if (!muzzle)
			return false;

		return muzzle.IsMuzzleSuppressed();
	}

	protected void DebugValidateConfiguration(IEntity owner)
	{
		if (!IsDebugChannelEnabled(BS5_DebugChannel.DRIVER))
			return;

		if (ResolveMasterEmitterPrefab(false) == string.Empty)
			PrintFormat("BS5 sanity warning: missing tail emitter prefab on %1", owner);

		if (ResolveMasterEmitterPrefab(true) == string.Empty)
			PrintFormat("BS5 sanity warning: missing suppressed tail emitter prefab on %1", owner);

		if (ResolveMasterEventName() == string.Empty)
			PrintFormat("BS5 sanity warning: missing tail event name on %1", owner);

		if (m_bEnableSlapback && ResolveSlapbackEmitterPrefab() == string.Empty)
			PrintFormat("BS5 sanity warning: missing slapback emitter prefab on %1", owner);

		if (m_bEnableSlapback && ResolveSlapbackEventName() == string.Empty)
			PrintFormat("BS5 sanity warning: missing slapback event name on %1", owner);

		BS5_CloseReflectionSettingsComponent closeSettings = GetCloseReflectionSettingsComponent();
		if (!closeSettings)
			PrintFormat("BS5 sanity warning: missing close reflection settings component on %1", owner);
		else if (closeSettings.IsEnabled())
			PrintFormat("BS5 close reflection config: acp=%1 prefab=%2 event=%3", closeSettings.ResolveCloseSlapbackAcp(), closeSettings.ResolveCloseSlapbackEmitterPrefab(), closeSettings.ResolveCloseSlapbackEventName());

		if (GetMaxCandidateCount() < GetMaxTailEmittersPerShot())
			PrintFormat("BS5 sanity warning: max candidates (%1) is lower than tail emitters per shot (%2) on %3", GetMaxCandidateCount(), GetMaxTailEmittersPerShot(), owner);

		PrintFormat("BS5 soundmap anchor config: enabled=%1 legacyFallback=%2 cone=%3 maxDistance=%4 rays=%5 samples=%6",
			IsSoundMapAnchorPlannerEnabled(),
			AllowLegacyAnchorFallback(),
			GetSoundMapForwardConeDegrees(),
			GetSoundMapForwardMaxDistanceMeters(),
			GetSoundMapForwardRayCount(),
			GetSoundMapForwardSampleCount());

		PrintFormat("BS5 soundmap anchor thresholds: omniRadius=%1 omniDirs=%2 omniAnchors=%3 city=%4 forest=%5 meadow=%6 hill=%7",
			GetSoundMapOmniRadiusMeters(),
			GetSoundMapOmniDirectionCount(),
			GetSoundMapOmniAnchorCount(),
			GetSoundMapCityThreshold(),
			GetSoundMapForestThreshold(),
			GetSoundMapMeadowThreshold(),
			GetSoundMapHillReliefThreshold());

		PrintFormat("BS5 soundmap physical tuning: frontSlope=%1 profileSamples=%2 backsideDrop=%3 urbanMicro=%4 urbanRadius=%5 urbanEntities=%6 urbanBoost=%7 distJitter=%8",
			IsSoundMapTerrainFrontSlopeValidationEnabled(),
			GetSoundMapTerrainProfileSampleCount(),
			GetSoundMapTerrainBacksideDropMeters(),
			IsSoundMapUrbanMicroScanEnabled(),
			GetSoundMapUrbanMicroScanRadiusMeters(),
			GetSoundMapUrbanMicroMaxEntities(),
			GetSoundMapUrbanScoreBoost(),
			GetSoundMapDistanceJitter());

		string pathTuning = "BS5 soundmap path gate:";
		pathTuning += " enabled=" + IsSoundMapPathPlausibilityValidationEnabled();
		pathTuning += " samples=" + GetSoundMapPathSampleCount();
		pathTuning += " clearance=" + GetSoundMapPathTerrainClearanceMeters();
		pathTuning += " softFar=" + GetSoundMapFarTailSoftLimitMeters();
		pathTuning += " hardFar=" + GetSoundMapFarTailHardLimitMeters();
		pathTuning += " nearUrban=" + GetSoundMapNearUrbanTailMaxDistanceMeters();
		pathTuning += " raycastDistance=" + GetSoundMapPathRaycastDistanceMeters();
		Print(pathTuning);

		PrintFormat("BS5 tail scan config: sectors=%1 heights=%2 low=%3 high=%4 cluster=%5 groundNormalMaxY=%6 forwardWeight=%7 sideWeight=%8",
			GetTailSectorCount(),
			GetTailHeightSampleCount(),
			GetTailScanHeightLowMeters(),
			GetTailScanHeightHighMeters(),
			GetTailClusterDistanceMeters(),
			GetTailGroundNormalMaxY(),
			GetTailForwardSectorWeight(),
			GetTailSideSectorWeight());

		PrintFormat("BS5 hybrid tail config: envRadius=%1 forwardSeeds=%2 lateralSeeds=%3 pitchClamp=%4 merge=%5 settlementMax=%6 terrainMax=%7 openMin=%8 openMax=%9",
			GetEnvQueryRadiusMeters(),
			GetForwardAnchorTraceCount(),
			GetLateralAnchorTraceCount(),
			GetAnchorPitchClampDegrees(),
			GetAnchorMergeDistanceMeters(),
			GetSettlementMaxDistanceMeters(),
			GetTerrainMaxDistanceMeters(),
			GetOpenTailMinDistanceMeters(),
			GetOpenTailMaxDistanceMeters());

		PrintFormat("BS5 production tail config: suppressedEmitters=%1 suppressedDistance=%2 suppressedIntensity=%3 indoorCount=%4 indoorMax=%5 indoorLateral=%6",
			GetMaxSuppressedTailEmittersPerShot(),
			GetSuppressedDistanceMultiplier(),
			GetSuppressedIntensityMultiplier(),
			GetIndoorTailTargetCount(),
			GetIndoorMaxDistanceMeters(),
			GetIndoorLateralSpreadScale());

		PrintFormat("BS5 distance gain config: near=%1 far=%2 farVolume=%3 curve=%4",
			GetDistanceGainNearMeters(),
			GetDistanceGainFarMeters(),
			GetDistanceGainFarVolume(),
			GetDistanceGainCurvePower());
	}
}

[ComponentEditorProps(category: "BS5 Audio", description: "Legacy BS5 echo settings carrier for prefab compatibility.")]
class BS5_WeaponEchoSettingsComponentClass : ScriptComponentClass
{
}

class BS5_WeaponEchoSettingsComponent : ScriptComponent
{
	[Attribute(defvalue: "")]
	protected ResourceName m_EchoProject;

	[Attribute(defvalue: "")]
	protected ResourceName m_SuppressedEchoProject;

	[Attribute(defvalue: "")]
	protected ResourceName m_SlapbackProject;

	[Attribute(defvalue: "")]
	protected ResourceName m_SuppressedSlapbackProject;

	[Attribute(defvalue: "SOUND_SHOT")]
	protected string m_sEchoEventName;

	[Attribute(defvalue: "SOUND_SHOT")]
	protected string m_sSlapbackEventName;

	ResourceName GetEchoProject()
	{
		return m_EchoProject;
	}

	ResourceName GetSuppressedEchoProject()
	{
		return m_SuppressedEchoProject;
	}

	ResourceName GetSlapbackProject()
	{
		return m_SlapbackProject;
	}

	ResourceName GetSuppressedSlapbackProject()
	{
		return m_SuppressedSlapbackProject;
	}

	string GetEchoEventName()
	{
		return m_sEchoEventName;
	}

	string GetSlapbackEventName()
	{
		return m_sSlapbackEventName;
	}
}

[ComponentEditorProps(category: "BS5 Audio", description: "Character-side default BS5 echo ACP holder.")]
class BS5_WeaponEchoRplCharacterComponentClass : ScriptComponentClass
{
}

class BS5_WeaponEchoRplCharacterComponent : ScriptComponent
{
	[Attribute(defvalue: "")]
	protected ResourceName m_DefaultEchoAcp;

	ResourceName GetDefaultEchoAcp()
	{
		return m_DefaultEchoAcp;
	}
}

modded class SCR_MuzzleEffectComponent
{
	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		super.OnFired(effectEntity, muzzle, projectileEntity);

		BS5_EchoDriverComponent driver = BS5_EchoRuntime.FindDriver(effectEntity, muzzle);
		if (driver)
			driver.HandleWeaponFire(effectEntity, muzzle, projectileEntity);
	}
}
