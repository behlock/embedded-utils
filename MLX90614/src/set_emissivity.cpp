#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>
#include <SPI.h>

// *** SET EMISSIVITY *** 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  mlx.begin();

  // The emissivity of 304 stainless steel depends on the surface
  // finish and oxidation levels. Generally, the emissivity of 304 stainless steel
  // ranges from approximately 0.1 to 0.35. For a polished 304 stainless steel
  // surface, the emissivity is usually lower, around 0.1 to 0.2. 
  // As the surface becomes rougher or more oxidized, the emissivity 
  // increases. For a rough or oxidized 304 stainless steel surface, the emissivity
  // can be between 0.2 and 0.35.
  mlx.writeEmissivity(0.3);

  Serial.print("Emissivity is now "); Serial.println(mlx.readEmissivity());
}

void loop() {
}