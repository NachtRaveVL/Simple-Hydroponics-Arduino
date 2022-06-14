/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensorsare 
*/

#include "Hydroponics.h"

Hydroponics_UnitsType getDefaultSensorUnits(Hydroponics_SensorType sensorType, Hydroponics_MeasurementMode measureMode)
{
    if (measureMode == Hydroponics_MeasurementMode_Undefined) {
        measureMode = Hydroponics::getActiveInstance()->getMeasurementMode();
    }

    switch (sensorType) {
        case Hydroponics_SensorType_AirCarbonDioxide:
            return Hydroponics_UnitsType_Concentration_PPM500;  // FIXME: dont think this is right
        case Hydroponics_SensorType_PotentialHydrogen:
            return Hydroponics_UnitsType_pHScale_0_14;
        case Hydroponics_SensorType_TotalDissolvedSolids:
        case Hydroponics_SensorType_SoilMoisture:
            return Hydroponics_UnitsType_Concentration_EC;
        case Hydroponics_SensorType_WaterTemperature:
            switch (measureMode) {
                case Hydroponics_MeasurementMode_Imperial:
                    return Hydroponics_UnitsType_Temperature_Fahrenheit;
                case Hydroponics_MeasurementMode_Metric:
                    return Hydroponics_UnitsType_Temperature_Celsius;
                case Hydroponics_MeasurementMode_Scientific:
                    return Hydroponics_UnitsType_Temperature_Kelvin;
                default: break;
            }
            break;
        case Hydroponics_SensorType_WaterPumpFlowSensor:
            switch (measureMode) {
                case Hydroponics_MeasurementMode_Imperial:
                    return Hydroponics_UnitsType_LiquidFlow_GallonsPerMin;
                case Hydroponics_MeasurementMode_Metric:
                case Hydroponics_MeasurementMode_Scientific:
                    return Hydroponics_UnitsType_LiquidFlow_LitersPerMin;
                default: break;
            }
            break;
        case Hydroponics_SensorType_WaterHeightMeter:
            switch (measureMode) {
                case Hydroponics_MeasurementMode_Imperial:
                    return Hydroponics_UnitsType_Distance_Feet;
                case Hydroponics_MeasurementMode_Metric:
                case Hydroponics_MeasurementMode_Scientific:
                    return Hydroponics_UnitsType_Distance_Meters;
                default: break;
            }
            break;
        default:
            break;
    }

    return Hydroponics_UnitsType_Undefined;
}

HydroponicsSensor::HydroponicsSensor(Hydroponics_SensorType sensorType,
                                     Hydroponics_PositionIndex sensorIndex,
                                     byte inputPin)
    : HydroponicsObject(HydroponicsIdentity(sensorType, sensorIndex)),
      _inputPin(inputPin)
{ ; }

HydroponicsSensor::~HydroponicsSensor()
{ ; }

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

HydroponicsBinarySensor::HydroponicsBinarySensor(Hydroponics_SensorType sensorType,
                                                 Hydroponics_PositionIndex sensorIndex,
                                                 byte inputPin,
                                                 bool activeLow)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin),
      _activeLow(activeLow)
{
    pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
}

HydroponicsBinarySensor::~HydroponicsBinarySensor()
{ ; }

HydroponicsSensorMeasurement *HydroponicsBinarySensor::takeMeasurement(bool force)
{
    auto rawRead = digitalRead(_inputPin);
    auto timestamp = now();

    HydroponicsBinarySensorMeasurement newMeasurement((bool)rawRead, timestamp);

    return &(_lastMeasurement = newMeasurement);
}

HydroponicsSensorMeasurement *HydroponicsBinarySensor::getLastMeasurement() const
{
    return &_lastMeasurement;
}

time_t HydroponicsBinarySensor::getLastMeasurementTime() const
{
    return _lastMeasurement.timestamp;
}

bool HydroponicsBinarySensor::getActiveLow() const
{
    return _activeLow;
}


HydroponicsAnalogSensor::HydroponicsAnalogSensor(Hydroponics_SensorType sensorType,
                                                 Hydroponics_PositionIndex sensorIndex,
                                                 byte inputPin,
                                                 byte inputBitRes)
    : HydroponicsSensor(sensorType, sensorIndex, inputPin),
      _inputResolution(inputBitRes), _lastMeasurement(), _inputCalib(nullptr)
{
    pinMode(_inputPin, INPUT);
}

HydroponicsAnalogSensor::~HydroponicsAnalogSensor()
{ ; }

HydroponicsSensorMeasurement *HydroponicsAnalogSensor::takeMeasurement(bool force)
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
        analogReadResolution(_inputResolution.bitRes);
    #endif

    auto rawRead = analogRead(_inputPin);
    auto timestamp = now();

    HydroponicsAnalogSensorMeasurement
        newMeasurement(_inputResolution.transform(rawRead), Hydroponics_UnitsType_Raw_0_1, timestamp);

    if (_inputCalib) {
        newMeasurement.value = _inputCalib->transform(newMeasurement.value);
        newMeasurement.units = _inputCalib->calibUnits;
    } else {
        convertStdUnits(&newMeasurement.value, &newMeasurement.units, getDefaultSensorUnits(_id.as.sensorType));
    }

    return &(_lastMeasurement = newMeasurement);
}

HydroponicsSensorMeasurement *HydroponicsAnalogSensor::getLastMeasurement() const
{
    return &_lastMeasurement;
}

time_t HydroponicsAnalogSensor::getLastMeasurementTime() const
{
    return _lastMeasurement.timestamp;
}

HydroponicsBitResolution HydroponicsAnalogSensor::getInputResolution() const
{
    return _inputResolution;
}


HydroponicsDHTOneWireSensor::HydroponicsDHTOneWireSensor(Hydroponics_PositionIndex sensorIndex,
                                                         byte inputPin,
                                                         uint8_t dhtType,
                                                         bool computeHeatIndex)
    : HydroponicsSensor(Hydroponics_SensorType_AirTempHumidity, sensorIndex, inputPin),
      _dht(new DHT(inputPin, dhtType)), _computeHeatIndex(computeHeatIndex)
{
    //assert(_dht && "DHT instance creation failure");
    if (_dht) {
        _dht->begin();
    }
}

HydroponicsDHTOneWireSensor::~HydroponicsDHTOneWireSensor()
{
    if (_dht) { delete _dht; _dht = nullptr; }
}

HydroponicsSensorMeasurement *HydroponicsDHTOneWireSensor::takeMeasurement(bool force)
{
    Hydroponics_UnitsType unitsOut = getDefaultSensorUnits(_id.as.sensorType);
    bool readInFahrenheit = unitsOut == Hydroponics_UnitsType_Temperature_Fahrenheit;
    Hydroponics_UnitsType readUnits = readInFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit :
                                                         Hydroponics_UnitsType_Temperature_Celsius;

    auto tempRead = _dht->readTemperature(readInFahrenheit, force);
    auto humidRead = _dht->readHumidity(force);
    auto timestamp = now();

    HydroponicsDHTOneWireSensorMeasurement
        newMeasurement(tempRead, readUnits, humidRead, Hydroponics_UnitsType_Percentile_0_100,
                       _computeHeatIndex ? _dht->computeHeatIndex(tempRead, humidRead, readInFahrenheit) : 0.0f,
                       _computeHeatIndex ? readUnits : Hydroponics_UnitsType_Undefined,
                       timestamp);

    convertStdUnits(&newMeasurement.temperature, &newMeasurement.temperatureUnits, unitsOut);
    if (_computeHeatIndex) {
        convertStdUnits(&newMeasurement.heatIndex, &newMeasurement.heatIndexUnits, unitsOut);
    }

    return &(_lastMeasurement = newMeasurement);
}

HydroponicsSensorMeasurement *HydroponicsDHTOneWireSensor::getLastMeasurement() const
{
    return &_lastMeasurement;
}

time_t HydroponicsDHTOneWireSensor::getLastMeasurementTime() const
{
    return _lastMeasurement.timestamp;
}

void HydroponicsDHTOneWireSensor::setComputeHeatIndex(bool computeHeatIndex)
{
    _computeHeatIndex = computeHeatIndex;
}

bool HydroponicsDHTOneWireSensor::getComputeHeatIndex()
{
    return _computeHeatIndex;
}


HydroponicsDSOneWireSensor::HydroponicsDSOneWireSensor(Hydroponics_PositionIndex sensorIndex,
                                                       byte inputPin,
                                                       byte inputBitRes)
    : HydroponicsSensor(Hydroponics_SensorType_WaterTemperature, sensorIndex, inputPin),
      _oneWire(new OneWire(inputPin)), _dt(new DallasTemperature())
{
    //assert(_oneWire && "OneWire instance creation failure");
    //assert(_dt && "DallasTemperature instance creation failure");

    if (_dt && _oneWire) {
        _dt->setOneWire(_oneWire);
        _dt->setPullupPin(inputPin); // TODO: needed?
        _dt->setWaitForConversion(true); // TODO: make measurement async
        _dt->begin();
        _dt->setResolution(inputBitRes);
        //assert(_dt->getResolution() == readBitResolution && "Resolved resolution mismatch with passed resolution");
    }
}

HydroponicsDSOneWireSensor::~HydroponicsDSOneWireSensor()
{
    if (_oneWire) { delete _oneWire; _oneWire = nullptr; }
    if (_dt) { delete _dt; _dt = nullptr; }
}

HydroponicsSensorMeasurement *HydroponicsDSOneWireSensor::takeMeasurement(bool force)
{
    Hydroponics_UnitsType unitsOut = getDefaultSensorUnits(_id.as.sensorType);
    bool readInFahrenheit = unitsOut == Hydroponics_UnitsType_Temperature_Fahrenheit;
    Hydroponics_UnitsType readUnits = readInFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit :
                                                         Hydroponics_UnitsType_Temperature_Celsius;

    _dt->requestTemperatures(); // Blocking

    auto tempRead = readInFahrenheit ? _dt->getTempFByIndex(0) : _dt->getTempCByIndex(0);
    auto timestamp = now();

    HydroponicsAnalogSensorMeasurement newMeasurement(tempRead, readUnits, timestamp);

    bool deviceDisconnected = isFPEqual(tempRead, (readInFahrenheit ? DEVICE_DISCONNECTED_F : DEVICE_DISCONNECTED_C));
    //assert(!deviceDisconnected && "Measurement failed device disconnected");

    if (!deviceDisconnected) {
        convertStdUnits(&newMeasurement.value, &newMeasurement.units, unitsOut);
        return &(_lastMeasurement = newMeasurement);
    }
    return &_lastMeasurement;
}

HydroponicsSensorMeasurement *HydroponicsDSOneWireSensor::getLastMeasurement() const
{
    return &_lastMeasurement;
}

time_t HydroponicsDSOneWireSensor::getLastMeasurementTime() const
{
    return _lastMeasurement.timestamp;
}

OneWire &HydroponicsDSOneWireSensor::getOneWire() const
{
    return *_oneWire;
}


HydroponicsTMPOneWireSensor::HydroponicsTMPOneWireSensor(Hydroponics_PositionIndex sensorIndex,
                                                         byte inputPin,
                                                         byte inputBitRes)
    : HydroponicsSensor(Hydroponics_SensorType_SoilMoisture, sensorIndex, inputPin),
      _oneWire(new OneWire(inputPin))
{
    //assert(_oneWire && "OneWire instance creation failure");
}

HydroponicsTMPOneWireSensor::~HydroponicsTMPOneWireSensor()
{
    if (_oneWire) { delete _oneWire; _oneWire = nullptr; }
}

HydroponicsSensorMeasurement *HydroponicsTMPOneWireSensor::takeMeasurement(bool force)
{
    Hydroponics_UnitsType unitsOut = getDefaultSensorUnits(_id.as.sensorType);

    //auto rawRead = TODO();
    //auto timestamp = now();

    //HydroponicsAnalogSensorMeasurement
    //    newMeasurement(rawRead, Hydroponics_UnitsType_Raw_0_1, timestamp);

    //convertStdUnits(&newMeasurement.value, &newMeasurement.units, unitsOut);

    return &_lastMeasurement;
}

HydroponicsSensorMeasurement *HydroponicsTMPOneWireSensor::getLastMeasurement() const
{
    return &_lastMeasurement;
}

time_t HydroponicsTMPOneWireSensor::getLastMeasurementTime() const
{
    return _lastMeasurement.timestamp;
}

OneWire &HydroponicsTMPOneWireSensor::getOneWire() const
{
    return *_oneWire;
}
