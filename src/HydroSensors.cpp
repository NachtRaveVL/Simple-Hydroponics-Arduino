/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Sensorsare 
*/

#include "Hydruino.h"

HydroSensor *newSensorObjectFromData(const HydroSensorData *dataIn)
{
    if (dataIn && isValidType(dataIn->id.object.idType)) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(HStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (int8_t)HydroSensor::Binary:
                return new HydroBinarySensor((const HydroBinarySensorData *)dataIn);
            case (int8_t)HydroSensor::Analog:
                return new HydroAnalogSensor((const HydroAnalogSensorData *)dataIn);
            //case 2: // Digital (not instance-able)
            case (int8_t)HydroSensor::DHT1W:
                return new HydroDHTTempHumiditySensor((const HydroDHTTempHumiditySensorData *)dataIn);
            case (int8_t)HydroSensor::DS1W:
                return new HydroDSTemperatureSensor((const HydroDSTemperatureSensorData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}

Hydro_UnitsType defaultUnitsForSensor(Hydro_SensorType sensorType, uint8_t measureRow, Hydro_MeasurementMode measureMode)
{
    measureMode = (measureMode == Hydro_MeasurementMode_Undefined && getController() ? getController()->getMeasurementMode() : measureMode);

    switch (sensorType) {
        case Hydro_SensorType_AirCarbonDioxide:
            return Hydro_UnitsType_Concentration_PPM;
        case Hydro_SensorType_AirTempHumidity:
        case Hydro_SensorType_WaterTemperature:
            return defaultTemperatureUnits(measureMode);
        case Hydro_SensorType_PotentialHydrogen:
            return Hydro_UnitsType_Alkalinity_pH_14;
        case Hydro_SensorType_PowerLevel:
            return defaultPowerUnits(measureMode);
        case Hydro_SensorType_PumpFlow:
            return defaultFlowRateUnits(measureMode);
        case Hydro_SensorType_SoilMoisture:
            return Hydro_UnitsType_Concentration_EC_5;
        case Hydro_SensorType_TotalDissolvedSolids:
            return Hydro_UnitsType_Concentration_TDS;
        case Hydro_SensorType_WaterHeight:
        case Hydro_SensorType_WaterLevel:
            return defaultDistanceUnits(measureMode);
        default:
            return Hydro_UnitsType_Undefined;
    }
}

Hydro_UnitsCategory defaultCategoryForSensor(Hydro_SensorType sensorType, uint8_t measureRow)
{
    switch (sensorType) {
        case Hydro_SensorType_AirCarbonDioxide:
            return Hydro_UnitsCategory_Concentration;
        case Hydro_SensorType_PotentialHydrogen:
            return Hydro_UnitsCategory_Alkalinity;
        case Hydro_SensorType_PowerLevel:
            return Hydro_UnitsCategory_Power;
        case Hydro_SensorType_PumpFlow:
            return Hydro_UnitsCategory_LiqFlowRate;
        case Hydro_SensorType_SoilMoisture:
        case Hydro_SensorType_TotalDissolvedSolids:
            return Hydro_UnitsCategory_Concentration;
        case Hydro_SensorType_WaterTemperature:
            return Hydro_UnitsCategory_Temperature;
        case Hydro_SensorType_WaterHeight:
        case Hydro_SensorType_WaterLevel:
            return Hydro_UnitsCategory_LiqVolume;
        case Hydro_SensorType_AirTempHumidity:
            switch (measureRow) {
                case 0: return Hydro_UnitsCategory_Temperature;
                case 1: return Hydro_UnitsCategory_Percentile; // humidity
                case 2: return Hydro_UnitsCategory_Temperature; // heat index
                default: break;
            }
        default:
            return Hydro_UnitsCategory_Undefined;
    }
}


HydroSensor::HydroSensor(Hydro_SensorType sensorType, hposi_t sensorIndex, int classTypeIn)
    : HydroObject(HydroIdentity(sensorType, sensorIndex)), classType((typeof(classType))classTypeIn),
      _isTakingMeasure(false), _crop(this), _reservoir(this), _calibrationData(nullptr)
{
    _calibrationData = getController() ? getController()->getUserCalibrationData(_id.key) : nullptr;
}

HydroSensor::HydroSensor(const HydroSensorData *dataIn)
    : HydroObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _isTakingMeasure(false), _crop(this), _reservoir(this), _calibrationData(nullptr)
{
    _calibrationData = getController() ? getController()->getUserCalibrationData(_id.key) : nullptr;
    _crop.setObject(dataIn->cropName);
    _reservoir.setObject(dataIn->reservoirName);
}

HydroSensor::~HydroSensor()
{
    _isTakingMeasure = false;
}

void HydroSensor::update()
{
    HydroObject::update();

    _crop.resolve();
    _reservoir.resolve();
}

bool HydroSensor::isTakingMeasurement() const
{
    return _isTakingMeasure;
}

bool HydroSensor::getNeedsPolling(hframe_t allowance) const
{
    auto latestMeasurement = getLatestMeasurement();
    return getController() && latestMeasurement ? getController()->isPollingFrameOld(latestMeasurement->frame, allowance) : false;
}

HydroAttachment &HydroSensor::getParentCrop()
{
    return _crop;
}

HydroAttachment &HydroSensor::getParentReservoir()
{
    return _reservoir;
}

void HydroSensor::setUserCalibrationData(HydroCalibrationData *userCalibrationData)
{
    if (getController()) {
        if (userCalibrationData && getController()->setUserCalibrationData(userCalibrationData)) {
            _calibrationData = getController()->getUserCalibrationData(_id.key);
        } else if (!userCalibrationData && _calibrationData && getController()->dropUserCalibrationData(_calibrationData)) {
            _calibrationData = nullptr;
        }
    } else {
        _calibrationData = userCalibrationData;
    }
}

Signal<const HydroMeasurement *, HYDRO_SENSOR_SIGNAL_SLOTS> &HydroSensor::getMeasurementSignal()
{
    return _measureSignal;
}

HydroData *HydroSensor::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroSensor::saveToData(HydroData *dataOut)
{
    HydroObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    if (_reservoir.getId()) {
        strncpy(((HydroSensorData *)dataOut)->reservoirName, _reservoir.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    if (_crop.getId()) {
        strncpy(((HydroSensorData *)dataOut)->cropName, _crop.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
}


HydroBinarySensor::HydroBinarySensor(Hydro_SensorType sensorType, hposi_t sensorIndex, HydroDigitalPin inputPin, int classType)
    : HydroSensor(sensorType, sensorIndex, classType),
      _inputPin(inputPin), _usingISR(false)
{
    HYDRO_HARD_ASSERT(_inputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    _inputPin.init();
}

HydroBinarySensor::HydroBinarySensor(const HydroBinarySensorData *dataIn)
    : HydroSensor(dataIn), _inputPin(&dataIn->inputPin), _usingISR(false)
{
    HYDRO_HARD_ASSERT(_inputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    _inputPin.init();
    if (dataIn->usingISR) { tryRegisterAsISR(); }
}

HydroBinarySensor::~HydroBinarySensor()
{
    if (_usingISR) {
        // TODO: detach ISR from taskManger (not currently possible, maybe in future?)
    }
}

bool HydroBinarySensor::takeMeasurement(bool force)
{
    if (_inputPin.isValid() && (force || getNeedsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;
        bool stateBefore = _lastMeasurement.state;

        bool state = _inputPin.isActive();
        auto timestamp = unixNow();

        _lastMeasurement = HydroBinaryMeasurement(state, timestamp);
        _isTakingMeasure = false;

        #ifdef HYDRO_USE_MULTITASKING
            scheduleSignalFireOnce<const HydroMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);
        #else
            _measureSignal.fire(&_lastMeasurement);
        #endif

        if (state != stateBefore) {
            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<bool>(getSharedPtr(), _stateSignal, _lastMeasurement.state);
            #else
                _stateSignal.fire(_lastMeasurement.state);
            #endif
        }

        return true;
    }
    return false;
}

const HydroMeasurement *HydroBinarySensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroBinarySensor::setMeasureUnits(Hydro_UnitsType measureUnits, uint8_t measureRow)
{ ; }

Hydro_UnitsType HydroBinarySensor::getMeasureUnits(uint8_t measureRow) const
{
    return Hydro_UnitsType_Raw_1;
}

bool HydroBinarySensor::tryRegisterAsISR()
{
    #ifdef HYDRO_USE_MULTITASKING
        if (!_usingISR && checkPinCanInterrupt(_inputPin.pin)) {
            taskManager.addInterrupt(&interruptImpl, _inputPin.pin, CHANGE);
            _usingISR = true;
        }
    #endif
    return _usingISR;
}

Signal<bool, HYDRO_SENSOR_SIGNAL_SLOTS> &HydroBinarySensor::getStateSignal()
{
    return _stateSignal;
}

void HydroBinarySensor::saveToData(HydroData *dataOut)
{
    HydroSensor::saveToData(dataOut);

    _inputPin.saveToData(&((HydroBinarySensorData *)dataOut)->inputPin);
    ((HydroBinarySensorData *)dataOut)->usingISR = _usingISR;
}


HydroAnalogSensor::HydroAnalogSensor(Hydro_SensorType sensorType, hposi_t sensorIndex, HydroAnalogPin inputPin, bool inputInversion, int classType)
    : HydroSensor(sensorType, sensorIndex, classType),
      HydroMeasureUnitsStorage<1>(defaultUnitsForSensor(sensorType)),
      _inputPin(inputPin), _inputInversion(inputInversion)
{
    HYDRO_HARD_ASSERT(_inputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    _inputPin.init();
}

HydroAnalogSensor::HydroAnalogSensor(const HydroAnalogSensorData *dataIn)
    : HydroSensor(dataIn),
      HydroMeasureUnitsStorage<1>(definedUnitsElse(dataIn->measureUnits, defaultUnitsForSensor((Hydro_SensorType)(dataIn->id.object.objType)))),
      _inputPin(&dataIn->inputPin), _inputInversion(dataIn->inputInversion)
{
    HYDRO_HARD_ASSERT(_inputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    _inputPin.init();
}

bool HydroAnalogSensor::takeMeasurement(bool force)
{
    if (_inputPin.isValid() && (force || getNeedsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;

        #ifdef HYDRO_USE_MULTITASKING
            if (isValidTask(scheduleObjectMethodCallWithTaskIdOnce(::getSharedPtr<HydroAnalogSensor>(this), &HydroAnalogSensor::_takeMeasurement))) {
                return true;
            } else {
                HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
                _isTakingMeasure = false;
            }
        #else
            _takeMeasurement(0xffffU);
        #endif
    }
    return false;
}

void HydroAnalogSensor::_takeMeasurement(unsigned int taskId)
{
    if (_isTakingMeasure && _inputPin.isValid()) {
        if (getController()->tryGetPinLock(_inputPin.pin, 5)) {
            Hydro_UnitsType outUnits = definedUnitsElse(_measureUnits[0],
                                                        _calibrationData ? _calibrationData->calibrationUnits : Hydro_UnitsType_Undefined,
                                                        defaultUnitsForSensor(_id.objTypeAs.sensorType));

            unsigned int rawRead = 0;
            #if HYDRO_SENSOR_ANALOGREAD_SAMPLES > 1
                for (int sampleIndex = 0; sampleIndex < HYDRO_SENSOR_ANALOGREAD_SAMPLES; ++sampleIndex) {
                    #if HYDRO_SENSOR_ANALOGREAD_DELAY > 0
                        if (sampleIndex) { delay(HYDRO_SENSOR_ANALOGREAD_DELAY); }
                    #endif
                    rawRead += _inputPin.analogRead_raw();
                }
                rawRead /= HYDRO_SENSOR_ANALOGREAD_SAMPLES;
            #else
                rawRead = _inputPin.analogRead_raw();
            #endif // /if HYDRO_SENSOR_ANALOGREAD_SAMPLES > 1
            if (_inputInversion) { rawRead = _inputPin.bitRes.maxVal - rawRead; }
            auto timestamp = unixNow();

            HydroSingleMeasurement newMeasurement(
                _inputPin.bitRes.transform(rawRead),
                Hydro_UnitsType_Raw_1,
                timestamp
            );

            calibrationTransform(&newMeasurement);
            convertUnits(&newMeasurement, outUnits);

            _lastMeasurement = newMeasurement;
            getController()->returnPinLock(_inputPin.pin);
            _isTakingMeasure = false;

            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<const HydroMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);
            #else
                _measureSignal.fire(&_lastMeasurement);
            #endif
        } else {
            _isTakingMeasure = false;
        }
    }
}

const HydroMeasurement *HydroAnalogSensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroAnalogSensor::setMeasureUnits(Hydro_UnitsType measureUnits, uint8_t measureRow)
{
    if (_measureUnits[0] != measureUnits) {
        _measureUnits[0] = measureUnits;

        if (_lastMeasurement.frame) {
            convertUnits(&_lastMeasurement, _measureUnits[0]);
        }
    }
}

Hydro_UnitsType HydroAnalogSensor::getMeasureUnits(uint8_t measureRow) const
{
    return _measureUnits[0];
}

void HydroAnalogSensor::saveToData(HydroData *dataOut)
{
    HydroSensor::saveToData(dataOut);

    _inputPin.saveToData(&((HydroAnalogSensorData *)dataOut)->inputPin);
    ((HydroAnalogSensorData *)dataOut)->inputInversion = _inputInversion;
    ((HydroAnalogSensorData *)dataOut)->measureUnits = _measureUnits[0];
}


HydroDigitalSensor::HydroDigitalSensor(Hydro_SensorType sensorType, hposi_t sensorIndex, HydroDigitalPin inputPin, uint8_t bitRes1W, bool allocate1W, int classType)
    : HydroSensor(sensorType, sensorIndex, classType), _inputPin(inputPin), _oneWire(nullptr), _wireBitRes(bitRes1W), _wirePosIndex(-1), _wireDevAddress{0}
{
    HYDRO_HARD_ASSERT(_inputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    if (allocate1W && _inputPin.isValid()) {
        _oneWire = getController() ? getController()->getOneWireForPin(_inputPin.pin) : nullptr;
        HYDRO_SOFT_ASSERT(_oneWire, SFP(HStr_Err_AllocationFailure));
    }
}

HydroDigitalSensor::HydroDigitalSensor(const HydroDigitalSensorData *dataIn, bool allocate1W)
    : HydroSensor(dataIn), _inputPin(&dataIn->inputPin), _oneWire(nullptr), _wireBitRes(dataIn->wireBitRes), _wirePosIndex(-1), _wireDevAddress{0}
{
    HYDRO_HARD_ASSERT(_inputPin.isValid(), SFP(HStr_Err_InvalidPinOrType));
    if (allocate1W && _inputPin.isValid()) {
        _oneWire = getController() ? getController()->getOneWireForPin(_inputPin.pin) : nullptr;
        HYDRO_SOFT_ASSERT(_oneWire, SFP(HStr_Err_AllocationFailure));

        if (!arrayElementsEqual<uint8_t>(dataIn->wireDevAddress, 8, 0)) {
            _wirePosIndex = -1 - dataIn->wirePosIndex;
            memcpy(_wireDevAddress, dataIn->wireDevAddress, 8);
        } else {
            _wirePosIndex = -1 - dataIn->wirePosIndex;
        }
    }
}

bool HydroDigitalSensor::setWirePositionIndex(hposi_t wirePosIndex)
{
    wirePosIndex = constrain(wirePosIndex, 0, 62);
    if (_oneWire && wirePosIndex >= 0 && (_wirePosIndex != wirePosIndex || arrayElementsEqual<uint8_t>(_wireDevAddress, 8, 0)) &&
        getController()->tryGetPinLock(_inputPin.pin)) {
        hposi_t posIndex = 0;
        uint8_t devAddress[8];

        _oneWire->reset_search();
        while (posIndex <= wirePosIndex && _oneWire->search(devAddress)) {
            if (posIndex == wirePosIndex && _oneWire->crc8(devAddress, 7) == devAddress[7]) {
                _wirePosIndex = posIndex;
                memcpy(_wireDevAddress, devAddress, 8);

                getController()->returnPinLock(_inputPin.pin);
                return true;
            }
            posIndex++;
        }

        getController()->returnPinLock(_inputPin.pin);
    }
    return false;
}

hposi_t HydroDigitalSensor::getWirePositionIndex() const
{
    return _wirePosIndex >= 0 ? _wirePosIndex : (_wirePosIndex > -64 ? -_wirePosIndex - 1 : -_wirePosIndex - 64);
}

bool HydroDigitalSensor::setWireDeviceAddress(const uint8_t wireDevAddress[8])
{
    if (_oneWire && !arrayElementsEqual<uint8_t>(wireDevAddress, 8, 0) && (_wirePosIndex < 0 || memcmp(_wireDevAddress, wireDevAddress, 8) != 0) &&
        _oneWire->crc8(wireDevAddress, 7) == wireDevAddress[7] && getController()->tryGetPinLock(_inputPin.pin)) {
        hposi_t posIndex = 0;
        uint8_t devAddress[8];

        _oneWire->reset_search();
        while (_oneWire->search(devAddress)) {
            if (memcmp(devAddress, wireDevAddress, 8) == 0) {
                _wirePosIndex = posIndex;
                memcpy(_wireDevAddress, devAddress, 8);

                getController()->returnPinLock(_inputPin.pin);
                return true;
            }
            posIndex++;
        }

        getController()->returnPinLock(_inputPin.pin);
    }
    return false;
}

const uint8_t *HydroDigitalSensor::getWireDeviceAddress() const
{
    return _wireDevAddress;
}

void HydroDigitalSensor::resolveDeviceAddress()
{
    if (_oneWire && !(_wirePosIndex >= 0)) {
        setWireDeviceAddress(_wireDevAddress);

        if (!(_wirePosIndex >= 0) && _wirePosIndex > -64) {
            hposi_t posIndex = -_wirePosIndex - 1;
            setWirePositionIndex(posIndex);

            if (!(_wirePosIndex >= 0)) { _wirePosIndex = -64 - posIndex; } // disables further resolve attempts
        }
    }
}

void HydroDigitalSensor::saveToData(HydroData *dataOut)
{
    HydroSensor::saveToData(dataOut);

    _inputPin.saveToData(&((HydroDigitalSensorData *)dataOut)->inputPin);
    ((HydroDigitalSensorData *)dataOut)->wireBitRes = _wireBitRes;
    ((HydroDigitalSensorData *)dataOut)->wirePosIndex = getWirePositionIndex();
    memcpy(((HydroDigitalSensorData *)dataOut)->wireDevAddress, _wireDevAddress, 8);
}


HydroDHTTempHumiditySensor::HydroDHTTempHumiditySensor(hposi_t sensorIndex, HydroDigitalPin inputPin, Hydro_DHTType dhtType, bool computeHeatIndex, int classType)
    : HydroDigitalSensor(Hydro_SensorType_AirTempHumidity, sensorIndex, inputPin, 9, false, classType),
      HydroMeasureUnitsStorage<3>(defaultUnitsForSensor(Hydro_SensorType_AirTempHumidity)),
      _dht(new DHT(inputPin.pin, dhtType)), _dhtType(dhtType), _computeHeatIndex(computeHeatIndex)
{
    _measureUnits[1] = Hydro_UnitsType_Percentile_100;
    HYDRO_SOFT_ASSERT(_dht, SFP(HStr_Err_AllocationFailure));
    if (_inputPin.isValid() && _dht) { _dht->begin(); }
    else if (_dht) { delete _dht; _dht = nullptr; }
}

HydroDHTTempHumiditySensor::HydroDHTTempHumiditySensor(const HydroDHTTempHumiditySensorData *dataIn)
    : HydroDigitalSensor(dataIn, false),
      HydroMeasureUnitsStorage<3>(definedUnitsElse(dataIn->measureUnits, defaultUnitsForSensor(Hydro_SensorType_AirTempHumidity))),
      _dht(new DHT(dataIn->inputPin.pin, dataIn->dhtType)), _dhtType(dataIn->dhtType), _computeHeatIndex(dataIn->computeHeatIndex)
{
    _measureUnits[1] = Hydro_UnitsType_Percentile_100;
    HYDRO_SOFT_ASSERT(_dht, SFP(HStr_Err_AllocationFailure));
    if (_inputPin.isValid() && _dht) { _dht->begin(); }
    else if (_dht) { delete _dht; _dht = nullptr; }
}

HydroDHTTempHumiditySensor::~HydroDHTTempHumiditySensor()
{
    if (_dht) { delete _dht; _dht = nullptr; }
}

bool HydroDHTTempHumiditySensor::takeMeasurement(bool force)
{
    if (getController() && _dht && (force || getNeedsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;

        #ifdef HYDRO_USE_MULTITASKING
            if (isValidTask(scheduleObjectMethodCallWithTaskIdOnce(::getSharedPtr<HydroDHTTempHumiditySensor>(this), &HydroDHTTempHumiditySensor::_takeMeasurement))) {
                return true;
            } else {
                HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
                _isTakingMeasure = false;
            }
        #else
            _takeMeasurement(0xffffU);
        #endif
    }
    return false;
}

void HydroDHTTempHumiditySensor::_takeMeasurement(unsigned int taskId)
{
    if (_isTakingMeasure && _dht) {
        if (getController()->tryGetPinLock(_inputPin.pin, 5)) {
            Hydro_UnitsType outUnits[3] = { definedUnitsElse(_measureUnits[0],
                                                             _calibrationData ? _calibrationData->calibrationUnits : Hydro_UnitsType_Undefined,
                                                             defaultTemperatureUnits()),
                                            definedUnitsElse(_measureUnits[1],
                                                             Hydro_UnitsType_Percentile_100),
                                            definedUnitsElse(_measureUnits[2],
                                                             _calibrationData ? _calibrationData->calibrationUnits : Hydro_UnitsType_Undefined,
                                                             defaultTemperatureUnits()) };
            bool readInFahrenheit = outUnits[0] == Hydro_UnitsType_Temperature_Fahrenheit;
            Hydro_UnitsType readUnits = readInFahrenheit ? Hydro_UnitsType_Temperature_Fahrenheit
                                                         : Hydro_UnitsType_Temperature_Celsius;

            auto tempRead = _dht->readTemperature(readInFahrenheit, true);
            auto humidRead = _dht->readHumidity(true);
            auto timestamp = unixNow();

            HydroTripleMeasurement newMeasurement(
                tempRead, readUnits,
                humidRead, Hydro_UnitsType_Percentile_100,
                0.0f, Hydro_UnitsType_Undefined,
                timestamp
            );

            calibrationTransform(&newMeasurement.value[0], &newMeasurement.units[0]);
            convertUnits(&newMeasurement.value[0], &newMeasurement.units[0], outUnits[0]);
            convertUnits(&newMeasurement.value[1], &newMeasurement.units[1], outUnits[1]);

            if (_computeHeatIndex) {
                convertUnits(newMeasurement.value[0], &newMeasurement.value[2], newMeasurement.units[0], readUnits, &newMeasurement.units[2]);
                newMeasurement.value[2] = _dht->computeHeatIndex(newMeasurement.value[2], humidRead, readInFahrenheit);
                convertUnits(&newMeasurement.value[2], &newMeasurement.units[2], outUnits[2]);
            }

            _lastMeasurement = newMeasurement;
            getController()->returnPinLock(_inputPin.pin);
            _isTakingMeasure = false;

            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<const HydroMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);
            #else
                _measureSignal.fire(&_lastMeasurement);
            #endif
        } else {
            _isTakingMeasure = false;
        }
    }
}

const HydroMeasurement *HydroDHTTempHumiditySensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroDHTTempHumiditySensor::setMeasureUnits(Hydro_UnitsType measureUnits, uint8_t measureRow)
{
    if (_measureUnits[measureRow] != measureUnits) {
        _measureUnits[measureRow] = measureUnits;

        if (_lastMeasurement.frame) {
            convertUnits(&_lastMeasurement.value[measureRow], &_lastMeasurement.units[measureRow], _measureUnits[measureRow]);
        }
    }
}

Hydro_UnitsType HydroDHTTempHumiditySensor::getMeasureUnits(uint8_t measureRow) const
{
    return _measureUnits[measureRow];
}

bool HydroDHTTempHumiditySensor::setWirePositionIndex(hposi_t wirePosIndex)
{
    return false;
}

hposi_t HydroDHTTempHumiditySensor::getWirePositionIndex() const
{
    return -1;
}

bool HydroDHTTempHumiditySensor::setWireDeviceAddress(const uint8_t wireDevAddress[8])
{
    return false;
}

const uint8_t *HydroDHTTempHumiditySensor::getWireDeviceAddress() const
{
    return nullptr;
}

void HydroDHTTempHumiditySensor::setComputeHeatIndex(bool computeHeatIndex)
{
    if (_computeHeatIndex != computeHeatIndex) {
        _computeHeatIndex = computeHeatIndex;
    }
}

void HydroDHTTempHumiditySensor::saveToData(HydroData *dataOut)
{
    HydroDigitalSensor::saveToData(dataOut);

    ((HydroDHTTempHumiditySensorData *)dataOut)->dhtType = _dhtType;
    ((HydroDHTTempHumiditySensorData *)dataOut)->computeHeatIndex = _computeHeatIndex;
    ((HydroDHTTempHumiditySensorData *)dataOut)->measureUnits = _measureUnits[0];
}


HydroDSTemperatureSensor::HydroDSTemperatureSensor(hposi_t sensorIndex, HydroDigitalPin inputPin, uint8_t bitRes1W, HydroDigitalPin pullupPin, int classType)
    : HydroDigitalSensor(Hydro_SensorType_WaterTemperature, sensorIndex, inputPin, bitRes1W, true, classType),
      HydroMeasureUnitsStorage<1>(defaultUnitsForSensor(Hydro_SensorType_WaterTemperature)),
      _dt(new DallasTemperature()), _pullupPin(pullupPin)
{
    HYDRO_SOFT_ASSERT(_dt, SFP(HStr_Err_AllocationFailure));

    if (_inputPin.isValid() && _oneWire && _dt) {
        _dt->setOneWire(_oneWire);
        if (_pullupPin.isValid()) { _dt->setPullupPin(_pullupPin.pin); }
        _dt->setWaitForConversion(true); // reads will be done in their own task, waits will delay and yield
        _dt->begin();
        if (_dt->getResolution() != _wireBitRes) { _dt->setResolution(_wireBitRes); }
        HYDRO_SOFT_ASSERT(_dt->getResolution() == _wireBitRes, SFP(HStr_Err_OperationFailure));
    } else if (_dt) { delete _dt; _dt = nullptr; }
}

HydroDSTemperatureSensor::HydroDSTemperatureSensor(const HydroDSTemperatureSensorData *dataIn)
    : HydroDigitalSensor(dataIn, true),
      HydroMeasureUnitsStorage<1>(definedUnitsElse(dataIn->measureUnits, defaultUnitsForSensor(Hydro_SensorType_WaterTemperature))),
      _dt(new DallasTemperature()), _pullupPin(&dataIn->pullupPin)
{
    HYDRO_SOFT_ASSERT(_dt, SFP(HStr_Err_AllocationFailure));

    if (_inputPin.isValid() && _oneWire && _dt) {
        _dt->setOneWire(_oneWire);
        if (_pullupPin.isValid()) { _dt->setPullupPin(_pullupPin.pin); }
        _dt->setWaitForConversion(true); // reads will be done in their own task, waits will delay and yield
        _dt->begin();
        if (_dt->getResolution() != _wireBitRes) { _dt->setResolution(_wireBitRes); }
        HYDRO_SOFT_ASSERT(_dt->getResolution() == _wireBitRes, SFP(HStr_Err_OperationFailure));
    } else if (_dt) { delete _dt; _dt = nullptr; }
}

HydroDSTemperatureSensor::~HydroDSTemperatureSensor()
{
    if (_dt) { delete _dt; _dt = nullptr; }
}

bool HydroDSTemperatureSensor::takeMeasurement(bool force)
{
    if (!(_wirePosIndex >= 0)) { resolveDeviceAddress(); }

    if (_dt && _wirePosIndex >= 0 && (force || getNeedsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;

        #ifdef HYDRO_USE_MULTITASKING
            if (isValidTask(scheduleObjectMethodCallWithTaskIdOnce(::getSharedPtr<HydroDSTemperatureSensor>(this), &HydroDSTemperatureSensor::_takeMeasurement))) {
                return true;
            } else {
                HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_OperationFailure));
                _isTakingMeasure = false;
            }
        #else
            _takeMeasurement(0xffffU);
        #endif
    }
    return false;
}

void HydroDSTemperatureSensor::_takeMeasurement(unsigned int taskId)
{
    if (_isTakingMeasure && _dt) {
        if (getController()->tryGetPinLock(_inputPin.pin, 5)) {
            if (_dt->requestTemperaturesByAddress(_wireDevAddress)) {
                Hydro_UnitsType outUnits = definedUnitsElse(_measureUnits[0],
                                                            _calibrationData ? _calibrationData->calibrationUnits : Hydro_UnitsType_Undefined,
                                                            defaultTemperatureUnits());
                bool readInFahrenheit = _measureUnits[0] == Hydro_UnitsType_Temperature_Fahrenheit;
                Hydro_UnitsType readUnits = readInFahrenheit ? Hydro_UnitsType_Temperature_Fahrenheit
                                                             : Hydro_UnitsType_Temperature_Celsius;

                auto tempRead = readInFahrenheit ? _dt->getTempF(_wireDevAddress) : _dt->getTempC(_wireDevAddress);
                auto timestamp = unixNow();

                HydroSingleMeasurement newMeasurement(
                    tempRead,
                    readUnits,
                    timestamp
                );

                bool deviceDisconnected = isFPEqual(tempRead, (float)(readInFahrenheit ? DEVICE_DISCONNECTED_F : DEVICE_DISCONNECTED_C));
                HYDRO_SOFT_ASSERT(!deviceDisconnected, SFP(HStr_Err_MeasurementFailure)); // device disconnected

                if (!deviceDisconnected) {
                    calibrationTransform(&newMeasurement);
                    convertUnits(&newMeasurement, outUnits);

                    _lastMeasurement = newMeasurement;
                    getController()->returnPinLock(_inputPin.pin);
                    _isTakingMeasure = false;

                    #ifdef HYDRO_USE_MULTITASKING
                        scheduleSignalFireOnce<const HydroMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);
                    #else
                        _measureSignal.fire(&_lastMeasurement);
                    #endif
                }
            } else {
                getController()->returnPinLock(_inputPin.pin);
                _isTakingMeasure = false;
                HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_MeasurementFailure)); // device disconnected or no device by that addr
            }
        } else {
            _isTakingMeasure = false;
        }
    }
}

const HydroMeasurement *HydroDSTemperatureSensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroDSTemperatureSensor::setMeasureUnits(Hydro_UnitsType measureUnits, uint8_t measureRow)
{
    if (_measureUnits[0] != measureUnits) {
        _measureUnits[0] = measureUnits;

        if (_lastMeasurement.frame) {
            convertUnits(&_lastMeasurement, _measureUnits[0]);
        }
    }
}

Hydro_UnitsType HydroDSTemperatureSensor::getMeasureUnits(uint8_t measureRow) const
{
    return _measureUnits[0];
}

void HydroDSTemperatureSensor::saveToData(HydroData *dataOut)
{
    HydroDigitalSensor::saveToData(dataOut);

    _pullupPin.saveToData(&((HydroDSTemperatureSensorData *)dataOut)->pullupPin);
    ((HydroDSTemperatureSensorData *)dataOut)->measureUnits = _measureUnits[0];
}


HydroSensorData::HydroSensorData()
    : HydroObjectData(), inputPin(), cropName{0}, reservoirName{0}
{
    _size = sizeof(*this);
}

void HydroSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroObjectData::toJSONObject(objectOut);

    if (isValidPin(inputPin.pin)) {
        JsonObject inputPinObj = objectOut.createNestedObject(SFP(HStr_Key_InputPin));
        inputPin.toJSONObject(inputPinObj);
    }
    if (cropName[0]) { objectOut[SFP(HStr_Key_CropName)] = charsToString(cropName, HYDRO_NAME_MAXSIZE); }
    if (reservoirName[0]) { objectOut[SFP(HStr_Key_ReservoirName)] = charsToString(reservoirName, HYDRO_NAME_MAXSIZE); }
}

void HydroSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroObjectData::fromJSONObject(objectIn);

    JsonObjectConst inputPinObj = objectIn[SFP(HStr_Key_InputPin)];
    if (!inputPinObj.isNull()) { inputPin.fromJSONObject(inputPinObj); }
    const char *cropStr = objectIn[SFP(HStr_Key_CropName)];
    if (cropStr && cropStr[0]) { strncpy(cropName, cropStr, HYDRO_NAME_MAXSIZE); }
    const char *reservoirNameStr = objectIn[SFP(HStr_Key_ReservoirName)];
    if (reservoirNameStr && reservoirNameStr[0]) { strncpy(reservoirName, reservoirNameStr, HYDRO_NAME_MAXSIZE); }
}

HydroBinarySensorData::HydroBinarySensorData()
    : HydroSensorData(), usingISR(false)
{
    _size = sizeof(*this);
}

void HydroBinarySensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroSensorData::toJSONObject(objectOut);

    if (usingISR != false) { objectOut[SFP(HStr_Key_UsingISR)] = usingISR; }
}

void HydroBinarySensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroSensorData::fromJSONObject(objectIn);

    usingISR = objectIn[SFP(HStr_Key_UsingISR)] | usingISR;
}

HydroAnalogSensorData::HydroAnalogSensorData()
    : HydroSensorData(), inputInversion(false), measureUnits(Hydro_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroAnalogSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroSensorData::toJSONObject(objectOut);

    if (inputInversion != false) { objectOut[SFP(HStr_Key_InputInversion)] = inputInversion; }
    if (measureUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_MeasureUnits)] = unitsTypeToSymbol(measureUnits); }
}

void HydroAnalogSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroSensorData::fromJSONObject(objectIn);

    inputInversion = objectIn[SFP(HStr_Key_InputInversion)] | inputInversion;
    measureUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_MeasureUnits)]);
}

HydroDigitalSensorData::HydroDigitalSensorData()
    : HydroSensorData(), wireBitRes(9), wirePosIndex(-1), wireDevAddress{0}
{
    _size = sizeof(*this);
}

void HydroDigitalSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroSensorData::toJSONObject(objectOut);

    if (wireBitRes != 9) { objectOut[SFP(HStr_Key_BitRes)] = wireBitRes; }
    if (wirePosIndex > 0) { objectOut[SFP(HStr_Key_WirePosIndex)] = wirePosIndex; }
    if (!arrayElementsEqual<uint8_t>(wireDevAddress, 8, 0)) { objectOut[SFP(HStr_Key_WireDevAddress)] = hexStringFromBytes(wireDevAddress, 8); }
}

void HydroDigitalSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroSensorData::fromJSONObject(objectIn);

    wireBitRes = objectIn[SFP(HStr_Key_BitRes)] | wireBitRes;
    wirePosIndex = objectIn[SFP(HStr_Key_WirePosIndex)] | wirePosIndex;
    JsonVariantConst wireDevAddressVar = objectIn[SFP(HStr_Key_WireDevAddress)];
    hexStringToBytes(wireDevAddressVar, wireDevAddress, 8);
    for (int addrIndex = 0; addrIndex < 8; ++addrIndex) { 
        wireDevAddress[addrIndex] = wireDevAddressVar[addrIndex] | wireDevAddress[addrIndex];
    }
}

HydroDHTTempHumiditySensorData::HydroDHTTempHumiditySensorData()
    : HydroDigitalSensorData(), dhtType(Hydro_DHTType_None), computeHeatIndex(false), measureUnits(Hydro_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroDHTTempHumiditySensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroDigitalSensorData::toJSONObject(objectOut);

    if (dhtType != Hydro_DHTType_None) { objectOut[SFP(HStr_Key_DHTType)] = dhtType; }
    if (computeHeatIndex != false) { objectOut[SFP(HStr_Key_ComputeHeatIndex)] = computeHeatIndex; }
    if (measureUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_MeasureUnits)] = unitsTypeToSymbol(measureUnits); }
}

void HydroDHTTempHumiditySensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroDigitalSensorData::fromJSONObject(objectIn);

    dhtType = objectIn[SFP(HStr_Key_DHTType)] | dhtType;
    computeHeatIndex = objectIn[SFP(HStr_Key_ComputeHeatIndex)] | computeHeatIndex;
    measureUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_MeasureUnits)]);
}

HydroDSTemperatureSensorData::HydroDSTemperatureSensorData()
    : HydroDigitalSensorData(), pullupPin(), measureUnits(Hydro_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroDSTemperatureSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroDigitalSensorData::toJSONObject(objectOut);

    if (isValidPin(pullupPin.pin)) {
        JsonObject pullupPinObj = objectOut.createNestedObject(SFP(HStr_Key_PullupPin));
        pullupPin.toJSONObject(pullupPinObj);
    }
    if (measureUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_MeasureUnits)] = unitsTypeToSymbol(measureUnits); }
}

void HydroDSTemperatureSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroDigitalSensorData::fromJSONObject(objectIn);

    JsonObjectConst pullupPinObj = objectIn[SFP(HStr_Key_PullupPin)];
    if (!pullupPinObj.isNull()) { pullupPin.fromJSONObject(pullupPinObj); }
    measureUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_MeasureUnits)]);
}
