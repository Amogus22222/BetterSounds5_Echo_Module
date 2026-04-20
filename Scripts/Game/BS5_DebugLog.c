class BS5_DebugLog
{
	static void Line(BS5_EchoDriverComponent driver, string message)
	{
		if (!driver || !driver.IsDebugEnabled())
			return;

		Print("[BS5] " + message);
	}

	static void LineEnabled(bool enabled, string message)
	{
		if (!enabled)
			return;

		Print("[BS5] " + message);
	}

	static string BoolText(bool value)
	{
		if (value)
			return "1";

		return "0";
	}
}
