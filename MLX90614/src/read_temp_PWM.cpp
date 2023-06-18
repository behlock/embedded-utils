#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>
#include <SPI.h>

// *** READ TEMPERATURE FROM PWM MODE ***
const int PWM_PIN = 6;

void setup() {
  pinMode(PWM_PIN, INPUT);
  Serial.begin(9600);
}

void loop() {
  // Measure the duration of the HIGH state of the PWM signal
  unsigned long highDuration = pulseIn(PWM_PIN, HIGH);

  // Serial.println(highDuration);
  // Constants for the temperature range
  double T0_MIN = -10; // Minimum temperature in Celsius
  double T0_MAX = 125.0; // Maximum temperature in Celsius
  double T2 = 2048.0;    // Total number of clock cycles in a PWM period

  // Calculate the temperature in Celsius
  // double temperatureInCelsius = (2 * (highDuration / T2) * (T0_MAX - T0_MIN)) / 40 + T0_MIN;
  
  Serial.print("Temperature: ");
  Serial.print(temperatureInCelsius);
  Serial.println(" C");
  
  delay(1000);
}