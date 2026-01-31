/*
  ==============================================================================

    SynthOsc.h
    Created: 31 Jan 2026 4:39:52pm
    Author:  Alexis ZBIK

  ==============================================================================
*/

#pragma once

#include "daisysp.h"
#include "DaisyYMNK/DSP/DSP.h"
#include "DaisyYMNK/Common/Common.h"

using namespace std;
using namespace daisysp;

class SynthOsc {
public:
    void init(double sampleRate);
    void setWaveform(float value);
    void setPitch(float pitch);
    
    float process();
    void reset();
    
private:
    static const uint8_t count = 2;
    Oscillator oscs[count];
    
    const static uint8_t sawWavf = Oscillator::WAVE_SAW;
    const static uint8_t sqrWavf = Oscillator::WAVE_SQUARE;
    
    float oscMix = 0.f;
    float sawDetune = 0.f;
    float sawMix = 0.f;
};
