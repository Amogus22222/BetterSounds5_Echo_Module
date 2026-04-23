[ComponentEditorProps(category: "BS5 Audio", description: "Optional multi-channel BS5 runtime debug routing.")]
class BS5_AudioDebugSettingsComponentClass : ScriptComponentClass
{
}

class BS5_AudioDebugSettingsComponent : ScriptComponent
{
	[Attribute(defvalue: "0", desc: "Master BS5 debug switch. Keep disabled in normal gameplay.")]
	protected bool m_bEnableDebug;

	[Attribute(defvalue: "1", desc: "Debug detail level. 0=off, 1=basic, 2=verbose.")]
	protected int m_iDebugLevel;

	[Attribute(defvalue: "1", desc: "Enables driver lifecycle and dispatch logs.")]
	protected bool m_bDebugDriver;

	[Attribute(defvalue: "0", desc: "Enables legacy general logs that were not yet assigned to a specific channel.")]
	protected bool m_bDebugGeneral;

	[Attribute(defvalue: "1", desc: "Enables environment-analysis summary logs.")]
	protected bool m_bDebugAnalysis;

	[Attribute(defvalue: "1", desc: "Enables general slapback candidate collection logs.")]
	protected bool m_bDebugSlapback;

	[Attribute(defvalue: "1", desc: "Enables dedicated close reflection logs.")]
	protected bool m_bDebugClose;

	[Attribute(defvalue: "1", desc: "Enables emission queue and emitter-playback logs.")]
	protected bool m_bDebugEmit;

	[Attribute(defvalue: "1", desc: "Enables playback limiter and voice-stealing logs.")]
	protected bool m_bDebugLimiter;

	[Attribute(defvalue: "0", desc: "Enables SoundMap-specific planner logs.")]
	protected bool m_bDebugSoundMap;

	bool IsAnyDebugEnabled()
	{
		return m_bEnableDebug && GetDebugLevel() != BS5_DebugLevel.OFF;
	}

	BS5_DebugLevel GetDebugLevel()
	{
		if (!m_bEnableDebug)
			return BS5_DebugLevel.OFF;

		if (m_iDebugLevel >= 2)
			return BS5_DebugLevel.VERBOSE;

		return BS5_DebugLevel.BASIC;
	}

	bool Allows(BS5_DebugChannel channel, BS5_DebugLevel level = BS5_DebugLevel.BASIC)
	{
		if (!IsAnyDebugEnabled())
			return false;

		if (GetDebugLevel() < level)
			return false;

		switch (channel)
		{
			case BS5_DebugChannel.GENERAL:
				return m_bDebugGeneral;

			case BS5_DebugChannel.DRIVER:
				return m_bDebugDriver;

			case BS5_DebugChannel.ANALYSIS:
				return m_bDebugAnalysis;

			case BS5_DebugChannel.SLAPBACK:
				return m_bDebugSlapback;

			case BS5_DebugChannel.CLOSE:
				return m_bDebugClose;

			case BS5_DebugChannel.EMIT:
				return m_bDebugEmit;

			case BS5_DebugChannel.LIMITER:
				return m_bDebugLimiter;

			case BS5_DebugChannel.SOUNDMAP:
				return m_bDebugSoundMap;
		}

		return false;
	}
}
