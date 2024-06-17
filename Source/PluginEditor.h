/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BandModComponent.h"

//==============================================================================
/**
*/
class BandModAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BandModAudioProcessorEditor (BandModAudioProcessor&, juce::AudioProcessorValueTreeState& , BandMod&);
    ~BandModAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BandModAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    BandModComponent BMC;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandModAudioProcessorEditor)
};
