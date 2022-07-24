// Simple-Hydroponics-Arduino Full System
// This sketch will build the entire library onto a device, while supporting all of its
// functionality, and thus has the highest cost. Not meant for constrained devices.
// TODO

#include <Hydroponics.h>

// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN      -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_SIZE    0               // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
#define SETUP_SD_CARD_CS_PIN        -1              // SD card CS pin, else -1
#define SETUP_CTRL_INPUT_PIN1       -1              // First pin of input ribbon, else -1 (ribbon pins can be individually customized later)
#define SETUP_EEPROM_I2C_ADDR       B000            // EEPROM address
#define SETUP_RTC_I2C_ADDR          B000            // RTC i2c address (only B000 can be used atm)
#define SETUP_LCD_I2C_ADDR          B000            // LCD i2c address
#define SETUP_I2C_WIRE_INST         Wire            // I2C wire class instance
#define SETUP_I2C_SPEED             400000U         // I2C speed, in Hz
#define SETUP_SD_CARD_SPI_SPEED     4000000U        // SD card SPI speed, in Hz (ignored if on Teensy)
#define SETUP_WIFI_INST             WiFi            // WiFi class instance

// System Settings
#define SETUP_RUN_MODE              Recycling       // System run mode (Recycling, DrainToWaste)
#define SETUP_MEASURE_MODE          Default         // System measurement mode (Default, Imperial, Metric, Scientific)
#define SETUP_LCD_OUT_MODE          Disabled        // System LCD output mode (Disabled, 20x4LCD, 20x4LCD_Swapped, 16x2LCD, 16x2LCD_Swapped)
#define SETUP_CTRL_IN_MODE          Disabled        // System control input mode (Disabled, 2x2Matrix, 4xButton, 6xButton, RotaryEncoder)
#define SETUP_SYS_NAME              "Hydruino"      // System name
#define SETUP_SYS_TIMEZONE          +0              // System timezone offset
#define SETUP_CONFIG_FILE           "hydruino.cfg"  // System config file name

// WiFi Settings
#define SETUP_ENABLE_WIFI           false           // Whenever or not WiFi is enabled
#define SETUP_WIFI_SSID             "CHANGE_ME"     // WiFi SSID
#define SETUP_WIFI_PASS             "CHANGE_ME"     // WiFi password

// Logging & Data Publishing Settings
#define SETUP_LOG_SD_ENABLE         false           // Whenever or system logging is enabled to SD card
#define SETUP_LOG_FILE_PREFIX       "logs/hy"       // System logs file prefix (appended with YYMMDD.txt)
#define SETUP_DATA_SD_ENABLE        false           // Whenever or system data publishing is enabled to SD card
#define SETUP_DATA_FILE_PREFIX      "data/hy"       // System data publishing files prefix (appended with YYMMDD.csv)


Hydroponics hydroController(SETUP_PIEZO_BUZZER_PIN,
                            SETUP_EEPROM_DEVICE_SIZE,
                            SETUP_SD_CARD_CS_PIN,
                            SETUP_CTRL_INPUT_PIN1,
                            SETUP_EEPROM_I2C_ADDR,
                            SETUP_RTC_I2C_ADDR,
                            SETUP_LCD_I2C_ADDR,
                            SETUP_I2C_WIRE_INST,
                            SETUP_I2C_SPEED,
                            SETUP_SD_CARD_SPI_SPEED,
                            SETUP_WIFI_INST);

void setup() {
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        Serial.begin(115200);           // Begin USB Serial interface
        while(!Serial) { ; }            // Wait for USB Serial to connect (remove in production)
    #endif
    #if SETUP_ENABLE_WIFI
        String wifiSSID = F(SETUP_WIFI_SSID);
        String wifiPassword = F(SETUP_WIFI_SSID);
    #endif

    // Sets system config name used in any of the following inits.
    hydroController.setSystemConfigFile(F(SETUP_CONFIG_FILE));

    // Initializes controller with first initialization method that successfully returns.
    if (!(false
        #if SETUP_SD_CARD_CS_PIN >= 0
            || hydroController.initFromSDCard()
        #endif
        #if SETUP_EEPROM_DEVICE_SIZE
            || hydroController.initFromEEPROM()
        #endif
        #if SETUP_ENABLE_WIFI
            //|| hydroController.initFromURL(wifiSSID, wifiPassword, TODO)
        #endif
        )) {
        // First time running controller, set up default initial empty environment.
        hydroController.init(JOIN(Hydroponics_SystemMode_,SETUP_RUN_MODE),
                             JOIN(Hydroponics_MeasurementMode_,SETUP_MEASURE_MODE),
                             JOIN(Hydroponics_DisplayOutputMode_,SETUP_LCD_OUT_MODE),
                             JOIN(Hydroponics_ControlInputMode_,SETUP_CTRL_IN_MODE));

        // Set settings
        hydroController.setSystemName(F(SETUP_SYS_NAME));
        hydroController.setTimeZoneOffset(SETUP_SYS_TIMEZONE);
        #if SETUP_ENABLE_WIFI
            hydroController.setWiFiConnection(wifiSSID, wifiPassword);
            hydroController.getWiFi();      // Forces start, may block for a while
        #endif
        #if SETUP_LOG_SD_ENABLE
            hydroController.enableSysLoggingToSDCard(F(SETUP_LOG_FILE_PREFIX));
        #endif
        #if SETUP_DATA_SD_ENABLE
            hydroController.enableDataPublishingToSDCard(F(SETUP_DATA_FILE_PREFIX));
        #endif
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
