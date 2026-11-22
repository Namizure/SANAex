#include "SimpleVoice.h"

namespace {
	const float HALF_PI = MathConstants<float>::halfPi;
	const float ONE_PI = MathConstants<float>::pi;
	const float TWO_PI = MathConstants<float>::twoPi;
}

SimpleVoice::SimpleVoice(
	ChipOscillatorParameters* chipOscParams,
	SweepParameters* sweepParams,
	VibratoParameters* vibratoParams,
	VoicingParameters* voicingParams,
	OptionsParameters* optionsParams,
	MidiEchoParameters* midiEchoParams,
	WaveformMemoryParameters* waveformMemoryParams,
	WavePatternParameters* wavePatternParams,
	ArpParameters* arpParams,
	PresetsParameters* presetsParams,
	Service::PresetManager* presetManager)
	: _chipOscParamsPtr(chipOscParams),
	_sweepParamsPtr(sweepParams),
	_vibratoParamsPtr(vibratoParams),
	_voicingParamsPtr(voicingParams),
	_optionsParamsPtr(optionsParams),
	_midiEchoParamsPtr(midiEchoParams),
	_waveformMemoryParamsPtr(waveformMemoryParams),
	_wavePatternParams(wavePatternParams),
	_arpParamsPtr(arpParams),
	_presetsParamsPtr(presetsParams),
	_presetManagerPtr(presetManager),
	ampEnv(chipOscParams->Attack->get(), chipOscParams->Decay->get(),
		chipOscParams->Sustain->get(), chipOscParams->Release->get(),
		midiEchoParams->EchoDuration->get()* midiEchoParams->EchoRepeat->get()),
	vibratoEnv(vibratoParams->VibratoAttackTime->get(), 0.1f, 1.0f, 0.1f, 0.0f),
	portaEnv(voicingParams->StepTime->get(), 0.0f, 1.0f, 0.0f, 0.0f),
	//colorEnv(chipOscParams),
	eb((std::int32_t)getSampleRate(),
		(float)midiEchoParams->EchoDuration->get(),
		midiEchoParams->EchoRepeat->get()) {
	clear();
}

bool SimpleVoice::canPlaySound(SynthesiserSound* sound) {
	return dynamic_cast<const SimpleSound*>(sound) != nullptr;
}

void SimpleVoice::startNote(int midiNoteNumber, float velocity,
	SynthesiserSound* sound,
	int currentPitchWheelPosition) {
	DBG("[StartNote] NoteNumber: " + juce::String(midiNoteNumber) +
		", Velocity: " + juce::String(velocity));
	const bool drumMode = _presetsParamsPtr->DrumMode->get();
	const Service::DrumPreset* pad = drumMode ? _presetManagerPtr->getDrumPreset(midiNoteNumber - DRUM_BASE_NOTE) : nullptr;

	if (drumMode && pad == nullptr) {
		clearCurrentNote();
		return;
	}

	const int pitchNote = (pad != nullptr) ? 60 : midiNoteNumber;
	currentMidiNote = pitchNote;

	if (!canStartNote()) {
		return;
	}
	SimpleSound* soundForPlay = dynamic_cast<SimpleSound*>(sound);
	if (soundForPlay == nullptr) {
		return;
	}
	clear();

	isDrumVoice = (pad != nullptr);
	if (isDrumVoice) {
		drumPreset = *pad;
		eb.updateParam(drumPreset.echoDuration, drumPreset.echoRepeat);
	}
	else {
		eb.updateParam(_midiEchoParamsPtr->EchoDuration->get(),
			_midiEchoParamsPtr->EchoRepeat->get());
	}

	velocity = std::max(0.01f, velocity);
	level = velocity * 0.8f;

	pitchBend = ((float)currentPitchWheelPosition - 8192.0f) / 8192.0f;
	float cyclesPerSecond = (float)MidiMessage::getMidiNoteInHertz(
		pitchNote, _optionsParamsPtr->PitchStandard->get());
	float cyclesPerSample = (float)cyclesPerSecond / (float)getSampleRate();
	angleDelta = cyclesPerSample * TWO_PI;

	ampEnv.attackStart();
	vibratoEnv.attackStart();
	portaEnv.attackStart();
	//colorEnv.clear();
	patternWaveClear();
	currentWaveIndex = isDrumVoice ? drumPreset.waveType : _chipOscParamsPtr->OscWaveType->getIndex();

	if (isDrumVoice ? drumPreset.patternEnabled : _wavePatternParams->PatternEnabled->get()) {
		patternIndex = -1;
		patternCounter = 99999999;
	}

	if (isDrumVoice ? drumPreset.arpEnabled : _arpParamsPtr->ArpEnabled->get()) {
		arpIndex = -1;
		arpCounter = 99999999;
	}
}

void SimpleVoice::stopNote(float /*velocity*/, bool allowTailOff) {
	DBG("stopNote : " + juce::String((std::int32_t)allowTailOff));

	if (allowTailOff) {
		portaAngleDelta = 0.0f;
		ampEnv.releaseStart();
		return;
	}
	if (ampEnv.isHolding()) {
		portaAngleDelta = angleDelta;
		ampEnv.releaseStart();
		return;
	}

	portaAngleDelta = 0.0f;

	clear();
	clearCurrentNote();
}

void SimpleVoice::pitchWheelMoved(int newPitchWheelValue) {
	pitchBend = ((float)newPitchWheelValue - 8192.0f) / 8192.0f;
}

void SimpleVoice::controllerMoved(int x,
	int y) {
}

void SimpleVoice::renderNextBlock(AudioBuffer<float>& outputBuffer,
	int startSample, int numSamples) {

	const Service::DrumPreset* pad = isDrumVoice ? &drumPreset : nullptr;

	auto activeWaveIndex = currentWaveIndex;
	auto isEchoEnabled = pad ? pad->echoEnabled : _midiEchoParamsPtr->IsEchoEnable->get();
	auto echoRepeatCount = pad ? pad->echoRepeat : _midiEchoParamsPtr->EchoRepeat->get();
	auto echoDuration = pad ? pad->echoDuration : _midiEchoParamsPtr->EchoDuration->get();
	auto echoVolumeOffset = pad ? pad->echoVolumeOffset : _midiEchoParamsPtr->VolumeOffset->get();
	auto isVibratoEnabled = pad ? pad->vibratoEnabled : _vibratoParamsPtr->VibratoEnable->get();
	auto isInVibratoDelay =
		((pad ? pad->vibratoAttackDelaySwitch : _vibratoParamsPtr->VibratoAttackDeleySwitch->get()) == false) &&
		(vibratoEnv.getState() == AmpEnvelope::AMPENV_STATE::ATTACK);
	auto vibratoSpeed = pad ? pad->vibratoSpeed : _vibratoParamsPtr->VibratoSpeed->get();
	auto pitchBendRange = _optionsParamsPtr->PitchBendRange->get();
	auto isPortaMode = !pad && (_voicingParamsPtr->VoicingSwitch->getCurrentChoiceName() == "PORTAMENTO");
	auto sweepMode = pad ? pad->sweepMode : _sweepParamsPtr->SweepSwitch->getIndex();
	auto isPositiveSweepEnbaled = (sweepMode == 1);
	auto isNegativeSweepEnbaled = (sweepMode == 2);
	auto sweepTime = pad ? pad->sweepTime : (float)_sweepParamsPtr->SweepTime->get();
	auto isPatternWaveEnabled = pad ? pad->patternEnabled : _wavePatternParams->PatternEnabled->get();
	auto isPatternLoopEnabled = pad ? pad->patternLoopEnabled : _wavePatternParams->LoopEnabled->get();
	patternStepNum = (pad ? pad->patternStepTime : (float)_wavePatternParams->StepTime->get()) * getSampleRate();

	auto isArpEnabled = pad ? pad->arpEnabled : _arpParamsPtr->ArpEnabled->get();
	auto isArpLoopEnabled = pad ? pad->arpLoopEnabled : _arpParamsPtr->LoopEnabled->get();
	auto isFixedArp = pad ? pad->arpFixed : _arpParamsPtr->FixedEnabled->get();
	arpStepNum = (pad ? pad->arpStepTime : _arpParamsPtr->ArpStepTime->get()) * getSampleRate();

	const bool* arpEndSteps = pad ? pad->stepFlags.arpEnd : _arpParamsPtr->isEndStep;
	const bool* arpLoopSteps = pad ? pad->stepFlags.arpLoop : _arpParamsPtr->isLoopStep;
	const bool* patternEndSteps = pad ? pad->stepFlags.wavePatternEnd : _wavePatternParams->isEndStep;
	const bool* patternLoopSteps = pad ? pad->stepFlags.wavePatternLoop : _wavePatternParams->isLoopStep;

	const float presetGain = pad
		? Decibels::decibelsToGain(pad->volumeDb - _chipOscParamsPtr->VolumeLevel->get())
		: 1.0f;

	SimpleSound* playingSound = static_cast<SimpleSound*>(getCurrentlyPlayingSound().get());
	if (playingSound == nullptr) {
		clear();
		return;
	}

	updateEnvParams(ampEnv, vibratoEnv, portaEnv);
	eb.updateParam(echoDuration, echoRepeatCount);

	while (--numSamples >= 0) {
		if ((ampEnv.isReleaseEnded()) ||
			(isEchoEnabled && ampEnv.isEchoEnded())) {
			clearCurrentNote();
			clear();
			break;
		}
		float modulationFactor = 0.0f;
		if (!(isVibratoEnabled) || isInVibratoDelay) {
			modulationFactor = 0.0f;
		}
		else {
			modulationFactor = calcModulationFactor(vibratoAngle) * vibratoEnv.getValue();
		}

		const auto pitchBendFactor = pow(2.0f, pitchBend / 12.0f * pitchBendRange);
		const auto pitchModulationFactor = pow(2.0f, modulationFactor / 12.0f);
		const auto sweepFactor = pow(2.0f, pitchSweep);

		float customArpFactor = 1.0f;
		//////////////////////
		if (isArpEnabled) {
			int actualEndStepIndex = -1;
			int actualLoopStepIndex = 0;

			for (int i = 0; i < ArpParameters::ARP_LENGTH; i++) {
				if (arpEndSteps[i]) {
					actualEndStepIndex = i;
				}
				if (arpLoopSteps[i]) {
					actualLoopStepIndex = i;
				}
			}
			// if loop is placed after end, set to 0 just in case
			if (actualEndStepIndex >= 0 && actualLoopStepIndex > actualEndStepIndex) {
				actualLoopStepIndex = 0;
			}

			// check arpLoopLength based on eithe rMAX length or the end step
			int arpLoopLength = (actualEndStepIndex >= 0) ? (actualEndStepIndex + 1) : ArpParameters::ARP_LENGTH;

			if (arpIndex >= arpLoopLength) {
				arpIndex = isArpLoopEnabled ? actualLoopStepIndex : std::max(0, arpLoopLength - 1);
			}

			bool canAdvance = isArpLoopEnabled || (arpIndex < arpLoopLength - 1);

			if (canAdvance) {
				arpCounter++;
				if (arpCounter >= arpStepNum) {
					arpCounter = 0;
					int previousArpIndex = arpIndex;
					arpIndex++;

					if (arpIndex >= arpLoopLength) {
						if (isArpLoopEnabled) {
							arpIndex = actualLoopStepIndex;
						}
						else {
							arpIndex = arpLoopLength - 1;
						}
					}
				}
			}
			else {
				arpIndex = std::max(0, arpLoopLength - 1);
			}

			const int arpStep = std::max(0, std::min(arpIndex, ArpParameters::ARP_LENGTH - 1));
			int arpDegree = pad ? pad->arpPattern[arpStep] : _arpParamsPtr->ArpPatternArray[arpStep]->get();

			if (isFixedArp) {
				const int fixedRootNote = 60;
				customArpFactor = pow(2.0f, (float)(fixedRootNote + arpDegree - currentMidiNote) / 12.0f);
			}
			else {
				customArpFactor = pow(2.0f, (float)arpDegree / 12.0f);
			}
		}

		float totalPitchFactor = pitchBendFactor * pitchModulationFactor * sweepFactor * customArpFactor;
		float modulatedDelta = angleDelta * totalPitchFactor;
		auto currentSample = angle2wave(currentAngle, modulatedDelta, activeWaveIndex);
		currentSample *= ampEnv.getValue() * level * presetGain;
		if (isEchoEnabled) {
			eb.addSample(currentSample, echoVolumeOffset / 100.0f);
			eb.cycle();

			for (auto channelNum = outputBuffer.getNumChannels(); --channelNum >= 0;) {
				for (auto i = 0; i < echoRepeatCount; ++i) {
					outputBuffer.addSample(channelNum, startSample, eb.getSample(i));
				}
			}
		}
		for (auto channelNum = outputBuffer.getNumChannels(); --channelNum >= 0;) {
			outputBuffer.addSample(channelNum, startSample, currentSample);
		}
		++startSample;

		currentAngle += modulatedDelta;

		if (isPortaMode) {
			if (portaAngleDelta > 0.0f) {
				currentAngle -= (angleDelta - portaAngleDelta) * (1 - portaEnv.getValue());
			}
		}

		vibratoAngle += ((vibratoSpeed) / (float)getSampleRate()) * TWO_PI;

		if (isPositiveSweepEnbaled) {
			pitchSweep += 1 / (float)getSampleRate() / sweepTime;
			pitchSweep = std::min(10.0f, pitchSweep);
		}
		else if (isNegativeSweepEnbaled) {
			pitchSweep -= 1 / (float)getSampleRate() / sweepTime;
			pitchSweep = std::max(-10.0f, pitchSweep);
		}
		////////////////////////////
		if (isPatternWaveEnabled) {
			int actualEndStepIndex = -1;
			int actualLoopStepIndex = 0;

			for (int i = 0; i < WAVEPATTERN_LENGTH; i++) {
				if (patternEndSteps[i]) {
					actualEndStepIndex = i;
				}
				if (patternLoopSteps[i]) {
					actualLoopStepIndex = i;
				}
			}

			// if loop is placed after end, set to 0 just in case
			if (actualEndStepIndex >= 0 && actualLoopStepIndex > actualEndStepIndex) {
				actualLoopStepIndex = 0;
			}

			// check wavelooplength based on eithe rMAX length or the end step
			int waveLoopLength = (actualEndStepIndex >= 0) ? (actualEndStepIndex + 1) : WAVEPATTERN_LENGTH;


			if (patternIndex >= waveLoopLength) {
				patternIndex = isPatternLoopEnabled ? actualLoopStepIndex : std::max(0, waveLoopLength - 1);
			}

			bool canAdvance = isPatternLoopEnabled || (patternIndex < waveLoopLength - 1);

			if (canAdvance) {
				patternCounter++;
				if (patternCounter >= patternStepNum) {
					patternCounter = 0;
					patternIndex++;

					if (patternIndex >= waveLoopLength) {
						if (isPatternLoopEnabled) {
							patternIndex = actualLoopStepIndex;
						}
						else {
							patternIndex = waveLoopLength - 1;
						}
					}
				}
			}
			else {
				patternIndex = std::max(0, waveLoopLength - 1);
			}

			const int patternStep = std::max(0, std::min(patternIndex, WAVEPATTERN_LENGTH - 1));
			int patternSlot = pad ? pad->wavePattern[patternStep] : _wavePatternParams->WavePatternArray[patternStep]->get();
			if (patternSlot >= 0 && patternSlot < WAVEPATTERN_TYPES) {
				activeWaveIndex = pad ? pad->patternWaveTypes[patternSlot] : _wavePatternParams->WaveTypes[patternSlot]->get();
				currentWaveIndex = activeWaveIndex;
			}
		}

		currentAngle = fmod(currentAngle, TWO_PI);
		vibratoAngle = fmod(vibratoAngle, TWO_PI);

		ampEnv.cycle((float)getSampleRate());
		vibratoEnv.cycle((float)getSampleRate());
		portaEnv.cycle((float)getSampleRate());
		//colorEnv.cycle((float)getSampleRate());
	}
}

void SimpleVoice::clear() {
	currentAngle = 0.0f;
	vibratoAngle = 0.0f;
	angleDelta = 0.0f;
	// portaAngleDelta = 0.0f; // portaはリセットしない ... OK!
	level = 0.0f;
	pitchBend = 0.0f;
	pitchSweep = 0.0f;
	patternWaveClear();
	arpCounter = 0;
	arpIndex = 0;
	arpStepNum = 0.0f;
	waveForms.init();
}

void SimpleVoice::patternWaveClear() {
	patternCounter = 0;
	patternIndex = 0;
	patternStepNum = 0.0f;
}

float SimpleVoice::calcModulationFactor(float angle) {
	float factor = waveForms.sine(angle);
	factor *= isDrumVoice ? drumPreset.vibratoAmount : _vibratoParamsPtr->VibratoAmount->get();
	return factor;
}

float SimpleVoice::angle2wave(float angle, float angleDelta, int waveIndex) {
	switch (waveIndex) {
	case 0: return waveForms.nesSquare(angle);
	case 1: return waveForms.nesSquare25(angle);
	case 2: return waveForms.nesSquare125(angle);
	case 3: return waveForms.nesTriangle(angle);
	case 4: return waveForms.square(angle);
	case 5: return waveForms.square25(angle);
	case 6: return waveForms.square125(angle);
	case 7: return waveForms.triangle(angle);
	case 8: return waveForms.sine(angle);
	case 9: return waveForms.saw(angle);
	case 10: return waveForms.longNoise(angleDelta);
	case 11: return waveForms.shortNoise(angleDelta);
	case 12: return waveForms.noise(angleDelta);
	case 13: return waveForms.roughSine(angle);
	case 14: return waveForms.roughSaw(angle);
	case 15: return waveForms.lobitNoise(angleDelta);
	default: break;
	}
	if (waveIndex >= 16) {
		if (isDrumVoice) {
			const int memoryIndex = waveIndex - 16;
			return waveForms.waveformMemory(angle, drumPreset.waveSamples[memoryIndex < drumPreset.numWaveforms ? memoryIndex : 0]);
		}
		return waveForms.waveformMemory(angle, _waveformMemoryParamsPtr, waveIndex - 16);
	}
	return 0.0f;
}

bool SimpleVoice::canStartNote() {
	if (ampEnv.isReleasing() || ampEnv.isReleaseEnded() || ampEnv.isEchoEnded()) {
		return true;
	}

	return false;
}

void SimpleVoice::updateEnvParams(AmpEnvelope& ampEnv, AmpEnvelope& vibratoEnv, AmpEnvelope& portaEnv) {
	if (isDrumVoice) {
		ampEnv.setParameters(drumPreset.attack, drumPreset.decay, drumPreset.sustain, drumPreset.release,
			drumPreset.echoDuration * drumPreset.echoRepeat);
		vibratoEnv.setParameters(drumPreset.vibratoAttackTime, 0.1f, 1.0f, 0.1f, 0.0f);
	}
	else {
		ampEnv.setParameters(_chipOscParamsPtr->Attack->get(),
			_chipOscParamsPtr->Decay->get(),
			_chipOscParamsPtr->Sustain->get(),
			_chipOscParamsPtr->Release->get(),
			_midiEchoParamsPtr->EchoDuration->get() *
			_midiEchoParamsPtr->EchoRepeat->get());
		vibratoEnv.setParameters(_vibratoParamsPtr->VibratoAttackTime->get(), 0.1f, 1.0f, 0.1f, 0.0f);
	}
	portaEnv.setParameters(_voicingParamsPtr->StepTime->get(), 0.0f, 1.0f, 0.0f, 0.0f);
}