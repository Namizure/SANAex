/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace {
	const Colour PANEL_COLOUR() { return Colour(174, 187, 193); }
	const Colour HEADER_COLOUR() { return Colour(174, 187, 193); }
	const Colour FONT_COLOUR() { return Colours::black; }
	const Colour BACKGROUND_COLOUR() { return Colour(0.0f, 0.0f, 0.0f, 0.0f); }
	const float HEADER_HEIGHT = 24.0f;
	const std::int32_t LOCAL_MARGIN = 2;
}  // namespace

template <typename SampleType>
class AudioBufferQueue {
public:
	static constexpr size_t order =
		9;
	static constexpr size_t bufferSize =
		1U << order;
	static constexpr size_t numBuffers = 5;

	void push(const SampleType* dataToPush, size_t numSamples) {
		// DBG("PUSH");

		jassert(numSamples <= bufferSize);

		int start1, size1, start2, size2;
		abstractFifo.prepareToWrite(1, start1, size1, start2, size2);

		jassert(size1 <= 1);
		jassert(size2 == 0);

		if (size1 > 0) {
			FloatVectorOperations::copy(buffers[(size_t)start1].data(), dataToPush,
				(int)jmin(bufferSize, numSamples));
		}
		abstractFifo.finishedWrite(size1);
	}

	void pop(SampleType* outputBuffer) {
		// DBG("POP");

		int start1, size1, start2, size2;
		abstractFifo.prepareToRead(1, start1, size1, start2, size2);

		jassert(size1 <= 1);
		jassert(size2 == 0);

		if (size1 > 0) {
			FloatVectorOperations::copy(outputBuffer, buffers[(size_t)start1].data(),
				(int)bufferSize);
		}
		abstractFifo.finishedRead(size1);
	}

private:
	std::array<std::array<SampleType, bufferSize>, numBuffers> buffers;
	AbstractFifo abstractFifo{ numBuffers };
};

template <typename SampleType>
class ScopeDataCollector {
public:
	ScopeDataCollector(AudioBufferQueue<SampleType>& queueToUse)
		: audioBufferQueue(queueToUse) {
	}

	void process(const SampleType* data, size_t numSamples) {
		size_t index = 0;
		if (currentState == State::WaitingForTrigger) {
			while (index++ < numSamples) {
				auto currentSample = *data++;
				if (currentSample >= triggerLevel && prevSample < triggerLevel) {
					numCollected = 0;
					currentState = State::Collecting;
					break;
				}

				prevSample = currentSample;
			}
		}
		if (currentState == State::Collecting) {
			while (index++ < numSamples) {
				buffer[numCollected++] = *data++;
				if (numCollected == buffer.size()) {
					audioBufferQueue.push(buffer.data(), buffer.size());
					currentState = State::WaitingForTrigger;
					prevSample = SampleType(100);
					break;
				}
			}
		}
	}

private:
	enum class State { WaitingForTrigger, Collecting };
	static constexpr auto triggerLevel = SampleType(0.001);
	AudioBufferQueue<SampleType>& audioBufferQueue;
	std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize>
		buffer;
	size_t numCollected;
	SampleType prevSample = SampleType(100);
	State currentState{ State::WaitingForTrigger };
};

template <typename SampleType>
class ScopeComponent : public juce::Component, private juce::Timer {
public:
	using Queue = AudioBufferQueue<SampleType>;
	ScopeComponent(Queue& queueuToUse) : audioBufferQueue(queueuToUse) {
		sampleData.fill(SampleType(0));
		setFramePerSecond(30);
	}


	void setFramePerSecond(int framePerSecond) {
		jassert(framePerSecond > 0 && framePerSecond < 1000);
		startTimerHz(framePerSecond);
	}



	static void paintHeader(Graphics& g, Rectangle<int> bounds, std::string text) {
		{
			auto x = 0.0f,
				y = HEADER_HEIGHT - 2;
			auto width = (float)bounds.getWidth(),
				height = (float)bounds.getHeight() - y;
			auto cornerSize = 0.0f,
				thickness = 1.0f;

			g.setColour(Colour(19, 19, 19));
			g.fillRoundedRectangle(x, y, width, height, cornerSize);
			g.setColour(Colour(255, 255, 255)); //(22, 25, 30)
			g.drawRoundedRectangle(x, y, width, height, cornerSize, thickness);



		}

		{
			g.setFont(ProjectFonts::headerFont(32.0f));
			auto x = 1.f,
				y = -5.0f;
			auto width = (float)g.getCurrentFont().getStringWidth(text),
				height = HEADER_HEIGHT;

			g.setColour(BACKGROUND_COLOUR());
			g.fillRect(x, y, width, height);
			g.setColour(Colour(165, 178, 184));
			g.drawText(text, x, y, width, height, Justification::topLeft, false);
		}
	}

	void paint(Graphics& g) override {
		paintHeader(g, getLocalBounds(), "");


		Rectangle<int> drawArea = getLocalBounds();
		drawArea.removeFromTop(HEADER_HEIGHT);

		{
			g.setColour(Colour(64, 75, 76));
			auto center = drawArea.getHeight() / 2 + HEADER_HEIGHT;
			g.drawLine(0, center, drawArea.getWidth(), center, 1.0f);
		}


		SampleType drawX = (SampleType)drawArea.getX();
		SampleType drawY = (SampleType)drawArea.getY();
		SampleType drawH = (SampleType)drawArea.getHeight();
		SampleType drawW = (SampleType)drawArea.getWidth();
		Rectangle<SampleType> scopeRect =
			Rectangle<SampleType>{ drawX, drawY, drawW, drawH };


		g.setColour(Colour(164, 176, 181));
		plot(sampleData.data(), sampleData.size(), g, scopeRect, SampleType(1.0));
	}

	void resized() override {}

private:
	void timerCallback() override {
		audioBufferQueue.pop(sampleData.data());
		repaint();
	}

	static void plot(const SampleType* data, size_t numSamples, Graphics& g,
		juce::Rectangle<SampleType> rect,
		SampleType scaler = SampleType(1)) {
		auto w = rect.getWidth();
		auto h = rect.getHeight();
		auto right = rect.getRight();
		auto alignedCentre = rect.getBottom() - h / 2.f;
		auto gain = h * scaler;

		auto clipY = [&h](float y) {
			y = std::fmax(HEADER_HEIGHT, y);
			y = std::fmin(h + HEADER_HEIGHT, y);
			return y;
			};

		for (size_t i = 1; i < numSamples; ++i) {
			const float x1 =
				jmap(SampleType(i - 1), SampleType(0), SampleType(numSamples - 1),
					SampleType(right - w), SampleType(right));
			float y1 = clipY(alignedCentre - gain * data[i - 1]);
			const float x2 =
				jmap(SampleType(i), SampleType(0), SampleType(numSamples - 1),
					SampleType(right - w), SampleType(right));
			const float y2 = clipY(alignedCentre - gain * data[i]);
			const float t = 1.0f;
			g.drawLine(x1, y1, x2, y2, t);
		}
	}


	Queue& audioBufferQueue;
	std::array<SampleType, Queue::bufferSize>
		sampleData;
};
