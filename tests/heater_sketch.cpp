#include "MLX90614/src/get_to_target_temp.cpp"
#include <assert.h>
#include <stdio.h>
int main() {
    setup(); assert(Fake::pins[HEAT_PIN]==LOW);
    Fake::pulses={205,819}; loop(); assert(Fake::pins[HEAT_PIN]==HIGH);
    Fake::pulses={0}; loop(); assert(Fake::pins[HEAT_PIN]==LOW);
    Fake::pulses={205,819}; loop(); assert(Fake::pins[HEAT_PIN]==LOW);
    puts("Heater sketch disconnect regression passed");
}
