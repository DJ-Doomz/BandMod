/*
  ==============================================================================

    BandModComponent.h
    Created: 13 Jun 2024 12:05:10pm
    Author:  noone

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SliderAndLabel.h"
#include "SensitiveSlider.h"
#include "myDrawableButton.h"
#include "myLookAndFeel.h"
#include "myButton.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/*
*/

class BandComponent : public juce::Component
{
public:
    BandComponent()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        addAndMakeVisible(preSlider);
        addAndMakeVisible(postSlider);
        addAndMakeVisible(fmSlider);
        addAndMakeVisible(fmPitchSlider);
        addAndMakeVisible(feedbackSlider);
        addAndMakeVisible(delaySlider);
    }

    ~BandComponent() override
    {
    }

    void paint(juce::Graphics& g) override
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */
        g.setColour(juce::Colours::red);
        g.drawText("BandComponent", getLocalBounds(),
            juce::Justification::centred, true);   // draw some placeholder text
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));   // clear the background

        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 1);   // draw an outline around the component
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        juce::Rectangle r = getLocalBounds();

        preSlider.setBounds(r.removeFromTop(r.getHeight() / 6));
        postSlider.setBounds(r.removeFromTop(r.getHeight() / 5));
        fmSlider.setBounds(r.removeFromTop(r.getHeight() / 4));
        fmPitchSlider.setBounds(r.removeFromTop(r.getHeight() / 3));
        feedbackSlider.setBounds(r.removeFromTop(r.getHeight() / 2));
        delaySlider.setBounds(r.removeFromTop(r.getHeight() / 1));
    }

    void addAttachment(juce::AudioProcessorValueTreeState& vts, int bandNum)
    {
        auto stri = String(bandNum);
        auto fmname = String("fmAmt") + stri;
        auto prename = String("preGain") + stri;
        auto postname = String("postGain") + stri;
        auto feedbkname = String("feedBack") + stri;
        auto fmpitchname = String("fmPitch") + stri;
        auto delaytimename = String("delaytime") + stri;
        fmAttachment.reset(new SliderAttachment(vts, fmname, fmSlider.getSlider()));
        preAttachment.reset(new SliderAttachment(vts, prename, preSlider.getSlider()));
        postAttachment.reset(new SliderAttachment(vts, postname, postSlider.getSlider()));
        fmPitchAttachment.reset(new SliderAttachment(vts, fmpitchname, fmPitchSlider.getSlider()));
        feedbackAttachment.reset(new SliderAttachment(vts, feedbkname, feedbackSlider.getSlider()));
        delayAttachment.reset(new SliderAttachment(vts, delaytimename, delaySlider.getSlider()));
    }

private:
    SliderAndLabel preSlider{ "PreGain" },
        postSlider{ "PostGain" },
        fmSlider{ "FM Amount" },
        fmPitchSlider{ "FM Transpose" },
        feedbackSlider{ "Feedback" },
        delaySlider{ "Delay" };

    std::unique_ptr<SliderAttachment> preAttachment,
        postAttachment,
        fmAttachment,
        fmPitchAttachment,
        feedbackAttachment,
        delayAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandComponent)
};



class BandModComponent  : public juce::Component
{
public:
    BandModComponent()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        for (int i = 0; i < 4; i++)
        {
            addAndMakeVisible(bands[i]);
        }
        
    }

    ~BandModComponent() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */

        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

        g.setColour (juce::Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (juce::Colours::white);
        g.drawText ("BandModComponent", getLocalBounds(),
                    juce::Justification::centred, true);   // draw some placeholder text
    }

    void addAttachment(juce::AudioProcessorValueTreeState& vts)
    {
        for (int i = 0; i < 4; i++)
        {
            bands[i].addAttachment(vts, i);
        }
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        auto r = getLocalBounds();
        auto stamp = r.removeFromLeft(r.getWidth() / 4);
        for (int i = 0; i < 4; i++)
        {
            bands[i].setBounds(stamp);
            stamp.translate(stamp.getWidth(), 0);
        }
    }

private:
    BandComponent bands[4];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandModComponent)
};


