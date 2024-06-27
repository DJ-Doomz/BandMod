/*
  ==============================================================================

    BandVUMeeter.h
    Created: 27 Jun 2024 11:25:48am
    Author:  noone

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class BandVUMeeter  : public juce::Component
{
public:
    BandVUMeeter():
        currentLevel(0),
        db(nullptr)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        setOpaque(false);
    }

    ~BandVUMeeter() override
    {
    }

    void setValuetoMonitor(std::atomic<float>* value)
    {
        db = value;
    }

    void paint (juce::Graphics& g) override
    {
        auto lb = getLocalBounds();

        // update vu levels
        float fdb = abs(*db);
        if (fdb > currentLevel)
        {
            currentLevel = fdb;
        }
        else
        {
            currentLevel *= .7;
        }
        
        float mindB = -60, maxdB = 2;
        
        float ratio = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(currentLevel)), mindB, maxdB, 0.f, 1.f);

        g.setColour(Colours::white);
        g.fillRect(lb.withTrimmedTop(lb.proportionOfHeight(1 - ratio)));

        repaint();
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

private:
    std::atomic<float> *db;
    float currentLevel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandVUMeeter)
};
