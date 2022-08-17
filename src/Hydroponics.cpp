/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics System
*/

#include "Hydroponics.h"

static RTC_DS3231 *_rtcSyncProvider = nullptr;
time_t rtcNow() {
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}

void handleInterrupt(pintype_t pin)
{
    if (Hydroponics::_activeInstance) {
        for (auto iter = Hydroponics::_activeInstance->_objects.begin(); iter != Hydroponics::_activeInstance->_objects.end(); ++iter) {
            if (iter->second->isSensorType()) {
                auto sensor = hy_static_ptr_cast<HydroponicsSensor>(iter->second);
                if (sensor->getInputPin() == pin && sensor->isBinaryClass()) {
                    auto binarySensor = hy_static_ptr_cast<HydroponicsBinarySensor>(sensor);
                    if (binarySensor) { binarySensor->notifyISRTriggered(); }
                }
            }
        }
    }
}


Hydroponics *Hydroponics::_activeInstance = nullptr;

Hydroponics::Hydroponics(pintype_t piezoBuzzerPin,
                         uint32_t eepromDeviceSize,
                         uint8_t eepromI2CAddress,
                         uint8_t rtcI2CAddress,
                         pintype_t sdCardCSPin,
                         uint32_t sdCardSpeed,
#ifdef HYDRUINO_ENABLE_SPIRAM_VIRTMEM
                         uint32_t spiRAMDeviceSize,
                         pintype_t spiRAMCSPin,
                         uint32_t spiRAMSpeed,
#endif
                         pintype_t *ctrlInputPinMap,
                         uint8_t lcdI2CAddress,
                         TwoWire &i2cWire,
                         uint32_t i2cSpeed)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed),
#ifndef HYDRUINO_ENABLE_SD_VIRTMEM
#ifndef CORE_TEENSY
      _sdCardSpeed(sdCardSpeed),
#else
      _sdCardSpeed(25000000U),
#endif
#endif
#if defined(HYDRUINO_ENABLE_SD_VIRTMEM)
      _vAlloc(VIRTMEM_DEFAULT_POOLSIZE, sdCardCSPin, sdCardSpeed, getSDCard(false)),
#elif defined(HYDRUINO_ENABLE_SPIRAM_VIRTMEM)
      _vAlloc(spiRAMDeviceSize, spiRAMCSPin, spiRAMSpeed),
#ifndef VIRTMEM_SPIRAM_CAPTURESPEED
      _spiRAMSpeed(spiRAMSpeed),
#endif
#endif
#ifdef HYDRUINO_USE_WIFI
      _wifiBegan(false),
#endif
      _piezoBuzzerPin(piezoBuzzerPin), _eepromDeviceSize(eepromDeviceSize),
#ifndef HYDRUINO_ENABLE_SD_VIRTMEM
      _sdCardCSPin(sdCardCSPin),
#endif
      _ctrlInputPinMap(ctrlInputPinMap),
      _eepromI2CAddr(eepromI2CAddress | HYDRUINO_SYS_I2CEEPROM_BASEADDR), _rtcI2CAddr(rtcI2CAddress), _lcdI2CAddr(lcdI2CAddress),
      _eeprom(nullptr), _rtc(nullptr), _sd(nullptr),
      _eepromBegan(false), _rtcBegan(false), _rtcBattFail(false), _sdBegan(false),
#ifndef HYDRUINO_DISABLE_MULTITASKING
      _controlTaskId(TASKMGR_INVALIDID), _dataTaskId(TASKMGR_INVALIDID), _miscTaskId(TASKMGR_INVALIDID),
#endif
      _systemData(nullptr), _suspend(true), _pollingFrame(0), _lastSpaceCheck(0), _lastAutosave(0),
      _sysConfigFilename(SFP(HStr_Default_ConfigFilename)), _sysDataAddress(-1)
{
    _activeInstance = this;
}

Hydroponics::~Hydroponics()
{
    suspend();
    while (_objects.size()) { _objects.erase(_objects.begin()); }
    while (_oneWires.size()) { dropOneWireForPin(_oneWires.begin()->first); }
    deallocateEEPROM();
    deallocateRTC();
    deallocateSD();
    _i2cWire = nullptr;
    if (this == _activeInstance) { _activeInstance = nullptr; }
    if (_systemData) { delete _systemData; _systemData = nullptr; }
}

void Hydroponics::allocateEEPROM()
{
    if (!_eeprom && _eepromDeviceSize) {
        _eeprom = new I2C_eeprom(_eepromI2CAddr, _eepromDeviceSize, _i2cWire);
        HYDRUINO_SOFT_ASSERT(_eeprom, SFP(HStr_Err_AllocationFailure));
        _eepromBegan = false;
    }
}

void Hydroponics::deallocateEEPROM()
{
    if (_eeprom) { delete _eeprom; _eeprom = nullptr; }
}

void Hydroponics::allocateRTC()
{
    if (!_rtc) {
        _rtc = new RTC_DS3231();
        HYDRUINO_SOFT_ASSERT(_rtc, SFP(HStr_Err_AllocationFailure));
        HYDRUINO_HARD_ASSERT(_rtcI2CAddr == B000, F("RTClib does not support i2c multi-addressing, only i2c address B000 may be used"));
        _rtcBegan = false;
    }
}

void Hydroponics::deallocateRTC()
{
    if (_rtc) {
        if (_rtcSyncProvider == _rtc) { setSyncProvider(nullptr); _rtcSyncProvider = nullptr; }
        delete _rtc; _rtc = nullptr;
    }
}

void Hydroponics::allocateSD()
{
    if (!_sd) {
        #if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)) || defined(SD)
            _sd = &SD;
        #else
            _sd = new SDClass();
            HYDRUINO_SOFT_ASSERT(_sd, SFP(HStr_Err_AllocationFailure));
        #endif
        _sdBegan = false;
    }
}

void Hydroponics::deallocateSD()
{
    if (_sd) {
        #if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)) || defined(SD)
            _sd = nullptr;
        #else
            delete _sd; _sd = nullptr;
        #endif
    }
}

void Hydroponics::init(Hydroponics_SystemMode systemMode,
                       Hydroponics_MeasurementMode measureMode,
                       Hydroponics_DisplayOutputMode dispOutMode,
                       Hydroponics_ControlInputMode ctrlInMode)
{
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();

        HYDRUINO_SOFT_ASSERT((int)systemMode >= 0 && systemMode < Hydroponics_SystemMode_Count, SFP(HStr_Err_InvalidParameter));
        HYDRUINO_SOFT_ASSERT((int)measureMode >= 0 && measureMode < Hydroponics_MeasurementMode_Count, SFP(HStr_Err_InvalidParameter));
        #ifndef HYDRUINO_DISABLE_GUI
            HYDRUINO_SOFT_ASSERT((int)dispOutMode >= 0 && dispOutMode < Hydroponics_DisplayOutputMode_Count, SFP(HStr_Err_InvalidParameter));
            HYDRUINO_SOFT_ASSERT((int)ctrlInMode >= 0 && ctrlInMode < Hydroponics_ControlInputMode_Count, SFP(HStr_Err_InvalidParameter));
        #endif

        _systemData = new HydroponicsSystemData();
        HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_AllocationFailure));

        if (_systemData) {
            _systemData->systemMode = systemMode;
            _systemData->measureMode = measureMode;
            #ifndef HYDRUINO_DISABLE_GUI
                _systemData->dispOutMode = dispOutMode;
                _systemData->ctrlInMode = ctrlInMode;
            #else
                _systemData->dispOutMode = Hydroponics_DisplayOutputMode_Disabled;
                _systemData->ctrlInMode = Hydroponics_ControlInputMode_Disabled;
            #endif

            commonPostInit();
        }
    }  
}

bool Hydroponics::initFromEEPROM(bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();

        if (getEEPROM() && _eepromBegan && _sysDataAddress != -1) {
            HydroponicsEEPROMStream eepromStream(_sysDataAddress, _eepromDeviceSize - _sysDataAddress);
            return jsonFormat ? initFromJSONStream(&eepromStream) : initFromBinaryStream(&eepromStream);
        }
    }

    return false;
}

bool Hydroponics::saveToEEPROM(bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));

    if (_systemData) {
        if (getEEPROM() && _eepromBegan && _sysDataAddress != -1) {
            HydroponicsEEPROMStream eepromStream(_sysDataAddress, _eepromDeviceSize - _sysDataAddress);
            return jsonFormat ? saveToJSONStream(&eepromStream) : saveToBinaryStream(&eepromStream);
        }
    }

    return false;
}

bool Hydroponics::initFromSDCard(bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();
        auto sd = getSDCard();

        if (sd) {
            bool retVal = false;
            auto configFile = sd->open(_sysConfigFilename.c_str(), FILE_READ);

            if (configFile) {
                retVal = jsonFormat ? initFromJSONStream(&configFile) : initFromBinaryStream(&configFile);

                configFile.close();
            }

            endSDCard(sd);
            return retVal;
        }
    }

    return false;
}

bool Hydroponics::saveToSDCard(bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));

    if (!_systemData) {
        auto sd = getSDCard();

        if (sd) {
            bool retVal = false;
            auto configFile = sd->open(_sysConfigFilename.c_str(), FILE_READ);

            if (configFile) {
                retVal = jsonFormat ? saveToJSONStream(&configFile, false) : saveToBinaryStream(&configFile);

                configFile.flush();
                configFile.close();
            }

            endSDCard(sd);
            return retVal;
        }
    }

    return false;
}

#ifdef HYDRUINO_USE_WIFI_STORAGE

bool Hydroponics::initFromWiFiStorage(bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();

        auto configFile = WiFiStorage.open(_sysConfigFilename.c_str());

        if (configFile) {
            auto configFileStream = HydroponicsWiFiStorageFileStream(configFile);
            return jsonFormat ? initFromJSONStream(&configFileStream) : initFromBinaryStream(&configFileStream);

            configFile.close();
        }
    }

    return false;
}

bool Hydroponics::saveToWiFiStorage(bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));

    if (_systemData) {
        if (WiFiStorage.exists(_sysConfigFilename.c_str())) {
            WiFiStorage.remove(_sysConfigFilename.c_str());
        }
        auto configFile = WiFiStorage.open(_sysConfigFilename.c_str());

        if (configFile) {
            auto configFileStream = HydroponicsWiFiStorageFileStream(configFile);
            return jsonFormat ? saveToJSONStream(&configFileStream, false) : saveToBinaryStream(&configFileStream);

            configFileStream.flush();
            configFile.close();
        }
    }

    return false;
}

#endif

bool Hydroponics::initFromJSONStream(Stream *streamIn)
{
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));
    HYDRUINO_SOFT_ASSERT(streamIn && streamIn->available(), SFP(HStr_Err_InvalidParameter));

    if (!_systemData && streamIn && streamIn->available()) {
        commonPreInit();

        {   StaticJsonDocument<HYDRUINO_JSON_DOC_SYSSIZE> doc;
            deserializeJson(doc, *streamIn);
            JsonObjectConst systemDataObj = doc.as<JsonObjectConst>();
            HydroponicsSystemData *systemData = (HydroponicsSystemData *)newDataFromJSONObject(systemDataObj);

            HYDRUINO_SOFT_ASSERT(systemData && systemData->isSystemData(), SFP(HStr_Err_ImportFailure));
            if (systemData && systemData->isSystemData()) {
                _systemData = systemData;
            } else if (systemData) {
                delete systemData;
            }
        }

        if (_systemData) {
            while (streamIn->available()) {
                StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;
                deserializeJson(doc, *streamIn);
                JsonObjectConst dataObj = doc.as<JsonObjectConst>();
                HydroponicsData *data = newDataFromJSONObject(dataObj);

                HYDRUINO_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectData()), SFP(HStr_Err_ImportFailure));
                if (data && data->isStandardData()) {
                    if (data->isCalibrationData()) {
                        hydroCalibrations.setUserCalibrationData((HydroponicsCalibrationData *)data);
                    } else if (data->isCropsLibData()) {
                        hydroCropsLib.setUserCropData((HydroponicsCropsLibData *)data);
                    } else if (data->isAdditiveData()) {
                        hydroAdditives.setCustomAdditiveData((HydroponicsCustomAdditiveData *)data);
                    }
                    delete data; data = nullptr;
                } else if (data && data->isObjectData()) {
                    HydroponicsObject *obj = newObjectFromData(data);
                    delete data; data = nullptr;

                    if (obj && !obj->isUnknownType()) {
                        _objects[obj->getKey()] = SharedPtr<HydroponicsObject>(obj);
                    } else {
                        HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_ImportFailure));
                        if (obj) { delete obj; }
                        delete _systemData; _systemData = nullptr;
                        break;
                    }
                } else {
                    if (data) { delete data; data = nullptr; }
                    delete _systemData; _systemData = nullptr;
                    break;
                }
            }
        }

        HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_InitializationFailure));
        if (_systemData) { commonPostInit(); }
        return _systemData;
    }

    return false;
}

bool Hydroponics::saveToJSONStream(Stream *streamOut, bool compact)
{
    HYDRUINO_HARD_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(streamOut, SFP(HStr_Err_InvalidParameter));

    if (_systemData && streamOut) {
        {   StaticJsonDocument<HYDRUINO_JSON_DOC_SYSSIZE> doc;

            JsonObject systemDataObj = doc.to<JsonObject>();
            _systemData->toJSONObject(systemDataObj);

            if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
                return false;
            }
        }

        if (hydroCalibrations.hasUserCalibrations()) {
            for (auto iter = hydroCalibrations._calibrationData.begin(); iter != hydroCalibrations._calibrationData.end(); ++iter) {
                StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;

                JsonObject calibDataObj = doc.to<JsonObject>();
                iter->second->toJSONObject(calibDataObj);

                if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                    HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
                    return false;
                }
            }
        }

        if (hydroCropsLib.hasUserCrops()) {
            for (auto iter = hydroCropsLib._cropsData.begin(); iter != hydroCropsLib._cropsData.end(); ++iter) {
                if (iter->second->userSet) {
                    StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;

                    JsonObject cropDataObj = doc.to<JsonObject>();
                    iter->second->data.toJSONObject(cropDataObj);

                    if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                        HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
                        return false;
                    }
                }
            }
        }

        if (hydroAdditives.hasCustomAdditives()) {
            for (auto iter = hydroAdditives._additives.begin(); iter != hydroAdditives._additives.end(); ++iter) {
                StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;

                JsonObject additiveDataObj = doc.to<JsonObject>();
                iter->second->toJSONObject(additiveDataObj);

                if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                    HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
                    return false;
                }
            }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                HydroponicsData *data = iter->second->newSaveData();

                HYDRUINO_SOFT_ASSERT(data && data->isObjectData(), SFP(HStr_Err_AllocationFailure));
                if (data && data->isObjectData()) {
                    StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;

                    JsonObject objectDataObj = doc.to<JsonObject>();
                    data->toJSONObject(objectDataObj);
                    delete data; data = nullptr;

                    if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                        HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
                        return false;
                    }
                } else {
                    if (data) { delete data; data = nullptr; }
                    return false;
                }
            }
        }

        commonPostSave();
        return true;
    }

    return false;
}

bool Hydroponics::initFromBinaryStream(Stream *streamIn)
{
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));
    HYDRUINO_SOFT_ASSERT(streamIn && streamIn->available(), SFP(HStr_Err_InvalidParameter));

    if (!_systemData && streamIn && streamIn->available()) {
        commonPreInit();

        {   HydroponicsSystemData *systemData = (HydroponicsSystemData *)newDataFromBinaryStream(streamIn);

            HYDRUINO_SOFT_ASSERT(systemData && systemData->isSystemData(), SFP(HStr_Err_ImportFailure));
            if (systemData && systemData->isSystemData()) {
                _systemData = systemData;
            } else if (systemData) {
                delete systemData;
            }
        }

        if (_systemData) {
            while (streamIn->available()) {
                HydroponicsData *data = newDataFromBinaryStream(streamIn);

                HYDRUINO_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectData()), SFP(HStr_Err_AllocationFailure));
                if (data && data->isStandardData()) {
                    if (data->isCalibrationData()) {
                        hydroCalibrations.setUserCalibrationData((HydroponicsCalibrationData *)data);
                    } else if (data->isCropsLibData()) {
                        hydroCropsLib.setUserCropData((HydroponicsCropsLibData *)data);
                    } else if (data->isAdditiveData()) {
                        hydroAdditives.setCustomAdditiveData((HydroponicsCustomAdditiveData *)data);
                    }
                    delete data; data = nullptr;
                } else if (data && data->isObjectData()) {
                    HydroponicsObject *obj = newObjectFromData(data);
                    delete data; data = nullptr;

                    if (obj && !obj->isUnknownType()) {
                        _objects[obj->getKey()] = SharedPtr<HydroponicsObject>(obj);
                    } else {
                        HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_ImportFailure));
                        if (obj) { delete obj; }
                        delete _systemData; _systemData = nullptr;
                        break;
                    }
                } else {
                    if (data) { delete data; data = nullptr; }    
                    delete _systemData; _systemData = nullptr;
                    break;
                }
            }
        }

        HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_InitializationFailure));
        if (_systemData) { commonPostInit(); }
        return _systemData;
    }

    return false;
}

bool Hydroponics::saveToBinaryStream(Stream *streamOut)
{
    HYDRUINO_HARD_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(streamOut, SFP(HStr_Err_InvalidParameter));

    if (_systemData && streamOut) {
        {   size_t bytesWritten = serializeDataToBinaryStream(_systemData, streamOut);

            HYDRUINO_SOFT_ASSERT(!bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (hydroCalibrations.hasUserCalibrations()) {
            size_t bytesWritten = 0;

            for (auto iter = hydroCalibrations._calibrationData.begin(); iter != hydroCalibrations._calibrationData.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (hydroCropsLib.hasUserCrops()) {
            size_t bytesWritten = 0;

            for (auto iter = hydroCropsLib._cropsData.begin(); iter != hydroCropsLib._cropsData.end(); ++iter) {
                if (iter->first >= Hydroponics_CropType_CustomCrop1) {
                    bytesWritten += serializeDataToBinaryStream(&(iter->second->data), streamOut);
                }
            }

            HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (hydroAdditives.hasCustomAdditives()) {
            size_t bytesWritten = 0;

            for (auto iter = hydroAdditives._additives.begin(); iter != hydroAdditives._additives.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                HydroponicsData *data = iter->second->newSaveData();

                HYDRUINO_SOFT_ASSERT(data && data->isObjectData(), SFP(HStr_Err_AllocationFailure));
                if (data && data->isObjectData()) {
                    size_t bytesWritten = serializeDataToBinaryStream(data, streamOut);
                    delete data; data = nullptr;

                    HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
                    if (!bytesWritten) { return false; }
                } else {
                    if (data) { delete data; data = nullptr; }
                    return false;
                }
            }
        }

        commonPostSave();
        return true;
    }

    return false;
}

void Hydroponics::commonPreInit()
{
    if (isValidPin(_piezoBuzzerPin)) {
        pinMode(_piezoBuzzerPin, OUTPUT);
        #ifdef ESP32
            ledcSetup(0, 0, 10);
            ledcAttachPin(_piezoBuzzerPin, 0);
        #elif !defined(ARDUINO_SAM_DUE)
            noTone(_piezoBuzzerPin);
        #else
            digitalWrite(_piezoBuzzerPin, 0);
        #endif
    }
    if (_i2cWire) { _i2cWire->setClock(_i2cSpeed); }
    #ifdef HYDRUINO_USE_VIRTMEM
        _vAlloc.start();
    #endif
    if (isValidPin(getSDCardCSPin())) {
        getSPI()->begin();
        // some archs won't init pinMode/set CS high, so we do it manually to be on the safe side
        pinMode(getSDCardCSPin(), OUTPUT);
        digitalWrite(getSDCardCSPin(), HIGH);
    }
    #ifdef HYDRUINO_USE_WIFI_STORAGE
        //WiFiStorage.begin();
    #endif
    #ifndef HYDRUINO_DISABLE_MULTITASKING
        taskManager.setInterruptCallback(&handleInterrupt);
    #endif
}

void Hydroponics::commonPostInit()
{
    if ((_rtcSyncProvider = getRealTimeClock())) {
        setSyncProvider(rtcNow);
    }

    scheduler.updateDayTracking(); // also calls setNeedsScheduling
    logger.updateInitTracking();
    publisher.setNeedsTabulation();

    #ifdef HYDRUINO_USE_WIFI
        if (!_systemData->wifiPasswordSeed && _systemData->wifiPassword[0]) {
            setWiFiConnection(getWiFiSSID(), getWiFiPassword()); // sets seed and encrypts
        }
    #endif

    #ifndef HYDRUINO_DISABLE_GUI
        // TODO: tcMenu setup
    #endif

    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
        #if 1 // set to 0 if you just want this gone
            Serial.print(F("Hydroponics::commonPostInit piezoBuzzerPin: "));
            if (isValidPin(_piezoBuzzerPin)) { Serial.print(_piezoBuzzerPin); }
            else { Serial.print(SFP(HStr_Disabled)); }
            Serial.print(F(", eepromDeviceSize: "));
            if (getEEPROMDeviceSize()) { Serial.print(getEEPROMDeviceSize()); }
            else { Serial.print(SFP(HStr_Disabled)); }
            Serial.print(F(", eepromI2CAddress: 0x"));
            Serial.print(_eepromI2CAddr & ~HYDRUINO_SYS_I2CEEPROM_BASEADDR, HEX);
            Serial.print(F(", rtcI2CAddress: 0x"));
            Serial.print(_rtcI2CAddr, HEX);
            Serial.print(F(", sdCardCSPin: "));
            if (isValidPin(getSDCardCSPin())) { Serial.print(getSDCardCSPin()); }
            else { Serial.print(SFP(HStr_Disabled)); }
            Serial.print(F(", sdCardSpeed: "));
            Serial.print(roundf(getSDCardSpeed() / 1000000.0f)); Serial.print(F("MHz"));
#ifdef HYDRUINO_ENABLE_SPIRAM_VIRTMEM
            Serial.print(F(", spiRAMDeviceSize: "));
            if (getSPIRAMDeviceSize()) { Serial.print(getSPIRAMDeviceSize()); }
            else { Serial.print(SFP(HStr_Disabled)); }
            Serial.print(F(", spiRAMCSPin: "));
            if (isValidPin(getSPIRAMCSPin())) { Serial.print(getSPIRAMCSPin()); }
            else { Serial.print(SFP(HStr_Disabled)); }
            Serial.print(F(", spiRAMSpeed: "));
            Serial.print(roundf(getSPIRAMSpeed() / 1000000.0f)); Serial.print(F("MHz"));
#endif
            Serial.print(F(", controlInputPinMap: "));
            if (getControlInputRibbonPinCount() && _ctrlInputPinMap && isValidPin(_ctrlInputPinMap[0])) {
                Serial.print('{');
                for (int i = 0; i < getControlInputRibbonPinCount(); ++i) {
                    if (i) { Serial.print(','); }
                    Serial.print(_ctrlInputPinMap[i]);
                }
                Serial.print('}');
            }
            else { Serial.print(SFP(HStr_Disabled)); }
            Serial.print(F(", lcdI2CAddress: 0x"));
            Serial.print(_lcdI2CAddr, HEX);
            Serial.print(F(", i2cSpeed: "));
            Serial.print(roundf(getI2CSpeed() / 1000.0f)); Serial.print(F("kHz"));
            Serial.print(F(", systemMode: "));
            Serial.print(systemModeToString(getSystemMode()));
            Serial.print(F(", measureMode: "));
            Serial.print(measurementModeToString(getMeasurementMode()));
            Serial.print(F(", dispOutMode: "));
            Serial.print(displayOutputModeToString(getDisplayOutputMode()));
            Serial.print(F(", ctrlInMode: "));
            Serial.print(controlInputModeToString(getControlInputMode()));
            Serial.println(); flushYield();
        #endif
    #endif // /ifdef HYDRUINO_USE_VERBOSE_OUTPUT
}

void Hydroponics::commonPostSave()
{
    logger.logSystemSave();

    if (hydroCalibrations.hasUserCalibrations()) {
        for (auto iter = hydroCalibrations._calibrationData.begin(); iter != hydroCalibrations._calibrationData.end(); ++iter) {
            iter->second->_unsetModded();
        }
    }

    if (hydroCropsLib.hasUserCrops()) {
        for (auto iter = hydroCropsLib._cropsData.begin(); iter != hydroCropsLib._cropsData.end(); ++iter) {
            if (iter->second->userSet) {
                iter->second->data._unsetModded();
            }
        }
    }

    if (hydroAdditives.hasCustomAdditives()) {
        for (auto iter = hydroAdditives._additives.begin(); iter != hydroAdditives._additives.end(); ++iter) {
            iter->second->_unsetModded();
        }
    }
}

void controlLoop()
{
    if (Hydroponics::_activeInstance && !Hydroponics::_activeInstance->_suspend) {
        #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
            Serial.println(F("controlLoop")); flushYield();
        #endif

        for (auto iter = Hydroponics::_activeInstance->_objects.begin(); iter != Hydroponics::_activeInstance->_objects.end(); ++iter) {
            iter->second->update();
        }

        Hydroponics::_activeInstance->scheduler.update();

        #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
            Serial.println(F("~controlLoop")); flushYield();
        #endif
    }

    yield();
}

void dataLoop()
{
    if (Hydroponics::_activeInstance && !Hydroponics::_activeInstance->_suspend) {
        #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
            Serial.println(F("dataLoop")); flushYield();
        #endif

        Hydroponics::_activeInstance->publisher.advancePollingFrame();

        for (auto iter = Hydroponics::_activeInstance->_objects.begin(); iter != Hydroponics::_activeInstance->_objects.end(); ++iter) {
            if (iter->second->isSensorType()) {
                auto sensor = hy_static_ptr_cast<HydroponicsSensor>(iter->second);
                if (sensor->needsPolling()) {
                    sensor->takeMeasurement(); // no force if already current for this frame #, we're just ensuring data for publisher
                }
            }
        }

        #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
            Serial.println(F("~dataLoop")); flushYield();
        #endif
    }

    yield();
}

void miscLoop()
{
    if (Hydroponics::_activeInstance && !Hydroponics::_activeInstance->_suspend) {
        #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
            Serial.println(F("miscLoop")); flushYield();
        #endif

        Hydroponics::_activeInstance->checkFreeMemory();
        Hydroponics::_activeInstance->checkFreeSpace();
        Hydroponics::_activeInstance->checkAutosave();

        Hydroponics::_activeInstance->publisher.update();

        #if HYDRUINO_SYS_MEM_LOGGING_ENABLE
        {   static time_t _lastMemLog = unixNow();
            if (unixNow() >= _lastMemLog + 15) {
                _lastMemLog = unixNow();
                Hydroponics::_activeInstance->logger.logMessage(String(F("Free memory: ")), String(freeMemory()));
            }
        }
        #endif

        #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
            Serial.println(F("~miscLoop")); flushYield();
        #endif
    }

    yield();
}

void Hydroponics::launch()
{
    // Forces all sensors to get a new measurement
    publisher.advancePollingFrame();

    // Create/enable main runloops
    _suspend = false;
    #ifndef HYDRUINO_DISABLE_MULTITASKING
        if (!isValidTask(_controlTaskId)) {
            _controlTaskId = taskManager.scheduleFixedRate(HYDRUINO_CONTROL_LOOP_INTERVAL, controlLoop);
        } else {
            taskManager.setTaskEnabled(_controlTaskId, true);
        }
        if (!isValidTask(_dataTaskId)) {
            _dataTaskId = taskManager.scheduleFixedRate(getPollingInterval(), dataLoop);
        } else {
            taskManager.setTaskEnabled(_dataTaskId, true);
        }
        if (!isValidTask(_miscTaskId)) {
            _miscTaskId = taskManager.scheduleFixedRate(HYDRUINO_MISC_LOOP_INTERVAL, miscLoop);
        } else {
            taskManager.setTaskEnabled(_miscTaskId, true);
        }
    #endif

    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
        Serial.println(F("Hydroponics::launch System launched!")); flushYield();
    #endif
}

void Hydroponics::suspend()
{
    _suspend = true;
    #ifndef HYDRUINO_DISABLE_MULTITASKING
        if (isValidTask(_controlTaskId)) {
            taskManager.setTaskEnabled(_controlTaskId, false);
        }
        if (isValidTask(_dataTaskId)) {
            taskManager.setTaskEnabled(_dataTaskId, false);
        }
        if (isValidTask(_miscTaskId)) {
            taskManager.setTaskEnabled(_miscTaskId, false);
        }
    #endif

    #ifdef HYDRUINO_USE_VERBOSE_OUTPUT
        Serial.println(F("Hydroponics::suspend System suspended!")); flushYield();
    #endif
}

void Hydroponics::update()
{
    #ifndef HYDRUINO_DISABLE_MULTITASKING
        taskManager.runLoop(); // tcMenu also uses this system to run its UI
    #else
        controlLoop();
        dataLoop();
        miscLoop();
    #endif
}

bool Hydroponics::registerObject(SharedPtr<HydroponicsObject> obj)
{
    HYDRUINO_SOFT_ASSERT(obj->getId().posIndex >= 0 && obj->getId().posIndex < HYDRUINO_POS_MAXSIZE, SFP(HStr_Err_InvalidParameter));
    if (obj && _objects.find(obj->getKey()) == _objects.end()) {
        _objects[obj->getKey()] = obj;

        if (obj->isActuatorType() || obj->isCropType() || obj->isReservoirType()) {
            scheduler.setNeedsScheduling();
        }
        if (obj->isSensorType()) {
            publisher.setNeedsTabulation();
        }

        return true;
    }
    return false;
}

bool Hydroponics::unregisterObject(SharedPtr<HydroponicsObject> obj)
{
    auto iter = _objects.find(obj->getKey());
    if (iter != _objects.end()) {
        _objects.erase(iter);
        scheduler.setNeedsScheduling();
        return true;
    }
    return false;
}

SharedPtr<HydroponicsObject> Hydroponics::objectById(HydroponicsIdentity id) const
{
    if (id.posIndex == HYDRUINO_POS_SEARCH_FROMBEG) {
        while (++id.posIndex < HYDRUINO_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (iter != _objects.end()) {
                if (id.keyString == iter->second->getKeyString()) {
                    return iter->second;
                } else {
                    objectById_Col(id);
                }
            }
        }
    } else if (id.posIndex == HYDRUINO_POS_SEARCH_FROMEND) {
        while (--id.posIndex >= 0) {
            auto iter = _objects.find(id.regenKey());
            if (iter != _objects.end()) {
                if (id.keyString == iter->second->getKeyString()) {
                    return iter->second;
                } else {
                    objectById_Col(id);
                }
            }
        }
    } else {
        auto iter = _objects.find(id.key);
        if (iter != _objects.end()) {
            if (id.keyString == iter->second->getKeyString()) {
                return iter->second;
            } else {
                objectById_Col(id);
            }
        }
    }

    return nullptr;
}

SharedPtr<HydroponicsObject> Hydroponics::objectById_Col(const HydroponicsIdentity &id) const
{
    HYDRUINO_SOFT_ASSERT(false, F("Hashing collision")); // exhaustive search must be performed, publishing may miss values

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (id.keyString == iter->second->getKeyString()) {
            return iter->second;
        }
    }

    return nullptr;
}

Hydroponics_PositionIndex Hydroponics::firstPosition(HydroponicsIdentity id, bool taken)
{
    if (id.posIndex != HYDRUINO_POS_SEARCH_FROMEND) {
        id.posIndex = HYDRUINO_POS_SEARCH_FROMBEG;
        while (++id.posIndex < HYDRUINO_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    } else {
        id.posIndex = HYDRUINO_POS_SEARCH_FROMEND;
        while (--id.posIndex >= 0) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    }

    return -1;
}

bool Hydroponics::tryGetPinLock(pintype_t pin, time_t waitMillis)
{
    time_t startMillis = millis();
    while (1) {
        auto iter = _pinLocks.find(pin);
        if (iter == _pinLocks.end()) {
            _pinLocks[pin] = true;
            return (_pinLocks.find(pin) != _pinLocks.end());
        }
        else if (millis() - startMillis >= waitMillis) { return false; }
        else { yield(); }
    }
}

void Hydroponics::setSystemName(String systemName)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && !systemName.equals(getSystemName())) {
        _systemData->_bumpRevIfNotAlreadyModded();
        strncpy(_systemData->systemName, systemName.c_str(), HYDRUINO_NAME_MAXSIZE);
        // TODO: notify UI to update
    }
}

void Hydroponics::setTimeZoneOffset(int8_t timeZoneOffset)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && _systemData->timeZoneOffset != timeZoneOffset) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->timeZoneOffset = timeZoneOffset;
        // TODO: notify UI to update
        scheduler.setNeedsScheduling();
    }
}

void Hydroponics::setPollingInterval(uint16_t pollingInterval)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && _systemData->pollingInterval != pollingInterval) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->pollingInterval = pollingInterval;

        #ifndef HYDRUINO_DISABLE_MULTITASKING
            if (isValidTask(_dataTaskId)) {
                auto dataTask = taskManager.getTask(_dataTaskId);
                if (dataTask) {
                    bool enabled = dataTask->isEnabled();
                    auto next = dataTask->getNext();
                    dataTask->handleScheduling(getPollingInterval(), TIME_MILLIS, true);
                    dataTask->setNext(next);
                    dataTask->setEnabled(enabled);
                }
            }
        #endif
    }
}

void Hydroponics::setAutosaveEnabled(Hydroponics_Autosave autosaveEnabled, Hydroponics_Autosave autosaveFallback, uint16_t autosaveInterval)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && (_systemData->autosaveEnabled != autosaveEnabled || _systemData->autosaveFallback != autosaveFallback || _systemData->autosaveInterval != autosaveInterval)) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->autosaveEnabled = autosaveEnabled;
        _systemData->autosaveFallback = autosaveFallback;
        _systemData->autosaveInterval = autosaveInterval;
    }
}

#ifdef HYDRUINO_USE_WIFI

void Hydroponics::setWiFiConnection(String ssid, String pass)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData) {
        bool ssidChanged = ssid.equals(getWiFiSSID());
        bool passChanged = pass.equals(getWiFiPassword());

        if (ssidChanged || passChanged || (pass.length() && !_systemData->wifiPasswordSeed)) {
            _systemData->_bumpRevIfNotAlreadyModded();

            if (ssid.length()) {
                strncpy(_systemData->wifiSSID, ssid.c_str(), HYDRUINO_NAME_MAXSIZE);
            } else {
                memset(_systemData->wifiSSID, '\0', HYDRUINO_NAME_MAXSIZE);
            }

            if (pass.length()) {
                randomSeed(unixNow());
                _systemData->wifiPasswordSeed = random(1, RANDOM_MAX);

                randomSeed(_systemData->wifiPasswordSeed);
                for (int charIndex = 0; charIndex < HYDRUINO_NAME_MAXSIZE; ++charIndex) {
                    _systemData->wifiPassword[charIndex] = (uint8_t)(charIndex < pass.length() ? pass[charIndex] : '\0') ^ (uint8_t)random(256);
                }
            } else {
                _systemData->wifiPasswordSeed = 0;
                memset(_systemData->wifiPassword, '\0', HYDRUINO_NAME_MAXSIZE);
            }

            if (_wifiBegan && (ssidChanged || passChanged)) { WiFi.disconnect(); _wifiBegan = false; } // forces re-connect on next getWiFi
        }
    }
}

#endif

void Hydroponics::setRealTimeClockTime(DateTime time)
{
    auto rtc = getRealTimeClock();
    if (rtc) {
        rtc->adjust(DateTime((uint32_t)(time.unixtime() + (-getTimeZoneOffset() * SECS_PER_HOUR))));
        notifyRTCTimeUpdated();
    }
}

int Hydroponics::getControlInputRibbonPinCount() const
{
    switch (getControlInputMode()) {
        case Hydroponics_ControlInputMode_2x2Matrix:
        case Hydroponics_ControlInputMode_4xButton:
            return 4;
        case Hydroponics_ControlInputMode_6xButton:
            return 6;
        case Hydroponics_ControlInputMode_RotaryEncoder:
            return 5;
        default:
            return 0;
    }
}

pintype_t Hydroponics::getControlInputPin(int ribbonPinIndex) const
{
    int ctrlInPinCount = getControlInputRibbonPinCount();
    HYDRUINO_SOFT_ASSERT(ctrlInPinCount > 0, SFP(HStr_Err_UnsupportedOperation));
    HYDRUINO_SOFT_ASSERT(ctrlInPinCount <= 0 || (ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount), SFP(HStr_Err_InvalidParameter));

    return ctrlInPinCount && ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount ? _ctrlInputPinMap[ribbonPinIndex] : -1;
}

I2C_eeprom *Hydroponics::getEEPROM(bool begin)
{
    if (!_eeprom) { allocateEEPROM(); }

    if (_eeprom && begin && !_eepromBegan) {
        _eepromBegan = _eeprom->begin();

        if (!_eepromBegan) { deallocateEEPROM(); }
    }

    return _eeprom && (!begin || _eepromBegan) ? _eeprom : nullptr;
}

RTC_DS3231 *Hydroponics::getRealTimeClock(bool begin)
{
    if (!_rtc) { allocateRTC(); }

    if (_rtc && begin && !_rtcBegan) {
        _rtcBegan = _rtc->begin(_i2cWire);

        if (_rtcBegan) {
            bool rtcBattFailBefore = _rtcBattFail;
            _rtcBattFail = _rtc->lostPower();
            if (_rtcBattFail && !rtcBattFailBefore) {
                logger.logWarning(SFP(HStr_Log_RTCBatteryFailure));
            }
        } else {
            deallocateRTC();
        }
    }

    return _rtc && (!begin || _rtcBegan) ? _rtc : nullptr;
}

SDClass *Hydroponics::getSDCard(bool begin)
{
    if (!_sd) { allocateSD(); }

    if (_sd && begin) {
        if (!_sdBegan) {
            #ifdef HYDRUINO_ENABLE_SD_VIRTMEM
                if (!_systemData && !_vAlloc.getSDBegan()) { commonPreInit(); }
                _sdBegan = _vAlloc.getSDBegan();
            #elif defined(ESP32)
                _sdBegan = _sd->begin(getSDCardCSPin(), *getSPI(), getSDCardSpeed());
            #elif defined(CORE_TEENSY)
                _sdBegan = _sd->begin(getSDCardCSPin()); // card speed not possible to set on teensy
            #else
                _sdBegan = _sd->begin(getSDCardSpeed(), getSDCardCSPin());
            #endif
        }

        if (!_sdBegan && _sdOut == 0) { deallocateSD(); }

        if (_sd && _sdBegan) {
            _sdOut++;
            return _sd;
        }
        return nullptr;
    }

    return _sd;
}

void Hydroponics::endSDCard(SDClass *sd)
{
    #if defined(CORE_TEENSY)
        --_sdOut; // no delayed write on teensy's SD impl
    #else
        if (--_sdOut == 0 && _sd) {
            _sd->end();
        }
    #endif
}

#ifdef HYDRUINO_USE_WIFI

WiFiClass *Hydroponics::getWiFi(String ssid, String pass, bool begin)
{
    int status = WiFi.status();

    if (begin && (!_wifiBegan || status != WL_CONNECTED)) {
        if (status == WL_CONNECTED) {
            _wifiBegan = true;
        } else if (status == WL_NO_SHIELD) {
            _wifiBegan = false;
        } else { // attempt connection
            #ifdef HYDRUINO_USE_SERIALWIFI
                status = WiFi.begin(ssid.c_str(), pass.c_str());
            #else
                status = pass.length() ? WiFi.begin(const_cast<char *>(ssid.c_str()), pass.c_str())
                                       : WiFi.begin(const_cast<char *>(ssid.c_str()));
            #endif

            _wifiBegan = (status == WL_CONNECTED);
        }

        return _wifiBegan ? &WiFi : nullptr;
    }

    return &WiFi;
}

#endif

OneWire *Hydroponics::getOneWireForPin(pintype_t pin)
{
    auto wireIter = _oneWires.find(pin);
    if (wireIter != _oneWires.end()) {
        return wireIter->second;
    } else {
        OneWire *oneWire = new OneWire(pin);
        if (oneWire) {
            _oneWires[pin] = oneWire;
            if (_oneWires.find(pin) != _oneWires.end()) { return oneWire; }
            else if (oneWire) { delete oneWire; }
        } else if (oneWire) { delete oneWire; }
    }
    return nullptr;
}

void Hydroponics::dropOneWireForPin(pintype_t pin)
{
    auto wireIter = _oneWires.find(pin);
    if (wireIter != _oneWires.end()) {
        if (wireIter->second) {
            wireIter->second->depower();
            delete wireIter->second;
        }
        _oneWires.erase(wireIter);
    }
}

Hydroponics_SystemMode Hydroponics::getSystemMode() const
{
    return _systemData ? _systemData->systemMode : Hydroponics_SystemMode_Undefined;
}

Hydroponics_MeasurementMode Hydroponics::getMeasurementMode() const
{
    return _systemData ? _systemData->measureMode : Hydroponics_MeasurementMode_Undefined;
}

Hydroponics_DisplayOutputMode Hydroponics::getDisplayOutputMode() const
{
    return _systemData ? _systemData->dispOutMode : Hydroponics_DisplayOutputMode_Undefined;
}

Hydroponics_ControlInputMode Hydroponics::getControlInputMode() const
{
    return _systemData ? _systemData->ctrlInMode : Hydroponics_ControlInputMode_Undefined;
}

String Hydroponics::getSystemName() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? String(_systemData->systemName) : String();
}

int8_t Hydroponics::getTimeZoneOffset() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? _systemData->timeZoneOffset : 0;
}

uint16_t Hydroponics::getPollingInterval() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? _systemData->pollingInterval : 0;
}

bool Hydroponics::isPollingFrameOld(unsigned int frame, unsigned int allowance) const
{
    return _pollingFrame - frame > allowance;
}

bool Hydroponics::isAutosaveEnabled() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? _systemData->autosaveEnabled != Hydroponics_Autosave_Disabled : false;
}

bool Hydroponics::isAutosaveFallbackEnabled() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? _systemData->autosaveFallback != Hydroponics_Autosave_Disabled : false;
}

#ifdef HYDRUINO_USE_WIFI

String Hydroponics::getWiFiSSID()
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? String(_systemData->wifiSSID) : String();
}

String Hydroponics::getWiFiPassword()
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData) {
        char wifiPassword[HYDRUINO_NAME_MAXSIZE] = {0};

        if (_systemData->wifiPasswordSeed) {
            randomSeed(_systemData->wifiPasswordSeed);
            for (int charIndex = 0; charIndex < HYDRUINO_NAME_MAXSIZE; ++charIndex) {
                wifiPassword[charIndex] = (char)(_systemData->wifiPassword[charIndex] ^ (uint8_t)random(256));
            }
        } else {
            strncpy(wifiPassword, (const char *)(_systemData->wifiPassword), HYDRUINO_NAME_MAXSIZE);
        }

        return String(wifiPassword);
    }
    return String();
}

#endif

void Hydroponics::notifyRTCTimeUpdated()
{
    _rtcBattFail = false;
    _lastAutosave = 0;
    logger.updateInitTracking();
    scheduler.broadcastDayChange();
}

void Hydroponics::notifyDayChanged()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second->isReservoirType()) {
            auto reservoir = hy_static_ptr_cast<HydroponicsReservoir>(iter->second);

            if (reservoir && reservoir->isFeedClass()) {
                auto feedReservoir = hy_static_ptr_cast<HydroponicsFeedReservoir>(iter->second);
                if (feedReservoir) {feedReservoir->notifyDayChanged(); }
            }
        } else if (iter->second->isCropType()) {
            auto crop = hy_static_ptr_cast<HydroponicsCrop>(iter->second);

            if (crop) { crop->notifyDayChanged(); }
        }
    }
}

void Hydroponics::checkFreeMemory()
{
    auto memLeft = freeMemory();
    if (memLeft != -1 && memLeft < HYDRUINO_SYS_FREERAM_LOWBYTES) {
        broadcastLowMemory();
    }
}

void Hydroponics::broadcastLowMemory()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        iter->second->handleLowMemory();
    }
}

static uint64_t getSDCardFreeSpace()
{
    uint64_t retVal = HYDRUINO_SYS_FREESPACE_LOWSPACE;
    #if defined(CORE_TEENSY)
        auto sd = getHydroponicsInstance()->getSDCard();
        if (sd) {
            retVal = sd->totalSize() - sd->usedSize();
            getHydroponicsInstance()->endSDCard(sd);
        }
    #endif
    return retVal;
}

void Hydroponics::checkFreeSpace()
{
    if ((logger.isLoggingEnabled() || publisher.isPublishingEnabled()) &&
        (!_lastSpaceCheck || unixNow() >= _lastSpaceCheck + (HYDRUINO_SYS_FREESPACE_INTERVAL * SECS_PER_MIN))) {
        if (logger.isLoggingToSDCard() || publisher.isPublishingToSDCard()) {
            uint32_t freeKB = getSDCardFreeSpace() >> 10;
            while (freeKB < HYDRUINO_SYS_FREESPACE_LOWSPACE) {
                logger.cleanupOldestLogs(true);
                publisher.cleanupOldestData(true);
                freeKB = getSDCardFreeSpace();
            }
        }
        // TODO: URL free space
        _lastSpaceCheck = unixNow();
    }
}

void Hydroponics::checkAutosave()
{
    if (isAutosaveEnabled() && unixNow() >= _lastAutosave + (_systemData->autosaveInterval * SECS_PER_MIN)) {
        for (int index = 0; index < 2; ++index) {
            switch (index == 0 ? _systemData->autosaveEnabled : _systemData->autosaveFallback) {
                case Hydroponics_Autosave_EnabledToSDCardJson:
                    saveToSDCard(JSON);
                    break;
                case Hydroponics_Autosave_EnabledToSDCardRaw:
                    saveToSDCard(RAW);
                    break;
                case Hydroponics_Autosave_EnabledToEEPROMJson:
                    saveToEEPROM(JSON);
                    break;
                case Hydroponics_Autosave_EnabledToEEPROMRaw:
                    saveToEEPROM(RAW);
                    break;
                case Hydroponics_Autosave_EnabledToWiFiStorageJson:
                    #ifdef HYDRUINO_USE_WIFI_STORAGE
                        saveToWiFiStorage(JSON);
                    #endif
                    break;
                case Hydroponics_Autosave_EnabledToWiFiStorageRaw:
                    #ifdef HYDRUINO_USE_WIFI_STORAGE
                        saveToWiFiStorage(RAW);
                    #endif
                case Hydroponics_Autosave_Disabled:
                    break;
            }
        }
        _lastAutosave = unixNow();
    }
}
