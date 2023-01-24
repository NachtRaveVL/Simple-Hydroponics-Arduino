/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Streams
*/

#include "Hydruino.h"

HydroEEPROMStream::HydroEEPROMStream()
    : Stream(), _eeprom(nullptr), _readAddress(0), _writeAddress(0), _endAddress(0)
{
    if (getHydroInstance() && (_eeprom = getHydroInstance()->getEEPROM())) {
        _endAddress = _eeprom->getDeviceSize();
    }
    HYDRO_HARD_ASSERT(_eeprom, SFP(HStr_Err_UnsupportedOperation));
}

HydroEEPROMStream::HydroEEPROMStream(uint16_t dataAddress, size_t dataSize)
      : Stream(), _eeprom(nullptr), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + dataSize)
{
    if (getHydroInstance()) {
        _eeprom = getHydroInstance()->getEEPROM();
    }
    HYDRO_HARD_ASSERT(_eeprom, SFP(HStr_Err_UnsupportedOperation));
}

int HydroEEPROMStream::available()
{
    return _eeprom ? ((int)_endAddress - _readAddress) : 0;
}

int HydroEEPROMStream::read()
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    return (int)_eeprom->readByte(_readAddress++);
}

size_t HydroEEPROMStream::readBytes(char *buffer, size_t length)
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    size_t retVal = _eeprom->readBlock(_readAddress, (uint8_t *)buffer, length);
    _readAddress += retVal;
    return retVal;
}

int HydroEEPROMStream::peek()
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    return (int)_eeprom->readByte(_readAddress);
}

void HydroEEPROMStream::flush()
{
    //_eeprom->commit();
}

size_t HydroEEPROMStream::write(const uint8_t *buffer, size_t size)
{
    if (!_eeprom || _writeAddress >= _endAddress) { return 0; }
    size_t remaining = _endAddress - _writeAddress;
    if (size > remaining) { size = remaining; }
    if (_eeprom->updateBlockVerify(_writeAddress, buffer, size)) {
        _writeAddress += size;
        return size;
    } else {
        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
        return 0;
    }
}

size_t HydroEEPROMStream::write(uint8_t data)
{
    if (!_eeprom || _writeAddress >= _endAddress) { return 0; }
    if (_eeprom->updateByteVerify(_writeAddress, data)) {
        _writeAddress += 1;
        return 1;
    } else {
        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
        return 0;
    }
}

int HydroEEPROMStream::availableForWrite()
{
    return _eeprom ? ((int)_endAddress - _writeAddress) : 0;
}


HydroPROGMEMStream::HydroPROGMEMStream()
    : Stream(), _readAddress(0), _writeAddress(0), _endAddress(UINTPTR_MAX)
{ ; }

HydroPROGMEMStream::HydroPROGMEMStream(uintptr_t dataAddress)
    : Stream(), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + strlen_P((const char *)dataAddress))
{ ; }

HydroPROGMEMStream::HydroPROGMEMStream(uintptr_t dataAddress, size_t dataSize)
    : Stream(), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + dataSize)
{ ; }

int HydroPROGMEMStream::available()
{
    return _endAddress - _readAddress;
}

int HydroPROGMEMStream::read()
{
    if (_readAddress >= _endAddress) { return -1; }
    #ifdef ESP8266
        return pgm_read_byte((const void *)(_readAddress++));
    #else
        return pgm_read_byte(_readAddress++);
    #endif
}

int HydroPROGMEMStream::peek()
{
    if (_readAddress >= _endAddress) { return -1; }
    #ifdef ESP8266
        return pgm_read_byte((const void *)(_readAddress));
    #else
        return pgm_read_byte(_readAddress);
    #endif
}

void HydroPROGMEMStream::flush()
{ ; }

size_t HydroPROGMEMStream::write(const uint8_t *buffer, size_t size)
{
    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
    return 0;
}

size_t HydroPROGMEMStream::write(uint8_t data)
{
    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
    return 0;
}


#ifdef HYDRO_USE_WIFI_STORAGE

HydroWiFiStorageFileStream::HydroWiFiStorageFileStream(WiFiStorageFile file, uintptr_t seekPos)
    : Stream(), _file(file), _bufferOffset(0), _bufferFileOffset(-1), _bufferDirection(None), _readOffset(0), _writeOffset(0), _endOffset(0)
{
    if (_file) {
        _endOffset = _file.size();
        _readOffset = _writeOffset = seekPos;
    }
}

HydroWiFiStorageFileStream::~HydroWiFiStorageFileStream()
{
    if (_file) {
        if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
            _file.seek(_bufferFileOffset);
            _file.write((const void*)_buffer, _bufferOffset); _bufferOffset = 0;
        }
    }
}

int HydroWiFiStorageFileStream::available()
{
    return _file ? _endOffset - _readOffset : 0;
}

int HydroWiFiStorageFileStream::read()
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    prepareReadBuffer();
    _readOffset++;
    return _buffer[_bufferOffset++];
}

size_t HydroWiFiStorageFileStream::readBytes(char *buffer, size_t length)
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    while (length && _readOffset < _endOffset) {
        prepareReadBuffer();
        size_t howMany = min(length, _endOffset - _readOffset);
        howMany = min(howMany, HYDRO_WIFISTREAM_BUFFER_SIZE - _bufferOffset);
        memcpy(buffer, &_buffer[_bufferOffset], howMany);
        _readOffset += howMany;
        _bufferOffset += howMany;
        buffer += howMany;
        length -= howMany;
    }
}

int HydroWiFiStorageFileStream::peek()
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    prepareReadBuffer();
    return _buffer[_bufferOffset];
}

void HydroWiFiStorageFileStream::flush()
{
    if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
        _file.seek(_bufferFileOffset);
        _file.write((const void*)_buffer, _bufferOffset); _bufferOffset = 0;
    }
}

size_t HydroWiFiStorageFileStream::write(const uint8_t *buffer, size_t size)
{
    if (!_file || _writeOffset >= _endOffset) { return -1; }
    while (size) {
        prepareWriteBuffer();
        size_t howMany = min(size, HYDRO_WIFISTREAM_BUFFER_SIZE - _bufferOffset);
        memcpy(&_buffer[_bufferOffset], buffer, howMany);
        _writeOffset += howMany;
        _bufferOffset += howMany;
        buffer += howMany;
        size -= howMany;
    }
}

size_t HydroWiFiStorageFileStream::write(uint8_t data)
{
    if (!_file || _writeOffset >= _endOffset) { return -1; }
    prepareWriteBuffer();
    _buffer[_bufferOffset++] = data;
    _writeOffset++;
    return 1;
}

int HydroWiFiStorageFileStream::availableForWrite() 
{
    return _file ? _endOffset - _writeOffset : 0;
}

void HydroWiFiStorageFileStream::prepareReadBuffer()
{
    if (_bufferDirection != ReadBuffer || _bufferFileOffset == -1 || _readOffset < _bufferFileOffset || _readOffset >= _bufferFileOffset + HYDRO_WIFISTREAM_BUFFER_SIZE) {
        if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
            _file.seek(_bufferFileOffset);
            _file.write((const void*)_buffer, _bufferOffset); //_bufferOffset = 0;
        }
        _bufferDirection = ReadBuffer;
        _bufferFileOffset = _readOffset;
        _bufferOffset = 0;

        _file.seek(_bufferFileOffset);
        _file.read((void *)_buffer, HYDRO_WIFISTREAM_BUFFER_SIZE);
    }
}

void HydroWiFiStorageFileStream::prepareWriteBuffer()
{
    if (_bufferDirection != WriteBuffer || _bufferFileOffset == -1 || _writeOffset < _bufferFileOffset || _writeOffset >= _bufferFileOffset + HYDRO_WIFISTREAM_BUFFER_SIZE) {
        if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
            _file.seek(_bufferFileOffset);
            _file.write((const void*)_buffer, _bufferOffset); //_bufferOffset = 0;
        }
        _bufferDirection = WriteBuffer;
        _bufferFileOffset = _writeOffset;
        _bufferOffset = 0;
    }
}

#endif
