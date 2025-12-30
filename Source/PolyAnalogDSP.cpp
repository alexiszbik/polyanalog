/*
  ==============================================================================

    PolyAnalogDSP.cpp
    Created: 8 Nov 2023 4:51:13pm
    Author:  Alexis ZBIK

  ==============================================================================
*/

#include "PolyAnalogDSP.h"


#define DECLARE_LFO(_name, _label) \
{LfoType##_name,        _label " Lfo Type",        false}, \
{LfoDestination##_name, _label " Lfo Destination", false}, \
{LfoRate##_name,        _label " Lfo Rate",        false}, \
{LfoAmount##_name,      _label " Lfo Amount",      false}

PolyAnalogDSP::PolyAnalogDSP()
: DSPKernel({
    {PlayMode,      "Play Mode"},
    {Glide,         "Glide"},
    
    {Volume,        "Volume"},
    
    {OscWaveformA,      "OscWaveformA"},
    {OscOctaveA,        "OscOctaveA"},
    
    {OscWaveformB,    "OscWaveformB"},
    {OscTuneB,        "OscTuneB"},
    {OscPwB,        "OscPWB"},
    
    {OscMix,        "OscMix"},
    
    {FilterCutoff,     "FilterCutoff"},
    {FilterRes,        "FilterRes"},
    {FilterEnv,        "FilterEnv"},
    
    {Attack,        "Attack"},
    {Decay,          "Decay"},
    {Sustain,        "Sustain"},
    {Release,        "Release"},
    
    DECLARE_LFO(A,"A"),
    DECLARE_LFO(B,"B"),
    
    {EnvDestination,    "EnvDestination",   false},
    {EnvAttack,         "EnvAttack",        false},
    {EnvDecay,          "EnvDecay",         false},
    {EnvAmount,         "EnvAmount",        false},
    
}){
#if defined _SIMULATOR_
    std::cout << getParameterCount() << " parameters" << std::endl;
    
#endif
}

PolyAnalogDSP::~PolyAnalogDSP() {
}

void PolyAnalogDSP::init(int channelCount, double sampleRate) {
    DSPKernel::init(channelCount, sampleRate);

    synth.init(sampleRate);
    
    uint8_t k = lfoCount;
    while (k--) {
        lfo[k].init(sampleRate);
    }
}

void PolyAnalogDSP::processMIDI(MIDIMessageType messageType, int channel, int dataA, int dataB) {
    DSPKernel::processMIDI(messageType, channel, dataA, dataB);
    
    switch (messageType) {
        case MIDIMessageType::kNoteOn : {
            synth.setNote(true, Note(dataA, dataB, timeStamp++));
        }
            break;
        case MIDIMessageType::kNoteOff : {
            synth.setNote(false, Note(dataA, 0, 0));
        }
            break;
        case MIDIMessageType::kControlChange : {
            if (dataA == 1 /* mod wheel */) {
                synth.setModWheel(dataB/127.f);
            } else {
                int parameterIndex = dataA - MIDI_CC_START;
                if (parameterIndex >= 0 && parameterIndex < getParameterCount()) {
                    setParameterValue(parameterIndex,dataB);
                }
            }
        }
            break;
        case MIDIMessageType::kPitchBend : {
            static const int midPB = 8192;
            float pitchBend;
            if (dataA < midPB) {
                pitchBend = -(1.f-(dataA/(float)midPB))*2.f;
            } else if (dataA > midPB) {
                pitchBend = ((dataA - midPB)/(16383.f - midPB))*2.f;
            } else {
                pitchBend = 0;
            }
            synth.setPitchBend(pitchBend);
        }
            break;
        default:
            break;
    }
}

void PolyAnalogDSP::updateParameter(int index, float value) {
    auto param = static_cast<Parameters>(index);
    switch (param) {
        case PlayMode :
            synth.setPolyMode(static_cast<PolySynth::EPolyMode>(valueMap(value, 0, 2)));
            break;
        case OscWaveformA :
            synth.setWaveform(valueMap(value, 0, SynthVoice::wfCount - 1), 0);
            break;
        case OscOctaveA :
            synth.setOctave(valueMap(value, -2, 2));
            break;
        case OscWaveformB :
            synth.setWaveform(valueMap(value, 0, SynthVoice::wfCount - 1), 1);
            break;
        case OscTuneB :
            synth.setOscBTune(valueMap(value, 0, SynthVoice::btuneCount - 1));
            break;
        case OscPwB :
            synth.setOscBPW(value);
            break;
        case OscMix :
            synth.setOscMix(value);
            break;
        case FilterCutoff :
            synth.setFilterMidiFreq((value * 115.f) + 20.f);
            break;
        case FilterRes : {
                constexpr float Qmin = 0.5f;
                constexpr float Qmax = 6.0f;
                float lnRatio = std::log(Qmax / Qmin); // calculé à la compile
                
                float qvalue = std::exp(value * lnRatio);
                synth.setFilterRes(qvalue);
            }
            break;
        case FilterEnv :
            synth.setFilterEnv(value);
            break;
        case LfoDestinationA:
            lfo[0].setDestinationValue(value);
            break;
        case LfoDestinationB:
            lfo[1].setDestinationValue(value);
            break;
        default:
            break;
    }
}

int PolyAnalogDSP::getLfoParam(int lfoId, int aParam) {
    static int lfoParamCount = (LfoAmountA - LfoTypeA) + 1;
    return (lfoId * lfoParamCount) + aParam;
}

const char* PolyAnalogDSP::getLfoDestName(int lfoIdx) {
    auto dest = lfo[lfoIdx].getDestination();
    return lfo[lfoIdx].destinationNames[dest];
}

float PolyAnalogDSP::getLfoBuffer(Lfo::LfoDest target, uint8_t frame, float multiplier) {
    return lfo[0].getBuffer(target, frame, multiplier) + lfo[1].getBuffer(target, frame, multiplier);
}

void PolyAnalogDSP::process(float** buf, int frameCount) {
    DSPKernel::process(buf, frameCount);
    
    synth.setGlide(getValue(Glide));
    
    uint8_t k = lfoCount;
    while (k--) {
        lfo[k].setRate(getValue(getLfoParam(k, LfoRateA)));
        lfo[k].setAmount(getValue(getLfoParam(k, LfoAmountA)));
        lfo[k].process(frameCount);
    }
    float envAttack = getValue(EnvAttack);
    float envDecay = getValue(EnvDecay);
    
    float attack = valueMap(envAttack*envAttack, 0.0001f, 2.f);
    float decay = valueMap(envDecay*envDecay, 0.0001f, 2.f);
    
    synth.setEnvParameters(attack, decay, getValue(EnvAmount));
    
    synth.setADSR(valueMapPow3(getValue(Attack), 0.002f, 6.f),
                  valueMapPow3(getValue(Decay), 0.002f, 2.f),
                  valueMap(getValue(Sustain), 0.f, 1.f),
                  valueMapPow3(getValue(Release), 0.002, 16.f));
    
    synth.preprare();
    
    for (int i = 0; i < frameCount; i++) {
        //updateParameters(); // useless only for smoothed parameters
        
        float volume = getValue(Volume); 

        //If we apply lfo to pitch
        synth.setTune(getLfoBuffer(Lfo::LfoDest_Pitch, i, 24));
        
        float out = synth.process() * volume;
       
        buf[0][i] = SoftClip(out * 0.333);
        for (int channel = 1; channel < channelCount; channel++) {
            buf[channel][i] = buf[0][i];
        }
    }
}
