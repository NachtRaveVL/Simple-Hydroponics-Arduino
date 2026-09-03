#ifndef DALLAS_TEMPERATURE_H
#define DALLAS_TEMPERATURE_H
#include "Arduino.h"
#include "OneWire.h"
typedef uint8_t DeviceAddress[8];
class DallasTemperature {
public:
    explicit DallasTemperature(OneWire *oneWire = nullptr) : _oneWire(oneWire), _resolution(12), _waitForConversion(true) { ; }
    void setOneWire(OneWire *oneWire) { _oneWire = oneWire; }
    void setPullupPin(uint8_t) { ; }
    void setWaitForConversion(bool waitForConversion) { _waitForConversion = waitForConversion; }
    void begin() { ; }
    bool getAddress(uint8_t *, uint8_t) { return false; }
    uint8_t getResolution() const { return _resolution; }
    void setResolution(uint8_t resolution) { _resolution = resolution; }
    void setResolution(const uint8_t *, uint8_t resolution) { _resolution = resolution; }
    void requestTemperatures() { ; }
    bool requestTemperaturesByAddress(const uint8_t *) { return true; }
    float getTempC(const uint8_t *) { return 25.0f; }
    float getTempF(const uint8_t *) { return 77.0f; }
    float getTempCByIndex(uint8_t) { return 25.0f; }
    uint8_t getDeviceCount() { return 0; }
private:
    OneWire *_oneWire;
    uint8_t _resolution;
    bool _waitForConversion;
};
#ifndef DEVICE_DISCONNECTED_C
#define DEVICE_DISCONNECTED_C -127.0f
#endif
#ifndef DEVICE_DISCONNECTED_F
#define DEVICE_DISCONNECTED_F -196.6f
#endif
#endif
