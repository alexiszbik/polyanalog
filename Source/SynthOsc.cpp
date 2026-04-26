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
        float freq = fast_mtof(pitch + pitchOffset + sawDetune*factor);
        oscs[k].SetFreq(fminf(freq, halfSr));
    }
}

void SynthOsc::setWaveform(float value) {
    if (value < 0.3333f) {
        oscs[1].SetWaveform(sawWavf);
    } else {
        oscs[1].SetWaveform(sqrWavf);
    }
    float base = value * 4.f;
    if (base > 1.f) {
        base = 1.f;
    }
    sawDetune = 1.f - base;
    sawMix = sawDetune*0.5f;
    sawDetune *= sawDetune;
    float ranged = (value - 0.333f) * 1.492537f;
    if (ranged < 0.f) {
        ranged = 0.f;
    }
    const float v = ranged*2.f;
    oscMix = v;
    if (oscMix > 1.f) {
        oscMix = 1.f;
    }
    oscMix *= oscMix;
    float pulseAmount = v - 1.f;
    if (pulseAmount < 0.f) {
        pulseAmount = 0.f;
    }
    float pw = 0.5f - pulseAmount * 0.47f;
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


