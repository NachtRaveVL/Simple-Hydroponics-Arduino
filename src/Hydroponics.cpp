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


static RTC_DS3231 *_rtcSyncProvider = nullptr;
time_t rtcNow() {
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}


HydroponicsObject::HydroponicsObject(HydroponicsIdentity id)
    : _id(id)
{ ; }

HydroponicsObject::~HydroponicsObject()
{ ; }

void HydroponicsObject::update()
{ ; }

const HydroponicsIdentity HydroponicsObject::getId() const
{
    return _id;
}

const String HydroponicsObject::getKey() const
{
    return String(_id.key);
}


HydroponicsIdentity::HydroponicsIdentity()
    : type(Unknown), as{.actuatorType=(Hydroponics_ActuatorType)-1}, posIndex(-1), key{'\0'}
{ ; }

// Actuator identity
HydroponicsIdentity::HydroponicsIdentity(Hydroponics_ActuatorType actuatorTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Actuator), as{.actuatorType=actuatorTypeIn}, posIndex(positionIndex), key{'\0'}
{
    String keyStr = actuatorTypeToString(actuatorTypeIn, true) + positionIndexToString(positionIndex, true);
    strncpy(key, keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
}

// Sensor identity
HydroponicsIdentity::HydroponicsIdentity(Hydroponics_SensorType sensorTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Sensor), as{.sensorType=sensorTypeIn}, posIndex(positionIndex), key{'\0'}
{
    String keyStr = sensorTypeToString(sensorTypeIn, true) + positionIndexToString(positionIndex, true);
    strncpy(key, keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
}

// Crop identity
HydroponicsIdentity::HydroponicsIdentity(Hydroponics_CropType cropTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Crop), as{.cropType=cropTypeIn}, posIndex(positionIndex), key{'\0'}
{
    String keyStr = cropTypeToString(cropTypeIn, true) + positionIndexToString(positionIndex, true);
    strncpy(key, keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
}

// Reservoir identity
HydroponicsIdentity::HydroponicsIdentity(Hydroponics_ReservoirType reservoirTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Reservoir), as{.reservoirType=reservoirTypeIn}, posIndex(positionIndex), key{'\0'}
{
    String keyStr = reservoirTypeToString(reservoirTypeIn, true) + positionIndexToString(positionIndex, true);
    strncpy(key, keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
}

// Rail identity
HydroponicsIdentity::HydroponicsIdentity(Hydroponics_RailType railTypeIn, Hydroponics_PositionIndex positionIndex)
    : type(Rail), as{.railType=railTypeIn}, posIndex(positionIndex), key{'\0'}
{
    String keyStr = railTypeToString(railTypeIn, true) + positionIndexToString(positionIndex, true);
    strncpy(key, keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
}


Hydroponics *Hydroponics::_activeInstance = nullptr;

Hydroponics::Hydroponics(byte piezoBuzzerPin, byte sdCardCSPin, byte controlInputPin1,
                         byte eepromI2CAddress, byte rtcI2CAddress, byte lcdI2CAddress,
                         TwoWire& i2cWire, uint32_t i2cSpeed, uint32_t spiSpeed)
    : _i2cWire(&i2cWire), _i2cSpeed(i2cSpeed), _spiSpeed(spiSpeed),
      _piezoBuzzerPin(piezoBuzzerPin), _sdCardCSPin(sdCardCSPin), _ctrlInputPin1(controlInputPin1), _ctrlInputPinMap{-1},
      _eepromI2CAddr(eepromI2CAddress), _rtcI2CAddr(rtcI2CAddress), _lcdI2CAddr(lcdI2CAddress),
      _buzzer(&EasyBuzzer), _eeprom(nullptr), _rtc(nullptr), _sd(nullptr), _eepromBegan(false), _rtcBegan(false), _rtcBattFail(false),
      _systemData(nullptr),
      _uDelayMillisFunc(uDelayMillisFuncDef), _uDelayMicrosFunc(uDelayMicrosFuncDef)
{
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
      _piezoBuzzerPin(piezoBuzzerPin), _sdCardCSPin(sdCardCSPin), _ctrlInputPin1(controlInputPin1), _ctrlInputPinMap{-1},
      _eepromI2CAddr(eepromI2CAddress), _rtcI2CAddr(rtcI2CAddress), _lcdI2CAddr(lcdI2CAddress),
      _buzzer(&EasyBuzzer), _eeprom(nullptr), _rtc(nullptr), _sd(nullptr),
      _eepromBegan(false), _rtcBegan(false), _rtcBattFail(false),
      _systemData(nullptr),
      _uDelayMillisFunc(uDelayMillisFuncDef), _uDelayMicrosFunc(uDelayMicrosFuncDef)
{
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
    //assert(!_eeprom && "EEPROM already allocated");
    _eeprom = new I2C_eeprom(_eepromI2CAddr, HYDRUINO_EEPROM_MEMORYSIZE, _i2cWire);
    _eepromBegan = false;
}

void Hydroponics::deallocateEEPROM()
{
    if (_eeprom) { delete _eeprom; _eeprom = nullptr; }
}

void Hydroponics::allocateRTC()
{
    //assert(!_rtc && "RTC already allocated");
    _rtc = new RTC_DS3231();
    //assert(_rtcI2CAddr == B000 && "RTClib does not support i2c multi-addressing, only B000 may be used");
    _rtcBegan = false;
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
        //assert(!_sd && "SD already allocated");
        _sd = new SDClass();
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
    //assert(!_systemData && "Controller already initialized");
    //assert(!_activeInstance && "Controller already active");

    if (!_systemData && !_activeInstance) {
        //assert((int)systemMode >= 0 && systemMode < Hydroponics_SystemMode_Count && "Invalid system mode");
        //assert((int)measureMode >= 0 && measureMode < Hydroponics_MeasurementMode_Count && "Invalid measurement mode");
        //assert((int)dispOutMode >= 0 && dispOutMode < Hydroponics_DisplayOutputMode_Count && "Invalid LCD output mode");
        //assert((int)ctrlInMode >= 0 && ctrlInMode < Hydroponics_ControlInputMode_Count && "Invalid control input mode");

        _systemData = new HydroponicsSystemData();
        //assert(_systemData && "Invalid system data store");

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

bool Hydroponics::initFromSDCard(String configFile)
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
            _systemData->reservoirVolUnits = Hydroponics_UnitsType_LiquidVolume_Gallons; 
            _systemData->pumpFlowRateUnits = Hydroponics_UnitsType_LiquidFlow_GallonsPerMin;
            break;

        case Hydroponics_MeasurementMode_Metric:
        case Hydroponics_MeasurementMode_Scientific:
            _systemData->reservoirVolUnits = Hydroponics_UnitsType_LiquidVolume_Liters; 
            _systemData->pumpFlowRateUnits = Hydroponics_UnitsType_LiquidFlow_LitersPerMin;
            break;
    }

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

bool Hydroponics::registerObject(HydroponicsObject *obj)
{
    // TODO
    return true;
}

bool Hydroponics::unregisterObject(HydroponicsObject *obj)
{
    // TODO
    return true;
}

HydroponicsObject *Hydroponics::findObjectByKey(HydroponicsIdentity identity) const
{
    // TODO
    return nullptr;
}

HydroponicsRelayActuator *Hydroponics::addGrowLightsRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    //assert(outputPinIsDigital && "Output pin is not digital");

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(Hydroponics_ActuatorType_GrowLights,
                                                                          0, // TODO
                                                                          outputPin);
        if (registerObject(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return nullptr;
}

HydroponicsRelayActuator *Hydroponics::addWaterPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    //assert(outputPinIsDigital && "Output pin is not digital");

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(Hydroponics_ActuatorType_WaterPump,
                                                                          0, // TODO
                                                                          outputPin);
        if (registerObject(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return nullptr;
}

HydroponicsRelayActuator *Hydroponics::addWaterHeaterRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    //assert(outputPinIsDigital && "Output pin is not digital");

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(Hydroponics_ActuatorType_WaterHeater,
                                                                          0, // TODO
                                                                          outputPin);
        if (registerObject(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return nullptr;
}

HydroponicsRelayActuator *Hydroponics::addWaterAeratorRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    //assert(outputPinIsDigital && "Output pin is not digital");

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(Hydroponics_ActuatorType_WaterAerator,
                                                                          0, // TODO
                                                                          outputPin);
        if (registerObject(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return nullptr;
}

HydroponicsRelayActuator *Hydroponics::addFanExhaustRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    //assert(outputPinIsDigital && "Output pin is not digital");

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(Hydroponics_ActuatorType_FanExhaust,
                                                                          0, // TODO
                                                                          outputPin);
        if (registerObject(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return nullptr;
}

HydroponicsPWMActuator *Hydroponics::addFanExhaustPWM(byte outputPin, byte outputBitRes)
{
    bool outputPinIsPWM = checkPinCanPWMOutput(outputPin);
    //assert(outputPinIsPWM && "Output pin does not support PWM");

    if (outputPinIsPWM) {
        HydroponicsPWMActuator *actuator = new HydroponicsPWMActuator(Hydroponics_ActuatorType_FanExhaust,
                                                                      0, // TODO
                                                                      outputPin,
                                                                      outputBitRes);
        if (registerObject(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return nullptr;
}

HydroponicsRelayActuator *Hydroponics::addPeristalticPumpRelay(byte outputPin)
{
    bool outputPinIsDigital = checkPinIsDigital(outputPin);
    //assert(outputPinIsDigital && "Output pin is not digital");

    if (outputPinIsDigital) {
        HydroponicsRelayActuator *actuator = new HydroponicsRelayActuator(Hydroponics_ActuatorType_PeristalticPump,
                                                                          0, // TODO
                                                                          outputPin);
        if (registerObject(actuator)) { return actuator; }
        else { delete actuator; }
    }

    return nullptr;
}

HydroponicsBinarySensor *Hydroponics::addLevelIndicator(byte inputPin)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    //assert(inputPinIsDigital && "Input pin is not digital");

    if (inputPinIsDigital) {
        HydroponicsBinarySensor *sensor = new HydroponicsBinarySensor(Hydroponics_SensorType_WaterLevelIndicator,
                                                                      0, // TODO: Find next free index
                                                                      inputPin,
                                                                      true);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsAnalogSensor *Hydroponics::addCO2Sensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(Hydroponics_SensorType_AirCarbonDioxide,
                                                                      0, // TODO: Find next free index
                                                                      inputPin,
                                                                      inputBitRes);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsAnalogSensor *Hydroponics::addPhMeter(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(Hydroponics_SensorType_PotentialHydrogen,
                                                                      0, // TODO: Find next free index
                                                                      inputPin,
                                                                      inputBitRes);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsAnalogSensor *Hydroponics::addTempSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(Hydroponics_SensorType_WaterTemperature,
                                                                      0, // TODO: Find next free index
                                                                      inputPin,
                                                                      inputBitRes);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsAnalogSensor *Hydroponics::addTDSElectrode(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(Hydroponics_SensorType_TotalDissolvedSolids,
                                                                      0, // TODO: Find next free index
                                                                      inputPin,
                                                                      inputBitRes);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsAnalogSensor *Hydroponics::addPumpFlowSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(Hydroponics_SensorType_WaterPumpFlowSensor,
                                                                      0, // TODO: Find next free index
                                                                      inputPin,
                                                                      inputBitRes);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsAnalogSensor *Hydroponics::addWaterHeightMeter(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(Hydroponics_SensorType_WaterHeightMeter,
                                                                      0, // TODO: Find next free index
                                                                      inputPin,
                                                                      inputBitRes);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsAnalogSensor *Hydroponics::addUltrasonicDistanceSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsAnalogSensor *sensor = new HydroponicsAnalogSensor(Hydroponics_SensorType_WaterHeightMeter,
                                                                      0, // TODO: Find next free index
                                                                      inputPin,
                                                                      inputBitRes);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsDHTOneWireSensor *Hydroponics::addDHTTempHumiditySensor(byte inputPin, uint8_t dhtType)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    //assert(inputPinIsDigital && "Input pin is not digital");

    if (inputPinIsDigital) {
        HydroponicsDHTOneWireSensor *sensor = new HydroponicsDHTOneWireSensor(0, // TODO: Find next free index
                                                                              inputPin,
                                                                              dhtType);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsDSOneWireSensor *Hydroponics::addDSTemperatureSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsDigital = checkPinIsDigital(inputPin);
    //assert(inputPinIsDigital && "Input pin is not digital");

    if (inputPinIsDigital) {
        HydroponicsDSOneWireSensor *sensor = new HydroponicsDSOneWireSensor(0, // TODO: Find next free index
                                                                            inputPin,
                                                                            inputBitRes);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsTMPOneWireSensor *Hydroponics::addTMPSoilMoistureSensor(byte inputPin, byte inputBitRes)
{
    bool inputPinIsAnalog = checkPinIsAnalogInput(inputPin);
    //assert(inputPinIsAnalog && "Input pin is not analog");

    if (inputPinIsAnalog) {
        HydroponicsTMPOneWireSensor *sensor = new HydroponicsTMPOneWireSensor(0, // TODO: Find next free index
                                                                              inputPin,
                                                                              inputBitRes);
        if (registerObject(sensor)) { return sensor; }
        else { delete sensor; }
    }

    return nullptr;
}

HydroponicsCrop *Hydroponics::addCropFromSowDate(Hydroponics_CropType cropType, Hydroponics_SubstrateType substrateType, time_t sowDate)
{
    //assert((int)cropType >= 0 && cropType <= Hydroponics_CropType_Count && "Invalid crop type");
    //assert(sowDate > 0 && "Invalid sow date");
    Hydroponics_PositionIndex nextFreeIndex = 0; // TODO

    if (nextFreeIndex >= 0 && nextFreeIndex < HYDRUINO_ATPOS_MAXSIZE) {
        HydroponicsCrop *crop = new HydroponicsCrop(cropType, nextFreeIndex, substrateType, sowDate);
        if (registerObject(crop)) { return crop; }
        else { delete crop; }
    }

    return nullptr;
}

HydroponicsCrop *Hydroponics::addCropFromLastHarvest(Hydroponics_CropType cropType, Hydroponics_SubstrateType substrateType, time_t lastHarvestDate)
{
    HydroponicsCropLibData plantData(cropType);
    time_t sowDate = lastHarvestDate - (plantData.growWeeksToHarvest * SECS_PER_WEEK);
    return addCropFromSowDate(cropType, substrateType, sowDate);
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

int Hydroponics::getRelayCount(Hydroponics_RailType relayRail) const
{
    // TODO
    return 0;
}

int Hydroponics::getActiveRelayCount(Hydroponics_RailType relayRail) const
{
    // TODO
    return 0;
}

byte Hydroponics::getMaxActiveRelayCount(Hydroponics_RailType relayRail) const
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

String Hydroponics::getSystemName() const
{
    //assert(_systemData && "System data not yet initialized");
    return _systemData ? String(_systemData->systemName) : String();
}

uint32_t Hydroponics::getPollingIntervalMillis() const
{
    //assert(_systemData && "System data not yet initialized");
    return _systemData ? _systemData->pollingIntMs : 0;
}

float Hydroponics::getReservoirVolume(Hydroponics_ReservoirType forFluidReservoir) const
{
    //assert(_systemData && "System data not yet initialized");
    return _systemData ? _systemData->reservoirVol[forFluidReservoir] : 0;
}

float Hydroponics::getPumpFlowRate(Hydroponics_ReservoirType forFluidReservoir) const
{
    //assert(_systemData && "System data not yet initialized");
    return _systemData ? _systemData->pumpFlowRate[forFluidReservoir] : 0;
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

void Hydroponics::setMaxActiveRelayCount(byte maxActiveCount, Hydroponics_RailType relayRail)
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
    if (_systemData) {
        _systemData->maxActiveRelayCount[relayRail] = maxActiveCount;
    }
}

void Hydroponics::setSystemName(String systemName)
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
    if (_systemData) {
        strncpy(&_systemData->systemName[0], systemName.c_str(), HYDRUINO_NAME_MAXSIZE);
        // TODO lcd update
    }
}

void Hydroponics::setPollingIntervalMillis(uint32_t pollingIntMs)
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
    if (_systemData) {
        _systemData->pollingIntMs = pollingIntMs;
        // TODO adjust per sensor polling
    }
}

void Hydroponics::setReservoirVolume(float reservoirVol, Hydroponics_ReservoirType forFluidReservoir)
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
    if (_systemData) {
        _systemData->reservoirVol[forFluidReservoir] = reservoirVol;
        // TODO lcd update
    }
}

void Hydroponics::setPumpFlowRate(float pumpFlowRate, Hydroponics_ReservoirType forFluidReservoir)
{
    // TODO assert params
    //assert(_systemData && "System data not yet initialized");
    if (_systemData) {
        _systemData->pumpFlowRate[forFluidReservoir] = pumpFlowRate;
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
