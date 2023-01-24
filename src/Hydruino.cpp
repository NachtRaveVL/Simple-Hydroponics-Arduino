/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino System
*/

#include "Hydruino.h"

static HydroRTCInterface *_rtcSyncProvider = nullptr;
time_t rtcNow() {
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}

void handleInterrupt(pintype_t pin)
{
    if (Hydruino::_activeInstance) {
        for (auto iter = Hydruino::_activeInstance->_objects.begin(); iter != Hydruino::_activeInstance->_objects.end(); ++iter) {
            if (iter->second->isSensorType()) {
                auto sensor = static_pointer_cast<HydroSensor>(iter->second);
                if (sensor->isBinaryClass()) {
                    auto binarySensor = static_pointer_cast<HydroBinarySensor>(sensor);
                    if (binarySensor && binarySensor->getInputPin().pin == pin) { binarySensor->notifyISRTriggered(); }
                }
            }
        }
    }
}


Hydruino *Hydruino::_activeInstance = nullptr;

Hydruino::Hydruino(pintype_t piezoBuzzerPin,
                   Hydro_EEPROMType eepromType, DeviceSetup eepromSetup,
                   Hydro_RTCType rtcType, DeviceSetup rtcSetup,
                   DeviceSetup sdSetup,
                   DeviceSetup netSetup,
                   DeviceSetup gpsSetup,
                   pintype_t *ctrlInputPins,
                   DeviceSetup lcdSetup)
    : _piezoBuzzerPin(piezoBuzzerPin),
      _eepromType(eepromType), _eepromSetup(eepromSetup), _eeprom(nullptr), _eepromBegan(false),
      _rtcType(rtcType), _rtcSetup(rtcSetup), _rtc(nullptr), _rtcBegan(false), _rtcBattFail(false),
      _sdSetup(sdSetup), _sd(nullptr), _sdBegan(false), _sdOut(0),
#ifdef HYDRO_USE_NET
      _netSetup(netSetup), _netBegan(false),
#endif
#ifdef HYDRO_USE_GPS
      _gpsSetup(gpsSetup), _gps(nullptr), _gpsBegan(false),
#endif
#ifdef HYDRO_USE_GUI
      _activeUIInstance(nullptr), _ctrlInputPins(ctrlInputPins), _lcdSetup(lcdSetup),
#endif
#ifdef HYDRO_USE_MULTITASKING
      _controlTaskId(TASKMGR_INVALIDID), _dataTaskId(TASKMGR_INVALIDID), _miscTaskId(TASKMGR_INVALIDID),
#endif
      _systemData(nullptr), _suspend(true), _pollingFrame(0), _lastSpaceCheck(0), _lastAutosave(0),
      _sysConfigFilename(SFP(HStr_Default_ConfigFilename)), _sysDataAddress(-1)
{
    _activeInstance = this;
}

Hydruino::~Hydruino()
{
    suspend();
#ifdef HYDRO_USE_GUI
    if (_activeUIInstance) { delete _activeUIInstance; _activeUIInstance = nullptr; }
#endif
    deselectPinMuxers();
    while (_objects.size()) { _objects.erase(_objects.begin()); }
    while (_oneWires.size()) { dropOneWireForPin(_oneWires.begin()->first); }
    while (_pinMuxers.size()) { _pinMuxers.erase(_pinMuxers.begin()); }
    deallocateEEPROM();
    deallocateRTC();
    deallocateSD();
#ifdef HYDRO_USE_GPS
    deallocateGPS();
#endif
    if (this == _activeInstance) { _activeInstance = nullptr; }
    if (_systemData) { delete _systemData; _systemData = nullptr; }
}

void Hydruino::allocateEEPROM()
{
    if (!_eeprom && _eepromType != Hydro_EEPROMType_None && _eepromSetup.cfgType == DeviceSetup::I2CSetup) {
        _eeprom = new I2C_eeprom(_eepromSetup.cfgAs.i2c.address | HYDRO_SYS_I2CEEPROM_BASEADDR,
                                 getEEPROMSize(), _eepromSetup.cfgAs.i2c.wire);
        _eepromBegan = false;
        HYDRO_SOFT_ASSERT(_eeprom, SFP(HStr_Err_AllocationFailure));
    }
}

void Hydruino::deallocateEEPROM()
{
    if (_eeprom) {
        delete _eeprom; _eeprom = nullptr;
        _eepromBegan = false;
    }
}

void Hydruino::allocateRTC()
{
    if (!_rtc && _rtcType != Hydro_RTCType_None && _rtcSetup.cfgType == DeviceSetup::I2CSetup) {
        switch (_rtcType) {
            case Hydro_RTCType_DS1307:
                _rtc = new HydroRTCWrapper<RTC_DS1307>();
                break;
            case Hydro_RTCType_DS3231:
                _rtc = new HydroRTCWrapper<RTC_DS3231>();
                break;
            case Hydro_RTCType_PCF8523:
                _rtc = new HydroRTCWrapper<RTC_PCF8523>();
                break;
            case Hydro_RTCType_PCF8563:
                _rtc = new HydroRTCWrapper<RTC_PCF8563>();
                break;
            default: break;
        }
        _rtcBegan = false;
        HYDRO_SOFT_ASSERT(_rtc, SFP(HStr_Err_AllocationFailure));
        HYDRO_HARD_ASSERT(_rtcSetup.cfgAs.i2c.address == B000, F("RTClib does not support i2c multi-addressing, only i2c address B000 may be used"));
    }
}

void Hydruino::deallocateRTC()
{
    if (_rtc) {
        if (_rtcSyncProvider == _rtc) { setSyncProvider(nullptr); _rtcSyncProvider = nullptr; }
        delete _rtc; _rtc = nullptr;
        _rtcBegan = false;
    }
}

void Hydruino::allocateSD()
{
    if (!_sd && _sdSetup.cfgType == DeviceSetup::SPISetup) {
        #if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SD))
            _sd = &SD;
        #else
            _sd = new SDClass();
        #endif
        _sdBegan = false;
        HYDRO_SOFT_ASSERT(_sd, SFP(HStr_Err_AllocationFailure));
    }
}

void Hydruino::deallocateSD()
{
    if (_sd) {
        #if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SD))
            _sd = nullptr;
        #else
            delete _sd; _sd = nullptr;
        #endif
        _sdBegan = false;
    }
}

#ifdef HYDRO_USE_GPS

void Hydruino::allocateGPS()
{
    if (!_gps && _gpsSetup.cfgType != DeviceSetup::None) {
        switch (_gpsSetup.cfgType) {
            case DeviceSetup::TTLSetup:
                _gps = new GPSClass(_gpsSetup.cfgAs.ttl.serial);
                break;
            case DeviceSetup::I2CSetup:
                _gps = new GPSClass(_gpsSetup.cfgAs.i2c.wire);
                break;
            case DeviceSetup::SPISetup:
                _gps = new GPSClass(_gpsSetup.cfgAs.spi.spi, _gpsSetup.cfgAs.spi.cs);
                break;
            default: break;
        }
        _gpsBegan = false;
        HYDRO_SOFT_ASSERT(_gps, SFP(HStr_Err_AllocationFailure));
    }
}

void Hydruino::deallocateGPS()
{
    if (_gps) {
        delete _gps; _gps = nullptr;
        _gpsBegan = false;
    }
}

#endif

void Hydruino::init(Hydro_SystemMode systemMode,
                    Hydro_MeasurementMode measureMode,
                    Hydro_DisplayOutputMode dispOutMode,
                    Hydro_ControlInputMode ctrlInMode)
{
    HYDRO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();

        HYDRO_SOFT_ASSERT((int)systemMode >= 0 && systemMode < Hydro_SystemMode_Count, SFP(HStr_Err_InvalidParameter));
        HYDRO_SOFT_ASSERT((int)measureMode >= 0 && measureMode < Hydro_MeasurementMode_Count, SFP(HStr_Err_InvalidParameter));
        #ifdef HYDRO_USE_GUI
            HYDRO_SOFT_ASSERT((int)dispOutMode >= 0 && dispOutMode < Hydro_DisplayOutputMode_Count, SFP(HStr_Err_InvalidParameter));
            HYDRO_SOFT_ASSERT((int)ctrlInMode >= 0 && ctrlInMode < Hydro_ControlInputMode_Count, SFP(HStr_Err_InvalidParameter));
        #endif

        _systemData = new HydroSystemData();
        HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_AllocationFailure));

        if (_systemData) {
            _systemData->systemMode = systemMode;
            _systemData->measureMode = measureMode;
            #ifdef HYDRO_USE_GUI
                _systemData->dispOutMode = dispOutMode;
                _systemData->ctrlInMode = ctrlInMode;
            #else
                _systemData->dispOutMode = Hydro_DisplayOutputMode_Disabled;
                _systemData->ctrlInMode = Hydro_ControlInputMode_Disabled;
            #endif

            commonPostInit();
        }
    }  
}

bool Hydruino::initFromEEPROM(bool jsonFormat)
{
    HYDRO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();

        if (getEEPROM() && _eepromBegan && _sysDataAddress != -1) {
            HydroEEPROMStream eepromStream(_sysDataAddress, getEEPROMSize() - _sysDataAddress);
            return jsonFormat ? initFromJSONStream(&eepromStream) : initFromBinaryStream(&eepromStream);
        }
    }

    return false;
}

bool Hydruino::saveToEEPROM(bool jsonFormat)
{
    HYDRO_HARD_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));

    if (_systemData) {
        if (getEEPROM() && _eepromBegan && _sysDataAddress != -1) {
            HydroEEPROMStream eepromStream(_sysDataAddress, getEEPROMSize() - _sysDataAddress);
            return jsonFormat ? saveToJSONStream(&eepromStream) : saveToBinaryStream(&eepromStream);
        }
    }

    return false;
}

bool Hydruino::initFromSDCard(bool jsonFormat)
{
    HYDRO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));

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

bool Hydruino::saveToSDCard(bool jsonFormat)
{
    HYDRO_HARD_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));

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

#ifdef HYDRO_USE_WIFI_STORAGE

bool Hydruino::initFromWiFiStorage(bool jsonFormat)
{
    HYDRO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();

        auto configFile = WiFiStorage.open(_sysConfigFilename.c_str());

        if (configFile) {
            auto configFileStream = HydroWiFiStorageFileStream(configFile);
            return jsonFormat ? initFromJSONStream(&configFileStream) : initFromBinaryStream(&configFileStream);

            configFile.close();
        }
    }

    return false;
}

bool Hydruino::saveToWiFiStorage(bool jsonFormat)
{
    HYDRO_HARD_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));

    if (_systemData) {
        if (WiFiStorage.exists(_sysConfigFilename.c_str())) {
            WiFiStorage.remove(_sysConfigFilename.c_str());
        }
        auto configFile = WiFiStorage.open(_sysConfigFilename.c_str());

        if (configFile) {
            auto configFileStream = HydroWiFiStorageFileStream(configFile);
            return jsonFormat ? saveToJSONStream(&configFileStream, false) : saveToBinaryStream(&configFileStream);

            configFileStream.flush();
            configFile.close();
        }
    }

    return false;
}

#endif

bool Hydruino::initFromJSONStream(Stream *streamIn)
{
    HYDRO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));
    HYDRO_SOFT_ASSERT(streamIn && streamIn->available(), SFP(HStr_Err_InvalidParameter));

    if (!_systemData && streamIn && streamIn->available()) {
        commonPreInit();

        {   StaticJsonDocument<HYDRO_JSON_DOC_SYSSIZE> doc;
            deserializeJson(doc, *streamIn);
            JsonObjectConst systemDataObj = doc.as<JsonObjectConst>();
            HydroSystemData *systemData = (HydroSystemData *)newDataFromJSONObject(systemDataObj);

            HYDRO_SOFT_ASSERT(systemData && systemData->isSystemData(), SFP(HStr_Err_ImportFailure));
            if (systemData && systemData->isSystemData()) {
                _systemData = systemData;
            } else if (systemData) {
                delete systemData;
            }
        }

        if (_systemData) {
            while (streamIn->available()) {
                StaticJsonDocument<HYDRO_JSON_DOC_DEFSIZE> doc;
                deserializeJson(doc, *streamIn);
                JsonObjectConst dataObj = doc.as<JsonObjectConst>();
                HydroData *data = newDataFromJSONObject(dataObj);

                HYDRO_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectData()), SFP(HStr_Err_ImportFailure));
                if (data && data->isStandardData()) {
                    if (data->isCalibrationData()) {
                        hydroCalibrations.setUserCalibrationData((HydroCalibrationData *)data);
                    } else if (data->isCropsLibData()) {
                        hydroCropsLib.setUserCropData((HydroCropsLibData *)data);
                    } else if (data->isAdditiveData()) {
                        hydroAdditives.setCustomAdditiveData((HydroCustomAdditiveData *)data);
                    }
                    delete data; data = nullptr;
                } else if (data && data->isObjectData()) {
                    HydroObject *obj = newObjectFromData(data);
                    delete data; data = nullptr;

                    if (obj && !obj->isUnknownType()) {
                        _objects[obj->getKey()] = SharedPtr<HydroObject>(obj);
                    } else {
                        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_ImportFailure));
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

        HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_InitializationFailure));
        if (_systemData) { commonPostInit(); }
        return _systemData;
    }

    return false;
}

bool Hydruino::saveToJSONStream(Stream *streamOut, bool compact)
{
    HYDRO_HARD_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    HYDRO_SOFT_ASSERT(streamOut, SFP(HStr_Err_InvalidParameter));

    if (_systemData && streamOut) {
        {   StaticJsonDocument<HYDRO_JSON_DOC_SYSSIZE> doc;

            JsonObject systemDataObj = doc.to<JsonObject>();
            _systemData->toJSONObject(systemDataObj);

            if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
                return false;
            }
        }

        if (hydroCalibrations.hasUserCalibrations()) {
            for (auto iter = hydroCalibrations._calibrationData.begin(); iter != hydroCalibrations._calibrationData.end(); ++iter) {
                StaticJsonDocument<HYDRO_JSON_DOC_DEFSIZE> doc;

                JsonObject calibDataObj = doc.to<JsonObject>();
                iter->second->toJSONObject(calibDataObj);

                if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
                    return false;
                }
            }
        }

        if (hydroCropsLib.hasUserCrops()) {
            for (auto iter = hydroCropsLib._cropsData.begin(); iter != hydroCropsLib._cropsData.end(); ++iter) {
                if (iter->second->userSet) {
                    StaticJsonDocument<HYDRO_JSON_DOC_DEFSIZE> doc;

                    JsonObject cropDataObj = doc.to<JsonObject>();
                    iter->second->data.toJSONObject(cropDataObj);

                    if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
                        return false;
                    }
                }
            }
        }

        if (hydroAdditives.hasCustomAdditives()) {
            for (auto iter = hydroAdditives._additives.begin(); iter != hydroAdditives._additives.end(); ++iter) {
                StaticJsonDocument<HYDRO_JSON_DOC_DEFSIZE> doc;

                JsonObject additiveDataObj = doc.to<JsonObject>();
                iter->second->toJSONObject(additiveDataObj);

                if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
                    return false;
                }
            }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                HydroData *data = iter->second->newSaveData();

                HYDRO_SOFT_ASSERT(data && data->isObjectData(), SFP(HStr_Err_AllocationFailure));
                if (data && data->isObjectData()) {
                    StaticJsonDocument<HYDRO_JSON_DOC_DEFSIZE> doc;

                    JsonObject objectDataObj = doc.to<JsonObject>();
                    data->toJSONObject(objectDataObj);
                    delete data; data = nullptr;

                    if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
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

bool Hydruino::initFromBinaryStream(Stream *streamIn)
{
    HYDRO_HARD_ASSERT(!_systemData, SFP(HStr_Err_AlreadyInitialized));
    HYDRO_SOFT_ASSERT(streamIn && streamIn->available(), SFP(HStr_Err_InvalidParameter));

    if (!_systemData && streamIn && streamIn->available()) {
        commonPreInit();

        {   HydroSystemData *systemData = (HydroSystemData *)newDataFromBinaryStream(streamIn);

            HYDRO_SOFT_ASSERT(systemData && systemData->isSystemData(), SFP(HStr_Err_ImportFailure));
            if (systemData && systemData->isSystemData()) {
                _systemData = systemData;
            } else if (systemData) {
                delete systemData;
            }
        }

        if (_systemData) {
            while (streamIn->available()) {
                HydroData *data = newDataFromBinaryStream(streamIn);

                HYDRO_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectData()), SFP(HStr_Err_AllocationFailure));
                if (data && data->isStandardData()) {
                    if (data->isCalibrationData()) {
                        hydroCalibrations.setUserCalibrationData((HydroCalibrationData *)data);
                    } else if (data->isCropsLibData()) {
                        hydroCropsLib.setUserCropData((HydroCropsLibData *)data);
                    } else if (data->isAdditiveData()) {
                        hydroAdditives.setCustomAdditiveData((HydroCustomAdditiveData *)data);
                    }
                    delete data; data = nullptr;
                } else if (data && data->isObjectData()) {
                    HydroObject *obj = newObjectFromData(data);
                    delete data; data = nullptr;

                    if (obj && !obj->isUnknownType()) {
                        _objects[obj->getKey()] = SharedPtr<HydroObject>(obj);
                    } else {
                        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_ImportFailure));
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

        HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_InitializationFailure));
        if (_systemData) { commonPostInit(); }
        return _systemData;
    }

    return false;
}

bool Hydruino::saveToBinaryStream(Stream *streamOut)
{
    HYDRO_HARD_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    HYDRO_SOFT_ASSERT(streamOut, SFP(HStr_Err_InvalidParameter));

    if (_systemData && streamOut) {
        {   size_t bytesWritten = serializeDataToBinaryStream(_systemData, streamOut);

            HYDRO_SOFT_ASSERT(!bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (hydroCalibrations.hasUserCalibrations()) {
            size_t bytesWritten = 0;

            for (auto iter = hydroCalibrations._calibrationData.begin(); iter != hydroCalibrations._calibrationData.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            HYDRO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (hydroCropsLib.hasUserCrops()) {
            size_t bytesWritten = 0;

            for (auto iter = hydroCropsLib._cropsData.begin(); iter != hydroCropsLib._cropsData.end(); ++iter) {
                if (iter->first >= Hydro_CropType_CustomCrop1) {
                    bytesWritten += serializeDataToBinaryStream(&(iter->second->data), streamOut);
                }
            }

            HYDRO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (hydroAdditives.hasCustomAdditives()) {
            size_t bytesWritten = 0;

            for (auto iter = hydroAdditives._additives.begin(); iter != hydroAdditives._additives.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            HYDRO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                HydroData *data = iter->second->newSaveData();

                HYDRO_SOFT_ASSERT(data && data->isObjectData(), SFP(HStr_Err_AllocationFailure));
                if (data && data->isObjectData()) {
                    size_t bytesWritten = serializeDataToBinaryStream(data, streamOut);
                    delete data; data = nullptr;

                    HYDRO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
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

void Hydruino::commonPreInit()
{
    Map<uintptr_t,uint32_t> began;

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
    if (_eepromSetup.cfgType == DeviceSetup::I2CSetup) {
        if (began.find((uintptr_t)_eepromSetup.cfgAs.i2c.wire) == began.end() || _eepromSetup.cfgAs.i2c.speed < began[(uintptr_t)_eepromSetup.cfgAs.i2c.wire]) {
            _eepromSetup.cfgAs.i2c.wire->begin();
            _eepromSetup.cfgAs.i2c.wire->setClock((began[(uintptr_t)_eepromSetup.cfgAs.i2c.wire] = _eepromSetup.cfgAs.i2c.speed));
        }
    }
    if (_rtcSetup.cfgType == DeviceSetup::I2CSetup) {
        if (began.find((uintptr_t)_rtcSetup.cfgAs.i2c.wire) == began.end() || _rtcSetup.cfgAs.i2c.speed < began[(uintptr_t)_rtcSetup.cfgAs.i2c.wire]) {
            _rtcSetup.cfgAs.i2c.wire->begin();
            _rtcSetup.cfgAs.i2c.wire->setClock((began[(uintptr_t)_rtcSetup.cfgAs.i2c.wire] = _rtcSetup.cfgAs.i2c.speed));
        }
    }
    #ifdef HYDRO_USE_GUI
        if (_lcdSetup.cfgType == DeviceSetup::I2CSetup) {
            if (began.find((uintptr_t)_lcdSetup.cfgAs.i2c.wire) == began.end() || _lcdSetup.cfgAs.i2c.speed < began[(uintptr_t)_lcdSetup.cfgAs.i2c.wire]) {
                _lcdSetup.cfgAs.i2c.wire->begin();
                _lcdSetup.cfgAs.i2c.wire->setClock((began[(uintptr_t)_lcdSetup.cfgAs.i2c.wire] = _lcdSetup.cfgAs.i2c.speed));
            }
        }
    #endif
    if (_sdSetup.cfgType == DeviceSetup::SPISetup) {
        if (began.find((uintptr_t)_rtcSetup.cfgAs.spi.spi) == began.end()) {
            _sdSetup.cfgAs.spi.spi->begin();
            began[(uintptr_t)_rtcSetup.cfgAs.spi.spi] = 0;
        }
        if (isValidPin(_sdSetup.cfgAs.spi.cs)) {
            pinMode(_sdSetup.cfgAs.spi.cs, OUTPUT);
            digitalWrite(_sdSetup.cfgAs.spi.cs, HIGH);
        }
    }
    #ifdef HYDRO_USE_NET
        if (_netSetup.cfgType == DeviceSetup::SPISetup) {
            if (began.find((uintptr_t)_netSetup.cfgAs.spi.spi) == began.end()) {
                _netSetup.cfgAs.spi.spi->begin();
                began[(uintptr_t)_netSetup.cfgAs.spi.spi] = 0;
            }
            if (isValidPin(_netSetup.cfgAs.spi.cs)) {
                pinMode(_netSetup.cfgAs.spi.cs, OUTPUT);
                digitalWrite(_netSetup.cfgAs.spi.cs, HIGH);
            }
            #ifdef HYDRO_USE_ETHERNET
                Ethernet.init(_netSetup.cfgAs.spi.cs);
            #endif
        } else if (_netSetup.cfgType == DeviceSetup::TTLSetup) {
            if (began.find((uintptr_t)_netSetup.cfgAs.ttl.serial) == began.end() || _netSetup.cfgAs.ttl.baud < began[(uintptr_t)_netSetup.cfgAs.ttl.serial]) {
                _netSetup.cfgAs.ttl.serial->begin((began[(uintptr_t)_netSetup.cfgAs.ttl.serial] = _netSetup.cfgAs.ttl.baud), (uint16_t)HYDRO_SYS_ATWIFI_SERIALMODE);
            }
            #ifdef HYDRO_USE_AT_WIFI
                WiFi.init(_netSetup.cfgAs.ttl.serial);
            #endif
        }
    #endif
    #ifdef HYDRO_USE_WIFI_STORAGE
        //WiFiStorage.begin();
    #endif
    #ifdef HYDRO_USE_MULTITASKING
        taskManager.setInterruptCallback(&handleInterrupt);
    #endif
}

#ifdef HYDRO_USE_VERBOSE_OUTPUT
static void printDeviceSetup(String prefix, const DeviceSetup &devSetup)
{
    switch(devSetup.cfgType) {
        case DeviceSetup::I2CSetup:
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(F("I2CAddress: 0x"));
            Serial.print(devSetup.cfgAs.i2c.address, HEX);
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(F("I2CSpeed: "));
            Serial.print(roundf(devSetup.cfgAs.i2c.speed / 1000.0f)); Serial.print(F("kHz"));
            break;

        case DeviceSetup::SPISetup:
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(F("SPICSPin: "));
            if (isValidPin(devSetup.cfgAs.spi.cs)) { Serial.print(devSetup.cfgAs.spi.cs); }
            else { Serial.print(SFP(HStr_Disabled)); }
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(F("SPISpeed: "));
            Serial.print(roundf(devSetup.cfgAs.spi.speed / 1000000.0f)); Serial.print(F("MHz"));
            break;

        case DeviceSetup::TTLSetup:
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(F("TTLBaud: "));
            Serial.print(devSetup.cfgAs.ttl.baud); Serial.print(F("bps"));
            break;

        default:
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(':'); Serial.print(' ');
            Serial.print(SFP(HStr_Disabled));
            break;
    }
}
#endif

void Hydruino::commonPostInit()
{
    if ((_rtcSyncProvider = getRealTimeClock())) {
        setSyncProvider(rtcNow);
    }

    scheduler.updateDayTracking(); // also calls setNeedsScheduling
    logger.updateInitTracking();
    publisher.setNeedsTabulation();

    #ifdef HYDRO_USE_WIFI
        if (!_systemData->wifiPasswordSeed && _systemData->wifiPassword[0]) {
            setWiFiConnection(getWiFiSSID(), getWiFiPassword()); // sets seed and encrypts
        }
    #endif

    #ifdef HYDRO_USE_VERBOSE_OUTPUT
        #if 1 // set to 0 if you just want this gone
            Serial.print(F("Hydruino::commonPostInit piezoBuzzerPin: "));
            if (isValidPin(_piezoBuzzerPin)) { Serial.print(_piezoBuzzerPin); }
            else { Serial.print(SFP(HStr_Disabled)); }
            Serial.print(F(", eepromSize: "));
            if (getEEPROMSize()) { Serial.print(getEEPROMSize()); }
            else { Serial.print(SFP(HStr_Disabled)); }
            printDeviceSetup(F("eeprom"), _eepromSetup);
            Serial.print(F(", rtcType: "));
            if (_rtcType != Hydro_RTCType_None) { Serial.print(_rtcType); }
            else { Serial.print(SFP(HStr_Disabled)); }
            printDeviceSetup(F("rtc"), _rtcSetup);
            printDeviceSetup(F("sd"), _sdSetup);
            #ifdef HYDRO_USE_NET
                printDeviceSetup(F("net"), _netSetup);
            #endif
            #ifdef HYDRO_USE_GUI
                Serial.print(F(", controlInputPins: "));
                if (getControlInputRibbonPinCount() && _ctrlInputPins && isValidPin(_ctrlInputPins[0])) {
                    Serial.print('{');
                    for (int i = 0; i < getControlInputRibbonPinCount(); ++i) {
                        if (i) { Serial.print(','); }
                        Serial.print(_ctrlInputPins[i]);
                    }
                    Serial.print('}');
                }
                else { Serial.print(SFP(HStr_Disabled)); }
                printDeviceSetup(F("lcd"), _lcdSetup);
            #endif
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
    #endif // /ifdef HYDRO_USE_VERBOSE_OUTPUT
}

void Hydruino::commonPostSave()
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
    if (Hydruino::_activeInstance && !Hydruino::_activeInstance->_suspend) {
        #ifdef HYDRO_USE_VERBOSE_OUTPUT
            Serial.println(F("controlLoop")); flushYield();
        #endif

        for (auto iter = Hydruino::_activeInstance->_objects.begin(); iter != Hydruino::_activeInstance->_objects.end(); ++iter) {
            iter->second->update();
        }

        Hydruino::_activeInstance->scheduler.update();

        #ifdef HYDRO_USE_VERBOSE_OUTPUT
            Serial.println(F("~controlLoop")); flushYield();
        #endif
    }

    yield();
}

void dataLoop()
{
    if (Hydruino::_activeInstance && !Hydruino::_activeInstance->_suspend) {
        #ifdef HYDRO_USE_VERBOSE_OUTPUT
            Serial.println(F("dataLoop")); flushYield();
        #endif

        Hydruino::_activeInstance->publisher.advancePollingFrame();

        for (auto iter = Hydruino::_activeInstance->_objects.begin(); iter != Hydruino::_activeInstance->_objects.end(); ++iter) {
            if (iter->second->isSensorType()) {
                auto sensor = static_pointer_cast<HydroSensor>(iter->second);
                if (sensor->needsPolling()) {
                    sensor->takeMeasurement(); // no force if already current for this frame #, we're just ensuring data for publisher
                }
            }
        }

        #ifdef HYDRO_USE_VERBOSE_OUTPUT
            Serial.println(F("~dataLoop")); flushYield();
        #endif
    }

    yield();
}

void miscLoop()
{
    if (Hydruino::_activeInstance && !Hydruino::_activeInstance->_suspend) {
        #ifdef HYDRO_USE_VERBOSE_OUTPUT
            Serial.println(F("miscLoop")); flushYield();
        #endif

        Hydruino::_activeInstance->checkFreeMemory();
        Hydruino::_activeInstance->checkFreeSpace();
        Hydruino::_activeInstance->checkAutosave();

        Hydruino::_activeInstance->publisher.update();

        #if HYDRO_SYS_MEM_LOGGING_ENABLE
        {   static time_t _lastMemLog = unixNow();
            if (unixNow() >= _lastMemLog + 15) {
                _lastMemLog = unixNow();
                Hydruino::_activeInstance->logger.logMessage(String(F("Free memory: ")), String(freeMemory()));
            }
        }
        #endif

        #ifdef HYDRO_USE_VERBOSE_OUTPUT
            Serial.println(F("~miscLoop")); flushYield();
        #endif
    }

    yield();
}

void Hydruino::launch()
{
    // Forces all sensors to get a new measurement
    publisher.advancePollingFrame();

    // Create/enable main runloops
    _suspend = false;
    #ifdef HYDRO_USE_MULTITASKING
        if (!isValidTask(_controlTaskId)) {
            _controlTaskId = taskManager.scheduleFixedRate(HYDRO_CONTROL_LOOP_INTERVAL, controlLoop);
        } else {
            taskManager.setTaskEnabled(_controlTaskId, true);
        }
        if (!isValidTask(_dataTaskId)) {
            _dataTaskId = taskManager.scheduleFixedRate(getPollingInterval(), dataLoop);
        } else {
            taskManager.setTaskEnabled(_dataTaskId, true);
        }
        if (!isValidTask(_miscTaskId)) {
            _miscTaskId = taskManager.scheduleFixedRate(HYDRO_MISC_LOOP_INTERVAL, miscLoop);
        } else {
            taskManager.setTaskEnabled(_miscTaskId, true);
        }
    #endif

    #ifdef HYDRO_USE_VERBOSE_OUTPUT
        Serial.println(F("Hydruino::launch System launched!")); flushYield();
    #endif
}

void Hydruino::suspend()
{
    _suspend = true;
    #ifdef HYDRO_USE_MULTITASKING
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

    #ifdef HYDRO_USE_VERBOSE_OUTPUT
        Serial.println(F("Hydruino::suspend System suspended!")); flushYield();
    #endif
}

void Hydruino::update()
{
    #ifdef HYDRO_USE_MULTITASKING
        taskManager.runLoop(); // tcMenu also uses this system to run its UI
    #else
        controlLoop();
        dataLoop();
        miscLoop();
    #endif

    #ifdef HYDRO_USE_MQTT
        if (publisher._mqttClient) { publisher._mqttClient->loop(); }
    #endif
}

bool Hydruino::registerObject(SharedPtr<HydroObject> obj)
{
    HYDRO_SOFT_ASSERT(obj->getId().posIndex >= 0 && obj->getId().posIndex < HYDRO_POS_MAXSIZE, SFP(HStr_Err_InvalidParameter));
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

bool Hydruino::unregisterObject(SharedPtr<HydroObject> obj)
{
    auto iter = _objects.find(obj->getKey());
    if (iter != _objects.end()) {
        _objects.erase(iter);

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

SharedPtr<HydroObject> Hydruino::objectById(HydroIdentity id) const
{
    if (id.posIndex == HYDRO_POS_SEARCH_FROMBEG) {
        while (++id.posIndex < HYDRO_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (iter != _objects.end()) {
                if (id.keyString == iter->second->getKeyString()) {
                    return iter->second;
                } else {
                    objectById_Col(id);
                }
            }
        }
    } else if (id.posIndex == HYDRO_POS_SEARCH_FROMEND) {
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

SharedPtr<HydroObject> Hydruino::objectById_Col(const HydroIdentity &id) const
{
    HYDRO_SOFT_ASSERT(false, F("Hashing collision")); // exhaustive search must be performed, publishing may miss values

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (id.keyString == iter->second->getKeyString()) {
            return iter->second;
        }
    }

    return nullptr;
}

Hydro_PositionIndex Hydruino::firstPosition(HydroIdentity id, bool taken)
{
    if (id.posIndex != HYDRO_POS_SEARCH_FROMEND) {
        id.posIndex = HYDRO_POS_SEARCH_FROMBEG;
        while (++id.posIndex < HYDRO_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    } else {
        id.posIndex = HYDRO_POS_SEARCH_FROMEND;
        while (--id.posIndex >= 0) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    }

    return -1;
}

bool Hydruino::tryGetPinLock(pintype_t pin, time_t waitMillis)
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

void Hydruino::deselectPinMuxers()
{
    for (auto iter = Hydruino::_activeInstance->_pinMuxers.begin(); iter != Hydruino::_activeInstance->_pinMuxers.end(); ++iter) {
        iter->second->deselect();
    }
}

OneWire *Hydruino::getOneWireForPin(pintype_t pin)
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

void Hydruino::dropOneWireForPin(pintype_t pin)
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

void Hydruino::setSystemName(String systemName)
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && !systemName.equals(getSystemName())) {
        _systemData->_bumpRevIfNotAlreadyModded();
        strncpy(_systemData->systemName, systemName.c_str(), HYDRO_NAME_MAXSIZE);
        if (_activeUIInstance) { _activeUIInstance->setNeedsLayout(); }
    }
}

void Hydruino::setTimeZoneOffset(int8_t timeZoneOffset)
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && _systemData->timeZoneOffset != timeZoneOffset) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->timeZoneOffset = timeZoneOffset;
        scheduler.setNeedsScheduling();
        if (_activeUIInstance) { _activeUIInstance->setNeedsLayout(); }
    }
}

void Hydruino::setPollingInterval(uint16_t pollingInterval)
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && _systemData->pollingInterval != pollingInterval) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->pollingInterval = pollingInterval;

        #ifdef HYDRO_USE_MULTITASKING
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

void Hydruino::setAutosaveEnabled(Hydro_Autosave autosaveEnabled, Hydro_Autosave autosaveFallback, uint16_t autosaveInterval)
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && (_systemData->autosaveEnabled != autosaveEnabled || _systemData->autosaveFallback != autosaveFallback || _systemData->autosaveInterval != autosaveInterval)) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->autosaveEnabled = autosaveEnabled;
        _systemData->autosaveFallback = autosaveFallback;
        _systemData->autosaveInterval = autosaveInterval;
    }
}

#ifdef HYDRO_USE_WIFI

void Hydruino::setWiFiConnection(String ssid, String pass)
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData) {
        bool ssidChanged = ssid.equals(getWiFiSSID());
        bool passChanged = pass.equals(getWiFiPassword());

        if (ssidChanged || passChanged || (pass.length() && !_systemData->wifiPasswordSeed)) {
            _systemData->_bumpRevIfNotAlreadyModded();

            if (ssid.length()) {
                strncpy(_systemData->wifiSSID, ssid.c_str(), HYDRO_NAME_MAXSIZE);
            } else {
                memset(_systemData->wifiSSID, '\0', HYDRO_NAME_MAXSIZE);
            }

            if (pass.length()) {
                randomSeed(unixNow());
                _systemData->wifiPasswordSeed = random(1, RANDOM_MAX);

                randomSeed(_systemData->wifiPasswordSeed);
                for (int charIndex = 0; charIndex < HYDRO_NAME_MAXSIZE; ++charIndex) {
                    _systemData->wifiPassword[charIndex] = (uint8_t)(charIndex < pass.length() ? pass[charIndex] : '\0') ^ (uint8_t)random(256);
                }
            } else {
                _systemData->wifiPasswordSeed = 0;
                memset(_systemData->wifiPassword, '\0', HYDRO_NAME_MAXSIZE);
            }

            if (_netBegan && (ssidChanged || passChanged)) { WiFi.disconnect(); _netBegan = false; } // forces re-connect on next getWiFi
        }
    }
}

#endif
#ifdef HYDRO_USE_ETHERNET

void Hydruino::setEthernetConnection(const uint8_t *macAddress)
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData) {
        bool macChanged = memcmp(macAddress, getMACAddress(), sizeof(uint8_t[6])) != 0;

        if (macChanged) {
            _systemData->_bumpRevIfNotAlreadyModded();

            memcpy(_systemData->macAddress, macAddress, sizeof(uint8_t[6]));

            if (_netBegan) { Ethernet.setMACAddress(macAddress); }
        }
    }
}

#endif

void Hydruino::setRealTimeClockTime(DateTime time)
{
    auto rtc = getRealTimeClock();
    if (rtc) {
        rtc->adjust(DateTime((uint32_t)(time.unixtime() + (-getTimeZoneOffset() * SECS_PER_HOUR))));
        notifyRTCTimeUpdated();
    }
}

int Hydruino::getControlInputRibbonPinCount() const
{
    switch (getControlInputMode()) {
        case Hydro_ControlInputMode_2x2Matrix:
        case Hydro_ControlInputMode_4xButton:
            return 4;
        case Hydro_ControlInputMode_6xButton:
            return 6;
        case Hydro_ControlInputMode_RotaryEncoder:
            return 5;
        default:
            return 0;
    }
}

pintype_t Hydruino::getControlInputPin(int ribbonPinIndex) const
{
    int ctrlInPinCount = getControlInputRibbonPinCount();
    HYDRO_SOFT_ASSERT(ctrlInPinCount > 0, SFP(HStr_Err_UnsupportedOperation));
    HYDRO_SOFT_ASSERT(ctrlInPinCount <= 0 || (ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount), SFP(HStr_Err_InvalidParameter));

    return ctrlInPinCount && ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount ? _ctrlInputPins[ribbonPinIndex] : -1;
}

I2C_eeprom *Hydruino::getEEPROM(bool begin)
{
    if (!_eeprom) { allocateEEPROM(); }

    if (_eeprom && begin && !_eepromBegan) {
        _eepromBegan = _eeprom->begin();

        if (!_eepromBegan) { deallocateEEPROM(); }
    }

    return (!begin || _eepromBegan) ? _eeprom : nullptr;
}

HydroRTCInterface *Hydruino::getRealTimeClock(bool begin)
{
    if (!_rtc) { allocateRTC(); }

    if (_rtc && begin && !_rtcBegan) {
        _rtcBegan = _rtc->begin(_rtcSetup.cfgAs.i2c.wire);

        if (_rtcBegan) {
            bool rtcBattFailBefore = _rtcBattFail;
            _rtcBattFail = _rtc->lostPower();
            if (_rtcBattFail && !rtcBattFailBefore) {
                logger.logWarning(SFP(HStr_Log_RTCBatteryFailure));
            }
        } else { deallocateRTC(); }
    }

    return (!begin || _rtcBegan) ? _rtc : nullptr;
}

SDClass *Hydruino::getSDCard(bool begin)
{
    if (!_sd) { allocateSD(); }

    if (_sd && begin) {
        if (!_sdBegan) {
            #if defined(ESP32)
                _sdBegan = _sd->begin(_sdSetup.cfgAs.spi.cs, _sdSetup.cfgAs.spi.spi, _sdSetup.cfgAs.spi.speed);
            #elif defined(CORE_TEENSY)
                _sdBegan = _sd->begin(_sdSetup.cfgAs.spi.cs); // card speed not possible to set on teensy
            #else
                _sdBegan = _sd->begin(_sdSetup.cfgAs.spi.speed, _sdSetup.cfgAs.spi.cs);
            #endif
        }

        if (!_sdBegan && _sdOut == 0) { deallocateSD(); }

        if (_sd && _sdBegan) { _sdOut++; }
    }

    return (!begin || _sdBegan) ? _sd : nullptr;
}

void Hydruino::endSDCard(SDClass *sd)
{
    #if defined(CORE_TEENSY)
        --_sdOut; // no delayed write on teensy's SD impl
    #else
        if (--_sdOut == 0 && _sd) {
            _sd->end();
        }
    #endif
}

#ifdef HYDRO_USE_WIFI

WiFiClass *Hydruino::getWiFi(String ssid, String pass, bool begin)
{
    int status = WiFi.status();

    if (begin && (!_netBegan || status != WL_CONNECTED)) {
        if (status == WL_CONNECTED) {
            _netBegan = true;
        } else if (status == WL_NO_SHIELD) {
            _netBegan = false;
        } else { // attempt connection
            #ifdef HYDRO_USE_AT_WIFI
                status = WiFi.begin(ssid.c_str(), pass.c_str());
            #else
                status = pass.length() ? WiFi.begin(const_cast<char *>(ssid.c_str()), pass.c_str())
                                       : WiFi.begin(const_cast<char *>(ssid.c_str()));
            #endif

            _netBegan = (status == WL_CONNECTED);
        }
    }

    return (!begin || _netBegan) ? &WiFi : nullptr;
}

#endif
#ifdef HYDRO_USE_ETHERNET

EthernetClass *Hydruino::getEthernet(const uint8_t *macAddress, bool begin)
{
    int status = Ethernet.linkStatus();

    if (begin && (!_netBegan || status != LinkON)) {
        if (status == LinkON) {
            _netBegan = true;
        } else if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            _netBegan = false;
        } else { // attempt connection
            status = Ethernet.begin(const_cast<uint8_t *>(getMACAddress()));

            _netBegan = (status == LinkON);
        }
    }

    return (!begin || _netBegan) ? &Ethernet : nullptr;
}

#endif
#ifdef HYDRO_USE_GPS

GPSClass *Hydruino::getGPS(bool begin)
{
    if (!_gps) { allocateGPS(); }

    if (_gps && begin && !_gpsBegan) {
        switch (_gpsSetup.cfgType) {
            case DeviceSetup::TTLSetup:
                _gpsBegan = _gps->begin(_gpsSetup.cfgAs.ttl.baud);
            case DeviceSetup::I2CSetup:
                _gpsBegan = _gps->begin(_gpsSetup.cfgAs.i2c.speed);
            case DeviceSetup::SPISetup:
                _gpsBegan = _gps->begin(_gpsSetup.cfgAs.spi.speed);
            default: break;
        }
        if (!_gpsBegan) { deallocateGPS(); }
    }

    return (!begin || _gpsBegan) ? _gps : nullptr;
}

#endif

Hydro_SystemMode Hydruino::getSystemMode() const
{
    return _systemData ? _systemData->systemMode : Hydro_SystemMode_Undefined;
}

Hydro_MeasurementMode Hydruino::getMeasurementMode() const
{
    return _systemData ? _systemData->measureMode : Hydro_MeasurementMode_Undefined;
}

Hydro_DisplayOutputMode Hydruino::getDisplayOutputMode() const
{
    return _systemData ? _systemData->dispOutMode : Hydro_DisplayOutputMode_Undefined;
}

Hydro_ControlInputMode Hydruino::getControlInputMode() const
{
    return _systemData ? _systemData->ctrlInMode : Hydro_ControlInputMode_Undefined;
}

String Hydruino::getSystemName() const
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? String(_systemData->systemName) : String();
}

int8_t Hydruino::getTimeZoneOffset() const
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? _systemData->timeZoneOffset : 0;
}

uint16_t Hydruino::getPollingInterval() const
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? _systemData->pollingInterval : 0;
}

bool Hydruino::isPollingFrameOld(unsigned int frame, unsigned int allowance) const
{
    return _pollingFrame - frame > allowance;
}

bool Hydruino::isAutosaveEnabled() const
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? _systemData->autosaveEnabled != Hydro_Autosave_Disabled : false;
}

bool Hydruino::isAutosaveFallbackEnabled() const
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? _systemData->autosaveFallback != Hydro_Autosave_Disabled : false;
}

#ifdef HYDRO_USE_WIFI

String Hydruino::getWiFiSSID() const
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? String(_systemData->wifiSSID) : String();
}

String Hydruino::getWiFiPassword() const
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData) {
        char wifiPassword[HYDRO_NAME_MAXSIZE] = {0};

        if (_systemData->wifiPasswordSeed) {
            randomSeed(_systemData->wifiPasswordSeed);
            for (int charIndex = 0; charIndex < HYDRO_NAME_MAXSIZE; ++charIndex) {
                wifiPassword[charIndex] = (char)(_systemData->wifiPassword[charIndex] ^ (uint8_t)random(256));
            }
        } else {
            strncpy(wifiPassword, (const char *)(_systemData->wifiPassword), HYDRO_NAME_MAXSIZE);
        }

        return String(wifiPassword);
    }
    return String();
}

#endif
#ifdef HYDRO_USE_ETHERNET

const uint8_t *Hydruino::getMACAddress() const
{
    HYDRO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? &_systemData->macAddress[0] : nullptr;
}

#endif

void Hydruino::notifyRTCTimeUpdated()
{
    _rtcBattFail = false;
    _lastAutosave = 0;
    logger.updateInitTracking();
    scheduler.broadcastDayChange();
}

void Hydruino::notifyDayChanged()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second->isReservoirType()) {
            auto reservoir = static_pointer_cast<HydroReservoir>(iter->second);

            if (reservoir && reservoir->isFeedClass()) {
                auto feedReservoir = static_pointer_cast<HydroFeedReservoir>(iter->second);
                if (feedReservoir) {feedReservoir->notifyDayChanged(); }
            }
        } else if (iter->second->isCropType()) {
            auto crop = static_pointer_cast<HydroCrop>(iter->second);

            if (crop) { crop->notifyDayChanged(); }
        }
    }
}

void Hydruino::checkFreeMemory()
{
    auto memLeft = freeMemory();
    if (memLeft != -1 && memLeft < HYDRO_SYS_FREERAM_LOWBYTES) {
        broadcastLowMemory();
    }
}

void Hydruino::broadcastLowMemory()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        iter->second->handleLowMemory();
    }
}

static uint64_t getSDCardFreeSpace()
{
    uint64_t retVal = HYDRO_SYS_FREESPACE_LOWSPACE;
    #if defined(CORE_TEENSY)
        auto sd = getHydroInstance()->getSDCard();
        if (sd) {
            retVal = sd->totalSize() - sd->usedSize();
            getHydroInstance()->endSDCard(sd);
        }
    #endif
    return retVal;
}

void Hydruino::checkFreeSpace()
{
    if ((logger.isLoggingEnabled() || publisher.isPublishingEnabled()) &&
        (!_lastSpaceCheck || unixNow() >= _lastSpaceCheck + (HYDRO_SYS_FREESPACE_INTERVAL * SECS_PER_MIN))) {
        if (logger.isLoggingToSDCard() || publisher.isPublishingToSDCard()) {
            uint32_t freeKB = getSDCardFreeSpace() >> 10;
            while (freeKB < HYDRO_SYS_FREESPACE_LOWSPACE) {
                logger.cleanupOldestLogs(true);
                publisher.cleanupOldestData(true);
                freeKB = getSDCardFreeSpace();
            }
        }
        // TODO: URL free space
        _lastSpaceCheck = unixNow();
    }
}

void Hydruino::checkAutosave()
{
    if (isAutosaveEnabled() && unixNow() >= _lastAutosave + (_systemData->autosaveInterval * SECS_PER_MIN)) {
        for (int index = 0; index < 2; ++index) {
            switch (index == 0 ? _systemData->autosaveEnabled : _systemData->autosaveFallback) {
                case Hydro_Autosave_EnabledToSDCardJson:
                    saveToSDCard(JSON);
                    break;
                case Hydro_Autosave_EnabledToSDCardRaw:
                    saveToSDCard(RAW);
                    break;
                case Hydro_Autosave_EnabledToEEPROMJson:
                    saveToEEPROM(JSON);
                    break;
                case Hydro_Autosave_EnabledToEEPROMRaw:
                    saveToEEPROM(RAW);
                    break;
                case Hydro_Autosave_EnabledToWiFiStorageJson:
                    #ifdef HYDRO_USE_WIFI_STORAGE
                        saveToWiFiStorage(JSON);
                    #endif
                    break;
                case Hydro_Autosave_EnabledToWiFiStorageRaw:
                    #ifdef HYDRO_USE_WIFI_STORAGE
                        saveToWiFiStorage(RAW);
                    #endif
                case Hydro_Autosave_Disabled:
                    break;
            }
        }
        _lastAutosave = unixNow();
    }
}
