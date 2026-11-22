/*
  ==============================================================================

	MyComboBox2LookAndFeel.h
	Created with Maneki UI Designer.

	This file was generated from a Maneki design. You can edit it freely, but
	exporting the design again will overwrite it.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ManekiText.h"

class MyComboBox2LookAndFeel : public ManekiText::LookAndFeel<>
{
public:
	MyComboBox2LookAndFeel();

	void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
		int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;
	juce::Font getComboBoxFont(juce::ComboBox&) override;
	void positionComboBoxText(juce::ComboBox&, juce::Label&) override;

private:
	juce::Image backgroundSprite;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyComboBox2LookAndFeel)
};
