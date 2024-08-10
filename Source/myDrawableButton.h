/*
  ==============================================================================

    myDrawableButton.h
    Created: 4 Apr 2023 12:25:34pm
    Author:  noone

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/

typedef std::function<void(Graphics& g, bool, bool, juce::Rectangle<int>)> DrawingFunction;

class myDrawableButton : public Button //, LookAndFeel_V4
{
public:
    myDrawableButton(const String& buttonName) : Button(buttonName) { }
    ~myDrawableButton() { }
    void paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
    {

        drawf(g, shouldDrawButtonAsHighlighted, getToggleState(), getLocalBounds());
    }

    void setDrawingFunction(DrawingFunction& df)
    {
        drawf = df;
    }

private:
    DrawingFunction drawf;
};