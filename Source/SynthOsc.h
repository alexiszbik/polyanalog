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
    void setFreq(float freq);
    
    float process();
    void reset();
    
private:
    static const uint8_t count = 2;
    Oscillator oscs[count];
    
    float oscMix = 0;
};
