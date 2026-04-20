modded class SCR_AudioSettingsSubMenu
{
	protected static const string BS5_ECHO_VOLUME_LABEL = "Echo Volume";
	protected static const string BS5_SLAPBACK_VOLUME_LABEL = "Slapback Volume";
	protected static const string BS5_SLAPBACK_ENABLED_LABEL = "Slapback";
	protected static const string BS5_TECHNICAL_PRESET_LABEL = "Technical Preset";
	protected static const string BS5_SOUND_PRESET_LABEL = "Sound Preset";
	protected static const float BS5_ECHO_VOLUME_MIN = 0.0;
	protected static const float BS5_ECHO_VOLUME_MAX = 100.0;
	protected static const float BS5_ECHO_VOLUME_STEP = 5.0;
	protected static const float BS5_ECHO_VOLUME_ROW_HEIGHT = 48.0;
	protected static const int BS5_SETTING_TEXT_SIZE = 18;

	protected Widget m_wBs5EchoVolumeRow;
	protected HorizontalLayoutWidget m_wBs5EchoVolumeRowLayout;
	protected SliderWidget m_wBs5EchoVolumeSlider;
	protected TextWidget m_wBs5EchoVolumeLabel;
	protected TextWidget m_wBs5EchoVolumeValue;
	protected float m_fBs5SliderPercent = -1.0;
	protected float m_fBs5PendingEchoVolume = 0.5;
	protected bool m_bBs5EchoVolumeDirty;

	protected Widget m_wBs5SlapbackVolumeRow;
	protected HorizontalLayoutWidget m_wBs5SlapbackVolumeRowLayout;
	protected SliderWidget m_wBs5SlapbackVolumeSlider;
	protected TextWidget m_wBs5SlapbackVolumeLabel;
	protected TextWidget m_wBs5SlapbackVolumeValue;
	protected float m_fBs5SlapbackSliderPercent = -1.0;
	protected float m_fBs5PendingSlapbackVolume = 0.5;
	protected bool m_bBs5SlapbackVolumeDirty;

	protected Widget m_wBs5SlapbackEnabledRow;
	protected CheckBoxWidget m_wBs5SlapbackEnabledCheckbox;
	protected ButtonWidget m_wBs5SlapbackEnabledToggleButton;
	protected ButtonWidget m_wBs5SlapbackEnabledNextButton;
	protected TextWidget m_wBs5SlapbackEnabledLabel;
	protected TextWidget m_wBs5SlapbackEnabledToggleText;
	protected TextWidget m_wBs5SlapbackEnabledNextText;
	protected TextWidget m_wBs5SlapbackEnabledValue;

	protected Widget m_wBs5TechnicalPresetRow;
	protected ButtonWidget m_wBs5TechnicalPresetPrevButton;
	protected ButtonWidget m_wBs5TechnicalPresetNextButton;
	protected TextWidget m_wBs5TechnicalPresetLabel;
	protected TextWidget m_wBs5TechnicalPresetValue;
	protected TextWidget m_wBs5TechnicalPresetPrevText;
	protected TextWidget m_wBs5TechnicalPresetNextText;

	protected Widget m_wBs5SoundPresetRow;
	protected ButtonWidget m_wBs5SoundPresetPrevButton;
	protected ButtonWidget m_wBs5SoundPresetNextButton;
	protected TextWidget m_wBs5SoundPresetLabel;
	protected TextWidget m_wBs5SoundPresetValue;
	protected TextWidget m_wBs5SoundPresetPrevText;
	protected TextWidget m_wBs5SoundPresetNextText;
	protected bool m_bBs5RowsBuilding;
	protected bool m_bBs5RowsCleaned;
	protected bool m_bBs5PresetSettingsDirty;
	protected bool m_bBs5MouseActionConsumed;
	protected string m_sBs5MouseDownAction;

	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
	}

	override void OnTabShow()
	{
		super.OnTabShow();
		BuildBs5AudioSettingsRows();
	}

	protected void BuildBs5AudioSettingsRows()
	{
		if (m_bBs5RowsBuilding)
			return;

		m_bBs5RowsBuilding = true;
		if (!m_bBs5RowsCleaned)
		{
			RemoveExistingBs5ProceduralRows();
			m_bBs5RowsCleaned = true;
		}

		EnsureBs5SlapbackEnabledRow();
		EnsureBs5TechnicalPresetRow();
		EnsureBs5SoundPresetRow();
		EnsureBs5EchoVolumeRow();
		EnsureBs5SlapbackVolumeRow();
		ApplyBs5EchoVolumeStyles();
		RefreshBs5SlapbackEnabledRow();
		RefreshBs5PresetRows();
		RefreshBs5EchoVolumeRow(true);
		RefreshBs5SlapbackVolumeRow(true);
		m_bBs5RowsBuilding = false;
	}

	override void OnTabHide()
	{
		FlushBs5EchoVolumeSetting();
		FlushBs5SlapbackVolumeSetting();
		FlushBs5PresetSettings();
		super.OnTabHide();
	}

	override void OnMenuHide()
	{
		FlushBs5EchoVolumeSetting();
		FlushBs5SlapbackVolumeSetting();
		FlushBs5PresetSettings();
		super.OnMenuHide();
	}

	override void OnTabRemove()
	{
		FlushBs5EchoVolumeSetting();
		FlushBs5SlapbackVolumeSetting();
		FlushBs5PresetSettings();
		super.OnTabRemove();
	}

	override bool OnChange(Widget w, bool finished)
	{
		if (w == m_wBs5EchoVolumeSlider)
		{
			HandleBs5EchoVolumeChanged(finished);
			return false;
		}

		if (w == m_wBs5SlapbackVolumeSlider)
		{
			HandleBs5SlapbackVolumeChanged(finished);
			return false;
		}

		return super.OnChange(w, finished);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (ResolveBs5ClickAction(w) != string.Empty)
			return true;

		return super.OnClick(w, x, y, button);
	}

	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button == 0)
		{
			string action = ResolveBs5ClickAction(w);
			if (action != string.Empty)
			{
				m_sBs5MouseDownAction = action;
				m_bBs5MouseActionConsumed = false;
				return true;
			}
		}

		return super.OnMouseButtonDown(w, x, y, button);
	}

	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (button == 0)
		{
			string action = ResolveBs5ClickAction(w);
			if (action != string.Empty)
			{
				if (m_bBs5MouseActionConsumed && action == m_sBs5MouseDownAction)
					return true;

				if (m_sBs5MouseDownAction != string.Empty && action != m_sBs5MouseDownAction)
				{
					m_sBs5MouseDownAction = string.Empty;
					m_bBs5MouseActionConsumed = false;
					return true;
				}

				ExecuteBs5ClickAction(action);
				m_bBs5MouseActionConsumed = true;
				return true;
			}
		}

		return super.OnMouseButtonUp(w, x, y, button);
	}

	protected bool HandleBs5ClickAction(Widget w)
	{
		return ExecuteBs5ClickAction(ResolveBs5ClickAction(w));
	}

	protected bool ExecuteBs5ClickAction(string action)
	{
		if (action == "slapback_toggle")
		{
			BS5_PlayerAudioSettings.SetSlapbackEnabled(!BS5_PlayerAudioSettings.IsSlapbackEnabled(), false);
			m_bBs5PresetSettingsDirty = true;
			RefreshBs5SlapbackEnabledRow();
			return true;
		}

		if (action == "technical_prev")
		{
			StepBs5TechnicalPreset(-1);
			return true;
		}

		if (action == "technical_next")
		{
			StepBs5TechnicalPreset(1);
			return true;
		}

		if (action == "sound_prev")
		{
			StepBs5SoundPreset(-1);
			return true;
		}

		if (action == "sound_next")
		{
			StepBs5SoundPreset(1);
			return true;
		}

		return false;
	}

	protected string ResolveBs5ClickAction(Widget w)
	{
		Widget current = w;
		while (current)
		{
			string widgetName = current.GetName();
			if (widgetName == "Bs5SlapbackEnabledRow" || widgetName == "Bs5SlapbackEnabledPrevShell" || widgetName == "Bs5SlapbackEnabledNextShell" || widgetName == "Bs5SlapbackEnabledToggleButton" || widgetName == "Bs5SlapbackEnabledToggleButtonText" || widgetName == "Bs5SlapbackEnabledNextButton" || widgetName == "Bs5SlapbackEnabledNextButtonText" || widgetName == "Bs5SlapbackEnabledValueShell" || widgetName == "Bs5SlapbackEnabledValue")
				return "slapback_toggle";

			if (widgetName == "Bs5TechnicalPresetRowPrevShell" || widgetName == "Bs5TechnicalPresetRowPrevButton" || widgetName == "Bs5TechnicalPresetRowPrevButtonText")
				return "technical_prev";

			if (widgetName == "Bs5TechnicalPresetRowNextShell" || widgetName == "Bs5TechnicalPresetRowNextButton" || widgetName == "Bs5TechnicalPresetRowNextButtonText")
				return "technical_next";

			if (widgetName == "Bs5SoundPresetRowPrevShell" || widgetName == "Bs5SoundPresetRowPrevButton" || widgetName == "Bs5SoundPresetRowPrevButtonText")
				return "sound_prev";

			if (widgetName == "Bs5SoundPresetRowNextShell" || widgetName == "Bs5SoundPresetRowNextButton" || widgetName == "Bs5SoundPresetRowNextButtonText")
				return "sound_next";

			current = current.GetParent();
		}

		return string.Empty;
	}

	protected void EnsureBs5EchoVolumeRow()
	{
		if (m_wBs5EchoVolumeRow || !m_wScroll)
			return;

		VerticalLayoutWidget content = VerticalLayoutWidget.Cast(m_wScroll.FindAnyWidget("Content"));
		if (!content)
			content = VerticalLayoutWidget.Cast(m_wScroll.GetChildren());
		if (!content)
			return;

		WorkspaceWidget workspace = m_wScroll.GetWorkspace();
		if (!workspace)
			return;

		SizeLayoutWidget rowShell = SizeLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SizeLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		m_wBs5EchoVolumeRow = rowShell;
		if (!m_wBs5EchoVolumeRow)
			return;

		content.AddChild(m_wBs5EchoVolumeRow);
		m_wBs5EchoVolumeRow.SetName("Bs5EchoVolumeRow");
		if (rowShell)
		{
			rowShell.EnableHeightOverride(true);
			rowShell.SetHeightOverride(BS5_ECHO_VOLUME_ROW_HEIGHT);
		}
		VerticalLayoutSlot.SetPadding(m_wBs5EchoVolumeRow, 0.0, 0.0, 0.0, 0.0);
		VerticalLayoutSlot.SetHorizontalAlign(m_wBs5EchoVolumeRow, LayoutHorizontalAlign.Stretch);
		VerticalLayoutSlot.SetVerticalAlign(m_wBs5EchoVolumeRow, LayoutVerticalAlign.Center);
		VerticalLayoutSlot.SetSizeMode(m_wBs5EchoVolumeRow, LayoutSizeMode.Auto);

		m_wBs5EchoVolumeRowLayout = HorizontalLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.HorizontalLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (!m_wBs5EchoVolumeRowLayout)
		{
			m_wBs5EchoVolumeRow.RemoveFromHierarchy();
			m_wBs5EchoVolumeRow = null;
			return;
		}

		m_wBs5EchoVolumeRow.AddChild(m_wBs5EchoVolumeRowLayout);
		m_wBs5EchoVolumeRowLayout.SetName("Bs5EchoVolumeRowLayout");
		SizeLayoutSlot.SetHorizontalAlign(m_wBs5EchoVolumeRowLayout, LayoutHorizontalAlign.Stretch);
		SizeLayoutSlot.SetVerticalAlign(m_wBs5EchoVolumeRowLayout, LayoutVerticalAlign.Stretch);
		SizeLayoutSlot.SetPadding(m_wBs5EchoVolumeRowLayout, 0.0, 0.0, 0.0, 0.0);

		SizeLayoutWidget labelShell = SizeLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SizeLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		SizeLayoutWidget sliderShell = SizeLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SizeLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		SizeLayoutWidget valueShell = SizeLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SizeLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));

		if (labelShell)
		{
			m_wBs5EchoVolumeRowLayout.AddChild(labelShell);
			labelShell.SetName("Bs5EchoVolumeLabelShell");
			labelShell.EnableWidthOverride(true);
			labelShell.SetWidthOverride(220.0);
			HorizontalLayoutSlot.SetPadding(labelShell, 20.0, 8.0, 0.0, 8.0);
			HorizontalLayoutSlot.SetHorizontalAlign(labelShell, LayoutHorizontalAlign.Left);
			HorizontalLayoutSlot.SetVerticalAlign(labelShell, LayoutVerticalAlign.Center);
			HorizontalLayoutSlot.SetSizeMode(labelShell, LayoutSizeMode.Auto);
		}

		if (sliderShell)
		{
			m_wBs5EchoVolumeRowLayout.AddChild(sliderShell);
			sliderShell.SetName("Bs5EchoVolumeSliderShell");
			HorizontalLayoutSlot.SetPadding(sliderShell, 24.0, 8.0, 24.0, 8.0);
			HorizontalLayoutSlot.SetHorizontalAlign(sliderShell, LayoutHorizontalAlign.Stretch);
			HorizontalLayoutSlot.SetVerticalAlign(sliderShell, LayoutVerticalAlign.Center);
			HorizontalLayoutSlot.SetSizeMode(sliderShell, LayoutSizeMode.Fill);
			HorizontalLayoutSlot.SetFillWeight(sliderShell, 1.0);
		}

		if (valueShell)
		{
			m_wBs5EchoVolumeRowLayout.AddChild(valueShell);
			valueShell.SetName("Bs5EchoVolumeValueShell");
			valueShell.EnableWidthOverride(true);
			valueShell.SetWidthOverride(76.0);
			HorizontalLayoutSlot.SetPadding(valueShell, 0.0, 8.0, 20.0, 8.0);
			HorizontalLayoutSlot.SetHorizontalAlign(valueShell, LayoutHorizontalAlign.Right);
			HorizontalLayoutSlot.SetVerticalAlign(valueShell, LayoutVerticalAlign.Center);
			HorizontalLayoutSlot.SetSizeMode(valueShell, LayoutSizeMode.Auto);
		}

		m_wBs5EchoVolumeLabel = TextWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		m_wBs5EchoVolumeSlider = SliderWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SliderWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		m_wBs5EchoVolumeValue = TextWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));

		if (m_wBs5EchoVolumeLabel && labelShell)
		{
			labelShell.AddChild(m_wBs5EchoVolumeLabel);
			m_wBs5EchoVolumeLabel.SetName("Bs5EchoVolumeLabel");
			SizeLayoutSlot.SetHorizontalAlign(m_wBs5EchoVolumeLabel, LayoutHorizontalAlign.Left);
			SizeLayoutSlot.SetVerticalAlign(m_wBs5EchoVolumeLabel, LayoutVerticalAlign.Center);
			SizeLayoutSlot.SetPadding(m_wBs5EchoVolumeLabel, 0.0, 0.0, 0.0, 0.0);
			m_wBs5EchoVolumeLabel.SetText(BS5_ECHO_VOLUME_LABEL);
			m_wBs5EchoVolumeLabel.SetExactFontSize(BS5_SETTING_TEXT_SIZE);
		}

		if (m_wBs5EchoVolumeSlider && sliderShell)
		{
			sliderShell.AddChild(m_wBs5EchoVolumeSlider);
			m_wBs5EchoVolumeSlider.SetName("Bs5EchoVolumeSlider");
			SizeLayoutSlot.SetHorizontalAlign(m_wBs5EchoVolumeSlider, LayoutHorizontalAlign.Stretch);
			SizeLayoutSlot.SetVerticalAlign(m_wBs5EchoVolumeSlider, LayoutVerticalAlign.Center);
			SizeLayoutSlot.SetPadding(m_wBs5EchoVolumeSlider, 0.0, 0.0, 0.0, 0.0);
			m_wBs5EchoVolumeSlider.SetRange(BS5_ECHO_VOLUME_MIN, BS5_ECHO_VOLUME_MAX);
			m_wBs5EchoVolumeSlider.SetStep(BS5_ECHO_VOLUME_STEP);
			m_wBs5EchoVolumeSlider.AddHandler(this);
		}

		if (m_wBs5EchoVolumeValue && valueShell)
		{
			valueShell.AddChild(m_wBs5EchoVolumeValue);
			m_wBs5EchoVolumeValue.SetName("Bs5EchoVolumeValue");
			SizeLayoutSlot.SetHorizontalAlign(m_wBs5EchoVolumeValue, LayoutHorizontalAlign.Right);
			SizeLayoutSlot.SetVerticalAlign(m_wBs5EchoVolumeValue, LayoutVerticalAlign.Center);
			SizeLayoutSlot.SetPadding(m_wBs5EchoVolumeValue, 0.0, 0.0, 0.0, 0.0);
			m_wBs5EchoVolumeValue.SetExactFontSize(BS5_SETTING_TEXT_SIZE);
		}

		ApplyBs5EchoVolumeStyles();
	}

	protected void EnsureBs5SlapbackVolumeRow()
	{
		if (m_wBs5SlapbackVolumeRow || !m_wScroll)
			return;

		VerticalLayoutWidget content = VerticalLayoutWidget.Cast(m_wScroll.FindAnyWidget("Content"));
		if (!content)
			content = VerticalLayoutWidget.Cast(m_wScroll.GetChildren());
		if (!content)
			return;

		WorkspaceWidget workspace = m_wScroll.GetWorkspace();
		if (!workspace)
			return;

		SizeLayoutWidget rowShell = SizeLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SizeLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		m_wBs5SlapbackVolumeRow = rowShell;
		if (!m_wBs5SlapbackVolumeRow)
			return;

		content.AddChild(m_wBs5SlapbackVolumeRow);
		m_wBs5SlapbackVolumeRow.SetName("Bs5SlapbackVolumeRow");
		if (rowShell)
		{
			rowShell.EnableHeightOverride(true);
			rowShell.SetHeightOverride(BS5_ECHO_VOLUME_ROW_HEIGHT);
		}
		VerticalLayoutSlot.SetPadding(m_wBs5SlapbackVolumeRow, 0.0, 0.0, 0.0, 0.0);
		VerticalLayoutSlot.SetHorizontalAlign(m_wBs5SlapbackVolumeRow, LayoutHorizontalAlign.Stretch);
		VerticalLayoutSlot.SetVerticalAlign(m_wBs5SlapbackVolumeRow, LayoutVerticalAlign.Center);
		VerticalLayoutSlot.SetSizeMode(m_wBs5SlapbackVolumeRow, LayoutSizeMode.Auto);

		m_wBs5SlapbackVolumeRowLayout = HorizontalLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.HorizontalLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (!m_wBs5SlapbackVolumeRowLayout)
		{
			m_wBs5SlapbackVolumeRow.RemoveFromHierarchy();
			m_wBs5SlapbackVolumeRow = null;
			return;
		}

		m_wBs5SlapbackVolumeRow.AddChild(m_wBs5SlapbackVolumeRowLayout);
		m_wBs5SlapbackVolumeRowLayout.SetName("Bs5SlapbackVolumeRowLayout");
		SizeLayoutSlot.SetHorizontalAlign(m_wBs5SlapbackVolumeRowLayout, LayoutHorizontalAlign.Stretch);
		SizeLayoutSlot.SetVerticalAlign(m_wBs5SlapbackVolumeRowLayout, LayoutVerticalAlign.Stretch);
		SizeLayoutSlot.SetPadding(m_wBs5SlapbackVolumeRowLayout, 0.0, 0.0, 0.0, 0.0);

		SizeLayoutWidget labelShell = SizeLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SizeLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		SizeLayoutWidget sliderShell = SizeLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SizeLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		SizeLayoutWidget valueShell = SizeLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SizeLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));

		if (labelShell)
		{
			m_wBs5SlapbackVolumeRowLayout.AddChild(labelShell);
			labelShell.SetName("Bs5SlapbackVolumeLabelShell");
			labelShell.EnableWidthOverride(true);
			labelShell.SetWidthOverride(220.0);
			HorizontalLayoutSlot.SetPadding(labelShell, 20.0, 8.0, 0.0, 8.0);
			HorizontalLayoutSlot.SetHorizontalAlign(labelShell, LayoutHorizontalAlign.Left);
			HorizontalLayoutSlot.SetVerticalAlign(labelShell, LayoutVerticalAlign.Center);
			HorizontalLayoutSlot.SetSizeMode(labelShell, LayoutSizeMode.Auto);
		}

		if (sliderShell)
		{
			m_wBs5SlapbackVolumeRowLayout.AddChild(sliderShell);
			sliderShell.SetName("Bs5SlapbackVolumeSliderShell");
			HorizontalLayoutSlot.SetPadding(sliderShell, 24.0, 8.0, 24.0, 8.0);
			HorizontalLayoutSlot.SetHorizontalAlign(sliderShell, LayoutHorizontalAlign.Stretch);
			HorizontalLayoutSlot.SetVerticalAlign(sliderShell, LayoutVerticalAlign.Center);
			HorizontalLayoutSlot.SetSizeMode(sliderShell, LayoutSizeMode.Fill);
			HorizontalLayoutSlot.SetFillWeight(sliderShell, 1.0);
		}

		if (valueShell)
		{
			m_wBs5SlapbackVolumeRowLayout.AddChild(valueShell);
			valueShell.SetName("Bs5SlapbackVolumeValueShell");
			valueShell.EnableWidthOverride(true);
			valueShell.SetWidthOverride(76.0);
			HorizontalLayoutSlot.SetPadding(valueShell, 0.0, 8.0, 20.0, 8.0);
			HorizontalLayoutSlot.SetHorizontalAlign(valueShell, LayoutHorizontalAlign.Right);
			HorizontalLayoutSlot.SetVerticalAlign(valueShell, LayoutVerticalAlign.Center);
			HorizontalLayoutSlot.SetSizeMode(valueShell, LayoutSizeMode.Auto);
		}

		m_wBs5SlapbackVolumeLabel = TextWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		m_wBs5SlapbackVolumeSlider = SliderWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SliderWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		m_wBs5SlapbackVolumeValue = TextWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));

		if (m_wBs5SlapbackVolumeLabel && labelShell)
		{
			labelShell.AddChild(m_wBs5SlapbackVolumeLabel);
			m_wBs5SlapbackVolumeLabel.SetName("Bs5SlapbackVolumeLabel");
			SizeLayoutSlot.SetHorizontalAlign(m_wBs5SlapbackVolumeLabel, LayoutHorizontalAlign.Left);
			SizeLayoutSlot.SetVerticalAlign(m_wBs5SlapbackVolumeLabel, LayoutVerticalAlign.Center);
			SizeLayoutSlot.SetPadding(m_wBs5SlapbackVolumeLabel, 0.0, 0.0, 0.0, 0.0);
			m_wBs5SlapbackVolumeLabel.SetText(BS5_SLAPBACK_VOLUME_LABEL);
			m_wBs5SlapbackVolumeLabel.SetExactFontSize(BS5_SETTING_TEXT_SIZE);
		}

		if (m_wBs5SlapbackVolumeSlider && sliderShell)
		{
			sliderShell.AddChild(m_wBs5SlapbackVolumeSlider);
			m_wBs5SlapbackVolumeSlider.SetName("Bs5SlapbackVolumeSlider");
			SizeLayoutSlot.SetHorizontalAlign(m_wBs5SlapbackVolumeSlider, LayoutHorizontalAlign.Stretch);
			SizeLayoutSlot.SetVerticalAlign(m_wBs5SlapbackVolumeSlider, LayoutVerticalAlign.Center);
			SizeLayoutSlot.SetPadding(m_wBs5SlapbackVolumeSlider, 0.0, 0.0, 0.0, 0.0);
			m_wBs5SlapbackVolumeSlider.SetRange(BS5_ECHO_VOLUME_MIN, BS5_ECHO_VOLUME_MAX);
			m_wBs5SlapbackVolumeSlider.SetStep(BS5_ECHO_VOLUME_STEP);
			m_wBs5SlapbackVolumeSlider.AddHandler(this);
		}

		if (m_wBs5SlapbackVolumeValue && valueShell)
		{
			valueShell.AddChild(m_wBs5SlapbackVolumeValue);
			m_wBs5SlapbackVolumeValue.SetName("Bs5SlapbackVolumeValue");
			SizeLayoutSlot.SetHorizontalAlign(m_wBs5SlapbackVolumeValue, LayoutHorizontalAlign.Right);
			SizeLayoutSlot.SetVerticalAlign(m_wBs5SlapbackVolumeValue, LayoutVerticalAlign.Center);
			SizeLayoutSlot.SetPadding(m_wBs5SlapbackVolumeValue, 0.0, 0.0, 0.0, 0.0);
			m_wBs5SlapbackVolumeValue.SetExactFontSize(BS5_SETTING_TEXT_SIZE);
		}

		ApplyBs5EchoVolumeStyles();
	}

	protected void EnsureBs5SlapbackEnabledRow()
	{
		if (m_wBs5SlapbackEnabledRow || !m_wScroll)
			return;

		VerticalLayoutWidget content = GetBs5SettingsContent();
		WorkspaceWidget workspace = m_wScroll.GetWorkspace();
		if (!content || !workspace)
			return;

		HorizontalLayoutWidget rowLayout;
		m_wBs5SlapbackEnabledRow = CreateBs5BaseRow(content, workspace, "Bs5SlapbackEnabledRow", rowLayout);
		if (!m_wBs5SlapbackEnabledRow || !rowLayout)
			return;

		SizeLayoutWidget labelShell = CreateBs5SizedShell(rowLayout, "Bs5SlapbackEnabledLabelShell", 220.0, true, 20.0, 8.0, 0.0, 8.0);
		SizeLayoutWidget prevShell = CreateBs5SizedShell(rowLayout, "Bs5SlapbackEnabledPrevShell", 64.0, true, 16.0, 8.0, 8.0, 8.0);
		SizeLayoutWidget valueShell = CreateBs5SizedShell(rowLayout, "Bs5SlapbackEnabledValueShell", 180.0, false, 8.0, 8.0, 8.0, 8.0);
		SizeLayoutWidget nextShell = CreateBs5SizedShell(rowLayout, "Bs5SlapbackEnabledNextShell", 64.0, true, 8.0, 8.0, 16.0, 8.0);
		if (valueShell)
			valueShell.AddHandler(this);

		m_wBs5SlapbackEnabledLabel = TextWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (m_wBs5SlapbackEnabledLabel && labelShell)
		{
			labelShell.AddChild(m_wBs5SlapbackEnabledLabel);
			m_wBs5SlapbackEnabledLabel.SetName("Bs5SlapbackEnabledLabel");
			SizeLayoutSlot.SetHorizontalAlign(m_wBs5SlapbackEnabledLabel, LayoutHorizontalAlign.Left);
			SizeLayoutSlot.SetVerticalAlign(m_wBs5SlapbackEnabledLabel, LayoutVerticalAlign.Center);
			m_wBs5SlapbackEnabledLabel.SetText(BS5_SLAPBACK_ENABLED_LABEL);
			m_wBs5SlapbackEnabledLabel.SetExactFontSize(BS5_SETTING_TEXT_SIZE);
		}

		m_wBs5SlapbackEnabledToggleButton = CreateBs5Button(workspace, prevShell, "Bs5SlapbackEnabledToggleButton", "<", m_wBs5SlapbackEnabledToggleText);

		m_wBs5SlapbackEnabledValue = TextWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (m_wBs5SlapbackEnabledValue && valueShell)
		{
			valueShell.AddChild(m_wBs5SlapbackEnabledValue);
			m_wBs5SlapbackEnabledValue.SetName("Bs5SlapbackEnabledValue");
			m_wBs5SlapbackEnabledValue.SetExactFontSize(BS5_SETTING_TEXT_SIZE);
			m_wBs5SlapbackEnabledValue.AddHandler(this);
			SizeLayoutSlot.SetHorizontalAlign(m_wBs5SlapbackEnabledValue, LayoutHorizontalAlign.Center);
			SizeLayoutSlot.SetVerticalAlign(m_wBs5SlapbackEnabledValue, LayoutVerticalAlign.Center);
		}
		m_wBs5SlapbackEnabledNextButton = CreateBs5Button(workspace, nextShell, "Bs5SlapbackEnabledNextButton", ">", m_wBs5SlapbackEnabledNextText);
	}

	protected void EnsureBs5TechnicalPresetRow()
	{
		if (m_wBs5TechnicalPresetRow || !m_wScroll)
			return;

		VerticalLayoutWidget content = GetBs5SettingsContent();
		WorkspaceWidget workspace = m_wScroll.GetWorkspace();
		if (!content || !workspace)
			return;

		HorizontalLayoutWidget rowLayout;
		m_wBs5TechnicalPresetRow = CreateBs5PresetRow(content, workspace, "Bs5TechnicalPresetRow", BS5_TECHNICAL_PRESET_LABEL, m_wBs5TechnicalPresetLabel, m_wBs5TechnicalPresetValue, m_wBs5TechnicalPresetPrevButton, m_wBs5TechnicalPresetNextButton, m_wBs5TechnicalPresetPrevText, m_wBs5TechnicalPresetNextText);
	}

	protected void EnsureBs5SoundPresetRow()
	{
		if (m_wBs5SoundPresetRow || !m_wScroll)
			return;

		VerticalLayoutWidget content = GetBs5SettingsContent();
		WorkspaceWidget workspace = m_wScroll.GetWorkspace();
		if (!content || !workspace)
			return;

		HorizontalLayoutWidget rowLayout;
		m_wBs5SoundPresetRow = CreateBs5PresetRow(content, workspace, "Bs5SoundPresetRow", BS5_SOUND_PRESET_LABEL, m_wBs5SoundPresetLabel, m_wBs5SoundPresetValue, m_wBs5SoundPresetPrevButton, m_wBs5SoundPresetNextButton, m_wBs5SoundPresetPrevText, m_wBs5SoundPresetNextText);
	}

	protected Widget CreateBs5PresetRow(VerticalLayoutWidget content, WorkspaceWidget workspace, string rowName, string labelText, out TextWidget labelWidget, out TextWidget valueWidget, out ButtonWidget prevButton, out ButtonWidget nextButton, out TextWidget prevText, out TextWidget nextText)
	{
		HorizontalLayoutWidget rowLayout;
		Widget row = CreateBs5BaseRow(content, workspace, rowName, rowLayout);
		if (!row || !rowLayout)
			return null;

		SizeLayoutWidget labelShell = CreateBs5SizedShell(rowLayout, rowName + "LabelShell", 220.0, true, 20.0, 8.0, 0.0, 8.0);
		SizeLayoutWidget prevShell = CreateBs5SizedShell(rowLayout, rowName + "PrevShell", 64.0, true, 16.0, 8.0, 8.0, 8.0);
		SizeLayoutWidget valueShell = CreateBs5SizedShell(rowLayout, rowName + "ValueShell", 180.0, false, 8.0, 8.0, 8.0, 8.0);
		SizeLayoutWidget nextShell = CreateBs5SizedShell(rowLayout, rowName + "NextShell", 64.0, true, 8.0, 8.0, 16.0, 8.0);
		if (prevShell)
			prevShell.AddHandler(this);
		if (nextShell)
			nextShell.AddHandler(this);

		labelWidget = TextWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (labelWidget && labelShell)
		{
			labelShell.AddChild(labelWidget);
			labelWidget.SetName(rowName + "Label");
			labelWidget.SetText(labelText);
			labelWidget.SetExactFontSize(BS5_SETTING_TEXT_SIZE);
			SizeLayoutSlot.SetHorizontalAlign(labelWidget, LayoutHorizontalAlign.Left);
			SizeLayoutSlot.SetVerticalAlign(labelWidget, LayoutVerticalAlign.Center);
		}

		prevButton = CreateBs5Button(workspace, prevShell, rowName + "PrevButton", "<", prevText);
		valueWidget = TextWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (valueWidget && valueShell)
		{
			valueShell.AddChild(valueWidget);
			valueWidget.SetName(rowName + "Value");
			valueWidget.SetExactFontSize(BS5_SETTING_TEXT_SIZE);
			SizeLayoutSlot.SetHorizontalAlign(valueWidget, LayoutHorizontalAlign.Center);
			SizeLayoutSlot.SetVerticalAlign(valueWidget, LayoutVerticalAlign.Center);
		}
		nextButton = CreateBs5Button(workspace, nextShell, rowName + "NextButton", ">", nextText);

		return row;
	}

	protected ButtonWidget CreateBs5Button(WorkspaceWidget workspace, Widget parent, string name, string text, out TextWidget textWidget)
	{
		textWidget = null;
		if (!workspace || !parent)
			return null;

		ButtonWidget button = ButtonWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.ButtonWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (!button)
			return null;

		parent.AddChild(button);
		button.SetName(name);
		button.AddHandler(this);
		SizeLayoutSlot.SetHorizontalAlign(button, LayoutHorizontalAlign.Stretch);
		SizeLayoutSlot.SetVerticalAlign(button, LayoutVerticalAlign.Stretch);

		textWidget = TextWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (textWidget)
		{
			button.AddChild(textWidget);
			textWidget.SetName(name + "Text");
			textWidget.SetText(text);
			textWidget.SetExactFontSize(BS5_SETTING_TEXT_SIZE);
			textWidget.AddHandler(this);
			SizeLayoutSlot.SetHorizontalAlign(textWidget, LayoutHorizontalAlign.Center);
			SizeLayoutSlot.SetVerticalAlign(textWidget, LayoutVerticalAlign.Center);
		}

		return button;
	}

	protected Widget CreateBs5BaseRow(VerticalLayoutWidget content, WorkspaceWidget workspace, string rowName, out HorizontalLayoutWidget rowLayout)
	{
		rowLayout = null;
		SizeLayoutWidget rowShell = SizeLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SizeLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (!rowShell)
			return null;

		content.AddChild(rowShell);
		rowShell.SetName(rowName);
		rowShell.AddHandler(this);
		rowShell.EnableHeightOverride(true);
		rowShell.SetHeightOverride(BS5_ECHO_VOLUME_ROW_HEIGHT);
		VerticalLayoutSlot.SetPadding(rowShell, 0.0, 0.0, 0.0, 0.0);
		VerticalLayoutSlot.SetHorizontalAlign(rowShell, LayoutHorizontalAlign.Stretch);
		VerticalLayoutSlot.SetVerticalAlign(rowShell, LayoutVerticalAlign.Center);
		VerticalLayoutSlot.SetSizeMode(rowShell, LayoutSizeMode.Auto);

		rowLayout = HorizontalLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.HorizontalLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (!rowLayout)
		{
			rowShell.RemoveFromHierarchy();
			return null;
		}

		rowShell.AddChild(rowLayout);
		rowLayout.SetName(rowName + "Layout");
		rowLayout.AddHandler(this);
		SizeLayoutSlot.SetHorizontalAlign(rowLayout, LayoutHorizontalAlign.Stretch);
		SizeLayoutSlot.SetVerticalAlign(rowLayout, LayoutVerticalAlign.Stretch);
		SizeLayoutSlot.SetPadding(rowLayout, 0.0, 0.0, 0.0, 0.0);
		return rowShell;
	}

	protected SizeLayoutWidget CreateBs5SizedShell(HorizontalLayoutWidget rowLayout, string name, float width, bool fixedWidth, float left, float top, float right, float bottom)
	{
		if (!rowLayout)
			return null;

		WorkspaceWidget workspace = rowLayout.GetWorkspace();
		if (!workspace)
			return null;

		SizeLayoutWidget shell = SizeLayoutWidget.Cast(workspace.CreateWidgetInWorkspace(WidgetType.SizeLayoutWidgetTypeID, 0, 0, 0, 0, WidgetFlags.VISIBLE, null, 0));
		if (!shell)
			return null;

		rowLayout.AddChild(shell);
		shell.SetName(name);
		shell.AddHandler(this);
		if (fixedWidth)
		{
			shell.EnableWidthOverride(true);
			shell.SetWidthOverride(width);
			HorizontalLayoutSlot.SetSizeMode(shell, LayoutSizeMode.Auto);
		}
		else
		{
			HorizontalLayoutSlot.SetSizeMode(shell, LayoutSizeMode.Fill);
			HorizontalLayoutSlot.SetFillWeight(shell, 1.0);
		}
		HorizontalLayoutSlot.SetPadding(shell, left, top, right, bottom);
		HorizontalLayoutSlot.SetHorizontalAlign(shell, LayoutHorizontalAlign.Stretch);
		HorizontalLayoutSlot.SetVerticalAlign(shell, LayoutVerticalAlign.Center);
		return shell;
	}

	protected VerticalLayoutWidget GetBs5SettingsContent()
	{
		if (!m_wScroll)
			return null;

		VerticalLayoutWidget content = VerticalLayoutWidget.Cast(m_wScroll.FindAnyWidget("Content"));
		if (!content)
			content = VerticalLayoutWidget.Cast(m_wScroll.GetChildren());

		return content;
	}

	protected void RemoveExistingBs5ProceduralRows()
	{
		VerticalLayoutWidget content = GetBs5SettingsContent();
		if (!content)
			return;

		Widget child = content.GetChildren();
		while (child)
		{
			Widget next = child.GetSibling();
			if (IsBs5ProceduralRowName(child.GetName()))
				child.RemoveFromHierarchy();

			child = next;
		}

		ClearBs5RowWidgetRefs();
	}

	protected bool IsBs5ProceduralRowName(string name)
	{
		if (name == "Bs5SlapbackEnabledRow")
			return true;
		if (name == "Bs5TechnicalPresetRow")
			return true;
		if (name == "Bs5SoundPresetRow")
			return true;
		if (name == "Bs5EchoVolumeRow")
			return true;
		if (name == "Bs5SlapbackVolumeRow")
			return true;

		return false;
	}

	protected void ClearBs5RowWidgetRefs()
	{
		m_wBs5EchoVolumeRow = null;
		m_wBs5EchoVolumeRowLayout = null;
		m_wBs5EchoVolumeSlider = null;
		m_wBs5EchoVolumeLabel = null;
		m_wBs5EchoVolumeValue = null;

		m_wBs5SlapbackVolumeRow = null;
		m_wBs5SlapbackVolumeRowLayout = null;
		m_wBs5SlapbackVolumeSlider = null;
		m_wBs5SlapbackVolumeLabel = null;
		m_wBs5SlapbackVolumeValue = null;

		m_wBs5SlapbackEnabledRow = null;
		m_wBs5SlapbackEnabledCheckbox = null;
		m_wBs5SlapbackEnabledToggleButton = null;
		m_wBs5SlapbackEnabledNextButton = null;
		m_wBs5SlapbackEnabledLabel = null;
		m_wBs5SlapbackEnabledToggleText = null;
		m_wBs5SlapbackEnabledNextText = null;
		m_wBs5SlapbackEnabledValue = null;

		m_wBs5TechnicalPresetRow = null;
		m_wBs5TechnicalPresetPrevButton = null;
		m_wBs5TechnicalPresetNextButton = null;
		m_wBs5TechnicalPresetLabel = null;
		m_wBs5TechnicalPresetValue = null;
		m_wBs5TechnicalPresetPrevText = null;
		m_wBs5TechnicalPresetNextText = null;

		m_wBs5SoundPresetRow = null;
		m_wBs5SoundPresetPrevButton = null;
		m_wBs5SoundPresetNextButton = null;
		m_wBs5SoundPresetLabel = null;
		m_wBs5SoundPresetValue = null;
		m_wBs5SoundPresetPrevText = null;
		m_wBs5SoundPresetNextText = null;
	}

	protected void RefreshBs5EchoVolumeRow(bool force)
	{
		if (!m_wBs5EchoVolumeSlider)
			return;

		float percentValue = QuantizeBs5SliderPercent(BS5_PlayerAudioSettings.GetEchoVolume() * 100.0);
		if (force || Math.AbsFloat(percentValue - m_fBs5SliderPercent) > 0.1)
			m_wBs5EchoVolumeSlider.SetCurrent(percentValue);

		m_fBs5SliderPercent = percentValue;
		m_fBs5PendingEchoVolume = percentValue * 0.01;
		m_bBs5EchoVolumeDirty = false;
		UpdateBs5EchoVolumeValueText(percentValue);
	}

	protected void HandleBs5EchoVolumeChanged(bool finished)
	{
		if (!m_wBs5EchoVolumeSlider)
			return;

		float currentPercent = QuantizeBs5SliderPercent(m_wBs5EchoVolumeSlider.GetCurrent());
		UpdateBs5EchoVolumeValueText(currentPercent);

		if (Math.AbsFloat(currentPercent - m_fBs5SliderPercent) <= 0.1)
			return;

		m_fBs5SliderPercent = currentPercent;
		m_fBs5PendingEchoVolume = currentPercent * 0.01;
		m_bBs5EchoVolumeDirty = true;
		BS5_PlayerAudioSettings.SetEchoVolume(m_fBs5PendingEchoVolume, false);
		RefreshBs5PresetRows();
		if (finished)
			m_bBs5PresetSettingsDirty = true;
	}

	protected void FlushBs5EchoVolumeSetting()
	{
		if (!m_bBs5EchoVolumeDirty)
			return;

		BS5_PlayerAudioSettings.SetEchoVolume(m_fBs5PendingEchoVolume, false);
		m_bBs5EchoVolumeDirty = false;
		m_bBs5PresetSettingsDirty = true;
	}

	protected void RefreshBs5SlapbackVolumeRow(bool force)
	{
		if (!m_wBs5SlapbackVolumeSlider)
			return;

		float percentValue = QuantizeBs5SliderPercent(BS5_PlayerAudioSettings.GetSlapbackVolume() * 100.0);
		if (force || Math.AbsFloat(percentValue - m_fBs5SlapbackSliderPercent) > 0.1)
			m_wBs5SlapbackVolumeSlider.SetCurrent(percentValue);

		m_fBs5SlapbackSliderPercent = percentValue;
		m_fBs5PendingSlapbackVolume = percentValue * 0.01;
		m_bBs5SlapbackVolumeDirty = false;
		UpdateBs5SlapbackVolumeValueText(percentValue);
	}

	protected void HandleBs5SlapbackVolumeChanged(bool finished)
	{
		if (!m_wBs5SlapbackVolumeSlider)
			return;

		float currentPercent = QuantizeBs5SliderPercent(m_wBs5SlapbackVolumeSlider.GetCurrent());
		UpdateBs5SlapbackVolumeValueText(currentPercent);

		if (Math.AbsFloat(currentPercent - m_fBs5SlapbackSliderPercent) <= 0.1)
			return;

		m_fBs5SlapbackSliderPercent = currentPercent;
		m_fBs5PendingSlapbackVolume = currentPercent * 0.01;
		m_bBs5SlapbackVolumeDirty = true;
		BS5_PlayerAudioSettings.SetSlapbackVolume(m_fBs5PendingSlapbackVolume, false);
		RefreshBs5PresetRows();
		if (finished)
			m_bBs5PresetSettingsDirty = true;
	}

	protected void FlushBs5SlapbackVolumeSetting()
	{
		if (!m_bBs5SlapbackVolumeDirty)
			return;

		BS5_PlayerAudioSettings.SetSlapbackVolume(m_fBs5PendingSlapbackVolume, false);
		m_bBs5SlapbackVolumeDirty = false;
		m_bBs5PresetSettingsDirty = true;
	}

	protected void FlushBs5PresetSettings()
	{
		if (!m_bBs5PresetSettingsDirty)
			return;

		BS5_PlayerAudioSettings.Save();
		m_bBs5PresetSettingsDirty = false;
	}

	protected void RefreshBs5SlapbackEnabledRow()
	{
		bool enabled = BS5_PlayerAudioSettings.IsSlapbackEnabled();
		if (m_wBs5SlapbackEnabledCheckbox)
			m_wBs5SlapbackEnabledCheckbox.SetChecked(enabled);
		if (m_wBs5SlapbackEnabledToggleText)
			m_wBs5SlapbackEnabledToggleText.SetText("<");
		if (m_wBs5SlapbackEnabledNextText)
			m_wBs5SlapbackEnabledNextText.SetText(">");
		if (m_wBs5SlapbackEnabledValue)
		{
			if (enabled)
				m_wBs5SlapbackEnabledValue.SetText("Enabled");
			else
				m_wBs5SlapbackEnabledValue.SetText("Disabled");
		}
	}

	protected void RefreshBs5PresetRows()
	{
		if (m_wBs5TechnicalPresetValue)
			m_wBs5TechnicalPresetValue.SetText(BS5_PresetRegistry.GetTechnicalPresetDisplayName(BS5_PlayerAudioSettings.GetTechnicalPresetId()));

		if (m_wBs5SoundPresetValue)
			m_wBs5SoundPresetValue.SetText(BS5_PresetRegistry.GetSoundPresetDisplayName(BS5_PlayerAudioSettings.GetSoundPresetId()));
	}

	protected void StepBs5TechnicalPreset(int direction)
	{
		int count = BS5_PresetRegistry.GetTechnicalPresetCount();
		if (count <= 0)
			return;

		int index = BS5_PresetRegistry.GetTechnicalPresetIndex(BS5_PlayerAudioSettings.GetTechnicalPresetId());
		index += direction;
		if (index < 0)
			index = count - 1;
		else if (index >= count)
			index = 0;

		BS5_TechnicalPreset preset = BS5_PresetRegistry.GetTechnicalPresetByIndex(index);
		if (!preset)
			return;

		BS5_PresetRegistry.ApplyTechnicalPreset(preset.m_sId, false);
		m_bBs5PresetSettingsDirty = true;
		RefreshBs5SlapbackEnabledRow();
		RefreshBs5PresetRows();
	}

	protected void StepBs5SoundPreset(int direction)
	{
		int count = BS5_PresetRegistry.GetSoundPresetCount();
		if (count <= 0)
			return;

		int index = BS5_PresetRegistry.GetSoundPresetIndex(BS5_PlayerAudioSettings.GetSoundPresetId());
		index += direction;
		if (index < 0)
			index = count - 1;
		else if (index >= count)
			index = 0;

		BS5_SoundPreset preset = BS5_PresetRegistry.GetSoundPresetByIndex(index);
		if (!preset)
			return;

		FlushBs5EchoVolumeSetting();
		FlushBs5SlapbackVolumeSetting();
		BS5_PresetRegistry.ApplySoundPreset(preset.m_sId, false);
		m_bBs5PresetSettingsDirty = true;
		RefreshBs5EchoVolumeRow(true);
		RefreshBs5SlapbackVolumeRow(true);
		RefreshBs5PresetRows();
	}

	protected void UpdateBs5EchoVolumeValueText(float percentValue)
	{
		if (!m_wBs5EchoVolumeValue)
			return;

		int displayValue = (int)percentValue;
		m_wBs5EchoVolumeValue.SetText(string.Format("%1%%", displayValue));
	}

	protected void UpdateBs5SlapbackVolumeValueText(float percentValue)
	{
		if (!m_wBs5SlapbackVolumeValue)
			return;

		int displayValue = (int)percentValue;
		m_wBs5SlapbackVolumeValue.SetText(string.Format("%1%%", displayValue));
	}

	protected float QuantizeBs5SliderPercent(float percentValue)
	{
		percentValue = BS5_EchoMath.Clamp(percentValue, BS5_ECHO_VOLUME_MIN, BS5_ECHO_VOLUME_MAX);
		float steppedValue = Math.Floor((percentValue / BS5_ECHO_VOLUME_STEP) + 0.5) * BS5_ECHO_VOLUME_STEP;
		return BS5_EchoMath.Clamp(steppedValue, BS5_ECHO_VOLUME_MIN, BS5_ECHO_VOLUME_MAX);
	}

	protected void ApplyBs5EchoVolumeStyles()
	{
		VerticalLayoutWidget content = GetBs5SettingsContent();
		Widget sourceRow = FindBs5ReferenceAudioRow(content);
		SliderWidget sourceSlider = FindBs5FirstSliderWidget(sourceRow);
		if (sourceSlider && m_wBs5EchoVolumeSlider)
		{
			string sliderStyle = sourceSlider.GetStyleName();
			if (sliderStyle != string.Empty)
				m_wBs5EchoVolumeSlider.SetStyle(sliderStyle);
			m_wBs5EchoVolumeSlider.SetColorInt(sourceSlider.GetColorInt());
			m_wBs5EchoVolumeSlider.SetOpacity(sourceSlider.GetOpacity());
		}

		if (sourceSlider && m_wBs5SlapbackVolumeSlider)
		{
			string slapbackSliderStyle = sourceSlider.GetStyleName();
			if (slapbackSliderStyle != string.Empty)
				m_wBs5SlapbackVolumeSlider.SetStyle(slapbackSliderStyle);
			m_wBs5SlapbackVolumeSlider.SetColorInt(sourceSlider.GetColorInt());
			m_wBs5SlapbackVolumeSlider.SetOpacity(sourceSlider.GetOpacity());
		}

		array<ref TextWidget> sourceTexts = {};
		CollectBs5TextWidgets(sourceRow, sourceTexts);
		TextWidget sourceLabel = FindBs5LabelTextWidget(sourceTexts);
		TextWidget sourceValue = FindBs5ValueTextWidget(sourceTexts);

		ApplyBs5TextStyle(m_wBs5SlapbackEnabledLabel, sourceLabel);
		ApplyBs5TextStyle(m_wBs5SlapbackEnabledToggleText, sourceValue);
		ApplyBs5TextStyle(m_wBs5SlapbackEnabledNextText, sourceValue);
		ApplyBs5TextStyle(m_wBs5TechnicalPresetLabel, sourceLabel);
		ApplyBs5TextStyle(m_wBs5SoundPresetLabel, sourceLabel);
		ApplyBs5TextStyle(m_wBs5EchoVolumeLabel, sourceLabel);
		ApplyBs5TextStyle(m_wBs5SlapbackVolumeLabel, sourceLabel);

		ApplyBs5TextStyle(m_wBs5SlapbackEnabledValue, sourceValue);
		ApplyBs5TextStyle(m_wBs5TechnicalPresetValue, sourceValue);
		ApplyBs5TextStyle(m_wBs5SoundPresetValue, sourceValue);
		ApplyBs5TextStyle(m_wBs5EchoVolumeValue, sourceValue);
		ApplyBs5TextStyle(m_wBs5SlapbackVolumeValue, sourceValue);
		ApplyBs5TextStyle(m_wBs5TechnicalPresetPrevText, sourceValue);
		ApplyBs5TextStyle(m_wBs5TechnicalPresetNextText, sourceValue);
		ApplyBs5TextStyle(m_wBs5SoundPresetPrevText, sourceValue);
		ApplyBs5TextStyle(m_wBs5SoundPresetNextText, sourceValue);
	}

	protected void ApplyBs5TextStyle(TextWidget target, TextWidget sourceText)
	{
		if (!target || !sourceText)
			return;

		target.SetColorInt(sourceText.GetColorInt());
		target.SetOutline(sourceText.GetOutlineSize(), sourceText.GetOutlineColor());

		float shadowOffsetX;
		float shadowOffsetY;
		sourceText.GetShadowOffset(shadowOffsetX, shadowOffsetY);
		target.SetShadow(sourceText.GetShadowSize(), sourceText.GetShadowColor(), 1.0, shadowOffsetX, shadowOffsetY);
		target.SetItalic(sourceText.GetItalic());
		target.SetBold(sourceText.GetBold());
		target.SetForceFont(sourceText.GetForceFont());
		target.SetSharpness(sourceText.GetSharpness());
	}

	protected bool TryCloneBs5EchoVolumeRow(VerticalLayoutWidget content)
	{
		return false;
	}

	protected Widget FindBs5ReferenceAudioRow(VerticalLayoutWidget content)
	{
		Widget child = content.GetChildren();
		while (child)
		{
			if (!IsBs5ProceduralRowName(child.GetName()) && FindBs5FirstSliderWidget(child))
				return child;

			child = child.GetSibling();
		}

		return null;
	}

	protected SliderWidget FindBs5FirstSliderWidget(Widget root)
	{
		if (!root)
			return null;

		SliderWidget slider = SliderWidget.Cast(root);
		if (slider)
			return slider;

		Widget child = root.GetChildren();
		while (child)
		{
			slider = FindBs5FirstSliderWidget(child);
			if (slider)
				return slider;

			child = child.GetSibling();
		}

		return null;
	}

	protected void CollectBs5TextWidgets(Widget root, notnull array<ref TextWidget> textWidgets)
	{
		if (!root)
			return;

		TextWidget textWidget = TextWidget.Cast(root);
		if (textWidget)
			textWidgets.Insert(textWidget);

		Widget child = root.GetChildren();
		while (child)
		{
			CollectBs5TextWidgets(child, textWidgets);
			child = child.GetSibling();
		}
	}

	protected TextWidget FindBs5LabelTextWidget(notnull array<ref TextWidget> textWidgets)
	{
		foreach (TextWidget textWidget : textWidgets)
		{
			if (!textWidget)
				continue;

			string text = textWidget.GetText();
			if (text == string.Empty)
				continue;

			if (text.Contains("%"))
				continue;

			return textWidget;
		}

		return null;
	}

	protected TextWidget FindBs5ValueTextWidget(notnull array<ref TextWidget> textWidgets)
	{
		foreach (TextWidget textWidget : textWidgets)
		{
			if (!textWidget)
				continue;

			string text = textWidget.GetText();
			if (text == string.Empty)
				continue;

			if (text.Contains("%"))
				return textWidget;
		}

		if (textWidgets.Count() > 1)
			return textWidgets[textWidgets.Count() - 1];

		return null;
	}

	protected void ClearBs5ScriptHandlers(Widget widget)
	{
		if (!widget)
			return;

		for (int handlerIndex = widget.GetNumHandlers() - 1; handlerIndex >= 0; handlerIndex--)
		{
			ScriptedWidgetEventHandler handler = widget.GetHandler(handlerIndex);
			if (handler)
				widget.RemoveHandler(handler);
		}
	}
}
