
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Streams
*/

#ifndef HydroponicsStreams_H
#define HydroponicsStreams_H

class HydroponicsEEPROMStream;
class HydroponicsPROGMEMStream;

#include "Hydroponics.h"

// EEPROM Stream
// Stream class for working with I2C_EEPROM data.
class HydroponicsEEPROMStream : public Stream {
public:
    HydroponicsEEPROMStream();
    HydroponicsEEPROMStream(uint16_t dataAddress, size_t dataSize);

    virtual int available() override;
    virtual int read() override;
    virtual int peek() override;
    virtual void flush() override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual size_t write(uint8_t data) override;

private:
    I2C_eeprom *_eeprom;
    uint16_t _readAddress, _writeAddress, _end;
};


// PROGMEM Stream
// Stream class for working with PROGMEM data.
class HydroponicsPROGMEMStream : public Stream {
public:
    HydroponicsPROGMEMStream();
    HydroponicsPROGMEMStream(uintptr_t dataAddress);
    HydroponicsPROGMEMStream(uintptr_t dataAddress, size_t dataSize);

    virtual int available() override;
    virtual int read() override;
    virtual int peek() override;
    virtual void flush() override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual size_t write(uint8_t data) override;

private:
    uintptr_t _readAddress, _writeAddress, _end;
};

#endif // /ifndef HydroponicsStreams_H
