/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Streams
*/

#include "Hydroponics.h"

HydroponicsEEPROMStream::HydroponicsEEPROMStream()
    : Stream(), _eeprom(nullptr), _readAddress(0), _writeAddress(0), _end(0)
{
    if (getHydroponicsInstance() && (_eeprom = getHydroponicsInstance()->getEEPROM())) {
        _end = _eeprom->getDeviceSize();
    }
    HYDRUINO_HARD_ASSERT(_eeprom, SFP(HStr_Err_UnsupportedOperation));
}

HydroponicsEEPROMStream::HydroponicsEEPROMStream(uint16_t dataAddress, size_t dataSize)
      : Stream(), _eeprom(nullptr), _readAddress(dataAddress), _writeAddress(dataAddress), _end(dataAddress + dataSize)
{
    if (getHydroponicsInstance()) {
        _eeprom = getHydroponicsInstance()->getEEPROM();
    }
    HYDRUINO_HARD_ASSERT(_eeprom, SFP(HStr_Err_UnsupportedOperation));
}

int HydroponicsEEPROMStream::available()
{
    return _eeprom ? ((int)_end - _readAddress) : 0;
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
        HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
        return 0;
    }
}

size_t HydroponicsEEPROMStream::write(uint8_t data) {
    if (!_eeprom || _writeAddress >= _end) { return 0; }
    if (_eeprom->updateByteVerify(_writeAddress, data)) {
        _writeAddress += 1;
        return 1;
    } else {
        HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
        return 0;
    }
}


HydroponicsPROGMEMStream::HydroponicsPROGMEMStream()
    : Stream(), _readAddress(0), _writeAddress(0), _end(UINT16_MAX)
{ ; }

HydroponicsPROGMEMStream::HydroponicsPROGMEMStream(uint16_t dataAddress)
    : Stream(), _readAddress(dataAddress), _writeAddress(dataAddress), _end(dataAddress + strlen_P((const char *)dataAddress))
{ ; }

HydroponicsPROGMEMStream::HydroponicsPROGMEMStream(uint16_t dataAddress, size_t dataSize)
    : Stream(), _readAddress(dataAddress), _writeAddress(dataAddress), _end(dataAddress + dataSize)
{ ; }

int HydroponicsPROGMEMStream::available()
{
    return (int)_end - _readAddress;
}

int HydroponicsPROGMEMStream::read()
{
    if (_readAddress >= _end) { return -1; }
    byte value = (byte)-1;
    memcpy_P(&value, (const void *)_readAddress++, sizeof(value));
    return value;
}

int HydroponicsPROGMEMStream::peek()
{
    if (_readAddress >= _end) { return -1; }
    byte value = (byte)-1;
    memcpy_P(&value, (const void *)_readAddress, sizeof(value));
    return value;
}

void HydroponicsPROGMEMStream::flush()
{ ; }

size_t HydroponicsPROGMEMStream::write(const uint8_t *buffer, size_t size)
{
    HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
    return 0;
}

size_t HydroponicsPROGMEMStream::write(uint8_t data) {
    HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
    return 0;
}
