// Simple-Hydroponics-Arduino Vertical Nutrient Film Technique (NFT) Example
// The Vertical NFT Example sketch is the standard implementation for our 3D printed
// controller enclosure and for most vertical towers that will be used. It can be
// easily extended to include other functionality if desired.

#include <Hydroponics.h>

// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          11              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_SIZE        I2C_DEVICESIZE_24LC256 // EEPROM bit storage size, in bytes (use I2C_DEVICESIZE_* defines), else 0
#define SETUP_EEPROM_I2C_ADDR           B000            // EEPROM address
#define SETUP_RTC_I2C_ADDR              B000            // RTC i2c address (only B000 can be used atm)
#define SETUP_SD_CARD_CS_PIN            SS              // SD card CS pin, else -1
#define SETUP_SD_CARD_SPI_SPEED         F_SPD           // SD card SPI speed, in Hz (ignored on Teensy)
#define SETUP_SPIRAM_DEVICE_SIZE        0               // SPI serial RAM device size, in bytes, else 0 (note: define HYDRUINO_ENABLE_SPIRAM_VIRTMEM to enable SPIRAM)
#define SETUP_SPIRAM_CS_PIN             -1              // SPI serial RAM CS pin, else -1
#define SETUP_SPIRAM_SPI_SPEED          F_SPD           // SPI serial RAM SPI speed, in Hz
#define SETUP_LCD_I2C_ADDR              B000            // LCD i2c address
#define SETUP_CTRL_INPUT_PINS           {31,33,30,32}   // Control input pin ribbon, else {-1}
#define SETUP_I2C_WIRE_INST             Wire            // I2C wire class instance
#define SETUP_I2C_SPEED                 400000U         // I2C speed, in Hz
#define SETUP_ESP_I2C_SDA               SDA             // I2C SDA pin, if on ESP
#define SETUP_ESP_I2C_SCL               SCL             // I2C SCL pin, if on ESP

// System Settings
#define SETUP_SYSTEM_MODE               Recycling       // System run mode (Recycling, DrainToWaste)
#define SETUP_MEASURE_MODE              Imperial        // System measurement mode (Default, Imperial, Metric, Scientific)
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

// WiFi Settings                                        (note: define HYDRUINO_ENABLE_WIFI or HYDRUINO_ENABLE_ESPWIFI to enable WiFi)
#define SETUP_WIFI_SSID                 "CHANGE_ME"     // WiFi SSID
#define SETUP_WIFI_PASS                 "CHANGE_ME"     // WiFi password

// Logging & Data Publishing Settings
#define SETUP_LOG_SD_ENABLE             true           // If system logging is enabled to SD card
#define SETUP_LOG_FILE_PREFIX           "logs/hy"       // System logs file prefix (appended with YYMMDD.txt)
#define SETUP_DATA_SD_ENABLE            true           // If system data publishing is enabled to SD card
#define SETUP_DATA_FILE_PREFIX          "data/hy"       // System data publishing files prefix (appended with YYMMDD.csv)

// External Data Settings
#define SETUP_EXTDATA_SD_ENABLE         false           // If data should be read from an external SD Card (searched first for crops lib data)
#define SETUP_EXTDATA_SD_LIB_PREFIX     "lib/"          // Library data folder/data file prefix (appended with {type}##.dat)
#define SETUP_EXTDATA_EEPROM_ENABLE     false           // If data should be read from an external EEPROM (searched first for strings data)

// External EEPROM Settings
#define SETUP_EEPROM_SYSDATA_ADDR       0x2e50          // System data memory offset for EEPROM saves (from Data Writer output)
#define SETUP_EEPROM_CROPSLIB_ADDR      0x0000          // Start address for Crops Library data (from Data Writer output)
#define SETUP_EEPROM_STRINGS_ADDR       0x1b24          // Start address for Strings data (from Data Writer output)

// Device Setup
#define SETUP_PH_METER_PIN              A0              // pH meter sensor pin (analog), else -1
#define SETUP_TDS_METER_PIN             A1              // TDS meter sensor pin (analog), else -1
#define SETUP_CO2_SENSOR_PIN            -1              // CO2 meter sensor pin (analog), else -1
#define SETUP_POWER_SENSOR_PIN          -1              // Power meter sensor pin (analog), else -1
#define SETUP_FLOW_RATE_SENSOR_PIN      -1              // Main feed pump flow rate sensor pin (analog/PWM), else -1
#define SETUP_DS18_WTEMP_PIN            2               // DS18* water temp sensor data pin (digital), else -1
#define SETUP_DHT_ATEMP_PIN             3               // DHT* air temp sensor data pin (digital), else -1
#define SETUP_DHT_SENSOR_TYPE           DHT12           // DHT sensor type enum (use DHT* defines)
#define SETUP_VOL_FILLED_PIN            4               // Water level filled indicator pin (digital/ISR), else -1
#define SETUP_VOL_EMPTY_PIN             -1              // Water level empty indicator pin (digital/ISR), else -1
#define SETUP_GROW_LIGHTS_PIN           5               // Grow lights relay pin (digital), else -1
#define SETUP_WATER_AERATOR_PIN         6               // Aerator relay pin (digital), else -1
#define SETUP_FEED_PUMP_PIN             7               // Water level low indicator pin, else -1
#define SETUP_WATER_HEATER_PIN          8               // Water heater relay pin (digital), else -1
#define SETUP_WATER_SPRAYER_PIN         -1              // Water sprayer relay pin (digital), else -1
#define SETUP_FAN_EXHAUST_PIN           -1              // Fan exhaust relay pin (digital/PWM), else -1
#define SETUP_NUTRIENT_MIX_PIN          9               // Nutrient premix peristaltic pump relay pin (digital), else -1
#define SETUP_FRESH_WATER_PIN           10              // Fresh water peristaltic pump relay pin (digital), else -1
#define SETUP_PH_UP_PIN                 12              // pH up solution peristaltic pump relay pin (digital), else -1
#define SETUP_PH_DOWN_PIN               13              // pH down solution peristaltic pump relay pin (digital), else -1

// Base Setup
#define SETUP_FEED_RESERVOIR_SIZE       4               // Reservoir size, in default measurement units
#define SETUP_AC_POWER_RAIL_TYPE        AC110V          // Rail power type used for AC rail (AC110V, AC220V)
#define SETUP_DC_POWER_RAIL_TYPE        DC12V           // Rail power type used for peristaltic pump rail (DC5V, DC12V)
#define SETUP_AC_SUPPLY_POWER           0               // Maximum AC supply power wattage, else 0 if not known (-> use simple rails)
#define SETUP_DC_SUPPLY_POWER           0               // Maximum DC supply power wattage, else 0 if not known (-> use simple rails)
#define SETUP_FEED_PUMP_FLOWRATE        20              // The base continuous flow rate of the main feed pumps, in L/min
#define SETUP_PERI_PUMP_FLOWRATE        0.070           // The base continuous flow rate of any peristaltic pumps, in L/min

// Crop Setup
#define SETUP_CROP_ON_TIME              15              // Minutes feeding pumps are to be turned on for
#define SETUP_CROP_OFF_TIME             45              // Minutes feeding pumps are to be turned off for
#define SETUP_CROP_TYPE                 Lettuce         // Type of crop planted, else Undefined
#define SETUP_CROP_SUBSTRATE            ClayPebbles     // Type of crop substrate, else Undefined
#define SETUP_CROP_SOW_DATE             DateTime(2022, 5, 21) // Date that crop was planted at
#define SETUP_CROP_SOILM_PIN            -1              // Soil moisture sensor for adaptive crop


#if defined(HYDRUINO_USE_SERIALWIFI) && !defined(SERIAL_PORT_HARDWARE1)
#include "SoftwareSerial.h"
SoftwareSerial Serial1(RX, TX);                         // Replace with Rx/Tx pins of your choice
#endif

uint8_t _SETUP_CTRL_INPUT_PINS[] = SETUP_CTRL_INPUT_PINS;
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

#if SETUP_GROW_LIGHTS_PIN >= 0 || SETUP_WATER_AERATOR_PIN >= 0 ||  SETUP_FEED_PUMP_PIN >= 0 || SETUP_WATER_HEATER_PIN >= 0 || SETUP_WATER_SPRAYER_PIN >= 0 || SETUP_FAN_EXHAUST_PIN >= 0
#define SETUP_USE_AC_RAIL
#endif
#if SETUP_NUTRIENT_MIX_PIN >= 0 || SETUP_FRESH_WATER_PIN >= 0 ||  SETUP_PH_UP_PIN >= 0 || SETUP_PH_DOWN_PIN >= 0 || SETUP_FAN_EXHAUST_PIN >= 0
#define SETUP_USE_DC_RAIL
#endif
#if defined(ADC_RESOLUTION)
#define SETUP_USE_ANALOG_BITRES     ADC_RESOLUTION
#elif defined(ESP32)
#define SETUP_USE_ANALOG_BITRES     12
#else
#define SETUP_USE_ANALOG_BITRES     10
#endif
#define SETUP_USE_ONEWIRE_BITRES    12

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
        #ifdef HYDRUINO_USE_SERIALWIFI
            Serial1.begin(HYDRUINO_SYS_ESPWIFI_SERIALBAUD);
            HYDRUINO_SYS_WIFI_INSTANCE.init(&Serial1); // Change to Serial instance of your choice, otherwise
        #endif
    #endif

    // Begin external data storage devices for crop, strings, and other data.
    #if SETUP_EXTDATA_EEPROM_ENABLE
        beginStringsFromEEPROM(SETUP_EEPROM_STRINGS_ADDR);
        getCropsLibraryInstance()->beginCropsLibraryFromEEPROM(SETUP_EEPROM_CROPSLIB_ADDR);
    #endif
    #if SETUP_EXTDATA_SD_ENABLE
        beginStringsFromSDCard(String(F(SETUP_EXTDATA_SD_LIB_PREFIX)) + String(F("strings")));
        getCropsLibraryInstance()->beginCropsLibraryFromSDCard(String(F(SETUP_EXTDATA_SD_LIB_PREFIX)) + String(F("crop")));
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
        //#if defined(HYDRUINO_USE_WIFI) && SETUP_SAVES_NETURL_ENABLE
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
        #ifdef HYDRUINO_USE_WIFI
            hydroController.setWiFiConnection(wifiSSID, wifiPassword);
            hydroController.getWiFi();      // Forces start, but may block for a while if not already initialized
        #endif
        #if SETUP_SYS_AUTOSAVE_ENABLE && SETUP_SD_CARD_CS_PIN >= 0 && SETUP_SAVES_SD_CARD_ENABLE
            hydroController.setAutosaveEnabled(Hydroponics_Autosave_EnabledToSDCardJson);
        #elif SETUP_SYS_AUTOSAVE_ENABLE && SETUP_EEPROM_DEVICE_SIZE && SETUP_SAVES_EEPROM_ENABLE
            hydroController.setAutosaveEnabled(Hydroponics_Autosave_EnabledToEEPROMRaw);
        #endif

        // Base Objects
        #ifdef SETUP_USE_AC_RAIL
            #if SETUP_AC_SUPPLY_POWER
                auto acRelayPower = hydroController.addRegulatedPowerRail(JOIN(Hydroponics_RailType,SETUP_AC_POWER_RAIL_TYPE),SETUP_AC_SUPPLY_POWER);
                // TODO: power sensor
            #else
                auto acRelayPower = hydroController.addSimplePowerRail(JOIN(Hydroponics_RailType,SETUP_AC_POWER_RAIL_TYPE));
            #endif
        #endif
        #ifdef SETUP_USE_DC_RAIL
            #if SETUP_DC_SUPPLY_POWER
                auto dcRelayPower = hydroController.addRegulatedPowerRail(JOIN(Hydroponics_RailType,SETUP_DC_POWER_RAIL_TYPE),SETUP_DC_SUPPLY_POWER);
                // TODO: power sensor
            #else
                auto dcRelayPower = hydroController.addSimplePowerRail(JOIN(Hydroponics_RailType,SETUP_DC_POWER_RAIL_TYPE));
            #endif
        #endif
        auto feedReservoir = hydroController.addFeedWaterReservoir(SETUP_FEED_RESERVOIR_SIZE, hydroController.getSystemMode() != Hydroponics_SystemMode_DrainToWaste);
        auto drainagePipe = hydroController.getSystemMode() == Hydroponics_SystemMode_DrainToWaste ? hydroController.addDrainagePipe() : SharedPtr<HydroponicsInfiniteReservoir>();

        // Crop
        {   auto cropType = JOIN(Hydroponics_CropType,SETUP_CROP_TYPE);
            if (cropType != Hydroponics_CropType_Undefined) {
                #if SETUP_CROP_SOILM_PIN >= 0
                    auto moistureSensor = hydroController.addAnalogMoistureSensor(SETUP_CROP_SOILM_PIN, SETUP_USE_ANALOG_BITRES);
                    auto crop = hydroController.addAdaptiveFedCrop(JOIN(Hydroponics_CropType,SETUP_CROP_TYPE),
                                                                   JOIN(Hydroponics_SubstrateType,SETUP_CROP_SUBSTRATE),
                                                                   SETUP_CROP_SOW_DATE);
                    moistureSensor->setCrop(crop);
                    crop->setSoilMoistureSensor(moistureSensor);
                #else
                    auto crop = hydroController.addTimerFedCrop(JOIN(Hydroponics_CropType,SETUP_CROP_TYPE),
                                                                JOIN(Hydroponics_SubstrateType,SETUP_CROP_SUBSTRATE),
                                                                SETUP_CROP_SOW_DATE,
                                                                SETUP_CROP_ON_TIME,
                                                                SETUP_CROP_OFF_TIME);
                    crop->setFeedReservoir(feedReservoir);
                #endif
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
        #if SETUP_POWER_SENSOR_PIN >= 0
        {   auto powerMeter = hydroController.addPowerUsageMeter(SETUP_POWER_SENSOR_PIN, SETUP_USE_ANALOG_BITRES);
            powerMeter->setReservoir(feedReservoir);
            #if SETUP_DC_SUPPLY_POWER
                dcRelayPower->setPowerSensor(powerMeter);
            #endif
        }
        #endif
        #if SETUP_FLOW_RATE_SENSOR_PIN >= 0
        {   auto flowSensor = hydroController.addAnalogPWMPumpFlowSensor(SETUP_FLOW_RATE_SENSOR_PIN, SETUP_USE_ANALOG_BITRES);
            flowSensor->setReservoir(feedReservoir);
            // will be set to main feed pump later via delayed ref
        }
        #endif

        // Digital Sensors
        #if SETUP_DS18_WTEMP_PIN >= 0
        {   auto dsTemperatureSensor = hydroController.addDSTemperatureSensor(SETUP_DS18_WTEMP_PIN, SETUP_USE_ONEWIRE_BITRES);
            dsTemperatureSensor->setReservoir(feedReservoir);
            feedReservoir->setWaterTemperatureSensor(dsTemperatureSensor);
        }
        #endif
        #if SETUP_DHT_ATEMP_PIN >= 0
        {   auto dhtTemperatureSensor = hydroController.addDHTTempHumiditySensor(SETUP_DHT_ATEMP_PIN, SETUP_DHT_SENSOR_TYPE);
            dhtTemperatureSensor->setReservoir(feedReservoir);
            feedReservoir->setAirTemperatureSensor(dhtTemperatureSensor);
        }
        #endif

        // Binary->Volume Sensors
        #if SETUP_VOL_FILLED_PIN >= 0
        {   auto filledIndicator = hydroController.addLevelIndicator(SETUP_VOL_FILLED_PIN);
            filledIndicator->setReservoir(feedReservoir);
            feedReservoir->setFilledTrigger(new HydroponicsMeasurementValueTrigger(filledIndicator, 0.5, false));
        }
        #endif
        #if SETUP_VOL_EMPTY_PIN >= 0
        {   auto emptyIndicator = hydroController.addLevelIndicator(SETUP_VOL_EMPTY_PIN);
            emptyIndicator->setReservoir(feedReservoir);
            feedReservoir->setEmptyTrigger(new HydroponicsMeasurementValueTrigger(emptyIndicator, 0.5, false));
        }
        #endif

        // Distance->Volume Sensors
        // TODO: ultrasonic distance
        // TODO: water height meter

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
                feedPump->setFlowRateSensor(HydroponicsIdentity(Hydroponics_SensorType_WaterPumpFlowSensor, 1)); // delayed ref (auto-resolves on launch)
            #endif
            if (hydroController.getSystemMode() == Hydroponics_SystemMode_DrainToWaste) {
                feedPump->setOutputReservoir(drainagePipe);
            } else {
                feedPump->setOutputReservoir(feedReservoir);
            }
            feedPump->setContinuousFlowRate(SETUP_FEED_PUMP_FLOWRATE, Hydroponics_UnitsType_LiqFlowRate_LitersPerMin);
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
        {   auto nutrientMix = hydroController.addFluidReservoir(Hydroponics_ReservoirType_NutrientPremix, 1, true);
            auto nutrientPump = hydroController.addPeristalticPumpRelay(SETUP_NUTRIENT_MIX_PIN);
            nutrientPump->setRail(dcRelayPower);
            nutrientPump->setInputReservoir(nutrientMix);
            nutrientPump->setOutputReservoir(feedReservoir);
            nutrientPump->setContinuousFlowRate(SETUP_PERI_PUMP_FLOWRATE, Hydroponics_UnitsType_LiqFlowRate_LitersPerMin);
        }
        #endif
        #if SETUP_FRESH_WATER_PIN >= 0
        {   auto freshWater = hydroController.addFluidReservoir(Hydroponics_ReservoirType_FreshWater, 1, true);
            auto dilutionPump = hydroController.addPeristalticPumpRelay(SETUP_NUTRIENT_MIX_PIN);
            dilutionPump->setRail(dcRelayPower);
            dilutionPump->setInputReservoir(freshWater);
            dilutionPump->setOutputReservoir(feedReservoir);
            dilutionPump->setContinuousFlowRate(SETUP_PERI_PUMP_FLOWRATE, Hydroponics_UnitsType_LiqFlowRate_LitersPerMin);
        }
        #endif
        #if SETUP_PH_UP_PIN >= 0
        {   auto phUpSolution = hydroController.addFluidReservoir(Hydroponics_ReservoirType_PhUpSolution, 1, true);
            auto pHUpPump = hydroController.addPeristalticPumpRelay(SETUP_NUTRIENT_MIX_PIN);
            pHUpPump->setRail(dcRelayPower);
            pHUpPump->setInputReservoir(phUpSolution);
            pHUpPump->setOutputReservoir(feedReservoir);
            pHUpPump->setContinuousFlowRate(SETUP_PERI_PUMP_FLOWRATE, Hydroponics_UnitsType_LiqFlowRate_LitersPerMin);
        }
        #endif
        #if SETUP_PH_DOWN_PIN >= 0
        {   auto phDownSolution = hydroController.addFluidReservoir(Hydroponics_ReservoirType_PhDownSolution, 1, true);
            auto pHDownPump = hydroController.addPeristalticPumpRelay(SETUP_NUTRIENT_MIX_PIN);
            pHDownPump->setRail(dcRelayPower);
            pHDownPump->setInputReservoir(phDownSolution);
            pHDownPump->setOutputReservoir(feedReservoir);
            pHDownPump->setContinuousFlowRate(SETUP_PERI_PUMP_FLOWRATE, Hydroponics_UnitsType_LiqFlowRate_LitersPerMin);
        }
        #endif
    }

    #ifdef HYDRUINO_USE_WIFI
        wifiSSID = wifiPassword = String(); // no longer needed
    #endif

    // TODO: UI initialization, other setup options

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
