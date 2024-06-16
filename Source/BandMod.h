/*
  ==============================================================================

    BandMod.h
    Created: 13 Jun 2024 12:04:54pm
    Author:  noone

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "pitchTracker.h"
#include "CircularBuffer.h"

enum FilterNames { LP1 = 0, HP1 = 1, LP2 = 2, HP2 = 3, LP3 = 4, HP3 = 5};

class BandMod {
public:
    BandMod() : 
        preGain{ 1,1,1,1 }, 
        postGain{ 1,1,1,1 }, 
        fmAmt{ 0,0,0,0 }, 
        fmPitch{0,0,0,0}, 
        feedbackAmt{0,0,0,0},
        feedbackDelay{0,0,0,0},
        targetfeedbackDelay{ 0,0,0,0 },
        phase(0)
    {
        //assuming 48k samplerate for now bcz whatever
        filters[LP1].setType(dsp::LinkwitzRileyFilterType::lowpass);
        filters[LP1].setCutoffFrequency(1000);
        filters[HP1].setType(dsp::LinkwitzRileyFilterType::highpass);
        filters[HP1].setCutoffFrequency(1000);
        filters[LP2].setType(dsp::LinkwitzRileyFilterType::lowpass);
        filters[LP2].setCutoffFrequency(200);
        filters[HP2].setType(dsp::LinkwitzRileyFilterType::highpass);
        filters[HP2].setCutoffFrequency(200);
        filters[LP3].setType(dsp::LinkwitzRileyFilterType::lowpass);
        filters[LP3].setCutoffFrequency(5000);
        filters[HP3].setType(dsp::LinkwitzRileyFilterType::highpass);
        filters[HP3].setCutoffFrequency(5000);

    };

    void BandMod::prepare(juce::dsp::ProcessSpec& s);

    float process(float s);

    void setpre(int band, float p) {
        preGain[band] = p;
    }

    void setpost(int band, float p) {
            postGain[band] = p;
    }

    void setfmAmt(int band, float p) {
        fmAmt[band] = p;
    }

    void setfeedbackAmt(int band, float p) {
        feedbackAmt[band] = p;
    }

    void setfmPitch(int band, float p)
    {
        fmPitch[band] = p;
    }

    void setfeedbackDelay(int band, float p){
        targetfeedbackDelay[band] = p;
    }

private:

    pitchTracker pt;
    float phase;
    float preGain[4], postGain[4], fmAmt[4], fmPitch[4], feedbackDelay[4], targetfeedbackDelay[4], feedbackAmt[4];

    CircularBuffer fmBuffers[4];
    CircularBuffer delayBuffers[4];

    juce::dsp::IIR::Filter<float> hp[4];
    dsp::LinkwitzRileyFilter<float> filters[6];
    const static int numFilters = 6;

    void update_feedback_times()
    {
        const float smooth = .9999;
        for (int i = 0; i < 4; i++)
        {
            feedbackDelay[i] = smooth * feedbackDelay[i] + (1 - smooth) * targetfeedbackDelay[i];
        }
    }

    const float transpose_amts[9] = { .125, .25, .5, .75, 1, 2, 3, 4, 8 };
};