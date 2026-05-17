[BaseContainerProps(configRoot: true)]
class BS5_SoundPresetRegistryConfig
{
	[Attribute()]
	ref array<ref BS5_SoundPresetConfigEntry> m_aPresets;
}

[BaseContainerProps()]
class BS5_SoundPresetConfigEntry
{
	[Attribute(defvalue: "vanilla")]
	string m_sId;

	[Attribute(defvalue: "Vanilla")]
	string m_sDisplayName;

	[Attribute(defvalue: "0.75")]
	float m_fEchoVolume;

	[Attribute(defvalue: "0.60")]
	float m_fSlapbackVolume;

	[Attribute(defvalue: "0.80")]
	float m_fSlapbackCloseVolume;

	[Attribute(defvalue: "0.75")]
	float m_fExplosionVolume;
}

[BaseContainerProps(configRoot: true)]
class BS5_TechnicalPresetRegistryConfig
{
	[Attribute()]
	ref array<ref BS5_TechnicalPresetConfigEntry> m_aPresets;
}

[BaseContainerProps()]
class BS5_TechnicalPresetConfigEntry
{
	[Attribute(defvalue: "default")]
	string m_sId;

	[Attribute(defvalue: "Default")]
	string m_sDisplayName;

	[Attribute(defvalue: "1")]
	bool m_bUseSoundMapAnchorPlanner;

	[Attribute(defvalue: "0")]
	bool m_bAllowLegacyAnchorFallback;

	[Attribute(defvalue: "700")]
	float m_fScanRadius;

	[Attribute(defvalue: "25")]
	float m_fNearSlapbackRadius;

	[Attribute(defvalue: "16")]
	float m_fSuppressedNearSlapbackRadius;

	[Attribute(defvalue: "25")]
	float m_fExplosionNearSlapbackRadius;

	[Attribute(defvalue: "2")]
	int m_iNearReflectionImplementationMode;

	[Attribute(defvalue: "0")]
	bool m_bNearReflectionTailOcclusionEnabled;

	[Attribute(defvalue: "12")]
	int m_iNearReflectionProbeCount;

	[Attribute(defvalue: "42")]
	float m_fNearReflectionMaxDistanceMeters;

	[Attribute(defvalue: "12")]
	float m_fNearReflectionTightDistanceMeters;

	[Attribute(defvalue: "15")]
	float m_fNearReflectionSemiIndoorWallDistanceMeters;

	[Attribute(defvalue: "6")]
	float m_fNearReflectionCanopyMaxDistanceMeters;

	[Attribute(defvalue: "0.50")]
	float m_fNearReflectionSectorOcclusionStrength;

	[Attribute(defvalue: "2")]
	int m_iNearReflectionMaxDirectionalCloseCandidates;

	[Attribute(defvalue: "1")]
	int m_iNearReflectionMaxCoreCandidatesPerShot;

	[Attribute(defvalue: "1")]
	int m_iNearReflectionMaxSemiIndoorCandidatesPerShot;

	[Attribute(defvalue: "12")]
	int m_iExplosionNearReflectionProbeCount;

	[Attribute(defvalue: "95")]
	float m_fExplosionNearReflectionMaxDistanceMeters;

	[Attribute(defvalue: "18")]
	float m_fExplosionNearReflectionTightDistanceMeters;

	[Attribute(defvalue: "0.65")]
	float m_fExplosionNearReflectionSectorOcclusionStrength;

	[Attribute(defvalue: "2")]
	int m_iExplosionNearReflectionMaxCloseCandidatesPerShot;

	[Attribute(defvalue: "1.0")]
	float m_fExplosionNearReflectionCloseStrength;

	[Attribute(defvalue: "0")]
	float m_fExplosionScanRadiusMeters;

	[Attribute(defvalue: "9")]
	int m_iMaxCandidateCount;

	[Attribute(defvalue: "9")]
	int m_iMaxTraceCount;

	[Attribute(defvalue: "4")]
	int m_iMaxTailEmittersPerShot;

	[Attribute(defvalue: "0")]
	int m_iMaxExplosionEmittersPerShot;

	[Attribute(defvalue: "4")]
	int m_iMaxSuppressedTailEmittersPerShot;

	[Attribute(defvalue: "2")]
	int m_iMaxSlapbackEmittersPerShot;

	[Attribute(defvalue: "3")]
	int m_iMaxExplosionSlapbackEmittersPerShot;

	[Attribute(defvalue: "24")]
	int m_iMaxActiveTailEmitters;

	[Attribute(defvalue: "6")]
	int m_iMaxActiveSlapbackEmitters;

	[Attribute(defvalue: "24")]
	int m_iLimiterGlobalMaxTailVoices;

	[Attribute(defvalue: "6")]
	int m_iLimiterGlobalMaxSlapbackVoices;

	[Attribute(defvalue: "18")]
	int m_iLimiterMaxPendingTailVoices;

	[Attribute(defvalue: "8")]
	int m_iLimiterMaxTailVoicesPerOwner;

	[Attribute(defvalue: "3")]
	int m_iLimiterBurstCadenceNormal;

	[Attribute(defvalue: "5")]
	int m_iLimiterBurstCadenceHighPressure;

	[Attribute(defvalue: "3")]
	int m_iLimiterBurstFreeTailShotsNormal;

	[Attribute(defvalue: "2")]
	int m_iLimiterBurstFreeTailShotsHighPressure;

	[Attribute(defvalue: "2")]
	int m_iLimiterBurstTailEmittersPerShot;

	[Attribute(defvalue: "1")]
	int m_iLimiterHighPressureTailEmittersPerShot;

	[Attribute(defvalue: "0.72")]
	float m_fLimiterHighPressureThreshold;

	[Attribute(defvalue: "0.92")]
	float m_fLimiterCriticalPressureThreshold;

	[Attribute(defvalue: "4")]
	int m_iLimiterMaxTailStartsPer100Ms;

	[Attribute(defvalue: "2")]
	int m_iLimiterMaxSlapbackStartsPer100Ms;

	[Attribute(defvalue: "0.08")]
	float m_fLimiterStealFadeSeconds;

	[Attribute(defvalue: "2")]
	int m_iLimiterEstimatedSourcesPerTail;

	[Attribute(defvalue: "1")]
	int m_iLimiterEstimatedSourcesPerSlapback;

	[Attribute(defvalue: "4.5")]
	float m_fTailEmitterLifetimeSeconds;

	[Attribute(defvalue: "2.4")]
	float m_fTailEmitterHighPressureLifetimeSeconds;

	[Attribute(defvalue: "1.35")]
	float m_fTailEmitterMinManagedLifetimeSeconds;

	[Attribute(defvalue: "1.5")]
	float m_fSlapbackEmitterLifetimeSeconds;

	[Attribute(defvalue: "0")]
	float m_fExplosionSlapbackEmitterLifetimeSeconds;

	[Attribute(defvalue: "0")]
	float m_fExplosionEmitterLifetimeScale;

	[Attribute(defvalue: "0")]
	float m_fSlapbackMinDistanceMeters;

	[Attribute(defvalue: "400")]
	float m_fEnvQueryRadiusMeters;

	[Attribute(defvalue: "18")]
	int m_iForwardAnchorTraceCount;

	[Attribute(defvalue: "8")]
	int m_iLateralAnchorTraceCount;

	[Attribute(defvalue: "9")]
	int m_iTailSectorCount;

	[Attribute(defvalue: "3")]
	int m_iTailHeightSampleCount;

	[Attribute(defvalue: "120")]
	float m_fSoundMapForwardConeDegrees;

	[Attribute(defvalue: "700")]
	float m_fSoundMapForwardMaxDistanceMeters;

	[Attribute(defvalue: "9")]
	int m_iSoundMapForwardRayCount;

	[Attribute(defvalue: "7")]
	int m_iSoundMapForwardSampleCount;

	[Attribute(defvalue: "450")]
	float m_fSoundMapOmniRadiusMeters;

	[Attribute(defvalue: "8")]
	int m_iSoundMapOmniDirectionCount;

	[Attribute(defvalue: "2")]
	int m_iSoundMapOmniAnchorCount;

	[Attribute(defvalue: "0.08")]
	float m_fSoundMapCityThreshold;

	[Attribute(defvalue: "0.14")]
	float m_fSoundMapForestThreshold;

	[Attribute(defvalue: "0.25")]
	float m_fSoundMapMeadowThreshold;

	[Attribute(defvalue: "0.18")]
	float m_fSoundMapHillReliefThreshold;

	[Attribute(defvalue: "1")]
	bool m_bSoundMapTerrainFrontSlopeValidation;

	[Attribute(defvalue: "9")]
	int m_iSoundMapTerrainProfileSampleCount;

	[Attribute(defvalue: "2.8")]
	float m_fSoundMapTerrainBacksideDropMeters;

	[Attribute(defvalue: "1")]
	bool m_bSoundMapUrbanMicroScan;

	[Attribute(defvalue: "180")]
	float m_fSoundMapUrbanMicroScanRadiusMeters;

	[Attribute(defvalue: "28")]
	int m_iSoundMapUrbanMicroMaxEntities;

	[Attribute(defvalue: "0.18")]
	float m_fSoundMapUrbanScoreBoost;

	[Attribute(defvalue: "0.16")]
	float m_fSoundMapDistanceJitter;

	[Attribute(defvalue: "1")]
	bool m_bSoundMapPathPlausibilityValidation;

	[Attribute(defvalue: "6")]
	int m_iSoundMapPathSampleCount;

	[Attribute(defvalue: "2.2")]
	float m_fSoundMapPathTerrainClearanceMeters;

	[Attribute(defvalue: "420")]
	float m_fSoundMapFarTailSoftLimitMeters;

	[Attribute(defvalue: "560")]
	float m_fSoundMapFarTailHardLimitMeters;

	[Attribute(defvalue: "180")]
	float m_fSoundMapNearUrbanTailMaxDistanceMeters;

	[Attribute(defvalue: "220")]
	float m_fSoundMapPathRaycastDistanceMeters;
}

class BS5_SoundPreset
{
	string m_sId;
	string m_sDisplayName;
	float m_fEchoVolume;
	float m_fSlapbackVolume;
	float m_fSlapbackCloseVolume;
	float m_fExplosionVolume;
}

class BS5_TechnicalPreset
{
	string m_sId;
	string m_sDisplayName;
	bool m_bUseSoundMapAnchorPlanner;
	bool m_bAllowLegacyAnchorFallback;
	float m_fScanRadius;
	float m_fNearSlapbackRadius;
	float m_fSuppressedNearSlapbackRadius;
	float m_fExplosionNearSlapbackRadius;
	int m_iNearReflectionImplementationMode;
	bool m_bNearReflectionTailOcclusionEnabled;
	int m_iNearReflectionProbeCount;
	float m_fNearReflectionMaxDistanceMeters;
	float m_fNearReflectionTightDistanceMeters;
	float m_fNearReflectionSemiIndoorWallDistanceMeters;
	float m_fNearReflectionCanopyMaxDistanceMeters;
	float m_fNearReflectionSectorOcclusionStrength;
	int m_iNearReflectionMaxDirectionalCloseCandidates;
	int m_iNearReflectionMaxCoreCandidatesPerShot;
	int m_iNearReflectionMaxSemiIndoorCandidatesPerShot;
	int m_iExplosionNearReflectionProbeCount;
	float m_fExplosionNearReflectionMaxDistanceMeters;
	float m_fExplosionNearReflectionTightDistanceMeters;
	float m_fExplosionNearReflectionSectorOcclusionStrength;
	int m_iExplosionNearReflectionMaxCloseCandidatesPerShot;
	float m_fExplosionNearReflectionCloseStrength;
	float m_fExplosionScanRadiusMeters;
	int m_iMaxCandidateCount;
	int m_iMaxTraceCount;
	int m_iMaxTailEmittersPerShot;
	int m_iMaxExplosionEmittersPerShot;
	int m_iMaxSuppressedTailEmittersPerShot;
	int m_iMaxSlapbackEmittersPerShot;
	int m_iMaxExplosionSlapbackEmittersPerShot;
	int m_iMaxActiveTailEmitters;
	int m_iMaxActiveSlapbackEmitters;
	int m_iLimiterGlobalMaxTailVoices;
	int m_iLimiterGlobalMaxSlapbackVoices;
	int m_iLimiterMaxPendingTailVoices;
	int m_iLimiterMaxTailVoicesPerOwner;
	int m_iLimiterBurstCadenceNormal;
	int m_iLimiterBurstCadenceHighPressure;
	int m_iLimiterBurstFreeTailShotsNormal;
	int m_iLimiterBurstFreeTailShotsHighPressure;
	int m_iLimiterBurstTailEmittersPerShot;
	int m_iLimiterHighPressureTailEmittersPerShot;
	float m_fLimiterHighPressureThreshold;
	float m_fLimiterCriticalPressureThreshold;
	int m_iLimiterMaxTailStartsPer100Ms;
	int m_iLimiterMaxSlapbackStartsPer100Ms;
	float m_fLimiterStealFadeSeconds;
	int m_iLimiterEstimatedSourcesPerTail;
	int m_iLimiterEstimatedSourcesPerSlapback;
	float m_fTailEmitterLifetimeSeconds;
	float m_fTailEmitterHighPressureLifetimeSeconds;
	float m_fTailEmitterMinManagedLifetimeSeconds;
	float m_fSlapbackEmitterLifetimeSeconds;
	float m_fExplosionSlapbackEmitterLifetimeSeconds;
	float m_fExplosionEmitterLifetimeScale;
	float m_fSlapbackMinDistanceMeters;
	float m_fEnvQueryRadiusMeters;
	int m_iForwardAnchorTraceCount;
	int m_iLateralAnchorTraceCount;
	int m_iTailSectorCount;
	int m_iTailHeightSampleCount;
	float m_fSoundMapForwardConeDegrees;
	float m_fSoundMapForwardMaxDistanceMeters;
	int m_iSoundMapForwardRayCount;
	int m_iSoundMapForwardSampleCount;
	float m_fSoundMapOmniRadiusMeters;
	int m_iSoundMapOmniDirectionCount;
	int m_iSoundMapOmniAnchorCount;
	float m_fSoundMapCityThreshold;
	float m_fSoundMapForestThreshold;
	float m_fSoundMapMeadowThreshold;
	float m_fSoundMapHillReliefThreshold;
	bool m_bSoundMapTerrainFrontSlopeValidation;
	int m_iSoundMapTerrainProfileSampleCount;
	float m_fSoundMapTerrainBacksideDropMeters;
	bool m_bSoundMapUrbanMicroScan;
	float m_fSoundMapUrbanMicroScanRadiusMeters;
	int m_iSoundMapUrbanMicroMaxEntities;
	float m_fSoundMapUrbanScoreBoost;
	float m_fSoundMapDistanceJitter;
	bool m_bSoundMapPathPlausibilityValidation;
	int m_iSoundMapPathSampleCount;
	float m_fSoundMapPathTerrainClearanceMeters;
	float m_fSoundMapFarTailSoftLimitMeters;
	float m_fSoundMapFarTailHardLimitMeters;
	float m_fSoundMapNearUrbanTailMaxDistanceMeters;
	float m_fSoundMapPathRaycastDistanceMeters;
}

class BS5_PresetRegistry
{
	protected static const ResourceName SOUND_PRESETS_CONFIG = "{781576466220AFAF}Configs/BS5/Presets/BS5_SoundPresets.conf";
	protected static const ResourceName TECHNICAL_PRESETS_CONFIG = "{C1D3C936760CEC8B}Configs/BS5/Presets/BS5_TechnicalPresets.conf";
	protected static const string DEFAULT_SOUND_PRESET_ID = "vanilla";
	protected static const string DEFAULT_TECHNICAL_PRESET_ID = "default";
	protected static const string CUSTOM_SOUND_PRESET_ID = "custom";

	protected static bool s_bInitialized;
	protected static ref array<ref BS5_SoundPreset> s_aSoundPresets;
	protected static ref array<ref BS5_TechnicalPreset> s_aTechnicalPresets;

	static string GetDefaultSoundPresetId()
	{
		return DEFAULT_SOUND_PRESET_ID;
	}

	static string GetDefaultTechnicalPresetId()
	{
		return DEFAULT_TECHNICAL_PRESET_ID;
	}

	static string GetCustomSoundPresetId()
	{
		return CUSTOM_SOUND_PRESET_ID;
	}

	static int GetSoundPresetCount()
	{
		EnsureInitialized();
		return s_aSoundPresets.Count();
	}

	static int GetTechnicalPresetCount()
	{
		EnsureInitialized();
		return s_aTechnicalPresets.Count();
	}

	static BS5_SoundPreset GetSoundPresetByIndex(int index)
	{
		EnsureInitialized();
		if (s_aSoundPresets.IsEmpty())
			return null;

		index = WrapIndex(index, s_aSoundPresets.Count());
		return s_aSoundPresets[index];
	}

	static BS5_TechnicalPreset GetTechnicalPresetByIndex(int index)
	{
		EnsureInitialized();
		if (s_aTechnicalPresets.IsEmpty())
			return null;

		index = WrapIndex(index, s_aTechnicalPresets.Count());
		return s_aTechnicalPresets[index];
	}

	static BS5_SoundPreset GetSoundPreset(string id)
	{
		EnsureInitialized();
		if (id == string.Empty)
			id = DEFAULT_SOUND_PRESET_ID;

		foreach (BS5_SoundPreset preset : s_aSoundPresets)
		{
			if (preset && preset.m_sId == id)
				return preset;
		}

		return FindSoundPreset(DEFAULT_SOUND_PRESET_ID);
	}

	static BS5_TechnicalPreset GetTechnicalPreset(string id)
	{
		EnsureInitialized();
		if (id == string.Empty)
			id = DEFAULT_TECHNICAL_PRESET_ID;

		foreach (BS5_TechnicalPreset preset : s_aTechnicalPresets)
		{
			if (preset && preset.m_sId == id)
				return preset;
		}

		return FindTechnicalPreset(DEFAULT_TECHNICAL_PRESET_ID);
	}

	static BS5_TechnicalPreset GetActiveTechnicalPreset()
	{
		return GetTechnicalPreset(BS5_PlayerAudioSettings.GetTechnicalPresetId());
	}

	static int GetSoundPresetIndex(string id)
	{
		EnsureInitialized();
		for (int i = 0; i < s_aSoundPresets.Count(); i++)
		{
			BS5_SoundPreset preset = s_aSoundPresets[i];
			if (preset && preset.m_sId == id)
				return i;
		}

		return 0;
	}

	static int GetTechnicalPresetIndex(string id)
	{
		EnsureInitialized();
		for (int i = 0; i < s_aTechnicalPresets.Count(); i++)
		{
			BS5_TechnicalPreset preset = s_aTechnicalPresets[i];
			if (preset && preset.m_sId == id)
				return i;
		}

		return 0;
	}

	static string GetSoundPresetDisplayName(string id)
	{
		if (id == CUSTOM_SOUND_PRESET_ID)
			return "Custom";

		BS5_SoundPreset preset = GetSoundPreset(id);
		if (preset)
			return preset.m_sDisplayName;

		return "Vanilla";
	}

	static string GetTechnicalPresetDisplayName(string id)
	{
		BS5_TechnicalPreset preset = GetTechnicalPreset(id);
		if (preset)
			return preset.m_sDisplayName;

		return "Default";
	}

	static void ApplySoundPreset(string id, bool saveImmediately = true)
	{
		BS5_SoundPreset preset = GetSoundPreset(id);
		if (!preset)
			return;

		BS5_PlayerAudioSettings.BeginSettingsBatch();
		BS5_PlayerAudioSettings.SetSoundPresetId(preset.m_sId, false);
		BS5_PlayerAudioSettings.SetEchoVolume(preset.m_fEchoVolume, false, false);
		BS5_PlayerAudioSettings.SetSlapbackVolume(preset.m_fSlapbackVolume, false, false);
		BS5_PlayerAudioSettings.SetSlapbackCloseVolume(preset.m_fSlapbackCloseVolume, false, false);
		BS5_PlayerAudioSettings.SetExplosionVolume(preset.m_fExplosionVolume, false, false);
		BS5_PlayerAudioSettings.EndSettingsBatch(saveImmediately);
	}

	static void ApplyTechnicalPreset(string id, bool saveImmediately = true)
	{
		BS5_TechnicalPreset preset = GetTechnicalPreset(id);
		if (!preset)
			return;

		BS5_PlayerAudioSettings.BeginSettingsBatch();
		BS5_PlayerAudioSettings.SetTechnicalPresetId(preset.m_sId, false);
		BS5_PlayerAudioSettings.EndSettingsBatch(saveImmediately);
	}

	protected static void EnsureInitialized()
	{
		if (s_bInitialized)
			return;

		s_bInitialized = true;
		s_aSoundPresets = new array<ref BS5_SoundPreset>();
		s_aTechnicalPresets = new array<ref BS5_TechnicalPreset>();

		LoadSoundPresetsFromConfig();
		LoadTechnicalPresetsFromConfig();

		AddFallbackSoundPresets();
		if (s_aTechnicalPresets.IsEmpty())
			AddFallbackTechnicalPresets();
	}

	protected static void LoadSoundPresetsFromConfig()
	{
		Resource resource = BaseContainerTools.LoadContainer(SOUND_PRESETS_CONFIG);
		if (!resource || !resource.IsValid())
			return;

		BS5_SoundPresetRegistryConfig config = BS5_SoundPresetRegistryConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
		if (!config || !config.m_aPresets)
			return;

		foreach (BS5_SoundPresetConfigEntry entry : config.m_aPresets)
		{
			if (!entry || entry.m_sId == string.Empty)
				continue;

			AddSoundPreset(entry.m_sId, entry.m_sDisplayName, entry.m_fEchoVolume, entry.m_fSlapbackVolume, entry.m_fSlapbackCloseVolume, entry.m_fExplosionVolume);
		}
	}

	protected static void LoadTechnicalPresetsFromConfig()
	{
		Resource resource = BaseContainerTools.LoadContainer(TECHNICAL_PRESETS_CONFIG);
		if (!resource || !resource.IsValid())
			return;

		BS5_TechnicalPresetRegistryConfig config = BS5_TechnicalPresetRegistryConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
		if (!config || !config.m_aPresets)
			return;

		foreach (BS5_TechnicalPresetConfigEntry entry : config.m_aPresets)
		{
			if (!entry || entry.m_sId == string.Empty)
				continue;

			AddTechnicalPresetFromEntry(entry);
		}
	}

	protected static void AddFallbackSoundPresets()
	{
		AddSoundPreset("vanilla", "Vanilla", 0.75, 0.60, 0.80, 0.75);
		AddSoundPreset("bettersounds_v4", "BetterSoundsV4", 1.00, 1.00, 1.00, 1.00);
		AddSoundPreset("bettersounds_v5", "BetterSoundsV5", 0.80, 0.60, 0.60, 0.80);
		AddSoundPreset("lunacy_audio", "LunacyAudio", 0.80, 0.85, 0.95, 0.90);
	}

	protected static void AddFallbackTechnicalPresets()
	{
		BS5_TechnicalPresetConfigEntry entry = new BS5_TechnicalPresetConfigEntry();
		FillDefaultTechnicalPreset(entry);
		AddTechnicalPresetFromEntry(entry);

		entry = new BS5_TechnicalPresetConfigEntry();
		FillLightTechnicalPreset(entry);
		AddTechnicalPresetFromEntry(entry);

		entry = new BS5_TechnicalPresetConfigEntry();
		FillDynamicTechnicalPreset(entry);
		AddTechnicalPresetFromEntry(entry);
	}

	protected static void AddSoundPreset(string id, string displayName, float echoVolume, float slapbackVolume, float slapbackCloseVolume, float explosionVolume)
	{
		if (FindSoundPreset(id))
			return;

		BS5_SoundPreset preset = new BS5_SoundPreset();
		preset.m_sId = id;
		preset.m_sDisplayName = displayName;
		preset.m_fEchoVolume = BS5_EchoMath.Clamp01(echoVolume);
		preset.m_fSlapbackVolume = BS5_EchoMath.Clamp01(slapbackVolume);
		preset.m_fSlapbackCloseVolume = BS5_EchoMath.Clamp01(slapbackCloseVolume);
		preset.m_fExplosionVolume = BS5_EchoMath.Clamp01(explosionVolume);
		s_aSoundPresets.Insert(preset);
	}

	protected static void AddTechnicalPresetFromEntry(BS5_TechnicalPresetConfigEntry entry)
	{
		if (!entry || entry.m_sId == string.Empty)
			return;
		if (FindTechnicalPreset(entry.m_sId))
			return;

		BS5_TechnicalPreset preset = new BS5_TechnicalPreset();
		preset.m_sId = entry.m_sId;
		preset.m_sDisplayName = entry.m_sDisplayName;
		preset.m_bUseSoundMapAnchorPlanner = entry.m_bUseSoundMapAnchorPlanner;
		preset.m_bAllowLegacyAnchorFallback = entry.m_bAllowLegacyAnchorFallback;
		preset.m_fScanRadius = entry.m_fScanRadius;
		preset.m_fNearSlapbackRadius = entry.m_fNearSlapbackRadius;
		preset.m_fSuppressedNearSlapbackRadius = entry.m_fSuppressedNearSlapbackRadius;
		preset.m_fExplosionNearSlapbackRadius = entry.m_fExplosionNearSlapbackRadius;
		preset.m_iNearReflectionImplementationMode = entry.m_iNearReflectionImplementationMode;
		preset.m_bNearReflectionTailOcclusionEnabled = entry.m_bNearReflectionTailOcclusionEnabled;
		preset.m_iNearReflectionProbeCount = entry.m_iNearReflectionProbeCount;
		preset.m_fNearReflectionMaxDistanceMeters = entry.m_fNearReflectionMaxDistanceMeters;
		preset.m_fNearReflectionTightDistanceMeters = entry.m_fNearReflectionTightDistanceMeters;
		preset.m_fNearReflectionSemiIndoorWallDistanceMeters = entry.m_fNearReflectionSemiIndoorWallDistanceMeters;
		preset.m_fNearReflectionCanopyMaxDistanceMeters = entry.m_fNearReflectionCanopyMaxDistanceMeters;
		preset.m_fNearReflectionSectorOcclusionStrength = entry.m_fNearReflectionSectorOcclusionStrength;
		preset.m_iNearReflectionMaxDirectionalCloseCandidates = entry.m_iNearReflectionMaxDirectionalCloseCandidates;
		preset.m_iNearReflectionMaxCoreCandidatesPerShot = entry.m_iNearReflectionMaxCoreCandidatesPerShot;
		preset.m_iNearReflectionMaxSemiIndoorCandidatesPerShot = entry.m_iNearReflectionMaxSemiIndoorCandidatesPerShot;
		preset.m_iExplosionNearReflectionProbeCount = entry.m_iExplosionNearReflectionProbeCount;
		preset.m_fExplosionNearReflectionMaxDistanceMeters = entry.m_fExplosionNearReflectionMaxDistanceMeters;
		preset.m_fExplosionNearReflectionTightDistanceMeters = entry.m_fExplosionNearReflectionTightDistanceMeters;
		preset.m_fExplosionNearReflectionSectorOcclusionStrength = entry.m_fExplosionNearReflectionSectorOcclusionStrength;
		preset.m_iExplosionNearReflectionMaxCloseCandidatesPerShot = entry.m_iExplosionNearReflectionMaxCloseCandidatesPerShot;
		preset.m_fExplosionNearReflectionCloseStrength = entry.m_fExplosionNearReflectionCloseStrength;
		preset.m_fExplosionScanRadiusMeters = entry.m_fExplosionScanRadiusMeters;
		preset.m_iMaxCandidateCount = entry.m_iMaxCandidateCount;
		preset.m_iMaxTraceCount = entry.m_iMaxTraceCount;
		preset.m_iMaxTailEmittersPerShot = entry.m_iMaxTailEmittersPerShot;
		preset.m_iMaxExplosionEmittersPerShot = entry.m_iMaxExplosionEmittersPerShot;
		preset.m_iMaxSuppressedTailEmittersPerShot = entry.m_iMaxSuppressedTailEmittersPerShot;
		preset.m_iMaxSlapbackEmittersPerShot = entry.m_iMaxSlapbackEmittersPerShot;
		preset.m_iMaxExplosionSlapbackEmittersPerShot = entry.m_iMaxExplosionSlapbackEmittersPerShot;
		preset.m_iMaxActiveTailEmitters = entry.m_iMaxActiveTailEmitters;
		preset.m_iMaxActiveSlapbackEmitters = entry.m_iMaxActiveSlapbackEmitters;
		preset.m_iLimiterGlobalMaxTailVoices = entry.m_iLimiterGlobalMaxTailVoices;
		preset.m_iLimiterGlobalMaxSlapbackVoices = entry.m_iLimiterGlobalMaxSlapbackVoices;
		preset.m_iLimiterMaxPendingTailVoices = entry.m_iLimiterMaxPendingTailVoices;
		preset.m_iLimiterMaxTailVoicesPerOwner = entry.m_iLimiterMaxTailVoicesPerOwner;
		preset.m_iLimiterBurstCadenceNormal = entry.m_iLimiterBurstCadenceNormal;
		preset.m_iLimiterBurstCadenceHighPressure = entry.m_iLimiterBurstCadenceHighPressure;
		preset.m_iLimiterBurstFreeTailShotsNormal = entry.m_iLimiterBurstFreeTailShotsNormal;
		preset.m_iLimiterBurstFreeTailShotsHighPressure = entry.m_iLimiterBurstFreeTailShotsHighPressure;
		preset.m_iLimiterBurstTailEmittersPerShot = entry.m_iLimiterBurstTailEmittersPerShot;
		preset.m_iLimiterHighPressureTailEmittersPerShot = entry.m_iLimiterHighPressureTailEmittersPerShot;
		preset.m_fLimiterHighPressureThreshold = entry.m_fLimiterHighPressureThreshold;
		preset.m_fLimiterCriticalPressureThreshold = entry.m_fLimiterCriticalPressureThreshold;
		preset.m_iLimiterMaxTailStartsPer100Ms = entry.m_iLimiterMaxTailStartsPer100Ms;
		preset.m_iLimiterMaxSlapbackStartsPer100Ms = entry.m_iLimiterMaxSlapbackStartsPer100Ms;
		preset.m_fLimiterStealFadeSeconds = entry.m_fLimiterStealFadeSeconds;
		preset.m_iLimiterEstimatedSourcesPerTail = entry.m_iLimiterEstimatedSourcesPerTail;
		preset.m_iLimiterEstimatedSourcesPerSlapback = entry.m_iLimiterEstimatedSourcesPerSlapback;
		preset.m_fTailEmitterLifetimeSeconds = entry.m_fTailEmitterLifetimeSeconds;
		preset.m_fTailEmitterHighPressureLifetimeSeconds = entry.m_fTailEmitterHighPressureLifetimeSeconds;
		preset.m_fTailEmitterMinManagedLifetimeSeconds = entry.m_fTailEmitterMinManagedLifetimeSeconds;
		preset.m_fSlapbackEmitterLifetimeSeconds = entry.m_fSlapbackEmitterLifetimeSeconds;
		preset.m_fExplosionSlapbackEmitterLifetimeSeconds = entry.m_fExplosionSlapbackEmitterLifetimeSeconds;
		preset.m_fExplosionEmitterLifetimeScale = entry.m_fExplosionEmitterLifetimeScale;
		preset.m_fSlapbackMinDistanceMeters = entry.m_fSlapbackMinDistanceMeters;
		preset.m_fEnvQueryRadiusMeters = entry.m_fEnvQueryRadiusMeters;
		preset.m_iForwardAnchorTraceCount = entry.m_iForwardAnchorTraceCount;
		preset.m_iLateralAnchorTraceCount = entry.m_iLateralAnchorTraceCount;
		preset.m_iTailSectorCount = entry.m_iTailSectorCount;
		preset.m_iTailHeightSampleCount = entry.m_iTailHeightSampleCount;
		preset.m_fSoundMapForwardConeDegrees = entry.m_fSoundMapForwardConeDegrees;
		preset.m_fSoundMapForwardMaxDistanceMeters = entry.m_fSoundMapForwardMaxDistanceMeters;
		preset.m_iSoundMapForwardRayCount = entry.m_iSoundMapForwardRayCount;
		preset.m_iSoundMapForwardSampleCount = entry.m_iSoundMapForwardSampleCount;
		preset.m_fSoundMapOmniRadiusMeters = entry.m_fSoundMapOmniRadiusMeters;
		preset.m_iSoundMapOmniDirectionCount = entry.m_iSoundMapOmniDirectionCount;
		preset.m_iSoundMapOmniAnchorCount = entry.m_iSoundMapOmniAnchorCount;
		preset.m_fSoundMapCityThreshold = entry.m_fSoundMapCityThreshold;
		preset.m_fSoundMapForestThreshold = entry.m_fSoundMapForestThreshold;
		preset.m_fSoundMapMeadowThreshold = entry.m_fSoundMapMeadowThreshold;
		preset.m_fSoundMapHillReliefThreshold = entry.m_fSoundMapHillReliefThreshold;
		preset.m_bSoundMapTerrainFrontSlopeValidation = entry.m_bSoundMapTerrainFrontSlopeValidation;
		preset.m_iSoundMapTerrainProfileSampleCount = entry.m_iSoundMapTerrainProfileSampleCount;
		preset.m_fSoundMapTerrainBacksideDropMeters = entry.m_fSoundMapTerrainBacksideDropMeters;
		preset.m_bSoundMapUrbanMicroScan = entry.m_bSoundMapUrbanMicroScan;
		preset.m_fSoundMapUrbanMicroScanRadiusMeters = entry.m_fSoundMapUrbanMicroScanRadiusMeters;
		preset.m_iSoundMapUrbanMicroMaxEntities = entry.m_iSoundMapUrbanMicroMaxEntities;
		preset.m_fSoundMapUrbanScoreBoost = entry.m_fSoundMapUrbanScoreBoost;
		preset.m_fSoundMapDistanceJitter = entry.m_fSoundMapDistanceJitter;
		preset.m_bSoundMapPathPlausibilityValidation = entry.m_bSoundMapPathPlausibilityValidation;
		preset.m_iSoundMapPathSampleCount = entry.m_iSoundMapPathSampleCount;
		preset.m_fSoundMapPathTerrainClearanceMeters = entry.m_fSoundMapPathTerrainClearanceMeters;
		preset.m_fSoundMapFarTailSoftLimitMeters = entry.m_fSoundMapFarTailSoftLimitMeters;
		preset.m_fSoundMapFarTailHardLimitMeters = entry.m_fSoundMapFarTailHardLimitMeters;
		preset.m_fSoundMapNearUrbanTailMaxDistanceMeters = entry.m_fSoundMapNearUrbanTailMaxDistanceMeters;
		preset.m_fSoundMapPathRaycastDistanceMeters = entry.m_fSoundMapPathRaycastDistanceMeters;
		s_aTechnicalPresets.Insert(preset);
	}

	protected static void FillDefaultTechnicalPreset(BS5_TechnicalPresetConfigEntry entry)
	{
		entry.m_sId = "default";
		entry.m_sDisplayName = "Default";
		entry.m_bUseSoundMapAnchorPlanner = true;
		entry.m_bAllowLegacyAnchorFallback = false;
		entry.m_fScanRadius = 875.0;
		entry.m_fNearSlapbackRadius = 34.0;
		entry.m_fSuppressedNearSlapbackRadius = 16.0;
		entry.m_fExplosionNearSlapbackRadius = 125.0;
		entry.m_iNearReflectionImplementationMode = 2;
		entry.m_bNearReflectionTailOcclusionEnabled = false;
		entry.m_iNearReflectionProbeCount = 12;
		entry.m_fNearReflectionMaxDistanceMeters = 42.0;
		entry.m_fNearReflectionTightDistanceMeters = 12.0;
		entry.m_fNearReflectionSemiIndoorWallDistanceMeters = 15.0;
		entry.m_fNearReflectionCanopyMaxDistanceMeters = 6.0;
		entry.m_fNearReflectionSectorOcclusionStrength = 0.50;
		entry.m_iNearReflectionMaxDirectionalCloseCandidates = 2;
		entry.m_iNearReflectionMaxCoreCandidatesPerShot = 1;
		entry.m_iNearReflectionMaxSemiIndoorCandidatesPerShot = 1;
		entry.m_iExplosionNearReflectionProbeCount = 12;
		entry.m_fExplosionNearReflectionMaxDistanceMeters = 95.0;
		entry.m_fExplosionNearReflectionTightDistanceMeters = 18.0;
		entry.m_fExplosionNearReflectionSectorOcclusionStrength = 0.65;
		entry.m_iExplosionNearReflectionMaxCloseCandidatesPerShot = 2;
		entry.m_fExplosionNearReflectionCloseStrength = 1.0;
		entry.m_fExplosionScanRadiusMeters = 900.0;
		entry.m_iMaxCandidateCount = 17;
		entry.m_iMaxTraceCount = 13;
		entry.m_iMaxTailEmittersPerShot = 3;
		entry.m_iMaxExplosionEmittersPerShot = 3;
		entry.m_iMaxSuppressedTailEmittersPerShot = 1;
		entry.m_iMaxSlapbackEmittersPerShot = 5;
		entry.m_iMaxExplosionSlapbackEmittersPerShot = 16;
		entry.m_iMaxActiveTailEmitters = 28;
		entry.m_iMaxActiveSlapbackEmitters = 16;
		entry.m_iLimiterGlobalMaxTailVoices = 28;
		entry.m_iLimiterGlobalMaxSlapbackVoices = 24;
		entry.m_iLimiterMaxPendingTailVoices = 8;
		entry.m_iLimiterMaxTailVoicesPerOwner = 6;
		entry.m_iLimiterBurstCadenceNormal = 4;
		entry.m_iLimiterBurstCadenceHighPressure = 7;
		entry.m_iLimiterBurstFreeTailShotsNormal = 3;
		entry.m_iLimiterBurstFreeTailShotsHighPressure = 2;
		entry.m_iLimiterBurstTailEmittersPerShot = 1;
		entry.m_iLimiterHighPressureTailEmittersPerShot = 1;
		entry.m_fLimiterHighPressureThreshold = 0.80;
		entry.m_fLimiterCriticalPressureThreshold = 0.94;
		entry.m_iLimiterMaxTailStartsPer100Ms = 12;
		entry.m_iLimiterMaxSlapbackStartsPer100Ms = 24;
		entry.m_fLimiterStealFadeSeconds = 0.08;
		entry.m_iLimiterEstimatedSourcesPerTail = 2;
		entry.m_iLimiterEstimatedSourcesPerSlapback = 1;
		entry.m_fTailEmitterLifetimeSeconds = 3.9;
		entry.m_fTailEmitterHighPressureLifetimeSeconds = 2.8;
		entry.m_fTailEmitterMinManagedLifetimeSeconds = 1.2;
		entry.m_fSlapbackEmitterLifetimeSeconds = 1.0;
		entry.m_fExplosionSlapbackEmitterLifetimeSeconds = 1.65;
		entry.m_fExplosionEmitterLifetimeScale = 2.805;
		entry.m_fSlapbackMinDistanceMeters = 1.0;
		entry.m_fEnvQueryRadiusMeters = 500.0;
		entry.m_iForwardAnchorTraceCount = 23;
		entry.m_iLateralAnchorTraceCount = 10;
		entry.m_iTailSectorCount = 12;
		entry.m_iTailHeightSampleCount = 4;
		entry.m_fSoundMapForwardConeDegrees = 120.0;
		entry.m_fSoundMapForwardMaxDistanceMeters = 875.0;
		entry.m_iSoundMapForwardRayCount = 9;
		entry.m_iSoundMapForwardSampleCount = 5;
		entry.m_fSoundMapOmniRadiusMeters = 563.0;
		entry.m_iSoundMapOmniDirectionCount = 10;
		entry.m_iSoundMapOmniAnchorCount = 3;
		entry.m_fSoundMapCityThreshold = 0.036;
		entry.m_fSoundMapForestThreshold = 0.128;
		entry.m_fSoundMapMeadowThreshold = 0.256;
		entry.m_fSoundMapHillReliefThreshold = 0.16;
		entry.m_bSoundMapTerrainFrontSlopeValidation = true;
		entry.m_iSoundMapTerrainProfileSampleCount = 12;
		entry.m_fSoundMapTerrainBacksideDropMeters = 2.8;
		entry.m_bSoundMapUrbanMicroScan = true;
		entry.m_fSoundMapUrbanMicroScanRadiusMeters = 225.0;
		entry.m_iSoundMapUrbanMicroMaxEntities = 35;
		entry.m_fSoundMapUrbanScoreBoost = 0.225;
		entry.m_fSoundMapDistanceJitter = 0.16;
		entry.m_bSoundMapPathPlausibilityValidation = true;
		entry.m_iSoundMapPathSampleCount = 8;
		entry.m_fSoundMapPathTerrainClearanceMeters = 2.2;
		entry.m_fSoundMapFarTailSoftLimitMeters = 525.0;
		entry.m_fSoundMapFarTailHardLimitMeters = 700.0;
		entry.m_fSoundMapNearUrbanTailMaxDistanceMeters = 225.0;
		entry.m_fSoundMapPathRaycastDistanceMeters = 275.0;
	}

	protected static void FillLightTechnicalPreset(BS5_TechnicalPresetConfigEntry entry)
	{
		FillDefaultTechnicalPreset(entry);
		entry.m_sId = "light";
		entry.m_sDisplayName = "Light";
		entry.m_fScanRadius = 525.0;
		entry.m_fNearSlapbackRadius = 13.0;
		entry.m_fSuppressedNearSlapbackRadius = 16.0;
		entry.m_fExplosionNearSlapbackRadius = 125.0;
		entry.m_iNearReflectionImplementationMode = 2;
		entry.m_bNearReflectionTailOcclusionEnabled = false;
		entry.m_iNearReflectionProbeCount = 9;
		entry.m_fNearReflectionMaxDistanceMeters = 25.0;
		entry.m_fNearReflectionTightDistanceMeters = 8.0;
		entry.m_fNearReflectionSemiIndoorWallDistanceMeters = 12.0;
		entry.m_fNearReflectionCanopyMaxDistanceMeters = 5.0;
		entry.m_fNearReflectionSectorOcclusionStrength = 0.35;
		entry.m_iNearReflectionMaxDirectionalCloseCandidates = 1;
		entry.m_iNearReflectionMaxCoreCandidatesPerShot = 1;
		entry.m_iNearReflectionMaxSemiIndoorCandidatesPerShot = 1;
		entry.m_iExplosionNearReflectionProbeCount = 10;
		entry.m_fExplosionNearReflectionMaxDistanceMeters = 70.0;
		entry.m_fExplosionNearReflectionTightDistanceMeters = 14.0;
		entry.m_fExplosionNearReflectionSectorOcclusionStrength = 0.50;
		entry.m_iExplosionNearReflectionMaxCloseCandidatesPerShot = 1;
		entry.m_fExplosionNearReflectionCloseStrength = 0.9;
		entry.m_fExplosionScanRadiusMeters = 900.0;
		entry.m_iMaxCandidateCount = 6;
		entry.m_iMaxTraceCount = 5;
		entry.m_iMaxTailEmittersPerShot = 1;
		entry.m_iMaxExplosionEmittersPerShot = 3;
		entry.m_iMaxSuppressedTailEmittersPerShot = 1;
		entry.m_iMaxSlapbackEmittersPerShot = 2;
		entry.m_iMaxExplosionSlapbackEmittersPerShot = 5;
		entry.m_iMaxActiveTailEmitters = 6;
		entry.m_iMaxActiveSlapbackEmitters = 5;
		entry.m_iLimiterGlobalMaxTailVoices = 6;
		entry.m_iLimiterGlobalMaxSlapbackVoices = 5;
		entry.m_iLimiterMaxPendingTailVoices = 3;
		entry.m_iLimiterMaxTailVoicesPerOwner = 2;
		entry.m_iLimiterBurstCadenceNormal = 7;
		entry.m_iLimiterBurstCadenceHighPressure = 10;
		entry.m_iLimiterBurstFreeTailShotsNormal = 2;
		entry.m_iLimiterBurstFreeTailShotsHighPressure = 1;
		entry.m_iLimiterBurstTailEmittersPerShot = 1;
		entry.m_iLimiterHighPressureTailEmittersPerShot = 1;
		entry.m_fLimiterHighPressureThreshold = 0.70;
		entry.m_fLimiterCriticalPressureThreshold = 0.88;
		entry.m_iLimiterMaxTailStartsPer100Ms = 2;
		entry.m_iLimiterMaxSlapbackStartsPer100Ms = 5;
		entry.m_fLimiterStealFadeSeconds = 0.08;
		entry.m_iLimiterEstimatedSourcesPerTail = 2;
		entry.m_iLimiterEstimatedSourcesPerSlapback = 1;
		entry.m_fTailEmitterLifetimeSeconds = 2.4;
		entry.m_fTailEmitterHighPressureLifetimeSeconds = 1.5;
		entry.m_fTailEmitterMinManagedLifetimeSeconds = 0.8;
		entry.m_fSlapbackEmitterLifetimeSeconds = 1.0;
		entry.m_fExplosionSlapbackEmitterLifetimeSeconds = 1.65;
		entry.m_fExplosionEmitterLifetimeScale = 2.805;
		entry.m_fSlapbackMinDistanceMeters = 1.0;
		entry.m_fEnvQueryRadiusMeters = 200.0;
		entry.m_iForwardAnchorTraceCount = 5;
		entry.m_iLateralAnchorTraceCount = 0;
		entry.m_iTailSectorCount = 5;
		entry.m_iTailHeightSampleCount = 2;
		entry.m_fSoundMapForwardConeDegrees = 95.0;
		entry.m_fSoundMapForwardMaxDistanceMeters = 525.0;
		entry.m_iSoundMapForwardRayCount = 7;
		entry.m_iSoundMapForwardSampleCount = 4;
		entry.m_fSoundMapOmniRadiusMeters = 275.0;
		entry.m_iSoundMapOmniDirectionCount = 5;
		entry.m_iSoundMapOmniAnchorCount = 0;
		entry.m_fSoundMapCityThreshold = 0.056;
		entry.m_fSoundMapForestThreshold = 0.176;
		entry.m_fSoundMapMeadowThreshold = 0.304;
		entry.m_fSoundMapHillReliefThreshold = 0.216;
		entry.m_iSoundMapTerrainProfileSampleCount = 5;
		entry.m_fSoundMapTerrainBacksideDropMeters = 4.0;
		entry.m_bSoundMapUrbanMicroScan = true;
		entry.m_fSoundMapUrbanMicroScanRadiusMeters = 113.0;
		entry.m_iSoundMapUrbanMicroMaxEntities = 10;
		entry.m_fSoundMapUrbanScoreBoost = 0.125;
		entry.m_fSoundMapDistanceJitter = 0.08;
		entry.m_iSoundMapPathSampleCount = 4;
		entry.m_fSoundMapPathTerrainClearanceMeters = 2.6;
		entry.m_fSoundMapFarTailSoftLimitMeters = 350.0;
		entry.m_fSoundMapFarTailHardLimitMeters = 450.0;
		entry.m_fSoundMapNearUrbanTailMaxDistanceMeters = 163.0;
		entry.m_fSoundMapPathRaycastDistanceMeters = 999.0;
	}

	protected static void FillDynamicTechnicalPreset(BS5_TechnicalPresetConfigEntry entry)
	{
		FillDefaultTechnicalPreset(entry);
		entry.m_sId = "dynamic";
		entry.m_sDisplayName = "Dynamic";
		entry.m_fScanRadius = 875.0;
		entry.m_fNearSlapbackRadius = 40.0;
		entry.m_fSuppressedNearSlapbackRadius = 16.0;
		entry.m_fExplosionNearSlapbackRadius = 125.0;
		entry.m_iNearReflectionImplementationMode = 2;
		entry.m_bNearReflectionTailOcclusionEnabled = false;
		entry.m_iNearReflectionProbeCount = 16;
		entry.m_fNearReflectionMaxDistanceMeters = 55.0;
		entry.m_fNearReflectionTightDistanceMeters = 15.0;
		entry.m_fNearReflectionSemiIndoorWallDistanceMeters = 15.0;
		entry.m_fNearReflectionCanopyMaxDistanceMeters = 7.0;
		entry.m_fNearReflectionSectorOcclusionStrength = 0.60;
		entry.m_iNearReflectionMaxDirectionalCloseCandidates = 2;
		entry.m_iNearReflectionMaxCoreCandidatesPerShot = 1;
		entry.m_iNearReflectionMaxSemiIndoorCandidatesPerShot = 1;
		entry.m_iExplosionNearReflectionProbeCount = 16;
		entry.m_fExplosionNearReflectionMaxDistanceMeters = 125.0;
		entry.m_fExplosionNearReflectionTightDistanceMeters = 22.0;
		entry.m_fExplosionNearReflectionSectorOcclusionStrength = 0.75;
		entry.m_iExplosionNearReflectionMaxCloseCandidatesPerShot = 2;
		entry.m_fExplosionNearReflectionCloseStrength = 1.1;
		entry.m_fExplosionScanRadiusMeters = 900.0;
		entry.m_iMaxCandidateCount = 19;
		entry.m_iMaxTraceCount = 15;
		entry.m_iMaxTailEmittersPerShot = 3;
		entry.m_iMaxExplosionEmittersPerShot = 3;
		entry.m_iMaxSuppressedTailEmittersPerShot = 1;
		entry.m_iMaxSlapbackEmittersPerShot = 6;
		entry.m_iMaxExplosionSlapbackEmittersPerShot = 18;
		entry.m_iMaxActiveTailEmitters = 28;
		entry.m_iMaxActiveSlapbackEmitters = 20;
		entry.m_iLimiterGlobalMaxTailVoices = 28;
		entry.m_iLimiterGlobalMaxSlapbackVoices = 24;
		entry.m_iLimiterMaxPendingTailVoices = 8;
		entry.m_iLimiterMaxTailVoicesPerOwner = 6;
		entry.m_iLimiterBurstCadenceNormal = 4;
		entry.m_iLimiterBurstCadenceHighPressure = 7;
		entry.m_iLimiterBurstFreeTailShotsNormal = 3;
		entry.m_iLimiterBurstFreeTailShotsHighPressure = 2;
		entry.m_iLimiterBurstTailEmittersPerShot = 1;
		entry.m_iLimiterHighPressureTailEmittersPerShot = 1;
		entry.m_fLimiterHighPressureThreshold = 0.80;
		entry.m_fLimiterCriticalPressureThreshold = 0.94;
		entry.m_iLimiterMaxTailStartsPer100Ms = 12;
		entry.m_iLimiterMaxSlapbackStartsPer100Ms = 24;
		entry.m_fLimiterStealFadeSeconds = 0.08;
		entry.m_iLimiterEstimatedSourcesPerTail = 2;
		entry.m_iLimiterEstimatedSourcesPerSlapback = 1;
		entry.m_fTailEmitterLifetimeSeconds = 3.9;
		entry.m_fTailEmitterHighPressureLifetimeSeconds = 2.8;
		entry.m_fTailEmitterMinManagedLifetimeSeconds = 1.2;
		entry.m_fSlapbackEmitterLifetimeSeconds = 1.0;
		entry.m_fExplosionSlapbackEmitterLifetimeSeconds = 1.65;
		entry.m_fExplosionEmitterLifetimeScale = 2.805;
		entry.m_fSlapbackMinDistanceMeters = 1.0;
		entry.m_fEnvQueryRadiusMeters = 500.0;
		entry.m_iForwardAnchorTraceCount = 23;
		entry.m_iLateralAnchorTraceCount = 10;
		entry.m_iTailSectorCount = 12;
		entry.m_iTailHeightSampleCount = 5;
		entry.m_fSoundMapForwardConeDegrees = 95.0;
		entry.m_fSoundMapForwardMaxDistanceMeters = 775.0;
		entry.m_iSoundMapForwardRayCount = 11;
		entry.m_iSoundMapForwardSampleCount = 5;
		entry.m_fSoundMapOmniRadiusMeters = 400.0;
		entry.m_iSoundMapOmniDirectionCount = 15;
		entry.m_iSoundMapOmniAnchorCount = 2;
		entry.m_fSoundMapCityThreshold = 0.030;
		entry.m_fSoundMapForestThreshold = 0.152;
		entry.m_fSoundMapMeadowThreshold = 0.288;
		entry.m_fSoundMapHillReliefThreshold = 0.192;
		entry.m_bSoundMapTerrainFrontSlopeValidation = true;
		entry.m_iSoundMapTerrainProfileSampleCount = 13;
		entry.m_fSoundMapTerrainBacksideDropMeters = 2.4;
		entry.m_bSoundMapUrbanMicroScan = true;
		entry.m_fSoundMapUrbanMicroScanRadiusMeters = 275.0;
		entry.m_iSoundMapUrbanMicroMaxEntities = 40;
		entry.m_fSoundMapUrbanScoreBoost = 0.30;
		entry.m_fSoundMapDistanceJitter = 0.20;
		entry.m_bSoundMapPathPlausibilityValidation = true;
		entry.m_iSoundMapPathSampleCount = 9;
		entry.m_fSoundMapPathTerrainClearanceMeters = 2.0;
		entry.m_fSoundMapFarTailSoftLimitMeters = 450.0;
		entry.m_fSoundMapFarTailHardLimitMeters = 625.0;
		entry.m_fSoundMapNearUrbanTailMaxDistanceMeters = 275.0;
		entry.m_fSoundMapPathRaycastDistanceMeters = 225.0;
	}

	protected static BS5_SoundPreset FindSoundPreset(string id)
	{
		foreach (BS5_SoundPreset preset : s_aSoundPresets)
		{
			if (preset && preset.m_sId == id)
				return preset;
		}

		return null;
	}

	protected static BS5_TechnicalPreset FindTechnicalPreset(string id)
	{
		foreach (BS5_TechnicalPreset preset : s_aTechnicalPresets)
		{
			if (preset && preset.m_sId == id)
				return preset;
		}

		return null;
	}

	protected static int WrapIndex(int index, int count)
	{
		if (count <= 0)
			return 0;

		while (index < 0)
			index += count;
		while (index >= count)
			index -= count;

		return index;
	}
}
