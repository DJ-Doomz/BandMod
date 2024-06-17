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

// helper class

/*
TODO:
add options to change order (and maybe make the phases actually correct)
*/
class HigherOrderLRF
{
public:
    HigherOrderLRF() {};

    ~HigherOrderLRF() {};

    void setType(juce::dsp::LinkwitzRileyFilterType t)
    {
        for (int i = 0; i < 3; i++)
        {
            filters[i].setType(t);
        }
    }

    void setCutoffFrequency(float f)
    {
        for (int i = 0; i < 3; i++)
        {
            filters[i].setCutoffFrequency(f);
        }
    }

    void prepare(const juce::dsp::ProcessSpec& ps)
    {
        for (int i = 0; i < 3; i++)
        {
            filters[i].prepare(ps);
        }
    }

    float processSample(float s)
    {
        for (int i = 0; i < 3; i++)
        {
            s = filters[i].processSample(0, s);
        }
        return s;
    }

private:
    dsp::LinkwitzRileyFilter<float> filters[3];
};


class BandMod {
public:
    BandMod() :
        preGain{ 1,1,1,1 },
        postGain{ 1,1,1,1 },
        targetfmAmt{ 0,0,0,0 },
        fmAmt{ 0,0,0,0 }, 
        fmPitch{0,0,0,0}, 
        feedbackAmt{0,0,0,0},
        feedbackDelay{0,0,0,0},
        targetfeedbackDelay{ 0,0,0,0 },
        phase(0),
        bandFreqs{ 200, 1000, 5000 }
    {
        //assuming 48k samplerate for now bcz whatever

        filters[LP1].setType(dsp::LinkwitzRileyFilterType::lowpass);
        filters[HP1].setType(dsp::LinkwitzRileyFilterType::highpass);
        filters[LP2].setType(dsp::LinkwitzRileyFilterType::lowpass);
        filters[HP2].setType(dsp::LinkwitzRileyFilterType::highpass);
        filters[LP3].setType(dsp::LinkwitzRileyFilterType::lowpass);
        filters[HP3].setType(dsp::LinkwitzRileyFilterType::highpass);

        update_band_freqs();
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
        targetfmAmt[band] = p;
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

    void setBandFreq(int band, int f)
    {
        bandFreqs[band] = f;
        update_band_freqs();
    }

private:
    pitchTracker pt;
    float phase;
    float preGain[4], postGain[4], fmAmt[4], targetfmAmt[4], fmPitch[4], feedbackDelay[4], targetfeedbackDelay[4], feedbackAmt[4];

    CircularBuffer fmBuffers[4];
    CircularBuffer delayBuffers[4];

    int bandFreqs[3];

    juce::dsp::IIR::Filter<float> hp[4];
    
    HigherOrderLRF filters[6];

    const static int numFilters = 6;

    float smoothit(float x, float targetx, float smooth)
    {
        return smooth * x + (1 - smooth) * targetx;
    }

    void update_params()
    {
        for (int i = 0; i < 4; i++)
        {
            feedbackDelay[i] = smoothit(feedbackDelay[i], targetfeedbackDelay[i], .9999);
            //feedbackDelay[i] = smooth * feedbackDelay[i] + (1 - smooth) * targetfeedbackDelay[i];
            fmAmt[i] = smoothit(fmAmt[i], targetfmAmt[i], .999);
        }
    }

    void update_band_freqs()
    {
        for (int i = 0; i < 3; i++)
        {
            filters[LP1].setCutoffFrequency(bandFreqs[1]);
            filters[HP1].setCutoffFrequency(bandFreqs[1]);
            filters[LP2].setCutoffFrequency(bandFreqs[0]);
            filters[HP2].setCutoffFrequency(bandFreqs[0]);
            filters[LP3].setCutoffFrequency(bandFreqs[2]);
            filters[HP3].setCutoffFrequency(bandFreqs[2]);
        }
    }

    const float transpose_amts[9] = { .125, .25, .5, .75, 1, 2, 3, 4, 8 };
};