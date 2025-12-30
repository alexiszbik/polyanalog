/*
  ==============================================================================

    PolySynth.cpp
    Created: 10 Jan 2024 3:37:02pm
    Author:  Alexis ZBIK

  ==============================================================================
*/

#include "PolySynth.h"

PolySynth::PolySynth() {
    for (size_t i = 0; i < VOICE_COUNT; i++) {
        voices.push_back(new SynthVoice());
    }
}

PolySynth::~PolySynth() {
    for (auto v : voices)
    {
        delete v;
    }
    voices.clear();
}

void PolySynth::init(double sampleRate)  {
    for (auto v : voices)
    {
        v->init(sampleRate);
    }
    modulation.Init(sampleRate);
    modulation.SetFreq(8);
}

void PolySynth::setNote(bool isNoteOn, Note note) {
    
    int voiceCount = 1;
    switch (polyMode) {
        case Mono:
            voiceCount = 1;
            break;
        case Unison:
            voiceCount = UNISON_VOICE_COUNT;
            break;
        case Poly:
            voiceCount = VOICE_COUNT;
            break;
            
        default:
            break;
    }

    if (isNoteOn) {
        
        noteState.push_back(note);
        
        if (polyMode != Poly) {
            for (int i = 0; i < voiceCount; i++)
            {
                voices.at(i)->setNoteOn(note);
            }
        } else {
            for (int i = 0; i < voiceCount; i++)
            {
                if (voices.at(i)->currentPitch() == note.pitch) {
                    voices.at(i)->setNoteOn(note);
                    return;
                }
            }
            
            for (int i = 0; i < voiceCount; i++)
            {
                if (!voices.at(i)->isPlaying()) {
                    voices.at(i)->setNoteOn(note);
                    return;
                }
            }
            
            SynthVoice* oldest = voices.front();
            for (int i = 0; i < voiceCount; i++)
            {
                if (voices.at(i)->noteTimeStamp < oldest->noteTimeStamp) {
                    oldest = voices.at(i);
                }
            }
            
            oldest->setNoteOn(note);
        }
    } else {
        auto nIt = noteState.begin();
        
        while (nIt != noteState.end()) {
            if (nIt->pitch == note.pitch) {
                nIt = noteState.erase(nIt);
            } else {
                nIt++;
            }
        }
        
        bool sendNoteOff = true;
        
        if (polyMode != Poly) {
            if (noteState.size()) {
                for (int i = 0; i < voiceCount; i++)
                {
                    if (voices.at(i)->currentPitch() != noteState.back().pitch) {
                        voices.at(i)->setNoteOn(noteState.back());
                    }
                    sendNoteOff = false;
                }
            }
        }
        
        if (sendNoteOff) {
            for (int i = 0; i < voiceCount; i++)
            {
                if (voices.at(i)->currentPitch() == note.pitch && voices.at(i)->isPlaying()) {
                    voices.at(i)->setNoteOff();
                }
            }
        }
    }
}

void PolySynth::setPitchBend(float bend) {
    this->bend.setValue(bend);
}

void PolySynth::setModWheel(float value) {
    this->vibratoAmount.setValue(value);
}

void PolySynth::setTune(float tune) {
    this->tune = tune;
}

void PolySynth::setPolyMode(EPolyMode newPolyMode) {
    if (newPolyMode != polyMode) {
        polyMode = newPolyMode;
        for (int i = 0; i < VOICE_COUNT; i++)
        {
            voices.at(i)->setNoteOff();
        }
    }
}

void PolySynth::setGlide(float glide) {
    for (auto v : voices)
    {
        v->setGlide(glide);
    }
}

void PolySynth::setWaveform(uint8_t waveformIndex, uint8_t oscIndex) {
    for (auto v : voices)
    {
        v->setWaveform(waveformIndex, oscIndex);
    }
}

void PolySynth::setOctave(int8_t octave) {
    for (auto v : voices)
    {
        v->setOctave(octave);
    }
}

void PolySynth::setOscBTune(uint8_t tuneIndex) {
    for (auto v : voices)
    {
        v->setOscBTune(tuneIndex);
    }
}

void PolySynth::setOscBPW(float pw) {
    for (auto v : voices)
    {
        v->setOscBPW(pw);
    }
}

void PolySynth::setOscMix(float mix) {
    for (auto v : voices)
    {
        v->setOscMix(mix);
    }
}

void PolySynth::setADSR(float attack, float decay, float sustain, float release) {
    for (auto v : voices)
    {
        v->setADSR(attack, decay, sustain, release);
    }
}

void PolySynth::setEnvParameters(float attack, float decay, float amount) {
    for (auto v : voices)
    {
        v->setEnvParameters(attack, decay, amount);
    }
}

void PolySynth::setFilterMidiFreq(float freq) {
    for (auto v : voices)
    {
        v->setFilterMidiFreq(freq);
    }
}
void PolySynth::setFilterRes(float res) {
    for (auto v : voices)
    {
        v->setFilterRes(res);
    }
}
void PolySynth::setFilterEnv(float env) {
    for (auto v : voices)
    {
        v->setFilterEnv(env);
    }
}

void PolySynth::preprare() {
    for (auto v : voices)
    {
        v->prepare();
    }
}

float PolySynth::process() {
    float pitchMod = 0;
    
    float result = 0;
    float idx = 0;
    
    bend.dezipperCheck(smoothGlobal);
    vibratoAmount.dezipperCheck(smoothGlobal);
    pitchMod = tune + bend.getAndStep() + modulation.Process() * vibratoAmount.getAndStep();

    for (auto v : voices)
    {
        v->pitchMod = pitchMod;
        
        if (polyMode == Unison) {
            float unisonMod = -0.015625 + (idx*(0.03125/(UNISON_VOICE_COUNT-1)));
            v->pitchMod += unisonMod;
        }
        result += v->process();
        idx++;
    }
    if (polyMode == Unison) {
        result *= 0.707;
    }
    
    return result;
}
