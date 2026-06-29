#include "ParametersComponent.h"

namespace {
	const Colour PANEL_COLOUR() { return Colour(44, 10, 56); }
	const Colour HEADER_COLOUR() { return Colours::white; }
	const Colour FONT_COLOUR() { return Colours::black; }
	const Colour TEXT_COLOUR() { return Colours::white; }
	const Colour BACKGROUND_COLOUR() { return Colour(44, 51, 56); }

	const float MIN_DELTA = 0.0001f;
	const float HEADER_HEIGHT = 24.0f;
	const std::int32_t LOCAL_MARGIN = 2;

	static File preFilePath = File::getSpecialLocation(File::currentApplicationFile);
}

static File getPreFileDirectory(File* file) {
	if (file->isDirectory()) {
		return *file;
	}
	else {
		return file->getParentDirectory();
	}
}

static std::vector<std::string> split(std::string str, char del) {
	auto first = 0;
	auto last = str.find_first_of(del);

	std::vector<std::string> result;

	while (first < str.size()) {
		std::string subStr(str, first, last - first);

		result.push_back(subStr);

		first = last + 1;
		last = str.find_first_of(del, first);

		if (last == std::string::npos) {
			last = str.size();
		}
	}
	return result;
}


// This is where I would have to change some of the UI at, this I think
// points to each font



static void paintHeader(Graphics& g, Rectangle<int> bounds, std::string text) {
	{ // 枠の描画
		auto x = 0.0f,
			y = HEADER_HEIGHT - 2;
		auto width = (float)bounds.getWidth(),
			height = (float)bounds.getHeight() - y;
		auto cornerSize = 9.0f,
			thickness = 0.0f;
		g.setColour(Colour(36, 40, 43));
		g.fillRoundedRectangle(x, y, width, height, cornerSize);
		g.setColour(Colour(22, 25, 30));
		g.drawRoundedRectangle(x, y, width, height, cornerSize, thickness);


	}

	{ // ヘッダー描画
		g.setFont(ProjectFonts::headerFont(32.0f));
		auto x = 1.f,
			y = -5.0f;
		auto width = (float)g.getCurrentFont().getStringWidth(text),
			height = HEADER_HEIGHT;

		g.setColour(BACKGROUND_COLOUR());
		g.fillRect(x, y, width, height);
		g.setColour(Colour(165, 178, 184));
		g.drawText(text, x, y, width, height, Justification::centred, false);
	}
}



static void paintHeaderEffects(Graphics& g, Rectangle<int> bounds, std::string text) {
	{ // 枠の描画
		auto x = 0.0f,
			y = HEADER_HEIGHT - 2;
		auto width = (float)bounds.getWidth(),
			height = (float)bounds.getHeight() - y;
		auto cornerSize = 9.0f,
			thickness = 0.0f;
		g.setColour(Colour(36, 40, 43));
		g.fillRoundedRectangle(x, y, width, height, cornerSize);
		g.setColour(Colour(22, 25, 30));
		g.drawRoundedRectangle(x, y, width, height, cornerSize, thickness);


	}

	{ // ヘッダー描画
		g.setFont(ProjectFonts::headerFont(32.0f));
		auto x = 1.f,
			y = -5.0f;
		auto width = (float)g.getCurrentFont().getStringWidth(text),
			height = HEADER_HEIGHT;

		g.setColour(BACKGROUND_COLOUR());
		g.fillRect(x, y, width, height);
		g.setColour(Colour(165, 178, 184));
		g.drawText(text, x, y, width, height, Justification::centred, false);
	}
}


static void paintHeaderWaveFormMemory(Graphics& g, Rectangle<int> bounds, std::string text) {
	{ // 枠の描画
		auto x = 0.0f, y = HEADER_HEIGHT;
		auto width = (float)bounds.getWidth(),
			height = (float)bounds.getHeight();
		auto cornerSize = 0.0f,
			thickness = 2.0f;
		g.setColour(Colour(26, 33, 38));
		g.drawRoundedRectangle(x, y, width, height, cornerSize, thickness);
	}

	{ // ヘッダー描画
		g.setFont(ProjectFonts::headerFont(32.0f));
		auto x = 1.f, y = -5.0f;
		auto width = (float)g.getCurrentFont().getStringWidth(text),
			height = HEADER_HEIGHT;

		g.setColour(BACKGROUND_COLOUR());
		g.fillRect(x, y, width, height);
		g.setColour(Colour(165, 178, 184));
		g.drawText(text, x, y, width, height, Justification::centred, false);
	}
}

static void saveWaveFile(WaveformMemoryParameters* _waveformMemoryParamsPtr,
	int waveformIndex,
	std::unique_ptr<FileChooser>& fc) {
	auto chooser = new FileChooser("Save Waveform", preFilePath, "*.wfm");
	fc.reset(chooser);
	fc->launchAsync(FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles,
		[_waveformMemoryParamsPtr, waveformIndex](const FileChooser& chooser) {
			const auto result = chooser.getResult();
			if (result.getFullPathName() == "") {
				return;
			}
			File newFile(result);
			newFile.create();
			newFile.replaceWithText("");

			AudioParameterInt** waveArray = _waveformMemoryParamsPtr->getWaveformArray(waveformIndex);

			for (int i = 0; i < WAVESAMPLE_LENGTH; ++i) {
				newFile.appendText(std::to_string(*waveArray[i]));
				newFile.appendText(" ");
			}
			preFilePath = chooser.getResult();
		}
	);
}

static void loadWaveFile(WaveformMemoryParameters* _waveformMemoryParamsPtr,
	int waveformIndex,
	std::unique_ptr<FileChooser>& fc) {
	auto chooser = new FileChooser("Load Waveform", preFilePath, "*.wfm");
	fc.reset(chooser);
	fc->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
		[_waveformMemoryParamsPtr, waveformIndex](const FileChooser& chooser) {
			const auto result = chooser.getResult();
			if (result.getFullPathName() == "") {
				return;
			}
			File waveformFile(result);
			std::string data = waveformFile.loadFileAsString().toStdString();

			AudioParameterInt** waveArray = _waveformMemoryParamsPtr->getWaveformArray(waveformIndex);

			auto count = 0;
			for (const auto subStr : split(data, ' ')) {
				if (count < WAVESAMPLE_LENGTH) {
					*waveArray[count] = atoi(subStr.c_str());
					++count;
				}
			}
			preFilePath = chooser.getResult();
		}
	);
}


// UI LEFT BUTTONS


ChipOscillatorComponent::ChipOscillatorComponent(ChipOscillatorParameters* oscParams, WaveformMemoryParameters* waveformMemoryParams)
	: _oscParamsPtr(oscParams),
	_waveformMemoryParamsPtr(waveformMemoryParams),
	waveTypeSelector("Waves", _oscParamsPtr->OscWaveType, this),
	volumeLevelSlider("Volume", "dB", _oscParamsPtr->VolumeLevel, this, 0.01f),
	attackSlider("Attack", "sec", _oscParamsPtr->Attack, this, MIN_DELTA, 1.0f),
	decaySlider("Decay", "sec", _oscParamsPtr->Decay, this, MIN_DELTA, 1.0f),
	sustainSlider("Sustain", "", _oscParamsPtr->Sustain, this, MIN_DELTA),
	releaseSlider("Release", "sec", _oscParamsPtr->Release, this, MIN_DELTA, 1.0f) {
	//colorTypeSelector("Arp", _oscParamsPtr->ColorType, this),
	//colorDurationSlider("Duration", "sec", _oscParamsPtr->ColorDuration, this, MIN_DELTA, 0.2f) 

	addAndMakeVisible(waveTypeSelector);
	addAndMakeVisible(volumeLevelSlider);
	addAndMakeVisible(attackSlider);
	addAndMakeVisible(decaySlider);
	addAndMakeVisible(sustainSlider);
	addAndMakeVisible(releaseSlider);
	//addAndMakeVisible(colorTypeSelector);
	//addAndMakeVisible(colorDurationSlider);

	//colorDurationSlider.LABEL_WIDTH = 0.f;

	refreshWaveformList();
}

void ChipOscillatorComponent::paint(Graphics& g) {
	paintHeader(g, getLocalBounds(), "Waves");
}

void ChipOscillatorComponent::resized() {
	float rowSize = 6.0f;
	auto compHeight = ((getHeight() - HEADER_HEIGHT) / rowSize);

	Rectangle<int> bounds = getLocalBounds();  // コンポーネント基準の値
	bounds.removeFromTop(HEADER_HEIGHT);

	waveTypeSelector.setBounds(bounds.removeFromTop(compHeight));
	volumeLevelSlider.setBounds(bounds.removeFromTop(compHeight));
	attackSlider.setBounds(bounds.removeFromTop(compHeight));
	decaySlider.setBounds(bounds.removeFromTop(compHeight));
	sustainSlider.setBounds(bounds.removeFromTop(compHeight));
	releaseSlider.setBounds(bounds.removeFromTop(compHeight));
	/*{
	  auto area = bounds.removeFromTop(compHeight);
	  auto width = area.getWidth() / 2.0f;
	  colorTypeSelector.setBounds(area.removeFromLeft(width));
	  colorDurationSlider.setBounds(area);
	}*/
}

void ChipOscillatorComponent::refreshWaveformList() {
	StringArray fullList;
	fullList.add("NES_Square50%");
	fullList.add("NES_Square25%");
	fullList.add("NES_Square12.5%");
	fullList.add("NES_Triangle");
	fullList.add("Pure_Square50%");
	fullList.add("Pure_Square25%");
	fullList.add("Pure_Square12.5%");
	fullList.add("Pure_Triangle");
	fullList.add("Pure_Sine");
	fullList.add("Pure_Saw");
	fullList.add("NES_LongNoise");
	fullList.add("NES_ShortNoise");
	fullList.add("Pure_Noise");
	fullList.add("Rough_Sine");
	fullList.add("Rough_Saw");
	fullList.add("Rough_Noise");

	for (int w = 1; w <= _waveformMemoryParamsPtr->getNumWaveforms(); ++w) {
		if (w == 1) {
			fullList.add("Waveform Memory");
		}
		else {
			fullList.add("Waveform " + String(w));
		}
	}

	waveTypeSelector.selector.clear(dontSendNotification);
	waveTypeSelector.selector.addItemList(fullList, 1);
	int currentIdx = (int)*_oscParamsPtr->OscWaveType;
	waveTypeSelector.selector.setSelectedItemIndex(currentIdx, dontSendNotification);
}

void ChipOscillatorComponent::timerCallback() {
	static int lastNumWaveformsOsc = 0;
	if (_waveformMemoryParamsPtr->getNumWaveforms() != lastNumWaveformsOsc) {
		lastNumWaveformsOsc = _waveformMemoryParamsPtr->getNumWaveforms();
		refreshWaveformList();
	}

	// For waveform selector, use currentWaveformName instead of the parameter
	int paramIndex = (int)*_oscParamsPtr->OscWaveType;
	if (waveTypeSelector.selector.getSelectedItemIndex() != paramIndex) {
		waveTypeSelector.selector.setSelectedItemIndex(paramIndex, dontSendNotification);
	}

	volumeLevelSlider.setValue(_oscParamsPtr->VolumeLevel->get());
	attackSlider.setValue(_oscParamsPtr->Attack->get());
	decaySlider.setValue(_oscParamsPtr->Decay->get());
	sustainSlider.setValue(_oscParamsPtr->Sustain->get());
	releaseSlider.setValue(_oscParamsPtr->Release->get());

	//// Only update color selector if it changed
	//int newColorIndex = _oscParamsPtr->ColorType->getIndex();
	//if (colorTypeSelector.selector.getSelectedItemIndex() != newColorIndex) {
	//    colorTypeSelector.setSelectedItemIndex(newColorIndex);
	//}

	//colorDurationSlider.setValue(_oscParamsPtr->ColorDuration->get());
}

void ChipOscillatorComponent::sliderValueChanged(Slider* slider) {
	if (slider == &volumeLevelSlider.slider) {
		*_oscParamsPtr->VolumeLevel = (float)volumeLevelSlider.getValue();
	}
	else if (slider == &attackSlider.slider) {
		*_oscParamsPtr->Attack = (float)attackSlider.getValue();
	}
	else if (slider == &decaySlider.slider) {
		*_oscParamsPtr->Decay = (float)decaySlider.getValue();
	}
	else if (slider == &sustainSlider.slider) {
		*_oscParamsPtr->Sustain = (float)sustainSlider.getValue();
	}
	else if (slider == &releaseSlider.slider) {
		*_oscParamsPtr->Release = (float)releaseSlider.getValue();
	}

	/*else if (slider == &colorDurationSlider.slider) {
	  *_oscParamsPtr->ColorDuration = (float)colorDurationSlider.getValue();
	}*/
}



void ChipOscillatorComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged) {
	if (comboBoxThatHasChanged == &waveTypeSelector.selector) {
		int waveIndex = waveTypeSelector.selector.getSelectedItemIndex();
		*_oscParamsPtr->OscWaveType = waveIndex;

		/*DBG("waveform: " + waveTypeSelector.selector.getText());
		DBG("waveform index: " + String(waveIndex));
		DBG("waveform text: " + waveTypeSelector.selector.getText());*/

		if (waveIndex >= 16 && onWaveformChanged) {
			int customIndex = waveIndex - 16;
			onWaveformChanged(customIndex);
		}
		waveTypeSelector.selector.setText(waveTypeSelector.selector.getText(), dontSendNotification);
	}
	//else if (comboBoxThatHasChanged == &colorTypeSelector.selector) {
	//    int colorIndex = colorTypeSelector.selector.getSelectedItemIndex();
	//    *_oscParamsPtr->ColorType = colorIndex;

	//    //DBG("Color: " + colorTypeSelector.selector.getText());
	//    //DBG("Color index: " + String(colorIndex));
	//}
}


SweepParametersComponent::SweepParametersComponent(SweepParameters* sweepParams)
	: _sweepParamsPtr(sweepParams),
	sweepSwitchSelector("SweepType", _sweepParamsPtr->SweepSwitch, this),
	timeSlider("Speed", "sec", _sweepParamsPtr->SweepTime, this, 0.01f, 1.0f) {
	addAndMakeVisible(sweepSwitchSelector);
	addAndMakeVisible(timeSlider);
}

void SweepParametersComponent::paint(Graphics& g) {
	paintHeaderEffects(g, getLocalBounds(), "PITCH SWEEP");
}

void SweepParametersComponent::resized() {
	float rowSize = 1.0f;
	float divide = 1.0f / rowSize;
	std::int32_t compHeight =
		std::int32_t((getHeight() - HEADER_HEIGHT) * divide);

	Rectangle<int> bounds = getLocalBounds();  // コンポーネント基準の値
	bounds.removeFromTop(HEADER_HEIGHT);

	{
		float alpha = isEditable() ? 1.0f : 0.4f;
		timeSlider.setAlpha(alpha);
	}
	sweepSwitchSelector.setBounds(bounds.removeFromTop(compHeight * 0.6f));
	timeSlider.setBounds(bounds.removeFromTop(compHeight));
}

void SweepParametersComponent::timerCallback() {
	sweepSwitchSelector.setSelectedItemIndex(
		_sweepParamsPtr->SweepSwitch->getIndex());
	timeSlider.setValue(_sweepParamsPtr->SweepTime->get());
}

void SweepParametersComponent::sliderValueChanged(Slider* slider) {
	if (slider == &timeSlider.slider) {
		*_sweepParamsPtr->SweepTime = (float)timeSlider.getValue();
	}
}

void SweepParametersComponent::comboBoxChanged(
	ComboBox* comboBoxThatHasChanged) {
	if (comboBoxThatHasChanged == &sweepSwitchSelector.selector) {
		*_sweepParamsPtr->SweepSwitch = sweepSwitchSelector.getSelectedItemIndex();
	}
	resized();
}

bool SweepParametersComponent::isEditable() {
	return (_sweepParamsPtr->SweepSwitch->getIndex() >= 1) ? true : false;
}

VibratoParametersComponent::VibratoParametersComponent(
	VibratoParameters* vibratoParams)
	: _vibratoParamsPtr(vibratoParams),
	enableSwitch("On", _vibratoParamsPtr->VibratoEnable, this),
	attackDeleySwitch("Attack-Deley-Switch", _vibratoParamsPtr->VibratoAttackDeleySwitch, this),
	amountSlider("Depth", "HarfTone", _vibratoParamsPtr->VibratoAmount, this, MIN_DELTA, 2.0f),
	speedSlider("Speed", "hz", _vibratoParamsPtr->VibratoSpeed, this, MIN_DELTA, 10.0f),
	attackDeleyTimeSlider("Attack", "sec", _vibratoParamsPtr->VibratoAttackTime, this, 0.001f, 2.0f) {
	addAndMakeVisible(enableSwitch);
	addAndMakeVisible(attackDeleySwitch);
	addAndMakeVisible(amountSlider);
	addAndMakeVisible(speedSlider);
	addAndMakeVisible(attackDeleyTimeSlider);

	// 切り替えSwitchのカスタマイズ
	{ attackDeleySwitch.button.setButtonText("Attack / Delay"); }
}

void VibratoParametersComponent::paint(Graphics& g) {
	paintHeaderEffects(g, getLocalBounds(), "VIBRATO");
}

void VibratoParametersComponent::resized() {
	float rowSize = 4.0f;
	float divide = 1.0f / rowSize;
	std::int32_t compHeight = std::int32_t((getHeight() - HEADER_HEIGHT) * divide);
	Rectangle<int> bounds = getLocalBounds();  // コンポーネント基準の値
	bounds.removeFromTop(HEADER_HEIGHT);
	{
		float alpha = isEditable() ? 1.0f : 0.4f;
		amountSlider.setAlpha(alpha);
		speedSlider.setAlpha(alpha);
		attackDeleyTimeSlider.setAlpha(alpha);
	}
	{
		auto b = bounds.removeFromTop(compHeight);
		enableSwitch.setBounds(b.removeFromLeft(b.getWidth() * 0.3f));
		attackDeleySwitch.setBounds(b);
	}
	amountSlider.setBounds(bounds.removeFromTop(compHeight));
	speedSlider.setBounds(bounds.removeFromTop(compHeight));
	attackDeleyTimeSlider.setBounds(bounds.removeFromTop(compHeight));

	//切り替えスイッチ処理
	if (_vibratoParamsPtr->VibratoAttackDeleySwitch->get() == true) {
		attackDeleyTimeSlider.label.setText("Attack", dontSendNotification);
	}
	else {
		attackDeleyTimeSlider.label.setText("Delay", dontSendNotification);
	}
}

void VibratoParametersComponent::timerCallback() {
	enableSwitch.setToggleState(_vibratoParamsPtr->VibratoEnable->get());
	attackDeleySwitch.setToggleState(
		_vibratoParamsPtr->VibratoAttackDeleySwitch->get());
	amountSlider.setValue(_vibratoParamsPtr->VibratoAmount->get());
	speedSlider.setValue(_vibratoParamsPtr->VibratoSpeed->get());
	attackDeleyTimeSlider.setValue(_vibratoParamsPtr->VibratoAttackTime->get());
}

void VibratoParametersComponent::sliderValueChanged(Slider* slider) {
	if (slider == &amountSlider.slider) {
		*_vibratoParamsPtr->VibratoAmount = (float)amountSlider.getValue();
	}
	else if (slider == &speedSlider.slider) {
		*_vibratoParamsPtr->VibratoSpeed = (float)speedSlider.getValue();
	}
	else if (slider == &attackDeleyTimeSlider.slider) {
		*_vibratoParamsPtr->VibratoAttackTime =
			(float)attackDeleyTimeSlider.getValue();
	}
}

void VibratoParametersComponent::buttonClicked(Button* button) {
	if (button == &enableSwitch.button) {
		*_vibratoParamsPtr->VibratoEnable = enableSwitch.getToggleState();
	}
	else if (button == &attackDeleySwitch.button) {
		*_vibratoParamsPtr->VibratoAttackDeleySwitch =
			attackDeleySwitch.getToggleState();
	}
	resized();
}

bool VibratoParametersComponent::isEditable() {
	return _vibratoParamsPtr->VibratoEnable->get();
}

VoicingParametersComponent::VoicingParametersComponent(
	VoicingParameters* voicingParams)
	: _voicingParamsPtr(voicingParams),
	voicingTypeSelector("Type", _voicingParamsPtr->VoicingSwitch, this),
	stepTimeSlider("StepTime", "sec", _voicingParamsPtr->StepTime, this,
		0.001f, 0.5f) {
	addAndMakeVisible(voicingTypeSelector);
	addAndMakeVisible(stepTimeSlider);
}

void VoicingParametersComponent::paint(Graphics& g) {
	paintHeaderEffects(g, getLocalBounds(), "VOICING");
}

void VoicingParametersComponent::resized() {
	float rowSize = 2.0f;
	float divide = 1.0f / rowSize;
	std::int32_t compHeight =
		std::int32_t((getHeight() - HEADER_HEIGHT) * divide);

	Rectangle<int> bounds = getLocalBounds();  // コンポーネント基準の値
	bounds.removeFromTop(HEADER_HEIGHT);

	{
		auto alpha = 0.0f;
		if (_voicingParamsPtr->VoicingSwitch->getCurrentChoiceName() ==
			"PORTAMENTO" ||
			_voicingParamsPtr->VoicingSwitch->getCurrentChoiceName() ==
			"ARPEGGIO") {
			alpha = 1.0f;
		}
		else {
			alpha = 0.4f;
		}
		stepTimeSlider.setAlpha(alpha);
	}
	voicingTypeSelector.setBounds(bounds.removeFromTop(compHeight));
	stepTimeSlider.setBounds(bounds.removeFromTop(compHeight));
}

void VoicingParametersComponent::timerCallback() {
	voicingTypeSelector.setSelectedItemIndex(
		_voicingParamsPtr->VoicingSwitch->getIndex());
	stepTimeSlider.setValue(_voicingParamsPtr->StepTime->get());
}

void VoicingParametersComponent::sliderValueChanged(Slider* slider) {
	if (slider == &stepTimeSlider.slider) {
		*_voicingParamsPtr->StepTime = (float)stepTimeSlider.getValue();
	}
}

void VoicingParametersComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged) {
	if (comboBoxThatHasChanged == &voicingTypeSelector.selector) {
		*_voicingParamsPtr->VoicingSwitch =
			voicingTypeSelector.getSelectedItemIndex();
	}
	resized();
}

OptionsParametersComponent::OptionsParametersComponent(
	OptionsParameters* optionsParams
)
	: _optionsParamsPtr(optionsParams),
	pitchStandardSlider("Tunes", "", _optionsParamsPtr->PitchStandard, this),
	pitchBendRangeSlider("PB Range", "", _optionsParamsPtr->PitchBendRange, this) {
	addAndMakeVisible(pitchStandardSlider);
	addAndMakeVisible(pitchBendRangeSlider);
}

void OptionsParametersComponent::paint(Graphics& g) {
	paintHeaderEffects(g, getLocalBounds(), "OPTIONS");
}

void OptionsParametersComponent::resized() {
	Rectangle<int> bounds = getLocalBounds();
	bounds.removeFromTop(HEADER_HEIGHT);
	std::int32_t compHeight = (getHeight() - HEADER_HEIGHT) / 2.0f;

	pitchStandardSlider.setBounds(bounds.removeFromTop(compHeight));
	pitchBendRangeSlider.setBounds(bounds.removeFromTop(compHeight));
}

void OptionsParametersComponent::timerCallback() {
	pitchStandardSlider.setValue(_optionsParamsPtr->PitchStandard->get());
	pitchBendRangeSlider.setValue(_optionsParamsPtr->PitchBendRange->get());
}

void OptionsParametersComponent::sliderValueChanged(Slider* slider) {
	if (slider == &pitchStandardSlider.slider) {
		*_optionsParamsPtr->PitchStandard = (std::int32_t)pitchStandardSlider.getValue();

	}
	else if (slider == &pitchBendRangeSlider.slider) {
		*_optionsParamsPtr->PitchBendRange =
			(std::int32_t)pitchBendRangeSlider.getValue();
	}
}

ArpSequencerComponent::ArpSequencerComponent(ArpParameters* arpParameters)
	: BaseComponent(),
	_arpParameters(arpParameters),
	_enableSwitch("On", _arpParameters->ArpEnabled, this),
	_loopSwitch("Loop", _arpParameters->LoopEnabled, this),
	_stepTimeSlider("Duration", "sec", _arpParameters->ArpStepTime, this, MIN_DELTA, 0.25f),
	_rangeSliders(_arpParameters) {

	addAndMakeVisible(_enableSwitch);
	addAndMakeVisible(_loopSwitch);
	addAndMakeVisible(_rangeSliders);
	addAndMakeVisible(_stepTimeSlider);
}

void ArpSequencerComponent::paint(Graphics& g) {
	paintHeaderWaveFormMemory(g, getLocalBounds(), "ARPS");
}

void ArpSequencerComponent::resized() {
	Rectangle<int> bounds = getLocalBounds();
	bounds.removeFromTop(HEADER_HEIGHT);

	{
		auto area = bounds.removeFromBottom(40);
		auto width = area.getWidth() / 4.0f;
		_enableSwitch.setBounds(area.removeFromLeft(width).reduced(LOCAL_MARGIN));
		_loopSwitch.setBounds(area.removeFromLeft(width).reduced(LOCAL_MARGIN));
		_stepTimeSlider.setBounds(area.reduced(LOCAL_MARGIN));
	}

	_rangeSliders.setBounds(bounds.reduced(LOCAL_MARGIN));
}

void ArpSequencerComponent::timerCallback() {
	_enableSwitch.setToggleState(_arpParameters->ArpEnabled->get());
	_loopSwitch.setToggleState(_arpParameters->LoopEnabled->get());
	_stepTimeSlider.setValue(_arpParameters->ArpStepTime->get());
}

void ArpSequencerComponent::buttonClicked(Button* button) {
	if (button == &_enableSwitch.button) {
		*_arpParameters->ArpEnabled = _enableSwitch.getToggleState();
	}
	else if (button == &_loopSwitch.button) {
		*_arpParameters->LoopEnabled = _loopSwitch.getToggleState();
	}
	resized();
}

void ArpSequencerComponent::sliderValueChanged(Slider* slider) {
	if (slider == &_stepTimeSlider.slider) {
		*_arpParameters->ArpStepTime = (float)_stepTimeSlider.getValue();
	}
}

MidiEchoParametersComponent::MidiEchoParametersComponent(
	MidiEchoParameters* midiEchoParams)
	: _midiEchoParamsPtr(midiEchoParams),
	enableButton("ON", _midiEchoParamsPtr->IsEchoEnable, this),
	durationSlider("Duration", "sec", _midiEchoParamsPtr->EchoDuration, this, 0.01f, 0.5f),
	repeatSlider("Repeat", "", _midiEchoParamsPtr->EchoRepeat, this),
	volumeOffsetSlider("Vol Offset", "%", _midiEchoParamsPtr->VolumeOffset, this, 0.1f, 100.0f) {
	addAndMakeVisible(repeatSlider);
	addAndMakeVisible(durationSlider);
	addAndMakeVisible(enableButton);
	addAndMakeVisible(volumeOffsetSlider);
}

void MidiEchoParametersComponent::paint(Graphics& g) {
	paintHeaderEffects(g, getLocalBounds(), "MIDI ECHO");
}

void MidiEchoParametersComponent::resized() {
	float rowSize = 4.0f;
	float divide = 1.0f / rowSize;
	std::int32_t compHeight =
		std::int32_t((getHeight() - HEADER_HEIGHT) * divide);

	Rectangle<int> bounds = getLocalBounds();  // コンポーネント基準の値
	bounds.removeFromTop(HEADER_HEIGHT);

	{
		float alpha = isEditable() ? 1.0f : 0.4f;
		durationSlider.setAlpha(alpha);
		repeatSlider.setAlpha(alpha);
		volumeOffsetSlider.setAlpha(alpha);
	}

	enableButton.setBounds(bounds.removeFromTop(compHeight));
	durationSlider.setBounds(bounds.removeFromTop(compHeight));
	repeatSlider.setBounds(bounds.removeFromTop(compHeight));
	volumeOffsetSlider.setBounds(bounds.removeFromTop(compHeight));
}

void MidiEchoParametersComponent::timerCallback() {
	enableButton.setToggleState(_midiEchoParamsPtr->IsEchoEnable->get());
	durationSlider.setValue(_midiEchoParamsPtr->EchoDuration->get());
	repeatSlider.setValue(_midiEchoParamsPtr->EchoRepeat->get());
	volumeOffsetSlider.setValue(_midiEchoParamsPtr->VolumeOffset->get());
}

void MidiEchoParametersComponent::sliderValueChanged(Slider* slider) {
	if (slider == &durationSlider.slider) {
		*_midiEchoParamsPtr->EchoDuration = (float)durationSlider.getValue();
	}
	else if (slider == &repeatSlider.slider) {
		*_midiEchoParamsPtr->EchoRepeat = (std::int32_t)repeatSlider.getValue();
	}
	else if (slider == &volumeOffsetSlider.slider) {
		*_midiEchoParamsPtr->VolumeOffset = (float)volumeOffsetSlider.getValue();
	}
}

void MidiEchoParametersComponent::buttonClicked(Button* button) {
	if (button == &enableButton.button) {
		*_midiEchoParamsPtr->IsEchoEnable = enableButton.getToggleState();
	}
	resized();
}

bool MidiEchoParametersComponent::isEditable() {
	return _midiEchoParamsPtr->IsEchoEnable->get();
}


// ----------------------------------------------------------------------------------------

WaveformMemoryParametersComponent::WaveformMemoryParametersComponent(
	WaveformMemoryParameters* waveformMemoryParams,
	AudioProcessor& processor,
	ChipOscillatorParameters* chipOscParams)
	: _waveformMemoryParamsPtr(waveformMemoryParams),
	_processorRef(processor),
	_chipOscParamsPtr(chipOscParams),
	waveRangeSlider(waveformMemoryParams, chipOscParams),
	saveButton(),
	loadButton(),

	// new button test
	waveButton(),
	nextButton(),
	prevButton(),



	fileBrowserButton() {
	auto fileMode =
		FileBrowserComponent::FileChooserFlags::openMode +
		FileBrowserComponent::FileChooserFlags::canSelectFiles +
		FileBrowserComponent::FileChooserFlags::filenameBoxIsReadOnly +
		FileBrowserComponent::FileChooserFlags::doNotClearFileNameOnRootChange;

	_fileBrowser = new FileBrowserComponent(fileMode, preFilePath.getParentDirectory(), nullptr, nullptr);
	_fileBrowser->addListener(this);
	addAndMakeVisible(_fileBrowser);

	// declaring the new wave button, it didnt work so it must be somewhere else ... 
	waveButton.setButtonText("+");
	waveButton.addListener(this);
	addAndMakeVisible(waveButton);




	prevButton.setButtonText("<");
	prevButton.addListener(this);
	addAndMakeVisible(prevButton);

	nextButton.setButtonText(">");
	nextButton.addListener(this);
	addAndMakeVisible(nextButton);


	saveButton.setButtonText("Save");
	saveButton.addListener(this);
	addAndMakeVisible(saveButton);

	loadButton.setButtonText("Load");
	loadButton.addListener(this);
	addAndMakeVisible(loadButton);

	fileBrowserButton.setButtonText("Browse");
	fileBrowserButton.addListener(this);
	addAndMakeVisible(fileBrowserButton);

	waveRangeSlider.addMouseListener(this, true);
	addAndMakeVisible(waveRangeSlider);
}




WaveformMemoryParametersComponent::~WaveformMemoryParametersComponent() {
	delete _fileBrowser;
}

void WaveformMemoryParametersComponent::paint(Graphics& g) {
	paintHeaderWaveFormMemory(g, getLocalBounds(), "WAVEFORM " + to_string(currentWaveformIndex + 1));
}

void WaveformMemoryParametersComponent::resized() {
	Rectangle<int> bounds = getLocalBounds();
	bounds.removeFromTop(HEADER_HEIGHT);

	{
		Rectangle<int> area = bounds.removeFromBottom(BUTTON_HEIGHT);
		const auto width = area.getWidth() / 4.5f;
		fileBrowserButton.setBounds(area.removeFromLeft(width).reduced(LOCAL_MARGIN));
		saveButton.setBounds(area.removeFromLeft(width).reduced(LOCAL_MARGIN));
		loadButton.setBounds(area.removeFromLeft(width).reduced(LOCAL_MARGIN));
		waveButton.setBounds(area.removeFromLeft(width / 2).reduced(LOCAL_MARGIN));

		prevButton.setBounds(area.removeFromLeft(width / 2).reduced(LOCAL_MARGIN));

		nextButton.setBounds(area.removeFromLeft(width / 2).reduced(LOCAL_MARGIN));


	}

	if (isFileBrowserEnabled) {
		_fileBrowser->setBounds(bounds.removeFromLeft(FILE_BROWSER_WIDTH));
	}
	_fileBrowser->setVisible(isFileBrowserEnabled);

	waveRangeSlider.setBounds(bounds);
}

void WaveformMemoryParametersComponent::buttonClicked(Button* button) {
	if (button == &saveButton) {
		saveWaveFile(_waveformMemoryParamsPtr, currentWaveformIndex, fc);
		_fileBrowser->setRoot(getPreFileDirectory(&preFilePath));
	}
	else if (button == &loadButton) {
		loadWaveFile(_waveformMemoryParamsPtr, currentWaveformIndex, fc);
		_fileBrowser->setRoot(getPreFileDirectory(&preFilePath));
	}
	else if (button == &fileBrowserButton) {
		isFileBrowserEnabled = !isFileBrowserEnabled;
		_fileBrowser->setRoot(getPreFileDirectory(&preFilePath));
		resized();
	}
	else if (button == &waveButton) {
		_waveformMemoryParamsPtr->addNewWaveform(_processorRef);

		int newWaveformIndex = _waveformMemoryParamsPtr->getNumWaveforms();
		String newWaveName = "Waveform " + String(newWaveformIndex);

		*(_chipOscParamsPtr->OscWaveType) = 16 + (newWaveformIndex - 1);

		if (onAddWaveform) {
			onAddWaveform(newWaveName);
		}

		//DBG("new waveform at index " + String(16 + (newWaveformIndex - 1)));
	}

	else if (button == &nextButton) {

		int waveIndex = (int)*_chipOscParamsPtr->OscWaveType;

		if (waveIndex >= 16) {
			int currentCustomIndex = waveIndex - 16;
			int newCustomIndex = currentCustomIndex + 1;

			if (newCustomIndex < _waveformMemoryParamsPtr->getNumWaveforms()) {
				WaveformMemoryParametersComponent::setWaveformIndex(newCustomIndex);
				*_chipOscParamsPtr->OscWaveType = waveIndex + 1;
			}
		}
		else {

			if (requestChange) {
				requestChange(*_chipOscParamsPtr->OscWaveType = 16);
			}
			// repaint();
			// yet to be fixed, there is one small bug that doesn't show the right wavetable
			// when you: Go to waveform 2, pick NES 50%, previous arrow, shows index 2 but its index 1 internally
		}


		//DBG("next pressed");
		//DBG(waveIndex);

	}
	else if (button == &prevButton) {

		int waveIndex = (int)*_chipOscParamsPtr->OscWaveType;

		if (waveIndex >= 17) {
			int currentCustomIndex = waveIndex - 16;
			int newCustomIndex = currentCustomIndex - 1;

			if (newCustomIndex >= 0) {
				WaveformMemoryParametersComponent::setWaveformIndex(newCustomIndex);
				*_chipOscParamsPtr->OscWaveType = waveIndex - 1;
			}
		}
		else {
			//*_chipOscParamsPtr->OscWaveType = 16;
			//repaint();

			if (requestChange) {
				requestChange(*_chipOscParamsPtr->OscWaveType = 16);
			}

		}


		//DBG("prev pressed");
	}

}

bool WaveformMemoryParametersComponent::isInterestedInFileDrag(
	const StringArray& files) {
	return true;
}

void WaveformMemoryParametersComponent::filesDropped(const StringArray& files,
	int x, int y) {
	std::string filePath = files.begin()->toStdString();

	if (filePath.find(".wfm") != std::string::npos) {
		File waveformFile(filePath);
		std::string data = waveformFile.loadFileAsString().toStdString();
		std::int32_t count = 0;

		AudioParameterInt** waveArray = _waveformMemoryParamsPtr->getWaveformArray(currentWaveformIndex);

		for (const auto subStr : split(data, ' ')) {
			if (count < WAVESAMPLE_LENGTH) {
				*waveArray[count] = atoi(subStr.c_str());
				++count;
			}
		}

		//DBG("current waveform: " + String(currentWaveformIndex));
	}
}

void WaveformMemoryParametersComponent::fileClicked(const File& file, const MouseEvent& event) {
	std::string filePath = file.getFullPathName().toStdString();

	if (filePath.substr(filePath.length() - 4) == ".wfm") {
		File waveformFile(filePath);
		std::string data = waveformFile.loadFileAsString().toStdString();
		std::int32_t count = 0;

		AudioParameterInt** waveArray = _waveformMemoryParamsPtr->getWaveformArray(currentWaveformIndex);

		for (const auto subStr : split(data, ' ')) {
			if (count < WAVESAMPLE_LENGTH) {
				*waveArray[count] = atoi(subStr.c_str());
				++count;
			}
		}

		//DBG("current waveform: " + String(currentWaveformIndex));
	}
}



void WaveformMemoryParametersComponent::browserRootChanged(const File& file) {
	preFilePath = file.getFullPathName();
}


void WaveformMemoryParametersComponent::setWaveformIndex(int index) {
	if (index < 0 || index >= _waveformMemoryParamsPtr->getNumWaveforms()) {
		//DBG("i: " + String(index) + ", max: " + String(_waveformMemoryParamsPtr->getNumWaveforms()));
		return;
	}
	currentWaveformIndex = index;
	waveRangeSlider.waveformindex(index);
	repaint();
}


// ----------------------------------------------------------------------------------



FilterParametersComponent::FilterParametersComponent(FilterParameters* filterParams)
	: BaseComponent(),
	_filterParamsPtr(filterParams),
	hiCutSwitch("HiCut: ON / OFF", _filterParamsPtr->HicutEnable, this),
	lowCutSwitch("LowCut: ON / OFF", _filterParamsPtr->LowcutEnable, this),
	hicutFreqSlider("hicut", "Hz", _filterParamsPtr->HicutFreq, this, 0.1f, 2000.0f),
	lowcutFreqSlider("lowcut", "Hz", _filterParamsPtr->LowcutFreq, this, 0.1f, 2000.0f) {
	addAndMakeVisible(hiCutSwitch);
	addAndMakeVisible(lowCutSwitch);
	addAndMakeVisible(hicutFreqSlider);
	addAndMakeVisible(lowcutFreqSlider);
}

void FilterParametersComponent::paint(Graphics& g) {
	paintHeaderEffects(g, getLocalBounds(), "FILTER");
}

void FilterParametersComponent::resized() {
	float rowSize = 4.0f;
	float divide = 1.0f / rowSize;
	std::int32_t compHeight =
		std::int32_t((getHeight() - HEADER_HEIGHT) * divide);

	Rectangle<int> bounds = getLocalBounds();  // コンポーネント基準の値
	bounds.removeFromTop(HEADER_HEIGHT);

	{
		float alpha = _filterParamsPtr->HicutEnable->get() ? 1.0f : 0.4f;
		hicutFreqSlider.setAlpha(alpha);
	}
	{
		float alpha = _filterParamsPtr->LowcutEnable->get() ? 1.0f : 0.4f;
		lowcutFreqSlider.setAlpha(alpha);
	}

	hiCutSwitch.setBounds(bounds.removeFromTop(compHeight));
	hicutFreqSlider.setBounds(bounds.removeFromTop(compHeight));
	lowCutSwitch.setBounds(bounds.removeFromTop(compHeight));
	lowcutFreqSlider.setBounds(bounds.removeFromTop(compHeight));
}

void FilterParametersComponent::timerCallback() {
	hicutFreqSlider.setValue(_filterParamsPtr->HicutFreq->get());
	lowcutFreqSlider.setValue(_filterParamsPtr->LowcutFreq->get());
}

void FilterParametersComponent::sliderValueChanged(Slider* slider) {
	if (slider == &hicutFreqSlider.slider) {
		*_filterParamsPtr->HicutFreq = (float)hicutFreqSlider.getValue();
	}
	else if (slider == &lowcutFreqSlider.slider) {
		*_filterParamsPtr->LowcutFreq = (float)lowcutFreqSlider.getValue();
	}
}

void FilterParametersComponent::buttonClicked(Button* button) {
	if (button == &hiCutSwitch.button) {
		*_filterParamsPtr->HicutEnable = hiCutSwitch.getToggleState();
	}
	else if (button == &lowCutSwitch.button) {
		*_filterParamsPtr->LowcutEnable = lowCutSwitch.getToggleState();
	}
	resized();
}


WavePatternsComponent::WavePatternsComponent(WavePatternParameters* wavePatternParameters,
	ChipOscillatorParameters* chipOscParams,
	WaveformMemoryParameters* waveformMemoryParams)
	: BaseComponent(),
	_wavePatternParameters(wavePatternParameters),
	_chipOscParamsPtr(chipOscParams),
	_waveformMemoryParamsPtr(waveformMemoryParams),
	_enableSwitch("On", _wavePatternParameters->PatternEnabled, this),
	_loopSwitch("Loop", _wavePatternParameters->LoopEnabled, this),
	_stepTimeSlider("Duration", "sec", _wavePatternParameters->StepTime, this, MIN_DELTA, 0.25f),
	_rangeSliders(_wavePatternParameters) {
	StringArray availableWaveforms;
	availableWaveforms.add("NES_Square50%");
	availableWaveforms.add("NES_Square25%");
	availableWaveforms.add("NES_Square12.5%");
	availableWaveforms.add("NES_Triangle");
	availableWaveforms.add("Pure_Square50%");
	availableWaveforms.add("Pure_Square25%");
	availableWaveforms.add("Pure_Square12.5%");
	availableWaveforms.add("Pure_Triangle");
	availableWaveforms.add("Pure_Sine");
	availableWaveforms.add("Pure_Saw");
	availableWaveforms.add("NES_LongNoise");
	availableWaveforms.add("NES_ShortNoise");
	availableWaveforms.add("Pure_Noise");
	availableWaveforms.add("Rough_Sine");
	availableWaveforms.add("Rough_Saw");
	availableWaveforms.add("Rough_Noise");
	for (int w = 0; w < _waveformMemoryParamsPtr->getNumWaveforms(); ++w) {
		if (w == 0) {
			availableWaveforms.add("Waveform Memory");
		}
		else {
			availableWaveforms.add("Waveform " + String(w + 1));
		}
	}
	for (auto i = 0; i < WAVEPATTERN_TYPES; ++i) {
		_waveTypeSelectors[i] = new TextSelector("", availableWaveforms, this);
		_waveTypeSelectors[i]->removeLabel();
		int currentWaveIndex = _wavePatternParameters->WaveTypes[i]->get();
		_waveTypeSelectors[i]->setSelectedItemIndex(currentWaveIndex);

		addAndMakeVisible(*_waveTypeSelectors[i]);
	}

	addAndMakeVisible(_enableSwitch);
	addAndMakeVisible(_loopSwitch);
	addAndMakeVisible(_rangeSliders);
	addAndMakeVisible(_stepTimeSlider);
}


void WavePatternsComponent::paint(Graphics& g) {
	paintHeader(g, getLocalBounds(), "WavePatterns");
}

void WavePatternsComponent::resized() {
	Rectangle<int> bounds = getLocalBounds();
	bounds.removeFromTop(HEADER_HEIGHT);
	{
		auto area = bounds.removeFromTop(bounds.getHeight() / 12.0f);
		auto width = area.getWidth() / 6.0f;
		_enableSwitch.setBounds(area.removeFromLeft(width));
		_loopSwitch.setBounds(area.removeFromLeft(width));
		_stepTimeSlider.setBounds(area);

	}
	{
		auto area = bounds.removeFromLeft(150);
		auto compHeight = area.getHeight() / (float)WAVEPATTERN_TYPES;
		for (auto i = WAVEPATTERN_TYPES - 1; i >= 0; --i) {
			_waveTypeSelectors[i]->setBounds(area.removeFromTop(compHeight));
		}
	}

	_rangeSliders.setBounds(bounds);
}

void WavePatternsComponent::timerCallback() {
	_enableSwitch.setToggleState(_wavePatternParameters->PatternEnabled->get());
	_loopSwitch.setToggleState(_wavePatternParameters->LoopEnabled->get());
	static int lastNumWaveforms = 4;
	if (_waveformMemoryParamsPtr->getNumWaveforms() != lastNumWaveforms) {
		lastNumWaveforms = _waveformMemoryParamsPtr->getNumWaveforms();
		refreshWaveformList();
	}
	for (int i = 0; i < WAVEPATTERN_TYPES; ++i) {
		int currentValue = _wavePatternParameters->WaveTypes[i]->get();
		if (_waveTypeSelectors[i]->selector.getSelectedItemIndex() != currentValue) {
			_waveTypeSelectors[i]->setSelectedItemIndex(currentValue);
		}
	}
}




void WavePatternsComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged) {
	for (auto i = 0; i < WAVEPATTERN_TYPES; ++i) {
		if (comboBoxThatHasChanged == &_waveTypeSelectors[i]->selector) {
			int selectedIndex = _waveTypeSelectors[i]->selector.getSelectedItemIndex();
			*(_wavePatternParameters->WaveTypes[i]) = selectedIndex;
			String selectedName = _waveTypeSelectors[i]->selector.getText();
			*_chipOscParamsPtr->OscWaveType = selectedIndex;
			if (onSwitchWaveform) {
				onSwitchWaveform(selectedIndex);
			}
			DBG("WavePattern[" + String(i) + "]" + ", name: " + selectedName);
			return;
		}
	}
}


void WavePatternsComponent::buttonClicked(Button* button) {
	if (button == &_enableSwitch.button) {
		*_wavePatternParameters->PatternEnabled = _enableSwitch.getToggleState();
	}
	else if (button == &_loopSwitch.button) {
		*_wavePatternParameters->LoopEnabled = _loopSwitch.getToggleState();
	}
	resized();
}

void WavePatternsComponent::sliderValueChanged(Slider* slider) {
	if (slider == &_stepTimeSlider.slider) {
		*_wavePatternParameters->StepTime = (float)_stepTimeSlider.getValue();
	}
}