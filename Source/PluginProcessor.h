/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandMod.h"

//==============================================================================
/**
*/
class BandModAudioProcessor  : public juce::AudioProcessor
{
public:
    enum
    {
        fftOrder = 12,             // [1]
        fftSize = 1 << fftOrder,  // [2]
        scopeSize = 512             // [3]
    };

    //==============================================================================
    BandModAudioProcessor();
    ~BandModAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    AudioProcessorValueTreeState apvts;
    bool nextFFTBlockReady = false;

    std::atomic<float> fftData[2 * fftSize];
    bool showWarning;

private:
    juce::dsp::Limiter<float> limiter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> hp;

    float fifo[fftSize];                           // [6]
    int fifoIndex = 0;                              // [8]

    BandMod bm[2];

    // params
    std::atomic<float> 
        *n_fmAmt[4],
        *n_preGain[4],
        *n_postGain[4],
        *n_feedBack[4],
        *n_fmPitch[4],
        *n_delaytime[4],
        *n_muteBand[4],
        *n_lowFreq,
        *n_midFreq,
        *n_highFreq,
        *n_lowOrder,
        *n_midOrder,
        *n_highOrder,
        *n_mode,
        *n_wet,
        *n_dry,
        *n_release,
        *n_muteOnStartup;

    void pushNextSampleIntoFifo(float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next frame should now be rendered..
        if (fifoIndex == fftSize)               // [11]
        {
            if (!nextFFTBlockReady)            // [12]
            {
                juce::zeromem(fftData, sizeof(fftData));
                memcpy(fftData, fifo, sizeof(fifo));
                nextFFTBlockReady = true;
            }

            fifoIndex = 0;
        }

        fifo[fifoIndex++] = sample;             // [12]
    }


    

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandModAudioProcessor)
};
