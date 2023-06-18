#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>
#include <SPI.h>

// *** CHECK SENSOR'S COMMUNICATION MODE *** 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // if (!mlx.begin()) {
  //   Serial.println("Error connecting to MLX sensor. Check wiring.");
  //   while (1);
  // };

  mlx.begin();

  Serial.println(mlx.getCommunicationMode());
}

void loop() {
}