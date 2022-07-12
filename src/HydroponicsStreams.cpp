/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Streams
*/

#include "Hydroponics.h"

HydroponicsEEPROMStream::HydroponicsEEPROMStream()
    : Stream(), _eeprom(nullptr), _readAddress(0), _writeAddress(0), _end(0)
{
    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) {
        if ((_eeprom = hydroponics->getEEPROM())) {
            _end = _eeprom->getDeviceSize();
        }
    }
    HYDRUINO_HARD_ASSERT(_eeprom, F("EEPROM not available"));
}

HydroponicsEEPROMStream::HydroponicsEEPROMStream(size_t dataAddress, size_t dataSize)
      : Stream(), _eeprom(nullptr), _readAddress(dataAddress), _writeAddress(dataAddress), _end(dataAddress + dataSize)
{
    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) { _eeprom = hydroponics->getEEPROM(); }
    HYDRUINO_HARD_ASSERT(_eeprom, F("EEPROM not available"));
}

int HydroponicsEEPROMStream::available()
{
    return _eeprom ? (int)(_end - _readAddress) : 0;
}

int HydroponicsEEPROMStream::read()
{
    if (!_eeprom || _readAddress >= _end) { return -1; }
    return (int)_eeprom->readByte(_readAddress++);
}

int HydroponicsEEPROMStream::peek()
{
    if (!_eeprom || _readAddress >= _end) { return -1; }
    return (int)_eeprom->readByte(_readAddress);
}

void HydroponicsEEPROMStream::flush()
{
    //_eeprom->commit();
}

size_t HydroponicsEEPROMStream::write(const uint8_t *buffer, size_t size)
{
    if (!_eeprom || _writeAddress >= _end) { return 0; }
    size_t remaining = _end - _writeAddress;
    if (size > remaining) { size = remaining; }
    if (_eeprom->updateBlockVerify(_writeAddress, buffer, size)) {
        _writeAddress += size;
        return size;
    } else {
        HYDRUINO_SOFT_ASSERT(false, F("Failure verifying written data to EEPROM"));
        return 0;
    }
}

size_t HydroponicsEEPROMStream::write(uint8_t data) {
    if (!_eeprom || _writeAddress >= _end) { return 0; }
    if (_eeprom->updateByteVerify(_writeAddress, data)) {
        _writeAddress += 1;
        return 1;
    } else {
        HYDRUINO_SOFT_ASSERT(false, F("Failure verifying written data to EEPROM"));
        return 0;
    }
}