/*
  ==============================================================================

    myButton.h
    Created: 6 May 2023 10:55:37am
    Author:  noone

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class myButton  : public juce::Component
{
public:
    explicit myButton(const String& labelText)
    {
        addAndMakeVisible(button);
        addAndMakeVisible(label);

        //setup button any way you like here:
        button.setEnabled(true);
        button.setClickingTogglesState(true);
        
        //setup label any way you like here:
        label.setText(labelText, dontSendNotification);
        label.setJustificationType(Justification::right);
    }

    ~myButton() override
    {
    }

    void resized() override
    {
        auto lb = getLocalBounds();
        label.setBounds(lb.removeFromLeft(lb.getWidth()/2));
        button.setBounds(lb);
    }

    ToggleButton& getButton() { return button; }
    Label& getLabel() { return label; }

private:
    ToggleButton button;
    Label label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (myButton)
};
