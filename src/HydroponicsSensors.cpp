/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensorsare 
*/

#include "Hydroponics.h"

HydroponicsSensor *newSensorObjectFromData(const HydroponicsSensorData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectectData(), SFP(HS_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectectData()) {
        switch (dataIn->id.object.classType) {
            case 0: // Binary
                return new HydroponicsBinarySensor((const HydroponicsBinarySensorData *)dataIn);
            case 1: // Analog
                return new HydroponicsAnalogSensor((const HydroponicsAnalogSensorData *)dataIn);
            //case 2: // Digital (not instance-able)
            case 3: // DHT1W
                return new HydroponicsDHTTempHumiditySensor((const HydroponicsDHTTempHumiditySensorData *)dataIn);
            case 4: // DS1W
                return new HydroponicsDSTemperatureSensor((const HydroponicsDSTemperatureSensorData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}

Hydroponics_UnitsType defaultMeasureUnitsForSensorType(Hydroponics_SensorType sensorType, Hydroponics_PositionIndex measurementRow, Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (sensorType) {
        case Hydroponics_SensorType_PotentialHydrogen:
            return Hydroponics_UnitsType_Alkalinity_pH_0_14;
        case Hydroponics_SensorType_TotalDissolvedSolids:
        case Hydroponics_SensorType_SoilMoisture:
            return Hydroponics_UnitsType_Concentration_EC;
        case Hydroponics_SensorType_AirTempHumidity:
        case Hydroponics_SensorType_WaterTemperature:
            return defaultTemperatureUnits(measureMode);
        case Hydroponics_SensorType_AirCarbonDioxide:
            return Hydroponics_UnitsType_Concentration_PPM;
        case Hydroponics_SensorType_WaterPumpFlowSensor:
            return defaultLiquidFlowUnits(measureMode);
        case Hydroponics_SensorType_WaterLevelIndicator:
        case Hydroponics_SensorType_WaterHeightMeter:
            return defaultDistanceUnits(measureMode);
        case Hydroponics_SensorType_PowerUsageMeter:
            return Hydroponics_UnitsType_Power_Wattage;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}

Hydroponics_UnitsCategory defaultMeasureCategoryForSensorType(Hydroponics_SensorType sensorType, Hydroponics_PositionIndex measurementRow)
{
    switch (sensorType) {
        case Hydroponics_SensorType_PotentialHydrogen:
            return Hydroponics_UnitsCategory_Alkalinity;
        case Hydroponics_SensorType_TotalDissolvedSolids:
            return Hydroponics_UnitsCategory_DissolvedSolids;
        case Hydroponics_SensorType_SoilMoisture:
            return Hydroponics_UnitsCategory_SoilMoisture;
        case Hydroponics_SensorType_WaterTemperature:
            return Hydroponics_UnitsCategory_LiqTemperature;
        case Hydroponics_SensorType_WaterPumpFlowSensor:
            return Hydroponics_UnitsCategory_LiqFlowRate;
        case Hydroponics_SensorType_WaterLevelIndicator:
        case Hydroponics_SensorType_WaterHeightMeter:
            return Hydroponics_UnitsCategory_LiqVolume;
        case Hydroponics_SensorType_AirTempHumidity:
            switch (measurementRow) {
                case 0: return Hydroponics_UnitsCategory_AirTemperature;
                case 1: return Hydroponics_UnitsCategory_AirHumidity;
                case 2: return Hydroponics_UnitsCategory_AirHeatIndex;
                default: break;
            }
        case Hydroponics_SensorType_AirCarbonDioxide:
            return Hydroponics_UnitsCategory_AirConcentration;
        case Hydroponics_SensorType_PowerUsageMeter:
            return Hydroponics_UnitsCategory_Power;
        default: break;
    }
    return Hydroponics_UnitsCategory_Undefined;
}


HydroponicsSensor::HydroponicsSensor(Hydroponics_SensorType sensorType,
                                     Hydroponics_PositionIndex sensorIndex,
                                     byte inputPin,
                                     int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(sensorType, sensorIndex)), classType((typeof(classType))classTypeIn),
      _inputPin(inputPin), _isTakingMeasure(false), _crop(this), _reservoir(this), _calibrationData(nullptr)
{
    _calibrationData = getCalibrationsStoreInstance()->getUserCalibrationData(_id.key);
}

HydroponicsSensor::HydroponicsSensor(const HydroponicsSensorData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _inputPin(dataIn->inputPin), _isTakingMeasure(false), _crop(this), _reservoir(this), _calibrationData(nullptr)
{
    _calibrationData = getCalibrationsStoreInstance()->getUserCalibrationData(_id.key);
    _crop = dataIn->cropName;
    _reservoir = dataIn->reservoirName;
}

HydroponicsSensor::~HydroponicsSensor()
{
    _isTakingMeasure = false;
}

void HydroponicsSensor::update() {
    HydroponicsObject::update();

    _crop.resolve();
    _reservoir.resolve();
}

bool HydroponicsSensor::isTakingMeasurement() const
{
    return _isTakingMeasure;
}

bool HydroponicsSensor::needsPolling(uint32_t allowance) const
{
    auto hydroponics = getHydroponicsInstance();
    auto latestMeasurement = getLatestMeasurement();
    return hydroponics && latestMeasurement ? hydroponics->isPollingFrameOld(latestMeasurement->frame, allowance) : false;
}

HydroponicsAttachment &HydroponicsSensor::getParentCrop(bool resolve)
{
    if (resolve) { _crop.resolve(); }
    return _crop;
}

HydroponicsAttachment &HydroponicsSensor::getParentReservoir(bool resolve)
{
    if (resolve) { _reservoir.resolve(); }
    return _reservoir;
}

void HydroponicsSensor::setUserCalibrationData(HydroponicsCalibrationData *userCalibrationData)
{
    if (userCalibrationData && getCalibrationsStoreInstance()->setUserCalibrationData(userCalibrationData)) {
        _calibrationData = getCalibrationsStoreInstance()->getUserCalibrationData(_id.key);
    } else if (!userCalibrationData && _calibrationData && getCalibrationsStoreInstance()->dropUserCalibrationData(_calibrationData)) {
        _calibrationData = nullptr;
    }
}

Signal<const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS> &HydroponicsSensor::getMeasurementSignal()
{
    return _measureSignal;
}

HydroponicsData *HydroponicsSensor::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void HydroponicsSensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;
    ((HydroponicsSensorData *)dataOut)->inputPin = _inputPin;
    if (_reservoir.getId()) {
        strncpy(((HydroponicsSensorData *)dataOut)->reservoirName, _reservoir.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_crop.getId()) {
        strncpy(((HydroponicsSensorData *)dataOut)->cropName, _crop.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}


HydroponicsBinarySensor::HydroponicsBinarySensor(Hydroponics_SensorType sensorType,
                                                 Hydroponics_PositionIndex sensorIndex,
                                                 byte inputPin,
                                                 bool activeLow,
                                                 int classType)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin, classType),
      _activeLow(activeLow), _usingISR(false)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), SFP(HS_Err_InvalidPinOrType));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
    }
}

HydroponicsBinarySensor::HydroponicsBinarySensor(const HydroponicsBinarySensorData *dataIn)
    : HydroponicsSensor(dataIn), _activeLow(dataIn->activeLow), _usingISR(false)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), SFP(HS_Err_InvalidPinOrType));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
    }
    if (dataIn->usingISR) { tryRegisterAsISR(); }
}

HydroponicsBinarySensor::~HydroponicsBinarySensor()
{
    if (_usingISR) {
        // TODO: detach ISR from taskManger (not currently possible, maybe in future?)
    }
}

bool HydroponicsBinarySensor::takeMeasurement(bool force)
{
    if (isValidPin(_inputPin) && (force || needsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;
        bool stateBefore = _lastMeasurement.state;

        bool state = (digitalRead(_inputPin) == (_activeLow ? LOW : HIGH));
        auto timestamp = unixNow();

        _lastMeasurement = HydroponicsBinaryMeasurement(state, timestamp);

        #ifndef HYDRUINO_DISABLE_MULTITASKING
            scheduleSignalFireOnce<const HydroponicsMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);
        #else
            _measureSignal.fire(&_lastMeasurement);
        #endif

        if (state != stateBefore) {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<bool>(getSharedPtr(), _stateSignal, _lastMeasurement.state);
            #else
                _stateSignal.fire(_lastMeasurement.state);
            #endif
        }

        _isTakingMeasure = false;
        return true;
    }
    return false;
}

const HydroponicsMeasurement *HydroponicsBinarySensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsBinarySensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, byte measurementRow)
{ ; }

Hydroponics_UnitsType HydroponicsBinarySensor::getMeasurementUnits(byte measurementRow = 0) const
{
    return Hydroponics_UnitsType_Raw_0_1;
}

bool HydroponicsBinarySensor::tryRegisterAsISR()
{
    #ifndef HYDRUINO_DISABLE_MULTITASKING
        if (!_usingISR && checkPinCanInterrupt(_inputPin)) {
            taskManager.addInterrupt(&interruptImpl, _inputPin, CHANGE);
            _usingISR = true;
        }
    #endif
    return _usingISR;
}

Signal<bool> &HydroponicsBinarySensor::getStateSignal()
{
    return _stateSignal;
}

void HydroponicsBinarySensor::notifyISRTriggered()
{
    takeMeasurement(true);
}

void HydroponicsBinarySensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsSensor::saveToData(dataOut);

    ((HydroponicsBinarySensorData *)dataOut)->activeLow = _activeLow;
    ((HydroponicsBinarySensorData *)dataOut)->usingISR = _usingISR;
}


HydroponicsAnalogSensor::HydroponicsAnalogSensor(Hydroponics_SensorType sensorType,
                                                 Hydroponics_PositionIndex sensorIndex,
                                                 byte inputPin, byte inputBitRes, bool inputInversion,
                                                 int classType)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin, classType),
      _inputResolution(inputBitRes), _inputInversion(inputInversion), _measurementUnits(defaultMeasureUnitsForSensorType(sensorType))
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), SFP(HS_Err_InvalidPinOrType));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, INPUT);
    }
}

HydroponicsAnalogSensor::HydroponicsAnalogSensor(const HydroponicsAnalogSensorData *dataIn)
    : HydroponicsSensor(dataIn),
      _inputResolution(dataIn->inputBitRes), _inputInversion(dataIn->inputInversion),
      _measurementUnits(definedUnitsElse(dataIn->measurementUnits, defaultMeasureUnitsForSensorType((Hydroponics_SensorType)(dataIn->id.object.objType))))
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), SFP(HS_Err_InvalidPinOrType));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, INPUT);
    }
}

bool HydroponicsAnalogSensor::takeMeasurement(bool force)
{
    if (isValidPin(_inputPin) && (force || needsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;

        #ifndef HYDRUINO_DISABLE_MULTITASKING
            if (scheduleObjectMethodCallWithTaskIdOnce(::getSharedPtr<HydroponicsAnalogSensor>(this), &_takeMeasurement) != TASKMGR_INVALIDID) {
                return true;
            } else {
                HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_OperationFailure));
                _isTakingMeasure = false;
            }
        #else
            _takeMeasurement(0xffffU);
        #endif
    }
    return false;
}

void HydroponicsAnalogSensor::_takeMeasurement(unsigned int taskId)
{
    if (_isTakingMeasure && isValidPin(_inputPin)) {
        if (getHydroponicsInstance()->tryGetPinLock(_inputPin, 5)) {
            Hydroponics_UnitsType outUnits = definedUnitsElse(_measurementUnits,
                                                              _calibrationData ? _calibrationData->calibUnits : Hydroponics_UnitsType_Undefined,
                                                              defaultMeasureUnitsForSensorType(_id.objTypeAs.sensorType));

            unsigned int rawRead = 0;
            #if HYDRUINO_SENSOR_ANALOGREAD_SAMPLES > 1
                for (int sampleIndex = 0; sampleIndex < HYDRUINO_SENSOR_ANALOGREAD_SAMPLES; ++sampleIndex) {
                    #if HYDRUINO_SENSOR_ANALOGREAD_DELAY > 0
                        if (sampleIndex) { delay(HYDRUINO_SENSOR_ANALOGREAD_DELAY); }
                    #endif
                    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                        analogReadResolution(_inputResolution.bitRes);
                    #endif
                    rawRead += analogRead(_inputPin);
                }
                rawRead /= HYDRUINO_SENSOR_ANALOGREAD_SAMPLES;
            #else
                #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                    analogReadResolution(_inputResolution.bitRes);
                #endif
                rawRead = analogRead(_inputPin);
            #endif // /if HYDRUINO_SENSOR_ANALOGREAD_SAMPLES > 1

            if (_inputInversion) { rawRead = _inputResolution.maxVal - rawRead; }
            auto timestamp = unixNow();

            HydroponicsSingleMeasurement newMeasurement(
                _inputResolution.transform(rawRead),
                Hydroponics_UnitsType_Raw_0_1,
                timestamp
            );

            if (_calibrationData) {
                _calibrationData->transform(&newMeasurement);
            }

            convertUnits(&newMeasurement, outUnits);

            _lastMeasurement = newMeasurement;

            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<const HydroponicsMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);
            #else
                _measureSignal.fire(&_lastMeasurement);
            #endif

            getHydroponicsInstance()->returnPinLock(_inputPin);
            _isTakingMeasure = false;
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                tryDisableRepeatingTask(taskId);
            #endif
        } else {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                if (!tryEnableRepeatingTask(taskId)) { _isTakingMeasure = false; }
            #else
                _isTakingMeasure = false;
            #endif
        }
    } else {
        #ifndef HYDRUINO_DISABLE_MULTITASKING
            tryDisableRepeatingTask(taskId);
        #endif
    }
}

const HydroponicsMeasurement *HydroponicsAnalogSensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsAnalogSensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, byte measurementRow)
{
    if (_measurementUnits != measurementUnits) {
        _measurementUnits = measurementUnits;

        if (_lastMeasurement.frame) {
            convertUnits(&_lastMeasurement, _measurementUnits);
        }
    }
}

Hydroponics_UnitsType HydroponicsAnalogSensor::getMeasurementUnits(byte measurementRow) const
{
    return _measurementUnits;
}

void HydroponicsAnalogSensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsSensor::saveToData(dataOut);

    ((HydroponicsAnalogSensorData *)dataOut)->inputBitRes = _inputResolution.bitRes;
    ((HydroponicsAnalogSensorData *)dataOut)->inputInversion = _inputInversion;
    ((HydroponicsAnalogSensorData *)dataOut)->measurementUnits = _measurementUnits;
}


HydroponicsDigitalSensor::HydroponicsDigitalSensor(Hydroponics_SensorType sensorType,
                                                   Hydroponics_PositionIndex sensorIndex,
                                                   byte inputPin, byte inputBitRes,
                                                   bool allocate1W,
                                                   int classType)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin, classType), _inputBitRes(inputBitRes), _oneWire(nullptr), _wirePosIndex(-1), _wireDevAddress{0}
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), SFP(HS_Err_InvalidPinOrType));
    if (allocate1W && isValidPin(_inputPin)) {
        auto hydroponics = getHydroponicsInstance();
        _oneWire = hydroponics ? hydroponics->getOneWireForPin(_inputPin) : nullptr;
        HYDRUINO_SOFT_ASSERT(_oneWire, SFP(HS_Err_AllocationFailure));
    }
}

HydroponicsDigitalSensor::HydroponicsDigitalSensor(const HydroponicsDigitalSensorData *dataIn, bool allocate1W)
    : HydroponicsSensor(dataIn), _inputBitRes(dataIn->inputBitRes), _oneWire(nullptr), _wirePosIndex(-1), _wireDevAddress{0}
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), SFP(HS_Err_InvalidPinOrType));
    if (allocate1W && isValidPin(_inputPin)) {
        auto hydroponics = getHydroponicsInstance();
        _oneWire = hydroponics ? hydroponics->getOneWireForPin(_inputPin) : nullptr;
        HYDRUINO_SOFT_ASSERT(_oneWire, SFP(HS_Err_AllocationFailure));

        if (!arrayElementsEqual<uint8_t>(dataIn->wireDevAddress, 8, 0)) {
            _wirePosIndex = -1 - dataIn->wirePosIndex;
            memcpy(_wireDevAddress, dataIn->wireDevAddress, 8);
        } else {
            _wirePosIndex = -1 - dataIn->wirePosIndex;
        }
    }
}

bool HydroponicsDigitalSensor::setWirePositionIndex(Hydroponics_PositionIndex wirePosIndex)
{
    wirePosIndex = constrain(wirePosIndex, 0, 62);
    if (_oneWire && wirePosIndex >= 0 && (_wirePosIndex != wirePosIndex || arrayElementsEqual<uint8_t>(_wireDevAddress, 8, 0)) &&
        getHydroponicsInstance()->tryGetPinLock(_inputPin)) {
        Hydroponics_PositionIndex posIndex = 0;
        uint8_t devAddress[8];

        _oneWire->reset_search();
        while (posIndex <= wirePosIndex && _oneWire->search(devAddress)) {
            if (posIndex == wirePosIndex && _oneWire->crc8(devAddress, 7) == devAddress[7]) {
                _wirePosIndex = posIndex;
                memcpy(_wireDevAddress, devAddress, 8);

                getHydroponicsInstance()->returnPinLock(_inputPin);
                return true;
            }
            posIndex++;
        }

        getHydroponicsInstance()->returnPinLock(_inputPin);
    }
    return false;
}

Hydroponics_PositionIndex HydroponicsDigitalSensor::getWirePositionIndex() const
{
    return _wirePosIndex >= 0 ? _wirePosIndex : (_wirePosIndex > -64 ? -_wirePosIndex - 1 : -_wirePosIndex - 64);
}

bool HydroponicsDigitalSensor::setWireDeviceAddress(const uint8_t wireDevAddress[8])
{
    if (_oneWire && !arrayElementsEqual<uint8_t>(wireDevAddress, 8, 0) && (_wirePosIndex < 0 || memcmp(_wireDevAddress, wireDevAddress, 8) != 0) &&
        _oneWire->crc8(wireDevAddress, 7) == wireDevAddress[7] && getHydroponicsInstance()->tryGetPinLock(_inputPin)) {
        Hydroponics_PositionIndex posIndex = 0;
        uint8_t devAddress[8];

        _oneWire->reset_search();
        while (_oneWire->search(devAddress)) {
            if (memcmp(devAddress, wireDevAddress, 8) == 0) {
                _wirePosIndex = posIndex;
                memcpy(_wireDevAddress, devAddress, 8);

                getHydroponicsInstance()->returnPinLock(_inputPin);
                return true;
            }
            posIndex++;
        }

        getHydroponicsInstance()->returnPinLock(_inputPin);
    }
    return false;
}

const uint8_t *HydroponicsDigitalSensor::getWireDeviceAddress() const
{
    return _wireDevAddress;
}

void HydroponicsDigitalSensor::resolveDeviceAddress()
{
    if (_oneWire && !(_wirePosIndex >= 0)) {
        setWireDeviceAddress(_wireDevAddress);

        if (!(_wirePosIndex >= 0) && _wirePosIndex > -64) {
            Hydroponics_PositionIndex posIndex = -_wirePosIndex - 1;
            setWirePositionIndex(posIndex);

            if (!(_wirePosIndex >= 0)) { _wirePosIndex = -64 - posIndex; } // disables further resolve attempts
        }
    }
}

void HydroponicsDigitalSensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsSensor::saveToData(dataOut);

    ((HydroponicsDigitalSensorData *)dataOut)->inputBitRes = _inputBitRes;
    ((HydroponicsDigitalSensorData *)dataOut)->wirePosIndex = getWirePositionIndex();
    memcpy(((HydroponicsDigitalSensorData *)dataOut)->wireDevAddress, _wireDevAddress, 8);
}


HydroponicsDHTTempHumiditySensor::HydroponicsDHTTempHumiditySensor(Hydroponics_PositionIndex sensorIndex,
                                                                   byte inputPin,
                                                                   byte dhtType,
                                                                   bool computeHeatIndex,
                                                                   int classType)
    : HydroponicsDigitalSensor(Hydroponics_SensorType_AirTempHumidity, sensorIndex, inputPin, 9, false, classType),
      _dht(new DHT(inputPin, dhtType)), _dhtType(dhtType), _computeHeatIndex(computeHeatIndex),
      _measurementUnits{defaultTemperatureUnits(), Hydroponics_UnitsType_Percentile_0_100, defaultTemperatureUnits()}
{
    HYDRUINO_SOFT_ASSERT(_dht, SFP(HS_Err_AllocationFailure));
    if (isValidPin(_inputPin) && _dht) { _dht->begin(); }
    else if (_dht) { delete _dht; _dht = nullptr; }
}

HydroponicsDHTTempHumiditySensor::HydroponicsDHTTempHumiditySensor(const HydroponicsDHTTempHumiditySensorData *dataIn)
    : HydroponicsDigitalSensor(dataIn, false),
      _dht(new DHT(dataIn->inputPin, dataIn->dhtType)), _dhtType(dataIn->dhtType), _computeHeatIndex(dataIn->computeHeatIndex),
      _measurementUnits{definedUnitsElse(dataIn->measurementUnits, defaultTemperatureUnits()),
                        Hydroponics_UnitsType_Percentile_0_100,
                        definedUnitsElse(dataIn->measurementUnits, defaultTemperatureUnits())}
{
    HYDRUINO_SOFT_ASSERT(_dht, SFP(HS_Err_AllocationFailure));
    if (isValidPin(_inputPin) && _dht) { _dht->begin(); }
    else if (_dht) { delete _dht; _dht = nullptr; }
}

HydroponicsDHTTempHumiditySensor::~HydroponicsDHTTempHumiditySensor()
{
    if (_dht) { delete _dht; _dht = nullptr; }
}

bool HydroponicsDHTTempHumiditySensor::takeMeasurement(bool force)
{
    if (getHydroponicsInstance() && _dht && (force || needsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;

        #ifndef HYDRUINO_DISABLE_MULTITASKING
            if (scheduleObjectMethodCallWithTaskIdOnce(::getSharedPtr<HydroponicsDHTTempHumiditySensor>(this), &_takeMeasurement) != TASKMGR_INVALIDID) {
                return true;
            } else {
                HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_OperationFailure));
                _isTakingMeasure = false;
            }
        #else
            _takeMeasurement(0xffffU);
        #endif
    }
    return false;
}

void HydroponicsDHTTempHumiditySensor::_takeMeasurement(unsigned int taskId)
{
    if (_isTakingMeasure && _dht) {
        if (getHydroponicsInstance()->tryGetPinLock(_inputPin, 5)) {
            Hydroponics_UnitsType outUnits[3] = { definedUnitsElse(_measurementUnits[0],
                                                                   _calibrationData ? _calibrationData->calibUnits : Hydroponics_UnitsType_Undefined,
                                                                   defaultTemperatureUnits()),
                                                  definedUnitsElse(_measurementUnits[1],
                                                                   Hydroponics_UnitsType_Percentile_0_100),
                                                  definedUnitsElse(_measurementUnits[2],
                                                                   _calibrationData ? _calibrationData->calibUnits : Hydroponics_UnitsType_Undefined,
                                                                   defaultTemperatureUnits()) };
            bool readInFahrenheit = outUnits[0] == Hydroponics_UnitsType_Temperature_Fahrenheit;
            Hydroponics_UnitsType readUnits = readInFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit
                                                               : Hydroponics_UnitsType_Temperature_Celsius;

            auto tempRead = _dht->readTemperature(readInFahrenheit, true);
            auto humidRead = _dht->readHumidity(true);
            auto timestamp = unixNow();

            HydroponicsTripleMeasurement newMeasurement(
                tempRead, readUnits, humidRead, Hydroponics_UnitsType_Percentile_0_100,
                0.0f, Hydroponics_UnitsType_Undefined,
                timestamp
            );

            if (_calibrationData) {
                _calibrationData->transform(&newMeasurement.value[0], &newMeasurement.units[0]);
            }

            convertUnits(&newMeasurement.value[0], &newMeasurement.units[0], outUnits[0]);
            convertUnits(&newMeasurement.value[1], &newMeasurement.units[1], outUnits[1]);

            if (_computeHeatIndex) {
                convertUnits(newMeasurement.value[0], &newMeasurement.value[2], newMeasurement.units[0], readUnits, &newMeasurement.units[2]);
                newMeasurement.value[2] = _dht->computeHeatIndex(newMeasurement.value[2], humidRead, readInFahrenheit);

                convertUnits(&newMeasurement.value[2], &newMeasurement.units[2], outUnits[2]);
            }

            _lastMeasurement = newMeasurement;

            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<const HydroponicsMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);
            #else
                _measureSignal.fire(&_lastMeasurement);
            #endif

            getHydroponicsInstance()->returnPinLock(_inputPin);
            _isTakingMeasure = false;
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                tryDisableRepeatingTask(taskId);
            #endif
        } else {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                if (!tryEnableRepeatingTask(taskId)) { _isTakingMeasure = false; }
            #else
                _isTakingMeasure = false;
            #endif
        }
    } else {
        #ifndef HYDRUINO_DISABLE_MULTITASKING
            tryDisableRepeatingTask(taskId);
        #endif
    }
}

const HydroponicsMeasurement *HydroponicsDHTTempHumiditySensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsDHTTempHumiditySensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, byte measurementRow)
{
    if (_measurementUnits[measurementRow] != measurementUnits) {
        _measurementUnits[measurementRow] = measurementUnits;

        if (_lastMeasurement.frame) {
            convertUnits(&_lastMeasurement.value[measurementRow], &_lastMeasurement.units[measurementRow], _measurementUnits[measurementRow]);
        }
    }
}

Hydroponics_UnitsType HydroponicsDHTTempHumiditySensor::getMeasurementUnits(byte measurementRow) const
{
    return _measurementUnits[measurementRow];
}

bool HydroponicsDHTTempHumiditySensor::setWirePositionIndex(Hydroponics_PositionIndex wirePosIndex)
{
    return false;
}

Hydroponics_PositionIndex HydroponicsDHTTempHumiditySensor::getWirePositionIndex() const
{
    return -1;
}

bool HydroponicsDHTTempHumiditySensor::setWireDeviceAddress(const uint8_t wireDevAddress[8])
{
    return false;
}

const uint8_t *HydroponicsDHTTempHumiditySensor::getWireDeviceAddress() const
{
    return nullptr;
}

void HydroponicsDHTTempHumiditySensor::setComputeHeatIndex(bool computeHeatIndex)
{
    if (_computeHeatIndex != computeHeatIndex) {
        _computeHeatIndex = computeHeatIndex;
    }
}

void HydroponicsDHTTempHumiditySensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsDigitalSensor::saveToData(dataOut);

    ((HydroponicsDHTTempHumiditySensorData *)dataOut)->dhtType = _dhtType;
    ((HydroponicsDHTTempHumiditySensorData *)dataOut)->computeHeatIndex = _computeHeatIndex;
    ((HydroponicsDHTTempHumiditySensorData *)dataOut)->measurementUnits = _measurementUnits[0];
}


HydroponicsDSTemperatureSensor::HydroponicsDSTemperatureSensor(Hydroponics_PositionIndex sensorIndex,
                                                               byte inputPin, byte inputBitRes,
                                                               byte pullupPin,
                                                               int classType)
    : HydroponicsDigitalSensor(Hydroponics_SensorType_WaterTemperature, sensorIndex, inputPin, inputBitRes, true, classType),
      _dt(new DallasTemperature()), _pullupPin(pullupPin), _measurementUnits(defaultTemperatureUnits())
{
    HYDRUINO_SOFT_ASSERT(_dt, SFP(HS_Err_AllocationFailure));

    if (isValidPin(_inputPin) && _oneWire && _dt) {
        _dt->setOneWire(_oneWire);
        if (isValidPin(_pullupPin)) { _dt->setPullupPin(_pullupPin); }
        _dt->setWaitForConversion(true); // reads will be done in their own task, waits will delay and yield
        _dt->begin();
        if (_dt->getResolution() != _inputBitRes) { _dt->setResolution(_inputBitRes); }
        HYDRUINO_SOFT_ASSERT(_dt->getResolution() == _inputBitRes, SFP(HS_Err_ParameterMismatch));
    } else if (_dt) { delete _dt; _dt = nullptr; }
}

HydroponicsDSTemperatureSensor::HydroponicsDSTemperatureSensor(const HydroponicsDSTemperatureSensorData *dataIn)
    : HydroponicsDigitalSensor(dataIn, true),
      _dt(new DallasTemperature()), _pullupPin(dataIn->pullupPin),
      _measurementUnits(definedUnitsElse(dataIn->measurementUnits, defaultTemperatureUnits()))
{
    HYDRUINO_SOFT_ASSERT(_dt, SFP(HS_Err_AllocationFailure));

    if (isValidPin(_inputPin) && _oneWire && _dt) {
        _dt->setOneWire(_oneWire);
        if (isValidPin(_pullupPin)) { _dt->setPullupPin(_pullupPin); }
        _dt->setWaitForConversion(true); // reads will be done in their own task, waits will delay and yield
        _dt->begin();
        if (_dt->getResolution() != _inputBitRes) { _dt->setResolution(_inputBitRes); }
        HYDRUINO_SOFT_ASSERT(_dt->getResolution() == _inputBitRes, SFP(HS_Err_ParameterMismatch));
    } else if (_dt) { delete _dt; _dt = nullptr; }
}

HydroponicsDSTemperatureSensor::~HydroponicsDSTemperatureSensor()
{
    if (_dt) { delete _dt; _dt = nullptr; }
}

bool HydroponicsDSTemperatureSensor::takeMeasurement(bool force)
{
    if (!(_wirePosIndex >= 0)) { resolveDeviceAddress(); }

    if (_dt && _wirePosIndex >= 0 && (force || needsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;

        #ifndef HYDRUINO_DISABLE_MULTITASKING
            if (scheduleObjectMethodCallWithTaskIdOnce(::getSharedPtr<HydroponicsDSTemperatureSensor>(this), &_takeMeasurement) != TASKMGR_INVALIDID) {
                return true;
            } else {
                HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_OperationFailure));
                _isTakingMeasure = false;
            }
        #else
            _takeMeasurement(0xffffU);
        #endif
    }
    return false;
}

void HydroponicsDSTemperatureSensor::_takeMeasurement(unsigned int taskId)
{
    if (_isTakingMeasure && _dt) {
        if (getHydroponicsInstance()->tryGetPinLock(_inputPin, 5)) {
            if (_dt->requestTemperaturesByAddress(_wireDevAddress)) {
                Hydroponics_UnitsType outUnits = definedUnitsElse(_measurementUnits,
                                                                  _calibrationData ? _calibrationData->calibUnits : Hydroponics_UnitsType_Undefined,
                                                                  defaultTemperatureUnits());
                bool readInFahrenheit = _measurementUnits == Hydroponics_UnitsType_Temperature_Fahrenheit;
                Hydroponics_UnitsType readUnits = readInFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit
                                                                   : Hydroponics_UnitsType_Temperature_Celsius;

                auto tempRead = readInFahrenheit ? _dt->getTempF(_wireDevAddress) : _dt->getTempC(_wireDevAddress);
                auto timestamp = unixNow();

                HydroponicsSingleMeasurement newMeasurement(
                    tempRead,
                    readUnits,
                    timestamp
                );

                bool deviceDisconnected = isFPEqual(tempRead, (float)(readInFahrenheit ? DEVICE_DISCONNECTED_F : DEVICE_DISCONNECTED_C));
                HYDRUINO_SOFT_ASSERT(!deviceDisconnected, SFP(HS_Err_MeasurementFailure)); // device disconnected

                if (!deviceDisconnected) {
                    if (_calibrationData) {
                        _calibrationData->transform(&newMeasurement);
                    }

                    convertUnits(&newMeasurement, outUnits);

                    _lastMeasurement = newMeasurement;
                    #ifndef HYDRUINO_DISABLE_MULTITASKING
                        scheduleSignalFireOnce<const HydroponicsMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);
                    #else
                        _measureSignal.fire(&_lastMeasurement);
                    #endif
                }
            } else {
                HYDRUINO_SOFT_ASSERT(false, SFP(HS_Err_MeasurementFailure)); // device disconnected or no device by that addr
            }

            getHydroponicsInstance()->returnPinLock(_inputPin);
            _isTakingMeasure = false;
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                tryDisableRepeatingTask(taskId);
            #endif
        } else {
            #ifndef HYDRUINO_DISABLE_MULTITASKING
                if (!tryEnableRepeatingTask(taskId)) { _isTakingMeasure = false; }
            #else
                _isTakingMeasure = false;
            #endif
        }
    } else {
        #ifndef HYDRUINO_DISABLE_MULTITASKING
            tryDisableRepeatingTask(taskId);
        #endif
    }
}

const HydroponicsMeasurement *HydroponicsDSTemperatureSensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsDSTemperatureSensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, byte measurementRow)
{
    if (_measurementUnits != measurementUnits) {
        _measurementUnits = measurementUnits;

        if (_lastMeasurement.frame) {
            convertUnits(&_lastMeasurement, _measurementUnits);
        }
    }
}

Hydroponics_UnitsType HydroponicsDSTemperatureSensor::getMeasurementUnits(byte measurementRow) const
{
    return _measurementUnits;
}

void HydroponicsDSTemperatureSensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsDigitalSensor::saveToData(dataOut);

    ((HydroponicsDSTemperatureSensorData *)dataOut)->pullupPin = _pullupPin;
    ((HydroponicsDSTemperatureSensorData *)dataOut)->measurementUnits = _measurementUnits;
}


HydroponicsSensorData::HydroponicsSensorData()
    : HydroponicsObjectData(), inputPin(-1), cropName{0}, reservoirName{0}
{
    _size = sizeof(*this);
}

void HydroponicsSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);

    if (isValidPin(inputPin)) { objectOut[SFP(HS_Key_InputPin)] = inputPin; }
    if (cropName[0]) { objectOut[SFP(HS_Key_CropName)] = charsToString(cropName, HYDRUINO_NAME_MAXSIZE); }
    if (reservoirName[0]) { objectOut[SFP(HS_Key_ReservoirName)] = charsToString(reservoirName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    inputPin = objectIn[SFP(HS_Key_InputPin)] | inputPin;
    const char *cropStr = objectIn[SFP(HS_Key_CropName)];
    if (cropStr && cropStr[0]) { strncpy(cropName, cropStr, HYDRUINO_NAME_MAXSIZE); }
    const char *reservoirNameStr = objectIn[SFP(HS_Key_ReservoirName)];
    if (reservoirNameStr && reservoirNameStr[0]) { strncpy(reservoirName, reservoirNameStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsBinarySensorData::HydroponicsBinarySensorData()
    : HydroponicsSensorData(), activeLow(true), usingISR(false)
{
    _size = sizeof(*this);
}

void HydroponicsBinarySensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSensorData::toJSONObject(objectOut);

    objectOut[SFP(HS_Key_ActiveLow)] = activeLow;
    if (usingISR != false) { objectOut[SFP(HS_Key_UsingISR)] = usingISR; }
}

void HydroponicsBinarySensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSensorData::fromJSONObject(objectIn);

    activeLow = objectIn[SFP(HS_Key_ActiveLow)] | activeLow;
    usingISR = objectIn[SFP(HS_Key_UsingISR)] | usingISR;
}

HydroponicsAnalogSensorData::HydroponicsAnalogSensorData()
    : HydroponicsSensorData(), inputBitRes(8), inputInversion(false), measurementUnits(Hydroponics_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroponicsAnalogSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSensorData::toJSONObject(objectOut);

    if (inputBitRes != 8) { objectOut[SFP(HS_Key_InputBitRes)] = inputBitRes; }
    if (inputInversion != false) { objectOut[SFP(HS_Key_InputInversion)] = inputInversion; }
    if (measurementUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HS_Key_MeasurementUnits)] = unitsTypeToSymbol(measurementUnits); }
}

void HydroponicsAnalogSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSensorData::fromJSONObject(objectIn);

    inputBitRes = objectIn[SFP(HS_Key_InputBitRes)] | inputBitRes;
    inputInversion = objectIn[SFP(HS_Key_InputInversion)] | inputInversion;
    measurementUnits = unitsTypeFromSymbol(objectIn[SFP(HS_Key_MeasurementUnits)]);
}

HydroponicsDigitalSensorData::HydroponicsDigitalSensorData()
    : HydroponicsSensorData(), inputBitRes(9), wirePosIndex(-1), wireDevAddress{0}
{
    _size = sizeof(*this);
}

void HydroponicsDigitalSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSensorData::toJSONObject(objectOut);

    if (inputBitRes != 9) { objectOut[SFP(HS_Key_InputBitRes)] = inputBitRes; }
    if (wirePosIndex > 0) { objectOut[SFP(HS_Key_WirePosIndex)] = wirePosIndex; }
    if (!arrayElementsEqual<uint8_t>(wireDevAddress, 8, 0)) { objectOut[SFP(HS_Key_WireDevAddress)] = hexStringFromBytes(wireDevAddress, 8); }
}

void HydroponicsDigitalSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSensorData::fromJSONObject(objectIn);

    inputBitRes = objectIn[SFP(HS_Key_InputBitRes)] | inputBitRes;
    wirePosIndex = objectIn[SFP(HS_Key_WirePosIndex)] | wirePosIndex;
    JsonVariantConst wireDevAddressVar = objectIn[SFP(HS_Key_WireDevAddress)];
    hexStringToBytes(wireDevAddressVar, wireDevAddress, 8);
    for (int addrIndex = 0; addrIndex < 8; ++addrIndex) { 
        wireDevAddress[addrIndex] = wireDevAddressVar[addrIndex] | wireDevAddress[addrIndex];
    }
}

HydroponicsDHTTempHumiditySensorData::HydroponicsDHTTempHumiditySensorData()
    : HydroponicsDigitalSensorData(), dhtType(DHT12), computeHeatIndex(false), measurementUnits(Hydroponics_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroponicsDHTTempHumiditySensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsDigitalSensorData::toJSONObject(objectOut);

    if (dhtType != DHT12) { objectOut[SFP(HS_Key_DHTType)] = dhtType; }
    if (computeHeatIndex != false) { objectOut[SFP(HS_Key_ComputeHeatIndex)] = computeHeatIndex; }
    if (measurementUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HS_Key_MeasurementUnits)] = unitsTypeToSymbol(measurementUnits); }
}

void HydroponicsDHTTempHumiditySensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsDigitalSensorData::fromJSONObject(objectIn);

    dhtType = objectIn[SFP(HS_Key_DHTType)] | dhtType;
    computeHeatIndex = objectIn[SFP(HS_Key_ComputeHeatIndex)] | computeHeatIndex;
    measurementUnits = unitsTypeFromSymbol(objectIn[SFP(HS_Key_MeasurementUnits)]);
}

HydroponicsDSTemperatureSensorData::HydroponicsDSTemperatureSensorData()
    : HydroponicsDigitalSensorData(), pullupPin(-1), measurementUnits(Hydroponics_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroponicsDSTemperatureSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsDigitalSensorData::toJSONObject(objectOut);

    if (isValidPin(pullupPin)) { objectOut[SFP(HS_Key_PullupPin)] = pullupPin; }
    if (measurementUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HS_Key_MeasurementUnits)] = unitsTypeToSymbol(measurementUnits); }
}

void HydroponicsDSTemperatureSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsDigitalSensorData::fromJSONObject(objectIn);

    pullupPin = objectIn[SFP(HS_Key_PullupPin)] | pullupPin;
    measurementUnits = unitsTypeFromSymbol(objectIn[SFP(HS_Key_MeasurementUnits)]);
}
