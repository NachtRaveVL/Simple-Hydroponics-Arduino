/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Main
*/

#include "Hydroponics.h"

static void uDelayMillisFuncDef(unsigned int timeout) {
#ifndef HYDRUINO_DISABLE_MULTITASKING
    if (timeout > 0) {
        unsigned long currTime = millis();
        unsigned long endTime = currTime + (unsigned long)timeout;
        if (currTime < endTime) { // not overflowing
            while (millis() < endTime)
                yield();
        } else { // overflowing
            unsigned long begTime = currTime;
            while (currTime >= begTime || currTime < endTime) {
                yield();
                currTime = millis();
            }
        }
    } else {
        yield();
    }
#else
    delay(timeout);
#endif
}

static void uDelayMicrosFuncDef(unsigned int timeout) {
#ifndef HYDRUINO_DISABLE_MULTITASKING
    if (timeout > 1000) {
        unsigned long currTime = micros();
        unsigned long endTime = currTime + (unsigned long)timeout;
        if (currTime < endTime) { // not overflowing
            while (micros() < endTime)
                yield();
        } else { // overflowing
            unsigned long begTime = currTime;
            while (currTime >= begTime || currTime < endTime) {
                yield();
                currTime = micros();
            }
        }
    } else if (timeout > 0) {
        delayMicroseconds(timeout);
    } else {
        yield();
    }
#else
    delayMicroseconds(timeout);
#endif
}


static RTC_DS3231 *_rtcSyncProvider = NULL;
time_t rtcNow() {
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}


HydroponicsData::HydroponicsData(const char *ident, uint16_t version, uint16_t revision)
    : _ident{'\0'}, _version(version), _revision(revision)
{
    //assert(ident && "Invalid identity");
    strncpy(_ident, ident, 4);
}

HydroponicsSystemData::HydroponicsSystemData()
    : HydroponicsData("HSYS", 1),
      systemName{'\0'},
      cropPositionsCount(16), maxActiveRelayCount{2},
      reservoirSizeUnits(Hydroponics_UnitsType_Undefined),
      pumpFlowRateUnits(Hydroponics_UnitsType_Undefined)
{
    strncpy(systemName, String(F("Hydruino")).c_str(), HYDRUINO_NAME_MAXSIZE);
    memset(reservoirSize, 0, sizeof(reservoirSize));
    memset(pumpFlowRate, 0, sizeof(pumpFlowRate));

    // Moving this to its own thing
    //memset(calibrationData, 0, sizeof(calibrationData));
}

void HydroponicsSystemData::toJSONDocument(JsonDocument &docOut) const
{
    // TODO
}

void HydroponicsSystemData::fromJSONDocument(const JsonDocument &docIn)
{
    // TODO
}


Hydroponics *Hydroponics::_activeInstance = NULL;

Hydroponics::Hydroponics(byte piezoBuzzerPin, byte sdCardCSPin, byte controlInputPin1,
                         byte eepromI2CAddress, byte rtcI2CAddress, byte lcdI2CAddress,
                         TwoWire& i2cWire, uint32_t i2cSpeed, uint32_t spiSpeed)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed), _spiSpeed(spiSpeed),
      _piezoBuzzerPin(piezoBuzzerPin), _sdCardCSPin(sdCardCSPin), _ctrlInputPin1(controlInputPin1), _ctrlInputPinMap{-1},
      _eepromI2CAddr(eepromI2CAddress), _rtcI2CAddr(rtcI2CAddress), _lcdI2CAddr(lcdI2CAddress),
      _buzzer(&EasyBuzzer), _eeprom(NULL), _rtc(NULL), _sd(NULL), _eepromBegan(false), _rtcBegan(false), _rtcBattFail(false),
      _systemData(NULL),
      _uDelayMillisFunc(uDelayMillisFuncDef), _uDelayMicrosFunc(uDelayMicrosFuncDef)
{
    if (isValidPin(_piezoBuzzerPin)) {
        _buzzer->setPin(_piezoBuzzerPin);
    }
    if (isValidPin(_ctrlInputPin1)) {
        for (int pinIndex = 0; pinIndex < HYDRUINO_CTRLINPINMAP_MAXSIZE; ++pinIndex) {
            _ctrlInputPinMap[pinIndex] = _ctrlInputPin1 + pinIndex;
        }
    }
}

Hydroponics::Hydroponics(TwoWire& i2cWire, uint32_t i2cSpeed, uint32_t spiSpeed,
                         byte piezoBuzzerPin, byte sdCardCSPin, byte controlInputPin1,
                         byte eepromI2CAddress, byte rtcI2CAddress, byte lcdI2CAddress)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed), _spiSpeed(spiSpeed),
      _piezoBuzzerPin(piezoBuzzerPin), _sdCardCSPin(sdCardCSPin), _ctrlInputPin1(controlInputPin1), _ctrlInputPinMap{-1},
      _eepromI2CAddr(eepromI2CAddress), _rtcI2CAddr(rtcI2CAddress), _lcdI2CAddr(lcdI2CAddress),
      _buzzer(&EasyBuzzer), _eeprom(NULL), _rtc(NULL), _sd(NULL),
      _eepromBegan(false), _rtcBegan(false), _rtcBattFail(false),
      _systemData(NULL),
      _uDelayMillisFunc(uDelayMillisFuncDef), _uDelayMicrosFunc(uDelayMicrosFuncDef)
{
    if (isValidPin(_piezoBuzzerPin)) {
        _buzzer->setPin(_piezoBuzzerPin);
    }
    if (isValidPin(_ctrlInputPin1)) {
        for (int pinIndex = 0; pinIndex < HYDRUINO_CTRLINPINMAP_MAXSIZE; ++pinIndex) {
            _ctrlInputPinMap[pinIndex] = _ctrlInputPin1 + pinIndex;
        }
    }
}

Hydroponics::~Hydroponics()
{
    if (this == _activeInstance) { _activeInstance = NULL; }
    _i2cWire = NULL;
    _buzzer = NULL;
    deallocateEEPROM();
    deallocateRTC();
    deallocateSD();
    if (_systemData) { delete _systemData; _systemData = NULL; }
}

void Hydroponics::allocateEEPROM()
{
    //assert(!_eeprom && "EEPROM already allocated");
    _eeprom = new I2C_eeprom(_eepromI2CAddr, HYDRUINO_EEPROM_MEMORYSIZE, _i2cWire);
    _eepromBegan = false;
}

void Hydroponics::deallocateEEPROM()
{
    if (_eeprom) { delete _eeprom; _eeprom = NULL; }
}

void Hydroponics::allocateRTC()
{
    //assert(!_rtc && "RTC already allocated");
    _rtc = new RTC_DS3231();
    //assert(_rtcI2CAddr == B000 && "RTClib does not support RTC multiaddressing");
    _rtcBegan = false;
}

void Hydroponics::deallocateRTC()
{
    if (_rtc) {
        if (_rtcSyncProvider == _rtc) { setSyncProvider(NULL); _rtcSyncProvider = NULL; }
        delete _rtc; _rtc = NULL;
    }
}

void Hydroponics::allocateSD()
{
    #if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)) || defined(SD)
        _sd = &SD;
    #else
        //assert(!_sd && "SD already allocated");
        _sd = new SDClass();
    #endif
}

void Hydroponics::deallocateSD()
{
    #if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)) || defined(SD)
        _sd = NULL;
    #else
        if (_sd) { delete _sd; _sd = NULL; }
    #endif
}

void Hydroponics::init(Hydroponics_SystemMode systemMode,
                       Hydroponics_MeasurementMode measurementMode,
                       Hydroponics_LCDOutputMode lcdOutMode,
                       Hydroponics_ControlInputMode ctrlInMode)
{
    //assert(!_systemData && "Controller already initialized");
    //assert(!_activeInstance && "Controller already active");

    if (!_systemData && !_activeInstance) {
        //assert((int)systemMode >= 0 && systemMode < Hydroponics_SystemMode_Count && "Invalid system mode");
        //assert((int)measurementMode >= 0 && measurementMode < Hydroponics_MeasurementMode_Count && "Invalid measurement mode");
        //assert((int)lcdOutMode >= 0 && lcdOutMode < Hydroponics_LCDOutputMode_Count && "Invalid LCD output mode");
        //assert((int)ctrlInMode >= 0 && ctrlInMode < Hydroponics_ControlInputMode_Count && "Invalid control input mode");

        _systemData = new HydroponicsSystemData();
        //assert(_systemData && "Invalid system data store");

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
    //assert(!_systemData && "Controller already initialized");
    //assert(!_activeInstance && "Controller already active");

    if (!_systemData && !_activeInstance) {
        auto *eeprom = getEEPROM(); // Forces begin, if not already
        if (eeprom) {
            // TODO
            //auto *systemData = HydroponicsSystemData::fromEEPROMStore();
            //if (systemData) { _systemData = systemData; }
        }

        //assert(_systemData && "Invalid system data store");
        if (_systemData) {
            commonInit();
        }
        return _systemData;
    }

    return false;
}

bool Hydroponics::initFromSDCard(const char * configFile)
{
    //assert(!_systemData && "Controller already initialized");
    //assert(!_activeInstance && "Controller already active");

    if (!_systemData && !_activeInstance) {
        auto *sd = getSDCard();
        if (sd) {
            auto config = sd->open(configFile);
            if (config && config.size()) {
                // TODO
                //auto *systemData = HydroponicsSystemData::fromJSONDocument(configToJSON);
                //if (systemData) { _systemData = systemData; }
                config.close();
            }
            sd->end();
        }

        //assert(_systemData && "Invalid system data store");
        if (_systemData) {
            commonInit();
        }
        return _systemData;
    }

    return false;
}

void Hydroponics::commonInit()
{
    _activeInstance = this;

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

    if ((_rtcSyncProvider = getRealTimeClock())) {
        setSyncProvider(rtcNow);
    }

    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        Serial.print(F("Hydroponics::commonInit piezoBuzzerPin: "));
        Serial.print(_piezoBuzzerPin);
        Serial.print(F(", sdCardCSPin: "));
        Serial.print(_sdCardCSPin);
        Serial.print(F(", controlInputPin1: "));
        Serial.print(_ctrlInputPin1);
        Serial.print(F(", EEPROMi2cAddress: 0x"));
        Serial.print(_eepromI2CAddr, HEX);
        Serial.print(F(", RTCi2cAddress: 0x"));
        Serial.print(_rtcI2CAddr, HEX);
        Serial.print(F(", LCDi2cAddress: 0x"));
        Serial.print(_lcdI2CAddr, HEX);
        Serial.print(F(", i2cSpeed: "));
        Serial.print(roundf(getI2CSpeed() / 1000.0f)); Serial.print(F("kHz"));
        Serial.print(F(", SPISpeed: "));
        Serial.print(roundf(getSPISpeed() / 1000000.0f)); Serial.print(F("MHz"));
        Serial.print(F(", systemMode: "));
        switch(getSystemMode()) {
            case Hydroponics_SystemMode_Recycling: Serial.print(F("Recycling")); break;
            case Hydroponics_SystemMode_DrainToWaste: Serial.print(F("DrainToWaste")); break;
            case Hydroponics_SystemMode_Count:
            case Hydroponics_SystemMode_Undefined:
                Serial.print(getSystemMode()); break;
        }
        Serial.print(F(", measurementMode: "));
        switch (getMeasurementMode()) {
            case Hydroponics_MeasurementMode_Imperial: Serial.print(F("Imperial")); break;
            case Hydroponics_MeasurementMode_Metric: Serial.print(F("Metric")); break;
            case Hydroponics_MeasurementMode_Scientific: Serial.print(F("Scientific")); break;
            case Hydroponics_MeasurementMode_Count:
            case Hydroponics_MeasurementMode_Undefined:
                Serial.print(getMeasurementMode()); break;
        }
        Serial.print(F(", lcdOutMode: "));
        switch (getLCDOutputMode()) {
            case Hydroponics_LCDOutputMode_Disabled: Serial.print(F("Disabled")); break;
            case Hydroponics_LCDOutputMode_20x4LCD: Serial.print(F("20x4LCD")); break;
            case Hydroponics_LCDOutputMode_20x4LCD_Swapped: Serial.print(F("20x4LCD <Swapped>")); break;
            case Hydroponics_LCDOutputMode_16x2LCD: Serial.print(F("16x2LCD")); break;
            case Hydroponics_LCDOutputMode_16x2LCD_Swapped: Serial.print(F("16x2LCD <Swapped>")); break;
            case Hydroponics_LCDOutputMode_Count:
            case Hydroponics_LCDOutputMode_Undefined:
                Serial.print(getLCDOutputMode()); break;
        }
        Serial.print(F(", ctrlInMode: "));
        switch (getControlInputMode()) {
            case Hydroponics_ControlInputMode_Disabled: Serial.print(F("Disabled")); break;
            case Hydroponics_ControlInputMode_2x2Matrix: Serial.print(F("2x2Matrix")); break;
            case Hydroponics_ControlInputMode_4xButton: Serial.print(F("4xButton")); break;
            case Hydroponics_ControlInputMode_6xButton: Serial.print(F("6xButton")); break;
            case Hydroponics_ControlInputMode_RotaryEncoder: Serial.print(F("RotaryEncoder")); break;
            case Hydroponics_ControlInputMode_Count:
            case Hydroponics_ControlInputMode_Undefined:
                Serial.print(getControlInputMode()); break;
        }
        Serial.println("");
    #endif
}

void controlLoop()
{
    Hydroponics *hydroponics = Hydroponics::getActiveInstance();
    if (hydroponics) {
        hydroponics->updateCrops();
        hydroponics->updateScheduling();
        hydroponics->updateActuators();
    }

    yield();
}

void dataLoop()
{
    Hydroponics *hydroponics = Hydroponics::getActiveInstance();
    if (hydroponics) {
        hydroponics->updateSensors();
        hydroponics->updateLogging();
    }

    yield();
}

void guiLoop()
{
    Hydroponics *hydroponics = Hydroponics::getActiveInstance();
    if (hydroponics) {
        hydroponics->updateScreen();
    }

    yield();
}

void miscLoop()
{
    Hydroponics *hydroponics = Hydroponics::getActiveInstance();
    if (hydroponics) {
        hydroponics->updateBuzzer();
    }

    yield();
}

void Hydroponics::launch()
{
    #if defined(HYDRUINO_USE_TASKSCHEDULER)
        Task *controlTask = new Task(1000 * TASK_MILLISECOND, TASK_FOREVER, controlLoop, &_ts, true);
        Task *dataTask = new Task(100 * TASK_MILLISECOND, TASK_FOREVER, dataLoop, &_ts, true);
        Task *guiTask = new Task(20 * TASK_MILLISECOND, TASK_FOREVER, guiLoop, &_ts, true);
        Task *miscTask = new Task(TASK_IMMEDIATE, TASK_FOREVER, miscLoop, &_ts, true);
    #elif defined(HYDRUINO_USE_SCHEDULER)
        Scheduler.startLoop(controlLoop);
        Scheduler.startLoop(dataLoop);
        Scheduler.startLoop(guiLoop);
        Scheduler.startLoop(miscLoop);
    #elif defined(HYDRUINO_USE_COOPTASK)
        createCoopTask<void, CoopTaskStackAllocatorFromLoop<>>("controlLoop", controlLoop);
        createCoopTask<void, CoopTaskStackAllocatorFromLoop<>>("dataLoop", dataLoop);
        createCoopTask<void, CoopTaskStackAllocatorFromLoop<>>("guiLoop", guiLoop);
        createCoopTask<void, CoopTaskStackAllocatorFromLoop<>>("miscLoop", miscLoop);
    #endif
}

void Hydroponics::update()
{
    #ifndef HYDRUINO_DISABLE_MULTITASKING
        #ifdef HYDRUINO_USE_TASKSCHEDULER
            HYDRUINO_MAINLOOP(_ts);
        #else
            HYDRUINO_MAINLOOP();
        #endif
        taskManager.runLoop(); // tcMenu uses this system to run its UI
    #else
        controlLoop();
        dataLoop();
        guiLoop();
        miscLoop();
    #endif
}

void Hydroponics::updateActuators()
{
    // TODO
    // for (auto actuator in actuators.copy()) {
    //     actuator->update();
    // }
}

void Hydroponics::updateBuzzer()
{
    auto *buzzer = getPiezoBuzzer();
    if (buzzer) {
        buzzer->update();
    }
}

void Hydroponics::updateCrops()
{
    // TODO
    // for (auto crop in crops.copy()) {
    //     crop->update();
    // }
}

void Hydroponics::updateLogging()
{
    // TODO
}

void Hydroponics::updateScheduling()
{
    // TODO
}

void Hydroponics::updateScreen()
{
    // TODO
}

void Hydroponics::updateSensors()
{
    // TODO
    // for (auto sensor in sensors.copy()) {
    //     sensor->update();
    // }
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
    //assert(outputPinIsDigital && "Output pin is not digital");

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
    //assert(outputPinIsDigital && "Output pin is not digital");

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
    //assert(outputPinIsDigital && "Output pin is not digital");

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
    //assert(outputPinIsDigital && "Output pin is not digital");

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
    //assert(outputPinIsDigital && "Output pin is not digital");

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
    //assert(outputPinIsPWM && "Output pin does not support PWM");

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
    //assert(outputPinIsDigital && "Output pin is not digital");

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
    //assert(outputPinIsDigital && "Output pin is not digital");

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
    //assert(outputPinIsDigital && "Output pin is not digital");

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
    //assert(outputPinIsDigital && "Output pin is not digital");

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
    //assert(inputPinIsDigital && "Input pin is not digital");

    if (inputPinIsDigital) {
        HydroponicsDHTSensor *sensor = new HydroponicsDHTSensor(inputPin,
                                                                Hydroponics_FluidReservoir_FeedWater,
                                                                getMeasurementMode(),
                                                                dhtType);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsAnalogSensor *Hydroponics::addAirCO2Sensor(byte inputPin, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin,
                                                                      Hydroponics_SensorType_AirCarbonDioxide,
                                                                      Hydroponics_FluidReservoir_Undefined,
                                                                      getMeasurementMode(),
                                                                      readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsAnalogSensor *Hydroponics::addWaterPhMeter(byte inputPin, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin,
                                                                      Hydroponics_SensorType_PotentialHydrogen,
                                                                      Hydroponics_FluidReservoir_FeedWater,
                                                                      getMeasurementMode(),
                                                                      readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsAnalogSensor *Hydroponics::addWaterTDSElectrode(byte inputPin, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin,
                                                                      Hydroponics_SensorType_TotalDissolvedSolids,
                                                                      Hydroponics_FluidReservoir_FeedWater,
                                                                      getMeasurementMode(),
                                                                      readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsDSSensor *Hydroponics::addWaterDSTempSensor(byte inputPin, byte readBitResolution)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    //assert(inputPinIsDigital && "Input pin is not digital");

    if (inputPinIsDigital) {
        HydroponicsDSSensor *sensor = new HydroponicsDSSensor(inputPin,
                                                              Hydroponics_FluidReservoir_FeedWater,
                                                              getMeasurementMode(),
                                                              readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsAnalogSensor *Hydroponics::addWaterPumpFlowSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(inputPin,
                                                                      Hydroponics_SensorType_WaterPumpFlowSensor,
                                                                      fluidReservoir,
                                                                      getMeasurementMode(),
                                                                      readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinarySensor *Hydroponics::addLowWaterLevelIndicator(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    //assert(inputPinIsDigital && "Input pin is not digital");

    if (inputPinIsDigital) {
        HydroponicsBinarySensor *sensor = new HydroponicsBinarySensor(inputPin,
                                                                      Hydroponics_SensorType_LowWaterLevelIndicator,
                                                                      fluidReservoir,
                                                                      getMeasurementMode(),
                                                                      true);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinarySensor *Hydroponics::addHighWaterLevelIndicator(byte inputPin, Hydroponics_FluidReservoir fluidReservoir)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    //assert(inputPinIsDigital && "Input pin is not digital");

    if (inputPinIsDigital) {
        HydroponicsBinarySensor *sensor = new HydroponicsBinarySensor(inputPin,
                                                                      Hydroponics_SensorType_HighWaterLevelIndicator,
                                                                      fluidReservoir,
                                                                      getMeasurementMode(),
                                                                      false);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinaryAnalogSensor *Hydroponics::addLowWaterHeightMeter(byte inputPin, Hydroponics_FluidReservoir fluidReservoir, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        // TODO: actual tolerance value
        HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin,
                                                                                  0.1, true,
                                                                                  Hydroponics_SensorType_LowWaterHeightMeter,
                                                                                  fluidReservoir,
                                                                                  getMeasurementMode(),
                                                                                  readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinaryAnalogSensor *Hydroponics::addHighWaterHeightMeter(byte inputPin, Hydroponics_FluidReservoir fluidReservoir, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        // TODO: actual tolerance value
        HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin,
                                                                                  0.9, false,
                                                                                  Hydroponics_SensorType_HighWaterHeightMeter,
                                                                                  fluidReservoir,
                                                                                  getMeasurementMode(),
                                                                                  readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinaryAnalogSensor *Hydroponics::addLowWaterUltrasonicSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        // TODO: actual tolerance value
        HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin,
                                                                                  0.9, false,
                                                                                  Hydroponics_SensorType_LowWaterHeightMeter,
                                                                                  fluidReservoir,
                                                                                  getMeasurementMode(),
                                                                                  readBitResolution);
        if (registerSensor(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return NULL;
}

HydroponicsBinaryAnalogSensor *Hydroponics::addHighWaterUltrasonicSensor(byte inputPin, Hydroponics_FluidReservoir fluidReservoir, byte readBitResolution)
{
    bool inputPinIsAnalog = checkInputPinIsAnalog(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        // TODO: actual tolerance value
        HydroponicsBinaryAnalogSensor *sensor = new HydroponicsBinaryAnalogSensor(inputPin,
                                                                                  0.1, true,
                                                                                  Hydroponics_SensorType_LowWaterHeightMeter,
                                                                                  fluidReservoir,
                                                                                  getMeasurementMode(),
                                                                                  readBitResolution);
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
    //assert((int)cropType >= 0 && cropType <= Hydroponics_CropType_Count && "Invalid crop type");
    //assert(sowDate > 0 && "Invalid sow date");
    bool positionIsAvailable = positionIndex == -1; // TODO: position indexing.
    //assert(positionIsAvailable && "Invalid position index");

    if (positionIsAvailable) {
        HydroponicsCrop *crop = new HydroponicsCrop(cropType, positionIndex, sowDate);
        if (registerCrop(crop)) { return crop; }
        else { delete crop; }
    }

    return NULL;
}

HydroponicsCrop *Hydroponics::addCropFromLastHarvest(const Hydroponics_CropType cropType, time_t lastHarvestDate, int positionIndex)
{
    HydroponicsCropData cropData(cropType);
    time_t sowDate = lastHarvestDate - cropData.weeksBetweenHarvest * SECS_PER_WEEK;
    return addCropFromSowDate(cropType, sowDate, positionIndex);
}

Hydroponics *Hydroponics::getActiveInstance()
{
    return _activeInstance;
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

I2C_eeprom *Hydroponics::getEEPROM(bool begin)
{
    if (!_eeprom) {
        allocateEEPROM();
    }

    if (_eeprom && begin && !_eepromBegan) {
        _eepromBegan = _eeprom->begin();

        if (!_eepromBegan) {
            // TODO log failure/report not found?
            deallocateEEPROM();
        }
    }

    return _eeprom && (!begin || _eepromBegan) ? _eeprom : NULL;
}

RTC_DS3231 *Hydroponics::getRealTimeClock(bool begin)
{
    if (!_rtc) {
        allocateRTC();
    }

    if (_rtc && begin && !_rtcBegan) {
        _rtcBegan = _rtc->begin(_i2cWire);

        if (_rtcBegan) {
            _rtcBattFail = _rtc->lostPower();
        } else {
            // TODO log failure/report not found?
            deallocateRTC();
        }
    }

    return _rtc && (!begin || _rtcBegan) ? _rtc : NULL;
}

SDClass *Hydroponics::getSDCard(bool begin)
{
    if (!_sd) {
        allocateSD();
    }

    if (_sd && begin) {
        // TODO ESP8266/ESP32 differences
        bool sdBegan = _sd->begin(_spiSpeed, _sdCardCSPin);

        if (!sdBegan) {
            // TODO log failure/report not found?
            deallocateSD();
        }

        return _sd && sdBegan ? _sd : NULL;
    }

    return _sd;
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

byte Hydroponics::getMaxActiveRelayCount(Hydroponics_RelayRail relayRail) const
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
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
    //assert(_systemData && "System data not yet initialized");
    return _systemData ? &_systemData->systemName[0] : NULL;
}

byte Hydroponics::getCropPositionsCount() const
{
    //assert(_systemData && "System data not yet initialized");
    return _systemData ? _systemData->cropPositionsCount : 0;
}

float Hydroponics::getReservoirSize(Hydroponics_FluidReservoir fluidReservoir) const
{
    //assert(_systemData && "System data not yet initialized");
    return _systemData ? _systemData->reservoirSize[fluidReservoir] : 0;
}

float Hydroponics::getPumpFlowRate(Hydroponics_FluidReservoir fluidReservoir) const
{
    //assert(_systemData && "System data not yet initialized");
    return _systemData ? _systemData->pumpFlowRate[fluidReservoir] : 0;
}

int Hydroponics::getControlInputRibbonPinCount()
{
    switch (getControlInputMode()) {
        case Hydroponics_ControlInputMode_2x2Matrix:
        case Hydroponics_ControlInputMode_4xButton:
            return 4;
        case Hydroponics_ControlInputMode_6xButton:
            return 6;
        case Hydroponics_ControlInputMode_RotaryEncoder:
            return 5;
        default:
            return 0;
    }
}

byte Hydroponics::getControlInputPin(int ribbonPinIndex)
{
    int ctrlInPinCount = getControlInputRibbonPinCount();
    //assert(ctrlInPinCount > 0 && "Control input pinmap not used in this mode");
    //assert(ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount && "Ribbon pin index out of range");

    return _ctrlInputPinMap && ctrlInPinCount && ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount ? _ctrlInputPinMap[ribbonPinIndex] : -1;
}

void Hydroponics::setMaxActiveRelayCount(byte maxActiveCount, Hydroponics_RelayRail relayRail)
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
    if (_systemData) {
        _systemData->maxActiveRelayCount[relayRail] = maxActiveCount;
    }
}

void Hydroponics::setSystemName(const char * systemName)
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
    if (_systemData) {
        strncpy(&_systemData->systemName[0], systemName, HYDRUINO_NAME_MAXSIZE);
        // TODO lcd update
    }
}

void Hydroponics::setCropPositionsCount(byte cropPositionsCount)
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
    if (_systemData) {
        _systemData->cropPositionsCount = cropPositionsCount;
        // TODO remove out-of-range crops, lcd update
    }
}

void Hydroponics::setReservoirSize(float reservoirSize, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
    if (_systemData) {
        _systemData->reservoirSize[fluidReservoir] = reservoirSize;
        // TODO lcd update
    }
}

void Hydroponics::setPumpFlowRate(float pumpFlowRate, Hydroponics_FluidReservoir fluidReservoir)
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
    if (_systemData) {
        _systemData->pumpFlowRate[fluidReservoir] = pumpFlowRate;
        // TODO updates?
    }
}

void Hydroponics::setControlInputPinMap(byte *pinMap)
{
    //assert(pinMap && "Invalid pinMap");
    const int ctrlInPinCount = getControlInputRibbonPinCount();
    //assert(ctrlInPinCount > 0 && "Control input pinmap not used in this mode");

    for (int ribbonPinIndex = 0; ribbonPinIndex < ctrlInPinCount; ++ribbonPinIndex) {
        _ctrlInputPinMap[ribbonPinIndex] = pinMap[ribbonPinIndex];
    }
}

void Hydroponics::setUserDelayFuncs(UserDelayFunc delayMillisFunc, UserDelayFunc delayMicrosFunc)
{
    _uDelayMillisFunc = delayMillisFunc ? delayMillisFunc : uDelayMillisFuncDef;
    _uDelayMicrosFunc = delayMicrosFunc ? delayMicrosFunc : uDelayMicrosFuncDef;
}
