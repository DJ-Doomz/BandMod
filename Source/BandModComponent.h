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
#include "BandVUMeeter.h"

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
        addAndMakeVisible(muteButton);
        addAndMakeVisible(bandVu);

        muteButton.setButtonText("M");
        muteButton.setToggleable(true);
        muteButton.setClickingTogglesState(true);

        postSlider.setSliderStyle(Slider::LinearVertical);
        postSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    }

    ~BandComponent() override
    {
    }

    void paint(juce::Graphics& g) override
    {
        auto lb = getLocalBounds();
        auto h = lb.getHeight();
        auto w = lb.getWidth();
        g.setColour(Colours::black);
        g.fillAll();
        // draw lines around component

        ColourGradient lg(Colours::transparentWhite, 0, 0, Colours::transparentWhite, 0, h, false);
        lg.addColour(0.5, Colours::white);
        g.setGradientFill(lg);
        g.drawLine(w, 0, w, h, 2);

    }

    void resized() override
    {
        auto lb = getLocalBounds();
        auto h = lb.getHeight();
        auto w = lb.getWidth();
        
        auto gainArea = lb.removeFromTop(h / 2).expanded(-5, -5);
        preSlider.setBounds(gainArea.removeFromLeft(w / 2));
        postSlider.setBounds(gainArea.removeFromLeft(w / 4));
        bandVu.setBounds(gainArea.removeFromTop(gainArea.getHeight()*.75).expanded(-6, -4) );
        muteButton.setBounds(gainArea);

        auto fmArea = lb.removeFromTop(h / 4);
        fmSlider.setBounds(fmArea.removeFromLeft(w / 2));
        fmPitchSlider.setBounds(fmArea);

        auto feedArea = lb;
        feedbackSlider.setBounds(feedArea.removeFromLeft(w / 2));
        delaySlider.setBounds(feedArea);

    }

    void addAttachment(juce::AudioProcessorValueTreeState& vts, BandMod& bm, int bandNum)
    {
        auto stri = String(bandNum);
        auto fmname = String("fmAmt") + stri;
        auto prename = String("preGain") + stri;
        auto postname = String("postGain") + stri;
        auto feedbkname = String("feedBack") + stri;
        auto fmpitchname = String("fmPitch") + stri;
        auto mutebandname = String("muteBand") + stri;
        auto delaytimename = String("delaytime") + stri;
        fmAttachment.reset(new SliderAttachment(vts, fmname, fmSlider.getSlider()));
        preAttachment.reset(new SliderAttachment(vts, prename, preSlider.getSlider()));
        postAttachment.reset(new SliderAttachment(vts, postname, postSlider));
        fmPitchAttachment.reset(new SliderAttachment(vts, fmpitchname, fmPitchSlider.getSlider()));
        feedbackAttachment.reset(new SliderAttachment(vts, feedbkname, feedbackSlider.getSlider()));
        delayAttachment.reset(new SliderAttachment(vts, delaytimename, delaySlider.getSlider()));
        muteAttachment.reset(new ButtonAttachment(vts, mutebandname, muteButton));

        // set up vu meeter
        bandVu.setValuetoMonitor(bm.getVu(bandNum));
    }

private:
    SliderAndLabel preSlider{ "DISTORTION" },
        fmSlider{ "FM" },
        fmPitchSlider{ "TRANSPOSE" },
        feedbackSlider{ "FEEDBACK" },
        delaySlider{ "DELAY" };

    Slider postSlider;

    std::unique_ptr<SliderAttachment> preAttachment,
        postAttachment,
        fmAttachment,
        fmPitchAttachment,
        feedbackAttachment,
        delayAttachment;

    BandVUMeeter bandVu;

    TextButton muteButton;
    std::unique_ptr<ButtonAttachment> muteAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandComponent)
};
// TODO: split into more subcomponents maybe
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
        auto lb = getLocalBounds();
        auto w = lb.getWidth();
        auto h = lb.getHeight();
        g.setColour(Colours::black);
        g.fillAll();
        // draw lines n shiz
        
    }

    void addAttachment(juce::AudioProcessorValueTreeState& vts)
    {
        for (int i = 0; i < 4; i++)
        {
            bands[i].addAttachment(vts, bm, i);
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

    SliderAndLabel lowFreqSlider{ "LOWFREQ" },
        midFreqSlider{ "MIDFREQ" },
        highFreqSlider{ "HIGHFREQ" },
        drySlider{ "DRY" },
        wetSlider{ "WET" },
        releaseSlider{ "RELEASE" };
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


