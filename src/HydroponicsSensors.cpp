/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensorsare 
*/

#include "HydroponicsSensors.h"

HydroponicsCalibrationData::HydroponicsCalibrationData()
    : HydroponicsData("HCAL", 1),
      sensor(Hydroponics_SensorType_Undefined), reservoir(Hydroponics_FluidReservoir_Undefined),
      multiplier(1.0), offset(0.0)
{ ; }

HydroponicsCalibrationData::HydroponicsCalibrationData(Hydroponics_SensorType sensorIn, Hydroponics_FluidReservoir reservoirIn)
    : HydroponicsData("HCAL", 1),
      sensor(sensorIn), reservoir(reservoirIn),
      multiplier(1.0), offset(0.0)
{ ; }

void HydroponicsCalibrationData::toJSONDocument(JsonDocument &docOut) const
{
    // TODO
}

void HydroponicsCalibrationData::fromJSONDocument(const JsonDocument &docIn)
{
    // TODO
}


HydroponicsSensor::HydroponicsSensor(Hydroponics_SensorType sensorType,
                                     Hydroponics_FluidReservoir fluidReservoir,
                                     Hydroponics_MeasurementMode measurementMode)
    : _sensorType(sensorType), _fluidReservoir(fluidReservoir), _measurementUnits(Hydroponics_UnitsType_Undefined)
{
    switch (sensorType) {
        case Hydroponics_SensorType_AirCarbonDioxide:
            _measurementUnits = Hydroponics_UnitsType_Concentration_PPM;
            break;
        case Hydroponics_SensorType_PotentialHydrogen:
            _measurementUnits = Hydroponics_UnitsType_pHScale_0_14;
            break;
        case Hydroponics_SensorType_TotalDissolvedSolids:
            _measurementUnits = Hydroponics_UnitsType_Concentration_EC;
            break;
        case Hydroponics_SensorType_WaterTemperature:
            switch (measurementMode) {
                case Hydroponics_MeasurementMode_Imperial:
                    _measurementUnits = Hydroponics_UnitsType_Temperature_Fahrenheit;
                    break;
                case Hydroponics_MeasurementMode_Metric:
                    _measurementUnits = Hydroponics_UnitsType_Temperature_Celsius;
                    break;
                case Hydroponics_MeasurementMode_Scientific:
                    _measurementUnits = Hydroponics_UnitsType_Temperature_Kelvin;
                    break;
                default: break;
            }
            break;
        case Hydroponics_SensorType_WaterPumpFlowSensor:
            switch (measurementMode) {
                case Hydroponics_MeasurementMode_Imperial:
                    _measurementUnits = Hydroponics_UnitsType_LiquidFlow_GallonsPerMin;
                    break;
                case Hydroponics_MeasurementMode_Metric:
                case Hydroponics_MeasurementMode_Scientific:
                    _measurementUnits = Hydroponics_UnitsType_LiquidFlow_LitersPerMin;
                    break;
                default: break;
            }
            break;
        case Hydroponics_SensorType_LowWaterHeightMeter:
        case Hydroponics_SensorType_HighWaterHeightMeter:
            switch (measurementMode) {
                case Hydroponics_MeasurementMode_Imperial:
                    _measurementUnits = Hydroponics_UnitsType_Distance_Feet;
                    break;
                case Hydroponics_MeasurementMode_Metric:
                case Hydroponics_MeasurementMode_Scientific:
                    _measurementUnits = Hydroponics_UnitsType_Distance_Meters;
                    break;
                default: break;
            }
            break;
        default:
            break;
    }
}

HydroponicsSensor::~HydroponicsSensor()
{ ; }

HydroponicsSensorMeasurement *HydroponicsSensor::takeMeasurement()
{
    //assert(!"To be overridden in base classes.");
    return NULL;
}

HydroponicsSensorMeasurement *HydroponicsSensor::getLastMeasurement() const
{
    //assert(!"To be overridden in base classes.");
    return NULL;
}

time_t HydroponicsSensor::getLastMeasurementTime() const
{
    //assert(!"To be overridden in base classes.");
    return 0;
}

void HydroponicsSensor::update()
{ ; }

Hydroponics_SensorType HydroponicsSensor::getSensorType() const
{
    return _sensorType;
}

Hydroponics_FluidReservoir HydroponicsSensor::getFluidReservoir() const
{
    return _fluidReservoir;
}

Hydroponics_UnitsType HydroponicsSensor::getMeasurementUnits() const
{
    return _measurementUnits;
}


HydroponicsAnalogSensor::HydroponicsAnalogSensor(byte inputPin,
                                                 Hydroponics_SensorType sensorType,
                                                 Hydroponics_FluidReservoir fluidReservoir,
                                                 Hydroponics_MeasurementMode measurementMode,
                                                 byte readBitResolution)
    : HydroponicsSensor(sensorType, fluidReservoir, measurementMode),
      _inputPin(inputPin),
      #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
          _analogBitRes(constrain(readBitResolution, 8, 12)), _analogMaxAmount(1 << constrain(readBitResolution, 8, 12))
      #else
          _analogBitRes(8), _analogMaxAmount(256)
      #endif
{
    //assert(_analogBitRes == readBitResolution && "Resolved resolution mismatch with passed resolution");
    pinMode(_inputPin, INPUT);
}

HydroponicsAnalogSensor::~HydroponicsAnalogSensor()
{ ; }

HydroponicsAnalogSensorMeasurement *HydroponicsAnalogSensor::takeMeasurement()
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
        analogReadResolution(_analogBitRes);
    #endif

    HydroponicsAnalogSensorMeasurement newMeasurement;
    newMeasurement.value = analogRead(_inputPin) / (float)_analogMaxAmount;
    newMeasurement.units = Hydroponics_UnitsType_Raw_0_1;
    newMeasurement.timestamp = now();

    // TODO: Curve correction from calibration data

    // Convert value and assign units, if needed
    convertAndAssign(&newMeasurement.value, &newMeasurement.units, _measurementUnits);

    return &(_lastMeasurement = newMeasurement);
}

HydroponicsAnalogSensorMeasurement *HydroponicsAnalogSensor::getLastMeasurement() const
{
    return &_lastMeasurement;
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
                                           Hydroponics_MeasurementMode measurementMode,
                                           uint8_t dhtType)
    : HydroponicsSensor(Hydroponics_SensorType_AirTempHumidity, fluidReservoir, measurementMode),
      _dht(new DHT(inputPin, dhtType)), _computeHeatIndex(true)
{
    //assert(_dht && "DHT instance creation failure");
    memset(&_lastMeasurement, 0, sizeof(_lastMeasurement));

    if (_dht) {
        _dht->begin();
    }
}

HydroponicsDHTSensor::~HydroponicsDHTSensor()
{
    if (_dht) { delete _dht; _dht = NULL; }
}

HydroponicsDHTSensorMeasurement *HydroponicsDHTSensor::takeMeasurement(bool force)
{
    bool isFahrenheit = _measurementUnits == Hydroponics_UnitsType_Temperature_Fahrenheit;

    HydroponicsDHTSensorMeasurement newMeasurement;
    newMeasurement.temperature = _dht->readTemperature(isFahrenheit, force);
    newMeasurement.temperatureUnits = isFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit : Hydroponics_UnitsType_Temperature_Celsius;
    newMeasurement.humidity = _dht->readHumidity(force);
    newMeasurement.humidityUnits = Hydroponics_UnitsType_Percentile_0_100;
    newMeasurement.timestamp = now();
    if (_computeHeatIndex) {
        newMeasurement.heatIndex = _dht->computeHeatIndex(newMeasurement.temperature, newMeasurement.humidity, isFahrenheit);
        newMeasurement.heatIndexUnits = isFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit : Hydroponics_UnitsType_Temperature_Celsius;
    }

    // TODO: Curve correction from calibration data

    // Convert value and assign units, if needed
    convertAndAssign(&newMeasurement.temperature, &newMeasurement.temperatureUnits, _measurementUnits);
    if (_computeHeatIndex) {
        convertAndAssign(&newMeasurement.heatIndex, &newMeasurement.heatIndexUnits, _measurementUnits);
    }

    return &(_lastMeasurement = newMeasurement);
}

HydroponicsDHTSensorMeasurement *HydroponicsDHTSensor::getLastMeasurement() const
{
    return &_lastMeasurement;
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
                                         Hydroponics_MeasurementMode measurementMode,
                                         byte readBitResolution)
    : HydroponicsSensor(Hydroponics_SensorType_WaterTemperature, fluidReservoir, measurementMode),
      _oneWire(new OneWire(inputPin)), _dt(new DallasTemperature())
{
    //assert(_oneWire && "OneWire instance creation failure");
    //assert(_dt && "DallasTemperature instance creation failure");
    memset(&_lastMeasurement, 0, sizeof(_lastMeasurement));

    if (_dt && _oneWire) {
        _dt->setOneWire(_oneWire);
        _dt->setPullupPin(inputPin); // TODO: needed?
        _dt->setWaitForConversion(true); // TODO: make measurement async
        _dt->begin();
        _dt->setResolution(readBitResolution);
        //assert(_dt->getResolution() == readBitResolution && "Resolved resolution mismatch with passed resolution");
    }
}

HydroponicsDSSensor::~HydroponicsDSSensor()
{
    if (_oneWire) { delete _oneWire; _oneWire = NULL; }
    if (_dt) { delete _dt; _dt = NULL; }
}

HydroponicsAnalogSensorMeasurement *HydroponicsDSSensor::takeMeasurement()
{
    bool isFahrenheit = _measurementUnits == Hydroponics_UnitsType_Temperature_Fahrenheit;

    _dt->requestTemperatures(); // Blocking

    HydroponicsAnalogSensorMeasurement newMeasurement;
    newMeasurement.value = isFahrenheit ? _dt->getTempFByIndex(0) : _dt->getTempCByIndex(0); // TODO: Support more than one DS device on line
    newMeasurement.units = isFahrenheit ? Hydroponics_UnitsType_Temperature_Fahrenheit : Hydroponics_UnitsType_Temperature_Celsius;
    newMeasurement.timestamp = now();

    bool deviceDisconnected = isFPEqual(newMeasurement.value, (isFahrenheit ? DEVICE_DISCONNECTED_F : DEVICE_DISCONNECTED_C));
    //assert(!deviceDisconnected && "Measurement failed device disconnected");

    if (!deviceDisconnected) {
        // TODO: Curve correction from calibration data

        // Convert value and assign units, if needed
        convertAndAssign(&newMeasurement.value, &newMeasurement.units, _measurementUnits);

        return &(_lastMeasurement = newMeasurement);
    }
    return &_lastMeasurement;
}

HydroponicsAnalogSensorMeasurement *HydroponicsDSSensor::getLastMeasurement() const
{
    return &_lastMeasurement;
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
                                                 Hydroponics_MeasurementMode measurementMode,
                                                 bool activeLow)
    : HydroponicsSensor(sensorType, fluidReservoir, measurementMode),
      _inputPin(inputPin), _activeLow(activeLow)
{
    memset(&_lastMeasurement, 0, sizeof(_lastMeasurement));
    pinMode(_inputPin, _activeLow ? INPUT_PULLUP : INPUT);
}

HydroponicsBinarySensor::~HydroponicsBinarySensor()
{ ; }

HydroponicsBinarySensorMeasurement *HydroponicsBinarySensor::takeMeasurement()
{
    HydroponicsBinarySensorMeasurement newMeasurement;
    newMeasurement.state = digitalRead(_inputPin);
    newMeasurement.timestamp = now();

    return &(_lastMeasurement = newMeasurement);
}

HydroponicsBinarySensorMeasurement *HydroponicsBinarySensor::getLastMeasurement() const
{
    return &_lastMeasurement;
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
                                                             Hydroponics_MeasurementMode measurementMode,
                                                             byte readBitResolution)
    : HydroponicsSensor(sensorType, fluidReservoir, measurementMode),
      _inputPin(inputPin), _tolerance(tolerance), _activeBelow(activeBelow),
      #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
          _analogBitRes(constrain(readBitResolution, 8, 12)), _analogMaxAmount(1 << constrain(readBitResolution, 8, 12))
      #else
          _analogBitRes(8), _analogMaxAmount(256)
      #endif
{
    //assert(_analogBitRes == readBitResolution && "Resolved resolution mismatch with passed resolution");
    memset(&_lastMeasurement, 0, sizeof(_lastMeasurement));
    pinMode(_inputPin, INPUT);
}

HydroponicsBinaryAnalogSensor::~HydroponicsBinaryAnalogSensor()
{ ; }

HydroponicsBinaryAnalogSensorMeasurement *HydroponicsBinaryAnalogSensor::takeMeasurement()
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
        analogReadResolution(_analogBitRes);
    #endif

    HydroponicsBinaryAnalogSensorMeasurement newMeasurement;
    newMeasurement.value = analogRead(_inputPin) / (float)_analogMaxAmount;
    newMeasurement.units = Hydroponics_UnitsType_Raw_0_1;
    newMeasurement.timestamp = now();
    newMeasurement.state = _activeBelow ? newMeasurement.value <= _tolerance + FLT_EPSILON :
                                          newMeasurement.value >= _tolerance - FLT_EPSILON;

    // TODO: Curve correction from calibration data

    // Convert value and assign units, if needed
    convertAndAssign(&newMeasurement.value, &newMeasurement.units, _measurementUnits);

    return &(_lastMeasurement = newMeasurement);
}

HydroponicsBinaryAnalogSensorMeasurement *HydroponicsBinaryAnalogSensor::getLastMeasurement() const
{
    return &_lastMeasurement;
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
