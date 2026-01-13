#ifndef PWM_AUDIO_H
#define PWM_AUDIO_H

#include <Arduino.h>

namespace Synth {

// PWM Audio Output for Arduino
// Uses Timer1 or Timer2 for high-frequency PWM suitable for audio
//
// PWM frequency should be well above audio range (>20kHz)
// to allow filtering with a simple RC low-pass filter
//
// Hardware setup:
//   Arduino pin -> 1k resistor -> 10uF capacitor -> Audio output
//                                      |
//                                     GND
//
// Usage:
//   PWMAudio audio;
//   audio.begin(PWMAudio::TIMER1_PIN_9);  // Use pin 9
//   audio.setSampleRate(22050);
//   audio.write(sample);  // Write 8-bit sample

class PWMAudio {
public:
    // Available PWM pins by timer
    enum OutputPin {
        TIMER1_PIN_9  = 9,   // OC1A - 16-bit timer
        TIMER1_PIN_10 = 10,  // OC1B - 16-bit timer
        TIMER2_PIN_3  = 3,   // OC2B - 8-bit timer
        TIMER2_PIN_11 = 11   // OC2A - 8-bit timer
    };

    PWMAudio()
        : _pin(TIMER2_PIN_11)
        , _sampleRate(22050)
        , _initialized(false)
    {}

    // Initialize PWM audio output
    void begin(OutputPin pin = TIMER2_PIN_11) {
        _pin = pin;
        pinMode(_pin, OUTPUT);

        if (_pin == TIMER1_PIN_9 || _pin == TIMER1_PIN_10) {
            setupTimer1();
        } else {
            setupTimer2();
        }

        _initialized = true;
    }

    // Write 8-bit sample to PWM output
    void write(uint8_t sample) {
        if (!_initialized) return;

        switch (_pin) {
            case TIMER1_PIN_9:
                OCR1A = sample;
                break;
            case TIMER1_PIN_10:
                OCR1B = sample;
                break;
            case TIMER2_PIN_3:
                OCR2B = sample;
                break;
            case TIMER2_PIN_11:
                OCR2A = sample;
                break;
        }
    }

    // Write 16-bit sample (only for Timer1 pins)
    void write16(uint16_t sample) {
        if (!_initialized) return;

        if (_pin == TIMER1_PIN_9) {
            OCR1A = sample;
        } else if (_pin == TIMER1_PIN_10) {
            OCR1B = sample;
        }
    }

    void setSampleRate(uint32_t rate) {
        _sampleRate = rate;
    }

    uint32_t getSampleRate() const {
        return _sampleRate;
    }

    // Get delay in microseconds for sample rate timing
    uint16_t getSampleDelayMicros() const {
        return 1000000UL / _sampleRate;
    }

    void end() {
        if (_pin == TIMER1_PIN_9 || _pin == TIMER1_PIN_10) {
            TCCR1A = 0;
            TCCR1B = 0;
        } else {
            TCCR2A = 0;
            TCCR2B = 0;
        }
        _initialized = false;
    }

private:
    void setupTimer1() {
        // Timer1: Fast PWM, 8-bit mode
        // PWM frequency = 16MHz / 256 = 62.5 kHz
        cli();

        TCCR1A = 0;
        TCCR1B = 0;

        // Fast PWM, 8-bit (TOP = 0x00FF)
        TCCR1A |= (1 << WGM10);
        TCCR1B |= (1 << WGM12);

        // No prescaler (CS10 = 1)
        TCCR1B |= (1 << CS10);

        if (_pin == TIMER1_PIN_9) {
            // Non-inverting mode on OC1A
            TCCR1A |= (1 << COM1A1);
            OCR1A = 128; // 50% duty cycle initially
        } else {
            // Non-inverting mode on OC1B
            TCCR1A |= (1 << COM1B1);
            OCR1B = 128;
        }

        sei();
    }

    void setupTimer2() {
        // Timer2: Fast PWM mode
        // PWM frequency = 16MHz / 256 = 62.5 kHz
        cli();

        TCCR2A = 0;
        TCCR2B = 0;

        // Fast PWM mode (WGM21:20 = 11)
        TCCR2A |= (1 << WGM21) | (1 << WGM20);

        // No prescaler (CS20 = 1)
        TCCR2B |= (1 << CS20);

        if (_pin == TIMER2_PIN_11) {
            // Non-inverting mode on OC2A
            TCCR2A |= (1 << COM2A1);
            OCR2A = 128;
        } else {
            // Non-inverting mode on OC2B
            TCCR2A |= (1 << COM2B1);
            OCR2B = 128;
        }

        sei();
    }

    OutputPin _pin;
    uint32_t _sampleRate;
    bool _initialized;
};

// Interrupt-driven audio output using Timer1
// More consistent timing than polling
class PWMAudioISR {
public:
    static PWMAudioISR& instance() {
        static PWMAudioISR inst;
        return inst;
    }

    typedef uint8_t (*SampleCallback)();

    void begin(uint32_t sampleRate, SampleCallback callback) {
        _callback = callback;
        _sampleRate = sampleRate;

        pinMode(11, OUTPUT);

        cli();

        // Setup Timer2 for PWM output on pin 11
        TCCR2A = (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
        TCCR2B = (1 << CS20);
        OCR2A = 128;

        // Setup Timer1 for sample rate interrupt
        TCCR1A = 0;
        TCCR1B = (1 << WGM12) | (1 << CS10); // CTC mode, no prescaler

        // Calculate compare value for desired sample rate
        // Compare match = (16MHz / sampleRate) - 1
        OCR1A = (16000000UL / sampleRate) - 1;

        // Enable Timer1 compare match interrupt
        TIMSK1 |= (1 << OCIE1A);

        sei();
    }

    void end() {
        TIMSK1 &= ~(1 << OCIE1A);
        _callback = nullptr;
    }

    // Called from ISR - do not call directly
    void handleInterrupt() {
        if (_callback) {
            OCR2A = _callback();
        }
    }

private:
    PWMAudioISR() : _callback(nullptr), _sampleRate(22050) {}
    SampleCallback _callback;
    uint32_t _sampleRate;
};

} // namespace Synth

// Timer1 compare match ISR - must be in global scope
// Uncomment this in your main sketch if using PWMAudioISR:
/*
ISR(TIMER1_COMPA_vect) {
    Synth::PWMAudioISR::instance().handleInterrupt();
}
*/

#endif // PWM_AUDIO_H
