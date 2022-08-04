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

    Simple-Hydroponics-Arduino - Version 0.4
*/

#ifndef Hydroponics_H
#define Hydroponics_H

// Library Setup

// NOTE: It is recommended to use custom build flags instead of editing this file directly.

// Uncomment or -D this define to completely disable usage of any multitasking commands and libraries. Not recommended.
//#define HYDRUINO_DISABLE_MULTITASKING             // https://github.com/davetcc/TaskManagerIO

// Uncomment or -D this define to disable usage of tcMenu library, which will disable all GUI control. Not recommended.
//#define HYDRUINO_DISABLE_GUI                      // https://github.com/davetcc/tcMenu

// Uncomment or -D this define to enable external data storage (SD Card or EEPROM) to save on sketch size. Required for constrained devices.
//#define HYDRUINO_DISABLE_BUILTIN_DATA             // Disables built-in Crops Lib and String data, instead relying solely on external device.

// Uncomment or -D this define to enable debug output (treats Serial output as attached to serial monitor).
//#define HYDRUINO_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to enable verbose debug output (note: adds considerable size to compiled sketch).
//#define HYDRUINO_ENABLE_VERBOSE_DEBUG

// Uncomment or -D this define to enable debug assertions (note: adds significant size to compiled sketch).
//#define HYDRUINO_ENABLE_DEBUG_ASSERTIONS


#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#elif defined(__MBED__)
#include <mbed.h>
#else
#include <WProgram.h>
#endif
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <WiFi.h>

#if defined(ESP32) || defined(ESP8266)
typedef SDFileSystemClass SDClass;
#endif
#if defined(ARDUINO_ARCH_RP2040)
#include <api/Common.h>
#endif

#ifndef HYDRUINO_DISABLE_MULTITASKING
#if defined(__AVR__)
#include <util/atomic.h>
#define CRITICAL_SECTION ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#else
// See http://stackoverflow.com/questions/27998059/atomic-block-for-reading-vs-arm-systicks
extern int __int_disable_irq(void);
extern void __int_restore_irq(int *primask);
#define CRITICAL_SECTION for (int primask_save __attribute__((__cleanup__(__int_restore_irq))) = __int_disable_irq(), __ToDo = 1; __ToDo; __ToDo = 0)
#endif
#else
#ifndef HYDRUINO_DISABLE_GUI
#define HYDRUINO_DISABLE_GUI
#endif
#define secondsToMillis(val) ((val)*1000U)
#define CRITICAL_SECTION if (1)
#endif // /ifndef HYDRUINO_DISABLE_MULTITASKING

#if defined(NDEBUG) && defined(HYDRUINO_ENABLE_DEBUG_OUTPUT)
#undef HYDRUINO_ENABLE_DEBUG_OUTPUT
#endif
#if defined(HYDRUINO_ENABLE_DEBUG_OUTPUT) && defined(HYDRUINO_ENABLE_VERBOSE_DEBUG)
#define HYDRUINO_USE_VERBOSE_OUTPUT
#endif
#if defined(HYDRUINO_ENABLE_DEBUG_OUTPUT) && defined(HYDRUINO_ENABLE_DEBUG_ASSERTIONS)
#define HYDRUINO_SOFT_ASSERT(cond,msg)  softAssert((bool)(cond), String((msg)), __FILE__, __func__, __LINE__)
#define HYDRUINO_HARD_ASSERT(cond,msg)  hardAssert((bool)(cond), String((msg)), __FILE__, __func__, __LINE__)
#define HYDRUINO_USE_DEBUG_ASSERTIONS
#else
#define HYDRUINO_SOFT_ASSERT(cond,msg)  ((void)0)
#define HYDRUINO_HARD_ASSERT(cond,msg)  ((void)0)
#endif

#include "ArduinoJson.h"                // JSON library
#include "ArxContainer.h"               // STL-like container library
#include "ArxSmartPtr.h"                // Shared pointer library
#include "DallasTemperature.h"          // DS18* submersible water temp probe
#include "DHT.h"                        // DHT* air temp/humidity probe
#include "I2C_eeprom.h"                 // i2c EEPROM library
#ifdef ARDUINO_ARCH_STM32
#include <OneWireSTM.h>                 // STM32 version of OneWire (via stm32duino)
#else
#include "OneWire.h"                    // OneWire library
#endif
#include "RTClib.h"                     // i2c RTC library
#ifndef HYDRUINO_DISABLE_MULTITASKING
#include "TaskManagerIO.h"              // Task Manager library
#endif
#include "TimeLib.h"                    // Time library
#ifndef HYDRUINO_DISABLE_GUI
#include "tcMenu.h"                     // tcMenu library
#endif

#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L // Have libstdc++11
using namespace std;
template<typename T, size_t N = ARX_VECTOR_DEFAULT_SIZE> using Vector = std::vector<T>;
template<class T1, class T2> using Pair = std::pair<T1,T2>;
template<typename K, typename V, size_t N = ARX_MAP_DEFAULT_SIZE> using Map = std::map<K,V>;
#include "ArxSmartPtr/shared_ptr.h" // forced include
#else
using namespace arx;
using namespace arx::stdx;
template<typename T, size_t N = ARX_VECTOR_DEFAULT_SIZE> using Vector = arx::vector<T,N>;
template<class T1, class T2> using Pair = arx::pair<T1,T2>;
template<typename K, typename V, size_t N = ARX_MAP_DEFAULT_SIZE> using Map = arx::map<K,V,N>;
#endif
template <typename T> using SharedPtr = arx::stdx::shared_ptr<T>;

extern time_t unixNow();
extern void handleInterrupt(uint8_t pin);
extern void controlLoop();
extern void dataLoop();
extern void miscLoop();

#include "HydroponicsDefines.h"
#include "HydroponicsStrings.h"
#include "HydroponicsInlines.hh"
#include "HydroponicsCallback.hh"
#include "HydroponicsInterfaces.h"
#include "HydroponicsAttachments.h"
#include "HydroponicsData.h"
#include "HydroponicsObject.h"
#include "HydroponicsMeasurements.h"
#include "HydroponicsUtils.h"
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
#include "HydroponicsLogger.h"
#include "HydroponicsPublisher.h"
#include "HydroponicsFactory.h"

// Hydroponics Controller
// Main controller interface of the Hydroponics system.
class Hydroponics : public HydroponicsFactory {
public:
    HydroponicsScheduler scheduler;                         // Scheduler public instance
    HydroponicsLogger logger;                               // Logger public instance
    HydroponicsPublisher publisher;                         // Publisher public instance

    // Controller constructor. Typically called during class instantiation, before setup().
    Hydroponics(uint8_t piezoBuzzerPin = -1,                // Piezo buzzer pin, else -1
                uint32_t eepromDeviceSize = 0,              // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
                uint8_t sdCardCSPin = -1,                   // SD card CS pin, else -1
                uint8_t *ctrlInputPinMap = nullptr,         // Control input pin map, else nullptr
                uint8_t eepromI2CAddress = B000,            // EEPROM address
                uint8_t rtcI2CAddress = B000,               // RTC i2c address (only B000 can be used atm)
                uint8_t lcdI2CAddress = B000,               // LCD i2c address
                TwoWire &i2cWire = Wire,                    // I2C wire class instance
                uint32_t i2cSpeed = 400000U,                // I2C speed, in Hz
                uint32_t sdCardSpeed = 4000000U,            // SD card SPI speed, in Hz (ignored if on Teensy)
                WiFiClass &wifi = WiFi);                    // WiFi class instance
    // Library destructor. Just in case.
    ~Hydroponics();

    // Initializes default empty system. Typically called near top of setup().
    // See individual enums for more info.
    void init(Hydroponics_SystemMode systemMode = Hydroponics_SystemMode_Recycling,                 // What system of crop feeding is performed
              Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Default,        // What units of measurement should be used
              Hydroponics_DisplayOutputMode dispOutMode = Hydroponics_DisplayOutputMode_Disabled,   // What display output mode should be used
              Hydroponics_ControlInputMode ctrlInMode = Hydroponics_ControlInputMode_Disabled);     // What control input mode should be used

    // Initializes system from EEPROM save, returning success flag (set system data address with setSystemEEPROMAddress)
    bool initFromEEPROM(bool jsonFormat = false);
    // Initializes system from SD card file save, returning success flag (set config file name with setSystemConfigFile)
    bool initFromSDCard(bool jsonFormat = true);
    // Initializes system from custom JSON-based stream, returning success flag
    bool initFromJSONStream(Stream *streamIn);
    // Initializes system from custom binary stream, returning success flag
    bool initFromBinaryStream(Stream *streamIn);
    // TODO: Network URL init
    //bool initFromNetworkURL(urlDataTODO);

    // Saves current system setup to EEPROM save, returning success flag (set system data address with setSystemEEPROMAddress)
    bool saveToEEPROM(bool jsonFormat = false);
    // Saves current system setup to SD card file save, returning success flag (set config file name with setSystemConfigFile)
    bool saveToSDCard(bool jsonFormat = true);
    // Saves current system setup to custom JSON-based stream, returning success flag
    bool saveToJSONStream(Stream *streamOut, bool compact = true);
    // Saves current system setup to custom binary stream, returning success flag
    bool saveToBinaryStream(Stream *streamOut);
    // TODO: Network URL save
    //bool saveToNetworkURL(urlDataTODO);

    // System Operation.

    // Launches system into operational mode. Typically called near end of setup().
    void launch();

    // Suspends the system from operational mode (disables all runloops). Typically used during system setup UI.
    // Resume operation by a call to launch().
    void suspend();

    // Update method. Typically called in loop().
    void update();

    // System Logging.

    // Enables data logging to the SD card. Log file names will append YYMMDD.txt to the specified prefix. Returns success flag.
    inline bool enableSysLoggingToSDCard(String logFilePrefix) { return logger.beginLoggingToSDCard(logFilePrefix); }
    // TODO: Network URL sys logging
    //bool enableSysLoggingToNetworkURL(urlDataTODO, String logFilePrefix);

    // Data Publishing.

    // Enables data publishing to the SD card. Log file names will append YYMMDD.csv to the specified prefix. Returns success flag.
    inline bool enableDataPublishingToSDCard(String dataFilePrefix) { return publisher.beginPublishingToSDCard(dataFilePrefix); }
    // TODO: Network URL data pub
    //bool enableDataPublishingToNetworkURL(urlDataTODO, String dataFilePrefix);
    // TODO: MQTT data pub
    //bool enableDataPublishingToMQTT(mqttBrokerTODO, deviceDataTODO);
    // TODO: Web API data pub
    //bool enableDataPublishingToWebAPI(urlDataTODO, apiInterfaceTODO);

    // Object Registration.

    // Adds object to system, returning success
    bool registerObject(SharedPtr<HydroponicsObject> obj);
    // Removes object from system, returning success
    bool unregisterObject(SharedPtr<HydroponicsObject> obj);

    // Searches for object by id key (nullptr return = no obj by that id, position index may use HYDRUINO_POS_SEARCH* defines)
    SharedPtr<HydroponicsObject> objectById(HydroponicsIdentity id) const;

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
    bool tryGetPinLock(uint8_t pin, time_t waitMillis = 150);
    // Returns a locked pin lock for the given pin. Only call if pin lock was successfully locked.
    void returnPinLock(uint8_t pin);

    // Mutators.

    // Sets display name of system (HYDRUINO_NAME_MAXSIZE size limit)
    void setSystemName(String systemName);
    // Sets system time zone offset from UTC
    void setTimeZoneOffset(int8_t timeZoneOffset);
    // Sets system polling interval, in milliseconds (does not enable polling, see enable publishing methods)
    void setPollingInterval(uint16_t pollingInterval);
    // Sets system autosave enable mode and optional autosave interval, in minutes.
    void setAutosaveEnabled(Hydroponics_Autosave autosaveEnabled, uint16_t autosaveInterval = HYDRUINO_SYS_AUTOSAVE_INTERVAL);
    // Sets system config file as used in init and save by SD Card.
    inline void setSystemConfigFile(String configFileName) { _sysConfigFile = configFileName; }
    // Sets EEPROM system data address as used in init and save by EEPROM.
    inline void setSystemDataAddress(uint16_t sysDataAddress) { _sysDataAddress = sysDataAddress; }
    // Sets WiFi connection's SSID and password (note: password is stored encrypted, but is not hack-proof)
    void setWiFiConnection(String ssid, String password);

    // Sets the RTC's time to the passed time, with respect to set timezone. Will trigger significant time event.
    void setRealTimeClockTime(DateTime time);

    // Accessors.

    // Currently active Hydroponics instance
    static inline Hydroponics *getActiveInstance() { return _activeInstance; }
    // i2c Wire interface instance (available after constructor)
    inline TwoWire *getI2C() const { return _i2cWire; }
    // i2c clock speed, in Hz (default: 400kHz)
    inline uint32_t getI2CSpeed() const { return _i2cSpeed; }
    // SPI interface instance (hardwired, always available)
    inline SPIClass *getSPI() const { return &SPI; }
    // SD card SPI clock speed, in Hz (default: 4MHz, hardwired to 25MHz on Teensy)
    uint32_t getSDCardSpeed() const;
    // Total number of pins being used for the current control input ribbon mode
    int getControlInputRibbonPinCount() const;
    // Control input pin mapped to ribbon pin index, or -1 (255) if not used
    uint8_t getControlInputPin(int ribbonPinIndex) const;

    // EEPROM instance (lazily instantiated, nullptr return -> failure/no device)
    I2C_eeprom *getEEPROM(bool begin = true);
    // Real time clock instance (lazily instantiated, nullptr return -> failure/no device)
    RTC_DS3231 *getRealTimeClock(bool begin = true);
    // SD card instance (if began user code *must* call endSDCard(inst) to free interface, lazily instantiated, nullptr return -> failure/no device)
    SDClass *getSDCard(bool begin = true);
    // Ends SD card transaction with proper regards to platform
    void endSDCard(SDClass *sd);
    // WiFi instance (nullptr return -> failure/no device, note: this method may block for up to a minute)
    WiFiClass *getWiFi(bool begin = true);
    // OneWire instance for given pin (lazily instantiated)
    OneWire *getOneWireForPin(uint8_t pin);
    // Drops OneWire instance for given pin (if created)
    void dropOneWireForPin(uint8_t pin);

    // Whenever the system is in operational mode (has been launched), or not
    inline bool inOperationalMode() const { return !_suspend; }
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
    inline bool getRTCBatteryFailure() const { return _rtcBattFail; }
    // System sensor polling interval (time between sensor reads), in milliseconds (default: HYDRUINO_DATA_LOOP_INTERVAL)
    uint16_t getPollingInterval() const;
    // System polling frame number for sensor frame tracking
    inline uint16_t getPollingFrame() const { return _pollingFrame; }
    // Determines if a given frame # if out of date (true) or current (false), with optional frame # difference allowance
    bool isPollingFrameOld(unsigned int frame, unsigned int allowance = 0) const;
    // Returns if system autosaves are enabled or not
    bool isAutosaveEnabled() const;
    // System config file used in init and save by SD Card
    inline String getSystemConfigFile() const { return _sysConfigFile; }
    // System data address used in init and save by EEPROM
    inline uint16_t getSystemDataAddress() const { return _sysDataAddress; }
    // SSID for WiFi connection
    String getWiFiSSID();
    // Password for WiFi connection (plaintext)
    String getWiFiPassword();

    // Misc.

    // Called to notify system when RTC time is updated (also clears RTC battery failure flag)
    void notifyRTCTimeUpdated();

    // Called by scheduler to announce that a significant time event has occurred
    void notifyDayChanged();

protected:
    static Hydroponics *_activeInstance;                            // Current active instance (set after init, weak)
    HydroponicsSystemData *_systemData;                             // System data (owned, saved to storage)

    const uint32_t _eepromDeviceSize;                               // EEPROM device size (default: 0/Disabled)
    const uint8_t _piezoBuzzerPin;                                  // Piezo buzzer pin (default: Disabled)
    const uint8_t _sdCardCSPin;                                     // SD card cable select (CS) pin (default: Disabled)
    const uint8_t *_ctrlInputPinMap;                                // Control input pin mapping (weak, default: Disabled/nullptr)
    const uint8_t _eepromI2CAddr;                                   // EEPROM i2c address, format: {A2,A1,A0} (default: B000)
    const uint8_t _rtcI2CAddr;                                      // RTC i2c address, format: {A2,A1,A0} (default: B000, note: only B000 can be used atm)
    const uint8_t _lcdI2CAddr;                                      // LCD i2c address, format: {A2,A1,A0} (default: B000)
    TwoWire *_i2cWire;                                              // Controller's i2c wire class instance (strong, default: Wire)
    uint32_t _i2cSpeed;                                             // Controller's i2c clock speed (default: 400kHz)
    uint32_t _sdCardSpeed;                                          // SD card's SPI clock speed (default: 4MHz, ignored if on Teensy)
    WiFiClass *_wifi;                                               // WiFi class instance (strong, default: WiFi)
    I2C_eeprom *_eeprom;                                            // EEPROM instance (owned, lazy)
    RTC_DS3231 *_rtc;                                               // Real time clock instance (owned, lazy)
    SDClass *_sd;                                                   // SD card instance (owned/unowned, lazy)
    bool _eepromBegan;                                              // Status of EEPROM begin() call
    bool _rtcBegan;                                                 // Status of RTC begin() call
    bool _rtcBattFail;                                              // Status of RTC battery failure flag
    bool _wifiBegan;                                                // Status of WiFi begin() call

    #ifndef HYDRUINO_DISABLE_MULTITASKING
    taskid_t _controlTaskId;                                        // Control task Id if created, else TASKMGR_INVALIDID
    taskid_t _dataTaskId;                                           // Data polling task Id if created, else TASKMGR_INVALIDID
    taskid_t _miscTaskId;                                           // Misc task Id if created, else TASKMGR_INVALIDID
    #endif
    bool _suspend;                                                  // If system is currently suspended from operation
    uint16_t _pollingFrame;                                         // Current data polling frame # (index 0 reserved for disabled/undef, advanced by publisher)
    time_t _lastSpaceCheck;                                         // Last date storage media free space was checked, if able (UTC)
    time_t _lastAutosave;                                           // Last date autosave was performed, if able (UTC)
    String _sysConfigFile;                                          // SD Card system config filename used in serialization (default: "hydruino.cfg")
    uint16_t _sysDataAddress;                                       // EEPROM system data address used in serialization (default: -1/disabled)

    Map<Hydroponics_KeyType, SharedPtr<HydroponicsObject>, HYDRUINO_OBJ_LINKS_MAXSIZE> _objects; // Shared object collection, key'ed by HydroponicsIdentity
    Map<Hydroponics_ReservoirType, HydroponicsCustomAdditiveData *, Hydroponics_ReservoirType_CustomAdditiveCount> _additives; // Custom additives data
    Map<uint8_t, OneWire *, HYDRUINO_SYS_ONEWIRE_MAXSIZE> _oneWires; // pin->OneWire mapping
    Map<uint8_t, uint8_t, HYDRUINO_SYS_PINLOCKS_MAXSIZE> _pinLocks; // Pin locks mapping (existence = locked)

    friend Hydroponics *::getHydroponicsInstance();
    friend HydroponicsScheduler *::getSchedulerInstance();
    friend HydroponicsLogger *::getLoggerInstance();
    friend HydroponicsPublisher *::getPublisherInstance();
    friend class HydroponicsScheduler;
    friend class HydroponicsLogger;
    friend class HydroponicsPublisher;

    void allocateEEPROM();
    void deallocateEEPROM();
    void allocateRTC();
    void deallocateRTC();
    void allocateSD();
    void deallocateSD();

    void commonPreInit();
    void commonPostInit();
    void commonPostSave();

    SharedPtr<HydroponicsObject> objectById_Col(const HydroponicsIdentity &id) const;
    friend SharedPtr<HydroponicsObjInterface> HydroponicsDLinkObject::_getObject();
    friend void controlLoop();
    friend void dataLoop();
    friend void miscLoop();
    friend void handleInterrupt(uint8_t pin);

    void checkFreeMemory();
    void broadcastLowMemory();
    void checkFreeSpace();
    void checkAutosave();
};

// Template implementations
#include "Hydroponics.hpp"
#include "HydroponicsAttachments.hpp"
#include "HydroponicsUtils.hpp"

#endif // /ifndef Hydroponics_H
