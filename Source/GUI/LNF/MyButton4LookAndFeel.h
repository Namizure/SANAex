/*
  ==============================================================================

	MyButton4LookAndFeel.h
	Created with Maneki UI Designer.

	This file was generated from a Maneki design. You can edit it freely, but
	exporting the design again will overwrite it.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ManekiText.h"

class MyButton4LookAndFeel : public ManekiText::LookAndFeel<>
{
public:
	MyButton4LookAndFeel();

	void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
		bool isHighlighted, bool isDown) override;
	void drawButtonText(juce::Graphics&, juce::TextButton&, bool isHighlighted, bool isDown) override;

private:
	juce::Image normalSprite;
	juce::Image onSprite;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyButton4LookAndFeel)
};
