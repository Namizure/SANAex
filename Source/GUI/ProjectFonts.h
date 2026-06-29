/*
  ==============================================================================

	ProjectsFont.h
	Created: 12 Jun 2026 10:16pm
		Modified: 22 Jun 2026
		Author:  Namizure

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

namespace ProjectFonts
{
	inline juce::Typeface::Ptr getBarlowTypeface() {
		static auto typeface = juce::Typeface::createSystemTypefaceFor(
			BinaryData::BarlowCondensedRegular_ttf,
			BinaryData::BarlowCondensedRegular_ttfSize
		);
		return typeface;
	}

	inline juce::Font headerFont(float height = 24.0f) {
		return juce::Font(getBarlowTypeface()).withHeight(height);
	}

	inline juce::Font buttonFont(float height = 14.0f) {
		return juce::Font(getBarlowTypeface()).withHeight(height);
	}

	class LookAndFeel : public juce::LookAndFeel_V4 {
	public:
		juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override {
			return buttonFont((float)juce::jlimit(12, 18, buttonHeight - 6));
		}

		juce::Font getLabelFont(juce::Label&) override {
			return buttonFont(14.0f);
		}

		juce::Font getComboBoxFont(juce::ComboBox&) override {
			return buttonFont(14.0f);
		}
	};
}