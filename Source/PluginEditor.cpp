/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
BandModAudioProcessorEditor::BandModAudioProcessorEditor (BandModAudioProcessor& p, juce::AudioProcessorValueTreeState& vts, BandMod& bm)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts), BMC(p, bm)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (700, 700);
    setResizable(true, true);
    addAndMakeVisible(BMC);
    setResizeLimits(300, 300, 2000, 2000);
    BMC.addAttachment(vts);
    LookAndFeel::setDefaultLookAndFeel(&myLnF);

    startTimerHz(120);
}

BandModAudioProcessorEditor::~BandModAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void BandModAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void BandModAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto r = getLocalBounds();
    BMC.setBounds(r);
}
