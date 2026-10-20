#include "synth/synth.h"
#include "pwm_audio/pwm_audio.h"

Synth::Oscillator oscillator(22050);
Synth::ADSR envelope(22050);
Synth::LFO modulation(22050);
Synth::OnePoleFilter smoother;
Synth::StateVariableFilter svf(22050);
Synth::MoogFilter moog(22050);
Synth::DCBlocker dc;

// A small, deterministic ISR example. Floating-point filters are exercised in
// setup for build coverage; benchmark them before moving them into an audio ISR.
uint8_t sample() { return envelope.apply(oscillator.nextSample()); }
ISR(TIMER1_COMPA_vect) { Synth::PWMAudioISR::instance().handleInterrupt(); }
void setup() {
    oscillator.setFrequency(Synth::midiNoteToFrequency(Synth::Notes::A4));
    oscillator.setFrequency(Synth::midiNoteToFrequencyCalc(Synth::Notes::A4));
    oscillator.setFrequency(Synth::detuneByCents(Synth::applyPitchBend(440,8192),0));
    char name[5]; Synth::getNoteNameWithOctave(60,name,sizeof(name));
    (void)Synth::frequencyToMidiNote(440);
    (void)modulation.nextSampleFloat(); (void)modulation.modulateAmplitude(255);
    svf.setCutoff(1000); svf.process(-32768); (void)svf.notch();
    (void)smoother.processSigned(-128); (void)moog.process8(0); (void)dc.process(-32768);
    envelope.noteOn();
    Synth::PWMAudioISR::instance().begin(22050,sample);
}
void loop() {}
