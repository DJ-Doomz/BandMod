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
        layout.add(std::make_unique<juce::AudioParameterFloat>(prename, prename, juce::NormalisableRange<float>(0, 1000, 0), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(postname, postname, juce::NormalisableRange<float>(0, 2, 0), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(feedbkname, feedbkname, juce::NormalisableRange<float>(0, 1.1, 0), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(fmpitchname, fmpitchname, juce::NormalisableRange<float>(0, 1, 0), 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(delaytimename, delaytimename, juce::NormalisableRange<float>(0, 1, 0), 0.f));
    }
    // band frequencies
    layout.add(std::make_unique < juce::AudioParameterInt>("LowFreq", "LowFreq", 0, 400, 200));
    layout.add(std::make_unique < juce::AudioParameterInt>("MidFreq", "MidFreq", 400, 3000, 1000));
    layout.add(std::make_unique < juce::AudioParameterInt>("HighFreq", "HighFreq", 3000, 20000, 5000));

    //band orders
    layout.add(std::make_unique < juce::AudioParameterInt>("LowOrder", "LowOrder", 1, HigherOrderLRF::MAX_ORDER, 1));
    layout.add(std::make_unique < juce::AudioParameterInt>("MidOrder", "MidOrder", 1, HigherOrderLRF::MAX_ORDER, 1));
    layout.add(std::make_unique < juce::AudioParameterInt>("HighOrder", "HighOrder", 1, HigherOrderLRF::MAX_ORDER, 1));

    layout.add(std::make_unique < juce::AudioParameterBool>("Mode", "Mode", false));

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
    , apvts(*this, nullptr, "STATE", createParameterLayout()),
    fifo{ 0 },
    fftData{ 0 }
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
        n_fmAmt[i] = apvts.getRawParameterValue(fmname);
        n_preGain[i] = apvts.getRawParameterValue(prename);
        n_postGain[i] = apvts.getRawParameterValue(postname);
        n_feedBack[i] = apvts.getRawParameterValue(feedbkname);
        n_fmPitch[i] = apvts.getRawParameterValue(fmpitchname);
        n_delaytime[i] = apvts.getRawParameterValue(delaytimename);
    }

    n_lowFreq = apvts.getRawParameterValue("LowFreq");
    n_midFreq = apvts.getRawParameterValue("MidFreq");
    n_highFreq = apvts.getRawParameterValue("HighFreq");
    n_lowOrder = apvts.getRawParameterValue("LowOrder");
    n_midOrder = apvts.getRawParameterValue("MidOrder");
    n_highOrder = apvts.getRawParameterValue("HighOrder");
    n_mode = apvts.getRawParameterValue("Mode");
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
    return "empty";
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
    spec_stereo.numChannels = getTotalNumOutputChannels();
    spec_stereo.sampleRate = sampleRate;

    // set up master highpass

    hp.prepare(spec_stereo);
    *hp.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20);
    
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
            bm[c].setfmAmt(i, *n_fmAmt[i]);
            bm[c].setpre(i, *n_preGain[i]);
            bm[c].setpost(i, *n_postGain[i]);
            bm[c].setfeedbackAmt(i, *n_feedBack[i]);
            bm[c].setfmPitch(i, *n_fmPitch[i]);
            bm[c].setfeedbackDelay(i, *n_delaytime[i]);
        }
        bm[c].setBandFreq(0, *n_lowFreq);
        bm[c].setBandFreq(1, *n_midFreq);
        bm[c].setBandFreq(2, *n_highFreq);

        bm[c].setOrder(0, *n_lowOrder);
        bm[c].setOrder(1, *n_midOrder);
        bm[c].setOrder(2, *n_highOrder);

        bm[c].setFeedbackMode(*n_mode);
    }
    ///// done with params

    for (auto j = 0; j < buffer.getNumSamples(); ++j)
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* ptrOut = buffer.getWritePointer(channel);
            auto* ptrIn = buffer.getReadPointer(channel);
            ptrOut[j] = bm[channel].process(ptrIn[j]);
            if (channel == 0)
                pushNextSampleIntoFifo(ptrIn[j]);
        }
    }

    // apply limiting
    juce::dsp::AudioBlock<float> block(buffer);

    dsp::ProcessContextReplacing<float> ctx(block);
    //doing this high-pass causes it to crash for some reason????
    hp.process(ctx);
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
    return new BandModAudioProcessorEditor (*this, apvts, bm[0]);
}

//==============================================================================
void BandModAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BandModAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BandModAudioProcessor();
}
