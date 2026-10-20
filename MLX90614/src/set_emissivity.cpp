#include "sensor_setup.h"
Adafruit_MLX90614 mlx;
void setup() {
    Serial.begin(9600);
    if (!beginSensor(mlx)) return;
    // Choose for the actual surface finish; this is an application-specific value.
    if (!mlx.writeEmissivity(0.3)) {
        Serial.println("Emissivity write/verification failed.");
        return;
    }
    Serial.print("Stored emissivity: "); Serial.println(mlx.readEmissivity());
    Serial.println("Power-cycle the sensor to apply EEPROM changes.");
}
void loop() {}
