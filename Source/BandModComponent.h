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
    BandComponent() :
        muteButton("MUTE")
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

        postSlider.setSliderStyle(Slider::LinearVertical);
        postSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);

        feedbackSlider.getSlider().getProperties().set("bipolar", true);

        muteButton.setToggleable(true);
        muteButton.setClickingTogglesState(true);
        DrawingFunction muteDrawing = [&](Graphics& g, bool highlighted, bool down, juce::Rectangle<int> lb) {
            // just a basic circle to indicate on/off
            Rectangle<float> p(0, 0, 10, 10);
            p.setCentre(lb.getCentre().toFloat());
            if (down)
            {
                g.setColour(Colours::black);
            }
            else
            {
                g.setColour(Colours::lightgrey);
            }
            g.fillEllipse(p);
            g.setColour(Colours::white);
            g.drawEllipse(p, 1);
        };
        muteButton.setDrawingFunction(muteDrawing);
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
        const int margin = 4;
        auto lb = getLocalBounds();
        auto h = lb.getHeight();
        auto w = lb.getWidth();
        
        auto gainArea = lb.removeFromTop(h / 2).expanded(-5, -5);
        preSlider.setBounds(gainArea.removeFromLeft(w / 2).reduced(margin));
        postSlider.setBounds(gainArea.removeFromLeft(w / 4).reduced(margin));
        bandVu.setBounds(gainArea.removeFromTop(gainArea.getHeight()*.75).expanded(-6, -4).reduced(margin));
        muteButton.setBounds(gainArea.reduced(margin));

        auto fmArea = lb.removeFromTop(h / 4);
        fmSlider.setBounds(fmArea.removeFromLeft(w / 2).reduced(margin));
        fmPitchSlider.setBounds(fmArea.reduced(margin));

        auto feedArea = lb;
        feedbackSlider.setBounds(feedArea.removeFromLeft(w / 2).reduced(margin));
        delaySlider.setBounds(feedArea.reduced(margin));

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

    myDrawableButton muteButton;
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
        muteStartupButton("Mute on Startup"),
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
        addAndMakeVisible(muteStartupButton);
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
        muteStartupAttachment.reset(new ButtonAttachment(vts, "MuteOnStartup", muteStartupButton.getButton()));

        dryAttachment.reset(new SliderAttachment(vts, "Dry", drySlider.getSlider()));
        wetAttachment.reset(new SliderAttachment(vts, "Wet", wetSlider.getSlider()));
        releaseAttachment.reset(new SliderAttachment(vts, "Release", releaseSlider.getSlider()));
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        auto r = getLocalBounds();
        const int margin = 4;
        auto width = r.getWidth();

        warningMessage.setBounds(r.expanded(-r.getWidth()/6, -r.getHeight() / 6));

        auto bg = r.removeFromTop(r.getHeight() / 3);
        bandGraphComponent.setBounds(bg);

        auto t = r.removeFromTop(100);
        auto w = t.getWidth() / 8;
        auto stamp1 = t.removeFromLeft(w).reduced(margin);
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
        auto stamp = r.removeFromLeft(width / 4);
        for (int i = 0; i < 4; i++)
        {
            bands[i].setBounds(stamp);
            stamp.translate(width / 4, 0);
        }

        // bottom bar
        stamp = bottom.removeFromLeft(width / 4);
        muteStartupButton.setBounds(stamp);
        stamp.translate(width / 4, 0);
        drySlider.setBounds(stamp);
        stamp.translate(width / 4, 0);
        wetSlider.setBounds(stamp);
        stamp.translate(width / 4, 0);
        releaseSlider.setBounds(stamp);
    }

private:
    BandModAudioProcessor& processor;
    BandMod& bm;

    SliderAndLabel lowFreqSlider{ "LOW FREQ" },
        midFreqSlider{ "MID FREQ" },
        highFreqSlider{ "HIGH FREQ" },
        drySlider{ "DRY" },
        wetSlider{ "WET" },
        releaseSlider{ "RELEASE" };
    SensitiveSlider lowOrderSlider,
        midOrderSlider,
        highOrderSlider;

    myButton modeButton,
        muteStartupButton;

    std::unique_ptr<SliderAttachment> lowFreqAttachment,
        midFreqAttachment,
        highFreqAttachment,
        lowOrderAttachment,
        midOrderAttachment,
        highOrderAttachment,
        dryAttachment,
        wetAttachment,
        releaseAttachment;
        
    std::unique_ptr<ButtonAttachment> modeAttachment,
        muteStartupAttachment;
    BandComponent bands[4];

    BandGraphComponent bandGraphComponent;

    PopupMessage warningMessage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandModComponent)
};


