// Simple-Hydroponics-Arduino Vertical Nutrient Film Technique (NFT) Example
// The Vertical NFT Example sketch is the standard implementation for our 3D printed
// controller enclosure and for most vertical towers that will be used. It can be
// easily extended to include other functionality if desired.

#ifdef USE_SW_SERIAL
#include "SoftwareSerial.h"
SoftwareSerial SWSerial(RX, TX);                        // Replace with Rx/Tx pins of your choice
#define Serial1 SWSerial
#endif

#include <Hydruino.h>

/// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_TYPE        None            // EEPROM device type/size (24LC01, 24LC02, 24LC04, 24LC08, 24LC16, 24LC32, 24LC64, 24LC128, 24LC256, 24LC512, None)
#define SETUP_EEPROM_I2C_ADDR           B000            // EEPROM i2c address
#define SETUP_RTC_I2C_ADDR              B000            // RTC i2c address (only B000 can be used atm)
#define SETUP_RTC_DEVICE_TYPE           None            // RTC device type (DS1307, DS3231, PCF8523, PCF8563, None)
#define SETUP_SD_CARD_SPI               SPI             // SD card SPI class instance
#define SETUP_SD_CARD_SPI_CS            -1              // SD card CS pin, else -1
#define SETUP_SD_CARD_SPI_SPEED         F_SPD           // SD card SPI speed, in Hz (ignored on Teensy)
#define SETUP_LCD_I2C_ADDR              B000            // LCD i2c address
#define SETUP_CTRL_INPUT_PINS           {-1}            // Control input pin ribbon, else {-1}
#define SETUP_I2C_WIRE                  Wire            // I2C wire class instance
#define SETUP_I2C_SPEED                 400000U         // I2C speed, in Hz
#define SETUP_ESP_I2C_SDA               SDA             // I2C SDA pin, if on ESP
#define SETUP_ESP_I2C_SCL               SCL             // I2C SCL pin, if on ESP

// WiFi Settings                                        (note: define HYDRO_ENABLE_WIFI or HYDRO_ENABLE_AT_WIFI to enable WiFi)
// #include "secrets.h"                                 // Pro-tip: Put sensitive password information into a custom secrets.h
#define SETUP_WIFI_SSID                 "CHANGE_ME"     // WiFi SSID
#define SETUP_WIFI_PASS                 "CHANGE_ME"     // WiFi passphrase
#define SETUP_WIFI_SPI                  SPIWIFI         // WiFi SPI class instance, if using spi
#define SETUP_WIFI_SPI_CS               SPIWIFI_SS      // WiFi CS pin, if using spi
#define SETUP_WIFI_SERIAL               Serial1         // WiFi serial class instance, if using serial

// Ethernet Settings                                    (note: define HYDRO_ENABLE_ETHERNET to enable Ethernet)
#define SETUP_ETHERNET_MAC              { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED } // Ethernet MAC address
#define SETUP_ETHERNET_SPI              SPI1            // Ethernet SPI class instance
#define SETUP_ETHERNET_SPI_CS           SS1             // Ethernet CS pin

// GPS Settings                                         (note: defined HYDRO_ENABLE_GPS to enable GPS)
#define SETUP_GPS_TYPE                  Serial          // Type of GPS (Serial, I2C, SPI)
#define SETUP_GPS_SERIAL                Serial1         // GPS serial class instance, if using serial
#define SETUP_GPS_I2C_ADDR              B000            // GPS i2c address, if using i2c
#define SETUP_GPS_SPI                   SPI             // GPS SPI class instance, if using spi
#define SETUP_GPS_SPI_CS                SS              // GPS CS pin, if using spi

// System Settings
#define SETUP_SYSTEM_MODE               Recycling       // System run mode (Recycling, DrainToWaste)
#define SETUP_MEASURE_MODE              Default         // System measurement mode (Default, Imperial, Metric, Scientific)
#define SETUP_LCD_OUT_MODE              Disabled        // System LCD output mode (Disabled, 20x4LCD, 20x4LCD_Swapped, 16x2LCD, 16x2LCD_Swapped)
#define SETUP_CTRL_IN_MODE              Disabled        // System control input mode (Disabled, 2x2Matrix, 4xButton, 6xButton, RotaryEncoder)
#define SETUP_SYS_UI_MODE               Minimal         // System user interface mode (Disabled, Minimal, Full)
#define SETUP_SYS_NAME                  "Hydruino"      // System name
#define SETUP_SYS_TIMEZONE              +0              // System timezone offset
#define SETUP_SYS_LOGLEVEL              All             // System log level filter (All, Warnings, Errors, None)

// System Saves Settings                                (note: only one primary and one fallback mechanism may be enabled at a time)
#define SETUP_SAVES_CONFIG_FILE         "hydruino.cfg"  // System config file name for system saves
#define SETUP_SAVES_SD_CARD_MODE        Disabled        // If saving/loading from SD card is enable (Primary, Fallback, Disabled)
#define SETUP_SAVES_EEPROM_MODE         Disabled        // If saving/loading from EEPROM is enabled (Primary, Fallback, Disabled)
#define SETUP_SAVES_WIFISTORAGE_MODE    Disabled        // If saving/loading from WiFiStorage (OS/OTA filesystem / WiFiNINA_Generic only) is enabled (Primary, Fallback, Disabled)

// Logging & Data Publishing Settings
#define SETUP_LOG_FILE_PREFIX           "logs/hy"       // System logs file prefix (appended with YYMMDD.txt)
#define SETUP_DATA_FILE_PREFIX          "data/hy"       // System data publishing files prefix (appended with YYMMDD.csv)
#define SETUP_DATA_SD_ENABLE            false           // If system data publishing is enabled to SD card
#define SETUP_LOG_SD_ENABLE             false           // If system logging is enabled to SD card
#define SETUP_DATA_WIFISTORAGE_ENABLE   false           // If system data publishing is enabled to WiFiStorage (OS/OTA filesystem / WiFiNINA_Generic only)
#define SETUP_LOG_WIFISTORAGE_ENABLE    false           // If system logging is enabled to WiFiStorage (OS/OTA filesystem / WiFiNINA_Generic only)

// MQTT Settings                                        (note: define HYDRO_ENABLE_MQTT to enable MQTT)
#define SETUP_MQTT_BROKER_CONNECT_BY    Hostname        // Which style of address broker uses (Hostname, IPAddress)
#define SETUP_MQTT_BROKER_HOSTNAME      "hostname"      // Hostname that MQTT broker exists at
#define SETUP_MQTT_BROKER_IPADDR        { 192, 168, 1, 2 } // IP address that MQTT broker exists at
#define SETUP_MQTT_BROKER_PORT          1883            // Port number that MQTT broker exists at

// External Data Settings
#define SETUP_EXTDATA_SD_ENABLE         false           // If data should be read from an external SD card (searched first for crops lib data)
#define SETUP_EXTDATA_SD_LIB_PREFIX     "lib/"          // Library data folder/data file prefix (appended with {type}##.dat)
#define SETUP_EXTDATA_EEPROM_ENABLE     false           // If data should be read from an external EEPROM (searched first for strings data)

// External EEPROM Settings
#define SETUP_EEPROM_SYSDATA_ADDR       0x2e50          // System data memory offset for EEPROM saves (from Data Writer output)
#define SETUP_EEPROM_CROPSLIB_ADDR      0x0000          // Start address for Crops Library data (from Data Writer output)
#define SETUP_EEPROM_STRINGS_ADDR       0x1b24          // Start address for strings data (from Data Writer output)

// Device Setup
#define SETUP_PH_METER_PIN              -1              // pH meter sensor pin (analog), else -1
#define SETUP_TDS_METER_PIN             -1              // TDS meter sensor pin (analog), else -1
#define SETUP_CO2_SENSOR_PIN            -1              // CO2 meter sensor pin (analog), else -1
#define SETUP_AC_POWER_SENSOR_PIN       -1              // AC power meter sensor pin (analog), else -1
#define SETUP_DC_POWER_SENSOR_PIN       -1              // DC power meter sensor pin (analog), else -1
#define SETUP_FLOW_RATE_SENSOR_PIN      -1              // Main feed pump flow rate sensor pin (analog/PWM), else -1
#define SETUP_DS18_WATER_TEMP_PIN       -1              // DS18* water temp sensor data pin (digital), else -1
#define SETUP_DHT_AIR_TEMP_HUMID_PIN    -1              // DHT* air temp sensor data pin (digital), else -1
#define SETUP_DHT_SENSOR_TYPE           None            // DHT sensor type enum (DHT11, DHT12, DHT21, DHT22, AM2301, None)
#define SETUP_VOL_FILLED_PIN            -1              // Water level filled indicator pin (digital/ISR), else -1
#define SETUP_VOL_EMPTY_PIN             -1              // Water level empty indicator pin (digital/ISR), else -1
#define SETUP_VOL_INDICATOR_TYPE        ACTIVE_HIGH     // Water level indicator type/active level (ACTIVE_HIGH, ACTIVE_LOW)
#define SETUP_VOL_LEVEL_PIN             -1              // Water level sensor pin (analog)
#define SETUP_VOL_LEVEL_TYPE            Ultrasonic      // Water level device type (Ultrasonic, AnalogHeight)
#define SETUP_GROW_LIGHTS_PIN           -1              // Grow lights relay pin (digital), else -1
#define SETUP_WATER_AERATOR_PIN         -1              // Aerator relay pin (digital), else -1
#define SETUP_FEED_PUMP_PIN             -1              // Water level low indicator pin, else -1
#define SETUP_WATER_HEATER_PIN          -1              // Water heater relay pin (digital), else -1
#define SETUP_WATER_SPRAYER_PIN         -1              // Water sprayer relay pin (digital), else -1
#define SETUP_FAN_EXHAUST_PIN           -1              // Fan exhaust relay pin (digital/PWM), else -1
#define SETUP_NUTRIENT_MIX_PIN          -1              // Nutrient premix peristaltic pump relay pin (digital), else -1
#define SETUP_FRESH_WATER_PIN           -1              // Fresh water peristaltic pump relay pin (digital), else -1
#define SETUP_PH_UP_PIN                 -1              // pH up solution peristaltic pump relay pin (digital), else -1
#define SETUP_PH_DOWN_PIN               -1              // pH down solution peristaltic pump relay pin (digital), else -1

// Base Setup
#define SETUP_FEED_RESERVOIR_SIZE       5               // Reservoir size, in default measurement units
#define SETUP_AC_POWER_RAIL_TYPE        AC110V          // Rail power type used for actuator relay AC rail (AC110V, AC220V)
#define SETUP_DC_POWER_RAIL_TYPE        DC12V           // Rail power type used for peristaltic pump DC rail (DC5V, DC12V)
#define SETUP_AC_SUPPLY_POWER           0               // Maximum AC supply power wattage, else 0 if not known (-> use simple rails)
#define SETUP_DC_SUPPLY_POWER           0               // Maximum DC supply power wattage, else 0 if not known (-> use simple rails)
#define SETUP_FEED_PUMP_FLOWRATE        20              // The base continuous flow rate of the main feed pumps, in L/min
#define SETUP_PERI_PUMP_FLOWRATE        0.070           // The base continuous flow rate of any peristaltic pumps, in L/min

// Crop Setup
#define SETUP_CROP_ON_TIME              15              // Minutes feeding pumps are to be turned on for (per feeding cycle)
#define SETUP_CROP_OFF_TIME             45              // Minutes feeding pumps are to be turned off for (per feeding cycle)
#define SETUP_CROP_TYPE                 Lettuce         // Type of crop planted, else Undefined
#define SETUP_CROP_SUBSTRATE            ClayPebbles     // Type of crop substrate, else Undefined
#define SETUP_CROP_NUMBER               1               // Number of plants in crop position (aka averaging weight)
#define SETUP_CROP_SOW_DATE             DateTime(2022, 5, 21) // Date that crop was planted at
#define SETUP_CROP_SOILM_PIN            -1              // Soil moisture sensor for adaptive crop

#if defined(HYDRO_USE_WIFI)
WiFiClient netClient;
#elif defined(HYDRO_USE_ETHERNET)
EthernetClient netClient;
#endif
#ifdef HYDRO_USE_MQTT
MQTTClient mqttClient;
#endif

#if defined(HYDRO_USE_GUI) && SETUP_LCD_OUT_MODE != Disabled
#if SETUP_SYS_UI_MODE == Minimal
#include "min/HydruinoUI.h"
typedef HydruinoMinUI HydruinoUI;
#elif SETUP_SYS_UI_MODE == Full
#include "full/HydruinoUI.h"
typedef HydruinoFullUI HydruinoUI;
#endif
#endif

// Pre-init checks
#if (SETUP_SAVES_WIFISTORAGE_MODE != Disabled || SETUP_DATA_WIFISTORAGE_ENABLE || SETUP_LOG_WIFISTORAGE_ENABLE) && !defined(HYDRO_USE_WIFI_STORAGE)
#warning The HYDRO_ENABLE_WIFI flag is expected to be defined as well as WiFiNINA_Generic.h included in order to run this sketch with WiFiStorage features enabled
#endif
#if (SETUP_SAVES_SD_CARD_MODE != Disabled || SETUP_DATA_SD_ENABLE || SETUP_LOG_SD_ENABLE || SETUP_EXTDATA_SD_ENABLE) && SETUP_SD_CARD_SPI_CS == -1
#warning The SETUP_SD_CARD_SPI_CS define is expected to be set to a valid pin in order to run this sketch with SD card features enabled
#endif
#if (SETUP_SAVES_EEPROM_MODE != Disabled || SETUP_EXTDATA_EEPROM_ENABLE) && SETUP_EEPROM_DEVICE_SIZE == 0
#warning The SETUP_EEPROM_DEVICE_SIZE define is expected to be set to a valid size in order to run this sketch with EEPROM features enabled
#endif

pintype_t _SETUP_CTRL_INPUT_PINS[] = SETUP_CTRL_INPUT_PINS;
Hydruino hydroController((pintype_t)SETUP_PIEZO_BUZZER_PIN,
                         JOIN(Hydro_EEPROMType,SETUP_EEPROM_DEVICE_TYPE),
                         I2CDeviceSetup((uint8_t)SETUP_EEPROM_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
                         JOIN(Hydro_RTCType,SETUP_RTC_DEVICE_TYPE),
                         I2CDeviceSetup((uint8_t)SETUP_RTC_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
                         SPIDeviceSetup((pintype_t)SETUP_SD_CARD_SPI_CS, &SETUP_SD_CARD_SPI, SETUP_SD_CARD_SPI_SPEED),
#if defined(HYDRO_USE_AT_WIFI)
                         TTLDeviceSetup(&SETUP_WIFI_SERIAL, HYDRO_SYS_ATWIFI_SERIALBAUD),
#elif defined(HYDRO_USE_WIFI)
                         SPIDeviceSetup((pintype_t)SETUP_WIFI_SPI_CS, &SETUP_WIFI_SPI),
#elif defined(HYDRO_USE_ETHERNET)
                         SPIDeviceSetup((pintype_t)SETUP_ETHERNET_SPI_CS, &SETUP_ETHERNET_SPI),
#else
                         DeviceSetup(),
#endif
// #if defined(HYDRO_USE_GPS) && SETUP_GPS_TYPE == Serial
//                          TTLDeviceSetup(&SETUP_GPS_SERIAL, HYDRO_SYS_NMEAGPS_SERIALBAUD),
// #elif defined(HYDRO_USE_GPS) && SETUP_GPS_TYPE == I2C
//                          I2CDeviceSetup(SETUP_GPS_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
// #elif defined(HYDRO_USE_GPS) && SETUP_GPS_TYPE == SPI
//                          SPIDeviceSetup(SETUP_GPS_SPI_CS, &SETUP_GPS_SPI),
// #endif
                         _SETUP_CTRL_INPUT_PINS,
                         I2CDeviceSetup((uint8_t)SETUP_LCD_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED));

#if SETUP_GROW_LIGHTS_PIN >= 0 || SETUP_WATER_AERATOR_PIN >= 0 ||  SETUP_FEED_PUMP_PIN >= 0 || SETUP_WATER_HEATER_PIN >= 0 || SETUP_WATER_SPRAYER_PIN >= 0 || SETUP_FAN_EXHAUST_PIN >= 0
#define SETUP_USE_AC_RAIL
#endif
#if SETUP_NUTRIENT_MIX_PIN >= 0 || SETUP_FRESH_WATER_PIN >= 0 ||  SETUP_PH_UP_PIN >= 0 || SETUP_PH_DOWN_PIN >= 0 || SETUP_FAN_EXHAUST_PIN >= 0
#define SETUP_USE_DC_RAIL
#endif
#if defined(ADC_RESOLUTION)
#define SETUP_USE_ANALOG_BITRES     ADC_RESOLUTION
#else
#define SETUP_USE_ANALOG_BITRES     10
#endif
#define SETUP_USE_ONEWIRE_BITRES    12

void setup() {
    // Setup base interfaces
    #ifdef HYDRO_ENABLE_DEBUG_OUTPUT
        Serial.begin(115200);           // Begin USB Serial interface
        while (!Serial) { ; }           // Wait for USB Serial to connect
    #endif
    #if defined(ESP_PLATFORM)
        SETUP_I2C_WIRE.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL); // Begin i2c Wire for ESP
    #endif
    #ifdef HYDRO_USE_WIFI
        String wifiSSID = F(SETUP_WIFI_SSID);
        String wifiPassword = F(SETUP_WIFI_PASS);
        #ifdef HYDRO_USE_AT_WIFI
            SETUP_WIFI_SERIAL.begin(HYDRO_SYS_ATWIFI_SERIALBAUD);
            WiFi.init(SETUP_WIFI_SERIAL);
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
    #if (defined(HYDRO_USE_WIFI_STORAGE) && SETUP_SAVES_WIFISTORAGE_MODE != Disabled) || \
        (SETUP_SD_CARD_SPI_CS >= 0 && SETUP_SAVES_SD_CARD_MODE != Disabled)
        hydroController.setSystemConfigFilename(F(SETUP_SAVES_CONFIG_FILE));
    #endif
    // Sets the EEPROM memory address for system data.
    #if SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_MODE != Disabled
        hydroController.setSystemDataAddress(SETUP_EEPROM_SYSDATA_ADDR);
    #endif

    // Initializes controller with first initialization method that successfully returns.
    if (!(false
        #if defined(HYDRO_USE_WIFI_STORAGE) && SETUP_SAVES_WIFISTORAGE_MODE == Primary
            || hydroController.initFromWiFiStorage()
        #elif SETUP_SD_CARD_SPI_CS >= 0 && SETUP_SAVES_SD_CARD_MODE == Primary
            || hydroController.initFromSDCard()
        #elif SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_MODE == Primary
            || hydroController.initFromEEPROM()
        #endif
        #if defined(HYDRO_USE_WIFI_STORAGE) && SETUP_SAVES_WIFISTORAGE_MODE == Fallback
            || hydroController.initFromWiFiStorage()
        #elif SETUP_SD_CARD_SPI_CS >= 0 && SETUP_SAVES_SD_CARD_MODE == Fallback
            || hydroController.initFromSDCard()
        #elif SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_MODE == Fallback
            || hydroController.initFromEEPROM()
        #endif
        )) {
        // First time running controller, set up default initial empty environment.
        hydroController.init(JOIN(Hydro_SystemMode,SETUP_SYSTEM_MODE),
                             JOIN(Hydro_MeasurementMode,SETUP_MEASURE_MODE),
                             JOIN(Hydro_DisplayOutputMode,SETUP_LCD_OUT_MODE),
                             JOIN(Hydro_ControlInputMode,SETUP_CTRL_IN_MODE));

        // Set Settings
        hydroController.setSystemName(F(SETUP_SYS_NAME));
        hydroController.setTimeZoneOffset(SETUP_SYS_TIMEZONE);
        #ifdef HYDRO_USE_WIFI
            hydroController.setWiFiConnection(wifiSSID, wifiPassword); wifiSSID = wifiPassword = String();
        #endif
        #ifdef HYDRO_USE_ETHERNET
        {   uint8_t _SETUP_ETHERNET_MAC[] = SETUP_ETHERNET_MAC;
            hydroController.setEthernetConnection(_SETUP_ETHERNET_MAC);
        }
        #endif
        getLoggerInstance()->setLogLevel(JOIN(Hydro_LogLevel,SETUP_SYS_LOGLEVEL));
        #if SETUP_LOG_SD_ENABLE
            hydroController.enableSysLoggingToSDCard(F(SETUP_LOG_FILE_PREFIX));
        #endif
        #if SETUP_DATA_SD_ENABLE
            hydroController.enableDataPublishingToSDCard(F(SETUP_DATA_FILE_PREFIX));
        #endif
        #if defined(HYDRO_USE_WIFI_STORAGE) && SETUP_LOG_WIFISTORAGE_ENABLE
            hydroController.enableSysLoggingToWiFiStorage(F(SETUP_LOG_FILE_PREFIX));
        #endif
        #if defined(HYDRO_USE_WIFI_STORAGE) && SETUP_DATA_WIFISTORAGE_ENABLE
            hydroController.enableDataPublishingToWiFiStorage(F(SETUP_DATA_FILE_PREFIX));
        #endif
        #ifdef HYDRO_USE_MQTT
            bool netBegan = false;
            #if defined(HYDRO_USE_WIFI)
                netBegan = hydroController.getWiFi();
            #elif defined(HYDRO_USE_ETHERNET)
                netBegan = hydroController.getEthernet();
            #endif
            if (netBegan) {
                #if SETUP_MQTT_BROKER_CONNECT_BY == Hostname
                    mqttClient.begin(String(F(SETUP_MQTT_BROKER_HOSTNAME)).c_str(), SETUP_MQTT_BROKER_PORT, netClient);
                #elif SETUP_MQTT_BROKER_CONNECT_BY == IPAddress
                {   uint8_t ipAddr[4] = SETUP_MQTT_BROKER_IPADDR;
                    IPAddress ip(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
                    mqttClient.begin(ip, SETUP_MQTT_BROKER_PORT, netClient);
                }
                #endif
                hydroController.enableDataPublishingToMQTTClient(mqttClient);
            }
        #endif
        #if defined(HYDRO_USE_WIFI_STORAGE) && SETUP_SAVES_WIFISTORAGE_MODE == Primary
            hydroController.setAutosaveEnabled(Hydro_Autosave_EnabledToWiFiStorageJson
        #elif SETUP_SD_CARD_SPI_CS >= 0 && SETUP_SAVES_SD_CARD_MODE == Primary
            hydroController.setAutosaveEnabled(Hydro_Autosave_EnabledToSDCardJson
        #elif SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_MODE == Primary
            hydroController.setAutosaveEnabled(Hydro_Autosave_EnabledToEEPROMRaw
        #else
            hydroController.setAutosaveEnabled(Hydro_Autosave_Disabled
        #endif
        #if defined(HYDRO_USE_WIFI_STORAGE) && SETUP_SAVES_WIFISTORAGE_MODE == Fallback
            , Hydro_Autosave_EnabledToWiFiStorageJson);
        #elif SETUP_SD_CARD_SPI_CS >= 0 && SETUP_SAVES_SD_CARD_MODE == Fallback
            , Hydro_Autosave_EnabledToSDCardJson);
        #elif SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_MODE == Fallback
            , Hydro_Autosave_EnabledToEEPROMRaw);
        #else
            );
        #endif

        // Base Objects
        #ifdef SETUP_USE_AC_RAIL
            #if SETUP_AC_SUPPLY_POWER
                auto acRelayPower = hydroController.addRegulatedPowerRail(JOIN(Hydro_RailType,SETUP_AC_POWER_RAIL_TYPE),SETUP_AC_SUPPLY_POWER);
                #if SETUP_AC_POWER_SENSOR_PIN >= 0
                {   auto powerMeter = hydroController.addPowerUsageMeter(SETUP_AC_POWER_SENSOR_PIN, SETUP_USE_ANALOG_BITRES);
                    acRelayPower->setPowerSensor(powerMeter);
                }
                #endif
            #else
                auto acRelayPower = hydroController.addSimplePowerRail(JOIN(Hydro_RailType,SETUP_AC_POWER_RAIL_TYPE));
            #endif
        #endif
        #ifdef SETUP_USE_DC_RAIL
            #if SETUP_DC_SUPPLY_POWER
                auto dcRelayPower = hydroController.addRegulatedPowerRail(JOIN(Hydro_RailType,SETUP_DC_POWER_RAIL_TYPE),SETUP_DC_SUPPLY_POWER);
                #if SETUP_DC_POWER_SENSOR_PIN >= 0
                {   auto powerMeter = hydroController.addPowerUsageMeter(SETUP_DC_POWER_SENSOR_PIN, SETUP_USE_ANALOG_BITRES);
                    dcRelayPower->setPowerSensor(powerMeter);
                }
                #endif
            #else
                auto dcRelayPower = hydroController.addSimplePowerRail(JOIN(Hydro_RailType,SETUP_DC_POWER_RAIL_TYPE));
            #endif
        #endif
        auto feedReservoir = hydroController.addFeedWaterReservoir(SETUP_FEED_RESERVOIR_SIZE, hydroController.getSystemMode() != Hydro_SystemMode_DrainToWaste);
        auto drainagePipe = hydroController.getSystemMode() == Hydro_SystemMode_DrainToWaste ? hydroController.addDrainagePipe() : SharedPtr<HydroInfiniteReservoir>();

        // Crop
        {   auto cropType = JOIN(Hydro_CropType,SETUP_CROP_TYPE);
            if (cropType != Hydro_CropType_Undefined) {
                #if SETUP_CROP_SOILM_PIN >= 0
                    auto moistureSensor = hydroController.addAnalogMoistureSensor(SETUP_CROP_SOILM_PIN, SETUP_USE_ANALOG_BITRES);
                    auto crop = hydroController.addAdaptiveFedCrop(JOIN(Hydro_CropType,SETUP_CROP_TYPE),
                                                                   JOIN(Hydro_SubstrateType,SETUP_CROP_SUBSTRATE),
                                                                   SETUP_CROP_SOW_DATE);
                    moistureSensor->setCrop(crop);
                    crop->setSoilMoistureSensor(moistureSensor);
                #else
                    auto crop = hydroController.addTimerFedCrop(JOIN(Hydro_CropType,SETUP_CROP_TYPE),
                                                                JOIN(Hydro_SubstrateType,SETUP_CROP_SUBSTRATE),
                                                                SETUP_CROP_SOW_DATE,
                                                                SETUP_CROP_ON_TIME,
                                                                SETUP_CROP_OFF_TIME);
                #endif
                crop->setFeedReservoir(feedReservoir);
                crop->setFeedingWeight(SETUP_CROP_NUMBER);
            }
        }

        // Analog Sensors
        #if SETUP_PH_METER_PIN >= 0
        {   auto phMeter = hydroController.addAnalogPhMeter(SETUP_PH_METER_PIN, SETUP_USE_ANALOG_BITRES);
            phMeter->setReservoir(feedReservoir);
            feedReservoir->setWaterPHSensor(phMeter);
        }
        #endif
        #if SETUP_TDS_METER_PIN >= 0
        {   auto tdsElectrode = hydroController.addAnalogTDSElectrode(SETUP_TDS_METER_PIN, SETUP_USE_ANALOG_BITRES);
            tdsElectrode->setReservoir(feedReservoir);
            feedReservoir->setWaterTDSSensor(tdsElectrode);
        }
        #endif
        #if SETUP_CO2_SENSOR_PIN >= 0
        {   auto co2Sensor = hydroController.addAnalogCO2Sensor(SETUP_CO2_SENSOR_PIN, SETUP_USE_ANALOG_BITRES);
            co2Sensor->setReservoir(feedReservoir);
            feedReservoir->setAirCO2Sensor(co2Sensor);
        }
        #endif
        #if SETUP_FLOW_RATE_SENSOR_PIN >= 0
        {   auto flowSensor = hydroController.addAnalogPWMPumpFlowSensor(SETUP_FLOW_RATE_SENSOR_PIN, SETUP_USE_ANALOG_BITRES);
            flowSensor->setReservoir(feedReservoir);
            // will be set to main feed pump later via delayed ref
        }
        #endif

        // Digital Sensors
        #if SETUP_DS18_WATER_TEMP_PIN >= 0
        {   auto dsTemperatureSensor = hydroController.addDSTemperatureSensor(SETUP_DS18_WATER_TEMP_PIN, SETUP_USE_ONEWIRE_BITRES);
            dsTemperatureSensor->setReservoir(feedReservoir);
            feedReservoir->setWaterTemperatureSensor(dsTemperatureSensor);
        }
        #endif
        #if SETUP_DHT_AIR_TEMP_HUMID_PIN >= 0
        {   auto dhtTemperatureSensor = hydroController.addDHTTempHumiditySensor(SETUP_DHT_AIR_TEMP_HUMID_PIN, JOIN(Hydro_DHTType,SETUP_DHT_SENSOR_TYPE));
            dhtTemperatureSensor->setReservoir(feedReservoir);
            feedReservoir->setAirTemperatureSensor(dhtTemperatureSensor);
        }
        #endif

        // Binary->Volume Sensors
        #if SETUP_VOL_FILLED_PIN >= 0
        {   auto filledIndicator = hydroController.addLevelIndicator(SETUP_VOL_FILLED_PIN);
            filledIndicator->setReservoir(feedReservoir);
            feedReservoir->setFilledTrigger(new HydroMeasurementValueTrigger(filledIndicator, 0.5, SETUP_VOL_INDICATOR_TYPE));
        }
        #endif
        #if SETUP_VOL_EMPTY_PIN >= 0
        {   auto emptyIndicator = hydroController.addLevelIndicator(SETUP_VOL_EMPTY_PIN);
            emptyIndicator->setReservoir(feedReservoir);
            feedReservoir->setEmptyTrigger(new HydroMeasurementValueTrigger(emptyIndicator, 0.5, SETUP_VOL_INDICATOR_TYPE));
        }
        #endif

        // Distance->Volume Sensors
        #if SETUP_VOL_LEVEL_PIN >= 0
            #if SETUP_VOL_LEVEL_TYPE == Ultrasonic
            {   auto distanceSensor = hydroController.addUltrasonicDistanceSensor(SETUP_VOL_LEVEL_PIN, SETUP_USE_ANALOG_BITRES);
                distanceSensor->setReservoir(feedReservoir);
                feedReservoir->setWaterVolumeSensor(distanceSensor);
                #if SETUP_VOL_FILLED_PIN < 0
                    feedReservoir->setFilledTrigger(new HydroMeasurementValueTrigger(distanceSensor, HYDRO_FEEDRES_FRACTION_FILLED, ACTIVE_ABOVE));
                #endif
                #if SETUP_VOL_EMPTY_PIN < 0
                    feedReservoir->setEmptyTrigger(new HydroMeasurementValueTrigger(distanceSensor, HYDRO_FEEDRES_FRACTION_EMPTY, ACTIVE_BELOW));
                #endif
            }
            #elif SETUP_VOL_LEVEL_TYPE == AnalogHeight
            {   auto heightMeter = hydroController.addAnalogWaterHeightMeter(SETUP_VOL_LEVEL_PIN, SETUP_USE_ANALOG_BITRES);
                heightMeter->setReservoir(feedReservoir);
                feedReservoir->setWaterVolumeSensor(heightMeter);
                #if SETUP_VOL_FILLED_PIN < 0
                    feedReservoir->setFilledTrigger(new HydroMeasurementValueTrigger(heightMeter, HYDRO_FEEDRES_FRACTION_FILLED, ACTIVE_ABOVE));
                #endif
                #if SETUP_VOL_EMPTY_PIN < 0
                    feedReservoir->setEmptyTrigger(new HydroMeasurementValueTrigger(heightMeter, HYDRO_FEEDRES_FRACTION_EMPTY, ACTIVE_BELOW));
                #endif
            }
            #endif
        #endif

        // AC-Based Actuators
        #if SETUP_GROW_LIGHTS_PIN >= 0
        {   auto growLights = hydroController.addGrowLightsRelay(SETUP_GROW_LIGHTS_PIN);
            growLights->setRail(acRelayPower);
            growLights->setReservoir(feedReservoir);
        }
        #endif
        #if SETUP_WATER_AERATOR_PIN >= 0
        {   auto aerator = hydroController.addWaterAeratorRelay(SETUP_WATER_AERATOR_PIN);
            aerator->setRail(acRelayPower);
            aerator->setReservoir(feedReservoir);
        }
        #endif
        #if SETUP_FEED_PUMP_PIN >= 0
        {   auto feedPump = hydroController.addWaterPumpRelay(SETUP_FEED_PUMP_PIN);
            feedPump->setRail(acRelayPower);
            feedPump->setInputReservoir(feedReservoir);
            #if SETUP_FLOW_RATE_SENSOR_PIN >= 0
                feedPump->setFlowRateSensor(HydroIdentity(Hydro_SensorType_WaterPumpFlowSensor, 1)); // delayed ref (auto-resolves on launch)
            #endif
            if (hydroController.getSystemMode() == Hydro_SystemMode_DrainToWaste) {
                feedPump->setOutputReservoir(drainagePipe);
            } else {
                feedPump->setOutputReservoir(feedReservoir);
            }
            feedPump->setContinuousFlowRate(SETUP_FEED_PUMP_FLOWRATE, Hydro_UnitsType_LiqFlowRate_LitersPerMin);
        }
        #endif
        #if SETUP_WATER_HEATER_PIN >= 0
        {   auto heater = hydroController.addWaterHeaterRelay(SETUP_WATER_HEATER_PIN);
            heater->setRail(acRelayPower);
            heater->setReservoir(feedReservoir);
        }
        #endif
        #if SETUP_WATER_SPRAYER_PIN >= 0
        {   auto sprayer = hydroController.addWaterSprayerRelay(SETUP_WATER_SPRAYER_PIN);
            sprayer->setRail(acRelayPower);
            sprayer->setReservoir(feedReservoir);
        }
        #endif
        #if SETUP_FAN_EXHAUST_PIN >= 0
        if (checkPinIsPWMOutput(SETUP_FAN_EXHAUST_PIN)) {
            auto fanExhaust = hydroController.addAnalogPWMFanExhaust(SETUP_FAN_EXHAUST_PIN, SETUP_USE_ANALOG_BITRES);
            fanExhaust->setRail(dcRelayPower);          // PWM fans use DC relay
            fanExhaust->setReservoir(feedReservoir);
        } else {
            auto fanExhaust = hydroController.addFanExhaustRelay(SETUP_FAN_EXHAUST_PIN);
            fanExhaust->setRail(acRelayPower);
            fanExhaust->setReservoir(feedReservoir);
        }
        #endif

        // DC-Based Peristaltic Pumps
        #if SETUP_NUTRIENT_MIX_PIN >= 0
        {   auto nutrientMix = hydroController.addFluidReservoir(Hydro_ReservoirType_NutrientPremix, 1, true);
            auto nutrientPump = hydroController.addPeristalticPumpRelay(SETUP_NUTRIENT_MIX_PIN);
            nutrientPump->setRail(dcRelayPower);
            nutrientPump->setInputReservoir(nutrientMix);
            nutrientPump->setOutputReservoir(feedReservoir);
            nutrientPump->setContinuousFlowRate(SETUP_PERI_PUMP_FLOWRATE, Hydro_UnitsType_LiqFlowRate_LitersPerMin);
        }
        #endif
        #if SETUP_FRESH_WATER_PIN >= 0
        {   auto freshWater = hydroController.addFluidReservoir(Hydro_ReservoirType_FreshWater, 1, true);
            auto dilutionPump = hydroController.addPeristalticPumpRelay(SETUP_NUTRIENT_MIX_PIN);
            dilutionPump->setRail(dcRelayPower);
            dilutionPump->setInputReservoir(freshWater);
            dilutionPump->setOutputReservoir(feedReservoir);
            dilutionPump->setContinuousFlowRate(SETUP_PERI_PUMP_FLOWRATE, Hydro_UnitsType_LiqFlowRate_LitersPerMin);
        }
        #endif
        #if SETUP_PH_UP_PIN >= 0
        {   auto phUpSolution = hydroController.addFluidReservoir(Hydro_ReservoirType_PhUpSolution, 1, true);
            auto pHUpPump = hydroController.addPeristalticPumpRelay(SETUP_NUTRIENT_MIX_PIN);
            pHUpPump->setRail(dcRelayPower);
            pHUpPump->setInputReservoir(phUpSolution);
            pHUpPump->setOutputReservoir(feedReservoir);
            pHUpPump->setContinuousFlowRate(SETUP_PERI_PUMP_FLOWRATE, Hydro_UnitsType_LiqFlowRate_LitersPerMin);
        }
        #endif
        #if SETUP_PH_DOWN_PIN >= 0
        {   auto phDownSolution = hydroController.addFluidReservoir(Hydro_ReservoirType_PhDownSolution, 1, true);
            auto pHDownPump = hydroController.addPeristalticPumpRelay(SETUP_NUTRIENT_MIX_PIN);
            pHDownPump->setRail(dcRelayPower);
            pHDownPump->setInputReservoir(phDownSolution);
            pHDownPump->setOutputReservoir(feedReservoir);
            pHDownPump->setContinuousFlowRate(SETUP_PERI_PUMP_FLOWRATE, Hydro_UnitsType_LiqFlowRate_LitersPerMin);
        }
        #endif
    }

    #if defined(HYDRO_USE_GUI) && SETUP_LCD_OUT_MODE != Disabled && SETUP_SYS_UI_MODE != Disabled
        hydroController.enableUI(new HydruinoUI());
    #endif

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
