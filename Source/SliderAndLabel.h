/*
  ==============================================================================

    SliderAndLabel.h
    Created: 30 Apr 2023 10:28:54am
    Author:  noone

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "SensitiveSlider.h"

constexpr float sliderHeight = 0.8f;
constexpr float labelHeight = 1.f - sliderHeight;

class SliderAndLabel : public Component
{
public:
    explicit SliderAndLabel(const String& labelText)
    {
        addAndMakeVisible(slider);
        addAndMakeVisible(label);

        //setup slider any way you like here:
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
        
        //setup label any way you like here:
        label.setText(labelText, dontSendNotification);
        label.setJustificationType(Justification::centred);
    }
    

    void resized() override
    {
        label.setBoundsRelative(0.f, 1-labelHeight, 1.f, labelHeight);
        slider.setBoundsRelative(0.f, 0, 1.f, sliderHeight);
    }

    SensitiveSlider& getSlider() { return slider; }
    Label& getLabel() { return label; }

private:
    SensitiveSlider slider;
    Label label;
};