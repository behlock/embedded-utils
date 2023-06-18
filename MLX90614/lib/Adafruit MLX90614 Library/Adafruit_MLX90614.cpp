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

#include "Adafruit_MLX90614.h"

Adafruit_MLX90614::~Adafruit_MLX90614() {
  if (i2c_dev)
    delete i2c_dev;
}

/**
 * @brief Begin the I2C connection
 * @param addr I2C address for the device.
 * @param wire Pointer to Wire instance
 * @return True if the device was successfully initialized, otherwise false.
 */
bool Adafruit_MLX90614::begin(uint8_t addr, TwoWire *wire) {
  _addr = addr; // needed for CRC
  if (i2c_dev)
    delete i2c_dev;
  i2c_dev = new Adafruit_I2CDevice(addr, wire);
  return i2c_dev->begin();
}

/**
 * @brief Read the raw value from the emissivity register
 *
 * @return uint16_t The unscaled emissivity value or '0' if reading failed
 */
uint16_t Adafruit_MLX90614::readEmissivityReg(void) {
  return read16(MLX90614_EMISS);
}
/**
 * @brief Write the raw unscaled emissivity value to the emissivity register
 *
 * @param ereg The unscaled emissivity value
 */
void Adafruit_MLX90614::writeEmissivityReg(uint16_t ereg) {
  write16(MLX90614_EMISS, 0); // erase
  delay(10);
  write16(MLX90614_EMISS, ereg);
  delay(10);
}
/**
 * @brief Read the emissivity value from the sensor's register and scale
 *
 * @return double The emissivity value, ranging from 0.1 - 1.0 or NAN if reading
 * failed
 */
double Adafruit_MLX90614::readEmissivity(void) {
  uint16_t ereg = read16(MLX90614_EMISS);
  if (ereg == 0)
    return NAN;
  return ((double)ereg) / 65535.0;
}
/**
 * @brief Set the emissivity value
 *
 * @param emissivity The emissivity value to use, between 0.1 and 1.0
 */
void Adafruit_MLX90614::writeEmissivity(double emissivity) {
  uint16_t ereg = (uint16_t)(0xffff * emissivity);

  writeEmissivityReg(ereg);
}

/**
 * @brief Get the current temperature of an object in degrees Farenheit
 *
 * @return double The temperature in degrees Farenheit or NAN if reading failed
 */
double Adafruit_MLX90614::readObjectTempF(void) {
  return (readTemp(MLX90614_TOBJ1) * 9 / 5) + 32;
}
/**
 * @brief Get the current ambient temperature in degrees Farenheit
 *
 * @return double The temperature in degrees Farenheit or NAN if reading failed
 */
double Adafruit_MLX90614::readAmbientTempF(void) {
  return (readTemp(MLX90614_TA) * 9 / 5) + 32;
}

/**
 * @brief Get the current temperature of an object in degrees Celcius
 *
 * @return double The temperature in degrees Celcius or NAN if reading failed
 */
double Adafruit_MLX90614::readObjectTempC(void) {
  return readTemp(MLX90614_TOBJ1);
}

/**
 * @brief Get the current ambient temperature in degrees Celcius
 *
 * @return double The temperature in degrees Celcius or NAN if reading failed
 */
double Adafruit_MLX90614::readAmbientTempC(void) {
  return readTemp(MLX90614_TA);
}

float Adafruit_MLX90614::readTemp(uint8_t reg) {
  float temp;

  temp = read16(reg);
  if (temp == 0)
    return NAN;
  temp *= .02;
  temp -= 273.15;
  return temp;
}

/*********************************************************************/

uint16_t Adafruit_MLX90614::read16(uint8_t a) {
  uint8_t buffer[3];
  buffer[0] = a;
  // read two bytes of data + pec
  bool status = i2c_dev->write_then_read(buffer, 1, buffer, 3);
  if (!status)
    return 0;
  // return data, ignore pec
  return uint16_t(buffer[0]) | (uint16_t(buffer[1]) << 8);
}

byte Adafruit_MLX90614::crc8(byte *addr, byte len)
// The PEC calculation includes all bits except the START, REPEATED START, STOP,
// ACK, and NACK bits. The PEC is a CRC-8 with polynomial X8+X2+X1+1.
{
  byte crc = 0;
  while (len--) {
    byte inbyte = *addr++;
    for (byte i = 8; i; i--) {
      byte carry = (crc ^ inbyte) & 0x80;
      crc <<= 1;
      if (carry)
        crc ^= 0x7;
      inbyte <<= 1;
    }
  }
  return crc;
}

void Adafruit_MLX90614::write16(uint8_t a, uint16_t v) {
  uint8_t buffer[4];

  buffer[0] = _addr << 1;
  buffer[1] = a;
  buffer[2] = v & 0xff;
  buffer[3] = v >> 8;

  uint8_t pec = crc8(buffer, 4);

  buffer[0] = buffer[1];
  buffer[1] = buffer[2];
  buffer[2] = buffer[3];
  buffer[3] = pec;

  i2c_dev->write(buffer, 4);
}


//*** CUSTOM FUNCTIONALITY

// MIN TEMP
int Adafruit_MLX90614::readTempMin(void) {
  uint16_t tminreg = read16(MLX90614_TOMIN);
  return static_cast<int>(tminreg);
}

void Adafruit_MLX90614::writeTempMin(int temp_min) {
  // ensure temp is within the range of a 16-bit unsigned integer
  if (temp_min < 0 || temp_min > UINT16_MAX) {
    return;
  }

  uint16_t tminreg = static_cast<uint16_t>(temp_min);
  write16(MLX90614_TOMIN, 0); // erase
  delay(10);
  write16(MLX90614_TOMIN, tminreg);
  delay(10);
}

// MAX TEMP
int Adafruit_MLX90614::readTempMax(void) {
  uint16_t tmaxreg = read16(MLX90614_TOMAX);
  return static_cast<int>(tmaxreg);
}

void Adafruit_MLX90614::writeTempMax(int temp_max) {
  // ensure temp is within the range of a 16-bit unsigned integer
  if (temp_max < 0 || temp_max > UINT16_MAX) {
    return;
  }

  uint16_t tmaxreg = static_cast<uint16_t>(temp_max);

  write16(MLX90614_TOMAX, 0); // erase
  delay(10);
  write16(MLX90614_TOMAX, tmaxreg);
  delay(10);
}

// COMMUNICATION PROTOCOL
uint16_t Adafruit_MLX90614::readI2CAddr(void) {
  return read16(MLX90614_PWMCTRL);
}

String Adafruit_MLX90614::getCommunicationMode() {
  uint16_t registerValue = read16(MLX90614_PWMCTRL);

  // Check bit 1
  // 0 -> PWM mode disabled
  // 1 -> PWM mode enabled
  bool bit1 = registerValue & (1 << 1);

  if (bit1) {
    return "PWM";
  } else {
    return "I2C";
  }
}

void Adafruit_MLX90614::switchToI2C() {
  uint16_t registerValue = read16(MLX90614_PWMCTRL);

  // Set bit 1: Enable/disable PWM
    // 0 -> PWM mode disabled
  // 1 -> PWM mode enabled
  registerValue &= ~(1 << 1);

  // Erase the existing value in the register
  write16(MLX90614_PWMCTRL, 0);
  delay(10);

  // Write the updated value back to the register
  write16(MLX90614_PWMCTRL, registerValue);
  delay(10);
}

void Adafruit_MLX90614::switchToPWM() {
  bool singleMode = true;
  bool pushPull = true;

  uint16_t registerValue = read16(MLX90614_PWMCTRL);

  // Set bit 0: Select the type of PWM mode
  // 0 -> Extended mode
  // 1 -> Single mode
  if (singleMode) {
    registerValue |= (1 << 0);
  } else {
    registerValue &= ~(1 << 0);
  }

  // Set bit 1: Enable/disable PWM
  // 0 -> PWM mode disabled
  // 1 -> PWM mode enabled
  registerValue |= (1 << 1);

  // Set bit 2: Configuration of the pin PWM
  // 0 -> Open drain
  // 1 -> Push-pull
  if (pushPull) {
    registerValue |= (1 << 2);
  } else {
    registerValue &= ~(1 << 2);
  }

  // Erase the existing value in the register
  write16(MLX90614_PWMCTRL, 0);
  delay(10);

  // Write the updated value back to the register
  write16(MLX90614_PWMCTRL, registerValue);
  delay(10);

  // Erase the existing value in the register
  write16(MLX90614_CONFIG, 0);
  delay(10);

  // Write the default value back to the register
  write16(MLX90614_CONFIG, 0xB7F4);
  delay(10);
}

// DEBUGGING
void Adafruit_MLX90614::printAllRegisters() {
  Serial.println("MLX90614 Registers:");
  Serial.println("Register\tValue\t\tBinary");
  Serial.println("--------\t-----\t\t------");

  for (int i = 0; i < 0x40; i++) {
    uint16_t registerValue = read16(i);
    Serial.print(i, HEX);
    Serial.print("\t\t");
    Serial.print(registerValue, HEX);
    Serial.print("\t\t");
    Serial.println(registerValue, BIN);
  }
}
