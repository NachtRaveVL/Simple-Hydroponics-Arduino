#ifndef DHT_H
#define DHT_H
#include "Arduino.h"
#define DHT11 11
#define DHT12 12
#define DHT21 21
#define DHT22 22
#define AM2301 21
class DHT { public: DHT(uint8_t=0,uint8_t=0,uint8_t=6){} void begin(uint8_t=55){} float readTemperature(bool=false,bool=false){return 25.0f;} float readHumidity(bool=false){return 50.0f;} float computeHeatIndex(float t,float,bool=false){return t;} };
#endif
