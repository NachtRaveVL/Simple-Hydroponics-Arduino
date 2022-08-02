// Simple-Hydroponics-Arduino Full System
// This sketch will build the entire library onto a device, while supporting all of its
// functionality, and thus has the highest cost. Not meant for constrained devices.
//
// TODO: STILL A WIP!

#include <Hydroponics.h>

// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_SIZE        0               // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
#define SETUP_SD_CARD_CS_PIN            -1              // SD card CS pin, else -1
#define SETUP_CTRL_INPUT_PINS           {-1}            // Control input pin ribbon, else {-1}
#define SETUP_EEPROM_I2C_ADDR           B000            // EEPROM address
#define SETUP_RTC_I2C_ADDR              B000            // RTC i2c address (only B000 can be used atm)
#define SETUP_LCD_I2C_ADDR              B000            // LCD i2c address
#define SETUP_I2C_WIRE_INST             Wire            // I2C wire class instance
#define SETUP_I2C_SPEED                 400000U         // I2C speed, in Hz
#define SETUP_ESP_I2C_SDA               SDA             // I2C SDA pin, if on ESP
#define SETUP_ESP_I2C_SCL               SCL             // I2C SCL pin, if on ESP
#define SETUP_SD_CARD_SPI_SPEED         4000000U        // SD card SPI speed, in Hz (ignored if on Teensy)
#define SETUP_WIFI_INST                 WiFi            // WiFi class instance

// System Settings
#define SETUP_SYSTEM_MODE               Recycling       // System run mode (Recycling, DrainToWaste)
#define SETUP_MEASURE_MODE              Default         // System measurement mode (Default, Imperial, Metric, Scientific)
#define SETUP_LCD_OUT_MODE              Disabled        // System LCD output mode (Disabled, 20x4LCD, 20x4LCD_Swapped, 16x2LCD, 16x2LCD_Swapped)
#define SETUP_CTRL_IN_MODE              Disabled        // System control input mode (Disabled, 2x2Matrix, 4xButton, 6xButton, RotaryEncoder)
#define SETUP_SYS_NAME                  "Hydruino"      // System name
#define SETUP_SYS_TIMEZONE              +0              // System timezone offset
#define SETUP_SYS_LOGLEVEL              All             // System log level filter (All, Warnings, Errors, None)

// System Saves Settings                                (note: only one save mechanism may be enabled at a time)
#define SETUP_SYS_AUTOSAVE_ENABLE       false           // If autosaving system out is enabled or not
#define SETUP_SAVES_SD_CARD_ENABLE      false           // If saving/loading from SD card is enable
#define SETUP_SD_CARD_CONFIG_FILE       "hydruino.cfg"  // System config file name for SD Card saves
#define SETUP_SAVES_EEPROM_ENABLE       false           // If saving/loading from EEPROM is enabled 

// WiFi Settings
#define SETUP_ENABLE_WIFI               false           // If WiFi is enabled
#define SETUP_WIFI_SSID                 "CHANGE_ME"     // WiFi SSID
#define SETUP_WIFI_PASS                 "CHANGE_ME"     // WiFi password

// Logging & Data Publishing Settings
#define SETUP_LOG_SD_ENABLE             false           // If system logging is enabled to SD card
#define SETUP_LOG_FILE_PREFIX           "logs/hy"       // System logs file prefix (appended with YYMMDD.txt)
#define SETUP_DATA_SD_ENABLE            false           // If system data publishing is enabled to SD card
#define SETUP_DATA_FILE_PREFIX          "data/hy"       // System data publishing files prefix (appended with YYMMDD.csv)

// External Data Settings
#define SETUP_EXTDATA_SD_ENABLE         false           // If data should be read from an external SD Card (searched first for crops lib data)
#define SETUP_EXTDATA_SD_LIB_PREFIX     "lib/"          // Library data folder/data file prefix (appended with {type}##.dat)
#define SETUP_EXTDATA_EEPROM_ENABLE     false           // If data should be read from an external EEPROM (searched first for strings data)

// External EEPROM Settings
#define SETUP_EEPROM_SYSDATA_ADDR       0x2e2d          // System data memory offset for EEPROM saves (from Data Writer output)
#define SETUP_EEPROM_CROPSLIB_ADDR      0x0000          // Start address for Crops Library data (from Data Writer output)
#define SETUP_EEPROM_STRINGS_ADDR       0x1b24          // Start address for Strings data (from Data Writer output)


byte _SETUP_CTRL_INPUT_PINS[] = SETUP_CTRL_INPUT_PINS;
Hydroponics hydroController(SETUP_PIEZO_BUZZER_PIN,
                            SETUP_EEPROM_DEVICE_SIZE,
                            SETUP_SD_CARD_CS_PIN,
                            _SETUP_CTRL_INPUT_PINS,
                            SETUP_EEPROM_I2C_ADDR,
                            SETUP_RTC_I2C_ADDR,
                            SETUP_LCD_I2C_ADDR,
                            SETUP_I2C_WIRE_INST,
                            SETUP_I2C_SPEED,
                            SETUP_SD_CARD_SPI_SPEED,
                            SETUP_WIFI_INST);

void setup() {
    // Setup base interfaces
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        Serial.begin(115200);           // Begin USB Serial interface
        while (!Serial) { ; }           // Wait for USB Serial to connect
    #endif
    #if defined(ESP32) || defined(ESP8266)
        SETUP_I2C_WIRE_INST.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL); // Begin i2c Wire for ESP
    #endif
    #if SETUP_ENABLE_WIFI
        String wifiSSID = F(SETUP_WIFI_SSID);
        String wifiPassword = F(SETUP_WIFI_SSID);
    #endif

    // Begin external data storage devices for crop, strings, and other data.
    #if SETUP_EXTDATA_EEPROM_ENABLE
        beginStringsFromEEPROM(SETUP_EEPROM_STRINGS_ADDR);
    #endif
    #if SETUP_EXTDATA_SD_ENABLE
        beginStringsFromSDCard(String(F(SETUP_EXTDATA_SD_LIB_PREFIX)) + String(F("strings")));
    #endif
    #if SETUP_EXTDATA_SD_ENABLE
        getCropsLibraryInstance()->beginCropsLibraryFromSDCard(String(F(SETUP_EXTDATA_SD_LIB_PREFIX)) + String(F("crop")));
    #endif
    #if SETUP_EXTDATA_EEPROM_ENABLE
        getCropsLibraryInstance()->beginCropsLibraryFromEEPROM(SETUP_EEPROM_CROPSLIB_ADDR);
    #endif

    // Sets system config name used in any of the following inits.
    #if SETUP_SD_CARD_CS_PIN >= 0 && SETUP_SAVES_SD_CARD_ENABLE
        hydroController.setSystemConfigFile(F(SETUP_SD_CARD_CONFIG_FILE));
    #endif
    // Sets the EEPROM memory address for system data.
    #if SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_ENABLE
        hydroController.setSystemDataAddress(SETUP_EEPROM_SYSDATA_ADDR);
    #endif

    // Initializes controller with first initialization method that successfully returns.
    if (!(false
        //#if SETUP_ENABLE_WIFI && SETUP_SAVELOAD_NETWORKURL_ENABLE
            //|| hydroController.initFromURL(wifiSSID, wifiPassword, urlDataTODO)
        //#endif
        #if SETUP_SD_CARD_CS_PIN >= 0 && SETUP_SAVES_SD_CARD_ENABLE
            || hydroController.initFromSDCard()
        #elif SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_ENABLE
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
        getLoggerInstance()->setLogLevel(JOIN(Hydroponics_LogLevel,SETUP_SYS_LOGLEVEL));
        #if SETUP_LOG_SD_ENABLE
            hydroController.enableSysLoggingToSDCard(F(SETUP_LOG_FILE_PREFIX));
        #endif
        #if SETUP_DATA_SD_ENABLE
            hydroController.enableDataPublishingToSDCard(F(SETUP_DATA_FILE_PREFIX));
        #endif
        #if SETUP_ENABLE_WIFI
            hydroController.setWiFiConnection(wifiSSID, wifiPassword);
            hydroController.getWiFi();      // Forces start, may block for a while
        #endif
        #if SETUP_SYS_AUTOSAVE_ENABLE && SETUP_SD_CARD_CS_PIN >= 0 && SETUP_SAVES_SD_CARD_ENABLE
            hydroController.setAutosaveEnabled(Hydroponics_Autosave_EnabledToSDCardJson);    
        #elif SETUP_SYS_AUTOSAVE_ENABLE && SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_ENABLE
            hydroController.setAutosaveEnabled(Hydroponics_Autosave_EnabledToEEPROMRaw);
        #endif

        // No further setup is necessary, as system is assumed to be built/managed via UI.
    }

    // TODO: UI initialization, other setup options

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
