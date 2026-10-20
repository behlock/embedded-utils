#include <Arduino.h>
#include "heater_control.h"

const uint8_t HEAT_PIN = 36; // Active-high driver on Arduino Mega.
MLX::HeaterControl heater;

void setup() {
    digitalWrite(HEAT_PIN, LOW); // Set the output latch before enabling the pin.
    pinMode(HEAT_PIN, OUTPUT);
    pinMode(MLX::PWM_PIN, INPUT);
    Serial.begin(9600);
    Serial.println("Requires object1 single PWM, 0..125 C; sensor power-cycled after provisioning.");
    delay(300); // Allow the sensor to produce valid results after power-on.
}
void loop() {
    float temperature;
    if (!MLX::readPWMTemperature(MLX::PWM_PIN, temperature)) temperature = NAN;
    bool heating = heater.update(temperature, millis());
    digitalWrite(HEAT_PIN, heating ? HIGH : LOW);
    if (heater.hasFault()) Serial.println("Heater fault: output off; inspect sensor/system before restart.");
    else {
        Serial.print("Temperature C: "); Serial.println(temperature);
        Serial.println(heating ? "Heating" : "Heater off");
    }
    delay(100);
}
