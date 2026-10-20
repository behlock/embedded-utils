#include "sensor_setup.h"
Adafruit_MLX90614 mlx;
void setup() {
    Serial.begin(9600);
    if (!beginSensor(mlx)) return;
    Serial.print("EEPROM startup interface (currently communicating over SMBus): ");
    Serial.println(mlx.getCommunicationMode());
}
void loop() {}
