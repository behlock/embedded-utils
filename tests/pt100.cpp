#include "E3D PT100 Amplifier/src/read_temp.cpp"
#include <assert.h>
#include <stdio.h>
int main() {
    for(int i=0;i<numDataPoints;++i) assert(fabs(interpolateTemperature(vOut[i])-temperatures[i])<0.001f);
    for(int raw=0;raw<=1023;++raw) {
        float voltage=raw*5.0f/1023;
        float result=interpolateTemperature(voltage);
        if(voltage>vOut[numDataPoints-1]) assert(isnan(result)); else assert(isfinite(result));
    }
    assert(isnan(interpolateTemperature(NAN)) && isnan(interpolateTemperature(-1)));
    puts("PT100 interpolation regression tests passed");
}
