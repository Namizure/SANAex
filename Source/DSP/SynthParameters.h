#pragma once
#include <JuceHeader.h>

using namespace std;


namespace {
	const std::int32_t WAVESAMPLE_LENGTH = 32;
	const std::int32_t WAVEPATTERN_LENGTH = 16;
	const std::int32_t WAVEPATTERN_TYPES = 16;
	const std::int32_t NUM_OF_PRESETS = 12;
	const std::int32_t VOICE_MAX = 8;
	const std::int32_t UP_SAMPLING_FACTOR = 2;

	namespace {
		StringArray OSC_WAVE_TYPES{
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
	}

	const StringArray OSC_COLOR_TYPES{
	  "NONE",
	  "ARP_Octave",
	  "ARP_4th",
	  "ARP_5th",
	  "ARP_Major",
	  "ARP_Major7th",
	  "ORC_HIT",
	  "ORC_HIT2",
	  "ORC_HIT3",
	};
}

class SynthParametersBase {
public:
	virtual ~SynthParametersBase() {};

	virtual void addAllParameters(AudioProcessor& processor) = 0;
	virtual void saveParameters(XmlElement& xml) = 0;
	virtual void loadParameters(XmlElement& xml) = 0;
};

class ChipOscillatorParameters : public SynthParametersBase {
public:
	AudioParameterChoice* OscWaveType;
	AudioParameterFloat* VolumeLevel;
	AudioParameterFloat* Attack;
	AudioParameterFloat* Decay;
	AudioParameterFloat* Sustain;
	AudioParameterFloat* Release;
	AudioParameterChoice* ColorType;
	AudioParameterFloat* ColorDuration;

	ChipOscillatorParameters(AudioParameterChoice* OscWaveType,
		AudioParameterFloat* volumeLevel,
		AudioParameterFloat* attack,
		AudioParameterFloat* decay,
		AudioParameterFloat* sustain,
		AudioParameterFloat* release,
		AudioParameterChoice* colorType,
		AudioParameterFloat* colorDuration);

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;

private:
	ChipOscillatorParameters() {};
};

class SweepParameters : public SynthParametersBase {
public:
	AudioParameterChoice* SweepSwitch;
	AudioParameterFloat* SweepTime;

	SweepParameters(AudioParameterChoice* SweepSwitch,
		AudioParameterFloat* SweepTime);

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;

private:
	SweepParameters() {};
};

class VibratoParameters : public SynthParametersBase {
public:
	AudioParameterBool* VibratoEnable;
	AudioParameterBool* VibratoAttackDeleySwitch;
	AudioParameterFloat* VibratoAmount;
	AudioParameterFloat* VibratoSpeed;
	AudioParameterFloat* VibratoAttackTime;

	VibratoParameters(AudioParameterBool* vibratoEnable,
		AudioParameterBool* attackDeleySwitch,
		AudioParameterFloat* vibratoAmount,
		AudioParameterFloat* vibratoSpeed,
		AudioParameterFloat* vibratoAttackTime);

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;

private:
	VibratoParameters() {};
};

class VoicingParameters : public SynthParametersBase {
public:
	AudioParameterChoice* VoicingSwitch;
	AudioParameterFloat* StepTime;

	VoicingParameters(AudioParameterChoice* sweepSwitch,
		AudioParameterFloat* stepTime);

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;

private:
	VoicingParameters() {};
};

class OptionsParameters : public SynthParametersBase {
public:
	AudioParameterInt* PitchBendRange;
	AudioParameterInt* PitchStandard;
	float currentBPM;

	OptionsParameters(AudioParameterInt* pitchBendRange,
		AudioParameterInt* pitchStandard);

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;

private:
	OptionsParameters() {};
};


// -------------------------------------------------------

class WaveformMemoryParameters : public SynthParametersBase {
public:

	WaveformMemoryParameters();
	//~WaveformMemoryParameters();


	static const int MAX_WAVEFORMS = 20;
	static const int WAVESAMPLE_LENGTH = 32;

	vector<AudioParameterInt*> WaveSamplesArrays[MAX_WAVEFORMS];
	int currentNumWaveforms = 4;


	void addNewWaveform(AudioProcessor& processor);
	AudioParameterInt** getWaveformArray(int waveformIndex);
	int getNumWaveforms() const { return currentNumWaveforms; }

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;

private:
};

// -------------------------------------------------------



class MidiEchoParameters : public SynthParametersBase {
public:
	AudioParameterBool* IsEchoEnable;
	AudioParameterFloat* EchoDuration;
	AudioParameterInt* EchoRepeat;
	AudioParameterFloat* VolumeOffset;

	MidiEchoParameters(AudioParameterBool* isEchoEnable,
		AudioParameterFloat* echoDuration,
		AudioParameterInt* echoRepeat,
		AudioParameterFloat* volumeOffset);

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;

private:
	MidiEchoParameters() {};
};

class FilterParameters : public SynthParametersBase {
public:
	AudioParameterBool* HicutEnable;
	AudioParameterBool* LowcutEnable;
	AudioParameterFloat* HicutFreq;
	AudioParameterFloat* LowcutFreq;

	FilterParameters(AudioParameterBool* hicutEnable,
		AudioParameterBool* lowcutEnable,
		AudioParameterFloat* hicutFreq,
		AudioParameterFloat* lowcutFreq);

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;

private:
	FilterParameters() {};
};

class PresetsParameters : public SynthParametersBase {
public:
	AudioParameterInt* ProgramIndex;
	PresetsParameters();

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;

private:
};

class WavePatternParameters : public SynthParametersBase {
public:
	AudioParameterInt* WavePatternArray[WAVEPATTERN_LENGTH];
	AudioParameterInt* WaveTypes[WAVEPATTERN_TYPES];
	AudioParameterBool* PatternEnabled;
	AudioParameterBool* LoopEnabled;
	AudioParameterFloat* StepTime;
	bool isEndStep[WAVEPATTERN_LENGTH] = { false };
	int endStepIndex;


	// track waveform names
	StringArray storedWaveformNames;

	WavePatternParameters();

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;

private:
};

class ArpParameters : public SynthParametersBase {
public:
	static const int ARP_LENGTH = 16;
	AudioParameterInt* ArpPatternArray[ARP_LENGTH];
	AudioParameterBool* ArpEnabled;
	AudioParameterFloat* ArpStepTime;
	AudioParameterBool* LoopEnabled;
	bool isEndStep[ARP_LENGTH] = { false };
	int endStepIndex;

	ArpParameters();

	virtual void addAllParameters(AudioProcessor& processor) override;
	virtual void saveParameters(XmlElement& xml) override;
	virtual void loadParameters(XmlElement& xml) override;
};
