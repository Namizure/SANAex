/*
  ==============================================================================

	PresetManager.cpp
	Created: 18 Aug 2026 5:15:53pm
	Author:  NAMIZURE

  ==============================================================================
*/

#include "PresetManager.h"
#include "../PluginProcessor.h"
#include <JuceHeader.h>

namespace Service
{
	namespace
	{
		const juce::String presetStateProperty{ "state" };
		const juce::String packXmlTag{ "Pack" };

		using SavedValues = juce::HashMap<juce::String, double>;

		juce::String initPresetName(int index)
		{
			return "Init " + juce::String(index + 1);
		}
		double savedValue(const SavedValues& saved, juce::RangedAudioParameter* param)
		{
			if (saved.contains(param->paramID))
				return saved[param->paramID];

			return param->convertFrom0to1(param->getDefaultValue());
		}
	}

	const juce::File PresetManager::defaultdir{ juce::File::getSpecialLocation(
		juce::File::SpecialLocationType::commonDocumentsDirectory)
			.getChildFile(ProjectInfo::companyName)
			.getChildFile(ProjectInfo::projectName)
			.getChildFile("Presets")
	};
	const juce::String PresetManager::extension{ "saaex" };
	const juce::String PresetManager::presetNameProperty{ "presetName" };
	const juce::String PresetManager::packPathProperty{ "packPath" };

	PresetManager::PresetManager(PluginProcessor& proc, juce::AudioProcessorValueTreeState& apvts) :
		processor(proc),
		owner(proc),
		valueTreeState(apvts),
		packTree(packXmlTag)
	{
		if (!defaultdir.exists())
			defaultdir.createDirectory();

		addMissingPresetSlots();

		valueTreeState.state.addListener(this);
	}

	PresetManager::~PresetManager()
	{
		cancelPendingUpdate();
		if (isTimerRunning() && currentPackFile.existsAsFile())
			savePack();

		stopTimer();

		valueTreeState.state.removeListener(this);

		for (auto* param : listenedParameters)
			param->removeListener(this);
	}

	void PresetManager::addMissingPresetSlots()
	{
		while (packTree.getNumChildren() < NUM_PRESETS)
		{
			const int i = packTree.getNumChildren();
			juce::ValueTree presetNode("Preset");
			presetNode.setProperty("index", i, nullptr);
			presetNode.setProperty("name", initPresetName(i), nullptr);
			packTree.addChild(presetNode, -1, nullptr);
		}
	}

	void PresetManager::setPackTree(const juce::ValueTree& newTree)
	{
		packTree = newTree;
		addMissingPresetSlots();
	}

	void PresetManager::captureDefaultState()
	{
		owner.getPresetStateInformation(defaultStateData);
		decodePresetState(defaultStateData, defaultParamValues, defaultDrumPreset);
		rebuildCache();
		const auto& presetsParams = owner.presetsParameters;
		const auto& optionsParams = owner.optionsParameters;

		for (auto* param : processor.getParameters())
		{
			if (param == presetsParams.ProgramIndex || param == presetsParams.DrumMode
				|| param == optionsParams.PitchBendRange || param == optionsParams.PitchStandard)
				continue;

			param->addListener(this);
			listenedParameters.add(param);
		}
	}

	juce::String PresetManager::capturePresetState() const
	{
		juce::MemoryBlock destData;
		owner.getPresetStateInformation(destData);
		return destData.toBase64Encoding();
	}

	void PresetManager::decodePresetState(const juce::MemoryBlock& state, std::vector<float>& paramValues, DrumPreset& drum) const
	{
		SavedValues saved(2003);

		if (state.getSize() > 0)
		{
			if (auto xml = juce::AudioProcessor::getXmlFromBinary(state.getData(), (int)state.getSize()))
			{
				for (int i = 0; i < xml->getNumAttributes(); ++i)
					saved.set(xml->getAttributeName(i), xml->getAttributeValue(i).getDoubleValue());
			}
		}

		const auto allParams = processor.getParameters();
		paramValues.resize((size_t)allParams.size());

		for (int p = 0; p < allParams.size(); ++p)
		{
			auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(allParams[p]);

			if (ranged != nullptr && saved.contains(ranged->paramID))
				paramValues[(size_t)p] = ranged->getNormalisableRange().convertTo0to1((float)saved[ranged->paramID]);
			else
				paramValues[(size_t)p] = allParams[p]->getDefaultValue();
		}

		const auto number = [&saved](juce::RangedAudioParameter* param) { return (float)savedValue(saved, param); };
		const auto integer = [&saved](juce::RangedAudioParameter* param) { return juce::roundToInt(savedValue(saved, param)); };
		const auto toggle = [&saved](juce::RangedAudioParameter* param) { return savedValue(saved, param) >= 0.5; };
		const auto stepFlag = [&saved](const juce::String& name) { return saved.contains(name) && saved[name] != 0.0; };

		drum.waveType = integer(owner.chipOscParameters.OscWaveType);
		drum.volumeDb = number(owner.chipOscParameters.VolumeLevel);
		drum.attack = number(owner.chipOscParameters.Attack);
		drum.decay = number(owner.chipOscParameters.Decay);
		drum.sustain = number(owner.chipOscParameters.Sustain);
		drum.release = number(owner.chipOscParameters.Release);

		drum.sweepMode = integer(owner.sweepParameters.SweepSwitch);
		drum.sweepTime = number(owner.sweepParameters.SweepTime);

		drum.vibratoEnabled = toggle(owner.vibratoParameters.VibratoEnable);
		drum.vibratoAttackDelaySwitch = toggle(owner.vibratoParameters.VibratoAttackDeleySwitch);
		drum.vibratoAmount = number(owner.vibratoParameters.VibratoAmount);
		drum.vibratoSpeed = number(owner.vibratoParameters.VibratoSpeed);
		drum.vibratoAttackTime = number(owner.vibratoParameters.VibratoAttackTime);

		drum.echoEnabled = toggle(owner.midiEchoParameters.IsEchoEnable);
		drum.echoDuration = number(owner.midiEchoParameters.EchoDuration);
		drum.echoRepeat = integer(owner.midiEchoParameters.EchoRepeat);
		drum.echoVolumeOffset = number(owner.midiEchoParameters.VolumeOffset);

		drum.patternEnabled = toggle(owner.wavePatternParameters.PatternEnabled);
		drum.patternLoopEnabled = toggle(owner.wavePatternParameters.LoopEnabled);
		drum.patternStepTime = number(owner.wavePatternParameters.StepTime);

		drum.arpEnabled = toggle(owner.arpParameters.ArpEnabled);
		drum.arpLoopEnabled = toggle(owner.arpParameters.LoopEnabled);
		drum.arpFixed = toggle(owner.arpParameters.FixedEnabled);
		drum.arpStepTime = number(owner.arpParameters.ArpStepTime);

		for (int i = 0; i < WAVEPATTERN_LENGTH; ++i)
			drum.wavePattern[i] = integer(owner.wavePatternParameters.WavePatternArray[i]);

		for (int i = 0; i < WAVEPATTERN_TYPES; ++i)
			drum.patternWaveTypes[i] = integer(owner.wavePatternParameters.WaveTypes[i]);

		for (int i = 0; i < ArpParameters::ARP_LENGTH; ++i)
			drum.arpPattern[i] = integer(owner.arpParameters.ArpPatternArray[i]);

		for (int s = 0; s < NUM_STEPS; ++s)
		{
			drum.stepFlags.wavePatternEnd[s] = stepFlag("WP_EndStep_" + juce::String(s));
			drum.stepFlags.wavePatternLoop[s] = stepFlag("WP_LoopStep_" + juce::String(s));
			drum.stepFlags.arpEnd[s] = stepFlag("Arp_EndStep_" + juce::String(s));
			drum.stepFlags.arpLoop[s] = stepFlag("Arp_LoopStep_" + juce::String(s));
		}

		drum.numWaveforms = saved.contains("NUM_WAVEFORMS") ? juce::roundToInt(saved["NUM_WAVEFORMS"]) : 4;

		for (int w = 0; w < WaveformMemoryParameters::MAX_WAVEFORMS; ++w)
			for (int i = 0; i < WaveformMemoryParameters::WAVESAMPLE_LENGTH; ++i)
				drum.waveSamples[w][i] = integer(owner.waveformMemoryParameters.WaveSamplesArrays[w][(size_t)i]);
	}

	void PresetManager::rebuildCache()
	{
		for (int i = 0; i < NUM_PRESETS; ++i)
			updateSinglePresetInCache(i);

		cacheReady.store(true);
	}

	void PresetManager::updateSinglePresetInCache(int index)
	{
		if (index < 0 || index >= NUM_PRESETS)
			return;
		std::vector<float> newParamValues = defaultParamValues;
		DrumPreset newDrumPreset = defaultDrumPreset;

		const auto child = packTree.getChild(index);

		if (child.hasProperty(presetStateProperty))
		{
			juce::MemoryBlock state;

			if (state.fromBase64Encoding(child.getProperty(presetStateProperty).toString()))
				decodePresetState(state, newParamValues, newDrumPreset);
		}

		const juce::ScopedLock sl(processor.getCallbackLock());
		std::swap(presetParamCache[(size_t)index], newParamValues);
		drumPresetCache[(size_t)index] = newDrumPreset;
	}

	void PresetManager::syncCurrentPresetFromParameters()
	{
		if (isInternalPresetSwap.load())
			return;

		const int index = currentIndex.load();
		auto child = packTree.getChild(index);

		if (!child.isValid())
			return;

		child.setProperty(presetStateProperty, capturePresetState(), nullptr);
		updateSinglePresetInCache(index);
	}

	void PresetManager::parameterValueChanged(int, float)
	{
		if (isInternalPresetSwap.load() || syncBlockCount.load() > 0)
			return;

		needsSync.store(true, std::memory_order_release);
		triggerAsyncUpdate();
	}

	void PresetManager::markPresetModified(bool immediate)
	{
		if (isInternalPresetSwap.load() || syncBlockCount.load() > 0)
			return;

		if (immediate)
		{
			cancelPendingUpdate();
			needsSync.store(false, std::memory_order_relaxed);
			syncCurrentPresetFromParameters();

			if (currentPackFile.existsAsFile())
				startTimer(1200);
		}
		else
		{
			needsSync.store(true, std::memory_order_release);
			triggerAsyncUpdate();
		}
	}

	void PresetManager::parameterGestureChanged(int, bool) {}

	void PresetManager::handleAsyncUpdate()
	{
		if (!needsSync.exchange(false, std::memory_order_acq_rel))
			return;

		syncCurrentPresetFromParameters();
		if (currentPackFile.existsAsFile())
			startTimer(1200);
	}

	void PresetManager::flushPendingSync()
	{
		if (juce::MessageManager::existsAndIsCurrentThread())
			handleUpdateNowIfNeeded();
	}

	void PresetManager::timerCallback()
	{
		stopTimer();

		if (currentPackFile.existsAsFile())
			savePack();
	}

	bool PresetManager::hasActivePackFile() const
	{
		return currentPackFile.existsAsFile();
	}

	void PresetManager::loadPack(const juce::File& packFile, bool initFirstPreset)
	{
		if (!packFile.existsAsFile())
			return;

		juce::XmlDocument xmlDoc(packFile);
		auto xml = xmlDoc.getDocumentElement();

		if (xml == nullptr)
			return;

		flushPendingSync();

		setPackTree(juce::ValueTree::fromXml(*xml));
		currentPackFile = packFile;
		valueTreeState.state.setProperty(packPathProperty, currentPackFile.getFullPathName(), nullptr);

		rebuildCache();

		if (initFirstPreset)
			loadPreset(0);
	}

	void PresetManager::savePack(const juce::File& targetFile)
	{
		if (targetFile != juce::File{}) {
			currentPackFile = targetFile.withFileExtension(extension);
			valueTreeState.state.setProperty(packPathProperty, currentPackFile.getFullPathName(), nullptr);
		}

		if (currentPackFile.existsAsFile() || currentPackFile.getParentDirectory().exists())
		{
			if (auto xml = packTree.createXml()) {
				xml->writeTo(currentPackFile);
			}
		}
	}

	void PresetManager::savePreset(int index, const juce::String& presetName)
	{
		auto child = packTree.getChild(index);

		if (index < 0 || index >= NUM_PRESETS || !child.isValid())
			return;

		child.setProperty("name", presetName, nullptr);
		syncCurrentPresetFromParameters();
		savePack();
	}

	void PresetManager::loadPreset(int index)
	{
		if (index < 0 || index >= NUM_PRESETS)
			return;
		flushPendingSync();

		const juce::ScopedLock sl(processor.getCallbackLock());

		auto child = packTree.getChild(index);
		if (!child.isValid())
			return;

		currentIndex.store(index);
		isInternalPresetSwap.store(true);

		if (child.hasProperty(presetStateProperty))
		{
			juce::MemoryBlock srcData;

			if (srcData.fromBase64Encoding(child.getProperty(presetStateProperty).toString()))
				processor.setStateInformation(srcData.getData(), (int)srcData.getSize());
		}
		else if (defaultStateData.getSize() > 0)
		{
			processor.setStateInformation(defaultStateData.getData(), (int)defaultStateData.getSize());
		}

		*owner.presetsParameters.ProgramIndex = index;
		owner.setCurrentProgIndex(index);

		isInternalPresetSwap.store(false);

		if (onPresetLoaded)
			onPresetLoaded(index);
	}

	void PresetManager::deletePreset(int index)
	{
		auto child = packTree.getChild(index);

		if (index < 0 || index >= NUM_PRESETS || !child.isValid())
			return;

		child.setProperty("name", initPresetName(index), nullptr);
		child.removeProperty(presetStateProperty, nullptr);

		updateSinglePresetInCache(index);
		savePack();
		loadPreset(index);
	}

	void PresetManager::moveOrSwapPreset(int fromIndex, int toIndex)
	{
		auto fromNode = packTree.getChild(fromIndex);
		auto toNode = packTree.getChild(toIndex);

		if (fromIndex == toIndex || !fromNode.isValid() || !toNode.isValid())
			return;

		flushPendingSync();

		const juce::String fromName = fromNode.getProperty("name", initPresetName(fromIndex));
		const juce::String toName = toNode.getProperty("name", initPresetName(toIndex));
		const auto fromState = fromNode.getProperty(presetStateProperty);
		const auto toState = toNode.getProperty(presetStateProperty);

		const auto moveTo = [](juce::ValueTree& node, int index, const juce::String& name, const juce::var& state)
			{
				node.setProperty("name", name.startsWithIgnoreCase("Init ") ? initPresetName(index) : name, nullptr);

				if (state.isVoid())
					node.removeProperty(presetStateProperty, nullptr);
				else
					node.setProperty(presetStateProperty, state, nullptr);
			};

		moveTo(toNode, toIndex, fromName, fromState);
		moveTo(fromNode, fromIndex, toName, toState);

		updateSinglePresetInCache(fromIndex);
		updateSinglePresetInCache(toIndex);
		savePack();

		if (currentIndex.load() == fromIndex)
			loadPreset(toIndex);
		else if (currentIndex.load() == toIndex)
			loadPreset(fromIndex);
	}

	void PresetManager::saveStateToXml(juce::XmlElement& xml)
	{
		flushPendingSync();

		xml.setAttribute("activePackPath", currentPackFile.getFullPathName());

		if (auto packXml = packTree.createXml())
		{
			packXml->setTagName(packXmlTag);
			xml.addChildElement(packXml.release());
		}
	}

	void PresetManager::loadStateFromXml(const juce::XmlElement& xml)
	{
		const juce::String savedPackPath = xml.getStringAttribute("activePackPath");
		const bool packFileExists = savedPackPath.isNotEmpty() && juce::File(savedPackPath).existsAsFile();

		if (packFileExists)
		{
			loadPack(juce::File(savedPackPath), false);
		}
		else if (auto* savedPack = xml.getChildByName(packXmlTag))
		{
			setPackTree(juce::ValueTree::fromXml(*savedPack));
			currentPackFile = juce::File{};
			rebuildCache();
		}

		currentIndex.store(juce::jlimit(0, NUM_PRESETS - 1, owner.presetsParameters.ProgramIndex->get()));
		valueTreeState.state.sendPropertyChangeMessage(packPathProperty);
	}

	int PresetManager::getNextPresetIndex() const
	{
		const int index = currentIndex.load();
		return index + 1 >= NUM_PRESETS ? 0 : index + 1;
	}

	int PresetManager::getPreviousPresetIndex() const
	{
		const int index = currentIndex.load();
		return index - 1 < 0 ? NUM_PRESETS - 1 : index - 1;
	}

	juce::StringArray PresetManager::getAllPresetsInPack() const
	{
		juce::StringArray list;
		for (int i = 0; i < NUM_PRESETS; ++i) {
			auto child = packTree.getChild(i);
			if (child.isValid()) {
				list.add(child.getProperty("name", initPresetName(i)).toString());
			}
			else {
				list.add(initPresetName(i));
			}
		}
		return list;
	}

	int PresetManager::getCurrentPresetIndex() const
	{
		return currentIndex.load();
	}

	void PresetManager::setPresetIndex(int index)
	{
		currentIndex.store(juce::jlimit(0, NUM_PRESETS - 1, index));
	}

	juce::String PresetManager::getCurrentPackName() const
	{
		return currentPackFile.getFileNameWithoutExtension();
	}

	juce::String PresetManager::getCurrentPackPath() const
	{
		return currentPackFile.getFullPathName();
	}

	void PresetManager::valueTreeRedirected(juce::ValueTree&) {}

	void PresetManager::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		if (property.toString() == packPathProperty) {
			juce::String newPackPath = treeWhosePropertyHasChanged.getPropertyAsValue(packPathProperty, nullptr).toString();

			if (newPackPath.isNotEmpty() && currentPackFile.getFullPathName() != newPackPath) {
				loadPack(juce::File(newPackPath), false);
			}
		}
	}

}
