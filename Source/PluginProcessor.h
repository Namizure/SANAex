#pragma once

#include <JuceHeader.h>
#include "BaseAudioProcessor.h"
#include "DSP/DspUtils.h"
#include "DSP/SynthParameters.h"
#include "GUI/ScopeComponent.hpp"
#include "GUI/PresetManager.h"

class PluginProcessor : public BaseAudioProcessor {
public:
	PluginProcessor();
	~PluginProcessor();

	AudioProcessorEditor* createEditor() override;
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const String getProgramName(int index) override;

	void prepareToPlay(double sampleRate, std::int32_t samplesPerBlock) override;
	void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

	void getStateInformation(MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;
	void getPresetStateInformation(MemoryBlock& destData);
	bool isDrumModeActive() const { return presetsParameters.DrumMode->get(); }


	MidiKeyboardState& getKeyboardState() { return keyboardState; }
	AudioBufferQueue<float>& getAudioBufferQueue() { return scopeDataQueue; }

	const StringArray SWEEP_SWITCH{ "OFF", "POSITIVE", "NEGATIVE" };
	const StringArray VOICING_SWITCH{ "POLY", "MONO", "PORTAMENTO" };

	ChipOscillatorParameters chipOscParameters;
	SweepParameters sweepParameters;
	VibratoParameters vibratoParameters;
	VoicingParameters voicingParameters;
	OptionsParameters optionsParameters;
	WaveformMemoryParameters waveformMemoryParameters;
	MidiEchoParameters midiEchoParameters;
	FilterParameters filterParameters;
	PresetsParameters presetsParameters;
	WavePatternParameters wavePatternParameters;
	ArpParameters arpParameters;

	Service::PresetManager& getPresetManager() {
		return *presetManager;
	}

	juce::AudioProcessorValueTreeState& getAPVTS() {
		return valueTreeState;
	}

	void setCurrentProgIndex(std::int32_t index) noexcept { currentProgIndex.store(index, std::memory_order_relaxed); }
	std::int32_t getCurrentProgIndex() const noexcept { return currentProgIndex.load(std::memory_order_relaxed); }

private:
	std::unique_ptr<XmlElement> createParametersXml();
	void initProgram();
	std::int32_t getNumVoices();
	void addVoice();
	void changeVoiceSize();
	void clearBuffers(AudioBuffer<float>& buffer, AudioBuffer<float>& upSampleBuffer);
	static float clippingFunction(float inputValue);
	void initEffecters(dsp::ProcessSpec& spec);
	void procMidiMessages(const AudioBuffer<float>& buffer, const MidiBuffer& midiMessages);

	Synthesiser synth;

	std::atomic<std::int32_t> currentProgIndex{ -1 };
	antiAliasFilter antiAliasFilter;
	dsp::WaveShaper<float> clipper;
	dsp::Gain<float> drive;
	dsp::ProcessorDuplicator<dsp::IIR::Filter<float>,
		dsp::IIR::Coefficients<float>>
		hicutFilter;
	dsp::ProcessorDuplicator<dsp::IIR::Filter<float>,
		dsp::IIR::Coefficients<float>>
		lowcutFilter;
	MidiKeyboardState keyboardState;
	AudioBufferQueue<float> scopeDataQueue;
	ScopeDataCollector<float> scopeDataCollector;

	MidiBuffer eventsToAdd;

	AudioProcessorValueTreeState valueTreeState;

	std::unique_ptr<Service::PresetManager> presetManager;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};