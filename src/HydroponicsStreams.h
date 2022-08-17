
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Streams
*/

#ifndef HydroponicsStreams_H
#define HydroponicsStreams_H

class HydroponicsEEPROMStream;
class HydroponicsPROGMEMStream;

#include "Hydroponics.h"

#ifdef ARDUINO_ARCH_SAM // Stream doesn't have availableForWrite
#define HYDRUINO_STREAM_AVAIL4WRT_OVERRIDE
#else
#define HYDRUINO_STREAM_AVAIL4WRT_OVERRIDE override
#endif

// EEPROM Stream
// Stream class for working with I2C_EEPROM data.
class HydroponicsEEPROMStream : public Stream {
public:
    HydroponicsEEPROMStream();
    HydroponicsEEPROMStream(uint16_t dataAddress, size_t dataSize);

    virtual int available() override;
    virtual int read() override;
    size_t readBytes(char *buffer, size_t length);
    virtual int peek() override;
    virtual void flush() override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual size_t write(uint8_t data) override;
    virtual int availableForWrite() HYDRUINO_STREAM_AVAIL4WRT_OVERRIDE;

protected:
    I2C_eeprom *_eeprom;
    uint16_t _readAddress, _writeAddress, _endAddress;
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

protected:
    uintptr_t _readAddress, _writeAddress, _endAddress;
};

#ifdef HYDRUINO_USE_WIFI_STORAGE

class HydroponicsWiFiStorageFileStream : public Stream {
public:
    HydroponicsWiFiStorageFileStream(WiFiStorageFile file, uintptr_t seekPos = 0);
    virtual ~HydroponicsWiFiStorageFileStream();

    virtual int available() override;
    virtual int read() override;
    size_t readBytes(char *buffer, size_t length);
    virtual int peek() override;
    virtual void flush() override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual size_t write(uint8_t data) override;
    virtual int availableForWrite() HYDRUINO_STREAM_AVAIL4WRT_OVERRIDE;

protected:
    enum WiFiStorageFileDirection : signed char { ReadBuffer, WriteBuffer, None = -1 };

    WiFiStorageFile _file;
    uint8_t _buffer[HYDRUINO_WIFISTREAM_BUFFER_SIZE];
    size_t _bufferOffset;
    uintptr_t _bufferFileOffset;
    WiFiStorageFileDirection _bufferDirection;
    uintptr_t _readOffset, _writeOffset, _endOffset;

    void prepareReadBuffer();
    void prepareWriteBuffer();
};

#endif

#endif // /ifndef HydroponicsStreams_H
