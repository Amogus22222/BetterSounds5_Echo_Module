class BS5_CloseReflectionProbeHit
{
	bool m_bValid;
	vector m_vPosition;
	vector m_vNormal;
	float m_fDistance;
	float m_fScore;

	void BS5_CloseReflectionProbeHit()
	{
		m_bValid = false;
		m_vPosition = vector.Zero;
		m_vNormal = "0 1 0";
		m_fDistance = 0.0;
		m_fScore = 0.0;
	}
}

class BS5_CloseReflectionPlannerResult
{
	bool m_bAccepted;
	float m_fScore;
	int m_iRescueRayCount;
	float m_fBaseEvidence;
	float m_fBestSupport;
	float m_fCoverageScore;
	float m_fRayDensityScore;
	float m_fSidePairScore;
	float m_fFrontBackPairScore;
	float m_fCornerPairScore;
	int m_iContributorCount;
	int m_iRayCoverageCount;
	string m_sReason;
	ref BS5_EchoReflectorCandidate m_Candidate;

	void BS5_CloseReflectionPlannerResult()
	{
		m_bAccepted = false;
		m_fScore = 0.0;
		m_iRescueRayCount = 0;
		m_fBaseEvidence = 0.0;
		m_fBestSupport = 0.0;
		m_fCoverageScore = 0.0;
		m_fRayDensityScore = 0.0;
		m_fSidePairScore = 0.0;
		m_fFrontBackPairScore = 0.0;
		m_fCornerPairScore = 0.0;
		m_iContributorCount = 0;
		m_iRayCoverageCount = 0;
		m_sReason = "none";
		m_Candidate = null;
	}
}

class BS5_CloseReflectionSupportPoint
{
	vector m_vPosition;
	vector m_vNormal;
	float m_fDistance;
	float m_fWeight;
	bool m_bRoof;

	void BS5_CloseReflectionSupportPoint()
	{
		m_vPosition = vector.Zero;
		m_vNormal = "0 1 0";
		m_fDistance = 0.0;
		m_fWeight = 0.0;
		m_bRoof = false;
	}
}

class BS5_CloseReflectionEvidence
{
	ref array<ref BS5_EchoReflectorCandidate> m_aContributors;
	ref BS5_EchoReflectorCandidate m_Best;
	ref BS5_EchoReflectorCandidate m_Left;
	ref BS5_EchoReflectorCandidate m_Right;
	ref BS5_EchoReflectorCandidate m_Front;
	ref BS5_EchoReflectorCandidate m_Back;
	int m_iCoverageMask;

	void BS5_CloseReflectionEvidence()
	{
		m_aContributors = new array<ref BS5_EchoReflectorCandidate>();
		m_Best = null;
		m_Left = null;
		m_Right = null;
		m_Front = null;
		m_Back = null;
		m_iCoverageMask = 0;
	}
}

class BS5_CloseReflectionPlanner
{
	static BS5_CloseReflectionPlannerResult Evaluate(BS5_EchoDriverComponent settings, BS5_CloseReflectionSettingsComponent closeSettings, BS5_EchoAnalysisResult analysisResult, vector origin, vector probeOrigin, vector flatForward, vector flatRight, array<ref BS5_EchoReflectorCandidate> wallCandidates, int wallHitCount, int rayCount, int rawCoverageMask, BS5_EchoReflectorCandidate trenchCandidate, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot)
	{
		BS5_CloseReflectionPlannerResult plannerResult = new BS5_CloseReflectionPlannerResult();
		if (!settings || !analysisResult)
		{
			plannerResult.m_sReason = "missing_inputs";
			return plannerResult;
		}

		if (!closeSettings || !closeSettings.IsEnabled())
		{
			plannerResult.m_sReason = "disabled";
			return plannerResult;
		}

		float maxCloseDistance = closeSettings.GetMaxCloseDistanceMeters(settings.GetNearSlapbackRadius());
		BS5_CloseReflectionEvidence evidence = CollectEvidence(origin, flatForward, flatRight, wallCandidates, maxCloseDistance);
		plannerResult.m_iContributorCount = evidence.m_aContributors.Count();
		plannerResult.m_fBestSupport = ResolveBestSupport(evidence);
		plannerResult.m_fCoverageScore = ResolveCoverageScore(rawCoverageMask, evidence.m_iCoverageMask);
		plannerResult.m_iRayCoverageCount = CountCoverageBits(rawCoverageMask | evidence.m_iCoverageMask);
		plannerResult.m_fRayDensityScore = ResolveRayDensityScore(wallHitCount, rayCount);
		plannerResult.m_fBaseEvidence = ResolveBaseEvidence(analysisResult, plannerResult.m_fCoverageScore, plannerResult.m_fRayDensityScore, plannerResult.m_fBestSupport);

		if (evidence.m_aContributors.IsEmpty())
		{
			plannerResult.m_sReason = "no_close_contributors";
			return plannerResult;
		}

		if (ShouldRejectForTrench(settings, analysisResult, trenchCandidate))
		{
			plannerResult.m_sReason = "trench_like";
			return plannerResult;
		}

		plannerResult.m_fSidePairScore = ResolvePairScore(evidence.m_Left, evidence.m_Right, origin, flatForward, maxCloseDistance, true);
		plannerResult.m_fFrontBackPairScore = ResolvePairScore(evidence.m_Front, evidence.m_Back, origin, flatForward, maxCloseDistance, false);
		plannerResult.m_fCornerPairScore = ResolveCornerPairScore(evidence, origin, flatForward, maxCloseDistance);

		if (TryAcceptDirect(settings, closeSettings, origin, flatForward, flatRight, maxCloseDistance, evidence, plannerResult))
			return plannerResult;

		bool enoughBaseForRescue = plannerResult.m_fBaseEvidence >= closeSettings.GetBaseEvidenceMin();
		float strongestPhysicalScore = BS5_EchoMath.MaxFloat(plannerResult.m_fSidePairScore, plannerResult.m_fFrontBackPairScore);
		if (!enoughBaseForRescue && strongestPhysicalScore < closeSettings.GetRescuePairAcceptScore())
		{
			plannerResult.m_sReason = "low_base_evidence";
			return plannerResult;
		}

		if (TryAcceptRoofRescue(settings, closeSettings, analysisResult, origin, probeOrigin, flatForward, flatRight, maxCloseDistance, evidence, plannerResult, traceExcludeArray, traceExcludeRoot))
			return plannerResult;

		if (TryAcceptWallRescue(settings, closeSettings, analysisResult, origin, probeOrigin, flatForward, flatRight, maxCloseDistance, evidence, plannerResult, traceExcludeArray, traceExcludeRoot))
			return plannerResult;

		plannerResult.m_sReason = "not_enough_support";
		return plannerResult;
	}

	protected static BS5_CloseReflectionEvidence CollectEvidence(vector origin, vector flatForward, vector flatRight, array<ref BS5_EchoReflectorCandidate> wallCandidates, float maxCloseDistance)
	{
		BS5_CloseReflectionEvidence evidence = new BS5_CloseReflectionEvidence();
		if (!wallCandidates)
			return evidence;

		for (int i = 0; i < wallCandidates.Count(); i++)
		{
			BS5_EchoReflectorCandidate candidate = wallCandidates[i];
			if (!IsCandidateAccepted(candidate, maxCloseDistance))
				continue;

			evidence.m_aContributors.Insert(candidate);
			if (!evidence.m_Best || candidate.m_fScore > evidence.m_Best.m_fScore)
				evidence.m_Best = candidate;

			AssignDirectionalContributor(evidence, candidate, origin, flatForward, flatRight);
		}

		return evidence;
	}

	protected static bool IsCandidateAccepted(BS5_EchoReflectorCandidate candidate, float maxCloseDistance)
	{
		if (!candidate || !candidate.m_bValid)
			return false;
		if (candidate.m_eSourceType != BS5_EchoCandidateSourceType.SLAPBACK_WALL)
			return false;
		if (candidate.m_fDistance > maxCloseDistance)
			return false;
		if (candidate.m_fScore < 0.16)
			return false;
		return true;
	}

	protected static void AssignDirectionalContributor(BS5_CloseReflectionEvidence evidence, BS5_EchoReflectorCandidate candidate, vector origin, vector flatForward, vector flatRight)
	{
		if (!evidence || !candidate)
			return;

		vector direction = GetPlanarDirection(candidate.m_vPosition - origin, flatForward);
		float sideDot = vector.Dot(direction, flatRight);
		float forwardDot = vector.Dot(direction, flatForward);

		if (candidate.m_fPanBias <= -0.18 || sideDot <= -0.18)
		{
			evidence.m_iCoverageMask |= 1;
			if (!evidence.m_Left || candidate.m_fScore > evidence.m_Left.m_fScore)
				evidence.m_Left = candidate;
		}

		if (candidate.m_fPanBias >= 0.18 || sideDot >= 0.18)
		{
			evidence.m_iCoverageMask |= 2;
			if (!evidence.m_Right || candidate.m_fScore > evidence.m_Right.m_fScore)
				evidence.m_Right = candidate;
		}

		if (forwardDot >= 0.12)
		{
			evidence.m_iCoverageMask |= 4;
			if (!evidence.m_Front || candidate.m_fScore > evidence.m_Front.m_fScore)
				evidence.m_Front = candidate;
		}

		if (forwardDot <= -0.12)
		{
			evidence.m_iCoverageMask |= 8;
			if (!evidence.m_Back || candidate.m_fScore > evidence.m_Back.m_fScore)
				evidence.m_Back = candidate;
		}
	}

	protected static float ResolveBestSupport(BS5_CloseReflectionEvidence evidence)
	{
		if (!evidence || !evidence.m_Best)
			return 0.0;

		return evidence.m_Best.m_fScore;
	}

	protected static float ResolveCoverageScore(int rawCoverageMask, int contributorCoverageMask)
	{
		int coverageMask = rawCoverageMask | contributorCoverageMask;
		return BS5_EchoMath.Clamp01(CountCoverageBits(coverageMask) / 4.0);
	}

	protected static int CountCoverageBits(int mask)
	{
		int count = 0;
		if ((mask & 1) != 0)
			count++;
		if ((mask & 2) != 0)
			count++;
		if ((mask & 4) != 0)
			count++;
		if ((mask & 8) != 0)
			count++;
		return count;
	}

	protected static float ResolveRayDensityScore(int wallHitCount, int rayCount)
	{
		if (rayCount < 1)
			return 0.0;

		return BS5_EchoMath.Clamp01(wallHitCount / rayCount);
	}

	protected static float ResolveBaseEvidence(BS5_EchoAnalysisResult analysisResult, float coverageScore, float rayDensityScore, float bestSupport)
	{
		if (!analysisResult)
			return 0.0;

		float axisPairEvidence = BS5_EchoMath.MaxFloat(
			BS5_EchoMath.MinFloat(analysisResult.m_fLeftConfinement, analysisResult.m_fRightConfinement),
			BS5_EchoMath.MinFloat(analysisResult.m_fFrontConfinement, analysisResult.m_fBackConfinement));

		float baseEvidence = 0.0;
		baseEvidence += analysisResult.m_fNearConfinement * 0.28;
		baseEvidence += analysisResult.m_fHardSurfaceScore * 0.20;
		baseEvidence += analysisResult.m_fIndoorScore * 0.14;
		baseEvidence += coverageScore * 0.12;
		baseEvidence += rayDensityScore * 0.08;
		baseEvidence += bestSupport * 0.10;
		baseEvidence += axisPairEvidence * 0.08;
		return BS5_EchoMath.Clamp01(baseEvidence);
	}

	protected static bool ShouldRejectForTrench(BS5_EchoDriverComponent settings, BS5_EchoAnalysisResult analysisResult, BS5_EchoReflectorCandidate trenchCandidate)
	{
		if (!settings || !analysisResult || !trenchCandidate)
			return false;

		if (trenchCandidate.m_fScore < settings.GetTrenchSlapbackMinConfidence())
			return false;

		if (analysisResult.m_fHardSurfaceScore >= 0.60)
			return false;

		if (analysisResult.m_fTrenchScore >= 0.58)
			return true;

		if (analysisResult.m_fTrenchScore >= analysisResult.m_fIndoorScore && analysisResult.m_fNearConfinement >= 0.50)
			return true;

		return false;
	}

	protected static float ResolvePairScore(BS5_EchoReflectorCandidate candidateA, BS5_EchoReflectorCandidate candidateB, vector origin, vector flatForward, float maxCloseDistance, bool lateralPair)
	{
		if (!candidateA || !candidateB)
			return 0.0;

		float averageDistance = (candidateA.m_fDistance + candidateB.m_fDistance) * 0.5;
		if (averageDistance > maxCloseDistance)
			return 0.0;

		float minDistance = 0.25;
		float supportScore = (candidateA.m_fScore + candidateB.m_fScore) * 0.5;
		float closenessScore = 1.0 - BS5_EchoMath.Clamp01((averageDistance - minDistance) / BS5_EchoMath.MaxFloat(0.1, maxCloseDistance - minDistance));
		float symmetryScore = 1.0 - BS5_EchoMath.Clamp01(Math.AbsFloat(candidateA.m_fDistance - candidateB.m_fDistance) / BS5_EchoMath.MaxFloat(0.1, averageDistance));
		float separationScore = 0.0;
		if (lateralPair)
		{
			float sideSeparation = (Math.AbsFloat(candidateA.m_fPanBias) + Math.AbsFloat(candidateB.m_fPanBias)) * 0.55;
			separationScore = BS5_EchoMath.Clamp01(sideSeparation);
		}
		else
		{
			vector directionA = GetPlanarDirection(candidateA.m_vPosition - origin, flatForward);
			vector directionB = GetPlanarDirection(candidateB.m_vPosition - origin, flatForward);
			float forwardSeparation = (Math.AbsFloat(vector.Dot(directionA, flatForward)) + Math.AbsFloat(vector.Dot(directionB, flatForward))) * 0.5;
			separationScore = BS5_EchoMath.Clamp01(forwardSeparation);
		}

		float pairScore = 0.0;
		pairScore += supportScore * 0.46;
		pairScore += closenessScore * 0.24;
		pairScore += symmetryScore * 0.20;
		pairScore += separationScore * 0.10;
		return BS5_EchoMath.Clamp01(pairScore);
	}

	protected static float ResolveCornerPairScore(BS5_CloseReflectionEvidence evidence, vector origin, vector flatForward, float maxCloseDistance)
	{
		if (!evidence)
			return 0.0;

		float bestScore = 0.0;
		bestScore = BS5_EchoMath.MaxFloat(bestScore, ResolveCrossPairScore(evidence.m_Left, evidence.m_Front, origin, flatForward, maxCloseDistance));
		bestScore = BS5_EchoMath.MaxFloat(bestScore, ResolveCrossPairScore(evidence.m_Right, evidence.m_Front, origin, flatForward, maxCloseDistance));
		bestScore = BS5_EchoMath.MaxFloat(bestScore, ResolveCrossPairScore(evidence.m_Left, evidence.m_Back, origin, flatForward, maxCloseDistance));
		bestScore = BS5_EchoMath.MaxFloat(bestScore, ResolveCrossPairScore(evidence.m_Right, evidence.m_Back, origin, flatForward, maxCloseDistance));
		return bestScore;
	}

	protected static float ResolveCrossPairScore(BS5_EchoReflectorCandidate candidateA, BS5_EchoReflectorCandidate candidateB, vector origin, vector flatForward, float maxCloseDistance)
	{
		if (!candidateA || !candidateB)
			return 0.0;

		float averageDistance = (candidateA.m_fDistance + candidateB.m_fDistance) * 0.5;
		if (averageDistance > maxCloseDistance)
			return 0.0;

		vector directionA = GetPlanarDirection(candidateA.m_vPosition - origin, flatForward);
		vector directionB = GetPlanarDirection(candidateB.m_vPosition - origin, flatForward);
		float orthogonalScore = 1.0 - Math.AbsFloat(vector.Dot(directionA, directionB));
		float supportScore = (candidateA.m_fScore + candidateB.m_fScore) * 0.5;
		float closenessScore = 1.0 - BS5_EchoMath.Clamp01(averageDistance / BS5_EchoMath.MaxFloat(0.1, maxCloseDistance));
		return BS5_EchoMath.Clamp01((supportScore * 0.50) + (closenessScore * 0.28) + (orthogonalScore * 0.22));
	}

	protected static bool TryAcceptDirect(BS5_EchoDriverComponent settings, BS5_CloseReflectionSettingsComponent closeSettings, vector origin, vector flatForward, vector flatRight, float maxCloseDistance, BS5_CloseReflectionEvidence evidence, BS5_CloseReflectionPlannerResult plannerResult)
	{
		if (!settings || !closeSettings || !evidence || !plannerResult)
			return false;

		float acceptScoreMin = closeSettings.GetAcceptScoreMin();

		float sideAcceptScore = BS5_EchoMath.Clamp01((plannerResult.m_fSidePairScore * 0.74) + (plannerResult.m_fBaseEvidence * 0.26));
		if (plannerResult.m_fSidePairScore >= closeSettings.GetSidePairAcceptScore() && sideAcceptScore >= acceptScoreMin)
		{
			ref array<ref BS5_CloseReflectionSupportPoint> supportPoints = new array<ref BS5_CloseReflectionSupportPoint>();
			AppendCandidateSupportPoint(supportPoints, evidence.m_Left);
			AppendCandidateSupportPoint(supportPoints, evidence.m_Right);
			plannerResult.m_Candidate = BuildCloseCandidate(settings, origin, flatForward, flatRight, maxCloseDistance, supportPoints, sideAcceptScore, plannerResult.m_fSidePairScore);
			if (plannerResult.m_Candidate)
			{
				plannerResult.m_fScore = sideAcceptScore;
				plannerResult.m_bAccepted = true;
				plannerResult.m_sReason = "direct_side_pair";
				return true;
			}
		}

		float frontBackAcceptScore = BS5_EchoMath.Clamp01((plannerResult.m_fFrontBackPairScore * 0.74) + (plannerResult.m_fBaseEvidence * 0.26));
		if (plannerResult.m_fFrontBackPairScore >= closeSettings.GetFrontBackPairAcceptScore() && frontBackAcceptScore >= acceptScoreMin)
		{
			ref array<ref BS5_CloseReflectionSupportPoint> supportPointsFrontBack = new array<ref BS5_CloseReflectionSupportPoint>();
			AppendCandidateSupportPoint(supportPointsFrontBack, evidence.m_Front);
			AppendCandidateSupportPoint(supportPointsFrontBack, evidence.m_Back);
			plannerResult.m_Candidate = BuildCloseCandidate(settings, origin, flatForward, flatRight, maxCloseDistance, supportPointsFrontBack, frontBackAcceptScore, plannerResult.m_fFrontBackPairScore);
			if (plannerResult.m_Candidate)
			{
				plannerResult.m_fScore = frontBackAcceptScore;
				plannerResult.m_bAccepted = true;
				plannerResult.m_sReason = "direct_front_back_pair";
				return true;
			}
		}

		return false;
	}

	protected static bool TryAcceptRoofRescue(BS5_EchoDriverComponent settings, BS5_CloseReflectionSettingsComponent closeSettings, BS5_EchoAnalysisResult analysisResult, vector origin, vector probeOrigin, vector flatForward, vector flatRight, float maxCloseDistance, BS5_CloseReflectionEvidence evidence, BS5_CloseReflectionPlannerResult plannerResult, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot)
	{
		if (!settings || !closeSettings || !analysisResult || !evidence || !plannerResult)
			return false;
		if (!closeSettings.AllowRoofRescue())
			return false;
		if (!evidence.m_Best)
			return false;
		if (plannerResult.m_fBestSupport < 0.44 && plannerResult.m_fBaseEvidence < closeSettings.GetBaseEvidenceMin())
			return false;

		float roofTraceDistance = BS5_EchoMath.Clamp(maxCloseDistance * 1.05, 1.2, 4.8);
		BS5_CloseReflectionProbeHit roofHit = TraceProbeHit(probeOrigin, "0 1 0", roofTraceDistance, traceExcludeArray, traceExcludeRoot, true, settings);
		plannerResult.m_iRescueRayCount++;
		if (!roofHit || !roofHit.m_bValid)
		{
			plannerResult.m_sReason = "roof_rescue_miss";
			return false;
		}

		float roofSupport = ResolveRoofSupport(roofHit, maxCloseDistance);
		float roofAcceptScore = 0.0;
		roofAcceptScore += plannerResult.m_fBestSupport * 0.34;
		roofAcceptScore += plannerResult.m_fBaseEvidence * 0.22;
		roofAcceptScore += roofSupport * 0.24;
		roofAcceptScore += analysisResult.m_fIndoorScore * 0.10;
		roofAcceptScore += analysisResult.m_fHardSurfaceScore * 0.10;
		roofAcceptScore = BS5_EchoMath.Clamp01(roofAcceptScore);

		float roofAcceptMin = closeSettings.GetRoofSingleAcceptScore();
		if (roofAcceptScore < roofAcceptMin || roofAcceptScore < closeSettings.GetAcceptScoreMin())
		{
			plannerResult.m_sReason = "roof_rescue_low_score";
			return false;
		}

		ref array<ref BS5_CloseReflectionSupportPoint> supportPoints = new array<ref BS5_CloseReflectionSupportPoint>();
		AppendCandidateSupportPoint(supportPoints, evidence.m_Best);
		AppendProbeSupportPoint(supportPoints, roofHit, true);
		plannerResult.m_Candidate = BuildCloseCandidate(settings, origin, flatForward, flatRight, maxCloseDistance, supportPoints, roofAcceptScore, plannerResult.m_fBestSupport);
		if (!plannerResult.m_Candidate)
		{
			plannerResult.m_sReason = "roof_rescue_build_fail";
			return false;
		}

		plannerResult.m_fScore = roofAcceptScore;
		plannerResult.m_bAccepted = true;
		plannerResult.m_sReason = "roof_rescue";
		return true;
	}

	protected static bool TryAcceptWallRescue(BS5_EchoDriverComponent settings, BS5_CloseReflectionSettingsComponent closeSettings, BS5_EchoAnalysisResult analysisResult, vector origin, vector probeOrigin, vector flatForward, vector flatRight, float maxCloseDistance, BS5_CloseReflectionEvidence evidence, BS5_CloseReflectionPlannerResult plannerResult, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot)
	{
		if (!settings || !closeSettings || !analysisResult || !evidence || !plannerResult)
			return false;
		if (!closeSettings.AllowWallRescue())
			return false;

		BS5_EchoReflectorCandidate primaryCandidate = null;
		vector rescueDirection = vector.Zero;
		bool lateralRescue = false;
		string rescueReason = string.Empty;
		if (evidence.m_Left && !evidence.m_Right && evidence.m_Left.m_fScore >= 0.46)
		{
			primaryCandidate = evidence.m_Left;
			rescueDirection = flatRight;
			lateralRescue = true;
			rescueReason = "wall_rescue_right";
		}
		else if (evidence.m_Right && !evidence.m_Left && evidence.m_Right.m_fScore >= 0.46)
		{
			primaryCandidate = evidence.m_Right;
			rescueDirection = flatRight * -1.0;
			lateralRescue = true;
			rescueReason = "wall_rescue_left";
		}
		else if (evidence.m_Front && !evidence.m_Back && evidence.m_Front.m_fScore >= 0.46)
		{
			primaryCandidate = evidence.m_Front;
			rescueDirection = flatForward * -1.0;
			lateralRescue = false;
			rescueReason = "wall_rescue_back";
		}

		if (!primaryCandidate)
			return false;

		float rescueTraceDistance = BS5_EchoMath.Clamp(maxCloseDistance * 1.05, 1.2, 4.8);
		BS5_CloseReflectionProbeHit rescueHit = TraceProbeHit(probeOrigin, rescueDirection, rescueTraceDistance, traceExcludeArray, traceExcludeRoot, false, settings);
		plannerResult.m_iRescueRayCount++;
		if (!rescueHit || !rescueHit.m_bValid)
		{
			plannerResult.m_sReason = rescueReason + "_miss";
			return false;
		}

		float rescuePairScore = ResolveRescuePairScore(primaryCandidate, rescueHit, origin, flatForward, maxCloseDistance, lateralRescue);
		if (lateralRescue)
			plannerResult.m_fSidePairScore = BS5_EchoMath.MaxFloat(plannerResult.m_fSidePairScore, rescuePairScore);
		else
			plannerResult.m_fFrontBackPairScore = BS5_EchoMath.MaxFloat(plannerResult.m_fFrontBackPairScore, rescuePairScore);

		float rescueAcceptScore = BS5_EchoMath.Clamp01((rescuePairScore * 0.72) + (plannerResult.m_fBaseEvidence * 0.28));
		if (rescuePairScore < closeSettings.GetRescuePairAcceptScore() || rescueAcceptScore < closeSettings.GetAcceptScoreMin())
		{
			plannerResult.m_sReason = rescueReason + "_low_score";
			return false;
		}

		ref array<ref BS5_CloseReflectionSupportPoint> supportPoints = new array<ref BS5_CloseReflectionSupportPoint>();
		AppendCandidateSupportPoint(supportPoints, primaryCandidate);
		AppendProbeSupportPoint(supportPoints, rescueHit, false);
		plannerResult.m_Candidate = BuildCloseCandidate(settings, origin, flatForward, flatRight, maxCloseDistance, supportPoints, rescueAcceptScore, rescuePairScore);
		if (!plannerResult.m_Candidate)
		{
			plannerResult.m_sReason = rescueReason + "_build_fail";
			return false;
		}

		plannerResult.m_fScore = rescueAcceptScore;
		plannerResult.m_bAccepted = true;
		plannerResult.m_sReason = rescueReason;
		return true;
	}

	protected static BS5_CloseReflectionProbeHit TraceProbeHit(vector probeOrigin, vector direction, float maxDistance, array<IEntity> traceExcludeArray, IEntity traceExcludeRoot, bool roofTrace, BS5_EchoDriverComponent settings)
	{
		BS5_CloseReflectionProbeHit probeHit = new BS5_CloseReflectionProbeHit();
		BaseWorld world = GetGame().GetWorld();
		if (!world || maxDistance <= 0.0)
			return probeHit;

		vector traceDirection = direction;
		if (traceDirection.LengthSq() < 0.0001)
			return probeHit;
		traceDirection.Normalize();

		TraceParam trace = new TraceParam();
		trace.Start = probeOrigin;
		trace.End = probeOrigin + (traceDirection * maxDistance);
		if (traceExcludeArray)
			trace.ExcludeArray = traceExcludeArray;
		else if (traceExcludeRoot)
			trace.Exclude = traceExcludeRoot;

		float hitFraction = ChimeraCharacter.TraceMoveWithoutCharacters(world, trace);
		if (hitFraction < 0.0 || hitFraction >= 1.0)
			return probeHit;

		if (traceExcludeRoot && BS5_EchoEnvironmentAnalyzer.IsSelfHierarchyHit(trace.TraceEnt, traceExcludeRoot))
			return probeHit;

		if (!BS5_EchoEnvironmentAnalyzer.IsSlapbackWallTraceAccepted(trace.TraceEnt))
			return probeHit;

		float hitDistance = maxDistance * hitFraction;
		if (hitDistance < 0.25 || hitDistance > maxDistance)
			return probeHit;

		vector hitNormal = trace.TraceNorm;
		if (hitNormal.LengthSq() > 0.0001)
			hitNormal.Normalize();
		else
			hitNormal = traceDirection * -1.0;

		if (roofTrace)
		{
			if (!IsRoofNormalAccepted(hitNormal))
				return probeHit;
		}
		else
		{
			if (!IsWallNormalAccepted(hitNormal, settings))
				return probeHit;
		}

		probeHit.m_bValid = true;
		probeHit.m_vPosition = probeOrigin + (traceDirection * hitDistance);
		probeHit.m_vNormal = hitNormal;
		probeHit.m_fDistance = hitDistance;
		probeHit.m_fScore = ResolveProbeScore(hitDistance, maxDistance, hitNormal, roofTrace);
		return probeHit;
	}

	protected static bool IsRoofNormalAccepted(vector hitNormal)
	{
		if (hitNormal[1] <= -0.20)
			return true;

		return Math.AbsFloat(hitNormal[1]) >= 0.82;
	}

	protected static bool IsWallNormalAccepted(vector hitNormal, BS5_EchoDriverComponent settings)
	{
		if (!settings)
			return false;

		return Math.AbsFloat(hitNormal[1]) <= settings.GetSlapbackWallNormalMaxY();
	}

	protected static float ResolveProbeScore(float hitDistance, float maxDistance, vector hitNormal, bool roofTrace)
	{
		float minDistance = 0.25;
		float closenessScore = 1.0 - BS5_EchoMath.Clamp01((hitDistance - minDistance) / BS5_EchoMath.MaxFloat(0.1, maxDistance - minDistance));
		float normalScore = Math.AbsFloat(hitNormal[1]);
		if (!roofTrace)
			normalScore = 1.0 - normalScore;

		return BS5_EchoMath.Clamp01((closenessScore * 0.68) + (normalScore * 0.32));
	}

	protected static float ResolveRoofSupport(BS5_CloseReflectionProbeHit roofHit, float maxCloseDistance)
	{
		if (!roofHit || !roofHit.m_bValid)
			return 0.0;

		float closenessScore = 1.0 - BS5_EchoMath.Clamp01(roofHit.m_fDistance / BS5_EchoMath.MaxFloat(0.1, maxCloseDistance));
		float normalScore = BS5_EchoMath.Clamp01(Math.AbsFloat(roofHit.m_vNormal[1]));
		return BS5_EchoMath.Clamp01((roofHit.m_fScore * 0.44) + (closenessScore * 0.30) + (normalScore * 0.26));
	}

	protected static float ResolveRescuePairScore(BS5_EchoReflectorCandidate primaryCandidate, BS5_CloseReflectionProbeHit rescueHit, vector origin, vector flatForward, float maxCloseDistance, bool lateralRescue)
	{
		if (!primaryCandidate || !rescueHit || !rescueHit.m_bValid)
			return 0.0;

		float averageDistance = (primaryCandidate.m_fDistance + rescueHit.m_fDistance) * 0.5;
		float supportScore = (primaryCandidate.m_fScore + rescueHit.m_fScore) * 0.5;
		float symmetryScore = 1.0 - BS5_EchoMath.Clamp01(Math.AbsFloat(primaryCandidate.m_fDistance - rescueHit.m_fDistance) / BS5_EchoMath.MaxFloat(0.1, averageDistance));
		float closenessScore = 1.0 - BS5_EchoMath.Clamp01(averageDistance / BS5_EchoMath.MaxFloat(0.1, maxCloseDistance));
		float directionalScore = 0.0;
		if (lateralRescue)
		{
			float rescuePan = vector.Dot(GetPlanarDirection(rescueHit.m_vPosition - origin, flatForward), BuildFlatRight(flatForward));
			directionalScore = BS5_EchoMath.Clamp01((Math.AbsFloat(primaryCandidate.m_fPanBias) + Math.AbsFloat(rescuePan)) * 0.55);
		}
		else
		{
			vector primaryDirection = GetPlanarDirection(primaryCandidate.m_vPosition - origin, flatForward);
			vector rescueDirection = GetPlanarDirection(rescueHit.m_vPosition - origin, flatForward);
			float primaryForward = Math.AbsFloat(vector.Dot(primaryDirection, flatForward));
			float rescueForward = Math.AbsFloat(vector.Dot(rescueDirection, flatForward));
			directionalScore = BS5_EchoMath.Clamp01((primaryForward + rescueForward) * 0.5);
		}

		float rescuePairScore = 0.0;
		rescuePairScore += supportScore * 0.46;
		rescuePairScore += closenessScore * 0.24;
		rescuePairScore += symmetryScore * 0.20;
		rescuePairScore += directionalScore * 0.10;
		return BS5_EchoMath.Clamp01(rescuePairScore);
	}

	protected static void AppendCandidateSupportPoint(notnull array<ref BS5_CloseReflectionSupportPoint> supportPoints, BS5_EchoReflectorCandidate candidate)
	{
		if (!candidate)
			return;

		BS5_CloseReflectionSupportPoint supportPoint = new BS5_CloseReflectionSupportPoint();
		supportPoint.m_vPosition = candidate.m_vPosition;
		supportPoint.m_vNormal = candidate.m_vNormal;
		supportPoint.m_fDistance = candidate.m_fDistance;
		supportPoint.m_fWeight = BS5_EchoMath.Clamp01(candidate.m_fScore);
		supportPoint.m_bRoof = false;
		supportPoints.Insert(supportPoint);
	}

	protected static void AppendProbeSupportPoint(notnull array<ref BS5_CloseReflectionSupportPoint> supportPoints, BS5_CloseReflectionProbeHit probeHit, bool roofPoint)
	{
		if (!probeHit || !probeHit.m_bValid)
			return;

		BS5_CloseReflectionSupportPoint supportPoint = new BS5_CloseReflectionSupportPoint();
		supportPoint.m_vPosition = probeHit.m_vPosition;
		supportPoint.m_vNormal = probeHit.m_vNormal;
		supportPoint.m_fDistance = probeHit.m_fDistance;
		supportPoint.m_fWeight = BS5_EchoMath.Clamp01(probeHit.m_fScore);
		supportPoint.m_bRoof = roofPoint;
		supportPoints.Insert(supportPoint);
	}

	protected static BS5_EchoReflectorCandidate BuildCloseCandidate(BS5_EchoDriverComponent settings, vector origin, vector flatForward, vector flatRight, float maxCloseDistance, array<ref BS5_CloseReflectionSupportPoint> supportPoints, float score, float directionSupport)
	{
		if (!settings || !supportPoints || supportPoints.IsEmpty())
			return null;

		vector weightedPosition = vector.Zero;
		vector weightedNormal = vector.Zero;
		float totalWeight = 0.0;
		float weightedDistance = 0.0;
		float weightedPanBias = 0.0;
		int panContributorCount = 0;
		bool hasRoofPoint = false;

		for (int i = 0; i < supportPoints.Count(); i++)
		{
			BS5_CloseReflectionSupportPoint supportPoint = supportPoints[i];
			if (!supportPoint)
				continue;

			float weight = BS5_EchoMath.Clamp(supportPoint.m_fWeight, 0.12, 1.0);
			totalWeight += weight;
			weightedPosition += supportPoint.m_vPosition * weight;
			weightedNormal += supportPoint.m_vNormal * weight;
			weightedDistance += supportPoint.m_fDistance * weight;
			if (supportPoint.m_bRoof)
			{
				hasRoofPoint = true;
			}
			else
			{
				vector lateralDirection = GetPlanarDirection(supportPoint.m_vPosition - origin, flatForward);
				weightedPanBias += vector.Dot(lateralDirection, flatRight) * weight;
				panContributorCount++;
			}
		}

		if (totalWeight <= 0.0)
			return null;

		weightedPosition = weightedPosition * (1.0 / totalWeight);
		weightedNormal = weightedNormal * (1.0 / totalWeight);
		weightedDistance = weightedDistance / totalWeight;

		vector direction = GetPlanarDirection(weightedPosition - origin, flatForward);
		float hitDistance = BS5_EchoMath.Clamp(weightedDistance, settings.GetSlapbackMinDistanceMeters(), maxCloseDistance);
		vector hitPosition = origin + (direction * hitDistance);

		float averageHeightOffset = weightedPosition[1] - origin[1];
		if (hasRoofPoint)
			hitPosition[1] = origin[1] + BS5_EchoMath.Clamp(averageHeightOffset * 0.25, 0.08, 0.65);
		else
			hitPosition[1] = origin[1] + BS5_EchoMath.Clamp(averageHeightOffset * 0.55, -0.20, 0.35);

		if (hasRoofPoint)
			hitPosition += flatForward * 0.22;

		vector hitNormal = weightedNormal;
		if (hitNormal.LengthSq() < 0.0001)
			hitNormal = origin - hitPosition;
		if (hitNormal.LengthSq() < 0.0001)
			hitNormal = flatForward * -1.0;
		hitNormal.Normalize();

		BS5_EchoReflectorCandidate candidate = CreateCloseCandidate(settings, origin, hitPosition, hitNormal, direction, flatRight, hitDistance, maxCloseDistance, score, directionSupport);
		if (!candidate)
			return null;

		if (panContributorCount > 0 && totalWeight > 0.0)
			candidate.m_fPanBias = BS5_EchoMath.Clamp(weightedPanBias / totalWeight, -1.0, 1.0);
		else
			candidate.m_fPanBias = 0.0;
		return candidate;
	}

	protected static BS5_EchoReflectorCandidate CreateCloseCandidate(BS5_EchoDriverComponent settings, vector origin, vector hitPosition, vector hitNormal, vector direction, vector flatRight, float hitDistance, float maxDistance, float score, float directionSupport)
	{
		if (!settings)
			return null;

		BS5_EchoReflectorCandidate candidate = new BS5_EchoReflectorCandidate();
		candidate.m_bValid = true;
		candidate.m_iRank = -10;
		candidate.m_fDistance = vector.Distance(origin, hitPosition);
		candidate.m_fScore = BS5_EchoMath.Clamp01(score);
		float slapbackPath = candidate.m_fDistance * settings.GetSlapbackPathLengthScale();
		float slapbackDelay = slapbackPath / settings.GetSoundSpeedMetersPerSecond();
		float maxSlapbackDelay = settings.GetSlapbackWallDelayMaxSeconds();
		candidate.m_fDelaySeconds = BS5_EchoMath.Clamp(slapbackDelay, settings.GetSlapbackDelayMinSeconds(), maxSlapbackDelay);
		candidate.m_fDistanceNorm = BS5_EchoMath.Clamp01(hitDistance / BS5_EchoMath.MaxFloat(0.01, maxDistance));
		candidate.m_fPanBias = vector.Dot(direction, flatRight);
		candidate.m_fDirectionSupport = directionSupport;
		candidate.m_vNormal = hitNormal;
		candidate.m_vPosition = hitPosition + (hitNormal * settings.GetSlapbackWallOffsetMeters());
		candidate.m_eSourceType = BS5_EchoCandidateSourceType.SLAPBACK_CLOSE_SPACE;
		return candidate;
	}

	protected static vector GetPlanarDirection(vector direction, vector fallbackDirection)
	{
		vector planarDirection = direction;
		planarDirection[1] = 0.0;
		if (planarDirection.LengthSq() < 0.0001)
			return fallbackDirection;

		planarDirection.Normalize();
		return planarDirection;
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
}
