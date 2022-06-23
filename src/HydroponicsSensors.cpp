/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensorsare 
*/

#include "Hydroponics.h"

Hydroponics_UnitsType defaultSensorMeasurementUnits(Hydroponics_SensorType sensorType, Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        auto hydroponics = getHydroponicsInstance();
        measureMode = (hydroponics ? hydroponics->getMeasurementMode() : Hydroponics_MeasurementMode_Default);
    }

    switch (sensorType) {
        case Hydroponics_SensorType_AirCarbonDioxide:
            return Hydroponics_UnitsType_Concentration_PPM500;  // FIXME: dont think this is right, will need to verify
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
        default:
            return Hydroponics_UnitsType_Undefined;
    }
}


HydroponicsSensor::HydroponicsSensor(Hydroponics_SensorType sensorType,
                                     Hydroponics_PositionIndex sensorIndex,
                                     byte inputPin,
                                     int classTypeIn)
    : HydroponicsObject(HydroponicsIdentity(sensorType, sensorIndex)), classType((typeof(classType))classTypeIn),
      _inputPin(inputPin), _measureSignal()
{ ; }

HydroponicsSensor::~HydroponicsSensor()
{
    //discardFromTaskManager(&_measureSignal);
    if (_crop) { _crop->removeSensor(this); }
    if (_reservoir) { _reservoir->removeSensor(this); }
}

void HydroponicsSensor::resolveLinks()
{
    HydroponicsObject::resolveLinks();
    if (_crop.needsResolved()) { getCrop(); }
    if (_reservoir.needsResolved()) { getReservoir(); }
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

byte HydroponicsSensor::getInputPin() const
{
    return _inputPin;
}

Hydroponics_SensorType HydroponicsSensor::getSensorType() const
{
    return _id.as.sensorType;
}

Hydroponics_PositionIndex HydroponicsSensor::getSensorIndex() const
{
    return _id.posIndex;
}

Signal<HydroponicsMeasurement *> &HydroponicsSensor::getMeasurementSignal()
{
    return _measureSignal;
}


HydroponicsBinarySensor::HydroponicsBinarySensor(Hydroponics_SensorType sensorType,
                                                 Hydroponics_PositionIndex sensorIndex,
                                                 byte inputPin,
                                                 bool activeLow,
                                                 int classType)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin, classType),
      _activeLow(activeLow)
{
    pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
}

HydroponicsBinarySensor::~HydroponicsBinarySensor()
{
    //discardFromTaskManager(&_stateSignal);
}

HydroponicsMeasurement *HydroponicsBinarySensor::getMeasurement(bool force)
{
    auto hydroponics = getHydroponicsInstance();
    force = force || (hydroponics ? hydroponics->isPollingFrameOld(_lastMeasurement.frame) : false);

    if (force) {
        auto rawRead = digitalRead(_inputPin);
        auto timestamp = now();

        HydroponicsBinaryMeasurement newMeasurement(
            !!rawRead,
            timestamp
        );

        bool stateChanged = _lastMeasurement.state != newMeasurement.state;
        _lastMeasurement = newMeasurement;

        scheduleSignalFireOnce<HydroponicsMeasurement *>(_measureSignal, &_lastMeasurement);
        if (stateChanged) {
            scheduleSignalFireOnce<bool>(_stateSignal, _lastMeasurement.state);
        }
    }

    return &_lastMeasurement;
}

HydroponicsMeasurement *HydroponicsBinarySensor::getLastMeasurement()
{
    return &_lastMeasurement;
}

bool HydroponicsBinarySensor::getActiveLow() const
{
    return _activeLow;
}

Signal<bool> &HydroponicsBinarySensor::getStateSignal()
{
    return _stateSignal;
}


HydroponicsAnalogSensor::HydroponicsAnalogSensor(Hydroponics_SensorType sensorType,
                                                 Hydroponics_PositionIndex sensorIndex,
                                                 byte inputPin,
                                                 byte inputBitRes,
                                                 int classType)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin, classType),
      _inputResolution(inputBitRes), _lastMeasurement(), _inputCalib(nullptr)
{
    pinMode(_inputPin, INPUT);
}

HydroponicsAnalogSensor::~HydroponicsAnalogSensor()
{ ; }

void HydroponicsAnalogSensor::resolveLinks()
{
    HydroponicsSensor::resolveLinks();
    if (_tempSensor.needsResolved()) { getTemperatureSensor(); }
}

HydroponicsMeasurement *HydroponicsAnalogSensor::getMeasurement(bool force)
{
    auto hydroponics = getHydroponicsInstance();
    force = force || (hydroponics ? hydroponics->isPollingFrameOld(_lastMeasurement.frame) : false);

    if (force) {
        Hydroponics_UnitsType unitsOut = _measurementUnits != Hydroponics_UnitsType_Undefined ? _measurementUnits
                                                                                              : (_inputCalib && _inputCalib->calibUnits != Hydroponics_UnitsType_Undefined ? _inputCalib->calibUnits
                                                                                                                                                                           : defaultSensorMeasurementUnits(_id.as.sensorType));

        #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
            analogReadResolution(_inputResolution.bitRes);
        #endif

        auto rawRead = analogRead(_inputPin);
        auto timestamp = now();

        HydroponicsSingleMeasurement newMeasurement(
            _inputResolution.transform(rawRead),
            Hydroponics_UnitsType_Raw_0_1,
            timestamp
        );

        if (_inputCalib) {
            newMeasurement.value = _inputCalib->transform(newMeasurement.value);
            newMeasurement.units = _inputCalib->calibUnits;
        }

        convertStdUnits(&newMeasurement.value, &newMeasurement.units, unitsOut);

        _lastMeasurement = newMeasurement;
        scheduleSignalFireOnce<HydroponicsMeasurement *>(_measureSignal, &_lastMeasurement);
    }
    
    return &_lastMeasurement;
}

HydroponicsMeasurement *HydroponicsAnalogSensor::getLastMeasurement()
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

HydroponicsCalibrationData *HydroponicsAnalogSensor::loadInputCalibration(bool create)
{
    // TODO
}

void HydroponicsAnalogSensor::saveInputCalibration(HydroponicsCalibrationData *newData)
{
    // TODO
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


HydroponicsDigitalSensor::HydroponicsDigitalSensor(Hydroponics_SensorType sensorType,
                                                   Hydroponics_PositionIndex sensorIndex,
                                                   byte inputPin,
                                                   int classType)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin, classType)
{ ; }

HydroponicsDigitalSensor::~HydroponicsDigitalSensor()
{ ; }


HydroponicsDHTTempHumiditySensor::HydroponicsDHTTempHumiditySensor(Hydroponics_PositionIndex sensorIndex,
                                                                   byte inputPin,
                                                                   uint8_t dhtType,
                                                                   bool computeHeatIndex,
                                                                   int classType)
    : HydroponicsDigitalSensor(Hydroponics_SensorType_AirTempHumidity, sensorIndex, inputPin, classType),
      _dht(new DHT(inputPin, dhtType)), _computeHeatIndex(computeHeatIndex)
{
    HYDRUINO_SOFT_ASSERT(_dht, "DHT instance creation failure");
    if (_dht) {
        _dht->begin();
    }
}

HydroponicsDHTTempHumiditySensor::~HydroponicsDHTTempHumiditySensor()
{
    if (_dht) { delete _dht; _dht = nullptr; }
}

HydroponicsMeasurement *HydroponicsDHTTempHumiditySensor::getMeasurement(bool force)
{
    auto hydroponics = getHydroponicsInstance();
    force = force || (hydroponics ? hydroponics->isPollingFrameOld(_lastMeasurement.frame) : false);

    if (force) {
        Hydroponics_UnitsType unitsOut[3] = { _measurementUnits[0] != Hydroponics_UnitsType_Undefined ? _measurementUnits[0] : defaultSensorMeasurementUnits(_id.as.sensorType),
                                              _measurementUnits[1] != Hydroponics_UnitsType_Undefined ? _measurementUnits[1] : Hydroponics_UnitsType_Percentile_0_100,
                                              _measurementUnits[2] != Hydroponics_UnitsType_Undefined ? _measurementUnits[2] : defaultSensorMeasurementUnits(_id.as.sensorType) };
        bool readInFahrenheit = unitsOut[0] == Hydroponics_UnitsType_Temperature_Fahrenheit;
        Hydroponics_UnitsType readUnits = readInFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit
                                                           : Hydroponics_UnitsType_Temperature_Celsius;

        auto tempRead = _dht->readTemperature(readInFahrenheit, force);
        auto humidRead = _dht->readHumidity(force);
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
        scheduleSignalFireOnce<HydroponicsMeasurement *>(_measureSignal, &_lastMeasurement);
    }

    return &_lastMeasurement;
}

HydroponicsMeasurement *HydroponicsDHTTempHumiditySensor::getLastMeasurement()
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


HydroponicsDSTemperatureSensor::HydroponicsDSTemperatureSensor(Hydroponics_PositionIndex sensorIndex,
                                                               byte inputPin, byte inputBitRes,
                                                               int classType)
    : HydroponicsDigitalSensor(Hydroponics_SensorType_WaterTemperature, sensorIndex, inputPin, classType),
      _oneWire(new OneWire(inputPin)), _dt(new DallasTemperature())
{
    HYDRUINO_SOFT_ASSERT(_oneWire, "OneWire instance creation failure");
    HYDRUINO_SOFT_ASSERT(_dt, "DallasTemperature instance creation failure");

    if (_dt && _oneWire) {
        _dt->setOneWire(_oneWire);
        //_dt->setPullupPin(pullupPin); // TODO: needed?
        _dt->setWaitForConversion(true); // This makes calls blocking
        _dt->begin();
        _dt->setResolution(inputBitRes);
        HYDRUINO_SOFT_ASSERT(_dt->getResolution() == inputBitRes, "Resolved resolution mismatch with passed resolution");
    }
}

HydroponicsDSTemperatureSensor::~HydroponicsDSTemperatureSensor()
{
    if (_oneWire) { delete _oneWire; _oneWire = nullptr; }
    if (_dt) { delete _dt; _dt = nullptr; }
}

HydroponicsMeasurement *HydroponicsDSTemperatureSensor::getMeasurement(bool force)
{
    auto hydroponics = getHydroponicsInstance();
    force = force || (hydroponics ? hydroponics->isPollingFrameOld(_lastMeasurement.frame) : false);

    if (force) {
        Hydroponics_UnitsType unitsOut = _measurementUnits != Hydroponics_UnitsType_Undefined ? _measurementUnits : defaultSensorMeasurementUnits(_id.as.sensorType);
        bool readInFahrenheit = _measurementUnits == Hydroponics_UnitsType_Temperature_Fahrenheit;
        Hydroponics_UnitsType readUnits = readInFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit :
                                                            Hydroponics_UnitsType_Temperature_Celsius;

        auto tempRead = readInFahrenheit ? _dt->getTempFByIndex(0) : _dt->getTempCByIndex(0); // TODO: Multiple devices on same OneWire line
        auto timestamp = now();

        HydroponicsSingleMeasurement newMeasurement(
            tempRead,
            readUnits,
            timestamp
        );

        bool deviceDisconnected = isFPEqual(tempRead, (readInFahrenheit ? DEVICE_DISCONNECTED_F : DEVICE_DISCONNECTED_C));
        HYDRUINO_SOFT_ASSERT(!deviceDisconnected, "Measurement failed device disconnected");

        if (!deviceDisconnected) {
            convertStdUnits(&newMeasurement.value, &newMeasurement.units, unitsOut);

            _lastMeasurement = newMeasurement;
            scheduleSignalFireOnce<HydroponicsMeasurement *>(_measureSignal, &_lastMeasurement);
        }
    }

    return &_lastMeasurement;
}

HydroponicsMeasurement *HydroponicsDSTemperatureSensor::getLastMeasurement()
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


HydroponicsTMPSoilMoistureSensor::HydroponicsTMPSoilMoistureSensor(Hydroponics_PositionIndex sensorIndex,
                                                                   byte inputPin, byte inputBitRes,
                                                                   int classType)
    : HydroponicsDigitalSensor(Hydroponics_SensorType_SoilMoisture, sensorIndex, inputPin, classType),
      _oneWire(new OneWire(inputPin))
{
    HYDRUINO_SOFT_ASSERT(_oneWire, "OneWire instance creation failure");
}

HydroponicsTMPSoilMoistureSensor::~HydroponicsTMPSoilMoistureSensor()
{
    if (_oneWire) { delete _oneWire; _oneWire = nullptr; }
}

HydroponicsMeasurement *HydroponicsTMPSoilMoistureSensor::getMeasurement(bool force)
{
    auto hydroponics = getHydroponicsInstance();
    force = force || (hydroponics ? hydroponics->isPollingFrameOld(_lastMeasurement.frame) : false);

    if (force) {
        Hydroponics_UnitsType unitsOut = _measurementUnits != Hydroponics_UnitsType_Undefined ? _measurementUnits : defaultSensorMeasurementUnits(_id.as.sensorType);

        //auto rawRead = TODO();
        //auto timestamp = now();

        //HydroponicsSingleMeasurement
        //    newMeasurement(rawRead, Hydroponics_UnitsType_Raw_0_1, timestamp);

        //convertStdUnits(&newMeasurement.value, &newMeasurement.units, unitsOut);

        //_lastMeasurement = newMeasurement;
        scheduleSignalFireOnce<HydroponicsMeasurement *>(_measureSignal, &_lastMeasurement);
    }

    return &_lastMeasurement;
}

HydroponicsMeasurement *HydroponicsTMPSoilMoistureSensor::getLastMeasurement()
{
    return &_lastMeasurement;
}

void HydroponicsTMPSoilMoistureSensor::setMeasurementUnits(Hydroponics_UnitsType measurementUnits, int measurementRow)
{
    _measurementUnits = measurementUnits;
}

Hydroponics_UnitsType HydroponicsTMPSoilMoistureSensor::getMeasurementUnits(int measurementRow) const
{
    return _measurementUnits;
}

OneWire &HydroponicsTMPSoilMoistureSensor::getOneWire() const
{
    return *_oneWire;
}
