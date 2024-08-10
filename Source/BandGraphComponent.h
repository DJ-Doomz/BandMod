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
    BandGraphComponent(BandModAudioProcessor& p, BandMod& b)
        :processor(p),
        bm(b),
        forwardFFT(BandModAudioProcessor::fftOrder),
        window(BandModAudioProcessor::fftSize, juce::dsp::WindowingFunction<float>::hann),
        scopeData{ 0 },
        nextScopeData{ 0 },
        buffer(Image::PixelFormat::ARGB, 700, 700/3, true),
        lowColor(0.f, .3f, .3f, 1.f),
        mid1Color(.25, .3f, .3f, 1.f),
        mid2Color(.5, .3f, .3f, 1.f),
        highColor(.75, .3f, .3f, 1.f)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        setOpaque(true);
    }

    ~BandGraphComponent() override
    {
    }


    void paint (juce::Graphics& m) override
    {
        Rectangle<float> shrunk = getLocalBounds().toFloat().expanded(2, 2).translated(0, 0);
        Graphics g(buffer);
        buffer.multiplyAllAlphas(.4);
        g.drawImage(buffer, shrunk);
        // turn down the alpha for funnies
        g.fillAll(Colour(0.f, 0.f, 0.f, .4f));

        auto lb = getLocalBounds();
        auto responseArea = lb;
        auto w = lb.getWidth();
        auto h = lb.getHeight();

        // draw background image
        m.fillCheckerBoard(lb.toFloat(), h / 6, h / 6, Colour::fromFloatRGBA(0.1, 0.1, 0.1, 1), Colours::black);

        const double outputMin = responseArea.getBottom();
        const double outputMax = responseArea.getY();

        // draw tracked pitch
        auto pitchx = mapFromLog10(bm.getTrackedPitch(), 20.f, 20000.f) * w;
        if (pitchx < 0)pitchx = 0;
        pitchx += responseArea.getX();
        
        ColourGradient pitchGrad(Colours::transparentWhite, 0, lb.getBottom(), Colours::transparentWhite, 0, lb.getY(), false);
        pitchGrad.addColour(.5, Colours::white);
        g.setGradientFill(pitchGrad);
        g.drawLine(pitchx, lb.getBottom(), pitchx, lb.getY());

        // draw response curves
        g.setColour(Colours::white);
        drawResponseCurves(g);
        
        // draw spectrogram
        drawNextFrameOfSpectrum(g);
        //g.setColour(Colour(hue, 1.f, 1.f, 1.f));
        //float thue = hue;
        /*
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, .6f));
        for (int i = 1; i < BandModAudioProcessor::scopeSize; ++i)
        {
            g.drawLine({ (float)jmap(i - 1, 0, BandModAudioProcessor::scopeSize - 1, 0, w),
                                  jmap(scopeData[i - 1], 0.0f, 1.0f, (float)h, 0.0f),
                          (float)jmap(i,     0, BandModAudioProcessor::scopeSize - 1, 0, w),
                                  jmap(scopeData[i],     0.0f, 1.0f, (float)h, 0.0f) }, 2);
        }
        */

        //double trash;
        //hue = modf(thue + .003, &trash);

        m.drawImageAt(buffer, 0, 0);
        // add vignette
        m.setGradientFill(hvignette);
        m.fillAll();
        m.setGradientFill(vignette);
        m.fillAll();
        repaint();
    }

    void resized() override
    {
        auto lb = getLocalBounds();
        auto w = lb.getWidth();
        auto h = lb.getHeight();
        hvignette = ColourGradient(juce::Colours::black, 0, 0, juce::Colours::black, w, 0, false);
        hvignette.addColour(.1, juce::Colour::fromFloatRGBA(0, 0, 0, 0));
        hvignette.addColour(.9, juce::Colour::fromFloatRGBA(0, 0, 0, 0));
        vignette = ColourGradient(juce::Colours::black, 0, 0, juce::Colours::black, 0, h, false);
        vignette.addColour(.1, juce::Colour::fromFloatRGBA(0, 0, 0, 0));
        vignette.addColour(.9, juce::Colour::fromFloatRGBA(0, 0, 0, 0));
        // This method is where you should set the bounds of any child
        // components that your component contains..
        
        buffer = buffer.rescaled(lb.getWidth(), lb.getHeight());
    }

    

private:
    BandModAudioProcessor& processor;
    BandMod& bm;

    const float mindB = -100.0f;
    const float maxdB = 0.0f;
    // FFT stuff
    float fftData[2 * BandModAudioProcessor::fftSize];

    juce::dsp::FFT forwardFFT;                      // [4]
    juce::dsp::WindowingFunction<float> window;     // [5]
    float scopeData[BandModAudioProcessor::scopeSize];
    float nextScopeData[BandModAudioProcessor::scopeSize];
    Path spectrumPath;

    // drawing stuff
    Image buffer;

    juce::ColourGradient hvignette, vignette;

    juce::Colour lowColor, mid1Color, mid2Color, highColor;
    
    void drawResponseCurves(Graphics& g)
    {
        const float margin = 0.;
        auto lb = getLocalBounds();
        float w = lb.getWidth();
        float h = lb.getHeight();
        auto centery = lb.getHeight() / 2;
        auto by = lb.getHeight();
        float lowf = bm.getLowFreq();
        float midf = bm.getMidFreq();
        float highf = bm.getHighFreq();
        float lowx, midx, highx, endlowx;
        
        lowx = jlimit(0.f, w, mapFromLog10(lowf, 20.f, 20000.f) * w);
        midx = jlimit(0.f, w, mapFromLog10(midf, 20.f, 20000.f) * w);
        highx = jlimit(0.f, w, mapFromLog10(highf, 20.f, 20000.f) * w);

        //just approximate curves for now bcz who cares
        
        auto lowGain = centery * (2.0 - bm.getBandGain(0));
        auto mid1Gain = centery * (2.0 - bm.getBandGain(1));
        auto mid2Gain = centery * (2.0 - bm.getBandGain(2));
        auto highGain = centery * (2.0 - bm.getBandGain(3));
        // draw lowpass response
        
        ColourGradient lowGrad(lowColor.withAlpha(0.4f), 0, 0, lowColor.withAlpha(0.f), 0, by, false);
        ColourGradient mid1Grad(mid1Color.withAlpha(0.4f), 0, 0, mid1Color.withAlpha(0.f), 0, by, false);
        ColourGradient mid2Grad(mid2Color.withAlpha(0.4f), 0, 0, mid2Color.withAlpha(0.f), 0, by, false);
        ColourGradient highGrad(highColor.withAlpha(0.4f), 0, 0, highColor.withAlpha(0.f), 0, by, false);

        float cornerSize = w*.15;
        Rectangle<float> lowRect(-cornerSize, lowGain, 2 * cornerSize + lowx, h * 2);
        Rectangle<float> mid1Rect(-cornerSize + lowx, mid1Gain, 2 * cornerSize + (midx - lowx), h * 2);
        Rectangle<float> mid2Rect(-cornerSize + midx, mid2Gain, 2 * cornerSize + (highx - midx), h * 2);
        Rectangle<float> highRect(-cornerSize + highx, highGain, 2 * cornerSize + (w - highx), h * 2);
        g.setColour(lowColor);
        g.drawRoundedRectangle(lowRect, cornerSize, 2);
        g.setGradientFill(lowGrad);
        g.fillRoundedRectangle(lowRect, cornerSize);

        g.setColour(mid1Color);
        g.drawRoundedRectangle(mid1Rect, cornerSize, 2);
        g.setGradientFill(mid1Grad);
        g.fillRoundedRectangle(mid1Rect, cornerSize);

        g.setColour(mid2Color);
        g.drawRoundedRectangle(mid2Rect, cornerSize, 2);
        g.setGradientFill(mid2Grad);
        g.fillRoundedRectangle(mid2Rect, cornerSize);

        g.setColour(highColor);
        g.drawRoundedRectangle(highRect, cornerSize, 2);
        g.setGradientFill(highGrad);
        g.fillRoundedRectangle(highRect, cornerSize);
    }

    inline float indexToX(float index, float minFreq) const
    {
        const auto freq = (processor.getSampleRate() * index) / forwardFFT.getSize();
        return (freq > 0.01f) ? std::log(freq / minFreq) / std::log(2.0f) : 0.0f;
    }

    inline float binToY(float bin, Rectangle<float> bounds) const
    {
        const float infinity = -80.0f;
        return juce::jmap(juce::Decibels::gainToDecibels(bin, infinity) - Decibels::gainToDecibels((float)BandModAudioProcessor::fftSize),
            infinity, 0.0f, 0.f, 1.0f);
    }


    void drawNextFrameOfSpectrum(Graphics& g)
    {
        Rectangle<float> lb = getLocalBounds().toFloat();
        float bottom = lb.getBottom();
        float top = lb.getY();
        const auto factor = lb.getWidth() / 10.0f;

        const float smoothing = 0.7;
        // do smooth towards next scope data
        for (int i = 0; i < BandModAudioProcessor::scopeSize; ++i)                         // [3]
        {
            scopeData[i] = smoothing * scopeData[i] + (1 - smoothing) * nextScopeData[i];
        }

        // first apply a windowing function to our data
        // make a local copy of the data
        if (processor.nextFFTBlockReady)
        {
            juce::zeromem(fftData, sizeof(fftData));
            memcpy(fftData, processor.fftData, sizeof(processor.fftData));
            processor.nextFFTBlockReady = false;

            window.multiplyWithWindowingTable(fftData, BandModAudioProcessor::fftSize);       // [1]

            // then render our FFT data..
            forwardFFT.performFrequencyOnlyForwardTransform(fftData);  // [2]
            
            //TODO: redo this in a not-stupid way
            for (int i = 0; i < BandModAudioProcessor::scopeSize; ++i)                         // [3]
            {
                nextScopeData[i] = binToY(fftData[i], lb);
            }
        }

        // actual drawing stuff
        spectrumPath.clear();
        spectrumPath.preallocateSpace(8 + BandModAudioProcessor::scopeSize * 3);
        spectrumPath.startNewSubPath(lb.getX() + factor * indexToX(0, 20.f), jmap(scopeData[0], bottom, top));
        for (int i = 0; i < BandModAudioProcessor::scopeSize; ++i)                         // [3]
        {
            spectrumPath.lineTo(lb.getX() + factor * indexToX(float(i), 20.f), jmap(scopeData[i], bottom, top));
        }
        g.setColour(Colour::fromFloatRGBA(1.f, 1.f, 1.f, .6f));
        g.strokePath(spectrumPath.createPathWithRoundedCorners(5), PathStrokeType(2.0));
    }
    
    
    // helper functions (should probably make my own little library of these at one point)
    float smoothit(float x, float targetx, float smooth)
    {
        return smooth * x + (1 - smooth) * targetx;
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandGraphComponent)
};
