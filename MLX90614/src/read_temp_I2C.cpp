#include "sensor_setup.h"
#include <math.h>
Adafruit_MLX90614 mlx;

void setup() {
    Serial.begin(9600);
    if (!beginSensor(mlx)) return;
    Serial.print("Emissivity: "); Serial.println(mlx.readEmissivity());
    Serial.print("PWM minimum C: "); Serial.println(mlx.readTempMin());
    Serial.print("PWM maximum C: "); Serial.println(mlx.readTempMax());
    delay(300);
}
void loop() {
    double ambient = mlx.readAmbientTempC(), object = mlx.readObjectTempC();
    if (!isfinite(ambient) || !isfinite(object)) Serial.println("Temperature read failed.");
    else {
        Serial.print("Ambient C: "); Serial.print(ambient);
        Serial.print("; object C: "); Serial.println(object);
    }
    delay(1000);
}
