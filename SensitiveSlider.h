/*
  ==============================================================================

    SensitiveSlider.h
    Created: 4 Apr 2023 10:19:49am
    Author:  noone

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class SensitiveSlider :
    public juce::Slider
{
public:
    using Slider = juce::Slider;
    using Style = Slider::SliderStyle;
    using BoxPos = Slider::TextEntryBoxPosition;
    using Mouse = juce::MouseEvent;
    using Wheel = juce::MouseWheelDetails;

    SensitiveSlider() :
        Slider()
    {
        setSliderStyle(Style::RotaryVerticalDrag);
    }

    void mouseWheelMove(const Mouse& mouse, const Wheel& wheel) override
    {
        auto newWheel = wheel;
        const auto speed = mouse.mods.isCtrlDown() ? SensitiveWheel : NormalWheel;
        newWheel.deltaY *= speed;

        Slider::mouseWheelMove(mouse, newWheel);
    }

private:
    static constexpr float NormalWheel = .8f;
    static constexpr float SensitiveWheel = .05f;
};
