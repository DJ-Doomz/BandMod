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
#include "BandGraphComponent.h"
#include "PopupMessage.h"

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
    BandModComponent(BandModAudioProcessor& p, BandMod& b) : processor(p),
        bm(b),
        bandGraphComponent(p, b),
        modeButton("FB Mode"),
        warningMessage(p)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        for (int i = 0; i < 4; i++)
        {
            addAndMakeVisible(bands[i]);
        }
        addAndMakeVisible(lowFreqSlider);
        addAndMakeVisible(midFreqSlider);
        addAndMakeVisible(highFreqSlider);
        addAndMakeVisible(lowOrderSlider);
        addAndMakeVisible(midOrderSlider);
        addAndMakeVisible(highOrderSlider);
        addAndMakeVisible(bandGraphComponent);
        addAndMakeVisible(modeButton);
        addAndMakeVisible(drySlider);
        addAndMakeVisible(wetSlider);
        addAndMakeVisible(releaseSlider);

        if (p.showWarning)
        {
            addAndMakeVisible(warningMessage);
            warningMessage.toFront(true);
        }

        lowOrderSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        midOrderSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        highOrderSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        lowOrderSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        midOrderSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        highOrderSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
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
    }

    void addAttachment(juce::AudioProcessorValueTreeState& vts)
    {
        for (int i = 0; i < 4; i++)
        {
            bands[i].addAttachment(vts, i);
        }
       
        lowFreqAttachment.reset(new SliderAttachment(vts, "LowFreq", lowFreqSlider.getSlider()));
        midFreqAttachment.reset(new SliderAttachment(vts, "MidFreq", midFreqSlider.getSlider()));
        highFreqAttachment.reset(new SliderAttachment(vts, "HighFreq", highFreqSlider.getSlider()));

        lowOrderAttachment.reset(new SliderAttachment(vts, "LowOrder", lowOrderSlider));
        midOrderAttachment.reset(new SliderAttachment(vts, "MidOrder", midOrderSlider));
        highOrderAttachment.reset(new SliderAttachment(vts, "HighOrder", highOrderSlider));

        modeAttachment.reset(new ButtonAttachment(vts, "Mode", modeButton.getButton()));

        dryAttachment.reset(new SliderAttachment(vts, "Dry", drySlider.getSlider()));
        wetAttachment.reset(new SliderAttachment(vts, "Wet", wetSlider.getSlider()));
        releaseAttachment.reset(new SliderAttachment(vts, "Release", releaseSlider.getSlider()));
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        auto r = getLocalBounds();

        warningMessage.setBounds(r.expanded(-r.getWidth()/6, -r.getHeight() / 6));

        auto bg = r.removeFromTop(r.getHeight() / 3);
        bandGraphComponent.setBounds(bg);

        auto t = r.removeFromTop(100);
        auto w = t.getWidth() / 8;
        auto stamp1 = t.removeFromLeft(w);
        auto smaller = stamp1.translated(w, 0);
        smaller.setWidth(w / 2);
        smaller.setHeight(w / 2);
        stamp1.translate(w * 1.5, 0);
        smaller.translate(w * 1.5, 0);
        lowFreqSlider.setBounds(stamp1);
        lowOrderSlider.setBounds(smaller);
        stamp1.translate(w * 2, 0);
        smaller.translate(w * 2, 0);
        midFreqSlider.setBounds(stamp1);
        midOrderSlider.setBounds(smaller);
        stamp1.translate(w * 2, 0);
        smaller.translate(w * 2, 0);
        highFreqSlider.setBounds(stamp1);
        highOrderSlider.setBounds(smaller);
        stamp1.translate(w*1.5, 0);
        modeButton.setBounds(stamp1);

        auto bottom = r.removeFromBottom(r.getHeight() / 4);
        auto stamp = r.removeFromLeft(r.getWidth() / 4);
        for (int i = 0; i < 4; i++)
        {
            bands[i].setBounds(stamp);
            stamp.translate(stamp.getWidth(), 0);
        }

        // bottom bar
        stamp = bottom.removeFromLeft(bottom.getWidth() / 3);
        drySlider.setBounds(stamp);
        stamp.translate(stamp.getWidth(), 0);
        wetSlider.setBounds(stamp);
        stamp.translate(stamp.getWidth(), 0);
        releaseSlider.setBounds(stamp);
    }

private:
    BandModAudioProcessor& processor;
    BandMod& bm;

    SliderAndLabel lowFreqSlider{ "LowFreq" },
        midFreqSlider{ "MidFreq" },
        highFreqSlider{ "HighFreq" },
        drySlider{ "Dry" },
        wetSlider{ "Wet" },
        releaseSlider{ "Release" };
    SensitiveSlider lowOrderSlider,
        midOrderSlider,
        highOrderSlider;

    myButton modeButton;

    std::unique_ptr<SliderAttachment> lowFreqAttachment,
        midFreqAttachment,
        highFreqAttachment,
        lowOrderAttachment,
        midOrderAttachment,
        highOrderAttachment,
        dryAttachment,
        wetAttachment,
        releaseAttachment;
        
    std::unique_ptr<ButtonAttachment> modeAttachment;
    BandComponent bands[4];

    BandGraphComponent bandGraphComponent;

    PopupMessage warningMessage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandModComponent)
};


