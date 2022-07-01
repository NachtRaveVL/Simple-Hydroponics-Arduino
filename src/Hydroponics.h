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
#if defined(ESP32) || defined(ESP8266)
typedef SDFileSystemClass SDClass;
#else
#include <util/atomic.h>
#endif

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
#include "HydroponicsDatas.h"
#include "HydroponicsObject.h"
#include "HydroponicsUtils.hpp"
#include "HydroponicsCropsLibrary.h"
#include "HydroponicsCalibrationsStore.h"
#include "HydroponicsMeasurements.h"
#include "HydroponicsStreams.h"
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
                uint32_t eepromDeviceSize = 0,              // use I2C_DEVICESIZE_* defines
                byte sdCardCSPin = -1,
                byte controlInputPin1 = -1,                 // first pin of ribbon (can be customized later)
                byte eepromI2CAddress = B000,
                byte rtcI2CAddress = B000,                  // only B000 can be used atm
                byte lcdI2CAddress = B000,
                TwoWire& i2cWire = Wire, uint32_t i2cSpeed = 400000U,
                uint32_t spiSpeed = 4000000U);
    ~Hydroponics();

    // Initializes default empty system. Typically called in setup().
    // See individual enums for more info.
    void init(Hydroponics_SystemMode systemMode = Hydroponics_SystemMode_Recycling,                 // How feed reservoirs are treated
              Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Default,
              Hydroponics_DisplayOutputMode dispOutMode = Hydroponics_DisplayOutputMode_Disabled,
              Hydroponics_ControlInputMode ctrlInMode = Hydroponics_ControlInputMode_Disabled);
    // Initializes system from EEPROM save, returning success flag
    bool initFromEEPROM(bool jsonFormat = false);
    // Initializes system from SD card file save, returning success flag
    bool initFromSDCard(String configFile = "hydruino.cfg", bool jsonFormat = true);
    // Initializes system from custom JSON-based stream, returning success flag
    bool initFromJSONStream(Stream *streamIn);
    // Initializes system from custom binary stream, returning success flag
    bool initFromBinaryStream(Stream *streamIn);

    // Saves current system setup to EEPROM save, returning success flag
    bool saveToEEPROM(bool jsonFormat = false);
    // Saves current system setup to SD card file save, returning success flag
    bool saveToSDCard(String configFile = "hydruino.cfg", bool jsonFormat = true);
    // Saves current system setup to custom JSON-based stream, returning success flag
    bool saveToJSONStream(Stream *streamOut, bool compact = true);
    // Saves current system setup 
    bool saveToBinaryStream(Stream *streamOut);

    // TODO: maybe like this?
    //bool initFromNetworkURL(urlData, configFile = "hydruino.cfg");
    //bool saveToNetworkURL(urlData, configFile = "hydruino.cfg");

    //bool enableSysLoggingToSDCard(String logFilePrefix = "logs/sys");
    //bool enableSysLoggingToNetworkURL(urlData, String logFilePrefix = "logs/sys");

    //bool enableDataPublishingToSDCard(String csvFilePrefix = "logs/dat");
    //bool enableDataPublishingToNetworkURL(urlData, String csvFilePrefix = "logs/dat");
    //bool enableDataPublishingToMQTT(mqttBroker, deviceData);
    //bool enableDataPublishingToWebAPI(urlData, apiInterface);

    // Launches system into operational mode. Typically called last in setup().
    void launch();

    // Suspends the system from operational mode (disables all runloops). Resume operation by a call to launch().
    void suspend();

    // Update method. Typically called last in loop().
    // By default this method simply calls into the active scheduler's main loop mechanism, unless multitasking is disabled in which case calls all runloops.
    void update();


    // Object Registration.

    // Adds/removes objects to/from system, returning success
    bool registerObject(shared_ptr<HydroponicsObject> obj);
    bool unregisterObject(shared_ptr<HydroponicsObject> obj);

    // Searches for object by id key (nullptr return = no obj by that id, position index may use HYDRUINO_POS_SEARCH* defines)
    shared_ptr<HydroponicsObject> objectById(HydroponicsIdentity id) const;
    Hydroponics_PositionIndex firstPosition(HydroponicsIdentity id, bool taken);
    inline Hydroponics_PositionIndex firstPositionTaken(HydroponicsIdentity id) { return firstPosition(id, true); }
    inline Hydroponics_PositionIndex firstPositionOpen(HydroponicsIdentity id) { return firstPosition(id, false); }
 
    inline shared_ptr<HydroponicsActuator> actuatorById(HydroponicsIdentity id) const { return reinterpret_pointer_cast<HydroponicsActuator>(objectById(id)); }
    inline shared_ptr<HydroponicsActuator> actuatorById(Hydroponics_ActuatorType actuatorType, Hydroponics_PositionIndex actuatorIndex = HYDRUINO_POS_SEARCH_FROMBEG) const { return reinterpret_pointer_cast<HydroponicsActuator>(objectById(HydroponicsIdentity(actuatorType, actuatorIndex))); }
    inline shared_ptr<HydroponicsSensor> sensorById(HydroponicsIdentity id) const { return reinterpret_pointer_cast<HydroponicsSensor>(objectById(id)); }
    inline shared_ptr<HydroponicsSensor> sensorById(Hydroponics_SensorType sensorType, Hydroponics_PositionIndex sensorIndex = HYDRUINO_POS_SEARCH_FROMBEG) const { return reinterpret_pointer_cast<HydroponicsSensor>(objectById(HydroponicsIdentity(sensorType, sensorIndex))); }
    inline shared_ptr<HydroponicsCrop> cropById(HydroponicsIdentity id) const { return reinterpret_pointer_cast<HydroponicsCrop>(objectById(id)); }
    inline shared_ptr<HydroponicsCrop> cropById(Hydroponics_CropType cropType, Hydroponics_PositionIndex cropIndex = HYDRUINO_POS_SEARCH_FROMBEG) const { return reinterpret_pointer_cast<HydroponicsCrop>(objectById(HydroponicsIdentity(cropType, cropIndex))); }
    inline shared_ptr<HydroponicsReservoir> reservoirById(HydroponicsIdentity id) const { return reinterpret_pointer_cast<HydroponicsReservoir>(objectById(id)); }
    inline shared_ptr<HydroponicsReservoir> reservoirById(Hydroponics_ReservoirType reservoirType, Hydroponics_PositionIndex reservoirIndex = HYDRUINO_POS_SEARCH_FROMBEG) const { return reinterpret_pointer_cast<HydroponicsReservoir>(objectById(HydroponicsIdentity(reservoirType, reservoirIndex))); }
    inline shared_ptr<HydroponicsRail> railById(HydroponicsIdentity id) const { return reinterpret_pointer_cast<HydroponicsRail>(objectById(id)); }
    inline shared_ptr<HydroponicsRail> railById(Hydroponics_RailType railType, Hydroponics_PositionIndex railIndex = HYDRUINO_POS_SEARCH_FROMBEG) const { return reinterpret_pointer_cast<HydroponicsRail>(objectById(HydroponicsIdentity(railType, railIndex))); }


    // Object Factory.

    // Convenience builders for common actuators (shared, nullptr return = failure).

    // Adds a new grow light relay to the system using the given parameters.
    // Grow lights are essential to almost all plants and are used to mimic natural sun rhythms.
    shared_ptr<HydroponicsRelayActuator> addGrowLightsRelay(byte outputPin);                        // Digital output pin this actuator sits on
    // Adds a new water pump relay to the system using the given parameters.
    // Water pumps are used to feed crops and move liquids around from one reservoir to another.
    shared_ptr<HydroponicsPumpRelayActuator> addWaterPumpRelay(byte outputPin);                     // Digital output pin this actuator sits on
    // Adds a new water heater relay to the system using the given parameters.
    // Water heaters can keep feed water heated during colder months and save off root damage.
    shared_ptr<HydroponicsRelayActuator> addWaterHeaterRelay(byte outputPin);                       // Digital output pin this actuator sits on
    // Adds a new water aerator relay to the system using the given parameters.
    // Water aerators can help plants grow while also discouraging pathogens from taking root.
    shared_ptr<HydroponicsRelayActuator> addWaterAeratorRelay(byte outputPin);                      // Digital output pin this actuator sits on
    // Adds a new fan exhaust relay to the system using the given parameters.
    // Fan exhausts can move air around to modify nearby CO2 levels that plants use to breathe.
    shared_ptr<HydroponicsRelayActuator> addFanExhaustRelay(byte outputPin);                        // Digital output pin this actuator sits on
    // Adds a new PWM-based fan exhaust to the system using the given parameters.
    // PWM fan exhausts allow a graduated adaptive speed control to manage CO2 levels.
    shared_ptr<HydroponicsPWMActuator> addFanExhaustPWM(byte outputPin,                             // PWM output pin this actuator sits on
                                                        byte outputBitRes = 8);                     // PWM bit resolution to use
    // Adds a new peristaltic dosing pump relay to the system using the given parameters.
    // Peristaltic pumps allow proper dosing of nutrients and other additives.
    shared_ptr<HydroponicsPumpRelayActuator> addPeristalticPumpRelay(byte outputPin);               // Digital output pin this actuator sits on

    // Convenience builders for common sensors (shared, nullptr return = failure).

    // Adds a new binary level indicator to the system using the given parameters.
    // Level indicators can be used to control filled/empty status of a liquid reservoir.
    shared_ptr<HydroponicsBinarySensor> addLevelIndicator(byte inputPin);                           // Digital input pin this sensor sits on (can make interruptable)

    // Adds a new analog CO2 sensor to the system using the given parameters.
    // CO2 sensors are used to control and measure the amount of CO2 plants have available.
    shared_ptr<HydroponicsAnalogSensor> addAnalogCO2Sensor(byte inputPin,                           // Analog input pin this sensor sits on
                                                           byte inputBitRes = 8);                   // ADC bit resolution to use
    // Adds a new analog PH meter to the system using the given parameters.
    // pH meters are vital in ensuring the proper alkalinity of the feed water.
    shared_ptr<HydroponicsAnalogSensor> addAnalogPhMeter(byte inputPin,                             // Analog input pin this sensor sits on
                                                         byte inputBitRes = 8);                     // ADC bit resolution to use
    // Adds a new analog temperature sensor to the system using the given parameters.
    // Temperature sensors improve measurement precision as well as ensure proper water conditions.
    shared_ptr<HydroponicsAnalogSensor> addAnalogTemperatureSensor(byte inputPin,                   // Analog input pin this sensor sits on
                                                                   byte inputBitRes = 8);           // ADC bit resolution to use
    // Adds a new analog TDS electrode to the system using the given parameters.
    // TDS electrodes are vital in ensuring the proper nutrition levels of the feed water.
    shared_ptr<HydroponicsAnalogSensor> addAnalogTDSElectrode(byte inputPin,                        // Analog input pin this sensor sits on
                                                              int ppmScale = 500,                   // PPM measurement scaling (default: 500, aka TDS/PPM500)
                                                              byte inputBitRes = 8);                // ADC bit resolution to use
    // Adds a new analog pump flow sensor to the system using the given parameters.
    // Flow sensors allow for more precise liquid volume pumping calculations to be performed.
    shared_ptr<HydroponicsAnalogSensor> addPWMPumpFlowSensor(byte inputPin,                         // Analog input pin this sensor sits on
                                                             byte inputBitRes = 8);                 // ADC bit resolution to use
    // Adds a new analog water height meter to the system using the given parameters.
    // Water height meters can determine the filled/empty status of a liquid reservoir.
    shared_ptr<HydroponicsAnalogSensor> addAnalogWaterHeightMeter(byte inputPin,                    // Analog input pin this sensor sits on
                                                                  byte inputBitRes = 8);            // ADC bit resolution to use
    // Adds a new analog ultrasonic distance sensor to the system using the given parameters.
    // Downward-facing ultrasonic distance sensors call also determine filled/empty status.
    shared_ptr<HydroponicsAnalogSensor> addUltrasonicDistanceSensor(byte inputPin,                  // Analog input pin this sensor sits on
                                                                    byte inputBitRes = 8);          // ADC bit resolution to use

    // TODO: addPowerSensor

    // Adds a new digital DHT* OneWire temperature & humidity sensor to the system using the given parameters.
    // Uses the DHT library. A very common digital sensor, included in most Arduino starter kits.
    shared_ptr<HydroponicsDHTTempHumiditySensor> addDHTTempHumiditySensor(byte inputPin,            // OneWire digital input pin this sensor sits on
                                                                          byte dhtType = DHT12); // Kind of DHT sensor (see DHT* defines)
    // Adds a new digital DS18* OneWire submersible temperature sensor to the system using the given parameters.
    // Uses the DallasTemperature library. A specialized submersible sensor meant for long-term usage.
    shared_ptr<HydroponicsDSTemperatureSensor> addDSTemperatureSensor(byte inputPin,                // OneWire digital input pin this sensor sits on
                                                                      byte inputBitRes = 9);        // Sensor ADC bit resolution to use
    // Adds a new digital TMP* OneWire soil moisture sensor to the system using the given parameters.
    // Uses the XXXTODO library. A blah blah blah blah todo.
    shared_ptr<HydroponicsTMPSoilMoistureSensor> addTMPSoilMoistureSensor(byte inputPin,            // OneWire digital input pin this sensor sits on
                                                                          byte inputBitRes = 9);    // Sensor ADC bit resolution to use

    // TODO: addDigitalPHMeter, addDigitalECMeter, addDigitalCO2Sensor

    // Convenience builders for common crops (shared, nullptr return = failure).

    // Adds a new simple timer-fed crop to the system using the given parameters.
    // Timer fed crops use a simple on/off timer for driving their feeding signal.
    shared_ptr<HydroponicsTimedCrop> addTimerFedCrop(Hydroponics_CropType cropType,                 // Crop type
                                                     Hydroponics_SubstrateType substrateType,       // Substrate type (use Undefined enum to not have feeding times slightly altered due to substrate type)
                                                     time_t sowDate,                                // Sow date
                                                     byte minsOn = 15,                              // Feeding signal on-time interval, in minutes
                                                     byte minsOff = 45);                            // Feeding signal off-time interval, in minutes
    // Adds a new simple timer-fed crop to the system using the given parameters (perennials only).
    // Perennials that grow back are easier to define from their last end-of-harvest date instead of when they were planted.
    shared_ptr<HydroponicsTimedCrop> addTimerFedPerennialCrop(Hydroponics_CropType cropType,        // Crop type
                                                              Hydroponics_SubstrateType substrateType, // Substrate type
                                                              time_t lastHarvestDate,               // Last harvest date
                                                              byte minsOn = 15,                     // Feeding signal on-time interval, in minutes
                                                              byte minsOff = 45);                   // Feeding signal off-time interval, in minutes

    // Adds a new adaptive trigger-fed crop to the system using the given parameters.
    // Adaptive crops use soil based sensors, such as moisture sensors, to drive their feeding signal.
    shared_ptr<HydroponicsAdaptiveCrop> addAdaptiveFedCrop(Hydroponics_CropType cropType,           // Crop type
                                                           Hydroponics_SubstrateType substrateType, // Substrate type (use Undefined enum to not have feeding times slightly altered due to substrate type)
                                                           time_t sowDate);                         // Sow date
    // Adds a new adaptive trigger-fed crop to the system using the given parameters (perennials only).
    // Perennials that grow back are easier to define from their last end-of-harvest date instead of when they were planted.
    shared_ptr<HydroponicsAdaptiveCrop> addAdaptiveFedPerennialCrop(Hydroponics_CropType cropType,  // Crop type
                                                                    Hydroponics_SubstrateType substrateType, // Substrate type (use Undefined enum to not have feeding times slightly altered due to substrate type)
                                                                    time_t lastHarvestDate);        // Last harvest date

    // Convenience builders for common reservoirs (shared, nullptr return = failure).

    // Adds a new simple fluid reservoir to the system using the given parameters.
    // Fluid reservoirs are basically just buckets of some liquid solution. Nothing too fancy.
    shared_ptr<HydroponicsFluidReservoir> addFluidReservoir(Hydroponics_ReservoirType reservoirType, // Reservoir type
                                                            float maxVolume);                        // Maximum volume

    // Adds a drainage pipe to the system using the given parameters.
    // Drainage pipes are never-filled infinite reservoirs that can always be pumped into.
    shared_ptr<HydroponicsInfiniteReservoir> addDrainagePipe();
    // Adds a fresh water main to the system using the given parameters.
    // Fresh water mains are always-filled infinite reservoirs that can always be pumped from.
    shared_ptr<HydroponicsInfiniteReservoir> addFreshWaterMain();

    // Convenience builders for common power rails (shared, nullptr return = failure).

    // Adds a new simple power rail to the system using the given parameters.
    // Simple power rail uses a max active at once counting strategy to manage energy consumption.
    shared_ptr<HydroponicsSimpleRail> addSimplePowerRail(Hydroponics_RailType railType,             // Rail type
                                                         int maxActiveAtOnce = 2);                  // Maximum active devices

    // Adds a new regulated power rail to the system using the given parameters.
    // Regulated power rails can use a power meter to measure energy consumption to limit overdraw.
    shared_ptr<HydroponicsRegulatedRail> addRegulatedPowerRail(Hydroponics_RailType railType,       // Rail type
                                                               float maxPower);                     // Maximum allowed power

    // Accessors.

    static Hydroponics *getActiveInstance();                        // Currently active Hydroponics instance (stack-owned)
    uint32_t getI2CSpeed() const;                                   // i2c clock speed (Hz, default: 400kHz)
    uint32_t getSPISpeed() const;                                   // SPI clock speed (Hz, default: 4MHz)
    Hydroponics_SystemMode getSystemMode() const;                   // System type mode (default: Recycling)
    Hydroponics_MeasurementMode getMeasurementMode() const;         // System measurement mode (default: Imperial)
    Hydroponics_DisplayOutputMode getDisplayOutputMode() const;     // System LCD output mode (default: disabled)
    Hydroponics_ControlInputMode getControlInputMode() const;       // System control input mode (default: disabled)

    inline EasyBuzzerClass *getPiezoBuzzer() const { return &EasyBuzzer; }; // Piezo buzzer instance
    I2C_eeprom *getEEPROM(bool begin = true);                       // EEPROM instance (lazily instantiated, nullptr return = failure/no device)
    RTC_DS3231 *getRealTimeClock(bool begin = true);                // Real time clock instance (lazily instantiated, nullptr return = failure/no device)
    SDClass *getSDCard(bool begin = true);                          // SD card instance (if began user code *must* call end() to free SPI interface, lazily instantiated, nullptr return = failure/no device)

    int getActuatorCount() const;                                   // Current number of total actuators registered with system
    int getSensorCount() const;                                     // Current number of total sensors registered with system
    int getCropCount() const;                                       // Current number of total crops registered with system
    int getReservoirCount() const;                                  // Current number of total reservoirs registered with system
    int getRailCount() const;                                       // Current number of total power rails registered with system

    String getSystemName() const;                                   // System display name (default: "Hydruino")
    uint32_t getPollingInterval() const;                            // System sensor polling interval (time between sensor reads), in milliseconds (default: HYDRUINO_DATA_LOOP_INTERVAL)
    uint32_t getPollingFrame() const;                               // System polling frame number for sensor frame tracking
    bool isPollingFrameOld(uint32_t frame) const;                   // Determines if a given frame # if out of date (true) or current (false)
    DateTime getLastWaterChange() const;                            // Time of last water change, if tracked (recycling system only)

    int getControlInputRibbonPinCount();                            // Total number of pins being used for the current control input ribbon mode
    byte getControlInputPin(int ribbonPinIndex);                    // Control input pin mapped to ribbon pin index, or -1 (255) if not used

    // Mutators.

    void setSystemName(String systemName);                          // Sets display name of system (HYDRUINO_NAME_MAXSIZE size limit)
    void setPollingInterval(uint32_t pollingInterval);              // Sets system polling interval, in milliseconds (does not enable polling, see enable publishing methods)

    void setControlInputPinMap(byte *pinMap);                       // Sets custom pin mapping for control input, overriding consecutive ribbon pin numbers as default

protected:
    static Hydroponics *_activeInstance;                            // Current active instance (set after init)

    const byte _piezoBuzzerPin;                                     // Piezo buzzer pin (default: disabled)
    const int _eepromDeviceSize;                                    // EEPROM device size (default: disabled)
    const byte _sdCardCSPin;                                        // SD card cable select (CS) pin (default: disabled)
    const byte _ctrlInputPin1;                                      // Control input pin 1 (default: disabled)
    const byte _eepromI2CAddr;                                      // EEPROM i2c address, format: {A2,A1,A0} (default: B000)
    const byte _rtcI2CAddr;                                         // RTC i2c address, format: {A2,A1,A0} (default: B000, note: only B000 can be used atm)
    const byte _lcdI2CAddr;                                         // LCD i2c address, format: {A2,A1,A0} (default: B000)
    TwoWire* _i2cWire;                                              // Wire class instance (unowned) (default: Wire)
    uint32_t _i2cSpeed;                                             // Controller's i2c clock speed (default: 400kHz)
    uint32_t _spiSpeed;                                             // Controller's SPI clock speed (default: 4MHz)

#ifdef HYDRUINO_USE_TASKSCHEDULER
    Scheduler _ts;                                                  // Task scheduler
    Task *_controlTask;                                             // Main control task
    Task *_dataTask;                                                // Data collection task (on polling interval)
    Task *_miscTask;                                                // Misc task
#elif defined(HYDRUINO_USE_SCHEDULER)
    bool _suspend;                                                  // Suspend tracking
#endif
    I2C_eeprom *_eeprom;                                            // EEPROM instance (owned, lazy)
    RTC_DS3231 *_rtc;                                               // Real time clock instance (owned, lazy)
    SDClass *_sd;                                                   // SD card instance (owned/unowned, lazy)
    bool _eepromBegan;                                              // Status of EEPROM begin()
    bool _rtcBegan;                                                 // Status of RTC begin() call
    bool _rtcBattFail;                                              // Status of RTC battery failure flag
    byte _ctrlInputPinMap[HYDRUINO_CTRLINPINMAP_MAXSIZE];           // Control input pin map (aliasing for systemData until created)

    HydroponicsSystemData *_systemData;                             // System data (owned, saved to storage)
    uint32_t _pollingFrame;                                         // Polling frame #

    arx::map<Hydroponics_KeyType, shared_ptr<HydroponicsObject> > _objects; // Shared object collection, key'ed by HydroponicsIdentity.

    void allocateEEPROM();
    void deallocateEEPROM();
    void allocateRTC();
    void deallocateRTC();
    void allocateSD();
    void deallocateSD();

    void commonInit();
    void commonPostSave();

    friend void ::controlLoop();
    friend void ::dataLoop();
    friend void ::miscLoop();
    void updateObjects(int pass);
    void updateScheduling();
    void updateLogging();

    void checkFreeMemory();

    //void forwardPublishData(paramsTODO);
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
    void forwardLogMessage(String message, bool flushAfter = false);
    friend void ::logMessage(String,bool);
    #endif // /ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
};

#endif // /ifndef Hydroponics_H
