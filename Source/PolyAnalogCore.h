/*
  ==============================================================================

    PolyAnalogCore.h
    Created: 19 Jan 2024 10:00:43am
    Author:  Alexis ZBIK

  ==============================================================================
*/

#pragma once

#include "DaisyYMNK/DSP/DSP.h"
#include "DaisyYMNK/Helpers/BoundedInt.h"
#include "PolyAnalogDSP.h"

#define DSP_PARAM_OP(_name) \
PolyAnalogDSP::Coarse##_name, \
PolyAnalogDSP::Fine##_name, \
PolyAnalogDSP::Mode##_name, \
PolyAnalogDSP::Amount##_name, \
PolyAnalogDSP::Attack##_name, \
PolyAnalogDSP::Decay##_name, \
PolyAnalogDSP::Sustain##_name, \
PolyAnalogDSP::Release##_name

class PolyAnalogCore : public ModuleCore {
public:
    enum {
        MuxKnob_1 = 0,
        MuxKnob_2,
        MuxKnob_3,
        MuxKnob_4,
        MuxKnob_5,
        MuxKnob_6,
        MuxKnob_7,
        MuxKnob_8,
        MuxKnob_9,
        MuxKnob_10,
        MuxKnob_11,
        MuxKnob_12,
        MuxKnob_13,
        MuxKnob_14,
        MuxKnob_15,
        MuxKnob_16,
        
        KnobVolume,
        KnobGlide,
        KnobOscMix,
        
        ButtonSave,
        ButtonPlayMode,
        ButtonPreviousPreset,
        ButtonNextPreset,
        
        
        MidiLed
    };
public:
    PolyAnalogCore();

    int getCurrentPage();
    void loadPreset(const float* values);

    virtual void processMIDI(MIDIMessageType messageType, int channel, int dataA, int dataB) override;
    
protected:
    void updateHIDValue(unsigned int index, float value) override;
    
private:
    void lockAllKnobs();
    void changeCurrentPreset(bool increment);
    void saveCurrentPreset();
    
    void displayLastParameterOnScreen();
    
public:
    void displayValuesOnScreen();
    
private:
    int parameterMap[16] = {
        PolyAnalogDSP::OscWaveformA,
        PolyAnalogDSP::OscOctaveA,
        PolyAnalogDSP::OscWaveformB,
        PolyAnalogDSP::OscTuneB,
        PolyAnalogDSP::OscPwB,
        PolyAnalogDSP::FilterCutoff,
        PolyAnalogDSP::FilterRes,
        PolyAnalogDSP::FilterEnv,
        PolyAnalogDSP::Attack,
        PolyAnalogDSP::Decay,
        PolyAnalogDSP::Sustain,
        PolyAnalogDSP::Release,
        PolyAnalogDSP::LfoType,
        PolyAnalogDSP::LfoDestination,
        PolyAnalogDSP::LfoRate,
        PolyAnalogDSP::LfoAmount
    };

    BoundedInt<0,15> currentPreset = 0;
    
    char numCharBuffer[4];
    char fullNumCharBuffer[20];
    
    ydaisy::Parameter* lastParam = nullptr;
    int lastParamIndex = 0;
    bool needsToUpdateValue = false;
    
    bool needsResetDisplay = false;
    
    PolyAnalogDSP polySynth;
};
