#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>
#include <SPI.h>

// *** SWITCH TO I2C MODE *** 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  while (!Serial);


  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };

  mlx.begin();
  Serial.println("Configuring MLX90614 for I2C mode...");

  mlx.switchToI2C();
  if (mlx.getCommunicationMode() == "I2C") {
    Serial.println("Switched to I2C mode");
  } else {
    Serial.println("Failed to switch to I2C mode");
  }
}

void loop() {
  // Your main loop code
}