/*
  ==============================================================================

    BandGraphComponent.h
    Created: 17 Jun 2024 11:06:42am
    Author:  noone

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BandMod.h"

//==============================================================================
/*
*/
class BandGraphComponent  : public juce::Component
{
public:
    BandGraphComponent(BandModAudioProcessor& p, BandMod& b) :processor(p), bm(b)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

    }

    ~BandGraphComponent() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll(Colours::black);

        auto lb = getLocalBounds();
        auto responseArea = lb;
        auto w = lb.getWidth();

        const double outputMin = responseArea.getBottom();
        const double outputMax = responseArea.getY();

        auto sampleRate = processor.getSampleRate();

        auto pitchx = responseArea.getX() + mapFromLog10(bm.getTrackedPitch(), 20.f, 20000.f) * w - 2;

        g.setColour(Colours::white);
        g.drawLine(pitchx, lb.getBottom(), pitchx, lb.getY());

        repaint();
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

private:
    BandModAudioProcessor& processor;
    BandMod& bm;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandGraphComponent)
};
