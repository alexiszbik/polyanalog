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
        OscPwB,
        
        OscMix,
        
        FilterCutoff,
        FilterRes,
        FilterEnv,

        Attack,
        Decay,
        Sustain,
        Release,
        
        LFO_PARAM(A),
        LFO_PARAM(B),
        
        EnvDestination,
        EnvAttack,
        EnvDecay,
        EnvAmount,

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
    
protected:
    virtual void updateParameter(int index, float value) override;
    
private:
    int getLfoParam(int lfoId, int aParam);
    float opTimeValue(int operatorId, int aParam, bool applyTimeRatio, float min = 0.002f, float max = 2.f);
    
    float getLfoBuffer(Lfo::LfoDest target, uint8_t frame, float multiplier = 1.f);
    
private:
    PolySynth synth;
    
    static constexpr uint8_t lfoCount = 2;
    const float multipliers[5] = { 0.001f, 0.01f, 0.1f, 1.f, 10.f };
    
    Lfo lfo[lfoCount];
    
    unsigned long timeStamp = 0;

};
