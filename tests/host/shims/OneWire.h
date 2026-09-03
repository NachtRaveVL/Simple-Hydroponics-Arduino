#ifndef ONEWIRE_H
#define ONEWIRE_H
#include "Arduino.h"
class OneWire { public: explicit OneWire(uint8_t=0){} void reset_search(){} bool search(uint8_t*){return false;} uint8_t reset(){return 1;} void select(const uint8_t*){} void write(uint8_t,uint8_t=0){} void depower(){} uint8_t read(){return 0;} static uint8_t crc8(const uint8_t*,uint8_t){return 0;} };
#endif
