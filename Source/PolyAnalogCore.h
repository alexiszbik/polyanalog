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
    using WillNeedToResetScreenHandler = void (*)();

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
        KnobCutoff,
        KnobRes,
        
        ButtonShift,
        ButtonOK,
        ButtonPrevious,
        ButtonNext,
        
        
        MidiLed
    };
public:
    PolyAnalogCore();

    int getCurrentPage();
    void loadPreset(const float* values);
    void ready();

    void setWillNeedToResetScreenHandler(WillNeedToResetScreenHandler handler)
    {
        willNeedToResetScreenHandler_ = handler;
    }

    void updateScreen();

    virtual void processMIDI(MIDIMessageType messageType, int channel, int dataA, int dataB) override;
    virtual bool unlockCondition(unsigned int index, float value, HIDState* hidState) override;
    
protected:
    void updateHIDValue(unsigned int index, float value) override;
    
private:
    void lockAllKnobs();
    void changeCurrentPreset(bool increment);
    void loadPresetAtIndex(int presetIndex);
    void saveCurrentPreset();
    void switchToSaveMode();
    
    void displaySaveIndex();
    
public:
    void displayValuesOnScreen();
    
private:
    int parameterMap[16] = {
        PolyAnalogDSP::OscMix,
        PolyAnalogDSP::OscWaveformB,
        PolyAnalogDSP::OscWaveformA,
        PolyAnalogDSP::OscOctaveA,
        PolyAnalogDSP::OscTuneB,
        PolyAnalogDSP::OscNoise,
        PolyAnalogDSP::Glide,
        PolyAnalogDSP::HighPass,
        PolyAnalogDSP::FilterEnv,
        PolyAnalogDSP::Decay,
        PolyAnalogDSP::Sustain,
        PolyAnalogDSP::Attack,
        PolyAnalogDSP::LfoAmountB,
        PolyAnalogDSP::LfoRateB,
        PolyAnalogDSP::LfoAmountA,
        PolyAnalogDSP::LfoRateA
    };
    
    static const int intParameterCount = 3;
    int intParameterMap[intParameterCount] = {
        PolyAnalogDSP::PlayMode,
        PolyAnalogDSP::LfoDestinationA,
        PolyAnalogDSP::LfoDestinationB
    };
    
    int currentIntParameterIndex = 0;

    BoundedInt<0,MAX_PRESETS-1> currentPreset = 0;
    BoundedInt<0,MAX_PRESETS-1> indexToSaveNewPreset = 0;
    
    char numCharBuffer[4];
    //char fullNumCharBuffer[20];
    
    ydaisy::Parameter* lastParam = nullptr;

    PolyAnalogDSP polySynth;

    bool shiftState = false;
    bool saveMode = false;

    WillNeedToResetScreenHandler willNeedToResetScreenHandler_ = nullptr;
};
