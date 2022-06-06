/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensorsare 
*/

#include "HydroponicsSensors.h"

HydroponicsSensor::HydroponicsSensor(Hydroponics_SensorType sensorType,
                                     Hydroponics_FluidReservoir fluidReservoir)
    : _sensorType(sensorType), _fluidReservoir(fluidReservoir),
      _lastMeasureTime(0)
{ ; }

HydroponicsSensor::~HydroponicsSensor()
{ ; }

Hydroponics_SensorType HydroponicsSensor::getSensorType() const
{
    return _sensorType;
}

Hydroponics_FluidReservoir HydroponicsSensor::getFluidReservoir() const
{
    return _fluidReservoir;
}

time_t HydroponicsSensor::getLastMeasurementTime() const
{
    return _lastMeasureTime;
}

HydroponicsAnalogSensor::HydroponicsAnalogSensor(byte inputPin,
                                                 Hydroponics_SensorType sensorType,
                                                 Hydroponics_FluidReservoir fluidReservoir,
                                                 byte readBitResolution)
    : HydroponicsSensor(sensorType, fluidReservoir),
      _inputPin(inputPin), _lastMeasurement(0),
      #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
          _analogBitRes(readBitResolution), _analogMaxAmount((1 << readBitResolution) - 1)
      #else
          _analogBitRes(8), _analogMaxAmount(255)
      #endif
{
    pinMode(_inputPin, INPUT);
}

HydroponicsAnalogSensor::~HydroponicsAnalogSensor()
{ ; }

float HydroponicsAnalogSensor::getLastMeasurement() const
{
    return _lastMeasurement;
}

float HydroponicsAnalogSensor::takeMeasurement()
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
        analogReadResolution(_analogBitRes);
    #endif
    _lastMeasureTime = now();
    _lastMeasurement = analogRead(_inputPin) / (float)_analogMaxAmount;
    // TODO: Curve correction for calibration data
    return _lastMeasurement;
}

byte HydroponicsAnalogSensor::getInputPin() const
{
    return _inputPin;
}

int HydroponicsAnalogSensor::getAnalogMaxAmount() const
{
    return _analogMaxAmount;
}

int HydroponicsAnalogSensor::getAnalogBitResolution() const
{
    return _analogBitRes;
}


HydroponicsDHTSensor::HydroponicsDHTSensor(byte inputPin,
                                           Hydroponics_FluidReservoir fluidReservoir,
                                           uint8_t dhtType)
    : HydroponicsSensor(Hydroponics_SensorType_AirTempHumidity, fluidReservoir),
      _dht(new DHT(inputPin, dhtType))
{
    assert(!(_dht && "DHT instance creation failure"));
    _lastMeasurement.humidity = _lastMeasurement.temperature = 0.0f;

    if (_dht) {
        _dht->begin();
    }
}

HydroponicsDHTSensor::~HydroponicsDHTSensor()
{
    if (_dht) { delete _dht; _dht = NULL; }
}

DHTMeasurement HydroponicsDHTSensor::getLastMeasurement() const
{
    return _lastMeasurement;
}

DHTMeasurement HydroponicsDHTSensor::takeMeasurement(bool force)
{
    _lastMeasureTime = now();
    _lastMeasurement.temperature = _dht->readTemperature(false, force);
    _lastMeasurement.humidity = _dht->readHumidity(force);
    return _lastMeasurement;
}

HydroponicsDSSensor::HydroponicsDSSensor(byte inputPin,
                                         Hydroponics_FluidReservoir fluidReservoir,
                                         byte readBitResolution)
    : HydroponicsSensor(Hydroponics_SensorType_WaterTemperature, fluidReservoir),
      _oneWire(new OneWire(inputPin)), _dt(new DallasTemperature()),
      _lastMeasurement(0.0f)
{
    assert(!(_oneWire && "OneWire instance creation failure"));
    assert(!(_dt && "DallasTemperature instance creation failure"));

    if (_dt && _oneWire) {
        _dt->setOneWire(_oneWire);
        _dt->setPullupPin(inputPin);
        _dt->begin();
        _dt->setResolution(readBitResolution);
    }
}

HydroponicsDSSensor::~HydroponicsDSSensor()
{
    if (_oneWire) { delete _oneWire; _oneWire = NULL; }
    if (_dt) { delete _dt; _dt = NULL; }
}

float HydroponicsDSSensor::getLastMeasurement() const
{
    return _lastMeasurement;
}

float HydroponicsDSSensor::takeMeasurement()
{
    _lastMeasureTime = now();
    _lastMeasurement = _dt->getTempCByIndex(0);
    return _lastMeasurement;
}

OneWire &HydroponicsDSSensor::getOneWire() const
{
    return *_oneWire;
}


HydroponicsBinarySensor::HydroponicsBinarySensor(byte inputPin,
                                                 Hydroponics_SensorType sensorType,
                                                 Hydroponics_FluidReservoir fluidReservoir,
                                                 bool activeLow)
    : HydroponicsSensor(sensorType, fluidReservoir),
      _inputPin(inputPin), _activeLow(activeLow), _lastState(false)
{
    pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
}

HydroponicsBinarySensor::~HydroponicsBinarySensor()
{ ; }

bool HydroponicsBinarySensor::pollState()
{
    _lastMeasureTime = now();
    _lastState = digitalRead(_inputPin);
    return _lastState;
}

bool HydroponicsBinarySensor::getLastState() const
{
    return _lastState;
}

byte HydroponicsBinarySensor::getInputPin() const
{
    return _inputPin;
}

bool HydroponicsBinarySensor::getActiveLow() const
{
    return _activeLow;
}


HydroponicsBinaryAnalogSensor::HydroponicsBinaryAnalogSensor(byte inputPin,
                                                             float tolerance, bool activeBelow,
                                                             Hydroponics_SensorType sensorType,
                                                             Hydroponics_FluidReservoir fluidReservoir,
                                                             byte readBitResolution)
    : HydroponicsSensor(sensorType, fluidReservoir),
      _inputPin(inputPin), _tolerance(tolerance), _activeBelow(activeBelow),
      _lastState(false), _lastMeasurement(0),
      #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
          _analogBitRes(readBitResolution), _analogMaxAmount((1 << readBitResolution) - 1)
      #else
          _analogBitRes(8), _analogMaxAmount(255)
      #endif
{
    pinMode(_inputPin, INPUT);
}

HydroponicsBinaryAnalogSensor::~HydroponicsBinaryAnalogSensor()
{ ; }

bool HydroponicsBinaryAnalogSensor::pollState()
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
        analogReadResolution(_analogBitRes);
    #endif
    _lastMeasureTime = now();
    _lastMeasurement = analogRead(_inputPin) / (float)_analogMaxAmount;
    // TODO: Curve correction for calibration data
    _lastState = _activeBelow ? _lastMeasurement <= _tolerance + FLT_EPSILON :
                                _lastMeasurement >= _tolerance - FLT_EPSILON;
    return _lastState;
}

bool HydroponicsBinaryAnalogSensor::getLastState() const
{
    return _lastState;
}

float HydroponicsBinaryAnalogSensor::getLastMeasurement() const
{
    return _lastMeasurement;
}

byte HydroponicsBinaryAnalogSensor::getInputPin() const
{
    return _inputPin;
}

float HydroponicsBinaryAnalogSensor::getTolerance() const
{
    return _tolerance;
}

bool HydroponicsBinaryAnalogSensor::getActiveBelow() const
{
    return _activeBelow;
}

int HydroponicsBinaryAnalogSensor::getAnalogMaxAmount() const
{
    return _analogMaxAmount;
}

int HydroponicsBinaryAnalogSensor::getAnalogBitResolution() const
{
    return _analogBitRes;
}
