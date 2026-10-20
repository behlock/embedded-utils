#include <Arduino.h>
#include "pwm_temperature.h"

void setup() {
    pinMode(MLX::PWM_PIN, INPUT);
    Serial.begin(9600);
    delay(300);
}
void loop() {
    float temperature;
    if (MLX::readPWMTemperature(MLX::PWM_PIN, temperature)) {
        Serial.print("Object temperature C: "); Serial.println(temperature);
    } else Serial.println("Invalid or missing single-mode PWM signal.");
    delay(1000);
}
