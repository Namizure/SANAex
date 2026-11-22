/*
  ==============================================================================

	MyInterface.h
	Created with Maneki UI Designer.

	This file was generated from a Maneki design. You can edit it freely, but
	exporting the design again will overwrite it.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MyButton4LookAndFeel.h"
#include "MyButton5LookAndFeel.h"
#include "MyButton6LookAndFeel.h"
#include "MyComboBox2LookAndFeel.h"

class MyInterface : public juce::Component
{
public:
	MyInterface();
	~MyInterface() override;

	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	MyButton4LookAndFeel   myButton4LookAndFeel;
	MyButton5LookAndFeel   myButton5LookAndFeel;
	MyButton6LookAndFeel   myButton6LookAndFeel;
	MyComboBox2LookAndFeel myComboBox2LookAndFeel;
	juce::TextButton       SaveBtn;
	juce::TextButton       LoadPackBtn;
	juce::TextButton       drumModeButton;
	juce::TextButton       Prev;
	juce::TextButton       Next;
	juce::ComboBox         presetList;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyInterface)
};
