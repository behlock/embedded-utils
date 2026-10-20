# Embedded utils

Arduino sensor diagnostics and small synthesizer helpers. Sensor projects target
Arduino Mega 2560. PWM audio supports the classic Uno/Nano (ATmega328P/168) and
Mega (ATmega2560/1280); the DSP helpers do not access timer registers.

## Build and test

Install PlatformIO Core 6.1.19. The projects pin `atmelavr@5.3.0`; checked-in
Adafruit BusIO is version 1.14.1. The MLX90614 library is a local fork of 2.1.3.

```sh
python3 tests/run.py
pio run                           # Synth smoke build: Uno and Mega
pio run -d MLX90614                # Default PWM reader
pio run -d MLX90614 -e read_temp_I2C
pio run -d 'E3D PT100 Amplifier'
```

Host tests require Python 3 and a C++11 compiler with address/undefined-behavior
sanitizers (Clang or GCC). Set `CXX=clang++` to choose Clang, or `SANITIZE=0` when
the host compiler lacks sanitizers. No hardware is accessed. CI runs these tests
and builds both synth targets and every sensor environment.

Some Apple Silicon hosts cannot run the bundled AVR binaries (`Bad CPU type in
executable`). An optional native AVR toolchain can be supplied through
`EMBEDDED_UTILS_AVR_PATH`, a colon-separated list of directories containing a
matching `avr-gcc`, `avr-g++`, `avr-gcc-ar`, `avr-gcc-ranlib`, and binutils. Include
both the compiler wrapper and underlying GCC binary directories when using Nix;
the latter contains its LTO-aware archiver. The project scripts use this variable
only when explicitly set. Clean with `pio run -t clean` after changing toolchains.

## Synth helpers

Add this repository root to the include path and include `synth/synth.h`, or
include individual headers such as `adsr/adsr.h`. LFO includes its oscillator
dependency automatically. Include `pwm_audio/pwm_audio.h` separately on supported
AVR boards. A complete interrupt-driven example is in `examples/synth_smoke`.

```cpp
#include "synth/synth.h"
Synth::Oscillator oscillator(22050);
Synth::ADSR envelope(22050);

void startNote() {
    oscillator.setFrequency(Synth::midiNoteToFrequency(69));
    envelope.setAttack(10);       // Milliseconds
    envelope.setDecay(50);
    envelope.setSustain(180);     // 0..255
    envelope.setRelease(100);
    envelope.noteOn();
}
uint8_t renderSample() { return envelope.apply(oscillator.nextSample()); }
```

- Each `nextSample`, `apply`, or modulation call advances one sample. Render each
  generator once per audio tick; do not call several output helpers for one tick.
- Unsigned audio is centered on 128; signed audio is centered on zero. ADSR's
  unsigned `apply()` preserves that midpoint. Envelope and LFO levels themselves
  are unipolar and are not audio samples.
- ADSR times describe complete linear segments, measured from their current
  level. Retriggering continues from that level; repeated `noteOff()` does not
  restart release. Changing an active segment's time restarts that segment from
  its current level. Sustain changes apply immediately while sustaining.
- Oscillator/LFO frequencies are clamped to 0..sampleRate/2; nonfinite frequencies
  become zero. Zero-rate constructors use 44100Hz; zero sample-rate setters are
  ignored. The 32-bit phase accumulator improves tuning; saw and pulse waveforms
  remain naive and can alias. Band-limited synthesis is outside these helpers.
- MIDI lookup frequencies use a consistent 32-bit fixed-point scale. Note names
  accept buffer sizes: `char name[5]; getNoteNameWithOctave(note,name,sizeof(name))`.
  Name helpers return false for truncated output. Legacy two-argument calls
  require at least 3 bytes for names and 5 for names with octaves.
- The state-variable filter caps cutoff at sampleRate/8 for stability of its
  single-step recurrence. Filter outputs saturate to the requested integer range;
  one-pole, SVF, and DC history retain fractional precision.
- `PWMAudio::TIMER1_A/B` and `TIMER2_A/B` select the correct board pins. Timer1 A/B
  are Uno 9/10 and Mega 11/12; Timer2 A/B are Uno 11/3 and Mega 10/9.
- PWM output is always 8-bit at F_CPU/256 (62.5kHz on 16MHz boards). `write16()`
  accepts normalized unsigned 16-bit input and reduces it to 8-bit output.
  `PWMAudio.setSampleRate()` stores polling metadata; it does not schedule writes.
- `PWMAudioISR` owns Timer1 for sampling and Timer2 for PWM. Define its global ISR
  exactly once as shown in the example. `begin()` returns false for an invalid
  callback or unrepresentable timer rate; `getSampleRate()` returns the actual
  integer timer rate. Initialization preserves the caller's interrupt state.
  Give these timers exclusive ownership: do not combine with Servo, tone, or
  another PWM audio instance on the same timers.
- Use a reconstruction filter and output coupling/buffer suited to the load. A
  1kΩ/10nF RC pole is approximately 16kHz; a 1kΩ/10µF pole would remove most audio.
  Benchmark the callback on the MCU before using floating-point filters in it.

## MLX90614

The local Adafruit fork adds PEC checking, failure propagation, verified EEPROM
writes, and Celsius PWM limits. Reads return `NAN` on temperature/emissivity
failure; configuration setters return `bool`. Address-read failure returns
`0xFFFF`. The device owns its I2C interface and cannot be copied.

All SMBus sketches issue an SCL-low request before initialization, so they can
reach a sensor whose startup mode is PWM. This assumes a dedicated sensor bus and
external pull-ups appropriate for its supply voltage. `requestSMBus()` must be
called before Wire is enabled. `getCommunicationMode()` describes EEPROM startup
configuration; the request has already switched the active interface to SMBus.

PWM readers require **single object1 PWM, range 0..125°C, on pin 4**. Set the shared
range, pin, or pulse timeout in `MLX90614/include/pwm_temperature.h` if your wiring
or application differs. Pin 4 replaces the old standalone PWM reader's pin 6.
Provision the sensor before using PWM measurement or heating:

1. Build/upload `set_temp_limits` to store the shared temperature range.
2. Build/upload `switch_to_PWM` to select single object1 PWM with a nominal
   1.024ms period, no repetitions, and push-pull output. Calibration/filter bits
   in the main configuration register are preserved.
3. Power-cycle the sensor, then build/upload the PWM reader or heater sketch.

```sh
pio run -d MLX90614 -e set_temp_limits
pio run -d MLX90614 -e switch_to_PWM
pio run -d MLX90614 -e switch_to_I2C
```

These commands compile only. Add `-t upload` intentionally when connected to the
correct board. Configuration utilities verify erase/write readback and skip
unchanged EEPROM cells. Writes cannot be atomic across multiple cells; a reported
failure needs inspection before use. Power-cycle the sensor after any EEPROM
change. `writeTempMin/Max` now accept and return Celsius rather than raw words;
use `writeTemperatureRange(min,max)` for an ordered range update. Register addresses
and PWM encoding follow the [Melexis datasheet](https://www.melexis.com/-/media/files/documents/datasheets/mlx90614-datasheet-melexis.pdf).

The heater example assumes an active-high driver on Mega pin 36. It starts off,
uses a 40°C target with 0.5°C turn-on hysteresis, and latches off on invalid or
missing pulses, a reading 10°C above target, or 60 seconds of continuous heating.
After a fault, inspect the system before restarting it. These limits are example
settings in `heater_control.h`; adapt them to the actual heater. Use an independent
thermal cutoff for physical heating equipment. The software cannot detect every
plausible but incorrect sensor value or verify EEPROM configuration through PWM.

## E3D PT100 amplifier

The diagnostic sketch uses Mega A13 and a 10-bit ADC with a 5V reference. Measure
and set `VREF` for the actual board. Interpolation rejects nonfinite/out-of-range
voltages with `NAN`, and the sketch reports those as invalid readings. The legacy
voltage table is retained: verify it against your amplifier and calibration before
using its output for control. The zero-voltage entry is not sensor-fault detection.

## Validation limits

Host tests cover numeric edge cases, envelope timings, all 128 MIDI notes, filter
ranges, sensor failures/PEC, EEPROM units/readback, PWM timer mappings, and heater
disconnect/timeout behavior. AVR builds check compatibility and linking, not
physical waveforms, sensor calibration, audio performance, or heater behavior.
