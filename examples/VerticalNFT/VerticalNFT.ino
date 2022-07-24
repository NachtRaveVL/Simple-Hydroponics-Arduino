// Simple-Hydroponics-Arduino Vertical Nutrient Film Technique (NFT) Example
// TODO

#include <Hydroponics.h>

// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN      -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_SIZE    I2C_DEVICESIZE_24LC256 // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
#define SETUP_SD_CARD_CS_PIN        SS              // SD card CS pin, else -1
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
#define SETUP_MEASURE_MODE          Imperial        // System measurement mode (Default, Imperial, Metric, Scientific)
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
#define SETUP_LOG_SD_ENABLE         true            // Whenever or system logging is enabled to SD card
#define SETUP_LOG_FILE_PREFIX       "logs/hy"       // System logs file prefix (appended with YYMMDD.txt)
#define SETUP_DATA_SD_ENABLE        true            // Whenever or system data publishing is enabled to SD card
#define SETUP_DATA_FILE_PREFIX      "data/hy"       // System data publishing files prefix (appended with YYMMDD.csv)

// Device Pin Setup
#define SETUP_PH_METER_PIN          A0              // pH meter sensor pin (analog), else -1
#define SETUP_TDS_METER_PIN         A1              // TDS meter sensor pin (analog), else -1
#define SETUP_CO2_METER_PIN         -1              // CO2 meter sensor pin (analog), else -1
#define SETUP_POWER_METER_PIN       -1              // Power meter sensor pin (analog), else -1
#define SETUP_DS18_WTEMP_PIN        3               // DS18* water temp sensor data pin (digital), else -1
#define SETUP_DHT_ATEMP_PIN         4               // DHT* air temp sensor data pin (digital), else -1
#define SETUP_VOL_FILLED_PIN        -1              // Water level filled indicator pin (digital), else -1
#define SETUP_VOL_EMPTY_PIN         -1              // Water level empty indicator pin (digital), else -1
#define SETUP_GROW_LIGHTS_PIN       22              // Grow lights relay pin (digital), else -1
#define SETUP_AERATOR_PIN           24              // Aerator relay pin (digital), else -1
#define SETUP_FEED_PUMP_PIN         26              // Water level low indicator pin, else -1
#define SETUP_WATER_HEATER_PIN      -1              // Water heater relay pin (digital), else -1
#define SETUP_FAN_EXHAUST_PIN       -1              // Fan exhaust relay pin (digital), else -1
#define SETUP_NUTRIENT_MIX_PIN      23              // Nutrient premix peristaltic pump relay pin (digital), else -1
#define SETUP_FRESH_WATER_PIN       25              // Fresh water peristaltic pump relay pin (digital), else -1
#define SETUP_PH_UP_PIN             27              // pH up solution peristaltic pump relay pin (digital), else -1
#define SETUP_PH_DOWN_PIN           29              // pH down solution peristaltic pump relay pin (digital), else -1

// Object Setup
#define SETUP_FEED_RESERVOIR_SIZE   4               // Reservoir size, in default measurement units
#define SETUP_AC_POWER_RAIL_TYPE    AC110V          // Rail power type used for AC rail (AC110V, AC220V)
#define SETUP_DC_POWER_RAIL_TYPE    DC12V           // Rail power type used for peristaltic pump rail (DC5V, DC12V)

// Crop Setup
#define SETUP_CROP_ON_TIME          15              // Minutes feeding pumps are to be turned on for
#define SETUP_CROP_OFF_TIME         45              // Minutes feeding pumps are to be turned off for
#define SETUP_CROP1_TYPE            Lettuce
#define SETUP_CROP1_SUBSTRATE       ClayPebbles
#define SETUP_CROP1_SOW_DATE        DateTime(2022, 5, 21)
#define SETUP_CROP2_TYPE            Lettuce
#define SETUP_CROP2_SUBSTRATE       ClayPebbles
#define SETUP_CROP2_SOW_DATE        DateTime(2022, 5, 21)
#define SETUP_CROP3_TYPE            Lettuce
#define SETUP_CROP3_SUBSTRATE       ClayPebbles
#define SETUP_CROP3_SOW_DATE        DateTime(2022, 5, 21)
#define SETUP_CROP4_TYPE            Lettuce
#define SETUP_CROP4_SUBSTRATE       ClayPebbles
#define SETUP_CROP4_SOW_DATE        DateTime(2022, 5, 21)
#define SETUP_CROP5_TYPE            Lettuce
#define SETUP_CROP5_SUBSTRATE       ClayPebbles
#define SETUP_CROP5_SOW_DATE        DateTime(2022, 5, 21)


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

        // TODO: Object setup from above defines
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
