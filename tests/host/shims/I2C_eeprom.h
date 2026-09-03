#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H
#include "Arduino.h"
#include "Wire.h"
#define I2C_DEVICESIZE_24LC01 128
#define I2C_DEVICESIZE_24LC02 256
#define I2C_DEVICESIZE_24LC04 512
#define I2C_DEVICESIZE_24LC08 1024
#define I2C_DEVICESIZE_24LC16 2048
#define I2C_DEVICESIZE_24LC32 4096
#define I2C_DEVICESIZE_24LC64 8192
#define I2C_DEVICESIZE_24LC128 16384
#define I2C_DEVICESIZE_24LC256 32768
#define I2C_DEVICESIZE_24LC512 65536
class I2C_eeprom { public: I2C_eeprom(uint8_t=0x50,uint32_t=32768,TwoWire *wire=&Wire){} bool begin(){return true;} bool isConnected(){return true;} uint8_t readByte(uint32_t){return 0;} int writeByte(uint32_t,uint8_t){return 0;} uint32_t readBlock(uint32_t,uint8_t*buf,uint32_t n){std::memset(buf,0,n);return n;} uint32_t writeBlock(uint32_t,const uint8_t*,uint32_t n){return n;} uint32_t getSize(){return 32768;} uint32_t getDeviceSize(){return 32768;} bool updateBlockVerify(uint32_t,const uint8_t*,uint32_t){return true;} bool updateByteVerify(uint32_t,uint8_t){return true;} };
#endif
