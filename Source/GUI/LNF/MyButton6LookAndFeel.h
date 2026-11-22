/*
  ==============================================================================

	MyButton6LookAndFeel.h
	Created with Maneki UI Designer.

	This file was generated from a Maneki design. You can edit it freely, but
	exporting the design again will overwrite it.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class MyButton6LookAndFeel : public juce::LookAndFeel_V4
{
public:
	MyButton6LookAndFeel();

	void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
		bool isHighlighted, bool isDown) override;
	void drawButtonText(juce::Graphics&, juce::TextButton&, bool isHighlighted, bool isDown) override;

private:
	juce::Image normalSprite;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyButton6LookAndFeel)
};
