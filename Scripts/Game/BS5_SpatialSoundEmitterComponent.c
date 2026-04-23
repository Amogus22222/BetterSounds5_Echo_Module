[ComponentEditorProps(category: "BS5 Audio", description: "Runtime helper component for spawned spatial BS5 emitters.")]
class BS5_SpatialSoundEmitterComponentClass : ScriptComponentClass
{
}

class BS5_SpatialSoundEmitterComponent : ScriptComponent
{
	protected SoundComponent m_SoundComponent;
	protected SignalsManagerComponent m_SignalsManager;
	protected static ref array<ResourceName> s_aInitializedAudioProjects;
	protected static ref array<ResourceName> s_aInvalidAudioProjects;
	protected static ref array<string> s_aInvalidAudioProjectEvents;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		ResolveComponents();
	}

	bool IsReady()
	{
		return ResolveComponents();
	}

	AudioHandle Play(BS5_PendingEmissionContext context, bool debugEnabled)
	{
		if (!context || !ResolveComponents())
			return -1;

		IEntity owner = GetOwner();
		if (!owner)
			return -1;

		owner.SetOrigin(context.m_vEmitPosition);

		vector transform[4];
		owner.GetTransform(transform);

		SetSignalValue("BS5_Intensity", context.m_fIntensity);
		SetSignalValue("BS5_DelaySeconds", context.m_fDelaySeconds);

		float slapbackValue = 0.0;
		if (context.m_bSlapback)
			slapbackValue = 1.0;
		SetSignalValue("BS5_IsSlapback", slapbackValue);
		SetSignalValue("BS5_SlapbackMode", GetSlapbackModeSignal(context));

		float explosionValue = 0.0;
		if (context.m_bExplosion)
			explosionValue = 1.0;
		SetSignalValue("BS5_IsExplosion", explosionValue);

		float suppressedValue = 0.0;
		if (context.m_bSuppressed)
			suppressedValue = 1.0;
		SetSignalValue("BS5_IsSuppressed", suppressedValue);

		BS5_EchoEnvironmentType environmentType = BS5_EchoEnvironmentType.OPEN_FIELD;
		float masterDelay = 0.0;
		float slapbackDelay = 0.0;
		if (context.m_Result)
		{
			environmentType = context.m_Result.m_eEnvironment;
			masterDelay = context.m_Result.m_fMasterDelaySeconds;
			slapbackDelay = context.m_Result.m_fSlapbackDelaySeconds;
		}

		SetSignalValue("BS5_EnvironmentId", BS5_EchoMath.EnvironmentId(environmentType));
		SetSignalValue("BS5_MasterDelaySeconds", masterDelay);
		SetSignalValue("BS5_SlapbackDelaySeconds", slapbackDelay);
		SetSignalValue("BS5_CandidateRank", context.m_iCandidateRank);
		SetSignalValue("BS5_DistanceNorm", context.m_fDistanceNorm);
		SetSignalValue("BS5_DistanceGain", context.m_fDistanceGain);
		float slapbackDistanceMeters = 0.0;
		float slapbackDistanceNorm = 0.0;
		float slapbackDistanceGain = 1.0;
		if (context.m_bSlapback)
		{
			slapbackDistanceMeters = context.m_fCandidateDistance;
			slapbackDistanceNorm = context.m_fDistanceNorm;
			slapbackDistanceGain = context.m_fDistanceGain;
		}
		SetSignalValue("BS5_SlapbackDistanceMeters", slapbackDistanceMeters);
		SetSignalValue("BS5_SlapbackDistanceNorm", slapbackDistanceNorm);
		SetSignalValue("BS5_SlapbackDistanceGain", slapbackDistanceGain);
		SetSignalValue("BS5_UserEchoVolume", context.m_fUserEchoVolume);
		SetSignalValue("BS5_UserSlapbackVolume", context.m_fUserSlapbackVolume);
		SetSignalValue("BS5_UserSlapbackCloseVolume", context.m_fUserSlapbackCloseVolume);
		SetSignalValue("BS5_PanBias", context.m_fPanBias);
		SetSignalValue("BS5_DirectionSupport", context.m_fDirectionSupport);
		SetSignalValue("BS5_ReverbSend", context.m_fReverbSend);
		SetSignalValue("BS5_TailWidth", context.m_fTailWidth);
		SetSignalValue("BS5_TailBrightness", context.m_fTailBrightness);
		SetSignalValue("BS5_SurfaceHardness", context.m_fSurfaceHardness);

		if (context.m_Result)
		{
			float indoorSignal = context.m_Result.m_fIndoorScore;
			float roomSizeSignal = context.m_Result.m_fRoomSize;
			float roomSizeNorm = 0.0;
			if (roomSizeSignal > 0.0)
				roomSizeNorm = BS5_EchoMath.Clamp01(roomSizeSignal / 140.0);

			SetSignalValue("Interior", indoorSignal);
			SetSignalValue("RoomSize", roomSizeSignal);
			SetSignalValue("BS5_IndoorBias", indoorSignal);
			SetSignalValue("BS5_RoomSizeNorm", roomSizeNorm);
			SetSignalValue("BS5_EnvOpen", context.m_Result.m_fOpenScore);
			SetSignalValue("BS5_EnvUrban", context.m_Result.m_fUrbanScore);
			SetSignalValue("BS5_EnvForest", context.m_Result.m_fForestScore);
			SetSignalValue("BS5_EnvHill", context.m_Result.m_fHillScore);
			SetSignalValue("BS5_EnvHard", context.m_Result.m_fHardSurfaceScore);
			SetSignalValue("BS5_EnvIndoor", context.m_Result.m_fIndoorScore);
			SetSignalValue("BS5_EnvTrench", context.m_Result.m_fTrenchScore);
			SetSignalValue("BS5_EnvFront", context.m_Result.m_fFrontConfinement);
			SetSignalValue("BS5_EnvBack", context.m_Result.m_fBackConfinement);
			SetSignalValue("BS5_EnvLeft", context.m_Result.m_fLeftConfinement);
			SetSignalValue("BS5_EnvRight", context.m_Result.m_fRightConfinement);
			SetSignalValue("BS5_Confidence", context.m_Result.m_fConfidence);
			SetSignalValue("BS5_ReflectorCount", context.m_Result.m_aCandidates.Count());
		}

		m_SoundComponent.EnableDynamicSimulation(true);
		m_SoundComponent.SetScriptedMethodsCall(true);

		AudioHandle handle = -1;
		int eventIndex = m_SoundComponent.GetEventIndex(context.m_sEventName);
		string soundComponentEvent = context.m_sEventName;
		if (eventIndex < 0 && context.m_bSlapback && context.m_sEventName != "SOUND_SHOT")
		{
			int fallbackEventIndex = m_SoundComponent.GetEventIndex("SOUND_SHOT");
			if (fallbackEventIndex >= 0)
			{
				soundComponentEvent = "SOUND_SHOT";
				eventIndex = fallbackEventIndex;
				if (debugEnabled)
					PrintFormat("BS5 emitter slapback event fallback: requested=%1 fallback=%2 prefab=%3", context.m_sEventName, soundComponentEvent, context.m_sEmitterPrefab);
			}
		}

		if (context.m_bAllowDirectProjectPlayback && context.m_sProject != string.Empty)
		{
			handle = TryPlayAudioSystemProject(context, transform);
			if (handle == -1 && debugEnabled)
				PrintFormat("BS5 emitter project play failed, fallback to prefab SoundComponent: project=%1 event=%2 prefab=%3", context.m_sProject, context.m_sEventName, context.m_sEmitterPrefab);
		}

		if (handle == -1 && eventIndex >= 0)
		{
			m_SoundComponent.SetTransformation(transform);
			m_SoundComponent.UpdateTrigger();
			handle = m_SoundComponent.PlayStr(soundComponentEvent);
			if (handle == -1)
				handle = m_SoundComponent.SoundEventTransform(soundComponentEvent, transform);
		}

		if (debugEnabled)
			PrintFormat("BS5 emitter play: project=%1 event=%2 soundEvent=%3 eventIndex=%4 handle=%5 emitterPos=%6", context.m_sProject, context.m_sEventName, soundComponentEvent, eventIndex, handle, transform[3]);

		if (handle != -1)
			context.m_hPlayback = handle;

		return handle;
	}

	protected AudioHandle TryPlayAudioSystemProject(BS5_PendingEmissionContext context, inout vector transform[4])
	{
		if (!context || context.m_sProject == string.Empty || context.m_sEventName == string.Empty)
			return -1;

		string eventKey = BuildAudioProjectEventKey(context.m_sProject, context.m_sEventName);
		if (IsAudioProjectEventKnownInvalid(eventKey))
			return -1;

		if (!EnsureAudioProjectReady(context.m_sProject))
			return -1;

		array<string> signalNames = {};
		array<float> signalValues = {};
		BuildAudioSystemSignals(context, signalNames, signalValues);
		AudioHandle handle = AudioSystem.PlayEvent(context.m_sProject, context.m_sEventName, transform, signalNames, signalValues);
		if (handle == -1)
			MarkAudioProjectEventInvalid(eventKey);

		return handle;
	}

	protected static bool EnsureAudioProjectReady(ResourceName project)
	{
		EnsureAudioProjectCaches();
		for (int validIndex = 0; validIndex < s_aInitializedAudioProjects.Count(); validIndex++)
		{
			if (s_aInitializedAudioProjects[validIndex] == project)
				return true;
		}

		for (int invalidIndex = 0; invalidIndex < s_aInvalidAudioProjects.Count(); invalidIndex++)
		{
			if (s_aInvalidAudioProjects[invalidIndex] == project)
				return false;
		}

		if (AudioSystem.PlayEventInitialize(project))
		{
			s_aInitializedAudioProjects.Insert(project);
			return true;
		}

		s_aInvalidAudioProjects.Insert(project);
		return false;
	}

	protected static string BuildAudioProjectEventKey(ResourceName project, string eventName)
	{
		return project + "|" + eventName;
	}

	protected static bool IsAudioProjectEventKnownInvalid(string eventKey)
	{
		EnsureAudioProjectCaches();
		for (int eventIndex = 0; eventIndex < s_aInvalidAudioProjectEvents.Count(); eventIndex++)
		{
			if (s_aInvalidAudioProjectEvents[eventIndex] == eventKey)
				return true;
		}

		return false;
	}

	protected static void MarkAudioProjectEventInvalid(string eventKey)
	{
		EnsureAudioProjectCaches();
		if (!IsAudioProjectEventKnownInvalid(eventKey))
			s_aInvalidAudioProjectEvents.Insert(eventKey);
	}

	protected static void EnsureAudioProjectCaches()
	{
		if (!s_aInitializedAudioProjects)
			s_aInitializedAudioProjects = new array<ResourceName>();
		if (!s_aInvalidAudioProjects)
			s_aInvalidAudioProjects = new array<ResourceName>();
		if (!s_aInvalidAudioProjectEvents)
			s_aInvalidAudioProjectEvents = new array<string>();
	}

	static void BuildAudioSystemSignals(BS5_PendingEmissionContext context, notnull array<string> signalNames, notnull array<float> signalValues)
	{
		AppendAudioSignal(signalNames, signalValues, "BS5_Intensity", context.m_fIntensity);
		AppendAudioSignal(signalNames, signalValues, "BS5_DelaySeconds", context.m_fDelaySeconds);
		if (context.m_bSlapback)
			AppendAudioSignal(signalNames, signalValues, "BS5_IsSlapback", 1.0);
		else
			AppendAudioSignal(signalNames, signalValues, "BS5_IsSlapback", 0.0);
		AppendAudioSignal(signalNames, signalValues, "BS5_SlapbackMode", GetSlapbackModeSignal(context));
		if (context.m_bExplosion)
			AppendAudioSignal(signalNames, signalValues, "BS5_IsExplosion", 1.0);
		else
			AppendAudioSignal(signalNames, signalValues, "BS5_IsExplosion", 0.0);
		if (context.m_bSuppressed)
			AppendAudioSignal(signalNames, signalValues, "BS5_IsSuppressed", 1.0);
		else
			AppendAudioSignal(signalNames, signalValues, "BS5_IsSuppressed", 0.0);

		BS5_EchoEnvironmentType environmentType = BS5_EchoEnvironmentType.OPEN_FIELD;
		float masterDelay = 0.0;
		float slapbackDelay = 0.0;
		if (context.m_Result)
		{
			environmentType = context.m_Result.m_eEnvironment;
			masterDelay = context.m_Result.m_fMasterDelaySeconds;
			slapbackDelay = context.m_Result.m_fSlapbackDelaySeconds;
		}

		AppendAudioSignal(signalNames, signalValues, "BS5_EnvironmentId", BS5_EchoMath.EnvironmentId(environmentType));
		AppendAudioSignal(signalNames, signalValues, "BS5_MasterDelaySeconds", masterDelay);
		AppendAudioSignal(signalNames, signalValues, "BS5_SlapbackDelaySeconds", slapbackDelay);
		AppendAudioSignal(signalNames, signalValues, "BS5_CandidateRank", context.m_iCandidateRank);
		AppendAudioSignal(signalNames, signalValues, "BS5_DistanceNorm", context.m_fDistanceNorm);
		AppendAudioSignal(signalNames, signalValues, "BS5_DistanceGain", context.m_fDistanceGain);
		float slapbackDistanceMeters = 0.0;
		float slapbackDistanceNorm = 0.0;
		float slapbackDistanceGain = 1.0;
		if (context.m_bSlapback)
		{
			slapbackDistanceMeters = context.m_fCandidateDistance;
			slapbackDistanceNorm = context.m_fDistanceNorm;
			slapbackDistanceGain = context.m_fDistanceGain;
		}
		AppendAudioSignal(signalNames, signalValues, "BS5_SlapbackDistanceMeters", slapbackDistanceMeters);
		AppendAudioSignal(signalNames, signalValues, "BS5_SlapbackDistanceNorm", slapbackDistanceNorm);
		AppendAudioSignal(signalNames, signalValues, "BS5_SlapbackDistanceGain", slapbackDistanceGain);
		AppendAudioSignal(signalNames, signalValues, "BS5_UserEchoVolume", context.m_fUserEchoVolume);
		AppendAudioSignal(signalNames, signalValues, "BS5_UserSlapbackVolume", context.m_fUserSlapbackVolume);
		AppendAudioSignal(signalNames, signalValues, "BS5_UserSlapbackCloseVolume", context.m_fUserSlapbackCloseVolume);
		AppendAudioSignal(signalNames, signalValues, "BS5_PanBias", context.m_fPanBias);
		AppendAudioSignal(signalNames, signalValues, "BS5_DirectionSupport", context.m_fDirectionSupport);
		AppendAudioSignal(signalNames, signalValues, "BS5_ReverbSend", context.m_fReverbSend);
		AppendAudioSignal(signalNames, signalValues, "BS5_TailWidth", context.m_fTailWidth);
		AppendAudioSignal(signalNames, signalValues, "BS5_TailBrightness", context.m_fTailBrightness);
		AppendAudioSignal(signalNames, signalValues, "BS5_SurfaceHardness", context.m_fSurfaceHardness);

		if (!context.m_Result)
			return;

		float roomSizeNorm = 0.0;
		if (context.m_Result.m_fRoomSize > 0.0)
			roomSizeNorm = BS5_EchoMath.Clamp01(context.m_Result.m_fRoomSize / 140.0);

		AppendAudioSignal(signalNames, signalValues, "Interior", context.m_Result.m_fIndoorScore);
		AppendAudioSignal(signalNames, signalValues, "RoomSize", context.m_Result.m_fRoomSize);
		AppendAudioSignal(signalNames, signalValues, "BS5_IndoorBias", context.m_Result.m_fIndoorScore);
		AppendAudioSignal(signalNames, signalValues, "BS5_RoomSizeNorm", roomSizeNorm);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvOpen", context.m_Result.m_fOpenScore);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvUrban", context.m_Result.m_fUrbanScore);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvForest", context.m_Result.m_fForestScore);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvHill", context.m_Result.m_fHillScore);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvHard", context.m_Result.m_fHardSurfaceScore);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvIndoor", context.m_Result.m_fIndoorScore);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvTrench", context.m_Result.m_fTrenchScore);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvFront", context.m_Result.m_fFrontConfinement);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvBack", context.m_Result.m_fBackConfinement);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvLeft", context.m_Result.m_fLeftConfinement);
		AppendAudioSignal(signalNames, signalValues, "BS5_EnvRight", context.m_Result.m_fRightConfinement);
		AppendAudioSignal(signalNames, signalValues, "BS5_Confidence", context.m_Result.m_fConfidence);
		AppendAudioSignal(signalNames, signalValues, "BS5_ReflectorCount", context.m_Result.m_aCandidates.Count());
	}

	protected static void AppendAudioSignal(notnull array<string> signalNames, notnull array<float> signalValues, string signalName, float value)
	{
		signalNames.Insert(signalName);
		signalValues.Insert(value);
	}

	protected static float GetSlapbackModeSignal(BS5_PendingEmissionContext context)
	{
		if (!context || !context.m_bSlapback)
			return 0.0;

		if (context.m_eSourceType == BS5_EchoCandidateSourceType.SLAPBACK_CLOSE_SPACE)
			return 3.0;

		if (context.m_eSourceType == BS5_EchoCandidateSourceType.SLAPBACK_TRENCH)
			return 2.0;

		if (context.m_eSourceType == BS5_EchoCandidateSourceType.SLAPBACK_WALL)
			return 1.0;

		return 1.0;
	}

	protected bool ResolveComponents()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;

		if (!m_SignalsManager)
			m_SignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));

		if (!m_SoundComponent)
			m_SoundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));

		return m_SoundComponent != null;
	}

	protected void SetSignalValue(string signalName, float value)
	{
		if (signalName == string.Empty)
			return;

		if (m_SignalsManager)
		{
			int signalIndex = m_SignalsManager.AddOrFindSignal(signalName, value);
			if (signalIndex >= 0)
				m_SignalsManager.SetSignalValue(signalIndex, value);
		}

		// Push the same value directly to the sound graph so runtime-controlled
		// gains are visible immediately on the same frame as PlayStr().
		if (m_SoundComponent)
			m_SoundComponent.SetSignalValueStr(signalName, value);
	}
}
