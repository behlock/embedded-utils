#include <Arduino.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_SPIDevice.h>
#include <SPI.h>

const int PT100_PIN = A13; // Analog input pin connected to the PT100 amplifier output
const float VREF = 5.0;    // Voltage reference of the Arduino (3.3V or 5V, depending on the board)

// Add the provided temperature and VOut values to two arrays
// taken from https://wiki.e3d-online.com/E3D_PT100_Amplifier_Documentation
const int numDataPoints = 49;
const int temperatures[numDataPoints] = {0, 1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370, 380, 390, 400, 500, 600, 700, 800, 900, 1000, 1100};
const float vOut[numDataPoints] = {0.00, 1.11, 1.15, 1.20, 1.24, 1.28, 1.32, 1.36, 1.40, 1.44, 1.48, 1.52, 1.56, 1.61, 1.65, 1.68, 1.72, 1.76, 1.80, 1.84, 1.88, 1.92, 1.96, 2.00, 2.04, 2.07, 2.11, 2.15, 2.18, 2.22, 2.26, 2.29, 2.33, 2.37, 2.41, 2.44, 2.48, 2.51, 2.55, 2.58, 2.62, 2.66, 3.00, 3.33, 3.63, 3.93, 4.21, 4.48, 4.73};

void setup() {
  Serial.begin(9600);
  Serial.println("E3D PT100 Amplifier Sensor Test");
}

float interpolateTemperature(float voltage) {
  // If the voltage is outside the valid range, return an error value
  if (voltage < vOut[0] || voltage > vOut[numDataPoints - 1]) {
    return -1000;
  }

  // Find the index of the voltage interval
  int index = 0;
  while (voltage > vOut[index + 1]) {
    index++;
  }

  // Calculate the interpolated temperature using linear interpolation
  float slope = (temperatures[index + 1] - temperatures[index]) / (vOut[index + 1] - vOut[index]);
  float temperature = temperatures[index] + slope * (voltage - vOut[index]);

  return temperature;
}

void loop() {
  int raw_adc = analogRead(PT100_PIN);
  float voltage = (float)raw_adc * VREF / 1023.0;

  float temperature = interpolateTemperature(voltage);

  Serial.print("Raw ADC value: "); Serial.println(raw_adc);
  Serial.print("Voltage: "); Serial.print(voltage, 3); Serial.println(" V");
  Serial.print("Temperature: "); Serial.print(temperature, 2); Serial.println(" °C");

  delay(1000);
}