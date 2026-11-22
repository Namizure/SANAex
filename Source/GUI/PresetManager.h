/*
  ==============================================================================

	PresetManager.h
	Created: 18 Aug 2026 5:15:53pm
	Author:  NAMIZURE

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <vector>
#include "../DSP/SynthParameters.h"

class PluginProcessor;

static constexpr int NUM_PRESETS = 64;
static constexpr int NUM_PRESET_STEPS = 16;
static constexpr int DRUM_BASE_NOTE = 60;

namespace Service {
	struct PresetStepFlags {
		bool wavePatternEnd[NUM_PRESET_STEPS] = { false };
		bool wavePatternLoop[NUM_PRESET_STEPS] = { false };
		bool arpEnd[NUM_PRESET_STEPS] = { false };
		bool arpLoop[NUM_PRESET_STEPS] = { false };
	};

	struct DrumPreset
	{
		int waveType = 0;
		float volumeDb = 0.0f;
		float attack = 0.0f;
		float decay = 0.0f;
		float sustain = 1.0f;
		float release = 0.0f;

		int sweepMode = 0;
		float sweepTime = 1.0f;

		bool vibratoEnabled = false;
		bool vibratoAttackDelaySwitch = true;
		float vibratoAmount = 0.0f;
		float vibratoSpeed = 0.0f;
		float vibratoAttackTime = 0.0f;

		bool echoEnabled = false;
		float echoDuration = 0.1f;
		int echoRepeat = 1;
		float echoVolumeOffset = 50.0f;

		bool patternEnabled = false;
		bool patternLoopEnabled = true;
		float patternStepTime = 0.1f;
		int wavePattern[WAVEPATTERN_LENGTH] = { 0 };
		int patternWaveTypes[WAVEPATTERN_TYPES] = { 0 };

		bool arpEnabled = false;
		bool arpLoopEnabled = true;
		bool arpFixed = false;
		float arpStepTime = 0.1f;
		int arpPattern[ArpParameters::ARP_LENGTH] = { 0 };

		PresetStepFlags stepFlags;

		int numWaveforms = 4;
		int waveSamples[WaveformMemoryParameters::MAX_WAVEFORMS][WaveformMemoryParameters::WAVESAMPLE_LENGTH] = { { 0 } };
	};

	class PresetManager : public juce::ValueTree::Listener,
		public juce::AudioProcessorParameter::Listener,
		private juce::Timer,
		private juce::AsyncUpdater {
	public:
		static const juce::File defaultdir;
		static const juce::String extension;
		static const juce::String presetNameProperty;
		static const juce::String packPathProperty;

		static constexpr int NUM_STEPS = NUM_PRESET_STEPS;

		PresetManager(PluginProcessor& processor, juce::AudioProcessorValueTreeState&);
		~PresetManager() override;
		void loadPack(const juce::File& packFile, bool initFirstPreset = true);
		void savePack(const juce::File& targetFile = juce::File{});
		bool hasActivePackFile() const;
		void savePreset(int index, const juce::String& presetName);
		void loadPreset(int index);
		void deletePreset(int index);
		void moveOrSwapPreset(int fromIndex, int toIndex);
		void syncCurrentPresetFromParameters();
		void markPresetModified(bool immediate = false);
		void cancelPendingSync() noexcept
		{
			needsSync.store(false, std::memory_order_release);
		}

		std::function<void(int)> onPresetLoaded;

		int getNextPresetIndex() const;
		int getPreviousPresetIndex() const;

		juce::StringArray getAllPresetsInPack() const;
		int getCurrentPresetIndex() const;
		void setPresetIndex(int index);
		juce::String getCurrentPackName() const;
		juce::String getCurrentPackPath() const;
		void captureDefaultState();
		void rebuildCache();
		const DrumPreset* getDrumPreset(int index) const
		{
			return (index >= 0 && index < NUM_PRESETS) ? &drumPresetCache[(size_t)index] : nullptr;
		}
		void saveStateToXml(juce::XmlElement& xml);
		void loadStateFromXml(const juce::XmlElement& xml);

		class ScopedSyncBlock {
		public:
			explicit ScopedSyncBlock(PresetManager& pm) : owner(pm) { ++owner.syncBlockCount; }
			~ScopedSyncBlock() { --owner.syncBlockCount; }
		private:
			PresetManager& owner;
			JUCE_DECLARE_NON_COPYABLE(ScopedSyncBlock)
		};

		std::array<std::vector<float>, NUM_PRESETS> presetParamCache;
		std::atomic<bool> cacheReady{ false };
		std::atomic<bool> isInternalPresetSwap{ false };

		juce::AudioProcessor& processor;

	private:
		void valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged) override;
		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void parameterValueChanged(int parameterIndex, float newValue) override;
		void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;
		void timerCallback() override;
		void handleAsyncUpdate() override;
		void flushPendingSync();

		juce::String capturePresetState() const;
		void decodePresetState(const juce::MemoryBlock& state, std::vector<float>& paramValues, DrumPreset& drumPreset) const;
		void updateSinglePresetInCache(int index);
		void setPackTree(const juce::ValueTree& newTree);
		void addMissingPresetSlots();

		PluginProcessor& owner;
		juce::AudioProcessorValueTreeState& valueTreeState;

		juce::File currentPackFile;
		juce::ValueTree packTree;
		std::atomic<int> currentIndex{ 0 };

		std::atomic<bool> needsSync{ false };
		std::atomic<int> syncBlockCount{ 0 };
		juce::Array<juce::AudioProcessorParameter*> listenedParameters;

		std::vector<float> defaultParamValues;
		DrumPreset defaultDrumPreset;
		juce::MemoryBlock defaultStateData;
		std::array<DrumPreset, NUM_PRESETS> drumPresetCache;
	};
}
