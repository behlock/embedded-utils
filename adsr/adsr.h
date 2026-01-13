#ifndef ADSR_H
#define ADSR_H

#include <Arduino.h>

namespace Synth {

// ADSR Envelope Generator
// Attack-Decay-Sustain-Release envelope for amplitude shaping
//
// Typical usage:
//   ADSR env(44100);
//   env.setAttack(50);    // 50ms attack
//   env.setDecay(100);    // 100ms decay
//   env.setSustain(180);  // ~70% sustain level (0-255)
//   env.setRelease(200);  // 200ms release
//
//   env.noteOn();         // Trigger envelope
//   uint8_t amplitude = env.nextSample();  // Get current level
//   env.noteOff();        // Begin release phase

enum class EnvelopeState {
    IDLE,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
};

class ADSR {
public:
    ADSR(uint32_t sampleRate = 44100)
        : _sampleRate(sampleRate)
        , _state(EnvelopeState::IDLE)
        , _level(0)
        , _attackRate(0)
        , _decayRate(0)
        , _sustainLevel(180)
        , _releaseRate(0)
    {
        setAttack(10);    // 10ms default
        setDecay(50);     // 50ms default
        setRelease(100);  // 100ms default
    }

    // Set attack time in milliseconds
    void setAttack(uint16_t ms) {
        _attackMs = ms;
        if (ms == 0) {
            _attackRate = 65535;
        } else {
            uint32_t samples = ((uint32_t)ms * _sampleRate) / 1000;
            _attackRate = 65535 / samples;
        }
    }

    // Set decay time in milliseconds
    void setDecay(uint16_t ms) {
        _decayMs = ms;
        if (ms == 0) {
            _decayRate = 65535;
        } else {
            uint32_t samples = ((uint32_t)ms * _sampleRate) / 1000;
            _decayRate = 65535 / samples;
        }
    }

    // Set sustain level (0-255)
    void setSustain(uint8_t level) {
        _sustainLevel = level;
    }

    // Set release time in milliseconds
    void setRelease(uint16_t ms) {
        _releaseMs = ms;
        if (ms == 0) {
            _releaseRate = 65535;
        } else {
            uint32_t samples = ((uint32_t)ms * _sampleRate) / 1000;
            _releaseRate = 65535 / samples;
        }
    }

    // Trigger the envelope (note pressed)
    void noteOn() {
        _state = EnvelopeState::ATTACK;
        // Optionally reset level for new attack, or continue from current
        // _level = 0; // Uncomment for hard reset
    }

    // Release the envelope (note released)
    void noteOff() {
        if (_state != EnvelopeState::IDLE) {
            _state = EnvelopeState::RELEASE;
            _releaseStartLevel = _level;
        }
    }

    // Get next envelope sample (0-255)
    uint8_t nextSample() {
        switch (_state) {
            case EnvelopeState::IDLE:
                _level = 0;
                break;

            case EnvelopeState::ATTACK:
                _level += _attackRate;
                if (_level >= 65535) {
                    _level = 65535;
                    _state = EnvelopeState::DECAY;
                }
                break;

            case EnvelopeState::DECAY:
                {
                    uint16_t sustainLevel16 = (uint16_t)_sustainLevel << 8;
                    if (_level > sustainLevel16 + _decayRate) {
                        _level -= _decayRate;
                    } else {
                        _level = sustainLevel16;
                        _state = EnvelopeState::SUSTAIN;
                    }
                }
                break;

            case EnvelopeState::SUSTAIN:
                // Level stays at sustain until noteOff
                break;

            case EnvelopeState::RELEASE:
                if (_level > _releaseRate) {
                    _level -= _releaseRate;
                } else {
                    _level = 0;
                    _state = EnvelopeState::IDLE;
                }
                break;
        }

        return _level >> 8; // Return top 8 bits
    }

    // Apply envelope to a sample
    uint8_t apply(uint8_t sample) {
        uint8_t envLevel = nextSample();
        return ((uint16_t)sample * envLevel) >> 8;
    }

    // Apply envelope to signed sample
    int8_t applySigned(int8_t sample) {
        uint8_t envLevel = nextSample();
        return ((int16_t)sample * envLevel) >> 8;
    }

    void reset() {
        _state = EnvelopeState::IDLE;
        _level = 0;
    }

    EnvelopeState getState() const { return _state; }
    uint8_t getLevel() const { return _level >> 8; }
    bool isActive() const { return _state != EnvelopeState::IDLE; }

    uint16_t getAttack() const { return _attackMs; }
    uint16_t getDecay() const { return _decayMs; }
    uint8_t getSustain() const { return _sustainLevel; }
    uint16_t getRelease() const { return _releaseMs; }

private:
    uint32_t _sampleRate;
    EnvelopeState _state;
    uint16_t _level;           // 16-bit internal resolution
    uint16_t _attackRate;
    uint16_t _decayRate;
    uint8_t _sustainLevel;
    uint16_t _releaseRate;
    uint16_t _releaseStartLevel;

    // Store original ms values for getters
    uint16_t _attackMs;
    uint16_t _decayMs;
    uint16_t _releaseMs;
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
