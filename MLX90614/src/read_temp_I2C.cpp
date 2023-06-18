#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>
#include <SPI.h>

// *** READ TEMP IN I2C MODE ***
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // if (!mlx.begin()) {
  //   Serial.println("Error connecting to MLX sensor. Check wiring.");
  //   while (1);
  // };

  Serial.print("Emissivity = "); Serial.println(mlx.readEmissivity());
  Serial.println("================================================");

  Serial.print("Temp Max= "); Serial.println(mlx.readTempMax());
  Serial.println("================================================");

  Serial.print("Temp Min = "); Serial.println(mlx.readTempMin());
  Serial.println("================================================");
}

void loop() {
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC());
  Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");

  Serial.println();
  delay(1000);
}