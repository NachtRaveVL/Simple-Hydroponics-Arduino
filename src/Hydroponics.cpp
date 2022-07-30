/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Main
*/

#include "Hydroponics.h"

static RTC_DS3231 *_rtcSyncProvider = nullptr;
time_t rtcNow() {
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}

void handleInterrupt(byte pin)
{
    if (Hydroponics::_activeInstance) {
        Hydroponics::_activeInstance->handleInterrupt(pin);
    }
}


#ifndef HYDRUINO_DISABLE_MULTITASKING
#ifdef __arm__

int __int_disable_irq(void)
{
    int primask;
    asm volatile("mrs %0, PRIMASK\n" : "=r"(primask));
    asm volatile("cpsid i\n");
    return primask & 1;
}

void __int_restore_irq(int *primask)
{
    if (!(*primask)) {
        asm volatile ("" ::: "memory");
        asm volatile("cpsie i\n");
    }
}

#else

static int8_t _irqCnt = 0;

int __int_disable_irq(void)
{
    if (_irqCnt++ == 0) { noInterrupts(); return 1; }
    return 0;
}

void __int_restore_irq(int *primask)
{
    if (--_irqCnt == 0) { interrupts(); }
}

#endif // /ifdef __arm__
#endif // /ifndef HYDRUINO_DISABLE_MULTITASKING


Hydroponics *Hydroponics::_activeInstance = nullptr;

Hydroponics::Hydroponics(byte piezoBuzzerPin, uint32_t eepromDeviceSize, byte sdCardCSPin, byte controlInputPin1,
                         byte eepromI2CAddress, byte rtcI2CAddress, byte lcdI2CAddress,
                         TwoWire &i2cWire, uint32_t i2cSpeed, uint32_t sdCardSpeed, WiFiClass &wifi)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed), _sdCardSpeed(sdCardSpeed), _wifi(&wifi),
      _piezoBuzzerPin(piezoBuzzerPin), _eepromDeviceSize(eepromDeviceSize), _sdCardCSPin(sdCardCSPin),
      _ctrlInputPin1(controlInputPin1), _ctrlInputPinMap{-1},
      _eepromI2CAddr(eepromI2CAddress | HYDRUINO_SYS_I2CEEPROM_BASEADDR), _rtcI2CAddr(rtcI2CAddress), _lcdI2CAddr(lcdI2CAddress),
      _eeprom(nullptr), _rtc(nullptr), _sd(nullptr),
      _eepromBegan(false), _rtcBegan(false), _rtcBattFail(false), _wifiBegan(false),
#ifndef HYDRUINO_DISABLE_MULTITASKING
      _controlTaskId(TASKMGR_INVALIDID), _dataTaskId(TASKMGR_INVALIDID), _miscTaskId(TASKMGR_INVALIDID),
#endif
      _systemData(nullptr), _suspend(true), _pollingFrame(0), _lastSpaceCheck(0), _lastAutosave(0),
      _sysConfigFile(F("hydruino.cfg")), _sysDataAddress(-1)
{
    _activeInstance = this;
    if (isValidPin(_ctrlInputPin1)) {
        for (byte pinIndex = 0; pinIndex < HYDRUINO_CTRLINPINMAP_MAXSIZE; ++pinIndex) {
            _ctrlInputPinMap[pinIndex] = _ctrlInputPin1 + pinIndex;
        }
    }

    #ifndef HYDRUINO_DISABLE_MULTITASKING
        taskManager.setInterruptCallback(::handleInterrupt);
    #endif
}

Hydroponics::~Hydroponics()
{
    suspend();
    while (_objects.size()) { _objects.erase(_objects.begin()); }
    while (_additives.size()) { dropCustomAdditiveData(_additives.begin()->second); }
    while (_oneWires.size()) { dropOneWireForPin(_oneWires.begin()->first); }
    deallocateEEPROM();
    deallocateRTC();
    deallocateSD();
    _i2cWire = nullptr; _wifi = nullptr;
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
    #if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)) || defined(SD)
        _sd = &SD;
    #else
        if (!_sd) {
            _sd = new SDClass();
            HYDRUINO_SOFT_ASSERT(_sd, SFP(HStr_Err_AllocationFailure));
        }
    #endif
}

void Hydroponics::deallocateSD()
{
    #if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)) || defined(SD)
        _sd = nullptr;
    #else
        if (_sd) { delete _sd; _sd = nullptr; }
    #endif
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

            _scheduler.initFromData(&(_systemData->scheduler));
            _logger.initFromData(&(_systemData->logger));
            _publisher.initFromData(&(_systemData->publisher));

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
            auto configFile = sd->open(_sysConfigFile.c_str(), FILE_READ);

            if (configFile && configFile.available()) {
                retVal = jsonFormat ? initFromJSONStream(&configFile) : initFromBinaryStream(&configFile);
            }

            if (configFile) { configFile.close(); }

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
            auto configFile = sd->open(_sysConfigFile.c_str(), FILE_READ);

            if (configFile && configFile.availableForWrite()) {
                retVal = jsonFormat ? saveToJSONStream(&configFile, false) : saveToBinaryStream(&configFile);
            }

            if (configFile) { configFile.close(); }

            endSDCard(sd);
            return retVal;
        }
    }

    return false;
}

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
                _scheduler.initFromData(&(_systemData->scheduler));
                _logger.initFromData(&(_systemData->logger));
                _publisher.initFromData(&(_systemData->publisher));
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

                HYDRUINO_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectectData()), SFP(HStr_Err_ImportFailure));
                if (data && data->isStandardData()) {
                    if (data->isCalibrationData()) {
                        getCalibrationsStoreInstance()->setUserCalibrationData((HydroponicsCalibrationData *)data);
                    } else if (data->isCropsLibData()) {
                        getCropsLibraryInstance()->setCustomCropData((HydroponicsCropsLibData *)data);
                    } else if (data->isAdditiveData()) {
                        setCustomAdditiveData((HydroponicsCustomAdditiveData *)data);
                    }
                    delete data; data = nullptr;
                } else if (data && data->isObjectectData()) {
                    HydroponicsObject *obj = newObjectFromData(data);
                    delete data; data = nullptr;

                    if (obj && !obj->isUnknownType()) {
                        _objects[obj->getKey()] = shared_ptr<HydroponicsObject>(obj);
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

        if (getCalibrationsStoreInstance()->hasUserCalibrations()) {
            auto calibsStore = getCalibrationsStoreInstance();

            for (auto iter = calibsStore->_calibrationData.begin(); iter != calibsStore->_calibrationData.end(); ++iter) {
                StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;

                JsonObject calibDataObj = doc.to<JsonObject>();
                iter->second->toJSONObject(calibDataObj);

                if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                    HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_ExportFailure));
                    return false;
                }
            }
        }

        if (getCropsLibraryInstance()->hasCustomCrops()) {
            auto cropsLib = getCropsLibraryInstance();

            for (auto iter = cropsLib->_cropsData.begin(); iter != cropsLib->_cropsData.end(); ++iter) {
                if (iter->first >= Hydroponics_CropType_CustomCrop1) {
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

        if (_additives.size()) {
            for (auto iter = _additives.begin(); iter != _additives.end(); ++iter) {
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

                HYDRUINO_SOFT_ASSERT(data && data->isObjectectData(), SFP(HStr_Err_AllocationFailure));
                if (data && data->isObjectectData()) {
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
                _scheduler.initFromData(&(_systemData->scheduler));
                _logger.initFromData(&(_systemData->logger));
                _publisher.initFromData(&(_systemData->publisher));
            } else if (systemData) {
                delete systemData;
            }
        }

        if (_systemData) {
            while (streamIn->available()) {
                HydroponicsData *data = newDataFromBinaryStream(streamIn);

                HYDRUINO_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectectData()), SFP(HStr_Err_AllocationFailure));
                if (data && data->isStandardData()) {
                    if (data->isCalibrationData()) {
                        getCalibrationsStoreInstance()->setUserCalibrationData((HydroponicsCalibrationData *)data);
                    } else if (data->isCropsLibData()) {
                        getCropsLibraryInstance()->setCustomCropData((HydroponicsCropsLibData *)data);
                    } else if (data->isAdditiveData()) {
                        setCustomAdditiveData((HydroponicsCustomAdditiveData *)data);
                    }
                    delete data; data = nullptr;
                } else if (data && data->isObjectectData()) {
                    HydroponicsObject *obj = newObjectFromData(data);
                    delete data; data = nullptr;

                    if (obj && !obj->isUnknownType()) {
                        _objects[obj->getKey()] = shared_ptr<HydroponicsObject>(obj);
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

        if (getCalibrationsStoreInstance()->hasUserCalibrations()) {
            auto calibsStore = getCalibrationsStoreInstance();
            size_t bytesWritten = 0;

            for (auto iter = calibsStore->_calibrationData.begin(); iter != calibsStore->_calibrationData.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (getCropsLibraryInstance()->hasCustomCrops()) {
            auto cropsLib = getCropsLibraryInstance();
            size_t bytesWritten = 0;

            for (auto iter = cropsLib->_cropsData.begin(); iter != cropsLib->_cropsData.end(); ++iter) {
                if (iter->first >= Hydroponics_CropType_CustomCrop1) {
                    bytesWritten += serializeDataToBinaryStream(&(iter->second->data), streamOut);
                }
            }

            HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (_additives.size()) {
            size_t bytesWritten = 0;

            for (auto iter = _additives.begin(); iter != _additives.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                HydroponicsData *data = iter->second->newSaveData();

                HYDRUINO_SOFT_ASSERT(data && data->isObjectectData(), SFP(HStr_Err_AllocationFailure));
                if (data && data->isObjectectData()) {
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
        noTone(_piezoBuzzerPin);
    }
    if (_i2cWire) {
        _i2cWire->setClock(_i2cSpeed);
    }
    if (isValidPin(_sdCardCSPin)) {
        getSPI()->begin();
        // some archs won't init pinMode/set CS high, so we do it manually to be on the safe side
        pinMode(_sdCardCSPin, OUTPUT);
        digitalWrite(_sdCardCSPin, HIGH);
    }
}

void Hydroponics::commonPostInit()
{
    if ((_rtcSyncProvider = getRealTimeClock())) {
        setSyncProvider(rtcNow);
    }

    _scheduler.updateDayTracking();
    _logger.updateInitTracking();

    _lastAutosave = isAutosaveEnabled() ? unixNow() : 0;

    if (!_systemData->wifiPasswordSeed && _systemData->wifiPassword[0]) {
        setWiFiConnection(getWiFiSSID(), getWiFiPassword()); // sets seed and encrypts
    }

    #ifndef HYDRUINO_DISABLE_GUI
        // TODO: tcMenu setup
    #endif

    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        Serial.print(F("Hydroponics::commonPostInit piezoBuzzerPin: "));
        if (isValidPin(_piezoBuzzerPin)) { Serial.print(_piezoBuzzerPin); }
        else { Serial.print(SFP(HStr_Disabled)); }
        Serial.print(F(", eepromDeviceSize: "));
        if (_eepromDeviceSize) { Serial.print(_eepromDeviceSize); }
        else { Serial.print(SFP(HStr_Disabled)); }
        Serial.print(F(", sdCardCSPin: "));
        if (isValidPin(_sdCardCSPin)) { Serial.print(_sdCardCSPin); }
        else { Serial.print(SFP(HStr_Disabled)); }
        Serial.print(F(", controlInputPin1: "));
        if (isValidPin(_ctrlInputPin1)) { Serial.print(_ctrlInputPin1); }
        else { Serial.print(SFP(HStr_Disabled)); }
        Serial.print(F(", EEPROMi2cAddress: 0x"));
        Serial.print(_eepromI2CAddr & ~HYDRUINO_SYS_I2CEEPROM_BASEADDR, HEX);
        Serial.print(F(", RTCi2cAddress: 0x"));
        Serial.print(_rtcI2CAddr, HEX);
        Serial.print(F(", LCDi2cAddress: 0x"));
        Serial.print(_lcdI2CAddr, HEX);
        Serial.print(F(", i2cSpeed: "));
        Serial.print(roundf(getI2CSpeed() / 1000.0f)); Serial.print(F("kHz"));
        Serial.print(F(", sdCardSpeed: "));
        Serial.print(roundf(getSDCardSpeed() / 1000000.0f)); Serial.print(F("MHz"));
        Serial.print(F(", systemMode: "));
        Serial.print(systemModeToString(getSystemMode()));
        Serial.print(F(", measureMode: "));
        Serial.print(measurementModeToString(getMeasurementMode()));
        Serial.print(F(", dispOutMode: "));
        Serial.print(displayOutputModeToString(getDisplayOutputMode()));
        Serial.print(F(", ctrlInMode: "));
        Serial.print(controlInputModeToString(getControlInputMode()));
        Serial.println();
    #endif // /ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
}

void Hydroponics::commonPostSave()
{
    _logger.logSystemSave();

    if (getCalibrationsStoreInstance()->hasUserCalibrations()) {
        auto calibsStore = getCalibrationsStoreInstance();

        for (auto iter = calibsStore->_calibrationData.begin(); iter != calibsStore->_calibrationData.end(); ++iter) {
            iter->second->_unsetModded();
        }
    }

    if (getCropsLibraryInstance()->hasCustomCrops()) {
        auto cropsLib = getCropsLibraryInstance();

        for (auto iter = cropsLib->_cropsData.begin(); iter != cropsLib->_cropsData.end(); ++iter) {
            if (iter->first >= Hydroponics_CropType_CustomCrop1) {
                iter->second->data._unsetModded();
            }
        }
    }

    if (_additives.size()) {
        for (auto iter = _additives.begin(); iter != _additives.end(); ++iter) {
            iter->second->_unsetModded();
        }
    }
}

void controlLoop()
{
    if (Hydroponics::_activeInstance && !Hydroponics::_activeInstance->_suspend) {
        Hydroponics::_activeInstance->updateObjects(0);
        Hydroponics::_activeInstance->_scheduler.update();

        #if HYDRUINO_SYS_ALIVE_LOGGING_ENABLE
            static time_t _lastImAlive = unixNow();
            if (unixNow() >= _lastImAlive + 30) {
                _lastImAlive = unixNow();
                Hydroponics::_activeInstance->_logger.logMessage(F("controlLoopAlive"));
            }
        #endif
    }

    yield();
}

void dataLoop()
{
    if (Hydroponics::_activeInstance && !Hydroponics::_activeInstance->_suspend) {
        Hydroponics::_activeInstance->updateObjects(1);

        #if HYDRUINO_SYS_ALIVE_LOGGING_ENABLE
            static time_t _lastImAlive = unixNow();
            if (unixNow() >= _lastImAlive + 30) {
                _lastImAlive = unixNow();
                Hydroponics::_activeInstance->_logger.logMessage(F("dataLoopAlive"));
            }
        #endif
    }

    yield();
}

void miscLoop()
{
    if (Hydroponics::_activeInstance && !Hydroponics::_activeInstance->_suspend) {
        Hydroponics::_activeInstance->checkFreeMemory();
        Hydroponics::_activeInstance->checkFreeSpace();
        Hydroponics::_activeInstance->checkAutosave();
        Hydroponics::_activeInstance->_logger.update();
        Hydroponics::_activeInstance->_publisher.update();

        #if HYDRUINO_SYS_ALIVE_LOGGING_ENABLE
            static time_t _lastImAlive = unixNow();
            if (unixNow() >= _lastImAlive + 30) {
                _lastImAlive = unixNow();
                Hydroponics::_activeInstance->_logger.logMessage(F("miscLoopAlive"));
            }
        #endif
        #if HYDRUINO_SYS_MEM_LOGGING_ENABLE
            static time_t _lastMemLog = unixNow();
            if (unixNow() >= _lastMemLog + 15) {
                _lastMemLog = unixNow();
                Hydroponics::_activeInstance->_logger.logMessage(F("Free memory: "), String(freeMemory()));
            }
        #endif
    }

    yield();
}

void Hydroponics::launch()
{
    // Forces all sensors to get a new measurement
    _publisher.advancePollingFrame();

    // Create/enable main runloops
    _suspend = false;
    #ifndef HYDRUINO_DISABLE_MULTITASKING
        if (_controlTaskId == TASKMGR_INVALIDID) {
            _controlTaskId = taskManager.scheduleFixedRate(HYDRUINO_CONTROL_LOOP_INTERVAL, controlLoop);
        } else {
            auto controlTask = taskManager.getTask(_controlTaskId);
            if (controlTask) { controlTask->setEnabled(true); }
        }
        if (_dataTaskId == TASKMGR_INVALIDID) {
            _dataTaskId = taskManager.scheduleFixedRate(getPollingInterval(), dataLoop);
        } else {
            auto dataTask = taskManager.getTask(_dataTaskId);
            if (dataTask) { dataTask->setEnabled(true); }
        }
        if (_miscTaskId == TASKMGR_INVALIDID) {
            _miscTaskId = taskManager.scheduleFixedRate(HYDRUINO_MISC_LOOP_INTERVAL, miscLoop);
        } else {
            auto miscTask = taskManager.getTask(_miscTaskId);
            if (miscTask) { miscTask->setEnabled(true); }
        }
    #endif
}

void Hydroponics::suspend()
{
    _suspend = true;
    #ifndef HYDRUINO_DISABLE_MULTITASKING
        if (_controlTaskId != TASKMGR_INVALIDID) {
            auto controlTask = taskManager.getTask(_controlTaskId);
            if (controlTask) { controlTask->setEnabled(false); }
        }
        if (_dataTaskId != TASKMGR_INVALIDID) {
            auto dataTask = taskManager.getTask(_dataTaskId);
            if (dataTask) { dataTask->setEnabled(false); }
        }
        if (_miscTaskId != TASKMGR_INVALIDID) {
            auto miscTask = taskManager.getTask(_miscTaskId);
            if (miscTask) { miscTask->setEnabled(false); }
        }
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

void Hydroponics::updateObjects(int pass)
{
    switch (pass) {
        case 1: {
            _publisher.advancePollingFrame();

            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                if (iter->second->isSensorType()) {
                    auto sensor = static_pointer_cast<HydroponicsSensor>(iter->second);
                    if (sensor->needsPolling()) {
                        sensor->takeMeasurement(); // no force if already current for this frame #
                    }
                }
            }
        } break;

        case 0:
        default: {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                iter->second->update();
            } 
        } break;
    }
}

bool Hydroponics::registerObject(shared_ptr<HydroponicsObject> obj)
{
    HYDRUINO_SOFT_ASSERT(obj->getId().posIndex >= 0 && obj->getId().posIndex < HYDRUINO_POS_MAXSIZE, SFP(HStr_Err_InvalidParameter));
    if (obj && _objects.find(obj->getKey()) == _objects.end()) {
        _objects[obj->getKey()] = obj;

        if (obj->isActuatorType() || obj->isCropType() || obj->isReservoirType()) {
            _scheduler.setNeedsScheduling();
        }
        if (obj->isSensorType()) {
            _publisher.setNeedsTabulation();
        }

        return true;
    }
    return false;
}

bool Hydroponics::unregisterObject(shared_ptr<HydroponicsObject> obj)
{
    auto iter = _objects.find(obj->getKey());
    if (iter != _objects.end()) {
        _objects.erase(iter);
        _scheduler.setNeedsScheduling();
        return true;
    }
    return false;
}

shared_ptr<HydroponicsObject> Hydroponics::objectById(HydroponicsIdentity id) const
{
    if (id.posIndex == HYDRUINO_POS_SEARCH_FROMBEG) {
        while(++id.posIndex < HYDRUINO_POS_MAXSIZE) {
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
        while(--id.posIndex >= 0) {
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

shared_ptr<HydroponicsObject> Hydroponics::objectById_Col(const HydroponicsIdentity &id) const
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
        while(++id.posIndex < HYDRUINO_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    } else {
        id.posIndex = HYDRUINO_POS_SEARCH_FROMEND;
        while(--id.posIndex >= 0) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    }

    return -1;
}

bool Hydroponics::setCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData)
{
    HYDRUINO_SOFT_ASSERT(customAdditiveData, SFP(HStr_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!customAdditiveData || (customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                                                 customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount), SFP(HStr_Err_InvalidParameter));

    if (customAdditiveData && customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
        auto iter = _additives.find(customAdditiveData->reservoirType);
        bool retVal = false;

        if (iter == _additives.end()) {
            auto additiveData = new HydroponicsCustomAdditiveData();

            HYDRUINO_SOFT_ASSERT(additiveData, SFP(HStr_Err_AllocationFailure));
            if (additiveData) {
                *additiveData = *customAdditiveData;
                _additives[customAdditiveData->reservoirType] = additiveData;
                retVal = (_additives.find(customAdditiveData->reservoirType) != _additives.end());
            }
        } else {
            *(iter->second) = *customAdditiveData;
            retVal = true;
        }

        if (retVal) {
            _scheduler.setNeedsScheduling();
            return true;
        }
    }
    return false;
}

bool Hydroponics::dropCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData)
{
    HYDRUINO_HARD_ASSERT(customAdditiveData, SFP(HStr_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!customAdditiveData || (customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                                                 customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount), SFP(HStr_Err_InvalidParameter));

    if (customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
        auto iter = _additives.find(customAdditiveData->reservoirType);
        bool retVal = false;

        if (iter != _additives.end()) {
            if (iter->second) { delete iter->second; }
            _additives.erase(iter);
            retVal = true;
        }

        if (retVal) {
            _scheduler.setNeedsScheduling();
            return true;
        }
    }
    return false;
}

const HydroponicsCustomAdditiveData *Hydroponics::getCustomAdditiveData(Hydroponics_ReservoirType reservoirType) const
{
    HYDRUINO_SOFT_ASSERT(reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                         reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount, SFP(HStr_Err_InvalidParameter));

    if (reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
        auto iter = _additives.find(reservoirType);

        if (iter != _additives.end()) {
            return iter->second;
        }
    }
    return nullptr;
}

bool Hydroponics::tryGetPinLock(byte pin, time_t waitMillis)
{
    time_t startMillis = millis();
    while (1) {
        bool gotLock = false;
        CRITICAL_SECTION {
            auto iter = _pinLocks.find(pin);
            if (iter == _pinLocks.end()) {
                _pinLocks[pin] = true;
                gotLock = (_pinLocks.find(pin) != _pinLocks.end());
            }
        }
        if (gotLock) { return true; }
        else if (millis() - startMillis >= waitMillis) { return false; }
        else { yield(); }
    }
}

void Hydroponics::returnPinLock(byte pin)
{
    CRITICAL_SECTION {
        _pinLocks.erase(pin);
    }
}

void Hydroponics::setControlInputPinMap(byte *pinMap)
{
    HYDRUINO_SOFT_ASSERT(pinMap, SFP(HStr_Err_InvalidParameter));
    const int ctrlInPinCount = getControlInputRibbonPinCount();
    HYDRUINO_SOFT_ASSERT(!pinMap || (ctrlInPinCount > 0), SFP(HStr_Err_UnsupportedOperation));

    if (pinMap && ctrlInPinCount) {
        for (int ribbonPinIndex = 0; ribbonPinIndex < ctrlInPinCount; ++ribbonPinIndex) {
            _ctrlInputPinMap[ribbonPinIndex] = pinMap[ribbonPinIndex];
        }
    }
}

void Hydroponics::setSystemName(String systemName)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && !systemName.equals(getSystemName())) {
        _systemData->_bumpRevIfNotAlreadyModded();
        strncpy(_systemData->systemName, systemName.c_str(), HYDRUINO_NAME_MAXSIZE);
        // TODO: notify GUI to update
    }
}

void Hydroponics::setTimeZoneOffset(int8_t timeZoneOffset)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && _systemData->timeZoneOffset != timeZoneOffset) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->timeZoneOffset = timeZoneOffset;
        // TODO: notify GUI to update
        _scheduler.setNeedsScheduling();
    }
}

void Hydroponics::setPollingInterval(uint16_t pollingInterval)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && _systemData->pollingInterval != pollingInterval) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->pollingInterval = pollingInterval;

        #ifndef HYDRUINO_DISABLE_MULTITASKING
            if (_dataTaskId != TASKMGR_INVALIDID) {
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

void Hydroponics::setAutosaveEnabled(Hydroponics_Autosave autosaveEnabled, uint16_t autosaveInterval)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData && (_systemData->autosaveEnabled != autosaveEnabled || _systemData->autosaveInterval != autosaveInterval)) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->autosaveEnabled = autosaveEnabled;
        _systemData->autosaveInterval = autosaveInterval;
        _lastAutosave = autosaveEnabled ? unixNow() : 0;
    }
}

void Hydroponics::setWiFiConnection(String ssid, String password)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    if (_systemData) {
        bool ssidChanged = ssid.equals(getWiFiSSID());
        bool passChanged = password.equals(getWiFiPassword());

        if (ssidChanged || passChanged || (password.length() && !_systemData->wifiPasswordSeed)) {
            _systemData->_bumpRevIfNotAlreadyModded();

            if (ssid.length()) {
                strncpy(_systemData->wifiSSID, ssid.c_str(), HYDRUINO_NAME_MAXSIZE);
            } else {
                memset(_systemData->wifiSSID, '\0', HYDRUINO_NAME_MAXSIZE);
            }

            if (password.length()) {
                randomSeed(unixNow());
                _systemData->wifiPasswordSeed = random(1, RANDOM_MAX);

                randomSeed(_systemData->wifiPasswordSeed);
                for (int charIndex = 0; charIndex < HYDRUINO_NAME_MAXSIZE; ++charIndex) {
                    _systemData->wifiPassword[charIndex] = (byte)(charIndex < password.length() ? password[charIndex] : '\0') ^ (byte)random(256);
                }
            } else {
                _systemData->wifiPasswordSeed = 0;
                memset(_systemData->wifiPassword, '\0', HYDRUINO_NAME_MAXSIZE);
            }

            if (_wifiBegan && (ssidChanged || passChanged)) { _wifi->disconnect(); _wifiBegan = false; } // forces re-connect on next getWifi
        }
    }
}

void Hydroponics::setRealTimeClockTime(DateTime time)
{
    auto rtc = getRealTimeClock();
    if (rtc) {
        rtc->adjust(DateTime((uint32_t)(time.unixtime() + (-getTimeZoneOffset() * SECS_PER_HOUR))));
        notifyRTCTimeUpdated();
    }
}

uint32_t Hydroponics::getSDCardSpeed() const
{
    #if defined(CORE_TEENSY)
        return 25000000U;
    #else
        return _sdCardSpeed;
    #endif
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

byte Hydroponics::getControlInputPin(int ribbonPinIndex) const
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
                _logger.logWarning(F("RTC battery failure, time needs reset."));
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
        #if defined(ESP32) || defined(ESP8266)
            bool sdBegan = _sd->begin(_sdCardCSPin, *getSPI(), getSDCardSpeed());
        #elif defined(CORE_TEENSY)
            bool sdBegan = _sd->begin(_sdCardCSPin); // card speed not possible to set on teensy
        #else
            bool sdBegan = _sd->begin(getSDCardSpeed(), _sdCardCSPin);
        #endif

        if (!sdBegan) { deallocateSD(); }

        return _sd && sdBegan ? _sd : nullptr;
    }

    return _sd;
}

void Hydroponics::endSDCard(SDClass *sd)
{
    #if !defined(CORE_TEENSY) // no delayed write on teensy's SD impl
        sd->end();
    #endif
}

WiFiClass *Hydroponics::getWiFi(bool begin)
{
    int status = _wifi ? _wifi->status() : WL_NO_SHIELD;

    if (_wifi && begin && (!_wifiBegan || status != WL_CONNECTED)) {
        if (status == WL_CONNECTED) {
            _wifiBegan = true;
        } else if (status == WL_NO_SHIELD) {
            _wifiBegan = false;
        } else { // attempt connection
            String ssid = getWiFiSSID();
            String pass = getWiFiPassword();

            status = pass.length() ? _wifi->begin(const_cast<char *>(ssid.c_str()), pass.c_str())
                                   : _wifi->begin(const_cast<char *>(ssid.c_str()));

            _wifiBegan = (status == WL_CONNECTED);
        }

        return _wifi && _wifiBegan ? _wifi : nullptr;
    }

    return _wifi;
}

OneWire *Hydroponics::getOneWireForPin(byte pin)
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

void Hydroponics::dropOneWireForPin(byte pin)
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

bool Hydroponics::inOperationalMode() const
{
    return !_suspend;
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

bool Hydroponics::getRTCBatteryFailure() const
{
    return _rtcBattFail;
}

uint16_t Hydroponics::getPollingInterval() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HStr_Err_NotYetInitialized));
    return _systemData ? _systemData->pollingInterval : 0;
}

uint16_t Hydroponics::getPollingFrame() const
{
    return _pollingFrame;
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
                wifiPassword[charIndex] = (char)(_systemData->wifiPassword[charIndex] ^ (byte)random(256));
            }
        } else {
            strncpy(wifiPassword, (const char *)(_systemData->wifiPassword), HYDRUINO_NAME_MAXSIZE);
        }

        return String(wifiPassword);
    }
    return String();
}

void Hydroponics::notifyRTCTimeUpdated()
{
    _rtcBattFail = false;
    _lastAutosave = isAutosaveEnabled() ? unixNow() : 0;
    _logger.updateInitTracking();
    _scheduler.broadcastDayChange();
}

void Hydroponics::notifyDayChanged()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second) {
            if (iter->second->isReservoirType()) {
                auto reservoir = static_pointer_cast<HydroponicsReservoir>(iter->second);

                if (reservoir && reservoir->isFeedClass()) {
                    auto feedReservoir = static_pointer_cast<HydroponicsFeedReservoir>(iter->second);
                    if (feedReservoir) {feedReservoir->notifyDayChanged(); }
                }
            } else if (iter->second->isCropType()) {
                auto crop = static_pointer_cast<HydroponicsCrop>(iter->second);

                if (crop) { crop->notifyDayChanged(); }
            }
        }
    }
}

void Hydroponics::handleInterrupt(byte pin)
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second && iter->second->isSensorType()) {
            auto sensor = static_pointer_cast<HydroponicsSensor>(iter->second);
            if (sensor->getInputPin() == pin && sensor->isBinaryClass()) {
                auto binarySensor = static_pointer_cast<HydroponicsBinarySensor>(sensor);
                if (binarySensor) { binarySensor->notifyISRTriggered(); }
            }
        }
    }
}

void Hydroponics::checkFreeMemory()
{
    int memLeft = freeMemory();
    if (memLeft != -1 && memLeft < HYDRUINO_SYS_FREERAM_LOWBYTES) {
        broadcastLowMemory();
    }
}

void Hydroponics::broadcastLowMemory()
{
    #ifdef HYDRUINO_USE_STDCPP_CONTAINERS
        _objects.shrink_to_fit();
        _additives.shrink_to_fit();
        _oneWires.shrink_to_fit();
        _pinLocks.shrink_to_fit();
        getCalibrationsStoreInstance()->_calibrationData.shrink_to_fit();
        getCropsLibraryInstance()->_cropsData.shrink_to_fit();
    #endif

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second) {
            iter->second->handleLowMemory();
        }
    }

    _scheduler.handleLowMemory();
    _logger.handleLowMemory();
    _publisher.handleLowMemory();
}

static uint32_t getSDCardFreeSpace()
{
    uint32_t retVal = HYDRUINO_SYS_FREESPACE_LOWSPACE;
    #if defined(CORE_TEENSY)
        auto sd = getHydroponicsInstance()->getSDCard();
        if (sd) {
            retVal = sd.vol()->freeClusterCount() * (sd.vol()->blocksPerCluster() >> 1);
            getHydroponicsInstance()->endSDCard(sd);
        }
    #endif
    return retVal;
}

void Hydroponics::checkFreeSpace()
{
    if ((_logger.isLoggingEnabled() || _publisher.isPublishingEnabled()) &&
        (!_lastSpaceCheck || unixNow() >= _lastSpaceCheck + (HYDRUINO_SYS_FREESPACE_INTERVAL * SECS_PER_MIN))) {
        if (_logger.isLoggingToSDCard() || _publisher.isPublishingToSDCard()) {
            uint32_t freeKB = getSDCardFreeSpace();
            while(freeKB < HYDRUINO_SYS_FREESPACE_LOWSPACE) {
                _logger.cleanupOldestLogs(true);
                _publisher.cleanupOldestData(true);
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
        switch (_systemData->autosaveEnabled) {
            case Hydroponics_Autosave_EnabledToSDCardJson:
                saveToSDCard(true);
                break;
            case Hydroponics_Autosave_EnabledToSDCardRaw:
                saveToSDCard(false);
                break;
            case Hydroponics_Autosave_EnabledToEEPROMJson:
                saveToEEPROM(true);
                break;
            case Hydroponics_Autosave_EnabledToEEPROMRaw:
                saveToEEPROM(false);
                break;
            case Hydroponics_Autosave_Disabled:
                break;
        }
        _lastAutosave = unixNow();
    }
}
