#include "sensor_setup.h"
Adafruit_MLX90614 mlx;
void setup() {
    Serial.begin(9600);
    if (!beginSensor(mlx)) return;
    if (!mlx.switchToI2C()) {
        Serial.println("Interface write/verification failed.");
        return;
    }
    Serial.print("Stored startup interface: "); Serial.println(mlx.getCommunicationMode());
    Serial.println("Power-cycle the sensor to activate the stored interface.");
}
void loop() {}
