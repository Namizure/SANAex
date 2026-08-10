#pragma once
#include "JuceHeader.h"
#include "ProjectFonts.h"

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


class CustomSliderLookAndFeel : public juce::LookAndFeel_V4 {
public:
	CustomSliderLookAndFeel() {
		setColour(juce::Slider::backgroundColourId, juce::Colour(255, 255, 255));
		setColour(juce::Slider::thumbColourId, juce::Colours::white);
		setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
		setColour(juce::Slider::trackColourId, juce::Colour(200, 48, 48));
	}

	void drawLabel(juce::Graphics& g, juce::Label& label) override {
		auto bounds = label.getLocalBounds().toFloat();
		// background
		g.setColour(Colour(19, 19, 19));
		g.fillRoundedRectangle(bounds, 10.f);
		// outline
		g.setColour(juce::Colours::white);
		g.drawRoundedRectangle(bounds.reduced(1.5f / 2.0f), 10.f, 1.f);
		//text
		g.setColour(juce::Colours::white);
		g.setFont(ProjectFonts::semiboldFont(18));
		g.setOpacity(label.isEnabled() ? 1.0f : 0.5f);

		g.drawFittedText(label.getText(),
			label.getLocalBounds().reduced(6, 2),
			label.getJustificationType(),
			juce::jmax(1, (int)((float)label.getHeight() / g.getCurrentFont().getHeight())),
			label.getMinimumHorizontalScale());
	}


	void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
		float sliderPos, float minSliderPos, float maxSliderPos,
		const juce::Slider::SliderStyle style, juce::Slider& slider) override {

		// slider
		float trackHeight = 6.0f;
		float trackRadius = trackHeight / 2.0f;
		float trackY = (float)y + ((float)height - trackHeight) / 2.0f;

		// white part
		g.setColour(slider.findColour(juce::Slider::backgroundColourId));
		g.fillRoundedRectangle((float)x, trackY, (float)width, trackHeight, trackRadius);

		// red part
		g.setColour(slider.findColour(juce::Slider::trackColourId));
		g.fillRoundedRectangle((float)x, trackY, sliderPos - (float)x, trackHeight, trackRadius);


		// thumb (circle in the middle)(
		float thumbWidth = 9.0f;
		float thumbHeight = 16.0f;

		juce::Rectangle<float> thumbRect(sliderPos - (thumbWidth / 2.0f),
			(float)y + ((float)height - thumbHeight) / 2.0f,
			thumbWidth / 1.15, thumbHeight);

		float cornerSize = 2.f;

		// around
		g.setColour(juce::Colours::white);
		g.fillRoundedRectangle(thumbRect, cornerSize);

		// middle
		g.setColour(juce::Colour(20, 20, 24));
		g.fillRoundedRectangle(thumbRect.reduced(1.25f), 0.f);


		// middle line
		g.setColour(juce::Colours::white);
		float lineWidth = 1.f;

		g.fillRoundedRectangle(thumbRect.getCentreX() - (lineWidth / 2.0f),
			thumbRect.getY() + 4.0f,
			lineWidth,
			thumbRect.getHeight() - 8.0f,
			0.5f);
	}
};






class TextSlider : public Component, public juce::Slider::Listener {
public:
	int LOCAL_MARGIN = 2;
	int LABEL_WIDTH = 60;

	Slider slider;
	Label label;

	CustomSliderLookAndFeel CustomSliderLookAndFeel;

	TextSlider(std::string labelName, std::string unit, float value, float start,
		float end, Slider::Listener* listener, float degree = 0.1f,
		float pivot = NULL)
		: slider(Slider::SliderStyle::LinearHorizontal,
			Slider::TextEntryBoxPosition::TextBoxLeft) {


		slider.setLookAndFeel(&CustomSliderLookAndFeel);



		slider.setRange(start, end, degree);
		slider.setValue(value, dontSendNotification);
		slider.setTextValueSuffix(std::string(" ") + unit);
		slider.addListener(listener);
		slider.addListener(this);

		if (pivot != NULL) {
			slider.setSkewFactorFromMidPoint(pivot);
		}

		label.setFont(ProjectFonts::headerFont(24));
		label.setColour(juce::Label::textColourId, juce::Colour(98, 100, 110));

		label.setText(labelName, dontSendNotification);
		label.setJustificationType(Justification::centred);
		label.setEditable(false, false, false);

		addAndMakeVisible(slider);
		addAndMakeVisible(label);
	};

	~TextSlider() override {
		slider.setLookAndFeel(nullptr);
	}

	void sliderDragStarted(juce::Slider* slider) override {
		label.setFont(ProjectFonts::boldFont(26));
	}

	void sliderDragEnded(juce::Slider* slider) override {
		label.setFont(ProjectFonts::headerFont(24));
	}

	void sliderValueChanged(juce::Slider* slider) override {}


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

	virtual void paint(Graphics& g) {

	}



	virtual void resized() override {
		Rectangle<int> bounds = getLocalBounds();
		label.setBounds(bounds.removeFromLeft(45));
		slider.setBounds(bounds);
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



class CustomSmallSliderLookAndFeel : public juce::LookAndFeel_V4 {
public:
	CustomSmallSliderLookAndFeel() {
		setColour(juce::Slider::backgroundColourId, juce::Colour(19, 19, 19));
		setColour(juce::Slider::trackColourId, juce::Colour(200, 48, 48));
	}

	void drawLabel(juce::Graphics& g, juce::Label& label) override {
		auto bounds = label.getLocalBounds().toFloat();
		g.setColour(label.findColour(juce::Label::backgroundColourId));
		g.fillRoundedRectangle(bounds, 3.f);

		g.setColour(label.findColour(juce::Label::outlineColourId));
		g.drawRoundedRectangle(bounds.reduced(1.5f / 2.0f), 3.f, 1.f);

		g.setColour(label.findColour(juce::Label::textColourId));
		g.setFont(ProjectFonts::semiboldFont(18));
		g.setOpacity(label.isEnabled() ? 1.0f : 0.5f);

		g.drawFittedText(label.getText(),
			label.getLocalBounds().reduced(6, 2),
			label.getJustificationType(),
			juce::jmax(1, (int)((float)label.getHeight() / g.getCurrentFont().getHeight())),
			label.getMinimumHorizontalScale());
	}

	void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
		float sliderPos, float minSliderPos, float maxSliderPos,
		const juce::Slider::SliderStyle style, juce::Slider& slider) override {

		if (style == juce::Slider::LinearBar) {
			juce::Rectangle<float> bounds((float)x, (float)y, (float)width, (float)height);
			float cornerSize = 3.f;

			g.setColour(slider.findColour(juce::Slider::backgroundColourId));
			g.fillRoundedRectangle(bounds, cornerSize);

			float padding = 2.0f;
			juce::Rectangle<float> innerBounds = bounds.reduced(padding);
			float innerCornerSize = 1.5f;

			if (sliderPos > innerBounds.getX()) {
				g.saveState();
				juce::Path clipPath;
				clipPath.addRoundedRectangle(innerBounds, innerCornerSize);
				g.reduceClipRegion(clipPath);

				g.setColour(slider.findColour(juce::Slider::trackColourId));
				g.fillRect(innerBounds.withWidth(sliderPos - innerBounds.getX()));

				g.restoreState();
			}
		}

	}
};


class TextSliderSmall : public Component {
public:
	int LOCAL_MARGIN = 2;
	int LABEL_WIDTH = 60;

	Slider slider;
	Label label;

	CustomSmallSliderLookAndFeel CustomSliderLookAndFeel;

	TextSliderSmall(std::string labelName, std::string unit, float value, float start,
		float end, Slider::Listener* listener, float degree = 0.1f,
		float pivot = NULL)
		: slider(Slider::SliderStyle::LinearBar, Slider::TextEntryBoxPosition::TextBoxLeft) {

		slider.setLookAndFeel(&CustomSliderLookAndFeel);
		slider.setRange(start, end, degree);
		slider.setValue(value, dontSendNotification);
		slider.setTextValueSuffix(std::string(" ") + unit);
		slider.addListener(listener);

		if (pivot != NULL) {
			slider.setSkewFactorFromMidPoint(pivot);
		}

		slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
		slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::white);
		slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);

		label.setFont(ProjectFonts::boldFont(20));
		label.setColour(juce::Label::textColourId, juce::Colour(32, 33, 40));
		label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
		label.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);

		label.setText(labelName, dontSendNotification);
		label.setJustificationType(Justification::centred);
		label.setEditable(false, false, false);

		addAndMakeVisible(slider);
		addAndMakeVisible(label);
	};

	~TextSliderSmall() override {
		slider.setLookAndFeel(nullptr);
	}

	TextSliderSmall(std::string labelName, std::string unit,
		AudioParameterFloat* param, Slider::Listener* listener,
		float degree = 0.1f, float pivot = NULL)
		: TextSliderSmall(labelName, unit, param->get(), param->range.start,
			param->range.end, listener, degree, pivot) {
	};
	TextSliderSmall(std::string labelName, std::string unit, AudioParameterInt* param,
		Slider::Listener* listener)
		: TextSliderSmall(labelName, unit, param->get(), param->getRange().getStart(),
			param->getRange().getEnd(), listener, 1.0f) {
	};

	virtual void setAlpha(float alpha) {
		slider.setAlpha(alpha);
		label.setAlpha(alpha);
	};

	virtual void resized() override {
		Rectangle<int> bounds = getLocalBounds();
		label.setBounds(bounds.removeFromLeft(60));
		slider.setBounds(bounds);
	};

	virtual void setValue(float val) {
		slider.setValue(val, dontSendNotification);
	};

	virtual float getValue() { return slider.getValue(); };

	virtual void addListener(Slider::Listener* listener) {
		slider.addListener(listener);
	};

private:
	TextSliderSmall();
};


class CustomTextSlider : public juce::LookAndFeel_V4 {
public:
	CustomTextSlider() {
		setColour(juce::Slider::textBoxBackgroundColourId, Colour(19, 19, 19));
		setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
		setColour(juce::TextButton::buttonColourId, juce::Colour(21, 22, 27));
		setColour(juce::TextButton::textColourOffId, Colour(98, 100, 110));
	}

	void drawLabel(juce::Graphics& g, juce::Label& label) override {
		auto bounds = label.getLocalBounds().toFloat();
		// background
		g.setColour(Colour(19, 19, 19));
		g.fillRoundedRectangle(bounds, 10.f);
		// outline
		g.setColour(juce::Colours::white);
		g.drawRoundedRectangle(bounds.reduced(1.5f / 2.0f), 10.f, 1.f);
		//text
		g.setColour(juce::Colours::white);
		g.setFont(ProjectFonts::semiboldFont(18));
		g.setOpacity(label.isEnabled() ? 1.0f : 0.5f);

		g.drawFittedText(label.getText(),
			label.getLocalBounds().reduced(1, 2),
			label.getJustificationType(),
			juce::jmax(1, (int)((float)label.getHeight() / g.getCurrentFont().getHeight())),
			label.getMinimumHorizontalScale());
	}

	juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override {
		if (slider.getSliderStyle() == juce::Slider::IncDecButtons) {
			juce::Slider::SliderLayout layout;
			auto bounds = slider.getLocalBounds();

			int textBoxWidth = 100;
			int buttonWidth = 75;
			int gap = 8;

			layout.textBoxBounds = bounds.removeFromLeft(textBoxWidth);

			bounds.removeFromLeft(gap);
			layout.sliderBounds = bounds.removeFromLeft(buttonWidth * 2);

			return layout;
		}
		return juce::LookAndFeel_V4::getSliderLayout(slider);
	}
};

class TextSliderIncDec : public TextSlider {
public:
	CustomTextSlider customlook;

	TextSliderIncDec(std::string labelName, std::string unit, int value,
		int start, int end, Slider::Listener* listener)
		: TextSlider(labelName, unit, value, start, end, listener, 1.0f) {

		juce::Font(ProjectFonts::semiboldFont(26.f));
		slider.setSliderStyle(Slider::SliderStyle::IncDecButtons);
		slider.setLookAndFeel(&customlook);
	};

	TextSliderIncDec(std::string labelName, std::string unit,
		AudioParameterInt* param, Slider::Listener* listener)
		: TextSliderIncDec(labelName, unit, param->get(),
			param->getRange().getStart(),
			param->getRange().getEnd(), listener) {

	};

	void resized() override {
		juce::Rectangle<int> bounds = getLocalBounds();
		label.setBounds(bounds.removeFromLeft(75));
		slider.setBounds(bounds);
	}


	~TextSliderIncDec() override {
		slider.setLookAndFeel(nullptr);
	}

private:
	TextSliderIncDec();
};


class CustomTextButton : public juce::LookAndFeel_V4 {
public:


	CustomTextButton() {
		setColour(juce::TextButton::buttonColourId, juce::Colour(147, 145, 150));
		setColour(juce::TextButton::textColourOffId, juce::Colour(32, 33, 40));

	}

	juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override {
		return ProjectFonts::boldFont(26.f);
	}

	void drawButtonBackground(juce::Graphics& g, juce::Button& button,
		const juce::Colour& backgroundColour,
		bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override
	{
		auto bounds = button.getLocalBounds().toFloat();

		auto baseColour = juce::Colour(147, 145, 150);
		if (shouldDrawButtonAsDown) {
			baseColour = baseColour.darker(0.1f);
		}
		else if (shouldDrawButtonAsHighlighted) {
			baseColour = baseColour.brighter(0.05f);
		}

		g.setColour(baseColour);
		g.fillRoundedRectangle(bounds, 2.0f);

		float strokeWidth = 1.5f;
		auto strokeBounds = bounds.reduced(strokeWidth * 0.5f);

		float x = strokeBounds.getX();
		float y = strokeBounds.getY();
		float width = strokeBounds.getWidth();
		float height = strokeBounds.getHeight();

		// bevel effect
		juce::Path highlightPath;
		highlightPath.startNewSubPath(x, y + height);
		highlightPath.lineTo(x, y);
		highlightPath.lineTo(x + width, y);


		juce::Path shadowPath;
		shadowPath.startNewSubPath(x + width, y);
		shadowPath.lineTo(x + width, y + height);
		shadowPath.lineTo(x, y + height);

		juce::Colour lightColour = juce::Colour(206, 203, 210);
		juce::Colour darkColour = juce::Colour(98, 97, 100);

		g.setColour(shouldDrawButtonAsDown ? darkColour : lightColour);
		g.strokePath(highlightPath, juce::PathStrokeType(strokeWidth));

		g.setColour(shouldDrawButtonAsDown ? lightColour : darkColour);
		g.strokePath(shadowPath, juce::PathStrokeType(strokeWidth));
	}


};

class TextButtonSmall : public juce::TextButton {
public:
	CustomTextButton TextButtonLnF;

	TextButtonSmall() : juce::TextButton() {
		setLookAndFeel(&TextButtonLnF);
		juce::Font(ProjectFonts::semiboldFont(26.f));
	}

	TextButtonSmall(const juce::String& labelName)
		: juce::TextButton(labelName) {
		setLookAndFeel(&TextButtonLnF);
		juce::Font(ProjectFonts::semiboldFont(26.f));

	}

	~TextButtonSmall() override {
		setLookAndFeel(nullptr);
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextButtonSmall)
};



class CustomComboBoxLookAndFeel : public juce::LookAndFeel_V4 {
public:
	CustomComboBoxLookAndFeel() {
		setColour(juce::ComboBox::backgroundColourId, juce::Colour(0.0f, 0.0f, 0.0f, 0.0f));
		setColour(juce::ComboBox::textColourId, juce::Colour(32, 33, 40));
		setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack.withAlpha(0.0f));
		setColour(juce::ComboBox::arrowColourId, juce::Colour(32, 33, 40));
	}

	juce::Font getComboBoxFont(juce::ComboBox& box) override {
		return juce::Font(ProjectFonts::semiboldFont(26.f));

	}

	void labelposition(juce::ComboBox& box, juce::Label& label) {
		label.setJustificationType(juce::Justification::centred);
	}

	void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
		int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override {
		juce::Rectangle<int> boxBounds(0, 0, width, height);


		g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
		g.fillRoundedRectangle(boxBounds.toFloat(), 0.0f);
		g.setColour(box.findColour(juce::ComboBox::outlineColourId));
		g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), 0.0f, 2.0f);

		juce::Path path;
		auto x = (float)buttonX + (float)buttonW * 0.3f - 12;
		auto y = (float)buttonY + (float)buttonH * 0.45f;
		auto w = (float)buttonW * 0.5f;
		auto h = (float)buttonH * 0.2f;

		path.startNewSubPath(x, y);
		path.lineTo(x + w / 2.0f, y + h);
		path.lineTo(x + w, y);
		path.closeSubPath();

		g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha((box.isEnabled() ? 1.0f : 0.2f)));
		g.fillPath(path);
		g.strokePath(path, juce::PathStrokeType(1.0f));
	}
};




class PatternComboBoxLookAndFeel : public juce::LookAndFeel_V4 {
public:
	PatternComboBoxLookAndFeel() {
		setColour(juce::ComboBox::backgroundColourId, juce::Colour(71, 71, 78));
		setColour(juce::ComboBox::textColourId, juce::Colour(170, 170, 175));
		setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack.withAlpha(0.0f));
	}

	juce::Font getComboBoxFont(juce::ComboBox& box) override {
		return juce::Font(ProjectFonts::headerFont(22.f));
	}

	void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override {
		label.setBounds(0, 0, box.getWidth(), box.getHeight());
		label.setFont(getComboBoxFont(box));
		label.setJustificationType(juce::Justification::centred);
	}

	void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override {
		juce::Rectangle<int> boxBounds(0, 0, width, height);
		g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
		float cornerRadius = 5.0f;
		g.fillRoundedRectangle(boxBounds.toFloat(), cornerRadius);
		g.setColour(box.findColour(juce::ComboBox::outlineColourId));
		g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerRadius, 1.0f);
	}
};


class TextSelector : public Component {
public:
	int LOCAL_MARGIN = 2;
	int LABEL_WIDTH = 60;

	ComboBox selector;
	Label label;

	CustomComboBoxLookAndFeel customComboLnF;
	PatternComboBoxLookAndFeel patternComboLnF;

	TextSelector(std::string labelName, AudioParameterChoice* paramList, ComboBox::Listener* listener)
		: selector(labelName) {

		selector.setLookAndFeel(&customComboLnF);

		selector.addItemList(paramList->getAllValueStrings(), 1);
		selector.setSelectedItemIndex(paramList->getIndex(), dontSendNotification);
		selector.setJustificationType(Justification::centred);
		selector.addListener(listener);
		addAndMakeVisible(selector);

		label.setFont(ProjectFonts::semiboldFont(32.f));
		label.setColour(juce::Label::textColourId, juce::Colour(32, 33, 40));

		label.setText(labelName, dontSendNotification);
		label.setJustificationType(Justification::centred);
		label.setEditable(false, false, false);
		addAndMakeVisible(label);
	};


	TextSelector(std::string labelName, const StringArray& items, ComboBox::Listener* listener)
		: selector(labelName) {

		selector.setLookAndFeel(&patternComboLnF);

		selector.addItemList(items, 1);
		selector.setSelectedItemIndex(0, dontSendNotification);
		selector.setJustificationType(Justification::centred);
		selector.addListener(listener);
		addAndMakeVisible(selector);

		label.setFont(ProjectFonts::boldFont(24.f));

		label.setText(labelName, dontSendNotification);
		label.setJustificationType(Justification::centred);
		label.setEditable(false, false, false);
		addAndMakeVisible(label);
	};

	~TextSelector() override {
		selector.setLookAndFeel(nullptr);
	}


	virtual void resized() override {
		Rectangle<int> bounds = getLocalBounds();
		label.setBounds(bounds.removeFromLeft(LABEL_WIDTH + 15).reduced(LOCAL_MARGIN).withY(-1));
		selector.setBounds(bounds.reduced(LOCAL_MARGIN * 2).withWidth(170).withHeight(35).withY(0));

	};


	virtual void paint(Graphics& g) {
		int x = selector.getX();
		int y = selector.getY();
		int height = selector.getHeight();
		int width = selector.getWidth();

		// highlight
		auto highlightCol = juce::Colour(177, 174, 180);
		auto shadowCol = juce::Colour(99, 97, 102);
		g.setColour(highlightCol);
		g.fillRect(x + 1, y + 4, 1, height - 6);
		g.fillRect(x + width - 1, y + 4, 1, height - 6);

		// outline
		g.setColour(Colour(15, 19, 21));
		g.fillRect(x, y + 2, 1, height - 4);
		g.fillRect(x + width - 2, y + 2, 1, height - 4);

		// shadow
		g.setColour(shadowCol);
		g.fillRect(x - 1, y + 2, 1, height - 4);
		g.fillRect(x + width - 3, y + 2, 1, height - 4);
	}

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

class TextSelectorSmall : public Component {
public:
	int LOCAL_MARGIN = 2;
	int LABEL_WIDTH = 60;

	ComboBox selector;
	Label label;

	CustomComboBoxLookAndFeel customComboLnF;
	PatternComboBoxLookAndFeel patternComboLnF;




	TextSelectorSmall(std::string labelName, AudioParameterChoice* paramList, ComboBox::Listener* listener)
		: selector(labelName) {

		selector.setLookAndFeel(&customComboLnF);

		selector.addItemList(paramList->getAllValueStrings(), 1);
		selector.setSelectedItemIndex(paramList->getIndex(), dontSendNotification);
		selector.setJustificationType(Justification::centred);
		selector.addListener(listener);
		addAndMakeVisible(selector);

		label.setFont(ProjectFonts::semiboldFont(32.f));
		label.setColour(juce::Label::textColourId, juce::Colour(32, 33, 40));

		label.setText(labelName, dontSendNotification);
		label.setJustificationType(Justification::centred);
		label.setEditable(false, false, false);
		addAndMakeVisible(label);
	};


	TextSelectorSmall(std::string labelName, const StringArray& items, ComboBox::Listener* listener)
		: selector(labelName) {

		selector.setLookAndFeel(&patternComboLnF);

		selector.addItemList(items, 1);
		selector.setSelectedItemIndex(0, dontSendNotification);
		selector.setJustificationType(Justification::centred);
		selector.addListener(listener);
		addAndMakeVisible(selector);

		label.setFont(ProjectFonts::headerFont(12.f));

		label.setText(labelName, dontSendNotification);
		label.setJustificationType(Justification::centred);
		label.setEditable(false, false, false);
		addAndMakeVisible(label);
	};

	~TextSelectorSmall() override {
		selector.setLookAndFeel(nullptr);
	}


	virtual void resized() override {
		Rectangle<int> bounds = getLocalBounds();

		if (LABEL_WIDTH > 0) {
			label.setBounds(bounds.removeFromLeft(LABEL_WIDTH + 120).reduced(LOCAL_MARGIN).withY(-1));
		}

		selector.setBounds(bounds.reduced(LOCAL_MARGIN * 4));
	};


	virtual void paint(Graphics& g) {
	}

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
	TextSelectorSmall();
};



class CustomSwitchButtonLookAndFeel : public juce::LookAndFeel_V4 {
public:
	CustomSwitchButtonLookAndFeel() {
		setColour(juce::ToggleButton::textColourId, juce::Colour(32, 33, 40));
		setColour(juce::ToggleButton::tickColourId, juce::Colour(200, 48, 48));
		setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(32, 33, 40));
	}

	void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override {
		auto bounds = button.getLocalBounds().toFloat();

		// circle congif
		float circleSize = 15.0f;
		float circleX = 8.0f;
		float circleY = (bounds.getHeight() - circleSize) / 2.0f;

		if (button.getToggleState()) {
			// background circle
			g.setColour(juce::Colour(32, 33, 40));
			g.fillEllipse(circleX, circleY, circleSize, circleSize);

			// ontop circle
			g.setColour(juce::Colour(200, 48, 48));
			g.fillEllipse(circleX + 2.0f, circleY + 2.0f, circleSize - 4.0f, circleSize - 4.0f);
		}
		else {
			// background circle
			g.setColour(juce::Colour(32, 33, 40));
			g.fillEllipse(circleX, circleY, circleSize, circleSize);
		}
		// fontt
		g.setColour(juce::Colour(32, 33, 40));
		g.setFont(ProjectFonts::boldFont(28));
		auto textArea = bounds.withTrimmedLeft(circleX + circleSize + 6.0f);
		g.drawText(button.getButtonText(), textArea, juce::Justification::centredLeft, true);

		// divider
		int x = 0;
		int y = 0;
		int height = (int)button.getHeight();
		int width = (int)button.getWidth();

		// highlight
		auto highlightCol = juce::Colour(177, 174, 180);
		auto shadowCol = juce::Colour(99, 97, 102);
		g.setColour(highlightCol);
		g.fillRect(x - 3, y, 1, height);
		g.fillRect(x + width - 1, y, 1, height);

		// outline
		g.setColour(Colour(15, 19, 21));
		g.fillRect(x - 3, y, 1, height);
		g.fillRect(x + width - 2, y, 1, height);

		// shadow
		g.setColour(shadowCol);
		g.fillRect(x - 3, y, 1, height);
		g.fillRect(x + width - 3, y, 1, height);
	}

};

class SwitchButton : public Component {
public:
	ToggleButton button;
	CustomSwitchButtonLookAndFeel switchbuttonLAF;

	SwitchButton(std::string label, AudioParameterBool* param, ToggleButton::Listener* listener)
	{
		button.setLookAndFeel(&switchbuttonLAF);
		button.setButtonText(label);

		if (param != nullptr)
			button.setToggleState(param->get(), dontSendNotification);

		button.addListener(listener);
		addAndMakeVisible(button);
	};

	~SwitchButton()
	{
		button.setLookAndFeel(nullptr);
	}

	void resized() override
	{
		button.setBounds(getLocalBounds());
	};

	void setToggleState(bool flag)
	{
		button.setToggleState(flag, dontSendNotification);
	};

	bool getToggleState()
	{
		return button.getToggleState();
	}

	void addListener(Button::Listener* listener)
	{
		button.addListener(listener);
	};


private:
	SwitchButton();
};







// special buttons for wavepatterns
class FilterButtonLook : public juce::LookAndFeel_V4 {
public:
	FilterButtonLook() {
		setColour(juce::ToggleButton::textColourId, juce::Colour(32, 33, 40));
		setColour(juce::ToggleButton::tickColourId, juce::Colour(200, 48, 48));
		setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(32, 33, 40));
	}

	void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override {
		auto bounds = button.getLocalBounds().toFloat();

		// rectangle congif
		float rectSize = 15.0f;
		float rectX = 8.0f;
		float rectY = (bounds.getHeight() - rectSize) / 2.0f;

		if (button.getToggleState())
		{
			g.setColour(juce::Colour(98, 100, 110));
			g.fillRect(rectX, rectY, rectSize, rectSize);

			g.setColour(juce::Colour(32, 33, 40));
			g.fillRect(rectX + 1.0f, rectY + 1.0f, rectSize - 2.0f, rectSize - 2.0f);

			g.setColour(juce::Colour(200, 48, 48));
			g.fillRect(rectX + 3.0f, rectY + 3.0f, rectSize - 6.0f, rectSize - 6.0f);
		}
		else {
			g.setColour(juce::Colour(98, 100, 110));
			g.fillRect(rectX, rectY, rectSize, rectSize);

			g.setColour(juce::Colour(32, 33, 40));
			g.fillRect(rectX + 1.0f, rectY + 1.0f, rectSize - 2.0f, rectSize - 2.0f);

		}
		// fontt
		g.setColour(juce::Colour(98, 100, 110));
		g.setFont(ProjectFonts::boldFont(26));
		auto textArea = bounds.withTrimmedLeft(rectX + rectSize + 6.0f);
		g.drawText(button.getButtonText(), textArea, juce::Justification::centredLeft, true);


	}
};

class FilterButton : public Component {
public:
	ToggleButton button;
	FilterButtonLook lnfbutton;

	FilterButton(std::string label, AudioParameterBool* param, ToggleButton::Listener* listener)
	{
		button.setLookAndFeel(&lnfbutton);
		button.setButtonText(label);

		if (param != nullptr)
			button.setToggleState(param->get(), dontSendNotification);

		button.addListener(listener);
		addAndMakeVisible(button);
	};

	~FilterButton()
	{
		button.setLookAndFeel(nullptr);
	}

	void resized() override
	{
		button.setBounds(getLocalBounds());
	};

	void setToggleState(bool flag)
	{
		button.setToggleState(flag, dontSendNotification);
	};

	bool getToggleState()
	{
		return button.getToggleState();
	}

	void addListener(Button::Listener* listener)
	{
		button.addListener(listener);
	};


private:
	FilterButton();
};







// special buttons for wavepatterns
class CustomSwitchButtonSmallLookAndFeel : public juce::LookAndFeel_V4 {
public:
	CustomSwitchButtonSmallLookAndFeel() {
		setColour(juce::ToggleButton::textColourId, juce::Colour(32, 33, 40));
		setColour(juce::ToggleButton::tickColourId, juce::Colour(200, 48, 48));
		setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(32, 33, 40));
	}

	void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override {
		auto bounds = button.getLocalBounds().toFloat();

		// circle congif
		float circleSize = 15.0f;
		float circleX = 8.0f;
		float circleY = (bounds.getHeight() - circleSize) / 2.0f;

		if (button.getToggleState()) {
			// background circle
			g.setColour(juce::Colour(32, 33, 40));
			g.fillEllipse(circleX, circleY, circleSize, circleSize);

			// ontop circle
			g.setColour(juce::Colour(200, 48, 48));
			g.fillEllipse(circleX + 2.0f, circleY + 2.0f, circleSize - 4.0f, circleSize - 4.0f);
		}
		else {
			// background circle
			g.setColour(juce::Colour(32, 33, 40));
			g.fillEllipse(circleX, circleY, circleSize, circleSize);
		}
		// fontt
		g.setColour(juce::Colour(32, 33, 40));
		g.setFont(ProjectFonts::boldFont(28));
		auto textArea = bounds.withTrimmedLeft(circleX + circleSize + 6.0f);
		g.drawText(button.getButtonText(), textArea, juce::Justification::centredLeft, true);

		// divider
		int x = 0;
		int y = 0;
		int height = (int)button.getHeight() * 5;
		int width = (int)button.getWidth();

		// highlight
		auto highlightCol = juce::Colour(177, 174, 180);
		auto shadowCol = juce::Colour(99, 97, 102);
		g.setColour(highlightCol);
		g.fillRect(x - 3, y, 1, height);
		g.fillRect(x + width - 1, y, 1, height * 2);

		// outline
		g.setColour(Colour(15, 19, 21));
		g.fillRect(x - 3, y, 1, height);
		g.fillRect(x + width - 2, y, 1, height * 2);

		// shadow
		g.setColour(shadowCol);
		g.fillRect(x - 3, y, 1, height);
		g.fillRect(x + width - 3, y, 1, height * 2);
	}

};

class SwitchButtonSmall : public Component {
public:
	ToggleButton button;
	CustomSwitchButtonSmallLookAndFeel switchbuttonLAF;

	SwitchButtonSmall(std::string label, AudioParameterBool* param, ToggleButton::Listener* listener)
	{
		button.setLookAndFeel(&switchbuttonLAF);
		button.setButtonText(label);

		if (param != nullptr)
			button.setToggleState(param->get(), dontSendNotification);

		button.addListener(listener);
		addAndMakeVisible(button);
	};

	~SwitchButtonSmall()
	{
		button.setLookAndFeel(nullptr);
	}

	void resized() override
	{
		button.setBounds(getLocalBounds());
	};

	void setToggleState(bool flag)
	{
		button.setToggleState(flag, dontSendNotification);
	};

	bool getToggleState()
	{
		return button.getToggleState();
	}

	void addListener(Button::Listener* listener)
	{
		button.addListener(listener);
	};


private:
	SwitchButtonSmall();
};


class SwitchOptions : public juce::LookAndFeel_V4 {
public:
	SwitchOptions() {
		setColour(juce::ToggleButton::textColourId, juce::Colour(32, 33, 40));
		setColour(juce::ToggleButton::tickColourId, juce::Colour(200, 48, 48));
		setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(32, 33, 40));
	}

	void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override {
		auto bounds = button.getLocalBounds().toFloat();

		// circle config
		float circleSize = 15.0f;
		float circleX = 8.0f;
		float circleY = (bounds.getHeight() - circleSize) / 2.0f;

		if (button.getToggleState()) {
			// background circle
			g.setColour(juce::Colour(32, 33, 40));
			g.fillEllipse(circleX, circleY, circleSize, circleSize);

			// ontop circle
			g.setColour(juce::Colour(200, 48, 48));
			g.fillEllipse(circleX + 2.0f, circleY + 2.0f, circleSize - 4.0f, circleSize - 4.0f);
		}
		else {
			// background circle
			g.setColour(juce::Colour(32, 33, 40));
			g.fillEllipse(circleX, circleY, circleSize, circleSize);
		}

		// font
		g.setColour(juce::Colour(32, 33, 40));
		g.setFont(ProjectFonts::boldFont(28));
		auto textArea = bounds.withTrimmedLeft(circleX + circleSize + 6.0f);
		g.drawText(button.getButtonText(), textArea, juce::Justification::centredLeft, true);

		int rightEdge = (int)button.getWidth() - 2;
		int y = -2;
		int height = (int)button.getHeight();

		// dividers
		g.setColour(juce::Colour(99, 97, 102));
		g.fillRect(rightEdge - 1, y, 1, height);

		g.setColour(juce::Colour(15, 19, 21));
		g.fillRect(rightEdge, y, 1, height);

		g.setColour(juce::Colour(177, 174, 180));
		g.fillRect(rightEdge + 1, y, 1, height);
	}
};

class SwitchButtonOptions : public Component {
public:
	ToggleButton button;
	SwitchOptions switchOptions;

	SwitchButtonOptions(std::string label, AudioParameterBool* param, ToggleButton::Listener* listener)
	{
		button.setLookAndFeel(&switchOptions);
		button.setButtonText(label);

		if (param != nullptr)
			button.setToggleState(param->get(), dontSendNotification);

		button.addListener(listener);
		addAndMakeVisible(button);
	};

	~SwitchButtonOptions()
	{
		button.setLookAndFeel(nullptr);
	}

	void resized() override
	{
		button.setBounds(getLocalBounds());
	};

	void setToggleState(bool flag)
	{
		button.setToggleState(flag, dontSendNotification);
	};

	bool getToggleState()
	{
		return button.getToggleState();
	}

	void addListener(Button::Listener* listener)
	{
		button.addListener(listener);
	};


private:
	SwitchButtonOptions();
};


class PageButton : public juce::Component {
public:
	const int LOCAL_MARGIN = 2;
	const int LABEL_WIDTH = 20;

	juce::TextButton button;

	PageButton(const std::string& labelName, juce::Button::Listener* listener)
		: button(labelName) {
		if (listener != nullptr) {
			button.addListener(listener);
		}
		button.setLookAndFeel(&customLook);
		button.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colour(202, 202, 202));
		button.setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colour(200, 48, 48));

		button.setToggleState(true, juce::NotificationType::dontSendNotification);

		addAndMakeVisible(button);
	}

	~PageButton() override {
		button.setLookAndFeel(nullptr);
	}

	void resized() override {
		juce::Rectangle<int> bounds = getLocalBounds();
		button.setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), 30);
	}

	void setToggleState(bool flag) {
		button.setToggleState(flag, juce::NotificationType::dontSendNotification);
	}

private:
	class ButtonLook : public juce::LookAndFeel_V4 {
	public:
		void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&,
			bool isMouseOverButton, bool isButtonDown) override {
			bool isactive = b.getToggleState();
			auto bounds = b.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
			// button
			if (isactive || isButtonDown) {
				g.setColour(juce::Colour(37, 37, 39));
			}
			else {
				g.setColour(juce::Colour(37, 37, 39));
			}

			if (isMouseOverButton) {
				juce::ColourGradient gradient(
					juce::Colour(37, 37, 39), 0.0f, 0.0f,
					juce::Colour(28, 28, 29), 0.0f, 50.0f,
					false);

				g.setGradientFill(gradient);
			}

			if (isButtonDown) {
				g.setColour(juce::Colour(37, 37, 39));
			}


			g.fillRoundedRectangle(bounds, 0.0f);
			// line
			if (isactive || isMouseOverButton || isButtonDown) {
				g.setColour(juce::Colour(200, 48, 48));
			}
			else {
				g.setColour(juce::Colour(73, 73, 73));
			}
			g.drawLine(1.0f, 0.0f, (float)b.getWidth() - 0.5f, 0.0f, 7.0f);


		}
		juce::Font getTextButtonFont(juce::TextButton&, int) override {
			return ProjectFonts::headerFont(26.0f);
		}

	};

	ButtonLook customLook;
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
		// background
		if (*_chipOscParamsPtr->OscWaveType >= 16) {
			juce::ColourGradient gradient(
				juce::Colour(51, 51, 54), 0.0f, 150.0f,
				juce::Colour(84, 55, 59), 0.0f, 350.0f,
				false);
			g.setGradientFill(gradient);
		}
		else {
			juce::ColourGradient gradient(
				juce::Colour(28, 28, 29), 0.0f, 150.0f,
				juce::Colour(17, 20, 24).withAlpha(0.25f), 0.0f, 350.0f,
				false);
			g.setGradientFill(gradient);
		}
		g.fillAll();


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
		// sliders
		{
			Rectangle<int> bounds = getLocalBounds();
			float columnSize = (float)WAVESAMPLE_LENGTH;
			float rowSize = (float)16;
			float compWidth = getWidth();

			// grid 1
			for (auto i = 0; i <= 16; ++i) {
				float p_y = getHeight() * (0.25f / 4) * i;
				Line<float> line(0.0f, p_y, (float)getWidth(), p_y);
				g.setColour(Colour(20, 20, 20));
				g.drawLine(line, 1.0f);
			}
			for (auto i = 0; i <= 32; ++i) {
				float p_x = getWidth() * (0.125f / 4) * i;
				Line<float> line(p_x, 0.0f, p_x, (float)getHeight());
				g.setColour(Colour(20, 20, 20));
				g.drawLine(line, 1.0f);
			}

			// grid 2
			for (auto i = 0; i <= 4; ++i) {
				float p_y = getHeight() * (0.25f) * i;
				Line<float> line(0.0f, p_y, (float)getWidth(), p_y);
				g.setColour(Colour(20, 20, 20));
				g.drawLine(line, 1.5f);
			}
			for (auto i = 0; i <= 8; ++i) {
				float p_x = getWidth() * (0.125f) * i;
				Line<float> line(p_x, 0.0f, p_x, (float)getHeight());
				g.setColour(Colour(20, 20, 20));
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


			for (auto i = 0; i < WAVESAMPLE_LENGTH; ++i) {
				auto barHeight = getHeight() * (_sampleSliders[i].getValue() + 1.0f) / rowSize;
				auto barWidth = compWidth / (float)WAVESAMPLE_LENGTH;
				Rectangle<float> area2 = Rectangle<float>(
					i * barWidth, getHeight() - barHeight,
					barWidth, barHeight);
				float saturateRate = (i + WAVESAMPLE_LENGTH / 2.f) / (WAVESAMPLE_LENGTH + WAVESAMPLE_LENGTH / 2.f);

				if (*_chipOscParamsPtr->OscWaveType >= 16) {
					g.setColour(Colour(200, 48, 48));
				}
				else {
					g.setColour(Colour(101, 101, 105));
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




		juce::ColourGradient gradient(
			juce::Colour(32, 33, 40), 0.0f, 150.0f,
			juce::Colour(45, 33, 39), 0.0f, 350.0f,
			false);
		g.setGradientFill(gradient);
		g.fillAll();

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

				Colour barColour(200, 48, 48);
				if (i == endStepIndex) {
					barColour = Colours::white;
				}
				else if (endStepIndex >= 0 && i > endStepIndex) {
					barColour = Colour(110, 18, 29);
				}

				g.setColour(barColour);
				g.fillRect(area2.reduced(0.5f));

			}

			// draw grid
			for (auto i = 0; i <= valueMax; ++i) {
				float p_y = getHeight() / float(valueMax) * i;
				Line<float> line(0.0f, p_y, (float)getWidth(), p_y);
				g.setColour(Colour(17, 18, 22));
				g.drawLine(line, 1.5f);
			}

			for (auto i = 0; i <= sampleNum; ++i) {
				float p_x = getWidth() / float(sampleNum) * i;
				Line<float> line(p_x, 0.0f, p_x, (float)getHeight());
				g.setColour(Colour(17, 18, 22));
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

		juce::ColourGradient gradient(
			juce::Colour(51, 51, 54), 0.0f, 150.0f,
			juce::Colour(84, 55, 59), 0.0f, 350.0f,
			false);
		g.setGradientFill(gradient);
		g.fillAll();


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
					: Colour(200, 48, 48));
				g.fillRect(area2.reduced(0.5f));
				Colour barColour(200, 48, 48);
				if (i == endStepIndex) {
					barColour = Colours::white;
				}
				else if (endStepIndex >= 0 && i > endStepIndex) {
					barColour = Colour(110, 18, 29);
				}
				g.setColour(barColour);
				g.fillRect(area2.reduced(0.5f));
			}
			// draw grid
			for (auto i = 1; i < 48; ++i) {
				float p_y = getHeight() / 48.0f * i;
				Line<float> line(0.0f, p_y, (float)getWidth(), p_y);
				g.setColour(i == 24 ? Colour(20, 20, 20) : Colour(26, 33, 38));
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
				g.setColour(Colour(20, 20, 20));
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
