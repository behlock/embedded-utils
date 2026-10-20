#ifndef TEST_I2C_DEVICE_H
#define TEST_I2C_DEVICE_H
#include <Arduino.h>
#include <array>
struct TwoWire {};
extern TwoWire Wire;
namespace Fake {
struct Bus {
    std::array<uint16_t, 64> words;
    std::vector<std::array<uint8_t, 4> > writes;
    std::vector<uint8_t> reads;
    uint8_t address;
    bool detected, readFail, writeFail, badPEC, ignoreWrites;
    Bus() : address(0x5A), detected(true), readFail(false), writeFail(false),
            badPEC(false), ignoreWrites(false) {
        words.fill(0);
        words[0x20] = 39315; words[0x21] = 25315;
        words[0x24] = 65535; words[0x25] = 0xB7C4;
        words[0x2E] = 0x5A; words[0x07] = 15000;
    }
};
inline Bus& bus() { static Bus value; return value; }
inline uint8_t pec(const uint8_t* data, size_t size) {
    uint8_t crc = 0;
    for (size_t n = 0; n < size; ++n) {
        crc ^= data[n];
        for (int i=0; i<8; ++i) crc = (crc & 0x80) ? (crc << 1) ^ 7 : crc << 1;
    }
    return crc;
}
}
class Adafruit_I2CDevice {
public:
    Adafruit_I2CDevice(uint8_t addr, TwoWire*) : address(addr) {}
    bool begin() { Fake::bus().address = address; return Fake::bus().detected; }
    bool write_then_read(const uint8_t* request, size_t n, uint8_t* result, size_t size) {
        Fake::Bus& bus = Fake::bus();
        if (n != 1 || size != 3 || *request >= 64 || bus.readFail) return false;
        bus.reads.push_back(*request);
        uint16_t word = bus.words[*request];
        result[0] = word; result[1] = word >> 8;
        uint8_t frame[] = {static_cast<uint8_t>(address << 1), *request,
            static_cast<uint8_t>((address << 1) | 1), result[0], result[1]};
        result[2] = Fake::pec(frame, sizeof(frame)) ^ (bus.badPEC ? 1 : 0);
        return true;
    }
    bool write(const uint8_t* data, size_t size) {
        Fake::Bus& bus = Fake::bus();
        if (size != 4 || data[0] >= 64) return false;
        std::array<uint8_t, 4> record = {{data[0],data[1],data[2],data[3]}};
        bus.writes.push_back(record);
        uint8_t frame[] = {static_cast<uint8_t>(address << 1), data[0], data[1], data[2]};
        if (bus.writeFail || Fake::pec(frame, sizeof(frame)) != data[3]) return false;
        if (!bus.ignoreWrites) bus.words[data[0]] = data[1] | (static_cast<uint16_t>(data[2]) << 8);
        return true;
    }
private:
    uint8_t address;
};
#endif
