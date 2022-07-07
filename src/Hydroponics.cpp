/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Main
*/

#include "Hydroponics.h"

static RTC_DS3231 *_rtcSyncProvider = nullptr;
time_t rtcNow() {
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}

Hydroponics *Hydroponics::_activeInstance = nullptr;

Hydroponics::Hydroponics(byte piezoBuzzerPin, uint32_t eepromDeviceSize, byte sdCardCSPin, byte controlInputPin1,
                         byte eepromI2CAddress, byte rtcI2CAddress, byte lcdI2CAddress,
                         TwoWire& i2cWire, uint32_t i2cSpeed, uint32_t spiSpeed)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed), _spiSpeed(spiSpeed),
      _piezoBuzzerPin(piezoBuzzerPin), _eepromDeviceSize(eepromDeviceSize), _sdCardCSPin(sdCardCSPin),
      _ctrlInputPin1(controlInputPin1), _ctrlInputPinMap{-1},
      _eepromI2CAddr(eepromI2CAddress), _rtcI2CAddr(rtcI2CAddress), _lcdI2CAddr(lcdI2CAddress),
      _eeprom(nullptr), _rtc(nullptr), _sd(nullptr), _eepromBegan(false), _rtcBegan(false), _rtcBattFail(false),
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
        HYDRUINO_SOFT_ASSERT(_eeprom, F("Failure allocating EEPROM device"));
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
        HYDRUINO_SOFT_ASSERT(_rtc, F("Failure allocating RTC device"));
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
            HYDRUINO_SOFT_ASSERT(_sd, F("Failure allocating SD card device"));
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
    HYDRUINO_HARD_ASSERT(!_systemData, F("Controller already initialized"));

    if (!_systemData) {
        HYDRUINO_SOFT_ASSERT((int)systemMode >= 0 && systemMode < Hydroponics_SystemMode_Count, F("Invalid system mode"));
        HYDRUINO_SOFT_ASSERT((int)measureMode >= 0 && measureMode < Hydroponics_MeasurementMode_Count, F("Invalid measurement mode"));
        HYDRUINO_SOFT_ASSERT((int)dispOutMode >= 0 && dispOutMode < Hydroponics_DisplayOutputMode_Count, F("Invalid LCD output mode"));
        HYDRUINO_SOFT_ASSERT((int)ctrlInMode >= 0 && ctrlInMode < Hydroponics_ControlInputMode_Count, F("Invalid control input mode"));

        _systemData = new HydroponicsSystemData();
        HYDRUINO_SOFT_ASSERT(_systemData, F("Failure allocating system data store"));

        if (_systemData) {
            _systemData->systemMode = systemMode;
            _systemData->measureMode = measureMode;
            _systemData->dispOutMode = dispOutMode;
            _systemData->ctrlInMode = ctrlInMode;

            commonInit();
        }
    }  
}

bool Hydroponics::initFromEEPROM(bool jsonFormat)
{
    HYDRUINO_HARD_ASSERT(!_systemData, F("Controller already initialized"));

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
    HYDRUINO_HARD_ASSERT(_systemData, F("Controller not initialized"));

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
    HYDRUINO_HARD_ASSERT(!_systemData, F("Controller already initialized"));

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
    HYDRUINO_HARD_ASSERT(_systemData, F("Controller not initialized"));

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
    HYDRUINO_HARD_ASSERT(!_systemData, F("Controller already initialized"));
    HYDRUINO_SOFT_ASSERT(streamIn && streamIn->available(), F("Invalid stream or no data present"));

    if (!_systemData && streamIn && streamIn->available()) {

        {   StaticJsonDocument<HYDRUINO_JSON_DOC_MAXSIZE> doc;
            deserializeJson(doc, *streamIn);
            JsonObjectConst systemDataObj = doc.as<JsonObjectConst>();
            HydroponicsSystemData *systemData = (HydroponicsSystemData *)newDataFromJSONObject(systemDataObj);

            HYDRUINO_SOFT_ASSERT(systemData && systemData->isSystemData(), F("Failure importing system data"));
            if (systemData && systemData->isSystemData()) {
                _systemData = systemData;
                _scheduler.initFromData(&(_systemData->scheduler));
                //_actQueue.initFromData(&(_systemData->actQueue));
                //_publisher.initFromData(&(_systemData->publisher));
            } else if (systemData) {
                delete systemData;
            }
        }

        if (_systemData) {
            while (streamIn->available()) {
                StaticJsonDocument<HYDRUINO_JSON_DOC_MAXSIZE> doc;
                deserializeJson(doc, *streamIn);
                JsonObjectConst dataObj = doc.as<JsonObjectConst>();
                HydroponicsData *data = newDataFromJSONObject(dataObj);

                HYDRUINO_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectData()), F("Failure importing data, unsupported type"));
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
                        HYDRUINO_SOFT_ASSERT(false, F("Failure creating object"));
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

        HYDRUINO_SOFT_ASSERT(_systemData, F("Failure initializing, no system data"));
        if (_systemData) {
            commonInit();
        }
        return _systemData;
    }

    return false;
}

bool Hydroponics::saveToJSONStream(Stream *streamOut, bool compact)
{
    HYDRUINO_HARD_ASSERT(_systemData, F("Controller not initialized"));
    HYDRUINO_SOFT_ASSERT(streamOut, F("Invalid stream"));

    if (_systemData && streamOut) {
        {   StaticJsonDocument<HYDRUINO_JSON_DOC_MAXSIZE> doc;

            JsonObject systemDataObj = doc.to<JsonObject>();
            _systemData->toJSONObject(systemDataObj);

            if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                HYDRUINO_SOFT_ASSERT(false, F("Failure exporting system data"));
                return false;
            }
        }

        if (getCalibrationsStoreInstance()->hasUserCalibrations()) {
            auto calibsStore = getCalibrationsStoreInstance();

            for (auto iter = calibsStore->_calibrationData.begin(); iter != calibsStore->_calibrationData.end(); ++iter) {
                StaticJsonDocument<HYDRUINO_JSON_DOC_MAXSIZE> doc;

                JsonObject calibDataObj = doc.to<JsonObject>();
                iter->second->toJSONObject(calibDataObj);

                if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                    HYDRUINO_SOFT_ASSERT(false, F("Failure exporting user calibration data"));
                    return false;
                }
            }
        }

        if (getCropsLibraryInstance()->hasCustomCrops()) {
            auto cropsLib = getCropsLibraryInstance();
            
            for (auto iter = cropsLib->_cropsData.begin(); iter != cropsLib->_cropsData.end(); ++iter) {
                if (iter->first >= Hydroponics_CropType_CustomCrop1) {
                    StaticJsonDocument<HYDRUINO_JSON_DOC_MAXSIZE> doc;

                    JsonObject cropDataObj = doc.to<JsonObject>();
                    iter->second->data.toJSONObject(cropDataObj);

                    if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                        HYDRUINO_SOFT_ASSERT(false, F("Failure exporting custom crop data"));
                        return false;
                    }
                }
            }
        }

        if (_additives.size()) {
            for (auto iter = _additives.begin(); iter != _additives.end(); ++iter) {
                StaticJsonDocument<HYDRUINO_JSON_DOC_MAXSIZE> doc;

                JsonObject additiveDataObj = doc.to<JsonObject>();
                iter->second->toJSONObject(additiveDataObj);

                if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                    HYDRUINO_SOFT_ASSERT(false, F("Failure exporting custom additive data"));
                    return false;
                }
            }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                HydroponicsData *data = iter->second->saveToData();

                HYDRUINO_SOFT_ASSERT(data && data->isObjectData(), F("Failure saving object to data"));
                if (data && data->isObjectData()) {
                    StaticJsonDocument<HYDRUINO_JSON_DOC_MAXSIZE> doc;

                    JsonObject objectDataObj = doc.to<JsonObject>();
                    data->toJSONObject(objectDataObj);
                    delete data; data = nullptr;

                    if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                        HYDRUINO_SOFT_ASSERT(false, F("Failure exporting object data"));
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
    HYDRUINO_HARD_ASSERT(!_systemData, F("Controller already initialized"));
    HYDRUINO_SOFT_ASSERT(streamIn && streamIn->available(), F("Invalid stream or no data present"));

    if (!_systemData && streamIn && streamIn->available()) {
        {   HydroponicsSystemData *systemData = (HydroponicsSystemData *)newDataFromBinaryStream(streamIn);

            HYDRUINO_SOFT_ASSERT(systemData && systemData->isSystemData(), F("Failure importing system data"));
            if (systemData && systemData->isSystemData()) {
                _systemData = systemData;
                _scheduler.initFromData(&(_systemData->scheduler));
                //_actQueue.initFromData(&(_systemData->actQueue));
                //_publisher.initFromData(&(_systemData->publisher));
            } else if (systemData) {
                delete systemData;
            }
        }

        if (_systemData) {
            while (streamIn->available()) {
                HydroponicsData *data = newDataFromBinaryStream(streamIn);

                HYDRUINO_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectData()), F("Failure importing data, unsupported type"));
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
                        HYDRUINO_SOFT_ASSERT(false, F("Failure creating object"));
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

        HYDRUINO_SOFT_ASSERT(_systemData, F("Failure initializing, no system data"));
        if (_systemData) {
            commonInit();
        }
        return _systemData;
    }

    return false;
}

bool Hydroponics::saveToBinaryStream(Stream *streamOut)
{
    HYDRUINO_HARD_ASSERT(_systemData, F("Controller not initialized"));
    HYDRUINO_SOFT_ASSERT(streamOut, F("Invalid stream"));

    if (_systemData && streamOut) {
        {   size_t bytesWritten = serializeDataToBinaryStream(_systemData, streamOut);

            HYDRUINO_SOFT_ASSERT(!bytesWritten, F("Failure exporting system data"));
            if (!bytesWritten) { return false; }
        }

        if (getCalibrationsStoreInstance()->hasUserCalibrations()) {
            auto calibsStore = getCalibrationsStoreInstance();
            size_t bytesWritten = 0;

            for (auto iter = calibsStore->_calibrationData.begin(); iter != calibsStore->_calibrationData.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            HYDRUINO_SOFT_ASSERT(bytesWritten, F("Failure exporting user calibration data"));
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

            HYDRUINO_SOFT_ASSERT(bytesWritten, F("Failure exporting custom crop data"));
            if (!bytesWritten) { return false; }
        }

        if (_additives.size()) {
            size_t bytesWritten = 0;

            for (auto iter = _additives.begin(); iter != _additives.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            HYDRUINO_SOFT_ASSERT(bytesWritten, F("Failure exporting custom additive data"));
            if (!bytesWritten) { return false; }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                HydroponicsData *data = iter->second->saveToData();

                HYDRUINO_SOFT_ASSERT(data && data->isObjectData(), F("Failure saving object to data"));
                if (data && data->isObjectData()) {
                    size_t bytesWritten = serializeDataToBinaryStream(data, streamOut);
                    delete data; data = nullptr;

                    HYDRUINO_SOFT_ASSERT(bytesWritten, F("Failure exporting object data"));
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

    // TODO: tcMenu setup

    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        Serial.print(F("Hydroponics::commonInit piezoBuzzerPin: "));
        if (isValidPin(_piezoBuzzerPin)) { Serial.print(_piezoBuzzerPin); }
        else { Serial.print(F("Disabled")); }
        Serial.print(F(", eepromDeviceSize: "));
        if (_eepromDeviceSize) { Serial.print(_eepromDeviceSize); }
        else { Serial.print(F("Disabled")); }
        Serial.print(F(", sdCardCSPin: "));
        if (isValidPin(_sdCardCSPin)) { Serial.print(_sdCardCSPin); }
        else { Serial.print(F("Disabled")); }
        Serial.print(F(", controlInputPin1: "));
        if (isValidPin(_ctrlInputPin1)) { Serial.print(_ctrlInputPin1); }
        else { Serial.print(F("Disabled")); }
        Serial.print(F(", EEPROMi2cAddress: 0x"));
        Serial.print(_eepromI2CAddr, HEX);
        Serial.print(F(", RTCi2cAddress: 0x"));
        Serial.print(_rtcI2CAddr, HEX);
        Serial.print(F(", LCDi2cAddress: 0x"));
        Serial.print(_lcdI2CAddr, HEX);
        Serial.print(F(", i2cSpeed: "));
        Serial.print(roundf(getI2CSpeed() / 1000.0f)); Serial.print(F("kHz"));
        Serial.print(F(", SPISpeed: "));
        Serial.print(roundf(getSPISpeed() / 1000000.0f)); Serial.print(F("MHz"));
        Serial.print(F(", systemMode: "));
        switch(getSystemMode()) {
            case Hydroponics_SystemMode_Recycling: Serial.print(F("Recycling")); break;
            case Hydroponics_SystemMode_DrainToWaste: Serial.print(F("DrainToWaste")); break;
            case Hydroponics_SystemMode_Count:
            case Hydroponics_SystemMode_Undefined:
                Serial.print(getSystemMode()); break;
        }
        Serial.print(F(", measureMode: "));
        switch (getMeasurementMode()) {
            case Hydroponics_MeasurementMode_Imperial: Serial.print(F("Imperial")); break;
            case Hydroponics_MeasurementMode_Metric: Serial.print(F("Metric")); break;
            case Hydroponics_MeasurementMode_Scientific: Serial.print(F("Scientific")); break;
            case Hydroponics_MeasurementMode_Count:
            case Hydroponics_MeasurementMode_Undefined:
                Serial.print(getMeasurementMode()); break;
        }
        Serial.print(F(", dispOutMode: "));
        switch (getDisplayOutputMode()) {
            case Hydroponics_DisplayOutputMode_Disabled: Serial.print(F("Disabled")); break;
            case Hydroponics_DisplayOutputMode_20x4LCD: Serial.print(F("20x4LCD")); break;
            case Hydroponics_DisplayOutputMode_20x4LCD_Swapped: Serial.print(F("20x4LCD <Swapped>")); break;
            case Hydroponics_DisplayOutputMode_16x2LCD: Serial.print(F("16x2LCD")); break;
            case Hydroponics_DisplayOutputMode_16x2LCD_Swapped: Serial.print(F("16x2LCD <Swapped>")); break;
            case Hydroponics_DisplayOutputMode_Count:
            case Hydroponics_DisplayOutputMode_Undefined:
                Serial.print(getDisplayOutputMode()); break;
        }
        Serial.print(F(", ctrlInMode: "));
        switch (getControlInputMode()) {
            case Hydroponics_ControlInputMode_Disabled: Serial.print(F("Disabled")); break;
            case Hydroponics_ControlInputMode_2x2Matrix: Serial.print(F("2x2Matrix")); break;
            case Hydroponics_ControlInputMode_4xButton: Serial.print(F("4xButton")); break;
            case Hydroponics_ControlInputMode_6xButton: Serial.print(F("6xButton")); break;
            case Hydroponics_ControlInputMode_RotaryEncoder: Serial.print(F("RotaryEncoder")); break;
            case Hydroponics_ControlInputMode_Count:
            case Hydroponics_ControlInputMode_Undefined:
                Serial.print(getControlInputMode()); break;
        }
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
        hydroponics->_scheduler.update();
        //hydroponics->_actQueue.update();
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
    //_actQueue.resolveLinks();
    //_publisher.resolveLinks();
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        auto obj = iter->second;
        if (obj) { obj->resolveLinks(); }
    }

    // Create main runloops
    #if defined(HYDRUINO_USE_TASKSCHEDULER)
        if (!_controlTask) {
            _controlTask = new Task(HYDRUINO_CONTROL_LOOP_INTERVAL * TASK_MILLISECOND, TASK_FOREVER, controlLoop, &_ts, true);
            HYDRUINO_HARD_ASSERT(_controlTask, F("Failure allocating control task loop"));
        } else {
            _controlTask->enable();
        }
        if (!_dataTask) {
            _dataTask = new Task(getPollingInterval() * TASK_MILLISECOND, TASK_FOREVER, dataLoop, &_ts, true);
            HYDRUINO_HARD_ASSERT(_dataTask, F("Failure allocating data task loop"));
        } else {
            _dataTask->enable();
        }
        if (!_miscTask) {
            _miscTask = new Task(HYDRUINO_MISC_LOOP_INTERVAL * TASK_MILLISECOND, TASK_FOREVER, miscLoop, &_ts, true);
            HYDRUINO_HARD_ASSERT(_miscTask, F("Failure allocating misc task loop"));
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

    taskManager.runLoop(); // tcMenu also uses this system to run its UI, which we also use to run our Signal fires
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
            _pollingFrame++;
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                auto obj = iter->second;
                if (obj && obj->isSensorType()) {
                    auto sensorObj = reinterpret_pointer_cast<HydroponicsSensor>(obj);
                    if (sensorObj && sensorObj->getNeedsPolling()) {
                        sensorObj->takeMeasurement(true);
                    }
                }
            }
        } break;

        default: break;
    }
}

bool Hydroponics::setCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData)
{
    HYDRUINO_SOFT_ASSERT(customAdditiveData, F("Invalid custom additive data"));
    HYDRUINO_SOFT_ASSERT(!customAdditiveData || (customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                         customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomCount), F("Invalid reservoir type"));

    if (customAdditiveData && customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomCount) {
        auto iter = _additives.find(customAdditiveData->reservoirType);
        bool retVal = false;

        if (iter == _additives.end()) {
            auto additiveData = new HydroponicsCustomAdditiveData();

            HYDRUINO_SOFT_ASSERT(additiveData, F("Failure allocating custom additive data"));
            if (additiveData) {
                *additiveData = *customAdditiveData;
                retVal = _additives.insert(customAdditiveData->reservoirType, additiveData).second;
            }
        } else {
            *(iter->second) = *customAdditiveData;
            retVal = true;
        }

        if (retVal) {
            //scheduleSignalFireOnce<Hydroponics_ReservoirType>(_additivesSignal, customAdditiveData->reservoirType);
            return true;
        }
    }
    return false;
}

bool Hydroponics::dropCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData)
{
    HYDRUINO_SOFT_ASSERT(customAdditiveData, F("Invalid custom additive data"));
    HYDRUINO_SOFT_ASSERT(!customAdditiveData || (customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                                                 customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomCount), F("Invalid reservoir type"));

    if (customAdditiveData && customAdditiveData->reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        customAdditiveData->reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomCount) {
        auto iter = _additives.find(customAdditiveData->reservoirType);
        bool retVal = false;

        if (iter != _additives.end()) {
            if (iter->second) { delete iter->second; }
            _additives.erase(iter);
            retVal = true;
        }

        if (retVal) {
            //scheduleSignalFireOnce<Hydroponics_ReservoirType>(_additivesSignal, customAdditiveData->reservoirType);
            return true;
        }
    }
    return false;
}

const HydroponicsCustomAdditiveData *Hydroponics::getCustomAdditiveData(Hydroponics_ReservoirType reservoirType) const
{
    HYDRUINO_SOFT_ASSERT(reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
                         reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomCount, F("Invalid reservoir type"));

    if (reservoirType >= Hydroponics_ReservoirType_CustomAdditive1 &&
        reservoirType < Hydroponics_ReservoirType_CustomAdditive1 + Hydroponics_ReservoirType_CustomCount) {
        auto iter = _additives.find(reservoirType);

        if (iter != _additives.end()) {
            return iter->second;
        }
    }
    return nullptr;
}

bool Hydroponics::registerObject(shared_ptr<HydroponicsObject> obj)
{
    HYDRUINO_SOFT_ASSERT(obj->getId().posIndex >= 0 && obj->getId().posIndex < HYDRUINO_POS_MAXSIZE, F("Invalid position index"));
    if(_objects.insert(obj->getKey(), obj).second) {
        _scheduler.setNeedsRescheduling();
        return true;
    }
    return false;
}

bool Hydroponics::unregisterObject(shared_ptr<HydroponicsObject> obj)
{
    auto iter = _objects.find(obj->getKey());
    if (iter != _objects.end()) {
        _objects.erase(iter);
        _scheduler.setNeedsRescheduling();
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

    return shared_ptr<HydroponicsObject>(nullptr);
}

shared_ptr<HydroponicsObject> Hydroponics::objectById_Col(const HydroponicsIdentity &id) const
{
    HYDRUINO_SOFT_ASSERT(false, F("Hashing collision"));
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

void Hydroponics::setSystemName(String systemName)
{
    HYDRUINO_SOFT_ASSERT(_systemData, F("System data not yet initialized"));
    if (_systemData) {
        _systemData->_bumpRevIfNotAlreadyModded();
        strncpy(&_systemData->systemName[0], systemName.c_str(), HYDRUINO_NAME_MAXSIZE);
        // TODO system name or just lcd update signal
    }
}

void Hydroponics::setTimeZoneOffset(int8_t timeZoneOffset)
{
    HYDRUINO_SOFT_ASSERT(_systemData, F("System data not yet initialized"));
    if (_systemData) {
        _systemData->_bumpRevIfNotAlreadyModded();
        _systemData->timeZoneOffset = timeZoneOffset;
        // TODO system TZ or just lcd update signal
    }
}

void Hydroponics::setPollingInterval(uint32_t pollingInterval)
{
    HYDRUINO_SOFT_ASSERT(_systemData, F("System data not yet initialized"));
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

void Hydroponics::setControlInputPinMap(byte *pinMap)
{
    HYDRUINO_SOFT_ASSERT(pinMap, F("Invalid pinMap"));
    const int ctrlInPinCount = getControlInputRibbonPinCount();
    HYDRUINO_SOFT_ASSERT(!pinMap || (ctrlInPinCount > 0), F("Control input pinmap not used in this mode"));

    if (pinMap && ctrlInPinCount) {
        for (int ribbonPinIndex = 0; ribbonPinIndex < ctrlInPinCount; ++ribbonPinIndex) {
            _ctrlInputPinMap[ribbonPinIndex] = pinMap[ribbonPinIndex];
        }
    }
}

Hydroponics *Hydroponics::getActiveInstance()
{
    return _activeInstance;
}

uint32_t Hydroponics::getI2CSpeed() const
{
    return _i2cSpeed;
}

uint32_t Hydroponics::getSPISpeed() const
{
    return _spiSpeed;
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
            bool sdBegan = _sd->begin(_sdCardCSPin, SPI, _spiSpeed);
        #else
            bool sdBegan = _sd->begin(_spiSpeed, _sdCardCSPin);
        #endif

        if (!sdBegan) { deallocateSD(); }

        return _sd && sdBegan ? _sd : nullptr;
    }

    return _sd;
}

bool Hydroponics::getIsInOperationalMode() const
{
    #if defined(HYDRUINO_USE_TASKSCHEDULER)
        return _controlTask && _controlTask->isEnabled();
    #elif defined(HYDRUINO_USE_SCHEDULER)
        return _loopsStarted && !_suspend;
    #endif
}

String Hydroponics::getSystemName() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, F("System data not yet initialized"));
    return _systemData ? String(_systemData->systemName) : String();
}

int8_t Hydroponics::getTimeZoneOffset() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, F("System data not yet initialized"));
    return _systemData ? _systemData->timeZoneOffset : 0;
}

bool Hydroponics::getRTCBatteryFailure() const
{
    return _rtcBattFail;
}

uint32_t Hydroponics::getPollingInterval() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, F("System data not yet initialized"));
    return _systemData ? _systemData->pollingInterval : 0;
}

uint32_t Hydroponics::getPollingFrame() const
{
    return _pollingFrame;
}

bool Hydroponics::getIsPollingFrameOld(uint32_t frame) const
{
    return _pollingFrame > frame || (frame == UINT32_MAX && _pollingFrame < UINT32_MAX);
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
    HYDRUINO_SOFT_ASSERT(ctrlInPinCount > 0, F("Control input pinmap not used in this mode"));
    HYDRUINO_SOFT_ASSERT(ctrlInPinCount <= 0 || (ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount), F("Ribbon pin index out of range"));

    return ctrlInPinCount && ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount ? _ctrlInputPinMap[ribbonPinIndex] : -1;
}

void Hydroponics::notifyRTCTimeUpdated()
{
    _rtcBattFail = false;
    _scheduler.setNeedsRescheduling();
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
        //_actQueue.handleLowMemory();
        //_publisher.handleLowMemory();
    }
}

#ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT

void Hydroponics::forwardLogMessage(String message, bool flushAfter = false)
{
    // TODO
}

#endif // /ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT


shared_ptr<HydroponicsRelayActuator> Hydroponics::addGrowLightsRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_GrowLights));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, F("Output pin is not digital"));

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsRelayActuator> actuator(new HydroponicsRelayActuator(
            Hydroponics_ActuatorType_GrowLights,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsPumpRelayActuator> Hydroponics::addWaterPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_WaterPump));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, F("Output pin is not digital"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsPumpRelayActuator> actuator(new HydroponicsPumpRelayActuator(
            Hydroponics_ActuatorType_WaterPump,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsRelayActuator> Hydroponics::addWaterHeaterRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_WaterHeater));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, F("Output pin is not digital"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsRelayActuator> actuator(new HydroponicsRelayActuator(
            Hydroponics_ActuatorType_WaterHeater,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsRelayActuator> Hydroponics::addWaterAeratorRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_WaterAerator));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, F("Output pin is not digital"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsRelayActuator> actuator(new HydroponicsRelayActuator(
            Hydroponics_ActuatorType_WaterAerator,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsRelayActuator> Hydroponics::addFanExhaustRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_FanExhaust));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, F("Output pin is not digital"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsRelayActuator> actuator(new HydroponicsRelayActuator(
            Hydroponics_ActuatorType_FanExhaust,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsPWMActuator> Hydroponics::addFanExhaustPWM(byte outputPin, byte outputBitRes)
{
    bool outputPinIsPWM = checkPinIsPWMOutput(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_FanExhaust));
    HYDRUINO_HARD_ASSERT(outputPinIsPWM, F("Output pin does not support PWM"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (outputPinIsPWM && positionIndex != -1) {
        shared_ptr<HydroponicsPWMActuator> actuator(new HydroponicsPWMActuator(
            Hydroponics_ActuatorType_FanExhaust,
            positionIndex,
            outputPin, outputBitRes
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsPumpRelayActuator> Hydroponics::addPeristalticPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_PeristalticPump));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, F("Output pin is not digital"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsPumpRelayActuator> actuator(new HydroponicsPumpRelayActuator(
            Hydroponics_ActuatorType_PeristalticPump,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsBinarySensor> Hydroponics::addLevelIndicator(byte inputPin)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterLevelIndicator));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, F("Input pin is not digital"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsBinarySensor> sensor(new HydroponicsBinarySensor(
            Hydroponics_SensorType_WaterLevelIndicator,
            positionIndex,
            inputPin
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addAnalogCO2Sensor(byte inputPin, byte inputBitRes = 8)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_AirCarbonDioxide));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, F("Input pin is not analog"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_AirCarbonDioxide,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addAnalogPhMeter(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_PotentialHydrogen));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, F("Input pin is not analog"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_PotentialHydrogen,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addAnalogTemperatureSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterTemperature));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, F("Input pin is not analog"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_WaterTemperature,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addAnalogTDSElectrode(byte inputPin, int ppmScale, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_TotalDissolvedSolids));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, F("Input pin is not analog"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_TotalDissolvedSolids,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) {
            if (ppmScale != 500) {
                HydroponicsCalibrationData userCalibData(sensor->getId());
                userCalibData.setFromScale(ppmScale / 500.0f);
                userCalibData.calibUnits = Hydroponics_UnitsType_Concentration_EC;
                sensor->setUserCalibrationData(&userCalibData);
            }
            return sensor;
        } else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addPWMPumpFlowSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterPumpFlowSensor));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, F("Input pin is not analog"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_WaterPumpFlowSensor,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addAnalogWaterHeightMeter(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterHeightMeter));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, F("Input pin is not analog"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_WaterHeightMeter,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addUltrasonicDistanceSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterHeightMeter));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, F("Input pin is not analog"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_WaterHeightMeter,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) {
            HydroponicsCalibrationData userCalibData(sensor->getId());
            userCalibData.multiplier = -1.0f; userCalibData.offset = 1.0f;
            userCalibData.calibUnits = Hydroponics_UnitsType_Raw_0_1;
            sensor->setUserCalibrationData(&userCalibData);

            return sensor;
        } else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsDHTTempHumiditySensor> Hydroponics::addDHTTempHumiditySensor(byte inputPin, byte dhtType)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_AirTempHumidity));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, F("Input pin is not digital"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsDHTTempHumiditySensor> sensor(new HydroponicsDHTTempHumiditySensor(
            positionIndex,
            inputPin,
            dhtType
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsDSTemperatureSensor> Hydroponics::addDSTemperatureSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterTemperature));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, F("Input pin is not digital"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsDSTemperatureSensor> sensor(new HydroponicsDSTemperatureSensor(
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsTMPMoistureSensor> Hydroponics::addTMPMoistureSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_SoilMoisture));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, F("Input pin is not digital"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (inputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsTMPMoistureSensor> sensor(new HydroponicsTMPMoistureSensor(
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsTimedCrop> Hydroponics::addTimerFedCrop(Hydroponics_CropType cropType,
                                                              Hydroponics_SubstrateType substrateType,
                                                              DateTime sowDate,
                                                              byte minsOn, byte minsOff)
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(cropType));
    HYDRUINO_SOFT_ASSERT((int)cropType >= 0 && cropType <= Hydroponics_CropType_Count, F("Invalid crop type"));
    HYDRUINO_SOFT_ASSERT((int)substrateType >= 0 && substrateType <= Hydroponics_SubstrateType_Count, F("Invalid substrate type"));
    HYDRUINO_SOFT_ASSERT(sowDate.unixtime(), F("Invalid sow date"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if ((int)cropType >= 0 && cropType < Hydroponics_CropType_Count && sowDate.unixtime() && positionIndex != -1) {
        shared_ptr<HydroponicsTimedCrop> crop(new HydroponicsTimedCrop(
            cropType,
            positionIndex,
            substrateType,
            sowDate,
            TimeSpan(0,0,minsOn,0),
            TimeSpan(0,0,minsOff,0)
        ));
        if (registerObject(crop)) { return crop; }
        else { crop = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsTimedCrop> Hydroponics::addTimerFedPerennialCrop(Hydroponics_CropType cropType,
                                                                       Hydroponics_SubstrateType substrateType,
                                                                       DateTime lastHarvestDate,
                                                                       byte minsOn, byte minsOff)
{
    HydroponicsCropsLibData cropData(cropType);
    time_t sowDate = lastHarvestDate.unixtime() - (cropData.totalGrowWeeks * SECS_PER_WEEK);
    auto crop = addTimerFedCrop(cropType, substrateType, DateTime((uint32_t)sowDate), minsOn, minsOff);
    return crop;
}

shared_ptr<HydroponicsAdaptiveCrop> Hydroponics::addAdaptiveFedCrop(Hydroponics_CropType cropType,
                                                                    Hydroponics_SubstrateType substrateType,
                                                                    DateTime sowDate)
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(cropType));
    HYDRUINO_SOFT_ASSERT((int)cropType >= 0 && cropType <= Hydroponics_CropType_Count, F("Invalid crop type"));
    HYDRUINO_SOFT_ASSERT((int)substrateType >= 0 && substrateType <= Hydroponics_SubstrateType_Count, F("Invalid substrate type"));
    HYDRUINO_SOFT_ASSERT(sowDate.unixtime(), F("Invalid sow date"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if ((int)cropType >= 0 && cropType < Hydroponics_CropType_Count && sowDate.unixtime() && positionIndex != -1) {
        shared_ptr<HydroponicsAdaptiveCrop> crop(new HydroponicsAdaptiveCrop(
            cropType,
            positionIndex,
            substrateType,
            sowDate
        ));
        if (registerObject(crop)) { return crop; }
        else { crop = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsAdaptiveCrop> Hydroponics::addAdaptiveFedPerennialCrop(Hydroponics_CropType cropType,
                                                                             Hydroponics_SubstrateType substrateType,
                                                                             DateTime lastHarvestDate)
{
    HydroponicsCropsLibData cropData(cropType);
    time_t sowDate = lastHarvestDate.unixtime() - (cropData.totalGrowWeeks * SECS_PER_WEEK);
    auto crop = addAdaptiveFedCrop(cropType, substrateType, DateTime((uint32_t)sowDate));
    return crop;
}

shared_ptr<HydroponicsFluidReservoir> Hydroponics::addFluidReservoir(Hydroponics_ReservoirType reservoirType, float maxVolume)
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(reservoirType));
    HYDRUINO_SOFT_ASSERT((int)reservoirType >= 0 && reservoirType <= Hydroponics_ReservoirType_Count, F("Invalid reservoir type"));
    HYDRUINO_SOFT_ASSERT(maxVolume > 0.0f, F("Invalid max volume"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if ((int)reservoirType >= 0 && reservoirType < Hydroponics_ReservoirType_Count && maxVolume > 0.0f && positionIndex != -1) {
        shared_ptr<HydroponicsFluidReservoir> reservoir(new HydroponicsFluidReservoir(
            reservoirType,
            positionIndex,
            maxVolume
        ));
        if (registerObject(reservoir)) { return reservoir; }
        else { reservoir = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsFeedReservoir> Hydroponics::addFeedWaterReservoir(float maxVolume, DateTime lastChangeDate, DateTime lastPruningDate)
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ReservoirType_FeedWater));
    HYDRUINO_SOFT_ASSERT(maxVolume > 0.0f, F("Invalid max volume"));
    HYDRUINO_SOFT_ASSERT(lastChangeDate.unixtime(), F("Invalid last water change date"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (maxVolume > 0.0f && positionIndex != -1) {
        shared_ptr<HydroponicsFeedReservoir> reservoir(new HydroponicsFeedReservoir(
            positionIndex,
            maxVolume,
            lastChangeDate,
            lastPruningDate
        ));
        if (registerObject(reservoir)) { return reservoir; }
        else { reservoir = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsInfiniteReservoir> Hydroponics::addDrainagePipe()
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ReservoirType_DrainageWater));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (positionIndex != -1) {
        shared_ptr<HydroponicsInfiniteReservoir> reservoir(new HydroponicsInfiniteReservoir(
            Hydroponics_ReservoirType_DrainageWater,
            positionIndex,
            false
        ));
        if (registerObject(reservoir)) { return reservoir; }
        else { reservoir = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsInfiniteReservoir> Hydroponics::addFreshWaterMain()
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ReservoirType_FreshWater));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if (positionIndex != -1) {
        shared_ptr<HydroponicsInfiniteReservoir> reservoir(new HydroponicsInfiniteReservoir(
            Hydroponics_ReservoirType_FreshWater,
            positionIndex,
            true
        ));
        if (registerObject(reservoir)) { return reservoir; }
        else { reservoir = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsSimpleRail> Hydroponics::addSimplePowerRail(Hydroponics_RailType railType, int maxActiveAtOnce)
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(railType));
    HYDRUINO_SOFT_ASSERT((int)railType >= 0 && railType <= Hydroponics_RailType_Count, F("Invalid rail type"));
    HYDRUINO_SOFT_ASSERT(maxActiveAtOnce > 0, F("Invalid max active at once"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if ((int)railType >= 0 && railType < Hydroponics_RailType_Count && maxActiveAtOnce > 0 && positionIndex != -1) {
        shared_ptr<HydroponicsSimpleRail> rail(new HydroponicsSimpleRail(
            railType,
            positionIndex,
            maxActiveAtOnce
        ));
        if (registerObject(rail)) { return rail; }
        else { rail = nullptr; }
    }

    return nullptr;
}

shared_ptr<HydroponicsRegulatedRail> Hydroponics::addRegulatedPowerRail(Hydroponics_RailType railType, float maxPower)
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(railType));
    HYDRUINO_SOFT_ASSERT((int)railType >= 0 && railType <= Hydroponics_RailType_Count, F("Invalid rail type"));
    HYDRUINO_SOFT_ASSERT(maxPower > 0, F("Invalid max power"));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, F("No more positions available"));

    if ((int)railType >= 0 && railType < Hydroponics_RailType_Count && maxPower > 0 && positionIndex != -1) {
        shared_ptr<HydroponicsRegulatedRail> rail(new HydroponicsRegulatedRail(
            railType,
            positionIndex,
            maxPower
        ));
        if (registerObject(rail)) { return rail; }
        else { rail = nullptr; }
    }

    return nullptr;
}
