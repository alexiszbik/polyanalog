/*
  ==============================================================================

    PolyFMCore.cpp
    Created: 19 Jan 2024 10:00:43am
    Author:  Alexis ZBIK

  ==============================================================================
*/

#include "PolyAnalogCore.h"
#include "DaisyYMNK/Helpers/StrConverters.h"

bool isBetweenParameterIndex(int x, int a, int b) {
    return x >= a && x <= b;
}

PolyAnalogCore::PolyAnalogCore()
: ModuleCore(&polySynth,
     {
        {MuxKnob_1,                 kKnob,      HIDPin(0,0),    "MuxKnob_1"},
        {MuxKnob_2,                 kKnob,      HIDPin(0,1),    "MuxKnob_2"},
        {MuxKnob_3,                 kKnob,      HIDPin(0,2),    "MuxKnob_3"},
        {MuxKnob_4,                 kKnob,      HIDPin(0,3),    "MuxKnob_4"},
        {MuxKnob_5,                 kKnob,      HIDPin(0,4),    "MuxKnob_5"},
        {MuxKnob_6,                 kKnob,      HIDPin(0,5),    "MuxKnob_6"},
        {MuxKnob_7,                 kKnob,      HIDPin(0,6),    "MuxKnob_7"},
        {MuxKnob_8,                 kKnob,      HIDPin(0,7),    "MuxKnob_8"},
        {MuxKnob_9,                 kKnob,      HIDPin(0,8),    "MuxKnob_9"},
        {MuxKnob_10,                kKnob,      HIDPin(0,9),    "MuxKnob_10"},
        {MuxKnob_11,                kKnob,      HIDPin(0,10),   "MuxKnob_11"},
        {MuxKnob_12,                kKnob,      HIDPin(0,11),   "MuxKnob_12"},
        {MuxKnob_13,                kKnob,      HIDPin(0,12),   "MuxKnob_13"},
        {MuxKnob_14,                kKnob,      HIDPin(0,13),   "MuxKnob_14"},
        {MuxKnob_15,                kKnob,      HIDPin(0,14),   "MuxKnob_15"},
        {MuxKnob_16,                kKnob,      HIDPin(0,15),   "MuxKnob_16"},
    
        {KnobVolume,                kKnob,      16,             "Volume"},
        {KnobCutoff,                kKnob,      18,             "Cutoff"},
        {KnobRes,                   kKnob,      17,             "Res"},
    
        {ButtonShift,               kSwitch,    5,              "Shift"},
        {ButtonSave,                kButton,    6,              "Button Save"},
        {ButtonPreviousPreset,      kButton,    7,              "Previous Preset"},
        {ButtonNextPreset,          kButton,    8,              "Next Preset"},

        {MidiLed,                   kLed,       10,             "Led"},
     }, (5 - 1)) //do something for midi channel who's not correct
{
    lockAllKnobs();
    
    needsResetDisplay = true;
}

void PolyAnalogCore::lockAllKnobs() {
    for (auto knob = (int)MuxKnob_1; knob <= (int)KnobVolume; knob++) {
        lockHID(knob);
    }
}

void PolyAnalogCore::loadPreset(const float* values) {
    dspKernel->loadPreset(values);
    lockAllKnobs();
}

void PolyAnalogCore::changeCurrentPreset(bool increment) {
    if (increment) {
        currentPreset.increment();
    } else {
        currentPreset.decrement();
    }
    
    const float* dataToLoad = presetManager->Load(currentPreset.get());
    if (dataToLoad) {
        loadPreset(dataToLoad);
    }
    intToCString2(currentPreset.get(), numCharBuffer);
    displayManager->Write("Load Preset", numCharBuffer);
    needsResetDisplay = true;
}

void PolyAnalogCore::saveCurrentPreset() {
    float pData[MAX_PRESET_SIZE];
    auto allParam = getAllParameters();
    uint8_t k = 0;
    for (auto& param : allParam) {
        pData[k++] = param->getUIValue();
    }

    bool result = presetManager->Save(pData, k, currentPreset.get());
    if (result) {
        displayManager->Write("Save Success!");
    } else {
        displayManager->Write("Save Failed!");
    }
    needsResetDisplay = true;
}

void PolyAnalogCore::displayValuesOnScreen() {
    if (!needsToUpdateValue) {
        return;
    }
    if (lastParam) {
        if (lastParamIndex == PolyAnalogDSP::LfoDestinationA) {
            const char* destName = polySynth.getLfoDestName(0);
            displayManager->WriteLine(2, destName);
            
        } else {
            float value = lastParam->getUIValue();
            floatToCString2(value, numCharBuffer);
            
            displayManager->WriteLine(2, numCharBuffer);
        }
    }
    needsToUpdateValue = false;
}

//Well we should make a loop again
void PolyAnalogCore::displayParameterOnScreen(unsigned int index) {
    Parameter* lastChanged = dspKernel->getParameter(index);
    
    if (lastChanged && lastChanged != lastParam) {
        const char* name = lastChanged->getName();
        lastParam = lastChanged;
        displayManager->WriteLine(1, name);
    }
    
    needsToUpdateValue = true;
}

void PolyAnalogCore::processMIDI(MIDIMessageType messageType, int channel, int dataA, int dataB) {
    ModuleCore::processMIDI(messageType, channel, dataA, dataB);
    if (midiChannel == -1 || midiChannel == channel) { // Maybe do something better
        if (messageType == kNoteOn) {
            setHIDValue(MidiLed, 1);
        } else if (messageType == kNoteOff) {
            setHIDValue(MidiLed, 0);
        }
    }
}

void PolyAnalogCore::updateHIDValue(unsigned int index, float value) {

    switch (index) {

        case ButtonShift:
            shiftState = (bool)value;
            break;
            
        case ButtonSave: {
            if (shiftState) {
                saveCurrentPreset();
            } else {
                polySynth.togglePlayMode();
                displayParameterOnScreen(PolyAnalogDSP::PlayMode);
            }
        }
            break;
            
        case ButtonPreviousPreset: {
            if (shiftState) {
                changeCurrentPreset(false);
            } else {
                displayParameterOnScreen(PolyAnalogDSP::PlayMode);
            }
        }
            break;
            
        case ButtonNextPreset: {
            if (shiftState) {
                changeCurrentPreset(true);
            } else {
                displayParameterOnScreen(PolyAnalogDSP::PlayMode);
            }
        }
            break;
            
        case MidiLed:
            //Hmmm, this should never happen
            break;
            
        case KnobVolume: dspKernel->setParameterValue(PolyAnalogDSP::Volume, value); break;
        case KnobCutoff: dspKernel->setParameterValue(PolyAnalogDSP::FilterCutoff, value); break;
        case KnobRes: dspKernel->setParameterValue(PolyAnalogDSP::FilterRes, value); break;
            
        default:
            if (isBetweenParameterIndex(index, MuxKnob_1, MuxKnob_16)) {
                dspKernel->setParameterValue(parameterMap[index - MuxKnob_1], value);
            }
            
            break;
    }
}
