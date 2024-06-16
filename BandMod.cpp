/*
  ==============================================================================

    BandMod.cpp
    Created: 13 Jun 2024 12:04:54pm
    Author:  noone

  ==============================================================================
*/

#include "BandMod.h"

void BandMod::prepare(juce::dsp::ProcessSpec& s)
{
    for (int i = 0; i < numFilters; i++)
    {
        filters[i].prepare(s);
        
    }

    for (int i = 0; i < 4; i++)
    {
        hp[i].prepare(s);
        hp[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(s.sampleRate, 10);
    }
}

float BandMod::process(float s)
{
    /*
    *            v-delay---|
               HP3--clip--fm\
          HP1--| v-delay---| \
          |    LP3--clip--fm--\
    s ----|                    smoosh --> shit
          |    HP2--clip--fm--/
          LP1--| ^-delay---| /
               LP2--clip--fm/
                 ^-delay---|
    */
    double trash;   //unused garbage
    float o = 0;
    float bs[4] = { 0,0,0,0 };  // for keeping track of each band's processing
    update_feedback_times();
    
    // pitch tracking stuff
    float tracked = pt.getPitch(s);

    if (tracked < 0) tracked = 0;
    float d = (0.3183098f * tracked)/2.0;
    phase += d;
    phase = modf(phase, &trash);

    // add feedback from each band
    /*
    for (int i = 0; i < 4; i++)
    {
        s += delayBuffers[i].get(1 + feedbackDelay[i] * 40000) * feedbackAmt[i];
    }
    */

    //filter into different bands
    float tmp = 0;
    tmp = filters[HP1].processSample(0, s);
    bs[3] = filters[HP3].processSample(0, tmp);
    bs[2] = filters[LP3].processSample(0, tmp);
    tmp = filters[LP1].processSample(0, s);
    bs[0] = filters[LP2].processSample(0, tmp);
    bs[1] = filters[HP2].processSample(0, tmp);

    // apply gain & clipping & write to buffers
    for (int i = 0; i < 4; i++)
    {
        bs[i] = jlimit(-1.f, 1.f, bs[i] * preGain[i])*postGain[i];
        // add in feedback
        bs[i] += hp[i].processSample(jlimit(-1.f, 1.f, delayBuffers[i].get(1 + feedbackDelay[i] * 40000) * feedbackAmt[i]));
        hp[i].snapToZero();
        fmBuffers[i].put(bs[i]);
    }

    // get fm-modulated samples & add to output
    for (int i = 0; i < 4; i++)
    {
        float transpose = transpose_amts[(int)floor(fmPitch[i]* 8.0)];
        bs[i] = fmBuffers[i].get(fmAmt[i] + fmAmt[i] * sin(transpose * phase * MathConstants<float>::pi));
        delayBuffers[i].put(bs[i]);

        o += bs[i];
    }

    // also output the tracked pitch for debugging
    // o += sin(phase * MathConstants<float>::pi);

    return o;
}