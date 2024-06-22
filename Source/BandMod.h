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
    static const int MAX_ORDER = 4;
    HigherOrderLRF():
    order(1){};

    ~HigherOrderLRF() {};

    void setType(juce::dsp::LinkwitzRileyFilterType t)
    {
        for (int i = 0; i < MAX_ORDER; i++)
        {
            filters[i].setType(t);
        }
    }

    void setCutoffFrequency(float f)
    {
        if (f != filters[0].getCutoffFrequency())
        {
            for (int i = 0; i < MAX_ORDER; i++)
            {
                filters[i].setCutoffFrequency(f);
            }
        }
    }

    void prepare(const juce::dsp::ProcessSpec& ps)
    {
        for (int i = 0; i < MAX_ORDER; i++)
        {
            filters[i].prepare(ps);
        }
    }

    float processSample(float s)
    {
        for (int i = 0; i < order; i++)
        {
            s = filters[i].processSample(0, s);
        }
        return s;
    }

    void setOrder(int o)
    {
        if (o != order)
        {
            order = o;
            for (int i = 0; i < order; i++)
            {
                filters[i].reset();
            }
        }
    }

private:
    dsp::LinkwitzRileyFilter<float> filters[MAX_ORDER];
    int order;
};


class BandMod {
public:
    BandMod() :
        preGain{ 1,1,1,1 },
        targetpreGain{ 1,1,1,1 },
        postGain{ 1,1,1,1 },
        targetpostGain{ 1,1,1,1 },
        targetfmAmt{ 0,0,0,0 },
        fmAmt{ 0,0,0,0 },
        fmPitch{ 0,0,0,0 },
        feedbackAmt{ 0,0,0,0 },
        feedbackDelay{ 0,0,0,0 },
        targetfeedbackDelay{ 0,0,0,0 },
        phase(0),
        d(0),
        sampleRate(48000),
        bandFreqs{ 200, 1000, 5000 },
        feedBackMode(0),
        release(1),
        noiseGate(1),
        startupTime(196000),
        startup(startupTime)
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
        targetpreGain[band] = p;
    }

    void setpost(int band, float p) {
        targetpostGain[band] = p;
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

    void setOrder(int filter, int o)
    {
        if (filter == 0)
        {
            filters[LP2].setOrder(o);
            filters[HP2].setOrder(o);
        }
        else if (filter == 1)
        {
            filters[LP1].setOrder(o);
            filters[HP1].setOrder(o);
        }
        else
        {
            filters[HP3].setOrder(o);
            filters[LP3].setOrder(o);
        }
    }

    void setFeedbackMode(int fb)
    {
        feedBackMode = fb;
    }

    void setRelease(float r)
    {
        release = r;
    }

    // returns estimated pitch in hertz
    float getTrackedPitch()
    {
        return d*sampleRate;
    }

    float getLowFreq()
    {
        return bandFreqs[0];
    }

    float getMidFreq()
    {
        return bandFreqs[1];
    }

    float getHighFreq()
    {
        return bandFreqs[2];
    }

    float getBandGain(int band)
    {
        return postGain[band];
    }

private:
    // prevent the plugin from blasting ppls ears on startup
    const int startupTime;
    int startup;

    // pitch tracking
    pitchTracker pt;
    float phase;
    std::atomic<float> d;
    std::atomic<double> sampleRate;
    const float transpose_amts[9] = { .125, .25, .5, 1, 2, 4, 8, 16, 32 };

    // params
    float preGain[4], targetpreGain[4],
        targetpostGain[4],
        fmAmt[4], targetfmAmt[4],
        fmPitch[4],
        feedbackDelay[4], targetfeedbackDelay[4],
        feedbackAmt[4], release;

    std::atomic<float> postGain[4];

    int feedBackMode;

    float noiseGate;

    // buffers
    CircularBuffer fmBuffers[4];
    CircularBuffer delayBuffers[4];

    // filters
    std::atomic<int> bandFreqs[3];

    juce::dsp::IIR::Filter<float> hp[4], lp[4];
    
    HigherOrderLRF filters[6];

    const static int numFilters = 6;

    // helper functions
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
            postGain[i] = smoothit(postGain[i], targetpostGain[i], .999);
            preGain[i] = smoothit(preGain[i], targetpreGain[i], .999);
        }
    }

    void update_band_freqs()
    {
        filters[LP1].setCutoffFrequency(bandFreqs[1]);
        filters[HP1].setCutoffFrequency(bandFreqs[1]);
        filters[LP2].setCutoffFrequency(bandFreqs[0]);
        filters[HP2].setCutoffFrequency(bandFreqs[0]);
        filters[LP3].setCutoffFrequency(bandFreqs[2]);
        filters[HP3].setCutoffFrequency(bandFreqs[2]);
    }

};