#pragma once

#include <JuceHeader.h>
#include "GUI/ParametersComponent.h"
#include "GUI/ScopeComponent.hpp"

class PluginProcessor;

class EditorGUI : public AudioProcessorEditor,
	public Button::Listener {
public:
	EditorGUI(PluginProcessor& p);
	~EditorGUI();
	void paint(Graphics& g) override;
	void resized() override;
	void buttonClicked(Button* button) override;

private:
	PluginProcessor& processor;

	MidiKeyboardComponent keyboardComponent;

	PageButton OscButton;
	PageButton EffectButton;
	PageButton ArpButton;
	PageButton CycleButton;

	// Oscillator Page Component
	ScopeComponent<float> scopeComponent;
	ChipOscillatorComponent chipOscComponent;
	SweepParametersComponent sweepParamsComponent;
	VibratoParametersComponent vibratoParamsComponent;
	VoicingParametersComponent voicingParamsComponent;
	OptionsParametersComponent optionsParamsComponent;

	WaveformMemoryParameters* _waveformMemoryParamsPtr;
	WaveformMemoryParametersComponent waveformMemoryParamsComponent;

	// Effects Page Component
	MidiEchoParametersComponent midiEchoParamsComponent;
	FilterParametersComponent filterParamsComponent;

	LookAndFeel* customLookAndFeel;

	WavePatternsComponent wavePatternsComponent;

	ArpSequencerComponent arpSequencer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditorGUI)
};
