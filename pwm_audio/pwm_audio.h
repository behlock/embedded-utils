#ifndef PWM_AUDIO_H
#define PWM_AUDIO_H
#include <Arduino.h>
#include <avr/interrupt.h>

#if !defined(__AVR_ATmega328P__) && !defined(__AVR_ATmega168__) && \
    !defined(__AVR_ATmega2560__) && !defined(__AVR_ATmega1280__)
#error "PWMAudio supports classic Uno/Nano (ATmega328P/168) and Mega (ATmega2560/1280) only"
#endif

namespace Synth {
// Exclusive ownership of the selected timer is required (no Servo/tone/analogWrite
// on that timer). Output is always 8-bit PWM at F_CPU/256, not the sample rate.
// Feed a suitable reconstruction filter and AC coupling/buffer for the audio load.
// For example, 1k/10nF is a ~16kHz RC pole; 1k/10uF would remove most audio.
class PWMAudio {
public:
    enum OutputPin {
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
        TIMER1_PIN_11 = 11, TIMER1_PIN_12 = 12,
        TIMER2_PIN_9 = 9, TIMER2_PIN_10 = 10,
        TIMER1_A = TIMER1_PIN_11, TIMER1_B = TIMER1_PIN_12,
        TIMER2_A = TIMER2_PIN_10, TIMER2_B = TIMER2_PIN_9
#else
        TIMER1_PIN_9 = 9, TIMER1_PIN_10 = 10,
        TIMER2_PIN_3 = 3, TIMER2_PIN_11 = 11,
        TIMER1_A = TIMER1_PIN_9, TIMER1_B = TIMER1_PIN_10,
        TIMER2_A = TIMER2_PIN_11, TIMER2_B = TIMER2_PIN_3
#endif
    };
    PWMAudio() : _pin(TIMER2_A), _sampleRate(22050), _initialized(false) {}
    PWMAudio(const PWMAudio&) = delete;
    PWMAudio& operator=(const PWMAudio&) = delete;
    bool begin(OutputPin pin = TIMER2_A) {
        if (pin != TIMER1_A && pin != TIMER1_B && pin != TIMER2_A && pin != TIMER2_B)
            return false;
        uint8_t saved = SREG;
        cli();
        end();
        _pin = pin;
        pinMode(_pin, OUTPUT);
        if (isTimer1()) {
            TIMSK1 = 0;
            TCCR1A = 0;
            TCCR1B = 0;
            TCNT1 = 0;
            TIFR1 = 0xFF; // Clear pending interrupts before taking ownership.
            OCR1A = OCR1B = 128;
            TCCR1A = _BV(WGM10) | (_pin == TIMER1_A ? _BV(COM1A1) : _BV(COM1B1));
            TCCR1B = _BV(WGM12) | _BV(CS10);
        } else {
            TIMSK2 = 0;
            TCCR2A = 0;
            TCCR2B = 0;
            TCNT2 = 0;
            TIFR2 = 0xFF;
            OCR2A = OCR2B = 128;
            TCCR2A = _BV(WGM21) | _BV(WGM20)
                | (_pin == TIMER2_A ? _BV(COM2A1) : _BV(COM2B1));
            TCCR2B = _BV(CS20);
        }
        _initialized = true;
        SREG = saved;
        return true;
    }
    void write(uint8_t sample) {
        if (!_initialized) return;
        if (_pin == TIMER1_A) OCR1A = sample;
        else if (_pin == TIMER1_B) OCR1B = sample;
        else if (_pin == TIMER2_A) OCR2A = sample;
        else OCR2B = sample;
    }
    // Accept normalized unsigned 16-bit input; reduce to the hardware's 8 bits.
    void write16(uint16_t sample) { write(static_cast<uint8_t>(sample >> 8)); }
    // Polling metadata only: callers must supply their own sample clock.
    bool setSampleRate(uint32_t rate) {
        if (rate == 0 || rate > 1000000UL) return false;
        _sampleRate = rate;
        return true;
    }
    uint32_t getSampleRate() const { return _sampleRate; }
    uint32_t getSampleDelayMicros() const { return 1000000UL / _sampleRate; }
    void end() {
        if (!_initialized) return;
        uint8_t saved = SREG;
        cli();
        if (isTimer1()) { TIMSK1 = 0; TCCR1A = 0; TCCR1B = 0; }
        else { TIMSK2 = 0; TCCR2A = 0; TCCR2B = 0; }
        digitalWrite(_pin, LOW);
        _initialized = false;
        SREG = saved;
    }
private:
    bool isTimer1() const { return _pin == TIMER1_A || _pin == TIMER1_B; }
    OutputPin _pin;
    uint32_t _sampleRate;
    bool _initialized;
};

// Owns both Timer1 (sample clock) and Timer2 (PWM). Do not use with PWMAudio or
// another timer owner. Supply the global TIMER1_COMPA ISR shown below exactly once.
class PWMAudioISR {
public:
    typedef uint8_t (*SampleCallback)();
    static PWMAudioISR& instance() { static PWMAudioISR inst; return inst; }
    bool begin(uint32_t sampleRate, SampleCallback callback) {
        if (!callback || sampleRate == 0 || sampleRate > F_CPU) return false;
        uint32_t ticks = (F_CPU + sampleRate / 2) / sampleRate;
        if (ticks == 0 || ticks > 65536UL) return false;
        uint8_t saved = SREG;
        cli();
        _callback = callback;
        _sampleRate = F_CPU / ticks;
        pinMode(PWMAudio::TIMER2_A, OUTPUT);
        TIMSK1 = TIMSK2 = 0;
        TCCR1A = TCCR1B = TCCR2A = TCCR2B = 0;
        TCNT1 = TCNT2 = 0;
        TIFR1 = TIFR2 = 0xFF;
        OCR2A = 128;
        TCCR2A = _BV(COM2A1) | _BV(WGM21) | _BV(WGM20);
        TCCR2B = _BV(CS20);
        OCR1A = static_cast<uint16_t>(ticks - 1);
        TCCR1B = _BV(WGM12) | _BV(CS10);
        TIMSK1 = _BV(OCIE1A);
        _initialized = true;
        SREG = saved;
        return true;
    }
    void end() {
        if (!_initialized) return;
        uint8_t saved = SREG;
        cli();
        TIMSK1 = TIMSK2 = 0;
        TCCR1A = TCCR1B = TCCR2A = TCCR2B = 0;
        _callback = nullptr;
        _initialized = false;
        digitalWrite(PWMAudio::TIMER2_A, LOW);
        SREG = saved;
    }
    uint32_t getSampleRate() const { return _sampleRate; }
    void handleInterrupt() { if (_callback) OCR2A = _callback(); }
private:
    PWMAudioISR() : _callback(nullptr), _sampleRate(0), _initialized(false) {}
    PWMAudioISR(const PWMAudioISR&) = delete;
    PWMAudioISR& operator=(const PWMAudioISR&) = delete;
    SampleCallback volatile _callback;
    uint32_t _sampleRate;
    bool _initialized;
};
} // namespace Synth

// In the main sketch, with #include <avr/interrupt.h>:
// ISR(TIMER1_COMPA_vect) { Synth::PWMAudioISR::instance().handleInterrupt(); }
#endif
