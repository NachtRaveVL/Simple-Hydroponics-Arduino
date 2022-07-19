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

// Uncomment or -D this define to completely disable usage of any multitasking commands, such as yield(), as well as libraries. Not recommended.
//#define HYDRUINO_DISABLE_MULTITASKING

// Uncomment or -D this define to disable usage of the TaskScheduler library, which is used by default.
//#define HYDRUINO_DISABLE_TASKSCHEDULER          // https://github.com/arkhipenko/TaskScheduler

// Uncomment or -D this define to enable usage of the Scheduler library, iff TaskScheduler disabled, for SAM/SAMD architectures only.
//#define HYDRUINO_ENABLE_SCHEDULER               // https://github.com/arduino-libraries/Scheduler

// Uncomment or -D this define to disable usage of tcMenu library, which will disable all GUI control. Not recommended.
//#define HYDRUINO_DISABLE_GUI                    // https://github.com/davetcc/tcMenu

// Uncomment or -D this define to enable debug output.
//#define HYDRUINO_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to specifically disable debug assertions.
//#define HYDRUINO_DISABLE_DEBUG_ASSERTIONS


#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#elif defined(__MBED__)
#include <mbed.h>
#else
#include <WProgram.h>
#endif
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <Wire.h>
#if defined(ESP32) || defined(ESP8266)
typedef SDFileSystemClass SDClass;
#else
#include <util/atomic.h>
#endif

#if defined(NDEBUG) && defined(HYDRUINO_ENABLE_DEBUG_OUTPUT)
#undef HYDRUINO_ENABLE_DEBUG_OUTPUT
#endif
#if defined(HYDRUINO_ENABLE_DEBUG_OUTPUT) && !defined(HYDRUINO_DISABLE_DEBUG_ASSERTIONS)
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
#ifndef __STM32F1__
#include "OneWire.h"                    // OneWire for DS18* probes
#else
#include <OneWireSTM.h>                 // STM32 version of OneWire
#endif
#include "RTClib.h"                     // i2c RTC library
#include "TaskManager.h"                // Task Manager library
#include "TimeLib.h"                    // Time library
#ifndef HYDRUINO_DISABLE_GUI
#include "tcMenu.h"                     // tcMenu library
#endif


#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L // Have libstdc++11
using namespace std;
#else
using namespace arx;
#endif
using namespace arx::stdx;

#include "HydroponicsDefines.h"
#include "HydroponicsStrings.h"
#include "HydroponicsInlines.hpp"
#include "HydroponicsInterfaces.h"
#include "HydroponicsData.h"
#include "HydroponicsObject.h"
#include "HydroponicsMeasurements.h"
#include "HydroponicsUtils.hpp"
#include "HydroponicsDatas.h"
#include "HydroponicsCropsLibrary.h"
#include "HydroponicsCalibrationsStore.h"
#include "HydroponicsStreams.h"
#include "HydroponicsTriggers.h"
#include "HydroponicsBalancers.h"
#include "HydroponicsActuators.h"
#include "HydroponicsSensors.h"
#include "HydroponicsCrops.h"
#include "HydroponicsReservoirs.h"
#include "HydroponicsRails.h"
#include "HydroponicsScheduler.h"

// Hydroponics Controller
class Hydroponics {
public:
    // Library constructor. Typically called during class instantiation, before setup().
    // TODO
    Hydroponics(byte piezoBuzzerPin = -1,
                uint32_t eepromDeviceSize = 0,              // use I2C_DEVICESIZE_* defines
                byte sdCardCSPin = -1,
                byte controlInputPin1 = -1,                 // first pin of ribbon (pins can be individually customized later)
                byte eepromI2CAddress = B000,
                byte rtcI2CAddress = B000,                  // only B000 can be used atm
                byte lcdI2CAddress = B000,
                TwoWire &i2cWire = Wire, uint32_t i2cSpeed = 400000U,
                SPIClass &spi = SPI, uint32_t spiSpeed = 4000000U,
                WiFiClass &wifi = WiFi);
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

    // Custom Additives.

    // Sets custom additive data, returning success flag.
    bool setCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData);
    // Drops custom additive data, returning success flag.
    bool dropCustomAdditiveData(const HydroponicsCustomAdditiveData *customAdditiveData);
    // Returns custom additive data (if any), else nullptr.
    const HydroponicsCustomAdditiveData *getCustomAdditiveData(Hydroponics_ReservoirType reservoirType) const;

    // Pin Locks.

    // Attempts to get a lock on pin #, to prevent multi-device comm overlap (e.g. for OneWire comms).
    bool tryGetPinLock(byte pin, time_t waitMillis = 250);
    // Returns a locked pin lock for the given pin. Only call if pin lock was successfully locked.
    void returnPinLock(byte pin);

    // Object Registration.

    // Adds/removes objects to/from system, returning success
    bool registerObject(shared_ptr<HydroponicsObject> obj);
    bool unregisterObject(shared_ptr<HydroponicsObject> obj);

    // Searches for object by id key (nullptr return = no obj by that id, position index may use HYDRUINO_POS_SEARCH* defines)
    shared_ptr<HydroponicsObject> objectById(HydroponicsIdentity id) const;
    Hydroponics_PositionIndex firstPosition(HydroponicsIdentity id, bool taken);
    inline Hydroponics_PositionIndex firstPositionTaken(HydroponicsIdentity id) { return firstPosition(id, true); }
    inline Hydroponics_PositionIndex firstPositionOpen(HydroponicsIdentity id) { return firstPosition(id, false); }
 
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

    // Adds a new analog PH meter to the system using the given parameters.
    // pH meters are vital in ensuring the proper alkalinity level is used in feed water.
    shared_ptr<HydroponicsAnalogSensor> addAnalogPhMeter(byte inputPin,                             // Analog input pin this sensor sits on
                                                         byte inputBitRes = 8);                     // ADC bit resolution to use
    // Adds a new analog TDS electrode to the system using the given parameters.
    // TDS electrodes are vital in ensuring the proper nutrition levels are used in feed water.
    shared_ptr<HydroponicsAnalogSensor> addAnalogTDSElectrode(byte inputPin,                        // Analog input pin this sensor sits on
                                                              int ppmScale = 500,                   // PPM measurement scaling (default: 500, aka TDS/PPM500)
                                                              byte inputBitRes = 8);                // ADC bit resolution to use
    // Adds a new analog temperature sensor to the system using the given parameters.
    // Temperature sensors can be used to ensure proper temperature conditions.
    shared_ptr<HydroponicsAnalogSensor> addAnalogTemperatureSensor(byte inputPin,                   // Analog input pin this sensor sits on
                                                                   byte inputBitRes = 8);           // ADC bit resolution to use
    // Adds a new analog CO2 sensor to the system using the given parameters.
    // CO2 sensors can be used to ensure proper CO2 levels.
    shared_ptr<HydroponicsAnalogSensor> addAnalogCO2Sensor(byte inputPin,                           // Analog input pin this sensor sits on
                                                           byte inputBitRes = 8);                   // ADC bit resolution to use

    // Adds a new analog PWM-based pump flow sensor to the system using the given parameters.
    // Pump flow sensors can allow for more precise liquid volume pumping calculations.
    shared_ptr<HydroponicsAnalogSensor> addPWMPumpFlowSensor(byte inputPin,                         // Analog input pin this sensor sits on
                                                             byte inputBitRes = 8);                 // ADC bit resolution to use
    // Adds a new analog water height meter to the system using the given parameters.
    // Water height meters can be used to determine the volume of a container.
    shared_ptr<HydroponicsAnalogSensor> addAnalogWaterHeightMeter(byte inputPin,                    // Analog input pin this sensor sits on
                                                                  byte inputBitRes = 8);            // ADC bit resolution to use
    // Adds a new downward-facing analog ultrasonic distance sensor to the system using the given parameters.
    // Downward-facing ultrasonic distance sensors can be used to determine the volume of a container.
    // (Pro-tip: These widely available inexpensive sensors don't sit in the water and thus won't corrode.)
    shared_ptr<HydroponicsAnalogSensor> addUltrasonicDistanceSensor(byte inputPin,                  // Analog input pin this sensor sits on
                                                                    byte inputBitRes = 8);          // ADC bit resolution to use

    // Adds a new analog power usage meter to the system using the given parameters.
    // Power usage meters can be used to determine and manage the energy demands of a power rail.
    shared_ptr<HydroponicsAnalogSensor> addPowerUsageMeter(byte inputPin,                           // Analog input pin this sensor sits on
                                                           bool isWattageBased,                     // If power meter measures wattage (true) or amperage (false)
                                                           byte inputBitRes = 8);                   // ADC bit resolution to use

    // Adds a new digital DHT* OneWire temperature & humidity sensor to the system using the given parameters.
    // Uses the DHT library. A very common digital sensor, included in most Arduino starter kits.
    shared_ptr<HydroponicsDHTTempHumiditySensor> addDHTTempHumiditySensor(byte inputPin,            // OneWire digital input pin this sensor sits on
                                                                          byte dhtType = DHT12);    // Kind of DHT sensor (see DHT* defines)
    // Adds a new digital DS18* OneWire submersible temperature sensor to the system using the given parameters.
    // Uses the DallasTemperature library. A specialized submersible sensor meant for long-term usage.
    shared_ptr<HydroponicsDSTemperatureSensor> addDSTemperatureSensor(byte inputPin,                // OneWire digital input pin this sensor sits on
                                                                      byte inputBitRes = 9,         // Sensor ADC bit resolution to use
                                                                      byte pullupPin = -1);         // Strong pullup pin (if used, else -1)

    // Convenience builders for common crops (shared, nullptr return = failure).

    // Adds a new simple timer-fed crop to the system using the given parameters.
    // Timer fed crops use a simple on/off timer for driving their feeding signal.
    shared_ptr<HydroponicsTimedCrop> addTimerFedCrop(Hydroponics_CropType cropType,                 // Crop type
                                                     Hydroponics_SubstrateType substrateType,       // Substrate type
                                                     DateTime sowDate,                              // Sow date
                                                     byte minsOn = 15,                              // Feeding signal on-time interval, in minutes
                                                     byte minsOff = 45);                            // Feeding signal off-time interval, in minutes
    // Adds a new simple timer-fed crop to the system using the given parameters (perennials only).
    // Perennials that grow back are easier to define from their last end-of-harvest date instead of when they were planted.
    shared_ptr<HydroponicsTimedCrop> addTimerFedPerennialCrop(Hydroponics_CropType cropType,        // Crop type
                                                              Hydroponics_SubstrateType substrateType, // Substrate type
                                                              DateTime lastHarvestDate,             // Last harvest date
                                                              byte minsOn = 15,                     // Feeding signal on-time interval, in minutes
                                                              byte minsOff = 45);                   // Feeding signal off-time interval, in minutes

    // Adds a new adaptive trigger-fed crop to the system using the given parameters.
    // Adaptive crops use soil based sensors, such as moisture sensors, to drive their feeding signal.
    shared_ptr<HydroponicsAdaptiveCrop> addAdaptiveFedCrop(Hydroponics_CropType cropType,           // Crop type
                                                           Hydroponics_SubstrateType substrateType, // Substrate type
                                                           DateTime sowDate);                       // Sow date
    // Adds a new adaptive trigger-fed crop to the system using the given parameters (perennials only).
    // Perennials that grow back are easier to define from their last end-of-harvest date instead of when they were planted.
    shared_ptr<HydroponicsAdaptiveCrop> addAdaptiveFedPerennialCrop(Hydroponics_CropType cropType,  // Crop type
                                                                    Hydroponics_SubstrateType substrateType, // Substrate type
                                                                    DateTime lastHarvestDate);      // Last harvest date

    // Convenience builders for common reservoirs (shared, nullptr return = failure).

    // Adds a new simple fluid reservoir to the system using the given parameters.
    // Fluid reservoirs are basically just buckets of some liquid solution with a known or measurable volume.
    shared_ptr<HydroponicsFluidReservoir> addFluidReservoir(Hydroponics_ReservoirType reservoirType, // Reservoir type
                                                            float maxVolume);                        // Maximum volume

    // Adds a new feed reservoir to the system using the given parameters.
    // Feed reservoirs, aka channels, are the reservoirs used to feed crops and provide a central point for managing feeding.
    shared_ptr<HydroponicsFeedReservoir> addFeedWaterReservoir(float maxVolume,                     // Maximum volume
                                                               DateTime lastChangeDate = DateTime((uint32_t)now()), // Last water change date
                                                               DateTime lastPruningDate = DateTime((uint32_t)0)); // Last pruning date

    // Adds a drainage pipe to the system using the given parameters.
    // Drainage pipes are never-filled infinite reservoirs that can always be pumped/drained into.
    shared_ptr<HydroponicsInfiniteReservoir> addDrainagePipe();
    // Adds a fresh water main to the system using the given parameters.
    // Fresh water mains are always-filled infinite reservoirs that can always be pumped/sourced from.
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

    // Mutators.

    void setControlInputPinMap(byte *pinMap);                       // Sets custom pin mapping for control input, overriding consecutive ribbon pin numbers as default
    void setSystemName(String systemName);                          // Sets display name of system (HYDRUINO_NAME_MAXSIZE size limit)
    void setTimeZoneOffset(int8_t timeZoneOffset);                  // Sets system time zone offset from UTC
    void setPollingInterval(uint32_t pollingInterval);              // Sets system polling interval, in milliseconds (does not enable polling, see enable publishing methods)
    void setWiFiConnection(String ssid, String password);           // Sets WiFi connection's SSID and password (note: password is stored encrypted, but is not hack-proof)

    // Accessors.

    static Hydroponics *getActiveInstance();                        // Currently active Hydroponics instance
    inline TwoWire *getI2C() const { return _i2cWire; }             // i2c Wire interface instance
    inline uint32_t getI2CSpeed() const { return _i2cSpeed; }       // i2c clock speed (Hz, default: 400kHz)
    inline SPIClass *getSPI() const { return &SPI; }                // SPI interface instance
    inline uint32_t getSPISpeed() const { return _spiSpeed; }       // SPI clock speed (Hz, default: 4MHz)
    int getControlInputRibbonPinCount() const;                      // Total number of pins being used for the current control input ribbon mode
    byte getControlInputPin(int ribbonPinIndex) const;              // Control input pin mapped to ribbon pin index, or -1 (255) if not used

    inline EasyBuzzerClass *getPiezoBuzzer() const { return &EasyBuzzer; }; // Piezo buzzer instance
    I2C_eeprom *getEEPROM(bool begin = true);                       // EEPROM instance (lazily instantiated, nullptr return = failure/no device)
    RTC_DS3231 *getRealTimeClock(bool begin = true);                // Real time clock instance (lazily instantiated, nullptr return = failure/no device)
    SDClass *getSDCard(bool begin = true);                          // SD card instance (if began user code *must* call end() to free SPI interface, lazily instantiated, nullptr return = failure/no device)
    WiFiClass *getWiFi(bool begin = true);                          // WiFi instance (nullptr return = failure/no device, this method may block for a minute or more)
    OneWire *getOneWireForPin(byte pin);                            // OneWire instance for given pin (lazily instantiated)
    void dropOneWireForPin(byte pin);                               // Drops OneWire instance for given pin (if created)

    bool getInOperationalMode() const;                              // Whenever the system is in operational mode (has been launched), or not
    Hydroponics_SystemMode getSystemMode() const;                   // System type mode (default: Recycling)
    Hydroponics_MeasurementMode getMeasurementMode() const;         // System measurement mode (default: Imperial)
    Hydroponics_DisplayOutputMode getDisplayOutputMode() const;     // System LCD output mode (default: disabled)
    Hydroponics_ControlInputMode getControlInputMode() const;       // System control input mode (default: disabled)
    String getSystemName() const;                                   // System display name (default: "Hydruino")
    int8_t getTimeZoneOffset() const;                               // System time zone offset from UTC (default: +0)
    bool getRTCBatteryFailure() const;                              // Whenever the system booted up with RTC battery failure flag set
    uint32_t getPollingInterval() const;                            // System sensor polling interval (time between sensor reads), in milliseconds (default: HYDRUINO_DATA_LOOP_INTERVAL)
    uint32_t getPollingFrame() const;                               // System polling frame number for sensor frame tracking
    bool getIsPollingFrameOld(unsigned int frame, unsigned int allowance = 0) const; // Determines if a given frame # if out of date (true) or current (false), with optional frame # difference allowance
    String getWiFiSSID();                                           // SSID for WiFi connection
    String getWiFiPassword();                                       // Password for WiFi connection (plaintext)

    // Misc.

    void notifyRTCTimeUpdated();                                    // Called when RTC time is updated, unsets battery failure flag and sets rescheduling flag

protected:
    static Hydroponics *_activeInstance;                            // Current active instance (set after init)

    const byte _piezoBuzzerPin;                                     // Piezo buzzer pin (default: disabled)
    const int _eepromDeviceSize;                                    // EEPROM device size (default: disabled)
    const byte _sdCardCSPin;                                        // SD card cable select (CS) pin (default: disabled)
    const byte _ctrlInputPin1;                                      // Control input pin 1 (default: disabled)
    const byte _eepromI2CAddr;                                      // EEPROM i2c address, format: {A2,A1,A0} (default: B000)
    const byte _rtcI2CAddr;                                         // RTC i2c address, format: {A2,A1,A0} (default: B000, note: only B000 can be used atm)
    const byte _lcdI2CAddr;                                         // LCD i2c address, format: {A2,A1,A0} (default: B000)
    TwoWire *_i2cWire;                                              // Controller's i2c wire class instance (strong) (default: Wire)
    uint32_t _i2cSpeed;                                             // Controller's i2c clock speed (default: 400kHz)
    SPIClass *_spi;                                                 // Controller's SPI class interface (strong) (default: SPI)
    uint32_t _spiSpeed;                                             // Controller's SPI clock speed (default: 4MHz)
    WiFiClass *_wifi;                                               // WiFi class instance (strong) (default: WiFi)

#ifdef HYDRUINO_USE_TASKSCHEDULER
    Scheduler _ts;                                                  // Task scheduler
    Task *_controlTask;                                             // Main control task
    Task *_dataTask;                                                // Data collection task (on polling interval)
    Task *_miscTask;                                                // Misc task
#elif defined(HYDRUINO_USE_SCHEDULER)
    bool _loopsStarted;                                             // Loops started flag
    bool _suspend;                                                  // Suspend operation flag
#endif
    I2C_eeprom *_eeprom;                                            // EEPROM instance (owned, lazy)
    RTC_DS3231 *_rtc;                                               // Real time clock instance (owned, lazy)
    SDClass *_sd;                                                   // SD card instance (owned/unowned, lazy)
    bool _eepromBegan;                                              // Status of EEPROM begin() call
    bool _rtcBegan;                                                 // Status of RTC begin() call
    bool _rtcBattFail;                                              // Status of RTC battery failure flag
    bool _wifiBegan;                                                // Status of WiFi begin() call
    byte _ctrlInputPinMap[HYDRUINO_CTRLINPINMAP_MAXSIZE];           // Control input pin map

    HydroponicsSystemData *_systemData;                             // System data (owned, saved to storage)
    uint32_t _pollingFrame;                                         // Polling frame #

    arx::map<Hydroponics_KeyType, shared_ptr<HydroponicsObject>, HYDRUINO_OBJ_LINKS_MAXSIZE> _objects; // Shared object collection, key'ed by HydroponicsIdentity
    arx::map<Hydroponics_ReservoirType, HydroponicsCustomAdditiveData *, Hydroponics_ReservoirType_CustomAdditiveCount> _additives; // Custom additives data
    arx::map<byte, OneWire *, HYDRUINO_SYS_ONEWIRE_MAXSIZE> _oneWires; // pin->OneWire list
    arx::map<byte, byte> _pinLocks;                                 // Pin locks list (existence = locked)

    HydroponicsScheduler _scheduler;                                // Scheduler piggy-back instance
    friend class HydroponicsScheduler;
    friend HydroponicsScheduler *::getSchedulerInstance();

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

    shared_ptr<HydroponicsObject> objectById_Col(const HydroponicsIdentity &id) const;

    void handleInterrupt(pintype_t pin);
    friend void ::handleInterrupt(pintype_t pin);

    void checkFreeMemory();
    //void forwardPublishData(paramsTODO);
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
    void forwardLogMessage(String message, bool flushAfter = false);
    friend void ::logMessage(String,bool);
    #endif // /ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
};

#endif // /ifndef Hydroponics_H
