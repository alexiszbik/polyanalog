/*
  ==============================================================================

    PolySynth.h
    Created: 10 Jan 2024 3:37:02pm
    Author:  Alexis ZBIK

  ==============================================================================
*/

#pragma once

#include "SynthVoice.h"
#include "DaisyYMNK/Common/Common.h"
#include "daisysp.h"

#define VOICE_COUNT 4
#define UNISON_VOICE_COUNT 3

using namespace std;
using namespace daisysp;

class PolySynth {
public:
    enum EPolyMode {
        Mono = 0,
        Unison,
        Poly
    };

public:
    PolySynth();
    ~PolySynth();

public:
    void init(double sampleRate);
    void setNote(bool isNoteOn, Note note);
    
    void preprare();
    float process();
    
    void setPitchBend(float bend);
    void setModWheel(float value);
    void setPolyMode(EPolyMode newPolyMode);
    void setGlide(float glide);
    void setTune(float tune);
    
    void setADSR(float attack, float decay, float sustain, float release);
    void setWaveform(uint8_t oscIndex, float value);
    void setOctave(int8_t octave);
    void setOscBTune(uint8_t tuneIndex);
    void setOscMix(float mix);
    void setNoiseMix(float mix);
    void setFilterMidiFreq(float freq);
    void setFilterRes(float res);
    void setFilterEnv(float env);
    
private:
    EPolyMode polyMode = Mono;
    vector<SynthVoice*> voices;
    
    float tune = 0;
    
    SmoothValue bend;
    SmoothValue vibratoAmount;
    
    Oscillator modulation;
    WhiteNoise whiteNoise;
    
    vector<Note> noteState;
    
    static constexpr int smoothGlobal = 800;
};
