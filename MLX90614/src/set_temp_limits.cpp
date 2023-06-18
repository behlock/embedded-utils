#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>
#include <SPI.h>

// *** SET MIN AND MAX TEMPS *** 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };

  mlx.begin();

  mlx.writeTempMin(0);
  mlx.writeTempMax(125);

  Serial.print("Temp Min = "); Serial.println(mlx.readTempMin());
  Serial.println("================================================");

  Serial.print("Temp Max= "); Serial.println(mlx.readTempMax());
  Serial.println("================================================");
}

void loop() {
}