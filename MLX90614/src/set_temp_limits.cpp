#include "sensor_setup.h"
#include "pwm_temperature.h"
Adafruit_MLX90614 mlx;
void setup() {
    Serial.begin(9600);
    if (!beginSensor(mlx)) return;
    if (!mlx.writeTemperatureRange(MLX::PWM_MIN_C, MLX::PWM_MAX_C)) {
        Serial.println("PWM range write/verification failed.");
        return;
    }
    Serial.print("Stored minimum C: "); Serial.println(mlx.readTempMin());
    Serial.print("Stored maximum C: "); Serial.println(mlx.readTempMax());
    Serial.println("Power-cycle the sensor to apply EEPROM changes.");
}
void loop() {}
