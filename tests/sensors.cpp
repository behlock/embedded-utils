#include "MLX90614/include/heater_control.h"
#include "Adafruit_MLX90614.h"
#include "Adafruit_MLX90614.h"
#include <assert.h>
#include <stdio.h>
static void near(float a,float b) { assert(fabs(a-b)<0.005f); }
int main() {
    float t;
    assert(MLX::decodeSinglePWM(400,600,0,50,t)); near(t,27.5f);
    assert(MLX::decodeSinglePWM(125,875,0,125,t)); near(t,0);
    assert(MLX::decodeSinglePWM(625,375,0,125,t)); near(t,125);
    assert(!MLX::decodeSinglePWM(875,125,0,125,t) && isnan(t));
    assert(!MLX::decodeSinglePWM(0,0,0,125,t));
    assert(!MLX::decodeSinglePWM(400,600,50,0,t));
    Fake::pulses={400,600}; assert(MLX::readPWMTemperature(4,t)); near(t,68.75f);
    assert(Fake::timeouts.back()==20000);
    Fake::pulses={0}; assert(!MLX::readPWMTemperature(4,t) && isnan(t));
    MLX::HeaterControl heater;
    assert(heater.update(20,0)); assert(heater.update(39.9f,10));
    assert(!heater.update(40,20)); assert(!heater.update(39.8f,30)); assert(heater.update(39,40));
    assert(!heater.update(NAN,50) && heater.hasFault()); assert(!heater.update(20,60));
    MLX::HeaterControl timed(40,100); assert(timed.update(20,10));
    assert(!timed.update(20,110) && timed.hasFault());
    MLX::HeaterControl overheated; assert(!overheated.update(50,0) && overheated.hasFault());
    MLX::HeaterControl wrapped(40,100);
    assert(wrapped.update(20,UINT32_MAX-49));
    assert(!wrapped.update(20,50) && wrapped.hasFault());
    const uint8_t check[] = {'1','2','3','4','5','6','7','8','9'};
    assert(Fake::pec(check,sizeof(check))==0xF4); // CRC-8/SMBus check vector.
    Adafruit_MLX90614::requestSMBus();
    assert(Fake::modes[Fake::modes.size()-2]==std::make_pair(int(SCL),int(OUTPUT)));
    assert(Fake::modes.back()==std::make_pair(int(SCL),int(INPUT)));
    Adafruit_MLX90614 sensor;
    assert(isnan(sensor.readObjectTempC()) && isnan(sensor.readTempMin()));
    assert(!sensor.writeEmissivity(0.3) && sensor.getCommunicationMode()=="Unknown");
    assert(sensor.readI2CAddr()==0xFFFF);
    Fake::bus().detected=false; assert(!sensor.begin()); assert(isnan(sensor.readObjectTempC()));
    Fake::bus().detected=true; assert(sensor.begin());
    near(sensor.readObjectTempC(),26.85f);
    assert(sensor.readI2CAddr()==0x5A && Fake::bus().reads.back()==0x2E);
    Fake::bus().words[0x07] |= 0x8000; assert(isnan(sensor.readObjectTempC()));
    Fake::bus().words[0x07]=15000; Fake::bus().badPEC=true;
    assert(isnan(sensor.readObjectTempC()) && sensor.getCommunicationMode()=="Unknown");
    assert(!sensor.writeTempMin(0) && Fake::bus().writes.empty());
    Fake::bus().badPEC=false;
    assert(sensor.writeTemperatureRange(0,125));
    assert(Fake::bus().words[0x21]==27315 && Fake::bus().words[0x20]==39815);
    near(sensor.readTempMin(),0); near(sensor.readTempMax(),125);
    size_t writes=Fake::bus().writes.size(); assert(sensor.writeTemperatureRange(0,125));
    assert(Fake::bus().writes.size()==writes);
    assert(!sensor.writeTemperatureRange(125,0) && !sensor.writeTempMax(NAN));
    assert(sensor.writeTemperatureRange(150,200)); // Raise max before min.
    assert(sensor.writeTemperatureRange(-10,125));
    Fake::bus().writeFail=true; assert(!sensor.writeTempMin(1));
    Fake::bus().writeFail=false; Fake::bus().ignoreWrites=true;
    assert(!sensor.writeTempMin(1)); Fake::bus().ignoreWrites=false;
    assert(!sensor.writeEmissivity(0.01) && !sensor.writeEmissivity(INFINITY));
    assert(sensor.writeEmissivity(0.3)); near(sensor.readEmissivity(),0.3f);
    uint16_t original=Fake::bus().words[0x25];
    assert(sensor.switchToPWM());
    assert(Fake::bus().words[0x25]==((original & ~uint16_t(0x30)) | 0x30));
    assert(Fake::bus().words[0x22]==0x207 && sensor.getCommunicationMode()=="PWM");
    assert(sensor.switchToI2C() && sensor.getCommunicationMode()=="I2C");
    assert(!sensor.switchToPWM(0));
    puts("Sensor and heater regression tests passed");
}
