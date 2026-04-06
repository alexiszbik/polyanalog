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
    {ButtonOK,                kButton,    6,              "Button Save"},
    {ButtonPrevious,      kButton,    7,              "Previous Preset"},
    {ButtonNext,          kButton,    8,              "Next Preset"},
    
    {MidiLed,                   kLed,       10,             "Led"},
}, (5 - 1)) //do something for midi channel who's not correct
{
    lockAllKnobs();
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

void PolyAnalogCore::ready() {
    updateScreen();
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
    updateScreen();
}

void PolyAnalogCore::saveCurrentPreset() {
    float pData[MAX_PRESET_SIZE];
    auto allParam = getAllParameters();
    uint8_t k = 0;
    for (auto& param : allParam) {
        pData[k++] = param->getUIValue();
    }

    int presetIndex = indexToSaveNewPreset.get();
    bool result = presetManager->Save(pData, k, presetIndex);
    
    intToCString2(presetIndex, numCharBuffer);
    if (result) {
        displayManager->Write("Save Success!");
        displayManager->WriteLine(1, numCharBuffer);
        currentPreset = indexToSaveNewPreset;
    } else {
        displayManager->Write("Save Failed!");
    }
    
    saveMode = false;
}

void PolyAnalogCore::switchToSaveMode() {
    displayManager->WriteLine(0, "Save ?");
    indexToSaveNewPreset = currentPreset;
    displaySaveIndex();
    saveMode = true;
}

void PolyAnalogCore::displaySaveIndex() {
    int presetIndex = indexToSaveNewPreset.get();

    intToCString2(presetIndex, numCharBuffer, "Preset: ");
    displayManager->WriteLine(1, numCharBuffer);
}

void PolyAnalogCore::displayValuesOnScreen() {
    //We will make this later ...
    /*if (!needsToUpdateValue) {
        return;
    }
    if (lastParam) {
        float value = lastParam->getUIValue();
        floatToCString2(value, numCharBuffer);
        
        displayManager->WriteLine(2, numCharBuffer);
    
    needsToUpdateValue = false;*/
    return;
}

//Well we should make a loop again
void PolyAnalogCore::updateScreen() {
    int presetIndex = currentPreset.get();

    intToCString2(presetIndex, numCharBuffer, "Preset: ");
    displayManager->WriteLine(0, numCharBuffer);
    
    int index = intParameterMap[currentIntParameterIndex];
    Parameter* param = dspKernel->getParameter(index);
    
    const char* name = param->getName();
    displayManager->WriteLine(1, name);
    
    if (index == PolyAnalogDSP::LfoDestinationA) {
        const char* destName = polySynth.getLfoDestName(0);
        displayManager->WriteLine(2, destName);
    } else if (index == PolyAnalogDSP::LfoDestinationB) {
        const char* destName = polySynth.getLfoDestName(1);
        displayManager->WriteLine(2, destName);
    } else if (index == PolyAnalogDSP::PlayMode) {
        const char* destName = polySynth.getPlayModeName();
        displayManager->WriteLine(2, destName);
    }
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
            if (saveMode && value == 1) {
                saveMode = false;
                updateScreen();
            } else {
                shiftState = (bool)value;
            }
            break;
            
        case ButtonOK: {
            if (saveMode) {
                saveCurrentPreset();
            } else if (shiftState) {
                switchToSaveMode();
            } else {
                switch (intParameterMap[currentIntParameterIndex]) {
                    case PolyAnalogDSP::PlayMode:
                        polySynth.togglePlayMode();
                        break;
                    case PolyAnalogDSP::LfoDestinationA:
                        polySynth.toggleLfoDestination(0);
                        break;
                    case PolyAnalogDSP::LfoDestinationB:
                        polySynth.toggleLfoDestination(1);
                        break;
                        
                    default:
                        break;
                }
                updateScreen();
            }
        }
            break;
            
        case ButtonPrevious: {
            if (saveMode) {
                indexToSaveNewPreset.decrement();
                displaySaveIndex();
            } else if (shiftState) {
                changeCurrentPreset(false);
            } else {
                currentIntParameterIndex = ((currentIntParameterIndex - 1) + intParameterCount) % intParameterCount;
                updateScreen();
            }
        }
            break;
            
        case ButtonNext: {
            if (saveMode) {
                indexToSaveNewPreset.increment();
                displaySaveIndex();
            } else if (shiftState) {
                changeCurrentPreset(true);
            } else {
                currentIntParameterIndex = ((currentIntParameterIndex + 1) + intParameterCount) % intParameterCount;
                updateScreen();
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
