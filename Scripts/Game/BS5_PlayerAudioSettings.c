class BS5_GameAudioSettings : ModuleGameSettings
{
	[Attribute(defvalue: "0.75", desc: "Client-local BS5 echo volume scalar.")]
	float echoVolume;

	[Attribute(defvalue: "0.6", desc: "Client-local BS5 slapback volume scalar.")]
	float slapbackVolume;

	[Attribute(defvalue: "0.8", desc: "Client-local BS5 close slapback volume scalar.")]
	float slapbackCloseVolume;

	[Attribute(defvalue: "0.75", desc: "Client-local BS5 explosion echo volume scalar.")]
	float explosionVolume;

	[Attribute(defvalue: "1", desc: "Client-local BS5 slapback master switch.")]
	bool slapbackEnabled;

	[Attribute(defvalue: "default", desc: "Client-local BS5 technical preset id.")]
	string technicalPresetId;

	[Attribute(defvalue: "vanilla", desc: "Client-local BS5 sound preset id.")]
	string soundPresetId;
}

class BS5_PlayerAudioSettings
{
	protected static const string MODULE_NAME = "BS5_GameAudioSettings";
	protected static const string FIELD_ECHO_VOLUME = "echoVolume";
	protected static const string FIELD_SLAPBACK_VOLUME = "slapbackVolume";
	protected static const string FIELD_SLAPBACK_CLOSE_VOLUME = "slapbackCloseVolume";
	protected static const string FIELD_EXPLOSION_VOLUME = "explosionVolume";
	protected static const string FIELD_SLAPBACK_ENABLED = "slapbackEnabled";
	protected static const string FIELD_TECHNICAL_PRESET_ID = "technicalPresetId";
	protected static const string FIELD_SOUND_PRESET_ID = "soundPresetId";
	protected static const float DEFAULT_ECHO_VOLUME = 0.75;
	protected static const float DEFAULT_SLAPBACK_VOLUME = 0.6;
	protected static const float DEFAULT_SLAPBACK_CLOSE_VOLUME = 0.8;
	protected static const float DEFAULT_EXPLOSION_VOLUME = 0.75;
	protected static bool s_bInitialized;
	protected static float s_fEchoVolume = DEFAULT_ECHO_VOLUME;
	protected static float s_fSlapbackVolume = DEFAULT_SLAPBACK_VOLUME;
	protected static float s_fSlapbackCloseVolume = DEFAULT_SLAPBACK_CLOSE_VOLUME;
	protected static float s_fExplosionVolume = DEFAULT_EXPLOSION_VOLUME;
	protected static bool s_bSlapbackEnabled = true;
	protected static string s_sTechnicalPresetId = "default";
	protected static string s_sSoundPresetId = "vanilla";

	static float GetEchoVolume()
	{
		EnsureInitialized();
		return s_fEchoVolume;
	}

	static void SetEchoVolume(float value, bool saveImmediately = false, bool markSoundPresetCustom = true)
	{
		EnsureInitialized();

		float clampedValue = BS5_EchoMath.Clamp01(value);
		s_fEchoVolume = clampedValue;
		if (markSoundPresetCustom)
			s_sSoundPresetId = BS5_PresetRegistry.GetCustomSoundPresetId();

		Game game = GetGame();
		if (!game)
			return;

		UserSettings userSettings = game.GetGameUserSettings();
		if (!userSettings)
			return;

		BaseContainer module = userSettings.GetModule(MODULE_NAME);
		if (!module)
			return;

		module.Set(FIELD_ECHO_VOLUME, clampedValue);
		if (markSoundPresetCustom)
			module.Set(FIELD_SOUND_PRESET_ID, s_sSoundPresetId);
		game.UserSettingsChanged();
		if (saveImmediately)
			game.SaveUserSettings();
	}

	static float GetSlapbackVolume()
	{
		EnsureInitialized();
		return s_fSlapbackVolume;
	}

	static void SetSlapbackVolume(float value, bool saveImmediately = false, bool markSoundPresetCustom = true)
	{
		EnsureInitialized();

		float clampedValue = BS5_EchoMath.Clamp01(value);
		s_fSlapbackVolume = clampedValue;
		if (markSoundPresetCustom)
			s_sSoundPresetId = BS5_PresetRegistry.GetCustomSoundPresetId();

		Game game = GetGame();
		if (!game)
			return;

		UserSettings userSettings = game.GetGameUserSettings();
		if (!userSettings)
			return;

		BaseContainer module = userSettings.GetModule(MODULE_NAME);
		if (!module)
			return;

		module.Set(FIELD_SLAPBACK_VOLUME, clampedValue);
		if (markSoundPresetCustom)
			module.Set(FIELD_SOUND_PRESET_ID, s_sSoundPresetId);
		game.UserSettingsChanged();
		if (saveImmediately)
			game.SaveUserSettings();
	}

	static float GetSlapbackCloseVolume()
	{
		EnsureInitialized();
		return s_fSlapbackCloseVolume;
	}

	static void SetSlapbackCloseVolume(float value, bool saveImmediately = false, bool markSoundPresetCustom = true)
	{
		EnsureInitialized();

		float clampedValue = BS5_EchoMath.Clamp01(value);
		s_fSlapbackCloseVolume = clampedValue;
		if (markSoundPresetCustom)
			s_sSoundPresetId = BS5_PresetRegistry.GetCustomSoundPresetId();

		Game game = GetGame();
		if (!game)
			return;

		UserSettings userSettings = game.GetGameUserSettings();
		if (!userSettings)
			return;

		BaseContainer module = userSettings.GetModule(MODULE_NAME);
		if (!module)
			return;

		module.Set(FIELD_SLAPBACK_CLOSE_VOLUME, clampedValue);
		if (markSoundPresetCustom)
			module.Set(FIELD_SOUND_PRESET_ID, s_sSoundPresetId);
		game.UserSettingsChanged();
		if (saveImmediately)
			game.SaveUserSettings();
	}

	static bool IsSlapbackEnabled()
	{
		EnsureInitialized();
		return s_bSlapbackEnabled;
	}

	static float GetExplosionVolume()
	{
		EnsureInitialized();
		return s_fExplosionVolume;
	}

	static void SetExplosionVolume(float value, bool saveImmediately = false, bool markSoundPresetCustom = true)
	{
		EnsureInitialized();

		float clampedValue = BS5_EchoMath.Clamp01(value);
		s_fExplosionVolume = clampedValue;
		if (markSoundPresetCustom)
			s_sSoundPresetId = BS5_PresetRegistry.GetCustomSoundPresetId();

		Game game = GetGame();
		if (!game)
			return;

		UserSettings userSettings = game.GetGameUserSettings();
		if (!userSettings)
			return;

		BaseContainer module = userSettings.GetModule(MODULE_NAME);
		if (!module)
			return;

		module.Set(FIELD_EXPLOSION_VOLUME, clampedValue);
		if (markSoundPresetCustom)
			module.Set(FIELD_SOUND_PRESET_ID, s_sSoundPresetId);
		game.UserSettingsChanged();
		if (saveImmediately)
			game.SaveUserSettings();
	}

	static void SetSlapbackEnabled(bool value, bool saveImmediately = false)
	{
		EnsureInitialized();
		s_bSlapbackEnabled = value;

		Game game = GetGame();
		if (!game)
			return;

		UserSettings userSettings = game.GetGameUserSettings();
		if (!userSettings)
			return;

		BaseContainer module = userSettings.GetModule(MODULE_NAME);
		if (!module)
			return;

		module.Set(FIELD_SLAPBACK_ENABLED, value);
		game.UserSettingsChanged();
		if (saveImmediately)
			game.SaveUserSettings();
	}

	static string GetTechnicalPresetId()
	{
		EnsureInitialized();
		if (s_sTechnicalPresetId == string.Empty)
			return BS5_PresetRegistry.GetDefaultTechnicalPresetId();
		return s_sTechnicalPresetId;
	}

	static void SetTechnicalPresetId(string value, bool saveImmediately = false)
	{
		EnsureInitialized();
		if (value == string.Empty)
			value = BS5_PresetRegistry.GetDefaultTechnicalPresetId();
		s_sTechnicalPresetId = value;

		Game game = GetGame();
		if (!game)
			return;

		UserSettings userSettings = game.GetGameUserSettings();
		if (!userSettings)
			return;

		BaseContainer module = userSettings.GetModule(MODULE_NAME);
		if (!module)
			return;

		module.Set(FIELD_TECHNICAL_PRESET_ID, value);
		game.UserSettingsChanged();
		if (saveImmediately)
			game.SaveUserSettings();
	}

	static string GetSoundPresetId()
	{
		EnsureInitialized();
		if (s_sSoundPresetId == string.Empty)
			return BS5_PresetRegistry.GetDefaultSoundPresetId();
		return s_sSoundPresetId;
	}

	static void SetSoundPresetId(string value, bool saveImmediately = false)
	{
		EnsureInitialized();
		if (value == string.Empty)
			value = BS5_PresetRegistry.GetDefaultSoundPresetId();
		s_sSoundPresetId = value;

		Game game = GetGame();
		if (!game)
			return;

		UserSettings userSettings = game.GetGameUserSettings();
		if (!userSettings)
			return;

		BaseContainer module = userSettings.GetModule(MODULE_NAME);
		if (!module)
			return;

		module.Set(FIELD_SOUND_PRESET_ID, value);
		game.UserSettingsChanged();
		if (saveImmediately)
			game.SaveUserSettings();
	}

	static void Save()
	{
		EnsureInitialized();

		Game game = GetGame();
		if (game)
			game.SaveUserSettings();
	}

	protected static void EnsureInitialized()
	{
		if (s_bInitialized)
			return;

		s_bInitialized = true;

		ArmaReforgerScripted scriptedGame = GetGame();
		if (scriptedGame)
			scriptedGame.OnUserSettingsChangedInvoker().Insert(OnUserSettingsChanged);

		LoadFromUserSettings();
	}

	protected static void OnUserSettingsChanged()
	{
		BS5_SpatialSoundEmitterComponent.ClearAudioProjectCaches();
		LoadFromUserSettings();
	}

	protected static void LoadFromUserSettings()
	{
		float echoValue = DEFAULT_ECHO_VOLUME;
		float slapbackValue = DEFAULT_SLAPBACK_VOLUME;
		float slapbackCloseValue = DEFAULT_SLAPBACK_CLOSE_VOLUME;
		float explosionValue = DEFAULT_EXPLOSION_VOLUME;
		bool slapbackEnabled = true;
		string technicalPresetId = BS5_PresetRegistry.GetDefaultTechnicalPresetId();
		string soundPresetId = BS5_PresetRegistry.GetDefaultSoundPresetId();

		Game game = GetGame();
		if (!game)
		{
			s_fEchoVolume = echoValue;
			s_fSlapbackVolume = slapbackValue;
			s_fSlapbackCloseVolume = slapbackCloseValue;
			s_fExplosionVolume = explosionValue;
			s_bSlapbackEnabled = slapbackEnabled;
			s_sTechnicalPresetId = technicalPresetId;
			s_sSoundPresetId = soundPresetId;
			return;
		}

		UserSettings userSettings = game.GetGameUserSettings();
		if (!userSettings)
		{
			s_fEchoVolume = echoValue;
			s_fSlapbackVolume = slapbackValue;
			s_fSlapbackCloseVolume = slapbackCloseValue;
			s_fExplosionVolume = explosionValue;
			s_bSlapbackEnabled = slapbackEnabled;
			s_sTechnicalPresetId = technicalPresetId;
			s_sSoundPresetId = soundPresetId;
			return;
		}

		BaseContainer module = userSettings.GetModule(MODULE_NAME);
		if (module)
		{
			module.Get(FIELD_ECHO_VOLUME, echoValue);
			module.Get(FIELD_SLAPBACK_VOLUME, slapbackValue);
			module.Get(FIELD_SLAPBACK_CLOSE_VOLUME, slapbackCloseValue);
			module.Get(FIELD_EXPLOSION_VOLUME, explosionValue);
			module.Get(FIELD_SLAPBACK_ENABLED, slapbackEnabled);
			module.Get(FIELD_TECHNICAL_PRESET_ID, technicalPresetId);
			module.Get(FIELD_SOUND_PRESET_ID, soundPresetId);
		}

		s_fEchoVolume = BS5_EchoMath.Clamp01(echoValue);
		s_fSlapbackVolume = BS5_EchoMath.Clamp01(slapbackValue);
		s_fSlapbackCloseVolume = BS5_EchoMath.Clamp01(slapbackCloseValue);
		s_fExplosionVolume = BS5_EchoMath.Clamp01(explosionValue);
		s_bSlapbackEnabled = slapbackEnabled;
		if (technicalPresetId == string.Empty)
			technicalPresetId = BS5_PresetRegistry.GetDefaultTechnicalPresetId();
		if (soundPresetId == string.Empty)
			soundPresetId = BS5_PresetRegistry.GetDefaultSoundPresetId();
		s_sTechnicalPresetId = technicalPresetId;
		s_sSoundPresetId = soundPresetId;
	}
}
