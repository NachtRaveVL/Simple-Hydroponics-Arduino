/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensorsare 
*/

#include "HydroponicsSensors.h"

HydroponicsSensor::HydroponicsSensor(Hydroponics_SensorType sensorType,
                                     Hydroponics_FluidReservoir fluidReservoir)
    : _sensorType(sensorType), _fluidReservoir(fluidReservoir)
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


HydroponicsAnalogSensor::HydroponicsAnalogSensor(byte inputPin,
                                                 Hydroponics_SensorType sensorType,
                                                 Hydroponics_FluidReservoir fluidReservoir,
                                                 byte readBitResolution)
    : HydroponicsSensor(sensorType, fluidReservoir),
      _inputPin(inputPin),
      #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
          _analogBitRes(constrain(readBitResolution, 8, 12)), _analogMaxAmount(1 << constrain(readBitResolution, 8, 12))
      #else
          _analogBitRes(8), _analogMaxAmount(256)
      #endif
{
    assert(!(_analogBitRes == readBitResolution && "Resolved resolution mismatch with passed resolution"));
    memset(&_lastMeasurement, 0, sizeof(_lastMeasurement));
    pinMode(_inputPin, INPUT);
}

HydroponicsAnalogSensor::~HydroponicsAnalogSensor()
{ ; }

HydroponicsSensorAnalogMeasurement HydroponicsAnalogSensor::takeMeasurement()
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
        analogReadResolution(_analogBitRes);
    #endif

    HydroponicsSensorAnalogMeasurement newMeasurement;
    newMeasurement.value = analogRead(_inputPin) / (float)_analogMaxAmount;
    newMeasurement.timestamp = now();
    // TODO: Curve correction from calibration data

    return (_lastMeasurement = newMeasurement);
}

HydroponicsSensorAnalogMeasurement HydroponicsAnalogSensor::getLastMeasurement() const
{
    return _lastMeasurement;
}

time_t HydroponicsAnalogSensor::getLastMeasurementTime() const
{
    return _lastMeasurement.timestamp;
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
      _dht(new DHT(inputPin, dhtType)), _computeHeatIndex(true)
{
    assert(!(_dht && "DHT instance creation failure"));
    memset(&_lastMeasurement, 0, sizeof(_lastMeasurement));

    if (_dht) {
        _dht->begin();
    }
}

HydroponicsDHTSensor::~HydroponicsDHTSensor()
{
    if (_dht) { delete _dht; _dht = NULL; }
}

HydroponicsSensorDHTMeasurement HydroponicsDHTSensor::takeMeasurement(bool force)
{
    HydroponicsSensorDHTMeasurement newMeasurement;
    newMeasurement.temperature = _dht->readTemperature(false, force);
    newMeasurement.humidity = _dht->readHumidity(force);
    newMeasurement.timestamp = now();
    newMeasurement.heatIndex = _computeHeatIndex ? _dht->computeHeatIndex(newMeasurement.temperature, newMeasurement.humidity, false) : 0.0f;

    return (_lastMeasurement = newMeasurement);
}

HydroponicsSensorDHTMeasurement HydroponicsDHTSensor::getLastMeasurement() const
{
    return _lastMeasurement;
}

time_t HydroponicsDHTSensor::getLastMeasurementTime() const
{
    return _lastMeasurement.timestamp;
}

void HydroponicsDHTSensor::setComputeHeatIndex(bool computeHeatIndex)
{
    _computeHeatIndex = computeHeatIndex;
}

bool HydroponicsDHTSensor::getComputeHeatIndex()
{
    return _computeHeatIndex;
}


HydroponicsDSSensor::HydroponicsDSSensor(byte inputPin,
                                         Hydroponics_FluidReservoir fluidReservoir,
                                         byte readBitResolution)
    : HydroponicsSensor(Hydroponics_SensorType_WaterTemperature, fluidReservoir),
      _oneWire(new OneWire(inputPin)), _dt(new DallasTemperature())
{
    assert(!(_oneWire && "OneWire instance creation failure"));
    assert(!(_dt && "DallasTemperature instance creation failure"));
    memset(&_lastMeasurement, 0, sizeof(_lastMeasurement));

    if (_dt && _oneWire) {
        _dt->setOneWire(_oneWire);
        _dt->setPullupPin(inputPin); // TODO: needed?
        _dt->setWaitForConversion(true); // TODO: make calls async
        _dt->begin();
        _dt->setResolution(readBitResolution);
        assert(!(_dt->getResolution() == readBitResolution && "Resolved resolution mismatch with passed resolution"));
    }
}

HydroponicsDSSensor::~HydroponicsDSSensor()
{
    if (_oneWire) { delete _oneWire; _oneWire = NULL; }
    if (_dt) { delete _dt; _dt = NULL; }
}

HydroponicsSensorAnalogMeasurement HydroponicsDSSensor::takeMeasurement()
{
    _dt->requestTemperatures(); // Blocking

    HydroponicsSensorAnalogMeasurement newMeasurement;
    newMeasurement.value = _dt->getTempCByIndex(0); // TODO: Support more than one DS device on line
    newMeasurement.timestamp = now();

    if (!(fabs(newMeasurement.value - DEVICE_DISCONNECTED_C) < FLT_EPSILON)) {
        return (_lastMeasurement = newMeasurement);
    }
    return _lastMeasurement;
}

HydroponicsSensorAnalogMeasurement HydroponicsDSSensor::getLastMeasurement() const
{
    return _lastMeasurement;
}

time_t HydroponicsDSSensor::getLastMeasurementTime() const
{
    return _lastMeasurement.timestamp;
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
      _inputPin(inputPin), _activeLow(activeLow)
{
    memset(&_lastMeasurement, 0, sizeof(_lastMeasurement));
    pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
}

HydroponicsBinarySensor::~HydroponicsBinarySensor()
{ ; }

HydroponicsSensorBinaryMeasurement HydroponicsBinarySensor::takeMeasurement()
{
    HydroponicsSensorBinaryMeasurement newMeasurement;
    newMeasurement.state = digitalRead(_inputPin);
    newMeasurement.timestamp = now();

    return (_lastMeasurement = newMeasurement);
}

HydroponicsSensorBinaryMeasurement HydroponicsBinarySensor::getLastMeasurement() const
{
    return _lastMeasurement;
}

time_t HydroponicsBinarySensor::getLastMeasurementTime() const
{
    return _lastMeasurement.timestamp;
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
      #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
          _analogBitRes(constrain(readBitResolution, 8, 12)), _analogMaxAmount(1 << constrain(readBitResolution, 8, 12))
      #else
          _analogBitRes(8), _analogMaxAmount(256)
      #endif
{
    assert(!(_analogBitRes == readBitResolution && "Resolved resolution mismatch with passed resolution"));
    memset(&_lastMeasurement, 0, sizeof(_lastMeasurement));
    pinMode(_inputPin, INPUT);
}

HydroponicsBinaryAnalogSensor::~HydroponicsBinaryAnalogSensor()
{ ; }

HydroponicsSensorBinaryAnalogMeasurement HydroponicsBinaryAnalogSensor::takeMeasurement()
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
        analogReadResolution(_analogBitRes);
    #endif

    HydroponicsSensorBinaryAnalogMeasurement newMeasurement;
    newMeasurement.value = analogRead(_inputPin) / (float)_analogMaxAmount;
    newMeasurement.timestamp = now();
    // TODO: Curve correction from calibration data
    newMeasurement.state = _activeBelow ? newMeasurement.value <= _tolerance + FLT_EPSILON :
                                          newMeasurement.value >= _tolerance - FLT_EPSILON;

    return (_lastMeasurement = newMeasurement);
}

HydroponicsSensorBinaryAnalogMeasurement HydroponicsBinaryAnalogSensor::getLastMeasurement() const
{
    return _lastMeasurement;
}

time_t HydroponicsBinaryAnalogSensor::getLastMeasurementTime() const
{
    return _lastMeasurement.timestamp;
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
