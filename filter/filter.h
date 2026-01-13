#ifndef FILTER_H
#define FILTER_H

#include <Arduino.h>

namespace Synth {

// Digital Filter Implementations for Synthesizers
//
// Common filter types:
// - Low-pass: Removes high frequencies (muffled sound)
// - High-pass: Removes low frequencies (thin sound)
// - Band-pass: Keeps frequencies in a range
// - Resonant: Emphasis at cutoff frequency (classic synth sound)

// Simple one-pole low-pass filter
// Very efficient, good for basic smoothing
class OnePoleFilter {
public:
    OnePoleFilter()
        : _coefficient(128)
        , _lastOutput(0)
    {}

    // Set filter coefficient (0-255)
    // 0 = no filtering (pass-through)
    // 255 = maximum filtering (very smooth)
    void setCoefficient(uint8_t coeff) {
        _coefficient = coeff;
    }

    // Set cutoff as ratio of sample rate (0.0 to 1.0)
    void setCutoff(float ratio) {
        // Approximate coefficient from cutoff ratio
        // Lower ratio = lower cutoff = more filtering
        if (ratio >= 1.0f) {
            _coefficient = 0;
        } else if (ratio <= 0.0f) {
            _coefficient = 255;
        } else {
            _coefficient = (uint8_t)((1.0f - ratio) * 255);
        }
    }

    // Process sample (8-bit)
    uint8_t process(uint8_t input) {
        // output = (input * (256-coeff) + lastOutput * coeff) / 256
        uint16_t temp = (uint16_t)input * (256 - _coefficient);
        temp += (uint16_t)_lastOutput * _coefficient;
        _lastOutput = temp >> 8;
        return _lastOutput;
    }

    // Process signed sample
    int8_t processSigned(int8_t input) {
        int16_t temp = (int16_t)input * (256 - _coefficient);
        temp += (int16_t)_lastOutputSigned * _coefficient;
        _lastOutputSigned = temp >> 8;
        return _lastOutputSigned;
    }

    void reset() {
        _lastOutput = 0;
        _lastOutputSigned = 0;
    }

private:
    uint8_t _coefficient;
    uint8_t _lastOutput;
    int8_t _lastOutputSigned;
};

// State Variable Filter (SVF)
// Provides low-pass, high-pass, and band-pass outputs simultaneously
// Supports resonance for classic synthesizer sounds
class StateVariableFilter {
public:
    StateVariableFilter(uint32_t sampleRate = 44100)
        : _sampleRate(sampleRate)
        , _cutoff(1000.0f)
        , _resonance(0.5f)
        , _low(0)
        , _band(0)
        , _high(0)
    {
        calculateCoefficients();
    }

    void setCutoff(float freq) {
        _cutoff = constrain(freq, 20.0f, _sampleRate / 2.0f);
        calculateCoefficients();
    }

    // Resonance: 0.0 to 1.0 (higher = more resonant peak)
    void setResonance(float res) {
        _resonance = constrain(res, 0.0f, 0.99f);
        calculateCoefficients();
    }

    void setSampleRate(uint32_t rate) {
        _sampleRate = rate;
        calculateCoefficients();
    }

    // Process sample and update all outputs
    void process(int16_t input) {
        // State variable filter algorithm
        _low += _f * _band;
        _high = _scale * input - _low - _q * _band;
        _band += _f * _high;

        // Clamp to prevent overflow
        _low = constrain(_low, -32768, 32767);
        _band = constrain(_band, -32768, 32767);
        _high = constrain(_high, -32768, 32767);
    }

    // Get outputs after processing
    int16_t lowPass() const { return _low; }
    int16_t highPass() const { return _high; }
    int16_t bandPass() const { return _band; }

    // Notch (band-reject) = low + high
    int16_t notch() const { return _low + _high; }

    // Convenience methods for 8-bit audio
    uint8_t lowPass8() const {
        return constrain((_low >> 8) + 128, 0, 255);
    }

    uint8_t highPass8() const {
        return constrain((_high >> 8) + 128, 0, 255);
    }

    uint8_t bandPass8() const {
        return constrain((_band >> 8) + 128, 0, 255);
    }

    void reset() {
        _low = 0;
        _band = 0;
        _high = 0;
    }

    float getCutoff() const { return _cutoff; }
    float getResonance() const { return _resonance; }

private:
    void calculateCoefficients() {
        // f = 2 * sin(pi * cutoff / sampleRate)
        _f = 2.0f * sin(PI * _cutoff / _sampleRate);

        // q = 1/Q = damping factor
        // Higher resonance = lower damping
        _q = 2.0f - 2.0f * _resonance;

        // Scale factor for input
        _scale = sqrt(_q);
    }

    uint32_t _sampleRate;
    float _cutoff;
    float _resonance;
    float _f;      // Frequency coefficient
    float _q;      // Damping coefficient
    float _scale;  // Input scaling

    int16_t _low;
    int16_t _band;
    int16_t _high;
};

// Moog-style ladder filter approximation
// 4-pole (24dB/octave) low-pass with resonance
// Classic analog synthesizer sound
class MoogFilter {
public:
    MoogFilter(uint32_t sampleRate = 44100)
        : _sampleRate(sampleRate)
        , _cutoff(1000.0f)
        , _resonance(0.0f)
    {
        reset();
        calculateCoefficients();
    }

    void setCutoff(float freq) {
        _cutoff = constrain(freq, 20.0f, _sampleRate / 2.5f);
        calculateCoefficients();
    }

    // Resonance 0.0 to 1.0 (can self-oscillate near 1.0)
    void setResonance(float res) {
        _resonance = constrain(res, 0.0f, 1.0f);
        calculateCoefficients();
    }

    void setSampleRate(uint32_t rate) {
        _sampleRate = rate;
        calculateCoefficients();
    }

    int16_t process(int16_t input) {
        float in = input / 32768.0f;

        // Feedback with resonance
        in -= _resonance * _stage[3];

        // Four cascaded one-pole filters
        _stage[0] += _p * (tanh(in) - tanh(_stage[0]));
        _stage[1] += _p * (tanh(_stage[0]) - tanh(_stage[1]));
        _stage[2] += _p * (tanh(_stage[1]) - tanh(_stage[2]));
        _stage[3] += _p * (tanh(_stage[2]) - tanh(_stage[3]));

        return (int16_t)(_stage[3] * 32767.0f);
    }

    uint8_t process8(uint8_t input) {
        int16_t in16 = ((int16_t)input - 128) << 8;
        int16_t out16 = process(in16);
        return (out16 >> 8) + 128;
    }

    void reset() {
        for (int i = 0; i < 4; i++) {
            _stage[i] = 0.0f;
        }
    }

private:
    // Fast tanh approximation
    float tanh(float x) {
        if (x < -3.0f) return -1.0f;
        if (x > 3.0f) return 1.0f;
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    void calculateCoefficients() {
        // Attempt to match analog filter response
        float fc = _cutoff / _sampleRate;
        _p = fc * (1.8f - 0.8f * fc);
    }

    uint32_t _sampleRate;
    float _cutoff;
    float _resonance;
    float _p;           // Pole coefficient
    float _stage[4];    // Filter stages
};

// DC blocking filter
// Removes DC offset from audio signal
class DCBlocker {
public:
    DCBlocker(float coefficient = 0.995f)
        : _R(coefficient)
        , _xPrev(0)
        , _yPrev(0)
    {}

    int16_t process(int16_t input) {
        // y[n] = x[n] - x[n-1] + R * y[n-1]
        float output = input - _xPrev + _R * _yPrev;
        _xPrev = input;
        _yPrev = output;
        return (int16_t)output;
    }

    void reset() {
        _xPrev = 0;
        _yPrev = 0;
    }

private:
    float _R;
    float _xPrev;
    float _yPrev;
};

} // namespace Synth

#endif // FILTER_H
