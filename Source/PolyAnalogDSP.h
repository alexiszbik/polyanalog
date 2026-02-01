/*
  ==============================================================================

    PolyAnalogDSP.h
    Created: 8 Nov 2023 4:51:13pm
    Author:  Alexis ZBIK

  ==============================================================================
*/

#pragma once

#include "DaisyYMNK/DSP/DSP.h"
#include "PolySynth.h"
#include "Lfo.h"

#include "daisysp.h"

using namespace daisysp;
using namespace ydaisy;

#define MIDI_CC_START 10

#define LFO_PARAM(_name) \
LfoType##_name, \
LfoDestination##_name, \
LfoRate##_name, \
LfoAmount##_name

class PolyAnalogDSP : public DSPKernel {
public:
    enum Parameters {
        PlayMode,
        Glide,
        Volume,
        
        OscWaveformA,
        OscOctaveA,
        
        OscWaveformB,
        OscTuneB,
        
        OscNoise,
        OscMix,
        
        FilterCutoff,
        FilterRes,
        FilterEnv,

        Attack,
        Decay,
        Sustain,
        
        HighPass,
        
        LFO_PARAM(A),
        LFO_PARAM(B),

        Count
    };
    
public:
    PolyAnalogDSP();
    ~PolyAnalogDSP();
    
public:
    virtual void init(int channelCount, double sampleRate) override;
    virtual void process(float** buf, int frameCount) override;
    virtual void processMIDI(MIDIMessageType messageType, int channel, int dataA, int dataB) override;

    const char* getLfoDestName(int lfoIdx);
    
    void togglePlayMode();
    
protected:
    virtual void updateParameter(int index, float value) override;
    
private:
    float getLfoBuffer(int lfoIdx, Lfo::LfoDest target, uint8_t frame, float multiplier = 1.f);
    
private:
    PolySynth synth;
    FastOnePole hpFilter;
    
    static constexpr uint8_t lfoCount = 2;
    const float multipliers[5] = { 0.001f, 0.01f, 0.1f, 1.f, 10.f };
    
    Lfo lfo[lfoCount];
    
    unsigned long timeStamp = 0;
    static constexpr float Qmin = 0.5f;
    static constexpr float Qmax = 8.0f;
    float lnRatio = std::log(Qmax / Qmin);

};
