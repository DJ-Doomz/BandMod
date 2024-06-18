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
class BandGraphComponent  : public juce::Component, juce::Timer
{
public:
    BandGraphComponent(BandModAudioProcessor& p, BandMod& b) 
        :processor(p),
        bm(b),
        forwardFFT(BandModAudioProcessor::fftOrder),
        window(BandModAudioProcessor::fftSize, juce::dsp::WindowingFunction<float>::hann),
        scopeData{0}
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        startTimerHz(60);
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
        auto h = lb.getHeight();

        const double outputMin = responseArea.getBottom();
        const double outputMax = responseArea.getY();

        auto sampleRate = processor.getSampleRate();

        auto pitchx = responseArea.getX() + mapFromLog10(bm.getTrackedPitch(), 20.f, 20000.f) * w;

        g.setColour(Colours::white);
        g.drawLine(pitchx, lb.getBottom(), pitchx, lb.getY());

        for (int i = 1; i < BandModAudioProcessor::scopeSize; ++i)
        {
            g.drawLine({ (float)juce::jmap(i - 1, 0, BandModAudioProcessor::scopeSize - 1, 0, w),
                                  juce::jmap(scopeData[i - 1], 0.0f, 1.0f, (float)h, 0.0f),
                          (float)juce::jmap(i,     0, BandModAudioProcessor::scopeSize - 1, 0, w),
                                  juce::jmap(scopeData[i],     0.0f, 1.0f, (float)h, 0.0f) });

        }

        repaint();
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

    void drawNextFrameOfSpectrum()
    {
        // first apply a windowing function to our data
        // make a local copy of the data
        
        juce::zeromem(fftData, sizeof(fftData));
        memcpy(fftData, processor.fftData, sizeof(processor.fftData));

        window.multiplyWithWindowingTable(fftData, BandModAudioProcessor::fftSize);       // [1]

        // then render our FFT data..
        forwardFFT.performFrequencyOnlyForwardTransform(fftData);  // [2]

        auto mindB = -100.0f;
        auto maxdB = 0.0f;

        for (int i = 0; i < BandModAudioProcessor::scopeSize; ++i)                         // [3]
        {
            auto skewedProportionX = mapToLog10((float)i / (float)BandModAudioProcessor::scopeSize, 20.f, 20000.f) / ((float)processor.getSampleRate() / 2.f);
            auto fftDataIndex = juce::jlimit(0, BandModAudioProcessor::fftSize / 2, (int)(skewedProportionX * (float)BandModAudioProcessor::fftSize * 0.5f));
            auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(fftData[fftDataIndex])
                - juce::Decibels::gainToDecibels((float)BandModAudioProcessor::fftSize)),
                mindB, maxdB, 0.0f, 1.0f);

            scopeData[i] = level;                                   // [4]
        }
    }

    void timerCallback() override
    {
        if (processor.nextFFTBlockReady)
        {
            drawNextFrameOfSpectrum();
            processor.nextFFTBlockReady = false;
            repaint();
        }
    }

private:
    BandModAudioProcessor& processor;
    BandMod& bm;

    float fftData[2 * BandModAudioProcessor::fftSize];

    juce::dsp::FFT forwardFFT;                      // [4]
    juce::dsp::WindowingFunction<float> window;     // [5]
    float scopeData[BandModAudioProcessor::scopeSize];
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandGraphComponent)
};
