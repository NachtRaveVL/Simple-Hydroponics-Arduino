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

    Simple-Hydroponics-Arduino - Version 0.5
*/

#ifndef Hydroponics_H
#define Hydroponics_H

// Library Setup

// NOTE: It is recommended to use custom build flags instead of editing this file directly.

// Uncomment or -D this define to completely disable usage of any multitasking commands and libraries. Not recommended.
//#define HYDRUINO_DISABLE_MULTITASKING             // https://github.com/davetcc/TaskManagerIO

// Uncomment or -D this define to disable usage of tcMenu library, which will disable all GUI control. Not recommended.
//#define HYDRUINO_DISABLE_GUI                      // https://github.com/davetcc/tcMenu

// Uncomment or -D this define to enable usage of the platform WiFi library, which enables networking capabilities.
//#define HYDRUINO_ENABLE_WIFI                      // Library used depends on your device architecture.

// Uncomment or -D this define to enable usage of the external serial ESP AT WiFi library, which enables networking capabilities.
//#define HYDRUINO_ENABLE_ESP_WIFI                  // https://github.com/jandrassy/WiFiEspAT

// Uncomment or -D this define to enable usage of the Arduino MQTT library, which enables IoT data publishing capabilities.
//#define HYDRUINO_ENABLE_MQTT                      // https://github.com/256dpi/arduino-mqtt

// Uncomment or -D this define to enable usage of SD card based virtual memory, which extends available RAM.
//#define HYDRUINO_ENABLE_SD_VIRTMEM                // https://github.com/NachtRaveVL/virtmem-continued

// Uncomment or -D this define to enable usage of SPI RAM based virtual memory, which extends available RAM.
//#define HYDRUINO_ENABLE_SPIRAM_VIRTMEM            // https://github.com/NachtRaveVL/virtmem-continued

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
#else
#include <WProgram.h>
#endif
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

#ifdef HYDRUINO_ENABLE_WIFI
#if defined(ARDUINO_SAMD_MKR1000)
#include <WiFi101.h>                                // https://github.com/arduino-libraries/WiFi101
#else
#include <WiFiNINA_Generic.h>                       // https://github.com/khoih-prog/WiFiNINA_Generic
#define HYDRUINO_USE_WIFI_STORAGE
#endif
#define HYDRUINO_USE_WIFI
#endif
#ifdef HYDRUINO_ENABLE_ESP_WIFI
#define HYDRUINO_USE_WIFI
#endif

#ifdef ESP32
typedef SDFileSystemClass SDClass;
#endif
#ifdef ESP8266
using namespace sdfat;
#endif

#ifdef HYDRUINO_DISABLE_MULTITASKING
#ifndef HYDRUINO_DISABLE_GUI
#define HYDRUINO_DISABLE_GUI
#endif
#define secondsToMillis(val) ((val)*1000U)
#if defined(ARDUINO_ARCH_MBED)
typedef uint32_t pintype_t;
#else
typedef uint8_t pintype_t;
#endif
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
#ifdef HYDRUINO_ENABLE_MQTT
#include "MQTT.h"                       // MQTT library
#endif
#if defined(ARDUINO_ARCH_STM32) && 1
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
#if defined(HYDRUINO_ENABLE_SD_VIRTMEM) || defined(HYDRUINO_ENABLE_SPIRAM_VIRTMEM)
#include "virtmem-continued.h"          // Note: Original library is no longer maintained, use our fork
#if defined(HYDRUINO_ENABLE_SD_VIRTMEM)
#include "alloc/sd_alloc.h"             // Note: If building fails, verify original library is not present
#elif defined(HYDRUINO_ENABLE_SPIRAM_VIRTMEM)
#include "alloc/spiram_alloc.h"         // Note: If building fails, verify original library is not present
#endif
#endif
#ifdef HYDRUINO_ENABLE_ESP_WIFI
#include "WiFiEspAT.h"                  // WiFi ESP AT library
#endif

#include "HydroponicsDefines.h"

#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L // Have libstdc++11
#include "ArxSmartPtr/shared_ptr.h"     // Forced shared pointer library
using namespace std;
template<typename T, size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE> using Vector = std::vector<T>;
template<class T1, class T2> using Pair = std::pair<T1,T2>;
template<typename K, typename V, size_t N = HYDRUINO_OBJ_LINKSFILTER_DEFSIZE> using Map = std::map<K,V>;
#else
using namespace arx;
template<typename T, size_t N = ARX_VECTOR_DEFAULT_SIZE> using Vector = arx::vector<T,N>;
template<class T1, class T2> using Pair = arx::pair<T1,T2>;
template<typename K, typename V, size_t N = ARX_MAP_DEFAULT_SIZE> using Map = arx::map<K,V,N>;
#endif
using namespace arx::stdx;

#if defined(HYDRUINO_ENABLE_SD_VIRTMEM) || defined(HYDRUINO_ENABLE_SPIRAM_VIRTMEM)
using namespace virtmem;
#define HYDRUINO_USE_VIRTMEM
#endif
#if defined(HYDRUINO_ENABLE_SD_VIRTMEM)
template <typename T> using VirtualPtr = VPtr<T, SDVAlloc>;
template <typename T> using SharedPtr = arx::stdx::shared_ptr<VirtualPtr<T>>;
#elif defined(HYDRUINO_ENABLE_SPIRAM_VIRTMEM)
template <typename T> using VirtualPtr = VPtr<T, SPIRAMVAlloc>;
template <typename T> using SharedPtr = arx::stdx::shared_ptr<VirtualPtr<T>>;
#else
template <typename T> using SharedPtr = arx::stdx::shared_ptr<T>;
#endif

extern time_t unixNow();
extern void handleInterrupt(pintype_t pin);
extern void controlLoop();
extern void dataLoop();
extern void miscLoop();

#include "HydroponicsStrings.h"
#include "HydroponicsSharedVirtualPtr.hh"
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
#include "HydroponicsAdditivesMarket.h"
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
    Hydroponics(pintype_t piezoBuzzerPin = -1,              // Piezo buzzer pin, else -1
                uint32_t eepromDeviceSize = 0,              // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
                uint8_t eepromI2CAddress = B000,            // EEPROM i2c address
                uint8_t rtcI2CAddress = B000,               // RTC i2c address (only B000 can be used atm)
                pintype_t sdCardCSPin = -1,                 // SD card CS pin, else -1
                uint32_t sdCardSpeed = F_SPD,               // SD card SPI speed, in Hz (ignored on Teensy)
#ifdef HYDRUINO_ENABLE_SPIRAM_VIRTMEM
                uint32_t spiRAMDeviceSize = 0,              // SPI RAM device size, else 0
                pintype_t spiRAMCSPin = -1,                 // SPI RAM CS pin, else -1
                uint32_t spiRAMSpeed = F_SPD,               // SPI RAM SPI speed, in Hz
#endif
                pintype_t *ctrlInputPinMap = nullptr,       // Control input pin map, else nullptr
                uint8_t lcdI2CAddress = B000,               // LCD i2c address
#if (!defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TWOWIRE)) || defined(Wire)
                TwoWire &i2cWire = Wire,                    // I2C wire class instance
#else
                TwoWire &i2cWire = new TwoWire(),           // I2C wire class instance
#endif
                uint32_t i2cSpeed = 400000U);               // I2C speed, in Hz
    // Library destructor. Just in case.
    ~Hydroponics();

    // Initializes default empty system. Typically called near top of setup().
    // See individual enums for more info.
    void init(Hydroponics_SystemMode systemMode = Hydroponics_SystemMode_Recycling,                 // What system of crop feeding is performed
              Hydroponics_MeasurementMode measureMode = Hydroponics_MeasurementMode_Default,        // What units of measurement should be used
              Hydroponics_DisplayOutputMode dispOutMode = Hydroponics_DisplayOutputMode_Disabled,   // What display output mode should be used
              Hydroponics_ControlInputMode ctrlInMode = Hydroponics_ControlInputMode_Disabled);     // What control input mode should be used

    // Initializes system from EEPROM save, returning success flag
    // Set system data address with setSystemEEPROMAddress
    bool initFromEEPROM(bool jsonFormat = false);
    // Initializes system from SD card file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool initFromSDCard(bool jsonFormat = true);
#ifdef HYDRUINO_USE_WIFI_STORAGE
    // Initializes system from a WiFiStorage file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool initFromWiFiStorage(bool jsonFormat = true);
#endif
    // Initializes system from custom JSON-based stream, returning success flag
    bool initFromJSONStream(Stream *streamIn);
    // Initializes system from custom binary stream, returning success flag
    bool initFromBinaryStream(Stream *streamIn);

    // Saves current system setup to EEPROM save, returning success flag
    // Set system data address with setSystemEEPROMAddress
    bool saveToEEPROM(bool jsonFormat = false);
    // Saves current system setup to SD card file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool saveToSDCard(bool jsonFormat = true);
#ifdef HYDRUINO_USE_WIFI_STORAGE
    // Saves current system setup to WiFiStorage file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool saveToWiFiStorage(bool jsonFormat = true);
#endif
    // Saves current system setup to custom JSON-based stream, returning success flag
    bool saveToJSONStream(Stream *streamOut, bool compact = true);
    // Saves current system setup to custom binary stream, returning success flag
    bool saveToBinaryStream(Stream *streamOut);

    // System Operation.

    // Launches system into operational mode. Typically called near end of setup().
    void launch();

    // Suspends the system from operational mode (disables all runloops). Typically used during system setup UI.
    // Resume operation by a call to launch().
    void suspend();

    // Update method. Typically called in loop().
    void update();

    // System Logging.

    // Enables system logging to the SD card. Log file names will append YYMMDD.txt to the specified prefix. Returns success flag.
    inline bool enableSysLoggingToSDCard(String logFilePrefix) { return logger.beginLoggingToSDCard(logFilePrefix); }
#ifdef HYDRUINO_USE_WIFI_STORAGE
    // Enables system logging to WiFiStorage. Log file names will append YYMMDD.txt to the specified prefix. Returns success flag.
    inline bool enableSysLoggingToWiFiStorage(String logFilePrefix) { return logger.beginLoggingToWiFiStorage(logFilePrefix); }
#endif

    // Data Publishing.

    // Enables data publishing to the SD card. Data file names will append YYMMDD.csv to the specified prefix. Returns success flag.
    inline bool enableDataPublishingToSDCard(String dataFilePrefix) { return publisher.beginPublishingToSDCard(dataFilePrefix); }
#ifdef HYDRUINO_USE_WIFI_STORAGE
    // Enables data publishing to WiFiStorage. Data file names will append YYMMDD.csv to the specified prefix. Returns success flag.
    inline bool enableDataPublishingToWiFiStorage(String dataFilePrefix) { return publisher.beginPublishingToWiFiStorage(dataFilePrefix); }
#endif
#ifdef HYDRUINO_ENABLE_MQTT
    // Enables data publishing to MQTT broker. Client is expected to be began/connected (with proper broker address/net client) *before* calling this method. Returns success flag.
    inline bool enableDataPublishingToMQTTClient(MQTTClient &client) { return publisher.beginPublishingToMQTTClient(client); }
#endif

    // User Interface.

#ifndef HYDRUINO_DISABLE_GUI
    // Enables UI to run with passed instance.
    // Minimal UI only allows the user to edit existing objects, not create nor delete them.
    // Full UI allows the user to add/remove system objects, customize features, change settings, etc.
    // Note: Be sure to manually include the appropriate UI system header file (e.g. #include "min/HydroponicsUI.h") in Arduino sketch.
    inline bool enableUI(HydroponicsUIInterface *ui) { _activeUIInstance = ui; ui->begin(); }
#endif

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

    // Pin Locks.

    // Attempts to get a lock on pin #, to prevent multi-device comm overlap (e.g. for OneWire comms).
    bool tryGetPinLock(pintype_t pin, time_t waitMillis = 150);
    // Returns a locked pin lock for the given pin. Only call if pin lock was successfully locked.
    inline void returnPinLock(pintype_t pin);

    // Mutators.

    // Sets display name of system (HYDRUINO_NAME_MAXSIZE size limit)
    void setSystemName(String systemName);
    // Sets system time zone offset from UTC
    void setTimeZoneOffset(int8_t timeZoneOffset);
    // Sets system polling interval, in milliseconds (does not enable polling, see enable publishing methods)
    void setPollingInterval(uint16_t pollingInterval);
    // Sets system autosave enable mode and optional fallback mode and interval, in minutes.
    void setAutosaveEnabled(Hydroponics_Autosave autosaveEnabled, Hydroponics_Autosave autosaveFallback = Hydroponics_Autosave_Disabled, uint16_t autosaveInterval = HYDRUINO_SYS_AUTOSAVE_INTERVAL);
    // Sets system config file as used in init and save by SD Card.
    inline void setSystemConfigFilename(String configFilename) { _sysConfigFilename = configFilename; }
    // Sets EEPROM system data address as used in init and save by EEPROM.
    inline void setSystemDataAddress(uint16_t sysDataAddress) { _sysDataAddress = sysDataAddress; }
#ifdef HYDRUINO_USE_WIFI
    // Sets WiFi connection's SSID/pass combo (note: password is stored encrypted, but is not hack-proof)
    void setWiFiConnection(String ssid, String pass);
#endif

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
    // EEPROM device size, in bytes (default: Disabled)
    inline uint32_t getEEPROMDeviceSize() const { return _eepromDeviceSize; }
#ifdef HYDRUINO_ENABLE_SD_VIRTMEM
    // SD card cable-select (CS) pin (default: Disabled)
    inline pintype_t getSDCardCSPin() const { return _vAlloc.getSDCSPin(); }
    // SD card SPI clock speed, in Hz (default: same as CPU (/0 divider), hardwired to 25MHz on Teensy)
    inline uint32_t getSDCardSpeed() const { return _vAlloc.getSDSpeed(); }
#else
    // SD card cable-select (CS) pin (default: Disabled)
    inline pintype_t getSDCardCSPin() const { return _sdCardCSPin; }
    // SD card SPI clock speed, in Hz (default: same as CPU (/0 divider), hardwired to 25MHz on Teensy)
    inline uint32_t getSDCardSpeed() const { return _sdCardSpeed; }
#ifdef HYDRUINO_ENABLE_SPIRAM_VIRTMEM
    // SPI RAM device size, in bytes (default: Disabled)
    inline uint32_t getSPIRAMDeviceSize() const { return _vAlloc.getPoolSize(); }
    // SPI RAM cable-select (CS) pin (default: Disabled)
    inline pintype_t getSPIRAMCSPin() const { return _vAlloc.getSRAMCSPin(); }
#ifdef VIRTMEM_SPIRAM_CAPTURESPEED
    // SPI RAM SPI clock speed, in Hz (default: same as CPU (/0 divider))
    inline uint32_t getSPIRAMSpeed() const { return _vAlloc.getSRAMSpeed(); }
#else
    // SPI RAM SPI clock speed, in Hz (default: same as CPU (/0 divider))
    inline uint32_t getSPIRAMSpeed() const { return _spiRAMSpeed; }
#endif
#endif // /ifdef HYDRUINO_ENABLE_SPIRAM_VIRTMEM
#endif // /ifdef HYDRUINO_ENABLE_SD_VIRTMEM
    // Total number of pins being used for the current control input ribbon mode
    int getControlInputRibbonPinCount() const;
    // Control input pin mapped to ribbon pin index, or -1 (255) if not used
    pintype_t getControlInputPin(int ribbonPinIndex) const;

    // EEPROM instance (lazily instantiated, nullptr return -> failure/no device)
    I2C_eeprom *getEEPROM(bool begin = true);
    // Real time clock instance (lazily instantiated, nullptr return -> failure/no device)
    RTC_DS3231 *getRealTimeClock(bool begin = true);
    // SD card instance (user code *must* call endSDCard(inst) to return interface, lazily instantiated, nullptr return -> failure/no device)
    SDClass *getSDCard(bool begin = true);
    // Ends SD card transaction with proper regards to platform once all instances returned (note: some instancing may be expected to never return)
    void endSDCard(SDClass *sd = nullptr);
#ifdef HYDRUINO_USE_WIFI
    // WiFi instance (nullptr return -> failure/no device, note: this method may block for up to a minute)
    inline WiFiClass *getWiFi(bool begin = true);
    // WiFi instance with fallback ssid/pass combo (nullptr return -> failure/no device, note: this method may block for up to a minute)
    WiFiClass *getWiFi(String ssid, String pass, bool begin = true);
#endif
    // OneWire instance for given pin (lazily instantiated)
    OneWire *getOneWireForPin(pintype_t pin);
    // Drops OneWire instance for given pin (if created)
    void dropOneWireForPin(pintype_t pin);

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
    // Returns if system fallback autosaves are enabled or not
    bool isAutosaveFallbackEnabled() const;
    // System config file used in init and save by SD Card
    inline String getSystemConfigFile() const { return _sysConfigFilename; }
    // System data address used in init and save by EEPROM
    inline uint16_t getSystemDataAddress() const { return _sysDataAddress; }
#ifdef HYDRUINO_USE_WIFI
    // SSID for WiFi connection
    String getWiFiSSID();
    // Password for WiFi connection (plaintext)
    String getWiFiPassword();
#endif

    // Misc.

    // Called to notify system when RTC time is updated (also clears RTC battery failure flag)
    void notifyRTCTimeUpdated();

    // Called by scheduler to announce that a significant time event has occurred
    void notifyDayChanged();

protected:
    static Hydroponics *_activeInstance;                            // Current active instance (set after init, weak)
#ifndef HYDRUINO_DISABLE_GUI
    HydroponicsUIInterface *_activeUIInstance;                      // Current active UI instance (owned)
#endif
    HydroponicsSystemData *_systemData;                             // System data (owned, saved to storage)

    const pintype_t _piezoBuzzerPin;                                // Piezo buzzer pin (default: Disabled)
    const uint32_t _eepromDeviceSize;                               // EEPROM device size (default: 0/Disabled)
    const uint8_t _eepromI2CAddr;                                   // EEPROM i2c address, format: {A2,A1,A0} (default: B000)
    const uint8_t _rtcI2CAddr;                                      // RTC i2c address, format: {A2,A1,A0} (default: B000, note: only B000 can be used atm)
#ifndef HYDRUINO_ENABLE_SD_VIRTMEM
    const pintype_t _sdCardCSPin;                                   // SD card cable select (CS) pin (default: Disabled)
    const uint32_t _sdCardSpeed;                                    // SD card's SPI clock speed (default: same as CPU (/0 divider), ignored on Teensy)
#endif
    const pintype_t *_ctrlInputPinMap;                              // Control input pin mapping (weak, default: Disabled/nullptr)
    const uint8_t _lcdI2CAddr;                                      // LCD i2c address, format: {A2,A1,A0} (default: B000)
    TwoWire *_i2cWire;                                              // Controller's i2c wire class instance (strong, default: Wire)
    const uint32_t _i2cSpeed;                                       // Controller's i2c clock speed (default: 400kHz)
#if defined(HYDRUINO_ENABLE_SPIRAM_VIRTMEM) && !defined(VIRTMEM_SPIRAM_CAPTURESPEED)
    const uint32_t _spiRAMSpeed;                                    // SPI RAM's SPI clock speed (default: same as CPU (/0 divider))
#endif
    I2C_eeprom *_eeprom;                                            // EEPROM instance (owned, lazy)
    RTC_DS3231 *_rtc;                                               // Real time clock instance (owned, lazy)
    SDClass *_sd;                                                   // SD card instance (owned/unowned, lazy)
    int8_t _sdOut;                                                  // Number of SD card instances out
#if defined(HYDRUINO_ENABLE_SD_VIRTMEM)
    SDVAlloc _vAlloc;                                               // SD card virtual memory allocator
#elif defined(HYDRUINO_ENABLE_SPIRAM_VIRTMEM)
    SPIRAMVAlloc _vAlloc;                                           // SPI ram virtual memory allocator
#endif
    bool _eepromBegan;                                              // Status of EEPROM begin() call
    bool _rtcBegan;                                                 // Status of RTC begin() call
    bool _rtcBattFail;                                              // Status of RTC battery failure flag
    bool _sdBegan;                                                  // Status of SD begin() call
#ifdef HYDRUINO_USE_WIFI
    bool _wifiBegan;                                                // Status of WiFi begin() call
#endif

#ifndef HYDRUINO_DISABLE_MULTITASKING
    taskid_t _controlTaskId;                                        // Control task Id if created, else TASKMGR_INVALIDID
    taskid_t _dataTaskId;                                           // Data polling task Id if created, else TASKMGR_INVALIDID
    taskid_t _miscTaskId;                                           // Misc task Id if created, else TASKMGR_INVALIDID
#endif
    bool _suspend;                                                  // If system is currently suspended from operation
    uint16_t _pollingFrame;                                         // Current data polling frame # (index 0 reserved for disabled/undef, advanced by publisher)
    time_t _lastSpaceCheck;                                         // Last date storage media free space was checked, if able (UTC)
    time_t _lastAutosave;                                           // Last date autosave was performed, if able (UTC)
    String _sysConfigFilename;                                      // System config filename used in serialization (default: "hydruino.cfg")
    uint16_t _sysDataAddress;                                       // EEPROM system data address used in serialization (default: -1/disabled)

    Map<Hydroponics_KeyType, SharedPtr<HydroponicsObject>, HYDRUINO_SYS_OBJECTS_MAXSIZE> _objects; // Shared object collection, key'ed by HydroponicsIdentity
    Map<pintype_t, OneWire *, HYDRUINO_SYS_ONEWIRE_MAXSIZE> _oneWires; // pin->OneWire mapping
    Map<pintype_t, pintype_t, HYDRUINO_SYS_PINLOCKS_MAXSIZE> _pinLocks; // Pin locks mapping (existence = locked)

    friend Hydroponics *::getHydroponicsInstance();
    friend HydroponicsScheduler *::getSchedulerInstance();
    friend HydroponicsLogger *::getLoggerInstance();
    friend HydroponicsPublisher *::getPublisherInstance();
#ifdef HYDRUINO_USE_VIRTMEM
    friend BaseVAlloc *::getVirtualAllocator();
#endif
#ifndef HYDRUINO_DISABLE_GUI
    friend HydroponicsUIInterface *::getUIInstance();
#endif
    friend class HydroponicsCalibrationsStore;
    friend class HydroponicsCropsLibrary;
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
    friend void handleInterrupt(pintype_t pin);

    void checkFreeMemory();
    void broadcastLowMemory();
    void checkFreeSpace();
    void checkAutosave();
};

// Template implementations
#include "HydroponicsInterfaces.hpp"
#include "Hydroponics.hpp"
#include "HydroponicsAttachments.hpp"
#include "HydroponicsUtils.hpp"

#endif // /ifndef Hydroponics_H
