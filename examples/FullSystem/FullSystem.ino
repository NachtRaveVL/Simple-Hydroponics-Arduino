// Simple-Hydroponics-Arduino Full System
// This sketch will build the entire library onto a device, while supporting all of its
// functionality, and thus has the highest cost. Not meant for constrained devices.
//
// TODO: STILL A WIP!

#include <Hydroponics.h>
#include "full/HydroponicsUI.h"

// Compiler flag checks
#ifdef HYDRUINO_DISABLE_GUI
#error The HYDRUINO_DISABLE_GUI flag is expected to be undefined in order to run this sketch
#endif

// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_SIZE        0               // EEPROM bit storage size, in bytes (use I2C_DEVICESIZE_* defines), else 0
#define SETUP_EEPROM_I2C_ADDR           B000            // EEPROM address
#define SETUP_RTC_I2C_ADDR              B000            // RTC i2c address (only B000 can be used atm)
#define SETUP_SD_CARD_CS_PIN            -1              // SD card CS pin, else -1
#define SETUP_SD_CARD_SPI_SPEED         F_SPD           // SD card SPI speed, in Hz (ignored on Teensy)
#define SETUP_SPIRAM_DEVICE_SIZE        0               // SPI RAM device size, in bytes, else 0 (note: define HYDRUINO_ENABLE_SPIRAM_VIRTMEM to enable SPIRAM)
#define SETUP_SPIRAM_CS_PIN             -1              // SPI RAM CS pin, else -1
#define SETUP_SPIRAM_SPI_SPEED          F_SPD           // SPI RAM SPI speed, in Hz
#define SETUP_LCD_I2C_ADDR              B000            // LCD i2c address
#define SETUP_CTRL_INPUT_PINS           {-1}            // Control input pin ribbon, else {-1}
#define SETUP_I2C_WIRE_INST             Wire            // I2C wire class instance
#define SETUP_I2C_SPEED                 400000U         // I2C speed, in Hz
#define SETUP_ESP_I2C_SDA               SDA             // I2C SDA pin, if on ESP
#define SETUP_ESP_I2C_SCL               SCL             // I2C SCL pin, if on ESP

// WiFi Settings                                        (note: define HYDRUINO_ENABLE_WIFI or HYDRUINO_ENABLE_ESP_WIFI to enable WiFi)
#define SETUP_WIFI_SSID                 "CHANGE_ME"     // WiFi SSID
#define SETUP_WIFI_PASS                 "CHANGE_ME"     // WiFi passphrase

// System Settings
#define SETUP_SYSTEM_MODE               Recycling       // System run mode (Recycling, DrainToWaste)
#define SETUP_MEASURE_MODE              Default         // System measurement mode (Default, Imperial, Metric, Scientific)
#define SETUP_LCD_OUT_MODE              Disabled        // System LCD output mode (Disabled, 20x4LCD, 20x4LCD_Swapped, 16x2LCD, 16x2LCD_Swapped)
#define SETUP_CTRL_IN_MODE              Disabled        // System control input mode (Disabled, 2x2Matrix, 4xButton, 6xButton, RotaryEncoder)
#define SETUP_SYS_NAME                  "Hydruino"      // System name
#define SETUP_SYS_TIMEZONE              +0              // System timezone offset
#define SETUP_SYS_LOGLEVEL              All             // System log level filter (All, Warnings, Errors, None)

// System Saves Settings                                (note: only one primary and one fallback mechanism may be enabled at a time)
#define SETUP_SAVES_CONFIG_FILE         "hydruino.cfg"  // System config file name for system saves
#define SETUP_SAVES_SD_CARD_MODE        Disabled        // If saving/loading from SD card is enable (Primary, Fallback, Disabled)
#define SETUP_SAVES_EEPROM_MODE         Disabled        // If saving/loading from EEPROM is enabled (Primary, Fallback, Disabled)
#define SETUP_SAVES_WIFISTORAGE_MODE    Disabled        // If saving/loading from WiFiStorage (OS/OTA filesystem / WiFiNINA_Generic only only) is enabled (Primary, Fallback, Disabled)

// Logging & Data Publishing Settings
#define SETUP_LOG_FILE_PREFIX           "logs/hy"       // System logs file prefix (appended with YYMMDD.txt)
#define SETUP_DATA_FILE_PREFIX          "data/hy"       // System data publishing files prefix (appended with YYMMDD.csv)
#define SETUP_DATA_SD_ENABLE            false           // If system data publishing is enabled to SD card
#define SETUP_LOG_SD_ENABLE             false           // If system logging is enabled to SD card
#define SETUP_DATA_WIFISTORAGE_ENABLE   false           // If system data publishing is enabled to WiFiStorage (OS/OTA filesystem / WiFiNINA_Generic only only)
#define SETUP_LOG_WIFISTORAGE_ENABLE    false           // If system logging is enabled to WiFiStorage (OS/OTA filesystem / WiFiNINA_Generic only only)

// External Data Settings
#define SETUP_EXTDATA_SD_ENABLE         false           // If data should be read from an external SD card (searched first for crops lib data)
#define SETUP_EXTDATA_SD_LIB_PREFIX     "lib/"          // Library data folder/data file prefix (appended with {type}##.dat)
#define SETUP_EXTDATA_EEPROM_ENABLE     false           // If data should be read from an external EEPROM (searched first for strings data)

// External EEPROM Settings
#define SETUP_EEPROM_SYSDATA_ADDR       0x2e50          // System data memory offset for EEPROM saves (from Data Writer output)
#define SETUP_EEPROM_CROPSLIB_ADDR      0x0000          // Start address for Crops Library data (from Data Writer output)
#define SETUP_EEPROM_STRINGS_ADDR       0x1b24          // Start address for strings data (from Data Writer output)


#if defined(HYDRUINO_ENABLE_ESP_WIFI) && !(defined(SERIAL_PORT_HARDWARE1) || defined(Serial1))
#include "SoftwareSerial.h"
SoftwareSerial Serial1(RX, TX);                         // Replace with Rx/Tx pins of your choice
#endif

// Pre-init checks
#if (SETUP_SAVES_WIFISTORAGE_MODE != Disabled || SETUP_DATA_WIFISTORAGE_ENABLE || SETUP_LOG_WIFISTORAGE_ENABLE) && !defined(HYDRUINO_USE_WIFI_STORAGE)
#warning The HYDRUINO_ENABLE_WIFI flag is expected to be defined as well as WiFiNINA_Generic.h included in order to run this sketch with WiFiStorage features enabled
#endif
#if (SETUP_SAVES_SD_CARD_MODE != Disabled || SETUP_DATA_SD_ENABLE || SETUP_LOG_SD_ENABLE || SETUP_EXTDATA_SD_ENABLE) && SETUP_SD_CARD_CS_PIN == -1
#warning The SETUP_SD_CARD_CS_PIN define is expected to be set to a valid pin in order to run this sketch with SD card features enabled
#endif
#if (SETUP_SAVES_EEPROM_MODE != Disabled || SETUP_EXTDATA_EEPROM_ENABLE) && SETUP_EEPROM_DEVICE_SIZE == 0
#warning The SETUP_EEPROM_DEVICE_SIZE define is expected to be set to a valid size in order to run this sketch with EEPROM features enabled
#endif

pintype_t _SETUP_CTRL_INPUT_PINS[] = SETUP_CTRL_INPUT_PINS;
Hydroponics hydroController(SETUP_PIEZO_BUZZER_PIN,
                            SETUP_EEPROM_DEVICE_SIZE,
                            SETUP_EEPROM_I2C_ADDR,
                            SETUP_RTC_I2C_ADDR,
                            SETUP_SD_CARD_CS_PIN,
                            SETUP_SD_CARD_SPI_SPEED,
#ifdef HYDRUINO_ENABLE_SPIRAM_VIRTMEM
                            SETUP_SPIRAM_DEVICE_SIZE,
                            SETUP_SPIRAM_CS_PIN,
                            SETUP_SPIRAM_SPI_SPEED,
#endif
                            _SETUP_CTRL_INPUT_PINS,
                            SETUP_LCD_I2C_ADDR,
                            SETUP_I2C_WIRE_INST,
                            SETUP_I2C_SPEED);

void setup() {
    // Setup base interfaces
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        Serial.begin(115200);           // Begin USB Serial interface
        while (!Serial) { ; }           // Wait for USB Serial to connect
    #endif
    #if defined(ESP_PLATFORM)
        SETUP_I2C_WIRE_INST.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL); // Begin i2c Wire for ESP
    #endif
    #ifdef HYDRUINO_USE_WIFI
        String wifiSSID = F(SETUP_WIFI_SSID);
        String wifiPassword = F(SETUP_WIFI_PASS);
        #ifdef HYDRUINO_ENABLE_ESP_WIFI
            Serial1.begin(HYDRUINO_SYS_ESPWIFI_SERIALBAUD);
            WiFi.init(Serial1); // Change to Serial instance of your choice, otherwise
        #endif
    #endif

    // Begin external data storage devices for crop, strings, and other data.
    #if SETUP_EXTDATA_EEPROM_ENABLE
        beginStringsFromEEPROM(SETUP_EEPROM_STRINGS_ADDR);
        hydroCropsLib.beginCropsLibraryFromEEPROM(SETUP_EEPROM_CROPSLIB_ADDR);
    #endif
    #if SETUP_EXTDATA_SD_ENABLE
        beginStringsFromSDCard(String(F(SETUP_EXTDATA_SD_LIB_PREFIX)) + String(F("strings")));
        hydroCropsLib.beginCropsLibraryFromSDCard(String(F(SETUP_EXTDATA_SD_LIB_PREFIX)) + String(F("crop")));
    #endif

    // Sets system config name used in any of the following inits.
    #if (defined(HYDRUINO_USE_WIFI_STORAGE) && SETUP_SAVES_WIFISTORAGE_MODE != Disabled) || \
        (SETUP_SD_CARD_CS_PIN >= 0 && SETUP_SAVES_SD_CARD_MODE != Disabled)
        hydroController.setSystemConfigFilename(F(SETUP_SAVES_CONFIG_FILE));
    #endif
    // Sets the EEPROM memory address for system data.
    #if SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_MODE != Disabled
        hydroController.setSystemDataAddress(SETUP_EEPROM_SYSDATA_ADDR);
    #endif

    // Initializes controller with first initialization method that successfully returns.
    if (!(false
        #if defined(HYDRUINO_USE_WIFI_STORAGE) && SETUP_SAVES_WIFISTORAGE_MODE == Primary
            || hydroController.initFromWiFiStorage()
        #elif SETUP_SD_CARD_CS_PIN >= 0 && SETUP_SAVES_SD_CARD_MODE == Primary
            || hydroController.initFromSDCard()
        #elif SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_MODE == Primary
            || hydroController.initFromEEPROM()
        #endif
        #if defined(HYDRUINO_USE_WIFI_STORAGE) && SETUP_SAVES_WIFISTORAGE_MODE == Fallback
            || hydroController.initFromWiFiStorage()
        #elif SETUP_SD_CARD_CS_PIN >= 0 && SETUP_SAVES_SD_CARD_MODE == Fallback
            || hydroController.initFromSDCard()
        #elif SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_MODE == Fallback
            || hydroController.initFromEEPROM()
        #endif
        )) {
        // First time running controller, set up default initial empty environment.
        hydroController.init(JOIN(Hydroponics_SystemMode,SETUP_SYSTEM_MODE),
                             JOIN(Hydroponics_MeasurementMode,SETUP_MEASURE_MODE),
                             JOIN(Hydroponics_DisplayOutputMode,SETUP_LCD_OUT_MODE),
                             JOIN(Hydroponics_ControlInputMode,SETUP_CTRL_IN_MODE));

        // Set Settings
        hydroController.setSystemName(F(SETUP_SYS_NAME));
        hydroController.setTimeZoneOffset(SETUP_SYS_TIMEZONE);
        #ifdef HYDRUINO_USE_WIFI
            hydroController.setWiFiConnection(wifiSSID, wifiPassword); wifiSSID = wifiPassword = String();
        #endif
        getLoggerInstance()->setLogLevel(JOIN(Hydroponics_LogLevel,SETUP_SYS_LOGLEVEL));
        #if SETUP_LOG_SD_ENABLE
            hydroController.enableSysLoggingToSDCard(F(SETUP_LOG_FILE_PREFIX));
        #endif
        #if SETUP_DATA_SD_ENABLE
            hydroController.enableDataPublishingToSDCard(F(SETUP_DATA_FILE_PREFIX));
        #endif
        #if defined(HYDRUINO_USE_WIFI_STORAGE) && SETUP_LOG_WIFISTORAGE_ENABLE
            hydroController.enableSysLoggingToWiFiStorage(F(SETUP_LOG_FILE_PREFIX));
        #endif
        #if defined(HYDRUINO_USE_WIFI_STORAGE) && SETUP_DATA_WIFISTORAGE_ENABLE
            hydroController.enableDataPublishingToWiFiStorage(F(SETUP_DATA_FILE_PREFIX));
        #endif
        #if defined(HYDRUINO_USE_WIFI_STORAGE) && SETUP_SAVES_WIFISTORAGE_MODE == Primary
            hydroController.setAutosaveEnabled(Hydroponics_Autosave_EnabledToWiFiStorageJson
        #elif SETUP_SD_CARD_CS_PIN >= 0 && SETUP_SAVES_SD_CARD_MODE == Primary
            hydroController.setAutosaveEnabled(Hydroponics_Autosave_EnabledToSDCardJson
        #elif SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_MODE == Primary
            hydroController.setAutosaveEnabled(Hydroponics_Autosave_EnabledToEEPROMRaw
        #else
            hydroController.setAutosaveEnabled(Hydroponics_Autosave_Disabled
        #endif
        #if defined(HYDRUINO_USE_WIFI_STORAGE) && SETUP_SAVES_WIFISTORAGE_MODE == Fallback
            , Hydroponics_Autosave_EnabledToWiFiStorageJson);
        #elif SETUP_SD_CARD_CS_PIN >= 0 && SETUP_SAVES_SD_CARD_MODE == Fallback
            , Hydroponics_Autosave_EnabledToSDCardJson);
        #elif SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_MODE == Fallback
            , Hydroponics_Autosave_EnabledToEEPROMRaw);
        #else
            );
        #endif

        // No further setup is necessary, as system is assumed to be built/managed via UI.
    }

    #if !defined(HYDRUINO_DISABLE_GUI) && SETUP_LCD_OUT_MODE != Disabled
        hydroController.enableUI(new HydroponicsFullUI());
    #endif

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
