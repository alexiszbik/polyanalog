/*
  ==============================================================================

    SynthOsc.cpp
    Created: 31 Jan 2026 4:39:52pm
    Author:  Alexis ZBIK

  ==============================================================================
*/

#include "SynthOsc.h"

void SynthOsc::init(double sampleRate) {
    uint8_t k = count;
    while(k--) {
        oscs[k].Init(sampleRate);
        halfSr = sampleRate * 0.49;
        oscs[k].SetAmp(1);
        
        setWaveform(0.f);
    }
    
    oscs[0].SetWaveform(sawWavf);
}

void SynthOsc::setPitch(float pitch) {
    uint8_t k = count;
    while(k--) {
        float factor = k == 1 ? 0.2f : -0.2f;
        float freq = fast_mtof(pitch + sawDetune*factor);
        oscs[k].SetFreq(fminf(freq, halfSr));
    }
}

void SynthOsc::setWaveform(float value) {
    if (value < 0.3333f) {
        oscs[1].SetWaveform(sawWavf);
    } else {
        oscs[1].SetWaveform(sqrWavf);
    }
    const float base = fminf(value * 4.f, 1.f);
    sawDetune = 1.f - base;
    sawMix = sawDetune*0.5f;
    sawDetune *= sawDetune;
    const float ranged = fmaxf((value-0.333f)*1.492537f, 0.f);
    const float v = ranged*2.f;
    oscMix = fminf(v, 1.f);
    oscMix *= oscMix;
    float pw = 0.5f - fmaxf(v - 1.f, 0.f) * 0.47f;
    oscs[1].SetPw(pw);
}

void SynthOsc::reset() {
    uint8_t k = count;
    while(k--) {
        oscs[k].Reset();
    }
}

float SynthOsc::process() {
    return ydaisy::sqrtDryWet(oscs[1].Process(), oscs[0].Process(), fmaxf(oscMix, sawMix));
}


