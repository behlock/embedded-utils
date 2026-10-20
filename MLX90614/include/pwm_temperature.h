#ifndef MLX_PWM_TEMPERATURE_H
#define MLX_PWM_TEMPERATURE_H
#include <Arduino.h>
#include <math.h>

namespace MLX {
// Provision with set_temp_limits and switch_to_PWM, then power-cycle the sensor.
constexpr uint8_t PWM_PIN = 4;
constexpr float PWM_MIN_C = 0.0f;
constexpr float PWM_MAX_C = 125.0f;
constexpr unsigned long PULSE_TIMEOUT_US = 20000;

inline bool decodeSinglePWM(unsigned long highUs, unsigned long lowUs,
                            float minimum, float maximum, float &temperature) {
    temperature = NAN;
    if (!highUs || !lowUs || !isfinite(minimum) || !isfinite(maximum) || minimum >= maximum)
        return false;
    float duty = highUs / (static_cast<float>(highUs) + lowUs);
    // Single format: 1/8 start, up to 1/2 data; 7/8 indicates a fatal error.
    if (duty < 0.125f || duty > 0.625f) return false;
    temperature = minimum + 2.0f * (duty - 0.125f) * (maximum - minimum);
    return isfinite(temperature);
}
inline bool readPWMTemperature(uint8_t pin, float &temperature) {
    unsigned long highUs = pulseIn(pin, HIGH, PULSE_TIMEOUT_US);
    if (!highUs) { temperature = NAN; return false; }
    unsigned long lowUs = pulseIn(pin, LOW, PULSE_TIMEOUT_US);
    return decodeSinglePWM(highUs, lowUs, PWM_MIN_C, PWM_MAX_C, temperature);
}
} // namespace MLX
#endif
