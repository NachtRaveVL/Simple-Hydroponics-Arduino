#ifndef SPI_H
#define SPI_H
#include "Arduino.h"
class SPISettings { public: SPISettings(uint32_t=1000000, uint8_t=MSBFIRST, uint8_t=SPI_MODE0){} };
class SPIClass { public: void begin(){} void end(){} void beginTransaction(const SPISettings&){} void endTransaction(){} uint8_t transfer(uint8_t v){return v;} uint16_t transfer16(uint16_t v){return v;} };
inline SPIClass SPI; inline SPIClass SPI1;
#ifndef SPI_INTERFACES_COUNT
#define SPI_INTERFACES_COUNT 1
#endif
#endif
