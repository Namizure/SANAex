/*
  ==============================================================================

	ManekiText.h
	Created with Maneki UI Designer.

	This file was generated from a Maneki design. You can edit it freely, but
	exporting the design again will overwrite it.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


#include <optional>
namespace ManekiText
{
	struct Settings : public juce::ReferenceCountedObject
	{
		juce::String name;
		juce::Typeface::Ptr typeface;
		float height = 0.0f;
		std::optional<bool> isBold, isItalic;
		std::optional<juce::Justification> justification;
		juce::Point<int> offset;

		Settings& font(const juce::String& familyName) { name = familyName; return *this; }
		Settings& font(juce::Typeface::Ptr t) { typeface = std::move(t); return *this; }
		Settings& size(float newHeight) { height = newHeight; return *this; }
		Settings& bold(bool b = true) { isBold = b; return *this; }
		Settings& italic(bool b = true) { isItalic = b; return *this; }
		Settings& justify(juce::Justification j) { justification = j; return *this; }
		Settings& move(int dx, int dy) { offset = { dx, dy }; return *this; }

		juce::Font apply(juce::Font f) const
		{
			auto h = height > 0.0f ? height : f.getHeight();

			if (typeface != nullptr)
				return juce::Font(juce::FontOptions(typeface).withHeight(h));

			auto flags = f.getStyleFlags();

			if (isBold.has_value())    flags = *isBold ? (flags | juce::Font::bold) : (flags & ~juce::Font::bold);
			if (isItalic.has_value())  flags = *isItalic ? (flags | juce::Font::italic) : (flags & ~juce::Font::italic);

			if (name.isNotEmpty())
				return juce::Font(juce::FontOptions(name, h, flags));

			f.setHeight(h);
			f.setStyleFlags(flags);
			return f;
		}
	};

	inline const juce::Identifier propertyId{ "manekiText" };
	inline Settings& set(juce::Component& c)
	{
		if (auto* s = dynamic_cast<Settings*> (c.getProperties()[propertyId].getObject()))
			return *s;

		auto* s = new Settings();
		c.getProperties().set(propertyId, juce::var(s));
		return *s;
	}

	inline void clear(juce::Component& c) { c.getProperties().remove(propertyId); }

	inline const Settings* get(const juce::Component& c)
	{
		return dynamic_cast<const Settings*> (c.getProperties()[propertyId].getObject());
	}

	inline juce::Font font(const juce::Component& c, juce::Font lookFont)
	{
		auto* s = get(c);
		return s != nullptr ? s->apply(lookFont) : lookFont;
	}

	inline juce::Justification justification(const juce::Component& c, juce::Justification lookJustification)
	{
		auto* s = get(c);
		return s != nullptr && s->justification.has_value() ? *s->justification : lookJustification;
	}

	inline juce::Rectangle<int> area(const juce::Component& c, juce::Rectangle<int> lookArea)
	{
		auto* s = get(c);
		return s != nullptr ? lookArea + s->offset : lookArea;
	}
	template <typename Base = juce::LookAndFeel_V4>
	class LookAndFeel : public Base
	{
	public:
		using Base::Base;

		juce::Font getTextButtonFont(juce::TextButton& b, int buttonHeight) override
		{
			return font(b, Base::getTextButtonFont(b, buttonHeight));
		}

		void drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) override
		{
			auto f = getTextButtonFont(b, b.getHeight());
			g.setFont(f);
			g.setColour(b.findColour(b.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId)
				.withMultipliedAlpha(b.isEnabled() ? 1.0f : 0.5f));

			auto yIndent = juce::jmin(4, b.proportionOfHeight(0.3f));
			auto cornerSize = juce::jmin(b.getHeight(), b.getWidth()) / 2;
			auto fontHeight = juce::roundToInt(f.getHeight() * 0.6f);
			auto leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (b.isConnectedOnLeft() ? 4 : 2));
			auto rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (b.isConnectedOnRight() ? 4 : 2));

			g.drawFittedText(b.getButtonText(),
				area(b, { leftIndent, yIndent, b.getWidth() - leftIndent - rightIndent, b.getHeight() - yIndent * 2 }),
				justification(b, juce::Justification::centred), 2);
		}

		void drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool isHighlighted, bool isDown) override
		{
			auto fontSize = juce::jmin(15.0f, (float)b.getHeight() * 0.75f);
			auto tickWidth = fontSize * 1.1f;

			this->drawTickBox(g, b, 4.0f, ((float)b.getHeight() - tickWidth) * 0.5f, tickWidth, tickWidth,
				b.getToggleState(), b.isEnabled(), isHighlighted, isDown);

			g.setColour(b.findColour(juce::ToggleButton::textColourId).withMultipliedAlpha(b.isEnabled() ? 1.0f : 0.5f));
			g.setFont(font(b, juce::FontOptions(fontSize)));
			g.drawFittedText(b.getButtonText(),
				area(b, b.getLocalBounds().withTrimmedLeft(juce::roundToInt(tickWidth) + 10).withTrimmedRight(2)),
				justification(b, juce::Justification::centredLeft), 10);
		}

		juce::Font getComboBoxFont(juce::ComboBox& box) override
		{
			return font(box, Base::getComboBoxFont(box));
		}

		void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
		{
			Base::positionComboBoxText(box, label);
			label.setBounds(area(box, label.getBounds()));
			label.setFont(getComboBoxFont(box));
		}

		juce::Label* createSliderTextBox(juce::Slider& slider) override
		{
			auto* label = Base::createSliderTextBox(slider);
			label->setFont(font(slider, label->getFont()));
			label->setJustificationType(justification(slider, label->getJustificationType()));
			return label;
		}

		void drawGroupComponentOutline(juce::Graphics& g, int width, int height, const juce::String& text,
			const juce::Justification& position, juce::GroupComponent& group) override
		{
			const float textH = 15.0f, indent = 3.0f, textEdgeGap = 4.0f;
			auto cs = 5.0f;

			auto f = font(group, this->withDefaultMetrics(juce::FontOptions{ textH }));

			juce::Path p;
			auto x = indent;
			auto y = f.getAscent() - 3.0f;
			auto w = juce::jmax(0.0f, (float)width - x * 2.0f);
			auto h = juce::jmax(0.0f, (float)height - y - indent);
			cs = juce::jmin(cs, w * 0.5f, h * 0.5f);
			auto cs2 = 2.0f * cs;

			auto textW = text.isEmpty() ? 0.0f
				: juce::jlimit(0.0f, juce::jmax(0.0f, w - cs2 - textEdgeGap * 2),
					(float)juce::GlyphArrangement::getStringWidthInt(f, text) + textEdgeGap * 2.0f);
			auto textX = cs + textEdgeGap;

			if (position.testFlags(juce::Justification::horizontallyCentred))
				textX = cs + (w - cs2 - textW) * 0.5f;
			else if (position.testFlags(juce::Justification::right))
				textX = w - cs - textW - textEdgeGap;

			p.startNewSubPath(x + textX + textW, y);
			p.lineTo(x + w - cs, y);
			p.addArc(x + w - cs2, y, cs2, cs2, 0, juce::MathConstants<float>::halfPi);
			p.lineTo(x + w, y + h - cs);
			p.addArc(x + w - cs2, y + h - cs2, cs2, cs2, juce::MathConstants<float>::halfPi, juce::MathConstants<float>::pi);
			p.lineTo(x + cs, y + h);
			p.addArc(x, y + h - cs2, cs2, cs2, juce::MathConstants<float>::pi, juce::MathConstants<float>::pi * 1.5f);
			p.lineTo(x, y + cs);
			p.addArc(x, y, cs2, cs2, juce::MathConstants<float>::pi * 1.5f, juce::MathConstants<float>::twoPi);
			p.lineTo(x + textX, y);

			auto alpha = group.isEnabled() ? 1.0f : 0.5f;

			g.setColour(group.findColour(juce::GroupComponent::outlineColourId).withMultipliedAlpha(alpha));
			g.strokePath(p, juce::PathStrokeType(2.0f));

			g.setColour(group.findColour(juce::GroupComponent::textColourId).withMultipliedAlpha(alpha));
			g.setFont(f);
			g.drawText(text, juce::roundToInt(x + textX), 0, juce::roundToInt(textW), juce::roundToInt(f.getHeight()),
				juce::Justification::centred, true);
		}
	};
}
