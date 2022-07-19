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
#endif

#if defined(__AVR__)
#include <util/atomic.h>
#define CRITICAL_SECTION ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#elif defined(__arm__)
// See http://stackoverflow.com/questions/27998059/atomic-block-for-reading-vs-arm-systicks
extern int __int_disable_irq(void);
extern void __int_restore_irq(int *primask);
#define CRITICAL_SECTION for (int primask_save __attribute__((__cleanup__(__int_restore_irq))) = __int_disable_irq(), __ToDo = 1; __ToDo; __ToDo = 0)
#else
#define CRITICAL_SECTION for (noInterrupts(), __ToDo = 1; __ToDo; interrupts(), __ToDo = 0)
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
#ifndef __STM32__
#include "OneWire.h"                    // OneWire library
#else
#include <OneWireSTM.h>                 // STM32 version of OneWire (via stm32duino)
#endif
#include "RTClib.h"                     // i2c RTC library
#include "TaskManagerIO.h"              // Task Manager library
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
#include "HydroponicsFactory.h"

// Hydroponics Controller
class Hydroponics : public HydroponicsFactory {
public:
    // Library constructor. Typically called during class instantiation, before setup().
    Hydroponics(byte piezoBuzzerPin = -1,                   // Piezo buzzer pin, else -1
                uint32_t eepromDeviceSize = 0,              // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
                byte sdCardCSPin = -1,                      // SD card CS pin, else -1
                byte controlInputPin1 = -1,                 // First pin of input ribbon, else -1 (ribbon pins can be individually customized later)
                byte eepromI2CAddress = B000,               // EEPROM address
                byte rtcI2CAddress = B000,                  // RTC i2c address (only B000 can be used atm)
                byte lcdI2CAddress = B000,                  // LCD i2c address
                TwoWire &i2cWire = Wire,                    // I2C wire class instance
                uint32_t i2cSpeed = 400000U,                // I2C speed
                uint32_t sdCardSpeed = 4000000U,            // SD card SPI speed
                WiFiClass &wifi = WiFi);                    // WiFi class instance
    // Library destructor. Just in case.
    ~Hydroponics();

    // Initializes default empty system. Typically called near top of setup().
    // See individual enums for more info.
    void init(Hydroponics_SystemMode systemMode = Hydroponics_SystemMode_Recycling,                 // What system of crop feeding is performed
              Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Default,        // What units of measurement should be used
              Hydroponics_DisplayOutputMode dispOutMode = Hydroponics_DisplayOutputMode_Disabled,   // What display output mode should be used
              Hydroponics_ControlInputMode ctrlInMode = Hydroponics_ControlInputMode_Disabled);     // What control input mode should be used

    // Initializes system from EEPROM save, returning success flag
    bool initFromEEPROM(bool jsonFormat = false);
    // Initializes system from SD card file save, returning success flag
    bool initFromSDCard(String configFile = "hydruino.cfg", bool jsonFormat = true);
    // Initializes system from custom JSON-based stream, returning success flag
    bool initFromJSONStream(Stream *streamIn);
    // Initializes system from custom binary stream, returning success flag
    bool initFromBinaryStream(Stream *streamIn);
    // TODO: Network URL init
    //bool initFromNetworkURL(urlData, configFile = "hydruino.cfg");

    // Saves current system setup to EEPROM save, returning success flag
    bool saveToEEPROM(bool jsonFormat = false);
    // Saves current system setup to SD card file save, returning success flag
    bool saveToSDCard(String configFile = "hydruino.cfg", bool jsonFormat = true);
    // Saves current system setup to custom JSON-based stream, returning success flag
    bool saveToJSONStream(Stream *streamOut, bool compact = true);
    // Saves current system setup 
    bool saveToBinaryStream(Stream *streamOut);
    // TODO: Network URL save
    //bool saveToNetworkURL(urlData, configFile = "hydruino.cfg");

    // System Operation.

    // Launches system into operational mode. Typically called near end of setup().
    void launch();

    // Suspends the system from operational mode (disables all runloops). Typically used during system setup UI.
    // Resume operation by a call to launch().
    void suspend();

    // Update method. Typically called in loop().
    void update();

    // System Logging.

    // TODO: SD card sys logging
    //bool enableSysLoggingToSDCard(String logFilePrefix = "logs/sys");
    // TODO: Network URL sys logging
    //bool enableSysLoggingToNetworkURL(urlData, String logFilePrefix = "logs/sys");

    // Data Publishing.

    // TODO: SD card data pub
    //bool enableDataPublishingToSDCard(String csvFilePrefix = "logs/dat");
    // TODO: Network URL data pub
    //bool enableDataPublishingToNetworkURL(urlData, String csvFilePrefix = "logs/dat");
    // TODO: MQTT data pub
    //bool enableDataPublishingToMQTT(mqttBroker, deviceData);
    // TODO: Web API data pub
    //bool enableDataPublishingToWebAPI(urlData, apiInterface);

    // Object Registration.

    // Adds object to system, returning success
    bool registerObject(shared_ptr<HydroponicsObject> obj);
    // Removes object from system, returning success
    bool unregisterObject(shared_ptr<HydroponicsObject> obj);

    // Searches for object by id key (nullptr return = no obj by that id, position index may use HYDRUINO_POS_SEARCH* defines)
    shared_ptr<HydroponicsObject> objectById(HydroponicsIdentity id) const;

    // Finds first position either open or taken, given the id type
    Hydroponics_PositionIndex firstPosition(HydroponicsIdentity id, bool taken);
    // Finds first position taken, given the id type
    inline Hydroponics_PositionIndex firstPositionTaken(HydroponicsIdentity id) { return firstPosition(id, true); }
    // Finds first position open, given the id type
    inline Hydroponics_PositionIndex firstPositionOpen(HydroponicsIdentity id) { return firstPosition(id, false); }

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

    // Mutators.

    // Sets custom pin mapping for control input, overriding consecutive ribbon pin numbers as default
    void setControlInputPinMap(byte *pinMap);
    // Sets display name of system (HYDRUINO_NAME_MAXSIZE size limit)
    void setSystemName(String systemName);
    // Sets system time zone offset from UTC
    void setTimeZoneOffset(int8_t timeZoneOffset);
    // Sets system polling interval, in milliseconds (does not enable polling, see enable publishing methods)
    void setPollingInterval(uint32_t pollingInterval);
    // Sets WiFi connection's SSID and password (note: password is stored encrypted, but is not hack-proof)
    void setWiFiConnection(String ssid, String password);

    // Accessors.

    // Currently active Hydroponics instance
    static Hydroponics *getActiveInstance();
    // i2c Wire interface instance (customizable)
    inline TwoWire *getI2C() const { return _i2cWire; }
    // i2c clock speed, in Hz (default: 400kHz)
    inline uint32_t getI2CSpeed() const { return _i2cSpeed; }
    // SPI interface instance (hardwired)
    inline SPIClass *getSPI() const { return &SPI; }
    // SD card SPI clock speed, in Hz (default: 4MHz)
    inline uint32_t getSDCardSpeed() const { return _sdCardSpeed; }
    // Total number of pins being used for the current control input ribbon mode
    int getControlInputRibbonPinCount() const;
    // Control input pin mapped to ribbon pin index, or -1 (255) if not used
    byte getControlInputPin(int ribbonPinIndex) const;

    // Piezo buzzer instance
    inline EasyBuzzerClass *getPiezoBuzzer() const { return &EasyBuzzer; };
    // EEPROM instance (lazily instantiated, nullptr return = failure/no device)
    I2C_eeprom *getEEPROM(bool begin = true);
    // Real time clock instance (lazily instantiated, nullptr return = failure/no device)
    RTC_DS3231 *getRealTimeClock(bool begin = true);
    // SD card instance (if began user code *must* call end() to free interface, lazily instantiated, nullptr return = failure/no device)
    SDClass *getSDCard(bool begin = true);
    // WiFi instance (nullptr return = failure/no device, note: this method may block for up to a minute)
    WiFiClass *getWiFi(bool begin = true);
    // OneWire instance for given pin (lazily instantiated)
    OneWire *getOneWireForPin(byte pin);
    // Drops OneWire instance for given pin (if created)
    void dropOneWireForPin(byte pin);

    // Whenever the system is in operational mode (has been launched), or not
    bool getInOperationalMode() const;
    // System type mode (default: Recycling)
    Hydroponics_SystemMode getSystemMode() const;
    // System measurement mode (default: Metric)
    Hydroponics_MeasurementMode getMeasurementMode() const;
    // System LCD output mode (default: Disabled)
    Hydroponics_DisplayOutputMode getDisplayOutputMode() const;
    // System control input mode (default: Disabled)
    Hydroponics_ControlInputMode getControlInputMode() const;
    // System display name (default: "Hydruino")
    String getSystemName() const;
    // System time zone offset from UTC (default: +0/UTC)
    int8_t getTimeZoneOffset() const;
    // Whenever the system booted up with the RTC battery failure flag set (meaning the time is not set correctly)
    bool getRTCBatteryFailure() const;
    // System sensor polling interval (time between sensor reads), in milliseconds (default: HYDRUINO_DATA_LOOP_INTERVAL)
    uint32_t getPollingInterval() const;
    // System polling frame number for sensor frame tracking
    uint32_t getPollingFrame() const;
    // Determines if a given frame # if out of date (true) or current (false), with optional frame # difference allowance
    bool getIsPollingFrameOld(unsigned int frame, unsigned int allowance = 0) const;
    // SSID for WiFi connection
    String getWiFiSSID();
    // Password for WiFi connection (plaintext)
    String getWiFiPassword();

    // Misc.

    // Called to notify system when RTC time is updated
    void notifyRTCTimeUpdated();

protected:
    static Hydroponics *_activeInstance;                            // Current active instance (set after init)

    const byte _piezoBuzzerPin;                                     // Piezo buzzer pin (default: Disabled)
    const int _eepromDeviceSize;                                    // EEPROM device size (default: 0/Disabled)
    const byte _sdCardCSPin;                                        // SD card cable select (CS) pin (default: Disabled)
    const byte _ctrlInputPin1;                                      // Control input pin 1 (default: Disabled)
    const byte _eepromI2CAddr;                                      // EEPROM i2c address, format: {A2,A1,A0} (default: B000)
    const byte _rtcI2CAddr;                                         // RTC i2c address, format: {A2,A1,A0} (default: B000, note: only B000 can be used atm)
    const byte _lcdI2CAddr;                                         // LCD i2c address, format: {A2,A1,A0} (default: B000)
    TwoWire *_i2cWire;                                              // Controller's i2c wire class instance (strong) (default: Wire)
    uint32_t _i2cSpeed;                                             // Controller's i2c clock speed (default: 400kHz)
    uint32_t _sdCardSpeed;                                          // SD card's SPI clock speed (default: 4MHz)
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
