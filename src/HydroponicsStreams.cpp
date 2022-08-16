/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Streams
*/

#include "Hydroponics.h"

HydroponicsEEPROMStream::HydroponicsEEPROMStream()
    : Stream(), _eeprom(nullptr), _readAddress(0), _writeAddress(0), _endAddress(0)
{
    if (getHydroponicsInstance() && (_eeprom = getHydroponicsInstance()->getEEPROM())) {
        _endAddress = _eeprom->getDeviceSize();
    }
    HYDRUINO_HARD_ASSERT(_eeprom, SFP(HStr_Err_UnsupportedOperation));
}

HydroponicsEEPROMStream::HydroponicsEEPROMStream(uint16_t dataAddress, size_t dataSize)
      : Stream(), _eeprom(nullptr), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + dataSize)
{
    if (getHydroponicsInstance()) {
        _eeprom = getHydroponicsInstance()->getEEPROM();
    }
    HYDRUINO_HARD_ASSERT(_eeprom, SFP(HStr_Err_UnsupportedOperation));
}

int HydroponicsEEPROMStream::available()
{
    return _eeprom ? ((int)_endAddress - _readAddress) : 0;
}

int HydroponicsEEPROMStream::read()
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    return (int)_eeprom->readByte(_readAddress++);
}

size_t HydroponicsEEPROMStream::readBytes(char *buffer, size_t length)
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    size_t retVal = _eeprom->readBlock(_readAddress, (uint8_t *)buffer, length);
    _readAddress += retVal;
    return retVal;
}

int HydroponicsEEPROMStream::peek()
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    return (int)_eeprom->readByte(_readAddress);
}

void HydroponicsEEPROMStream::flush()
{
    //_eeprom->commit();
}

size_t HydroponicsEEPROMStream::write(const uint8_t *buffer, size_t size)
{
    if (!_eeprom || _writeAddress >= _endAddress) { return 0; }
    size_t remaining = _endAddress - _writeAddress;
    if (size > remaining) { size = remaining; }
    if (_eeprom->updateBlockVerify(_writeAddress, buffer, size)) {
        _writeAddress += size;
        return size;
    } else {
        HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
        return 0;
    }
}

size_t HydroponicsEEPROMStream::write(uint8_t data)
{
    if (!_eeprom || _writeAddress >= _endAddress) { return 0; }
    if (_eeprom->updateByteVerify(_writeAddress, data)) {
        _writeAddress += 1;
        return 1;
    } else {
        HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
        return 0;
    }
}

int HydroponicsEEPROMStream::availableForWrite()
{
    return _eeprom ? ((int)_endAddress - _writeAddress) : 0;
}


HydroponicsPROGMEMStream::HydroponicsPROGMEMStream()
    : Stream(), _readAddress(0), _writeAddress(0), _endAddress(UINTPTR_MAX)
{ ; }

HydroponicsPROGMEMStream::HydroponicsPROGMEMStream(uintptr_t dataAddress)
    : Stream(), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + strlen_P((const char *)dataAddress))
{ ; }

HydroponicsPROGMEMStream::HydroponicsPROGMEMStream(uintptr_t dataAddress, size_t dataSize)
    : Stream(), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + dataSize)
{ ; }

int HydroponicsPROGMEMStream::available()
{
    return _endAddress - _readAddress;
}

int HydroponicsPROGMEMStream::read()
{
    if (_readAddress >= _endAddress) { return -1; }
    #ifdef ESP8266
        return pgm_read_byte((const void *)(_readAddress++));
    #else
        return pgm_read_byte(_readAddress++);
    #endif
}

int HydroponicsPROGMEMStream::peek()
{
    if (_readAddress >= _endAddress) { return -1; }
    #ifdef ESP8266
        return pgm_read_byte((const void *)(_readAddress));
    #else
        return pgm_read_byte(_readAddress);
    #endif
}

void HydroponicsPROGMEMStream::flush()
{ ; }

size_t HydroponicsPROGMEMStream::write(const uint8_t *buffer, size_t size)
{
    HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
    return 0;
}

size_t HydroponicsPROGMEMStream::write(uint8_t data)
{
    HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
    return 0;
}


#ifdef HYDRUINO_USE_WIFI_STORAGE

HydroponicsWiFiStorageFileStream::HydroponicsWiFiStorageFileStream(WiFiStorageFile file, uintptr_t seekPos)
    : Stream(), _file(file), _bufferOffset(0), _bufferFileOffset(-1), _bufferDirection(None), _readOffset(0), _writeOffset(0), _endOffset(0)
{
    if (_file) {
        _endOffset = _file.size();
        _readOffset = _writeOffset = seekPos;
    }
}

HydroponicsWiFiStorageFileStream::~HydroponicsWiFiStorageFileStream()
{
    if (_file) {
        if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
            _file.seek(_bufferFileOffset);
            _file.write((const void*)_buffer, _bufferOffset); _bufferOffset = 0;
        }
        //_file.flush();
        _file.close();
    }
}

int HydroponicsWiFiStorageFileStream::available()
{
    return _file ? _endOffset - _readOffset : 0;
}

int HydroponicsWiFiStorageFileStream::read()
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    prepareReadBuffer();
    _readOffset++;
    return _buffer[_bufferOffset++];
}

size_t HydroponicsWiFiStorageFileStream::readBytes(char *buffer, size_t length)
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    while (length && _readOffset < _endOffset) {
        prepareReadBuffer();
        size_t howMany = min(length, _endOffset - _readOffset);
        howMany = min(howMany, HYDRUINO_WIFISTREAM_BUFFER_SIZE - _bufferOffset);
        memcpy(buffer, &_buffer[_bufferOffset], howMany);
        _readOffset += howMany;
        _bufferOffset += howMany;
        buffer += howMany;
        length -= howMany;
    }
}

int HydroponicsWiFiStorageFileStream::peek()
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    prepareReadBuffer();
    return _buffer[_bufferOffset];
}

void HydroponicsWiFiStorageFileStream::flush()
{
    if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
        _file.seek(_bufferFileOffset);
        _file.write((const void*)_buffer, _bufferOffset); _bufferOffset = 0;
    }
    //_file.flush();
}

size_t HydroponicsWiFiStorageFileStream::write(const uint8_t *buffer, size_t size)
{
    if (!_file || _writeOffset >= _endOffset) { return -1; }
    while (size) {
        prepareWriteBuffer();
        size_t howMany = min(size, HYDRUINO_WIFISTREAM_BUFFER_SIZE - _bufferOffset);
        memcpy(&_buffer[_bufferOffset], buffer, howMany);
        _writeOffset += howMany;
        _bufferOffset += howMany;
        buffer += howMany;
        size -= howMany;
    }
}

size_t HydroponicsWiFiStorageFileStream::write(uint8_t data)
{
    if (!_file || _writeOffset >= _endOffset) { return -1; }
    prepareWriteBuffer();
    _buffer[_bufferOffset++] = data;
    _writeOffset++;
    return 1;
}

int HydroponicsWiFiStorageFileStream::availableForWrite() 
{
    return _file ? _endOffset - _writeOffset : 0;
}

void HydroponicsWiFiStorageFileStream::prepareReadBuffer()
{
    if (_bufferDirection != ReadBuffer || _bufferFileOffset == -1 || _readOffset < _bufferFileOffset || _readOffset >= _bufferFileOffset + HYDRUINO_WIFISTREAM_BUFFER_SIZE) {
        if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
            _file.seek(_bufferFileOffset);
            _file.write((const void*)_buffer, _bufferOffset); //_bufferOffset = 0;
        }
        _bufferDirection = ReadBuffer;
        _bufferFileOffset = _readOffset;
        _bufferOffset = 0;

        _file.seek(_bufferFileOffset);
        _file.read((void *)_buffer, HYDRUINO_WIFISTREAM_BUFFER_SIZE);
    }
}

void HydroponicsWiFiStorageFileStream::prepareWriteBuffer()
{
    if (_bufferDirection != WriteBuffer || _bufferFileOffset == -1 || _writeOffset < _bufferFileOffset || _writeOffset >= _bufferFileOffset + HYDRUINO_WIFISTREAM_BUFFER_SIZE) {
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
