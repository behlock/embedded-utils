#ifndef LFO_H
#define LFO_H

#include <Arduino.h>
#include "oscillator.h"

namespace Synth {

// Low Frequency Oscillator for modulation effects
// Typical uses: vibrato (pitch), tremolo (amplitude), filter sweep, PWM
//
// Example for vibrato:
//   LFO lfo(44100);
//   lfo.setRate(5.0);       // 5 Hz vibrato rate
//   lfo.setDepth(10);       // Modulation depth
//   lfo.setWaveform(Waveform::SINE);
//
//   float modulation = lfo.nextSampleBipolar();  // -1.0 to 1.0
//   float pitchMod = baseFreq * (1.0 + modulation * 0.02);  // +/- 2% pitch bend

class LFO {
public:
    LFO(uint32_t sampleRate = 44100)
        : _sampleRate(sampleRate)
        , _rate(1.0f)
        , _depth(127)
        , _phase(0)
        , _phaseIncrement(0)
        , _waveform(Waveform::SINE)
    {
        setRate(_rate);
    }

    // Set LFO rate in Hz (typically 0.1 - 20 Hz)
    void setRate(float hz) {
        _rate = hz;
        // Using 24-bit fixed point for very low frequencies
        _phaseIncrement = (uint32_t)((hz * 16777216.0f) / _sampleRate);
    }

    // Set modulation depth (0-255)
    void setDepth(uint8_t depth) {
        _depth = depth;
    }

    void setWaveform(Waveform wf) {
        _waveform = wf;
    }

    void setSampleRate(uint32_t rate) {
        _sampleRate = rate;
        setRate(_rate);
    }

    // Returns unipolar sample (0-255)
    uint8_t nextSample() {
        uint8_t sample = 0;
        uint8_t phaseIndex = _phase >> 16; // Top 8 bits of 24-bit phase

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
                sample = (phaseIndex < 64) ? 255 : 0; // 25% duty for LFO
                break;
        }

        _phase += _phaseIncrement;
        if (_phase >= 16777216) _phase -= 16777216;

        // Scale by depth
        return ((uint16_t)sample * _depth) >> 8;
    }

    // Returns bipolar sample (-128 to 127), scaled by depth
    int8_t nextSampleBipolar() {
        return (int8_t)(nextSample() - (_depth >> 1));
    }

    // Returns floating point bipolar (-1.0 to 1.0), scaled by depth
    float nextSampleFloat() {
        int16_t bipolar = (int16_t)nextSample() - 128;
        return (float)bipolar / 128.0f * ((float)_depth / 255.0f);
    }

    // Modulate a frequency value (for vibrato)
    // Returns frequency multiplied by (1.0 + lfo_output * depth)
    float modulateFrequency(float baseFreq, float maxDeviation = 0.05f) {
        float mod = nextSampleFloat() * maxDeviation;
        return baseFreq * (1.0f + mod);
    }

    // Modulate amplitude (for tremolo)
    // Returns amplitude scaled by LFO
    uint8_t modulateAmplitude(uint8_t baseAmp) {
        uint8_t lfoVal = nextSample();
        // Mix: base amplitude modulated by LFO
        // At depth=0, no modulation. At depth=255, full 0-to-base modulation
        uint16_t modAmount = 255 - (((255 - lfoVal) * _depth) >> 8);
        return ((uint16_t)baseAmp * modAmount) >> 8;
    }

    void reset() {
        _phase = 0;
    }

    void sync() {
        _phase = 0;
    }

    float getRate() const { return _rate; }
    uint8_t getDepth() const { return _depth; }
    Waveform getWaveform() const { return _waveform; }

private:
    uint32_t _sampleRate;
    float _rate;
    uint8_t _depth;
    uint32_t _phase;         // 24-bit phase accumulator for low frequencies
    uint32_t _phaseIncrement;
    Waveform _waveform;
};

} // namespace Synth

#endif // LFO_H
