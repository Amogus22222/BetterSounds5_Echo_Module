class BS5_DebugLog
{
	static bool IsEnabled(BS5_EchoDriverComponent driver, BS5_DebugChannel channel = BS5_DebugChannel.GENERAL, BS5_DebugLevel level = BS5_DebugLevel.BASIC)
	{
		if (!driver)
			return false;

		if (driver.IsLegacyDebugEnabled())
			return true;

		BS5_AudioDebugSettingsComponent debugSettings = driver.GetDebugSettingsComponent();
		if (!debugSettings)
			return false;

		return debugSettings.Allows(channel, level);
	}

	static void Channel(BS5_EchoDriverComponent driver, BS5_DebugChannel channel, string message, BS5_DebugLevel level = BS5_DebugLevel.BASIC)
	{
		if (!IsEnabled(driver, channel, level))
			return;

		Print("[BS5][" + ChannelName(channel) + "] " + message);
	}

	static void Line(BS5_EchoDriverComponent driver, string message)
	{
		Channel(driver, BS5_DebugChannel.GENERAL, message);
	}

	static void LineEnabled(bool enabled, string message)
	{
		if (!enabled)
			return;

		Print("[BS5][general] " + message);
	}

	static void ChannelEnabled(bool enabled, BS5_DebugChannel channel, string message)
	{
		if (!enabled)
			return;

		Print("[BS5][" + ChannelName(channel) + "] " + message);
	}

	static string BoolText(bool value)
	{
		if (value)
			return "1";

		return "0";
	}

	protected static string ChannelName(BS5_DebugChannel channel)
	{
		switch (channel)
		{
			case BS5_DebugChannel.DRIVER:
				return "driver";

			case BS5_DebugChannel.ANALYSIS:
				return "analysis";

			case BS5_DebugChannel.SLAPBACK:
				return "slapback";

			case BS5_DebugChannel.CLOSE:
				return "close";

			case BS5_DebugChannel.EMIT:
				return "emit";

			case BS5_DebugChannel.LIMITER:
				return "limiter";

			case BS5_DebugChannel.SOUNDMAP:
				return "soundmap";
		}

		return "general";
	}
}
