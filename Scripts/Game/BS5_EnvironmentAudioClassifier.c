class BS5_EnvironmentAudioClassifier
{
	protected static ref array<string> s_RiverSignalNames;
	protected static ref array<string> s_LakeSignalNames;

	protected static void EnsureSignalNameCaches()
	{
		if (!s_RiverSignalNames)
		{
			s_RiverSignalNames = new array<string>();
			s_RiverSignalNames.Insert("RiverL");
			s_RiverSignalNames.Insert("RiverR");
			s_RiverSignalNames.Insert("RiverRB");
			s_RiverSignalNames.Insert("RiverLB");
		}
		if (!s_LakeSignalNames)
		{
			s_LakeSignalNames = new array<string>();
			s_LakeSignalNames.Insert("LakeL");
			s_LakeSignalNames.Insert("LakeR");
			s_LakeSignalNames.Insert("LakeRB");
			s_LakeSignalNames.Insert("LakeLB");
		}
	}

	static BS5_EnvironmentSnapshot BuildSnapshot(BS5_EchoDriverComponent settings, IEntity owner, vector origin, vector flatForward, vector flatRight, BS5_EchoAnalysisResult result)
	{
		BS5_EnvironmentSnapshot snapshot = new BS5_EnvironmentSnapshot();
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return snapshot;

		float entityInterior = ReadEntitySignal(owner, "Interior");
		float entityRoomSize = ReadEntitySignal(owner, "RoomSize");
		float globalInterior = ReadGlobalSignal("GInterior");
		float roomSize = ReadGlobalSignal("GRoomSize");
		if (entityRoomSize > roomSize)
			roomSize = entityRoomSize;
		float grass = ReadGlobalSignal("Grass");
		EnsureSignalNameCaches();
		float water = BS5_EchoMath.MaxFloat(ReadSignalArray(s_RiverSignalNames), ReadSignalArray(s_LakeSignalNames));
		float aboveTerrain = ReadGlobalSignal("AboveTerrain");

		float terrainHeightBias = 0.0;
		float terrainSlopeBias = 0.0;
		SampleTerrainBias(world, origin, flatForward, flatRight, terrainHeightBias, terrainSlopeBias);

		float interiorSignal = BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(globalInterior, entityInterior));
		float localIndoorBias = BS5_EchoMath.Clamp01((result.m_fIndoorScore * 0.65) + (result.m_fNearConfinement * 0.20) + (result.m_fVerticalConfinement * 0.15));
		float normalizedRoomSize = 0.0;
		if (roomSize > 0.0)
			normalizedRoomSize = BS5_EchoMath.Clamp01(roomSize / 140.0);

		float roomInteriorBias = 0.0;
		if (roomSize > 0.0)
			roomInteriorBias = BS5_EchoMath.Clamp01(0.58 + ((1.0 - normalizedRoomSize) * 0.30));

		float interiorWeight = BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(interiorSignal, roomInteriorBias));
		interiorWeight = BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(interiorWeight, localIndoorBias * 0.85));
		if (interiorSignal > 0.55)
			interiorWeight = BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(interiorWeight, 0.68 + ((interiorSignal - 0.55) * 0.70)));

		float forestWeight = BS5_EchoMath.Clamp01((grass * 0.70) + (terrainSlopeBias * 0.15) + ((1.0 - result.m_fOpenScore) * 0.15));
		float openWeight = BS5_EchoMath.Clamp01((result.m_fOpenScore * 0.55) + ((1.0 - forestWeight) * 0.20) + ((1.0 - interiorWeight) * 0.15) + ((1.0 - water) * 0.10));
		float canopyBias = BS5_EchoMath.Clamp01((forestWeight * 0.75) + (grass * 0.25));

		if (aboveTerrain > 0.0)
			terrainHeightBias = BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(terrainHeightBias, aboveTerrain / 35.0));

		openWeight = BS5_EchoMath.Clamp01(openWeight * BS5_EchoMath.Clamp(1.0 - (interiorWeight * 0.92), 0.04, 1.0));

		snapshot.m_fOpenWeight = openWeight;
		snapshot.m_fForestWeight = forestWeight;
		snapshot.m_fWaterWeight = water;
		snapshot.m_fInteriorWeight = interiorWeight;
		snapshot.m_fTerrainHeightBias = terrainHeightBias;
		snapshot.m_fTerrainSlopeBias = terrainSlopeBias;
		snapshot.m_fCanopyBias = canopyBias;
		snapshot.m_fRoomSize = roomSize;
		snapshot.m_eDominantVegetationClass = ResolveDominantVegetation(forestWeight, grass);

		return snapshot;
	}

	protected static float ReadSignalArray(notnull array<string> signalNames)
	{
		float bestValue = 0.0;
		foreach (string signalName : signalNames)
			bestValue = BS5_EchoMath.MaxFloat(bestValue, ReadGlobalSignal(signalName));

		return bestValue;
	}

	protected static float ReadGlobalSignal(string signalName)
	{
		ChimeraGame chimeraGame = GetGame();
		if (!chimeraGame)
			return 0.0;

		GameSignalsManager signalsManager = chimeraGame.GetSignalsManager();
		if (!signalsManager)
			return 0.0;

		int signalIndex = signalsManager.FindSignal(signalName);
		if (signalIndex < 0)
			return 0.0;

		return signalsManager.GetSignalValue(signalIndex);
	}

	protected static float ReadEntitySignal(IEntity owner, string signalName)
	{
		if (!owner || signalName == string.Empty)
			return 0.0;

		float signalValue = ReadEntitySignalComponent(owner, signalName);
		IEntity rootEntity = owner.GetRootParent();
		if (rootEntity && rootEntity != owner)
		{
			float rootSignalValue = ReadEntitySignalComponent(rootEntity, signalName);
			if (rootSignalValue > signalValue)
				signalValue = rootSignalValue;
		}

		return signalValue;
	}

	protected static float ReadEntitySignalComponent(IEntity entity, string signalName)
	{
		if (!entity)
			return 0.0;

		SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(entity.FindComponent(SignalsManagerComponent));
		if (!signalsManager)
			return 0.0;

		int signalIndex = signalsManager.FindSignal(signalName);
		if (signalIndex < 0)
			return 0.0;

		return signalsManager.GetSignalValue(signalIndex);
	}

	protected static void SampleTerrainBias(BaseWorld world, vector origin, vector flatForward, vector flatRight, out float heightBias, out float slopeBias)
	{
		heightBias = 0.0;
		slopeBias = 0.0;

		array<vector> sampleOffsets = {
			"0 0 0",
			flatForward * 12.0,
			flatForward * -12.0,
			flatRight * 12.0,
			flatRight * -12.0,
			(flatForward + flatRight).Normalized() * 18.0,
			(flatForward - flatRight).Normalized() * 18.0
		};

		float minHeight = origin[1];
		float maxHeight = origin[1];
		float maxSlope = 0.0;
		vector baseNormal = ResolveTerrainNormal(world, origin);

		foreach (vector offset : sampleOffsets)
		{
			vector samplePos = origin + offset;
			float terrainHeight = ResolveTerrainHeight(world, samplePos, origin[1] + 120.0);
			minHeight = BS5_EchoMath.MinFloat(minHeight, terrainHeight);
			maxHeight = BS5_EchoMath.MaxFloat(maxHeight, terrainHeight);

			vector sampleNormal = ResolveTerrainNormal(world, samplePos);
			float slope = 1.0 - Math.AbsFloat(sampleNormal[1]);
			maxSlope = BS5_EchoMath.MaxFloat(maxSlope, slope);
		}

		heightBias = BS5_EchoMath.Clamp01((maxHeight - minHeight) / 14.0);
		slopeBias = BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(maxSlope, 1.0 - Math.AbsFloat(baseNormal[1])) / 0.55);
	}

	protected static BS5_VegetationClass ResolveDominantVegetation(float forestWeight, float grass)
	{
		if (forestWeight > 0.58)
			return BS5_VegetationClass.FOREST;
		if (grass > 0.22)
			return BS5_VegetationClass.GRASSLAND;

		return BS5_VegetationClass.OPEN;
	}

	static float ResolveTerrainHeight(BaseWorld world, vector position, float referenceHeight)
	{
		vector terrainProbe = position;
		terrainProbe[1] = referenceHeight;
		float heightAboveTerrain = SCR_TerrainHelper.GetHeightAboveTerrain(terrainProbe, world, false, null);
		return terrainProbe[1] - heightAboveTerrain;
	}

	static vector ResolveTerrainNormal(BaseWorld world, vector position)
	{
		float sampleOffset = 1.5;
		float centerHeight = ResolveTerrainHeight(world, position, position[1] + 80.0);

		vector sampleForward = position + "0 0 1" * sampleOffset;
		vector sampleRight = position + "1 0 0" * sampleOffset;
		float forwardHeight = ResolveTerrainHeight(world, sampleForward, position[1] + 80.0);
		float rightHeight = ResolveTerrainHeight(world, sampleRight, position[1] + 80.0);

		vector tangentForward = "0 0 1";
		tangentForward[1] = forwardHeight - centerHeight;
		vector tangentRight = "1 0 0";
		tangentRight[1] = rightHeight - centerHeight;
		vector normal = SCR_Math3D.Cross(tangentRight, tangentForward);
		if (normal.LengthSq() < 0.0001)
			return "0 1 0";

		normal.Normalize();
		if (normal[1] < 0.0)
			normal = normal * -1.0;
		return normal;
	}
}

class BS5_HybridTailPlanner
{
	protected static const int FORWARD_FACADE_QUERY_ENTITY_LIMIT = 24;
	protected static const int FORWARD_FACADE_CONFIRM_ENTITY_LIMIT = 8;
	protected static ref array<IEntity> s_ForwardFacadeQueryEntities;
	protected static ref array<float> s_ForwardFacadeQueryScores;
	protected static ref array<string> s_BuildingPrefabHints;
	protected static ref array<string> s_VegetationPrefabHints;
	protected static ref array<string> s_StructurePrefabHints;
	protected static ref array<string> s_FacadeRejectPrefabHints;
	protected static ref array<string> s_HousePrefabHints;
	protected static IEntity s_ForwardFacadeQueryExcludeRoot;
	protected static vector s_ForwardFacadeQueryOrigin;
	protected static vector s_ForwardFacadeQueryViewFlat;
	protected static int s_ForwardFacadeQueryEntityLimit;
	protected static bool s_ForwardFacadeQueryActive;

	protected static void EnsureFacadeCaches()
	{
		if (s_ForwardFacadeQueryViewFlat == vector.Zero)
			s_ForwardFacadeQueryViewFlat = "0 0 1";
		if (!s_ForwardFacadeQueryEntities)
			s_ForwardFacadeQueryEntities = new array<IEntity>();
		if (!s_ForwardFacadeQueryScores)
			s_ForwardFacadeQueryScores = new array<float>();
		if (!s_BuildingPrefabHints)
		{
			s_BuildingPrefabHints = new array<string>();
			s_BuildingPrefabHints.Insert("house");
			s_BuildingPrefabHints.Insert("building");
			s_BuildingPrefabHints.Insert("residential");
			s_BuildingPrefabHints.Insert("civilian");
			s_BuildingPrefabHints.Insert("village");
			s_BuildingPrefabHints.Insert("town");
			s_BuildingPrefabHints.Insert("apartment");
			s_BuildingPrefabHints.Insert("garage");
			s_BuildingPrefabHints.Insert("barn");
			s_BuildingPrefabHints.Insert("farm");
			s_BuildingPrefabHints.Insert("church");
			s_BuildingPrefabHints.Insert("school");
			s_BuildingPrefabHints.Insert("hospital");
			s_BuildingPrefabHints.Insert("shop");
			s_BuildingPrefabHints.Insert("store");
			s_BuildingPrefabHints.Insert("industrial");
			s_BuildingPrefabHints.Insert("warehouse");
			s_BuildingPrefabHints.Insert("factory");
			s_BuildingPrefabHints.Insert("office");
			s_BuildingPrefabHints.Insert("hangar");
			s_BuildingPrefabHints.Insert("depot");
			s_BuildingPrefabHints.Insert("ruin");
			s_BuildingPrefabHints.Insert("shed");
		}
		if (!s_VegetationPrefabHints)
		{
			s_VegetationPrefabHints = new array<string>();
			s_VegetationPrefabHints.Insert("tree");
			s_VegetationPrefabHints.Insert("bush");
			s_VegetationPrefabHints.Insert("shrub");
			s_VegetationPrefabHints.Insert("hedge");
			s_VegetationPrefabHints.Insert("forest");
			s_VegetationPrefabHints.Insert("vegetation");
			s_VegetationPrefabHints.Insert("foliage");
			s_VegetationPrefabHints.Insert("grass");
			s_VegetationPrefabHints.Insert("reed");
			s_VegetationPrefabHints.Insert("plant");
		}
		if (!s_StructurePrefabHints)
		{
			s_StructurePrefabHints = new array<string>();
			s_StructurePrefabHints.Insert("compound");
			s_StructurePrefabHints.Insert("checkpoint");
			s_StructurePrefabHints.Insert("camp");
			s_StructurePrefabHints.Insert("bunker");
			s_StructurePrefabHints.Insert("fort");
			s_StructurePrefabHints.Insert("wall");
			s_StructurePrefabHints.Insert("tower");
			s_StructurePrefabHints.Insert("gate");
			s_StructurePrefabHints.Insert("yard");
		}
		if (!s_FacadeRejectPrefabHints)
		{
			s_FacadeRejectPrefabHints = new array<string>();
			s_FacadeRejectPrefabHints.Insert("buildingparts");
			s_FacadeRejectPrefabHints.Insert("doorstep");
			s_FacadeRejectPrefabHints.Insert("entry");
			s_FacadeRejectPrefabHints.Insert("stair");
			s_FacadeRejectPrefabHints.Insert("stairs");
			s_FacadeRejectPrefabHints.Insert("step");
			s_FacadeRejectPrefabHints.Insert("fence");
			s_FacadeRejectPrefabHints.Insert("pole");
			s_FacadeRejectPrefabHints.Insert("railing");
			s_FacadeRejectPrefabHints.Insert("debris");
			s_FacadeRejectPrefabHints.Insert("wreck");
		}
		if (!s_HousePrefabHints)
		{
			s_HousePrefabHints = new array<string>();
			s_HousePrefabHints.Insert("house");
			s_HousePrefabHints.Insert("residential");
			s_HousePrefabHints.Insert("apartment");
			s_HousePrefabHints.Insert("villa");
			s_HousePrefabHints.Insert("cottage");
			s_HousePrefabHints.Insert("hut");
			s_HousePrefabHints.Insert("village");
		}
	}

	static void BuildCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, BS5_EnvironmentSnapshot snapshot, BS5_EchoAnalysisResult result, IEntity owner, vector origin, vector flatForward, vector flatRight, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot, bool explosionLike)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		vector viewOrigin;
		vector viewForward;
		ResolveViewBasis(world, owner, origin, flatForward, settings.GetAnchorPitchClampDegrees(), viewOrigin, viewForward);
		vector viewFlat = FlattenDirection(viewForward, flatForward);
		vector viewRight = BuildFlatRight(viewFlat);

		BS5_TailProfileType initialProfile = ResolveTailProfile(snapshot, result);
		result.m_eTailProfile = initialProfile;
		result.m_iTailSectorCount = settings.GetTailSectorCount();
		result.m_iTailHeightSamples = settings.GetTailHeightSampleCount();
		result.m_iTailForwardSeeds = 0;
		result.m_iTailLateralSeeds = 0;

		array<ref BS5_EchoReflectorCandidate> sectorCandidates = new array<ref BS5_EchoReflectorCandidate>();
		int anchorHits = 0;
		int facadeHits = 0;
		int forwardQueryEntities = 0;
		int forwardBuildingCandidates = 0;
		int forwardConfirmedFacades = 0;
		string forwardTopPrefabs = string.Empty;
		string forwardConfirmTopPrefabs = string.Empty;
		bool sectorCacheHit = settings.TryGetTailSectorCache(origin, viewFlat, initialProfile, explosionLike, sectorCandidates, anchorHits, facadeHits);
		bool forwardNegativeCacheHit = false;
		if (!sectorCacheHit)
		{
			CollectSectorFieldCandidates(sectorCandidates, settings, snapshot, result, owner, origin, viewOrigin, viewFlat, viewRight, traceExcludeArray, traceExcludeRoot, initialProfile);
			CountSectorCandidateSources(sectorCandidates, anchorHits, facadeHits);

			bool shouldRunForwardFacadePass = ShouldRunForwardFacadeMicroPass(initialProfile, facadeHits);
			if (shouldRunForwardFacadePass)
				forwardNegativeCacheHit = settings.TryGetForwardFacadeNegativeCache(origin, viewFlat, initialProfile, explosionLike);

			if (shouldRunForwardFacadePass && !forwardNegativeCacheHit)
			{
				CollectForwardFacadeEntityCandidates(sectorCandidates, settings, owner, origin, viewFlat, viewRight, traceExcludeRoot, false, forwardQueryEntities, forwardBuildingCandidates, forwardConfirmedFacades, forwardTopPrefabs, forwardConfirmTopPrefabs);
				CountSectorCandidateSources(sectorCandidates, anchorHits, facadeHits);
				if (ShouldRunForwardFacadeMicroPass(initialProfile, facadeHits))
				{
					CollectForwardFacadeMicroCandidates(sectorCandidates, settings, owner, origin, viewOrigin, viewFlat, viewRight, traceExcludeArray, traceExcludeRoot);
					CountSectorCandidateSources(sectorCandidates, anchorHits, facadeHits);
				}
			}
			if (facadeHits > 0)
				settings.ClearForwardFacadeNegativeCache();
			else if (shouldRunForwardFacadePass && !forwardNegativeCacheHit)
				settings.StoreForwardFacadeNegativeCache(origin, viewFlat, initialProfile, explosionLike);

			settings.StoreTailSectorCache(origin, viewFlat, initialProfile, explosionLike, sectorCandidates, anchorHits, facadeHits);
		}
		result.m_bTailSectorCacheHit = sectorCacheHit;
		result.m_bTailForwardNegativeCacheHit = forwardNegativeCacheHit;
		result.m_iTailForwardQueryEntities = forwardQueryEntities;
		result.m_iTailForwardBuildingCandidates = forwardBuildingCandidates;
		result.m_iTailForwardConfirmedFacades = forwardConfirmedFacades;
		result.m_sTailForwardTopPrefabs = forwardTopPrefabs;
		result.m_sTailForwardConfirmTopPrefabs = forwardConfirmTopPrefabs;

		BS5_TailProfileType profile = initialProfile;
		if (ShouldPromoteSettlementProfile(settings, initialProfile, snapshot, sectorCandidates, facadeHits))
			profile = BS5_TailProfileType.SETTLEMENT_EDGE;

		int targetCount = ResolveTargetCount(settings, profile, result);
		float mergeDistanceSq = settings.GetAnchorMergeDistanceMeters();
		mergeDistanceSq *= mergeDistanceSq;

		SynthesizeCandidatesFromSectorField(candidates, sectorCandidates, settings, profile, result, targetCount);

		bool usedFallback = false;
		int minimumUsefulCount = ResolveMinimumUsefulCount(profile, targetCount);
		if (candidates.Count() < minimumUsefulCount)
		{
			usedFallback = true;
			AddProfileFallbackCandidates(candidates, settings, snapshot, result, origin, viewFlat, viewRight, profile, targetCount, mergeDistanceSq);
		}

		RebalanceCandidateMix(candidates, settings, profile, result);

		result.m_eTailProfile = profile;
		result.m_iTailAnchorHits = anchorHits;
		result.m_bTailAnchorFallback = usedFallback;

		switch (profile)
		{
			case BS5_TailProfileType.SETTLEMENT_EDGE:
				result.m_eEnvironment = BS5_EchoEnvironmentType.URBAN;
				result.m_fUrbanScore = BS5_EchoMath.Clamp01(0.55 + facadeHits * 0.12);
				break;

			case BS5_TailProfileType.FOREST_EDGE:
				result.m_eEnvironment = BS5_EchoEnvironmentType.FOREST;
				break;

			case BS5_TailProfileType.HILL_TERRAIN:
				result.m_eEnvironment = BS5_EchoEnvironmentType.HILL;
				break;

			case BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE:
				result.m_eEnvironment = BS5_EchoEnvironmentType.INDOOR_LIKE;
				break;
		}
	}

	protected static void CountSectorCandidateSources(array<ref BS5_EchoReflectorCandidate> sectorCandidates, out int anchorHits, out int facadeHits)
	{
		anchorHits = 0;
		facadeHits = 0;
		foreach (BS5_EchoReflectorCandidate candidate : sectorCandidates)
		{
			if (!candidate || !candidate.m_bValid)
				continue;

			if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT)
			{
				facadeHits++;
				anchorHits++;
				continue;
			}

			if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_HIT)
				anchorHits++;
		}
	}

	protected static bool ShouldPromoteSettlementProfile(BS5_EchoDriverComponent settings, BS5_TailProfileType initialProfile, BS5_EnvironmentSnapshot snapshot, array<ref BS5_EchoReflectorCandidate> sectorCandidates, int facadeHits)
	{
		if (initialProfile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
			return false;

		if (facadeHits >= 2)
			return true;

		if (facadeHits < 1)
			return false;

		float settlementMaxDistance = settings.GetSettlementMaxDistanceMeters();
		foreach (BS5_EchoReflectorCandidate candidate : sectorCandidates)
		{
			if (!candidate || !candidate.m_bValid || candidate.m_eSourceType != BS5_EchoCandidateSourceType.FACADE_HIT)
				continue;

			if (candidate.m_fDirectionSupport < 0.72)
				continue;

			if (candidate.m_fDistance > settlementMaxDistance)
				continue;

			if (snapshot.m_fOpenWeight < 0.985)
				return true;
		}

		return false;
	}

	protected static bool ShouldRunForwardFacadeMicroPass(BS5_TailProfileType profile, int facadeHits)
	{
		if (facadeHits > 0)
			return false;

		switch (profile)
		{
			case BS5_TailProfileType.OPEN_MEADOW:
			case BS5_TailProfileType.OPEN_COASTAL:
			case BS5_TailProfileType.HILL_TERRAIN:
				return true;
		}

		return false;
	}

	protected static void CollectSectorFieldCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, BS5_EnvironmentSnapshot snapshot, BS5_EchoAnalysisResult result, IEntity owner, vector origin, vector viewOrigin, vector viewFlat, vector viewRight, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot, BS5_TailProfileType profile)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		int sectorCount = settings.GetTailSectorCount();
		int heightSampleCount = settings.GetTailHeightSampleCount();
		int maxCandidates = settings.GetMaxCandidateCount();
		float mergeDistanceSq = settings.GetTailClusterDistanceMeters();
		mergeDistanceSq *= mergeDistanceSq;
		float normalMergeDot = settings.GetTailNormalMergeDot();
		float coneHalf = settings.GetTailForwardConeDegrees() * 0.5;
		float maxAzimuth = ResolveSectorMaxAzimuthDegrees(settings, profile);
		float maxDistance = ResolveProfileMaxDistance(settings, profile, result);

		for (int sectorIndex = 0; sectorIndex < sectorCount; sectorIndex++)
		{
			float angleDegrees = ResolveSectorAngleDegrees(sectorIndex, sectorCount, maxAzimuth);
			vector sectorDirection = GetFlatDirection(viewFlat, viewRight, angleDegrees);
			float sectorWeight = ResolveSectorWeight(settings, angleDegrees, maxAzimuth);
			if (sectorWeight <= 0.0)
				continue;

			bool forwardSector = Math.AbsFloat(angleDegrees) <= coneHalf;
			BS5_EchoReflectorCandidate bestCandidate = null;

			float primaryProbeDistance = ResolveSectorProbeDistance(settings, profile, result, sectorWeight, forwardSector, false);
			BS5_EchoReflectorCandidate primaryCandidate = BuildBestSectorProbeCandidate(world, settings, owner, origin, viewOrigin, sectorDirection, viewRight, angleDegrees, sectorWeight, maxDistance, primaryProbeDistance, heightSampleCount, traceExcludeArray, traceExcludeRoot, profile, forwardSector);
			if (primaryCandidate)
				bestCandidate = primaryCandidate;

			if (ShouldUseLongSectorProbe(profile, sectorWeight, forwardSector))
			{
				float longProbeDistance = ResolveSectorProbeDistance(settings, profile, result, sectorWeight, forwardSector, true);
				BS5_EchoReflectorCandidate longCandidate = BuildBestSectorProbeCandidate(world, settings, owner, origin, viewOrigin, sectorDirection, viewRight, angleDegrees, sectorWeight, maxDistance, longProbeDistance, heightSampleCount, traceExcludeArray, traceExcludeRoot, profile, forwardSector);
				if (longCandidate && (!bestCandidate || longCandidate.m_fScore > bestCandidate.m_fScore))
					bestCandidate = longCandidate;
			}

			if (!bestCandidate)
				bestCandidate = CreateTerrainPrimaryCandidate(world, settings, snapshot, origin, sectorDirection, viewRight, angleDegrees, sectorWeight, maxDistance, primaryProbeDistance, profile, forwardSector);

			MergeSectorCandidate(candidates, bestCandidate, maxCandidates, mergeDistanceSq, normalMergeDot);
		}
	}

	protected static void CollectForwardFacadeMicroCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, IEntity owner, vector origin, vector viewOrigin, vector viewFlat, vector viewRight, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		int maxCandidates = settings.GetMaxCandidateCount();
		float mergeDistanceSq = settings.GetTailClusterDistanceMeters();
		mergeDistanceSq *= mergeDistanceSq;
		float settlementMaxDistance = settings.GetSettlementMaxDistanceMeters();

		array<float> probeAngles = {-14.0, 0.0, 14.0};
		array<float> probeHeights = {2.6, 5.8};
		array<float> probeDistances = {65.0, 95.0, BS5_EchoMath.MinFloat(settlementMaxDistance, 130.0)};

		foreach (float angleDegrees : probeAngles)
		{
			vector direction = GetFlatDirection(viewFlat, viewRight, angleDegrees);
			float azimuthSupport = 1.0 - (Math.AbsFloat(angleDegrees) / 14.0);
			foreach (float probeDistance : probeDistances)
			{
				vector samplePoint = origin + direction * probeDistance;
				float terrainHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, samplePoint, origin[1] + 24.0);
				vector traceEnd = origin + (direction * 10.0);
				traceEnd[1] = origin[1] + 1.8;
				foreach (float probeHeight : probeHeights)
				{
					TraceSphere trace = new TraceSphere();
					trace.Radius = 0.80;
					trace.Start = samplePoint + (direction * 12.0);
					trace.Start[1] = terrainHeight + probeHeight;
					trace.End = traceEnd;
					if (traceExcludeArray)
						trace.ExcludeArray = traceExcludeArray;
					else
						trace.Exclude = owner;

					vector hitPosition = vector.Zero;
					vector hitNormal = "0 1 0";
					if (!ResolveGeometryHit(world, trace, traceExcludeRoot, hitPosition, hitNormal))
						continue;

					float verticality = 1.0 - Math.AbsFloat(hitNormal[1]);
					if (verticality < 0.24)
						continue;

					BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
					candidate.m_bValid = true;
					candidate.m_vPosition = hitPosition + (hitNormal * 0.12);
					candidate.m_vNormal = hitNormal;
					candidate.m_fDistance = vector.Distance(origin, candidate.m_vPosition);
					candidate.m_fDelaySeconds = ResolveTailDelaySeconds(settings, candidate.m_fDistance, BS5_EchoCandidateSourceType.FACADE_HIT);
					candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(candidate.m_fDistance / BS5_EchoMath.MaxFloat(1.0, settlementMaxDistance));
					candidate.m_fPanBias = vector.Dot(direction, viewRight);
					candidate.m_fDirectionSupport = BS5_EchoMath.Clamp01(0.88 + (azimuthSupport * 0.12));
					candidate.m_iRank = 80 + (int)Math.AbsFloat(angleDegrees);
					candidate.m_eSourceType = BS5_EchoCandidateSourceType.FACADE_HIT;
					candidate.m_fScore = BS5_EchoMath.Clamp01((0.74 + (verticality * 0.20)) * (0.78 + (azimuthSupport * 0.22)) * (0.76 + ((1.0 - candidate.m_fDistanceNorm) * 0.24)));

					MergeCandidate(candidates, candidate, maxCandidates, mergeDistanceSq);
				}
			}
		}
	}

	protected static void CollectForwardFacadeEntityCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, IEntity owner, vector origin, vector viewFlat, vector viewRight, IEntity traceExcludeRoot, bool urbanMicroPass, out int queryEntities, out int buildingCandidates, out int confirmedFacades, out string topPrefabs, out string confirmTopPrefabs)
	{
		queryEntities = 0;
		buildingCandidates = 0;
		confirmedFacades = 0;
		topPrefabs = string.Empty;
		confirmTopPrefabs = string.Empty;

		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;
		EnsureFacadeCaches();
		if (s_ForwardFacadeQueryActive)
		{
			BS5_DebugLog.Line(settings, "forward facade query reentry drop");
			return;
		}

		s_ForwardFacadeQueryEntities.Clear();
		s_ForwardFacadeQueryScores.Clear();
		s_ForwardFacadeQueryExcludeRoot = traceExcludeRoot;
		s_ForwardFacadeQueryOrigin = origin;
		s_ForwardFacadeQueryViewFlat = viewFlat;
		s_ForwardFacadeQueryEntityLimit = FORWARD_FACADE_QUERY_ENTITY_LIMIT;
		if (urbanMicroPass)
			s_ForwardFacadeQueryEntityLimit = settings.GetSoundMapUrbanMicroMaxEntities();
		s_ForwardFacadeQueryActive = true;
		if (s_ForwardFacadeQueryEntityLimit <= 0)
		{
			s_ForwardFacadeQueryActive = false;
			return;
		}

		float settlementMaxDistance = settings.GetSettlementMaxDistanceMeters();
		if (urbanMicroPass)
			settlementMaxDistance = settings.GetSoundMapUrbanMicroScanRadiusMeters();
		float mergeDistanceSq = settings.GetTailClusterDistanceMeters();
		mergeDistanceSq *= mergeDistanceSq;
		int maxCandidates = settings.GetMaxCandidateCount();

		vector queryStart = origin + "0 1.8 0";
		vector queryEnd = queryStart + (viewFlat * BS5_EchoMath.MinFloat(settlementMaxDistance + 28.0, 280.0));
		float queryEndTerrain = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, queryEnd, queryStart[1] + 96.0);
		queryEnd[1] = queryEndTerrain + 5.5;
		vector bevelMins = "-14 -5 -14";
		vector bevelMaxs = "14 10 14";
		world.QueryEntitiesByBeveledLine(queryStart, queryEnd, bevelMins, bevelMaxs, CollectForwardFacadeEntityCallback);

		array<float> queryDistances = {
			Lerp(40.0, settlementMaxDistance, 0.18),
			Lerp(58.0, settlementMaxDistance, 0.40),
			Lerp(76.0, settlementMaxDistance, 0.68),
			Lerp(92.0, settlementMaxDistance, 0.92)
		};
		array<float> queryRadii = {18.0, 28.0, 40.0, 52.0};
		for (int queryIndex = 0; queryIndex < queryDistances.Count(); queryIndex++)
		{
			if (s_ForwardFacadeQueryEntities.Count() >= s_ForwardFacadeQueryEntityLimit)
				break;

			vector queryCenter = queryStart + (viewFlat * queryDistances[queryIndex]);
			float queryTerrain = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, queryCenter, queryStart[1] + 96.0);
			queryCenter[1] = queryTerrain + BS5_EchoMath.Clamp(queryRadii[queryIndex] * 0.18, 3.0, 7.0);
			world.QueryEntitiesBySphere(queryCenter, queryRadii[queryIndex], CollectForwardFacadeEntityCallback);
		}

		queryEntities = s_ForwardFacadeQueryEntities.Count();
		topPrefabs = BuildForwardFacadePrefabDebugString(s_ForwardFacadeQueryEntities, 4);
		array<IEntity> confirmEntities = new array<IEntity>();
		array<float> confirmScores = new array<float>();

		foreach (IEntity entity : s_ForwardFacadeQueryEntities)
		{
			if (!entity)
				continue;

			float prefabHintScore = 0.0;
			bool strongBuildingHint = false;
			if (!ResolveForwardFacadePrefabHints(entity, prefabHintScore, strongBuildingHint))
				continue;

			vector mins;
			vector maxs;
			entity.GetWorldBounds(mins, maxs);
			vector extents = maxs - mins;
			float width = Math.AbsFloat(extents[0]);
			float height = Math.AbsFloat(extents[1]);
			float depth = Math.AbsFloat(extents[2]);
			float footprintMax = BS5_EchoMath.MaxFloat(width, depth);
			float footprintMin = BS5_EchoMath.MinFloat(width, depth);
			float footprintArea = width * depth;

			float minHeight = 2.8;
			float minFootprintMax = 2.8;
			float minFootprintArea = 8.0;
			if (strongBuildingHint)
			{
				minHeight = 2.2;
				minFootprintMax = 2.2;
				minFootprintArea = 6.0;
			}
			if (urbanMicroPass && strongBuildingHint)
			{
				minHeight = 2.0;
				minFootprintMax = 1.8;
				minFootprintArea = 5.0;
			}
			if (height < minHeight || footprintMax < minFootprintMax || footprintArea < minFootprintArea)
				continue;
			if (height > 60.0 || footprintMax > 120.0)
				continue;
			if (footprintMin < 1.2 && height > footprintMax * 2.5)
				continue;
			if (footprintArea < 18.0 && height > 10.0)
				continue;

			vector center = (mins + maxs) * 0.5;
			vector toCenter = center - origin;
			toCenter[1] = 0.0;
			if (toCenter.LengthSq() < 1.0)
				continue;

			float flatDistance = toCenter.Length();
			if (flatDistance < 16.0 || flatDistance > settlementMaxDistance + 40.0)
				continue;

			toCenter.Normalize();
			float forwardSupport = vector.Dot(toCenter, viewFlat);
			float minForwardSupport = 0.52;
			if (strongBuildingHint)
				minForwardSupport = 0.38;
			if (urbanMicroPass && strongBuildingHint)
				minForwardSupport = 0.30;
			if (forwardSupport < minForwardSupport)
				continue;
			buildingCandidates++;

			float distanceNorm = BS5_EchoMath.Clamp01(flatDistance / BS5_EchoMath.MaxFloat(1.0, settlementMaxDistance + 40.0));
			float preScore = BS5_EchoMath.Clamp01(
				(prefabHintScore * 0.32) +
				(forwardSupport * 0.28) +
				((1.0 - distanceNorm) * 0.18) +
				BS5_EchoMath.Clamp01(height / 12.0) * 0.10 +
				BS5_EchoMath.Clamp01(footprintArea / 90.0) * 0.12
			);
			InsertRankedForwardFacadeEntity(confirmEntities, confirmScores, entity, preScore, FORWARD_FACADE_CONFIRM_ENTITY_LIMIT);
		}

		foreach (IEntity entityToConfirm : confirmEntities)
		{
			if (!entityToConfirm)
				continue;

			float prefabHintScoreConfirmed = 0.0;
			bool strongBuildingHintConfirmed = false;
			if (!ResolveForwardFacadePrefabHints(entityToConfirm, prefabHintScoreConfirmed, strongBuildingHintConfirmed))
				continue;

			vector mins;
			vector maxs;
			entityToConfirm.GetWorldBounds(mins, maxs);
			vector extents = maxs - mins;
			float width = Math.AbsFloat(extents[0]);
			float height = Math.AbsFloat(extents[1]);
			float depth = Math.AbsFloat(extents[2]);
			float footprintMax = BS5_EchoMath.MaxFloat(width, depth);
			float footprintMin = BS5_EchoMath.MinFloat(width, depth);
			float footprintArea = width * depth;
			vector center = (mins + maxs) * 0.5;
			vector toCenter = center - origin;
			toCenter[1] = 0.0;
			if (toCenter.LengthSq() < 1.0)
				continue;

			float flatDistance = toCenter.Length();
			if (flatDistance < 16.0 || flatDistance > settlementMaxDistance + 40.0)
				continue;

			toCenter.Normalize();
			float forwardSupport = vector.Dot(toCenter, viewFlat);

			vector faceNormal = origin - center;
			faceNormal[1] = 0.0;
			if (faceNormal.LengthSq() < 0.0001)
				continue;
			faceNormal.Normalize();

			vector faceTangent = "0 0 0";
			faceTangent[0] = -faceNormal[2];
			faceTangent[2] = faceNormal[0];
			if (faceTangent.LengthSq() < 0.0001)
				continue;
			faceTangent.Normalize();

			float halfDepth = (Math.AbsFloat(faceNormal[0]) * (width * 0.5)) + (Math.AbsFloat(faceNormal[2]) * (depth * 0.5));
			float confirmHeight = mins[1] + BS5_EchoMath.Clamp(height * 0.42, 2.0, 4.8);
			float sideOffset = BS5_EchoMath.Clamp(footprintMin * 0.18, 0.45, 2.20);

			int confirmHits = 0;
			vector accumulatedPosition = vector.Zero;
			vector accumulatedNormal = vector.Zero;
			vector firstHitPosition = vector.Zero;
			vector firstHitNormal = vector.Zero;
			bool houseLikePrefab = IsHouseLikePrefab(entityToConfirm);
			bool detachedHouseCandidate = strongBuildingHintConfirmed && footprintArea <= 42.0 && footprintMax <= 10.0 && height >= 2.2 && height <= 12.5;
			if (urbanMicroPass && strongBuildingHintConfirmed && footprintArea >= 6.0 && footprintArea <= 42.0 && footprintMax <= 10.0 && height >= 2.0 && height <= 12.5)
				detachedHouseCandidate = true;
			if (detachedHouseCandidate)
			{
				vector detachedHitPosition = vector.Zero;
				vector detachedHitNormal = vector.Zero;
				if (TryResolveDetachedHouseFacade(world, owner, origin, traceExcludeRoot, entityToConfirm, center, faceNormal, faceTangent, halfDepth, footprintMin, mins[1], height, detachedHitPosition, detachedHitNormal))
				{
					accumulatedPosition = detachedHitPosition;
					accumulatedNormal = detachedHitNormal;
					firstHitPosition = detachedHitPosition;
					firstHitNormal = detachedHitNormal;
					confirmHits = 1;
				}
			}

			for (int confirmIndex = 0; confirmIndex < 3; confirmIndex++)
			{
				if (confirmHits > 0 && detachedHouseCandidate)
					break;

				float tangentSign = 0.0;
				if (confirmIndex == 1)
					tangentSign = -1.0;
				if (confirmIndex == 2)
					tangentSign = 1.0;

				vector targetPoint = center + (faceNormal * halfDepth) + (faceTangent * sideOffset * tangentSign);
				targetPoint[1] = confirmHeight;

				TraceParam confirmTrace = new TraceParam();
				confirmTrace.Start = origin + "0 1.8 0";
				confirmTrace.End = targetPoint - (faceNormal * 0.45);
				confirmTrace.Exclude = owner;

				vector hitPosition = vector.Zero;
				vector hitNormal = "0 1 0";
				if (!ResolveEntityGeometryHit(world, confirmTrace, traceExcludeRoot, entityToConfirm, hitPosition, hitNormal))
					continue;

				float verticality = 1.0 - Math.AbsFloat(hitNormal[1]);
				if (verticality < 0.52)
					continue;

				if (vector.Dot(hitNormal, faceNormal) < 0.05)
					continue;

				if (confirmHits > 0)
				{
					if (vector.Dot(hitNormal, firstHitNormal) < 0.78)
						continue;

					vector hitDelta = hitPosition - firstHitPosition;
					hitDelta[1] = 0.0;
					if (hitDelta.LengthSq() < 0.60 * 0.60)
						continue;
				}
				else
				{
					firstHitPosition = hitPosition;
					firstHitNormal = hitNormal;
				}

				accumulatedPosition = accumulatedPosition + hitPosition;
				accumulatedNormal = accumulatedNormal + hitNormal;
				confirmHits++;
			}

			int requiredConfirmHits = 1;
			if (detachedHouseCandidate)
				requiredConfirmHits = 1;
			else if (!strongBuildingHintConfirmed && (footprintArea < 18.0 || footprintMin < 1.8))
				requiredConfirmHits = 2;
			else if (footprintArea >= 80.0 && footprintMin >= 5.0 && height >= 5.5)
				requiredConfirmHits = 1;

			bool usedHouseBoundsFallback = false;
			if (confirmHits < requiredConfirmHits && houseLikePrefab && strongBuildingHintConfirmed)
			{
				vector proxyPosition = center + (faceNormal * halfDepth);
				proxyPosition[1] = mins[1] + BS5_EchoMath.Clamp(height * 0.40, 1.9, 4.4);
				vector proxyNormal = faceNormal;
				float proxyForwardSupport = 0.42;
				float proxyFootprintMin = 12.0;
				float proxyFootprintMax = 120.0;
				float proxyHeightMax = 14.0;
				if (urbanMicroPass && detachedHouseCandidate)
				{
					proxyForwardSupport = 0.35;
					proxyFootprintMin = 6.0;
				}
				if (proxyNormal.LengthSq() >= 0.0001 && forwardSupport >= proxyForwardSupport && footprintArea >= proxyFootprintMin && footprintArea <= proxyFootprintMax && height >= 1.2 && height <= proxyHeightMax)
				{
					proxyNormal.Normalize();
					accumulatedPosition = proxyPosition;
					accumulatedNormal = proxyNormal;
					confirmHits = requiredConfirmHits;
					usedHouseBoundsFallback = true;
				}
			}

			if (confirmHits < requiredConfirmHits)
				continue;
			confirmedFacades++;

			vector candidatePosition = accumulatedPosition * (1.0 / confirmHits);
			vector candidateNormal = accumulatedNormal;
			if (candidateNormal.LengthSq() < 0.0001)
				candidateNormal = faceNormal;
			else
				candidateNormal.Normalize();

			BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
			candidate.m_bValid = true;
			candidate.m_vPosition = candidatePosition + (candidateNormal * 0.12);
			candidate.m_vNormal = candidateNormal;
			candidate.m_fDistance = vector.Distance(origin, candidatePosition);
			candidate.m_fDelaySeconds = ResolveTailDelaySeconds(settings, candidate.m_fDistance, BS5_EchoCandidateSourceType.FACADE_HIT);
			candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(candidate.m_fDistance / BS5_EchoMath.MaxFloat(1.0, settlementMaxDistance));
			candidate.m_fPanBias = vector.Dot(toCenter, viewRight);
			candidate.m_fDirectionSupport = BS5_EchoMath.Clamp01(0.65 + (forwardSupport * 0.35));
			candidate.m_iRank = 60;
			candidate.m_eSourceType = BS5_EchoCandidateSourceType.FACADE_HIT;
			candidate.m_fScore = BS5_EchoMath.Clamp01(0.54 + (forwardSupport * 0.22) + ((1.0 - candidate.m_fDistanceNorm) * 0.16) + BS5_EchoMath.Clamp01(height / 12.0) * 0.08 + (prefabHintScoreConfirmed * 0.10));
			if (urbanMicroPass)
				candidate.m_fScore = BS5_EchoMath.Clamp01(candidate.m_fScore + settings.GetSoundMapUrbanScoreBoost());
			candidate.m_fPhysicalScore = 1.0;
			candidate.m_fZonePriority = 1.0;
			candidate.m_sTerrainProfile = "facade";
			if (usedHouseBoundsFallback)
				candidate.m_fScore *= 0.88;

			MergeCandidate(candidates, candidate, maxCandidates, mergeDistanceSq);
		}
		confirmTopPrefabs = BuildForwardFacadePrefabDebugString(confirmEntities, 4);
		s_ForwardFacadeQueryActive = false;
	}

	protected static bool CollectForwardFacadeEntityCallback(IEntity entity)
	{
		if (!s_ForwardFacadeQueryActive)
			return false;
		EnsureFacadeCaches();
		if (!entity)
			return true;

		IEntity normalizedEntity = entity.GetRootParent();
		if (!normalizedEntity)
			normalizedEntity = entity;

		if (s_ForwardFacadeQueryExcludeRoot && BS5_EchoEnvironmentAnalyzer.IsSelfHierarchyHit(normalizedEntity, s_ForwardFacadeQueryExcludeRoot))
			return true;

		if (ContainsEntityReference(s_ForwardFacadeQueryEntities, normalizedEntity))
			return true;

		float prefabHintScore = 0.0;
		bool strongBuildingHint = false;
		if (!ResolveForwardFacadePrefabHints(normalizedEntity, prefabHintScore, strongBuildingHint))
			return true;

		float callbackScore = prefabHintScore * 0.55;
		vector mins;
		vector maxs;
		normalizedEntity.GetWorldBounds(mins, maxs);
		vector center = (mins + maxs) * 0.5;
		vector toCenter = center - s_ForwardFacadeQueryOrigin;
		toCenter[1] = 0.0;
		if (toCenter.LengthSq() > 1.0)
		{
			float flatDistance = toCenter.Length();
			toCenter.Normalize();
			float forwardSupport = vector.Dot(toCenter, s_ForwardFacadeQueryViewFlat);
			callbackScore += BS5_EchoMath.Clamp01(0.35 + (forwardSupport * 0.35));
			callbackScore += BS5_EchoMath.Clamp01(1.0 - (flatDistance / 320.0)) * 0.10;
		}
		if (strongBuildingHint)
			callbackScore += 0.20;

		InsertRankedForwardFacadeEntity(s_ForwardFacadeQueryEntities, s_ForwardFacadeQueryScores, normalizedEntity, callbackScore, s_ForwardFacadeQueryEntityLimit);
		if (s_ForwardFacadeQueryEntities.Count() >= s_ForwardFacadeQueryEntityLimit)
			return false;

		return true;
	}

	protected static void SynthesizeCandidatesFromSectorField(array<ref BS5_EchoReflectorCandidate> candidates, array<ref BS5_EchoReflectorCandidate> sectorCandidates, BS5_EchoDriverComponent settings, BS5_TailProfileType profile, BS5_EchoAnalysisResult result, int targetCount)
	{
		candidates.Clear();
		float maxDistance = ResolveProfileMaxDistance(settings, profile, result);
		foreach (BS5_EchoReflectorCandidate sourceCandidate : sectorCandidates)
		{
			BS5_EchoReflectorCandidate candidate = BS5_EchoMath.CloneCandidate(sourceCandidate);
			if (!candidate || !candidate.m_bValid)
				continue;

			if (!AdaptSectorCandidateForProfile(candidate, settings, profile, maxDistance))
				continue;

			InsertCandidate(candidates, candidate, targetCount);
		}
	}

	protected static bool AdaptSectorCandidateForProfile(BS5_EchoReflectorCandidate candidate, BS5_EchoDriverComponent settings, BS5_TailProfileType profile, float maxDistance)
	{
		if (!candidate)
			return false;

		if (maxDistance <= 0.0)
			return false;

		candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(candidate.m_fDistance / maxDistance);
		float adjustedScore = candidate.m_fScore;
		switch (profile)
		{
			case BS5_TailProfileType.SETTLEMENT_EDGE:
				if (candidate.m_fDistance > settings.GetSettlementMaxDistanceMeters() * 1.10)
					return false;
				if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT)
					adjustedScore *= 1.22;
				else if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_HIT)
					adjustedScore *= 0.88;
				else if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_PRIMARY)
					adjustedScore *= 0.52;
				break;

			case BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE:
				if (candidate.m_fDistance > settings.GetIndoorMaxDistanceMeters())
					return false;
				if (Math.AbsFloat(candidate.m_fPanBias) > 0.55)
					adjustedScore *= 0.78;
				if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_PRIMARY)
					adjustedScore *= 0.45;
				adjustedScore *= 1.0 - (candidate.m_fDistanceNorm * 0.18);
				break;

			case BS5_TailProfileType.HILL_TERRAIN:
				if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_HIT || candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_PRIMARY)
					adjustedScore *= 1.08;
				break;

			case BS5_TailProfileType.OPEN_COASTAL:
			case BS5_TailProfileType.OPEN_MEADOW:
				if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT)
					adjustedScore *= 1.12;
				else if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_PRIMARY)
					adjustedScore *= 0.88;
				break;

			case BS5_TailProfileType.FOREST_EDGE:
				if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT)
					adjustedScore *= 0.94;
				break;
		}

		if (candidate.m_fDirectionSupport >= 0.95)
			adjustedScore *= 1.06;
		else
			adjustedScore *= 0.95 + (candidate.m_fDirectionSupport * 0.08);

		adjustedScore *= 0.72 + ((1.0 - candidate.m_fDistanceNorm) * 0.28);
		candidate.m_fScore = BS5_EchoMath.Clamp01(adjustedScore);
		return settings.ShouldAcceptCandidate(candidate.m_fScore);
	}

	protected static int ResolveMinimumUsefulCount(BS5_TailProfileType profile, int targetCount)
	{
		if (targetCount <= 1)
			return 1;

		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
			return 2;

		return 2;
	}

	protected static float ResolveSectorMaxAzimuthDegrees(BS5_EchoDriverComponent settings, BS5_TailProfileType profile)
	{
		float coneHalf = settings.GetTailForwardConeDegrees() * 0.5;
		float maxAzimuth = coneHalf + settings.GetOutsideConeOffsetDegrees();
		maxAzimuth = BS5_EchoMath.Clamp(BS5_EchoMath.MaxFloat(maxAzimuth, 85.0), coneHalf, 120.0);
		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
			maxAzimuth = BS5_EchoMath.Clamp(BS5_EchoMath.MaxFloat(coneHalf + 6.0, maxAzimuth * settings.GetIndoorLateralSpreadScale()), coneHalf, 85.0);

		return maxAzimuth;
	}

	protected static float ResolveSectorAngleDegrees(int sectorIndex, int sectorCount, float maxAzimuthDegrees)
	{
		if (sectorCount <= 1)
			return 0.0;

		float alpha = (float)sectorIndex / (float)(sectorCount - 1);
		return Lerp(-maxAzimuthDegrees, maxAzimuthDegrees, alpha);
	}

	protected static float ResolveSectorWeight(BS5_EchoDriverComponent settings, float angleDegrees, float maxAzimuthDegrees)
	{
		float absAngle = Math.AbsFloat(angleDegrees);
		float coneHalf = settings.GetTailForwardConeDegrees() * 0.5;
		float forwardWeight = settings.GetTailForwardSectorWeight();
		float sideWeight = settings.GetTailSideSectorWeight();
		if (absAngle <= coneHalf)
		{
			float innerAlpha = 1.0 - (absAngle / BS5_EchoMath.MaxFloat(1.0, coneHalf));
			return BS5_EchoMath.Clamp01(sideWeight + ((forwardWeight - sideWeight) * (0.45 + innerAlpha * 0.55)));
		}

		float outsideRange = BS5_EchoMath.MaxFloat(1.0, maxAzimuthDegrees - coneHalf);
		float outsideAlpha = BS5_EchoMath.Clamp01((absAngle - coneHalf) / outsideRange);
		return BS5_EchoMath.Clamp01(sideWeight * (1.0 - outsideAlpha * 0.20));
	}

	protected static bool ShouldUseLongSectorProbe(BS5_TailProfileType profile, float sectorWeight, bool forwardSector)
	{
		if (!forwardSector)
			return false;

		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
			return false;

		if (profile == BS5_TailProfileType.SETTLEMENT_EDGE)
			return sectorWeight > 0.86;

		if (profile == BS5_TailProfileType.FOREST_EDGE)
			return sectorWeight > 0.82;

		return sectorWeight > 0.74;
	}

	protected static float ResolveSectorProbeDistance(BS5_EchoDriverComponent settings, BS5_TailProfileType profile, BS5_EchoAnalysisResult result, float sectorWeight, bool forwardSector, bool longProbe)
	{
		float distance = settings.GetPrimaryTailDistanceMin();
		switch (profile)
		{
			case BS5_TailProfileType.SETTLEMENT_EDGE:
				if (longProbe)
					distance = settings.GetSettlementMaxDistanceMeters();
				else
					distance = Lerp(settings.GetPrimaryTailDistanceMin(), settings.GetSettlementMaxDistanceMeters(), 0.52 + (sectorWeight * 0.18));
				break;

			case BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE:
				if (longProbe)
					distance = BS5_EchoMath.MinFloat(settings.GetIndoorMaxDistanceMeters(), 26.0);
				else
					distance = Lerp(10.0, BS5_EchoMath.MinFloat(settings.GetIndoorMaxDistanceMeters(), 18.0), 0.45 + (sectorWeight * 0.20));
				break;

			case BS5_TailProfileType.FOREST_EDGE:
				if (longProbe)
					distance = BS5_EchoMath.MinFloat(settings.GetTerrainMaxDistanceMeters(), 135.0);
				else
					distance = Lerp(48.0, BS5_EchoMath.MinFloat(settings.GetTerrainMaxDistanceMeters(), 92.0), 0.45 + (sectorWeight * 0.18));
				break;

			case BS5_TailProfileType.HILL_TERRAIN:
				if (longProbe)
					distance = Lerp(settings.GetTertiaryTailDistanceMin(), BS5_EchoMath.MinFloat(settings.GetTertiaryTailDistanceMax(), settings.GetTerrainMaxDistanceMeters()), 0.40 + (sectorWeight * 0.22));
				else
					distance = Lerp(BS5_EchoMath.MaxFloat(70.0, settings.GetPrimaryTailDistanceMin()), BS5_EchoMath.MinFloat(settings.GetPrimaryTailDistanceMax(), 150.0), 0.52 + (sectorWeight * 0.20));
				break;

			case BS5_TailProfileType.OPEN_COASTAL:
				if (longProbe)
					distance = Lerp(settings.GetTertiaryTailDistanceMin(), BS5_EchoMath.MinFloat(settings.GetTertiaryTailDistanceMax(), settings.GetOpenTailMaxDistanceMeters()), 0.38 + (sectorWeight * 0.22));
				else
					distance = Lerp(settings.GetPrimaryTailDistanceMin(), BS5_EchoMath.MinFloat(settings.GetPrimaryTailDistanceMax(), settings.GetOpenTailMinDistanceMeters()), 0.55 + (sectorWeight * 0.20));
				break;

			default:
				if (longProbe)
					distance = Lerp(settings.GetTertiaryTailDistanceMin(), BS5_EchoMath.MinFloat(settings.GetTertiaryTailDistanceMax(), settings.GetOpenTailMaxDistanceMeters()), 0.35 + (sectorWeight * 0.20));
				else
					distance = Lerp(settings.GetPrimaryTailDistanceMin(), BS5_EchoMath.MinFloat(settings.GetPrimaryTailDistanceMax(), settings.GetOpenTailMinDistanceMeters()), 0.48 + (sectorWeight * 0.18));
				break;
		}

		if (!forwardSector && !longProbe)
			distance *= 0.92;

		if (result && result.m_bSuppressedShot)
			distance *= settings.GetSuppressedDistanceMultiplier();

		return distance;
	}

	protected static BS5_EchoReflectorCandidate BuildBestSectorProbeCandidate(BaseWorld world, BS5_EchoDriverComponent settings, IEntity owner, vector origin, vector viewOrigin, vector sectorDirection, vector viewRight, float angleDegrees, float sectorWeight, float maxDistance, float probeDistance, int heightSampleCount, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot, BS5_TailProfileType profile, bool forwardSector)
	{
		float overshootDistance = ResolveSectorOvershootDistance(probeDistance, maxDistance, profile);
		vector probeAnchor = origin + sectorDirection * probeDistance;
		BS5_EchoReflectorCandidate bestCandidate = null;
		for (int heightSampleIndex = 0; heightSampleIndex < heightSampleCount; heightSampleIndex++)
		{
			float sampleHeight = ResolveSectorHeightSample(settings, profile, heightSampleIndex, heightSampleCount);
			vector samplePoint = probeAnchor;
			samplePoint[1] = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, samplePoint, origin[1] + sampleHeight + overshootDistance) + sampleHeight;

			TraceParam trace = new TraceParam();
			trace.Start = viewOrigin;
			trace.End = samplePoint + (sectorDirection * overshootDistance);
			if (traceExcludeArray)
				trace.ExcludeArray = traceExcludeArray;
			else
				trace.Exclude = owner;

			vector hitPosition = vector.Zero;
			vector hitNormal = "0 1 0";
			if (!ResolveGeometryHit(world, trace, traceExcludeRoot, hitPosition, hitNormal))
				continue;

			if (!IsUsefulSectorSurface(settings, hitNormal))
				continue;

			BS5_EchoReflectorCandidate hitCandidate = CreateSectorHitCandidate(settings, origin, sectorDirection, viewRight, angleDegrees, sectorWeight, maxDistance, hitPosition, hitNormal, profile, forwardSector);
			if (!hitCandidate)
				continue;

			if (!bestCandidate || hitCandidate.m_fScore > bestCandidate.m_fScore)
				bestCandidate = hitCandidate;
		}

		return bestCandidate;
	}

	protected static BS5_EchoReflectorCandidate CreateSectorHitCandidate(BS5_EchoDriverComponent settings, vector origin, vector sectorDirection, vector viewRight, float angleDegrees, float sectorWeight, float maxDistance, vector hitPosition, vector hitNormal, BS5_TailProfileType profile, bool forwardSector)
	{
		BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
		candidate.m_bValid = true;
		candidate.m_vPosition = hitPosition + (hitNormal * 0.12);
		candidate.m_vNormal = hitNormal;
		candidate.m_fDistance = vector.Distance(origin, candidate.m_vPosition);
		float verticality = 1.0 - Math.AbsFloat(hitNormal[1]);
		if (verticality > 0.28)
			candidate.m_eSourceType = BS5_EchoCandidateSourceType.FACADE_HIT;
		else
			candidate.m_eSourceType = BS5_EchoCandidateSourceType.TERRAIN_HIT;
		candidate.m_fDelaySeconds = ResolveTailDelaySeconds(settings, candidate.m_fDistance, candidate.m_eSourceType);
		candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(candidate.m_fDistance / BS5_EchoMath.MaxFloat(1.0, maxDistance));
		candidate.m_fPanBias = vector.Dot(sectorDirection, viewRight);
		if (forwardSector)
			candidate.m_fDirectionSupport = 1.0;
		else
			candidate.m_fDirectionSupport = BS5_EchoMath.Clamp01(0.55 + (sectorWeight * 0.25));
		candidate.m_iRank = (int)Math.AbsFloat(angleDegrees);

		float surfaceScore = ResolveSectorSurfaceScore(settings, hitNormal);
		float score = 0.34;
		score += sectorWeight * 0.24;
		score += (1.0 - candidate.m_fDistanceNorm) * 0.16;
		score += surfaceScore * 0.16;
		if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT)
			score += 0.12;
		if (forwardSector)
			score += 0.07;
		if (profile == BS5_TailProfileType.SETTLEMENT_EDGE && candidate.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT)
			score += 0.08;
		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE && candidate.m_fDistance <= settings.GetIndoorMaxDistanceMeters())
			score += 0.05;
		candidate.m_fScore = BS5_EchoMath.Clamp01(score);
		return candidate;
	}

	protected static BS5_EchoReflectorCandidate CreateTerrainPrimaryCandidate(BaseWorld world, BS5_EchoDriverComponent settings, BS5_EnvironmentSnapshot snapshot, vector origin, vector sectorDirection, vector viewRight, float angleDegrees, float sectorWeight, float maxDistance, float probeDistance, BS5_TailProfileType profile, bool forwardSector)
	{
		vector samplePoint = origin + sectorDirection * probeDistance;
		float terrainHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, samplePoint, origin[1] + 120.0);
		vector terrainPosition = samplePoint;
		terrainPosition[1] = terrainHeight;
		vector terrainNormal = BS5_EnvironmentAudioClassifier.ResolveTerrainNormal(world, terrainPosition);
		if (!ShouldUseTerrainPrimaryCandidate(profile, snapshot, terrainNormal))
			return null;

		if (!forwardSector)
		{
			float sideCutoff = 0.50;
			if (profile == BS5_TailProfileType.HILL_TERRAIN || profile == BS5_TailProfileType.OPEN_MEADOW)
				sideCutoff = 0.38;
			if (sectorWeight < sideCutoff)
				return null;
		}

		BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
		candidate.m_bValid = true;
		candidate.m_vPosition = terrainPosition + (terrainNormal * 0.08);
		candidate.m_vNormal = terrainNormal;
		candidate.m_fDistance = vector.Distance(origin, candidate.m_vPosition);
		candidate.m_fDelaySeconds = ResolveTailDelaySeconds(settings, candidate.m_fDistance, BS5_EchoCandidateSourceType.TERRAIN_PRIMARY);
		candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(candidate.m_fDistance / BS5_EchoMath.MaxFloat(1.0, maxDistance));
		candidate.m_fPanBias = vector.Dot(sectorDirection, viewRight);
		if (forwardSector)
			candidate.m_fDirectionSupport = 1.0;
		else
			candidate.m_fDirectionSupport = BS5_EchoMath.Clamp01(0.52 + (sectorWeight * 0.22));
		candidate.m_iRank = 500 + (int)Math.AbsFloat(angleDegrees);
		candidate.m_eSourceType = BS5_EchoCandidateSourceType.TERRAIN_PRIMARY;

		float slope = 1.0 - Math.AbsFloat(terrainNormal[1]);
		float heightRelief = Math.AbsFloat(terrainPosition[1] - origin[1]);
		float reliefScore = BS5_EchoMath.Clamp01(heightRelief / BS5_EchoMath.MaxFloat(1.0, probeDistance * 0.18));
		float score = 0.26;
		score += sectorWeight * 0.26;
		score += slope * 0.24;
		score += reliefScore * 0.16;
		score += (1.0 - candidate.m_fDistanceNorm) * 0.14;
		if (profile == BS5_TailProfileType.HILL_TERRAIN)
			score += 0.08;
		if (profile == BS5_TailProfileType.OPEN_MEADOW)
			score += reliefScore * 0.08;
		if (!forwardSector)
			score *= 0.92;
		candidate.m_fScore = BS5_EchoMath.Clamp01(score);
		return candidate;
	}

	protected static float ResolveSectorOvershootDistance(float probeDistance, float maxDistance, BS5_TailProfileType profile)
	{
		float overshootDistance = BS5_EchoMath.Clamp(maxDistance * 0.05, 8.0, 18.0);
		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
			overshootDistance = 6.0;
		if (probeDistance < 24.0)
			overshootDistance = BS5_EchoMath.MinFloat(overshootDistance, 8.0);
		return overshootDistance;
	}

	protected static float ResolveSectorHeightSample(BS5_EchoDriverComponent settings, BS5_TailProfileType profile, int sampleIndex, int sampleCount)
	{
		float lowHeight = settings.GetTailScanHeightLowMeters();
		float highHeight = settings.GetTailScanHeightHighMeters();
		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
		{
			lowHeight = 1.6;
			highHeight = 3.2;
		}

		if (sampleCount <= 1)
			return lowHeight;

		if (sampleIndex <= 0)
			return lowHeight;
		if (sampleIndex >= sampleCount - 1)
			return highHeight;

		float alpha = (float)sampleIndex / (float)(sampleCount - 1);
		return Lerp(lowHeight, highHeight, alpha);
	}

	protected static bool IsUsefulSectorSurface(BS5_EchoDriverComponent settings, vector hitNormal)
	{
		return Math.AbsFloat(hitNormal[1]) <= settings.GetTailGroundNormalMaxY();
	}

	protected static float ResolveSectorSurfaceScore(BS5_EchoDriverComponent settings, vector hitNormal)
	{
		float normalY = Math.AbsFloat(hitNormal[1]);
		float maxGroundY = settings.GetTailGroundNormalMaxY();
		float surfaceAlpha = 1.0 - BS5_EchoMath.Clamp01(normalY / BS5_EchoMath.MaxFloat(0.01, maxGroundY));
		return BS5_EchoMath.Clamp01(0.35 + (surfaceAlpha * 0.65));
	}

	protected static void MergeSectorCandidate(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoReflectorCandidate candidate, int maxCandidates, float mergeDistanceSq, float normalMergeDot)
	{
		if (!candidate || !candidate.m_bValid)
			return;

		for (int i = 0; i < candidates.Count(); i++)
		{
			BS5_EchoReflectorCandidate existing = candidates[i];
			if (!existing)
				continue;

			if (vector.DistanceSq(existing.m_vPosition, candidate.m_vPosition) > mergeDistanceSq)
				continue;

			if (vector.Dot(existing.m_vNormal, candidate.m_vNormal) < normalMergeDot)
				continue;

			if (candidate.m_fScore > existing.m_fScore)
				candidates[i] = candidate;
			return;
		}

		InsertCandidate(candidates, candidate, maxCandidates);
	}

	protected static void ResolveViewBasis(BaseWorld world, IEntity owner, vector origin, vector fallbackForward, float pitchClampDegrees, out vector viewOrigin, out vector viewForward)
	{
		viewOrigin = origin + "0 1.6 0";
		viewForward = fallbackForward;

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		IEntity controlledEntity = playerController.GetControlledEntity();
		if (!controlledEntity || !owner)
			return;

		IEntity controlledRoot = controlledEntity.GetRootParent();
		if (!controlledRoot)
			controlledRoot = controlledEntity;

		IEntity ownerRoot = owner.GetRootParent();
		if (!ownerRoot)
			ownerRoot = owner;

		if (controlledRoot != ownerRoot)
			return;

		vector cameraMatrix[4];
		world.GetCurrentCamera(cameraMatrix);
		viewOrigin = cameraMatrix[3];
		viewForward = ClampPitch(cameraMatrix[2].Normalized(), pitchClampDegrees);
	}

	protected static BS5_TailProfileType ResolveTailProfile(BS5_EnvironmentSnapshot snapshot, BS5_EchoAnalysisResult result)
	{
		float localConfinement = result.m_fFrontConfinement;
		localConfinement += result.m_fBackConfinement;
		localConfinement += result.m_fLeftConfinement;
		localConfinement += result.m_fRightConfinement;
		localConfinement *= 0.25;

		bool strongInterior = snapshot.m_fInteriorWeight > 0.38 || result.m_fIndoorScore > 0.52;
		bool confinedInterior = snapshot.m_fInteriorWeight > 0.24 && result.m_fIndoorScore > 0.30 && localConfinement > 0.34;
		bool compactRoom = snapshot.m_fInteriorWeight > 0.28 && snapshot.m_fRoomSize > 0.0 && snapshot.m_fRoomSize < 140.0;
		if (strongInterior || confinedInterior || compactRoom)
			return BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE;

		if (snapshot.m_fWaterWeight > 0.38 && snapshot.m_fOpenWeight > 0.55)
			return BS5_TailProfileType.OPEN_COASTAL;

		if (snapshot.m_fTerrainSlopeBias > 0.42 || snapshot.m_fTerrainHeightBias > 0.45)
			return BS5_TailProfileType.HILL_TERRAIN;
		if ((snapshot.m_fTerrainSlopeBias > 0.20 || snapshot.m_fTerrainHeightBias > 0.18) && snapshot.m_fOpenWeight < 0.96)
			return BS5_TailProfileType.HILL_TERRAIN;

		if (snapshot.m_fForestWeight > 0.48 && snapshot.m_fOpenWeight < 0.72)
			return BS5_TailProfileType.FOREST_EDGE;

		return BS5_TailProfileType.OPEN_MEADOW;
	}

	protected static int ResolveTargetCount(BS5_EchoDriverComponent settings, BS5_TailProfileType profile, BS5_EchoAnalysisResult result)
	{
		int targetCount = settings.GetTerrainTailTargetCount();
		switch (profile)
		{
			case BS5_TailProfileType.OPEN_COASTAL:
			case BS5_TailProfileType.OPEN_MEADOW:
				targetCount = settings.GetTerrainTailTargetCount();
				break;

			case BS5_TailProfileType.SETTLEMENT_EDGE:
				targetCount = settings.GetSettlementTailTargetCount();
				break;

			case BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE:
				targetCount = settings.GetIndoorTailTargetCount();
				break;
		}

		if (result && result.m_bSuppressedShot)
		{
			int suppressedTargetCount = settings.GetMaxSuppressedTailEmittersPerShot();
			if (suppressedTargetCount < targetCount)
				targetCount = suppressedTargetCount;
		}

		return targetCount;
	}

	protected static float ResolveProfileMaxDistance(BS5_EchoDriverComponent settings, BS5_TailProfileType profile, BS5_EchoAnalysisResult result)
	{
		float maxDistance = settings.GetTerrainMaxDistanceMeters();
		switch (profile)
		{
			case BS5_TailProfileType.SETTLEMENT_EDGE:
				maxDistance = settings.GetSettlementMaxDistanceMeters();
				break;

			case BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE:
				maxDistance = settings.GetIndoorMaxDistanceMeters();
				break;

			case BS5_TailProfileType.OPEN_COASTAL:
			case BS5_TailProfileType.OPEN_MEADOW:
				maxDistance = settings.GetOpenTailMaxDistanceMeters();
				break;
		}

		if (result && result.m_bSuppressedShot)
			maxDistance *= settings.GetSuppressedDistanceMultiplier();

		return maxDistance;
	}

	protected static float ResolveSeedDistance(BS5_EchoDriverComponent settings, BS5_TailProfileType profile, BS5_EchoAnalysisResult result, int seedIndex, int seedCount, bool forwardSeed)
	{
		float nearDistance = settings.GetPrimaryTailDistanceMin();
		float midDistance = settings.GetSecondaryTailDistanceMax();
		float farDistance = BS5_EchoMath.MinFloat(settings.GetTertiaryTailDistanceMax(), settings.GetTerrainMaxDistanceMeters());

		switch (profile)
		{
			case BS5_TailProfileType.SETTLEMENT_EDGE:
				nearDistance = settings.GetPrimaryTailDistanceMin();
				midDistance = BS5_EchoMath.MinFloat(settings.GetSettlementMaxDistanceMeters(), 95.0);
				farDistance = settings.GetSettlementMaxDistanceMeters();
				break;

			case BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE:
				nearDistance = 10.0;
				midDistance = 22.0;
				farDistance = settings.GetIndoorMaxDistanceMeters();
				break;

			case BS5_TailProfileType.OPEN_COASTAL:
			case BS5_TailProfileType.OPEN_MEADOW:
				nearDistance = settings.GetOpenTailMinDistanceMeters();
				midDistance = Lerp(settings.GetOpenTailMinDistanceMeters(), settings.GetOpenTailMaxDistanceMeters(), 0.55);
				farDistance = BS5_EchoMath.MinFloat(settings.GetTertiaryTailDistanceMax(), settings.GetOpenTailMaxDistanceMeters());
				break;

			case BS5_TailProfileType.FOREST_EDGE:
				nearDistance = 45.0;
				midDistance = 95.0;
				farDistance = BS5_EchoMath.MinFloat(settings.GetTerrainMaxDistanceMeters(), 150.0);
				break;
		}

		int ringIndex = seedIndex % 3;
		if (!forwardSeed)
			ringIndex = (seedIndex + 1) % 3;

		switch (ringIndex)
		{
			case 0:
				if (result && result.m_bSuppressedShot)
					return nearDistance * settings.GetSuppressedDistanceMultiplier();
				return nearDistance;
			case 1:
				if (result && result.m_bSuppressedShot)
					return midDistance * settings.GetSuppressedDistanceMultiplier();
				return midDistance;
		}

		if (result && result.m_bSuppressedShot)
			return farDistance * settings.GetSuppressedDistanceMultiplier();
		return farDistance;
	}

	protected static float ResolveForwardSeedAngle(int seedIndex, int seedCount, float coneHalf)
	{
		if (seedCount <= 1)
			return 0.0;

		float alpha = (float)seedIndex / (float)(seedCount - 1);
		return Lerp(-coneHalf, coneHalf, alpha);
	}

	protected static float ResolveLateralSeedAngle(int seedIndex, int seedCount, float coneHalf, float maxAzimuth, BS5_EchoDriverComponent settings, BS5_TailProfileType profile)
	{
		if (seedCount <= 0)
			return maxAzimuth;

		int halfCount = seedCount / 2;
		if (halfCount < 1)
			halfCount = 1;
		bool left = seedIndex >= halfCount;
		int localIndex = seedIndex;
		if (left)
			localIndex -= halfCount;

		float alpha = 0.0;
		if (halfCount > 1)
			alpha = (float)localIndex / (float)(halfCount - 1);

		float lateralMaxAzimuth = maxAzimuth;
		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
			lateralMaxAzimuth = BS5_EchoMath.MaxFloat(coneHalf + 6.0, maxAzimuth * settings.GetIndoorLateralSpreadScale());

		float angle = Lerp(coneHalf + 10.0, lateralMaxAzimuth, alpha);
		if (left)
			return angle;

		return -angle;
	}

	protected static BS5_EchoReflectorCandidate BuildAnchorCandidate(BS5_EchoDriverComponent settings, BS5_EnvironmentSnapshot snapshot, IEntity owner, vector origin, vector viewOrigin, vector viewFlat, vector viewRight, float angleDegrees, float distance, float maxDistance, bool forwardSeed, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot, out int facadeHits, out int anchorHits, BS5_TailProfileType profile)
	{
		BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return candidate;

		vector direction = GetFlatDirection(viewFlat, viewRight, angleDegrees);
		vector samplePoint = origin + direction * distance;
		float terrainHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, samplePoint, origin[1] + 120.0);
		vector terrainPosition = samplePoint;
		terrainPosition[1] = terrainHeight;
		vector terrainNormal = BS5_EnvironmentAudioClassifier.ResolveTerrainNormal(world, terrainPosition);
		float targetHeight = ResolveTargetHeight(snapshot, terrainPosition, terrainNormal, forwardSeed, profile);

		TraceParam trace = new TraceParam();
		trace.Start = viewOrigin;
		trace.End = samplePoint + ("0 1 0" * targetHeight) + (direction * 10.0);
		if (traceExcludeArray)
			trace.ExcludeArray = traceExcludeArray;
		else
			trace.Exclude = owner;

		vector hitPosition = vector.Zero;
		vector hitNormal = "0 1 0";
		bool geometryHit = ResolveGeometryHit(world, trace, traceExcludeRoot, hitPosition, hitNormal);
		float surfaceVerticality = 0.0;
		if (geometryHit)
			surfaceVerticality = 1.0 - Math.AbsFloat(hitNormal[1]);
		bool facadeLike = geometryHit && surfaceVerticality > 0.32;

		if (geometryHit)
			anchorHits++;
		if (facadeLike)
			facadeHits++;

		if (!geometryHit)
		{
			if (!ShouldUseTerrainPrimaryCandidate(profile, snapshot, terrainNormal))
				return candidate;

			candidate.m_vPosition = terrainPosition + (terrainNormal * 0.08);
			candidate.m_vNormal = terrainNormal;
			candidate.m_eSourceType = BS5_EchoCandidateSourceType.TERRAIN_PRIMARY;
		}
		else
		{
			candidate.m_vPosition = hitPosition + (hitNormal * 0.12);
			candidate.m_vNormal = hitNormal;
			if (facadeLike)
				candidate.m_eSourceType = BS5_EchoCandidateSourceType.FACADE_HIT;
			else
				candidate.m_eSourceType = BS5_EchoCandidateSourceType.TERRAIN_HIT;
		}

		candidate.m_bValid = true;
		candidate.m_fDistance = vector.Distance(origin, candidate.m_vPosition);
		candidate.m_fDelaySeconds = candidate.m_fDistance / settings.GetSoundSpeedMetersPerSecond();
		candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(candidate.m_fDistance / BS5_EchoMath.MaxFloat(1.0, maxDistance));
		candidate.m_fPanBias = vector.Dot(direction, viewRight);
		if (forwardSeed)
			candidate.m_fDirectionSupport = 1.0;
		else
			candidate.m_fDirectionSupport = 0.6;

		candidate.m_iRank = (int)Math.AbsFloat(angleDegrees);
		if (!forwardSeed)
			candidate.m_iRank += 100;

		float sectorWeight = 0.62;
		if (forwardSeed)
			sectorWeight = 1.0;
		float distanceWeight = 1.0 - candidate.m_fDistanceNorm;
		float surfaceScore = BS5_EchoMath.Clamp01(0.30 + ((1.0 - Math.AbsFloat(candidate.m_vNormal[1])) * 0.70));
		if (geometryHit)
			surfaceScore = BS5_EchoMath.Clamp01(0.35 + surfaceVerticality * 0.65);
		float profileWeight = 1.0;
		if (snapshot.m_fWaterWeight > 0.35 && forwardSeed)
			profileWeight += 0.08;
		if (snapshot.m_fTerrainSlopeBias > 0.35 && !geometryHit)
			profileWeight += 0.12;
		if (facadeLike)
			profileWeight += 0.18;
		if (!geometryHit)
			profileWeight *= 0.82;

		float score = sectorWeight;
		score *= 0.55 + (distanceWeight * 0.45);
		score *= 0.45 + (surfaceScore * 0.55);
		score *= profileWeight;
		candidate.m_fScore = BS5_EchoMath.Clamp01(score);

		if (!settings.ShouldAcceptCandidate(candidate.m_fScore))
			candidate.m_bValid = false;

		return candidate;
	}

	protected static bool ShouldUseTerrainPrimaryCandidate(BS5_TailProfileType profile, BS5_EnvironmentSnapshot snapshot, vector terrainNormal)
	{
		float slope = 1.0 - Math.AbsFloat(terrainNormal[1]);
		switch (profile)
		{
			case BS5_TailProfileType.HILL_TERRAIN:
				return slope > 0.10 && (snapshot.m_fTerrainSlopeBias > 0.18 || snapshot.m_fTerrainHeightBias > 0.18);

			case BS5_TailProfileType.OPEN_COASTAL:
				return snapshot.m_fWaterWeight > 0.32 && slope > 0.04;

			case BS5_TailProfileType.OPEN_MEADOW:
				return (snapshot.m_fTerrainHeightBias > 0.16 || snapshot.m_fTerrainSlopeBias > 0.14) && slope > 0.05;
		}

		return false;
	}

	protected static void CollectSettlementFacadeCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, IEntity owner, vector origin, vector viewOrigin, vector viewFlat, vector viewRight, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot, out int anchorHits, out int facadeHits, BS5_TailProfileType profile, BS5_EchoAnalysisResult result)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		anchorHits = 0;
		facadeHits = 0;

		array<float> probeDistances = {45.0, 70.0, 95.0, 120.0, settings.GetSettlementMaxDistanceMeters()};
		array<float> probeHeights = {2.2, 4.6, 7.2};
		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
		{
			probeDistances.Clear();
			probeDistances.Insert(8.0);
			probeDistances.Insert(14.0);
			probeDistances.Insert(22.0);
			probeDistances.Insert(32.0);
			probeDistances.Insert(settings.GetIndoorMaxDistanceMeters());

			probeHeights.Clear();
			probeHeights.Insert(1.6);
			probeHeights.Insert(2.4);
			probeHeights.Insert(3.2);
		}
		if (result && result.m_bSuppressedShot)
		{
			float suppressedDistanceMultiplier = settings.GetSuppressedDistanceMultiplier();
			for (int probeIndex = 0; probeIndex < probeDistances.Count(); probeIndex++)
				probeDistances[probeIndex] = probeDistances[probeIndex] * suppressedDistanceMultiplier;
		}
		int traceCount = 7;
		float coneHalf = BS5_EchoMath.MinFloat(settings.GetTailForwardConeDegrees() * 0.5, 55.0);
		float mergeDistanceSq = settings.GetAnchorMergeDistanceMeters();
		mergeDistanceSq *= mergeDistanceSq;
		float facadeMaxDistance = settings.GetSettlementMaxDistanceMeters();
		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
			facadeMaxDistance = settings.GetIndoorMaxDistanceMeters();

		for (int angleIndex = 0; angleIndex < traceCount; angleIndex++)
		{
			float angleDegrees = ResolveForwardSeedAngle(angleIndex, traceCount, coneHalf);
			vector direction = GetFlatDirection(viewFlat, viewRight, angleDegrees);
			float azimuthSupport = 1.0 - (Math.AbsFloat(angleDegrees) / BS5_EchoMath.MaxFloat(1.0, coneHalf));

			foreach (float probeDistance : probeDistances)
			{
				if (probeDistance > facadeMaxDistance)
					continue;

				vector samplePoint = origin + direction * probeDistance;
				foreach (float probeHeight : probeHeights)
				{
					TraceParam trace = new TraceParam();
					trace.Start = viewOrigin;
					trace.End = samplePoint + ("0 1 0" * probeHeight);
					if (traceExcludeArray)
						trace.ExcludeArray = traceExcludeArray;
					else
						trace.Exclude = owner;

					vector hitPosition = vector.Zero;
					vector hitNormal = "0 1 0";
					if (!ResolveGeometryHit(world, trace, traceExcludeRoot, hitPosition, hitNormal))
						continue;

					float verticality = 1.0 - Math.AbsFloat(hitNormal[1]);
					if (verticality < 0.26)
						continue;

					anchorHits++;
					facadeHits++;

					BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
					candidate.m_bValid = true;
					candidate.m_vPosition = hitPosition + (hitNormal * 0.12);
					candidate.m_vNormal = hitNormal;
					candidate.m_fDistance = vector.Distance(origin, candidate.m_vPosition);
					candidate.m_fDelaySeconds = candidate.m_fDistance / settings.GetSoundSpeedMetersPerSecond();
						candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(candidate.m_fDistance / BS5_EchoMath.MaxFloat(1.0, facadeMaxDistance));
						candidate.m_fPanBias = vector.Dot(direction, viewRight);
						candidate.m_fDirectionSupport = 1.0;
						candidate.m_iRank = (int)Math.AbsFloat(angleDegrees);
						candidate.m_eSourceType = BS5_EchoCandidateSourceType.FACADE_HIT;
						candidate.m_fScore = BS5_EchoMath.Clamp01((0.70 + (verticality * 0.22)) * (0.70 + (azimuthSupport * 0.30)) * (0.72 + ((1.0 - candidate.m_fDistanceNorm) * 0.28)));
						int facadeTargetCount = settings.GetSettlementTailTargetCount();
					if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
						facadeTargetCount = settings.GetIndoorTailTargetCount();
					MergeCandidate(candidates, candidate, facadeTargetCount, mergeDistanceSq);
				}
			}
		}
	}

	protected static void AddProfileFallbackCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, BS5_EnvironmentSnapshot snapshot, BS5_EchoAnalysisResult result, vector origin, vector viewFlat, vector viewRight, BS5_TailProfileType profile, int targetCount, float mergeDistanceSq)
	{
		int missingCount = targetCount - candidates.Count();
		if (missingCount <= 0)
			return;

		int fallbackCount = missingCount;
		if (profile == BS5_TailProfileType.OPEN_COASTAL || profile == BS5_TailProfileType.OPEN_MEADOW)
		{
			int openFallbackCount = settings.GetOpenTailFallbackCount();
			if (openFallbackCount < fallbackCount)
				fallbackCount = openFallbackCount;
		}

		array<float> fallbackAngles = {0.0, -22.0, 22.0, -48.0, 48.0};
		for (int i = 0; i < fallbackCount && i < fallbackAngles.Count(); i++)
		{
			float distance = ResolveFallbackDistance(settings, profile, result, i, fallbackCount);
			vector direction = GetFlatDirection(viewFlat, viewRight, fallbackAngles[i]);
			vector samplePoint = origin + direction * distance;

			BaseWorld world = GetGame().GetWorld();
			if (!world)
				continue;

			float terrainHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, samplePoint, origin[1] + 120.0);
			vector terrainPosition = samplePoint;
			terrainPosition[1] = terrainHeight;
			vector terrainNormal = BS5_EnvironmentAudioClassifier.ResolveTerrainNormal(world, terrainPosition);

			BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
			candidate.m_bValid = true;
			candidate.m_vPosition = terrainPosition + (terrainNormal * 0.08);
			candidate.m_vNormal = terrainNormal;
			candidate.m_fDistance = vector.Distance(origin, candidate.m_vPosition);
			candidate.m_fDelaySeconds = ResolveTailDelaySeconds(settings, candidate.m_fDistance, BS5_EchoCandidateSourceType.PROFILE_FALLBACK);
			candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(candidate.m_fDistance / BS5_EchoMath.MaxFloat(1.0, ResolveProfileMaxDistance(settings, profile, result)));
			candidate.m_fPanBias = vector.Dot(direction, viewRight);
			candidate.m_fDirectionSupport = 0.55;
			if (i == 0)
				candidate.m_fDirectionSupport = 1.0;
			candidate.m_iRank = 1000 + i;
			candidate.m_eSourceType = BS5_EchoCandidateSourceType.PROFILE_FALLBACK;
			candidate.m_fScore = BS5_EchoMath.Clamp01(0.38 + ((1.0 - candidate.m_fDistanceNorm) * 0.22) + (snapshot.m_fOpenWeight * 0.20) + (snapshot.m_fTerrainSlopeBias * 0.20));

			MergeCandidate(candidates, candidate, targetCount, mergeDistanceSq);
		}
	}

	protected static float ResolveFallbackDistance(BS5_EchoDriverComponent settings, BS5_TailProfileType profile, BS5_EchoAnalysisResult result, int fallbackIndex, int fallbackCount)
	{
		float nearDistance = settings.GetPrimaryTailDistanceMin();
		float midDistance = settings.GetSecondaryTailDistanceMax();
		float farDistance = BS5_EchoMath.MinFloat(settings.GetTertiaryTailDistanceMax(), settings.GetTerrainMaxDistanceMeters());

		switch (profile)
		{
			case BS5_TailProfileType.SETTLEMENT_EDGE:
				nearDistance = settings.GetPrimaryTailDistanceMin();
				midDistance = BS5_EchoMath.MinFloat(settings.GetSettlementMaxDistanceMeters(), settings.GetSecondaryTailDistanceMax());
				farDistance = settings.GetSettlementMaxDistanceMeters();
				break;

			case BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE:
				nearDistance = 10.0;
				midDistance = 22.0;
				farDistance = settings.GetIndoorMaxDistanceMeters();
				break;

			case BS5_TailProfileType.OPEN_COASTAL:
			case BS5_TailProfileType.OPEN_MEADOW:
				nearDistance = settings.GetOpenTailMinDistanceMeters();
				midDistance = Lerp(settings.GetOpenTailMinDistanceMeters(), settings.GetOpenTailMaxDistanceMeters(), 0.55);
				farDistance = settings.GetOpenTailMaxDistanceMeters();
				break;

			case BS5_TailProfileType.FOREST_EDGE:
				nearDistance = 45.0;
				midDistance = 120.0;
				farDistance = BS5_EchoMath.MinFloat(settings.GetTerrainMaxDistanceMeters(), 220.0);
				break;
		}

		if (result && result.m_bSuppressedShot)
		{
			float suppressedDistanceMultiplier = settings.GetSuppressedDistanceMultiplier();
			nearDistance *= suppressedDistanceMultiplier;
			midDistance *= suppressedDistanceMultiplier;
			farDistance *= suppressedDistanceMultiplier;
		}

		if (fallbackCount <= 1)
			return nearDistance;
		if (fallbackCount == 2)
		{
			if (fallbackIndex == 0)
				return nearDistance;
			return farDistance;
		}
		if (fallbackCount == 3)
		{
			if (fallbackIndex == 0)
				return nearDistance;
			if (fallbackIndex == 1)
				return midDistance;
			return farDistance;
		}

		if (fallbackIndex == 0)
			return nearDistance;
		if (fallbackIndex == 1)
			return midDistance;
		if (fallbackIndex == 2)
			return farDistance;

		return Lerp(midDistance, farDistance, 0.55);
	}

	protected static float ResolveTargetHeight(BS5_EnvironmentSnapshot snapshot, vector terrainPosition, vector terrainNormal, bool forwardSeed, BS5_TailProfileType profile)
	{
		float slope = 1.0 - Math.AbsFloat(terrainNormal[1]);
		float targetHeight = 2.8;
		targetHeight += snapshot.m_fTerrainSlopeBias * 2.0;
		targetHeight += snapshot.m_fCanopyBias * 1.5;
		if (forwardSeed)
			targetHeight += 0.8;
		if (snapshot.m_fInteriorWeight > 0.45)
			targetHeight = 1.8;
		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
			targetHeight = 1.55;

		if (slope > 0.30)
			targetHeight = BS5_EchoMath.MaxFloat(1.6, targetHeight - 0.8);

		return targetHeight;
	}

	protected static void RebalanceCandidateMix(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, BS5_TailProfileType profile, BS5_EchoAnalysisResult result)
	{
		if (!candidates || candidates.Count() < 3)
			return;

		int balancedCount = candidates.Count();
		int emittedCount = settings.GetEmissionCount(result, false);
		if (emittedCount < balancedCount)
			balancedCount = emittedCount;
		if (balancedCount < 2)
			return;

		array<ref BS5_EchoReflectorCandidate> forwardCandidates = new array<ref BS5_EchoReflectorCandidate>();
		array<ref BS5_EchoReflectorCandidate> sideCandidates = new array<ref BS5_EchoReflectorCandidate>();
		for (int candidateIndex = 0; candidateIndex < candidates.Count(); candidateIndex++)
		{
			BS5_EchoReflectorCandidate candidate = candidates[candidateIndex];
			if (!candidate)
				continue;

			if (Math.AbsFloat(candidate.m_fPanBias) > 0.42 && candidate.m_fDirectionSupport < 0.9)
				sideCandidates.Insert(candidate);
			else
				forwardCandidates.Insert(candidate);
		}

		if (forwardCandidates.IsEmpty() || sideCandidates.IsEmpty())
			return;

		int desiredForwardCount = (balancedCount * 65) / 100;
		desiredForwardCount = Math.Clamp(desiredForwardCount, 1, balancedCount - 1);
		if (profile == BS5_TailProfileType.INTERIOR_OR_ROOM_EDGE)
		{
			if (desiredForwardCount < (balancedCount - 1))
				desiredForwardCount = balancedCount - 1;
		}

		int desiredSideCount = balancedCount - desiredForwardCount;
		array<ref BS5_EchoReflectorCandidate> balancedCandidates = new array<ref BS5_EchoReflectorCandidate>();

		AppendCandidates(balancedCandidates, forwardCandidates, desiredForwardCount);
		AppendCandidates(balancedCandidates, sideCandidates, desiredSideCount);

		for (int sourceIndex = 0; sourceIndex < candidates.Count() && balancedCandidates.Count() < balancedCount; sourceIndex++)
		{
			BS5_EchoReflectorCandidate sourceCandidate = candidates[sourceIndex];
			if (!sourceCandidate || ContainsCandidateReference(balancedCandidates, sourceCandidate))
				continue;

			balancedCandidates.Insert(sourceCandidate);
		}

		array<ref BS5_EchoReflectorCandidate> reorderedCandidates = new array<ref BS5_EchoReflectorCandidate>();
		foreach (BS5_EchoReflectorCandidate balancedCandidate : balancedCandidates)
			reorderedCandidates.Insert(balancedCandidate);
		foreach (BS5_EchoReflectorCandidate candidateRemainder : candidates)
		{
			if (!candidateRemainder || ContainsCandidateReference(reorderedCandidates, candidateRemainder))
				continue;

			reorderedCandidates.Insert(candidateRemainder);
		}

		candidates.Clear();
		foreach (BS5_EchoReflectorCandidate reorderedCandidate : reorderedCandidates)
			candidates.Insert(reorderedCandidate);
	}

	protected static void AppendCandidates(array<ref BS5_EchoReflectorCandidate> target, array<ref BS5_EchoReflectorCandidate> source, int count)
	{
		int initialCount = target.Count();
		for (int i = 0; i < source.Count(); i++)
		{
			BS5_EchoReflectorCandidate candidate = source[i];
			if (!candidate || ContainsCandidateReference(target, candidate))
				continue;

			target.Insert(candidate);
			if ((target.Count() - initialCount) >= count)
				return;
		}
	}

	protected static bool ContainsCandidateReference(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoReflectorCandidate candidate)
	{
		if (!candidates || !candidate)
			return false;

		foreach (BS5_EchoReflectorCandidate existing : candidates)
		{
			if (existing == candidate)
				return true;
		}

		return false;
	}

	protected static bool ContainsEntityReference(array<IEntity> entities, IEntity entity)
	{
		if (!entities || !entity)
			return false;

		foreach (IEntity existing : entities)
		{
			if (existing == entity)
				return true;
		}

		return false;
	}

	protected static void InsertRankedForwardFacadeEntity(array<IEntity> entities, array<float> scores, IEntity entity, float score, int maxCount)
	{
		if (!entities || !scores || !entity)
			return;

		if (ContainsEntityReference(entities, entity))
			return;

		int insertIndex = entities.Count();
		for (int i = 0; i < scores.Count(); i++)
		{
			if (score > scores[i])
			{
				insertIndex = i;
				break;
			}
		}

		entities.InsertAt(entity, insertIndex);
		scores.InsertAt(score, insertIndex);

		if (entities.Count() > maxCount)
		{
			entities.Remove(maxCount);
			scores.Remove(maxCount);
		}
	}

	protected static string BuildForwardFacadePrefabDebugString(array<IEntity> entities, int maxEntries)
	{
		if (!entities || entities.IsEmpty())
			return string.Empty;

		string result = string.Empty;
		int emitted = 0;
		foreach (IEntity entity : entities)
		{
			if (!entity)
				continue;

			string prefabPath = ResolveForwardFacadePrefabPath(entity);
			if (prefabPath == string.Empty)
				prefabPath = "<unknown>";

			if (result != string.Empty)
				result += "|";
			result += prefabPath;
			emitted++;
			if (emitted >= maxEntries)
				break;
		}

		return result;
	}

	protected static string ResolveForwardFacadePrefabPath(IEntity entity)
	{
		if (!entity)
			return string.Empty;

		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return string.Empty;

		ResourceName prefabName = prefabData.GetPrefabName();
		if (prefabName == string.Empty)
			return string.Empty;

		return prefabName;
	}

	protected static bool ResolveForwardFacadePrefabHints(IEntity entity, out float prefabHintScore, out bool strongBuildingHint)
	{
		prefabHintScore = 0.0;
		strongBuildingHint = false;
		if (!entity)
			return false;
		EnsureFacadeCaches();

		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return true;

		ResourceName prefabName = prefabData.GetPrefabName();
		if (prefabName == string.Empty)
			return true;

		prefabName.ToLower();
		if (HasPrefabHint(prefabName, s_FacadeRejectPrefabHints))
			return false;

		if (HasPrefabHint(prefabName, s_VegetationPrefabHints))
			return false;

		if (prefabName.IndexOf("structures") != -1)
		{
			prefabHintScore = 0.84;
			strongBuildingHint = true;
			return true;
		}

		if (HasPrefabHint(prefabName, s_BuildingPrefabHints))
		{
			prefabHintScore = 1.0;
			strongBuildingHint = true;
			return true;
		}

		if (HasPrefabHint(prefabName, s_StructurePrefabHints))
		{
			prefabHintScore = 0.62;
			return true;
		}

		prefabHintScore = 0.28;
		return true;
	}

	protected static bool IsHouseLikePrefab(IEntity entity)
	{
		EnsureFacadeCaches();
		ResourceName prefabName = ResolveForwardFacadePrefabPath(entity);
		if (prefabName == string.Empty)
			return false;

		prefabName.ToLower();
		if (HasPrefabHint(prefabName, s_HousePrefabHints))
			return true;
		if (prefabName.IndexOf("structures") != -1)
			return true;

		return false;
	}

	protected static bool HasPrefabHint(ResourceName prefabName, notnull array<string> hints)
	{
		foreach (string hint : hints)
		{
			if (prefabName.IndexOf(hint) != -1)
				return true;
		}

		return false;
	}

	protected static bool TryResolveDetachedHouseFacade(BaseWorld world, IEntity owner, vector origin, IEntity traceExcludeRoot, IEntity targetEntity, vector center, vector faceNormal, vector faceTangent, float halfDepth, float footprintMin, float baseHeight, float height, out vector hitPosition, out vector hitNormal)
	{
		hitPosition = vector.Zero;
		hitNormal = "0 1 0";
		if (!world || !targetEntity)
			return false;

		array<float> heightFactors = {0.34, 0.56, 0.78};
		float sideOffset = BS5_EchoMath.Clamp(footprintMin * 0.12, 0.20, 0.90);
		for (int i = 0; i < heightFactors.Count(); i++)
		{
			vector targetPoint = center + (faceNormal * halfDepth);
			if (i == 1)
				targetPoint = targetPoint + (faceTangent * sideOffset);
			if (i == 2)
				targetPoint = targetPoint - (faceTangent * sideOffset);
			targetPoint[1] = baseHeight + BS5_EchoMath.Clamp(height * heightFactors[i], 1.7, 4.8);

			TraceParam confirmTrace = new TraceParam();
			confirmTrace.Start = origin + "0 1.8 0";
			confirmTrace.End = targetPoint - (faceNormal * 0.35);
			confirmTrace.Exclude = owner;

			vector candidateHitPosition = vector.Zero;
			vector candidateHitNormal = "0 1 0";
			if (!ResolveEntityGeometryHit(world, confirmTrace, traceExcludeRoot, targetEntity, candidateHitPosition, candidateHitNormal))
				continue;

			float verticality = 1.0 - Math.AbsFloat(candidateHitNormal[1]);
			if (verticality < 0.38)
				continue;

			if (vector.Dot(candidateHitNormal, faceNormal) < -0.10)
				continue;

			hitPosition = candidateHitPosition;
			hitNormal = candidateHitNormal;
			return true;
		}

		return false;
	}

	protected static float ResolveTailDelaySeconds(BS5_EchoDriverComponent settings, float distance, BS5_EchoCandidateSourceType sourceType)
	{
		float delayScale = 1.0;
		if (sourceType == BS5_EchoCandidateSourceType.TERRAIN_RIDGE)
			delayScale = 1.15;
		else if (sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_FOREST)
			delayScale = 1.05;

		return (distance * delayScale) / settings.GetSoundSpeedMetersPerSecond();
	}

	protected static bool ResolveEntityGeometryHit(BaseWorld world, inout TraceParam trace, IEntity traceExcludeRoot, IEntity targetEntity, out vector hitPosition, out vector hitNormal)
	{
		float hitFraction = ChimeraCharacter.TraceMoveWithoutCharacters(world, trace);
		if (hitFraction < 0.0 || hitFraction >= 1.0)
			return false;

		if (BS5_EchoEnvironmentAnalyzer.IsSelfHierarchyHit(trace.TraceEnt, traceExcludeRoot))
			return false;

		if (!IsSameEntityHierarchy(trace.TraceEnt, targetEntity))
			return false;

		vector traceDelta = trace.End - trace.Start;
		hitPosition = trace.Start + (traceDelta * hitFraction);
		hitNormal = trace.TraceNorm;
		if (hitNormal.LengthSq() < 0.0001)
			return false;

		hitNormal.Normalize();
		return true;
	}

	protected static bool IsSameEntityHierarchy(IEntity traceEnt, IEntity targetEntity)
	{
		if (!traceEnt || !targetEntity)
			return false;

		if (traceEnt == targetEntity)
			return true;

		IEntity traceRoot = traceEnt.GetRootParent();
		if (!traceRoot)
			traceRoot = traceEnt;

		IEntity targetRoot = targetEntity.GetRootParent();
		if (!targetRoot)
			targetRoot = targetEntity;

		return traceRoot == targetRoot;
	}

	protected static bool ResolveGeometryHit(BaseWorld world, TraceParam trace, IEntity traceExcludeRoot, out vector hitPosition, out vector hitNormal)
	{
		float hitFraction = ChimeraCharacter.TraceMoveWithoutCharacters(world, trace);
		if (hitFraction < 0.0 || hitFraction >= 1.0)
			return false;

		if (BS5_EchoEnvironmentAnalyzer.IsSelfHierarchyHit(trace.TraceEnt, traceExcludeRoot))
			return false;

		vector traceDelta = trace.End - trace.Start;
		hitPosition = trace.Start + (traceDelta * hitFraction);
		hitNormal = trace.TraceNorm;
		if (hitNormal.LengthSq() < 0.0001)
			hitNormal = "0 1 0";
		else
			hitNormal.Normalize();

		return true;
	}

	protected static void MergeCandidate(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoReflectorCandidate candidate, int maxCandidates, float mergeDistanceSq)
	{
		if (!candidate || !candidate.m_bValid)
			return;

		for (int i = 0; i < candidates.Count(); i++)
		{
			BS5_EchoReflectorCandidate existing = candidates[i];
			if (!existing)
				continue;

			if (vector.DistanceSq(existing.m_vPosition, candidate.m_vPosition) > mergeDistanceSq)
				continue;

			if (vector.Dot(existing.m_vNormal, candidate.m_vNormal) < 0.45)
				continue;

			if (candidate.m_fScore > existing.m_fScore)
				candidates[i] = candidate;
			return;
		}

		InsertCandidate(candidates, candidate, maxCandidates);
	}

	protected static void InsertCandidate(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoReflectorCandidate candidate, int maxCandidates)
	{
		int insertIndex = candidates.Count();
		for (int i = 0; i < candidates.Count(); i++)
		{
			if (candidate.m_fScore > candidates[i].m_fScore)
			{
				insertIndex = i;
				break;
			}
		}

		candidates.Insert(candidate);
		for (int j = candidates.Count() - 1; j > insertIndex; j--)
		{
			BS5_EchoReflectorCandidate swap = candidates[j];
			candidates[j] = candidates[j - 1];
			candidates[j - 1] = swap;
		}

		while (candidates.Count() > maxCandidates)
			candidates.Remove(candidates.Count() - 1);
	}

	protected static vector GetFlatDirection(vector flatForward, vector flatRight, float angleDegrees)
	{
		float radians = Math.DEG2RAD * angleDegrees;
		vector direction = (flatForward * Math.Cos(radians)) + (flatRight * Math.Sin(radians));
		direction[1] = 0.0;
		if (direction.LengthSq() < 0.0001)
			return flatForward;

		direction.Normalize();
		return direction;
	}

	protected static vector FlattenDirection(vector direction, vector fallbackDirection)
	{
		vector flatDirection = direction;
		flatDirection[1] = 0.0;
		if (flatDirection.LengthSq() < 0.0001)
			return fallbackDirection;

		flatDirection.Normalize();
		return flatDirection;
	}

	protected static vector BuildFlatRight(vector flatForward)
	{
		vector flatRight = "0 0 0";
		flatRight[0] = flatForward[2];
		flatRight[2] = -flatForward[0];
		if (flatRight.LengthSq() < 0.0001)
			return "1 0 0";

		flatRight.Normalize();
		return flatRight;
	}

	protected static vector ClampPitch(vector direction, float pitchClampDegrees)
	{
		vector clampedDirection = direction;
		float flatLength = Math.Sqrt((direction[0] * direction[0]) + (direction[2] * direction[2]));
		float maxVertical = flatLength * Math.Tan(Math.DEG2RAD * pitchClampDegrees);
		clampedDirection[1] = BS5_EchoMath.Clamp(clampedDirection[1], -maxVertical, maxVertical);
		if (clampedDirection.LengthSq() < 0.0001)
			return "0 0 1";

		clampedDirection.Normalize();
		return clampedDirection;
	}

	protected static float Lerp(float from, float to, float alpha)
	{
		return from + (to - from) * alpha;
	}
}

class BS5_SoundMapAnchorSample
{
	vector m_vPosition;
	vector m_vNormal;
	vector m_vDirection;
	float m_fDistance;
	float m_fAngleDegrees;
	float m_fScore;
	float m_fCity;
	float m_fForest;
	float m_fMeadow;
	float m_fRelief;
	BS5_EchoCandidateSourceType m_eSourceType;

	void BS5_SoundMapAnchorSample()
	{
		m_vPosition = vector.Zero;
		m_vNormal = "0 1 0";
		m_vDirection = "0 0 1";
		m_fDistance = 0.0;
		m_fAngleDegrees = 0.0;
		m_fScore = 0.0;
		m_fCity = 0.0;
		m_fForest = 0.0;
		m_fMeadow = 0.0;
		m_fRelief = 0.0;
		m_eSourceType = BS5_EchoCandidateSourceType.UNKNOWN;
	}
}

class BS5_SoundMapAnchorPlanner : BS5_HybridTailPlanner
{
	static bool TryBuildCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, BS5_EnvironmentSnapshot snapshot, BS5_EchoAnalysisResult result, IEntity owner, vector origin, vector flatForward, vector flatRight, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot, bool explosionLike)
	{
		if (!candidates || !settings || !result)
			return false;

		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		ChimeraWorld chimeraWorld = ChimeraWorld.CastFrom(world);
		if (!chimeraWorld)
			return false;

		SoundWorld soundWorld = chimeraWorld.GetSoundWorld();
		if (!soundWorld)
			return false;

		vector viewOrigin;
		vector viewForward;
		ResolveViewBasis(world, owner, origin, flatForward, settings.GetAnchorPitchClampDegrees(), viewOrigin, viewForward);
		vector viewFlat = FlattenDirection(viewForward, flatForward);
		vector viewRight = BuildFlatRight(viewFlat);

		array<ref BS5_SoundMapAnchorSample> citySamples = new array<ref BS5_SoundMapAnchorSample>();
		array<ref BS5_SoundMapAnchorSample> hillSamples = new array<ref BS5_SoundMapAnchorSample>();
		array<ref BS5_SoundMapAnchorSample> forestSamples = new array<ref BS5_SoundMapAnchorSample>();
		array<ref BS5_SoundMapAnchorSample> meadowSamples = new array<ref BS5_SoundMapAnchorSample>();

		bool hasSoundMapSignal = CollectForwardSamples(soundWorld, world, settings, result, origin, viewFlat, viewRight, citySamples, hillSamples, forestSamples, meadowSamples);
		if (!hasSoundMapSignal)
			return false;

		candidates.Clear();
		result.m_bTailSoundMapPlannerUsed = true;
		result.m_sAnchorPlannerMode = "soundmap";
		result.m_bTailSectorCacheHit = false;
		result.m_bTailForwardNegativeCacheHit = false;
		result.m_iTailSectorCount = settings.GetSoundMapForwardRayCount();
		result.m_iTailHeightSamples = settings.GetSoundMapForwardSampleCount();

		int maxCandidates = settings.GetMaxCandidateCount();
		float mergeDistanceSq = settings.GetTailClusterDistanceMeters();
		mergeDistanceSq *= mergeDistanceSq;

		bool urbanMicroPass = citySamples.IsEmpty() && settings.IsSoundMapUrbanMicroScanEnabled();
		if (!citySamples.IsEmpty() || urbanMicroPass)
		{
			int queryEntities = 0;
			int buildingCandidates = 0;
			int confirmedFacades = 0;
			string topPrefabs = string.Empty;
			string confirmTopPrefabs = string.Empty;
			CollectForwardFacadeEntityCandidates(candidates, settings, owner, origin, viewFlat, viewRight, traceExcludeRoot, urbanMicroPass, queryEntities, buildingCandidates, confirmedFacades, topPrefabs, confirmTopPrefabs);
			result.m_iTailForwardQueryEntities = queryEntities;
			result.m_iTailForwardBuildingCandidates = buildingCandidates;
			result.m_iTailForwardConfirmedFacades = confirmedFacades;
			if (urbanMicroPass)
			{
				result.m_iSoundMapUrbanMicroQueries = queryEntities;
				result.m_iSoundMapUrbanMicroFacades = confirmedFacades;
			}
			result.m_sTailForwardTopPrefabs = topPrefabs;
			result.m_sTailForwardConfirmTopPrefabs = confirmTopPrefabs;
		}

		AppendSamplesAsCandidates(candidates, citySamples, settings, result, origin, viewRight, BS5_EchoCandidateSourceType.SOUNDMAP_CITY, 4, maxCandidates, mergeDistanceSq, true, soundWorld, world, owner, traceExcludeRoot);
		AppendSamplesAsCandidates(candidates, hillSamples, settings, result, origin, viewRight, BS5_EchoCandidateSourceType.TERRAIN_RIDGE, 3, maxCandidates, mergeDistanceSq, false, soundWorld, world, owner, traceExcludeRoot);
		AppendSamplesAsCandidates(candidates, forestSamples, settings, result, origin, viewRight, BS5_EchoCandidateSourceType.SOUNDMAP_FOREST, 2, maxCandidates, mergeDistanceSq, false, soundWorld, world, owner, traceExcludeRoot);
		AppendSamplesAsCandidates(candidates, meadowSamples, settings, result, origin, viewRight, BS5_EchoCandidateSourceType.SOUNDMAP_MEADOW, 1, maxCandidates, mergeDistanceSq, false, soundWorld, world, owner, traceExcludeRoot);

		AddSoftForwardTerrainWaveAnchor(candidates, soundWorld, world, settings, result, origin, viewFlat, viewRight, owner, traceExcludeRoot, maxCandidates, mergeDistanceSq);
		AddForwardFallbacks(candidates, settings, origin, viewFlat, viewRight, soundWorld, world, owner, traceExcludeRoot, maxCandidates, mergeDistanceSq, result);
		AddOmniContextAnchors(candidates, soundWorld, world, settings, result, origin, viewFlat, viewRight, owner, traceExcludeRoot, maxCandidates, mergeDistanceSq);

		bool shouldRunUrbanRescue = false;
		if (settings.IsSoundMapUrbanMicroScanEnabled())
		{
			if (result.m_iTailForwardConfirmedFacades <= 0)
				shouldRunUrbanRescue = true;
		}
		if (shouldRunUrbanRescue)
		{
			int rescueQueryEntities = 0;
			int rescueBuildingCandidates = 0;
			int rescueConfirmedFacades = 0;
			string rescueTopPrefabs = string.Empty;
			string rescueConfirmTopPrefabs = string.Empty;
			CollectForwardFacadeEntityCandidates(candidates, settings, owner, origin, viewFlat, viewRight, traceExcludeRoot, true, rescueQueryEntities, rescueBuildingCandidates, rescueConfirmedFacades, rescueTopPrefabs, rescueConfirmTopPrefabs);
			if (rescueQueryEntities > result.m_iTailForwardQueryEntities)
				result.m_iTailForwardQueryEntities = rescueQueryEntities;
			if (rescueBuildingCandidates > result.m_iTailForwardBuildingCandidates)
				result.m_iTailForwardBuildingCandidates = rescueBuildingCandidates;
			if (rescueConfirmedFacades > result.m_iTailForwardConfirmedFacades)
				result.m_iTailForwardConfirmedFacades = rescueConfirmedFacades;
			if (rescueQueryEntities > result.m_iSoundMapUrbanMicroQueries)
				result.m_iSoundMapUrbanMicroQueries = rescueQueryEntities;
			if (rescueConfirmedFacades > result.m_iSoundMapUrbanMicroFacades)
				result.m_iSoundMapUrbanMicroFacades = rescueConfirmedFacades;
			if (result.m_sTailForwardTopPrefabs == string.Empty)
				result.m_sTailForwardTopPrefabs = rescueTopPrefabs;
			if (result.m_sTailForwardConfirmTopPrefabs == string.Empty)
				result.m_sTailForwardConfirmTopPrefabs = rescueConfirmTopPrefabs;
		}

		ApplyUrbanCandidatePressure(candidates, result, settings);
		SortCandidatesByScore(candidates);
		result.m_iTailAnchorHits = candidates.Count();
		result.m_bTailAnchorFallback = result.m_iSoundMapFallbackAnchors > 0;
		ResolveSoundMapProfile(result, snapshot);
		return candidates.Count() > 0;
	}

	protected static array<float> BuildSoundMapForwardDistances(BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, vector origin, vector viewFlat)
	{
		array<float> baseDistances = new array<float>();
		int sampleCount = settings.GetSoundMapForwardSampleCount();
		float maxDistance = BS5_EchoMath.MinFloat(settings.GetSoundMapForwardMaxDistanceMeters(), settings.GetSoundMapFarTailHardLimitMeters());
		if (sampleCount <= 5)
		{
			baseDistances.Insert(55.0);
			baseDistances.Insert(115.0);
			baseDistances.Insert(210.0);
			baseDistances.Insert(320.0);
			baseDistances.Insert(410.0);
		}
		else if (maxDistance <= 660.0)
		{
			baseDistances.Insert(38.0);
			baseDistances.Insert(72.0);
			baseDistances.Insert(118.0);
			baseDistances.Insert(185.0);
			baseDistances.Insert(280.0);
			baseDistances.Insert(380.0);
			baseDistances.Insert(500.0);
		}
		else
		{
			baseDistances.Insert(42.0);
			baseDistances.Insert(78.0);
			baseDistances.Insert(125.0);
			baseDistances.Insert(195.0);
			baseDistances.Insert(300.0);
			baseDistances.Insert(420.0);
			baseDistances.Insert(520.0);
		}

		int seed = ComputeSoundMapDistanceSeed(origin, viewFlat, sampleCount);
		float jitter = settings.GetSoundMapDistanceJitter();
		array<float> distances = new array<float>();
		string bands = string.Empty;
		for (int i = 0; i < baseDistances.Count() && i < sampleCount; i++)
		{
			float unit = DeterministicUnit(seed, i);
			float signedJitter = (unit * 2.0) - 1.0;
			float distance = baseDistances[i] * (1.0 + (signedJitter * jitter));
			distance = BS5_EchoMath.Clamp(distance, 24.0, maxDistance);
			distances.Insert(distance);
			if (bands != string.Empty)
				bands += ",";
			bands += string.Empty + distance;
		}

		if (result)
		{
			result.m_iSoundMapDistanceSeed = seed;
			result.m_sSoundMapDistanceBands = bands;
		}
		return distances;
	}

	protected static int ComputeSoundMapDistanceSeed(vector origin, vector viewFlat, int sampleCount)
	{
		float seedFloat = Math.AbsFloat((origin[0] * 37.0) + (origin[2] * 53.0) + (viewFlat[0] * 997.0) + (viewFlat[2] * 661.0) + (sampleCount * 41.0));
		return (int)seedFloat;
	}

	protected static float DeterministicUnit(int seed, int index)
	{
		int value = seed + ((index + 1) * 97);
		value = (value * 73) + 41;
		if (value < 0)
			value = -value;
		int bucket = value % 1000;
		return (float)bucket / 999.0;
	}

	protected static bool CollectForwardSamples(SoundWorld soundWorld, BaseWorld world, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, vector origin, vector viewFlat, vector viewRight, array<ref BS5_SoundMapAnchorSample> citySamples, array<ref BS5_SoundMapAnchorSample> hillSamples, array<ref BS5_SoundMapAnchorSample> forestSamples, array<ref BS5_SoundMapAnchorSample> meadowSamples)
	{
		float originTerrain = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, origin, origin[1] + 120.0);
		float coneHalf = settings.GetSoundMapForwardConeDegrees() * 0.5;
		int rayCount = settings.GetSoundMapForwardRayCount();
		int sampleCount = settings.GetSoundMapForwardSampleCount();
		float maxDistance = settings.GetSoundMapForwardMaxDistanceMeters();
		array<float> distances = BuildSoundMapForwardDistances(settings, result, origin, viewFlat);
		bool hasSoundMapSignal = false;

		for (int rayIndex = 0; rayIndex < rayCount; rayIndex++)
		{
			float alpha = 0.0;
			if (rayCount > 1)
				alpha = (float)rayIndex / (float)(rayCount - 1);

			float angleDegrees = Lerp(-coneHalf, coneHalf, alpha);
			vector direction = GetFlatDirection(viewFlat, viewRight, angleDegrees);
			float previousHeight = originTerrain;
			for (int sampleIndex = 0; sampleIndex < sampleCount && sampleIndex < distances.Count(); sampleIndex++)
			{
				float distance = distances[sampleIndex];
				if (distance > maxDistance)
					continue;

				vector samplePosition = origin + (direction * distance);
				float terrainHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, samplePosition, origin[1] + 180.0);
				vector terrainNormal = BS5_EnvironmentAudioClassifier.ResolveTerrainNormal(world, samplePosition);
				samplePosition[1] = terrainHeight + 1.8;

				float sea;
				float forest;
				float city;
				float meadow;
				soundWorld.GetMapValuesAtPos(samplePosition, sea, forest, city, meadow);
				result.m_iSoundMapSamples++;
				if ((sea + forest + city + meadow) > 0.01)
					hasSoundMapSignal = true;

				float relief = ComputeReliefScore(world, origin, originTerrain, samplePosition, terrainHeight, previousHeight, direction, viewRight);
				previousHeight = terrainHeight;

				float centerSupport = 1.0 - (Math.AbsFloat(angleDegrees) / BS5_EchoMath.MaxFloat(1.0, coneHalf));
				centerSupport = BS5_EchoMath.Clamp01(centerSupport);

				if (city >= settings.GetSoundMapCityThreshold())
				{
					result.m_iSoundMapCityHits++;
					BS5_SoundMapAnchorSample citySample = CreateSample(samplePosition, terrainNormal, direction, distance, angleDegrees, BS5_EchoCandidateSourceType.SOUNDMAP_CITY);
					citySample.m_fCity = city;
					citySample.m_fForest = forest;
					citySample.m_fMeadow = meadow;
					citySample.m_fRelief = relief;
					citySample.m_fScore = BS5_EchoMath.Clamp01(0.82 + (city * 0.22) + (centerSupport * 0.10) + ((1.0 - BS5_EchoMath.Clamp01(distance / maxDistance)) * 0.06));
					InsertSample(citySamples, citySample, 6);
				}

				if (relief >= settings.GetSoundMapHillReliefThreshold())
				{
					result.m_iSoundMapHillHits++;
					BS5_SoundMapAnchorSample hillSample = CreateSample(samplePosition, terrainNormal, direction, distance, angleDegrees, BS5_EchoCandidateSourceType.TERRAIN_RIDGE);
					hillSample.m_fCity = city;
					hillSample.m_fForest = forest;
					hillSample.m_fMeadow = meadow;
					hillSample.m_fRelief = relief;
					hillSample.m_fScore = BS5_EchoMath.Clamp01(0.66 + (relief * 0.30) + (centerSupport * 0.08));
					InsertSample(hillSamples, hillSample, 5);
				}

				if (forest >= settings.GetSoundMapForestThreshold())
				{
					result.m_iSoundMapForestHits++;
					BS5_SoundMapAnchorSample forestSample = CreateSample(samplePosition, terrainNormal, direction, distance, angleDegrees, BS5_EchoCandidateSourceType.SOUNDMAP_FOREST);
					forestSample.m_fCity = city;
					forestSample.m_fForest = forest;
					forestSample.m_fMeadow = meadow;
					forestSample.m_fRelief = relief;
					forestSample.m_fScore = BS5_EchoMath.Clamp01(0.50 + (forest * 0.28) + (relief * 0.10) + (centerSupport * 0.06));
					InsertSample(forestSamples, forestSample, 4);
				}

				if (meadow >= settings.GetSoundMapMeadowThreshold())
				{
					result.m_iSoundMapMeadowHits++;
					BS5_SoundMapAnchorSample meadowSample = CreateSample(samplePosition, terrainNormal, direction, distance, angleDegrees, BS5_EchoCandidateSourceType.SOUNDMAP_MEADOW);
					meadowSample.m_fCity = city;
					meadowSample.m_fForest = forest;
					meadowSample.m_fMeadow = meadow;
					meadowSample.m_fRelief = relief;
					meadowSample.m_fScore = BS5_EchoMath.Clamp01(0.34 + (meadow * 0.22) + (centerSupport * 0.04));
					InsertSample(meadowSamples, meadowSample, 3);
				}
			}
		}

		return hasSoundMapSignal;
	}

	protected static void AddForwardFallbacks(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, vector origin, vector viewFlat, vector viewRight, SoundWorld soundWorld, BaseWorld world, IEntity owner, IEntity traceExcludeRoot, int maxCandidates, float mergeDistanceSq, BS5_EchoAnalysisResult result)
	{
		if (candidates.Count() >= 3)
			return;

		array<float> fallbackDistances = BuildSoundMapForwardDistances(settings, result, origin, viewFlat);
		for (int i = 0; i < fallbackDistances.Count(); i++)
		{
			if (candidates.Count() >= 3)
				return;
			if (i > 2)
				return;

			float distance = fallbackDistances[i];
			if (i == 1 && fallbackDistances.Count() > 3)
				distance = fallbackDistances[3];
			else if (i == 2 && fallbackDistances.Count() > 0)
				distance = fallbackDistances[fallbackDistances.Count() - 1];
			if (distance > settings.GetSoundMapForwardMaxDistanceMeters())
				continue;

			vector position = origin + (viewFlat * distance);
			float terrainHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, position, origin[1] + 180.0);
			vector normal = BS5_EnvironmentAudioClassifier.ResolveTerrainNormal(world, position);
			position[1] = terrainHeight + 1.4;

			BS5_SoundMapAnchorSample fallbackSample = CreateSample(position, normal, viewFlat, distance, 0.0, BS5_EchoCandidateSourceType.FORWARD_FALLBACK);
			fallbackSample.m_fScore = BS5_EchoMath.Clamp01(0.44 - ((float)i * 0.055));
			BS5_EchoReflectorCandidate candidate = CreateCandidateFromSample(fallbackSample, settings, result, origin, viewRight, BS5_EchoCandidateSourceType.FORWARD_FALLBACK, false, soundWorld, world, owner, traceExcludeRoot);
			MergeCandidate(candidates, candidate, maxCandidates, mergeDistanceSq);
			result.m_iSoundMapFallbackAnchors++;
		}
	}

	protected static void AddSoftForwardTerrainWaveAnchor(array<ref BS5_EchoReflectorCandidate> candidates, SoundWorld soundWorld, BaseWorld world, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, vector origin, vector viewFlat, vector viewRight, IEntity owner, IEntity traceExcludeRoot, int maxCandidates, float mergeDistanceSq)
	{
		if (!candidates || !soundWorld || !world || !settings || !result)
			return;

		if (HasNearForwardReflectiveAnchor(candidates, origin, viewFlat, 185.0))
			return;

		float originTerrain = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, origin, origin[1] + 120.0);
		array<float> distances = {110.0, 150.0};
		BS5_SoundMapAnchorSample bestSample = null;
		float bestScore = 0.0;
		BS5_EchoCandidateSourceType bestSource = BS5_EchoCandidateSourceType.SOUNDMAP_MEADOW;

		for (int i = 0; i < distances.Count(); i++)
		{
			float distance = distances[i];
			if (distance > settings.GetSoundMapForwardMaxDistanceMeters())
				continue;

			vector samplePosition = origin + (viewFlat * distance);
			float terrainHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, samplePosition, origin[1] + 180.0);
			vector terrainNormal = BS5_EnvironmentAudioClassifier.ResolveTerrainNormal(world, samplePosition);
			samplePosition[1] = terrainHeight + 1.55;

			float sea;
			float forest;
			float city;
			float meadow;
			soundWorld.GetMapValuesAtPos(samplePosition, sea, forest, city, meadow);

			float meadowThreshold = settings.GetSoundMapMeadowThreshold();
			if (meadow < meadowThreshold * 0.45)
				continue;
			if (city >= settings.GetSoundMapCityThreshold())
				continue;

			float rise = terrainHeight - originTerrain;
			float slope = 1.0 - Math.AbsFloat(terrainNormal[1]);
			float relief = ComputeReliefScore(world, origin, originTerrain, samplePosition, terrainHeight, originTerrain, viewFlat, viewRight);
			bool softRise = rise >= 3.2;
			bool softSlope = slope >= 0.045;
			bool softRelief = relief >= settings.GetSoundMapHillReliefThreshold() * 0.42;
			bool flatOpenMeadow = meadow >= meadowThreshold * 0.78 && forest < settings.GetSoundMapForestThreshold() * 0.65 && sea < 0.18;
			if (!softRise && !softSlope && !softRelief && !flatOpenMeadow)
				continue;

			BS5_EchoCandidateSourceType sourceType = BS5_EchoCandidateSourceType.SOUNDMAP_MEADOW;
			if (rise >= 5.0 || relief >= settings.GetSoundMapHillReliefThreshold() * 0.58)
				sourceType = BS5_EchoCandidateSourceType.TERRAIN_RIDGE;

			float score = 0.54;
			score += BS5_EchoMath.Clamp01(meadow) * 0.12;
			score += BS5_EchoMath.Clamp01(Math.AbsFloat(rise) / 18.0) * 0.14;
			score += BS5_EchoMath.Clamp01(slope / 0.14) * 0.12;
			score += BS5_EchoMath.Clamp01(relief / BS5_EchoMath.MaxFloat(0.01, settings.GetSoundMapHillReliefThreshold())) * 0.10;
			if (flatOpenMeadow)
				score += 0.07;
			if (distance <= 125.0)
				score += 0.04;
			score = BS5_EchoMath.Clamp01(score);

			if (score <= bestScore)
				continue;

			bestSample = CreateSample(samplePosition, terrainNormal, viewFlat, distance, 0.0, sourceType);
			bestSample.m_fCity = city;
			bestSample.m_fForest = forest;
			bestSample.m_fMeadow = meadow;
			bestSample.m_fRelief = relief;
			bestSample.m_fScore = score;
			bestScore = score;
			bestSource = sourceType;
		}

		if (!bestSample)
			return;

		BS5_EchoReflectorCandidate candidate = CreateCandidateFromSample(bestSample, settings, result, origin, viewRight, bestSource, false, soundWorld, world, owner, traceExcludeRoot);
		MergeCandidate(candidates, candidate, maxCandidates, mergeDistanceSq);
		if (!ContainsCandidateReference(candidates, candidate))
			return;

		result.m_iSoundMapFallbackAnchors++;
		if (bestSource == BS5_EchoCandidateSourceType.TERRAIN_RIDGE)
			result.m_iSoundMapHillHits++;
		else
			result.m_iSoundMapMeadowHits++;
	}

	protected static bool HasNearForwardReflectiveAnchor(array<ref BS5_EchoReflectorCandidate> candidates, vector origin, vector viewFlat, float maxDistance)
	{
		if (!candidates)
			return false;

		for (int i = 0; i < candidates.Count(); i++)
		{
			BS5_EchoReflectorCandidate candidate = candidates[i];
			if (!candidate || !candidate.m_bValid)
				continue;

			if (candidate.m_fDistance > maxDistance)
				continue;

			vector toCandidate = candidate.m_vPosition - origin;
			toCandidate[1] = 0.0;
			if (toCandidate.LengthSq() < 0.01)
				continue;
			toCandidate.Normalize();
			if (vector.Dot(toCandidate, viewFlat) < 0.62)
				continue;

			if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.SOUNDMAP_CITY || candidate.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT || candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_RIDGE || candidate.m_eSourceType == BS5_EchoCandidateSourceType.SOUNDMAP_FOREST)
				return true;
		}

		return false;
	}

	protected static void AddOmniContextAnchors(array<ref BS5_EchoReflectorCandidate> candidates, SoundWorld soundWorld, BaseWorld world, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, vector origin, vector viewFlat, vector viewRight, IEntity owner, IEntity traceExcludeRoot, int maxCandidates, float mergeDistanceSq)
	{
		int omniLimit = settings.GetSoundMapOmniAnchorCount();
		if (omniLimit <= 0)
			return;

		array<ref BS5_SoundMapAnchorSample> omniSamples = new array<ref BS5_SoundMapAnchorSample>();
		array<float> distances = {100.0, 220.0, 360.0, 450.0};
		int directionCount = settings.GetSoundMapOmniDirectionCount();
		float forwardHalf = settings.GetSoundMapForwardConeDegrees() * 0.5;
		float originTerrain = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, origin, origin[1] + 120.0);

		for (int dirIndex = 0; dirIndex < directionCount; dirIndex++)
		{
			float angleDegrees = ((float)dirIndex / (float)directionCount) * 360.0;
			float signedAngle = NormalizeSignedAngle(angleDegrees);
			if (Math.AbsFloat(signedAngle) <= forwardHalf)
				continue;

			vector direction = GetFlatDirection(viewFlat, viewRight, signedAngle);
			float previousHeight = originTerrain;
			for (int sampleIndex = 0; sampleIndex < distances.Count(); sampleIndex++)
			{
				float distance = distances[sampleIndex];
				if (distance > settings.GetSoundMapOmniRadiusMeters())
					continue;

				vector samplePosition = origin + (direction * distance);
				float terrainHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, samplePosition, origin[1] + 180.0);
				vector terrainNormal = BS5_EnvironmentAudioClassifier.ResolveTerrainNormal(world, samplePosition);
				samplePosition[1] = terrainHeight + 1.6;

				float sea;
				float forest;
				float city;
				float meadow;
				soundWorld.GetMapValuesAtPos(samplePosition, sea, forest, city, meadow);
				float relief = ComputeReliefScore(world, origin, originTerrain, samplePosition, terrainHeight, previousHeight, direction, viewRight);
				previousHeight = terrainHeight;

				float bestSignal = 0.0;
				if (city >= settings.GetSoundMapCityThreshold())
					bestSignal = BS5_EchoMath.MaxFloat(bestSignal, city * 1.0);
				if (relief >= settings.GetSoundMapHillReliefThreshold())
					bestSignal = BS5_EchoMath.MaxFloat(bestSignal, relief * 0.92);
				if (forest >= settings.GetSoundMapForestThreshold())
					bestSignal = BS5_EchoMath.MaxFloat(bestSignal, forest * 0.72);

				if (bestSignal <= 0.0)
					continue;

				BS5_SoundMapAnchorSample omniSample = CreateSample(samplePosition, terrainNormal, direction, distance, signedAngle, BS5_EchoCandidateSourceType.OMNI_CONTEXT);
				omniSample.m_fCity = city;
				omniSample.m_fForest = forest;
				omniSample.m_fMeadow = meadow;
				omniSample.m_fRelief = relief;
				omniSample.m_fScore = BS5_EchoMath.Clamp01(0.38 + (bestSignal * 0.30) + (BS5_EchoMath.Clamp01(distance / settings.GetSoundMapOmniRadiusMeters()) * 0.05));
				InsertSample(omniSamples, omniSample, 6);
			}
		}

		int added = 0;
		for (int i = 0; i < omniSamples.Count(); i++)
		{
			if (added >= omniLimit)
				return;

			BS5_EchoReflectorCandidate candidate = CreateCandidateFromSample(omniSamples[i], settings, result, origin, viewRight, BS5_EchoCandidateSourceType.OMNI_CONTEXT, false, soundWorld, world, owner, traceExcludeRoot);
			int beforeCount = candidates.Count();
			MergeCandidate(candidates, candidate, maxCandidates, mergeDistanceSq);
			if (candidates.Count() > beforeCount)
			{
				added++;
				result.m_iSoundMapOmniAnchors++;
			}
		}
	}

	protected static void AppendSamplesAsCandidates(array<ref BS5_EchoReflectorCandidate> candidates, array<ref BS5_SoundMapAnchorSample> samples, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, vector origin, vector viewRight, BS5_EchoCandidateSourceType sourceType, int maxAdd, int maxCandidates, float mergeDistanceSq, bool allowFacadeSnap, SoundWorld soundWorld, BaseWorld world, IEntity owner, IEntity traceExcludeRoot)
	{
		int added = 0;
		for (int i = 0; i < samples.Count(); i++)
		{
			if (added >= maxAdd)
				return;

			BS5_EchoReflectorCandidate candidate = CreateCandidateFromSample(samples[i], settings, result, origin, viewRight, sourceType, allowFacadeSnap, soundWorld, world, owner, traceExcludeRoot);
			int beforeCount = candidates.Count();
			MergeCandidate(candidates, candidate, maxCandidates, mergeDistanceSq);
			if (candidates.Count() > beforeCount || beforeCount >= maxCandidates)
				added++;
		}
	}

	protected static BS5_EchoReflectorCandidate CreateCandidateFromSample(BS5_SoundMapAnchorSample sample, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, vector origin, vector viewRight, BS5_EchoCandidateSourceType sourceType, bool allowFacadeSnap, SoundWorld soundWorld, BaseWorld world, IEntity owner, IEntity traceExcludeRoot)
	{
		if (!sample)
			return null;

		vector position = sample.m_vPosition;
		vector normal = sample.m_vNormal;
		if (allowFacadeSnap)
		{
			vector snappedPosition;
			vector snappedNormal;
			if (TrySnapFacadeByTrace(world, owner, origin, sample, traceExcludeRoot, snappedPosition, snappedNormal))
			{
				position = snappedPosition + (snappedNormal * 0.12);
				normal = snappedNormal;
				sourceType = BS5_EchoCandidateSourceType.FACADE_HIT;
			}
		}

		BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
		candidate.m_bValid = true;
		candidate.m_vPosition = position;
		candidate.m_vNormal = normal;
		candidate.m_fDistance = vector.Distance(origin, position);
		candidate.m_fDelaySeconds = ResolveTailDelaySeconds(settings, candidate.m_fDistance, sourceType);
		candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(candidate.m_fDistance / BS5_EchoMath.MaxFloat(1.0, settings.GetSoundMapForwardMaxDistanceMeters()));
		candidate.m_fPanBias = vector.Dot(sample.m_vDirection, viewRight);
		float coneHalf = settings.GetSoundMapForwardConeDegrees() * 0.5;
		candidate.m_fDirectionSupport = BS5_EchoMath.Clamp01(1.0 - ((Math.AbsFloat(sample.m_fAngleDegrees) / BS5_EchoMath.MaxFloat(1.0, coneHalf)) * 0.45));
		if (sourceType == BS5_EchoCandidateSourceType.OMNI_CONTEXT)
			candidate.m_fDirectionSupport = 0.45;
		candidate.m_iRank = (int)Math.AbsFloat(sample.m_fAngleDegrees);
		candidate.m_eSourceType = sourceType;
		candidate.m_fScore = BS5_EchoMath.Clamp01(sample.m_fScore);
		candidate.m_fZonePriority = ResolveSoundMapSourcePriority(sourceType);
		candidate.m_fPhysicalScore = ResolveInitialPhysicalScore(sourceType);
		candidate.m_sTerrainProfile = ResolveInitialTerrainProfile(sourceType);

		if (!ValidateTerrainProfileCandidate(candidate, settings, result, world, origin))
			return null;

		if (!EvaluateSoundMapPathPlausibility(candidate, settings, result, world, soundWorld, origin, owner, traceExcludeRoot))
			return null;

		float distanceFit = ResolveSoundMapDistanceFit(candidate, settings);
		candidate.m_fScore = BS5_EchoMath.Clamp01((sample.m_fScore * 0.45) + (candidate.m_fZonePriority * 0.25) + (candidate.m_fPhysicalScore * 0.20) + (distanceFit * 0.10));
		candidate.m_fScore = BS5_EchoMath.Clamp01(candidate.m_fScore * (0.78 + (candidate.m_fPathPlausibility * 0.22)));
		if (candidate.m_sPathProfile == "near_urban")
			candidate.m_fScore = BS5_EchoMath.Clamp01(candidate.m_fScore + 0.10);
		return candidate;
	}

	protected static float ResolveSoundMapSourcePriority(BS5_EchoCandidateSourceType sourceType)
	{
		if (sourceType == BS5_EchoCandidateSourceType.FACADE_HIT)
			return 1.0;
		if (sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_CITY)
			return 0.95;
		if (sourceType == BS5_EchoCandidateSourceType.TERRAIN_RIDGE || sourceType == BS5_EchoCandidateSourceType.TERRAIN_HIT || sourceType == BS5_EchoCandidateSourceType.TERRAIN_PRIMARY)
			return 0.82;
		if (sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_FOREST)
			return 0.65;
		if (sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_MEADOW)
			return 0.48;
		if (sourceType == BS5_EchoCandidateSourceType.FORWARD_FALLBACK)
			return 0.35;
		if (sourceType == BS5_EchoCandidateSourceType.OMNI_CONTEXT)
			return 0.55;
		return 0.40;
	}

	protected static float ResolveInitialPhysicalScore(BS5_EchoCandidateSourceType sourceType)
	{
		if (sourceType == BS5_EchoCandidateSourceType.FACADE_HIT)
			return 1.0;
		if (sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_CITY)
			return 0.90;
		if (sourceType == BS5_EchoCandidateSourceType.TERRAIN_RIDGE || sourceType == BS5_EchoCandidateSourceType.TERRAIN_HIT || sourceType == BS5_EchoCandidateSourceType.TERRAIN_PRIMARY)
			return 0.78;
		if (sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_FOREST)
			return 0.66;
		if (sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_MEADOW)
			return 0.50;
		return 0.45;
	}

	protected static string ResolveInitialTerrainProfile(BS5_EchoCandidateSourceType sourceType)
	{
		if (sourceType == BS5_EchoCandidateSourceType.FACADE_HIT || sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_CITY)
			return "facade";
		if (sourceType == BS5_EchoCandidateSourceType.TERRAIN_RIDGE || sourceType == BS5_EchoCandidateSourceType.TERRAIN_HIT || sourceType == BS5_EchoCandidateSourceType.TERRAIN_PRIMARY)
			return "crest";
		return "flat";
	}

	protected static float ResolveSoundMapDistanceFit(BS5_EchoReflectorCandidate candidate, BS5_EchoDriverComponent settings)
	{
		if (!candidate)
			return 0.0;

		float minDistance = 80.0;
		float idealDistance = 180.0;
		float maxDistance = settings.GetSoundMapForwardMaxDistanceMeters();
		if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT || candidate.m_eSourceType == BS5_EchoCandidateSourceType.SOUNDMAP_CITY)
		{
			minDistance = 45.0;
			idealDistance = 105.0;
			maxDistance = settings.GetSoundMapNearUrbanTailMaxDistanceMeters();
		}
		else if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_RIDGE || candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_HIT || candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_PRIMARY)
		{
			minDistance = 90.0;
			idealDistance = 210.0;
			maxDistance = 400.0;
		}
		else if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.SOUNDMAP_MEADOW)
		{
			minDistance = 140.0;
			idealDistance = 260.0;
			maxDistance = settings.GetSoundMapFarTailSoftLimitMeters();
		}
		else if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.SOUNDMAP_FOREST)
		{
			minDistance = 100.0;
			idealDistance = 220.0;
			maxDistance = settings.GetSoundMapFarTailSoftLimitMeters();
		}

		if (candidate.m_fDistance < minDistance)
			return BS5_EchoMath.Clamp01(candidate.m_fDistance / BS5_EchoMath.MaxFloat(1.0, minDistance));
		if (candidate.m_fDistance <= idealDistance)
			return 1.0;
		return BS5_EchoMath.Clamp01(1.0 - ((candidate.m_fDistance - idealDistance) / BS5_EchoMath.MaxFloat(1.0, maxDistance - idealDistance)));
	}

	protected static bool IsUrbanSoundMapSource(BS5_EchoCandidateSourceType sourceType)
	{
		return sourceType == BS5_EchoCandidateSourceType.FACADE_HIT || sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_CITY;
	}

	protected static bool IsTerrainSoundMapSource(BS5_EchoCandidateSourceType sourceType)
	{
		return sourceType == BS5_EchoCandidateSourceType.TERRAIN_RIDGE || sourceType == BS5_EchoCandidateSourceType.TERRAIN_HIT || sourceType == BS5_EchoCandidateSourceType.TERRAIN_PRIMARY;
	}

	protected static bool IsForestOrMeadowSource(BS5_EchoCandidateSourceType sourceType)
	{
		return sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_FOREST || sourceType == BS5_EchoCandidateSourceType.SOUNDMAP_MEADOW;
	}

	protected static bool IsFrontSlopeOrCrest(BS5_EchoReflectorCandidate candidate)
	{
		if (!candidate)
			return false;
		return candidate.m_sTerrainProfile == "front_slope" || candidate.m_sTerrainProfile == "crest";
	}

	protected static int ResolvePathPlausibilitySampleCount(BS5_EchoDriverComponent settings, float distance)
	{
		int maxSamples = settings.GetSoundMapPathSampleCount();
		int desiredSamples = 6;
		if (distance <= 180.0)
			desiredSamples = 3;
		else if (distance <= 350.0)
			desiredSamples = 4;
		if (desiredSamples > maxSamples)
			return maxSamples;
		return desiredSamples;
	}

	protected static int ResolveDominantPathZone(float city, float forest, float meadow, BS5_EchoDriverComponent settings)
	{
		if (city >= settings.GetSoundMapCityThreshold() && city >= forest && city >= meadow)
			return 1;
		if (forest >= settings.GetSoundMapForestThreshold() && forest >= meadow)
			return 2;
		if (meadow >= settings.GetSoundMapMeadowThreshold())
			return 3;
		return 0;
	}

	protected static bool EvaluateSoundMapPathPlausibility(BS5_EchoReflectorCandidate candidate, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, BaseWorld world, SoundWorld soundWorld, vector origin, IEntity owner, IEntity traceExcludeRoot)
	{
		if (!candidate || !settings || !world || !soundWorld)
			return true;

		candidate.m_fPathPlausibility = 1.0;
		candidate.m_fPathOcclusion = 0.0;
		candidate.m_sPathProfile = "clean";

		if (!settings.IsSoundMapPathPlausibilityValidationEnabled())
			return true;

		float distance = candidate.m_fDistance;
		bool urbanSource = IsUrbanSoundMapSource(candidate.m_eSourceType);
		bool terrainSource = IsTerrainSoundMapSource(candidate.m_eSourceType);
		bool forestOrMeadow = IsForestOrMeadowSource(candidate.m_eSourceType);
		bool fallbackSource = candidate.m_eSourceType == BS5_EchoCandidateSourceType.FORWARD_FALLBACK;
		bool frontSlopeOrCrest = IsFrontSlopeOrCrest(candidate);

		if (urbanSource && distance > 450.0)
		{
			if (result)
				result.m_iSoundMapFarRejects++;
			return false;
		}

		if (fallbackSource && distance > 420.0)
		{
			if (result)
				result.m_iSoundMapFarRejects++;
			return false;
		}

		vector direction = candidate.m_vPosition - origin;
		direction[1] = 0.0;
		float flatDistance = direction.Length();
		if (flatDistance < 1.0)
			return true;
		direction.Normalize();

		vector originEar = origin;
		originEar[1] = originEar[1] + 1.8;
		vector candidateEar = candidate.m_vPosition;
		candidateEar[1] = candidateEar[1] + 1.35;
		float clearance = settings.GetSoundMapPathTerrainClearanceMeters();
		int sampleCount = ResolvePathPlausibilitySampleCount(settings, flatDistance);
		float cityAlongPath = 0.0;
		float forestAlongPath = 0.0;
		float meadowAlongPath = 0.0;
		float maxBlocker = 0.0;
		float blockerWeight = 0.0;
		int blockerCount = 0;
		int zoneTransitions = 0;
		int previousZone = -1;

		for (int i = 1; i <= sampleCount; i++)
		{
			float alpha = (float)i / (float)(sampleCount + 1);
			float sampleDistance = flatDistance * alpha;
			vector samplePosition = origin + (direction * sampleDistance);
			float terrainHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, samplePosition, origin[1] + 180.0);
			samplePosition[1] = terrainHeight + 1.6;

			float lineHeight = Lerp(originEar[1], candidateEar[1], alpha);
			float blockerMeters = terrainHeight - (lineHeight - clearance);
			if (blockerMeters > 0.0)
			{
				blockerCount++;
				maxBlocker = BS5_EchoMath.MaxFloat(maxBlocker, blockerMeters);
				blockerWeight += BS5_EchoMath.Clamp01(blockerMeters / 8.0);
			}

			float sea;
			float forest;
			float city;
			float meadow;
			soundWorld.GetMapValuesAtPos(samplePosition, sea, forest, city, meadow);
			cityAlongPath = BS5_EchoMath.MaxFloat(cityAlongPath, city);
			forestAlongPath = BS5_EchoMath.MaxFloat(forestAlongPath, forest);
			meadowAlongPath = BS5_EchoMath.MaxFloat(meadowAlongPath, meadow);
			int zone = ResolveDominantPathZone(city, forest, meadow, settings);
			if (previousZone >= 0 && zone != previousZone)
				zoneTransitions++;
			previousZone = zone;
		}

		float blockerRatio = (float)blockerCount / BS5_EchoMath.MaxFloat(1.0, (float)sampleCount);
		float blockerAverage = blockerWeight / BS5_EchoMath.MaxFloat(1.0, (float)sampleCount);
		float blockerPeak = BS5_EchoMath.Clamp01(maxBlocker / 12.0) * 0.55;
		float blockerPresence = blockerRatio * 0.25;
		float pathOcclusion = BS5_EchoMath.Clamp01(blockerAverage + blockerPeak + blockerPresence);
		float zoneMismatch = 0.0;
		if (urbanSource && forestAlongPath >= settings.GetSoundMapForestThreshold() * 1.2)
			zoneMismatch += 0.22;
		if (urbanSource && meadowAlongPath >= settings.GetSoundMapMeadowThreshold() * 1.1 && cityAlongPath < settings.GetSoundMapCityThreshold())
			zoneMismatch += 0.12;
		if (forestOrMeadow && cityAlongPath >= settings.GetSoundMapCityThreshold() && distance > settings.GetSoundMapNearUrbanTailMaxDistanceMeters() + 35.0)
			zoneMismatch += 0.18;
		if (zoneTransitions > 1)
			zoneMismatch += BS5_EchoMath.Clamp01((float)(zoneTransitions - 1) * 0.06);

		float pathPlausibility = 1.0 - pathOcclusion - zoneMismatch;
		pathPlausibility = BS5_EchoMath.Clamp01(pathPlausibility);
		candidate.m_fPathOcclusion = pathOcclusion;
		candidate.m_fPathPlausibility = pathPlausibility;

		bool nearUrban = urbanSource && distance >= 45.0 && distance <= settings.GetSoundMapNearUrbanTailMaxDistanceMeters() && pathOcclusion < 0.22;
		if (blockerCount > 0)
			candidate.m_sPathProfile = "terrain_blocked";
		else if (zoneMismatch >= 0.18)
			candidate.m_sPathProfile = "zone_mismatch";
		else if (nearUrban)
			candidate.m_sPathProfile = "near_urban";
		else if (distance > 350.0)
			candidate.m_sPathProfile = "far_clean";
		else
			candidate.m_sPathProfile = "clean";

		if (nearUrban)
		{
			candidate.m_fZonePriority = BS5_EchoMath.MaxFloat(candidate.m_fZonePriority, 1.0);
			candidate.m_fPhysicalScore = BS5_EchoMath.MaxFloat(candidate.m_fPhysicalScore, 0.96);
			if (result)
				result.m_iSoundMapNearUrbanBoosts++;
		}

		if (blockerCount > 0 && !frontSlopeOrCrest)
		{
			if (result)
			{
				result.m_iSoundMapPathRejects++;
				if (distance > settings.GetSoundMapFarTailSoftLimitMeters())
					result.m_iSoundMapFarRejects++;
			}
			return false;
		}

		bool allowFarTerrainCrest = terrainSource && frontSlopeOrCrest && pathPlausibility >= 0.75 && blockerCount == 0;
		if (distance > settings.GetSoundMapFarTailHardLimitMeters() && !allowFarTerrainCrest)
		{
			if (result)
				result.m_iSoundMapFarRejects++;
			return false;
		}

		if (distance > 500.0)
		{
			bool stableFarSource = (terrainSource && frontSlopeOrCrest) || (forestOrMeadow && zoneMismatch < 0.12);
			if (pathPlausibility < 0.75 || blockerCount > 0 || !stableFarSource)
			{
				if (result)
					result.m_iSoundMapFarRejects++;
				return false;
			}
		}
		else if (distance > 350.0 && pathPlausibility < 0.55)
		{
			if (result)
			{
				result.m_iSoundMapPathRejects++;
				result.m_iSoundMapFarRejects++;
			}
			return false;
		}

		if (ShouldTraceSoundMapPath(candidate, settings, distance, pathOcclusion, urbanSource, nearUrban))
		{
			if (result)
				result.m_iSoundMapPathRaycasts++;
			if (PathTraceRejectsCandidate(world, originEar, candidateEar, owner, traceExcludeRoot, nearUrban))
			{
				if (result)
				{
					result.m_iSoundMapPathRayRejects++;
					result.m_iSoundMapPathRejects++;
					if (distance > settings.GetSoundMapFarTailSoftLimitMeters())
						result.m_iSoundMapFarRejects++;
				}
				return false;
			}
		}

		return true;
	}

	protected static bool ShouldTraceSoundMapPath(BS5_EchoReflectorCandidate candidate, BS5_EchoDriverComponent settings, float distance, float pathOcclusion, bool urbanSource, bool nearUrban)
	{
		if (!candidate || !settings)
			return false;
		if (nearUrban && distance <= 150.0 && pathOcclusion < 0.18)
			return false;
		if (distance > settings.GetSoundMapPathRaycastDistanceMeters())
			return true;
		if (pathOcclusion > 0.35)
			return true;
		return urbanSource && distance > 180.0;
	}

	protected static bool PathTraceRejectsCandidate(BaseWorld world, vector originEar, vector candidateEar, IEntity owner, IEntity traceExcludeRoot, bool nearUrban)
	{
		if (!world)
			return false;

		TraceParam trace = new TraceParam();
		trace.Start = originEar;
		trace.End = candidateEar;
		trace.Exclude = owner;
		float hitFraction = ChimeraCharacter.TraceMoveWithoutCharacters(world, trace);
		if (hitFraction < 0.0 || hitFraction >= 1.0)
			return false;
		if (BS5_EchoEnvironmentAnalyzer.IsSelfHierarchyHit(trace.TraceEnt, traceExcludeRoot))
			return false;
		if (nearUrban && hitFraction > 0.72)
			return false;
		return hitFraction < 0.82;
	}

	protected static bool ValidateTerrainProfileCandidate(BS5_EchoReflectorCandidate candidate, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, BaseWorld world, vector origin)
	{
		if (!candidate || !settings || !world)
			return false;

		if (!settings.IsSoundMapTerrainFrontSlopeValidationEnabled())
			return true;

		if (candidate.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT || candidate.m_eSourceType == BS5_EchoCandidateSourceType.SOUNDMAP_CITY)
		{
			candidate.m_sTerrainProfile = "facade";
			candidate.m_fPhysicalScore = BS5_EchoMath.MaxFloat(candidate.m_fPhysicalScore, 0.90);
			return true;
		}

		vector direction = candidate.m_vPosition - origin;
		direction[1] = 0.0;
		float flatDistance = direction.Length();
		if (flatDistance < 35.0)
		{
			candidate.m_sTerrainProfile = "flat";
			return true;
		}
		direction.Normalize();

		int sampleCount = settings.GetSoundMapTerrainProfileSampleCount();
		float originTerrain = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, origin, origin[1] + 120.0);
		float previousHeight = originTerrain;
		float bestScore = -999.0;
		float bestDistance = 0.0;
		float bestHeight = originTerrain;
		int bestIndex = 0;
		bool hasFrontSlope = false;

		for (int i = 1; i <= sampleCount; i++)
		{
			float alpha = (float)i / (float)sampleCount;
			float sampleDistance = flatDistance * alpha;
			vector samplePosition = origin + (direction * sampleDistance);
			float sampleHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, samplePosition, origin[1] + 180.0);
			float rise = sampleHeight - originTerrain;
			float localRise = sampleHeight - previousHeight;
			float angularRise = rise / BS5_EchoMath.MaxFloat(1.0, sampleDistance);
			float localScore = angularRise + (BS5_EchoMath.Clamp01(localRise / 7.0) * 0.10) + (BS5_EchoMath.Clamp01(rise / 18.0) * 0.08);
			if (localScore > bestScore)
			{
				bestScore = localScore;
				bestDistance = sampleDistance;
				bestHeight = sampleHeight;
				bestIndex = i;
			}
			if (rise >= 2.0 && (localRise >= 0.35 || angularRise >= 0.035))
				hasFrontSlope = true;
			previousHeight = sampleHeight;
		}

		float candidateTerrain = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, candidate.m_vPosition, origin[1] + 180.0);
		float dropAfterCrest = bestHeight - candidateTerrain;
		bool candidateBehindCrest = bestIndex < sampleCount - 1 && dropAfterCrest >= settings.GetSoundMapTerrainBacksideDropMeters();
		bool canSnapToCrest = bestDistance >= 28.0 && bestHeight > originTerrain + 1.8;

		if (candidateBehindCrest)
		{
			if (canSnapToCrest)
			{
				SnapCandidateToTerrainProfile(candidate, settings, world, origin, direction, bestDistance, bestHeight, "crest", 0.92);
				if (result)
					result.m_iSoundMapTerrainSnaps++;
				return true;
			}

			candidate.m_sTerrainProfile = "backside";
			candidate.m_fPhysicalScore *= 0.35;
			candidate.m_fScore *= 0.45;
			if (result)
				result.m_iSoundMapBacksideRejects++;
			return false;
		}

		if (hasFrontSlope && canSnapToCrest && candidate.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_RIDGE && bestDistance < flatDistance * 0.86)
		{
			SnapCandidateToTerrainProfile(candidate, settings, world, origin, direction, bestDistance, bestHeight, "front_slope", 1.0);
			if (result)
				result.m_iSoundMapTerrainSnaps++;
			return true;
		}

		if (hasFrontSlope)
		{
			candidate.m_sTerrainProfile = "front_slope";
			candidate.m_fPhysicalScore = BS5_EchoMath.MaxFloat(candidate.m_fPhysicalScore, 0.76);
		}
		else
		{
			candidate.m_sTerrainProfile = "flat";
		}

		return true;
	}

	protected static void SnapCandidateToTerrainProfile(BS5_EchoReflectorCandidate candidate, BS5_EchoDriverComponent settings, BaseWorld world, vector origin, vector direction, float distance, float terrainHeight, string profileName, float scoreScale)
	{
		vector snappedPosition = origin + (direction * distance);
		snappedPosition[1] = terrainHeight + 1.6;
		candidate.m_vPosition = snappedPosition;
		candidate.m_vNormal = BS5_EnvironmentAudioClassifier.ResolveTerrainNormal(world, snappedPosition);
		candidate.m_fDistance = vector.Distance(origin, snappedPosition);
		candidate.m_fDelaySeconds = ResolveTailDelaySeconds(settings, candidate.m_fDistance, candidate.m_eSourceType);
		candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(candidate.m_fDistance / BS5_EchoMath.MaxFloat(1.0, settings.GetSoundMapForwardMaxDistanceMeters()));
		candidate.m_sTerrainProfile = profileName;
		candidate.m_bTerrainSnapped = true;
		candidate.m_fPhysicalScore = BS5_EchoMath.MaxFloat(candidate.m_fPhysicalScore, 0.82);
		candidate.m_fScore = BS5_EchoMath.Clamp01(candidate.m_fScore * scoreScale);
	}

	protected static bool TrySnapFacadeByTrace(BaseWorld world, IEntity owner, vector origin, BS5_SoundMapAnchorSample sample, IEntity traceExcludeRoot, out vector hitPosition, out vector hitNormal)
	{
		hitPosition = vector.Zero;
		hitNormal = "0 1 0";
		if (!world || !sample)
			return false;

		TraceParam trace = new TraceParam();
		trace.Start = origin + "0 1.8 0";
		trace.End = sample.m_vPosition + "0 1.2 0";
		trace.Exclude = owner;

		if (!ResolveGeometryHit(world, trace, traceExcludeRoot, hitPosition, hitNormal))
			return false;

		float verticality = 1.0 - Math.AbsFloat(hitNormal[1]);
		return verticality >= 0.24;
	}

	protected static BS5_SoundMapAnchorSample CreateSample(vector position, vector normal, vector direction, float distance, float angleDegrees, BS5_EchoCandidateSourceType sourceType)
	{
		BS5_SoundMapAnchorSample sample = new BS5_SoundMapAnchorSample();
		sample.m_vPosition = position;
		sample.m_vNormal = normal;
		sample.m_vDirection = direction;
		sample.m_fDistance = distance;
		sample.m_fAngleDegrees = angleDegrees;
		sample.m_eSourceType = sourceType;
		return sample;
	}

	protected static void InsertSample(array<ref BS5_SoundMapAnchorSample> samples, BS5_SoundMapAnchorSample sample, int maxCount)
	{
		if (!samples || !sample)
			return;

		int insertIndex = samples.Count();
		for (int i = 0; i < samples.Count(); i++)
		{
			if (sample.m_fScore > samples[i].m_fScore)
			{
				insertIndex = i;
				break;
			}
		}

		samples.InsertAt(sample, insertIndex);
		while (samples.Count() > maxCount)
			samples.Remove(samples.Count() - 1);
	}

	protected static float ComputeReliefScore(BaseWorld world, vector origin, float originTerrain, vector samplePosition, float terrainHeight, float previousHeight, vector direction, vector viewRight)
	{
		vector terrainNormal = BS5_EnvironmentAudioClassifier.ResolveTerrainNormal(world, samplePosition);
		float slope = 1.0 - Math.AbsFloat(terrainNormal[1]);
		float forwardDelta = Math.AbsFloat(terrainHeight - previousHeight);
		float originDelta = Math.AbsFloat(terrainHeight - originTerrain);
		float sideOffset = BS5_EchoMath.Clamp(vector.Distance(origin, samplePosition) * 0.08, 12.0, 35.0);

		vector leftPoint = samplePosition - (viewRight * sideOffset);
		vector rightPoint = samplePosition + (viewRight * sideOffset);
		float leftHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, leftPoint, samplePosition[1] + 120.0);
		float rightHeight = BS5_EnvironmentAudioClassifier.ResolveTerrainHeight(world, rightPoint, samplePosition[1] + 120.0);
		float sideContrast = Math.AbsFloat(leftHeight - rightHeight);

		float relief = 0.0;
		relief += BS5_EchoMath.Clamp01(forwardDelta / 18.0) * 0.32;
		relief += BS5_EchoMath.Clamp01(originDelta / 55.0) * 0.28;
		relief += BS5_EchoMath.Clamp01(slope / 0.42) * 0.25;
		relief += BS5_EchoMath.Clamp01(sideContrast / 26.0) * 0.15;
		return BS5_EchoMath.Clamp01(relief);
	}

	protected static void ApplyUrbanCandidatePressure(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoAnalysisResult result, BS5_EchoDriverComponent settings)
	{
		if (!candidates || !result || !settings)
			return;

		bool urbanPresent = result.m_iSoundMapCityHits > 0 || result.m_iTailForwardConfirmedFacades > 0 || result.m_iSoundMapUrbanMicroFacades > 0;
		if (!urbanPresent)
			return;

		float bestUrbanDistance = 99999.0;
		for (int i = 0; i < candidates.Count(); i++)
		{
			BS5_EchoReflectorCandidate urbanCandidate = candidates[i];
			if (!urbanCandidate || !urbanCandidate.m_bValid)
				continue;
			if (urbanCandidate.m_eSourceType != BS5_EchoCandidateSourceType.FACADE_HIT && urbanCandidate.m_eSourceType != BS5_EchoCandidateSourceType.SOUNDMAP_CITY)
				continue;
			if (urbanCandidate.m_fDistance < bestUrbanDistance)
				bestUrbanDistance = urbanCandidate.m_fDistance;
			urbanCandidate.m_fScore = BS5_EchoMath.Clamp01(urbanCandidate.m_fScore + settings.GetSoundMapUrbanScoreBoost());
			urbanCandidate.m_fZonePriority = BS5_EchoMath.MaxFloat(urbanCandidate.m_fZonePriority, 0.95);
			urbanCandidate.m_fPhysicalScore = BS5_EchoMath.MaxFloat(urbanCandidate.m_fPhysicalScore, 0.90);
		}

		if (bestUrbanDistance >= 99998.0)
			return;

		for (int j = 0; j < candidates.Count(); j++)
		{
			BS5_EchoReflectorCandidate candidate = candidates[j];
			if (!candidate || !candidate.m_bValid)
				continue;
			if (candidate.m_eSourceType != BS5_EchoCandidateSourceType.SOUNDMAP_MEADOW && candidate.m_eSourceType != BS5_EchoCandidateSourceType.SOUNDMAP_FOREST)
				continue;
			if (candidate.m_fDistance <= bestUrbanDistance + 35.0)
				continue;

			float penalty = 0.15;
			if (candidate.m_fDistance > bestUrbanDistance + 80.0)
				penalty = 0.25;
			candidate.m_fScore = BS5_EchoMath.Clamp01(candidate.m_fScore - penalty);
		}
	}

	protected static void SortCandidatesByScore(array<ref BS5_EchoReflectorCandidate> candidates)
	{
		if (!candidates)
			return;

		for (int i = 0; i < candidates.Count(); i++)
		{
			for (int j = i + 1; j < candidates.Count(); j++)
			{
				if (!candidates[j] || !candidates[i])
					continue;
				if (candidates[j].m_fScore <= candidates[i].m_fScore)
					continue;

				BS5_EchoReflectorCandidate swap = candidates[i];
				candidates[i] = candidates[j];
				candidates[j] = swap;
			}
		}
	}

	protected static float NormalizeSignedAngle(float angleDegrees)
	{
		float normalized = angleDegrees;
		while (normalized > 180.0)
			normalized -= 360.0;
		while (normalized < -180.0)
			normalized += 360.0;
		return normalized;
	}

	protected static void ResolveSoundMapProfile(BS5_EchoAnalysisResult result, BS5_EnvironmentSnapshot snapshot)
	{
		if (result.m_iSoundMapCityHits > 0 || result.m_iTailForwardConfirmedFacades > 0 || result.m_iSoundMapUrbanMicroFacades > 0)
		{
			result.m_eTailProfile = BS5_TailProfileType.SETTLEMENT_EDGE;
			result.m_eEnvironment = BS5_EchoEnvironmentType.URBAN;
			int urbanHits = result.m_iSoundMapCityHits + result.m_iTailForwardConfirmedFacades + result.m_iSoundMapUrbanMicroFacades;
			result.m_fUrbanScore = BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(result.m_fUrbanScore, 0.70 + (urbanHits * 0.035)));
			return;
		}

		if (result.m_iSoundMapHillHits > 0)
		{
			result.m_eTailProfile = BS5_TailProfileType.HILL_TERRAIN;
			result.m_eEnvironment = BS5_EchoEnvironmentType.HILL;
			result.m_fHillScore = BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(result.m_fHillScore, 0.62 + (result.m_iSoundMapHillHits * 0.025)));
			return;
		}

		if (result.m_iSoundMapForestHits > 0 || (snapshot && snapshot.m_fForestWeight > 0.48))
		{
			result.m_eTailProfile = BS5_TailProfileType.FOREST_EDGE;
			result.m_eEnvironment = BS5_EchoEnvironmentType.FOREST;
			result.m_fForestScore = BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(result.m_fForestScore, 0.58));
			return;
		}

		result.m_eTailProfile = BS5_TailProfileType.OPEN_MEADOW;
		result.m_eEnvironment = BS5_EchoEnvironmentType.OPEN_FIELD;
	}
}
