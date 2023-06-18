#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>
#include <SPI.h>

// PIN DEFINITIONS
const int IR_SENSOR_PIN = 4;
const int HEAT_PIN = 36;

// TARGET TEMPERATURE
const float TARGET_TEMP = 40.0;

// GLOBAL VARIABLES
unsigned long previousMillis = 0;
float previousTemperature = 0;
float heatingRate = 0;

bool shouldStopHeating(float currentTemperature, float targetTemperature, float heatingRate) {
  float timeToReachTarget = (targetTemperature - currentTemperature) / heatingRate;
  float predictedTemperature = currentTemperature + (heatingRate * timeToReachTarget);

  return predictedTemperature >= targetTemperature;
}

void setup() {
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(HEAT_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // Get the current time in milliseconds
  unsigned long currentMillis = millis();
  // Measure the duration of the HIGH state of the PWM signal
  unsigned long highDuration = pulseIn(IR_SENSOR_PIN, HIGH);
  
  // Constants for the temperature range
  float T0_MIN = -10.0; // Minimum temperature in Celsius
  float T0_MAX = 160.0; // Maximum temperature in Celsius
  float T2 = 2048.0;    // Total number of clock cycles in a PWM period

  // Calculate the temperature in Celsius
  float temperatureInCelsius = (2 * (highDuration / T2) * (T0_MAX - T0_MIN)) + T0_MIN;
  
  // Print the temperature
  Serial.print("Temperature: "); Serial.print(temperatureInCelsius); Serial.println(" C");

  // Calculate the heating rate
  if (previousMillis != 0) {
    float deltaTime = (currentMillis - previousMillis) / 1000.0; // convert to seconds
    heatingRate = (temperatureInCelsius - previousTemperature) / deltaTime;
  }

  if (temperatureInCelsius < TARGET_TEMP) {
      Serial.println("Heating...");
      digitalWrite(HEAT_PIN, HIGH);
  
  } else if (temperatureInCelsius > TARGET_TEMP) {
      Serial.println("Cooling...");
      digitalWrite(HEAT_PIN, LOW);
  
  } else {
      Serial.println("Temperature is at target.");
      digitalWrite(HEAT_PIN, LOW);
  }

//   // Turn the heating element on or off
//   if (shouldStopHeating(temperatureInCelsius, TARGET_TEMP, heatingRate)) {
//     Serial.println("Stopping heating...");
//     digitalWrite(HEAT_PIN, LOW);
//   } else {
//     Serial.println("Heating...");
//     digitalWrite(HEAT_PIN, HIGH);
//   }
  
  delay(1000);
}