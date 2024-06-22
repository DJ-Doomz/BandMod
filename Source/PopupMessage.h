/*
  ==============================================================================

    PopupMessage.h
    Created: 22 Jun 2024 2:21:11pm
    Author:  noone

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class PopupMessage  : public juce::Component
{
public:
    PopupMessage(BandModAudioProcessor& p):
        processor(p)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        addAndMakeVisible(closeButton);
        closeButton.setButtonText("ok whatever");
        closeButton.onClick = [this] {setVisible(false); processor.showWarning = false; };
    }

    ~PopupMessage() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colours::black);
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 6);   // draw an outline around the component
        g.setColour (juce::Colours::white);
        g.drawText ("noise warning", getLocalBounds(),
                    juce::Justification::centred, true);   // draw some placeholder text


    }

    void resized() override
    {
        auto lb = getLocalBounds();
        auto bb = lb.removeFromBottom(lb.getHeight() / 8);
        closeButton.setBounds(bb);
    }

private:
    BandModAudioProcessor& processor;
    juce::TextButton closeButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupMessage)
};
