/*
  ==============================================================================

    myLookAndFeel.h
    Created: 22 Mar 2023 12:55:15pm
    Author:  noone

  ==============================================================================
*/
#include <JuceHeader.h>

#pragma once
class myLookAndFeel : public juce::LookAndFeel_V4
{
public:
    myLookAndFeel();

    void drawLinearSlider(Graphics&, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        Slider::SliderStyle, Slider&) override;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override;

    void drawRegularSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&);

    void drawBipolarSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&);

    /*
    void drawButtonBackground(Graphics& g,
        Button& button,
        const Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;
        */

    enum myColours
    {
        backgroundColourId,
        outlineColourId,
        knobBGColourId,
        buttonBGColourId
    };


private:
    ColourGradient blueGlow;
};