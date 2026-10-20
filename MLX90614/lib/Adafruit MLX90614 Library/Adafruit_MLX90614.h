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

  Written by Limor Fried/Ladyada for Adafruit in any redistribution
 ****************************************************/

#ifndef ADAFRUIT_MLX90614_H
#define ADAFRUIT_MLX90614_H

// Local fork: checked transactions, Celsius PWM limits, and interface configuration.
#include <Adafruit_I2CDevice.h>
#include <Arduino.h>

#define MLX90614_I2CADDR 0x5A

// RAM
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08
// EEPROM
#define MLX90614_TOMAX 0x20
#define MLX90614_TOMIN 0x21
#define MLX90614_PWMCTRL 0x22
#define MLX90614_TARANGE 0x23
#define MLX90614_EMISS 0x24
#define MLX90614_CONFIG 0x25
#define MLX90614_ADDR 0x2E
#define MLX90614_ID1 0x3C
#define MLX90614_ID2 0x3D
#define MLX90614_ID3 0x3E
#define MLX90614_ID4 0x3F

/**
 * @brief Class to read from and control a MLX90614 Temp Sensor
 *
 */
class Adafruit_MLX90614 {
public:
  Adafruit_MLX90614() = default;
  Adafruit_MLX90614(const Adafruit_MLX90614&) = delete;
  Adafruit_MLX90614& operator=(const Adafruit_MLX90614&) = delete;
  ~Adafruit_MLX90614();
  bool begin(uint8_t addr = MLX90614_I2CADDR, TwoWire *wire = &Wire);

  // TEMPERATURE
  double readObjectTempC(void);
  double readAmbientTempC(void);
  double readObjectTempF(void);
  double readAmbientTempF(void);

  // EMISSIVITY
  uint16_t readEmissivityReg(void);
  bool writeEmissivityReg(uint16_t ereg);
  double readEmissivity(void);
  bool writeEmissivity(double emissivity);

  // MIN TEMP
  float readTempMin(void);
  bool writeTempMin(float minTemp);

  // MAX TEMP
  float readTempMax(void);
  bool writeTempMax(float maxTemp);
  bool writeTemperatureRange(float minTemp, float maxTemp);

  // COMMUNICATION PROTOCOL
  // Reports EEPROM startup mode, not the active interface after an SMBus request.
  String getCommunicationMode(void);
  // Call before Wire.begin(); requires external pull-ups, releases SCL afterward.
  static void requestSMBus(uint8_t sclPin = SCL);
  uint16_t readI2CAddr(void);
  // Single object1 PWM, no repetitions; period in nominal 1.024ms units (1..127).
  bool switchToPWM(uint8_t period = 1);
  bool switchToI2C(void);

  // DEBUGGING
  void printAllRegisters(void);

private:
  Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface
  float readTemp(uint8_t reg);

  bool read16(uint8_t addr, uint16_t &value);
  bool write16(uint8_t addr, uint16_t data);
  bool writeEEPROM(uint8_t addr, uint16_t data);
  static bool encodeLimit(float celsius, uint16_t &word);
  byte crc8(byte *addr, byte len);
  uint8_t _addr = MLX90614_I2CADDR;
};

#endif // ADAFRUIT_MLX90614_H
