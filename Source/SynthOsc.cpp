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
        oscs[k].SetAmp(1);
        
        setWaveform(0.f);
    }
    
    oscs[0].SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    oscs[1].SetWaveform(Oscillator::WAVE_POLYBLEP_SQUARE);
    oscs[1].SetPw(0.5);
}

void SynthOsc::setFreq(float freq) {
    uint8_t k = count;
    while(k--) {
        oscs[k].SetFreq(freq);
    }
}

void SynthOsc::setWaveform(float value) {
    const float ranged = fmax((value-0.333f)*1.492537f, 0);
    const float v = ranged*2.f;
    oscMix = 1.f - fminf(v, 1.f);
    oscMix *= oscMix;
    float pw = 0.5f - fmaxf(v - 1.f, 0.f) * 0.49f;
    oscs[1].SetPw(pw);
}

void SynthOsc::reset() {
    uint8_t k = count;
    while(k--) {
        oscs[k].Reset();
    }
}

float SynthOsc::process() {
    return ydaisy::dryWet(oscs[0].Process(), oscs[1].Process(), oscMix);
}


