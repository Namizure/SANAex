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
	ArpParameters* arpParams)
	: _chipOscParamsPtr(chipOscParams),
	_sweepParamsPtr(sweepParams),
	_vibratoParamsPtr(vibratoParams),
	_voicingParamsPtr(voicingParams),
	_optionsParamsPtr(optionsParams),
	_midiEchoParamsPtr(midiEchoParams),
	_waveformMemoryParamsPtr(waveformMemoryParams),
	_wavePatternParams(wavePatternParams),
	_arpParamsPtr(arpParams),
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

	if (!canStartNote()) {
		return;
	}
	SimpleSound* soundForPlay = dynamic_cast<SimpleSound*>(sound);
	if (soundForPlay == nullptr) {
		return;
	}
	clear();

	eb.updateParam(_midiEchoParamsPtr->EchoDuration->get(),
		_midiEchoParamsPtr->EchoRepeat->get());

	velocity = std::max(0.01f, velocity);
	level = velocity * 0.8f;

	pitchBend = ((float)currentPitchWheelPosition - 8192.0f) / 8192.0f;

	// 生成する波形のピッチを再現するサンプルデータ間の角度差⊿θ[rad]の値を決定する。
	float cyclesPerSecond = (float)MidiMessage::getMidiNoteInHertz(
		midiNoteNumber, _optionsParamsPtr->PitchStandard->get());
	float cyclesPerSample = (float)cyclesPerSecond / (float)getSampleRate();
	angleDelta = cyclesPerSample * TWO_PI;

	ampEnv.attackStart();
	vibratoEnv.attackStart();
	portaEnv.attackStart();
	//colorEnv.clear();
	patternWaveClear();
	currentWaveIndex = _chipOscParamsPtr->OscWaveType->getIndex();

	// 波形パターン初期設定
	if (_wavePatternParams->PatternEnabled->get()) {
		patternIndex = -1;
		patternCounter = 99999999;
	}

	if (_arpParamsPtr->ArpEnabled->get()) {
		arpIndex = -1;
		arpCounter = 99999999;
	}
}

/// キーリリースだとallowTailOff == true
/// キーリリース直後のボイススチールではallowTailOff == false
void SimpleVoice::stopNote(float /*velocity*/, bool allowTailOff) {
	DBG("stopNote : " + juce::String((std::int32_t)allowTailOff));

	if (allowTailOff) {
		portaAngleDelta = 0.0f;
		ampEnv.releaseStart();
		return;
	}
	// キーホールド中(ADSのいずれか)であればangleDeltaをリリース状態に移行
	if (ampEnv.isHolding()) {
		//ポルタメント処理，一つ前のピッチを記憶しておく
		portaAngleDelta = angleDelta;

		// NOTE: ボイススチールを受けて直ぐに音量を0にしてしまうと、急峻な変化となりノイズの発生を引き起こすため、それを予防する処理。
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

void SimpleVoice::controllerMoved(int /*controllerNumber*/,
	int /*newControllerValue*/) {
}

void SimpleVoice::renderNextBlock(AudioBuffer<float>& outputBuffer,
	int startSample, int numSamples) {
	// 現状のパラメータを取得しておく
	// NOTE: 適宜取得すると何度も取得関数が走るため重くなる
	auto activeWaveIndex = currentWaveIndex;
	auto isEchoEnabled = _midiEchoParamsPtr->IsEchoEnable->get();
	auto echoRepeatCount = _midiEchoParamsPtr->EchoRepeat->get();
	auto isVibratoEnabled = _vibratoParamsPtr->VibratoEnable->get();
	auto isInVibratoDelay =
		(_vibratoParamsPtr->VibratoAttackDeleySwitch->get() == false) &&
		(vibratoEnv.getState() == AmpEnvelope::AMPENV_STATE::ATTACK);
	auto vibratoSpeed = _vibratoParamsPtr->VibratoSpeed->get();
	auto pitchBendRange = _optionsParamsPtr->PitchBendRange->get();
	auto isPortaMode = (_voicingParamsPtr->VoicingSwitch->getCurrentChoiceName() == "PORTAMENTO");
	auto isPositiveSweepEnbaled = (_sweepParamsPtr->SweepSwitch->getCurrentChoiceName() == "Positive");
	auto isNegativeSweepEnbaled = (_sweepParamsPtr->SweepSwitch->getCurrentChoiceName() == "Negative");
	auto sweepTime = (float)_sweepParamsPtr->SweepTime->get();
	auto isPatternWaveEnabled = (float)_wavePatternParams->PatternEnabled->get();
	auto isPatternLoopEnabled = (float)_wavePatternParams->LoopEnabled->get();
	patternStepNum = (float)_wavePatternParams->StepTime->get() * getSampleRate();

	auto isArpEnabled = _arpParamsPtr->ArpEnabled->get();
	arpStepNum = _arpParamsPtr->ArpStepTime->get() * getSampleRate();

	SimpleSound* playingSound = static_cast<SimpleSound*>(getCurrentlyPlayingSound().get());
	if (playingSound == nullptr) {
		clear();
		return;
	}

	updateEnvParams(ampEnv, vibratoEnv, portaEnv);
	eb.updateParam(_midiEchoParamsPtr->EchoDuration->get(), _midiEchoParamsPtr->EchoRepeat->get());

	while (--numSamples >= 0) {
		// エンベロープにおいて，エフェクトエコーが終わっている or
		// リリース状態のとき
		if ((ampEnv.isReleaseEnded()) ||
			(isEchoEnabled && ampEnv.isEchoEnded())) {
			clearCurrentNote();
			clear();
			break;
		}

		// 現在のサンプル値を計算する 
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
		if (isArpEnabled) {

			int actualEndStepIndex = -1;
			for (int i = 0; i < ArpParameters::ARP_LENGTH; i++) {
				if (_arpParamsPtr->isEndStep[i]) {
					actualEndStepIndex = i;
					break;
				}
			}

			int arpLoopLength = ArpParameters::ARP_LENGTH;
			if (actualEndStepIndex >= 0) {
				arpLoopLength = actualEndStepIndex + 1;
			}
			else {
				for (int i = ArpParameters::ARP_LENGTH - 1; i >= 0; i--) {
					if (_arpParamsPtr->ArpPatternArray[i]->get() != 0) {
						arpLoopLength = i + 1;
						break;
					}
				}
			}
			bool canAdvance = _arpParamsPtr->LoopEnabled->get() || (arpIndex < arpLoopLength - 1);
			if (canAdvance) {
				arpCounter++;
				if (arpCounter >= arpStepNum) {
					arpCounter = 0;
					int previousArpIndex = arpIndex;
					arpIndex++;
					if (arpIndex >= arpLoopLength) {
						int prevDegree = _arpParamsPtr->ArpPatternArray[std::max(0, std::min(previousArpIndex, ArpParameters::ARP_LENGTH - 1))]->get();
						int nextDegree = _arpParamsPtr->ArpPatternArray[0]->get();
						float prevFactor = pow(2.0f, (float)prevDegree / 12.0f);
						float nextFactor = pow(2.0f, (float)nextDegree / 12.0f);

						if (nextFactor > 0.0f) {
							currentAngle = fmod(currentAngle * (prevFactor / nextFactor), TWO_PI);
						}

						arpIndex = 0;
					}
				}
			}
			else {
				arpIndex = std::max(0, arpLoopLength - 1);
			}

			int arpDegree = _arpParamsPtr->ArpPatternArray[std::max(0, std::min(arpIndex, ArpParameters::ARP_LENGTH - 1))]->get();
			customArpFactor = pow(2.0f, (float)arpDegree / 12.0f);
		}

		//const auto colorFactor = colorEnv.getManipulateAngle() + 1;
		float totalPitchFactor = pitchBendFactor * pitchModulationFactor * sweepFactor * customArpFactor;
		float modulatedDelta = angleDelta * totalPitchFactor;
		auto currentSample = angle2wave(currentAngle, modulatedDelta, activeWaveIndex);
		currentSample *= ampEnv.getValue() * level;

		//エコー処理とエコーレンダリング 
		if (isEchoEnabled) {
			eb.addSample(currentSample, _midiEchoParamsPtr->VolumeOffset->get() / 100.0f);
			eb.cycle();

			for (auto channelNum = outputBuffer.getNumChannels(); --channelNum >= 0;) {
				for (auto i = 0; i < echoRepeatCount; ++i) {
					outputBuffer.addSample(channelNum, startSample, eb.getSample(i));
				}
			}
		}
		// バッファ書き込み
		for (auto channelNum = outputBuffer.getNumChannels(); --channelNum >= 0;) {
			outputBuffer.addSample(channelNum, startSample, currentSample);
		}
		++startSample;

		currentAngle += modulatedDelta;

		// ポルタメントをcurrentAngleに反映 
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

		if (isPatternWaveEnabled) {
			int actualEndStepIndex = -1;
			for (int i = 0; i < WAVEPATTERN_LENGTH; i++) {
				if (_wavePatternParams->isEndStep[i]) {
					actualEndStepIndex = i;
					break;
				}
			}
			int waveLoopLength = WAVEPATTERN_LENGTH;
			if (actualEndStepIndex >= 0) {
				waveLoopLength = actualEndStepIndex + 1;
			}
			else {

				for (int i = WAVEPATTERN_LENGTH - 1; i >= 0; i--) {
					if (_wavePatternParams->WavePatternArray[i]->get() != 0) {
						waveLoopLength = i + 1;
						break;
					}
				}
			}

			bool canAdvance = isPatternLoopEnabled || (patternIndex < waveLoopLength - 1);

			if (canAdvance) {
				patternCounter++;
				if (patternCounter >= patternStepNum) {
					patternCounter = 0;

					patternIndex++;
					if (patternIndex >= waveLoopLength) {
						if (isPatternLoopEnabled) {
							patternIndex = 0;
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

			int patternSlot = _wavePatternParams->WavePatternArray[std::max(0, std::min(patternIndex, WAVEPATTERN_LENGTH - 1))]->get();
			if (patternSlot >= 0 && patternSlot < WAVEPATTERN_TYPES) {
				activeWaveIndex = _wavePatternParams->WaveTypes[patternSlot]->get();
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

	// factorの値が0.5を中心とした0.0～1.0の値となるように調整する。
	factor *= _vibratoParamsPtr->VibratoAmount->get();
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
	ampEnv.setParameters(_chipOscParamsPtr->Attack->get(),
		_chipOscParamsPtr->Decay->get(),
		_chipOscParamsPtr->Sustain->get(),
		_chipOscParamsPtr->Release->get(),
		_midiEchoParamsPtr->EchoDuration->get() *
		_midiEchoParamsPtr->EchoRepeat->get());
	vibratoEnv.setParameters(_vibratoParamsPtr->VibratoAttackTime->get(), 0.1f, 1.0f, 0.1f, 0.0f);
	portaEnv.setParameters(_voicingParamsPtr->StepTime->get(), 0.0f, 1.0f, 0.0f, 0.0f);
}