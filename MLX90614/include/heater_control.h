#ifndef MLX_HEATER_CONTROL_H
#define MLX_HEATER_CONTROL_H
#include "pwm_temperature.h"

namespace MLX {
// Faults latch until restart; a later valid reading cannot silently reheat.
class HeaterControl {
public:
    explicit HeaterControl(float target = 40.0f, uint32_t maxHeatingMs = 60000)
        : _target(target), _maxHeatingMs(maxHeatingMs), _started(0),
          _heating(false), _fault(!isfinite(target) || target <= PWM_MIN_C
                                  || target >= PWM_MAX_C || maxHeatingMs == 0) {}
    bool update(float temperature, uint32_t now) {
        if (!isfinite(temperature) || temperature < PWM_MIN_C || temperature > PWM_MAX_C
            || temperature >= _target + 10.0f
            || (_heating && static_cast<uint32_t>(now - _started) >= _maxHeatingMs))
            _fault = true;
        if (_fault) { _heating = false; return false; }
        if (temperature >= _target) _heating = false;
        else if (!_heating && temperature < _target - 0.5f) {
            _heating = true;
            _started = now;
        }
        return _heating;
    }
    bool hasFault() const { return _fault; }
private:
    float _target;
    uint32_t _maxHeatingMs, _started;
    bool _heating, _fault;
};
} // namespace MLX
#endif
