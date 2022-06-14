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


// Hookup Callouts
// -PLEASE READ-
// TODO.

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

#include "HydroponicsDefines.h"
#include "HydroponicsInlines.hpp"
#include "HydroponicsInterfaces.h"
#include "HYdroponicsUtils.h"
#include "HydroponicsDatas.h"
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

    // TODO: maybe?
    //bool initFromNetworkURL(wifiClient, serverData, configFile = "hydruino.cfg");
    //void enableLoggingToSDCard(sd, logFilePrefix = "logs/sys");
    //void enableLoggingToNetworkURL(netClient, urlData, logFilePrefix = "logs/sys");
    //void enablePublishingToSDCard(sd, csvFilePrefix = "logs/dat");
    //void enablePublishingToNetworkURL(netClient, urlData, csvFilePrefix = "logs/dat");
    //void enablePublishingToMQTT(mqttBroker, deviceData);
    //void enablePublishingToWebAPI(netClient, urlData, apiInterface);

    // Launches system into operational mode. Typically called last in setup().
    // Once launch is called further system setup may no longer be available due to dependency constraints.
    void launch();

    // Update method. Typically called last in loop().
    // By default this method simply calls into the active scheduler's main loop mechanism, unless multitasking is disabled in which case calls all runloops.
    void update();


    // Object Registration.

    // Adds/removes objects to/from system (ownership transfer)
    bool registerObject(HydroponicsObject *obj);
    bool unregisterObject(HydroponicsObject *obj);

    // Searches for object by key (nullptr return = no obj by that identity, index may use HYDRUINO_ATPOS_SEARCH* defines)
    HydroponicsObject *findObjectByKey(HydroponicsIdentity identity) const;

    inline HydroponicsActuator *findActuatorByKey(Hydroponics_ActuatorType actuatorType, Hydroponics_PositionIndex actuatorIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG) const {
        return reinterpret_cast<HydroponicsActuator *>(findObjectByKey(HydroponicsIdentity(actuatorType, actuatorIndex)));
    }
    inline HydroponicsSensor *findSensorByKey(Hydroponics_SensorType sensorType, Hydroponics_PositionIndex sensorIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG) const {
        return reinterpret_cast<HydroponicsSensor *>(findObjectByKey(HydroponicsIdentity(sensorType, sensorIndex)));
    }
    inline HydroponicsCrop *findCropByKey(Hydroponics_CropType cropType, Hydroponics_PositionIndex cropIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG) const {
        return reinterpret_cast<HydroponicsCrop *>(findObjectByKey(HydroponicsIdentity(cropType, cropIndex)));
    }
    inline HydroponicsReservoir *findReservoirByKey(Hydroponics_ReservoirType reservoirType, Hydroponics_PositionIndex reservoirIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG) const {
        return reinterpret_cast<HydroponicsReservoir *>(findObjectByKey(HydroponicsIdentity(reservoirType, reservoirIndex)));
    }
    inline HydroponicsRail *findRailByKey(Hydroponics_RailType railType, Hydroponics_PositionIndex railIndex = HYDRUINO_ATPOS_SEARCH_FROMBEG) const {
        return reinterpret_cast<HydroponicsRail *>(findObjectByKey(HydroponicsIdentity(railType, railIndex)));
    }

    // Convenience builders for actuators (unowned, nullptr return = failure).

    HydroponicsRelayActuator *addGrowLightsRelay(byte outputPin);
    HydroponicsRelayActuator *addWaterPumpRelay(byte outputPin);
    HydroponicsRelayActuator *addWaterHeaterRelay(byte outputPin);
    HydroponicsRelayActuator *addWaterAeratorRelay(byte outputPin);
    HydroponicsRelayActuator *addFanExhaustRelay(byte outputPin);
    HydroponicsPWMActuator *addFanExhaustPWM(byte outputPin, byte outputBitRes = 8);
    HydroponicsRelayActuator *addPeristalticPumpRelay(byte outputPin);

    // Convenience builders for common sensors (unowned, nullptr return = failure).

    // Adds a new binary level indicator to the system.
    HydroponicsBinarySensor *addLevelIndicator(byte inputPin);                          // Digital input pin this sensor sits on

    // Adds a new analog CO2 sensor to the system.
    HydroponicsAnalogSensor *addCO2Sensor(byte inputPin,                                // Analog input pin this sensor sits on
                                          byte inputBitRes = 8);                        // ADC bit resolution to use
    // Adds a new analog PH meter to the system.
    HydroponicsAnalogSensor *addPhMeter(byte inputPin,                                  // Analog input pin this sensor sits on
                                        byte inputBitRes = 8);                          // ADC bit resolution to use
    // Adds a new analog temperature sensor to the system.
    HydroponicsAnalogSensor *addTempSensor(byte inputPin,                               // Analog input pin this sensor sits on
                                           byte inputBitRes = 8);                       // ADC bit resolution to use
    // Adds a new analog TDS electrode to the system.
    HydroponicsAnalogSensor *addTDSElectrode(byte inputPin,                             // Analog input pin this sensor sits on
                                             byte inputBitRes = 8);                     // ADC bit resolution to use
    // Adds a new analog pump flow sensor to the system.
    HydroponicsAnalogSensor *addPumpFlowSensor(byte inputPin,                           // Analog input pin this sensor sits on
                                               byte inputBitRes = 8);                   // ADC bit resolution to use
    // Adds a new analog water height meter to the system.
    HydroponicsAnalogSensor *addWaterHeightMeter(byte inputPin,                         // Analog input pin this sensor sits on
                                                 byte inputBitRes = 8);                 // ADC bit resolution to use
    // Adds a new analog ultrasonic distance sensor to the system.
    HydroponicsAnalogSensor *addUltrasonicDistanceSensor(byte inputPin,                 // Analog input pin this sensor sits on
                                                         byte inputBitRes = 8);         // ADC bit resolution to use

    // Adds a new digital DHT* OneWire temperature & humidity sensor to the system.
    HydroponicsDHTOneWireSensor *addDHTTempHumiditySensor(byte inputPin,                // OneWire-based input pin this sensor sits on
                                                          uint8_t dhtType = DHT12);     // Kind of DHT sensor (see DHT* defines)
    // Adds a new digital DS18* OneWire submersible temperature sensor to the system.
    HydroponicsDSOneWireSensor *addDSTemperatureSensor(byte inputPin,                   // OneWire-based input pin this sensor sits on
                                                       byte inputBitRes = 12);          // OneWire bit resolution to use
    // Adds a new digital TMP* OneWire soil moisture sensor to the system.
    HydroponicsTMPOneWireSensor *addTMPSoilMoistureSensor(byte inputPin,                // OneWire-based input pin this sensor sits on
                                                          byte inputBitRes = 12);       // OneWire bit resolution to use

    // Convenience builders for crops (weak, nullptr return = failure, pos index may use HYDRUINO_ATPOS_SEARCH* defines)
    HydroponicsCrop *addCropFromSowDate(Hydroponics_CropType cropType, Hydroponics_SubstrateType substrateType, time_t sowDate);
    HydroponicsCrop *addCropFromLastHarvest(Hydroponics_CropType cropType, Hydroponics_SubstrateType substrateType, time_t lastHarvestDate);


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

    int getRelayCount(Hydroponics_RailType relayRail = Hydroponics_RailType_Undefined) const;         // Current number of relay devices registered with system, for the given rail (default params = from all rails)
    int getActiveRelayCount(Hydroponics_RailType relayRail = Hydroponics_RailType_Undefined) const;   // Current number of active relay devices, for the given rail (default params = from all rails)
    byte getMaxActiveRelayCount(Hydroponics_RailType relayRail) const;                                 // Maximum number of relay devices allowed active at a time, for the given rail (default: 2)

    int getActuatorCount() const;                                   // Current number of total actuators registered with system
    int getSensorCount() const;                                     // Current number of total sensors registered with system
    int getCropCount() const;                                       // Current number of total crops registered with system

    String getSystemName() const;                                   // System display name (default: "Hydruino")
    uint32_t getPollingIntervalMillis() const;                      // System sensor polling interval (time between sensor reads) in milliseconds (default: 0 when disabled, 5000 when enabled)
    float getReservoirVolume(Hydroponics_ReservoirType forFluidReservoir = Hydroponics_ReservoirType_FeedWater) const;    // Fluid reservoir volume, for given reservoir
    float getPumpFlowRate(Hydroponics_ReservoirType forFluidReservoir = Hydroponics_ReservoirType_FeedWater) const;       // Fluid pump flow rate, for given reservoir

    int getControlInputRibbonPinCount();                            // Total number of pins being used for the current control input ribbon mode
    byte getControlInputPin(int ribbonPinIndex);                    // Control input pin mapped to ribbon pin index, or -1 (255) if not used

    // Mutators.

    void setMaxActiveRelayCount(byte maxActiveCount, Hydroponics_RailType relayRail);   // Sets maximum number of relay devices allowed active at a time, for the given rail. This is useful for managing power limits on your system.

    void setSystemName(String systemName);                          // Sets display name of system (HYDRUINO_NAME_MAXSIZE char limit)
    void setPollingIntervalMillis(uint32_t pollingIntMs);           // Sets system polling interval in milliseconds (does not enable polling, see enablePublishingTo* methods)
    void setReservoirVolume(float reservoirVol, Hydroponics_ReservoirType forFluidReservoir);  // Sets reservoir volume, for the given reservoir
    void setPumpFlowRate(float pumpFlowRate, Hydroponics_ReservoirType forFluidReservoir);     // Sets pump flow rate, for the given reservoir

    void setControlInputPinMap(byte *pinMap);                       // Sets custom pin mapping for control input, overriding consecutive ribbon pin numbers as default

    typedef void(*UserDelayFunc)(unsigned int);                     // Passes delay timeout (where 0 indicates inside long blocking call / yield attempt suggested)
    // Sets user delay functions to call when a delay has to occur for processing to
    // continue. User functions here can customize what this means - typically it would
    // mean to call into a thread barrier() or yield() mechanism. Default implementation
    // is to call yield() when timeout >= 1ms, unless multitasking is disabled.
    void setUserDelayFuncs(UserDelayFunc delayMillisFunc, UserDelayFunc delayMicrosFunc);

protected:
    static Hydroponics *_activeInstance;                    // Current active instance (set after init)

    const byte _piezoBuzzerPin;                             // Piezo buzzer pin (default: disabled)
    const byte _sdCardCSPin;                                // SD card cable select (CS) pin (default: disabled)
    const byte _ctrlInputPin1;                              // Control input pin 1 (default: disabled)
    const byte _eepromI2CAddr;                              // EEPROM i2c address, format: {A2,A1,A0} (default: B000)
    const byte _rtcI2CAddr;                                 // RTC i2c address, format: {A2,A1,A0} (default: B000)
    const byte _lcdI2CAddr;                                 // LCD i2c address, format: {A2,A1,A0} (default: B000)
    TwoWire* _i2cWire;                                      // Wire class instance (unowned) (default: Wire)
    uint32_t _i2cSpeed;                                     // Controller's i2c clock speed (default: 400kHz)
    uint32_t _spiSpeed;                                     // Controller's SPI clock speed (default: 4MHz)

#ifdef HYDRUINO_USE_TASKSCHEDULER
    Scheduler _ts;                                          // Task scheduler
#endif
    EasyBuzzerClass *_buzzer;                               // Piezo buzzer instance (unowned)
    I2C_eeprom *_eeprom;                                    // EEPROM instance (owned, lazy)
    RTC_DS3231 *_rtc;                                       // Real time clock instance (owned, lazy)
    SDClass *_sd;                                           // SD card instance (owned/unowned, lazy)
    bool _eepromBegan;                                      // Status of EEPROM begin()
    bool _rtcBegan;                                         // Status of RTC begin() call
    bool _rtcBattFail;                                      // Status of RTC battery failure flag
    byte _ctrlInputPinMap[HYDRUINO_CTRLINPINMAP_MAXSIZE];   // Control input pin map

    HydroponicsSystemData *_systemData;                     // System data (owned, saved to storage)

    UserDelayFunc _uDelayMillisFunc;                        // User millisecond delay function
    UserDelayFunc _uDelayMicrosFunc;                        // User microsecond delay function

    //BtreeList<String, shared_ptr<HydroponicsObject> > _objects; // Objects in system

    // Allocation & initialization.

    void allocateEEPROM();
    void deallocateEEPROM();
    void allocateRTC();
    void deallocateRTC();
    void allocateSD();
    void deallocateSD();
    void commonInit();

    // Runloops & update segmentation.

    friend void ::controlLoop();
    friend void ::dataLoop();
    friend void ::guiLoop();
    friend void ::miscLoop();
    void updateActuators();
    void updateBuzzer();
    void updateCrops();
    void updateLogging();
    void updateScheduling();
    void updateScreen();
    void updateSensors();
};

#endif // /ifndef Hydroponics_H
