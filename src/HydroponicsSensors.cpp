/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensorsare 
*/

#include "Hydroponics.h"

HydroponicsSensor *newSensorObjectFromData(const HydroponicsSensorData *dataIn)
{
    if (dataIn && dataIn->id.object.idType == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), F("Invalid data"));

    if (dataIn && dataIn->isObjectData()) {
        switch(dataIn->id.object.classType) {
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

Hydroponics_UnitsType defaultMeasureUnitsForSensorType(Hydroponics_SensorType sensorType, Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (sensorType) {
        case Hydroponics_SensorType_PotentialHydrogen:
            return Hydroponics_UnitsType_pHScale_0_14;
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
        case Hydroponics_SensorType_WaterHeightMeter:
            return defaultDistanceUnits(measureMode);
        case Hydroponics_SensorType_PowerUsageMeter:
            return Hydroponics_UnitsType_Power_Wattage;
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}


HydroponicsSensor::HydroponicsSensor(Hydroponics_SensorType sensorType,
                                     Hydroponics_PositionIndex sensorIndex,
                                     byte inputPin,
                                     int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(sensorType, sensorIndex)), classType((typeof(classType))classTypeIn),
      _inputPin(inputPin), _isTakingMeasure(false), _calibrationData(nullptr)
{
    _calibrationData = getCalibrationsStoreInstance()->getUserCalibrationData(_id.key);
}

HydroponicsSensor::HydroponicsSensor(const HydroponicsSensorData *dataIn)
    : HydroponicsObject(dataIn), classType((typeof(classType))(dataIn->id.object.classType)),
      _inputPin(dataIn->inputPin), _isTakingMeasure(false), _calibrationData(nullptr),
      _crop(dataIn->cropName), _reservoir(dataIn->reservoirName)
{
    _calibrationData = getCalibrationsStoreInstance()->getUserCalibrationData(_id.key);
}

HydroponicsSensor::~HydroponicsSensor()
{
    //discardFromTaskManager(&_measureSignal);
    if (_crop) { _crop->removeSensor(this); }
    if (_reservoir) { _reservoir->removeSensor(this); }
}

void HydroponicsSensor::update()
{
    HydroponicsObject::update();

    if (getNeedsPolling()) { takeMeasurement(); }
}

void HydroponicsSensor::resolveLinks()
{
    HydroponicsObject::resolveLinks();

    if (_crop.needsResolved()) { getCrop(); }
    if (_reservoir.needsResolved()) { getReservoir(); }
}

void HydroponicsSensor::handleLowMemory()
{
    HydroponicsObject::handleLowMemory();
}

bool HydroponicsSensor::getIsTakingMeasurement() const
{
    return _isTakingMeasure;
}

bool HydroponicsSensor::getNeedsPolling() const
{
    auto hydroponics = getHydroponicsInstance();
    auto latestMeasurement = getLatestMeasurement();
    return hydroponics && latestMeasurement ? hydroponics->getIsPollingFrameOld(latestMeasurement->frame) : false;
}

void HydroponicsSensor::setCrop(HydroponicsIdentity cropId)
{
    if (_crop != cropId) {
        if (_crop) { _crop->removeSensor(this); }
        _crop = cropId;
    }
}

void HydroponicsSensor::setCrop(shared_ptr<HydroponicsCrop> crop)
{
    if (_crop != crop) {
        if (_crop) { _crop->removeSensor(this); }
        _crop = crop;
        if (_crop) { _crop->addSensor(this); }
    }
}

shared_ptr<HydroponicsCrop> HydroponicsSensor::getCrop()
{
    if (_crop.resolveIfNeeded()) { _crop->addSensor(this); }
    return _crop.getObj();
}

void HydroponicsSensor::setReservoir(HydroponicsIdentity reservoirId)
{
    if (_reservoir != reservoirId) {
        if (_reservoir) { _reservoir->removeSensor(this); }
        _reservoir = reservoirId;
    }
}

void HydroponicsSensor::setReservoir(shared_ptr<HydroponicsReservoir> reservoir)
{
    if (_reservoir != reservoir) {
        if (_reservoir) { _reservoir->removeSensor(this); }
        _reservoir = reservoir;
        if (_reservoir) { _reservoir->addSensor(this); }
    }
}

shared_ptr<HydroponicsReservoir> HydroponicsSensor::getReservoir()
{
    if (_reservoir.resolveIfNeeded()) { _reservoir->addSensor(this); }
    return _reservoir.getObj();
}

void HydroponicsSensor::setUserCalibrationData(HydroponicsCalibrationData *userCalibrationData)
{
    if (userCalibrationData && getCalibrationsStoreInstance()->setUserCalibrationData(userCalibrationData)) {
        _calibrationData = getCalibrationsStoreInstance()->getUserCalibrationData(_id.key);
    } else if (!userCalibrationData && _calibrationData && getCalibrationsStoreInstance()->dropUserCalibrationData(_calibrationData)) {
        _calibrationData = nullptr;
    }
}

const HydroponicsCalibrationData *HydroponicsSensor::getUserCalibrationData() const
{
    return _calibrationData;
}

byte HydroponicsSensor::getInputPin() const
{
    return _inputPin;
}

Hydroponics_SensorType HydroponicsSensor::getSensorType() const
{
    return _id.objTypeAs.sensorType;
}

Hydroponics_PositionIndex HydroponicsSensor::getSensorIndex() const
{
    return _id.posIndex;
}

Signal<const HydroponicsMeasurement *> &HydroponicsSensor::getMeasurementSignal()
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
        strncpy(((HydroponicsSensorData *)dataOut)->reservoirName, _reservoir.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    if (_crop.getId()) {
        strncpy(((HydroponicsSensorData *)dataOut)->cropName, _crop.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
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
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
    }
}

HydroponicsBinarySensor::HydroponicsBinarySensor(const HydroponicsBinarySensorData *dataIn)
    : HydroponicsSensor(dataIn), _activeLow(dataIn->activeLow), _usingISR(false)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
    }
    if (dataIn->usingISR) { tryRegisterAsISR(); }
}

HydroponicsBinarySensor::~HydroponicsBinarySensor()
{
    //discardFromTaskManager(&_stateSignal);
}

bool HydroponicsBinarySensor::takeMeasurement(bool override)
{
    if (isValidPin(_inputPin) && (override || getNeedsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;

        auto rawRead = digitalRead(_inputPin);
        auto timestamp = now();

        HydroponicsBinaryMeasurement newMeasurement(!!rawRead, timestamp);

        bool stateChanged = _lastMeasurement.state != newMeasurement.state;
        _lastMeasurement = newMeasurement;

        scheduleSignalFireOnce<const HydroponicsMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);
        if (stateChanged) {
            scheduleSignalFireOnce<bool>(getSharedPtr(), _stateSignal, _lastMeasurement.state);
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

void HydroponicsBinarySensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow)
{ ; }

Hydroponics_UnitsType HydroponicsBinarySensor::getMeasurementUnits(int measurementRow = 0) const
{
    return Hydroponics_UnitsType_Raw_0_1;
}

bool HydroponicsBinarySensor::tryRegisterAsISR()
{
    if (!_usingISR && checkPinCanInterrupt(_inputPin)) {
        taskManager.addInterrupt(&interruptImpl, _inputPin, CHANGE);
        _usingISR = true;
    }
    return _usingISR;
}

bool HydroponicsBinarySensor::getActiveLow() const
{
    return _activeLow;
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
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, INPUT);
    }
}

HydroponicsAnalogSensor::HydroponicsAnalogSensor(const HydroponicsAnalogSensorData *dataIn)
    : HydroponicsSensor(dataIn),
      _inputResolution(dataIn->inputBitRes), _inputInversion(dataIn->inputInversion),
      _measurementUnits(definedUnitsElse(dataIn->measurementUnits, defaultMeasureUnitsForSensorType((Hydroponics_SensorType)(dataIn->id.object.objType)))),
      _tempSensor(dataIn->waterTempSensorName)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, INPUT);
    }
}

HydroponicsAnalogSensor::~HydroponicsAnalogSensor()
{ ; }

void HydroponicsAnalogSensor::resolveLinks()
{
    HydroponicsSensor::resolveLinks();

    if (_tempSensor.needsResolved()) { getTemperatureSensor(); }
}

bool HydroponicsAnalogSensor::takeMeasurement(bool override)
{
    if (isValidPin(_inputPin) && (override || getNeedsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;

        #ifndef HYDRUINO_DISABLE_MULTITASKING
            if (scheduleObjectMethodCallWithTaskIdOnce(::getSharedPtr<HydroponicsAnalogSensor>(this), &_takeMeasurement) != TASKMGR_INVALIDID) {
                return true;
            } else {
                _isTakingMeasure = false;
            }
        #else
            _takeMeasurement(TASKMGR_INVALIDID);
        #endif
    }
    return false;
}

void HydroponicsAnalogSensor::_takeMeasurement(taskid_t taskId)
{
    if (_isTakingMeasure && isValidPin(_inputPin)) {
        if (getHydroponicsInstance()->tryGetPinLock(_inputPin, 5)) {
            Hydroponics_UnitsType unitsOut = definedUnitsElse(_measurementUnits, _calibrationData ? _calibrationData->calibUnits : Hydroponics_UnitsType_Undefined, defaultMeasureUnitsForSensorType(_id.objTypeAs.sensorType));

            #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
                analogReadResolution(_inputResolution.bitRes);
            #endif

            unsigned int rawRead = 0;
            #if HYDRUINO_SENSOR_ANALOGREAD_SAMPLES > 1
                for (int sampleIndex = 0; sampleIndex < HYDRUINO_SENSOR_ANALOGREAD_SAMPLES; ++sampleIndex) {
                    #if HYDRUINO_SENSOR_ANALOGREAD_DELAY > 0
                        if (sampleIndex) { delay(HYDRUINO_SENSOR_ANALOGREAD_DELAY); }
                    #endif
                    rawRead += analogRead(_inputPin);
                }
                rawRead /= HYDRUINO_SENSOR_ANALOGREAD_SAMPLES;
            #else
                rawRead = analogRead(_inputPin);
            #endif
            if (_inputInversion) { rawRead = _inputResolution.maxVal - rawRead; }
            auto timestamp = now();

            HydroponicsSingleMeasurement newMeasurement(
                _inputResolution.transform(rawRead),
                Hydroponics_UnitsType_Raw_0_1,
                timestamp
            );

            if (_calibrationData) {
                _calibrationData->transform(&newMeasurement.value, &newMeasurement.units);
            }

            convertUnits(&newMeasurement.value, &newMeasurement.units, unitsOut);

            _lastMeasurement = newMeasurement;
            scheduleSignalFireOnce<const HydroponicsMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);

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
    }
}

const HydroponicsMeasurement *HydroponicsAnalogSensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsAnalogSensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow)
{
    if (_measurementUnits != measurementUnits) {
        _measurementUnits = measurementUnits;

        convertUnits(&_lastMeasurement, _measurementUnits);
    }
}

Hydroponics_UnitsType HydroponicsAnalogSensor::getMeasurementUnits(int measurementRow) const
{
    return _measurementUnits;
}

void HydroponicsAnalogSensor::setTemperatureSensor(HydroponicsIdentity sensorId)
{
    if (_tempSensor != sensorId) {
        _tempSensor = sensorId;
    }
}

void HydroponicsAnalogSensor::setTemperatureSensor(shared_ptr<HydroponicsSensor> sensor)
{
    if (_tempSensor != sensor) {
        _tempSensor = sensor;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsAnalogSensor::getTemperatureSensor()
{
    if (_tempSensor.resolveIfNeeded()) { ; }
    return _tempSensor.getObj();
}

HydroponicsBitResolution HydroponicsAnalogSensor::getInputResolution() const
{
    return _inputResolution;
}

bool HydroponicsAnalogSensor::getInputInversion() const
{
    return _inputInversion;
}

void HydroponicsAnalogSensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsSensor::saveToData(dataOut);

    if (_tempSensor.getId()) {
        strncpy(((HydroponicsAnalogSensorData *)dataOut)->waterTempSensorName, _tempSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}


HydroponicsDigitalSensor::HydroponicsDigitalSensor(Hydroponics_SensorType sensorType,
                                                   Hydroponics_PositionIndex sensorIndex,
                                                   byte inputPin, byte inputBitRes,
                                                   bool allocate1W,
                                                   int classType)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin, classType), _inputBitRes(inputBitRes), _oneWire(nullptr), _wirePosIndex(-1), _wireDevAddress{0}
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (allocate1W && isValidPin(_inputPin)) {
        auto hydroponics = getHydroponicsInstance();
        _oneWire = hydroponics ? hydroponics->getOneWireForPin(_inputPin) : nullptr;
        HYDRUINO_SOFT_ASSERT(_oneWire, F("Failure creating OneWire instance"));
    }
}

HydroponicsDigitalSensor::HydroponicsDigitalSensor(const HydroponicsDigitalSensorData *dataIn, bool allocate1W)
    : HydroponicsSensor(dataIn), _inputBitRes(dataIn->inputBitRes), _oneWire(nullptr), _wirePosIndex(-1), _wireDevAddress{0}
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (allocate1W && isValidPin(_inputPin)) {
        auto hydroponics = getHydroponicsInstance();
        _oneWire = hydroponics ? hydroponics->getOneWireForPin(_inputPin) : nullptr;
        HYDRUINO_SOFT_ASSERT(_oneWire, F("Failure creating OneWire instance"));

        if (!arrayElementsEqual<uint8_t>(dataIn->wireDevAddress, 8, 0)) {
            _wirePosIndex = -1 - dataIn->wirePosIndex;
            memcpy(_wireDevAddress, dataIn->wireDevAddress, 8);
        } else {
            _wirePosIndex = -1 - dataIn->wirePosIndex;
        }
    }
}

HydroponicsDigitalSensor::~HydroponicsDigitalSensor()
{ ; }

void HydroponicsDigitalSensor::resolveLinks()
{
    HydroponicsSensor::resolveLinks();

    if (_tempSensor.needsResolved()) { getTemperatureSensor(); }
    if (!(_wirePosIndex >= 0)) { resolveDeviceAddress(); }
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

void HydroponicsDigitalSensor::setTemperatureSensor(HydroponicsIdentity sensorId)
{
    if (_tempSensor != sensorId) {
        _tempSensor = sensorId;
    }
}

void HydroponicsDigitalSensor::setTemperatureSensor(shared_ptr<HydroponicsSensor> sensor)
{
    if (_tempSensor != sensor) {
        _tempSensor = sensor;
    }
}

shared_ptr<HydroponicsSensor> HydroponicsDigitalSensor::getTemperatureSensor()
{
    if (_tempSensor.resolveIfNeeded()) { ; }
    return _tempSensor.getObj();
}

OneWire *HydroponicsDigitalSensor::getOneWire()
{
    return _oneWire;
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
    if (_tempSensor.getId()) {
        strncpy(((HydroponicsDigitalSensorData *)dataOut)->waterTempSensorName, _tempSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
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
    HYDRUINO_SOFT_ASSERT(_dht, F("Failure creating DHT instance"));
    if (isValidPin(_inputPin) && _dht) { _dht->begin(); }
    else if (_dht) { delete _dht; _dht = nullptr; }
}

HydroponicsDHTTempHumiditySensor::HydroponicsDHTTempHumiditySensor(const HydroponicsDHTTempHumiditySensorData *dataIn)
    : HydroponicsDigitalSensor(dataIn, false),
      _dht(new DHT(dataIn->inputPin, dataIn->dhtType)), _dhtType(dataIn->dhtType), _computeHeatIndex(dataIn->computeHeatIndex),
      _measurementUnits{dataIn->measurementUnits[0],dataIn->measurementUnits[1],dataIn->measurementUnits[2]}
{
    HYDRUINO_SOFT_ASSERT(_dht, F("Failure creating DHT instance"));
    if (isValidPin(_inputPin) && _dht) { _dht->begin(); }
    else if (_dht) { delete _dht; _dht = nullptr; }
}

HydroponicsDHTTempHumiditySensor::~HydroponicsDHTTempHumiditySensor()
{
    if (_dht) { delete _dht; _dht = nullptr; }
}

bool HydroponicsDHTTempHumiditySensor::takeMeasurement(bool override)
{
    if (getHydroponicsInstance() && _dht && (override || getNeedsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;

        #ifndef HYDRUINO_DISABLE_MULTITASKING
            if (scheduleObjectMethodCallWithTaskIdOnce(::getSharedPtr<HydroponicsDHTTempHumiditySensor>(this), &_takeMeasurement) != TASKMGR_INVALIDID) {
                return true;
            } else {
                _isTakingMeasure = false;
            }
        #else
            _takeMeasurement(TASKMGR_INVALIDID);
        #endif
    }
    return false;
}

void HydroponicsDHTTempHumiditySensor::_takeMeasurement(taskid_t taskId)
{
    if (_isTakingMeasure && _dht) {
        if (getHydroponicsInstance()->tryGetPinLock(_inputPin, 5)) {
            Hydroponics_UnitsType unitsOut[3] = { definedUnitsElse(_measurementUnits[0], _calibrationData ? _calibrationData->calibUnits : Hydroponics_UnitsType_Undefined, defaultTemperatureUnits()),
                                                  definedUnitsElse(_measurementUnits[1], Hydroponics_UnitsType_Percentile_0_100),
                                                  definedUnitsElse(_measurementUnits[2], _calibrationData ? _calibrationData->calibUnits : Hydroponics_UnitsType_Undefined, defaultTemperatureUnits()) };
            bool readInFahrenheit = unitsOut[0] == Hydroponics_UnitsType_Temperature_Fahrenheit;
            Hydroponics_UnitsType readUnits = readInFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit
                                                               : Hydroponics_UnitsType_Temperature_Celsius;

            auto tempRead = _dht->readTemperature(readInFahrenheit, true);
            auto humidRead = _dht->readHumidity(true);
            auto timestamp = now();

            HydroponicsTripleMeasurement newMeasurement(
                tempRead, readUnits, humidRead, Hydroponics_UnitsType_Percentile_0_100,
                0.0f, Hydroponics_UnitsType_Undefined,
                timestamp
            );

            if (_calibrationData) {
                _calibrationData->transform(&newMeasurement.value[0], &newMeasurement.units[0]);
            }

            convertUnits(&newMeasurement.value[0], &newMeasurement.units[0], unitsOut[0]);
            convertUnits(&newMeasurement.value[1], &newMeasurement.units[1], unitsOut[1]);

            if (_computeHeatIndex) {
                convertUnits(newMeasurement.value[0], &newMeasurement.value[2], newMeasurement.units[0], readUnits, &newMeasurement.units[2]);
                newMeasurement.value[2] = _dht->computeHeatIndex(newMeasurement.value[2], humidRead, readInFahrenheit);

                convertUnits(&newMeasurement.value[2], &newMeasurement.units[2], unitsOut[2]);
            }

            _lastMeasurement = newMeasurement;
            scheduleSignalFireOnce<const HydroponicsMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);

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
    }
}

const HydroponicsMeasurement *HydroponicsDHTTempHumiditySensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsDHTTempHumiditySensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow)
{
    if (_measurementUnits[measurementRow] != measurementUnits) {
        _measurementUnits[measurementRow] = measurementUnits;

        convertUnits(&_lastMeasurement.value[measurementRow], &_lastMeasurement.units[measurementRow], _measurementUnits[measurementRow]);
    }
}

Hydroponics_UnitsType HydroponicsDHTTempHumiditySensor::getMeasurementUnits(int measurementRow) const
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
    _computeHeatIndex = computeHeatIndex;
}

bool HydroponicsDHTTempHumiditySensor::getComputeHeatIndex() const
{
    return _computeHeatIndex;
}

void HydroponicsDHTTempHumiditySensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsDigitalSensor::saveToData(dataOut);

    ((HydroponicsDHTTempHumiditySensorData *)dataOut)->dhtType = _dhtType;
    ((HydroponicsDHTTempHumiditySensorData *)dataOut)->computeHeatIndex = _computeHeatIndex;
    ((HydroponicsDHTTempHumiditySensorData *)dataOut)->measurementUnits[0] = _measurementUnits[0];
    ((HydroponicsDHTTempHumiditySensorData *)dataOut)->measurementUnits[1] = _measurementUnits[1];
    ((HydroponicsDHTTempHumiditySensorData *)dataOut)->measurementUnits[2] = _measurementUnits[2];
}


HydroponicsDSTemperatureSensor::HydroponicsDSTemperatureSensor(Hydroponics_PositionIndex sensorIndex,
                                                               byte inputPin, byte inputBitRes,
                                                               byte pullupPin,
                                                               int classType)
    : HydroponicsDigitalSensor(Hydroponics_SensorType_WaterTemperature, sensorIndex, inputPin, inputBitRes, true, classType),
      _dt(new DallasTemperature()), _pullupPin(pullupPin), _measurementUnits(defaultTemperatureUnits())
{
    HYDRUINO_SOFT_ASSERT(_dt, F("DallasTemperature instance creation failure"));

    if (isValidPin(_inputPin) && _oneWire && _dt) {
        _dt->setOneWire(_oneWire);
        if (isValidPin(_pullupPin)) { _dt->setPullupPin(_pullupPin); }
        _dt->setWaitForConversion(true); // reads will be done in their own task, waits will delay and yield
        _dt->begin();
        if (_dt->getResolution() != inputBitRes) { _dt->setResolution(inputBitRes); }
        HYDRUINO_SOFT_ASSERT(_dt->getResolution() == inputBitRes, F("Resolved resolution mismatch with passed resolution"));
    } else if (_dt) { delete _dt; _dt = nullptr; }
}

HydroponicsDSTemperatureSensor::HydroponicsDSTemperatureSensor(const HydroponicsDSTemperatureSensorData *dataIn)
    : HydroponicsDigitalSensor(dataIn, true),
      _dt(new DallasTemperature()), _pullupPin(dataIn->pullupPin),
      _measurementUnits(definedUnitsElse(dataIn->measurementUnits, defaultTemperatureUnits()))
{
    HYDRUINO_SOFT_ASSERT(_dt, F("DallasTemperature instance creation failure"));

    if (isValidPin(_inputPin) && _oneWire && _dt) {
        _dt->setOneWire(_oneWire);
        if (isValidPin(_pullupPin)) { _dt->setPullupPin(_pullupPin); }
        _dt->setWaitForConversion(true); // reads will be done in their own task, waits will delay and yield
        _dt->begin();
        if (_dt->getResolution() != dataIn->inputBitRes) { _dt->setResolution(dataIn->inputBitRes); }
        HYDRUINO_SOFT_ASSERT(_dt->getResolution() == dataIn->inputBitRes, F("Resolved resolution mismatch with passed resolution"));
    } else if (_dt) { delete _dt; _dt = nullptr; }
}

HydroponicsDSTemperatureSensor::~HydroponicsDSTemperatureSensor()
{
    if (_dt) { delete _dt; _dt = nullptr; }
}

bool HydroponicsDSTemperatureSensor::takeMeasurement(bool override)
{
    if (!(_wirePosIndex >= 0)) { resolveDeviceAddress(); }

    if (_dt && _wirePosIndex >= 0 && (override || getNeedsPolling()) && !_isTakingMeasure) {
        _isTakingMeasure = true;

        #ifndef HYDRUINO_DISABLE_MULTITASKING
            if (scheduleObjectMethodCallWithTaskIdOnce(::getSharedPtr<HydroponicsDSTemperatureSensor>(this), &_takeMeasurement) != TASKMGR_INVALIDID) {
                return true;
            } else {
                _isTakingMeasure = false;
            }
        #else
            _takeMeasurement(TASKMGR_INVALIDID);
        #endif
    }
    return false;
}

void HydroponicsDSTemperatureSensor::_takeMeasurement(taskid_t taskId)
{
    if (_isTakingMeasure && _dt) {
        if (getHydroponicsInstance()->tryGetPinLock(_inputPin, 5)) {
            if (_dt->requestTemperaturesByAddress(_wireDevAddress)) {
                Hydroponics_UnitsType unitsOut = definedUnitsElse(_measurementUnits, _calibrationData ? _calibrationData->calibUnits : Hydroponics_UnitsType_Undefined, defaultTemperatureUnits());
                bool readInFahrenheit = _measurementUnits == Hydroponics_UnitsType_Temperature_Fahrenheit;
                Hydroponics_UnitsType readUnits = readInFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit
                                                                   : Hydroponics_UnitsType_Temperature_Celsius;

                auto tempRead = readInFahrenheit ? _dt->getTempF(_wireDevAddress) : _dt->getTempC(_wireDevAddress);
                auto timestamp = now();

                HydroponicsSingleMeasurement newMeasurement(
                    tempRead,
                    readUnits,
                    timestamp
                );

                bool deviceDisconnected = isFPEqual(tempRead, (float)(readInFahrenheit ? DEVICE_DISCONNECTED_F : DEVICE_DISCONNECTED_C));
                HYDRUINO_SOFT_ASSERT(!deviceDisconnected, F("Measurement failed, device disconnected"));

                if (!deviceDisconnected) {
                    if (_calibrationData) {
                        _calibrationData->transform(&newMeasurement.value, &newMeasurement.units);
                    }

                    convertUnits(&newMeasurement.value, &newMeasurement.units, unitsOut);

                    _lastMeasurement = newMeasurement;
                    scheduleSignalFireOnce<const HydroponicsMeasurement *>(getSharedPtr(), _measureSignal, &_lastMeasurement);
                }
            } else {
                HYDRUINO_SOFT_ASSERT(false, F("Measurement failed, device disconnected"));
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
    }
}

const HydroponicsMeasurement *HydroponicsDSTemperatureSensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsDSTemperatureSensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow)
{
    if (_measurementUnits != measurementUnits) {
        _measurementUnits = measurementUnits;

        convertUnits(&_lastMeasurement, _measurementUnits);
    }
}

Hydroponics_UnitsType HydroponicsDSTemperatureSensor::getMeasurementUnits(int measurementRow) const
{
    return _measurementUnits;
}

byte HydroponicsDSTemperatureSensor::getPullupPin() const
{
    return _pullupPin;
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

    if (isValidPin(inputPin)) { objectOut[F("inputPin")] = inputPin; }
    if (cropName[0]) { objectOut[F("cropName")] = stringFromChars(cropName, HYDRUINO_NAME_MAXSIZE); }
    if (reservoirName[0]) { objectOut[F("reservoirName")] = stringFromChars(reservoirName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsObjectData::fromJSONObject(objectIn);

    inputPin = objectIn[F("inputPin")] | inputPin;
    const char *cropNameStr = objectIn[F("cropName")];
    if (cropNameStr && cropNameStr[0]) { strncpy(cropName, cropNameStr, HYDRUINO_NAME_MAXSIZE); }
    const char *reservoirNameStr = objectIn[F("reservoirName")];
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

    if (activeLow != true) { objectOut[F("activeLow")] = activeLow; }
    if (usingISR != false) { objectOut[F("usingISR")] = usingISR; }
}

void HydroponicsBinarySensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSensorData::fromJSONObject(objectIn);

    activeLow = objectIn[F("activeLow")] | activeLow;
    usingISR = objectIn[F("usingISR")] | usingISR;
}

HydroponicsAnalogSensorData::HydroponicsAnalogSensorData()
    : HydroponicsSensorData(), inputBitRes(8), inputInversion(false), measurementUnits(Hydroponics_UnitsType_Undefined), waterTempSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsAnalogSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSensorData::toJSONObject(objectOut);

    if (inputBitRes != 8) { objectOut[F("inputBitRes")] = inputBitRes; }
    if (inputInversion != false) { objectOut[F("inputInversion")] = inputInversion; }
    if (measurementUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("measurementUnits")] = measurementUnits; }
    if (waterTempSensorName[0]) { objectOut[F("waterTempSensorName")] = stringFromChars(waterTempSensorName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsAnalogSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSensorData::fromJSONObject(objectIn);

    inputBitRes = objectIn[F("inputBitRes")] | inputBitRes;
    inputInversion = objectIn[F("inputInversion")] | inputInversion;
    measurementUnits = objectIn[F("measurementUnits")] | measurementUnits;
    const char *waterTempSensorNameStr = objectIn[F("waterTempSensorName")];
    if (waterTempSensorNameStr && waterTempSensorNameStr[0]) { strncpy(waterTempSensorName, waterTempSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsDigitalSensorData::HydroponicsDigitalSensorData()
    : HydroponicsSensorData(), inputBitRes(9), wirePosIndex(-1), wireDevAddress{0}, waterTempSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsDigitalSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSensorData::toJSONObject(objectOut);

    if (inputBitRes != 9) { objectOut[F("inputBitRes")] = inputBitRes; }
    if (wirePosIndex >= 0) { objectOut[F("wirePosIndex")] = wirePosIndex; }
    if (!arrayElementsEqual<uint8_t>(wireDevAddress, 8, 0)) { objectOut[F("wireDevAddress")] = hexStringFromBytes(wireDevAddress, 8); }
    if (waterTempSensorName[0]) { objectOut[F("waterTempSensorName")] = stringFromChars(waterTempSensorName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsDigitalSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSensorData::fromJSONObject(objectIn);

    inputBitRes = objectIn[F("inputBitRes")] | inputBitRes;
    wirePosIndex = objectIn[F("wirePosIndex")] | wirePosIndex;
    JsonVariantConst wireDevAddressVar = objectIn[F("wireDevAddress")];
    hexStringToBytes(wireDevAddressVar, wireDevAddress, 8);
    for (int addrIndex = 0; addrIndex < 8; ++addrIndex) { 
        wireDevAddress[addrIndex] = wireDevAddressVar[addrIndex] | wireDevAddress[addrIndex];
    }
    const char *waterTempSensorNameStr = objectIn[F("waterTempSensorName")];
    if (waterTempSensorNameStr && waterTempSensorNameStr[0]) { strncpy(waterTempSensorName, waterTempSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsDHTTempHumiditySensorData::HydroponicsDHTTempHumiditySensorData()
    : HydroponicsDigitalSensorData(), dhtType(DHT12), computeHeatIndex(false), measurementUnits{Hydroponics_UnitsType_Undefined,Hydroponics_UnitsType_Undefined,Hydroponics_UnitsType_Undefined}
{
    _size = sizeof(*this);
}

void HydroponicsDHTTempHumiditySensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsDigitalSensorData::toJSONObject(objectOut);

    if (dhtType != DHT12) { objectOut[F("dhtType")] = dhtType; }
    if (computeHeatIndex != false) { objectOut[F("computeHeatIndex")] = computeHeatIndex; }
    if (measurementUnits[0] != Hydroponics_UnitsType_Undefined || measurementUnits[1] != Hydroponics_UnitsType_Undefined || measurementUnits[2] != Hydroponics_UnitsType_Undefined) {
        if (!(measurementUnits[0] == measurementUnits[1] && measurementUnits[0] == measurementUnits[2])) {
            objectOut[F("measurementUnits")] = commaStringFromArray(measurementUnits, 3);
        } else {
            objectOut[F("measurementUnits")] = measurementUnits[0];
        }
    }
}

void HydroponicsDHTTempHumiditySensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsDigitalSensorData::fromJSONObject(objectIn);

    dhtType = objectIn[F("dhtType")] | dhtType;
    computeHeatIndex = objectIn[F("computeHeatIndex")] | computeHeatIndex;
    JsonVariantConst measurementUnitsVar = objectIn[F("measurementUnits")];
    commaStringToArray(measurementUnitsVar, measurementUnits, 3);
    measurementUnits[0] = measurementUnitsVar[0] | measurementUnits[0];
    measurementUnits[1] = measurementUnitsVar[1] | measurementUnits[1];
    measurementUnits[2] = measurementUnitsVar[2] | measurementUnits[2];
}

HydroponicsDSTemperatureSensorData::HydroponicsDSTemperatureSensorData()
    : HydroponicsDigitalSensorData(), pullupPin(-1), measurementUnits(Hydroponics_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroponicsDSTemperatureSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsDigitalSensorData::toJSONObject(objectOut);

    if (isValidPin(pullupPin)) { objectOut[F("pullupPin")] = pullupPin; }
    if (measurementUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("measurementUnits")] = measurementUnits; }
}

void HydroponicsDSTemperatureSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsDigitalSensorData::fromJSONObject(objectIn);

    pullupPin = objectIn[F("pullupPin")] | pullupPin;
    measurementUnits = objectIn[F("measurementUnits")] | measurementUnits;
}
