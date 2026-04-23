[ComponentEditorProps(category: "BS5 Audio", description: "Dedicated close slapback/reflection settings and content routing.")]
class BS5_CloseReflectionSettingsComponentClass : ScriptComponentClass
{
}

class BS5_CloseReflectionSettingsComponent : ScriptComponent
{
	protected static const ResourceName DEFAULT_CLOSE_SLAPBACK_ACP = "{1A9C4D83B5E24761}Sounds/Weapons/Rifles/BS5/Weapons_Slapbacks_Close_Master.acp";
	protected static const ResourceName DEFAULT_CLOSE_SLAPBACK_EMITTER_PREFAB = "{64E4F2017A4A2C5B}Prefabs/Props/BS5_SlapbackEmitter_Close.et";

	[Attribute(defvalue: "1", desc: "Enables the dedicated close slapback/reflection planner and playback routing.")]
	protected bool m_bEnableCloseReflection;

	[Attribute(defvalue: "{1A9C4D83B5E24761}Sounds/Weapons/Rifles/BS5/Weapons_Slapbacks_Close_Master.acp", desc: "Dedicated ACP override for close reflection playback.")]
	protected ResourceName m_sCloseSlapbackAcp;

	[Attribute(defvalue: "{64E4F2017A4A2C5B}Prefabs/Props/BS5_SlapbackEmitter_Close.et", desc: "Dedicated emitter prefab for close reflection playback.")]
	protected ResourceName m_sCloseSlapbackEmitterPrefab;

	[Attribute(defvalue: "SOUND_SHOT", desc: "Dedicated event name used for close reflection playback.")]
	protected string m_sCloseSlapbackEventName;

	[Attribute(defvalue: "3.8", desc: "Maximum wall distance in meters that still counts as a close reflection pocket.")]
	protected float m_fMaxCloseDistanceMeters;

	[Attribute(defvalue: "0.20", desc: "Minimum close-space evidence required before the planner attempts a close result.")]
	protected float m_fBaseEvidenceMin;

	[Attribute(defvalue: "0.60", desc: "Final close score required to accept the close reflection mode.")]
	protected float m_fAcceptScoreMin;

	[Attribute(defvalue: "0.58", desc: "Minimum side-pair score required to accept a close pocket from left/right walls.")]
	protected float m_fSidePairAcceptScore;

	[Attribute(defvalue: "0.62", desc: "Minimum front/back pair score required to accept a close pocket.")]
	protected float m_fFrontBackPairAcceptScore;

	[Attribute(defvalue: "0.56", desc: "Minimum rescue-pair score required after an extra wall probe is used.")]
	protected float m_fRescuePairAcceptScore;

	[Attribute(defvalue: "0.60", desc: "Minimum score required when the planner only finds one wall plus a roof rescue.")]
	protected float m_fRoofSingleAcceptScore;

	[Attribute(defvalue: "0.06", desc: "Extra trench score margin required for trench mode to override a valid close mode result.")]
	protected float m_fTrenchOverrideMargin;

	[Attribute(defvalue: "1", desc: "Allows one cheap upward rescue trace to confirm low-roof close pockets.")]
	protected bool m_bAllowRoofRescue;

	[Attribute(defvalue: "1", desc: "Allows one extra lateral/front-back rescue trace when only one strong wall is confirmed.")]
	protected bool m_bAllowWallRescue;

	[Attribute(defvalue: "1.20", desc: "Extra intensity multiplier applied to close slapbacks after normal slapback shaping.")]
	protected float m_fIntensityMultiplier;

	[Attribute(defvalue: "1.90", desc: "Additional reverb-send multiplier for close slapbacks.")]
	protected float m_fReverbSendBoost;

	[Attribute(defvalue: "0.42", desc: "Tail-width multiplier for close slapbacks so they read as a local pocket wash.")]
	protected float m_fTailWidthScale;

	[Attribute(defvalue: "0.74", desc: "Minimum surface-hardness signal forced for close slapbacks.")]
	protected float m_fSurfaceHardnessFloor;

	bool IsEnabled()
	{
		return m_bEnableCloseReflection;
	}

	ResourceName ResolveCloseSlapbackAcp()
	{
		if (m_sCloseSlapbackAcp != string.Empty)
			return m_sCloseSlapbackAcp;

		return DEFAULT_CLOSE_SLAPBACK_ACP;
	}

	ResourceName ResolveCloseSlapbackEmitterPrefab()
	{
		if (m_sCloseSlapbackEmitterPrefab != string.Empty)
			return m_sCloseSlapbackEmitterPrefab;

		return DEFAULT_CLOSE_SLAPBACK_EMITTER_PREFAB;
	}

	string ResolveCloseSlapbackEventName()
	{
		if (m_sCloseSlapbackEventName != string.Empty)
			return m_sCloseSlapbackEventName;

		return "SOUND_SHOT";
	}

	float GetMaxCloseDistanceMeters(float fallbackDistance)
	{
		float maxDistance = m_fMaxCloseDistanceMeters;
		if (maxDistance <= 0.1)
			maxDistance = fallbackDistance;

		return BS5_EchoMath.Clamp(maxDistance, 1.2, 7.0);
	}

	float GetBaseEvidenceMin()
	{
		return BS5_EchoMath.Clamp(m_fBaseEvidenceMin, 0.0, 0.95);
	}

	float GetAcceptScoreMin()
	{
		return BS5_EchoMath.Clamp(m_fAcceptScoreMin, 0.15, 0.98);
	}

	float GetSidePairAcceptScore()
	{
		return BS5_EchoMath.Clamp(m_fSidePairAcceptScore, 0.15, 0.98);
	}

	float GetFrontBackPairAcceptScore()
	{
		return BS5_EchoMath.Clamp(m_fFrontBackPairAcceptScore, 0.15, 0.98);
	}

	float GetRescuePairAcceptScore()
	{
		return BS5_EchoMath.Clamp(m_fRescuePairAcceptScore, 0.15, 0.98);
	}

	float GetRoofSingleAcceptScore()
	{
		return BS5_EchoMath.Clamp(m_fRoofSingleAcceptScore, 0.15, 0.98);
	}

	float GetTrenchOverrideMargin()
	{
		return BS5_EchoMath.Clamp(m_fTrenchOverrideMargin, 0.0, 0.25);
	}

	bool AllowRoofRescue()
	{
		return m_bAllowRoofRescue;
	}

	bool AllowWallRescue()
	{
		return m_bAllowWallRescue;
	}

	float GetIntensityMultiplier()
	{
		return BS5_EchoMath.Clamp(m_fIntensityMultiplier, 0.25, 2.5);
	}

	float GetReverbSendBoost()
	{
		return BS5_EchoMath.Clamp(m_fReverbSendBoost, 0.25, 3.0);
	}

	float GetTailWidthScale()
	{
		return BS5_EchoMath.Clamp(m_fTailWidthScale, 0.05, 1.0);
	}

	float GetSurfaceHardnessFloor()
	{
		return BS5_EchoMath.Clamp(m_fSurfaceHardnessFloor, 0.0, 1.0);
	}
}
