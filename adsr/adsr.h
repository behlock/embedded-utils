#ifndef ADSR_H
#define ADSR_H
#include "../synth/detail.h"

namespace Synth {
enum class EnvelopeState { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };

// Linear ADSR: times are complete segment durations, even after retriggering.
// Every nextSample()/apply() call advances one sample. Unsigned audio is centered
// on 128; signed audio is centered on zero. A zero duration takes no samples.
class ADSR {
public:
    explicit ADSR(uint32_t sampleRate = 44100)
        : _sampleRate(detail::sampleRate(sampleRate)), _state(EnvelopeState::IDLE),
          _level(0), _sustainLevel(180), _attackMs(10), _decayMs(50),
          _releaseMs(100), _remaining(0), _samples(0), _step(0),
          _remainder(0), _error(0), _direction(1), _target(0) {}

    void setAttack(uint16_t ms) {
        _attackMs = ms;
        if (_state == EnvelopeState::ATTACK) start(EnvelopeState::ATTACK, 65535, ms);
    }
    void setDecay(uint16_t ms) {
        _decayMs = ms;
        if (_state == EnvelopeState::DECAY) start(EnvelopeState::DECAY, sustain(), ms);
    }
    void setSustain(uint8_t level) {
        _sustainLevel = level;
        if (_state == EnvelopeState::SUSTAIN) _level = sustain();
        if (_state == EnvelopeState::DECAY) start(EnvelopeState::DECAY, sustain(), _decayMs);
    }
    void setRelease(uint16_t ms) {
        _releaseMs = ms;
        if (_state == EnvelopeState::RELEASE) start(EnvelopeState::RELEASE, 0, ms);
    }
    // Retrigger from the current level; repeated noteOff calls do not restart release.
    void noteOn() { start(EnvelopeState::ATTACK, 65535, _attackMs); }
    void noteOff() {
        if (_state != EnvelopeState::IDLE && _state != EnvelopeState::RELEASE)
            start(EnvelopeState::RELEASE, 0, _releaseMs);
    }
    uint8_t nextSample() {
        for (;;) {
            switch (_state) {
            case EnvelopeState::IDLE: return 0;
            case EnvelopeState::SUSTAIN: return getLevel();
            case EnvelopeState::ATTACK: {
                const bool consumedSample = _remaining != 0;
                if (_remaining) advance();
                if (_remaining) return getLevel();
                _level = 65535;
                start(EnvelopeState::DECAY, sustain(), _decayMs);
                if (consumedSample && _remaining) return getLevel();
                break;
            }
            case EnvelopeState::DECAY:
                if (_remaining) advance();
                if (_remaining) return getLevel();
                _level = sustain();
                _state = EnvelopeState::SUSTAIN;
                return getLevel();
            case EnvelopeState::RELEASE:
                if (_remaining) advance();
                if (!_remaining) reset();
                return getLevel();
            }
        }
    }
    uint8_t apply(uint8_t sample) {
        int16_t centered = static_cast<int16_t>(sample) - 128;
        return static_cast<uint8_t>(128 + centered * nextSample() / 255);
    }
    int8_t applySigned(int8_t sample) {
        return static_cast<int8_t>(static_cast<int16_t>(sample) * nextSample() / 255);
    }
    void reset() { _state = EnvelopeState::IDLE; _level = 0; _remaining = 0; }
    EnvelopeState getState() const { return _state; }
    uint8_t getLevel() const { return static_cast<uint8_t>(_level >> 8); }
    bool isActive() const { return _state != EnvelopeState::IDLE; }
    uint16_t getAttack() const { return _attackMs; }
    uint16_t getDecay() const { return _decayMs; }
    uint8_t getSustain() const { return _sustainLevel; }
    uint16_t getRelease() const { return _releaseMs; }
private:
    uint16_t sustain() const { return static_cast<uint16_t>(_sustainLevel) * 257U; }
    void start(EnvelopeState state, uint16_t target, uint16_t ms) {
        _state = state;
        _target = target;
        uint64_t count = (static_cast<uint64_t>(ms) * _sampleRate + 999) / 1000;
        _samples = count > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(count);
        _remaining = _level == target ? 0 : _samples;
        int32_t distance = static_cast<int32_t>(target) - _level;
        _direction = distance < 0 ? -1 : 1;
        uint32_t magnitude = distance < 0 ? -distance : distance;
        _step = _samples ? static_cast<int32_t>(magnitude / _samples) * _direction : 0;
        _remainder = _samples ? magnitude % _samples : 0;
        _error = 0;
    }
    void advance() {
        int32_t level = static_cast<int32_t>(_level) + _step;
        // Distribute the fractional increment without division in the sample loop.
        if (_error >= _samples - _remainder) {
            _error -= _samples - _remainder;
            level += _direction;
        } else {
            _error += _remainder;
        }
        _level = static_cast<uint16_t>(level);
        if (--_remaining == 0) _level = _target;
    }
    uint32_t _sampleRate;
    EnvelopeState _state;
    uint16_t _level;
    uint8_t _sustainLevel;
    uint16_t _attackMs, _decayMs, _releaseMs;
    uint32_t _remaining, _samples;
    int32_t _step;
    uint32_t _remainder, _error;
    int8_t _direction;
    uint16_t _target;
};

// Utility function to get state name
inline const char* stateName(EnvelopeState state) {
    switch (state) {
        case EnvelopeState::IDLE:    return "Idle";
        case EnvelopeState::ATTACK:  return "Attack";
        case EnvelopeState::DECAY:   return "Decay";
        case EnvelopeState::SUSTAIN: return "Sustain";
        case EnvelopeState::RELEASE: return "Release";
        default:                     return "Unknown";
    }
}

} // namespace Synth

#endif // ADSR_H
