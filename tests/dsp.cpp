#include "synth/synth.h"
#include <assert.h>
#include <stdio.h>
#include <limits>
#include <new>
using namespace Synth;
static void near(float actual, float expected, float tolerance) {
    if (fabs(actual - expected) > tolerance) {
        fprintf(stderr, "actual=%g expected=%g tolerance=%g\n", actual, expected, tolerance);
        assert(false);
    }
}
int main() {
    ADSR env;
    env.noteOn();
    for(int i=0;i<440;++i) { env.nextSample(); assert(env.getState()==EnvelopeState::ATTACK); }
    assert(env.nextSample()==255 && env.getState()==EnvelopeState::DECAY);
    for(int i=0;i<2205;++i) env.nextSample();
    assert(env.getState()==EnvelopeState::SUSTAIN && env.getLevel()==180);
    env.noteOff();
    for(int i=0;i<4409;++i) { env.nextSample(); env.noteOff(); assert(env.isActive()); }
    assert(env.nextSample()==0 && !env.isActive());
    env.setAttack(2000); env.noteOn();
    for(int i=0;i<88200;++i) env.nextSample();
    assert(env.getState()==EnvelopeState::DECAY && env.getLevel()==255);
    ADSR shortEnv(100); shortEnv.setAttack(1); shortEnv.noteOn();
    assert(shortEnv.nextSample()==255 && shortEnv.getState()==EnvelopeState::DECAY);
    ADSR instant; instant.setAttack(0); instant.setDecay(0); instant.setSustain(255);
    instant.noteOn(); assert(instant.apply(255)==255);
    instant.setSustain(180); assert(instant.getLevel()==180);
    instant.setRelease(0); instant.noteOff(); assert(instant.apply(0)==128);
    instant.reset(); assert(instant.apply(255)==128 && instant.applySigned(-128)==0);
    ADSR zeroAttack(1000); zeroAttack.setAttack(0); zeroAttack.setDecay(2);
    zeroAttack.noteOn(); zeroAttack.nextSample();
    assert(zeroAttack.getState()==EnvelopeState::DECAY);
    zeroAttack.nextSample(); assert(zeroAttack.getState()==EnvelopeState::SUSTAIN);
    ADSR maximum(UINT32_MAX); maximum.setAttack(UINT16_MAX); maximum.noteOn();
    for(int i=0;i<1000;++i) assert(maximum.nextSample()==0);

    for (int note=0;note<128;++note) {
        near(midiNoteToFrequency(note), midiNoteToFrequencyCalc(note), 0.006f);
        assert(frequencyToMidiNote(midiNoteToFrequency(note))==note);
    }
    assert(frequencyToMidiNote(NAN)==0 && frequencyToMidiNote(INFINITY)==127);
    char name[5]; assert(getNoteNameWithOctave(1,name,sizeof(name)) && !strcmp(name,"C#-1"));
    assert(getNoteNameWithOctave(60,name,sizeof(name)) && !strcmp(name,"C4"));
    char tiny[2]; assert(!getNoteNameWithOctave(1,tiny,sizeof(tiny)) && tiny[1]=='\0');
    near(applyPitchBend(440,8192),440,0.0001f);

    LFO lfo(1024); lfo.setRate(1); lfo.setDepth(127);
    float low=1,high=-1;
    for(int i=0;i<1024;++i) { float v=lfo.nextSampleFloat(); if(v<low)low=v;if(v>high)high=v; }
    assert(low < -0.49 && high > 0.49);
    lfo.setDepth(0); assert(lfo.modulateAmplitude(255)==255 && lfo.nextSampleFloat()==0);
    assert(lfo.modulateFrequency(440)==440);
    lfo.setDepth(255); lfo.setWaveform(Waveform::SQUARE); lfo.reset();
    assert(lfo.modulateAmplitude(255)==255);
    for(int i=0;i<512;++i) lfo.nextSample();
    assert(lfo.modulateAmplitude(255)==0);
    lfo.setDepth(127); lfo.reset(); assert(lfo.modulateAmplitude(255)==255);
    for(int i=0;i<512;++i) lfo.nextSample();
    assert(lfo.modulateAmplitude(255)==128);

    alignas(OnePoleFilter) unsigned char storage[sizeof(OnePoleFilter)];
    memset(storage,127,sizeof(storage));
    OnePoleFilter* pole = new(storage) OnePoleFilter;
    assert(pole->processSigned(0)==0);
    pole->setCoefficient(255);
    for(int i=0;i<10000;++i) pole->process(255);
    assert(pole->process(255)==255);
    pole->reset(); pole->setCoefficient(128);
    for(int i=0;i<1000;++i) pole->process(1);
    assert(pole->process(1)==1);
    pole->setCoefficient(0); assert(pole->process(255)==255 && pole->processSigned(-128)==-128);
    StateVariableFilter svf;
    for(float cutoff : {0.0f,1000.0f,22050.0f}) for(float res : {0.0f,0.5f,0.99f}) {
        svf.reset(); svf.setCutoff(cutoff); svf.setResonance(res);
        assert(svf.getCutoff()<=44100/8.0f);
        for(int i=0;i<10000;++i) {
            svf.process(i%2?32767:-32768);
            (void)svf.lowPass(); (void)svf.highPass(); (void)svf.bandPass(); (void)svf.notch();
            (void)svf.lowPass8(); (void)svf.highPass8(); (void)svf.bandPass8();
        }
        for(int i=0;i<10000;++i) svf.process(0);
    }
    svf.setSampleRate(0); svf.setCutoff(NAN); svf.setResonance(INFINITY); svf.process(32767);
    MoogFilter moog;
    for(int i=0;i<256;++i) moog.process8(i);
    moog.setSampleRate(0); moog.setCutoff(NAN); moog.setResonance(INFINITY); moog.process(-32768);
    DCBlocker dc; assert(dc.process(32767)==32767); assert(dc.process(-32768)==-32768);
    DCBlocker invalid(NAN); assert(invalid.process(1)==1);

    Oscillator osc(44100); osc.setFrequency(8.1758f); osc.setWaveform(Waveform::SAWTOOTH);
    int wraps=0; uint8_t previous=osc.nextSample();
    for(int i=0;i<441000;++i) { uint8_t value=osc.nextSample();if(value<previous)++wraps; previous=value; }
    assert(wraps==81); // Old 16-bit tuning wraps only80 times in ten seconds.
    osc.setSampleRate(0); osc.setFrequency(INFINITY); assert(osc.getFrequency()==0);
    osc.setFrequency(100000); assert(osc.getFrequency()==22050);
    puts("DSP regression tests passed");
}
