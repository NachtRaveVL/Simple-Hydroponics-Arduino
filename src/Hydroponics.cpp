/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Main
*/

#include "Hydroponics.h"

static void uDelayMillisFuncDef(unsigned int timeout) {
#ifdef HYDRO_USE_SCHEDULER
    if (timeout > 0) {
        unsigned long currTime = millis();
        unsigned long endTime = currTime + (unsigned long)timeout;
        if (currTime < endTime) { // not overflowing
            while (millis() < endTime)
                Scheduler.yield();
        } else { // overflowing
            unsigned long begTime = currTime;
            while (currTime >= begTime || currTime < endTime) {
                Scheduler.yield();
                currTime = millis();
            }
        }
    } else
        Scheduler.yield();
#else
    delay(timeout);
#endif
}

static void uDelayMicrosFuncDef(unsigned int timeout) {
#ifdef HYDRO_USE_SCHEDULER
    if (timeout > 1000) {
        unsigned long currTime = micros();
        unsigned long endTime = currTime + (unsigned long)timeout;
        if (currTime < endTime) { // not overflowing
            while (micros() < endTime)
                Scheduler.yield();
        } else { // overflowing
            unsigned long begTime = currTime;
            while (currTime >= begTime || currTime < endTime) {
                Scheduler.yield();
                currTime = micros();
            }
        }
    } else if (timeout > 0)
        delayMicroseconds(timeout);
    else
        Scheduler.yield();
#else
    delayMicroseconds(timeout);
#endif
}


static RTC_DS3231 *_rtcSyncProvider = NULL;
time_t rtcNow() {
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}


HydroponicsSystemData::HydroponicsSystemData()
    : _ident{'H','S', 'D'}, _version(1),
      systemName{'H','y','d','r','o','d','u','i','n','o', '\0'},
      cropPositionsCount(16), maxActiveRelayCount{2},
      reservoirSizeUnits(Hydroponics_UnitsType_Undefined),
      pumpFlowRateUnits(Hydroponics_UnitsType_Undefined)
{
    memset(reservoirSize, 0, sizeof(reservoirSize));
    memset(pumpFlowRate, 0, sizeof(pumpFlowRate));
    memset(calibrationData, 0, sizeof(calibrationData));

    for (int calibIndex = 0; calibIndex < HYDRO_CALIB_MAXSIZE; ++calibIndex) {
        calibrationData[calibIndex].sensor = Hydroponics_SensorType_Undefined;
        calibrationData[calibIndex].reservoir = Hydroponics_FluidReservoir_Undefined;
    }
}


Hydroponics::Hydroponics(byte piezoBuzzerPin, byte sdCardCSPin, byte controlInputPin1,
                        byte eepromI2CAddress, byte rtcI2CAddress, byte lcdI2CAddress,
                        TwoWire& i2cWire, uint32_t i2cSpeed, uint32_t spiSpeed)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed), _spiSpeed(spiSpeed),
      _buzzer(&EasyBuzzer), _eeprom(new I2C_eeprom(eepromI2CAddress, HYDRO_EEPROM_MEMORYSIZE, &i2cWire)), _rtc(new RTC_DS3231()),
      _sd(NULL), _eepromBegan(false), _rtcBegan(false), _lcd(NULL), _keypad(NULL), _systemData(NULL),
      _i2cAddressLCD(lcdI2CAddress), _ctrlInputPin1(controlInputPin1), _sdCardCSPin(sdCardCSPin),
      _uDelayMillisFunc(uDelayMillisFuncDef), _uDelayMicrosFunc(uDelayMicrosFuncDef)
{
    if (piezoBuzzerPin) {
        _buzzer->setPin(piezoBuzzerPin);
    }
    if (sdCardCSPin) {
        #if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)) || defined(SD)
            _sd = &SD;
        #else
            _sd = new SDClass();
        #endif
    }
}

Hydroponics::Hydroponics(TwoWire& i2cWire, uint32_t i2cSpeed, uint32_t spiSpeed,
                         byte piezoBuzzerPin, byte sdCardCSPin, byte controlInputPin1,
                         byte eepromI2CAddress, byte rtcI2CAddress, byte lcdI2CAddress)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed), _spiSpeed(spiSpeed),
      _buzzer(&EasyBuzzer), _eeprom(new I2C_eeprom(eepromI2CAddress, HYDRO_EEPROM_MEMORYSIZE, &i2cWire)), _rtc(new RTC_DS3231()),
      _sd(NULL), _eepromBegan(false), _rtcBegan(false), _lcd(NULL), _keypad(NULL), _systemData(NULL),
      _i2cAddressLCD(lcdI2CAddress), _ctrlInputPin1(controlInputPin1), _sdCardCSPin(sdCardCSPin),
      _uDelayMillisFunc(uDelayMillisFuncDef), _uDelayMicrosFunc(uDelayMicrosFuncDef)
{
    if (piezoBuzzerPin) {
        _buzzer->setPin(piezoBuzzerPin);
    }
    if (sdCardCSPin) {
        #if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)) || defined(SD)
            _sd = &SD;
        #else
            _sd = new SDClass();
        #endif
    }
}

Hydroponics::~Hydroponics()
{
    _i2cWire = NULL;
    _buzzer = NULL;
    if (_eeprom) { delete _eeprom; _eeprom = NULL; }
    if (_rtc) {
        if (_rtcSyncProvider == _rtc) { setSyncProvider(NULL); _rtcSyncProvider = NULL; }
        delete _rtc; _rtc = NULL;
    }
    #if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)
        _sd = NULL;
    #else
        if (_sd) { delete _sd; _sd = NULL; }
    #endif
    if (_lcd) { delete _lcd; _lcd = NULL; }
    if (_keypad) { delete _keypad; _keypad = NULL; }
    if (_systemData) { delete _systemData; _systemData = NULL; }
}

void Hydroponics::init(Hydroponics_SystemMode systemMode,
                       Hydroponics_MeasurementMode measurementMode,
                       Hydroponics_LCDOutputMode lcdOutMode,
                       Hydroponics_ControlInputMode ctrlInMode)
{
    assert(!(!_systemData && "Controller already initialized"));
    if (!_systemData) { 
        assert(!((int)systemMode >= 0 && systemMode < Hydroponics_SystemMode_Count && "Invalid system mode"));
        assert(!((int)measurementMode >= 0 && measurementMode < Hydroponics_MeasurementMode_Count && "Invalid measurement mode"));
        assert(!((int)lcdOutMode >= 0 && lcdOutMode < Hydroponics_LCDOutputMode_Count && "Invalid LCD output mode"));
        assert(!((int)ctrlInMode >= 0 && ctrlInMode < Hydroponics_ControlInputMode_Count && "Invalid control input mode"));

        _systemData = new HydroponicsSystemData();
        assert(!(_systemData && "Invalid system data store"));

        if (_systemData) {
            _systemData->systemMode = systemMode;
            _systemData->measurementMode = measurementMode;
            _systemData->lcdOutMode = lcdOutMode;
            _systemData->ctrlInMode = ctrlInMode;
            commonInit();
        }
    }  
}

bool Hydroponics::initFromEEPROM()
{
    assert(!(!_systemData && "Controller already initialized"));
    if (!_systemData) {
        getEEPROM(); // Forces begin, if not already
        if (_eeprom) {
            // TODO
        }

        assert(!(_systemData && "Invalid system data store"));
        if (_systemData) { commonInit(); }
        return _systemData;
    }

    return false;
}

bool Hydroponics::initFromMicroSD(const char * configFile)
{
    assert(!(!_systemData && "Controller already initialized"));
    if (!_systemData) {
        SDClass *sd = getSDCard();
        if (sd) {
            File config = sd->open(configFile);
            if (config && config.size()) {
                // TODO
                config.close();
            }
            sd->end();
        }

        assert(!(_systemData && "Invalid system data store"));
        if (_systemData) { commonInit(); }
        return _systemData;
    }

    return false;
}

void Hydroponics::commonInit()
{
    switch (getMeasurementMode()) {
        default:
        case Hydroponics_MeasurementMode_Imperial:
            _systemData->reservoirSizeUnits = Hydroponics_UnitsType_LiquidVolume_Gallons; 
            _systemData->pumpFlowRateUnits = Hydroponics_UnitsType_LiquidFlow_GallonsPerMin;
            break;

        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            _systemData->reservoirSizeUnits = Hydroponics_UnitsType_LiquidVolume_Liters; 
            _systemData->pumpFlowRateUnits = Hydroponics_UnitsType_LiquidFlow_LitersPerMin;
            break;
    }
}

void Hydroponics::makeRTCSyncProvider()
{
    if ((_rtcSyncProvider = getRealTimeClock())) {
        setSyncProvider(rtcNow);
    }
}

void Hydroponics::update()
{
    if (_buzzer) { _buzzer->update(); }
    // TODO
}

bool Hydroponics::registerActuator(HydroponicsActuator *actuator)
{
    // TODO
    return true;
}

bool Hydroponics::unregisterActuator(HydroponicsActuator *actuator)
{
    // TODO
    return true;
}

HydroponicsRelayActuator *Hydroponics::addGrowLightsRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    assert(!(outputPinIsDigital && "Output pin is not digital"));

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin,
                                                                          Hydroponics_ActuatorType_GrowLightsRelay,
                                                                          Hydroponics_RelayRail_ACRail);
        if (registerActuator(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return NULL;
}

HydroponicsRelayActuator *Hydroponics::addWaterPumpRelay(byte outputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    assert(!(outputPinIsDigital && "Output pin is not digital"));

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin,
                                                                          Hydroponics_ActuatorType_WaterPumpRelay,
                                                                          Hydroponics_RelayRail_ACRail,
                                                                          fluidReservoir);
        if (registerActuator(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return NULL;
}

HydroponicsRelayActuator *Hydroponics::addWaterHeaterRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    assert(!(outputPinIsDigital && "Output pin is not digital"));

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin,
                                                                          Hydroponics_ActuatorType_WaterHeaterRelay,
                                                                          Hydroponics_RelayRail_ACRail);
        if (registerActuator(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return NULL;
}

HydroponicsRelayActuator *Hydroponics::addWaterAeratorRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    assert(!(outputPinIsDigital && "Output pin is not digital"));

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin,
                                                                          Hydroponics_ActuatorType_WaterAeratorRelay,
                                                                          Hydroponics_RelayRail_ACRail);
        if (registerActuator(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return NULL;
}

HydroponicsRelayActuator *Hydroponics::addFanExhaustRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    assert(!(outputPinIsDigital && "Output pin is not digital"));

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin,
                                                                          Hydroponics_ActuatorType_FanExhaustRelay,
                                                                          Hydroponics_RelayRail_ACRail);
        if (registerActuator(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return NULL;
}

HydroponicsPWMActuator *Hydroponics::addFanExhaustPWM(byte outputPin, byte writeBitResolution)
{
    bool outputPinIsPWM = checkPinIsPWM(outputPin);
    assert(!(outputPinIsPWM && "Output pin does not support PWM"));

    if (outputPinIsPWM) {
        HydroponicsPWMActuator *actuator = new HydroponicsPWMActuator(outputPin,
                                                                      Hydroponics_ActuatorType_FanExhaustPWM,
                                                                      Hydroponics_FluidReservoir_Undefined,
                                                                      writeBitResolution);
        if (registerActuator(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return NULL;
}

HydroponicsRelayActuator *Hydroponics::addPhUpPeristalticPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    assert(!(outputPinIsDigital && "Output pin is not digital"));

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin,
                                                                          Hydroponics_ActuatorType_PeristalticPumpRelay,
                                                                          Hydroponics_RelayRail_DCRail,
                                                                          Hydroponics_FluidReservoir_PhUpSolution);
        if (registerActuator(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return NULL;
}

HydroponicsRelayActuator *Hydroponics::addPhDownPeristalticPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    assert(!(outputPinIsDigital && "Output pin is not digital"));

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin,
                                                                          Hydroponics_ActuatorType_PeristalticPumpRelay,
                                                                          Hydroponics_RelayRail_DCRail,
                                                                          Hydroponics_FluidReservoir_PhDownSolution);
        if (registerActuator(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return NULL;
}

HydroponicsRelayActuator *Hydroponics::addNutrientPremixPeristalticPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    assert(!(outputPinIsDigital && "Output pin is not digital"));

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin,
                                                                          Hydroponics_ActuatorType_PeristalticPumpRelay,
                                                                          Hydroponics_RelayRail_DCRail,
                                                                          Hydroponics_FluidReservoir_NutrientPremix);
        if (registerActuator(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return NULL;
}

HydroponicsRelayActuator *Hydroponics::addFreshWaterPeristalticPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    assert(!(outputPinIsDigital && "Output pin is not digital"));

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin,
                                                                          Hydroponics_ActuatorType_PeristalticPumpRelay,
                                                                          Hydroponics_RelayRail_DCRail,
                                                                          Hydroponics_FluidReservoir_FreshWater);
        if (registerActuator(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return NULL;
}

bool Hydroponics::registerSensor(HydroponicsSensor *sensor)
{
    // TODO
    return true;
}

bool Hydroponics::unregisterSensor(HydroponicsSensor *sensor)
{
    // TODO
    return true;
}

HydroponicsDHTSensor *Hydroponics::addAirDHTTempHumiditySensor(byte inputPin, uint8_t dhtType)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    assert(!(inputPinIsDigital && "Input pin is not digital"));

    if (inputPinIsDigital) {
        HydroponicsDHTSensor *sensor = new HydroponicsDHTSensor(inputPin,
                                                                Hydroponics_FluidReservoir_FeedWater,
                                                                dhtType);

        switch (getMeasurementMode()) {
            default:
            case Hydroponics_MeasurementMode_Imperial:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Temperature_Fahrenheit);
                break;
            case Hydroponics_MeasurementMode_Metric:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Temperature_Celsius);
                break;
            case Hydroponics_MeasurementMode_Scientific:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Temperature_Kelvin);
                break;
        }
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsAnalogSensor *Hydroponics::addAirCO2Sensor(byte inputPin, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    assert(!(inputPinIsAnalog && "Input pin is not analog"));

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin,
                                                                      Hydroponics_SensorType_AirCarbonDioxide,
                                                                      Hydroponics_FluidReservoir_Undefined,
                                                                      readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsAnalogSensor *Hydroponics::addWaterPhMeter(byte inputPin, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    assert(!(inputPinIsAnalog && "Input pin is not analog"));

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin,
                                                                      Hydroponics_SensorType_PotentialHydrogen,
                                                                      Hydroponics_FluidReservoir_FeedWater,
                                                                      readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsAnalogSensor *Hydroponics::addWaterTDSElectrode(byte inputPin, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    assert(!(inputPinIsAnalog && "Input pin is not analog"));

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin,
                                                                      Hydroponics_SensorType_TotalDissolvedSolids,
                                                                      Hydroponics_FluidReservoir_FeedWater,
                                                                      readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsDSSensor *Hydroponics::addWaterDSTempSensor(byte inputPin, byte readBitResolution)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    assert(!(inputPinIsDigital && "Input pin is not digital"));

    if (inputPinIsDigital) {
        HydroponicsDSSensor *sensor = new HydroponicsDSSensor(inputPin,
                                                              Hydroponics_FluidReservoir_FeedWater,
                                                              readBitResolution);

        switch (getMeasurementMode()) {
            default:
            case Hydroponics_MeasurementMode_Imperial:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Temperature_Fahrenheit);
                break;
            case Hydroponics_MeasurementMode_Metric:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Temperature_Celsius);
                break;
            case Hydroponics_MeasurementMode_Scientific:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Temperature_Kelvin);
                break;
        }

        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsAnalogSensor *Hydroponics::addWaterPumpFlowSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    assert(!(inputPinIsAnalog && "Input pin is not analog"));

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin,
                                                                      Hydroponics_SensorType_WaterPumpFlowSensor,
                                                                      fluidReservoir,
                                                                      readBitResolution);

        switch (getMeasurementMode()) {
            default:
            case Hydroponics_MeasurementMode_Imperial:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_LiquidFlow_GallonsPerMin);
                break;
            case Hydroponics_MeasurementMode_Metric:
            case Hydroponics_MeasurementMode_Scientific:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_LiquidFlow_LitersPerMin);
                break;
        }

        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinarySensor *Hydroponics::addLowWaterLevelIndicator(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    assert(!(inputPinIsDigital && "Input pin is not digital"));

    if (inputPinIsDigital) {
        HydroponicsBinarySensor *sensor = new HydroponicsBinarySensor(inputPin,
                                                                      Hydroponics_SensorType_LowWaterLevelIndicator,
                                                                      fluidReservoir,
                                                                      true);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinarySensor *Hydroponics::addHighWaterLevelIndicator(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    assert(!(inputPinIsDigital && "Input pin is not digital"));

    if (inputPinIsDigital) {
        HydroponicsBinarySensor *sensor = new HydroponicsBinarySensor(inputPin,
                                                                      Hydroponics_SensorType_HighWaterLevelIndicator,
                                                                      fluidReservoir,
                                                                      false);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinaryAnalogSensor *Hydroponics::addLowWaterHeightMeter(byte inputPin, Hydroponics_FluidReservoir fluidReservoir, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    assert(!(inputPinIsAnalog && "Input pin is not analog"));

    if (inputPinIsAnalog) {
        // TODO: actual tolerance value
        HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin,
                                                                                  0.1, true,
                                                                                  Hydroponics_SensorType_LowWaterHeightMeter,
                                                                                  fluidReservoir,
                                                                                  readBitResolution);

        switch (getMeasurementMode()) {
            default:
            case Hydroponics_MeasurementMode_Imperial:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Distance_Feet);
                break;
            case Hydroponics_MeasurementMode_Metric:
            case Hydroponics_MeasurementMode_Scientific:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Distance_Meters);
                break;
        }

        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinaryAnalogSensor *Hydroponics::addHighWaterHeightMeter(byte inputPin, Hydroponics_FluidReservoir fluidReservoir, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    assert(!(inputPinIsAnalog && "Input pin is not analog"));

    if (inputPinIsAnalog) {
        // TODO: actual tolerance value
        HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin,
                                                                                  0.9, false,
                                                                                  Hydroponics_SensorType_HighWaterHeightMeter,
                                                                                  fluidReservoir,
                                                                                  readBitResolution);

        switch (getMeasurementMode()) {
            default:
            case Hydroponics_MeasurementMode_Imperial:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Distance_Feet);
                break;
            case Hydroponics_MeasurementMode_Metric:
            case Hydroponics_MeasurementMode_Scientific:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Distance_Meters);
                break;
        }

        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinaryAnalogSensor *Hydroponics::addLowWaterUltrasonicSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    assert(!(inputPinIsAnalog && "Input pin is not analog"));

    if (inputPinIsAnalog) {
        // TODO: actual tolerance value
        HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin,
                                                                                  0.9, false,
                                                                                  Hydroponics_SensorType_LowWaterHeightMeter,
                                                                                  fluidReservoir,
                                                                                  readBitResolution);

        switch (getMeasurementMode()) {
            default:
            case Hydroponics_MeasurementMode_Imperial:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Distance_Feet);
                break;
            case Hydroponics_MeasurementMode_Metric:
            case Hydroponics_MeasurementMode_Scientific:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Distance_Meters);
                break;
        }

        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinaryAnalogSensor *Hydroponics::addHighWaterUltrasonicSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    assert(!(inputPinIsAnalog && "Input pin is not analog"));

    if (inputPinIsAnalog) {
        // TODO: actual tolerance value
        HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin,
                                                                                  0.1, true,
                                                                                  Hydroponics_SensorType_LowWaterHeightMeter,
                                                                                  fluidReservoir,
                                                                                  readBitResolution);

        switch (getMeasurementMode()) {
            default:
            case Hydroponics_MeasurementMode_Imperial:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Distance_Feet);
                break;
            case Hydroponics_MeasurementMode_Metric:
            case Hydroponics_MeasurementMode_Scientific:
                sensor->setMeasurementUnits(Hydroponics_UnitsType_Distance_Meters);
                break;
        }

        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

bool Hydroponics::registerCrop(HydroponicsCrop *crop)
{
    // TODO
    return true;
}

bool Hydroponics::unregisterCrop(HydroponicsCrop *crop)
{
    // TODO
    return true;
}

HydroponicsCrop *Hydroponics::addCropFromSowDate(const Hydroponics_CropType cropType, time_t sowDate, int positionIndex)
{
    assert(!((int)cropType >= 0 && cropType <= Hydroponics_CropType_Count && "Invalid crop type"));
    assert(!(sowDate > 0 && "Invalid sow date"));
    bool positionIsAvailable = positionIndex == -1; // TODO
    assert(!(positionIsAvailable && "Invalid position index"));

    if (positionIsAvailable) {
        HydroponicsCrop *crop = new HydroponicsCrop(cropType, positionIndex, sowDate);
        if (registerCrop(crop)) { return crop; }
        else { delete crop; }
    }

    return NULL;
}

HydroponicsCrop *Hydroponics::addCropFromLastHarvest(const Hydroponics_CropType cropType, time_t lastHarvestDate, int positionIndex)
{
    time_t sowDate = lastHarvestDate - (time_t)(HydroponicsCropsLibrary::getInstance()->getCropData(cropType)->weeksBetweenHarvest * SECS_PER_WEEK);
    return addCropFromSowDate(cropType, sowDate, positionIndex);
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
    return _systemData ? _systemData->measurementMode : Hydroponics_MeasurementMode_Undefined;
}

Hydroponics_LCDOutputMode Hydroponics::getLCDOutputMode() const
{
    return _systemData ? _systemData->lcdOutMode : Hydroponics_LCDOutputMode_Undefined;
}

Hydroponics_ControlInputMode Hydroponics::getControlInputMode() const
{
    return _systemData ? _systemData->ctrlInMode : Hydroponics_ControlInputMode_Undefined;
}

EasyBuzzerClass *Hydroponics::getPiezoBuzzer() const
{
    return _buzzer;
}

I2C_eeprom *Hydroponics::getEEPROM()
{
    if (_eeprom && !_eepromBegan) {
        _eepromBegan = _eeprom->begin();
        assert(!(_eepromBegan && "Failed starting EEPROM"));
    }
    return _eeprom && _eepromBegan ? _eeprom : NULL;
}

RTC_DS3231 *Hydroponics::getRealTimeClock()
{
    if (_rtc && !_rtcBegan) {
        _rtcBegan = _rtc->begin(_i2cWire);
        assert(!(_rtcBegan && "Failed starting RTC"));
    }
    return _rtc && _rtcBegan ? _rtc : NULL;
}

SDClass *Hydroponics::getSDCard(bool begin)
{
    if (_sd && begin) {
        // TODO ESP8266/ESP32 differences
        bool sdBegan = _sd->begin(_spiSpeed, _sdCardCSPin);
        assert(!(sdBegan && "Failed starting SD card"));
    }
    return _sd;
}

LiquidCrystal_I2C *Hydroponics::getLiquidCrystalDisplay()
{
    switch (getLCDOutputMode()) {
        case Hydroponics_LCDOutputMode_20x4LCD:
            if (!_lcd) {
                _lcd = new LiquidCrystal_I2C(_i2cAddressLCD, 20, 4);
                _lcd->init();
            }
            break;

        case Hydroponics_LCDOutputMode_16x2LCD:
            if (!_lcd) {
                _lcd = new LiquidCrystal_I2C(_i2cAddressLCD, 16, 2);
                _lcd->init();
            }
            break;

        default:
            if (_lcd) { delete _lcd; _lcd = NULL; }
            break;
    }

    return _lcd;
}

Keypad *Hydroponics::getControlKeypad()
{
    switch (getControlInputMode()) {
        case Hydroponics_ControlInputMode_2x2Matrix: {
            if (!_keypad) {
                char keys[2][2] = {
                    {'D','L'},
                    {'R','U'}
                };
                byte rowPins[2] = { _ctrlInputPin1, _ctrlInputPin1+1 };
                byte colPins[2] = { _ctrlInputPin1+2, _ctrlInputPin1+3 };
                _keypad = new Keypad(makeKeymap(keys), rowPins, colPins, 2, 2 );
            }
        } break;

        default:
            if (_keypad) { delete _keypad; _keypad = NULL; }
            break;
    }

    return _keypad;
}

int Hydroponics::getRelayCount(Hydroponics_RelayRail relayRail) const
{
    // TODO
    return 0;
}

int Hydroponics::getActiveRelayCount(Hydroponics_RelayRail relayRail) const
{
    // TODO
    return 0;
}

uint8_t Hydroponics::getMaxActiveRelayCount(Hydroponics_RelayRail relayRail) const
{
    // TODO assert params
    assert(!(_systemData && "System data not yet initialized"));
    return _systemData ? _systemData->maxActiveRelayCount[relayRail] : 0;
}

int Hydroponics::getActuatorCount() const
{
    // TODO
    return 0;
}

int Hydroponics::getSensorCount() const
{
    // TODO
    return 0;
}

int Hydroponics::getCropCount() const
{
    // TODO
    return 0;
}

const char * Hydroponics::getSystemName() const
{
    assert(!(_systemData && "System data not yet initialized"));
    return _systemData ? &_systemData->systemName[0] : NULL;
}

uint8_t Hydroponics::getCropPositionsCount() const
{
    assert(!(_systemData && "System data not yet initialized"));
    return _systemData ? _systemData->cropPositionsCount : 0;
}

float Hydroponics::getReservoirSize(Hydroponics_FluidReservoir fluidReservoir) const
{
    assert(!(_systemData && "System data not yet initialized"));
    return _systemData ? _systemData->reservoirSize[fluidReservoir] : 0;
}

float Hydroponics::getPumpFlowRate(Hydroponics_FluidReservoir fluidReservoir) const
{
    assert(!(_systemData && "System data not yet initialized"));
    return _systemData ? _systemData->pumpFlowRate[fluidReservoir] : 0;
}

void Hydroponics::setMaxActiveRelayCount(uint8_t maxActiveCount, Hydroponics_RelayRail relayRail)
{
    // TODO assert params
    assert(!(_systemData && "System data not yet initialized"));
    if (_systemData) {
        _systemData->maxActiveRelayCount[relayRail] = maxActiveCount;
    }
}

void Hydroponics::setSystemName(const char * systemName)
{
    // TODO assert params
    assert(!(_systemData && "System data not yet initialized"));
    if (_systemData) {
        strncpy(&_systemData->systemName[0], systemName, HYDRO_NAME_MAXSIZE);
        // TODO lcd update
    }
}

void Hydroponics::setCropPositionsCount(uint8_t cropPositionsCount)
{
    // TODO assert params
    assert(!(_systemData && "System data not yet initialized"));
    if (_systemData) {
        _systemData->cropPositionsCount = cropPositionsCount;
        // TODO remove out-of-range crops, lcd update
    }
}

void Hydroponics::setReservoirSize(float reservoirSize, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert params
    assert(!(_systemData && "System data not yet initialized"));
    if (_systemData) {
        _systemData->reservoirSize[fluidReservoir] = reservoirSize;
        // TODO lcd update
    }
}

void Hydroponics::setPumpFlowRate(float pumpFlowRate, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert params
    assert(!(_systemData && "System data not yet initialized"));
    if (_systemData) {
        _systemData->pumpFlowRate[fluidReservoir] = pumpFlowRate;
        // TODO updates?
    }
}

void Hydroponics::setUserDelayFuncs(UserDelayFunc delayMillisFunc, UserDelayFunc delayMicrosFunc)
{
    // TODO assert params
    _uDelayMillisFunc = delayMillisFunc ? delayMillisFunc : uDelayMillisFuncDef;
    _uDelayMicrosFunc = delayMicrosFunc ? delayMicrosFunc : uDelayMicrosFuncDef;
}
