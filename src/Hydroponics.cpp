/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Main
*/

#include "Hydroponics.h"

static RTC_DS3231 *_rtcSyncProvider = nullptr;
time_t rtcNow() {
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}

Hydroponics *Hydroponics::_activeInstance = nullptr;

Hydroponics::Hydroponics(byte piezoBuzzerPin, byte sdCardCSPin, byte controlInputPin1,
                         byte eepromI2CAddress, byte rtcI2CAddress, byte lcdI2CAddress,
                         TwoWire& i2cWire, uint32_t i2cSpeed, uint32_t spiSpeed)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed), _spiSpeed(spiSpeed),
      _piezoBuzzerPin(piezoBuzzerPin), _sdCardCSPin(sdCardCSPin), _ctrlInputPin1(controlInputPin1), _ctrlInputPinMap{-1}, _pollingFrame(0),
      _eepromI2CAddr(eepromI2CAddress), _rtcI2CAddr(rtcI2CAddress), _lcdI2CAddr(lcdI2CAddress),
      _buzzer(&EasyBuzzer), _eeprom(nullptr), _rtc(nullptr), _sd(nullptr), _eepromBegan(false), _rtcBegan(false), _rtcBattFail(false),
      _systemData(nullptr)
{
    _activeInstance = this;
    if (isValidPin(_piezoBuzzerPin)) {
        _buzzer->setPin(_piezoBuzzerPin);
    }
    if (isValidPin(_ctrlInputPin1)) {
        for (byte pinIndex = 0; pinIndex < HYDRUINO_CTRLINPINMAP_MAXSIZE; ++pinIndex) {
            _ctrlInputPinMap[pinIndex] = _ctrlInputPin1 + pinIndex;
        }
    }
}

Hydroponics::Hydroponics(TwoWire& i2cWire, uint32_t i2cSpeed, uint32_t spiSpeed,
                         byte piezoBuzzerPin, byte sdCardCSPin, byte controlInputPin1,
                         byte eepromI2CAddress, byte rtcI2CAddress, byte lcdI2CAddress)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed), _spiSpeed(spiSpeed),
      _piezoBuzzerPin(piezoBuzzerPin), _sdCardCSPin(sdCardCSPin), _ctrlInputPin1(controlInputPin1), _ctrlInputPinMap{-1}, _pollingFrame(0),
      _eepromI2CAddr(eepromI2CAddress), _rtcI2CAddr(rtcI2CAddress), _lcdI2CAddr(lcdI2CAddress),
      _buzzer(&EasyBuzzer), _eeprom(nullptr), _rtc(nullptr), _sd(nullptr),
      _eepromBegan(false), _rtcBegan(false), _rtcBattFail(false),
      _systemData(nullptr)
{
    _activeInstance = this;
    if (isValidPin(_piezoBuzzerPin)) {
        _buzzer->setPin(_piezoBuzzerPin);
    }
    if (isValidPin(_ctrlInputPin1)) {
        for (byte pinIndex = 0; pinIndex < HYDRUINO_CTRLINPINMAP_MAXSIZE; ++pinIndex) {
            _ctrlInputPinMap[pinIndex] = _ctrlInputPin1 + pinIndex;
        }
    }
}

Hydroponics::~Hydroponics()
{
    if (this == _activeInstance) { _activeInstance = nullptr; }
    _i2cWire = nullptr;
    _buzzer = nullptr;
    deallocateEEPROM();
    deallocateRTC();
    deallocateSD();
    if (_systemData) { delete _systemData; _systemData = nullptr; }
}

void Hydroponics::allocateEEPROM()
{
    if (!_eeprom) {
        _eeprom = new I2C_eeprom(_eepromI2CAddr, HYDRUINO_EEPROM_MEMORYSIZE, _i2cWire);
        _eepromBegan = false;
    }
}

void Hydroponics::deallocateEEPROM()
{
    if (_eeprom) { delete _eeprom; _eeprom = nullptr; }
}

void Hydroponics::allocateRTC()
{
    if (!_rtc) {
        _rtc = new RTC_DS3231();
        HYDRUINO_HARD_ASSERT(_rtcI2CAddr == B000, "RTClib does not support i2c multi-addressing, only i2c address B000 may be used");
        _rtcBegan = false;
    }
}

void Hydroponics::deallocateRTC()
{
    if (_rtc) {
        if (_rtcSyncProvider == _rtc) { setSyncProvider(nullptr); _rtcSyncProvider = nullptr; }
        delete _rtc; _rtc = nullptr;
    }
}

void Hydroponics::allocateSD()
{
    #if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)) || defined(SD)
        _sd = &SD;
    #else
        if (!_sd) {
            _sd = new SDClass();
        }
    #endif
}

void Hydroponics::deallocateSD()
{
    #if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SD)) || defined(SD)
        _sd = nullptr;
    #else
        if (_sd) { delete _sd; _sd = nullptr; }
    #endif
}

void Hydroponics::init(Hydroponics_SystemMode systemMode,
                       Hydroponics_MeasurementMode measureMode,
                       Hydroponics_DisplayOutputMode dispOutMode,
                       Hydroponics_ControlInputMode ctrlInMode)
{
    HYDRUINO_SOFT_ASSERT(!_systemData, "Controller already initialized");

    if (!_systemData) {
        HYDRUINO_SOFT_ASSERT((int)systemMode >= 0 && systemMode < Hydroponics_SystemMode_Count, "Invalid system mode");
        HYDRUINO_SOFT_ASSERT((int)measureMode >= 0 && measureMode < Hydroponics_MeasurementMode_Count, "Invalid measurement mode");
        HYDRUINO_SOFT_ASSERT((int)dispOutMode >= 0 && dispOutMode < Hydroponics_DisplayOutputMode_Count, "Invalid LCD output mode");
        HYDRUINO_SOFT_ASSERT((int)ctrlInMode >= 0 && ctrlInMode < Hydroponics_ControlInputMode_Count, "Invalid control input mode");

        _systemData = new HydroponicsSystemData();
        HYDRUINO_SOFT_ASSERT(_systemData, "Invalid system data store");

        if (_systemData) {
            _systemData->systemMode = systemMode;
            _systemData->measureMode = measureMode;
            _systemData->dispOutMode = dispOutMode;
            _systemData->ctrlInMode = ctrlInMode;

            commonInit();
        }
    }  
}

bool Hydroponics::initFromEEPROM()
{
    HYDRUINO_SOFT_ASSERT(!_systemData, "Controller already initialized");

    if (!_systemData) {
        auto *eeprom = getEEPROM(); // Forces begin, if not already
        if (eeprom) {
            // TODO
            //auto *systemData = HydroponicsSystemData::fromEEPROMStore();
            //if (systemData) { _systemData = systemData; }
        }

        HYDRUINO_SOFT_ASSERT(_systemData, "Invalid system data store");
        if (_systemData) {
            commonInit();
        }
        return _systemData;
    }

    return false;
}

bool Hydroponics::initFromSDCard(String configFile)
{
    HYDRUINO_SOFT_ASSERT(!_systemData, "Controller already initialized");

    if (!_systemData) {
        auto *sd = getSDCard();
        if (sd) {
            auto config = sd->open(configFile);
            if (config && config.size()) {
                // TODO
                //auto *systemData = HydroponicsSystemData::fromJSONElement(configToJSON);
                //if (systemData) { _systemData = systemData; }
                config.close();
            }
            sd->end();
        }

        HYDRUINO_SOFT_ASSERT(_systemData, "Invalid system data store");
        if (_systemData) {
            commonInit();
        }
        return _systemData;
    }

    return false;
}

void Hydroponics::commonInit()
{
    if ((_rtcSyncProvider = getRealTimeClock())) {
        setSyncProvider(rtcNow);
    }

    // TODO: tcMenu setup

    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        Serial.print(F("Hydroponics::commonInit piezoBuzzerPin: "));
        if (isValidPin(_piezoBuzzerPin)) { Serial.print(_piezoBuzzerPin); }
        else { Serial.print(F("Disabled")); }
        Serial.print(F(", sdCardCSPin: "));
        if (isValidPin(_sdCardCSPin)) { Serial.print(_sdCardCSPin); }
        else { Serial.print(F("Disabled")); }
        Serial.print(F(", controlInputPin1: "));
        if (isValidPin(_ctrlInputPin1)) { Serial.print(_ctrlInputPin1); }
        else { Serial.print(F("Disabled")); }
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
        Serial.print(F(", measureMode: "));
        switch (getMeasurementMode()) {
            case Hydroponics_MeasurementMode_Imperial: Serial.print(F("Imperial")); break;
            case Hydroponics_MeasurementMode_Metric: Serial.print(F("Metric")); break;
            case Hydroponics_MeasurementMode_Scientific: Serial.print(F("Scientific")); break;
            case Hydroponics_MeasurementMode_Count:
            case Hydroponics_MeasurementMode_Undefined:
                Serial.print(getMeasurementMode()); break;
        }
        Serial.print(F(", dispOutMode: "));
        switch (getDisplayOutputMode()) {
            case Hydroponics_DisplayOutputMode_Disabled: Serial.print(F("Disabled")); break;
            case Hydroponics_DisplayOutputMode_20x4LCD: Serial.print(F("20x4LCD")); break;
            case Hydroponics_DisplayOutputMode_20x4LCD_Swapped: Serial.print(F("20x4LCD <Swapped>")); break;
            case Hydroponics_DisplayOutputMode_16x2LCD: Serial.print(F("16x2LCD")); break;
            case Hydroponics_DisplayOutputMode_16x2LCD_Swapped: Serial.print(F("16x2LCD <Swapped>")); break;
            case Hydroponics_DisplayOutputMode_Count:
            case Hydroponics_DisplayOutputMode_Undefined:
                Serial.print(getDisplayOutputMode()); break;
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
    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) {
        hydroponics->updateObjects(0);
        hydroponics->updateScheduling();
        hydroponics->updateObjects(1);
    }

    yield();
}

void dataLoop()
{
    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) {
        hydroponics->updateObjects(2);
        hydroponics->updateLogging();
    }

    yield();
}

void guiLoop()
{
    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) {
        hydroponics->updateScreen();
    }

    yield();
}

void miscLoop()
{
    auto hydroponics = getHydroponicsInstance();
    if (hydroponics) {
        hydroponics->updateBuzzer();
    }

    yield();
}

void Hydroponics::launch()
{
    ++_pollingFrame; // Forces all sensors to get a new initial measurement

    // Resolves all unlinked objects
    for (auto pairObj : _objects) {
        auto obj = pairObj.second;
        if (obj) {
            obj->resolveLinks();
        }
    }

    // Create main runloops
    #if defined(HYDRUINO_USE_TASKSCHEDULER)
        Task *controlTask = new Task(1000 * TASK_MILLISECOND, TASK_FOREVER, controlLoop, &_ts, true);
        Task *dataTask = new Task(100 * TASK_MILLISECOND, TASK_FOREVER, dataLoop, &_ts, true);
        Task *guiTask = new Task(20 * TASK_MILLISECOND, TASK_FOREVER, guiLoop, &_ts, true);
        Task *miscTask = new Task(5 * TASK_MILLISECOND, TASK_FOREVER, miscLoop, &_ts, true);
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
    #else
        controlLoop();
        dataLoop();
        guiLoop();
        miscLoop();
    #endif

    taskManager.runLoop(); // tcMenu uses this system to run its UI
}

void Hydroponics::updateObjects(int pass)
{
    switch(pass) {
        case 0: {
            for (auto pairObj : _objects) {
                auto obj = pairObj.second;
                if (obj->isRailType() || obj->isReservoirType()) {
                    obj->update();
                }
            }
        } break;

        case 1: {
            for (auto pairObj : _objects) {
                auto obj = pairObj.second;
                if (obj->isActuatorType()) {
                    obj->update();
                }
            }
        } break;

        case 2: {
            for (auto pairObj : _objects) {
                auto obj = pairObj.second;
                if (obj->isSensorType()) {
                    obj->update();
                }
            }
        } break;
    }
}

void Hydroponics::updateScheduling()
{
    // TODO
}

void Hydroponics::updateLogging()
{
    // TODO
}

void Hydroponics::updateScreen()
{
    // TODO
}

void Hydroponics::updateBuzzer()
{
    auto *buzzer = getPiezoBuzzer();
    if (buzzer) {
        buzzer->update();
    }
}

bool Hydroponics::registerObject(shared_ptr<HydroponicsObject> obj)
{
    HYDRUINO_SOFT_ASSERT(obj->getId().posIndex >= 0 && obj->getId().posIndex < HYDRUINO_ATPOS_MAXSIZE, "Invalid position index");
    return _objects.insert(obj->getKey(), obj).second;
}

bool Hydroponics::unregisterObject(shared_ptr<HydroponicsObject> obj)
{
    auto iter = _objects.find(obj->getKey());
    if (iter != _objects.end()) {
        _objects.erase(iter);
        return true;
    }
    return false;
}

shared_ptr<HydroponicsObject> Hydroponics::objectById(HydroponicsIdentity id) const
{
    if (id.posIndex == HYDRUINO_ATPOS_SEARCH_FROMBEG) {
        while(++id.posIndex < HYDRUINO_ATPOS_MAXSIZE) {
            auto obj = _objects.at(id.regenKey());
            if (obj) { return obj; }
        }
    } else if (id.posIndex == HYDRUINO_ATPOS_SEARCH_FROMEND) {
        while(--id.posIndex >= 0) {
            auto obj = _objects.at(id.regenKey());
            if (obj) { return obj; }
        }
    } else {
        auto obj = _objects.at(id.key);
        if (obj) { return obj; }
    }

    return shared_ptr<HydroponicsObject>(nullptr);
}

Hydroponics_PositionIndex Hydroponics::firstPosition(HydroponicsIdentity id, bool taken)
{
    if (id.posIndex != HYDRUINO_ATPOS_SEARCH_FROMEND) {
        id.posIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG;
        while(++id.posIndex < HYDRUINO_ATPOS_MAXSIZE) {
            auto obj = _objects.at(id.regenKey());
            if (taken == (bool)obj) { return id.posIndex; }
        }
    } else {
        id.posIndex = HYDRUINO_ATPOS_SEARCH_FROMEND;
        while(--id.posIndex >= 0) {
            auto obj = _objects.at(id.regenKey());
            if (taken == (bool)obj) { return id.posIndex; }
        }
    }

    return -1;
}

shared_ptr<HydroponicsRelayActuator> Hydroponics::addGrowLightsRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_GrowLights));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, "Output pin is not digital");

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsRelayActuator> actuator(new HydroponicsRelayActuator(
            Hydroponics_ActuatorType_GrowLights,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return shared_ptr<HydroponicsRelayActuator>(nullptr);
}

shared_ptr<HydroponicsPumpRelayActuator> Hydroponics::addWaterPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_WaterPump));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, "Output pin is not digital");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsPumpRelayActuator> actuator(new HydroponicsPumpRelayActuator(
            Hydroponics_ActuatorType_WaterPump,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return shared_ptr<HydroponicsRelayActuator>(nullptr);
}

shared_ptr<HydroponicsRelayActuator> Hydroponics::addWaterHeaterRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_WaterHeater));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, "Output pin is not digital");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsRelayActuator> actuator(new HydroponicsRelayActuator(
            Hydroponics_ActuatorType_WaterHeater,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return shared_ptr<HydroponicsRelayActuator>(nullptr);
}

shared_ptr<HydroponicsRelayActuator> Hydroponics::addWaterAeratorRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_WaterAerator));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, "Output pin is not digital");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsRelayActuator> actuator(new HydroponicsRelayActuator(
            Hydroponics_ActuatorType_WaterAerator,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return shared_ptr<HydroponicsRelayActuator>(nullptr);
}

shared_ptr<HydroponicsRelayActuator> Hydroponics::addFanExhaustRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_FanExhaust));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, "Output pin is not digital");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsRelayActuator> actuator(new HydroponicsRelayActuator(
            Hydroponics_ActuatorType_FanExhaust,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return shared_ptr<HydroponicsRelayActuator>(nullptr);
}

shared_ptr<HydroponicsPWMActuator> Hydroponics::addFanExhaustPWM(byte outputPin, byte outputBitRes)
{
    bool outputPinIsPWM = checkPinIsPWMOutput(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_FanExhaust));
    HYDRUINO_HARD_ASSERT(outputPinIsPWM, "Output pin does not support PWM");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (outputPinIsPWM && positionIndex != -1) {
        shared_ptr<HydroponicsPWMActuator> actuator(new HydroponicsPWMActuator(
            Hydroponics_ActuatorType_FanExhaust,
            positionIndex,
            outputPin, outputBitRes
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return shared_ptr<HydroponicsPWMActuator>(nullptr);
}

shared_ptr<HydroponicsPumpRelayActuator> Hydroponics::addPeristalticPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ActuatorType_PeristalticPump));
    HYDRUINO_HARD_ASSERT(outputPinIsDigital, "Output pin is not digital");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (outputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsPumpRelayActuator> actuator(new HydroponicsPumpRelayActuator(
            Hydroponics_ActuatorType_PeristalticPump,
            positionIndex,
            outputPin
        ));
        if (registerObject(actuator)) { return actuator; }
        else { actuator = nullptr; }
    }

    return shared_ptr<HydroponicsPumpRelayActuator>(nullptr);
}

shared_ptr<HydroponicsBinarySensor> Hydroponics::addLevelIndicator(byte inputPin)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterLevelIndicator));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, "Input pin is not digital");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsBinarySensor> sensor(new HydroponicsBinarySensor(
            Hydroponics_SensorType_WaterLevelIndicator,
            positionIndex,
            inputPin
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsBinarySensor>(nullptr);
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addAnalogCO2Sensor(byte inputPin, byte inputBitRes = 8)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_AirCarbonDioxide));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, "Input pin is not analog");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_AirCarbonDioxide,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsAnalogSensor>(nullptr);
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addAnalogPhMeter(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_PotentialHydrogen));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, "Input pin is not analog");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_PotentialHydrogen,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsAnalogSensor>(nullptr);
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addAnalogTemperatureSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterTemperature));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, "Input pin is not analog");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_WaterTemperature,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsAnalogSensor>(nullptr);
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addAnalogTDSElectrode(byte inputPin, int ppmScale, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_TotalDissolvedSolids));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, "Input pin is not analog");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_TotalDissolvedSolids,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (ppmScale != 500) {
            auto calibData = sensor->loadInputCalibration(true);
            if (calibData) { calibData->setFromScale(ppmScale / 500.0f);
                             calibData->calibUnits = Hydroponics_UnitsType_Concentration_EC; }
        }
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsAnalogSensor>(nullptr);
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addPWMPumpFlowSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterPumpFlowSensor));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, "Input pin is not analog");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_WaterPumpFlowSensor,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsAnalogSensor>(nullptr);
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addAnalogWaterHeightMeter(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterHeightMeter));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, "Input pin is not analog");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_WaterHeightMeter,
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsAnalogSensor>(nullptr);
}

shared_ptr<HydroponicsAnalogSensor> Hydroponics::addUltrasonicDistanceSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterHeightMeter));
    HYDRUINO_HARD_ASSERT(inputPinIsAnalog, "Input pin is not analog");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsAnalog && positionIndex != -1) {
        shared_ptr<HydroponicsAnalogSensor> sensor(new HydroponicsAnalogSensor(
            Hydroponics_SensorType_WaterHeightMeter,
            positionIndex,
            inputPin, inputBitRes
        ));
        auto calibData = sensor->loadInputCalibration(true);
        if (calibData) { calibData->multiplier = -1.0f; calibData->offset = 1.0f;
                         calibData->calibUnits = defaultSensorMeasurementUnits(sensor->getId().as.sensorType, getMeasurementMode()); }
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsAnalogSensor>(nullptr);
}

shared_ptr<HydroponicsDHTTempHumiditySensor> Hydroponics::addDHTTempHumiditySensor(byte inputPin, uint8_t dhtType)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_AirTempHumidity));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, "Input pin is not digital");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsDHTTempHumiditySensor> sensor(new HydroponicsDHTTempHumiditySensor(
            positionIndex,
            inputPin,
            dhtType
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsDHTTempHumiditySensor>(nullptr);
}

shared_ptr<HydroponicsDSTemperatureSensor> Hydroponics::addDSTemperatureSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_WaterTemperature));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, "Input pin is not digital");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsDSTemperatureSensor> sensor(new HydroponicsDSTemperatureSensor(
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsDSTemperatureSensor>(nullptr);
}

shared_ptr<HydroponicsTMPSoilMoistureSensor> Hydroponics::addTMPSoilMoistureSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_SensorType_SoilMoisture));
    HYDRUINO_HARD_ASSERT(inputPinIsDigital, "Input pin is not digital");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (inputPinIsDigital && positionIndex != -1) {
        shared_ptr<HydroponicsTMPSoilMoistureSensor> sensor(new HydroponicsTMPSoilMoistureSensor(
            positionIndex,
            inputPin, inputBitRes
        ));
        if (registerObject(sensor)) { return sensor; }
        else { sensor = nullptr; }
    }

    return shared_ptr<HydroponicsTMPSoilMoistureSensor>(nullptr);
}

shared_ptr<HydroponicsSimpleCrop> Hydroponics::addCropFromSowDate(Hydroponics_CropType cropType, Hydroponics_SubstrateType substrateType, time_t sowDate)
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(cropType));
    HYDRUINO_SOFT_ASSERT((int)cropType >= 0 && cropType <= Hydroponics_CropType_Count, "Invalid crop type");
    HYDRUINO_SOFT_ASSERT((int)substrateType >= 0 && substrateType <= Hydroponics_SubstrateType_Count, "Invalid substrate type");
    HYDRUINO_SOFT_ASSERT(sowDate > 0, "Invalid sow date");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (cropType < Hydroponics_CropType_Count && sowDate > 0 && positionIndex != -1) {
        shared_ptr<HydroponicsSimpleCrop> crop(new HydroponicsSimpleCrop(
            cropType,
            positionIndex,
            substrateType,
            sowDate
        ));
        if (registerObject(crop)) { return crop; }
        else { crop = nullptr; }
    }

    return shared_ptr<HydroponicsSimpleCrop>(nullptr);
}

shared_ptr<HydroponicsSimpleCrop> Hydroponics::addCropFromLastHarvest(Hydroponics_CropType cropType, Hydroponics_SubstrateType substrateType, time_t lastHarvestDate)
{
    HydroponicsCropsLibData cropData(cropType);
    time_t sowDate = lastHarvestDate - (cropData.growWeeksToHarvest * SECS_PER_WEEK);
    auto crop = addCropFromSowDate(cropType, substrateType, sowDate);
    return crop;
}

shared_ptr<HydroponicsFluidReservoir> Hydroponics::addFluidReservoir(Hydroponics_ReservoirType reservoirType, float maxVolume, Hydroponics_UnitsType maxVolumeUnits)
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(reservoirType));
    HYDRUINO_SOFT_ASSERT((int)reservoirType >= 0 && reservoirType <= Hydroponics_ReservoirType_Count, "Invalid reservoir type");
    HYDRUINO_SOFT_ASSERT(maxVolume > 0.0f, "Invalid max volume");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (reservoirType < Hydroponics_ReservoirType_Count && maxVolume > 0.0f && positionIndex != -1) {
        shared_ptr<HydroponicsFluidReservoir> reservoir(new HydroponicsFluidReservoir(
            reservoirType,
            positionIndex,
            maxVolume, maxVolumeUnits
        ));
        if (registerObject(reservoir)) { return reservoir; }
        else { reservoir = nullptr; }
    }

    return shared_ptr<HydroponicsFluidReservoir>(nullptr);
}

shared_ptr<HydroponicsInfiniteReservoir> Hydroponics::addDrainagePipe()
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ReservoirType_DrainageWater));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (positionIndex != -1) {
        shared_ptr<HydroponicsInfiniteReservoir> reservoir(new HydroponicsInfiniteReservoir(
            Hydroponics_ReservoirType_DrainageWater,
            positionIndex,
            false
        ));
        if (registerObject(reservoir)) { return reservoir; }
        else { reservoir = nullptr; }
    }

    return shared_ptr<HydroponicsInfiniteReservoir>(nullptr);
}

shared_ptr<HydroponicsInfiniteReservoir> Hydroponics::addWaterMainPipe()
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(Hydroponics_ReservoirType_FreshWater));
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (positionIndex != -1) {
        shared_ptr<HydroponicsInfiniteReservoir> reservoir(new HydroponicsInfiniteReservoir(
            Hydroponics_ReservoirType_FreshWater,
            positionIndex,
            true
        ));
        if (registerObject(reservoir)) { return reservoir; }
        else { reservoir = nullptr; }
    }

    return shared_ptr<HydroponicsInfiniteReservoir>(nullptr);
}

shared_ptr<HydroponicsSimpleRail> Hydroponics::addRelayPowerRail(Hydroponics_RailType railType, int maxActiveAtOnce)
{
    Hydroponics_PositionIndex positionIndex = firstPositionOpen(HydroponicsIdentity(railType));
    HYDRUINO_SOFT_ASSERT((int)railType >= 0 && railType <= Hydroponics_RailType_Count, "Invalid rail type");
    HYDRUINO_SOFT_ASSERT(maxActiveAtOnce > 0, "Invalid max active at once");
    HYDRUINO_SOFT_ASSERT(positionIndex != -1, "No more positions available");

    if (railType < Hydroponics_RailType_Count && maxActiveAtOnce > 0) {
        shared_ptr<HydroponicsSimpleRail> rail(new HydroponicsSimpleRail(
            railType,
            positionIndex,
            maxActiveAtOnce
        ));
        if (registerObject(rail)) { return rail; }
        else { rail = nullptr; }
    }

    return shared_ptr<HydroponicsSimpleRail>(nullptr);
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
    return _systemData ? _systemData->measureMode : Hydroponics_MeasurementMode_Undefined;
}

Hydroponics_DisplayOutputMode Hydroponics::getDisplayOutputMode() const
{
    return _systemData ? _systemData->dispOutMode : Hydroponics_DisplayOutputMode_Undefined;
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

    return _eeprom && (!begin || _eepromBegan) ? _eeprom : nullptr;
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

    return _rtc && (!begin || _rtcBegan) ? _rtc : nullptr;
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

        return _sd && sdBegan ? _sd : nullptr;
    }

    return _sd;
}

int Hydroponics::getActuatorCount() const
{
    int retVal = 0;
    for (auto pairObj : _objects) {
        auto obj = pairObj.second;
        if (obj && obj->isActuatorType()) {
            ++retVal;
        }
    }
    return retVal;
}

int Hydroponics::getSensorCount() const
{
    int retVal = 0;
    for (auto pairObj : _objects) {
        auto obj = pairObj.second;
        if (obj && obj->isSensorType()) {
            ++retVal;
        }
    }
    return retVal;
}

int Hydroponics::getCropCount() const
{
    int retVal = 0;
    for (auto pairObj : _objects) {
        auto obj = pairObj.second;
        if (obj && obj->isCropType()) {
            ++retVal;
        }
    }
    return retVal;
}

int Hydroponics::getReservoirCount() const
{
    int retVal = 0;
    for (auto pairObj : _objects) {
        auto obj = pairObj.second;
        if (obj && obj->isReservoirType()) {
            ++retVal;
        }
    }
    return retVal;
}

int Hydroponics::getRailCount() const
{
    int retVal = 0;
    for (auto pairObj : _objects) {
        auto obj = pairObj.second;
        if (obj && obj->isRailType()) {
            ++retVal;
        }
    }
    return retVal;
}

String Hydroponics::getSystemName() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, "System data not yet initialized");
    return _systemData ? String(_systemData->systemName) : String();
}

uint32_t Hydroponics::getPollingIntervalMillis() const
{
    HYDRUINO_SOFT_ASSERT(_systemData, "System data not yet initialized");
    return _systemData ? _systemData->pollingIntMs : 0;
}

uint32_t Hydroponics::getPollingFrameNumber() const
{
    return _pollingFrame;
}

bool Hydroponics::isPollingFrameOld(uint32_t frame) const
{
    return _pollingFrame > frame || (frame == UINT32_MAX && _pollingFrame < UINT32_MAX);
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
    HYDRUINO_SOFT_ASSERT(ctrlInPinCount > 0, "Control input pinmap not used in this mode");
    HYDRUINO_SOFT_ASSERT(ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount, "Ribbon pin index out of range");

    return _ctrlInputPinMap && ctrlInPinCount && ribbonPinIndex >= 0 && ribbonPinIndex < ctrlInPinCount ? _ctrlInputPinMap[ribbonPinIndex] : -1;
}

void Hydroponics::setSystemName(String systemName)
{
    // TODO assert params
    HYDRUINO_SOFT_ASSERT(_systemData, "System data not yet initialized");
    if (_systemData) {
        strncpy(&_systemData->systemName[0], systemName.c_str(), HYDRUINO_NAME_MAXSIZE);
        // TODO lcd update
    }
}

void Hydroponics::setPollingIntervalMillis(uint32_t pollingIntMs)
{
    // TODO assert params
    HYDRUINO_SOFT_ASSERT(_systemData, "System data not yet initialized");
    if (_systemData) {
        _systemData->pollingIntMs = pollingIntMs;
        // TODO adjust per sensor polling
    }
}

void Hydroponics::setControlInputPinMap(byte *pinMap)
{
    HYDRUINO_SOFT_ASSERT(pinMap, "Invalid pinMap");
    const int ctrlInPinCount = getControlInputRibbonPinCount();
    HYDRUINO_SOFT_ASSERT(ctrlInPinCount > 0, "Control input pinmap not used in this mode");

    if (pinMap && ctrlInPinCount) {
        for (int ribbonPinIndex = 0; ribbonPinIndex < ctrlInPinCount; ++ribbonPinIndex) {
            _ctrlInputPinMap[ribbonPinIndex] = pinMap[ribbonPinIndex];
        }
    }
}

#ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT

void Hydroponics::forwardLogMessage(String message, bool flushAfter = false)
{
    // TODO
}

#endif // /ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
