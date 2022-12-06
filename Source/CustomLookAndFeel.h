#pragma once
#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
	void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
		float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
	{
		//Circle
		auto radius = ((float)juce::jmin(width / 2, height / 2) - 4.0f) * 0.7f;
		auto centreX = (float)x + (float)width * 0.5f;
		auto centreY = (float)y + (float)height * 0.5f;
		auto rx = centreX - radius;
		auto ry = centreY - radius;
		auto rw = radius * 2.0f;
		auto angle = (rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle)) - 0.2f;

		g.setColour(juce::Colours::white);
		g.drawEllipse(rx, ry, rw, rw, 2.0f);

		//Dot
		juce::Path dot;
		dot.addEllipse(0.0f, -radius * 0.8f, 6.0f, 6.0f);
		dot.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
		g.fillPath(dot);
	}

	void drawButtonBackground(juce::Graphics& g, juce::Button& button,
		const juce::Colour& backgroundColour, bool isHighlighted, bool isButtonDown) override
	{
		juce::Rectangle<int> buttonArea = button.getLocalBounds();
		g.setColour(juce::Colours::grey);
	}
};