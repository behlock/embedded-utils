#include "pwm_audio/pwm_audio.h"
#include <assert.h>
#include <stdio.h>
static uint8_t sample() { return 42; }
int main() {
    using Synth::PWMAudio;
#if defined(__AVR_ATmega2560__)
    static_assert(PWMAudio::TIMER1_A==11 && PWMAudio::TIMER1_B==12,"Mega timer1");
    static_assert(PWMAudio::TIMER2_A==10 && PWMAudio::TIMER2_B==9,"Mega timer2");
#else
    static_assert(PWMAudio::TIMER1_A==9 && PWMAudio::TIMER1_B==10,"Uno timer1");
    static_assert(PWMAudio::TIMER2_A==11 && PWMAudio::TIMER2_B==3,"Uno timer2");
#endif
    for(auto pin : {PWMAudio::TIMER1_A,PWMAudio::TIMER1_B,PWMAudio::TIMER2_A,PWMAudio::TIMER2_B}) {
        PWMAudio audio;
        SREG=0; assert(audio.begin(pin)); assert(SREG==0);
        audio.write16(65535);
        if(pin==PWMAudio::TIMER1_A) assert(OCR1A==255);
        if(pin==PWMAudio::TIMER1_B) assert(OCR1B==255);
        if(pin==PWMAudio::TIMER2_A) assert(OCR2A==255);
        if(pin==PWMAudio::TIMER2_B) assert(OCR2B==255);
        assert(!audio.setSampleRate(0)); assert(audio.setSampleRate(1));
        assert(audio.getSampleDelayMicros()==1000000UL);
        SREG=0x80; audio.end(); assert(SREG==0x80);
        assert(audio.begin()); assert(SREG==0x80); audio.end();
    }
    auto& isr=Synth::PWMAudioISR::instance();
    assert(!isr.begin(0,sample) && !isr.begin(1,sample) && !isr.begin(22050,nullptr));
    SREG=0; TCNT1=60000;
    assert(isr.begin(22050,sample) && SREG==0 && TCNT1==0);
    assert(OCR1A==725); assert(isr.getSampleRate()==16000000UL/726);
    isr.handleInterrupt(); assert(OCR2A==42);
    assert(!isr.begin(UINT32_MAX,sample));
    SREG=0x80; isr.end(); assert(SREG==0x80 && TIMSK1==0 && TCCR2B==0);
    OCR2A=128; isr.handleInterrupt(); assert(OCR2A==128);
    puts("PWM audio register tests passed");
}
