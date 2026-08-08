#include "EditorGUI.h"

#include "PluginProcessor.h"
#include "GUI/ScopeComponent.hpp"

namespace {
	const std::int32_t KEY_HEIGHT = 80;
	const float KEY_WIDTH = 32.0f;
	const float KEY_SCROLL_WIDTH = 32.0f;
	const std::int32_t PANEL_MARGIN = 3;

}  // namespace

EditorGUI::EditorGUI(PluginProcessor& p)
	: AudioProcessorEditor(&p),
	processor(p),
	keyboardComponent(p.getKeyboardState(),
		MidiKeyboardComponent::Orientation::horizontalKeyboard),
	OscButton("WAVE", this),
	CycleButton("CYCLES", this),
	ArpButton("ARP", this),
	EffectButton("EFFECTS", this),


	chipOscComponent(&p.chipOscParameters, &p.waveformMemoryParameters),
	sweepParamsComponent(&p.sweepParameters),
	vibratoParamsComponent(&p.vibratoParameters),
	voicingParamsComponent(&p.voicingParameters),
	optionsParamsComponent(&p.optionsParameters),


	waveformMemoryParamsComponent(&p.waveformMemoryParameters, p, &p.chipOscParameters),

	_waveformMemoryParamsPtr(&p.waveformMemoryParameters),

	// wfmpc2
	//waveformMemoryParamsComponent2(&p.waveformMemoryParameters2),



	midiEchoParamsComponent(&p.midiEchoParameters),
	filterParamsComponent(&p.filterParameters),

	wavePatternsComponent(&p.wavePatternParameters, &p.chipOscParameters, &p.waveformMemoryParameters),
	arpSequencer(&p.arpParameters),
	scopeComponent(p.getAudioBufferQueue()) {
	/*
			TabComponentを使いたかったが，Tabだとメモリリークが収まらないので現在の形に．
			デストラクタ時にメモリリーク，CustomLookAndFeelまわりでエラーが取れない．
			安定化のためにアルファ切り替えによるGUI管理方式に変更


	  I wanted to use TabComponent, but since memory leaks persisted with tabs, I settled on the current design.
	  Memory leaks occur during the destructor, and I can't catch errors related to CustomLookAndFeel.
	  To ensure stability, I switched to a GUI management method that uses alpha blending.



	*/

		{
			addAndMakeVisible(keyboardComponent);
			keyboardComponent.setKeyWidth(KEY_WIDTH);
			keyboardComponent.setScrollButtonWidth(KEY_SCROLL_WIDTH);



			addAndMakeVisible(OscButton);
			addAndMakeVisible(CycleButton);
			addAndMakeVisible(ArpButton);
			addAndMakeVisible(EffectButton);
			OscButton.setToggleState(true);
			ArpButton.setToggleState(false);
			CycleButton.setToggleState(false);
			EffectButton.setToggleState(false);
		}
		{
			addAndMakeVisible(chipOscComponent);
			addAndMakeVisible(sweepParamsComponent);
			addAndMakeVisible(vibratoParamsComponent);
			addAndMakeVisible(voicingParamsComponent);
			addAndMakeVisible(optionsParamsComponent);
			addAndMakeVisible(waveformMemoryParamsComponent);

			addAndMakeVisible(scopeComponent);
			addAndMakeVisible(wavePatternsComponent);
			addAndMakeVisible(arpSequencer);
		}
		{
			addAndMakeVisible(midiEchoParamsComponent);
			addAndMakeVisible(filterParamsComponent);
			addAndMakeVisible(scopeComponent);
		}
		setResizable(true, true);
		setSize(960, 540 + KEY_HEIGHT);

		setResizeLimits(640, 432, 1088, 612);

		{
			customLookAndFeel = new LookAndFeel_V4(LookAndFeel_V4::getLightColourScheme());

			customLookAndFeel->setColour(TextButton::ColourIds::buttonColourId, Colour(22, 25, 30));
			customLookAndFeel->setColour(TextButton::ColourIds::buttonOnColourId, Colour(22, 25, 30));
			customLookAndFeel->setColour(TextButton::ColourIds::textColourOffId, Colour(201, 213, 219));
			customLookAndFeel->setColour(TextButton::ColourIds::textColourOnId, Colour(201, 213, 219));

			//customLookAndFeel->setColour(Slider::ColourIds::trackColourId, Colour(200, 48, 48));
			//customLookAndFeel->setColour(Slider::ColourIds::thumbColourId, Colour(137, 140, 149));
			//customLookAndFeel->setColour(Slider::ColourIds::backgroundColourId, Colour(255, 255, 255));
			//customLookAndFeel->setColour(Slider::ColourIds::textBoxBackgroundColourId, Colour(45, 52, 57));
			//customLookAndFeel->setColour(Slider::ColourIds::textBoxTextColourId, Colour(201, 213, 219));

			customLookAndFeel->setColour(ToggleButton::ColourIds::textColourId, Colour(201, 213, 219));
			customLookAndFeel->setColour(ToggleButton::ColourIds::tickColourId, Colour(201, 213, 219));
			customLookAndFeel->setColour(ToggleButton::ColourIds::tickDisabledColourId, Colours::grey);

			customLookAndFeel->setColour(ComboBox::ColourIds::arrowColourId, Colour(174, 187, 193));
			customLookAndFeel->setColour(ComboBox::ColourIds::backgroundColourId, Colour(22, 25, 30));
			customLookAndFeel->setColour(ComboBox::ColourIds::buttonColourId, Colours::red);
			customLookAndFeel->setColour(ComboBox::ColourIds::focusedOutlineColourId, Colours::white);
			customLookAndFeel->setColour(ComboBox::ColourIds::outlineColourId, Colours::lightgrey);
			customLookAndFeel->setColour(ComboBox::ColourIds::textColourId, Colours::white);

			customLookAndFeel->setColour(Label::textColourId, Colours::white);

			for (Component* child : AudioProcessorEditor::getChildren()) {
				child->setLookAndFeel(customLookAndFeel);
			}

		}

		// init each GUI Alpha
		{
			scopeComponent.setVisible(true);

			chipOscComponent.setVisible(true);
			sweepParamsComponent.setVisible(true);
			vibratoParamsComponent.setVisible(true);
			voicingParamsComponent.setVisible(true);
			optionsParamsComponent.setVisible(true);
			waveformMemoryParamsComponent.setVisible(true);

			// wfmpc2
			//waveformMemoryParamsComponent2.setvisible(true);


			midiEchoParamsComponent.setVisible(false);
			filterParamsComponent.setVisible(false);
			wavePatternsComponent.setVisible(false);
			arpSequencer.setVisible(false);
		}

		chipOscComponent.onWaveformChanged = [this](int index) {
			DBG("onWaveformChanged called with: " + String(index));
			DBG("getNumWaveforms: " + String(_waveformMemoryParamsPtr->getNumWaveforms()));

			if (index >= 0 && index < _waveformMemoryParamsPtr->getNumWaveforms()) {
				DBG("exists");
				waveformMemoryParamsComponent.setWaveformIndex(index);
			}
			else {
				DBG("invalid waveform index: " + String(index) + ", max: " + String(_waveformMemoryParamsPtr->getNumWaveforms()));
			}
			};

		waveformMemoryParamsComponent.onAddWaveform = [this](const String& waveformName) {
			chipOscComponent.refreshWaveformList();
			wavePatternsComponent.refreshWaveformList();

			int newestIndex = _waveformMemoryParamsPtr->getNumWaveforms() - 1;
			waveformMemoryParamsComponent.setWaveformIndex(newestIndex);

			DBG("added '" + waveformName + "' and switched to it");
			};

		wavePatternsComponent.onSwitchWaveform = [this](const int& index) {
			chipOscComponent.refreshWaveformList();
			wavePatternsComponent.refreshWaveformList();
			//DBG("running new function");
			chipOscComponent.onWaveformChanged(index - 16);
			//DBG("switched to " + index);
			};

		waveformMemoryParamsComponent.requestChange = [this](const int& index) {
			chipOscComponent.refreshWaveformList();
			wavePatternsComponent.refreshWaveformList();

			chipOscComponent.onWaveformChanged(index - 16);
			};
}

EditorGUI::~EditorGUI() {
	for (Component* child : getChildren()) {
		child->setLookAndFeel(nullptr);
	}

	optionsParamsComponent.setLookAndFeel(nullptr);

	delete customLookAndFeel;
}



void EditorGUI::paint(Graphics& g) {
	juce::ColourGradient gradient(
		juce::Colour(37, 37, 39), 0.0f, 0.0f,
		juce::Colour(28, 28, 29), 0.0f, 350.0f,
		false);
	g.setGradientFill(gradient);
	g.fillAll();
	g.setColour(Colours::white);
	g.setFont(Font(32, Font::bold));
	std::string VersionName = "SANAex UI ";
	VersionName += JucePlugin_VersionString;
	g.drawFittedText(VersionName, AudioProcessorEditor::getLocalBounds(),
		Justification::topRight, 1);


}


void EditorGUI::resized() {
	Rectangle<int> bounds = getLocalBounds();

	keyboardComponent.setBounds(bounds.removeFromBottom(KEY_HEIGHT));

	{
		Rectangle<int> area = bounds.removeFromTop(40);
		OscButton.setBounds(area.removeFromLeft(80));
		CycleButton.setBounds(area.removeFromLeft(80));
		ArpButton.setBounds(area.removeFromLeft(80));
		EffectButton.setBounds(area.removeFromLeft(80));
	}

	// Oscillator Page (WAVE)
	if (OscButton.button.getToggleState() == true) {
		Rectangle<int> mainbounds = bounds;
		mainbounds.translate(0, -6);
		{
			Rectangle<int> leftArea = mainbounds.removeFromLeft(bounds.getWidth() * 0.45f);
			auto fullLeftPanelArea = leftArea.reduced(PANEL_MARGIN);
			chipOscComponent.setBounds(fullLeftPanelArea);
			int scopeHeight = 245;
			auto scopeArea = fullLeftPanelArea.removeFromBottom(scopeHeight).reduced(30, 30);
			scopeComponent.setBounds(scopeArea.translated(2.5, 5));
			scopeComponent.toFront(false);

		}
		{
			Rectangle<int> rightArea = mainbounds;
			waveformMemoryParamsComponent.setBounds(
				rightArea.reduced(PANEL_MARGIN)
			);
		}
	}

	if (EffectButton.button.getToggleState() == true) {
		Rectangle<int> mainbounds = bounds;
		mainbounds.translate(0, -6);

		auto rowHeight = mainbounds.getHeight() / 3;
		auto colWidth = mainbounds.getWidth() / 2;

		{
			Rectangle<int> leftArea = mainbounds.removeFromLeft(bounds.getWidth() * 0.45f);
			auto fullLeftPanelArea = leftArea.reduced(PANEL_MARGIN);
			vibratoParamsComponent.setBounds(fullLeftPanelArea);

			int sweepHeight = 150;
			fullLeftPanelArea.removeFromBottom(0);

			auto sweepArea = fullLeftPanelArea.removeFromBottom(sweepHeight);
			sweepParamsComponent.setBounds(sweepArea);
			sweepParamsComponent.toFront(false);

			auto voicingArea = fullLeftPanelArea.removeFromBottom(sweepHeight);
			voicingParamsComponent.setBounds(voicingArea.translated(0, 34));
			voicingParamsComponent.toFront(false);
		}


		//{
		//	auto rowBounds = mainbounds.removeFromTop(rowHeight);
		//	auto leftBox = rowBounds.removeFromLeft(colWidth);
		//	auto rightBox = rowBounds.reduced(PANEL_MARGIN);

		//	vibratoParamsComponent.setBounds(leftBox);

		//}



			//Rectangle<int> rightArea = mainbounds.reduced(PANEL_MARGIN);

			//auto topArea = rightArea.removeFromTop(rightArea.getHeight() / 2);
			//auto bottomArea = rightArea;

			//optionsParamsComponent.setBounds(topArea);
			//filterParamsComponent.setBounds(bottomArea);

			//midiEchoParamsComponent.setBounds(leftBox);



		{

			Rectangle<int> rightArea = mainbounds.reduced(PANEL_MARGIN);
			auto fullRightPanelArea = rightArea.reduced(PANEL_MARGIN);
			optionsParamsComponent.setBounds(fullRightPanelArea);

			int sweepHeight = 290;
			fullRightPanelArea.removeFromBottom(0);

			auto sweepArea = fullRightPanelArea.removeFromBottom(sweepHeight);
			filterParamsComponent.setBounds(sweepArea);
			filterParamsComponent.toFront(false);

			//auto voicingArea = fullRightPanelArea.removeFromBottom(sweepHeight);
			//filterParamsComponent.setBounds(voicingArea.translated(0, 34));
			//filterParamsComponent.toFront(false);

		}

	}

	if (CycleButton.button.getToggleState() == true)
	{
		Rectangle<int> mainbounds = bounds;
		mainbounds.translate(0, -6);


		Rectangle<int> leftArea = mainbounds.removeFromLeft(bounds.getWidth() * 0.45f);
		wavePatternsComponent.setBounds(leftArea.reduced(PANEL_MARGIN));
		int waveformHeight = mainbounds.getHeight() * 1.f;
		Rectangle<int> rightTopArea = mainbounds.removeFromTop(waveformHeight);
		waveformMemoryParamsComponent.setBounds(rightTopArea.reduced(PANEL_MARGIN));
		//scopeComponent.setBounds(mainbounds.reduced(PANEL_MARGIN));
	}

	if (ArpButton.button.getToggleState() == true)
	{
		Rectangle<int> mainbounds = bounds;
		mainbounds.translate(0, -6);

		{
			Rectangle<int> leftArea = mainbounds.removeFromLeft(bounds.getWidth() * 0.45f);
			auto fullLeftPanelArea = leftArea.reduced(PANEL_MARGIN);
			chipOscComponent.setBounds(fullLeftPanelArea);

			int scopeHeight = 245;
			auto scopeArea = fullLeftPanelArea.removeFromBottom(scopeHeight).reduced(30, 30);
			scopeComponent.setBounds(scopeArea.translated(2.5, 5));
			scopeComponent.toFront(false);
		}
		{
			Rectangle<int> rightArea = mainbounds;
			arpSequencer.setBounds(
				rightArea.reduced(PANEL_MARGIN)
			);
		}
	}
}

void EditorGUI::buttonClicked(Button* button) {
	{
		chipOscComponent.setVisible(false);
		sweepParamsComponent.setVisible(false);
		vibratoParamsComponent.setVisible(false);
		voicingParamsComponent.setVisible(false);
		optionsParamsComponent.setVisible(false);
		waveformMemoryParamsComponent.setVisible(false);
		midiEchoParamsComponent.setVisible(false);
		filterParamsComponent.setVisible(false);
		scopeComponent.setVisible(false);
		wavePatternsComponent.setVisible(false);
		arpSequencer.setVisible(false);
		OscButton.setToggleState(false);
		EffectButton.setToggleState(false);
		ArpButton.setToggleState(false);
		CycleButton.setToggleState(false);
	}

	if (button == &OscButton.button) {
		chipOscComponent.setVisible(true);
		waveformMemoryParamsComponent.setVisible(true);



		scopeComponent.setVisible(true);


		OscButton.setToggleState(true);
	}
	else if (button == &EffectButton.button) {
		//chipOscComponent.setVisible(true);
		//scopeComponent.setVisible(true);

		sweepParamsComponent.setVisible(true);
		vibratoParamsComponent.setVisible(true);
		//midiEchoParamsComponent.setVisible(true);
		filterParamsComponent.setVisible(true);
		voicingParamsComponent.setVisible(true);
		optionsParamsComponent.setVisible(true);

		EffectButton.setToggleState(true);
	}
	else if (button == &CycleButton.button) {
		wavePatternsComponent.setVisible(true);
		waveformMemoryParamsComponent.setVisible(true);
		//scopeComponent.setVisible(true);
		CycleButton.setToggleState(true);
	}

	else if (button == &ArpButton.button) {
		arpSequencer.setVisible(true);
		waveformMemoryParamsComponent.setVisible(false);
		chipOscComponent.setVisible(true);
		scopeComponent.setVisible(true);
		ArpButton.setToggleState(true);
	}
	resized();
}

