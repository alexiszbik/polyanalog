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
    {OscNoise,        "OscNoise"},
    
    {OscMix,        "OscMix"},
    
    {FilterCutoff,     "FilterCutoff"},
    {FilterRes,        "FilterRes"},
    {FilterEnv,        "FilterEnv"},
    
    {Attack,         "Attack"},
    {Decay,          "Decay"},
    {Sustain,        "Sustain"},
    {Release,        "Release"},
    
    {LfoType,          "LfoType"},
    {LfoDestination,   "LfoDestination"},
    {LfoRate,          "LfoRate"},
    {LfoAmount,        "LfoAmount"}
    
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
    
    lfo.init(sampleRate);
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

void PolyAnalogDSP::togglePlayMode() {
    auto playModeParam = getParameter(PlayMode);
    float fValue = playModeParam->getValue();
    int iValue = valueMap(fValue, 0, 2);
    iValue = (iValue + 1) % 3;
    setParameterValue(PlayMode, iValue * 0.5f);
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
        case OscNoise :
            //synth.setNoiseLevel(value);
            break;
        case OscMix :
            synth.setOscMix(value);
            break;
        case FilterCutoff :
            synth.setFilterMidiFreq((value * 115.f) + 20.f);
            break;
        case FilterRes : {
                float qvalue = std::exp(value * lnRatio);
                synth.setFilterRes(qvalue);
            }
            break;
        case FilterEnv :
            synth.setFilterEnv(value);
            break;
        case LfoDestination:
            lfo.setDestinationValue(value);
            break;

        default:
            break;
    }
}

const char* PolyAnalogDSP::getLfoDestName() {
    auto dest = lfo.getDestination();
    return lfo.destinationNames[dest];
}

float PolyAnalogDSP::getLfoBuffer(Lfo::LfoDest target, uint8_t frame, float multiplier) {
    return lfo.getBuffer(target, frame, multiplier);
}

void PolyAnalogDSP::process(float** buf, int frameCount) {
    DSPKernel::process(buf, frameCount);
    
    synth.setGlide(getValue(Glide));
    
    lfo.setRate(getValue(LfoRate));
    lfo.setAmount(getValue(LfoAmount));
    lfo.process(frameCount);
    
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
