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


HydroponicsSystemData::HydroponicsSystemData()
    : _ident({'H','S', 'D'}), _version(1),
      systemName({'H','y','d','r','o','d','u','i','n','o', '\0'}),
      cropPositionsCount(16), maxActiveRelayCount({2})
{
    memset(reservoirSize, 0, sizeof(reservoirSize));
    memset(pumpFlowRate, 0, sizeof(pumpFlowRate));
    memset(calibrationData, 0, sizeof(calibrationData));

    for (int calibIndex = 0; calibIndex < HYDRO_CALIB_MAXSIZE; ++calibIndex) {
        calibrationData[calibIndex].sensor = Hydroponics_SensorType_Undefined;
        calibrationData[calibIndex].reservoir = Hydroponics_FluidReservoir_Undefined;
    }
}


Hydroponics::Hydroponics(byte piezoBuzzerPin, byte i2cAddressEEPROM, byte i2cAddressRTC,
                         byte i2cAddressLCD, byte controlInputPin1,
                         TwoWire& i2cWire, uint32_t i2cSpeed)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed),
      _buzzer(&EasyBuzzer), _eeprom(new I2C_eeprom(i2cAddressEEPROM, HYDRO_EEPROM_MEMORYSIZE, &i2cWire)), _rtc(new RTC_DS3231()),
      _eepromBegan(false), _rtcBegan(false), _lcd(NULL), _keypad(NULL), _systemData(NULL),
      _i2cAddressLCD(i2cAddressLCD), _controlInputPin1(controlInputPin1),
      _waterResMode(Hydroponics_WaterReservoirMode_Undefined), _tempMode(Hydroponics_TemperatureMode_Undefined),
      _lcdOutMode(Hydroponics_LCDOutputMode_Undefined), _ctrlInMode(Hydroponics_ControlInputMode_Undefined),
      _uDelayMillisFunc(uDelayMillisFuncDef), _uDelayMicrosFunc(uDelayMicrosFuncDef)
{
    if (piezoBuzzerPin) {
        _buzzer->setPin(piezoBuzzerPin);
    }
}

Hydroponics::Hydroponics(TwoWire& i2cWire, uint32_t i2cSpeed,
                         byte piezoBuzzerPin, byte i2cAddressEEPROM, byte i2cAddressRTC,
                         byte i2cAddressLCD, byte controlInputPin1)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed),
      _buzzer(&EasyBuzzer), _eeprom(new I2C_eeprom(i2cAddressEEPROM, HYDRO_EEPROM_MEMORYSIZE, &i2cWire)), _rtc(new RTC_DS3231()),
      _eepromBegan(false), _rtcBegan(false), _lcd(NULL), _keypad(NULL), _systemData(NULL),
      _i2cAddressLCD(i2cAddressLCD), _controlInputPin1(controlInputPin1),
      _waterResMode(Hydroponics_WaterReservoirMode_Undefined), _tempMode(Hydroponics_TemperatureMode_Undefined),
      _lcdOutMode(Hydroponics_LCDOutputMode_Undefined), _ctrlInMode(Hydroponics_ControlInputMode_Undefined),
      _uDelayMillisFunc(uDelayMillisFuncDef), _uDelayMicrosFunc(uDelayMicrosFuncDef)
{
    if (piezoBuzzerPin) {
        _buzzer->setPin(piezoBuzzerPin);
    }
}

Hydroponics::~Hydroponics()
{
    _i2cWire = NULL;
    _buzzer = NULL;
    if (_eeprom) { delete _eeprom; _eeprom = NULL; }
    if (_rtc) { delete _rtc; _rtc = NULL; }
    if (_lcd) { delete _lcd; _lcd = NULL; }
    if (_keypad) { delete _keypad; _keypad = NULL; }
    if (_systemData) { delete _systemData; _systemData = NULL; }
    // TODO
}

void Hydroponics::init(Hydroponics_WaterReservoirMode waterResMode,
                       Hydroponics_TemperatureMode tempMode,
                       Hydroponics_LCDOutputMode lcdOutMode,
                       Hydroponics_ControlInputMode ctrlInMode)
{
    assert(!((int)waterResMode >= 0 && waterResMode < Hydroponics_WaterReservoirMode_Count && "Invalid water reservoir mode"));
    assert(!((int)tempMode >= 0 && tempMode < Hydroponics_TemperatureMode_Count && "Invalid temperature mode"));
    assert(!((int)lcdOutMode >= 0 && lcdOutMode < Hydroponics_LCDOutputMode_Count && "Invalid LCD output mode"));
    assert(!((int)ctrlInMode >= 0 && ctrlInMode < Hydroponics_ControlInputMode_Count && "Invalid control input mode"));

    _waterResMode = waterResMode;
    _tempMode = tempMode;
    _lcdOutMode = lcdOutMode;
    _ctrlInMode = ctrlInMode;

    // Forces begin on these if not already
    getEEPROM();
    getRealTimeClock();
    getLiquidCrystalDisplay();
    getControlKeypad();    

    if (!_systemData) { _systemData = new HydroponicsSystemData(); }

    // TODO
}

void Hydroponics::update()
{
    if (_buzzer) { _buzzer->update(); }
    // TODO
}

void Hydroponics::registerActuator(HydroponicsActuator *actuator)
{
    // TODO
}

void Hydroponics::unregisterActuator(HydroponicsActuator *actuator)
{
    // TODO
}

HydroponicsActuator *Hydroponics::addGrowLightsRelay(byte outputPin)
{
    // TODO assert outputPin in digital
    HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin, Hydroponics_ActuatorType_GrowLightsRelay, Hydroponics_RelayRail_ACRail);
    registerActuator(actuator);
    return actuator;
}

HydroponicsActuator *Hydroponics::addWaterPumpRelay(byte outputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert outputPin in digital
    HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin, Hydroponics_ActuatorType_WaterPumpRelay, Hydroponics_RelayRail_ACRail, fluidReservoir);
    registerActuator(actuator);
    return actuator;
}

HydroponicsActuator *Hydroponics::addWaterHeaterRelay(byte outputPin)
{
    // TODO assert outputPin in digital
    HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin, Hydroponics_ActuatorType_WaterHeaterRelay, Hydroponics_RelayRail_ACRail);
    registerActuator(actuator);
    return actuator;
}

HydroponicsActuator *Hydroponics::addWaterAeratorRelay(byte outputPin)
{
    // TODO assert outputPin in digital
    HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin, Hydroponics_ActuatorType_WaterAeratorRelay, Hydroponics_RelayRail_ACRail);
    registerActuator(actuator);
    return actuator;
}

HydroponicsActuator *Hydroponics::addFanCirculationRelay(byte outputPin)
{
    // TODO assert outputPin in digital
    HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin, Hydroponics_ActuatorType_FanCirculationRelay, Hydroponics_RelayRail_ACRail);
    registerActuator(actuator);
    return actuator;
}

HydroponicsActuator *Hydroponics::addFanExhaustRelay(byte outputPin)
{
    // TODO assert outputPin in digital
    HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin, Hydroponics_ActuatorType_FanExhaustRelay, Hydroponics_RelayRail_ACRail);
    registerActuator(actuator);
    return actuator;
}

HydroponicsActuator *Hydroponics::addPhUpPeristalticPumpRelay(byte outputPin)
{
    // TODO assert outputPin in digital
    HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin, Hydroponics_ActuatorType_PeristalticPumpRelay, Hydroponics_RelayRail_DCRail, Hydroponics_FluidReservoir_PhUpSolution);
    registerActuator(actuator);
    return actuator;
}

HydroponicsActuator *Hydroponics::addPhDownPeristalticPumpRelay(byte outputPin)
{
    // TODO assert outputPin in digital
    HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin, Hydroponics_ActuatorType_PeristalticPumpRelay, Hydroponics_RelayRail_DCRail, Hydroponics_FluidReservoir_PhDownSolution);
    registerActuator(actuator);
    return actuator;
}

HydroponicsActuator *Hydroponics::addNutrientPremixPeristalticPumpRelay(byte outputPin)
{
    // TODO assert outputPin in digital
    HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin, Hydroponics_ActuatorType_PeristalticPumpRelay, Hydroponics_RelayRail_DCRail, Hydroponics_FluidReservoir_NutrientPremix);
    registerActuator(actuator);
    return actuator;
}

HydroponicsActuator *Hydroponics::addFreshWaterPeristalticPumpRelay(byte outputPin)
{
    // TODO assert outputPin in digital
    HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(outputPin, Hydroponics_ActuatorType_PeristalticPumpRelay, Hydroponics_RelayRail_DCRail, Hydroponics_FluidReservoir_FreshWater);
    registerActuator(actuator);
    return actuator;
}

void Hydroponics::registerSensor(HydroponicsSensor *sensor)
{
    // TODO
}

void Hydroponics::unregisterSensor(HydroponicsSensor *sensor)
{
    // TODO
}

HydroponicsSensor *Hydroponics::addAirDHTTempHumiditySensor(OneWire &oneWire)
{
    HydroponicsOneWireSensor *sensor = new HydroponicsOneWireSensor(oneWire, Hydroponics_SensorType_AirTempHumidity);
    // TODO switch to proper constructor, set saved calibration data
    registerSensor(sensor);
    return sensor;
}

HydroponicsSensor *Hydroponics::addWaterPhMeter(byte inputPin)
{
    // TODO assert inputPin in AnalogIn
    HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin, Hydroponics_SensorType_PotentialHydrogen);
    // TODO set saved calibration data
    registerSensor(sensor);
    return sensor;
}

HydroponicsSensor *Hydroponics::addWaterTDSElectrode(byte inputPin)
{
    // TODO assert inputPin in AnalogIn
    HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin, Hydroponics_SensorType_TotalDissolvedSolids);
    // TODO set saved calibration data
    registerSensor(sensor);
    return sensor;
}

HydroponicsSensor *Hydroponics::addWaterDSTempSensor(byte inputPin)
{
    // TODO assert inputPin in AnalogIn
    HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin, Hydroponics_SensorType_WaterTemperature);
    // TODO set saved calibration data
    registerSensor(sensor);
    return sensor;
}

HydroponicsSensor *Hydroponics::addWaterPumpFlowSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert inputPin in PWM
    HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin, Hydroponics_SensorType_WaterPumpFlowSensor, fluidReservoir);
    // TODO set saved calibration data
    registerSensor(sensor);
    return sensor;
}

HydroponicsSensor *Hydroponics::addLowWaterLevelIndicator(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert inputPin in digital
    HydroponicsBinarySensor *sensor = new HydroponicsBinarySensor(inputPin, Hydroponics_SensorType_LowWaterLevelIndicator, fluidReservoir, true);
    // TODO test activeLow setting
    registerSensor(sensor);
    return sensor;
}

HydroponicsSensor *Hydroponics::addHighWaterLevelIndicator(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert inputPin in digital
    HydroponicsBinarySensor *sensor = new HydroponicsBinarySensor(inputPin, Hydroponics_SensorType_HighWaterLevelIndicator, fluidReservoir, false);
    // TODO test activeLow setting
    registerSensor(sensor);
    return sensor;
}

HydroponicsSensor *Hydroponics::addLowWaterHeightMeter(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert inputPin in AnalogIn
    HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin, 0.1, true, Hydroponics_SensorType_LowWaterHeightMeter, fluidReservoir);
    // TODO actual tolerance value, set saved calibration data
    registerSensor(sensor);
    return sensor;
}

HydroponicsSensor *Hydroponics::addHighWaterHeightMeter(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert inputPin in AnalogIn
    HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin, 0.9, false, Hydroponics_SensorType_HighWaterHeightMeter, fluidReservoir);
    // TODO actual tolerance value, set saved calibration data
    registerSensor(sensor);
    return sensor;
}

HydroponicsSensor *Hydroponics::addLowWaterUltrasonicSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert inputPin in AnalogIn
    HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin, 0.9, false, Hydroponics_SensorType_LowWaterHeightMeter, fluidReservoir);
    // TODO actual tolerance value, set saved calibration data
    registerSensor(sensor);
    return sensor;
}

HydroponicsSensor *Hydroponics::addHighWaterUltrasonicSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert inputPin in AnalogIn
    HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin, 0.1, true, Hydroponics_SensorType_LowWaterHeightMeter, fluidReservoir);
    // TODO actual tolerance value, set saved calibration data
    registerSensor(sensor);
    return sensor;
}

void Hydroponics::registerCrop(HydroponicsCrop *crop)
{
    // TODO
}

void Hydroponics::unregisterCrop(HydroponicsCrop *crop)
{
    // TODO
}

HydroponicsCrop *Hydroponics::addCropFromSowDate(const Hydroponics_CropType cropType, time_t sowDate, int positionIndex)
{
    // TODO assert cropType valid, sowDate validation, position index validation or next new lookup when -1
    HydroponicsCrop *crop = new HydroponicsCrop(cropType, positionIndex, sowDate);
    registerCrop(crop);
    return crop;
}

HydroponicsCrop *Hydroponics::addCropFromLastHarvest(const Hydroponics_CropType cropType, time_t lastHarvestDate, int positionIndex)
{
    // TODO assert cropType valid, position index validation or next new lookup when -1
    time_t sowDate = lastHarvestDate - (HydroponicsCropsLibrary::getInstance()->getCropData(cropType)->weeksBetweenHarvest * SECS_PER_WEEK);
    HydroponicsCrop *crop = new HydroponicsCrop(cropType, positionIndex, sowDate);
    registerCrop(crop);
    return crop;
}

Hydroponics_WaterReservoirMode Hydroponics::getWaterReservoirMode() const
{
    return _waterResMode;
}

Hydroponics_TemperatureMode Hydroponics::getTemperatureMode() const
{
    return _tempMode;
}

Hydroponics_LCDOutputMode Hydroponics::getLCDOutputMode() const
{
    return _lcdOutMode;
}

Hydroponics_ControlInputMode Hydroponics::getControlInputMode() const
{
    return _ctrlInMode;
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

LiquidCrystal_I2C *Hydroponics::getLiquidCrystalDisplay()
{
    switch(_lcdOutMode) {
        case Hydroponics_LCDOutputMode_20x4LCD:
        case Hydroponics_LCDOutputMode_20x4LCD_Reversed:
            if (!_lcd) {
                _lcd = new LiquidCrystal_I2C(_i2cAddressLCD, 20, 4);
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
    switch(_ctrlInMode) {
        case Hydroponics_ControlInputMode_2x2Directional: {
            if (!_keypad) {
                char keys[2][2] = {
                    {'D','L'},
                    {'R','U'}
                };
                byte rowPins[2] = { _controlInputPin1, _controlInputPin1+1 };
                byte colPins[2] = { _controlInputPin1+2, _controlInputPin1+3 };
                _keypad = new Keypad(makeKeymap(keys), rowPins, colPins, 2, 2 );
            }
        } break;

        case Hydroponics_ControlInputMode_2x2Directional_Reversed: {
            if (!_keypad) {
                char keys[2][2] = {
                    {'U','R'},
                    {'L','D'}
                };
                byte rowPins[2] = { _controlInputPin1, _controlInputPin1+1 };
                byte colPins[2] = { _controlInputPin1+2, _controlInputPin1+3 };
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
    // TODO assert params systemData
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

const char * Hydroponics::getSystemName() const
{
    // TODO assert systemData
    return _systemData ? &_systemData->systemName[0] : NULL;
}

uint8_t Hydroponics::getCropPositionsCount() const
{
    // TODO assert systemData
    return _systemData ? _systemData->cropPositionsCount : 0;
}

float Hydroponics::getReservoirSize(Hydroponics_FluidReservoir fluidReservoir) const
{
    // TODO assert params systemData
    return _systemData ? _systemData->reservoirSize[fluidReservoir] : 0;
}

float Hydroponics::getPumpFlowRate(Hydroponics_FluidReservoir fluidReservoir) const
{
    // TODO assert params systemData
    return _systemData ? _systemData->pumpFlowRate[fluidReservoir] : 0;
}

void Hydroponics::setMaxActiveRelayCount(uint8_t maxActiveCount, Hydroponics_RelayRail relayRail)
{
    // TODO assert params systemData
    if (_systemData) {
        _systemData->maxActiveRelayCount[relayRail] = maxActiveCount;
    }
}

void Hydroponics::setSystemName(const char * systemName)
{
    // TODO assert params systemData
    if (_systemData) {
        strncpy(&_systemData->systemName[0], systemName, HYDRO_NAME_MAXSIZE);
        // TODO lcd update
    }
}

void Hydroponics::setCropPositionsCount(uint8_t cropPositionsCount)
{
    // TODO assert params systemData
    if (_systemData) {
        _systemData->cropPositionsCount = cropPositionsCount;
        // TODO remove out-of-range crops, lcd update
    }
}

void Hydroponics::setReservoirSize(float reservoirSize, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert params systemData
    if (_systemData) {
        _systemData->reservoirSize[fluidReservoir] = reservoirSize;
        // TODO lcd update
    }
}

void Hydroponics::setPumpFlowRate(float pumpFlowRate, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert params systemData
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
