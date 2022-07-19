/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Main
*/

#include "Hydroponics.h"

static RTC_DS3231 *_rtcSyncProvider = nullptr;
time_t rtcNow() {
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}

void handleInterrupt(pintype_t pin)
{
    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) { hydroponics->handleInterrupt(pin); }
}

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

int8_t _irqCnt = 0;

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


Hydroponics *Hydroponics::_activeInstance = nullptr;

Hydroponics::Hydroponics(byte piezoBuzzerPin, uint32_t eepromDeviceSize, byte sdCardCSPin, byte controlInputPin1,
                         byte eepromI2CAddress, byte rtcI2CAddress, byte lcdI2CAddress,
                         TwoWire &i2cWire, uint32_t i2cSpeed, uint32_t sdCardSpeed, WiFiClass &wifi)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed), _sdCardSpeed(sdCardSpeed), _wifi(&wifi),
      _piezoBuzzerPin(piezoBuzzerPin), _eepromDeviceSize(eepromDeviceSize), _sdCardCSPin(sdCardCSPin),
      _ctrlInputPin1(controlInputPin1), _ctrlInputPinMap{-1},
      _eepromI2CAddr(eepromI2CAddress), _rtcI2CAddr(rtcI2CAddress), _lcdI2CAddr(lcdI2CAddress),
      _eeprom(nullptr), _rtc(nullptr), _sd(nullptr), _eepromBegan(false), _rtcBegan(false), _rtcBattFail(false), _wifiBegan(false),
#ifdef HYDRUINO_USE_TASKSCHEDULER
      _controlTask(nullptr), _dataTask(nullptr), _miscTask(nullptr),
#elif defined(HYDRUINO_USE_SCHEDULER)
      _loopsStarted(false), _suspend(false),
#endif
      _systemData(nullptr), _pollingFrame(0)
{
    _activeInstance = this;
    if (isValidPin(_piezoBuzzerPin)) {
        EasyBuzzer.setPin(_piezoBuzzerPin);
    }
    if (isValidPin(_ctrlInputPin1)) {
        for (byte pinIndex = 0; pinIndex < HYDRUINO_CTRLINPINMAP_MAXSIZE; ++pinIndex) {
            _ctrlInputPinMap[pinIndex] = _ctrlInputPin1 + pinIndex;
        }
    }
    taskManager.setInterruptCallback(::handleInterrupt);
}

Hydroponics::~Hydroponics()
{
    #ifdef HYDRUINO_USE_TASKSCHEDULER
        if (_controlTask) { _controlTask->abort(); }
        if (_dataTask) { _dataTask->abort(); }
        if (_miscTask) { _miscTask->abort(); }
        if (_controlTask) { delete _controlTask; _controlTask = nullptr; }
        if (_dataTask) { delete _dataTask; _dataTask = nullptr; }
        if (_miscTask) { delete _miscTask; _miscTask = nullptr; }
    #elif defined(HYDRUINO_USE_SCHEDULER)
        _suspend = true;
    #endif
    while (_objects.size()) { _objects.erase(_objects.begin()); }
    while (_additives.size()) { dropCustomAdditiveData(_additives.begin()->second); }
    while (_oneWires.size()) { delete _oneWires.begin()->second; _oneWires.erase(_oneWires.begin()); }
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
        HYDRUINO_SOFT_ASSERT(_eeprom, SFP(HS_Err_AllocationFailure));
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
        HYDRUINO_SOFT_ASSERT(_rtc, SFP(HS_Err_AllocationFailure));
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
            HYDRUINO_SOFT_ASSERT(_sd, SFP(HS_Err_AllocationFailure));
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
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HS_Err_AlreadyInitialized));

    if (!_systemData) {
        HYDRUINO_SOFT_ASSERT((int)systemMode >= 0 && systemMode < Hydroponics_SystemMode_Count, SFP(HS_Err_InvalidParameter));
        HYDRUINO_SOFT_ASSERT((int)measureMode >= 0 && measureMode < Hydroponics_MeasurementMode_Count, SFP(HS_Err_InvalidParameter));
        #ifndef HYDRUINO_DISABLE_GUI
            HYDRUINO_SOFT_ASSERT((int)dispOutMode >= 0 && dispOutMode < Hydroponics_DisplayOutputMode_Count, SFP(HS_Err_InvalidParameter));
            HYDRUINO_SOFT_ASSERT((int)ctrlInMode >= 0 && ctrlInMode < Hydroponics_ControlInputMode_Count, SFP(HS_Err_InvalidParameter));
        #endif

        _systemData = new HydroponicsSystemData();
        HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_AllocationFailure));

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

            commonInit();
        }
    }  
}

bool Hydroponics::initFromEEPROM(bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HS_Err_AlreadyInitialized));

    if (!_systemData) {
        if (getEEPROM() && _eepromBegan) {
            HydroponicsEEPROMStream eepromStream;
            return jsonFormat ? initFromJSONStream(&eepromStream) : initFromBinaryStream(&eepromStream);
        }
    }

    return false;
}

bool Hydroponics::saveToEEPROM(bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));

    if (_systemData) {
        if (getEEPROM() && _eepromBegan) {
            HydroponicsEEPROMStream eepromStream;
            return jsonFormat ? saveToJSONStream(&eepromStream) : saveToBinaryStream(&eepromStream);
        }
    }

    return false;
}

bool Hydroponics::initFromSDCard(String configFile, bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HS_Err_AlreadyInitialized));

    if (!_systemData) {
        auto sd = getSDCard();
        if (sd) {
            bool retVal = false;
            auto config = sd->open(configFile);
            if (config) {
                retVal = jsonFormat ? initFromJSONStream(&config) : initFromBinaryStream(&config);
                config.close();
            }
            sd->end();
            return retVal;
        }
    }

    return false;
}

bool Hydroponics::saveToSDCard(String configFile, bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));

    if (!_systemData) {
        auto sd = getSDCard();
        if (sd) {
            bool retVal = false;
            auto config = sd->open(configFile);
            if (config) {
                retVal = jsonFormat ? saveToJSONStream(&config) : saveToBinaryStream(&config);
                config.close();
            }
            sd->end();
            return retVal;
        }
    }

    return false;
}

bool Hydroponics::initFromJSONStream(Stream *streamIn)
{
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HS_Err_AlreadyInitialized));
    HYDRUINO_SOFT_ASSERT(streamIn && streamIn->available(), SFP(HS_Err_InvalidParameter));

    if (!_systemData && streamIn && streamIn->available()) {

        {   StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;
            deserializeJson(doc, *streamIn);
            JsonObjectConst systemDataObj = doc.as<JsonObjectConst>();
            HydroponicsSystemData *systemData = (HydroponicsSystemData *)newDataFromJSONObject(systemDataObj);

            HYDRUINO_SOFT_ASSERT(systemData && systemData->isSystemData(), SFP(HS_Err_ImportFailure));
            if (systemData && systemData->isSystemData()) {
                _systemData = systemData;
                _scheduler.initFromData(&(_systemData->scheduler));
                //_publisher.initFromData(&(_systemData->publisher));
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

                HYDRUINO_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectData()), SFP(HS_Err_ImportFailure));
                if (data && data->isStandardData()) {
                    if (data->isCalibrationData()) {
                        getCalibrationsStoreInstance()->setUserCalibrationData((HydroponicsCalibrationData *)data);
                    } else if (data->isCropsLibData()) {
                        getCropsLibraryInstance()->setCustomCropData((HydroponicsCropsLibData *)data);
                    } else if (data->isAdditiveData()) {
                        setCustomAdditiveData((HydroponicsCustomAdditiveData *)data);
                    }
                    delete data; data = nullptr;
                } else if (data && data->isObjectData()) {
                    HydroponicsObject *obj = newObjectFromData(data);
                    delete data; data = nullptr;

                    if (!(obj && !obj->isUnknownType() && _objects.insert(obj->getId().key, shared_ptr<HydroponicsObject>(obj)).second)) {
                        HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_ImportFailure));
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

        HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_InitializationFailure));
        if (_systemData) {
            commonInit();
        }
        return _systemData;
    }

    return false;
}

bool Hydroponics::saveToJSONStream(Stream *streamOut, bool compact)
{
    HYDRUINO_HARD_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(streamOut, SFP(HS_Err_InvalidParameter));

    if (_systemData && streamOut) {
        {   StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;

            JsonObject systemDataObj = doc.to<JsonObject>();
            _systemData->toJSONObject(systemDataObj);

            if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_ExportFailure));
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
                    HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_ExportFailure));
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
                        HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_ExportFailure));
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
                    HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_ExportFailure));
                    return false;
                }
            }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                HydroponicsData *data = iter->second->newSaveData();

                HYDRUINO_SOFT_ASSERT(data && data->isObjectData(), SFP(HS_Err_AllocationFailure));
                if (data && data->isObjectData()) {
                    StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;

                    JsonObject objectDataObj = doc.to<JsonObject>();
                    data->toJSONObject(objectDataObj);
                    delete data; data = nullptr;

                    if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                        HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_ExportFailure));
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
    HYDRUINO_HARD_ASSERT(!_systemData, SFP(HS_Err_AlreadyInitialized));
    HYDRUINO_SOFT_ASSERT(streamIn && streamIn->available(), SFP(HS_Err_InvalidParameter));

    if (!_systemData && streamIn && streamIn->available()) {
        {   HydroponicsSystemData *systemData = (HydroponicsSystemData *)newDataFromBinaryStream(streamIn);

            HYDRUINO_SOFT_ASSERT(systemData && systemData->isSystemData(), SFP(HS_Err_ImportFailure));
            if (systemData && systemData->isSystemData()) {
                _systemData = systemData;
                _scheduler.initFromData(&(_systemData->scheduler));
                //_publisher.initFromData(&(_systemData->publisher));
            } else if (systemData) {
                delete systemData;
            }
        }

        if (_systemData) {
            while (streamIn->available()) {
                HydroponicsData *data = newDataFromBinaryStream(streamIn);

                HYDRUINO_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectData()), SFP(HS_Err_AllocationFailure));
                if (data && data->isStandardData()) {
                    if (data->isCalibrationData()) {
                        getCalibrationsStoreInstance()->setUserCalibrationData((HydroponicsCalibrationData *)data);
                    } else if (data->isCropsLibData()) {
                        getCropsLibraryInstance()->setCustomCropData((HydroponicsCropsLibData *)data);
                    } else if (data->isAdditiveData()) {
                        setCustomAdditiveData((HydroponicsCustomAdditiveData *)data);
                    }
                    delete data; data = nullptr;
                } else if (data && data->isObjectData()) {
                    HydroponicsObject *obj = newObjectFromData(data);
                    delete data; data = nullptr;

                    if (!(obj && !obj->isUnknownType() && _objects.insert(obj->getId().key, shared_ptr<HydroponicsObject>(obj)).second)) {
                        HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_ImportFailure));
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

        HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_InitializationFailure));
        if (_systemData) {
            commonInit();
        }
        return _systemData;
    }

    return false;
}

bool Hydroponics::saveToBinaryStream(Stream *streamOut)
{
    HYDRUINO_HARD_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
    HYDRUINO_SOFT_ASSERT(streamOut, SFP(HS_Err_InvalidParameter));

    if (_systemData && streamOut) {
        {   size_t bytesWritten = serializeDataToBinaryStream(_systemData, streamOut);

            HYDRUINO_SOFT_ASSERT(!bytesWritten, SFP(HS_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (getCalibrationsStoreInstance()->hasUserCalibrations()) {
            auto calibsStore = getCalibrationsStoreInstance();
            size_t bytesWritten = 0;

            for (auto iter = calibsStore->_calibrationData.begin(); iter != calibsStore->_calibrationData.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HS_Err_ExportFailure));
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

            HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HS_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (_additives.size()) {
            size_t bytesWritten = 0;

            for (auto iter = _additives.begin(); iter != _additives.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HS_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                HydroponicsData *data = iter->second->newSaveData();

                HYDRUINO_SOFT_ASSERT(data && data->isObjectData(), SFP(HS_Err_AllocationFailure));
                if (data && data->isObjectData()) {
                    size_t bytesWritten = serializeDataToBinaryStream(data, streamOut);
                    delete data; data = nullptr;

                    HYDRUINO_SOFT_ASSERT(bytesWritten, SFP(HS_Err_ExportFailure));
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

void Hydroponics::commonInit()
{
    if ((_rtcSyncProvider = getRealTimeClock())) {
        setSyncProvider(rtcNow);
    }

    if (!_systemData->wifiPasswordSeed && _systemData->wifiPassword[0]) {
        setWiFiConnection(getWiFiSSID(), getWiFiPassword());
    }

    // TODO: tcMenu setup

    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        Serial.print(F("Hydroponics::commonInit piezoBuzzerPin: "));
        if (isValidPin(_piezoBuzzerPin)) { Serial.print(_piezoBuzzerPin); }
        else { Serial.print(SFP(HS_Disabled)); }
        Serial.print(F(", eepromDeviceSize: "));
        if (_eepromDeviceSize) { Serial.print(_eepromDeviceSize); }
        else { Serial.print(SFP(HS_Disabled)); }
        Serial.print(F(", sdCardCSPin: "));
        if (isValidPin(_sdCardCSPin)) { Serial.print(_sdCardCSPin); }
        else { Serial.print(SFP(HS_Disabled)); }
        Serial.print(F(", controlInputPin1: "));
        if (isValidPin(_ctrlInputPin1)) { Serial.print(_ctrlInputPin1); }
        else { Serial.print(SFP(HS_Disabled)); }
        Serial.print(F(", EEPROMi2cAddress: 0x"));
        Serial.print(_eepromI2CAddr, HEX);
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
    #endif
}

void Hydroponics::commonPostSave()
{
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
    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) {
        #if defined(HYDRUINO_USE_SCHEDULER)
            if (hydroponics->_suspend) { yield(); return; }
            {   static auto lastTimeCalled = millis();
                const int delayPeriod = HYDRUINO_CONTROL_LOOP_INTERVAL;
                int delayAmount = constrain(delayPeriod - (int)(millis() - lastTimeCalled), 0, delayPeriod);
                lastTimeCalled = millis();
                delay(delayAmount);
            }
            if (hydroponics->_suspend) { yield(); return; }
        #endif
        hydroponics->updateObjects(0);
        getSchedulerInstance()->update();
    }

    yield();
}

void dataLoop()
{
    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) {
        #if defined(HYDRUINO_USE_SCHEDULER)
            if (hydroponics->_suspend) { yield(); return; }
            {   static auto lastTimeCalled = millis();
                const int delayPeriod = hydroponics->getPollingInterval();
                int delayAmount = constrain(delayPeriod - (int)(millis() - lastTimeCalled), 0, delayPeriod);
                lastTimeCalled = millis();
                delay(delayAmount);
            }
            if (hydroponics->_suspend) { yield(); return; }
        #endif
        hydroponics->updateObjects(1);
        //hydroponics->_publisher.update();
    }

    yield();
}

void miscLoop()
{
    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) {
        #if defined(HYDRUINO_USE_SCHEDULER)
            if (hydroponics->_suspend) { yield(); return; }
            {   static auto lastTimeCalled = millis();
                const int delayPeriod = HYDRUINO_MISC_LOOP_INTERVAL;
                int delayAmount = constrain(delayPeriod - (int)(millis() - lastTimeCalled), 0, delayPeriod);
                lastTimeCalled = millis();
                delay(delayAmount);
            }
            if (hydroponics->_suspend) { yield(); return; }
        #endif
        hydroponics->checkFreeMemory();
    }

    yield();
}

void Hydroponics::launch()
{
    ++_pollingFrame; // Forces all sensors to get a new initial measurement

    // Ensures linkage (and reverse linkage) of unlinked objects
    _scheduler.resolveLinks();
    //_publisher.resolveLinks();
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        auto obj = iter->second;
        if (obj) { obj->resolveLinks(); }
    }

    // Create main runloops
    #if defined(HYDRUINO_USE_TASKSCHEDULER)
        if (!_controlTask) {
            _controlTask = new Task(HYDRUINO_CONTROL_LOOP_INTERVAL * TASK_MILLISECOND, TASK_FOREVER, controlLoop, &_ts, true);
            HYDRUINO_HARD_ASSERT(_controlTask, SFP(HS_Err_AllocationFailure));
        } else {
            _controlTask->enable();
        }
        if (!_dataTask) {
            _dataTask = new Task(getPollingInterval() * TASK_MILLISECOND, TASK_FOREVER, dataLoop, &_ts, true);
            HYDRUINO_HARD_ASSERT(_dataTask, SFP(HS_Err_AllocationFailure));
        } else {
            _dataTask->enable();
        }
        if (!_miscTask) {
            _miscTask = new Task(HYDRUINO_MISC_LOOP_INTERVAL * TASK_MILLISECOND, TASK_FOREVER, miscLoop, &_ts, true);
            HYDRUINO_HARD_ASSERT(_miscTask, SFP(HS_Err_AllocationFailure));
        } else {
            _miscTask->enable();
        }
    #elif defined(HYDRUINO_USE_SCHEDULER)
        if (!_loopsStarted) {
            Scheduler.startLoop(controlLoop);
            Scheduler.startLoop(dataLoop);
            Scheduler.startLoop(miscLoop);
            _loopsStarted = true;
        }
        _suspend = false;
    #endif
}

void Hydroponics::suspend()
{
    #if defined(HYDRUINO_USE_TASKSCHEDULER)
        if (_controlTask) {
            _controlTask->disable();
        }
        if (_dataTask) {
            _dataTask->disable();
        }
        if (_miscTask) {
            _miscTask->disable();
        }
    #elif defined(HYDRUINO_USE_SCHEDULER)
        _suspend = true;
    #endif
}

void Hydroponics::update()
{
    #ifndef HYDRUINO_DISABLE_MULTITASKING
        #ifdef HYDRUINO_USE_TASKSCHEDULER
            HYDRUINO_MAINLOOP(_ts);
        #else
            HYDRUINO_MAINLOOP();
        #endif
    #else
        controlLoop();
        dataLoop();
        miscLoop();
    #endif

    EasyBuzzer.update(); // EasyBuzzer does its own state updates efficiently

    taskManager.runLoop(); // tcMenu also uses this system to run its UI
}

void Hydroponics::updateObjects(int pass)
{
    switch(pass) {
        case 0: {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                auto obj = iter->second;
                if (obj) {
                    obj->update();
                }
            } 
        } break;

        case 1: {
            _pollingFrame++; if (_pollingFrame == 0) { _pollingFrame = 1; } // only valid frame #

            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                auto obj = iter->second;
                if (obj && obj->isSensorType()) {
                    auto sensorObj = static_pointer_cast<HydroponicsSensor>(obj);
                    if (sensorObj && sensorObj->getNeedsPolling()) {
                        sensorObj->takeMeasurement(); // no force if already current for this frame #
                    }
                }
            }
        } break;

        default: break;
    }
}

bool Hydroponics::registerObject(shared_ptr<HydroponicsObject> obj)
{
    HYDRUINO_SOFT_ASSERT(obj->getId().posIndex >= 0 && obj->getId().posIndex < HYDRUINO_POS_MAXSIZE, SFP(HS_Err_InvalidParameter));
    if(obj && _objects.insert(obj->getKey(), obj).second) {
        _scheduler.setNeedsScheduling();
        if (getInOperationalMode()) { obj->resolveLinks(); }
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
                if (id.keyStr == iter->second->getId().keyStr) {
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
                if (id.keyStr == iter->second->getId().keyStr) {
                    return iter->second;
                } else {
                    objectById_Col(id);
                }
            }
        }
    } else {
        auto iter = _objects.find(id.key);
        if (iter != _objects.end()) {
            if (id.keyStr == iter->second->getId().keyStr) {
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
    HYDRUINO_SOFT_ASSERT(false, F("Hashing collision")); // exhaustive search must be performed
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (id.keyStr == iter->second->getId().keyStr) {
            return iter->second;
        }
    }
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
    HYDRUINO_SOFT_ASSERT(customAdditiveData, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!customAdditiveData || (customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                                                 customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount), SFP(HS_Err_InvalidParameter));

    if (customAdditiveData && customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
        auto iter = _additives.find(customAdditiveData->reservoirType);
        bool retVal = false;

        if (iter == _additives.end()) {
            auto additiveData = new HydroponicsCustomAdditiveData();

            HYDRUINO_SOFT_ASSERT(additiveData, SFP(HS_Err_AllocationFailure));
            if (additiveData) {
                *additiveData = *customAdditiveData;
                retVal = _additives.insert(customAdditiveData->reservoirType, additiveData).second;
            }
        } else {
            *(iter->second) = *customAdditiveData;
            retVal = true;
        }

        if (retVal) {
            getSchedulerInstance()->setNeedsScheduling();
            return true;
        }
    }
    return false;
}

bool Hydroponics::dropCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData)
{
    HYDRUINO_SOFT_ASSERT(customAdditiveData, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!customAdditiveData || (customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                                                 customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount), SFP(HS_Err_InvalidParameter));

    if (customAdditiveData && customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount) {
        auto iter = _additives.find(customAdditiveData->reservoirType);
        bool retVal = false;

        if (iter != _additives.end()) {
            if (iter->second) { delete iter->second; }
            _additives.erase(iter);
            retVal = true;
        }

        if (retVal) {
            getSchedulerInstance()->setNeedsScheduling();
            return true;
        }
    }
    return false;
}

const HydroponicsCustomAdditiveData *Hydroponics::getCustomAdditiveData(Hydroponics_ReservoirType reservoirType) const
{
    HYDRUINO_SOFT_ASSERT(reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                         reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomAdditiveCount, SFP(HS_Err_InvalidParameter));

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
                gotLock = _pinLocks.insert(pin, (byte)true).second;
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
    HYDRUINO_SOFT_ASSERT(pinMap, SFP(HS_Err_InvalidParameter));
    const int ctrlInPinCount = getControlInputRibbonPinCount();
    HYDRUINO_SOFT_ASSERT(!pinMap || (ctrlInPinCount > 0), SFP(HS_Err_UnsupportedOperation));

    if (pinMap && ctrlInPinCount) {
        for (int ribbonPinIndex = 0; ribbonPinIndex < ctrlInPinCount; ++ribbonPinIndex) {
            _ctrlInputPinMap[ribbonPinIndex] = pinMap[ribbonPinIndex];
        }
    }
}

void Hydroponics::setSystemName(String systemName)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
    if (_systemData) {
        _systemData->_bumpRevIfNotAlreadyModded();
        strncpy(_systemData->systemName, systemName.c_str(), HYDRUINO_NAME_MAXSIZE);
        // TODO: notify GUI to update
    }
}

void Hydroponics::setTimeZoneOffset(int8_t timeZoneOffset)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
    if (_systemData) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->timeZoneOffset = timeZoneOffset;
        // TODO: notify GUI to update
        getSchedulerInstance()->setNeedsScheduling();
    }
}

void Hydroponics::setPollingInterval(uint32_t pollingInterval)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
    if (_systemData) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->pollingInterval = pollingInterval;
    }
    #ifdef HYDRUINO_USE_TASKSCHEDULER
        if (_dataTask) {
            _dataTask->setInterval(getPollingInterval() * TASK_MILLISECOND);
        }
    #endif
}

void Hydroponics::setWiFiConnection(String ssid, String password)
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
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
                setupRandomSeed();
                _systemData->wifiPasswordSeed = random(1, RANDOM_MAX);

                randomSeed(_systemData->wifiPasswordSeed);
                for (int charIndex = 0; charIndex < HYDRUINO_NAME_MAXSIZE; ++charIndex) {
                    _systemData->wifiPassword[charIndex] = (charIndex < password.length() ? password[charIndex] : '\0') ^ (byte)random(256);
                }
            } else {
                _systemData->wifiPasswordSeed = 0;
                memset(_systemData->wifiPassword, '\0', HYDRUINO_NAME_MAXSIZE);
            }

            if (_wifiBegan && (ssidChanged || passChanged)) { _wifi->disconnect(); _wifiBegan = false; } // forces re-connect on next getWifi
        }
    }
}

Hydroponics *Hydroponics::getActiveInstance()
{
    return _activeInstance;
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
    HYDRUINO_SOFT_ASSERT(ctrlInPinCount > 0, SFP(HS_Err_UnsupportedOperation));
    HYDRUINO_SOFT_ASSERT(ctrlInPinCount <= 0 || (ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount), SFP(HS_Err_InvalidParameter));

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
            _rtcBattFail = _rtc->lostPower();
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
            bool sdBegan = _sd->begin(_sdCardCSPin, *getSPI(), _sdCardSpeed);
        #else
            bool sdBegan = _sd->begin(_sdCardSpeed, _sdCardCSPin);
        #endif

        if (!sdBegan) { deallocateSD(); }

        return _sd && sdBegan ? _sd : nullptr;
    }

    return _sd;
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
        if (oneWire && _oneWires.insert(pin, oneWire).second) {
            return oneWire;
        } else if (oneWire) {
            delete oneWire;
        }
    }
    return nullptr;
}

void Hydroponics::dropOneWireForPin(byte pin)
{
    auto wireIter = _oneWires.find(pin);
    if (wireIter != _oneWires.end()) {
        if (wireIter->second) { delete wireIter->second; }
        _oneWires.erase(wireIter);
    }
}

bool Hydroponics::getInOperationalMode() const
{
    #if defined(HYDRUINO_USE_TASKSCHEDULER)
        return _controlTask && _controlTask->isEnabled();
    #elif defined(HYDRUINO_USE_SCHEDULER)
        return _loopsStarted && !_suspend;
    #endif
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
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
    return _systemData ? String(_systemData->systemName) : String();
}

int8_t Hydroponics::getTimeZoneOffset() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
    return _systemData ? _systemData->timeZoneOffset : 0;
}

bool Hydroponics::getRTCBatteryFailure() const
{
    return _rtcBattFail;
}

uint32_t Hydroponics::getPollingInterval() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
    return _systemData ? _systemData->pollingInterval : 0;
}

uint32_t Hydroponics::getPollingFrame() const
{
    return _pollingFrame;
}

bool Hydroponics::getIsPollingFrameOld(unsigned int frame, unsigned int allowance) const
{
    return _pollingFrame - frame > allowance;
}

String Hydroponics::getWiFiSSID()
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
    return _systemData ? String(_systemData->wifiSSID) : String();
}

String Hydroponics::getWiFiPassword()
{
    HYDRUINO_SOFT_ASSERT(_systemData, SFP(HS_Err_NotYetInitialized));
    if (_systemData) {
        char wifiPassword[HYDRUINO_NAME_MAXSIZE] = {0};

        if (_systemData->wifiPasswordSeed) {
            randomSeed(_systemData->wifiPasswordSeed);
            for (int charIndex = 0; charIndex <= HYDRUINO_NAME_MAXSIZE; ++charIndex) {
                wifiPassword[charIndex] = _systemData->wifiPassword[charIndex] ^ (byte)random(256);
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
    _scheduler.setNeedsScheduling();
}

void Hydroponics::handleInterrupt(pintype_t pin)
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second && iter->second->isSensorType()) {
            auto sensor = static_pointer_cast<HydroponicsSensor>(iter->second);
            if (sensor && sensor->getInputPin() == pin && sensor->isBinaryClass()) {
                auto binarySensor = static_pointer_cast<HydroponicsBinarySensor>(sensor);
                if (binarySensor) { binarySensor->notifyISRTriggered(); }
            }
        }
    }
}

void Hydroponics::checkFreeMemory()
{
    int memLeft = freeMemory();
    if (memLeft != -1 && memLeft < HYDRUINO_LOW_MEM_SIZE) {
        for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
            auto obj = iter->second;
            if (obj) {
                obj->handleLowMemory();
            }
        }
        _scheduler.handleLowMemory();
        //_publisher.handleLowMemory();
    }
}

#ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT

void Hydroponics::forwardLogMessage(String message, bool flushAfter = false)
{
    // TODO: Log message forwarding.
}

#endif // /ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
