#ifndef MIDI_FREQ_H
#define MIDI_FREQ_H

#include <Arduino.h>

namespace Synth {

// MIDI note to frequency conversion
// A4 (MIDI note 69) = 440 Hz standard tuning
//
// Usage:
//   float freq = midiNoteToFrequency(60);  // C4 = 261.63 Hz
//   uint8_t note = frequencyToMidiNote(440.0);  // A4 = 69
//   const char* name = noteName(60);  // "C4"

// Pre-computed frequency table for all 128 MIDI notes
// Stored as 16-bit fixed point (frequency * 100) to save memory
// For notes 0-127 (C-1 to G9)
const uint16_t MIDI_FREQ_TABLE[128] PROGMEM = {
    // Octave -1 (notes 0-11): C-1 to B-1
    817, 866, 918, 972, 1030, 1091, 1156, 1225, 1297, 1375, 1457, 1543,
    // Octave 0 (notes 12-23): C0 to B0
    1635, 1732, 1835, 1945, 2060, 2183, 2312, 2450, 2596, 2750, 2914, 3087,
    // Octave 1 (notes 24-35): C1 to B1
    3270, 3465, 3671, 3889, 4120, 4365, 4625, 4900, 5191, 5500, 5827, 6174,
    // Octave 2 (notes 36-47): C2 to B2
    6541, 6930, 7342, 7778, 8241, 8731, 9250, 9800, 10383, 11000, 11654, 12347,
    // Octave 3 (notes 48-59): C3 to B3
    13081, 13859, 14683, 15556, 16481, 17461, 18500, 19600, 20765, 22000, 23308, 24694,
    // Octave 4 (notes 60-71): C4 to B4 - Middle C octave
    26163, 27718, 29366, 31113, 32963, 34923, 36999, 39200, 41530, 44000, 46616, 49388,
    // Octave 5 (notes 72-83): C5 to B5
    52325, 55437, 58733, 62225, 65926, 69846, 73999, 78399, 83061, 88000, 93233, 98777,
    // Octave 6 (notes 84-95): C6 to B6
    // Values exceed uint16_t max, stored as freq/10
    10465, 11087, 11747, 12445, 13185, 13969, 14800, 15680, 16612, 17600, 18647, 19755,
    // Octave 7 (notes 96-107): C7 to B7 (stored as freq/10)
    20930, 22175, 23493, 24890, 26370, 27938, 29600, 31360, 33224, 35200, 37293, 39511,
    // Octave 8 (notes 108-119): C8 to B8 (stored as freq/10)
    41860, 44349, 46986, 49780, 52740, 55877, 59199, 62719, 66449, 70400, 74586, 79021,
    // Octave 9 (notes 120-127): C9 to G9 (stored as freq/10)
    83720, 88698, 93973, 99561, 105481, 111753, 118398, 125439
};

// Note names for display
const char NOTE_NAMES[12][3] PROGMEM = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// Convert MIDI note number to frequency in Hz
inline float midiNoteToFrequency(uint8_t note) {
    if (note > 127) note = 127;

    uint16_t tableValue = pgm_read_word(&MIDI_FREQ_TABLE[note]);

    // Notes 0-83 are stored as freq*100, notes 84-127 as freq/10
    if (note < 84) {
        return tableValue / 100.0f;
    } else {
        return tableValue * 10.0f;
    }
}

// Convert MIDI note to frequency using calculation (more precise but slower)
inline float midiNoteToFrequencyCalc(uint8_t note) {
    // f = 440 * 2^((note - 69) / 12)
    return 440.0f * pow(2.0f, (note - 69) / 12.0f);
}

// Convert frequency to nearest MIDI note
inline uint8_t frequencyToMidiNote(float freq) {
    // note = 12 * log2(freq / 440) + 69
    if (freq <= 0) return 0;
    float note = 12.0f * log2(freq / 440.0f) + 69.0f;
    if (note < 0) return 0;
    if (note > 127) return 127;
    return (uint8_t)(note + 0.5f);
}

// Get note name (e.g., "C", "C#", "D")
inline void getNoteName(uint8_t note, char* buffer) {
    uint8_t noteIndex = note % 12;
    strcpy_P(buffer, NOTE_NAMES[noteIndex]);
}

// Get full note name with octave (e.g., "C4", "A#3")
inline void getNoteNameWithOctave(uint8_t note, char* buffer) {
    uint8_t noteIndex = note % 12;
    int8_t octave = (note / 12) - 1;

    strcpy_P(buffer, NOTE_NAMES[noteIndex]);
    char octaveStr[4];
    itoa(octave, octaveStr, 10);
    strcat(buffer, octaveStr);
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
