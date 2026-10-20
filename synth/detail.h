#ifndef SYNTH_DETAIL_H
#define SYNTH_DETAIL_H
#include <Arduino.h>
#include <math.h>
namespace Synth { namespace detail {
inline uint32_t sampleRate(uint32_t rate) { return rate ? rate : 44100; }
inline float clamp(float value, float low, float high) {
    if (!isfinite(value)) return low;
    return value < low ? low : (value > high ? high : value);
}
inline int16_t signed16(float value) {
    return static_cast<int16_t>(clamp(value, -32768.0f, 32767.0f));
}
inline uint8_t unsigned8(float value) {
    return static_cast<uint8_t>(clamp(value, 0.0f, 255.0f));
}
} }
#endif
