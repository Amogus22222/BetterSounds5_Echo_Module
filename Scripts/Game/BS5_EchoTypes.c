[ComponentEditorProps(category: "BS5 Audio", description: "BS5 echo/tails runtime types.")]
class BS5_EchoTypesClass : ScriptComponentClass
{
}

enum BS5_EchoEnvironmentType
{
	OPEN_FIELD,
	URBAN,
	FOREST,
	HILL,
	HARD_SURFACE,
	INDOOR_LIKE,
	TRENCH
}

enum BS5_TailProfileType
{
	OPEN_COASTAL,
	OPEN_MEADOW,
	FOREST_EDGE,
	HILL_TERRAIN,
	SETTLEMENT_EDGE,
	INTERIOR_OR_ROOM_EDGE
}

enum BS5_VegetationClass
{
	UNKNOWN,
	OPEN,
	GRASSLAND,
	FOREST
}

enum BS5_EchoCandidateSourceType
{
	UNKNOWN,
	FACADE_HIT,
	TERRAIN_HIT,
	TERRAIN_PRIMARY,
	PROFILE_FALLBACK,
	SOUNDMAP_CITY,
	SOUNDMAP_FOREST,
	SOUNDMAP_MEADOW,
	TERRAIN_RIDGE,
	FORWARD_FALLBACK,
	OMNI_CONTEXT,
	SLAPBACK_WALL,
	SLAPBACK_TRENCH
}

class BS5_EchoReflectorCandidate
{
	vector m_vPosition;
	vector m_vNormal;
	float m_fDistance;
	float m_fScore;
	float m_fDelaySeconds;
	float m_fDistanceNorm;
	float m_fPanBias;
	float m_fDirectionSupport;
	float m_fPhysicalScore;
	float m_fZonePriority;
	float m_fPathPlausibility;
	float m_fPathOcclusion;
	int m_iRank;
	BS5_EchoCandidateSourceType m_eSourceType;
	string m_sTerrainProfile;
	string m_sPathProfile;
	bool m_bTerrainSnapped;
	bool m_bValid;

	void BS5_EchoReflectorCandidate()
	{
		m_vPosition = vector.Zero;
		m_vNormal = "0 1 0";
		m_fDistance = 0.0;
		m_fScore = 0.0;
		m_fDelaySeconds = 0.0;
		m_fDistanceNorm = 0.0;
		m_fPanBias = 0.0;
		m_fDirectionSupport = 0.0;
		m_fPhysicalScore = 0.0;
		m_fZonePriority = 0.0;
		m_fPathPlausibility = 1.0;
		m_fPathOcclusion = 0.0;
		m_iRank = 0;
		m_eSourceType = BS5_EchoCandidateSourceType.UNKNOWN;
		m_sTerrainProfile = "none";
		m_sPathProfile = "unchecked";
		m_bTerrainSnapped = false;
		m_bValid = false;
	}
}

class BS5_EchoAnalysisResult
{
	BS5_EchoEnvironmentType m_eEnvironment;
	BS5_TailProfileType m_eTailProfile;
	bool m_bSuppressedShot;
	float m_fOpenScore;
	float m_fUrbanScore;
	float m_fForestScore;
	float m_fHillScore;
	float m_fWaterScore;
	float m_fHardSurfaceScore;
	float m_fIndoorScore;
	float m_fTrenchScore;
	float m_fRoomSize;
	float m_fConfidence;
	float m_fMasterDelaySeconds;
	float m_fSlapbackDelaySeconds;
	float m_fIntensity;
	float m_fNearConfinement;
	float m_fVerticalConfinement;
	float m_fFrontConfinement;
	float m_fBackConfinement;
	float m_fLeftConfinement;
	float m_fRightConfinement;
	int m_iTailSectorCount;
	int m_iTailHeightSamples;
	int m_iTailForwardSeeds;
	int m_iTailLateralSeeds;
	int m_iTailAnchorHits;
	int m_iTailForwardQueryEntities;
	int m_iTailForwardBuildingCandidates;
	int m_iTailForwardConfirmedFacades;
	int m_iSoundMapSamples;
	int m_iSoundMapCityHits;
	int m_iSoundMapForestHits;
	int m_iSoundMapMeadowHits;
	int m_iSoundMapHillHits;
	int m_iSoundMapFallbackAnchors;
	int m_iSoundMapOmniAnchors;
	int m_iSoundMapTerrainSnaps;
	int m_iSoundMapBacksideRejects;
	int m_iSoundMapUrbanMicroQueries;
	int m_iSoundMapUrbanMicroFacades;
	int m_iSoundMapDistanceSeed;
	int m_iSoundMapPathRejects;
	int m_iSoundMapPathRaycasts;
	int m_iSoundMapPathRayRejects;
	int m_iSoundMapFarRejects;
	int m_iSoundMapNearUrbanBoosts;
	bool m_bTailAnchorFallback;
	bool m_bTailSectorCacheHit;
	bool m_bTailForwardNegativeCacheHit;
	bool m_bTailSoundMapPlannerUsed;
	bool m_bTailLegacyPlannerFallback;
	int m_iSlapbackRayCount;
	int m_iSlapbackHitCount;
	bool m_bSlapbackAnchorFallback;
	float m_fSlapbackWallScore;
	float m_fSlapbackTrenchScore;
	string m_sTailForwardTopPrefabs;
	string m_sTailForwardConfirmTopPrefabs;
	string m_sSoundMapDistanceBands;
	string m_sAnchorPlannerMode;
	string m_sSlapbackMode;
	ref array<ref BS5_EchoReflectorCandidate> m_aCandidates;
	ref array<ref BS5_EchoReflectorCandidate> m_aSlapbackCandidates;
	string m_sDebugSummary;

	void BS5_EchoAnalysisResult()
	{
		m_aCandidates = new array<ref BS5_EchoReflectorCandidate>();
		m_aSlapbackCandidates = new array<ref BS5_EchoReflectorCandidate>();
		Reset();
	}

	void Reset()
	{
		m_eEnvironment = BS5_EchoEnvironmentType.OPEN_FIELD;
		m_eTailProfile = BS5_TailProfileType.OPEN_MEADOW;
		m_bSuppressedShot = false;
		m_fOpenScore = 0.0;
		m_fUrbanScore = 0.0;
		m_fForestScore = 0.0;
		m_fHillScore = 0.0;
		m_fWaterScore = 0.0;
		m_fHardSurfaceScore = 0.0;
		m_fIndoorScore = 0.0;
		m_fTrenchScore = 0.0;
		m_fRoomSize = 0.0;
		m_fConfidence = 0.0;
		m_fMasterDelaySeconds = 0.0;
		m_fSlapbackDelaySeconds = 0.0;
		m_fIntensity = 1.0;
		m_fNearConfinement = 0.0;
		m_fVerticalConfinement = 0.0;
		m_fFrontConfinement = 0.0;
		m_fBackConfinement = 0.0;
		m_fLeftConfinement = 0.0;
		m_fRightConfinement = 0.0;
		m_iTailSectorCount = 0;
		m_iTailHeightSamples = 0;
		m_iTailForwardSeeds = 0;
		m_iTailLateralSeeds = 0;
		m_iTailAnchorHits = 0;
		m_iTailForwardQueryEntities = 0;
		m_iTailForwardBuildingCandidates = 0;
		m_iTailForwardConfirmedFacades = 0;
		m_iSoundMapSamples = 0;
		m_iSoundMapCityHits = 0;
		m_iSoundMapForestHits = 0;
		m_iSoundMapMeadowHits = 0;
		m_iSoundMapHillHits = 0;
		m_iSoundMapFallbackAnchors = 0;
		m_iSoundMapOmniAnchors = 0;
		m_iSoundMapTerrainSnaps = 0;
		m_iSoundMapBacksideRejects = 0;
		m_iSoundMapUrbanMicroQueries = 0;
		m_iSoundMapUrbanMicroFacades = 0;
		m_iSoundMapDistanceSeed = 0;
		m_iSoundMapPathRejects = 0;
		m_iSoundMapPathRaycasts = 0;
		m_iSoundMapPathRayRejects = 0;
		m_iSoundMapFarRejects = 0;
		m_iSoundMapNearUrbanBoosts = 0;
		m_bTailAnchorFallback = false;
		m_bTailSectorCacheHit = false;
		m_bTailForwardNegativeCacheHit = false;
		m_bTailSoundMapPlannerUsed = false;
		m_bTailLegacyPlannerFallback = false;
		m_iSlapbackRayCount = 0;
		m_iSlapbackHitCount = 0;
		m_bSlapbackAnchorFallback = false;
		m_fSlapbackWallScore = 0.0;
		m_fSlapbackTrenchScore = 0.0;
		m_sTailForwardTopPrefabs = string.Empty;
		m_sTailForwardConfirmTopPrefabs = string.Empty;
		m_sSoundMapDistanceBands = string.Empty;
		m_sAnchorPlannerMode = "legacy";
		m_sSlapbackMode = "none";
		m_sDebugSummary = string.Empty;
		m_aCandidates.Clear();
		m_aSlapbackCandidates.Clear();
	}
}

class BS5_EnvironmentSnapshot
{
	float m_fOpenWeight;
	float m_fForestWeight;
	float m_fWaterWeight;
	float m_fInteriorWeight;
	float m_fTerrainHeightBias;
	float m_fTerrainSlopeBias;
	float m_fCanopyBias;
	BS5_VegetationClass m_eDominantVegetationClass;
	float m_fRoomSize;

	void BS5_EnvironmentSnapshot()
	{
		Reset();
	}

	void Reset()
	{
		m_fOpenWeight = 0.0;
		m_fForestWeight = 0.0;
		m_fWaterWeight = 0.0;
		m_fInteriorWeight = 0.0;
		m_fTerrainHeightBias = 0.0;
		m_fTerrainSlopeBias = 0.0;
		m_fCanopyBias = 0.0;
		m_eDominantVegetationClass = BS5_VegetationClass.UNKNOWN;
		m_fRoomSize = 0.0;
	}
}

class BS5_PendingEmissionContext
{
	IEntity m_pOwner;
	ResourceName m_sProject;
	ResourceName m_sEmitterPrefab;
	string m_sEventName;
	vector m_vEmitPosition;
	float m_fIntensity;
	float m_fDistanceGain;
	float m_fUserEchoVolume;
	float m_fUserSlapbackVolume;
	float m_fDelaySeconds;
	bool m_bSlapback;
	bool m_bExplosion;
	bool m_bSuppressed;
	bool m_bAllowDirectProjectPlayback;
	AudioHandle m_hPlayback;
	ref SCR_AudioSource m_AudioSource;
	bool m_bPlayedViaSoundManager;
	bool m_bPlaybackTerminated;
	int m_iEstimatedNativeSources;
	int m_iStartGateDeferCount;
	int m_iEmitterRetryCount;
	int m_iCandidateRank;
	float m_fDistanceNorm;
	float m_fPanBias;
	float m_fDirectionSupport;
	float m_fReverbSend;
	float m_fTailWidth;
	float m_fTailBrightness;
	float m_fSurfaceHardness;
	float m_fEmitterLifetimeSeconds;
	float m_fCandidateDistance;
	BS5_EchoCandidateSourceType m_eSourceType;
	bool m_bLimiterPendingReserved;
	bool m_bLimiterPlaybackReserved;
	bool m_bLimiterActiveRegistered;
	bool m_bDriverBudgetAcquired;
	bool m_bEmitterCleanupDone;
	int m_iLimiterTicket;
	float m_fLimiterPriority;
	ref BS5_EchoAnalysisResult m_Result;

	void BS5_PendingEmissionContext()
	{
		m_vEmitPosition = vector.Zero;
		m_fIntensity = 1.0;
		m_fDistanceGain = 1.0;
		m_fUserEchoVolume = 1.0;
		m_fUserSlapbackVolume = 1.0;
		m_fDelaySeconds = 0.0;
		m_bSlapback = false;
		m_bExplosion = false;
		m_bSuppressed = false;
		m_bAllowDirectProjectPlayback = true;
		m_hPlayback = -1;
		m_AudioSource = null;
		m_bPlayedViaSoundManager = false;
		m_bPlaybackTerminated = false;
		m_iEstimatedNativeSources = 1;
		m_iStartGateDeferCount = 0;
		m_iEmitterRetryCount = 0;
		m_iCandidateRank = 0;
		m_fDistanceNorm = 0.0;
		m_fPanBias = 0.0;
		m_fDirectionSupport = 0.0;
		m_fReverbSend = 0.0;
		m_fTailWidth = 0.0;
		m_fTailBrightness = 0.0;
		m_fSurfaceHardness = 0.0;
		m_fEmitterLifetimeSeconds = 4.0;
		m_fCandidateDistance = 0.0;
		m_eSourceType = BS5_EchoCandidateSourceType.UNKNOWN;
		m_bLimiterPendingReserved = false;
		m_bLimiterPlaybackReserved = false;
		m_bLimiterActiveRegistered = false;
		m_bDriverBudgetAcquired = false;
		m_bEmitterCleanupDone = false;
		m_iLimiterTicket = 0;
		m_fLimiterPriority = 0.0;
	}
}

class BS5_ActiveEchoVoice
{
	IEntity m_pOwner;
	IEntity m_pEmitter;
	ref BS5_PendingEmissionContext m_Context;
	bool m_bSlapback;
	int m_iTicket;
	float m_fPriority;

	void BS5_ActiveEchoVoice()
	{
		m_pOwner = null;
		m_pEmitter = null;
		m_Context = null;
		m_bSlapback = false;
		m_iTicket = 0;
		m_fPriority = 0.0;
	}
}

class BS5_EchoMath
{
	static const float SPEED_OF_SOUND_MPS = 343.0;

	static float Clamp01(float value)
	{
		if (value < 0.0)
			return 0.0;
		if (value > 1.0)
			return 1.0;
		return value;
	}

	static float MaxFloat(float a, float b)
	{
		if (a > b)
			return a;
		return b;
	}

	static float MinFloat(float a, float b)
	{
		if (a < b)
			return a;
		return b;
	}

	static float Clamp(float value, float minValue, float maxValue)
	{
		if (value < minValue)
			return minValue;
		if (value > maxValue)
			return maxValue;
		return value;
	}

	static string EnvironmentName(BS5_EchoEnvironmentType environmentType)
	{
		switch (environmentType)
		{
			case BS5_EchoEnvironmentType.URBAN:
				return "urban";
			case BS5_EchoEnvironmentType.FOREST:
				return "forest";
			case BS5_EchoEnvironmentType.HILL:
				return "hill";
			case BS5_EchoEnvironmentType.HARD_SURFACE:
				return "hard_surface";
			case BS5_EchoEnvironmentType.INDOOR_LIKE:
				return "indoor_like";
			case BS5_EchoEnvironmentType.TRENCH:
				return "trench";
		}

		return "open_field";
	}

	static string TailProfileName(BS5_TailProfileType profileType)
	{
		switch (profileType)
		{
			case BS5_TailProfileType.OPEN_COASTAL:
				return "open_coastal";
			case BS5_TailProfileType.FOREST_EDGE:
				return "forest_edge";
			case BS5_TailProfileType.HILL_TERRAIN:
				return "hill_terrain";
			case BS5_TailProfileType.SETTLEMENT_EDGE:
				return "settlement_edge";
			case BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE:
				return "interior_or_room_edge";
		}

		return "open_meadow";
	}

	static float EnvironmentId(BS5_EchoEnvironmentType environmentType)
	{
		return (float)environmentType;
	}

	static string CandidateSourceName(BS5_EchoCandidateSourceType sourceType)
	{
		switch (sourceType)
		{
			case BS5_EchoCandidateSourceType.FACADE_HIT:
				return "facade_hit";
			case BS5_EchoCandidateSourceType.TERRAIN_HIT:
				return "terrain_hit";
			case BS5_EchoCandidateSourceType.TERRAIN_PRIMARY:
				return "terrain_primary";
			case BS5_EchoCandidateSourceType.PROFILE_FALLBACK:
				return "profile_fallback";
			case BS5_EchoCandidateSourceType.SOUNDMAP_CITY:
				return "soundmap_city";
			case BS5_EchoCandidateSourceType.SOUNDMAP_FOREST:
				return "soundmap_forest";
			case BS5_EchoCandidateSourceType.SOUNDMAP_MEADOW:
				return "soundmap_meadow";
			case BS5_EchoCandidateSourceType.TERRAIN_RIDGE:
				return "terrain_ridge";
			case BS5_EchoCandidateSourceType.FORWARD_FALLBACK:
				return "forward_fallback";
			case BS5_EchoCandidateSourceType.OMNI_CONTEXT:
				return "omni_context";
			case BS5_EchoCandidateSourceType.SLAPBACK_WALL:
				return "slapback_wall";
			case BS5_EchoCandidateSourceType.SLAPBACK_TRENCH:
				return "slapback_trench";
		}

		return "unknown";
	}

	static BS5_EchoReflectorCandidate CloneCandidate(BS5_EchoReflectorCandidate sourceCandidate)
	{
		if (!sourceCandidate)
			return null;

		BS5_EchoReflectorCandidate cloneCandidate = new BS5_EchoReflectorCandidate();
		cloneCandidate.m_vPosition = sourceCandidate.m_vPosition;
		cloneCandidate.m_vNormal = sourceCandidate.m_vNormal;
		cloneCandidate.m_fDistance = sourceCandidate.m_fDistance;
		cloneCandidate.m_fScore = sourceCandidate.m_fScore;
		cloneCandidate.m_fDelaySeconds = sourceCandidate.m_fDelaySeconds;
		cloneCandidate.m_fDistanceNorm = sourceCandidate.m_fDistanceNorm;
		cloneCandidate.m_fPanBias = sourceCandidate.m_fPanBias;
		cloneCandidate.m_fDirectionSupport = sourceCandidate.m_fDirectionSupport;
		cloneCandidate.m_fPhysicalScore = sourceCandidate.m_fPhysicalScore;
		cloneCandidate.m_fZonePriority = sourceCandidate.m_fZonePriority;
		cloneCandidate.m_fPathPlausibility = sourceCandidate.m_fPathPlausibility;
		cloneCandidate.m_fPathOcclusion = sourceCandidate.m_fPathOcclusion;
		cloneCandidate.m_iRank = sourceCandidate.m_iRank;
		cloneCandidate.m_eSourceType = sourceCandidate.m_eSourceType;
		cloneCandidate.m_sTerrainProfile = sourceCandidate.m_sTerrainProfile;
		cloneCandidate.m_sPathProfile = sourceCandidate.m_sPathProfile;
		cloneCandidate.m_bTerrainSnapped = sourceCandidate.m_bTerrainSnapped;
		cloneCandidate.m_bValid = sourceCandidate.m_bValid;
		return cloneCandidate;
	}
}
