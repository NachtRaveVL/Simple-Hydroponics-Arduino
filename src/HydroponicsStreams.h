
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Streams
*/

#ifndef HydroponicsStreams_H
#define HydroponicsStreams_H

class HydroponicsEEPROMStream;

#include "Hydroponics.h"

class HydroponicsEEPROMStream : public Stream {
public:
    HydroponicsEEPROMStream();
    HydroponicsEEPROMStream(size_t dataAddress, size_t dataSize);

    int available() override;
    int read() override;
    int peek() override;
    void flush() override;
    size_t write(const uint8_t *buffer, size_t size) override;
    size_t write(uint8_t data) override;

private:
    I2C_eeprom *_eeprom;
    size_t _readAddress, _writeAddress, _end;
};

#endif // /ifndef HydroponicsStreams_H
