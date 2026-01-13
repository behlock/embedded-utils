#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <Arduino.h>

namespace Synth {

// Waveform types
enum class Waveform {
    SINE,
    SQUARE,
    SAWTOOTH,
    TRIANGLE,
    PULSE
};

// Pre-computed sine table (256 entries, 8-bit resolution)
// Values range from 0-255 for unsigned output
const uint8_t SINE_TABLE[256] PROGMEM = {
    128,131,134,137,140,143,146,149,152,155,158,162,165,167,170,173,
    176,179,182,185,188,190,193,196,198,201,203,206,208,211,213,215,
    218,220,222,224,226,228,230,232,234,235,237,238,240,241,243,244,
    245,246,248,249,250,250,251,252,253,253,254,254,254,255,255,255,
    255,255,255,255,254,254,254,253,253,252,251,250,250,249,248,246,
    245,244,243,241,240,238,237,235,234,232,230,228,226,224,222,220,
    218,215,213,211,208,206,203,201,198,196,193,190,188,185,182,179,
    176,173,170,167,165,162,158,155,152,149,146,143,140,137,134,131,
    128,124,121,118,115,112,109,106,103,100,97,93,90,88,85,82,
    79,76,73,70,67,65,62,59,57,54,52,49,47,44,42,40,
    37,35,33,31,29,27,25,23,21,20,18,17,15,14,12,11,
    10,9,7,6,5,5,4,3,2,2,1,1,1,0,0,0,
    0,0,0,0,1,1,1,2,2,3,4,5,5,6,7,9,
    10,11,12,14,15,17,18,20,21,23,25,27,29,31,33,35,
    37,40,42,44,47,49,52,54,57,59,62,65,67,70,73,76,
    79,82,85,88,90,93,97,100,103,106,109,112,115,118,121,124
};

class Oscillator {
public:
    Oscillator(uint32_t sampleRate = 44100)
        : _sampleRate(sampleRate)
        , _frequency(440.0f)
        , _phase(0)
        , _phaseIncrement(0)
        , _waveform(Waveform::SINE)
        , _pulseWidth(128) // 50% duty cycle
    {
        setFrequency(_frequency);
    }

    void setFrequency(float freq) {
        _frequency = freq;
        // Phase increment for 16-bit phase accumulator (0-65535)
        _phaseIncrement = (uint16_t)((freq * 65536.0f) / _sampleRate);
    }

    void setWaveform(Waveform wf) {
        _waveform = wf;
    }

    void setPulseWidth(uint8_t pw) {
        _pulseWidth = pw;
    }

    void setSampleRate(uint32_t rate) {
        _sampleRate = rate;
        setFrequency(_frequency);
    }

    // Returns 8-bit sample (0-255)
    uint8_t nextSample() {
        uint8_t sample = 0;
        uint8_t phaseIndex = _phase >> 8; // Top 8 bits of phase

        switch (_waveform) {
            case Waveform::SINE:
                sample = pgm_read_byte(&SINE_TABLE[phaseIndex]);
                break;

            case Waveform::SQUARE:
                sample = (phaseIndex < 128) ? 255 : 0;
                break;

            case Waveform::SAWTOOTH:
                sample = phaseIndex;
                break;

            case Waveform::TRIANGLE:
                sample = (phaseIndex < 128)
                    ? (phaseIndex * 2)
                    : (255 - (phaseIndex - 128) * 2);
                break;

            case Waveform::PULSE:
                sample = (phaseIndex < _pulseWidth) ? 255 : 0;
                break;
        }

        _phase += _phaseIncrement;
        return sample;
    }

    // Returns signed sample (-128 to 127)
    int8_t nextSampleSigned() {
        return (int8_t)(nextSample() - 128);
    }

    void reset() {
        _phase = 0;
    }

    float getFrequency() const { return _frequency; }
    Waveform getWaveform() const { return _waveform; }

private:
    uint32_t _sampleRate;
    float _frequency;
    uint16_t _phase;
    uint16_t _phaseIncrement;
    Waveform _waveform;
    uint8_t _pulseWidth;
};

// Utility function to get waveform name
inline const char* waveformName(Waveform wf) {
    switch (wf) {
        case Waveform::SINE:     return "Sine";
        case Waveform::SQUARE:   return "Square";
        case Waveform::SAWTOOTH: return "Sawtooth";
        case Waveform::TRIANGLE: return "Triangle";
        case Waveform::PULSE:    return "Pulse";
        default:                 return "Unknown";
    }
}

} // namespace Synth

#endif // OSCILLATOR_H
