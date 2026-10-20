/***************************************************
  This is a library for the MLX90614 Temp Sensor

  Designed specifically to work with the MLX90614 sensors in the
  adafruit shop
  ----> https://www.adafruit.com/products/1747 (3V)
  ----> https://www.adafruit.com/products/1748 (5V)

  These sensors use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// Local fork; see README.md for API changes and configuration assumptions.
#include "Adafruit_MLX90614.h"
#include <math.h>

Adafruit_MLX90614::~Adafruit_MLX90614() { delete i2c_dev; }

bool Adafruit_MLX90614::begin(uint8_t addr, TwoWire *wire) {
  delete i2c_dev;
  i2c_dev = nullptr;
  if (!wire || addr > 0x7F) return false;
  _addr = addr;
  i2c_dev = new Adafruit_I2CDevice(addr, wire);
  if (!i2c_dev || !i2c_dev->begin()) {
    delete i2c_dev;
    i2c_dev = nullptr;
    return false;
  }
  return true;
}

void Adafruit_MLX90614::requestSMBus(uint8_t sclPin) {
  digitalWrite(sclPin, LOW);
  pinMode(sclPin, OUTPUT);
  delay(2); // tREQ > 1.44ms, below the SMBus low timeout.
  pinMode(sclPin, INPUT); // Release to the external bus pull-up.
  delay(1);
}

uint16_t Adafruit_MLX90614::readEmissivityReg() {
  uint16_t value;
  return read16(MLX90614_EMISS, value) ? value : 0;
}
bool Adafruit_MLX90614::writeEmissivityReg(uint16_t value) {
  return value >= 6554 && writeEEPROM(MLX90614_EMISS, value);
}
double Adafruit_MLX90614::readEmissivity() {
  uint16_t value = readEmissivityReg();
  return value ? value / 65535.0 : NAN;
}
bool Adafruit_MLX90614::writeEmissivity(double emissivity) {
  if (!isfinite(emissivity) || emissivity < 0.1 || emissivity > 1.0) return false;
  return writeEmissivityReg(static_cast<uint16_t>(65535.0 * emissivity + 0.5));
}

double Adafruit_MLX90614::readObjectTempC() { return readTemp(MLX90614_TOBJ1); }
double Adafruit_MLX90614::readAmbientTempC() { return readTemp(MLX90614_TA); }
double Adafruit_MLX90614::readObjectTempF() { return readObjectTempC() * 1.8 + 32; }
double Adafruit_MLX90614::readAmbientTempF() { return readAmbientTempC() * 1.8 + 32; }
float Adafruit_MLX90614::readTemp(uint8_t reg) {
  uint16_t value;
  if (!read16(reg, value) || value == 0 || (value & 0x8000)) return NAN;
  return value * 0.02f - 273.15f;
}

bool Adafruit_MLX90614::read16(uint8_t reg, uint16_t &value) {
  if (!i2c_dev) return false;
  uint8_t buffer[3];
  if (!i2c_dev->write_then_read(&reg, 1, buffer, 3)) return false;
  uint8_t frame[5] = {static_cast<uint8_t>(_addr << 1), reg,
    static_cast<uint8_t>((_addr << 1) | 1), buffer[0], buffer[1]};
  if (crc8(frame, 5) != buffer[2]) return false;
  value = static_cast<uint16_t>(buffer[0]) | (static_cast<uint16_t>(buffer[1]) << 8);
  return true;
}

byte Adafruit_MLX90614::crc8(byte *data, byte length) {
  byte crc = 0;
  while (length--) {
    crc ^= *data++;
    for (byte i = 0; i < 8; ++i)
      crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : (crc << 1);
  }
  return crc;
}

bool Adafruit_MLX90614::write16(uint8_t reg, uint16_t value) {
  if (!i2c_dev) return false;
  uint8_t frame[4] = {static_cast<uint8_t>(_addr << 1), reg,
    static_cast<uint8_t>(value), static_cast<uint8_t>(value >> 8)};
  uint8_t buffer[4] = {reg, frame[2], frame[3], crc8(frame, 4)};
  return i2c_dev->write(buffer, 4);
}

bool Adafruit_MLX90614::writeEEPROM(uint8_t reg, uint16_t value) {
  uint16_t current;
  if (!read16(reg, current)) return false;
  if (current == value) return true; // Avoid needless erase cycles at startup.
  if (!write16(reg, 0)) return false;
  delay(10);
  if (!read16(reg, current) || current != 0) return false;
  if (!write16(reg, value)) return false;
  delay(10);
  return read16(reg, current) && current == value;
}

bool Adafruit_MLX90614::encodeLimit(float celsius, uint16_t &word) {
  if (!isfinite(celsius) || celsius < -70.0f || celsius > 380.0f) return false;
  word = static_cast<uint16_t>((celsius + 273.15f) * 100.0f + 0.5f);
  return true;
}
float Adafruit_MLX90614::readTempMin() {
  uint16_t word;
  return read16(MLX90614_TOMIN, word) ? word / 100.0f - 273.15f : NAN;
}
float Adafruit_MLX90614::readTempMax() {
  uint16_t word;
  return read16(MLX90614_TOMAX, word) ? word / 100.0f - 273.15f : NAN;
}
bool Adafruit_MLX90614::writeTempMin(float minTemp) {
  uint16_t minimum, maximum;
  return encodeLimit(minTemp, minimum) && read16(MLX90614_TOMAX, maximum)
    && minimum < maximum && writeEEPROM(MLX90614_TOMIN, minimum);
}
bool Adafruit_MLX90614::writeTempMax(float maxTemp) {
  uint16_t minimum, maximum;
  return encodeLimit(maxTemp, maximum) && read16(MLX90614_TOMIN, minimum)
    && maximum > minimum && writeEEPROM(MLX90614_TOMAX, maximum);
}
bool Adafruit_MLX90614::writeTemperatureRange(float minTemp, float maxTemp) {
  uint16_t minimum, maximum, oldMaximum;
  if (!encodeLimit(minTemp, minimum) || !encodeLimit(maxTemp, maximum)
      || minimum >= maximum || !read16(MLX90614_TOMAX, oldMaximum)) return false;
  // Preserve an ordered range between the two writes. A failure is reported;
  // EEPROM updates cannot be made atomic across two cells.
  if (minimum >= oldMaximum)
    return writeEEPROM(MLX90614_TOMAX, maximum) && writeEEPROM(MLX90614_TOMIN, minimum);
  return writeEEPROM(MLX90614_TOMIN, minimum) && writeEEPROM(MLX90614_TOMAX, maximum);
}

uint16_t Adafruit_MLX90614::readI2CAddr() {
  uint16_t word;
  return read16(MLX90614_ADDR, word) ? word & 0x7F : 0xFFFF;
}
String Adafruit_MLX90614::getCommunicationMode() {
  uint16_t word;
  if (!read16(MLX90614_PWMCTRL, word)) return "Unknown";
  return (word & 2) ? "PWM" : "I2C";
}
bool Adafruit_MLX90614::switchToI2C() {
  uint16_t word;
  return read16(MLX90614_PWMCTRL, word)
    && writeEEPROM(MLX90614_PWMCTRL, word & ~uint16_t(2));
}
bool Adafruit_MLX90614::switchToPWM(uint8_t period) {
  if (period == 0 || period > 127) return false;
  uint16_t pwm, config;
  if (!read16(MLX90614_PWMCTRL, pwm) || !read16(MLX90614_CONFIG, config)) return false;
  // Single mode, enabled, push-pull. Config[5:4]=11 selects object1 for PWM1;
  // preserve calibration, filtering, and every unrelated configuration bit.
  if (!writeEEPROM(MLX90614_CONFIG, (config & ~uint16_t(0x30)) | 0x30)) return false;
  // Explicitly choose PWM (not thermal relay), no repetitions, and period.
  return writeEEPROM(MLX90614_PWMCTRL, (static_cast<uint16_t>(period) << 9) | 0x07);
}
void Adafruit_MLX90614::printAllRegisters() {
  Serial.println("MLX90614 registers (hex):");
  for (uint8_t reg = 0; reg < 0x40; ++reg) {
    uint16_t value;
    Serial.print(reg, HEX);
    Serial.print(": ");
    if (read16(reg, value)) Serial.println(value, HEX);
    else Serial.println("read failed");
  }
}
