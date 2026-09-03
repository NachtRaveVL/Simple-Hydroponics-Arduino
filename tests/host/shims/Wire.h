#ifndef WIRE_H
#define WIRE_H
#include "Arduino.h"
class TwoWire { public: void begin(){} void end(){} void beginTransmission(uint8_t){} uint8_t endTransmission(bool=true){return 0;} size_t write(uint8_t){return 1;} size_t write(const uint8_t*,size_t n){return n;} uint8_t requestFrom(uint8_t,uint8_t){return 0;} int available(){return 0;} int read(){return -1;} void setClock(uint32_t){} };
inline TwoWire Wire; inline TwoWire Wire1;
#ifndef WIRE_INTERFACES_COUNT
#define WIRE_INTERFACES_COUNT 1
#endif
#ifndef WIRE_HOWMANY
#define WIRE_HOWMANY 1
#endif
#endif
