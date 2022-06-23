/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    This permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    Simple-Hydroponics-Arduino - Version 0.1
*/

#ifndef Hydroponics_H
#define Hydroponics_H

// Library Setup

// NOTE: It is recommended to use custom build flags instead of editing this file directly.

// Uncomment or -D this define to completely disable usage of any multitasking commands, such as yield(), as well as libraries.
//#define HYDRUINO_DISABLE_MULTITASKING

// Uncomment or -D this define to disable usage of the TaskScheduler library, which is used by default.
//#define HYDRUINO_DISABLE_TASKSCHEDULER          // https://github.com/arkhipenko/TaskScheduler

// Uncomment or -D this define to enable usage of the Scheduler library, iff TaskScheduler disabled, for SAM/SAMD architectures only.
//#define HYDRUINO_ENABLE_SCHEDULER               // https://github.com/arduino-libraries/Scheduler

// Uncomment or -D this define to enable usage of the CoopTask library, iff both TaskScheduler/Scheduler disabled.
//#define HYDRUINO_ENABLE_COOPTASK                // https://github.com/dok-net/CoopTask

// Uncomment or -D this define to enable debug output.
#define HYDRUINO_ENABLE_DEBUG_OUTPUT


#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#elif defined(__MBED__)
#include <mbed.h>
#else
#include <WProgram.h>
#endif
#include <assert.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <util/atomic.h>

#if defined(NDEBUG) && defined(HYDRUINO_ENABLE_DEBUG_OUTPUT)
#undef HYDRUINO_ENABLE_DEBUG_OUTPUT
#endif
#ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
#define HYDRUINO_SOFT_ASSERT(cond,msg)  softAssert((bool)(cond), String((msg)), __FILE__, __func__, __LINE__)
#define HYDRUINO_HARD_ASSERT(cond,msg)  hardAssert((bool)(cond), String((msg)), __FILE__, __func__, __LINE__)
#else
#define HYDRUINO_SOFT_ASSERT(cond,msg)  ((void)0)
#define HYDRUINO_HARD_ASSERT(cond,msg)  ((void)0)
#endif

#ifndef HYDRUINO_DISABLE_MULTITASKING
#if !defined(HYDRUINO_DISABLE_TASKSCHEDULER)
#include "TaskSchedulerDeclarations.h"  // Including this forces user code to include TaskSheduler.h
#define HYDRUINO_USE_TASKSCHEDULER
#define HYDRUINO_MAINLOOP(scheduler)    (scheduler).execute()
#elif defined(HYDRUINO_ENABLE_SCHEDULER) && (defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD))
#include "Scheduler.h"
#define HYDRUINO_USE_SCHEDULER
#elif defined(HYDRUINO_ENABLE_COOPTASK)
#include "CoopTask.h"
#define HYDRUINO_USE_COOPTASK
#define HYDRUINO_MAINLOOP()             runCoopTasks()
#endif
#endif // /ifndef HYDRUINO_DISABLE_MULTITASKING
#ifndef HYDRUINO_MAINLOOP
#define HYDRUINO_MAINLOOP()             yield()
#endif

#include "ArduinoJson.h"                // JSON library
#include "ArxContainer.h"               // STL-like container library
#include "ArxSmartPtr.h"                // Shared pointer library
#include "Callback.h"                   // Callback library
#include "DallasTemperature.h"          // DS18* submersible water temp probe
#include "DHT.h"                        // DHT* air temp/humidity probe
#include "EasyBuzzer.h"                 // Async piezo buzzer library
#include "I2C_eeprom.h"                 // i2c EEPROM library
#if !defined(__STM32F1__)
#include "OneWire.h"                    // OneWire for DS18* probes
#else
#include <OneWireSTM.h>                 // STM32 version of OneWire
#endif
#include "RTClib.h"                     // i2c RTC library
#include "SimpleCollections.h"          // SimpleCollections library
#include "TimeLib.h"                    // Time library
#include "tcMenu.h"                     // tcMenu library

#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L // Have libstdc++11
using namespace std;
#else
using namespace arx;
#endif
using namespace arx::stdx;

#include "HydroponicsDefines.h"
#include "HydroponicsInlines.hpp"
#include "HydroponicsInterfaces.h"
#include "HydroponicsObject.h"
#include "HydroponicsUtils.hpp"
#include "HydroponicsDatas.h"
#include "HydroponicsCropsLibrary.h"
#include "HydroponicsMeasurements.h"
#include "HydroponicsTriggers.h"
#include "HydroponicsActuators.h"
#include "HydroponicsSensors.h"
#include "HydroponicsCrops.h"
#include "HydroponicsReservoirs.h"
#include "HydroponicsRails.h"

class Hydroponics {
public:
    // Library constructor. Typically called during class instantiation, before setup().
    // TODO
    Hydroponics(byte piezoBuzzerPin = -1,
                byte sdCardCSPin = -1,
                byte controlInputPin1 = -1,                 // First pin of ribbon (can be customized later)
                byte eepromI2CAddress = B000,
                byte rtcI2CAddress = B000,
                byte lcdI2CAddress = B000,
                TwoWire& i2cWire = Wire, uint32_t i2cSpeed = 400000U,
                uint32_t spiSpeed = 4000000U);
    Hydroponics(TwoWire& i2cWire, uint32_t i2cSpeed = 400000U,
                uint32_t spiSpeed = 4000000U,
                byte piezoBuzzerPin = -1,
                byte sdCardCSPin = -1,
                byte controlInputPin1 = -1,
                byte eepromI2CAddress = B000,
                byte rtcI2CAddress = B000,
                byte lcdI2CAddress = B000);
    ~Hydroponics();

    // Initializes module. Typically called in setup().
    // See individual enums for more info.
    void init(Hydroponics_SystemMode systemMode = Hydroponics_SystemMode_Recycling,
              Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Default,
              Hydroponics_DisplayOutputMode dispOutMode = Hydroponics_DisplayOutputMode_Disabled,
              Hydroponics_ControlInputMode ctrlInMode = Hydroponics_ControlInputMode_Disabled);
    // Initializes module from EEPROM save, returning success flag
    bool initFromEEPROM();
    // Initializes module from MicroSD save, returning success flag
    bool initFromSDCard(String configFile = "hydruino.cfg");

    // TODO: maybe like this?
    //bool initFromNetworkURL(wifiClient, serverData, configFile = "hydruino.cfg");
    //void enableSysLoggingToSDCard(sd, logFilePrefix = "logs/sys");
    //void enableSysLoggingToNetworkURL(netClient, urlData, logFilePrefix = "logs/sys");
    //void enableDataPublishingToSDCard(sd, csvFilePrefix = "logs/dat");
    //void enableDataPublishingToNetworkURL(netClient, urlData, csvFilePrefix = "logs/dat");
    //void enableDataPublishingToMQTT(mqttBroker, deviceData);
    //void enableDataPublishingToWebAPI(netClient, urlData, apiInterface);

    // Launches system into operational mode. Typically called last in setup().
    // Once launch is called further system setup may no longer be available due to dependency constraints.
    void launch();

    // Update method. Typically called last in loop().
    // By default this method simply calls into the active scheduler's main loop mechanism, unless multitasking is disabled in which case calls all runloops.
    void update();


    // Object Creation & Management.

    // Adds/removes objects to/from system, returning success
    bool registerObject(shared_ptr<HydroponicsObject> obj);
    bool unregisterObject(shared_ptr<HydroponicsObject> obj);

    // Searches for object by id key (nullptr return = no obj by that id, position index may use HYDRUINO_ATPOS_SEARCH* defines)
    shared_ptr<HydroponicsObject> objectById(HydroponicsIdentity id) const;
    Hydroponics_PositionIndex firstPosition(HydroponicsIdentity id, bool taken = true);
    inline Hydroponics_PositionIndex firstPositionTaken(HydroponicsIdentity id) { firstPosition(id, true); }
    inline Hydroponics_PositionIndex firstPositionOpen(HydroponicsIdentity id) { firstPosition(id, false); }

    inline shared_ptr<HydroponicsActuator> actuatorById(HydroponicsIdentity id) const { return reinterpret_pointer_cast<HydroponicsActuator>(objectById(id)); }
    inline shared_ptr<HydroponicsActuator> actuatorById(Hydroponics_ActuatorType actuatorType, Hydroponics_PositionIndex actuatorIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG) const { return reinterpret_pointer_cast<HydroponicsActuator>(objectById(HydroponicsIdentity(actuatorType, actuatorIndex))); }
    inline shared_ptr<HydroponicsSensor> sensorById(HydroponicsIdentity id) const { return reinterpret_pointer_cast<HydroponicsSensor>(objectById(id)); }
    inline shared_ptr<HydroponicsSensor> sensorById(Hydroponics_SensorType sensorType, Hydroponics_PositionIndex sensorIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG) const { return reinterpret_pointer_cast<HydroponicsSensor>(objectById(HydroponicsIdentity(sensorType, sensorIndex))); }
    inline shared_ptr<HydroponicsCrop> cropById(HydroponicsIdentity id) const { return reinterpret_pointer_cast<HydroponicsCrop>(objectById(id)); }
    inline shared_ptr<HydroponicsCrop> cropById(Hydroponics_CropType cropType, Hydroponics_PositionIndex cropIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG) const { return reinterpret_pointer_cast<HydroponicsCrop>(objectById(HydroponicsIdentity(cropType, cropIndex))); }
    inline shared_ptr<HydroponicsReservoir> reservoirById(HydroponicsIdentity id) const { return reinterpret_pointer_cast<HydroponicsReservoir>(objectById(id)); }
    inline shared_ptr<HydroponicsReservoir> reservoirById(Hydroponics_ReservoirType reservoirType, Hydroponics_PositionIndex reservoirIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG) const { return reinterpret_pointer_cast<HydroponicsReservoir>(objectById(HydroponicsIdentity(reservoirType, reservoirIndex))); }
    inline shared_ptr<HydroponicsRail> railById(HydroponicsIdentity id) const { return reinterpret_pointer_cast<HydroponicsRail>(objectById(id)); }
    inline shared_ptr<HydroponicsRail> railById(Hydroponics_RailType railType, Hydroponics_PositionIndex railIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG) const { return reinterpret_pointer_cast<HydroponicsRail>(objectById(HydroponicsIdentity(railType, railIndex))); }

    // Convenience builders for common actuators (shared, nullptr return = failure).

    // Adds a new grow light relay to the system using the given parameters.
    shared_ptr<HydroponicsRelayActuator> addGrowLightsRelay(byte outputPin);                        // Digital output pin this actuator sits on
    // Adds a new water pump relay to the system using the given parameters.
    shared_ptr<HydroponicsPumpRelayActuator> addWaterPumpRelay(byte outputPin);                     // Digital output pin this actuator sits on
    // Adds a new water heater relay to the system using the given parameters.
    shared_ptr<HydroponicsRelayActuator> addWaterHeaterRelay(byte outputPin);                       // Digital output pin this actuator sits on
    // Adds a new water aerator relay to the system using the given parameters.
    shared_ptr<HydroponicsRelayActuator> addWaterAeratorRelay(byte outputPin);                      // Digital output pin this actuator sits on
    // Adds a new fan exhaust relay to the system using the given parameters.
    shared_ptr<HydroponicsRelayActuator> addFanExhaustRelay(byte outputPin);                        // Digital output pin this actuator sits on
    // Adds a new PWM-based fan exhaust to the system using the given parameters.
    shared_ptr<HydroponicsPWMActuator> addFanExhaustPWM(byte outputPin,                             // PWM output pin this actuator sits on
                                                        byte outputBitRes = 8);                     // PWM bit resolution to use (see bitRes notice)
    // Adds a new peristaltic dosing pump relay to the system using the given parameters.
    shared_ptr<HydroponicsPumpRelayActuator> addPeristalticPumpRelay(byte outputPin);               // Digital output pin this actuator sits on

    // Convenience builders for common sensors (shared, nullptr return = failure).

    // Adds a new binary level indicator to the system using the given parameters.
    shared_ptr<HydroponicsBinarySensor> addLevelIndicator(byte inputPin);                           // Digital input pin this sensor sits on (can make interruptable)

    // Adds a new analog CO2 sensor to the system using the given parameters.
    shared_ptr<HydroponicsAnalogSensor> addAnalogCO2Sensor(byte inputPin,                           // Analog input pin this sensor sits on
                                                           byte inputBitRes = 8);                   // ADC bit resolution to use
    // Adds a new analog PH meter to the system using the given parameters.
    shared_ptr<HydroponicsAnalogSensor> addAnalogPhMeter(byte inputPin,                             // Analog input pin this sensor sits on
                                                         byte inputBitRes = 8);                     // ADC bit resolution to use
    // Adds a new analog temperature sensor to the system using the given parameters.
    shared_ptr<HydroponicsAnalogSensor> addAnalogTemperatureSensor(byte inputPin,                   // Analog input pin this sensor sits on
                                                                   byte inputBitRes = 8);           // ADC bit resolution to use
    // Adds a new analog TDS electrode to the system using the given parameters.
    shared_ptr<HydroponicsAnalogSensor> addAnalogTDSElectrode(byte inputPin,                        // Analog input pin this sensor sits on
                                                              int ppmScale = 500,                   // PPM measurement scaling (EC/TDS = 500)
                                                              byte inputBitRes = 8);                // ADC bit resolution to use
    // Adds a new analog pump flow sensor to the system using the given parameters.
    shared_ptr<HydroponicsAnalogSensor> addPWMPumpFlowSensor(byte inputPin,                         // Analog input pin this sensor sits on
                                                             byte inputBitRes = 8);                 // ADC bit resolution to use
    // Adds a new analog water height meter to the system using the given parameters.
    shared_ptr<HydroponicsAnalogSensor> addAnalogWaterHeightMeter(byte inputPin,                    // Analog input pin this sensor sits on
                                                                  byte inputBitRes = 8);            // ADC bit resolution to use
    // Adds a new analog ultrasonic distance sensor to the system using the given parameters.
    shared_ptr<HydroponicsAnalogSensor> addUltrasonicDistanceSensor(byte inputPin,                  // Analog input pin this sensor sits on
                                                                    byte inputBitRes = 8);          // ADC bit resolution to use

    // TODO:
    // addDigitalPHMeter
    // addDigitalECMeter

    // Adds a new digital DHT* OneWire temperature & humidity sensor to the system using the given parameters.
    shared_ptr<HydroponicsDHTTempHumiditySensor> addDHTTempHumiditySensor(byte inputPin,            // OneWire digital input pin this sensor sits on
                                                                          uint8_t dhtType = DHT12); // Kind of DHT sensor (see DHT* defines)
    // Adds a new digital DS18* OneWire submersible temperature sensor to the system using the given parameters.
    shared_ptr<HydroponicsDSTemperatureSensor> addDSTemperatureSensor(byte inputPin,                // OneWire digital input pin this sensor sits on
                                                                      byte inputBitRes = 9);        // Sensor ADC bit resolution to use
    // Adds a new digital TMP* OneWire soil moisture sensor to the system using the given parameters.
    shared_ptr<HydroponicsTMPSoilMoistureSensor> addTMPSoilMoistureSensor(byte inputPin,            // OneWire digital input pin this sensor sits on
                                                                          byte inputBitRes = 9);    // Sensor ADC bit resolution to use

    // Convenience builders for common crops (shared, nullptr return = failure).

    // Adds a new simple crop to the system using the given parameters.
    shared_ptr<HydroponicsSimpleCrop> addCropFromSowDate(Hydroponics_CropType cropType,             // Crop type
                                                         Hydroponics_SubstrateType substrateType,   // Substrate type
                                                         time_t sowDate);                           // Sow date (UTC unix time)
    // Adds a new simple crop to the system using the given parameters.
    shared_ptr<HydroponicsSimpleCrop> addCropFromLastHarvest(Hydroponics_CropType cropType,         // Crop type
                                                             Hydroponics_SubstrateType substrateType, // Substrate type
                                                             time_t lastHarvestDate);               // Last harvest date (UTC unix time)

    // Convenience builders for common reservoirs (shared, nullptr return = failure).

    // Adds a new simple fluid reservoir to the system using the given parameters.
    shared_ptr<HydroponicsFluidReservoir> addFluidReservoir(Hydroponics_ReservoirType reservoirType,// Reservoir type
                                                            float maxVolume = 5.0f,                 // Maximum volume
                                                            Hydroponics_UnitsType maxVolumeUnits = Hydroponics_UnitsType_LiquidVolume_Gallons);// Maximum volume units

    // Adds a drainage pipe to the system, which acts as an infinite reservoir.
    shared_ptr<HydroponicsInfiniteReservoir> addDrainagePipe();

    // Adds a fresh water main to the system, which acts as an infinite reservoir.
    shared_ptr<HydroponicsInfiniteReservoir> addWaterMainPipe();

    // Convenience builders for common power rails (shared, nullptr return = failure).

    // Adds a new relay power rail to the system using the given parameters.
    shared_ptr<HydroponicsSimpleRail> addRelayPowerRail(Hydroponics_RailType railType,               // Rail type
                                                        int maxActiveAtOnce = 2);                    // Maximum active devices

    // Accessors.

    static Hydroponics *getActiveInstance();                        // Currently active Hydroponics instance (unowned)
    uint32_t getI2CSpeed() const;                                   // i2c clock speed (Hz, default: 400kHz)
    uint32_t getSPISpeed() const;                                   // SPI clock speed (Hz, default: 4MHz)
    Hydroponics_SystemMode getSystemMode() const;                   // System type mode (default: Recycling)
    Hydroponics_MeasurementMode getMeasurementMode() const;         // System measurement mode (default: Imperial)
    Hydroponics_DisplayOutputMode getDisplayOutputMode() const;     // System LCD output mode (default: disabled)
    Hydroponics_ControlInputMode getControlInputMode() const;       // System control input mode (default: disabled)

    EasyBuzzerClass *getPiezoBuzzer() const;                        // Piezo buzzer instance
    I2C_eeprom *getEEPROM(bool begin = true);                       // EEPROM instance (lazily instantiated, nullptr return = failure/no device)
    RTC_DS3231 *getRealTimeClock(bool begin = true);                // Real time clock instance (lazily instantiated, nullptr return = failure/no device)
    SDClass *getSDCard(bool begin = true);                          // SD card instance (if began user code *must* call end() to free SPI interface, lazily instantiated, nullptr return = failure/no device)

    int getActuatorCount() const;                                   // Current number of total actuators registered with system
    int getSensorCount() const;                                     // Current number of total sensors registered with system
    int getCropCount() const;                                       // Current number of total crops registered with system
    int getReservoirCount() const;                                  // Current number of total reservoirs registered with system
    int getRailCount() const;                                       // Current number of total rails registered with system

    String getSystemName() const;                                   // System display name (default: "Hydruino")
    uint32_t getPollingIntervalMillis() const;                      // System sensor polling interval (time between sensor reads), in milliseconds
    uint32_t getPollingFrameNumber() const;                         // System polling frame number for sensor frame tracking
    bool isPollingFrameOld(uint32_t frame) const;             // Determines if a given frame # if out of date (true) or current (false)

    int getControlInputRibbonPinCount();                            // Total number of pins being used for the current control input ribbon mode
    byte getControlInputPin(int ribbonPinIndex);                    // Control input pin mapped to ribbon pin index, or -1 (255) if not used

    // Mutators.

    void setSystemName(String systemName);                          // Sets display name of system (HYDRUINO_NAME_MAXSIZE char limit)
    void setPollingIntervalMillis(uint32_t pollingIntMs);           // Sets system polling interval in milliseconds (does not enable polling, see enablePublishingTo* methods)

    void setControlInputPinMap(byte *pinMap);                       // Sets custom pin mapping for control input, overriding consecutive ribbon pin numbers as default

protected:
    static Hydroponics *_activeInstance;                            // Current active instance (set after init)

    const byte _piezoBuzzerPin;                                     // Piezo buzzer pin (default: disabled)
    const byte _sdCardCSPin;                                        // SD card cable select (CS) pin (default: disabled)
    const byte _ctrlInputPin1;                                      // Control input pin 1 (default: disabled)
    const byte _eepromI2CAddr;                                      // EEPROM i2c address, format: {A2,A1,A0} (default: B000)
    const byte _rtcI2CAddr;                                         // RTC i2c address, format: {A2,A1,A0} (default: B000)
    const byte _lcdI2CAddr;                                         // LCD i2c address, format: {A2,A1,A0} (default: B000)
    TwoWire* _i2cWire;                                              // Wire class instance (unowned) (default: Wire)
    uint32_t _i2cSpeed;                                             // Controller's i2c clock speed (default: 400kHz)
    uint32_t _spiSpeed;                                             // Controller's SPI clock speed (default: 4MHz)

#ifdef HYDRUINO_USE_TASKSCHEDULER
    Scheduler _ts;                                                  // Task scheduler
#endif
    EasyBuzzerClass *_buzzer;                                       // Piezo buzzer instance (unowned)
    I2C_eeprom *_eeprom;                                            // EEPROM instance (owned, lazy)
    RTC_DS3231 *_rtc;                                               // Real time clock instance (owned, lazy)
    SDClass *_sd;                                                   // SD card instance (owned/unowned, lazy)
    bool _eepromBegan;                                              // Status of EEPROM begin()
    bool _rtcBegan;                                                 // Status of RTC begin() call
    bool _rtcBattFail;                                              // Status of RTC battery failure flag
    byte _ctrlInputPinMap[HYDRUINO_CTRLINPINMAP_MAXSIZE];           // Control input pin map
    uint32_t _pollingFrame;                                         // Polling frame #

    HydroponicsSystemData *_systemData;                             // System data (owned, saved to storage)

    arx::map<Hydroponics_KeyType, shared_ptr<HydroponicsObject> > _objects; // Shared object collection, key'ed by HydroponicsIdentity.

    void allocateEEPROM();
    void deallocateEEPROM();
    void allocateRTC();
    void deallocateRTC();
    void allocateSD();
    void deallocateSD();
    void commonInit();

    friend void ::controlLoop();
    friend void ::dataLoop();
    friend void ::guiLoop();
    friend void ::miscLoop();
    void updateObjects(int pass);
    void updateScheduling();
    void updateLogging();
    void updateScreen();
    void updateBuzzer();

    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
    void forwardLogMessage(String message, bool flushAfter = false);
    //void forwardPublishData(paramsTODO);
    friend void ::logMessage(String,bool);
    #endif // /ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
};

#endif // /ifndef Hydroponics_H
