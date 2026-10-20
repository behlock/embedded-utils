#ifndef LFO_H
#define LFO_H
#include "../oscillator/oscillator.h"

namespace Synth {
// Each output/modulation call advances one sample. Choose one per sample.
// Vibrato: lfo.setDepth(255); float mod = lfo.nextSampleFloat();
//          float frequency = baseFrequency * (1.0f + mod * 0.02f);
class LFO {
public:
    explicit LFO(uint32_t sampleRate = 44100) : _oscillator(sampleRate), _depth(127) {
        _oscillator.setPulseWidth(64);
        setRate(1.0f);
    }
    void setRate(float hz) { _oscillator.setFrequency(hz); }
    void setDepth(uint8_t depth) { _depth = depth; }
    void setWaveform(Waveform wf) { _oscillator.setWaveform(wf); }
    void setSampleRate(uint32_t rate) { _oscillator.setSampleRate(rate); }
    uint8_t nextSample() {
        return static_cast<uint16_t>(_oscillator.nextSample()) * _depth / 255U;
    }
    int8_t nextSampleBipolar() {
        int16_t sample = static_cast<int16_t>(_oscillator.nextSample()) - 128;
        return static_cast<int8_t>(sample * _depth / 255);
    }
    float nextSampleFloat() {
        return (static_cast<int16_t>(_oscillator.nextSample()) - 128) / 128.0f
            * (_depth / 255.0f);
    }
    float modulateFrequency(float baseFreq, float maxDeviation = 0.05f) {
        return baseFreq * (1.0f + nextSampleFloat() * maxDeviation);
    }
    uint8_t modulateAmplitude(uint8_t baseAmp) {
        uint16_t raw = _oscillator.nextSample();
        uint16_t gain = 255U - (255U - raw) * _depth / 255U;
        return static_cast<uint16_t>(baseAmp) * gain / 255U;
    }
    void reset() { _oscillator.reset(); }
    void sync() { reset(); }
    float getRate() const { return _oscillator.getFrequency(); }
    uint8_t getDepth() const { return _depth; }
    Waveform getWaveform() const { return _oscillator.getWaveform(); }
private:
    Oscillator _oscillator;
    uint8_t _depth;
};
} // namespace Synth
#endif
