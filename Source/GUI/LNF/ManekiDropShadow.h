/*
  ==============================================================================

	ManekiDropShadow.h
	Created with Maneki UI Designer.

	This file was generated from a Maneki design. You can edit it freely, but
	exporting the design again will overwrite it.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


#include <vector>
#include <algorithm>
namespace ManekiGraphics
{
	inline void drawDropShadow(juce::Graphics& g, const juce::Path& shape, juce::Colour colour,
		int radius, juce::Point<int> offset)
	{
		if (shape.isEmpty() || colour.isTransparent())
			return;

		if (radius <= 0)
		{
			g.setColour(colour);
			g.fillPath(shape, juce::AffineTransform::translation((float)offset.x, (float)offset.y));
			return;
		}

		struct CachedShadow
		{
			juce::String key;
			juce::Image image;
			juce::Point<int> origin;
		};

		static std::vector<CachedShadow> cache;
		auto bounds = shape.getBounds().getSmallestIntegerContainer();
		auto relative = shape;
		relative.applyTransform(juce::AffineTransform::translation((float)-bounds.getX(), (float)-bounds.getY()));
		auto key = juce::String(radius) + " " + colour.toString() + " " + relative.toString();

		auto found = std::find_if(cache.begin(), cache.end(), [&key](const CachedShadow& c) { return c.key == key; });

		if (found == cache.end())
		{
			auto area = juce::Rectangle<int>(bounds.getWidth(), bounds.getHeight()).expanded(radius + 2);
			juce::Image image(juce::Image::ARGB, area.getWidth(), area.getHeight(), true, juce::SoftwareImageType());

			{
				juce::Graphics shadowGraphics(image);
				shadowGraphics.addTransform(juce::AffineTransform::translation((float)-area.getX(), (float)-area.getY()));
				juce::DropShadow(colour, radius, {}).drawForPath(shadowGraphics, relative);
			}

			cache.insert(cache.begin(), { key, image, area.getPosition() });

			if (cache.size() > 64)
				cache.pop_back();
		}
		else if (found != cache.begin())
		{
			std::rotate(cache.begin(), found, found + 1);
		}

		auto& shadow = cache.front();
		g.drawImageAt(shadow.image, bounds.getX() + shadow.origin.x + offset.x, bounds.getY() + shadow.origin.y + offset.y);
	}
}
