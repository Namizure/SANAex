#pragma once
#include "../DSP/SynthParameters.h"
#include "ComponentUtil.hpp"
#include <functional>
#include "ProjectFonts.h"

class BaseComponent : public Component, private juce::Timer {
public:
	BaseComponent()
	{
		setLookAndFeel(&lookAndFeel);
		startTimerHz(5);
	}

	~BaseComponent() override
	{
		setLookAndFeel(nullptr);
	}

	virtual void paint(Graphics& g) = 0;
	virtual void resized() = 0;

private:
	ProjectFonts::LookAndFeel lookAndFeel;
	virtual void timerCallback() = 0;
};

class WaveformMemoryParameters;
class ChipOscillatorComponent : public BaseComponent,
	ComboBox::Listener,
	Slider::Listener {
public:
	ChipOscillatorParameters* _oscParamsPtr;
	WaveformMemoryParameters* _waveformMemoryParamsPtr;
	ChipOscillatorComponent(ChipOscillatorParameters* oscParams, WaveformMemoryParameters* waveformMemoryParams);

	std::function<void(int)> onWaveformChanged;

	virtual void paint(Graphics& g) override;
	virtual void resized() override;
	TextSelector waveTypeSelector;
	void refreshWaveformList();

private:
	ChipOscillatorComponent();

	virtual void timerCallback() override;
	virtual void sliderValueChanged(Slider* slider) override;
	virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;



	TextSlider volumeLevelSlider;
	TextSlider attackSlider;
	TextSlider decaySlider;
	TextSlider sustainSlider;
	TextSlider releaseSlider;
	//TextSelector colorTypeSelector;
	//TextSlider colorDurationSlider;
};

class SweepParametersComponent : public BaseComponent,
	Slider::Listener,
	ComboBox::Listener {
public:
	SweepParametersComponent(SweepParameters* sweepParams);

	virtual void paint(Graphics& g) override;
	virtual void resized() override;

private:
	SweepParametersComponent();

	virtual void timerCallback() override;
	virtual void sliderValueChanged(Slider* slider) override;
	virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
	bool isEditable();

	SweepParameters* _sweepParamsPtr;

	TextSelector sweepSwitchSelector;
	TextSlider timeSlider;
};

class VibratoParametersComponent : public BaseComponent,
	Button::Listener,
	Slider::Listener {
public:
	VibratoParametersComponent(VibratoParameters* vibratoParams);

	virtual void paint(Graphics& g) override;
	virtual void resized() override;

private:
	VibratoParametersComponent();

	virtual void timerCallback() override;
	virtual void sliderValueChanged(Slider* slider) override;
	virtual void buttonClicked(Button* button) override;
	bool isEditable();

	VibratoParameters* _vibratoParamsPtr;

	SwitchButton enableSwitch;
	SwitchButton attackDeleySwitch;
	TextSlider amountSlider;
	TextSlider speedSlider;
	TextSlider attackDeleyTimeSlider;
};

class VoicingParametersComponent : public BaseComponent,
	ComboBox::Listener,
	Slider::Listener {
public:
	VoicingParametersComponent(VoicingParameters* voicingParams);

	virtual void paint(Graphics& g) override;
	virtual void resized() override;

private:
	VoicingParametersComponent();

	virtual void timerCallback() override;
	virtual void sliderValueChanged(Slider* slider) override;
	virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

	VoicingParameters* _voicingParamsPtr;

	TextSelector voicingTypeSelector;
	TextSlider stepTimeSlider;
};

class OptionsParametersComponent : public BaseComponent, Slider::Listener {
public:
	OptionsParametersComponent(OptionsParameters* optionsParams);

	virtual void paint(Graphics& g) override;
	virtual void resized() override;

private:
	OptionsParametersComponent();

	virtual void timerCallback() override;
	virtual void sliderValueChanged(Slider* slider) override;

	OptionsParameters* _optionsParamsPtr;

	TextSliderIncDec pitchStandardSlider;
	TextSliderIncDec pitchBendRangeSlider;
};

class MidiEchoParametersComponent : public BaseComponent,
	Button::Listener,
	Slider::Listener {
public:
	MidiEchoParametersComponent(MidiEchoParameters* midiEchoParams);

	virtual void paint(Graphics& g) override;
	virtual void resized() override;

private:
	MidiEchoParametersComponent();

	virtual void timerCallback() override;
	virtual void sliderValueChanged(Slider* slider) override;
	virtual void buttonClicked(Button* button) override;
	bool isEditable();

	MidiEchoParameters* _midiEchoParamsPtr;

	SwitchButton enableButton;
	TextSlider durationSlider;
	TextSlider repeatSlider;
	TextSlider volumeOffsetSlider;
};

// wfpc 1

class WaveformMemoryParametersComponent : public Component,
	Button::Listener,
	public FileDragAndDropTarget,
	public FileBrowserListener {
public:
	WaveformMemoryParametersComponent(WaveformMemoryParameters* waveformMemoryParams,
		AudioProcessor& processor,
		ChipOscillatorParameters* chipOscParams);
	~WaveformMemoryParametersComponent();

	virtual void paint(Graphics& g) override;
	virtual void resized() override;

	// I'm going to test out making my own function.
	void setWaveformIndex(int index);
	WaveSampleSliders waveRangeSlider;

	function<void(const String&)> onAddWaveform;

	function<void(const int&)> requestChange;

private:
	ChipOscillatorParameters* _chipOscParamsPtr;
	// this will keep track of which waveformarray we are in.
	int currentWaveformIndex = 0;


	WaveformMemoryParametersComponent();

	// Button::Listener
	virtual void buttonClicked(Button* button) override;

	// FIleDragAndDropTarget
	virtual bool isInterestedInFileDrag(const StringArray& files) override;
	virtual void filesDropped(const StringArray& files, int x, int y) override;
	virtual void fileDragEnter(const StringArray& files, int x, int y) override {};
	virtual void fileDragMove(const StringArray& files, int x, int y) override {};
	virtual void fileDragExit(const StringArray& files) override {};

	// FileBrowserListener
	virtual void fileClicked(const File& file, const MouseEvent& event) override;
	virtual void selectionChanged() override {};
	virtual void fileDoubleClicked(const File&) override {};
	virtual void browserRootChanged(const File& file) override;

	WaveformMemoryParameters* _waveformMemoryParamsPtr;
	AudioProcessor& _processorRef;


	std::unique_ptr<FileChooser> fc;


	FileBrowserComponent* _fileBrowser = nullptr;
	TextButton saveButton;
	TextButton loadButton;
	TextButton fileBrowserButton;

	// making new wave button
	TextButton waveButton;

	TextButton nextButton;
	TextButton prevButton;




	bool isFileBrowserEnabled = false;
	const int BUTTON_HEIGHT = 32;
	const int FILE_BROWSER_WIDTH = 240;
};


// --------------------------------------------------------------------------------------




// --------------------------------------------------------------------------------------




class FilterParametersComponent : public BaseComponent,
	Button::Listener,
	Slider::Listener {
public:
	FilterParametersComponent(FilterParameters* filterParams);

	virtual void paint(Graphics& g) override;
	virtual void resized() override;

private:
	FilterParametersComponent();

	virtual void timerCallback() override;
	virtual void sliderValueChanged(Slider* slider) override;
	virtual void buttonClicked(Button* button) override;

	FilterParameters* _filterParamsPtr;

	SwitchButton hiCutSwitch;
	SwitchButton lowCutSwitch;

	TextSlider hicutFreqSlider;
	TextSlider lowcutFreqSlider;
};

class WavePatternsComponent :
	public BaseComponent,
	public Button::Listener,
	public Slider::Listener,
	public ComboBox::Listener {
public:
	WavePatternsComponent(WavePatternParameters* wavePatternParameters,
		ChipOscillatorParameters* chipOscParams,
		WaveformMemoryParameters* waveformMemoryParams);

	virtual void paint(Graphics& g) override;
	virtual void resized() override;

	~WavePatternsComponent() {
		for (auto i = 0; i < WAVEPATTERN_TYPES; ++i) {
			delete _waveTypeSelectors[i];
		}
	}

	function<void(const int&)> onSwitchWaveform;

	void refreshWaveformList() {
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

		// add custom waveforms based on current count
		for (int w = 1; w <= _waveformMemoryParamsPtr->getNumWaveforms(); ++w) {
			if (w == 1) {
				fullList.add("Waveform Memory");
			}
			else {
				fullList.add("Waveform " + String(w));
			}
		}

		// update all selectors
		for (int i = 0; i < WAVEPATTERN_TYPES; ++i) {
			_waveTypeSelectors[i]->selector.clear(dontSendNotification);
			_waveTypeSelectors[i]->selector.addItemList(fullList, 1);
			_waveTypeSelectors[i]->setSelectedItemIndex(_wavePatternParameters->WaveTypes[i]->get());
		}

		_wavePatternParameters->storedWaveformNames = fullList;
	}

private:
	ChipOscillatorParameters* _chipOscParamsPtr;
	WaveformMemoryParameters* _waveformMemoryParamsPtr;
	virtual void timerCallback() override;
	virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
	virtual void buttonClicked(Button* button) override;
	virtual void sliderValueChanged(Slider* slider) override;

	WavePatternParameters* _wavePatternParameters;
	SwitchButton _enableSwitch;
	SwitchButton _loopSwitch;
	TextSlider _stepTimeSlider;
	TextSelector* _waveTypeSelectors[WAVEPATTERN_TYPES];
	PatternSliders _rangeSliders;

	const StringArray OSC_WAVE_TYPES{
	  "NES_Square50%",    "NES_Square25%",   "NES_Square12.5%",
	  "NES_Triangle",     "Pure_Square50%",  "Pure_Square25%",
	  "Pure_Square12.5%", "Pure_Triangle",   "Pure_Sine",
	  "Pure_Saw",         "NES_LongNoise",   "NES_ShortNoise",
	  "Pure_Noise",       "Rough_Sine",      "Rough_Saw",
	  "Rough_Noise",      "Waveform Memory", "Waveform 2",
	  "Waveform 3",       "Waveform 4",      "Waveform 5",
	  "Waveform 6",       "Waveform 7",      "Waveform 8",
	  "Waveform 9",       "Waveform 10",     "Waveform 11",
	  "Waveform 12",      "Waveform 13",     "Waveform 14",
	  "Waveform 15",      "Waveform 16",     "Waveform 17",
	  "Waveform 18",      "Waveform 19",     "Waveform 20"
	};
};




class ArpSequencerComponent : public BaseComponent,
	public Button::Listener,
	public Slider::Listener {
public:
	ArpSequencerComponent(ArpParameters* arpParameters);

	virtual void paint(Graphics& g) override;
	virtual void resized() override;
	virtual void timerCallback() override;
	virtual void buttonClicked(Button* button) override;
	virtual void sliderValueChanged(Slider* slider) override;

private:
	ArpParameters* _arpParameters;
	SwitchButton _loopSwitch;
	SwitchButton _enableSwitch;
	TextSlider _stepTimeSlider;
	ArpSliders _rangeSliders;
};