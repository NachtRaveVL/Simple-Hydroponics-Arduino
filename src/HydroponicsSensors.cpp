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
            case 5: // TMP1W
                return new HydroponicsTMPMoistureSensor((const HydroponicsTMPMoistureSensorData *)dataIn);
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
        case Hydroponics_SensorType_AirCarbonDioxide:
            return Hydroponics_UnitsType_Concentration_PPM;
        case Hydroponics_SensorType_PotentialHydrogen:
            return Hydroponics_UnitsType_pHScale_0_14;
        case Hydroponics_SensorType_TotalDissolvedSolids:
        case Hydroponics_SensorType_SoilMoisture:
            return defaultConcentrationUnits(measureMode);
        case Hydroponics_SensorType_WaterTemperature:
            return defaultTemperatureUnits(measureMode);
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
      _activeLow(activeLow)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
    }
}

HydroponicsBinarySensor::HydroponicsBinarySensor(const HydroponicsBinarySensorData *dataIn)
    : HydroponicsSensor(dataIn), _activeLow(dataIn->activeLow)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
    }
}

HydroponicsBinarySensor::~HydroponicsBinarySensor()
{
    //discardFromTaskManager(&_stateSignal);
}

void HydroponicsBinarySensor::takeMeasurement(bool override)
{
    if (isValidPin(_inputPin) && (override || getNeedsPolling())) {
        _isTakingMeasure = true;

        auto rawRead = digitalRead(_inputPin);
        auto timestamp = now();

        HydroponicsBinaryMeasurement newMeasurement(!!rawRead, timestamp);

        bool stateChanged = _lastMeasurement.state != newMeasurement.state;
        _lastMeasurement = newMeasurement;

        scheduleSignalFireOnce<const HydroponicsMeasurement *>(_measureSignal, &_lastMeasurement);
        if (stateChanged) {
            scheduleSignalFireOnce<bool>(_stateSignal, _lastMeasurement.state);
        }

        _isTakingMeasure = false;
    }
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

bool HydroponicsBinarySensor::getActiveLow() const
{
    return _activeLow;
}

Signal<bool> &HydroponicsBinarySensor::getStateSignal()
{
    return _stateSignal;
}

void HydroponicsBinarySensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsSensor::saveToData(dataOut);

    ((HydroponicsBinarySensorData *)dataOut)->activeLow = _activeLow;
}


HydroponicsAnalogSensor::HydroponicsAnalogSensor(Hydroponics_SensorType sensorType,
                                                 Hydroponics_PositionIndex sensorIndex,
                                                 byte inputPin, byte inputBitRes,
                                                 int classType)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin, classType),
      _inputResolution(inputBitRes), _measurementUnits(defaultMeasureUnitsForSensorType(sensorType))
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (isValidPin(_inputPin)) {
        pinMode(_inputPin, INPUT);
    }
}

HydroponicsAnalogSensor::HydroponicsAnalogSensor(const HydroponicsAnalogSensorData *dataIn)
    : HydroponicsSensor(dataIn),
      _inputResolution(dataIn->inputBitRes), _measurementUnits(dataIn->measurementUnits),
      _tempSensor(dataIn->tempSensorName)
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

void HydroponicsAnalogSensor::takeMeasurement(bool override)
{
    if (isValidPin(_inputPin) && (override || getNeedsPolling())) {
        _isTakingMeasure = true;

        Hydroponics_UnitsType unitsOut = _measurementUnits != Hydroponics_UnitsType_Undefined ? _measurementUnits
                                                                                              : (_calibrationData && _calibrationData->calibUnits != Hydroponics_UnitsType_Undefined ? _calibrationData->calibUnits
                                                                                                                                                                                     : defaultMeasureUnitsForSensorType(_id.objTypeAs.sensorType));

        #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
            analogReadResolution(_inputResolution.bitRes);
        #endif

        auto rawRead = analogRead(_inputPin); // TODO: switch to analog read async lib?
        auto timestamp = now();

        HydroponicsSingleMeasurement newMeasurement(
            _inputResolution.transform(rawRead),
            Hydroponics_UnitsType_Raw_0_1,
            timestamp
        );

        if (_calibrationData) {
            newMeasurement.value = _calibrationData->transform(newMeasurement.value);
            newMeasurement.units = _calibrationData->calibUnits;
        }

        convertStdUnits(&newMeasurement.value, &newMeasurement.units, unitsOut);

        _lastMeasurement = newMeasurement;
        scheduleSignalFireOnce<const HydroponicsMeasurement *>(_measureSignal, &_lastMeasurement);

        _isTakingMeasure = false;
    }
}

const HydroponicsMeasurement *HydroponicsAnalogSensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsAnalogSensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow)
{
    _measurementUnits = measurementUnits;
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

void HydroponicsAnalogSensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsSensor::saveToData(dataOut);

    if (_tempSensor.getId()) {
        strncpy(((HydroponicsAnalogSensorData *)dataOut)->tempSensorName, _tempSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}


HydroponicsDigitalSensor::HydroponicsDigitalSensor(Hydroponics_SensorType sensorType,
                                                   Hydroponics_PositionIndex sensorIndex,
                                                   byte inputPin,
                                                   bool allocate1W,
                                                   int classType)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin, classType), _oneWire(nullptr)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (allocate1W && isValidPin(_inputPin)) {
        _oneWire = new OneWire(_inputPin);
        HYDRUINO_SOFT_ASSERT(_oneWire, F("Failure creating OneWire instance"));
    }
}

HydroponicsDigitalSensor::HydroponicsDigitalSensor(const HydroponicsDigitalSensorData *dataIn, bool allocate1W)
    : HydroponicsSensor(dataIn), _oneWire(nullptr)
{
    HYDRUINO_HARD_ASSERT(isValidPin(_inputPin), F("Invalid input pin"));
    if (allocate1W && isValidPin(_inputPin)) {
        _oneWire = new OneWire(_inputPin);
        HYDRUINO_SOFT_ASSERT(_oneWire, F("Failure creating OneWire instance"));
    }
}

HydroponicsDigitalSensor::~HydroponicsDigitalSensor()
{
    if (_oneWire) { delete _oneWire; }
}

void HydroponicsDigitalSensor::resolveLinks()
{
    HydroponicsSensor::resolveLinks();

    if (_tempSensor.needsResolved()) { getTemperatureSensor(); }
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

void HydroponicsDigitalSensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsSensor::saveToData(dataOut);

    if (_tempSensor.getId()) {
        strncpy(((HydroponicsAnalogSensorData *)dataOut)->tempSensorName, _tempSensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
}


HydroponicsDHTTempHumiditySensor::HydroponicsDHTTempHumiditySensor(Hydroponics_PositionIndex sensorIndex,
                                                                   byte inputPin,
                                                                   byte dhtType,
                                                                   bool computeHeatIndex,
                                                                   int classType)
    : HydroponicsDigitalSensor(Hydroponics_SensorType_AirTempHumidity, sensorIndex, inputPin, false, classType),
      _dht(new DHT(inputPin, dhtType)), _dhtType(dhtType), _computeHeatIndex(computeHeatIndex),
      _measurementUnits{defaultTemperatureUnits(), Hydroponics_UnitsType_Percentile_0_100, defaultTemperatureUnits()}
{
    HYDRUINO_SOFT_ASSERT(_dht, F("Failure creating DHT instance"));
    if (_dht && isValidPin(_inputPin)) { _dht->begin(); }
}

HydroponicsDHTTempHumiditySensor::HydroponicsDHTTempHumiditySensor(const HydroponicsDHTTempHumiditySensorData *dataIn)
    : HydroponicsDigitalSensor(dataIn, false),
      _dht(new DHT(dataIn->inputPin, dataIn->dhtType)), _dhtType(dataIn->dhtType), _computeHeatIndex(dataIn->computeHeatIndex),
      _measurementUnits{dataIn->measurementUnits[0],dataIn->measurementUnits[1],dataIn->measurementUnits[2]}
{
    HYDRUINO_SOFT_ASSERT(_dht, F("Failure creating DHT instance"));
    if (_dht && isValidPin(_inputPin)) { _dht->begin(); }
}

HydroponicsDHTTempHumiditySensor::~HydroponicsDHTTempHumiditySensor()
{
    if (_dht) { delete _dht; _dht = nullptr; }
}

void HydroponicsDHTTempHumiditySensor::takeMeasurement(bool override)
{
    if (isValidPin(_inputPin) && (override || getNeedsPolling())) {
        _isTakingMeasure = true;

        Hydroponics_UnitsType unitsOut[3] = { _measurementUnits[0] != Hydroponics_UnitsType_Undefined ? _measurementUnits[0] : defaultTemperatureUnits(),
                                              _measurementUnits[1] != Hydroponics_UnitsType_Undefined ? _measurementUnits[1] : Hydroponics_UnitsType_Percentile_0_100,
                                              _measurementUnits[2] != Hydroponics_UnitsType_Undefined ? _measurementUnits[2] : defaultTemperatureUnits() };
        bool readInFahrenheit = unitsOut[0] == Hydroponics_UnitsType_Temperature_Fahrenheit;
        Hydroponics_UnitsType readUnits = readInFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit
                                                           : Hydroponics_UnitsType_Temperature_Celsius;

        auto tempRead = _dht->readTemperature(readInFahrenheit, true);
        auto humidRead = _dht->readHumidity(true);
        auto timestamp = now();

        HydroponicsTripleMeasurement newMeasurement(
            tempRead, readUnits, humidRead, Hydroponics_UnitsType_Percentile_0_100,
            _computeHeatIndex ? _dht->computeHeatIndex(tempRead, humidRead, readInFahrenheit) : 0.0f,
            _computeHeatIndex ? readUnits : Hydroponics_UnitsType_Undefined,
            timestamp
        );

        convertStdUnits(&newMeasurement.value[0], &newMeasurement.units[0], unitsOut[0]);
        convertStdUnits(&newMeasurement.value[1], &newMeasurement.units[1], unitsOut[1]);
        if (_computeHeatIndex) {
            convertStdUnits(&newMeasurement.value[2], &newMeasurement.units[2], unitsOut[2]);
        }

        _lastMeasurement = newMeasurement;
        scheduleSignalFireOnce<const HydroponicsMeasurement *>(_measureSignal, &_lastMeasurement);

        _isTakingMeasure = false;
    }
}

const HydroponicsMeasurement *HydroponicsDHTTempHumiditySensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsDHTTempHumiditySensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow)
{
    _measurementUnits[measurementRow] = measurementUnits;
}

Hydroponics_UnitsType HydroponicsDHTTempHumiditySensor::getMeasurementUnits(int measurementRow) const
{
    return _measurementUnits[measurementRow];
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
                                                               int classType)
    : HydroponicsDigitalSensor(Hydroponics_SensorType_WaterTemperature, sensorIndex, inputPin, true, classType),
      _dt(new DallasTemperature()), _measurementUnits(defaultTemperatureUnits())
{
    HYDRUINO_SOFT_ASSERT(_dt, F("DallasTemperature instance creation failure"));

    if (_dt && _oneWire && isValidPin(_inputPin)) {
        _dt->setOneWire(_oneWire);
        //_dt->setPullupPin(pullupPin); // TODO: needed?
        _dt->setWaitForConversion(true); // This makes calls blocking
        _dt->begin();
        _dt->setResolution(inputBitRes);
        HYDRUINO_SOFT_ASSERT(_dt->getResolution() == inputBitRes, F("Resolved resolution mismatch with passed resolution"));
    }
}

HydroponicsDSTemperatureSensor::HydroponicsDSTemperatureSensor(const HydroponicsDSTemperatureSensorData *dataIn)
    : HydroponicsDigitalSensor(dataIn, true),
      _dt(new DallasTemperature()), _measurementUnits(dataIn->measurementUnits)
{
    HYDRUINO_SOFT_ASSERT(_dt, F("DallasTemperature instance creation failure"));

    if (_dt && _oneWire && isValidPin(_inputPin)) {
        _dt->setOneWire(_oneWire);
        //_dt->setPullupPin(pullupPin); // TODO: needed?
        _dt->setWaitForConversion(true); // This makes calls blocking
        _dt->begin();
        _dt->setResolution(dataIn->inputBitRes);
        HYDRUINO_SOFT_ASSERT(_dt->getResolution() == dataIn->inputBitRes, F("Resolved resolution mismatch with passed resolution"));
    }
}

HydroponicsDSTemperatureSensor::~HydroponicsDSTemperatureSensor()
{
    if (_dt) { delete _dt; _dt = nullptr; }
}

void HydroponicsDSTemperatureSensor::takeMeasurement(bool override = false)
{
    if (isValidPin(_inputPin) && (override || getNeedsPolling())) {
        _isTakingMeasure = true;

        Hydroponics_UnitsType unitsOut = _measurementUnits != Hydroponics_UnitsType_Undefined ? _measurementUnits : defaultTemperatureUnits();
        bool readInFahrenheit = _measurementUnits == Hydroponics_UnitsType_Temperature_Fahrenheit;
        Hydroponics_UnitsType readUnits = readInFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit
                                                           : Hydroponics_UnitsType_Temperature_Celsius;

        auto tempRead = readInFahrenheit ? _dt->getTempFByIndex(0) : _dt->getTempCByIndex(0); // TODO: Multiple devices on same OneWire line
        auto timestamp = now();

        HydroponicsSingleMeasurement newMeasurement(
            tempRead,
            readUnits,
            timestamp
        );

        bool deviceDisconnected = isFPEqual(tempRead, (float)(readInFahrenheit ? DEVICE_DISCONNECTED_F : DEVICE_DISCONNECTED_C));
        HYDRUINO_SOFT_ASSERT(!deviceDisconnected, F("Measurement failed device disconnected"));

        if (!deviceDisconnected) {
            convertStdUnits(&newMeasurement.value, &newMeasurement.units, unitsOut);

            _lastMeasurement = newMeasurement;
            scheduleSignalFireOnce<const HydroponicsMeasurement *>(_measureSignal, &_lastMeasurement);
        }

        _isTakingMeasure = false;
    }
}

const HydroponicsMeasurement *HydroponicsDSTemperatureSensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsDSTemperatureSensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow)
{
    _measurementUnits = measurementUnits;
}

Hydroponics_UnitsType HydroponicsDSTemperatureSensor::getMeasurementUnits(int measurementRow) const
{
    return _measurementUnits;
}

OneWire &HydroponicsDSTemperatureSensor::getOneWire() const
{
    return *_oneWire;
}

void HydroponicsDSTemperatureSensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsDigitalSensor::saveToData(dataOut);

    ((HydroponicsDSTemperatureSensorData *)dataOut)->inputBitRes = _dt ? _dt->getResolution() : 9;
    ((HydroponicsDSTemperatureSensorData *)dataOut)->measurementUnits = _measurementUnits;
}


HydroponicsTMPMoistureSensor::HydroponicsTMPMoistureSensor(Hydroponics_PositionIndex sensorIndex,
                                                           byte inputPin, byte inputBitRes,
                                                           int classType)
    : HydroponicsDigitalSensor(Hydroponics_SensorType_SoilMoisture, sensorIndex, inputPin, true, classType),
      _inputBitRes(inputBitRes)
{ ; }

HydroponicsTMPMoistureSensor::HydroponicsTMPMoistureSensor(const HydroponicsTMPMoistureSensorData *dataIn)
    : HydroponicsDigitalSensor(dataIn, true), _inputBitRes(dataIn->inputBitRes)
{ ; }

HydroponicsTMPMoistureSensor::~HydroponicsTMPMoistureSensor()
{ ; }

void HydroponicsTMPMoistureSensor::takeMeasurement(bool override)
{
    if (isValidPin(_inputPin) && (override || getNeedsPolling())) {
        _isTakingMeasure = true;

        Hydroponics_UnitsType unitsOut = _measurementUnits != Hydroponics_UnitsType_Undefined ? _measurementUnits : defaultMeasureUnitsForSensorType(_id.objTypeAs.sensorType);

        //auto rawRead = TODO();
        //auto timestamp = now();

        //HydroponicsSingleMeasurement
        //    newMeasurement(rawRead, Hydroponics_UnitsType_Raw_0_1, timestamp);

        //convertStdUnits(&newMeasurement.value, &newMeasurement.units, unitsOut);

        //_lastMeasurement = newMeasurement;
        scheduleSignalFireOnce<const HydroponicsMeasurement *>(_measureSignal, &_lastMeasurement);

        _isTakingMeasure = false;
    }
}

const HydroponicsMeasurement *HydroponicsTMPMoistureSensor::getLatestMeasurement() const
{
    return &_lastMeasurement;
}

void HydroponicsTMPMoistureSensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow)
{
    _measurementUnits = measurementUnits;
}

Hydroponics_UnitsType HydroponicsTMPMoistureSensor::getMeasurementUnits(int measurementRow) const
{
    return _measurementUnits;
}

OneWire &HydroponicsTMPMoistureSensor::getOneWire() const
{
    return *_oneWire;
}

void HydroponicsTMPMoistureSensor::saveToData(HydroponicsData *dataOut)
{
    HydroponicsDigitalSensor::saveToData(dataOut);

    ((HydroponicsTMPMoistureSensorData *)dataOut)->inputBitRes = _inputBitRes;
    ((HydroponicsTMPMoistureSensorData *)dataOut)->measurementUnits = _measurementUnits;
}


HydroponicsSensorData::HydroponicsSensorData()
    : HydroponicsObjectData(), inputPin(-1), cropName{0}, reservoirName{0}
{
    _size = sizeof(*this);
}

void HydroponicsSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsObjectData::toJSONObject(objectOut);

    if (inputPin != -1) { objectOut[F("inputPin")] = inputPin; }
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
    : HydroponicsSensorData(), activeLow(true)
{
    _size = sizeof(*this);
}

void HydroponicsBinarySensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSensorData::toJSONObject(objectOut);

    objectOut[F("activeLow")] = activeLow;
}

void HydroponicsBinarySensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSensorData::fromJSONObject(objectIn);

    activeLow = objectIn[F("activeLow")] | activeLow;
}

HydroponicsAnalogSensorData::HydroponicsAnalogSensorData()
    : HydroponicsSensorData(), inputBitRes(8), measurementUnits(Hydroponics_UnitsType_Undefined), tempSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsAnalogSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSensorData::toJSONObject(objectOut);

    if (inputBitRes != 8) { objectOut[F("inputBitRes")] = inputBitRes; }
    if (measurementUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("measurementUnits")] = measurementUnits; }
    if (tempSensorName[0]) { objectOut[F("tempSensorName")] = stringFromChars(tempSensorName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsAnalogSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSensorData::fromJSONObject(objectIn);

    inputBitRes = objectIn[F("inputBitRes")] | inputBitRes;
    measurementUnits = objectIn[F("measurementUnits")] | measurementUnits;
    const char *tempSensorNameStr = objectIn[F("tempSensorName")];
    if (tempSensorNameStr && tempSensorNameStr[0]) { strncpy(tempSensorName, tempSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsDigitalSensorData::HydroponicsDigitalSensorData()
    : HydroponicsSensorData(), tempSensorName{0}
{
    _size = sizeof(*this);
}

void HydroponicsDigitalSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSensorData::toJSONObject(objectOut);

    if (tempSensorName[0]) { objectOut[F("tempSensorName")] = stringFromChars(tempSensorName, HYDRUINO_NAME_MAXSIZE); }
}

void HydroponicsDigitalSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSensorData::fromJSONObject(objectIn);

    const char *tempSensorNameStr = objectIn[F("tempSensorName")];
    if (tempSensorNameStr && tempSensorNameStr[0]) { strncpy(tempSensorName, tempSensorNameStr, HYDRUINO_NAME_MAXSIZE); }
}

HydroponicsDHTTempHumiditySensorData::HydroponicsDHTTempHumiditySensorData()
    : HydroponicsDigitalSensorData(), dhtType(DHT12), computeHeatIndex(false), measurementUnits{Hydroponics_UnitsType_Undefined,Hydroponics_UnitsType_Percentile_0_100,Hydroponics_UnitsType_Undefined}
{
    _size = sizeof(*this);
}

void HydroponicsDHTTempHumiditySensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsDigitalSensorData::toJSONObject(objectOut);

    if (dhtType != DHT12) { objectOut[F("dhtType")] = dhtType; }
    if (computeHeatIndex != false) { objectOut[F("computeHeatIndex")] = computeHeatIndex; }
    if (measurementUnits[0] != Hydroponics_UnitsType_Undefined || measurementUnits[1] != Hydroponics_UnitsType_Undefined || measurementUnits[2] != Hydroponics_UnitsType_Undefined) {
        if (measurementUnits[0] != measurementUnits[2] || measurementUnits[1] != Hydroponics_UnitsType_Percentile_0_100) {
            JsonArray measurementUnitsArray = objectOut.createNestedArray(F("measurementUnits"));
            measurementUnitsArray[0] = measurementUnits[0];
            measurementUnitsArray[1] = measurementUnits[1];
            measurementUnitsArray[2] = measurementUnits[2];
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
    : HydroponicsDigitalSensorData(), inputBitRes(9), measurementUnits(Hydroponics_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroponicsDSTemperatureSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsDigitalSensorData::toJSONObject(objectOut);

    if (inputBitRes != 9) { objectOut[F("inputBitRes")] = inputBitRes; }
    if (measurementUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("measurementUnits")] = measurementUnits; }
}

void HydroponicsDSTemperatureSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsDigitalSensorData::fromJSONObject(objectIn);

    inputBitRes = objectIn[F("inputBitRes")] | inputBitRes;
    measurementUnits = objectIn[F("measurementUnits")] | measurementUnits;
}

HydroponicsTMPMoistureSensorData::HydroponicsTMPMoistureSensorData()
    : HydroponicsDigitalSensorData(), inputBitRes(9), measurementUnits(Hydroponics_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void HydroponicsTMPMoistureSensorData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsDigitalSensorData::toJSONObject(objectOut);

    if (inputBitRes != 9) { objectOut[F("inputBitRes")] = inputBitRes; }
    if (measurementUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("measurementUnits")] = measurementUnits; }
}

void HydroponicsTMPMoistureSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsDigitalSensorData::fromJSONObject(objectIn);

    inputBitRes = objectIn[F("inputBitRes")] | inputBitRes;
    measurementUnits = objectIn[F("measurementUnits")] | measurementUnits;
}
