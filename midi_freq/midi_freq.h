#ifndef MIDI_FREQ_H
#define MIDI_FREQ_H

#include "../synth/detail.h"
#include <stdio.h>
#include <string.h>

namespace Synth {

// MIDI note to frequency conversion
// A4 (MIDI note 69) = 440 Hz standard tuning
//
// Usage:
//   float freq = midiNoteToFrequency(60);  // C4 = 261.63 Hz
//   uint8_t note = frequencyToMidiNote(440.0);  // A4 = 69
//   char name[5]; getNoteNameWithOctave(60, name, sizeof(name)); // "C4"

// All entries use frequency * 100, including the highest MIDI notes.
const uint32_t MIDI_FREQ_TABLE[128] PROGMEM = {
    818, 866, 918, 972, 1030, 1091, 1156, 1225, 1298, 1375, 1457, 1543,
    1635, 1732, 1835, 1945, 2060, 2183, 2312, 2450, 2596, 2750, 2914, 3087,
    3270, 3465, 3671, 3889, 4120, 4365, 4625, 4900, 5191, 5500, 5827, 6174,
    6541, 6930, 7342, 7778, 8241, 8731, 9250, 9800, 10383, 11000, 11654, 12347,
    13081, 13859, 14683, 15556, 16481, 17461, 18500, 19600, 20765, 22000, 23308, 24694,
    26163, 27718, 29366, 31113, 32963, 34923, 36999, 39200, 41530, 44000, 46616, 49388,
    52325, 55437, 58733, 62225, 65926, 69846, 73999, 78399, 83061, 88000, 93233, 98777,
    104650, 110873, 117466, 124451, 131851, 139691, 147998, 156798, 166122, 176000, 186466, 197553,
    209300, 221746, 234932, 248902, 263702, 279383, 295996, 313596, 332244, 352000, 372931, 395107,
    418601, 443492, 469864, 497803, 527404, 558765, 591991, 627193, 664488, 704000, 745862, 790213,
    837202, 886984, 939727, 995606, 1054808, 1117530, 1183982, 1254385,
};

// Note names for display
const char NOTE_NAMES[12][3] PROGMEM = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// Convert MIDI note number to frequency in Hz
inline float midiNoteToFrequency(uint8_t note) {
    if (note > 127) note = 127;

    return pgm_read_dword(&MIDI_FREQ_TABLE[note]) / 100.0f;
}

// Convert MIDI note to frequency using calculation (more precise but slower)
inline float midiNoteToFrequencyCalc(uint8_t note) {
    if (note > 127) note = 127;
    // f = 440 * 2^((note - 69) / 12)
    return 440.0f * pow(2.0f, (note - 69) / 12.0f);
}

// Convert frequency to nearest MIDI note
inline uint8_t frequencyToMidiNote(float freq) {
    // note = 12 * log2(freq / 440) + 69
    if (isnan(freq) || freq <= 0) return 0;
    if (isinf(freq)) return 127;
    float note = 12.0f * log(freq / 440.0f) * 1.44269504089f + 69.0f;
    if (note < 0) return 0;
    if (note > 127) return 127;
    return (uint8_t)(note + 0.5f);
}

// Callers using the default capacity must provide at least 3 / 5 bytes.
inline bool getNoteName(uint8_t note, char* buffer, size_t capacity = 3) {
    if (!buffer || capacity == 0) return false;
    if (note > 127) note = 127;
    char name[3];
    strcpy_P(name, NOTE_NAMES[note % 12]);
    int count = snprintf(buffer, capacity, "%s", name);
    return count >= 0 && static_cast<size_t>(count) < capacity;
}
inline bool getNoteNameWithOctave(uint8_t note, char* buffer, size_t capacity = 5) {
    if (!buffer || capacity == 0) return false;
    if (note > 127) note = 127;
    char name[3];
    strcpy_P(name, NOTE_NAMES[note % 12]);
    int count = snprintf(buffer, capacity, "%s%d", name, static_cast<int>(note / 12) - 1);
    return count >= 0 && static_cast<size_t>(count) < capacity;
}

// Common note definitions
namespace Notes {
    // Octave 4 (middle)
    const uint8_t C4  = 60;
    const uint8_t Cs4 = 61;
    const uint8_t D4  = 62;
    const uint8_t Ds4 = 63;
    const uint8_t E4  = 64;
    const uint8_t F4  = 65;
    const uint8_t Fs4 = 66;
    const uint8_t G4  = 67;
    const uint8_t Gs4 = 68;
    const uint8_t A4  = 69;  // 440 Hz reference
    const uint8_t As4 = 70;
    const uint8_t B4  = 71;

    // Octave 3
    const uint8_t C3  = 48;
    const uint8_t A3  = 57;

    // Octave 5
    const uint8_t C5  = 72;
    const uint8_t A5  = 81;
}

// Pitch bend utilities
// Standard MIDI pitch bend range is +/- 2 semitones

// Apply pitch bend to frequency
// bendValue: 0-16383 (8192 = center, no bend)
// range: semitones of max bend (default 2)
inline float applyPitchBend(float baseFreq, uint16_t bendValue, float range = 2.0f) {
    if (bendValue > 16383) bendValue = 16383;
    float bendSemitones = ((float)bendValue - 8192.0f) / 8192.0f * range;
    return baseFreq * pow(2.0f, bendSemitones / 12.0f);
}

// Detune utilities for chorus/unison effects
// Returns frequency detuned by cents (-100 to +100 cents = 1 semitone)
inline float detuneByCents(float freq, int16_t cents) {
    return freq * pow(2.0f, cents / 1200.0f);
}

} // namespace Synth

#endif // MIDI_FREQ_H
