#ifndef TEST_ARDUINO_H
#define TEST_ARDUINO_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <utility>
#define PROGMEM
#define PI 3.14159265358979323846
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define HEX 16
#define BIN 2
#define A13 67
#define _BV(bit) (1U << (bit))
typedef uint8_t byte;
typedef std::string String;
const uint8_t SCL = 21;
inline uint8_t pgm_read_byte(const uint8_t *p) { return *p; }
inline uint16_t pgm_read_word(const uint16_t *p) { return *p; }
inline uint32_t pgm_read_dword(const uint32_t *p) { return *p; }
#define strcpy_P strcpy
struct FakeSerial {
    void begin(unsigned long) {}
    operator bool() const { return true; }
    template<class T> void print(T, int = 0) {}
    template<class T> void println(T, int = 0) {}
    void println() {}
};
extern FakeSerial Serial;
namespace Fake {
extern uint32_t now;
extern int adc;
extern int pins[128];
extern std::vector<std::pair<int, int> > writes, modes;
extern std::vector<unsigned long> pulses, timeouts;
void reset();
}
inline void pinMode(uint8_t pin, uint8_t mode) { Fake::modes.push_back(std::make_pair(pin, mode)); }
inline void digitalWrite(uint8_t pin, uint8_t value) {
    Fake::pins[pin] = value; Fake::writes.push_back(std::make_pair(pin, value));
}
inline unsigned long millis() { return Fake::now; }
inline void delay(unsigned long ms) { Fake::now += ms; }
inline int analogRead(uint8_t) { return Fake::adc; }
inline unsigned long pulseIn(uint8_t, uint8_t, unsigned long timeout = 1000000UL) {
    Fake::timeouts.push_back(timeout);
    if (Fake::pulses.empty()) return 0;
    unsigned long result = Fake::pulses.front();
    Fake::pulses.erase(Fake::pulses.begin());
    return result;
}
extern volatile uint8_t SREG, TCCR1A, TCCR1B, TCCR2A, TCCR2B, TIMSK1, TIMSK2, TIFR1, TIFR2, TCNT2, OCR2A, OCR2B;
extern volatile uint16_t OCR1A, OCR1B, TCNT1;
enum {WGM10=0, WGM12=3, CS10=0, COM1A1=7, COM1B1=5,
      WGM21=1, WGM20=0, CS20=0, COM2A1=7, COM2B1=5, OCIE1A=1};
#endif
