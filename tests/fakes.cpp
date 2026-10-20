#include <Arduino.h>
#include <Adafruit_I2CDevice.h>
FakeSerial Serial;
TwoWire Wire;
namespace Fake {
uint32_t now = 0;
int adc = 0;
int pins[128] = {};
std::vector<std::pair<int, int> > writes, modes;
std::vector<unsigned long> pulses, timeouts;
void reset() {
    now=0; adc=0; memset(pins, 0, sizeof(pins));
    writes.clear(); modes.clear(); pulses.clear(); timeouts.clear();
    bus() = Bus();
}
}
volatile uint8_t SREG=0, TCCR1A=0, TCCR1B=0, TCCR2A=0, TCCR2B=0, TIMSK1=0, TIMSK2=0, TIFR1=0, TIFR2=0, TCNT2=0, OCR2A=0, OCR2B=0;
volatile uint16_t OCR1A=0, OCR1B=0, TCNT1=0;
