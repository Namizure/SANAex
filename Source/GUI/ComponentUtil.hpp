#pragma once
#include "JuceHeader.h"

struct Trail {
	Trail(const MouseInputSource& ms) : source(ms) {}

	void pushPoint(Point<float> newPoint, ModifierKeys newMods,
		float pressure) {
		currentPosition = newPoint;
		modifierKeys = newMods;

		if (lastPoint.getDistanceFrom(newPoint) > 5.0f) {
			if (lastPoint != Point<float>()) {
				Path newSegment;
				newSegment.startNewSubPath(lastPoint);
				newSegment.lineTo(newPoint);

				auto diameter =
					20.0f * (pressure > 0 && pressure < 1.0f ? pressure : 1.0f);

				PathStrokeType(diameter, PathStrokeType::curved,
					PathStrokeType::rounded)
					.createStrokedPath(newSegment, newSegment);
				path.addPath(newSegment);
			}

			lastPoint = newPoint;
		}
	}

	MouseInputSource source;
	Path path;
	Colour colour{ Colours::rebeccapurple.withAlpha(0.6f) };
	Point<float> lastPoint, currentPosition;
	ModifierKeys modifierKeys;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Trail)
};

class TextSlider : public Component {
public:
	int LOCAL_MARGIN = 2;
	int LABEL_WIDTH = 60;

	Slider slider;
	Label label;

	TextSlider(std::string labelName, std::string unit, float value, float start,
		float end, Slider::Listener* listener, float degree = 0.1f,
		float pivot = NULL)
		: slider(Slider::SliderStyle::LinearHorizontal,
			Slider::TextEntryBoxPosition::TextBoxLeft) {
		slider.setRange(start, end, degree);
		slider.setValue(value, dontSendNotification);
		slider.setTextValueSuffix(std::string(" ") + unit);
		slider.addListener(listener);

		if (pivot != NULL) {
			slider.setSkewFactorFromMidPoint(pivot);
		}

		label.setText(labelName, dontSendNotification);
		label.setJustificationType(Justification::centred);
		label.setEditable(false, false, false);

		addAndMakeVisible(slider);
		addAndMakeVisible(label);
	};

	TextSlider(std::string labelName, std::string unit,
		AudioParameterFloat* param, Slider::Listener* listener,
		float degree = 0.1f, float pivot = NULL)
		: TextSlider(labelName, unit, param->get(), param->range.start,
			param->range.end, listener, degree, pivot) {
	};
	TextSlider(std::string labelName, std::string unit, AudioParameterInt* param,
		Slider::Listener* listener)
		: TextSlider(labelName, unit, param->get(), param->getRange().getStart(),
			param->getRange().getEnd(), listener, 1.0f) {
	};

	virtual void setAlpha(float alpha) {
		slider.setAlpha(alpha);
		label.setAlpha(alpha);
	};

	virtual void resized() override {
		Rectangle<int> bounds = getLocalBounds();
		label.setBounds(bounds.removeFromLeft(LABEL_WIDTH).reduced(LOCAL_MARGIN));
		slider.setBounds(bounds.reduced(LOCAL_MARGIN));
	};

	virtual void setValue(float val) {
		slider.setValue(val, dontSendNotification);
	};

	virtual float getValue() { return slider.getValue(); };

	virtual void addListener(Slider::Listener* listener) {
		slider.addListener(listener);
	};

private:
	TextSlider();
};

class TextSliderIncDec : public TextSlider {
public:
	TextSliderIncDec(std::string labelName, std::string unit, int value,
		int start, int end, Slider::Listener* listener)
		: TextSlider(labelName, unit, value, start, end, listener, 1.0f) {
		slider.setSliderStyle(Slider::SliderStyle::IncDecButtons);
	};

	TextSliderIncDec(std::string labelName, std::string unit,
		AudioParameterInt* param, Slider::Listener* listener)
		: TextSliderIncDec(labelName, unit, param->get(),
			param->getRange().getStart(),
			param->getRange().getEnd(), listener) {
	};

private:
	TextSliderIncDec();
};

class TextSelector : public Component {
public:
	int LOCAL_MARGIN = 2;
	int LABEL_WIDTH = 60;

	ComboBox selector;
	Label label;

	TextSelector(std::string labelName, AudioParameterChoice* paramList, ComboBox::Listener* listener)
		: selector(labelName) {
		selector.addItemList(paramList->getAllValueStrings(), 1);
		selector.setSelectedItemIndex(paramList->getIndex(), dontSendNotification);
		selector.setJustificationType(Justification::centred);
		selector.addListener(listener);
		addAndMakeVisible(selector);

		label.setText(labelName, dontSendNotification);
		label.setJustificationType(Justification::centred);
		label.setEditable(false, false, false);
		addAndMakeVisible(label);
	};


	TextSelector(std::string labelName, const StringArray& items, ComboBox::Listener* listener)
		: selector(labelName) {
		selector.addItemList(items, 1);
		selector.setSelectedItemIndex(0, dontSendNotification);
		selector.setJustificationType(Justification::centred);
		selector.addListener(listener);
		addAndMakeVisible(selector);

		label.setText(labelName, dontSendNotification);
		label.setJustificationType(Justification::centred);
		label.setEditable(false, false, false);
		addAndMakeVisible(label);
	};

	virtual void resized() override {
		Rectangle<int> bounds = getLocalBounds();
		label.setBounds(bounds.removeFromLeft(LABEL_WIDTH).reduced(LOCAL_MARGIN));
		selector.setBounds(bounds.reduced(LOCAL_MARGIN * 2));
	};

	virtual void addListener(ComboBox::Listener* listener) {
		selector.addListener(listener);
	};

	virtual float getSelectedItemIndex() {
		return selector.getSelectedItemIndex();
	};

	virtual void setSelectedItemIndex(int index) {
		selector.setSelectedItemIndex(index, dontSendNotification);
	}

	void removeLabel() {
		this->LABEL_WIDTH = 0;
		this->LOCAL_MARGIN = 0;
	}

private:
	TextSelector();
};

class SwitchButton : public Component {
public:
	const std::int32_t LOCAL_MARGIN = 2;
	const std::int32_t LABEL_WIDTH = 60;

	ToggleButton button;

	SwitchButton(std::string label, AudioParameterBool* param, ToggleButton::Listener* listener) {
		button.setButtonText(label);
		button.setToggleState(param->get(), dontSendNotification);
		button.addListener(listener);
		addAndMakeVisible(button);
	};

	virtual void resized() override {
		Rectangle<int> bounds = getLocalBounds();
		bounds.removeFromLeft(LABEL_WIDTH * 0.2);
		button.setBounds(bounds.reduced(LOCAL_MARGIN));
	};

	virtual void setToggleState(bool flag) {
		button.setToggleState(flag, dontSendNotification);
	};

	virtual bool getToggleState() { return button.getToggleState(); }

	virtual void addListener(Button::Listener* listener) {
		button.addListener(listener);
	};

private:
	SwitchButton();
};

class PageButton : public Component {
public:
	const int LOCAL_MARGIN = 2;
	const int LABEL_WIDTH = 20;

	TextButton button;

	PageButton(std::string labelName, Button::Listener* listener)
		: button(labelName) {
		button.addListener(listener);
		button.setColour(TextButton::ColourIds::buttonColourId,
			Colour(33, 39, 43));
		button.setColour(TextButton::ColourIds::buttonOnColourId,
			Colour(253, 167, 63));
		button.setColour(TextButton::ColourIds::textColourOffId, Colours::white);
		button.setColour(TextButton::ColourIds::textColourOnId, Colours::black);
		button.setToggleState(true, NotificationType::dontSendNotification);

		addAndMakeVisible(button);
	};
	virtual void resized() override {
		Rectangle<int> bounds = getLocalBounds();
		button.setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), 30);
	};

	virtual void setToggleState(bool flag) {
		button.setToggleState(flag, NotificationType::dontSendNotification);
	};

private:
	PageButton();
};


// ===================


class WaveSampleSliders : public Component, private juce::Timer {
public:

	WaveSampleSliders(WaveformMemoryParameters* waveformMemoryParams,
		ChipOscillatorParameters* chipOscParams)
		: _waveformMemoryParamsPtr(waveformMemoryParams),
		_chipOscParamsPtr(chipOscParams),
		_current(0),
		_sampleSliders{} {
		for (auto i = 0; i < WAVESAMPLE_LENGTH; ++i) {
			_sampleSliders[i].setRange(0, 15, 1.0);
			_sampleSliders[i].setValue(7, dontSendNotification);
		}
		startTimerHz(10);
	}

	void waveformindex(int index) {
		if (index < 0 || index >= _waveformMemoryParamsPtr->getNumWaveforms()) {
			return;
		}
		_current = index;
		//DBG("_current: " + String(_current));
		//waveRangeSlider.waveformindex(index);
		repaint();
	}


	virtual void paint(Graphics& g) override {
		// update slider Params
		for (auto* trail : _trails) {
			auto compWidth = getWidth();
			auto compHeight = getHeight();

			std::int32_t index = (std::int32_t)(trail->currentPosition.x *
				(float)WAVESAMPLE_LENGTH / compWidth);
			index = std::min(index, 31);
			index = std::max(index, 0);
			float point = trail->currentPosition.y;
			std::int32_t value = 15 - (std::int32_t)(point * 16.0 / compHeight);
			_sampleSliders[index].setValue(value, dontSendNotification);
			updateValue(index);
		}
		// repaint Sliders
		{
			Rectangle<int> bounds = getLocalBounds();
			float columnSize = (float)WAVESAMPLE_LENGTH;
			float rowSize = (float)16;
			float compWidth = getWidth();



			// grid 1
			for (auto i = 0; i <= 16; ++i) {
				float p_y = getHeight() * (0.25f / 4) * i;
				Line<float> line(0.0f, p_y, (float)getWidth(), p_y);
				g.setColour(Colour(36, 43, 48));
				g.drawLine(line, 1.0f);
			}
			for (auto i = 0; i <= 32; ++i) {
				float p_x = getWidth() * (0.125f / 4) * i;
				Line<float> line(p_x, 0.0f, p_x, (float)getHeight());
				g.setColour(Colour(36, 43, 48));
				g.drawLine(line, 1.0f);
			}

			// grid 2
			for (auto i = 0; i <= 4; ++i) {
				float p_y = getHeight() * (0.25f) * i;
				Line<float> line(0.0f, p_y, (float)getWidth(), p_y);
				g.setColour(Colour(26, 33, 38));
				g.drawLine(line, 1.5f);





			}
			for (auto i = 0; i <= 8; ++i) {
				float p_x = getWidth() * (0.125f) * i;
				Line<float> line(p_x, 0.0f, p_x, (float)getHeight());
				g.setColour(Colour(26, 33, 38));
				g.drawLine(line, 1.5f);


			}

			/*if (_current) {
				String labelText = "waveform: " + to_string(_current);
				g.setColour(Colours::white);
				g.setFont(12.0f);
				int labelWidth = 230;
				int labelHeight = 12;
				Rectangle<int> textBounds(70 * 6, (int)getHeight() * (0.25f / 4) * 2 - (labelHeight + 100 / 2), labelWidth, labelHeight);

				g.drawText(labelText, textBounds, Justification::centredLeft, false);
			}*/

			// Draw Slider
			for (auto i = 0; i < WAVESAMPLE_LENGTH; ++i) {
				auto barHeight = getHeight() * (_sampleSliders[i].getValue() + 1.0f) / rowSize;
				auto barWidth = compWidth / (float)WAVESAMPLE_LENGTH;
				Rectangle<float> area2 = Rectangle<float>(
					i * barWidth, getHeight() - barHeight,
					barWidth, barHeight);
				float saturateRate = (i + WAVESAMPLE_LENGTH / 2.f) / (WAVESAMPLE_LENGTH + WAVESAMPLE_LENGTH / 2.f);

				if (*_chipOscParamsPtr->OscWaveType >= 16) {
					g.setColour(Colour(253, 167, 63));
				}
				else {
					g.setColour(Colour(164, 176, 181));
				}
				g.fillRect(area2.reduced(0.5f));
			}

		}
	};



	void updateValue() {
		AudioParameterInt** currentArray = getArrayForCurrentWaveform();
		for (auto i = 0; i < WAVESAMPLE_LENGTH; ++i) {
			*currentArray[i] = (std::int32_t)_sampleSliders[i].getValue();
		}
	};

	void updateValue(std::int32_t index) {
		AudioParameterInt** currentArray = getArrayForCurrentWaveform();
		*currentArray[index] = (std::int32_t)_sampleSliders[index].getValue();
	};

private:
	AudioParameterInt** getArrayForCurrentWaveform() {
		return _waveformMemoryParamsPtr->getWaveformArray(_current);
	}

	virtual void timerCallback() override {
		AudioParameterInt** currentArray = getArrayForCurrentWaveform();
		for (auto i = 0; i < WAVESAMPLE_LENGTH; ++i) {
			_sampleSliders[i].setValue(currentArray[i]->get(), dontSendNotification);
		}
		repaint();
	};

	virtual void mouseDrag(const MouseEvent& e) override {
		if (e.mods.isLeftButtonDown())
		{
			auto* t = getTrail(e.source);
			if (t == nullptr) {
				t = new Trail(e.source);
				t->path.startNewSubPath(e.position);
				_trails.add(t);
			}
			else {
				t->pushPoint(e.position, e.mods, e.pressure);
			}
			repaint();
		}
	};

	virtual void mouseDown(const MouseEvent& e) override {
		if (e.mods.isPopupMenu()) {
			_isRightClickActive = true;
			repaint();
			return;
		}

		_isRightClickActive = false;
		if (e.mods.isLeftButtonDown()) {
			auto* t = getTrail(e.source);
			if (t == nullptr) {
				t = new Trail(e.source);
				t->path.startNewSubPath(e.position);
				_trails.add(t);
			}
			t->pushPoint(e.position, e.mods, e.pressure);
			repaint();
		}
	};

	virtual void mouseUp(const MouseEvent& e) override {
		if (_isRightClickActive) {
			_isRightClickActive = false;
		}
		else {
			auto* t = getTrail(e.source);
			if (t != nullptr) {
				_trails.removeObject(t);
			}
		}
		repaint();
	};

	Trail* getTrail(const MouseInputSource& source) {
		for (auto* trail : _trails) {
			if (trail->source == source) return trail;
		}
		return nullptr;
	};

	Slider _sampleSliders[32];
	WaveformMemoryParameters* _waveformMemoryParamsPtr;
	ChipOscillatorParameters* _chipOscParamsPtr;
	int _current;
	bool _isRightClickActive = false;
	OwnedArray<Trail> _trails;
};

// ===================

class PatternSliders : public Component, private juce::Timer {
public:
	PatternSliders(WavePatternParameters* wavePatternParameters)
		: _wavePatternParameters(wavePatternParameters),
		_sampleSliders{} {
		for (auto i = 0; i < sampleNum; ++i) {
			_sampleSliders[i].setRange(0, valueMax - 1, 1.0);
			_sampleSliders[i].setValue(_wavePatternParameters->WavePatternArray[i]->get(), dontSendNotification);
		}
		startTimerHz(10);
	}
	virtual void paint(Graphics& g) override {
		for (auto* trail : _trails) {
			auto compWidth = getWidth();
			auto compHeight = getHeight();

			std::int32_t index = (std::int32_t)(trail->currentPosition.x *
				(float)sampleNum / compWidth);
			index = std::min(index, sampleNum - 1);
			index = std::max(index, 0);
			float point = trail->currentPosition.y;
			std::int32_t value = (valueMax - 1) - (std::int32_t)(point * valueMax / compHeight);
			_sampleSliders[index].setValue(value, dontSendNotification);
			updateValue(index);
		}
		// repaint Sliders
		{
			Rectangle<int> bounds = getLocalBounds();
			float columnSize = (float)sampleNum;
			float rowSize = valueMax;
			float compWidth = getWidth();
			float stepWidth = compWidth / (float)sampleNum;

			int endStepIndex = -1;
			for (int step = 0; step < sampleNum; ++step) {
				if (_wavePatternParameters->isEndStep[step]) {
					endStepIndex = step;
					break;
				}
			}

			if (endStepIndex >= 0 && endStepIndex < sampleNum - 1) {
				float darkStartX = (endStepIndex + 1) * stepWidth;
				g.setColour(Colour(10, 13, 15).withAlpha(0.65f));
				g.fillRect(Rectangle<float>(
					darkStartX,
					0.0f,
					(float)getWidth() - darkStartX,
					(float)getHeight()));
			}

			// draw slider
			for (auto i = 0; i < sampleNum; ++i) {
				auto barHeight = getHeight() * (_sampleSliders[i].getValue() + 1.0f) / rowSize;
				auto barWidth = compWidth / (float)sampleNum;
				Rectangle<float> area2 = Rectangle<float>(
					i * barWidth,
					getHeight() - barHeight,
					barWidth,
					getHeight() / float(valueMax));
				float saturateRate = (i + sampleNum / 2.f) / (sampleNum + sampleNum / 2.f);

				Colour barColour(253, 167, 63);
				if (i == endStepIndex) {
					barColour = Colour(220, 50, 50);
				}
				else if (endStepIndex >= 0 && i > endStepIndex) {
					barColour = Colour(202, 133, 50);
				}

				g.setColour(barColour);
				g.fillRect(area2.reduced(0.5f));
			}

			// draw grid
			for (auto i = 0; i <= valueMax; ++i) {
				float p_y = getHeight() / float(valueMax) * i;
				Line<float> line(0.0f, p_y, (float)getWidth(), p_y);
				g.setColour(Colour(26, 33, 38));
				g.drawLine(line, 1.5f);
			}

			for (auto i = 0; i <= sampleNum; ++i) {
				float p_x = getWidth() / float(sampleNum) * i;
				Line<float> line(p_x, 0.0f, p_x, (float)getHeight());
				g.setColour(Colour(26, 33, 38));
				g.drawLine(line, 1.5f);
			}
		}
	};


	void updateValue() {
		for (auto i = 0; i < sampleNum; ++i) {
			int newVal = (std::int32_t)_sampleSliders[i].getValue();
			if (_wavePatternParameters->WavePatternArray[i]->get() != newVal) {
				_wavePatternParameters->WavePatternArray[i]->beginChangeGesture();
				*_wavePatternParameters->WavePatternArray[i] = newVal;
				_wavePatternParameters->WavePatternArray[i]->endChangeGesture();
			}
		}
	};

	void updateValue(std::int32_t index) {
		int newVal = (std::int32_t)_sampleSliders[index].getValue();
		if (_wavePatternParameters->WavePatternArray[index]->get() != newVal) {
			_wavePatternParameters->WavePatternArray[index]->beginChangeGesture();
			*_wavePatternParameters->WavePatternArray[index] = newVal;
			_wavePatternParameters->WavePatternArray[index]->endChangeGesture();
		}
	};

private:
	virtual void timerCallback() override {
		for (auto i = 0; i < sampleNum; ++i) {
			_sampleSliders[i].setValue(_wavePatternParameters->WavePatternArray[i]->get(), dontSendNotification);
		}
		repaint();
	};

	virtual void mouseDrag(const MouseEvent& e) override {
		if (e.mods.isPopupMenu()) return;

		auto* t = getTrail(e.source);
		if (t == nullptr) {
			t = new Trail(e.source);
			t->path.startNewSubPath(e.position);
			_trails.add(t);
		}
		else {
			t->pushPoint(e.position, e.mods, e.pressure);
		}
		repaint();
	};


	virtual void mouseDown(const MouseEvent& e) override {
		if (e.mods.isPopupMenu()) {
			auto compWidth = getWidth();
			std::int32_t index = (std::int32_t)(e.position.x * (float)sampleNum / compWidth);
			index = std::min(index, sampleNum - 1);
			index = std::max(index, 0);

			const bool wasEndStep = _wavePatternParameters->isEndStep[index];

			for (int i = 0; i < sampleNum; i++) {
				_wavePatternParameters->isEndStep[i] = false;
			}

			if (!wasEndStep) {
				_wavePatternParameters->isEndStep[index] = true;
			}

			repaint();
			return;
		}
		else {
			auto* t = getTrail(e.source);
			if (t == nullptr) {
				t = new Trail(e.source);
				t->path.startNewSubPath(e.position);
				_trails.add(t);
			}
			t->pushPoint(e.position, e.mods, e.pressure);
			repaint();
		}
	}

	virtual void mouseUp(const MouseEvent& e) override {
		auto* t = getTrail(e.source);
		if (t != nullptr) {
			_trails.removeObject(t);
		}
		repaint();
	};

	Trail* getTrail(const MouseInputSource& source) {
		for (auto* trail : _trails) {
			if (trail->source == source) return trail;
		}

		return nullptr;
	};

	WavePatternParameters* _wavePatternParameters;

	const int valueMax = 16;
	const int sampleNum = 16;
	Slider _sampleSliders[16];
	bool* _isEndStep[16] = { false };
	int endStepIndex = -1;

	OwnedArray<Trail> _trails;

};

// ===================

class ArpSliders : public Component, private juce::Timer {
public:
	ArpSliders(ArpParameters* arpParameters)
		: _arpParameters(arpParameters),
		_sampleSliders{} {
		for (auto i = 0; i < sampleNum; ++i) {
			_sampleSliders[i].setRange(-24, 24, 1.0);
			_sampleSliders[i].setValue(_arpParameters->ArpPatternArray[i]->get(), dontSendNotification);
		}
		startTimerHz(10);
	}
	virtual void paint(Graphics& g) override {
		for (auto* trail : _trails) {
			auto compWidth = getWidth();
			auto compHeight = getHeight();

			std::int32_t index = (std::int32_t)(trail->currentPosition.x *
				(float)sampleNum / compWidth);
			index = std::min(index, sampleNum - 1);
			index = std::max(index, 0);
			float point = trail->currentPosition.y;
			std::int32_t value = 24 - (std::int32_t)((point / (float)compHeight) * 48.0f);
			value = std::max(-24, std::min(24, value));
			_sampleSliders[index].setValue(value, dontSendNotification);
			updateValue(index);
		}
		// repaint Sliders
		{
			Rectangle<int> bounds = getLocalBounds();
			float compWidth = getWidth();
			float stepWidth = compWidth / (float)sampleNum;

			int endStepIndex = -1;
			for (int step = 0; step < sampleNum; ++step) {
				if (_arpParameters->isEndStep[step]) {
					endStepIndex = step;
					break;
				}
			}

			if (endStepIndex >= 0 && endStepIndex < sampleNum - 1) {
				float darkStartX = std::round((compWidth / (float)sampleNum) * (endStepIndex + 1));
				g.setColour(Colour(10, 13, 15).withAlpha(0.65f));
				g.fillRect(Rectangle<float>(
					darkStartX,
					0.0f,
					compWidth - darkStartX,
					(float)getHeight()));
			}
			// draw slider
			for (auto i = 0; i < sampleNum; ++i) {
				auto val = _sampleSliders[i].getValue();
				float centerY = getHeight() / 2.0f;
				float barY = centerY;
				float barHeight = val * (getHeight() / 48.0f);
				if (val > 0) {
					barY -= barHeight;
				}
				else {
					barHeight = -barHeight;
				}

				auto barWidth = compWidth / (float)sampleNum;
				Rectangle<float> area2 = Rectangle<float>(
					i * barWidth,
					barY,
					barWidth,
					std::max(2.0f, barHeight));

				g.setColour(_arpParameters->isEndStep[i] ? Colour(220, 50, 50)
					: Colour(253, 167, 63));
				g.fillRect(area2.reduced(0.5f));
				Colour barColour(253, 167, 63);
				if (i == endStepIndex) {
					barColour = Colour(220, 50, 50);
				}
				else if (endStepIndex >= 0 && i > endStepIndex) {
					barColour = Colour(202, 133, 50);
				}
				g.setColour(barColour);
				g.fillRect(area2.reduced(0.5f));
			}
			// draw grid
			for (auto i = 1; i < 48; ++i) {
				float p_y = getHeight() / 48.0f * i;
				Line<float> line(0.0f, p_y, (float)getWidth(), p_y);
				g.setColour(i == 24 ? Colour(164, 176, 181) : Colour(26, 33, 38));
				g.drawLine(line, i == 24 ? 2.0f : 1.5f);

				int pitchValue = 24 - i;
				if (pitchValue == 12 || pitchValue == -12) {
					String labelText = (pitchValue > 0) ? "+" + String(pitchValue) : String(pitchValue);
					g.setColour(pitchValue == 0 ? Colours::white : Colours::white.withAlpha(0.6f));
					g.setFont(12.0f);
					int labelWidth = 30;
					int labelHeight = 12;
					Rectangle<int> textBounds(5, (int)p_y - (labelHeight / 2), labelWidth, labelHeight);

					g.drawText(labelText, textBounds, Justification::centredLeft, false);
				}
			}

			for (auto i = 1; i < sampleNum; ++i) {
				float p_x = std::round((compWidth / (float)sampleNum) * i);

				Line<float> line(p_x, 0.0f, p_x, (float)getHeight());
				g.setColour(Colour(26, 33, 38));
				g.drawLine(line, 1.5f);
			}
		}
	};

	void updateValue() {
		for (auto i = 0; i < sampleNum; ++i) {
			int newVal = (std::int32_t)_sampleSliders[i].getValue();
			if (_arpParameters->ArpPatternArray[i]->get() != newVal) {
				_arpParameters->ArpPatternArray[i]->beginChangeGesture();
				*_arpParameters->ArpPatternArray[i] = newVal;
				_arpParameters->ArpPatternArray[i]->endChangeGesture();
			}
		}
	};

	void updateValue(std::int32_t index) {
		int newVal = (std::int32_t)_sampleSliders[index].getValue();
		if (_arpParameters->ArpPatternArray[index]->get() != newVal) {
			_arpParameters->ArpPatternArray[index]->beginChangeGesture();
			*_arpParameters->ArpPatternArray[index] = newVal;
			_arpParameters->ArpPatternArray[index]->endChangeGesture();
		}
	};

private:
	virtual void timerCallback() override {
		for (auto i = 0; i < sampleNum; ++i) {
			_sampleSliders[i].setValue(_arpParameters->ArpPatternArray[i]->get(), dontSendNotification);
		}
		repaint();
	};

	virtual void mouseDrag(const MouseEvent& e) override {
		if (e.mods.isPopupMenu()) return;

		auto* t = getTrail(e.source);
		if (t == nullptr) {
			t = new Trail(e.source);
			t->path.startNewSubPath(e.position);
			_trails.add(t);
		}
		else {
			t->pushPoint(e.position, e.mods, e.pressure);
		}
		repaint();
	};

	virtual void mouseDown(const MouseEvent& e) override {
		if (e.mods.isPopupMenu()) {
			auto compWidth = getWidth();
			std::int32_t index = (std::int32_t)(e.position.x * (float)sampleNum / compWidth);
			index = std::min(index, sampleNum - 1);
			index = std::max(index, 0);

			const bool wasEndStep = _arpParameters->isEndStep[index];

			for (int i = 0; i < sampleNum; i++) {
				_arpParameters->isEndStep[i] = false;
			}

			if (!wasEndStep) {
				_arpParameters->isEndStep[index] = true;
			}

			//_isEndStep[index] = !_isEndStep[index];
			repaint();
			return;
		}

		auto* t = getTrail(e.source);
		if (t == nullptr) {
			t = new Trail(e.source);
			t->path.startNewSubPath(e.position);
			_trails.add(t);
		}
		t->pushPoint(e.position, e.mods, e.pressure);
		repaint();
	};

	virtual void mouseUp(const MouseEvent& e) override {
		auto* t = getTrail(e.source);
		if (t != nullptr) {
			_trails.removeObject(t);
		}
		repaint();
	};

	Trail* getTrail(const MouseInputSource& source) {
		for (auto* trail : _trails) {
			if (trail->source == source) return trail;
		}
		return nullptr;
	};

	ArpParameters* _arpParameters;

	const int sampleNum = 16;
	Slider _sampleSliders[16];
	bool* _isEndStep[16] = { false };
	int endStepIndex = -1;

	OwnedArray<Trail> _trails;
};
