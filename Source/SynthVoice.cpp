/*
  ==============================================================================

    SynthVoice.cpp
    Created: 10 Jan 2024 2:48:00pm
    Author:  Alexis ZBIK

  ==============================================================================
*/

#include "SynthVoice.h"


const float SynthVoice::btune[] = {-24, -17, -12, -5, 0, 0.08, 0.2, 7, 12, 19, 24};

void SynthVoice::init(double sampleRate) {

    this->sampleRate = sampleRate;
    
    pitch.setImmediate(60);

    adsr.Init(sampleRate);
    uint8_t k = oscCount;
    while(k--) {
        oscs[k].init(sampleRate);
    }
    filter.Init(sampleRate);
    
    filterFreqSmoother.Init(20, sampleRate);
}

void SynthVoice::setPitch(int pitch) {
    this->pitch.setValue((float)pitch);
}

void SynthVoice::setGate(bool gate) {
    this->gate = gate;
}

void SynthVoice::setNoteOn(Note note) {
    if (adsr.IsRunning() == false) { //In order to avoid clicks we should verify any output operators
        uint8_t k = oscCount;
        while(k--) {
            oscs[k].reset();
        }
    }
    
    setPitch(note.pitch);
    adsr.Retrigger(false);
    setGate(true);
    noteTimeStamp = note.timeStamp;
}

void SynthVoice::setNoteOff() {
    setGate(false);
}

void SynthVoice::setGlide(float glide) {
    this->glideFrameLength = (glide*glide)*sampleRate;
}

void SynthVoice::setADSR(float attack, float decay, float sustain, float release) {
   adsr.SetAttackTime(attack);
   adsr.SetDecayTime(decay);
   adsr.SetSustainLevel(sustain);
   adsr.SetReleaseTime(release);
}

void SynthVoice::setWaveform(uint8_t oscIndex, float value) {
    oscs[oscIndex].setWaveform(value);
}

void SynthVoice::setOctave(int8_t octave) {
    this->octave = octave;
}

void SynthVoice::setOscBTune(uint8_t tuneIndex) {
    this->tune = btune[tuneIndex];
}

void SynthVoice::setOscBPW(float pw) {
    this->pw = pw;
}

void SynthVoice::setOscMix(float mix) {
    this->mix = 1.f - (mix * mix);
}

void SynthVoice::setNoiseMix(float mix) {
    this->noiseMix = mix;
}

void SynthVoice::setFilterMidiFreq(float freq) {
    this->filterMidiFreq = freq;
}

void SynthVoice::setFilterRes(float res) {
    this->filterRes = res;
}

void SynthVoice::setFilterEnv(float env) {
    this->filterEnv = env;
}

void SynthVoice::prepare() {
}

float SynthVoice::process(float whiteNoiseIn, float filterMod) {
    
    pitch.dezipperCheck(glideFrameLength);
    
    float mainPitch = pitch.getAndStep() + pitchMod;
    
    float envOut = adsr.Process(gate);
    
    oscs[0].setPitch(mainPitch + octave*12.f);
    oscs[1].setPitch(mainPitch + tune);

    float oscMix = ydaisy::sqrtDryWet(oscs[0].process(), oscs[1].process(), mix);
    float outMix = ydaisy::sqrtDryWet(whiteNoiseIn, oscMix, noiseMix);
    
    float smoothMod = filterFreqSmoother.Process(filterMod);
    
    float fFreq = fast_mtof(fminf(filterMidiFreq + envOut*90.f*filterEnv + smoothMod, 132.f));
    filter.SetLowpass(fFreq, filterRes);

    return filter.Process(outMix) * envOut * envOut;
}
