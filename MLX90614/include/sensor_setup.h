#ifndef MLX_SENSOR_SETUP_H
#define MLX_SENSOR_SETUP_H
#include <Adafruit_MLX90614.h>

// Dedicated sensor bus: request SMBus before initializing Wire, including when
// the sensor's power-on mode is PWM. External pull-ups must suit sensor voltage.
inline bool beginSensor(Adafruit_MLX90614 &sensor) {
    Adafruit_MLX90614::requestSMBus();
    if (sensor.begin()) return true;
    Serial.println("Sensor initialization failed. Check power, wiring and mode.");
    return false;
}
#endif
