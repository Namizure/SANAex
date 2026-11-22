#pragma once

#include <JuceHeader.h>
#include "GUI/ParametersComponent.h"
#include "GUI/ScopeComponent.hpp"
#include "GUI/PresetUI.h"
#include "PluginProcessor.h"

class DrumKeyboardComponent : public juce::MidiKeyboardComponent
{
public:
	DrumKeyboardComponent(PluginProcessor& proc, juce::MidiKeyboardState& state, Orientation orient)
		: MidiKeyboardComponent(state, orient), processor(proc)
	{
		setOctaveForMiddleC(5);
	}

	std::function<void()> onPresetsChanged;

	void setDrumMode(bool shouldBeDrumMode) {
		isDrumMode = shouldBeDrumMode;

		if (isDrumMode) {
			setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xff181a1d));
			setKeyWidth(96.0f);
			setBlackNoteLengthProportion(1.0f);
			setScrollButtonWidth(22);
			setAvailableRange(DRUM_BASE_NOTE, 127);
			setLowestVisibleKey(DRUM_BASE_NOTE);
		}
		else {
			setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colours::white);
			setKeyWidth(PIANO_KEY_WIDTH);
			setBlackNoteLengthProportion(0.7f);
			setScrollButtonWidth((int)PIANO_SCROLL_WIDTH);
			setAvailableRange(0, 127);
			setLowestVisibleKey(PIANO_LOWEST_KEY);
		}

		repaint();
	}

	juce::Range<float> getKeyPosition(int midiNoteNumber, float width) const override {
		if (isDrumMode) {
			float startX = (float)getScrollButtonWidth();
			float start = startX + (float)(midiNoteNumber - getLowestVisibleKey()) * width;
			return { start, start + width };
		}

		return juce::MidiKeyboardComponent::getKeyPosition(midiNoteNumber, width);
	}

	void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override {
		float delta = (wheel.deltaY != 0.0f ? wheel.deltaY : wheel.deltaX);

		if (isDrumMode && delta != 0.0f) {
			int step = (delta > 0.0f ? -1 : 1);
			setLowestVisibleKey(juce::jlimit(getRangeStart(), getRangeEnd() - 5, getLowestVisibleKey() + step));
			repaint();
			return;
		}

		juce::MidiKeyboardComponent::mouseWheelMove(e, wheel);
	}

	juce::String getWhiteNoteText(int midiNoteNumber) override {
		if (isDrumMode) {
			return {};
		}

		return juce::MidiKeyboardComponent::getWhiteNoteText(midiNoteNumber);
	}

	void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
		bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override {
		if (!isDrumMode) {
			juce::MidiKeyboardComponent::drawWhiteNote(midiNoteNumber, g, area, isDown, isOver, lineColour, textColour);
			return;
		}

		drawPad(midiNoteNumber, g, area, isDown);
	}

	void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
		bool isDown, bool isOver, juce::Colour noteColour) override {
		if (!isDrumMode) {
			juce::MidiKeyboardComponent::drawBlackNote(midiNoteNumber, g, area, isDown, isOver, noteColour);
			return;
		}

		area.setHeight((float)getHeight());
		drawPad(midiNoteNumber, g, area, isDown);
	}

	void mouseDown(const juce::MouseEvent& e) override {
		const int clickedNote = getNoteForPosition(e.position);
		const int presetIdx = clickedNote - DRUM_BASE_NOTE;

		if (isDrumMode && isPresetIndex(presetIdx)) {
			if (processor.getPresetManager().getCurrentPresetIndex() != presetIdx) {
				processor.getPresetManager().loadPreset(presetIdx);

				if (onPresetsChanged)
					onPresetsChanged();
			}
		}

		MidiKeyboardComponent::mouseDown(e);
		dragSourcePos = e.position;

		const bool canDrag = isDrumMode && e.mods.isLeftButtonDown()
			&& isPresetIndex(presetIdx) && getNonInitPresetName(presetIdx).isNotEmpty();

		dragSourceNote = canDrag ? clickedNote : -1;
		dragPresetIdx = canDrag ? presetIdx : -1;

		setMouseCursor(juce::MouseCursor::NormalCursor);
		isDragging = false;
		hoverNote = -1;
	}

	void mouseDrag(const juce::MouseEvent& e) override {
		if (isDrumMode && dragPresetIdx >= 0) {
			if (!isDragging && (e.getDistanceFromDragStart() > 6 || getNoteForPosition(e.position) != dragSourceNote))
				isDragging = true;

			if (isDragging) {
				setMouseCursor(juce::MouseCursor::DraggingHandCursor);
				hoverNote = getNoteForPosition(e.position);
				repaint();
				return;
			}
		}
		else {
			setMouseCursor(juce::MouseCursor::NormalCursor);
		}

		MidiKeyboardComponent::mouseDrag(e);
	}

	void mouseUp(const juce::MouseEvent& e) override {
		if (isDragging) {
			const int targetIdx = getNoteForPosition(e.position) - DRUM_BASE_NOTE;
			const int sourceIdx = dragSourceNote - DRUM_BASE_NOTE;

			if (isPresetIndex(targetIdx) && isPresetIndex(sourceIdx) && targetIdx != sourceIdx) {
				processor.getPresetManager().moveOrSwapPreset(sourceIdx, targetIdx);

				if (onPresetsChanged)
					onPresetsChanged();
			}

			isDragging = false;
			dragPresetIdx = -1;
			hoverNote = -1;
			setMouseCursor(juce::MouseCursor::NormalCursor);
			repaint();
			MidiKeyboardComponent::mouseUp(e.withNewPosition(dragSourcePos));
			dragSourceNote = -1;
			return;
		}

		dragSourceNote = -1;
		dragPresetIdx = -1;
		hoverNote = -1;
		MidiKeyboardComponent::mouseUp(e);
	}

private:
	static constexpr float PIANO_KEY_WIDTH = 32.0f;
	static constexpr float PIANO_SCROLL_WIDTH = 32.0f;
	static constexpr int PIANO_LOWEST_KEY = 48;

	static bool isPresetIndex(int index) {
		return index >= 0 && index < NUM_PRESETS;
	}

	void drawPad(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area, bool isDown) {
		const int presetIdx = midiNoteNumber - DRUM_BASE_NOTE;
		const auto presetName = isPresetIndex(presetIdx) ? getNonInitPresetName(presetIdx) : juce::String();
		const bool hasPreset = presetName.isNotEmpty();

		g.setColour(juce::Colour(24, 26, 29));
		g.fillRect(area);

		const juce::Rectangle<float> inKey(area.getX() + 3, area.getY() + 3, area.getWidth() - 5, area.getHeight() - 5);

		const auto topColour = hasPreset ? juce::Colour(255, 61, 61) : juce::Colour(147, 145, 150);
		const auto bottomColour = hasPreset ? juce::Colour(200, 48, 48) : juce::Colour(73, 73, 73);
		const auto borderColour = hasPreset ? juce::Colour(25, 26, 28) : juce::Colour(25, 26, 28);

		g.setGradientFill(juce::ColourGradient(topColour, 0.0f, 0.0f, bottomColour, 0.0f, (float)getHeight(), false));
		g.fillRoundedRectangle(inKey, 4.0f);

		g.setColour(borderColour);
		g.drawRoundedRectangle(inKey, 4.0f, 3.0f);

		if (isDown || (isDragging && hoverNote == midiNoteNumber)) {
			g.setColour(juce::Colours::white.withAlpha(0.2f));
			g.fillRoundedRectangle(inKey, 4.0f);
		}

		g.setColour(hasPreset ? juce::Colours::black : juce::Colour(201, 213, 219));

		if (hasPreset) {
			auto presetTextArea = area.withTrimmedTop(area.getHeight() * 0.1f).withHeight(area.getHeight() * 0.65f).reduced(8.0f, 0.0f);
			g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
			g.drawText(presetName, presetTextArea, juce::Justification::centredLeft, true);
		}

		auto noteTextArea = area.withTrimmedTop(area.getHeight() * 0.75f).reduced(8.0f, 1.0f);
		g.setFont(juce::FontOptions(12.0f));
		g.drawText(juce::MidiMessage::getMidiNoteName(midiNoteNumber, true, true, getOctaveForMiddleC()),
			noteTextArea, juce::Justification::centredLeft, true);
	}

	int getNoteForPosition(juce::Point<float> pos) {
		return getNoteAndVelocityAtPosition(pos).note;
	}

	juce::String getNonInitPresetName(int presetIdx) const {
		const auto presets = processor.getPresetManager().getAllPresetsInPack();

		if (presetIdx >= 0 && presetIdx < presets.size()) {
			auto name = presets[presetIdx].trim();
			if (!name.startsWithIgnoreCase("Init") && name.isNotEmpty())
				return name;
		}

		return {};
	}

	PluginProcessor& processor;
	bool isDrumMode = false;
	bool isDragging = false;
	int dragSourceNote = -1;
	int dragPresetIdx = -1;
	int hoverNote = -1;
	juce::Point<float> dragSourcePos;
};

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

	DrumKeyboardComponent keyboardComponent;

	PageButton OscButton;
	PageButton EffectButton;
	PageButton ArpButton;
	PageButton CycleButton;

	ScopeComponent<float> scopeComponent;
	ChipOscillatorComponent chipOscComponent;
	SweepParametersComponent sweepParamsComponent;
	VibratoParametersComponent vibratoParamsComponent;
	VoicingParametersComponent voicingParamsComponent;
	OptionsParametersComponent optionsParamsComponent;

	WaveformMemoryParameters* _waveformMemoryParamsPtr;
	WaveformMemoryParametersComponent waveformMemoryParamsComponent;
	MidiEchoParametersComponent midiEchoParamsComponent;
	FilterParametersComponent filterParamsComponent;

	LookAndFeel* customLookAndFeel;

	WavePatternsComponent wavePatternsComponent;

	ArpSequencerComponent arpSequencer;

	GUI::PresetPanel presetPanel;
	std::unique_ptr<juce::ButtonParameterAttachment> drumModeAttachment;

	juce::Image logo;
	juce::Image bg;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditorGUI)
};
