ref array<IEntity> g_BS5SlapbackQueryEntities = new array<IEntity>();
IEntity g_BS5SlapbackQueryExcludeRoot;
bool g_BS5SlapbackQueryActive;
const int BS5_SLAPBACK_QUERY_ENTITY_LIMIT = 18;
ref array<string> g_BS5SlapbackHardPrefabHints = {
	"house", "building", "residential", "civilian", "village", "town", "apartment", "garage", "barn", "farm", "church", "school", "hospital", "shop", "store", "industrial", "warehouse", "factory", "office", "hangar", "depot", "ruin", "shed",
	"wall", "bunker", "compound", "checkpoint", "container", "silo", "vehicle", "car", "truck", "m998", "apc", "btr", "wreck", "concrete", "metal"
};
ref array<string> g_BS5SlapbackRejectPrefabHints = {
	"decal", "pavement", "kerb", "road", "runway", "manhole", "powerline", "pole", "sign", "lamp", "light", "wire", "tree", "bush", "shrub", "hedge", "vegetation", "foliage", "grass", "reed", "plant",
	"fence", "stone", "rock", "boulder", "lowwall", "low_wall", "retaining", "earth", "dirt", "mound", "embankment", "berm", "debris"
};

class BS5_EchoRuntime
{
	static BS5_EchoDriverComponent FindDriver(IEntity effectEntity, BaseMuzzleComponent muzzle)
	{
		BS5_EchoDriverComponent driver = TryFindDriverOnEntity(effectEntity);
		if (driver)
			return driver;

		if (!muzzle)
			return null;

		return TryFindDriverOnEntity(muzzle.GetOwner());
	}

	static BS5_EchoDriverComponent FindExplosionDriver(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		BS5_EchoDriverComponent driver = FindDriver(effectEntity, muzzle);
		if (driver)
			return driver;

		return TryFindDriverOnEntity(projectileEntity);
	}

	static BS5_EchoAnalysisResult AnalyzeShot(BS5_EchoDriverComponent settings, IEntity owner, vector origin, vector forward, bool suppressed)
	{
		return BS5_EchoEnvironmentAnalyzer.Analyze(settings, owner, origin, forward, false, suppressed);
	}

	static BS5_EchoAnalysisResult AnalyzeExplosion(BS5_EchoDriverComponent settings, IEntity owner, vector origin, vector forward)
	{
		return BS5_EchoEnvironmentAnalyzer.Analyze(settings, owner, origin, forward, true, false);
	}

	static void EmitShot(BS5_EchoDriverComponent settings, IEntity owner, BS5_EchoAnalysisResult result, bool emitTails)
	{
		BS5_EchoEmissionService.Emit(settings, owner, result, false, emitTails);
	}

	static void EmitExplosion(BS5_EchoDriverComponent settings, IEntity owner, BS5_EchoAnalysisResult result)
	{
		BS5_EchoEmissionService.Emit(settings, owner, result, true, true);
	}

	protected static BS5_EchoDriverComponent TryFindDriverOnEntity(IEntity entity)
	{
		if (!entity)
			return null;

		BS5_EchoDriverComponent driver = BS5_EchoDriverComponent.Cast(entity.FindComponent(BS5_EchoDriverComponent));
		if (driver)
			return driver;

		IEntity rootEntity = entity.GetRootParent();
		if (!rootEntity || rootEntity == entity)
			return null;

		return BS5_EchoDriverComponent.Cast(rootEntity.FindComponent(BS5_EchoDriverComponent));
	}
}

class BS5_EchoEnvironmentAnalyzer
{
	static BS5_EchoAnalysisResult Analyze(BS5_EchoDriverComponent settings, IEntity owner, vector origin, vector forward, bool explosionLike, bool suppressed)
	{
		BS5_EchoAnalysisResult result = new BS5_EchoAnalysisResult();
		result.m_bSuppressedShot = suppressed;
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return result;

		vector transform[4];
		if (owner)
			owner.GetTransform(transform);
		else
		{
			transform[0] = "1 0 0";
			transform[1] = "0 1 0";
			transform[2] = "0 0 1";
			transform[3] = origin;
		}

		vector right = transform[0].Normalized();
		vector forwardDir = transform[2].Normalized();
		if (forward.LengthSq() > 0.0001)
			forwardDir = forward.Normalized();

		vector worldUp = "0 1 0";
		vector flatForward = GetFlattenedDirection(forwardDir, "0 0 1");
		vector flatRight = BuildFlatRight(flatForward);
		vector back = flatForward * -1.0;
		vector left = flatRight * -1.0;
		vector probeOrigin = origin + (flatForward * settings.GetNearProbeForwardOffsetMeters()) + (worldUp * settings.GetNearProbeVerticalOffsetMeters());

		float nearRadius = settings.GetNearSlapbackRadius();
		if (explosionLike)
			nearRadius = settings.GetExplosionNearRadius();

		float nearTraceLength = nearRadius;
		if (nearTraceLength <= 0.0)
			nearTraceLength = 1.0;

		float intensity = 1.0;
		if (explosionLike)
			intensity = settings.GetExplosionIntensityMultiplier();

		IEntity traceExcludeRoot = ResolveTraceExcludeRoot(owner);
		array<IEntity> traceExcludeArray = null;
		if (owner)
		{
			if (traceExcludeRoot != owner)
			{
				traceExcludeArray = new array<IEntity>();
				traceExcludeArray.Insert(owner);
				traceExcludeArray.Insert(traceExcludeRoot);
			}
		}

		array<vector> directions = {};
		directions.Insert(flatForward);
		directions.Insert(back);
		directions.Insert(flatRight);
		directions.Insert(left);
		directions.Insert((flatForward + flatRight).Normalized());
		directions.Insert((flatForward + left).Normalized());
		directions.Insert((back + flatRight).Normalized());
		directions.Insert((back + left).Normalized());

		int traceLimit = settings.GetMaxTraceCount();
		if (traceLimit < 1)
			traceLimit = 1;
		if (traceLimit > directions.Count())
			traceLimit = directions.Count();

		float totalHitRatio = 0.0;
		float shortestHit = nearTraceLength;
		float sideHits = 0.0;
		float ceilingHits = 0.0;
		float frontConfinement = 0.0;
		float backConfinement = 0.0;
		float rightConfinement = 0.0;
		float leftConfinement = 0.0;
		float terrainClearance = SCR_TerrainHelper.GetHeightAboveTerrain(origin, world, false, null);

		for (int i = 0; i < traceLimit; i++)
		{
			vector direction = directions[i];

			TraceParam trace = new TraceParam();
			trace.Start = probeOrigin;
			trace.End = probeOrigin + direction * nearTraceLength;
			if (traceExcludeArray)
				trace.ExcludeArray = traceExcludeArray;
			else
				trace.Exclude = owner;

			float hitFraction = ChimeraCharacter.TraceMoveWithoutCharacters(world, trace);
			float hitRatio = 1.0;
			if (hitFraction >= 0.0 && hitFraction < 1.0)
			{
				if (IsSelfHierarchyHit(trace.TraceEnt, traceExcludeRoot))
					continue;

				float hitDistance = nearTraceLength * hitFraction;
				hitRatio = hitDistance / nearTraceLength;
				float hitStrength = 1.0 - hitRatio;
				totalHitRatio += hitStrength;
				if (hitDistance < shortestHit)
					shortestHit = hitDistance;

				float verticalWeight = Math.AbsFloat(direction[1]);
				if (verticalWeight > 0.4)
					ceilingHits += 1.0;
				else
					sideHits += 1.0;

				switch (i)
				{
					case 0:
						frontConfinement = BS5_EchoMath.MaxFloat(frontConfinement, hitStrength);
						break;

					case 1:
						backConfinement = BS5_EchoMath.MaxFloat(backConfinement, hitStrength);
						break;

					case 2:
						rightConfinement = BS5_EchoMath.MaxFloat(rightConfinement, hitStrength);
						break;

					case 3:
						leftConfinement = BS5_EchoMath.MaxFloat(leftConfinement, hitStrength);
						break;

					case 4:
						frontConfinement = BS5_EchoMath.MaxFloat(frontConfinement, hitStrength * 0.65);
						rightConfinement = BS5_EchoMath.MaxFloat(rightConfinement, hitStrength * 0.65);
						break;

					case 5:
						frontConfinement = BS5_EchoMath.MaxFloat(frontConfinement, hitStrength * 0.65);
						leftConfinement = BS5_EchoMath.MaxFloat(leftConfinement, hitStrength * 0.65);
						break;

					case 6:
						backConfinement = BS5_EchoMath.MaxFloat(backConfinement, hitStrength * 0.65);
						rightConfinement = BS5_EchoMath.MaxFloat(rightConfinement, hitStrength * 0.65);
						break;

					case 7:
						backConfinement = BS5_EchoMath.MaxFloat(backConfinement, hitStrength * 0.65);
						leftConfinement = BS5_EchoMath.MaxFloat(leftConfinement, hitStrength * 0.65);
						break;
				}

			}
		}

		float probeCount = traceLimit;
		float obstruction = 0.0;
		if (probeCount > 0.0)
			obstruction = totalHitRatio / probeCount;

		float closeConfinement = BS5_EchoMath.Clamp01((settings.GetNearSlapbackRadius() - shortestHit) / settings.GetNearSlapbackRadius());
		float sideConfinement = BS5_EchoMath.Clamp01(sideHits / 4.0);
		float verticalConfinement = BS5_EchoMath.Clamp01(ceilingHits / 2.0);
		float lowCeiling = BS5_EchoMath.Clamp01(1.0 - (terrainClearance / 5.0));
		float openScore = BS5_EchoMath.Clamp01(1.0 - obstruction);

		result.m_fOpenScore = openScore;
		result.m_fUrbanScore = 0.0;
		result.m_fForestScore = 0.0;
		result.m_fHillScore = 0.0;
		result.m_fWaterScore = 0.0;
		result.m_fHardSurfaceScore = sideConfinement;
		result.m_fIndoorScore = BS5_EchoMath.Clamp01((closeConfinement * 0.70) + (verticalConfinement * 0.20) + (lowCeiling * 0.10));
		result.m_fTrenchScore = BS5_EchoMath.Clamp01((closeConfinement * 0.70) + (lowCeiling * 0.30));
		result.m_fNearConfinement = closeConfinement;
		result.m_fVerticalConfinement = verticalConfinement;
		result.m_fFrontConfinement = frontConfinement;
		result.m_fBackConfinement = backConfinement;
		result.m_fRightConfinement = rightConfinement;
		result.m_fLeftConfinement = leftConfinement;
		result.m_eEnvironment = BS5_EchoEnvironmentType.OPEN_FIELD;
		if (result.m_fIndoorScore > 0.55)
			result.m_eEnvironment = BS5_EchoEnvironmentType.INDOOR_LIKE;
		if (result.m_fTrenchScore > 0.65)
			result.m_eEnvironment = BS5_EchoEnvironmentType.TRENCH;

		BS5_EnvironmentSnapshot environmentSnapshot = BS5_EnvironmentAudioClassifier.BuildSnapshot(settings, owner, origin, flatForward, flatRight, result);
		float blendedOpenScore = BS5_EchoMath.Clamp01((result.m_fOpenScore * 0.40) + (environmentSnapshot.m_fOpenWeight * 0.60));
		result.m_fForestScore = environmentSnapshot.m_fForestWeight;
		result.m_fHillScore = BS5_EchoMath.Clamp01((environmentSnapshot.m_fTerrainHeightBias * 0.35) + (environmentSnapshot.m_fTerrainSlopeBias * 0.65));
		result.m_fWaterScore = environmentSnapshot.m_fWaterWeight;
		result.m_fIndoorScore = BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(result.m_fIndoorScore, environmentSnapshot.m_fInteriorWeight));
		result.m_fRoomSize = environmentSnapshot.m_fRoomSize;
		result.m_fOpenScore = BS5_EchoMath.Clamp01(blendedOpenScore * BS5_EchoMath.Clamp(1.0 - (result.m_fIndoorScore * 0.78), 0.12, 1.0));

		if (result.m_fIndoorScore > 0.55)
			result.m_eEnvironment = BS5_EchoEnvironmentType.INDOOR_LIKE;
		else if (result.m_fHillScore > 0.48)
			result.m_eEnvironment = BS5_EchoEnvironmentType.HILL;
		else if (result.m_fForestScore > 0.48)
			result.m_eEnvironment = BS5_EchoEnvironmentType.FOREST;
		else
			result.m_eEnvironment = BS5_EchoEnvironmentType.OPEN_FIELD;

		bool soundMapPlannerBuilt = false;
		if (settings.IsSoundMapAnchorPlannerEnabled())
			soundMapPlannerBuilt = BS5_SoundMapAnchorPlanner.TryBuildCandidates(result.m_aCandidates, settings, environmentSnapshot, result, owner, origin, flatForward, flatRight, traceExcludeArray, traceExcludeRoot, explosionLike);

		if (!soundMapPlannerBuilt)
		{
			if (!settings.IsSoundMapAnchorPlannerEnabled() || settings.AllowLegacyAnchorFallback())
			{
				if (settings.IsSoundMapAnchorPlannerEnabled())
				{
					result.m_bTailLegacyPlannerFallback = true;
					result.m_sAnchorPlannerMode = "legacy_fallback";
				}
				else
				{
					result.m_sAnchorPlannerMode = "legacy";
				}

				BS5_HybridTailPlanner.BuildCandidates(result.m_aCandidates, settings, environmentSnapshot, result, owner, origin, flatForward, flatRight, traceExcludeArray, traceExcludeRoot, explosionLike);
			}
		}

		float firstDelay = 0.0;
		if (!result.m_aCandidates.IsEmpty())
			firstDelay = result.m_aCandidates[0].m_fDelaySeconds;

		result.m_fConfidence = 0.85;
		result.m_fIntensity = BS5_EchoMath.Clamp01(intensity * (0.48 + (result.m_fOpenScore * 0.18) + (result.m_fHillScore * 0.12) + (result.m_fForestScore * 0.10)));
		result.m_fMasterDelaySeconds = firstDelay;
		result.m_fSlapbackDelaySeconds = 0.0;

		if (!explosionLike)
		{
			if (settings.IsSlapbackEnabled())
				CollectSlapbackCandidates(result.m_aSlapbackCandidates, settings, result, owner, origin, probeOrigin, flatForward, flatRight, traceExcludeArray);
			else if (!settings.IsPlayerSlapbackEnabled())
				result.m_sSlapbackMode = "disabled_global";
			else
				result.m_sSlapbackMode = "disabled_driver";
		}

		if (!result.m_aSlapbackCandidates.IsEmpty())
			result.m_fSlapbackDelaySeconds = result.m_aSlapbackCandidates[0].m_fDelaySeconds;

		if (settings.IsDebugEnabled())
		{
			string debugSummary = "mode=engine_env";
			debugSummary += " candidates=" + result.m_aCandidates.Count();
			debugSummary += " firstDelay=" + result.m_fMasterDelaySeconds;
			debugSummary += " shortestNear=" + shortestHit;
			debugSummary += " open=" + result.m_fOpenScore;
			debugSummary += " forest=" + result.m_fForestScore;
			debugSummary += " water=" + result.m_fWaterScore;
			debugSummary += " indoor=" + result.m_fIndoorScore;
			debugSummary += " room=" + result.m_fRoomSize;
			debugSummary += " trench=" + result.m_fTrenchScore;
			debugSummary += " front=" + result.m_fFrontConfinement;
			debugSummary += " left=" + result.m_fLeftConfinement;
			debugSummary += " right=" + result.m_fRightConfinement;
			debugSummary += " envProfile=" + BS5_EchoMath.TailProfileName(result.m_eTailProfile);
			debugSummary += " suppressed=" + result.m_bSuppressedShot;
			debugSummary += " techPreset=" + BS5_PlayerAudioSettings.GetTechnicalPresetId();
			debugSummary += " soundPreset=" + BS5_PlayerAudioSettings.GetSoundPresetId();
			debugSummary += " slapGlobal=" + BS5_PlayerAudioSettings.IsSlapbackEnabled();
			debugSummary += " anchorMode=" + result.m_sAnchorPlannerMode;
			debugSummary += " sectors=" + result.m_iTailSectorCount;
			debugSummary += " heights=" + result.m_iTailHeightSamples;
			debugSummary += " sectorCacheHit=" + result.m_bTailSectorCacheHit;
			debugSummary += " forwardNegativeCacheHit=" + result.m_bTailForwardNegativeCacheHit;
			debugSummary += " legacyFallback=" + result.m_bTailLegacyPlannerFallback;
			debugSummary += " soundMapSamples=" + result.m_iSoundMapSamples;
			debugSummary += " cityHits=" + result.m_iSoundMapCityHits;
			debugSummary += " forestHits=" + result.m_iSoundMapForestHits;
			debugSummary += " meadowHits=" + result.m_iSoundMapMeadowHits;
			debugSummary += " hillHits=" + result.m_iSoundMapHillHits;
			debugSummary += " forwardFallbacks=" + result.m_iSoundMapFallbackAnchors;
			debugSummary += " omniAnchors=" + result.m_iSoundMapOmniAnchors;
			debugSummary += " terrainSnaps=" + result.m_iSoundMapTerrainSnaps;
			debugSummary += " backsideRejects=" + result.m_iSoundMapBacksideRejects;
			debugSummary += " urbanMicroQueries=" + result.m_iSoundMapUrbanMicroQueries;
			debugSummary += " urbanMicroFacades=" + result.m_iSoundMapUrbanMicroFacades;
			debugSummary += " distanceSeed=" + result.m_iSoundMapDistanceSeed;
			if (result.m_sSoundMapDistanceBands != string.Empty)
				debugSummary += " distanceBands=" + result.m_sSoundMapDistanceBands;
			debugSummary += " pathRejects=" + result.m_iSoundMapPathRejects;
			debugSummary += " pathRaycasts=" + result.m_iSoundMapPathRaycasts;
			debugSummary += " pathRayRejects=" + result.m_iSoundMapPathRayRejects;
			debugSummary += " farRejects=" + result.m_iSoundMapFarRejects;
			debugSummary += " nearUrbanBoosts=" + result.m_iSoundMapNearUrbanBoosts;
			debugSummary += " anchorHits=" + result.m_iTailAnchorHits;
			debugSummary += " queryEntities=" + result.m_iTailForwardQueryEntities;
			debugSummary += " queryBuildings=" + result.m_iTailForwardBuildingCandidates;
			debugSummary += " queryFacades=" + result.m_iTailForwardConfirmedFacades;
			if (result.m_sTailForwardTopPrefabs != string.Empty)
				debugSummary += " queryTop=" + result.m_sTailForwardTopPrefabs;
			if (result.m_sTailForwardConfirmTopPrefabs != string.Empty)
				debugSummary += " confirmTop=" + result.m_sTailForwardConfirmTopPrefabs;
			debugSummary += " anchorFallback=" + result.m_bTailAnchorFallback;
			debugSummary += " slapbacks=" + result.m_aSlapbackCandidates.Count();
			debugSummary += " slapMode=" + result.m_sSlapbackMode;
			debugSummary += " slapRays=" + result.m_iSlapbackRayCount;
			debugSummary += " slapHits=" + result.m_iSlapbackHitCount;
			debugSummary += " slapFallback=" + result.m_bSlapbackAnchorFallback;
			debugSummary += " wallScore=" + result.m_fSlapbackWallScore;
			debugSummary += " trenchScore=" + result.m_fSlapbackTrenchScore;
			result.m_sDebugSummary = debugSummary;
		}

		return result;
	}

	protected static void AddTailCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, IEntity owner, vector origin, vector flatForward, vector flatRight, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot, bool explosionLike)
	{
		CollectTailReflectorCandidates(candidates, settings, result, owner, origin, flatForward, flatRight, traceExcludeArray, traceExcludeRoot, explosionLike);
		if (candidates.Count() < settings.GetMaxCandidateCount())
			CollectTailTopSurfaceCandidates(candidates, settings, owner, origin, flatForward, flatRight, traceExcludeArray, traceExcludeRoot, explosionLike);
	}

	protected static void AddTailFallbackCandidate(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, vector origin, vector direction, float distance, float score)
	{
		if (candidates.Count() >= settings.GetMaxCandidateCount())
			return;

		BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
		candidate.m_bValid = true;
		candidate.m_fDistance = distance;
		candidate.m_fScore = score;
		candidate.m_fDelaySeconds = distance / settings.GetSoundSpeedMetersPerSecond();
		candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(distance / settings.GetScanRadius());
		candidate.m_fPanBias = 0.0;
		candidate.m_fDirectionSupport = 0.5;
		candidate.m_vNormal = "0 1 0";
		candidate.m_vPosition = origin + direction * distance;
		candidates.Insert(candidate);
	}

	protected static void CollectTailReflectorCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, IEntity owner, vector origin, vector flatForward, vector flatRight, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot, bool explosionLike)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		float scanRadius = settings.GetScanRadius();
		if (explosionLike)
			scanRadius = settings.GetExplosionScanRadius();

		float traceOriginHeight = BS5_EchoMath.MaxFloat(1.25, settings.GetNearProbeVerticalOffsetMeters());
		vector traceOrigin = origin + ("0 1 0" * traceOriginHeight);
		int sectorCount = settings.GetTailSectorCount();
		int heightSampleCount = settings.GetTailHeightSampleCount();
		int maxCandidates = settings.GetMaxCandidateCount();
		float clusterDistance = settings.GetTailClusterDistanceMeters();
		float clusterDistanceSq = clusterDistance * clusterDistance;
		float normalMergeDot = settings.GetTailNormalMergeDot();
		float maxAzimuthDegrees = GetTailMaxAzimuthDegrees(settings);

		for (int bandIndex = 0; bandIndex < 3; bandIndex++)
		{
			float bandMinDistance = GetTailBandMinDistance(settings, bandIndex);
			float bandMaxDistance = BS5_EchoMath.MinFloat(GetTailBandMaxDistance(settings, bandIndex), scanRadius);
			if (bandMaxDistance <= bandMinDistance)
				continue;

			float bandScore = GetTailBandBaseScore(settings, bandIndex);
			int distanceSampleCount = GetTailBandDistanceSampleCount(bandIndex);
			float overshootDistance = GetTailTraceOvershootDistance(bandMinDistance, bandMaxDistance);
			for (int sectorIndex = 0; sectorIndex < sectorCount; sectorIndex++)
			{
				float angleDegrees = GetTailSectorAngleDegrees(sectorIndex, sectorCount, maxAzimuthDegrees);
				vector sectorDirection = GetFlatSectorDirection(flatForward, flatRight, angleDegrees);
				float sectorWeight = GetTailSectorWeight(settings, angleDegrees, maxAzimuthDegrees);
				if (sectorWeight <= 0.0)
					continue;

				for (int distanceSampleIndex = 0; distanceSampleIndex < distanceSampleCount; distanceSampleIndex++)
				{
					float distanceSampleAlpha = GetTailBandDistanceSampleAlpha(distanceSampleIndex, distanceSampleCount);
					float sampleDistance = Lerp(bandMinDistance, bandMaxDistance, distanceSampleAlpha);
					vector sampleAnchor = origin + sectorDirection * sampleDistance;

					for (int heightSampleIndex = 0; heightSampleIndex < heightSampleCount; heightSampleIndex++)
					{
						float sampleHeight = GetTailHeightSample(settings, heightSampleIndex, heightSampleCount);
						vector samplePoint = sampleAnchor;
						samplePoint[1] = ResolveTerrainHeight(world, samplePoint, origin[1] + sampleHeight + overshootDistance) + sampleHeight;

						TraceParam trace = new TraceParam();
						trace.Start = traceOrigin;
						trace.End = samplePoint + (sectorDirection * overshootDistance);
						if (traceExcludeArray)
							trace.ExcludeArray = traceExcludeArray;
						else
							trace.Exclude = owner;

						float hitFraction = ChimeraCharacter.TraceMoveWithoutCharacters(world, trace);
						if (hitFraction < 0.0 || hitFraction >= 1.0)
							continue;

						if (IsSelfHierarchyHit(trace.TraceEnt, traceExcludeRoot))
							continue;

						vector traceDelta = trace.End - trace.Start;
						vector hitPosition = trace.Start + (traceDelta * hitFraction);
						float hitDistance = vector.Distance(origin, hitPosition);
						if (hitDistance < bandMinDistance || hitDistance > (bandMaxDistance + overshootDistance))
							continue;

						vector hitNormal = trace.TraceNorm;
						if (hitNormal.LengthSq() > 0.0001)
							hitNormal.Normalize();
						else
							hitNormal = sectorDirection * -1.0;

						if (!IsUsefulTailSurface(settings, hitNormal))
							continue;

						float bandDistanceAlpha = 1.0 - BS5_EchoMath.Clamp01((hitDistance - bandMinDistance) / BS5_EchoMath.MaxFloat(0.01, bandMaxDistance - bandMinDistance));
						float surfaceScore = GetTailSurfaceScore(settings, hitNormal);
						float score = bandScore;
						score *= 0.65 + (BS5_EchoMath.Clamp01(sectorWeight) * 0.35);
						score *= 0.55 + (surfaceScore * 0.45);
						score *= 0.70 + (bandDistanceAlpha * 0.30);

						if (!settings.ShouldAcceptCandidate(score))
							continue;

						BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
						candidate.m_bValid = true;
						candidate.m_iRank = bandIndex * 100 + sectorIndex * 10 + distanceSampleIndex * 3 + heightSampleIndex;
						candidate.m_fDistance = hitDistance;
						candidate.m_fScore = BS5_EchoMath.Clamp01(score);
						candidate.m_fDelaySeconds = hitDistance / settings.GetSoundSpeedMetersPerSecond();
						candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(hitDistance / scanRadius);
						candidate.m_fPanBias = vector.Dot(sectorDirection, flatRight);
						candidate.m_fDirectionSupport = BS5_EchoMath.Clamp01(sectorWeight);
						candidate.m_vNormal = hitNormal;
						candidate.m_vPosition = hitPosition + (hitNormal * 0.12);

						MergeTailCandidate(candidates, candidate, maxCandidates, clusterDistanceSq, normalMergeDot);
					}
				}
			}
		}
	}

	protected static void CollectTailTopSurfaceCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, IEntity owner, vector origin, vector flatForward, vector flatRight, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot, bool explosionLike)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		float scanRadius = settings.GetScanRadius();
		if (explosionLike)
			scanRadius = settings.GetExplosionScanRadius();

		int sectorCount = settings.GetTailSectorCount();
		int maxCandidates = settings.GetMaxCandidateCount();
		float clusterDistance = settings.GetTailClusterDistanceMeters();
		float clusterDistanceSq = clusterDistance * clusterDistance;
		float normalMergeDot = settings.GetTailNormalMergeDot();
		float maxAzimuthDegrees = GetTailMaxAzimuthDegrees(settings);
		float probeHeight = settings.GetTailScanHeightHighMeters() + 12.0;

		for (int bandIndex = 0; bandIndex < 3; bandIndex++)
		{
			float bandMinDistance = GetTailBandMinDistance(settings, bandIndex);
			float bandMaxDistance = BS5_EchoMath.MinFloat(GetTailBandMaxDistance(settings, bandIndex), scanRadius);
			if (bandMaxDistance <= bandMinDistance)
				continue;

			float bandScore = GetTailBandBaseScore(settings, bandIndex);
			int distanceSampleCount = GetTailBandDistanceSampleCount(bandIndex);
			for (int sectorIndex = 0; sectorIndex < sectorCount; sectorIndex++)
			{
				float angleDegrees = GetTailSectorAngleDegrees(sectorIndex, sectorCount, maxAzimuthDegrees);
				vector sectorDirection = GetFlatSectorDirection(flatForward, flatRight, angleDegrees);
				float sectorWeight = GetTailSectorWeight(settings, angleDegrees, maxAzimuthDegrees);
				if (sectorWeight <= 0.0)
					continue;

				for (int distanceSampleIndex = 0; distanceSampleIndex < distanceSampleCount; distanceSampleIndex++)
				{
					float distanceSampleAlpha = GetTailBandDistanceSampleAlpha(distanceSampleIndex, distanceSampleCount);
					float sampleDistance = Lerp(bandMinDistance, bandMaxDistance, distanceSampleAlpha);
					vector sampleAnchor = origin + sectorDirection * sampleDistance;
					float terrainHeight = ResolveTerrainHeight(world, sampleAnchor, origin[1] + probeHeight);

					TraceParam trace = new TraceParam();
					trace.Start = sampleAnchor;
					trace.Start[1] = terrainHeight + probeHeight;
					trace.End = sampleAnchor;
					trace.End[1] = terrainHeight - 4.0;
					if (traceExcludeArray)
						trace.ExcludeArray = traceExcludeArray;
					else
						trace.Exclude = owner;

					float hitFraction = ChimeraCharacter.TraceMoveWithoutCharacters(world, trace);
					if (hitFraction < 0.0 || hitFraction >= 1.0)
						continue;

					if (IsSelfHierarchyHit(trace.TraceEnt, traceExcludeRoot))
						continue;

					vector traceDelta = trace.End - trace.Start;
					vector hitPosition = trace.Start + (traceDelta * hitFraction);
					float hitDistance = vector.Distance(origin, hitPosition);
					if (hitDistance < bandMinDistance || hitDistance > bandMaxDistance)
						continue;

					vector hitNormal = trace.TraceNorm;
					if (hitNormal.LengthSq() > 0.0001)
						hitNormal.Normalize();
					else
						hitNormal = "0 1 0";

					float localSurfaceHeight = hitPosition[1] - terrainHeight;
					bool elevatedFeature = localSurfaceHeight > 1.2;
					if (!elevatedFeature && !IsUsefulTailSurface(settings, hitNormal))
						continue;

					float bandDistanceAlpha = 1.0 - BS5_EchoMath.Clamp01((hitDistance - bandMinDistance) / BS5_EchoMath.MaxFloat(0.01, bandMaxDistance - bandMinDistance));
					float surfaceScore = GetTailTopSurfaceScore(settings, hitNormal, localSurfaceHeight);
					float score = bandScore;
					score *= 0.60 + (BS5_EchoMath.Clamp01(sectorWeight) * 0.40);
					score *= 0.55 + (surfaceScore * 0.45);
					score *= 0.70 + (bandDistanceAlpha * 0.30);

					if (!settings.ShouldAcceptCandidate(score))
						continue;

					BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
					candidate.m_bValid = true;
					candidate.m_iRank = 1000 + bandIndex * 100 + sectorIndex * 10 + distanceSampleIndex;
					candidate.m_fDistance = hitDistance;
					candidate.m_fScore = BS5_EchoMath.Clamp01(score);
					candidate.m_fDelaySeconds = hitDistance / settings.GetSoundSpeedMetersPerSecond();
					candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(hitDistance / scanRadius);
					candidate.m_fPanBias = vector.Dot(sectorDirection, flatRight);
					candidate.m_fDirectionSupport = BS5_EchoMath.Clamp01(sectorWeight);
					candidate.m_vNormal = hitNormal;
					if (elevatedFeature && Math.AbsFloat(hitNormal[1]) > settings.GetTailGroundNormalMaxY())
						candidate.m_vPosition = hitPosition + ("0 1 0" * 0.35);
					else
						candidate.m_vPosition = hitPosition + (hitNormal * 0.12);

					MergeTailCandidate(candidates, candidate, maxCandidates, clusterDistanceSq, normalMergeDot);
				}
			}
		}
	}

	protected static void CollectSlapbackCandidates(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, IEntity owner, vector origin, vector probeOrigin, vector forwardDir, vector right, array<IEntity> traceExcludeArray)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		IEntity traceExcludeRoot = ResolveTraceExcludeRoot(owner);

		vector flatForward = forwardDir;
		flatForward[1] = 0.0;
		if (flatForward.LengthSq() < 0.0001)
			flatForward = "0 0 1";
		flatForward.Normalize();

		vector flatRight = right;
		flatRight[1] = 0.0;
		if (flatRight.LengthSq() < 0.0001)
			flatRight = "1 0 0";
		flatRight.Normalize();

		vector slapbackProbeOrigin = origin;
		slapbackProbeOrigin[1] = probeOrigin[1];
		slapbackProbeOrigin += flatForward * BS5_EchoMath.MinFloat(settings.GetNearProbeForwardOffsetMeters() * 0.35, 0.25);

		float forwardAngle = Math.DEG2RAD * settings.GetSlapbackForwardProbeAngleDegrees();
		float sideAngle = Math.DEG2RAD * settings.GetSlapbackSideProbeAngleDegrees();

		array<vector> directions = {};
		directions.Insert(flatForward);
		directions.Insert((flatForward * Math.Cos(forwardAngle) + flatRight * Math.Sin(forwardAngle)).Normalized());
		directions.Insert((flatForward * Math.Cos(forwardAngle) - flatRight * Math.Sin(forwardAngle)).Normalized());
		directions.Insert((flatForward * Math.Cos(sideAngle) + flatRight * Math.Sin(sideAngle)).Normalized());
		directions.Insert((flatForward * Math.Cos(sideAngle) - flatRight * Math.Sin(sideAngle)).Normalized());
		directions.Insert(flatRight);
		directions.Insert(flatRight * -1.0);
		directions.Insert(((flatForward * -0.35) + flatRight).Normalized());
		directions.Insert(((flatForward * -0.35) - flatRight).Normalized());

		array<float> directionBiases = {1.0, 0.96, 0.96, 0.90, 0.90, 0.88, 0.88, 0.70, 0.70};
		array<int> directionSectors = {0, 0, 0, 1, 2, 1, 2, 1, 2};
		array<int> directionRanks = {10, 11, 12, 20, 21, 30, 31, 40, 41};

		float maxDistance = settings.GetNearSlapbackRadius();
		float minDistance = settings.GetSlapbackMinDistanceMeters();
		int maxCandidates = settings.GetMaxSlapbackEmittersPerShot();
		maxCandidates = Math.Clamp(maxCandidates, 1, 2);
		float trenchMaxSideDistance = settings.GetTrenchSlapbackMaxSideDistanceMeters();

		array<ref BS5_EchoReflectorCandidate> wallCandidates = {};
		BS5_EchoReflectorCandidate bestRightSide;
		BS5_EchoReflectorCandidate bestLeftSide;
		int hitCount = 0;
		float bestWallScore = 0.0;
		for (int i = 0; i < directions.Count(); i++)
		{
			vector direction = directions[i];
			direction[1] = 0.0;
			direction.Normalize();

			TraceParam trace = new TraceParam();
			trace.Start = slapbackProbeOrigin;
			trace.End = slapbackProbeOrigin + direction * maxDistance;
			if (traceExcludeArray)
				trace.ExcludeArray = traceExcludeArray;
			else
				trace.Exclude = owner;

			float hitFraction = ChimeraCharacter.TraceMoveWithoutCharacters(world, trace);
			if (hitFraction < 0.0 || hitFraction >= 1.0)
				continue;

			float hitDistance = maxDistance * hitFraction;
			if (hitDistance < minDistance || hitDistance > maxDistance)
				continue;

			if (IsSelfHierarchyHit(trace.TraceEnt, traceExcludeRoot))
				continue;

			vector hitNormal = trace.TraceNorm;
			if (hitNormal.LengthSq() > 0.0001)
				hitNormal.Normalize();
			else
				hitNormal = direction * -1.0;

			if (Math.AbsFloat(hitNormal[1]) > settings.GetSlapbackWallNormalMaxY())
				continue;

			vector hitPosition = slapbackProbeOrigin + (direction * hitDistance);
			int sector = directionSectors[i];
			bool wallTraceAccepted = IsSlapbackWallTraceAccepted(trace.TraceEnt);
			bool trenchTraceAccepted = IsSlapbackTrenchTraceAccepted(trace.TraceEnt, hitNormal, hitDistance, sector, settings);
			if (!wallTraceAccepted && !trenchTraceAccepted)
				continue;

			float directionBias = directionBiases[i];
			float distanceBias = 1.0 - BS5_EchoMath.Clamp01((hitDistance - minDistance) / BS5_EchoMath.MaxFloat(0.01, maxDistance - minDistance));
			float normalFacing = BS5_EchoMath.Clamp01(vector.Dot(hitNormal, direction * -1.0));
			float verticalBias = 1.0 - BS5_EchoMath.Clamp01(Math.AbsFloat(hitNormal[1]) / settings.GetSlapbackWallNormalMaxY());
			float score = BS5_EchoMath.Clamp01((directionBias * 0.38) + (distanceBias * 0.36) + (normalFacing * 0.18) + (verticalBias * 0.08));
			if (!settings.ShouldAcceptCandidate(score))
				continue;

			hitCount++;

			BS5_EchoReflectorCandidate candidate = BuildSlapbackCandidate(settings, origin, hitPosition, hitNormal, direction, flatRight, hitDistance, maxDistance, score, directionBias, directionRanks[i], BS5_EchoCandidateSourceType.SLAPBACK_WALL);
			if (wallTraceAccepted)
			{
				InsertCandidate(wallCandidates, candidate, 4);
				if (candidate.m_fScore > bestWallScore)
					bestWallScore = candidate.m_fScore;
			}

			if (trenchTraceAccepted && hitDistance <= trenchMaxSideDistance && sector == 1)
			{
				if (!bestRightSide || candidate.m_fScore > bestRightSide.m_fScore)
					bestRightSide = candidate;
			}
			else if (trenchTraceAccepted && hitDistance <= trenchMaxSideDistance && sector == 2)
			{
				if (!bestLeftSide || candidate.m_fScore > bestLeftSide.m_fScore)
					bestLeftSide = candidate;
			}
		}

		int entityHitCount = 0;
		float bestEntityWallScore = 0.0;
		CollectSlapbackEntityCandidates(wallCandidates, settings, world, origin, slapbackProbeOrigin, flatForward, flatRight, traceExcludeRoot, directions, maxDistance, minDistance, maxCandidates, entityHitCount, bestEntityWallScore);
		hitCount += entityHitCount;
		if (bestEntityWallScore > bestWallScore)
			bestWallScore = bestEntityWallScore;

		BS5_EchoReflectorCandidate trenchCandidate = BuildTrenchSlapbackCandidate(settings, origin, flatForward, bestRightSide, bestLeftSide);
		float trenchScore = 0.0;
		if (trenchCandidate)
			trenchScore = trenchCandidate.m_fScore;

		float mergeDistance = settings.GetSlapbackMergeDistanceMeters();
		float mergeDistanceSq = mergeDistance * mergeDistance;
		bool trenchAccepted = false;
		if (trenchCandidate && trenchScore >= settings.GetTrenchSlapbackMinConfidence())
		{
			InsertCandidate(candidates, trenchCandidate, maxCandidates);
			trenchAccepted = true;
		}

		for (int winnerIndex = 0; winnerIndex < wallCandidates.Count() && candidates.Count() < maxCandidates; winnerIndex++)
		{
			BS5_EchoReflectorCandidate winner = wallCandidates[winnerIndex];
			if (!winner)
				continue;

			if (trenchCandidate && Math.AbsFloat(winner.m_fPanBias) > 0.82 && winner.m_fDistance <= trenchMaxSideDistance + 0.75)
				continue;

			bool merged = false;
			for (int existingIndex = 0; existingIndex < candidates.Count(); existingIndex++)
			{
				BS5_EchoReflectorCandidate existing = candidates[existingIndex];
				if (!existing)
					continue;

				if (vector.DistanceSq(existing.m_vPosition, winner.m_vPosition) > mergeDistanceSq)
					continue;

				if (existing.m_eSourceType == BS5_EchoCandidateSourceType.SLAPBACK_TRENCH)
				{
					merged = true;
					break;
				}

				if (winner.m_fScore > existing.m_fScore)
					candidates[existingIndex] = winner;
				merged = true;
				break;
			}

			if (!merged)
				InsertCandidate(candidates, winner, maxCandidates);
		}

		bool anchorFallback = false;
		if (candidates.IsEmpty() && settings.AllowSlapbackAnchorFallback())
			anchorFallback = TryBuildSlapbackAnchorFallback(candidates, settings, result, origin, flatRight, maxCandidates);

		if (result)
		{
			result.m_iSlapbackRayCount = directions.Count();
			result.m_iSlapbackHitCount = hitCount;
			result.m_bSlapbackAnchorFallback = anchorFallback;
			result.m_fSlapbackWallScore = bestWallScore;
			result.m_fSlapbackTrenchScore = trenchScore;
			if (trenchAccepted)
				result.m_sSlapbackMode = "trench";
			else if (anchorFallback)
				result.m_sSlapbackMode = "wall_anchor";
			else if (!candidates.IsEmpty())
				result.m_sSlapbackMode = "wall";
			else
				result.m_sSlapbackMode = "none";
		}

		if (settings && settings.IsDebugEnabled())
		{
			string debugSlapMode = "none";
			if (result)
				debugSlapMode = result.m_sSlapbackMode;
			string slapCollectLog = "slapback collect";
			slapCollectLog += " mode=" + debugSlapMode;
			slapCollectLog += " candidates=" + candidates.Count();
			slapCollectLog += " rays=" + directions.Count();
			slapCollectLog += " hits=" + hitCount;
			slapCollectLog += " entityHits=" + entityHitCount;
			BS5_DebugLog.Line(settings, slapCollectLog);
			string slapScoreLog = "slapback score";
			slapScoreLog += " bestWall=" + bestWallScore;
			slapScoreLog += " trenchScore=" + trenchScore;
			slapScoreLog += " trenchAccepted=" + BS5_DebugLog.BoolText(trenchAccepted);
			slapScoreLog += " anchorFallback=" + BS5_DebugLog.BoolText(anchorFallback);
			BS5_DebugLog.Line(settings, slapScoreLog);
		}
	}

	protected static bool IsSlapbackWallTraceAccepted(IEntity traceEntity)
	{
		if (!traceEntity)
			return false;

		IEntity normalizedEntity = traceEntity.GetRootParent();
		if (!normalizedEntity)
			normalizedEntity = traceEntity;

		float prefabScore = 0.0;
		if (!ResolveSlapbackEntityPrefabScore(normalizedEntity, prefabScore))
			return false;

		return IsSlapbackEntityShapeAccepted(normalizedEntity, prefabScore);
	}

	protected static bool IsSlapbackTrenchTraceAccepted(IEntity traceEntity, vector hitNormal, float hitDistance, int sector, BS5_EchoDriverComponent settings)
	{
		if (sector != 1 && sector != 2)
			return false;

		if (hitDistance > settings.GetTrenchSlapbackMaxSideDistanceMeters())
			return false;

		if (traceEntity)
		{
			ResourceName prefabName = GetSlapbackEntityPrefabNameLower(traceEntity);
			if (prefabName == string.Empty)
				return false;

			if (prefabName.IndexOf("trench") == -1 && prefabName.IndexOf("ditch") == -1 && prefabName.IndexOf("earthwork") == -1)
				return false;
		}

		float normalY = Math.AbsFloat(hitNormal[1]);
		if (normalY < 0.18 || normalY > 0.72)
			return false;

		return true;
	}

	protected static void CollectSlapbackEntityCandidates(array<ref BS5_EchoReflectorCandidate> wallCandidates, BS5_EchoDriverComponent settings, BaseWorld world, vector origin, vector probeOrigin, vector flatForward, vector flatRight, IEntity traceExcludeRoot, array<vector> directions, float maxDistance, float minDistance, int maxCandidates, out int entityHits, out float bestScore)
	{
		entityHits = 0;
		bestScore = 0.0;
		if (!world || !settings)
			return;
		if (g_BS5SlapbackQueryActive)
		{
			BS5_DebugLog.Line(settings, "slapback query reentry drop");
			return;
		}

		g_BS5SlapbackQueryEntities.Clear();
		g_BS5SlapbackQueryExcludeRoot = traceExcludeRoot;
		g_BS5SlapbackQueryActive = true;

		vector queryCenter = origin;
		queryCenter[1] = origin[1] + 1.1;
		world.QueryEntitiesBySphere(queryCenter, maxDistance + 1.5, CollectSlapbackEntityCallback);

		vector bevelMins = "-1.6 -1.1 -1.6";
		vector bevelMaxs = "1.6 2.2 1.6";
		for (int directionIndex = 0; directionIndex < directions.Count(); directionIndex++)
		{
			if (g_BS5SlapbackQueryEntities.Count() >= BS5_SLAPBACK_QUERY_ENTITY_LIMIT)
				break;

			vector direction = directions[directionIndex];
			direction[1] = 0.0;
			if (direction.LengthSq() < 0.0001)
				continue;
			direction.Normalize();
			world.QueryEntitiesByBeveledLine(probeOrigin, probeOrigin + (direction * maxDistance), bevelMins, bevelMaxs, CollectSlapbackEntityCallback);
		}

		for (int entityIndex = 0; entityIndex < g_BS5SlapbackQueryEntities.Count(); entityIndex++)
		{
			IEntity entity = g_BS5SlapbackQueryEntities[entityIndex];
			BS5_EchoReflectorCandidate candidate = BuildSlapbackEntityCandidate(settings, entity, origin, flatForward, flatRight, maxDistance, minDistance);
			if (!candidate)
				continue;

			entityHits++;
			if (candidate.m_fScore > bestScore)
				bestScore = candidate.m_fScore;

			InsertCandidate(wallCandidates, candidate, 4);
		}

		g_BS5SlapbackQueryActive = false;
	}

	protected static bool CollectSlapbackEntityCallback(IEntity entity)
	{
		if (!g_BS5SlapbackQueryActive)
			return false;
		if (!entity)
			return true;

		IEntity normalizedEntity = entity.GetRootParent();
		if (!normalizedEntity)
			normalizedEntity = entity;

		if (g_BS5SlapbackQueryExcludeRoot && IsSelfHierarchyHit(normalizedEntity, g_BS5SlapbackQueryExcludeRoot))
			return true;

		if (ContainsSlapbackEntityReference(g_BS5SlapbackQueryEntities, normalizedEntity))
			return true;

		float prefabScore = 0.0;
		if (!ResolveSlapbackEntityPrefabScore(normalizedEntity, prefabScore))
			return true;
		if (!IsSlapbackEntityShapeAccepted(normalizedEntity, prefabScore))
			return true;

		g_BS5SlapbackQueryEntities.Insert(normalizedEntity);
		if (g_BS5SlapbackQueryEntities.Count() >= BS5_SLAPBACK_QUERY_ENTITY_LIMIT)
			return false;

		return true;
	}

	protected static BS5_EchoReflectorCandidate BuildSlapbackEntityCandidate(BS5_EchoDriverComponent settings, IEntity entity, vector origin, vector flatForward, vector flatRight, float maxDistance, float minDistance)
	{
		if (!entity)
			return null;

		float prefabScore = 0.0;
		if (!ResolveSlapbackEntityPrefabScore(entity, prefabScore))
			return null;
		if (!IsSlapbackEntityShapeAccepted(entity, prefabScore))
			return null;

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

		if (height < 1.15 || footprintMax < 0.8 || footprintArea < 0.65)
			return null;
		if (height > 65.0 || footprintMax > 140.0)
			return null;
		if (height < 1.8 && footprintMin < 0.45)
			return null;

		vector hitPosition = origin;
		bool insideX = origin[0] >= mins[0] && origin[0] <= maxs[0];
		bool insideZ = origin[2] >= mins[2] && origin[2] <= maxs[2];
		if (insideX && insideZ)
		{
			float distanceMinX = Math.AbsFloat(origin[0] - mins[0]);
			float distanceMaxX = Math.AbsFloat(maxs[0] - origin[0]);
			float distanceMinZ = Math.AbsFloat(origin[2] - mins[2]);
			float distanceMaxZ = Math.AbsFloat(maxs[2] - origin[2]);
			float bestFaceDistance = distanceMinX;
			hitPosition[0] = mins[0];
			hitPosition[2] = origin[2];
			if (distanceMaxX < bestFaceDistance)
			{
				bestFaceDistance = distanceMaxX;
				hitPosition[0] = maxs[0];
				hitPosition[2] = origin[2];
			}
			if (distanceMinZ < bestFaceDistance)
			{
				bestFaceDistance = distanceMinZ;
				hitPosition[0] = origin[0];
				hitPosition[2] = mins[2];
			}
			if (distanceMaxZ < bestFaceDistance)
			{
				hitPosition[0] = origin[0];
				hitPosition[2] = maxs[2];
			}
		}
		else
		{
			hitPosition[0] = BS5_EchoMath.Clamp(origin[0], mins[0], maxs[0]);
			hitPosition[2] = BS5_EchoMath.Clamp(origin[2], mins[2], maxs[2]);
		}

		if (height > 1.0)
			hitPosition[1] = BS5_EchoMath.Clamp(origin[1], mins[1] + 0.65, maxs[1] - 0.25);
		else
			hitPosition[1] = origin[1];

		vector direction = hitPosition - origin;
		direction[1] = 0.0;
		float hitDistance = direction.Length();
		if (hitDistance < minDistance || hitDistance > maxDistance)
			return null;
		if (direction.LengthSq() < 0.0001)
			return null;
		direction.Normalize();

		vector hitNormal = origin - hitPosition;
		hitNormal[1] = 0.0;
		if (hitNormal.LengthSq() < 0.0001)
			hitNormal = direction * -1.0;
		else
			hitNormal.Normalize();

		float distanceBias = 1.0 - BS5_EchoMath.Clamp01((hitDistance - minDistance) / BS5_EchoMath.MaxFloat(0.01, maxDistance - minDistance));
		float forwardSupport = vector.Dot(direction, flatForward);
		float sideSupport = Math.AbsFloat(vector.Dot(direction, flatRight));
		float directionScore = BS5_EchoMath.Clamp01(0.50 + (BS5_EchoMath.MaxFloat(0.0, forwardSupport) * 0.22) + (sideSupport * 0.18));
		if (forwardSupport < -0.55)
			directionScore *= 0.72;

		float heightScore = BS5_EchoMath.Clamp01(height / 3.0);
		float areaScore = BS5_EchoMath.Clamp01(footprintArea / 18.0);
		float score = BS5_EchoMath.Clamp01((prefabScore * 0.30) + (distanceBias * 0.36) + (directionScore * 0.18) + (heightScore * 0.10) + (areaScore * 0.06));
		if (!settings.ShouldAcceptCandidate(score))
			return null;

		return BuildSlapbackCandidate(settings, origin, hitPosition, hitNormal, direction, flatRight, hitDistance, maxDistance, score, directionScore, 50, BS5_EchoCandidateSourceType.SLAPBACK_WALL);
	}

	protected static bool ResolveSlapbackEntityPrefabScore(IEntity entity, out float prefabScore)
	{
		prefabScore = 0.0;
		if (!entity)
			return false;

		ResourceName prefabName = GetSlapbackEntityPrefabNameLower(entity);
		if (prefabName == string.Empty)
			return false;
		if (HasSlapbackPrefabHint(prefabName, g_BS5SlapbackRejectPrefabHints))
			return false;
		if (IsSlapbackSoftOrThinPrefab(prefabName))
			return false;

		bool structurePrefab = prefabName.IndexOf("structures") != -1;
		if (!HasSlapbackPrefabHint(prefabName, g_BS5SlapbackHardPrefabHints) && !structurePrefab)
			return false;

		prefabScore = 0.72;
		if (structurePrefab)
			prefabScore = 0.86;
		else if (HasSlapbackPrefabHint(prefabName, g_BS5BuildingPrefabHints))
			prefabScore = 1.0;
		else if (prefabName.IndexOf("vehicle") != -1 || prefabName.IndexOf("car") != -1 || prefabName.IndexOf("truck") != -1 || prefabName.IndexOf("m998") != -1 || prefabName.IndexOf("apc") != -1)
			prefabScore = 0.82;
		else if (prefabName.IndexOf("fence") != -1 || prefabName.IndexOf("wall") != -1 || prefabName.IndexOf("barrier") != -1 || prefabName.IndexOf("container") != -1)
			prefabScore = 0.78;

		return true;
	}

	protected static bool IsSlapbackSoftOrThinPrefab(ResourceName prefabName)
	{
		if (prefabName.IndexOf("powerline") != -1)
			return true;
		if (prefabName.IndexOf("pole") != -1)
			return true;
		if (prefabName.IndexOf("lamp") != -1)
			return true;
		if (prefabName.IndexOf("light") != -1)
			return true;
		if (prefabName.IndexOf("wire") != -1)
			return true;
		if (prefabName.IndexOf("sign") != -1)
			return true;
		if (prefabName.IndexOf("haystackdryer") != -1)
			return true;
		if (prefabName.IndexOf("haystack") != -1)
			return true;
		if (prefabName.IndexOf("greenhouse") != -1)
			return true;

		return false;
	}

	protected static bool IsSlapbackEntityShapeAccepted(IEntity entity, float prefabScore)
	{
		if (!entity)
			return false;

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

		if (height < 1.15 || footprintMax < 0.8 || footprintArea < 0.65)
			return false;
		if (height > 65.0 || footprintMax > 140.0)
			return false;

		bool poleLike = false;
		if (height > footprintMax * 3.2)
		{
			if (footprintMin < 0.85)
				poleLike = true;
		}
		if (poleLike)
			return false;

		bool solidFace = false;
		if (footprintMin >= 0.85 && footprintArea >= 2.2)
			solidFace = true;

		bool longWall = false;
		if (footprintMax >= 2.4 && height >= 1.2)
		{
			if (footprintMin >= 0.16 && height <= 5.8)
				longWall = true;
		}

		bool massiveObject = false;
		if (footprintArea >= 4.5 && footprintMin >= 1.1)
			massiveObject = true;

		if (!solidFace && !longWall && !massiveObject)
			return false;

		if (prefabScore < 0.80 && footprintMin < 0.55)
			return false;

		return true;
	}

	protected static ResourceName GetSlapbackEntityPrefabNameLower(IEntity entity)
	{
		if (!entity)
			return string.Empty;

		IEntity normalizedEntity = entity.GetRootParent();
		if (!normalizedEntity)
			normalizedEntity = entity;

		EntityPrefabData prefabData = normalizedEntity.GetPrefabData();
		if (!prefabData)
			return string.Empty;

		ResourceName prefabName = prefabData.GetPrefabName();
		if (prefabName != string.Empty)
			prefabName.ToLower();

		return prefabName;
	}

	protected static bool ContainsSlapbackEntityReference(array<IEntity> entities, IEntity entity)
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

	protected static bool HasSlapbackPrefabHint(ResourceName prefabName, notnull array<string> hints)
	{
		foreach (string hint : hints)
		{
			if (prefabName.IndexOf(hint) != -1)
				return true;
		}

		return false;
	}

	protected static bool TryBuildSlapbackAnchorFallback(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult result, vector origin, vector flatRight, int maxCandidates)
	{
		if (!result || !result.m_aCandidates || result.m_aCandidates.IsEmpty())
			return false;

		float maxDistance = BS5_EchoMath.MinFloat(settings.GetSlapbackAnchorFallbackMaxDistanceMeters(), settings.GetNearSlapbackRadius());
		if (maxDistance <= 0.0)
			return false;

		BS5_EchoReflectorCandidate bestTail;
		float bestScore = 0.0;
		for (int i = 0; i < result.m_aCandidates.Count(); i++)
		{
			BS5_EchoReflectorCandidate tailCandidate = result.m_aCandidates[i];
			if (!tailCandidate || !tailCandidate.m_bValid)
				continue;

			if (tailCandidate.m_fDistance > maxDistance)
				continue;

			float sourceBias = 0.0;
			if (tailCandidate.m_eSourceType == BS5_EchoCandidateSourceType.SOUNDMAP_CITY)
				sourceBias = 1.0;
			else if (tailCandidate.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT)
				sourceBias = 0.95;
			else if (tailCandidate.m_eSourceType == BS5_EchoCandidateSourceType.OMNI_CONTEXT)
				sourceBias = 0.72;
			else
				continue;

			float distanceBias = 1.0 - BS5_EchoMath.Clamp01(tailCandidate.m_fDistance / maxDistance);
			float score = BS5_EchoMath.Clamp01((tailCandidate.m_fScore * 0.42) + (sourceBias * 0.38) + (distanceBias * 0.20));
			if (score > bestScore)
			{
				bestScore = score;
				bestTail = tailCandidate;
			}
		}

		if (!bestTail)
			return false;

		vector direction = bestTail.m_vPosition - origin;
		direction[1] = 0.0;
		if (direction.LengthSq() < 0.0001)
			return false;
		direction.Normalize();

		vector wallNormal = direction * -1.0;
		vector hitPosition = bestTail.m_vPosition;
		if (Math.AbsFloat(hitPosition[1] - origin[1]) > 2.5)
			hitPosition[1] = origin[1];

		BS5_EchoReflectorCandidate fallback = BuildSlapbackCandidate(settings, origin, hitPosition, wallNormal, direction, flatRight, bestTail.m_fDistance, maxDistance, bestScore, bestTail.m_fDirectionSupport, 60, BS5_EchoCandidateSourceType.SLAPBACK_WALL);
		InsertCandidate(candidates, fallback, maxCandidates);
		return !candidates.IsEmpty();
	}

	protected static BS5_EchoReflectorCandidate BuildSlapbackCandidate(BS5_EchoDriverComponent settings, vector origin, vector hitPosition, vector hitNormal, vector direction, vector flatRight, float hitDistance, float maxDistance, float score, float directionBias, int rank, BS5_EchoCandidateSourceType sourceType)
	{
		BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
		candidate.m_bValid = true;
		candidate.m_iRank = rank;
		candidate.m_fDistance = vector.Distance(origin, hitPosition);
		candidate.m_fScore = BS5_EchoMath.Clamp01(score);
		float slapbackPath = candidate.m_fDistance * settings.GetSlapbackPathLengthScale();
		float slapbackDelay = slapbackPath / settings.GetSoundSpeedMetersPerSecond();
		float maxSlapbackDelay = settings.GetSlapbackWallDelayMaxSeconds();
		if (sourceType == BS5_EchoCandidateSourceType.SLAPBACK_TRENCH)
			maxSlapbackDelay = settings.GetSlapbackTrenchDelayMaxSeconds();
		candidate.m_fDelaySeconds = BS5_EchoMath.Clamp(slapbackDelay, settings.GetSlapbackDelayMinSeconds(), maxSlapbackDelay);
		candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(hitDistance / BS5_EchoMath.MaxFloat(0.01, maxDistance));
		candidate.m_fPanBias = vector.Dot(direction, flatRight);
		candidate.m_fDirectionSupport = directionBias;
		candidate.m_vNormal = hitNormal;
		candidate.m_vPosition = hitPosition + (hitNormal * settings.GetSlapbackWallOffsetMeters());
		candidate.m_eSourceType = sourceType;
		return candidate;
	}

	protected static BS5_EchoReflectorCandidate BuildTrenchSlapbackCandidate(BS5_EchoDriverComponent settings, vector origin, vector flatForward, BS5_EchoReflectorCandidate rightSide, BS5_EchoReflectorCandidate leftSide)
	{
		if (!rightSide || !leftSide)
			return null;

		float maxSideDistance = settings.GetTrenchSlapbackMaxSideDistanceMeters();
		if (rightSide.m_fDistance > maxSideDistance || leftSide.m_fDistance > maxSideDistance)
			return null;

		float averageDistance = (rightSide.m_fDistance + leftSide.m_fDistance) * 0.5;
		float symmetry = 1.0 - BS5_EchoMath.Clamp01(Math.AbsFloat(rightSide.m_fDistance - leftSide.m_fDistance) / BS5_EchoMath.MaxFloat(0.1, averageDistance));
		if (symmetry < 0.65)
			return null;

		if (rightSide.m_fPanBias < 0.35 || leftSide.m_fPanBias > -0.35)
			return null;

		if (averageDistance < 0.75 || averageDistance > 3.8)
			return null;

		float rightNormalY = Math.AbsFloat(rightSide.m_vNormal[1]);
		float leftNormalY = Math.AbsFloat(leftSide.m_vNormal[1]);
		float averageNormalY = (rightNormalY + leftNormalY) * 0.5;
		if (averageNormalY < 0.18 || averageNormalY > 0.72)
			return null;

		float closeScore = 1.0 - BS5_EchoMath.Clamp01((averageDistance - settings.GetSlapbackMinDistanceMeters()) / BS5_EchoMath.MaxFloat(0.1, maxSideDistance - settings.GetSlapbackMinDistanceMeters()));
		float normalScore = 1.0 - BS5_EchoMath.Clamp01(averageNormalY / 0.72);
		float score = BS5_EchoMath.Clamp01((closeScore * 0.42) + (symmetry * 0.40) + (normalScore * 0.18));

		vector trenchPosition = (rightSide.m_vPosition + leftSide.m_vPosition) * 0.5;
		trenchPosition += flatForward * 0.65;
		trenchPosition[1] = origin[1] + 0.12;

		BS5_EchoReflectorCandidate candidate = BuildSlapbackCandidate(settings, origin, trenchPosition, "0 1 0", flatForward, "1 0 0", averageDistance, maxSideDistance, score, 1.0, -20, BS5_EchoCandidateSourceType.SLAPBACK_TRENCH);
		candidate.m_fPanBias = 0.0;
		candidate.m_fDirectionSupport = symmetry;
		return candidate;
	}

	protected static float GetTailBandMinDistance(BS5_EchoDriverComponent settings, int bandIndex)
	{
		switch (bandIndex)
		{
			case 0:
				return settings.GetPrimaryTailDistanceMin();
			case 1:
				return settings.GetSecondaryTailDistanceMin();
		}

		return settings.GetTertiaryTailDistanceMin();
	}

	protected static float GetTailBandMaxDistance(BS5_EchoDriverComponent settings, int bandIndex)
	{
		switch (bandIndex)
		{
			case 0:
				return settings.GetPrimaryTailDistanceMax();
			case 1:
				return settings.GetSecondaryTailDistanceMax();
		}

		return settings.GetTertiaryTailDistanceMax();
	}

	protected static float GetTailBandBaseScore(BS5_EchoDriverComponent settings, int bandIndex)
	{
		switch (bandIndex)
		{
			case 0:
				return settings.GetPrimaryCandidateScore();
			case 1:
				return settings.GetSecondaryCandidateScore();
		}

		return settings.GetTertiaryCandidateScore();
	}

	protected static int GetTailBandDistanceSampleCount(int bandIndex)
	{
		return 2;
	}

	protected static float GetTailBandDistanceSampleAlpha(int sampleIndex, int sampleCount)
	{
		if (sampleCount <= 1)
			return 1.0;

		if (sampleCount == 2)
		{
			if (sampleIndex <= 0)
				return 0.58;

			return 1.0;
		}

		return (float)(sampleIndex + 1) / (float)sampleCount;
	}

	protected static float GetTailTraceOvershootDistance(float bandMinDistance, float bandMaxDistance)
	{
		float bandWidth = bandMaxDistance - bandMinDistance;
		return BS5_EchoMath.Clamp(bandWidth * 0.35, 8.0, 24.0);
	}

	protected static float GetTailMaxAzimuthDegrees(BS5_EchoDriverComponent settings)
	{
		float coneHalf = settings.GetTailForwardConeDegrees() * 0.5;
		float maxAzimuth = coneHalf + settings.GetOutsideConeOffsetDegrees();
		return BS5_EchoMath.Clamp(BS5_EchoMath.MaxFloat(maxAzimuth, 85.0), coneHalf, 120.0);
	}

	protected static float GetTailSectorAngleDegrees(int sectorIndex, int sectorCount, float maxAzimuthDegrees)
	{
		if (sectorCount <= 1)
			return 0.0;

		float alpha = (float)sectorIndex / (float)(sectorCount - 1);
		return Lerp(-maxAzimuthDegrees, maxAzimuthDegrees, alpha);
	}

	protected static float GetTailSectorWeight(BS5_EchoDriverComponent settings, float angleDegrees, float maxAzimuthDegrees)
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

	protected static vector GetFlatSectorDirection(vector flatForward, vector flatRight, float angleDegrees)
	{
		float radians = Math.DEG2RAD * angleDegrees;
		vector direction = (flatForward * Math.Cos(radians)) + (flatRight * Math.Sin(radians));
		direction[1] = 0.0;
		if (direction.LengthSq() < 0.0001)
			return flatForward;

		direction.Normalize();
		return direction;
	}

	protected static float GetTailHeightSample(BS5_EchoDriverComponent settings, int sampleIndex, int sampleCount)
	{
		float lowHeight = settings.GetTailScanHeightLowMeters();
		float highHeight = settings.GetTailScanHeightHighMeters();
		if (sampleCount <= 1)
			return lowHeight;

		if (sampleIndex <= 0)
			return lowHeight;
		if (sampleIndex >= sampleCount - 1)
			return highHeight;

		float alpha = (float)sampleIndex / (float)(sampleCount - 1);
		return Lerp(lowHeight, highHeight, alpha);
	}

	protected static float ResolveTerrainHeight(BaseWorld world, vector position, float referenceHeight)
	{
		vector terrainProbe = position;
		terrainProbe[1] = referenceHeight;
		float heightAboveTerrain = SCR_TerrainHelper.GetHeightAboveTerrain(terrainProbe, world, false, null);
		return terrainProbe[1] - heightAboveTerrain;
	}

	protected static bool IsUsefulTailSurface(BS5_EchoDriverComponent settings, vector hitNormal)
	{
		return Math.AbsFloat(hitNormal[1]) <= settings.GetTailGroundNormalMaxY();
	}

	protected static float GetTailSurfaceScore(BS5_EchoDriverComponent settings, vector hitNormal)
	{
		float normalY = Math.AbsFloat(hitNormal[1]);
		float maxGroundY = settings.GetTailGroundNormalMaxY();
		float surfaceAlpha = 1.0 - BS5_EchoMath.Clamp01(normalY / BS5_EchoMath.MaxFloat(0.01, maxGroundY));
		return BS5_EchoMath.Clamp01(0.35 + (surfaceAlpha * 0.65));
	}

	protected static float GetTailTopSurfaceScore(BS5_EchoDriverComponent settings, vector hitNormal, float localSurfaceHeight)
	{
		float slopeScore = GetTailSurfaceScore(settings, hitNormal);
		float heightScore = BS5_EchoMath.Clamp01(localSurfaceHeight / 4.0);
		return BS5_EchoMath.Clamp01(BS5_EchoMath.MaxFloat(slopeScore, heightScore * 0.85));
	}

	protected static void MergeTailCandidate(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoReflectorCandidate candidate, int maxCandidates, float clusterDistanceSq, float normalMergeDot)
	{
		for (int i = 0; i < candidates.Count(); i++)
		{
			BS5_EchoReflectorCandidate existing = candidates[i];
			if (!existing)
				continue;

			if (vector.DistanceSq(existing.m_vPosition, candidate.m_vPosition) > clusterDistanceSq)
				continue;

			if (vector.Dot(existing.m_vNormal, candidate.m_vNormal) < normalMergeDot)
				continue;

			if (candidate.m_fScore > existing.m_fScore)
				candidates[i] = candidate;
			return;
		}

		InsertCandidate(candidates, candidate, maxCandidates);
	}

	protected static vector GetFlattenedDirection(vector direction, vector fallbackDirection)
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

	protected static float Lerp(float from, float to, float alpha)
	{
		return from + (to - from) * alpha;
	}

	protected static IEntity ResolveTraceExcludeRoot(IEntity owner)
	{
		if (!owner)
			return null;

		IEntity root = owner.GetRootParent();
		if (root)
			return root;

		return owner;
	}

	static bool IsSelfHierarchyHit(IEntity traceEnt, IEntity traceExcludeRoot)
	{
		if (!traceEnt || !traceExcludeRoot)
			return false;

		if (traceEnt == traceExcludeRoot)
			return true;

		IEntity tracedRoot = traceEnt.GetRootParent();
		if (!tracedRoot)
			tracedRoot = traceEnt;

		return tracedRoot == traceExcludeRoot;
	}

	protected static void InsertCandidate(array<ref BS5_EchoReflectorCandidate> candidates, BS5_EchoReflectorCandidate candidate, int maxCandidates)
	{
		if (!candidate || !candidate.m_bValid)
			return;

		if (maxCandidates < 1)
			maxCandidates = 1;

		int insertIndex = candidates.Count();
		for (int i = 0; i < candidates.Count(); i++)
		{
			if (candidate.m_fScore > candidates[i].m_fScore)
			{
				insertIndex = i;
				break;
			}
		}

		if (insertIndex >= maxCandidates && candidates.Count() >= maxCandidates)
			return;

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
}

class BS5_EchoEmissionService
{
	protected static ref array<ResourceName> s_aEmitterResourceNames;
	protected static ref array<ref Resource> s_aEmitterResources;
	protected static ref array<ResourceName> s_aInvalidEmitterResourceNames;
	protected static ref array<ref BS5_ActiveEchoVoice> s_aActiveVoices;
	protected static ref array<ref BS5_PendingEmissionContext> s_aPendingEmissionContexts;
	protected static ref array<ref BS5_PendingEmissionContext> s_aReservedPlaybackVoices;
	protected static int s_iPendingTailVoices;
	protected static int s_iPendingSlapbackVoices;
	protected static int s_iLimiterTicket;
	protected static int s_iTailStartsInGate;
	protected static int s_iSlapbackStartsInGate;
	protected static bool s_bStartGateResetQueued;
	protected static BaseWorld s_pLimiterWorld;
	protected static const int START_GATE_WINDOW_MS = 100;
	protected static const int START_GATE_DEFER_MS = 50;
	protected static const int INVALID_EMITTER_RESOURCE_CACHE_LIMIT = 32;

	protected static void ScrubLimiterContext(BS5_PendingEmissionContext context, bool cancel = false)
	{
		if (!context)
			return;

		if (cancel)
			context.m_bCancelled = true;
		context.m_bLimiterPendingReserved = false;
		context.m_bLimiterPlaybackReserved = false;
		context.m_bLimiterActiveRegistered = false;
		context.m_bDriverBudgetAcquired = false;
	}

	protected static void ResetLimiterState()
	{
		if (s_aPendingEmissionContexts)
		{
			for (int pendingIndex = 0; pendingIndex < s_aPendingEmissionContexts.Count(); pendingIndex++)
				ScrubLimiterContext(s_aPendingEmissionContexts[pendingIndex], true);
		}

		if (s_aReservedPlaybackVoices)
		{
			for (int reservedIndex = 0; reservedIndex < s_aReservedPlaybackVoices.Count(); reservedIndex++)
				ScrubLimiterContext(s_aReservedPlaybackVoices[reservedIndex], true);
		}

		if (s_aActiveVoices)
		{
			for (int activeIndex = 0; activeIndex < s_aActiveVoices.Count(); activeIndex++)
			{
				BS5_ActiveEchoVoice voice = s_aActiveVoices[activeIndex];
				if (voice)
					ScrubLimiterContext(voice.m_Context, true);
			}
		}

		s_iPendingTailVoices = 0;
		s_iPendingSlapbackVoices = 0;
		s_iTailStartsInGate = 0;
		s_iSlapbackStartsInGate = 0;
		s_bStartGateResetQueued = false;
		s_aPendingEmissionContexts = new array<ref BS5_PendingEmissionContext>();
		s_aReservedPlaybackVoices = new array<ref BS5_PendingEmissionContext>();
		s_aActiveVoices = new array<ref BS5_ActiveEchoVoice>();
	}

	protected static void EnsureLimiterWorldState()
	{
		Game game = GetGame();
		BaseWorld currentWorld = null;
		if (game)
			currentWorld = game.GetWorld();

		if (currentWorld == s_pLimiterWorld)
			return;

		ResetLimiterState();
		s_pLimiterWorld = currentWorld;
	}

	protected static bool ContextMatchesOwner(BS5_PendingEmissionContext context, IEntity owner, EntityID ownerId)
	{
		if (!context || !owner)
			return false;

		if (context.m_pOwner == owner)
			return true;

		return context.m_bOwnerIdValid && context.m_OwnerId == ownerId;
	}

	static void CancelOwnerContexts(IEntity owner)
	{
		if (!owner)
			return;

		EnsureActiveVoicePool();
		EntityID ownerId = owner.GetID();

		for (int pendingIndex = s_aPendingEmissionContexts.Count() - 1; pendingIndex >= 0; pendingIndex--)
		{
			BS5_PendingEmissionContext pendingContext = s_aPendingEmissionContexts[pendingIndex];
			if (!ContextMatchesOwner(pendingContext, owner, ownerId))
				continue;

			pendingContext.m_bCancelled = true;
			ReleasePendingVoice(pendingContext);
			ReleasePlaybackVoice(pendingContext);
			ReleaseDriverEmitterBudget(pendingContext);
		}

		for (int reservedIndex = s_aReservedPlaybackVoices.Count() - 1; reservedIndex >= 0; reservedIndex--)
		{
			BS5_PendingEmissionContext reservedContext = s_aReservedPlaybackVoices[reservedIndex];
			if (!ContextMatchesOwner(reservedContext, owner, ownerId))
				continue;

			reservedContext.m_bCancelled = true;
			ReleasePendingVoice(reservedContext);
			ReleasePlaybackVoice(reservedContext);
			ReleaseDriverEmitterBudget(reservedContext);
		}

		for (int activeIndex = s_aActiveVoices.Count() - 1; activeIndex >= 0; activeIndex--)
		{
			BS5_ActiveEchoVoice activeVoice = s_aActiveVoices[activeIndex];
			if (!activeVoice || !ContextMatchesOwner(activeVoice.m_Context, owner, ownerId))
				continue;

			activeVoice.m_Context.m_bCancelled = true;
			ReleaseAndCleanupEmitter(activeVoice.m_Context, activeVoice.m_pEmitter);
		}
	}

	protected static IEntity ResolveContextOwner(BS5_PendingEmissionContext context)
	{
		EnsureLimiterWorldState();
		if (!context)
			return null;
		if (!context.m_bOwnerIdValid)
			return context.m_pOwner;

		Game game = GetGame();
		if (!game)
			return null;

		BaseWorld world = game.GetWorld();
		if (!world)
			return null;

		return world.FindEntityByID(context.m_OwnerId);
	}

	protected static IEntity ResolveContextOwnerInCurrentWorld(BS5_PendingEmissionContext context)
	{
		if (!context)
			return null;
		if (!context.m_bOwnerIdValid)
			return context.m_pOwner;
		if (!s_pLimiterWorld)
			return null;

		return s_pLimiterWorld.FindEntityByID(context.m_OwnerId);
	}

	protected static void PruneOrphanedLimiterContexts()
	{
		if (s_aPendingEmissionContexts)
		{
			for (int pendingIndex = s_aPendingEmissionContexts.Count() - 1; pendingIndex >= 0; pendingIndex--)
			{
				BS5_PendingEmissionContext pendingContext = s_aPendingEmissionContexts[pendingIndex];
				if (!pendingContext)
				{
					s_aPendingEmissionContexts.Remove(pendingIndex);
					continue;
				}

				if (pendingContext.m_bCancelled || pendingContext.m_bEmitterCleanupDone)
				{
					ReleasePendingVoice(pendingContext);
					ReleasePlaybackVoice(pendingContext);
					continue;
				}

				if (pendingContext.m_bOwnerIdValid && !ResolveContextOwnerInCurrentWorld(pendingContext))
				{
					pendingContext.m_bCancelled = true;
					ReleasePendingVoice(pendingContext);
					ReleasePlaybackVoice(pendingContext);
					ReleaseDriverEmitterBudget(pendingContext);
				}
			}
		}

		if (s_aReservedPlaybackVoices)
		{
			for (int reservedIndex = s_aReservedPlaybackVoices.Count() - 1; reservedIndex >= 0; reservedIndex--)
			{
				BS5_PendingEmissionContext reservedContext = s_aReservedPlaybackVoices[reservedIndex];
				if (!reservedContext)
				{
					s_aReservedPlaybackVoices.Remove(reservedIndex);
					continue;
				}

				if (reservedContext.m_bCancelled || reservedContext.m_bEmitterCleanupDone)
				{
					ReleasePlaybackVoice(reservedContext);
					continue;
				}

				if (reservedContext.m_bOwnerIdValid && !ResolveContextOwnerInCurrentWorld(reservedContext))
				{
					reservedContext.m_bCancelled = true;
					ReleasePendingVoice(reservedContext);
					ReleasePlaybackVoice(reservedContext);
					ReleaseDriverEmitterBudget(reservedContext);
				}
			}
		}

		if (s_aActiveVoices)
		{
			for (int activeIndex = s_aActiveVoices.Count() - 1; activeIndex >= 0; activeIndex--)
			{
				BS5_ActiveEchoVoice activeVoice = s_aActiveVoices[activeIndex];
				if (!activeVoice)
				{
					s_aActiveVoices.Remove(activeIndex);
					continue;
				}

				if (!activeVoice.m_Context)
				{
					s_aActiveVoices.Remove(activeIndex);
					continue;
				}

				if (activeVoice.m_Context.m_bCancelled || activeVoice.m_Context.m_bEmitterCleanupDone)
				{
					ReleaseAndCleanupEmitter(activeVoice.m_Context, activeVoice.m_pEmitter);
					continue;
				}

				if (activeVoice.m_Context.m_bOwnerIdValid && !ResolveContextOwnerInCurrentWorld(activeVoice.m_Context))
				{
					activeVoice.m_Context.m_bCancelled = true;
					ReleaseAndCleanupEmitter(activeVoice.m_Context, activeVoice.m_pEmitter);
				}
			}
		}
	}

	static void Emit(BS5_EchoDriverComponent settings, IEntity owner, BS5_EchoAnalysisResult result, bool explosionLike, bool emitTails)
	{
		if (!settings || !owner || !result)
			return;

		bool debugEnabled = settings.IsDebugEnabled();
		float userEchoVolume = BS5_PlayerAudioSettings.GetEchoVolume();
		float userSlapbackVolume = BS5_PlayerAudioSettings.GetSlapbackVolume();
		bool canOutputTail = userEchoVolume > 0.001;
		bool canOutputSlapback = userSlapbackVolume > 0.001;
		if (!canOutputTail && !canOutputSlapback)
		{
			if (debugEnabled)
				BS5_DebugLog.Line(settings, "emit skip muted echoVol=" + userEchoVolume + " slapVol=" + userSlapbackVolume);
			return;
		}

		if (debugEnabled)
		{
			string emitEnterLog = "emit enter";
			emitEnterLog += " explosion=" + BS5_DebugLog.BoolText(explosionLike);
			emitEnterLog += " tailCandidates=" + result.m_aCandidates.Count();
			emitEnterLog += " slapCandidates=" + result.m_aSlapbackCandidates.Count();
			emitEnterLog += " slapMode=" + result.m_sSlapbackMode;
			BS5_DebugLog.Line(settings, emitEnterLog);
		}

		int tailEmitCount = 0;
		if (emitTails)
			tailEmitCount = settings.GetEmissionCount(result, explosionLike);
		int slapbackEmitCount = settings.GetMaxSlapbackEmittersPerShot();
		if (slapbackEmitCount > result.m_aSlapbackCandidates.Count())
			slapbackEmitCount = result.m_aSlapbackCandidates.Count();
		bool emitMaster = emitTails && settings.IsTailsEnabled();
		bool emitSlapback = settings.IsSlapbackEnabled() && !explosionLike;
		string masterEvent = settings.ResolveMasterEventName();
		ResourceName masterProject = settings.ResolveMasterAcp(result.m_bSuppressedShot);
		ResourceName masterEmitterPrefab = settings.ResolveMasterEmitterPrefab(result.m_bSuppressedShot);
		if (explosionLike)
		{
			masterEvent = settings.ResolveExplosionEventName();
			masterProject = settings.ResolveExplosionAcp();
			masterEmitterPrefab = settings.ResolveMasterEmitterPrefab(false);
		}
		string slapbackEvent = settings.ResolveSlapbackEventName();
		ResourceName slapbackEmitterPrefab = settings.ResolveSlapbackEmitterPrefab(BS5_EchoCandidateSourceType.UNKNOWN, result.m_bSuppressedShot);

		bool canEmitMaster = emitMaster && masterEmitterPrefab != string.Empty && masterEvent != string.Empty;
		bool canEmitSlapback = emitSlapback && slapbackEmitterPrefab != string.Empty && slapbackEvent != string.Empty;
		if (debugEnabled)
		{
			string emitConfigLog = "emit config";
			emitConfigLog += " canTail=" + BS5_DebugLog.BoolText(canEmitMaster);
			emitConfigLog += " canSlap=" + BS5_DebugLog.BoolText(canEmitSlapback);
			emitConfigLog += " outputTail=" + BS5_DebugLog.BoolText(canOutputTail);
			emitConfigLog += " outputSlap=" + BS5_DebugLog.BoolText(canOutputSlapback);
			BS5_DebugLog.Line(settings, emitConfigLog);
			BS5_DebugLog.Line(settings, "emit events master=" + masterEvent + " slap=" + slapbackEvent);
		}
		if (!canEmitMaster && !canEmitSlapback)
		{
			if (debugEnabled)
				BS5_DebugLog.Line(settings, "emit skip no valid playback path");
			return;
		}

		for (int i = 0; i < tailEmitCount && i < result.m_aCandidates.Count(); i++)
		{
			BS5_EchoReflectorCandidate candidate = result.m_aCandidates[i];
			if (!candidate || !candidate.m_bValid)
				continue;

			if (settings.IsDebugEnabled())
			{
				string candidateLog = "BS5 candidate[" + i + "]:";
				candidateLog += " source=" + BS5_EchoMath.CandidateSourceName(candidate.m_eSourceType);
				candidateLog += " pos=" + candidate.m_vPosition;
				candidateLog += " normal=" + candidate.m_vNormal;
				candidateLog += " distance=" + candidate.m_fDistance;
				candidateLog += " score=" + candidate.m_fScore;
				candidateLog += " delay=" + candidate.m_fDelaySeconds;
				candidateLog += " slapback=0";
				candidateLog += " physicalScore=" + candidate.m_fPhysicalScore;
				candidateLog += " zonePriority=" + candidate.m_fZonePriority;
				candidateLog += " terrainProfile=" + candidate.m_sTerrainProfile;
				candidateLog += " snapped=" + BS5_DebugLog.BoolText(candidate.m_bTerrainSnapped);
				candidateLog += " pathPlausibility=" + candidate.m_fPathPlausibility;
				candidateLog += " pathOcclusion=" + candidate.m_fPathOcclusion;
				candidateLog += " pathProfile=" + candidate.m_sPathProfile;
				Print(candidateLog);
			}

			if (canEmitMaster && canOutputTail)
				QueueEmission(owner, result, masterProject, masterEvent, candidate, false, explosionLike, settings);
		}

		for (int slapIndex = 0; slapIndex < slapbackEmitCount && slapIndex < result.m_aSlapbackCandidates.Count(); slapIndex++)
		{
			BS5_EchoReflectorCandidate slapCandidate = result.m_aSlapbackCandidates[slapIndex];
			if (!slapCandidate || !slapCandidate.m_bValid)
				continue;

			if (settings.IsDebugEnabled())
			{
				PrintFormat("BS5 slapback[%1]: source=%2 pos=%3 normal=%4 distance=%5 score=%6 delay=%7",
					slapIndex,
					BS5_EchoMath.CandidateSourceName(slapCandidate.m_eSourceType),
					slapCandidate.m_vPosition,
					slapCandidate.m_vNormal,
					slapCandidate.m_fDistance,
					slapCandidate.m_fScore,
					slapCandidate.m_fDelaySeconds);
			}

			if (canEmitSlapback && canOutputSlapback)
			{
				string candidateSlapbackEvent = settings.ResolveSlapbackEventName(slapCandidate.m_eSourceType);
				ResourceName candidateSlapbackProject = settings.ResolveSlapbackAcp(slapCandidate.m_eSourceType, result.m_bSuppressedShot);
				string slapQueueLog = "slapback queue";
				slapQueueLog += " index=" + slapIndex;
				slapQueueLog += " source=" + BS5_EchoMath.CandidateSourceName(slapCandidate.m_eSourceType);
				slapQueueLog += " event=" + candidateSlapbackEvent;
				slapQueueLog += " dist=" + slapCandidate.m_fDistance;
				slapQueueLog += " score=" + slapCandidate.m_fScore;
				slapQueueLog += " pan=" + slapCandidate.m_fPanBias;
				BS5_DebugLog.Line(settings, slapQueueLog);
				QueueEmission(owner, result, candidateSlapbackProject, candidateSlapbackEvent, slapCandidate, true, false, settings);
			}
		}
	}

	static void QueueEmission(IEntity owner, BS5_EchoAnalysisResult result, ResourceName project, string eventName, BS5_EchoReflectorCandidate candidate, bool slapback, bool explosionLike, BS5_EchoDriverComponent settings)
	{
		ResourceName emitterPrefab = settings.ResolveMasterEmitterPrefab(result.m_bSuppressedShot);
		if (slapback)
			emitterPrefab = settings.ResolveSlapbackEmitterPrefab(candidate.m_eSourceType, result.m_bSuppressedShot);

		if (eventName == string.Empty || emitterPrefab == string.Empty)
		{
			BS5_DebugLog.Line(settings, "queue skip invalid prefab/event event=" + eventName + " slapback=" + BS5_DebugLog.BoolText(slapback));
			return;
		}

		BS5_PendingEmissionContext context = new BS5_PendingEmissionContext();
		context.m_pOwner = owner;
		if (owner)
		{
			context.m_OwnerId = owner.GetID();
			context.m_bOwnerIdValid = true;
		}
		context.m_sProject = project;
		context.m_sEventName = eventName;
		context.m_sEmitterPrefab = emitterPrefab;
		context.m_vEmitPosition = candidate.m_vPosition;
		context.m_fDistanceGain = 1.0;
		if (slapback)
			context.m_fDistanceGain = ComputeSlapbackDistanceGain(settings, candidate.m_fDistance);
		else
			context.m_fDistanceGain = ComputeDistanceGain(settings, candidate.m_fDistance);
		context.m_fUserEchoVolume = BS5_PlayerAudioSettings.GetEchoVolume();
		context.m_fUserSlapbackVolume = BS5_PlayerAudioSettings.GetSlapbackVolume();
		context.m_fIntensity = BS5_EchoMath.Clamp01(result.m_fIntensity * candidate.m_fScore * context.m_fDistanceGain);
		context.m_fDelaySeconds = candidate.m_fDelaySeconds;
		context.m_bSlapback = slapback;
		context.m_bExplosion = explosionLike;
		context.m_bSuppressed = result.m_bSuppressedShot && !explosionLike;
		context.m_bAllowDirectProjectPlayback = !settings.IsPlaybackLimiterEnabled();
		context.m_iCandidateRank = candidate.m_iRank;
		context.m_fDistanceNorm = candidate.m_fDistanceNorm;
		context.m_fPanBias = candidate.m_fPanBias;
		context.m_fDirectionSupport = candidate.m_fDirectionSupport;
		context.m_fCandidateDistance = candidate.m_fDistance;
		context.m_eSourceType = candidate.m_eSourceType;
		context.m_Result = result;
		context.m_fEmitterLifetimeSeconds = settings.GetEmitterLifetimeSeconds(slapback);
		context.m_fReverbSend = BS5_EchoMath.Clamp01(((result.m_fIndoorScore * 0.45) + (result.m_fTrenchScore * 0.35) + (result.m_fHardSurfaceScore * 0.20)) * settings.GetReverbSendScale());
		context.m_fTailWidth = BS5_EchoMath.Clamp01(((result.m_fOpenScore * 0.65) + ((1.0 - result.m_fIndoorScore) * 0.35)) * settings.GetTailWidthScale());
		context.m_fTailBrightness = BS5_EchoMath.Clamp01(((result.m_fOpenScore * 0.55) + (result.m_fHardSurfaceScore * 0.45)) * settings.GetTailBrightnessScale());
		context.m_fSurfaceHardness = BS5_EchoMath.Clamp01(result.m_fHardSurfaceScore * settings.GetSurfaceHardnessScale());
		context.m_iEstimatedNativeSources = ResolveEstimatedNativeSources(context, settings);

		if (context.m_bSuppressed)
			context.m_fIntensity = BS5_EchoMath.Clamp01(context.m_fIntensity * settings.GetSuppressedIntensityMultiplier());

		if (slapback)
		{
			context.m_fIntensity = BS5_EchoMath.Clamp01(context.m_fIntensity * settings.GetSlapbackIntensityScale());
			context.m_fTailWidth *= 0.25;
			context.m_fReverbSend *= 0.35;
		}

		float userOutputVolume = context.m_fUserEchoVolume;
		if (slapback)
			userOutputVolume = context.m_fUserSlapbackVolume;

		context.m_fIntensity = BS5_EchoMath.Clamp01(context.m_fIntensity * userOutputVolume);
		context.m_fEmitterLifetimeSeconds = ComputeManagedEmitterLifetime(settings, context);

		if (settings && settings.IsDebugEnabled())
		{
			string queueLog = "queue";
			queueLog += " slapback=" + BS5_DebugLog.BoolText(slapback);
			queueLog += " event=" + eventName;
			queueLog += " delay=" + context.m_fDelaySeconds;
			queueLog += " intensity=" + context.m_fIntensity;
			queueLog += " gain=" + context.m_fDistanceGain;
			queueLog += " userVol=" + userOutputVolume;
			queueLog += " weight=" + context.m_iEstimatedNativeSources;
			queueLog += " source=" + BS5_EchoMath.CandidateSourceName(context.m_eSourceType);
			BS5_DebugLog.Line(settings, queueLog);
			PrintFormat("BS5 queue: project=%1 event=%2 pos=%3 delay=%4 intensity=%5 distanceGain=%6 userVolume=%7",
				project,
				eventName,
				context.m_vEmitPosition,
				context.m_fDelaySeconds,
				context.m_fIntensity,
				context.m_fDistanceGain,
				userOutputVolume);
			PrintFormat("BS5 queue flags: suppressed=%1 slapback=%2 explosion=%3 weight=%4",
				context.m_bSuppressed,
				slapback,
				explosionLike,
				context.m_iEstimatedNativeSources);
		}

		if (context.m_fIntensity <= 0.001)
		{
			BS5_DebugLog.Line(settings, "queue skip inaudible event=" + eventName + " slapback=" + BS5_DebugLog.BoolText(slapback));
			return;
		}

		if (!ReservePendingVoice(context, settings))
			return;

		if (context.m_fDelaySeconds > 0.001)
		{
			int delayMs = (int)(context.m_fDelaySeconds * 1000.0);
			ScriptCallQueue callQueue = GetGame().GetCallqueue();
			if (callQueue)
				callQueue.CallLater(BS5_EchoEmissionService.EmitPending, delayMs, false, context);
			else
				EmitPending(context);
		}
		else
		{
			EmitPending(context);
		}
	}

	static void EmitPending(BS5_PendingEmissionContext context)
	{
		ReleasePendingVoice(context);
		if (!context || context.m_bEmitterCleanupDone || context.m_bCancelled)
			return;
		IEntity owner = ResolveContextOwner(context);
		if (!owner)
			return;
		context.m_pOwner = owner;
		BS5_EchoDriverComponent driver = BS5_EchoDriverComponent.Cast(owner.FindComponent(BS5_EchoDriverComponent));
		bool debugEnabled = false;
		if (driver)
			debugEnabled = driver.IsDebugEnabled();

		if (!TryEnterStartGate(context, driver, debugEnabled))
			return;

		if (!TryAdmitPlaybackVoice(context, driver, debugEnabled))
			return;

		string pendingLog = "emit pending";
		pendingLog += " slapback=" + BS5_DebugLog.BoolText(context.m_bSlapback);
		pendingLog += " event=" + context.m_sEventName;
		pendingLog += " weight=" + GetContextVoiceWeight(context);
		pendingLog += " lifetime=" + context.m_fEmitterLifetimeSeconds;
		BS5_DebugLog.LineEnabled(debugEnabled, pendingLog);
		if (!context.m_bSlapback && TryPlayManagedAudioSource(context, driver, debugEnabled))
		{
			RegisterActiveVoice(context, null, driver, debugEnabled);
			ScriptCallQueue managedCallQueue = GetGame().GetCallqueue();
			if (managedCallQueue)
				managedCallQueue.CallLater(BS5_EchoEmissionService.ReleaseAndCleanupEmitter, (int)(context.m_fEmitterLifetimeSeconds * 1000.0), false, context, null);
			else
				BS5_EchoEmissionService.ReleaseAndCleanupEmitter(context, null);
			return;
		}
		else if (context.m_bSlapback)
		{
			BS5_DebugLog.LineEnabled(debugEnabled, "emit pending slapback uses prefab emitter path");
		}

		if (driver && !driver.TryAcquireActiveEmitterBudget(context.m_bSlapback))
		{
			ReleasePlaybackVoice(context);
			BS5_DebugLog.LineEnabled(debugEnabled, "emit pending skip active emitter budget slapback=" + BS5_DebugLog.BoolText(context.m_bSlapback));
			return;
		}
		if (driver)
			context.m_bDriverBudgetAcquired = true;

		IEntity emitterEntity = BS5_EchoEmissionService.SpawnEmitterEntity(context, debugEnabled);
		if (!emitterEntity)
		{
			ReleasePlaybackVoice(context);
			ReleaseDriverEmitterBudget(context);
			BS5_DebugLog.LineEnabled(debugEnabled, "emit pending abort fallback emitter spawn failed prefab=" + context.m_sEmitterPrefab);
			return;
		}

		emitterEntity.SetOrigin(context.m_vEmitPosition);

		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		if (context.m_bSlapback)
			EmitOnEmitter(context, emitterEntity);
		else if (callQueue)
			callQueue.CallLater(BS5_EchoEmissionService.EmitOnEmitter, 50, false, context, emitterEntity);
		else
			EmitOnEmitter(context, emitterEntity);
	}

	static void EmitOnEmitter(BS5_PendingEmissionContext context, IEntity emitterEntity)
	{
		if (!context || !emitterEntity || context.m_bEmitterCleanupDone || context.m_bCancelled)
			return;

		IEntity owner = ResolveContextOwner(context);
		if (!owner)
		{
			BS5_EchoEmissionService.ReleaseAndCleanupEmitter(context, emitterEntity);
			return;
		}
		context.m_pOwner = owner;
		BS5_EchoDriverComponent driver = BS5_EchoDriverComponent.Cast(owner.FindComponent(BS5_EchoDriverComponent));
		bool debugEnabled = false;
		if (driver)
			debugEnabled = driver.IsDebugEnabled();

		BS5_SpatialSoundEmitterComponent emitterComponent = BS5_SpatialSoundEmitterComponent.Cast(emitterEntity.FindComponent(BS5_SpatialSoundEmitterComponent));
		if (!emitterComponent || !emitterComponent.IsReady())
		{
			if (debugEnabled)
				PrintFormat("BS5 emit pending: emitter helper missing retry=%1 helper=%2 pos=%3", context.m_iEmitterRetryCount, emitterComponent != null, emitterEntity.GetOrigin());

			if (context.m_iEmitterRetryCount < 2)
			{
				context.m_iEmitterRetryCount++;
				ScriptCallQueue retryQueue = GetGame().GetCallqueue();
				if (retryQueue)
					retryQueue.CallLater(BS5_EchoEmissionService.EmitOnEmitter, 50, false, context, emitterEntity);
				else
					EmitOnEmitter(context, emitterEntity);
				return;
			}

			if (debugEnabled)
				Print("BS5 emit pending aborted: emitter helper missing");
			BS5_EchoEmissionService.ReleaseAndCleanupEmitter(context, emitterEntity);
			return;
		}

		if (debugEnabled)
		{
			PrintFormat("BS5 emit pending: owner=%1 emitter=%2 project=%3 event=%4 pos=%5 delay=%6 intensity=%7 signals=%8",
				context.m_pOwner,
				emitterEntity,
				context.m_sProject,
				context.m_sEventName,
				context.m_vEmitPosition,
				context.m_fDelaySeconds,
				context.m_fIntensity,
				true);
		}

		AudioHandle handle = emitterComponent.Play(context, debugEnabled);
		bool played = handle != -1;
		if (!played && debugEnabled)
			Print("BS5 emit pending: emitter helper failed to play");

		if (played)
			RegisterActiveVoice(context, emitterEntity, driver, debugEnabled);

		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		if (played && callQueue)
			callQueue.CallLater(BS5_EchoEmissionService.ReleaseAndCleanupEmitter, (int)(context.m_fEmitterLifetimeSeconds * 1000.0), false, context, emitterEntity);
		else
			BS5_EchoEmissionService.ReleaseAndCleanupEmitter(context, emitterEntity);
	}

	static float GetTailVoicePressure(BS5_EchoDriverComponent settings)
	{
		if (!settings || !settings.IsPlaybackLimiterEnabled())
			return 0.0;

		EnsureActiveVoicePool();
		PruneInvalidActiveVoices();

		int maxVoices = settings.GetGlobalMaxTailVoices();
		if (maxVoices <= 0)
			return 0.0;

		float load = CountBusyVoices(false) + (s_iPendingTailVoices * 0.35);
		return BS5_EchoMath.Clamp01(load / maxVoices);
	}

	protected static bool ReservePendingVoice(BS5_PendingEmissionContext context, BS5_EchoDriverComponent settings)
	{
		if (!context || !settings || !settings.IsPlaybackLimiterEnabled())
			return true;

		EnsureActiveVoicePool();
		int voiceWeight = GetContextVoiceWeight(context);
		if (context.m_bSlapback)
		{
			int maxPendingSlapback = settings.GetGlobalMaxSlapbackVoices();
			if (s_iPendingSlapbackVoices + voiceWeight > maxPendingSlapback)
			{
				BS5_DebugLog.Line(settings, "queue skip slapback pending cap pending=" + s_iPendingSlapbackVoices + " weight=" + voiceWeight + " max=" + maxPendingSlapback);
				return false;
			}

			s_iPendingSlapbackVoices += voiceWeight;
			context.m_bLimiterPendingReserved = true;
			s_aPendingEmissionContexts.Insert(context);
			return true;
		}

		int maxPendingTail = settings.GetMaxPendingTailVoices();
		if (s_iPendingTailVoices + voiceWeight > maxPendingTail)
		{
			string pendingCapLog = "queue skip tail pending cap";
			pendingCapLog += " pending=" + s_iPendingTailVoices;
			pendingCapLog += " weight=" + voiceWeight;
			pendingCapLog += " max=" + maxPendingTail;
			pendingCapLog += " source=" + BS5_EchoMath.CandidateSourceName(context.m_eSourceType);
			BS5_DebugLog.Line(settings, pendingCapLog);
			BS5_DebugLog.Line(settings, "queue skip tail detail dist=" + context.m_fCandidateDistance + " intensity=" + context.m_fIntensity);
			return false;
		}

		s_iPendingTailVoices += voiceWeight;
		context.m_bLimiterPendingReserved = true;
		s_aPendingEmissionContexts.Insert(context);
		return true;
	}

	protected static void ReleasePendingVoice(BS5_PendingEmissionContext context)
	{
		if (!context || !context.m_bLimiterPendingReserved)
			return;

		if (context.m_bSlapback)
		{
			s_iPendingSlapbackVoices -= GetContextVoiceWeight(context);
			if (s_iPendingSlapbackVoices < 0)
				s_iPendingSlapbackVoices = 0;
		}
		else
		{
			s_iPendingTailVoices -= GetContextVoiceWeight(context);
			if (s_iPendingTailVoices < 0)
				s_iPendingTailVoices = 0;
		}

		if (s_aPendingEmissionContexts)
		{
			for (int i = s_aPendingEmissionContexts.Count() - 1; i >= 0; i--)
			{
				BS5_PendingEmissionContext pending = s_aPendingEmissionContexts[i];
				if (!pending || pending == context)
					s_aPendingEmissionContexts.Remove(i);
			}
		}

		context.m_bLimiterPendingReserved = false;
	}

	protected static void ReservePlaybackVoice(BS5_PendingEmissionContext context)
	{
		if (!context || context.m_bLimiterPlaybackReserved)
			return;

		EnsureActiveVoicePool();
		context.m_bLimiterPlaybackReserved = true;
		s_aReservedPlaybackVoices.Insert(context);
	}

	protected static void ReleasePlaybackVoice(BS5_PendingEmissionContext context)
	{
		if (!context || !context.m_bLimiterPlaybackReserved || !s_aReservedPlaybackVoices)
			return;

		for (int i = s_aReservedPlaybackVoices.Count() - 1; i >= 0; i--)
		{
			BS5_PendingEmissionContext reserved = s_aReservedPlaybackVoices[i];
			if (!reserved || reserved == context)
				s_aReservedPlaybackVoices.Remove(i);
		}

		context.m_bLimiterPlaybackReserved = false;
	}

	protected static bool TryAdmitPlaybackVoice(BS5_PendingEmissionContext context, BS5_EchoDriverComponent driver, bool debugEnabled)
	{
		if (!context || !driver || !driver.IsPlaybackLimiterEnabled())
			return true;

		EnsureActiveVoicePool();
		PruneInvalidActiveVoices();
		int voiceWeight = GetContextVoiceWeight(context);

		if (!context.m_bSlapback)
		{
			int ownerLimit = driver.GetMaxTailVoicesPerOwner();
			int ownerActive = CountOwnerBusyVoices(context.m_pOwner, false);
			if (ownerLimit > 0 && ownerActive + voiceWeight > ownerLimit)
			{
				if (!StealPlaybackVoice(context, driver, true, debugEnabled))
				{
					string ownerCapLog = "emit pending skip owner cap";
					ownerCapLog += " active=" + ownerActive;
					ownerCapLog += " weight=" + voiceWeight;
					ownerCapLog += " max=" + ownerLimit;
					ownerCapLog += " source=" + BS5_EchoMath.CandidateSourceName(context.m_eSourceType);
					BS5_DebugLog.LineEnabled(debugEnabled, ownerCapLog);
					BS5_DebugLog.LineEnabled(debugEnabled, "emit pending owner detail dist=" + context.m_fCandidateDistance + " intensity=" + context.m_fIntensity);
					return false;
				}
			}
		}

		int globalMax = driver.GetGlobalMaxTailVoices();
		if (context.m_bSlapback)
			globalMax = driver.GetGlobalMaxSlapbackVoices();

		int globalActive = CountBusyVoices(context.m_bSlapback);
		if (globalMax > 0 && globalActive + voiceWeight > globalMax)
		{
			if (!StealPlaybackVoice(context, driver, false, debugEnabled))
			{
				string globalCapLog = "emit pending skip global cap";
				globalCapLog += " slapback=" + BS5_DebugLog.BoolText(context.m_bSlapback);
				globalCapLog += " active=" + globalActive;
				globalCapLog += " weight=" + voiceWeight;
				globalCapLog += " max=" + globalMax;
				globalCapLog += " source=" + BS5_EchoMath.CandidateSourceName(context.m_eSourceType);
				BS5_DebugLog.LineEnabled(debugEnabled, globalCapLog);
				BS5_DebugLog.LineEnabled(debugEnabled, "emit pending global detail dist=" + context.m_fCandidateDistance + " intensity=" + context.m_fIntensity);
				return false;
			}
		}

		ReservePlaybackVoice(context);
		return true;
	}

	protected static bool StealPlaybackVoice(BS5_PendingEmissionContext incoming, BS5_EchoDriverComponent driver, bool sameOwnerOnly, bool debugEnabled)
	{
		if (!incoming || !driver)
			return false;

		EnsureActiveVoicePool();

		float incomingPriority = ComputeVoicePriority(incoming);
		float pressure = GetTailVoicePressure(driver);
		bool critical = pressure >= driver.GetLimiterCriticalPressureThreshold();
		int victimIndex = -1;
		float victimScore = 9999.0;

		for (int i = 0; i < s_aActiveVoices.Count(); i++)
		{
			BS5_ActiveEchoVoice voice = s_aActiveVoices[i];
			if (!IsActiveVoiceValid(voice))
				continue;

			if (voice.m_bSlapback != incoming.m_bSlapback)
				continue;

			if (sameOwnerOnly && voice.m_pOwner != incoming.m_pOwner)
				continue;

			if (!CanStealVoice(voice.m_Context, incoming, critical))
				continue;

			float killScore = ComputeVoiceKillScore(voice);
			if (killScore < victimScore)
			{
				victimScore = killScore;
				victimIndex = i;
			}
		}

		if (victimIndex < 0)
			return false;

		BS5_ActiveEchoVoice victim = s_aActiveVoices[victimIndex];
		if (!victim || !victim.m_Context)
			return false;

		bool allowSteal = critical || incomingPriority >= (victim.m_fPriority + 0.08);
		if (sameOwnerOnly && !allowSteal)
			allowSteal = incomingPriority >= (victim.m_fPriority - 0.22) && IsLowValueTailVoice(victim.m_Context);
		if (!allowSteal)
			return false;

		if (debugEnabled)
		{
			string stealLog = "limiter steal";
			stealLog += " sameOwner=" + BS5_DebugLog.BoolText(sameOwnerOnly);
			stealLog += " critical=" + BS5_DebugLog.BoolText(critical);
			stealLog += " incomingPriority=" + incomingPriority;
			stealLog += " victimPriority=" + victim.m_fPriority;
			stealLog += " victimAge=" + (s_iLimiterTicket - victim.m_iTicket);
			BS5_DebugLog.LineEnabled(debugEnabled, stealLog);
			BS5_DebugLog.LineEnabled(debugEnabled, "limiter steal victim dist=" + victim.m_Context.m_fCandidateDistance + " intensity=" + victim.m_Context.m_fIntensity);
			PrintFormat("BS5 limiter steal: sameOwner=%1 critical=%2 incomingPriority=%3 victimPriority=%4 victimAge=%5 victimDistance=%6 victimIntensity=%7",
				sameOwnerOnly,
				critical,
				incomingPriority,
				victim.m_fPriority,
				s_iLimiterTicket - victim.m_iTicket,
				victim.m_Context.m_fCandidateDistance,
				victim.m_Context.m_fIntensity);
		}

		BS5_EchoEmissionService.ReleaseAndCleanupEmitter(victim.m_Context, victim.m_pEmitter);
		return true;
	}

	protected static bool CanStealVoice(BS5_PendingEmissionContext victim, BS5_PendingEmissionContext incoming, bool critical)
	{
		if (!victim || !incoming)
			return false;

		if (victim.m_bExplosion && !incoming.m_bExplosion)
			return false;

		if (critical)
			return true;

		if (IsProtectedTailVoice(victim))
			return false;

		return true;
	}

	protected static void RegisterActiveVoice(BS5_PendingEmissionContext context, IEntity emitterEntity, BS5_EchoDriverComponent driver, bool debugEnabled)
	{
		if (!context || !driver || !driver.IsPlaybackLimiterEnabled())
			return;

		EnsureActiveVoicePool();
		ReleasePlaybackVoice(context);
		if (context.m_bLimiterActiveRegistered)
			return;

		BS5_ActiveEchoVoice voice = new BS5_ActiveEchoVoice();
		voice.m_pOwner = context.m_pOwner;
		voice.m_pEmitter = emitterEntity;
		voice.m_Context = context;
		voice.m_bSlapback = context.m_bSlapback;
		s_iLimiterTicket++;
		voice.m_iTicket = s_iLimiterTicket;
		voice.m_fPriority = ComputeVoicePriority(context);

		context.m_bLimiterActiveRegistered = true;
		context.m_iLimiterTicket = voice.m_iTicket;
		context.m_fLimiterPriority = voice.m_fPriority;
		s_aActiveVoices.Insert(voice);

		string registerLog = "limiter register";
		registerLog += " slapback=" + BS5_DebugLog.BoolText(context.m_bSlapback);
		registerLog += " activeTail=" + CountActiveVoices(false);
		registerLog += " activeSlap=" + CountActiveVoices(true);
		registerLog += " pendingTail=" + s_iPendingTailVoices;
		registerLog += " pendingSlap=" + s_iPendingSlapbackVoices;
		registerLog += " weight=" + GetContextVoiceWeight(context);
		BS5_DebugLog.LineEnabled(debugEnabled, registerLog);
		string registerDetailLog = "limiter detail";
		registerDetailLog += " priority=" + voice.m_fPriority;
		registerDetailLog += " source=" + BS5_EchoMath.CandidateSourceName(context.m_eSourceType);
		registerDetailLog += " dist=" + context.m_fCandidateDistance;
		registerDetailLog += " lifetime=" + context.m_fEmitterLifetimeSeconds;
		registerDetailLog += " soundManager=" + BS5_DebugLog.BoolText(context.m_bPlayedViaSoundManager);
		BS5_DebugLog.LineEnabled(debugEnabled, registerDetailLog);
	}

	protected static void UnregisterActiveVoice(BS5_PendingEmissionContext context, IEntity emitterEntity)
	{
		if (!s_aActiveVoices)
			return;

		for (int i = s_aActiveVoices.Count() - 1; i >= 0; i--)
		{
			BS5_ActiveEchoVoice voice = s_aActiveVoices[i];
			if (!voice)
			{
				s_aActiveVoices.Remove(i);
				continue;
			}

			bool sameContext = context && voice.m_Context == context;
			bool sameEmitter = emitterEntity && voice.m_pEmitter == emitterEntity;
			if (!sameContext && !sameEmitter)
				continue;

			if (voice.m_Context)
				voice.m_Context.m_bLimiterActiveRegistered = false;
			s_aActiveVoices.Remove(i);
		}
	}

	protected static int CountActiveVoices(bool slapback)
	{
		EnsureActiveVoicePool();
		int count = 0;
		for (int i = 0; i < s_aActiveVoices.Count(); i++)
		{
			BS5_ActiveEchoVoice voice = s_aActiveVoices[i];
			if (IsActiveVoiceValid(voice) && voice.m_bSlapback == slapback)
				count += GetContextVoiceWeight(voice.m_Context);
		}

		return count;
	}

	protected static int CountBusyVoices(bool slapback)
	{
		EnsureActiveVoicePool();
		int count = CountActiveVoices(slapback);
		for (int i = s_aReservedPlaybackVoices.Count() - 1; i >= 0; i--)
		{
			BS5_PendingEmissionContext reserved = s_aReservedPlaybackVoices[i];
			if (!reserved || reserved.m_bEmitterCleanupDone)
			{
				s_aReservedPlaybackVoices.Remove(i);
				continue;
			}

			if (reserved.m_bSlapback == slapback)
				count += GetContextVoiceWeight(reserved);
		}

		return count;
	}

	protected static int CountOwnerBusyVoices(IEntity owner, bool slapback)
	{
		if (!owner)
			return 0;

		EnsureActiveVoicePool();
		int count = 0;
		for (int i = 0; i < s_aActiveVoices.Count(); i++)
		{
			BS5_ActiveEchoVoice voice = s_aActiveVoices[i];
			if (IsActiveVoiceValid(voice) && voice.m_pOwner == owner && voice.m_bSlapback == slapback)
				count += GetContextVoiceWeight(voice.m_Context);
		}

		for (int reservedIndex = s_aReservedPlaybackVoices.Count() - 1; reservedIndex >= 0; reservedIndex--)
		{
			BS5_PendingEmissionContext reserved = s_aReservedPlaybackVoices[reservedIndex];
			if (!reserved || reserved.m_bEmitterCleanupDone)
			{
				s_aReservedPlaybackVoices.Remove(reservedIndex);
				continue;
			}

			if (reserved.m_pOwner == owner && reserved.m_bSlapback == slapback)
				count += GetContextVoiceWeight(reserved);
		}

		return count;
	}

	protected static void PruneInvalidActiveVoices()
	{
		if (!s_aActiveVoices)
			return;

		for (int i = s_aActiveVoices.Count() - 1; i >= 0; i--)
		{
			BS5_ActiveEchoVoice voice = s_aActiveVoices[i];
			if (IsActiveVoiceValid(voice))
				continue;

			if (voice && voice.m_Context)
			{
				voice.m_Context.m_bLimiterActiveRegistered = false;
				ReleaseDriverEmitterBudget(voice.m_Context);
			}
			s_aActiveVoices.Remove(i);
		}
	}

	protected static float ComputeVoicePriority(BS5_PendingEmissionContext context)
	{
		if (!context)
			return 0.0;

		float priority = context.m_fIntensity * 1.4;
		priority += (1.0 - BS5_EchoMath.Clamp01(context.m_fDistanceNorm)) * 0.25;

		if (context.m_bSlapback)
			priority += 0.65;
		if (context.m_bExplosion)
			priority += 0.55;
		if (context.m_bSuppressed)
			priority -= 0.04;

		if (context.m_eSourceType == BS5_EchoCandidateSourceType.SOUNDMAP_CITY || context.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT)
			priority += 0.25;
		else if (context.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_RIDGE || context.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_HIT || context.m_eSourceType == BS5_EchoCandidateSourceType.TERRAIN_PRIMARY)
			priority += 0.16;
		else if (context.m_eSourceType == BS5_EchoCandidateSourceType.SOUNDMAP_FOREST)
			priority += 0.09;

		if (context.m_iCandidateRank <= 0)
			priority += 0.20;
		else
			priority -= BS5_EchoMath.MinFloat(0.25, context.m_iCandidateRank * 0.04);

		if (context.m_fCandidateDistance > 650.0)
			priority -= 0.24;
		else if (context.m_fCandidateDistance > 500.0)
			priority -= 0.14;

		return priority;
	}

	protected static float ComputeVoiceKillScore(BS5_ActiveEchoVoice voice)
	{
		if (!voice || !voice.m_Context)
			return -999.0;

		float score = voice.m_fPriority;
		int ageTickets = s_iLimiterTicket - voice.m_iTicket;
		if (ageTickets > 0)
			score -= BS5_EchoMath.MinFloat(0.35, ageTickets * 0.015);

		if (voice.m_Context.m_fCandidateDistance > 650.0)
			score -= 0.22;
		else if (voice.m_Context.m_fCandidateDistance > 500.0)
			score -= 0.14;

		if (voice.m_Context.m_fIntensity < 0.20)
			score -= 0.20;
		else if (voice.m_Context.m_fIntensity < 0.28)
			score -= 0.10;

		return score;
	}

	protected static bool IsProtectedTailVoice(BS5_PendingEmissionContext context)
	{
		if (!context || context.m_bSlapback || context.m_bExplosion)
			return false;

		if (context.m_iCandidateRank <= 0 && context.m_fCandidateDistance < 260.0 && context.m_fIntensity > 0.28)
			return true;

		if (context.m_iCandidateRank <= 1 && context.m_fCandidateDistance < 380.0 && context.m_fIntensity > 0.34)
			return true;

		if ((context.m_eSourceType == BS5_EchoCandidateSourceType.SOUNDMAP_CITY || context.m_eSourceType == BS5_EchoCandidateSourceType.FACADE_HIT) && context.m_fCandidateDistance < 420.0 && context.m_fIntensity > 0.24)
			return true;

		return false;
	}

	protected static bool IsLowValueTailVoice(BS5_PendingEmissionContext context)
	{
		if (!context || context.m_bSlapback || context.m_bExplosion)
			return false;

		if (context.m_iCandidateRank > 2)
			return true;

		if (context.m_fCandidateDistance > 600.0 && context.m_fIntensity < 0.28)
			return true;

		if (context.m_fIntensity < 0.18)
			return true;

		return false;
	}

	protected static float ComputeManagedEmitterLifetime(BS5_EchoDriverComponent settings, BS5_PendingEmissionContext context)
	{
		if (!settings || !context)
			return 1.0;

		float lifetime = settings.GetEmitterLifetimeSeconds(context.m_bSlapback);
		if (context.m_bSlapback || context.m_bExplosion || !settings.IsPlaybackLimiterEnabled())
			return lifetime;

		float minLifetime = settings.GetTailEmitterMinManagedLifetimeSeconds();
		float pressureLifetime = settings.GetTailEmitterHighPressureLifetimeSeconds();
		float pressure = GetTailVoicePressure(settings);
		bool highPressure = pressure >= settings.GetLimiterHighPressureThreshold();
		bool criticalPressure = pressure >= settings.GetLimiterCriticalPressureThreshold();

		if (!highPressure)
			return lifetime;

		if (IsProtectedTailVoice(context))
			return BS5_EchoMath.MaxFloat(pressureLifetime, lifetime * 0.82);

		bool farOrQuiet = context.m_fCandidateDistance > 560.0 || context.m_fIntensity < 0.22;
		bool expendable = IsLowValueTailVoice(context);
		if (farOrQuiet || expendable)
			lifetime = BS5_EchoMath.MinFloat(lifetime, pressureLifetime);

		if (criticalPressure && expendable)
			lifetime = BS5_EchoMath.MinFloat(lifetime, minLifetime);

		return BS5_EchoMath.MaxFloat(minLifetime, lifetime);
	}

	protected static int ResolveEstimatedNativeSources(BS5_PendingEmissionContext context, BS5_EchoDriverComponent settings)
	{
		if (!context)
			return 1;

		if (context.m_bSlapback)
		{
			if (settings)
				return settings.GetLimiterEstimatedSourcesPerSlapback();
			return 1;
		}

		if (context.m_bSuppressed)
			return 1;

		if (context.m_bExplosion)
			return 2;

		if (settings)
			return settings.GetLimiterEstimatedSourcesPerTail();
		return 2;
	}

	protected static int GetContextVoiceWeight(BS5_PendingEmissionContext context)
	{
		if (!context)
			return 1;

		if (context.m_iEstimatedNativeSources < 1)
			return 1;

		return context.m_iEstimatedNativeSources;
	}

	protected static bool TryEnterStartGate(BS5_PendingEmissionContext context, BS5_EchoDriverComponent driver, bool debugEnabled)
	{
		if (!context || !driver || !driver.IsPlaybackLimiterEnabled())
			return true;

		EnsureStartGateWindow();

		int weight = GetContextVoiceWeight(context);
		if (context.m_bSlapback)
		{
			int maxSlapbackStarts = driver.GetLimiterMaxSlapbackStartsPer100Ms();
			if (s_iSlapbackStartsInGate + weight <= maxSlapbackStarts)
			{
				s_iSlapbackStartsInGate += weight;
				return true;
			}

			return DeferOrDropAtStartGate(context, debugEnabled, s_iSlapbackStartsInGate, weight, maxSlapbackStarts);
		}

		int maxTailStarts = driver.GetLimiterMaxTailStartsPer100Ms();
		if (s_iTailStartsInGate + weight <= maxTailStarts)
		{
			s_iTailStartsInGate += weight;
			return true;
		}

		return DeferOrDropAtStartGate(context, debugEnabled, s_iTailStartsInGate, weight, maxTailStarts);
	}

	protected static bool DeferOrDropAtStartGate(BS5_PendingEmissionContext context, bool debugEnabled, int activeStarts, int weight, int maxStarts)
	{
		if (!context)
			return false;

		if (context.m_iStartGateDeferCount < 1 && !IsLowValueTailVoice(context))
		{
			context.m_iStartGateDeferCount++;
			ScriptCallQueue callQueue = GetGame().GetCallqueue();
			if (callQueue)
			{
				callQueue.CallLater(BS5_EchoEmissionService.EmitPending, START_GATE_DEFER_MS, false, context);
				string deferLog = "start gate defer";
				deferLog += " slapback=" + BS5_DebugLog.BoolText(context.m_bSlapback);
				deferLog += " active=" + activeStarts;
				deferLog += " weight=" + weight;
				deferLog += " max=" + maxStarts;
				deferLog += " source=" + BS5_EchoMath.CandidateSourceName(context.m_eSourceType);
				BS5_DebugLog.LineEnabled(debugEnabled, deferLog);
				return false;
			}
		}

		string dropLog = "start gate drop";
		dropLog += " slapback=" + BS5_DebugLog.BoolText(context.m_bSlapback);
		dropLog += " active=" + activeStarts;
		dropLog += " weight=" + weight;
		dropLog += " max=" + maxStarts;
		dropLog += " source=" + BS5_EchoMath.CandidateSourceName(context.m_eSourceType);
		BS5_DebugLog.LineEnabled(debugEnabled, dropLog);
		BS5_DebugLog.LineEnabled(debugEnabled, "start gate drop detail dist=" + context.m_fCandidateDistance + " intensity=" + context.m_fIntensity);
		return false;
	}

	protected static void EnsureStartGateWindow()
	{
		if (s_bStartGateResetQueued)
			return;

		s_bStartGateResetQueued = true;
		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		if (callQueue)
			callQueue.CallLater(BS5_EchoEmissionService.ResetStartGateWindow, START_GATE_WINDOW_MS, false);
		else
			ResetStartGateWindow();
	}

	protected static void ResetStartGateWindow()
	{
		s_iTailStartsInGate = 0;
		s_iSlapbackStartsInGate = 0;
		s_bStartGateResetQueued = false;
	}

	protected static bool TryPlayManagedAudioSource(BS5_PendingEmissionContext context, BS5_EchoDriverComponent driver, bool debugEnabled)
	{
		if (!context || context.m_sProject == string.Empty || context.m_sEventName == string.Empty)
		{
			BS5_DebugLog.LineEnabled(debugEnabled, "SoundManager skip invalid context/project/event");
			return false;
		}

		Game game = GetGame();
		if (!game)
		{
			BS5_DebugLog.LineEnabled(debugEnabled, "SoundManager skip no game");
			return false;
		}

		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(game.GetWorld());
		if (!soundManager)
		{
			BS5_DebugLog.LineEnabled(debugEnabled, "SoundManager skip missing module");
			return false;
		}

		if (TryPlayManagedAudioSourceEvent(context, soundManager, context.m_sEventName, debugEnabled))
			return true;

		if (context.m_bSlapback && context.m_sEventName != "SOUND_SHOT")
		{
			BS5_DebugLog.LineEnabled(debugEnabled, "SoundManager slapback retry fallback SOUND_SHOT requested=" + context.m_sEventName);
			return TryPlayManagedAudioSourceEvent(context, soundManager, "SOUND_SHOT", debugEnabled);
		}

		return false;
	}

	protected static bool TryPlayManagedAudioSourceEvent(BS5_PendingEmissionContext context, SCR_SoundManagerModule soundManager, string eventName, bool debugEnabled)
	{
		if (!context || !soundManager || eventName == string.Empty)
			return false;

		SCR_AudioSourceConfiguration audioConfig = new SCR_AudioSourceConfiguration();
		audioConfig.m_sSoundProject = context.m_sProject;
		audioConfig.m_sSoundEventName = eventName;
		if (!audioConfig.IsValid())
		{
			BS5_DebugLog.LineEnabled(debugEnabled, "SoundManager config invalid event=" + eventName + " slapback=" + BS5_DebugLog.BoolText(context.m_bSlapback));
			return false;
		}

		SCR_AudioSource audioSource = soundManager.CreateAudioSource(audioConfig, context.m_vEmitPosition);
		if (!audioSource)
		{
			BS5_DebugLog.LineEnabled(debugEnabled, "SoundManager source null event=" + eventName);
			return false;
		}

		array<string> signalNames = {};
		array<float> signalValues = {};
		BS5_SpatialSoundEmitterComponent.BuildAudioSystemSignals(context, signalNames, signalValues);
		for (int i = 0; i < signalNames.Count(); i++)
			audioSource.SetSignalValue(signalNames[i], signalValues[i]);

		soundManager.PlayAudioSource(audioSource);
		if (audioSource.m_AudioHandle == -1)
		{
			audioSource.Terminate(false);
			BS5_DebugLog.LineEnabled(debugEnabled, "SoundManager play failed event=" + eventName + " slapback=" + BS5_DebugLog.BoolText(context.m_bSlapback));
			return false;
		}

		context.m_sEventName = eventName;
		context.m_AudioSource = audioSource;
		context.m_hPlayback = audioSource.m_AudioHandle;
		context.m_bPlayedViaSoundManager = true;
		string managedPlayLog = "SoundManager play";
		managedPlayLog += " event=" + context.m_sEventName;
		managedPlayLog += " handle=" + context.m_hPlayback;
		managedPlayLog += " weight=" + GetContextVoiceWeight(context);
		managedPlayLog += " slapback=" + BS5_DebugLog.BoolText(context.m_bSlapback);
		BS5_DebugLog.LineEnabled(debugEnabled, managedPlayLog);
		return true;
	}

	protected static bool IsActiveVoiceValid(BS5_ActiveEchoVoice voice)
	{
		if (!voice || !voice.m_Context || voice.m_Context.m_bEmitterCleanupDone || voice.m_Context.m_bPlaybackTerminated || voice.m_Context.m_bCancelled)
			return false;

		if (voice.m_Context.m_hPlayback != -1 && !AudioSystem.IsSoundPlayed(voice.m_Context.m_hPlayback))
			return false;

		if (voice.m_pEmitter || voice.m_Context.m_AudioSource || voice.m_Context.m_hPlayback != -1)
			return true;

		return false;
	}

	protected static void StopPlayback(BS5_PendingEmissionContext context, IEntity emitterEntity)
	{
		if (!context || context.m_bPlaybackTerminated)
			return;

		float fadeSeconds = 0.08;
		IEntity owner = ResolveContextOwner(context);
		if (owner)
		{
			context.m_pOwner = owner;
			BS5_EchoDriverComponent driver = BS5_EchoDriverComponent.Cast(owner.FindComponent(BS5_EchoDriverComponent));
			if (driver)
				fadeSeconds = driver.GetLimiterStealFadeSeconds();
		}

		if (context.m_AudioSource)
		{
			context.m_AudioSource.Terminate(true);
			context.m_AudioSource = null;
			context.m_hPlayback = -1;
			context.m_bPlaybackTerminated = true;
			return;
		}

		if (emitterEntity)
		{
			SoundComponent soundComponent = SoundComponent.Cast(emitterEntity.FindComponent(SoundComponent));
			if (soundComponent)
			{
				if (context.m_hPlayback != -1)
					soundComponent.Terminate(context.m_hPlayback);
				else
					soundComponent.TerminateAll();
				context.m_hPlayback = -1;
				context.m_bPlaybackTerminated = true;
				return;
			}
		}

		if (context.m_hPlayback != -1)
		{
			AudioSystem.TerminateSoundFadeOut(context.m_hPlayback, true, fadeSeconds);
			context.m_hPlayback = -1;
		}

		context.m_bPlaybackTerminated = true;
	}

	protected static void ReleaseDriverEmitterBudget(BS5_PendingEmissionContext context)
	{
		if (!context || !context.m_bDriverBudgetAcquired)
			return;

		IEntity owner = ResolveContextOwner(context);
		if (!owner)
		{
			context.m_bDriverBudgetAcquired = false;
			return;
		}
		context.m_pOwner = owner;

		BS5_EchoDriverComponent driver = BS5_EchoDriverComponent.Cast(owner.FindComponent(BS5_EchoDriverComponent));
		if (driver)
			driver.ReleaseActiveEmitterBudget(context.m_bSlapback);

		context.m_bDriverBudgetAcquired = false;
	}

	protected static void EnsureActiveVoicePool()
	{
		EnsureLimiterWorldState();
		if (!s_aActiveVoices)
			s_aActiveVoices = new array<ref BS5_ActiveEchoVoice>();
		if (!s_aPendingEmissionContexts)
			s_aPendingEmissionContexts = new array<ref BS5_PendingEmissionContext>();
		if (!s_aReservedPlaybackVoices)
			s_aReservedPlaybackVoices = new array<ref BS5_PendingEmissionContext>();
		PruneOrphanedLimiterContexts();
	}

	protected static IEntity SpawnEmitterEntity(BS5_PendingEmissionContext context, bool debugEnabled)
	{
		if (!context || context.m_sEmitterPrefab == string.Empty)
			return null;

		Game game = GetGame();
		if (!game)
			return null;

		Resource emitterPrefab = ResolveEmitterPrefabResource(context.m_sEmitterPrefab, debugEnabled);
		if (!emitterPrefab || !emitterPrefab.IsValid())
			return null;

		IEntity emitterEntity = game.SpawnEntityPrefab(emitterPrefab, game.GetWorld(), null);
		if (!emitterEntity)
		{
			ChimeraGame chimeraGame = ChimeraGame.Cast(game);
			if (chimeraGame)
				emitterEntity = chimeraGame.SpawnEntityPrefabLocal(emitterPrefab, game.GetWorld(), null);
		}

		if (debugEnabled)
			PrintFormat("BS5 emitter spawn: prefab=%1 entity=%2", context.m_sEmitterPrefab, emitterEntity);
		return emitterEntity;
	}

	protected static Resource ResolveEmitterPrefabResource(ResourceName emitterPrefabName, bool debugEnabled)
	{
		if (emitterPrefabName == string.Empty)
			return null;

		EnsureEmitterResourceCache();

		for (int invalidIndex = 0; invalidIndex < s_aInvalidEmitterResourceNames.Count(); invalidIndex++)
		{
			if (s_aInvalidEmitterResourceNames[invalidIndex] == emitterPrefabName)
				return null;
		}

		for (int resourceIndex = 0; resourceIndex < s_aEmitterResourceNames.Count(); resourceIndex++)
		{
			if (s_aEmitterResourceNames[resourceIndex] != emitterPrefabName)
				continue;

			Resource cachedResource = s_aEmitterResources[resourceIndex];
			if (cachedResource && cachedResource.IsValid())
				return cachedResource;

			s_aEmitterResourceNames.Remove(resourceIndex);
			s_aEmitterResources.Remove(resourceIndex);
			break;
		}

		Resource loadedResource = Resource.Load(emitterPrefabName);
		if (!loadedResource || !loadedResource.IsValid())
		{
			if (s_aInvalidEmitterResourceNames.Count() >= INVALID_EMITTER_RESOURCE_CACHE_LIMIT)
				s_aInvalidEmitterResourceNames.Remove(0);
			s_aInvalidEmitterResourceNames.Insert(emitterPrefabName);
			if (debugEnabled)
				PrintFormat("BS5 emitter resource load failed and cached invalid: %1", emitterPrefabName);
			return null;
		}

		s_aEmitterResourceNames.Insert(emitterPrefabName);
		s_aEmitterResources.Insert(loadedResource);
		return loadedResource;
	}

	protected static void EnsureEmitterResourceCache()
	{
		if (!s_aEmitterResourceNames)
			s_aEmitterResourceNames = new array<ResourceName>();
		if (!s_aEmitterResources)
			s_aEmitterResources = new array<ref Resource>();
		if (!s_aInvalidEmitterResourceNames)
			s_aInvalidEmitterResourceNames = new array<ResourceName>();
	}

	static void CleanupEmitter(IEntity emitterEntity)
	{
		if (!emitterEntity)
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(emitterEntity);
	}

	static void ReleaseAndCleanupEmitter(BS5_PendingEmissionContext context, IEntity emitterEntity)
	{
		if (context && context.m_bEmitterCleanupDone)
			return;

		StopPlayback(context, emitterEntity);
		UnregisterActiveVoice(context, emitterEntity);

		if (context)
		{
			ReleasePendingVoice(context);
			ReleasePlaybackVoice(context);
			ReleaseDriverEmitterBudget(context);
			context.m_bEmitterCleanupDone = true;
		}

		CleanupEmitter(emitterEntity);
	}

	protected static float ComputeDistanceGain(BS5_EchoDriverComponent settings, float distanceMeters)
	{
		if (!settings)
			return 1.0;

		float nearDistance = settings.GetDistanceGainNearMeters();
		float farDistance = settings.GetDistanceGainFarMeters();
		float farVolume = settings.GetDistanceGainFarVolume();
		float curvePower = settings.GetDistanceGainCurvePower();

		if (distanceMeters <= nearDistance || farDistance <= nearDistance)
			return 1.0;

		float alpha = (distanceMeters - nearDistance) / (farDistance - nearDistance);
		alpha = BS5_EchoMath.Clamp01(alpha);
		alpha = Math.Pow(alpha, curvePower);
		return BS5_EchoMath.Clamp01(1.0 - ((1.0 - farVolume) * alpha));
	}

	protected static float ComputeSlapbackDistanceGain(BS5_EchoDriverComponent settings, float distanceMeters)
	{
		if (!settings)
			return 1.0;

		float maxDistance = BS5_EchoMath.MaxFloat(1.0, settings.GetNearSlapbackRadius());
		float alpha = BS5_EchoMath.Clamp01(distanceMeters / maxDistance);
		alpha = Math.Pow(alpha, settings.GetSlapbackDistanceGainCurvePower());

		float farGain = settings.GetSlapbackDistanceFarGain();
		return BS5_EchoMath.Clamp01(1.0 - ((1.0 - farGain) * alpha));
	}

}
