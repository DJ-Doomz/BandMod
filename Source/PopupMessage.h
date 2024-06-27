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
        processor(p),
        fade(1),
        fadingOut(false),
        animStyle(random.nextInt())
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        addAndMakeVisible(closeButton);
        closeButton.setButtonText("ok whatever");
        closeButton.onClick = [this] {closeButton.setVisible(false); fadingOut = true; processor.showWarning = false; };
        triangle.startNewSubPath(-.55, .3);
        triangle.lineTo(.55, .3);
        triangle.lineTo(0, -.6);
        triangle.closeSubPath();
        triangle = triangle.createPathWithRoundedCorners(.2);
    }

    ~PopupMessage() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        auto lb = getLocalBounds();
        auto w = lb.getWidth();
        auto h = lb.getHeight();

        if (fadingOut)
        {
            fade -= .05;
            switch (animStyle%2)    // pointless features are fun
            {
            case 0:
                g.excludeClipRegion(lb.expanded(-.5 * w * fade));
                break;
            case 1:
                g.excludeClipRegion(lb.withTop(h * fade));
                break;
            }
            
            if (fade <= 0)
            {
                fadingOut = false;
                setVisible(false);
            }
        }
        else
        {
            if (fade < 1)
                fade += .05;
        }
        fade = jlimit(0.f, 1.f, fade);

        g.beginTransparencyLayer(fade);

        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(lb.toFloat(), 6);   // draw an outline around the component
        // title bar
        auto tb = lb.removeFromTop(20);
        g.setColour(juce::Colour::fromFloatRGBA(.0, .04, .1, 1.));
        g.fillRoundedRectangle(tb.toFloat(), 6);
        g.setColour(juce::Colours::white);
        g.setFont(16);
        g.drawText("check it!!!!", tb, Justification::centred, false);

        auto c1 = lb.withSize(w / 6, w / 6);
        auto c2 = c1.withRightX(w);
        g.setColour(juce::Colours::white);
        g.setColour(Colours::yellow);
        AffineTransform aft;
        const int triSize = 34;
        g.setFont(triSize);

        aft = aft.scaled(triSize+7);
        auto aft1 = aft.translated(c1.getCentre());
        auto aft2 = aft.translated(c2.getCentre());
        
        g.fillPath(triangle, aft1);
        g.fillPath(triangle, aft2);

        g.setColour(Colours::black);
        g.drawText("!", c1, Justification::centred, false);
        g.drawText("!", c2, Justification::centred, false);
        
        g.setColour (juce::Colours::red);
        // warning symbols in top corners
        g.setFont(34);
        g.drawText ("noise warning", lb.removeFromTop(h/6),
                    juce::Justification::centred, true);   // draw some placeholder text
        AttributedString ats("This plugin makes very loud noises.\n\nUse the mute on startup feature to prevent sudden volume spikes when loading projects.\n\nPlease experiment at safe volume levels.");
        ats.setColour(Colours::white);
        ats.setFont(20);
        ats.draw(g, lb.toFloat().expanded(-20, 0));
        
        g.endTransparencyLayer();

        repaint();
    }

    void resized() override
    {
        auto lb = getLocalBounds().removeFromBottom(200);
        auto bb = Rectangle<int>(0, 0, 100, 50).withCentre(lb.getCentre());
        closeButton.setBounds(bb);
    }

private:
    BandModAudioProcessor& processor;
    juce::TextButton closeButton;
    Random random;
    int animStyle;

    Path triangle;

    float fade;
    bool fadingOut;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupMessage)
};
