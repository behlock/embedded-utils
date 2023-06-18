#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>
#include <SPI.h>

// *** SWITCH TO PWM MODE *** 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  while (!Serial);


  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };

  mlx.begin();
  Serial.println("Configuring MLX90614 for PWM mode...");

  mlx.switchToPWM();
  if (mlx.getCommunicationMode() == "PWM") {
    Serial.println("Switched to PWM mode");
  } else {
    Serial.println("Failed to switch to PWM mode");
  }
}

void loop() {
  // Nothing to do in the loop
}