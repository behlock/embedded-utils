#pragma once
#include <Arduino.h>
inline void cli() { SREG &= ~uint8_t(0x80); }
#define ISR(vector) void vector()
