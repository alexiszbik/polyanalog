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
    {HighPass,        "HighPass"},
    
    {LfoTypeA,          "LfoTypeA"},
    {LfoDestinationA,   "LfoDestinationA"},
    {LfoRateA,          "LfoRateA"},
    {LfoAmountA,        "LfoAmountA"},
    
    {LfoTypeB,          "LfoTypeB"},
    {LfoDestinationB,   "LfoDestinationB"},
    {LfoRateB,          "LfoRateB"},
    {LfoAmountB,        "LfoAmountB"}
    
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
    
    lfo[0].init(sampleRate);
    lfo[1].init(sampleRate);
    
    hpFilter.Init(sampleRate);
    hpFilter.SetHighpass(10);
    
    lfo[0].setDestinationValue(0.25);
    lfo[1].setDestinationValue(0.75);
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
            synth.setWaveform(0, value);
            break;
        case OscOctaveA :
            synth.setOctave(valueMap(value, -2, 2));
            break;
        case OscWaveformB :
            synth.setWaveform(1, value);
            break;
        case OscTuneB :
            synth.setOscBTune(valueMap(value, 0, SynthVoice::btuneCount - 1));
            break;
        case OscNoise :
            synth.setNoiseMix(value);
            break;
        case OscMix :
            synth.setOscMix(value);
            break;
        case FilterCutoff :
            synth.setFilterMidiFreq((value * 120.f) + 15.f);
            break;
        case HighPass :
            hpFilter.SetHighpass(fast_mtof((value * 120.f) + 15.f));
            break;
        case FilterRes : {
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

const char* PolyAnalogDSP::getLfoDestName(int lfoIdx) {
    auto dest = lfo[lfoIdx].getDestination();
    return lfo[lfoIdx].destinationNames[dest];
}

float PolyAnalogDSP::getLfoBuffer(int lfoIdx, Lfo::LfoDest target, uint8_t frame, float multiplier) {
    return lfo[lfoIdx].getBuffer(target, frame, multiplier);
}

void PolyAnalogDSP::process(float** buf, int frameCount) {
    DSPKernel::process(buf, frameCount);
    //TODO : move everything to updateParameter function
    synth.setGlide(getValue(Glide));
    
    lfo[0].setRate(getValue(LfoRateA));
    lfo[0].setAmount(getValue(LfoAmountA));
    lfo[0].process(frameCount);
    
    lfo[1].setRate(getValue(LfoRateB));
    lfo[1].setAmount(getValue(LfoAmountB));
    lfo[1].process(frameCount);
    
    float decay = valueMapPow3(getValue(Decay), 0.005f, 8.f);
    
    synth.setADSR(valueMapPow3(getValue(Attack), 0.002f, 16.f),
                  decay,
                  valueMap(getValue(Sustain), 0.f, 1.f),
                  decay);
    
    synth.preprare();
    
    for (int i = 0; i < frameCount; i++) {
        //updateParameters(); // useless only for smoothed parameters
        
        float volume = getValue(Volume); 

        synth.setPitchLfo(getLfoBuffer(0, Lfo::LfoDest_Pitch, i) + getLfoBuffer(1, Lfo::LfoDest_Pitch, i));
        synth.setFilterLfo(getLfoBuffer(0, Lfo::LfoDest_FilterCutoff, i) + getLfoBuffer(1, Lfo::LfoDest_FilterCutoff, i));
        
        float out = synth.process() * volume;
        
        out = hpFilter.Process(out);
       
        buf[0][i] = SoftClip(out * 0.333);
        for (int channel = 1; channel < channelCount; channel++) {
            buf[channel][i] = buf[0][i];
        }
    }
}
