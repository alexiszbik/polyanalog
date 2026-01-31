/*
  ==============================================================================

    SynthVoice.h
    Created: 10 Jan 2024 2:48:00pm
    Author:  Alexis ZBIK

  ==============================================================================
*/

#pragma once

#include "DaisyYMNK/DSP/DSP.h"
#include "DaisyYMNK/Common/Common.h"
#include "daisysp.h"

using namespace ydaisy;
using namespace std;

using namespace daisysp;
//using namespace ydaisy;

class SynthVoice {
public:
    void init(double sampleRate);
    
    void prepare();
    
    void setGlide(float glide);
    
    void setADSR(float attack, float decay, float sustain, float release);
    
    void setWaveform(uint8_t oscIndex, float value);
    void setOctave(int8_t octave);
    void setOscBTune(uint8_t tuneIndex);
    void setOscBPW(float pw);
    void setOscMix(float mix);
    void setFilterMidiFreq(float freq);
    void setFilterRes(float res);
    void setFilterEnv(float env);
    
    void setNoteOn(Note note);
    void setNoteOff();
    
    float process();
    
    //TO REWRITE
    inline int currentPitch() noexcept {
        return pitch.getGoal();
    }
    
    inline bool isPlaying() noexcept {
        return gate || adsr.IsRunning();
    }
    
private:
    void setPitch(int pitch);
    void setGate(bool gate);
    
public:
    static const uint8_t btuneCount = 11;
    
    static const float btune[];
    
    unsigned long noteTimeStamp; //TO REWRITE
    float pitchMod = 0; //TO REWRITE
    
    static const int kAlgorithmCount = 11;
    static const int kOperatorCount = 4;
    
private:
    int8_t octave;
    
    double sampleRate;
    float halfSr;
    
    float glide = 0;
    
    float tune = 0;
    float pw = 0.5;
    float mix = 0.5;
    
    float filterMidiFreq = 800;
    float filterRes = 0.5;
    float filterEnv = 0.25;

    SmoothValue pitch;
    bool gate = false;
    
    static const uint8_t oscCount = 2;
    
    float freq[oscCount] = {0, 0};
    
    Adsr adsr;
    Oscillator oscs[oscCount];
    BiquadFilter filter;
 
};
