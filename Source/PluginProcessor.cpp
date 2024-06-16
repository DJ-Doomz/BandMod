/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// programmatically create paramertererers
AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    for (int i = 0; i < 4; i++)
    {
        auto stri = String(i);
        auto fmname = String("fmAmt") + stri;
        auto prename = String("preGain") + stri;
        auto postname = String("postGain") + stri;
        auto feedbkname = String("feedBack") + stri;
        auto fmpitchname = String("fmPitch") + stri;
        auto delaytimename = String("delaytime") + stri;
        layout.add(std::make_unique<juce::AudioParameterFloat>(fmname, fmname, juce::NormalisableRange<float>(0, 1000, 0), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(prename, prename, juce::NormalisableRange<float>(0, 100, 0), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(postname, postname, juce::NormalisableRange<float>(0, 8, 0), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(feedbkname, feedbkname, juce::NormalisableRange<float>(0, 1.1, 0), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(fmpitchname, fmpitchname, juce::NormalisableRange<float>(0, 1, 0), 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(delaytimename, delaytimename, juce::NormalisableRange<float>(0, 1, 0), 0.f));
    }


    return layout;
}

//==============================================================================
BandModAudioProcessor::BandModAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , apvts(*this, nullptr, "STATE", createParameterLayout())
{
}

BandModAudioProcessor::~BandModAudioProcessor()
{
}

//==============================================================================
const juce::String BandModAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BandModAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BandModAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BandModAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BandModAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BandModAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BandModAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BandModAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BandModAudioProcessor::getProgramName (int index)
{
    return {};
}

void BandModAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BandModAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;

    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    bm[0].prepare(spec);
    bm[1].prepare(spec);

    juce::dsp::ProcessSpec spec_stereo;

    spec_stereo.maximumBlockSize = samplesPerBlock;
    spec_stereo.numChannels = 2;
    spec_stereo.sampleRate = sampleRate;

    // set up master highpass

    hp.prepare(spec_stereo);
    hp.state = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20);
    
    // set up master limiter

    limiter.prepare(spec_stereo);
    limiter.setThreshold(0);
    limiter.setRelease(10);
}

void BandModAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BandModAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BandModAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    //update params
    for (int c = 0; c < 2; c++)
    {
        for (int i = 0; i < 4; i++)
        {
            auto stri = String(i);
            auto fmname = String("fmAmt") + stri;
            auto prename = String("preGain") + stri;
            auto postname = String("postGain") + stri;
            auto feedbkname = String("feedBack") + stri;
            auto fmpitchname = String("fmPitch") + stri;
            auto delaytimename = String("delaytime") + stri;
            bm[c].setfmAmt(i, *apvts.getRawParameterValue(fmname));
            bm[c].setpre(i, *apvts.getRawParameterValue(prename));
            bm[c].setpost(i, *apvts.getRawParameterValue(postname));
            bm[c].setfeedbackAmt(i, *apvts.getRawParameterValue(feedbkname));
            bm[c].setfmPitch(i, *apvts.getRawParameterValue(fmpitchname));
            bm[c].setfeedbackDelay(i, *apvts.getRawParameterValue(delaytimename));
        }
    }

    for (auto j = 0; j < buffer.getNumSamples(); ++j)
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* ptrOut = buffer.getWritePointer(channel);
            auto* ptrIn = buffer.getReadPointer(channel);
            ptrOut[j] = bm[channel].process(ptrIn[j]);
        }
    }

    // apply limiting
    juce::dsp::AudioBlock<float> block(buffer);

    dsp::ProcessContextReplacing<float> ctx(block);
    //hp.process(ctx);
    limiter.process(ctx);
}

//==============================================================================
bool BandModAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BandModAudioProcessor::createEditor()
{
    //return new GenericAudioProcessorEditor(*this);
    return new BandModAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void BandModAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BandModAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BandModAudioProcessor();
}