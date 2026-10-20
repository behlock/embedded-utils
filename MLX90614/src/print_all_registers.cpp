#include "sensor_setup.h"
Adafruit_MLX90614 mlx;
void setup() {
    Serial.begin(9600);
    if (beginSensor(mlx)) mlx.printAllRegisters();
}
void loop() {}
