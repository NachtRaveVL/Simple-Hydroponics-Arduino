// Simple-Hydroponics-Arduino Vertical Nutrient Film Technique (NFT) Example
//
// The Vertical NFT Example sketch is our recommended standard implementation. It can
// be easily extended to include other functionality as desired. Follow the setup
// defines below, filling in for your own particular system setup.
//
// Under minimal UI mode, if any UI I/O is enabled, the UI will allow you to modify,
// but not create/destroy, objects created below. This restriction also applies to
// various other settings, which will be locked under the UI.
//
// To again modify these locked values, a new sketch will have to be re-built and
// re-uploaded. This process allows code-stripping to reduce build sizes to levels
// that may work with <512kB Flash on constrained devices. Otherwise, one may need to
// consider data export to SD Card or EEPROM - see the DataWriter example for details.

#ifdef USE_SW_SERIAL
#include "SoftwareSerial.h"
SoftwareSerial SWSerial(RX, TX);                        // Replace with Rx/Tx pins of your choice
#define Serial1 SWSerial
#endif

#include <Hydruino.h>

// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_TYPE        None            // EEPROM device type/size (AT24LC01, AT24LC02, AT24LC04, AT24LC08, AT24LC16, AT24LC32, AT24LC64, AT24LC128, AT24LC256, AT24LC512, None)
#define SETUP_EEPROM_I2C_ADDR           0b000           // EEPROM i2c address (A0-A2, bitwise or'ed with base address 0x50)
#define SETUP_RTC_DEVICE_TYPE           None            // RTC device type (DS1307, DS3231, PCF8523, PCF8563, None)
#define SETUP_SD_CARD_SPI               SPI             // SD card SPI class instance
#define SETUP_SD_CARD_SPI_CS            -1              // SD card CS pin, else -1
#define SETUP_SD_CARD_SPI_SPEED         F_SPD           // SD card SPI speed, in Hz (ignored on Teensy)
#define SETUP_DISP_LCD_I2C_ADDR         0b111           // LCD i2c address (A0-A2, bitwise or'ed with base address 0x20)
#define SETUP_DISP_OLED_I2C_ADDR        0b000           // OLED i2c address (A0-A2, bitwise or'ed with base address 0x78)
#define SETUP_DISP_SPI                  SPI             // Display SPI class instance
#define SETUP_DISP_SPI_CS               -1              // Display SPI CS pin, else -1
#define SETUP_DISP_SPI_SPEED            F_SPD           // Display SPI speed, in Hz
#define SETUP_CTRL_INPUT_PINS           {hpin_none}     // Control input pins array, else {-1} (should be same sized array as control input mode enum specifies)
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
#define SETUP_ETHERNET_MAC              { (uint8_t)0xDE, (uint8_t)0xAD, (uint8_t)0xBE, (uint8_t)0xEF, (uint8_t)0xFE, (uint8_t)0xED } // Ethernet MAC address
#define SETUP_ETHERNET_SPI              SPI1            // Ethernet SPI class instance
#define SETUP_ETHERNET_SPI_CS           SS1             // Ethernet CS pin

// GPS Settings                                         (note: define HYDRO_ENABLE_GPS to enable GPS)
#define SETUP_GPS_TYPE                  None            // Type of GPS (UART, I2C, SPI, None)
#define SETUP_GPS_SERIAL                Serial1         // GPS serial class instance, if using serial
#define SETUP_GPS_I2C_ADDR              0b000           // GPS i2c address (A0-A2, bitwise or'ed with base address 0x10), if using i2c
#define SETUP_GPS_SPI                   SPI             // GPS SPI class instance, if using spi
#define SETUP_GPS_SPI_CS                SS              // GPS CS pin, if using spi

// System Settings
#define SETUP_SYSTEM_MODE               Recycling       // System run mode (Recycling, DrainToWaste)
#define SETUP_MEASURE_MODE              Default         // System measurement mode (Default, Imperial, Metric, Scientific)
#define SETUP_DISPLAY_OUT_MODE          Disabled        // System display output mode (Disabled, LCD16x2_EN, LCD16x2_RS, LCD20x4_EN, LCD20x4_RS, SSD1305, SSD1305_x32Ada, SSD1305_x64Ada, SSD1306, SH1106, CustomOLED, SSD1607, IL3820, IL3820_V2, ST7735, ST7789, ILI9341, TFT)
#define SETUP_CONTROL_IN_MODE           Disabled        // System control input mode (Disabled, RotaryEncoderOk, RotaryEncoderOkLR, UpDownButtonsOk, UpDownButtonsOkLR, UpDownESP32TouchOk, UpDownESP32TouchOkLR, AnalogJoystickOk, Matrix2x2UpDownButtonsOkL, Matrix3x4Keyboard_OptRotEncOk, Matrix3x4Keyboard_OptRotEncOkLR, Matrix4x4Keyboard_OptRotEncOk, Matrix4x4Keyboard_OptRotEncOkLR, ResistiveTouch, TouchScreen, TFTTouch, RemoteControl)
#define SETUP_SYS_UI_MODE               Minimal         // System user interface mode (Disabled, Minimal, Full)
#define SETUP_SYS_NAME                  "Hydruino"      // System name
#define SETUP_SYS_TIMEZONE              +0              // System timezone offset, in hours (int or float)
#define SETUP_SYS_LOGLEVEL              All             // System log level filter (All, Warnings, Errors, None)
#define SETUP_SYS_STATIC_LAT            DBL_UNDEF       // System static latitude (if not using GPS/UI, else DBL_UNDEF), in degrees
#define SETUP_SYS_STATIC_LONG           DBL_UNDEF       // System static longitude (if not using GPS/UI, else DBL_UNDEF), in minutes
#define SETUP_SYS_STATIC_ALT            DBL_UNDEF       // System static altitude (if not using GPS/UI, else DBL_UNDEF), in meters above sea level (msl)

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
#define SETUP_MQTT_BROKER_IPADDR        { (uint8_t)192, (uint8_t)168, (uint8_t)1, (uint8_t)2 } // IP address that MQTT broker exists at
#define SETUP_MQTT_BROKER_PORT          1883            // Port number that MQTT broker exists at

// External Data Settings
#define SETUP_EXTDATA_SD_ENABLE         false           // If data should be read from an external SD card (searched first for crops lib data)
#define SETUP_EXTDATA_SD_LIB_PREFIX     "lib/"          // Library data folder/data file prefix (appended with {type}##.dat)
#define SETUP_EXTDATA_EEPROM_ENABLE     false           // If data should be read from an external EEPROM (searched first for strings data)

// External EEPROM Settings
#define SETUP_EEPROM_SYSDATA_ADDR       0x3333          // System data memory offset for EEPROM saves (from Data Writer output)
#define SETUP_EEPROM_CROPSLIB_ADDR      0x0000          // Start address for Crops Library data (from Data Writer output)
#define SETUP_EEPROM_STRINGS_ADDR       0x1111          // Start address for strings data (from Data Writer output)
#define SETUP_EEPROM_UIDSTRS_ADDR       0x2222          // Start address for UI strings data (from Data Writer output, GUI not disabled)

// UI Settings
#define SETUP_UI_LOGIC_LEVEL            ACT_LOW         // I/O signaling logic active level (ACT_LOW, ACT_HIGH)
#define SETUP_UI_ALLOW_INTERRUPTS       true            // Allow interrupt driven I/O if able, else force polling
#define SETUP_UI_USE_TCUNICODE_FONTS    false           // Use tcUnicode fonts instead of gfx-lib specific fonts, if using graphical display
#define SETUP_UI_USE_BUFFERED_VRAM      HAS_LARGE_SRAM  // Use sprite-sized buffered video RAM for smooth animations, if large SRAM
#define SETUP_UI_IS_DFROBOTSHIELD       false           // Using DFRobotShield as preset (SETUP_CTRL_INPUT_PINS may be left {-1})

// UI Display Output Settings
#define SETUP_UI_GFX_ROTATION           R0              // Display rotation (R0, R1, R2, R3, HorzMirror, VertMirror), if using graphical display or touchscreen
#define SETUP_UI_GFX_DC_PIN             -1              // Display interface DC/RS pin, if using SPI display
#define SETUP_UI_GFX_RESET_PIN          -1              // Optional display interface reset/RST pin, if using SPI display, else -1 (Note: Unused reset pin typically needs tied to HIGH for display to function)
#define SETUP_UI_GFX_ST7735_TAG         Undefined       // ST7735 tag color (B, Green, Green18, Red, Red18, Black, Black18, Green144, Mini, MiniPlugin, HalloWing), if using ST7735 display
#define SETUP_UI_GFX_ST7789_RES         Undefined       // ST7789 screen resolution (128x128, 135x240, 170x320, 172x320, 240x240, 240x280, 240x320, CustomTFT), if using ST7789 display
#define SETUP_UI_GFX_BACKLIGHT_PIN      -1              // Optional display interface backlight/LED/BL pin, if using SPI display (Note: Unused backlight pin can optionally be tied typically to HIGH for always-on)
#define SETUP_UI_GFX_BACKLIGHT_MODE     Normal          // Display backlight mode (Normal, Inverted, PWM), if using LCD or display /w backlight pin
#define SETUP_UI_GFX_BACKLIGHT_ESP_CHN  1               // Backlight PWM channel, if on ESP/using PWM backlight
#define SETUP_UI_GFX_BACKLIGHT_ESP_FRQ  1000            // Backlight PWM frequency, if on ESP/using PWM backlight

// UI Control Input Settings
#define SETUP_UI_ENC_ROTARY_SPEED       HalfCycle       // Rotary encoder cycling speed (FullCycle, HalfCycle, QuarterCycle)
#define SETUP_UI_KEY_REPEAT_SPEED       20              // Key repeat speed, in ticks (lower = faster)
#define SETUP_UI_KEY_REPEAT_DELAY       850             // Key repeat delay, in milliseconds
#define SETUP_UI_KEY_REPEAT_INTERVAL    350             // Key repeat interval, in milliseconds
#define SETUP_UI_JS_ACCELERATION        3.0f            // Joystick acceleration (decrease divisor), if using analog joystick
#define SETUP_UI_TOUCHSCREEN_ORIENT     Same            // Touchscreen orientation tuning (Same, Plus1, Plus2, Plus3, None, InvertX, InvertY, InvertXY, SwapXY, InvertX_SwapXY, InvertY_SwapXY, InvertXY_SwapXY), if using touchscreen
#define SETUP_UI_TOUCHSCREEN_SPI        SPI             // SPI class for XPT2046 touchscreen, if using XTP2046
#define SETUP_UI_ESP32TOUCH_SWITCH      800             // ESP32 Touch key switch threshold, if on ESP32/using ESP32Touch
#define SETUP_UI_ESP32TOUCH_HVOLTS      V_2V7           // ESP32 Touch key high reference voltage (Keep, V_2V4, V_2V5, V_2V6, V_2V7, Max), if on ESP32/using ESP32Touch
#define SETUP_UI_ESP32TOUCH_LVOLTS      V_0V5           // ESP32 Touch key low reference voltage (Keep, V_0V5, V_0V6, V_0V7, V_0V8, Max), if on ESP32/using ESP32Touch
#define SETUP_UI_ESP32TOUCH_HVATTEN     V_1V            // ESP32 Touch key high ref voltage attenuation (Keep, V_1V5, V_1V, V_0V5, V_0V, Max), if on ESP32/using ESP32Touch

// UI Remote Control Settings
#define SETUP_UI_DEVICE_UUID            "00000000-0000-0000-0000-000000000000" // Device UUID hex string, for unique identification of device
#define SETUP_UI_REMOTE1_TYPE           Disabled        // Type of first remote control (Disabled, Serial, Simhub, WiFi, Ethernet)
#define SETUP_UI_REMOTE1_UART           Serial1         // Serial setup for first remote control, if Serial/Simhub
#define SETUP_UI_REMOTE2_TYPE           Disabled        // Type of second remote control (Disabled, Serial, Simhub, WiFi, Ethernet)
#define SETUP_UI_REMOTE2_UART           Serial1         // Serial setup for second remote control, if Serial/Simhub
#define SETUP_UI_RC_NETWORKING_PORT     3333            // Remote controller networking port, if WiFi/Ethernet

// Device Pin Setup
#define SETUP_PH_METER_PIN              -1              // pH meter sensor pin (analog), else -1
#define SETUP_TDS_METER_PIN             -1              // TDS meter sensor pin (analog), else -1
#define SETUP_CO2_SENSOR_PIN            -1              // CO2 meter sensor pin (analog), else -1
#define SETUP_AC_USAGE_SENSOR_PIN       -1              // AC power usage meter sensor pin (analog), else -1
#define SETUP_DC_USAGE_SENSOR_PIN       -1              // DC power usage meter sensor pin (analog), else -1
#define SETUP_FLOW_RATE_SENSOR_PIN      -1              // Main feed pump flow rate sensor pin (analog/PWM), else -1
#define SETUP_DS18_WATER_TEMP_PIN       -1              // DS18* water temp sensor data pin (digital), else -1
#define SETUP_DHT_AIR_TEMP_HUMID_PIN    -1              // DHT* air temp sensor data pin (digital), else -1
#define SETUP_DHT_SENSOR_TYPE           None            // DHT sensor type enum (DHT11, DHT12, DHT21, DHT22, AM2301, None)
#define SETUP_VOL_FILLED_PIN            -1              // Water level filled indicator pin (digital/ISR), else -1
#define SETUP_VOL_EMPTY_PIN             -1              // Water level empty indicator pin (digital/ISR), else -1
#define SETUP_VOL_INDICATOR_TYPE        ACT_HIGH        // Water level indicator type/active level (ACT_HIGH, ACT_LOW)
#define SETUP_VOL_LEVEL_PIN             -1              // Water level sensor pin (analog)
#define SETUP_VOL_LEVEL_TYPE            Ultrasonic      // Water level device type (Ultrasonic, AnalogHeight)
#define SETUP_GROW_LIGHTS_PIN           -1              // Grow lights relay pin (digital), else -1
#define SETUP_WATER_AERATOR_PIN         -1              // Aerator relay pin (digital), else -1
#define SETUP_FEED_PUMP_PIN             -1              // Water level low indicator pin, else -1
#define SETUP_WATER_HEATER_PIN          -1              // Water heater relay pin (digital), else -1
#define SETUP_WATER_SPRAYER_PIN         -1              // Water sprayer relay pin (digital), else -1
#define SETUP_FAN_EXHAUST_PIN           -1              // Fan exhaust pin (digital/PWM), else -1
#define SETUP_FAN_EXHAUST_ESP_CHN       1               // Fan exhaust PWM channel, if on ESP
#define SETUP_FAN_EXHAUST_ESP_FRQ       1000            // Fan exhaust PWM frequency, if on ESP
#define SETUP_NUTRIENT_MIX_PIN          -1              // Nutrient premix peristaltic pump relay pin (digital), else -1
#define SETUP_FRESH_WATER_PIN           -1              // Fresh water peristaltic pump relay pin (digital), else -1
#define SETUP_PH_UP_PIN                 -1              // pH up solution peristaltic pump relay pin (digital), else -1
#define SETUP_PH_DOWN_PIN               -1              // pH down solution peristaltic pump relay pin (digital), else -1
#define SETUP_CROP_SOILM_PIN            -1              // Soil moisture sensor, for adaptive crop, pin (analog), else -1

// Device Multiplexing Setup
#define SETUP_MUXER_CHANNEL_BITS        -1              // Multiplexer channel bits (3 = 8-bit, 4 = 16-bit), else -1
#define SETUP_MUXER_ADDRESS_PINS        {hpin_none}     // Multiplexer addressing bus/channel pins, else {-1}
#define SETUP_MUXER_ENABLE_PIN          -1              // Multiplexer chip enable pin (optional), else -1
#define SETUP_MUXER_ENABLE_TYPE         ACT_LOW         // Multiplexer chip enable pin type/active level (ACT_HIGH, ACT_LOW)
#define SETUP_MUXER_ISR_PIN             -1              // Multiplexer interrupt pin, else -1
#define SETUP_MUXER_ISR_TYPE            ACT_LOW         // Multiplexer interrupt pin type/active level (ACT_HIGH, ACT_LOW)

// Device Pin Expanders Setup                           // (Note: Will redefine any used pins from channel setup below to a virtual pin = 100+chnl#)
#define SETUP_EXPANDER1_CHANNEL_BITS    -1              // Pin expander 1 channel bits (3 = 8-bit, 4 = 16-bit), else -1
#define SETUP_EXPANDER2_CHANNEL_BITS    -1              // Pin expander 2 channel bits (3 = 8-bit, 4 = 16-bit), else -1
#define SETUP_EXPANDER1_IOREF_I2C_ADDR  0x27            // Pin expander 1 full I2C device address (including device base offset)
#define SETUP_EXPANDER2_IOREF_I2C_ADDR  0x28            // Pin expander 2 full I2C device address (including device base offset)
#define SETUP_EXPANDER_IOREF_I2C_WIRE   Wire            // Pin expanders I2C wire class instance
#define SETUP_EXPANDER1_IOREF_ISR_PIN   -1              // Pin expander 1 interrupt pin, else -1
#define SETUP_EXPANDER2_IOREF_ISR_PIN   -1              // Pin expander 2 interrupt pin, else -1
#define SETUP_EXPANDER_IOREF_ISR_TYPE   ACT_LOW         // Pin expanders interrupt pin type/active level (ACT_HIGH, ACT_LOW)
// IORef allocation command using ioFrom* functions in IoAbstraction for pin expander 1
#define SETUP_EXPANDER1_IOREF_ALLOC()   ioFrom8575((uint8_t)SETUP_EXPANDER1_IOREF_I2C_ADDR, (pinid_t)SETUP_EXPANDER1_IOREF_ISR_PIN, &SETUP_EXPANDER_IOREF_I2C_WIRE)
// IORef allocation command using ioFrom* functions in IoAbstraction for pin expander 2
#define SETUP_EXPANDER2_IOREF_ALLOC()   ioFrom8575((uint8_t)SETUP_EXPANDER2_IOREF_I2C_ADDR, (pinid_t)SETUP_EXPANDER2_IOREF_ISR_PIN, &SETUP_EXPANDER_IOREF_I2C_WIRE)

// Pin Muxer/Expander Channel Setup                     // (Note: Only multiplexing or expanding may be done at the same time)
#define SETUP_PH_METER_PINCHNL          hpinchnl_none   // pH meter sensor pin muxer/expander channel #, else -127/none
#define SETUP_TDS_METER_PINCHNL         hpinchnl_none   // TDS meter sensor pin muxer/expander channel #, else -127/none
#define SETUP_CO2_SENSOR_PINCHNL        hpinchnl_none   // CO2 meter sensor pin muxer/expander channel #, else -127/none
#define SETUP_AC_USAGE_SENSOR_PINCHNL   hpinchnl_none   // AC power usage meter sensor pin muxer/expander channel #, else -127/none
#define SETUP_DC_USAGE_SENSOR_PINCHNL   hpinchnl_none   // DC power usage meter sensor pin muxer/expander channel #, else -127/none
#define SETUP_FLOW_RATE_SENSOR_PINCHNL  hpinchnl_none   // Main feed pump flow rate sensor pin muxer/expander channel #, else -127/none
#define SETUP_VOL_FILLED_PINCHNL        hpinchnl_none   // Water level filled indicator pin muxer/expander channel #, else -127/none
#define SETUP_VOL_EMPTY_PINCHNL         hpinchnl_none   // Water level empty indicator pin muxer/expander channel #, else -127/none
#define SETUP_VOL_LEVEL_PINCHNL         hpinchnl_none   // Water level sensor pin muxer/expander channel #, else -127/none
#define SETUP_GROW_LIGHTS_PINCHNL       hpinchnl_none   // Grow lights relay pin muxer/expander channel #, else -127/none
#define SETUP_WATER_AERATOR_PINCHNL     hpinchnl_none   // Aerator relay pin muxer/expander channel #, else -127/none
#define SETUP_FEED_PUMP_PINCHNL         hpinchnl_none   // Water level low indicator pin muxer/expander channel #, else -127/none
#define SETUP_WATER_HEATER_PINCHNL      hpinchnl_none   // Water heater relay pin muxer/expander channel #, else -127/none
#define SETUP_WATER_SPRAYER_PINCHNL     hpinchnl_none   // Water sprayer relay pin muxer/expander channel #, else -127/none
#define SETUP_FAN_EXHAUST_PINCHNL       hpinchnl_none   // Fan exhaust relay pin muxer/expander channel #, else -127/none
#define SETUP_NUTRIENT_MIX_PINCHNL      hpinchnl_none   // Nutrient premix peristaltic pump relay pin muxer/expander channel #, else -127/none
#define SETUP_FRESH_WATER_PINCHNL       hpinchnl_none   // Fresh water peristaltic pump relay pin muxer/expander channel #, else -127/none
#define SETUP_PH_UP_PINCHNL             hpinchnl_none   // pH up solution peristaltic pump relay pin muxer/expander channel #, else -127/none
#define SETUP_PH_DOWN_PINCHNL           hpinchnl_none   // pH down solution peristaltic pump relay pin muxer/expander channel #, else -127/none
#define SETUP_CROP_SOILM_PINCHNL        hpinchnl_none   // Soil moisture sensor, for adaptive crop, pin muxer/expander channel #, else -127/none

// System Setup
#define SETUP_AC_POWER_RAIL_TYPE        AC110V          // Rail power type used for actuator AC rail (AC110V, AC220V)
#define SETUP_DC_POWER_RAIL_TYPE        DC12V           // Rail power type used for actuator DC rail (DC3V3, DC5V, DC12V, DC24V, DC48V)
#define SETUP_AC_SUPPLY_POWER           0               // Maximum AC supply power wattage, else 0 if not known (-> use simple rails)
#define SETUP_DC_SUPPLY_POWER           0               // Maximum DC supply power wattage, else 0 if not known (-> use simple rails)
#define SETUP_FEED_RESERVOIR_SIZE       5               // Reservoir size, in default measurement units
#define SETUP_FEED_PUMP_FLOWRATE        20              // The base continuous flow rate of the main feed pumps, in L/min
#define SETUP_PERI_PUMP_FLOWRATE        0.070           // The base continuous flow rate of any peristaltic pumps, in L/min
#define SETUP_CROP_ON_TIME              15              // Minutes feeding pumps are to be turned on for (per feeding cycle)
#define SETUP_CROP_OFF_TIME             45              // Minutes feeding pumps are to be turned off for (per feeding cycle)
#define SETUP_CROP_TYPE                 Lettuce         // Type of crop planted, else Undefined
#define SETUP_CROP_SUBSTRATE            ClayPebbles     // Type of crop substrate, else Undefined
#define SETUP_CROP_NUMBER               1               // Number of plants in crop position (aka averaging weight)
#define SETUP_CROP_SOW_DATE             DateTime(2022, 5, 21) // Date that crop was planted at
#define SETUP_CROP_SOILM_FEED_LEVEL     0.1             // Soil moisture feeding trigger needs feeding level, for adaptive crop, in %
#define SETUP_CROP_SOILM_FED_LEVEL      0.75            // Soil moisture feeding trigger fed level, for adaptive crop, in %

#if defined(HYDRO_USE_WIFI)
WiFiClient netClient;
#elif defined(HYDRO_USE_ETHERNET)
EthernetClient netClient;
#endif
#ifdef HYDRO_USE_MQTT
MQTTClient mqttClient;
#endif

#if defined(HYDRO_USE_GUI) && (NOT_SETUP_AS(SETUP_CONTROL_IN_MODE, Disabled) || NOT_SETUP_AS(SETUP_DISPLAY_OUT_MODE, Disabled)) && NOT_SETUP_AS(SETUP_SYS_UI_MODE, Disabled)
#if IS_SETUP_AS(SETUP_SYS_UI_MODE, Minimal)
#include "min/HydruinoUI.h"
#elif IS_SETUP_AS(SETUP_SYS_UI_MODE, Full)
#include "full/HydruinoUI.h"
#endif
#if IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SSD1305) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SSD1305_x32Ada) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SSD1305_x64Ada) ||\
    IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SSD1306) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SH1106) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, CustomOLED) ||\
    IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SSD1607) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, IL3820) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, IL3820_V2)
// U8g2 Fonts
#define SETUP_UI_USE_MENU_FONT          u8g_font_unifont
#define SETUP_UI_USE_DETAIL_FONT        u8g_font_unifont
#elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, ST7735) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, ST7789) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, ILI9341) ||\
      IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, TFT)
// GFXfont/tcUnicode Fonts (AdafruitGFX/TFT_eSPI)
#if !SETUP_UI_USE_TCUNICODE_FONTS
#include "shared/tcMenu_Font_AdafruitGFXArial14.h"
#else
#include "shared/tcMenu_Font_tcUnicodeArial14.h"
#endif
#define SETUP_UI_USE_MENU_FONT          Arial14
#define SETUP_UI_USE_DETAIL_FONT        Arial14
#endif
#endif

#if SETUP_EXPANDER1_CHANNEL_BITS > 0 || SETUP_EXPANDER2_CHANNEL_BITS > 0
#include <IoAbstractionWire.h>
#endif

// Pre-init checks
#if (NOT_SETUP_AS(SETUP_SAVES_WIFISTORAGE_MODE, Disabled) || SETUP_DATA_WIFISTORAGE_ENABLE || SETUP_LOG_WIFISTORAGE_ENABLE) && !defined(HYDRO_USE_WIFI_STORAGE)
#warning The HYDRO_ENABLE_WIFI flag is expected to be defined as well as WiFiNINA_Generic.h included in order to run this sketch with WiFiStorage features enabled
#endif
#if (NOT_SETUP_AS(SETUP_SAVES_SD_CARD_MODE, Disabled) || SETUP_DATA_SD_ENABLE || SETUP_LOG_SD_ENABLE || SETUP_EXTDATA_SD_ENABLE) && SETUP_SD_CARD_SPI_CS == -1
#warning The SETUP_SD_CARD_SPI_CS define is expected to be set to a valid pin in order to run this sketch with SD card features enabled
#endif
#if (NOT_SETUP_AS(SETUP_SAVES_EEPROM_MODE, Disabled) || SETUP_EXTDATA_EEPROM_ENABLE) && IS_SETUP_AS(SETUP_EEPROM_DEVICE_TYPE, None)
#warning The SETUP_EEPROM_DEVICE_TYPE define is expected to be set to a valid size in order to run this sketch with EEPROM features enabled
#endif

pintype_t SETUP_CTRL_INPUT_PINS_[] = SETUP_CTRL_INPUT_PINS;
Hydruino hydroController((pintype_t)SETUP_PIEZO_BUZZER_PIN,
                         JOIN(Hydro_EEPROMType,SETUP_EEPROM_DEVICE_TYPE),
                         I2CDeviceSetup((uint8_t)SETUP_EEPROM_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
                         JOIN(Hydro_RTCType,SETUP_RTC_DEVICE_TYPE),
                         I2CDeviceSetup((uint8_t)0b000, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
                         SPIDeviceSetup((pintype_t)SETUP_SD_CARD_SPI_CS, &SETUP_SD_CARD_SPI, SETUP_SD_CARD_SPI_SPEED),
#if defined(HYDRO_USE_AT_WIFI)
                         UARTDeviceSetup(&SETUP_WIFI_SERIAL, HYDRO_SYS_ATWIFI_SERIALBAUD),
#elif defined(HYDRO_USE_WIFI)
                         SPIDeviceSetup((pintype_t)SETUP_WIFI_SPI_CS, &SETUP_WIFI_SPI),
#elif defined(HYDRO_USE_ETHERNET)
                         SPIDeviceSetup((pintype_t)SETUP_ETHERNET_SPI_CS, &SETUP_ETHERNET_SPI),
#else
                         DeviceSetup(),
#endif
#if defined(HYDRO_USE_GPS) && (IS_SETUP_AS(SETUP_GPS_TYPE, UART) || IS_SETUP_AS(SETUP_GPS_TYPE, Serial))
                         UARTDeviceSetup(&SETUP_GPS_SERIAL, HYDRO_SYS_NMEAGPS_SERIALBAUD),
#elif defined(HYDRO_USE_GPS) && IS_SETUP_AS(SETUP_GPS_TYPE, I2C)
                         I2CDeviceSetup(SETUP_GPS_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
#elif defined(HYDRO_USE_GPS) && IS_SETUP_AS(SETUP_GPS_TYPE, SPI)
                         SPIDeviceSetup(SETUP_GPS_SPI_CS, &SETUP_GPS_SPI),
#else
                         DeviceSetup(),
#endif
                         SETUP_CTRL_INPUT_PINS_,
#if SETUP_DISP_SPI_CS >= 0
                         SPIDeviceSetup(SETUP_DISP_SPI_CS, &SETUP_DISP_SPI, SETUP_DISP_SPI_SPEED)
#elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, LCD16x2_EN) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, LCD16x2_RS) ||\
      IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, LCD20x4_EN) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, LCD20x4_RS) ||\
      SETUP_UI_IS_DFROBOTSHIELD
                         I2CDeviceSetup((uint8_t)SETUP_DISP_LCD_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED)
#else
                         I2CDeviceSetup((uint8_t)SETUP_DISP_OLED_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED)
#endif
                         );

#if SETUP_GROW_LIGHTS_PIN >= 0 || SETUP_WATER_AERATOR_PIN >= 0 ||  SETUP_FEED_PUMP_PIN >= 0 || SETUP_WATER_HEATER_PIN >= 0 || SETUP_WATER_SPRAYER_PIN >= 0 || SETUP_FAN_EXHAUST_PIN >= 0
#define SETUP_USE_AC_RAIL
#endif
#if SETUP_NUTRIENT_MIX_PIN >= 0 || SETUP_FRESH_WATER_PIN >= 0 ||  SETUP_PH_UP_PIN >= 0 || SETUP_PH_DOWN_PIN >= 0 || SETUP_FAN_EXHAUST_PIN >= 0
#define SETUP_USE_DC_RAIL
#endif
#define SETUP_USE_ONEWIRE_BITRES    12

inline void setupPinChannels()
{
    #if SETUP_MUXER_CHANNEL_BITS >= 0
        pintype_t _SETUP_MUXER_ADDRESS_PINS[] = SETUP_MUXER_ADDRESS_PINS;
        HydroDigitalPin chipEnable(SETUP_MUXER_ENABLE_PIN, OUTPUT, SETUP_MUXER_ENABLE_TYPE);
        HydroDigitalPin interrupt(SETUP_MUXER_ISR_PIN, INPUT, SETUP_MUXER_ISR_TYPE);
        #if SETUP_PH_METER_PINCHNL >= 0 && SETUP_PH_METER_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_PH_METER_PIN)) { hydroController.setPinMuxer(SETUP_PH_METER_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_PH_METER_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_TDS_METER_PINCHNL >= 0 && SETUP_TDS_METER_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_TDS_METER_PIN)) { hydroController.setPinMuxer(SETUP_TDS_METER_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_TDS_METER_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_CO2_SENSOR_PINCHNL >= 0 && SETUP_CO2_SENSOR_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_CO2_SENSOR_PIN)) { hydroController.setPinMuxer(SETUP_CO2_SENSOR_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_CO2_SENSOR_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_AC_USAGE_SENSOR_PINCHNL >= 0 && SETUP_AC_USAGE_SENSOR_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_AC_USAGE_SENSOR_PIN)) { hydroController.setPinMuxer(SETUP_AC_USAGE_SENSOR_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_AC_USAGE_SENSOR_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_DC_USAGE_SENSOR_PINCHNL >= 0 && SETUP_DC_USAGE_SENSOR_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_DC_USAGE_SENSOR_PIN)) { hydroController.setPinMuxer(SETUP_DC_USAGE_SENSOR_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_DC_USAGE_SENSOR_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_FLOW_RATE_SENSOR_PINCHNL >= 0 && SETUP_FLOW_RATE_SENSOR_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_FLOW_RATE_SENSOR_PIN)) { hydroController.setPinMuxer(SETUP_FLOW_RATE_SENSOR_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_FLOW_RATE_SENSOR_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_VOL_FILLED_PINCHNL >= 0 && SETUP_VOL_FILLED_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_VOL_FILLED_PIN)) { hydroController.setPinMuxer(SETUP_VOL_FILLED_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_VOL_FILLED_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_VOL_EMPTY_PINCHNL >= 0 && SETUP_VOL_EMPTY_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_VOL_EMPTY_PIN)) { hydroController.setPinMuxer(SETUP_VOL_EMPTY_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_VOL_EMPTY_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_VOL_LEVEL_PINCHNL >= 0 && SETUP_VOL_LEVEL_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_VOL_LEVEL_PIN)) { hydroController.setPinMuxer(SETUP_VOL_LEVEL_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_VOL_LEVEL_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_GROW_LIGHTS_PINCHNL >= 0 && SETUP_GROW_LIGHTS_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_GROW_LIGHTS_PIN)) { hydroController.setPinMuxer(SETUP_GROW_LIGHTS_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_GROW_LIGHTS_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_WATER_AERATOR_PINCHNL >= 0 && SETUP_WATER_AERATOR_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_WATER_AERATOR_PIN)) { hydroController.setPinMuxer(SETUP_WATER_AERATOR_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_WATER_AERATOR_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_FEED_PUMP_PINCHNL >= 0 && SETUP_FEED_PUMP_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_FEED_PUMP_PIN)) { hydroController.setPinMuxer(SETUP_FEED_PUMP_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_FEED_PUMP_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_WATER_HEATER_PINCHNL >= 0 && SETUP_WATER_HEATER_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_WATER_HEATER_PIN)) { hydroController.setPinMuxer(SETUP_WATER_HEATER_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_WATER_HEATER_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_WATER_SPRAYER_PINCHNL >= 0 && SETUP_WATER_SPRAYER_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_WATER_SPRAYER_PIN)) { hydroController.setPinMuxer(SETUP_WATER_SPRAYER_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_WATER_SPRAYER_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_FAN_EXHAUST_PINCHNL >= 0 && SETUP_FAN_EXHAUST_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_FAN_EXHAUST_PIN)) { hydroController.setPinMuxer(SETUP_FAN_EXHAUST_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_FAN_EXHAUST_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_NUTRIENT_MIX_PINCHNL >= 0 && SETUP_NUTRIENT_MIX_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_NUTRIENT_MIX_PIN)) { hydroController.setPinMuxer(SETUP_NUTRIENT_MIX_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_NUTRIENT_MIX_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_FRESH_WATER_PINCHNL >= 0 && SETUP_FRESH_WATER_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_FRESH_WATER_PIN)) { hydroController.setPinMuxer(SETUP_FRESH_WATER_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_FRESH_WATER_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_PH_UP_PINCHNL >= 0 && SETUP_PH_UP_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_PH_UP_PIN)) { hydroController.setPinMuxer(SETUP_PH_UP_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_PH_UP_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_PH_DOWN_PINCHNL >= 0 && SETUP_PH_DOWN_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_PH_DOWN_PIN)) { hydroController.setPinMuxer(SETUP_PH_DOWN_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_PH_DOWN_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
        #if SETUP_CROP_SOILM_PINCHNL >= 0 && SETUP_CROP_SOILM_PIN >= 0
            if (!hydroController.getPinMuxer(SETUP_CROP_SOILM_PIN)) { hydroController.setPinMuxer(SETUP_CROP_SOILM_PIN, SharedPtr<HydroPinMuxer>(new HydroPinMuxer(SETUP_CROP_SOILM_PIN, _SETUP_MUXER_ADDRESS_PINS, SETUP_MUXER_CHANNEL_BITS, chipEnable, interrupt))); }
        #endif
    #elif SETUP_EXPANDER1_CHANNEL_BITS > 0 || SETUP_EXPANDER2_CHANNEL_BITS > 0
        auto expanders[] = {
            #if SETUP_EXPANDER1_CHANNEL_BITS > 0
                SharedPtr<HydroPinExpander>(new HydroPinExpander(0, SETUP_EXPANDER1_CHANNEL_BITS, SETUP_EXPANDER1_IOREF_ALLOC(), HydroDigitalPin(SETUP_EXPANDER1_IOREF_ISR_PIN, INPUT, SETUP_EXPANDER_IOREF_ISR_TYPE))),
            #else
                SharedPtr<HydroPinExpander>(nullptr),
            #endif
            #if SETUP_EXPANDER2_CHANNEL_BITS > 0
                SharedPtr<HydroPinExpander>(new HydroPinExpander(1, SETUP_EXPANDER2_CHANNEL_BITS, SETUP_EXPANDER2_IOREF_ALLOC(), HydroDigitalPin(SETUP_EXPANDER2_IOREF_ISR_PIN, INPUT, SETUP_EXPANDER_IOREF_ISR_TYPE)))
            #else
                SharedPtr<HydroPinExpander>(nullptr)
            #endif
        };
        // To setup control input pins as part of an expander, use pin #'s 100+, which are treated as virtual pins representing an expander index ((#-100) /16) and offset ((#-100) %16).
        if (isValidPin(SETUP_CTRL_INPUT_PINS_[0]) && SETUP_CTRL_INPUT_PINS_[0] >= hpin_virtual && !hydroController.getPinExpander(expanderPosForPinNumber(SETUP_CTRL_INPUT_PINS_[0]))) {
            hydroController.setPinExpander(expanderPosForPinNumber(SETUP_CTRL_INPUT_PINS_[0]), expanders[expanderPosForPinNumber(SETUP_CTRL_INPUT_PINS_[0])]);
        }
        #if SETUP_PH_METER_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_PH_METER_PINCHNL/16)) { hydroController.setPinExpander(SETUP_PH_METER_PINCHNL/16, expanders[SETUP_PH_METER_PINCHNL/16]); }
            #undef SETUP_PH_METER_PIN
            #define SETUP_PH_METER_PIN (100 + SETUP_PH_METER_PINCHNL)
        #endif
        #if SETUP_TDS_METER_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_TDS_METER_PINCHNL/16)) { hydroController.setPinExpander(SETUP_TDS_METER_PINCHNL/16, expanders[SETUP_TDS_METER_PINCHNL/16]); }
            #undef SETUP_TDS_METER_PIN
            #define SETUP_TDS_METER_PIN (100 + SETUP_TDS_METER_PINCHNL)
        #endif
        #if SETUP_CO2_SENSOR_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_CO2_SENSOR_PINCHNL/16)) { hydroController.setPinExpander(SETUP_CO2_SENSOR_PINCHNL/16, expanders[SETUP_CO2_SENSOR_PINCHNL/16]); }
            #undef SETUP_CO2_SENSOR_PIN
            #define SETUP_CO2_SENSOR_PIN (100 + SETUP_CO2_SENSOR_PINCHNL)
        #endif
        #if SETUP_AC_USAGE_SENSOR_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_AC_USAGE_SENSOR_PINCHNL/16)) { hydroController.setPinExpander(SETUP_AC_USAGE_SENSOR_PINCHNL/16, expanders[SETUP_AC_USAGE_SENSOR_PINCHNL/16]); }
            #undef SETUP_AC_USAGE_SENSOR_PIN
            #define SETUP_AC_USAGE_SENSOR_PIN (100 + SETUP_AC_USAGE_SENSOR_PINCHNL)
        #endif
        #if SETUP_DC_USAGE_SENSOR_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_DC_USAGE_SENSOR_PINCHNL/16)) { hydroController.setPinExpander(SETUP_DC_USAGE_SENSOR_PINCHNL/16, expanders[SETUP_DC_USAGE_SENSOR_PINCHNL/16]); }
            #undef SETUP_DC_USAGE_SENSOR_PIN
            #define SETUP_DC_USAGE_SENSOR_PIN (100 + SETUP_DC_USAGE_SENSOR_PINCHNL)
        #endif
        #if SETUP_FLOW_RATE_SENSOR_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_FLOW_RATE_SENSOR_PINCHNL/16)) { hydroController.setPinExpander(SETUP_FLOW_RATE_SENSOR_PINCHNL/16, expanders[SETUP_FLOW_RATE_SENSOR_PINCHNL/16]); }
            #undef SETUP_FLOW_RATE_SENSOR_PIN
            #define SETUP_FLOW_RATE_SENSOR_PIN (100 + SETUP_FLOW_RATE_SENSOR_PINCHNL)
        #endif
        #if SETUP_VOL_FILLED_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_VOL_FILLED_PINCHNL/16)) { hydroController.setPinExpander(SETUP_VOL_FILLED_PINCHNL/16, expanders[SETUP_VOL_FILLED_PINCHNL/16]); }
            #undef SETUP_VOL_FILLED_PIN
            #define SETUP_VOL_FILLED_PIN (100 + SETUP_VOL_FILLED_PINCHNL)
        #endif
        #if SETUP_VOL_EMPTY_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_VOL_EMPTY_PINCHNL/16)) { hydroController.setPinExpander(SETUP_VOL_EMPTY_PINCHNL/16, expanders[SETUP_VOL_EMPTY_PINCHNL/16]); }
            #undef SETUP_VOL_EMPTY_PIN
            #define SETUP_VOL_EMPTY_PIN (100 + SETUP_VOL_EMPTY_PINCHNL)
        #endif
        #if SETUP_VOL_LEVEL_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_VOL_LEVEL_PINCHNL/16)) { hydroController.setPinExpander(SETUP_VOL_LEVEL_PINCHNL/16, expanders[SETUP_VOL_LEVEL_PINCHNL/16]); }
            #undef SETUP_VOL_LEVEL_PIN
            #define SETUP_VOL_LEVEL_PIN (100 + SETUP_VOL_LEVEL_PINCHNL)
        #endif
        #if SETUP_GROW_LIGHTS_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_GROW_LIGHTS_PINCHNL/16)) { hydroController.setPinExpander(SETUP_GROW_LIGHTS_PINCHNL/16, expanders[SETUP_GROW_LIGHTS_PINCHNL/16]); }
            #undef SETUP_GROW_LIGHTS_PIN
            #define SETUP_GROW_LIGHTS_PIN (100 + SETUP_GROW_LIGHTS_PINCHNL)
        #endif
        #if SETUP_WATER_AERATOR_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_WATER_AERATOR_PINCHNL/16)) { hydroController.setPinExpander(SETUP_WATER_AERATOR_PINCHNL/16, expanders[SETUP_WATER_AERATOR_PINCHNL/16]); }
            #undef SETUP_WATER_AERATOR_PIN
            #define SETUP_WATER_AERATOR_PIN (100 + SETUP_WATER_AERATOR_PINCHNL)
        #endif
        #if SETUP_FEED_PUMP_PINCHNL >= 0
            if (!hydroController.getPinExpander(SETUP_FEED_PUMP_PINCHNL/16)) { hydroController.setPinExpander(SETUP_FEED_PUMP_PINCHNL/16, expanders[SETUP_FEED_PUMP_PINCHNL/16]); }
            #undef SETUP_FEED_PUMP_PIN
            #define SETUP_FEED_PUMP_PIN (100 + SETUP_FEED_PUMP_PINCHNL)
        #endif
        #if SETUP_WATER_HEATER_PINCHNL >= 0 || SETUP_WATER_HEATER_PIN >= 100
            if (!hydroController.getPinExpander(SETUP_WATER_HEATER_PINCHNL/16)) { hydroController.setPinExpander(SETUP_WATER_HEATER_PINCHNL/16, expanders[SETUP_WATER_HEATER_PINCHNL/16]); }
            #undef SETUP_WATER_HEATER_PIN
            #define SETUP_WATER_HEATER_PIN (100 + SETUP_WATER_HEATER_PINCHNL)
        #endif
        #if SETUP_WATER_SPRAYER_PINCHNL >= 0 || SETUP_WATER_SPRAYER_PIN >= 100
            if (!hydroController.getPinExpander(SETUP_WATER_SPRAYER_PINCHNL/16)) { hydroController.setPinExpander(SETUP_WATER_SPRAYER_PINCHNL/16, expanders[SETUP_WATER_SPRAYER_PINCHNL/16]); }
            #undef SETUP_WATER_SPRAYER_PIN
            #define SETUP_WATER_SPRAYER_PIN (100 + SETUP_WATER_SPRAYER_PINCHNL)
        #endif
        #if SETUP_FAN_EXHAUST_PINCHNL >= 0 || SETUP_FAN_EXHAUST_PIN >= 100
            if (!hydroController.getPinExpander(SETUP_FAN_EXHAUST_PINCHNL/16)) { hydroController.setPinExpander(SETUP_FAN_EXHAUST_PINCHNL/16, expanders[SETUP_FAN_EXHAUST_PINCHNL/16]); }
            #undef SETUP_FAN_EXHAUST_PIN
            #define SETUP_FAN_EXHAUST_PIN (100 + SETUP_FAN_EXHAUST_PINCHNL)
        #endif
        #if SETUP_NUTRIENT_MIX_PINCHNL >= 0 || SETUP_NUTRIENT_MIX_PIN >= 100
            if (!hydroController.getPinExpander(SETUP_NUTRIENT_MIX_PINCHNL/16)) { hydroController.setPinExpander(SETUP_NUTRIENT_MIX_PINCHNL/16, expanders[SETUP_NUTRIENT_MIX_PINCHNL/16]); }
            #undef SETUP_NUTRIENT_MIX_PIN
            #define SETUP_NUTRIENT_MIX_PIN (100 + SETUP_NUTRIENT_MIX_PINCHNL)
        #endif
        #if SETUP_FRESH_WATER_PINCHNL >= 0 || SETUP_FRESH_WATER_PIN >= 100
            if (!hydroController.getPinExpander(SETUP_FRESH_WATER_PINCHNL/16)) { hydroController.setPinExpander(SETUP_FRESH_WATER_PINCHNL/16, expanders[SETUP_FRESH_WATER_PINCHNL/16]); }
            #undef SETUP_FRESH_WATER_PIN
            #define SETUP_FRESH_WATER_PIN (100 + SETUP_FRESH_WATER_PINCHNL)
        #endif
        #if SETUP_PH_UP_PINCHNL >= 0 || SETUP_PH_UP_PIN >= 100
            if (!hydroController.getPinExpander(SETUP_PH_UP_PINCHNL/16)) { hydroController.setPinExpander(SETUP_PH_UP_PINCHNL/16, expanders[SETUP_PH_UP_PINCHNL/16]); }
            #undef SETUP_PH_UP_PIN
            #define SETUP_PH_UP_PIN (100 + SETUP_PH_UP_PINCHNL)
        #endif
        #if SETUP_PH_DOWN_PINCHNL >= 0 || SETUP_PH_DOWN_PIN >= 100
            if (!hydroController.getPinExpander(SETUP_PH_DOWN_PINCHNL/16)) { hydroController.setPinExpander(SETUP_PH_DOWN_PINCHNL/16, expanders[SETUP_PH_DOWN_PINCHNL/16]); }
            #undef SETUP_PH_DOWN_PIN
            #define SETUP_PH_DOWN_PIN (100 + SETUP_PH_DOWN_PINCHNL)
        #endif
        #if SETUP_CROP_SOILM_PINCHNL >= 0 || SETUP_CROP_SOILM_PIN >= 100
            if (!hydroController.getPinExpander(SETUP_CROP_SOILM_PINCHNL/16)) { hydroController.setPinExpander(SETUP_CROP_SOILM_PINCHNL/16, expanders[SETUP_CROP_SOILM_PINCHNL/16]); }
            #undef SETUP_CROP_SOILM_PIN
            #define SETUP_CROP_SOILM_PIN (100 + SETUP_CROP_SOILM_PINCHNL)
        #endif
    #endif
}

inline void setupOnce()
{
    hydroController.setSystemName(F(SETUP_SYS_NAME));
    hydroController.setTimeZoneOffset(SETUP_SYS_TIMEZONE);
    #ifdef HYDRO_USE_WIFI
    {   String wifiSSID = F(SETUP_WIFI_SSID);
        String wifiPassword = F(SETUP_WIFI_PASS);
        hydroController.setWiFiConnection(wifiSSID, wifiPassword);
    }
    #endif
    #ifdef HYDRO_USE_ETHERNET
    {   uint8_t _SETUP_ETHERNET_MAC[] = SETUP_ETHERNET_MAC;
        hydroController.setEthernetConnection(_SETUP_ETHERNET_MAC);
    }
    #endif
    getLogger()->setLogLevel(JOIN(Hydro_LogLevel,SETUP_SYS_LOGLEVEL));
    #ifndef HYDRO_USE_GPS
        hydroController.setSystemLocation(SETUP_SYS_STATIC_LAT, SETUP_SYS_STATIC_LONG, SETUP_SYS_STATIC_ALT);
    #endif
    #if defined(HYDRO_USE_WIFI_STORAGE) && IS_SETUP_AS(SETUP_SAVES_WIFISTORAGE_MODE, Primary)
        hydroController.setAutosaveEnabled(Hydro_Autosave_EnabledToWiFiStorageJson
    #elif SETUP_SD_CARD_SPI_CS >= 0 && IS_SETUP_AS(SETUP_SAVES_SD_CARD_MODE, Primary)
        hydroController.setAutosaveEnabled(Hydro_Autosave_EnabledToSDCardJson
    #elif NOT_SETUP_AS(SETUP_EEPROM_DEVICE_TYPE, None) && IS_SETUP_AS(SETUP_SAVES_EEPROM_MODE, Primary)
        hydroController.setAutosaveEnabled(Hydro_Autosave_EnabledToEEPROMRaw
    #else
        hydroController.setAutosaveEnabled(Hydro_Autosave_Disabled
    #endif
    #if defined(HYDRO_USE_WIFI_STORAGE) && IS_SETUP_AS(SETUP_SAVES_WIFISTORAGE_MODE, Fallback)
        , Hydro_Autosave_EnabledToWiFiStorageJson);
    #elif SETUP_SD_CARD_SPI_CS >= 0 && IS_SETUP_AS(SETUP_SAVES_SD_CARD_MODE, Fallback)
        , Hydro_Autosave_EnabledToSDCardJson);
    #elif NOT_SETUP_AS(SETUP_EEPROM_DEVICE_TYPE, None) && IS_SETUP_AS(SETUP_SAVES_EEPROM_MODE, Fallback)
        , Hydro_Autosave_EnabledToEEPROMRaw);
    #else
        );
    #endif
}

inline void setupAlways()
{
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
            #if IS_SETUP_AS(SETUP_MQTT_BROKER_CONNECT_BY, Hostname)
                mqttClient.begin(String(F(SETUP_MQTT_BROKER_HOSTNAME)).c_str(), SETUP_MQTT_BROKER_PORT, netClient);
            #elif IS_SETUP_AS(SETUP_MQTT_BROKER_CONNECT_BY, IPAddress)
            {   uint8_t ipAddr[4] = SETUP_MQTT_BROKER_IPADDR;
                IPAddress ip(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
                mqttClient.begin(ip, SETUP_MQTT_BROKER_PORT, netClient);
            }
            #endif
            hydroController.enableDataPublishingToMQTTClient(mqttClient);
        }
    #endif
    #ifdef HYDRO_USE_GPS
        hydroController.getGPS()->sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    #endif
}

inline void setupObjects()
{
    // Base Objects
    #ifdef SETUP_USE_AC_RAIL
        #if SETUP_AC_SUPPLY_POWER
            auto acRelayPower = hydroController.addRegulatedPowerRail(JOIN(Hydro_RailType,SETUP_AC_POWER_RAIL_TYPE),SETUP_AC_SUPPLY_POWER);
            #if SETUP_AC_USAGE_SENSOR_PIN >= 0
            {   auto powerMeter = hydroController.addPowerLevelMeter(SETUP_AC_USAGE_SENSOR_PIN, ADC_RESOLUTION, SETUP_AC_USAGE_SENSOR_PINCHNL);
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
            #if SETUP_DC_USAGE_SENSOR_PIN >= 0
            {   auto powerMeter = hydroController.addPowerLevelMeter(SETUP_DC_USAGE_SENSOR_PIN, ADC_RESOLUTION, SETUP_DC_USAGE_SENSOR_PINCHNL);
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
                auto moistureSensor = hydroController.addAnalogMoistureSensor(SETUP_CROP_SOILM_PIN, ADC_RESOLUTION, SETUP_CROP_SOILM_PINCHNL);
                auto crop = hydroController.addAdaptiveFedCrop(JOIN(Hydro_CropType,SETUP_CROP_TYPE),
                                                                JOIN(Hydro_SubstrateType,SETUP_CROP_SUBSTRATE),
                                                                SETUP_CROP_SOW_DATE);
                moistureSensor->setParentCrop(crop);
                crop->setSoilMoistureSensor(moistureSensor);
                crop->setFeedingTrigger(new HydroMeasurementValueTrigger(moistureSensor, SETUP_CROP_SOILM_FEED_LEVEL, ACT_BELOW, 0, SETUP_CROP_SOILM_FED_LEVEL - SETUP_CROP_SOILM_FEED_LEVEL, 30000));
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
    {   auto phMeter = hydroController.addAnalogPhMeter(SETUP_PH_METER_PIN, ADC_RESOLUTION, SETUP_PH_METER_PINCHNL);
        phMeter->setParentReservoir(feedReservoir);
        feedReservoir->setWaterPHSensor(phMeter);
    }
    #endif
    #if SETUP_TDS_METER_PIN >= 0
    {   auto tdsElectrode = hydroController.addAnalogTDSElectrode(SETUP_TDS_METER_PIN, ADC_RESOLUTION, SETUP_TDS_METER_PINCHNL);
        tdsElectrode->setParentReservoir(feedReservoir);
        feedReservoir->setWaterTDSSensor(tdsElectrode);
    }
    #endif
    #if SETUP_CO2_SENSOR_PIN >= 0
    {   auto co2Sensor = hydroController.addAnalogCO2Sensor(SETUP_CO2_SENSOR_PIN, ADC_RESOLUTION, SETUP_CO2_SENSOR_PINCHNL);
        co2Sensor->setParentReservoir(feedReservoir);
        feedReservoir->setAirCO2Sensor(co2Sensor);
    }
    #endif
    #if SETUP_FLOW_RATE_SENSOR_PIN >= 0
    {   auto flowSensor = hydroController.addAnalogPumpFlowSensor(SETUP_FLOW_RATE_SENSOR_PIN, ADC_RESOLUTION, SETUP_FLOW_RATE_SENSOR_PINCHNL);
        flowSensor->setParentReservoir(feedReservoir);
        // will be set to main feed pump later via delayed ref
    }
    #endif

    // Digital Sensors
    #if SETUP_DS18_WATER_TEMP_PIN >= 0
    {   auto dsTemperatureSensor = hydroController.addDSTemperatureSensor(SETUP_DS18_WATER_TEMP_PIN, SETUP_USE_ONEWIRE_BITRES);
        dsTemperatureSensor->setParentReservoir(feedReservoir);
        feedReservoir->setWaterTemperatureSensor(dsTemperatureSensor);
    }
    #endif
    #if SETUP_DHT_AIR_TEMP_HUMID_PIN >= 0
    {   auto dhtTemperatureSensor = hydroController.addDHTTempHumiditySensor(SETUP_DHT_AIR_TEMP_HUMID_PIN, JOIN(Hydro_DHTType,SETUP_DHT_SENSOR_TYPE));
        dhtTemperatureSensor->setParentReservoir(feedReservoir);
        feedReservoir->setAirTemperatureSensor(dhtTemperatureSensor);
    }
    #endif

    // Binary->Volume Sensors
    #if SETUP_VOL_FILLED_PIN >= 0
    {   auto filledIndicator = hydroController.addLevelIndicator(SETUP_VOL_FILLED_PIN, SETUP_VOL_INDICATOR_TYPE, SETUP_VOL_FILLED_PINCHNL);
        filledIndicator->setParentReservoir(feedReservoir);
        feedReservoir->setFilledTrigger(new HydroMeasurementValueTrigger(filledIndicator, 0.5, ACT_ABOVE));
    }
    #endif
    #if SETUP_VOL_EMPTY_PIN >= 0
    {   auto emptyIndicator = hydroController.addLevelIndicator(SETUP_VOL_EMPTY_PIN, SETUP_VOL_INDICATOR_TYPE, SETUP_VOL_EMPTY_PINCHNL);
        emptyIndicator->setParentReservoir(feedReservoir);
        feedReservoir->setEmptyTrigger(new HydroMeasurementValueTrigger(emptyIndicator, 0.5, ACT_ABOVE));
    }
    #endif

    // Distance->Volume Sensors
    #if SETUP_VOL_LEVEL_PIN >= 0
        #if SETUP_VOL_LEVEL_TYPE == Ultrasonic
        {   auto distanceSensor = hydroController.addUltrasonicDistanceSensor(SETUP_VOL_LEVEL_PIN, ADC_RESOLUTION, SETUP_VOL_LEVEL_PINCHNL);
            distanceSensor->setParentReservoir(feedReservoir);
            feedReservoir->setWaterVolumeSensor(distanceSensor);
            #if SETUP_VOL_FILLED_PIN < 0
                feedReservoir->setFilledTrigger(new HydroMeasurementValueTrigger(distanceSensor, HYDRO_FEEDRES_FRACTION_FILLED, ACT_ABOVE));
            #endif
            #if SETUP_VOL_EMPTY_PIN < 0
                feedReservoir->setEmptyTrigger(new HydroMeasurementValueTrigger(distanceSensor, HYDRO_FEEDRES_FRACTION_EMPTY, ACT_BELOW));
            #endif
        }
        #elif SETUP_VOL_LEVEL_TYPE == AnalogHeight
        {   auto heightMeter = hydroController.addAnalogWaterHeightMeter(SETUP_VOL_LEVEL_PIN, ADC_RESOLUTION, SETUP_VOL_LEVEL_PINCHNL);
            heightMeter->setParentReservoir(feedReservoir);
            feedReservoir->setWaterVolumeSensor(heightMeter);
            #if SETUP_VOL_FILLED_PIN < 0
                feedReservoir->setFilledTrigger(new HydroMeasurementValueTrigger(heightMeter, HYDRO_FEEDRES_FRACTION_FILLED, ACT_ABOVE));
            #endif
            #if SETUP_VOL_EMPTY_PIN < 0
                feedReservoir->setEmptyTrigger(new HydroMeasurementValueTrigger(heightMeter, HYDRO_FEEDRES_FRACTION_EMPTY, ACT_BELOW));
            #endif
        }
        #endif
    #endif

    // AC-Based Actuators
    #if SETUP_GROW_LIGHTS_PIN >= 0
    {   auto growLights = hydroController.addGrowLightsRelay(SETUP_GROW_LIGHTS_PIN, SETUP_GROW_LIGHTS_PINCHNL);
        growLights->setParentRail(acRelayPower);
        growLights->setParentReservoir(feedReservoir);
    }
    #endif
    #if SETUP_WATER_AERATOR_PIN >= 0
    {   auto aerator = hydroController.addWaterAeratorRelay(SETUP_WATER_AERATOR_PIN, SETUP_WATER_AERATOR_PINCHNL);
        aerator->setParentRail(acRelayPower);
        aerator->setParentReservoir(feedReservoir);
    }
    #endif
    #if SETUP_FEED_PUMP_PIN >= 0
    {   auto feedPump = hydroController.addWaterPumpRelay(SETUP_FEED_PUMP_PIN, SETUP_FEED_PUMP_PINCHNL);
        feedPump->setParentRail(acRelayPower);
        feedPump->setSourceReservoir(feedReservoir);
        #if SETUP_FLOW_RATE_SENSOR_PIN >= 0
            feedPump->setFlowRateSensor(HydroIdentity(Hydro_SensorType_PumpFlow, 1)); // delayed ref (auto-resolves on launch)
        #endif
        if (hydroController.getSystemMode() == Hydro_SystemMode_DrainToWaste) {
            feedPump->setDestinationReservoir(drainagePipe);
        } else {
            feedPump->setDestinationReservoir(feedReservoir);
        }
        feedPump->setContinuousFlowRate(HydroSingleMeasurement(SETUP_FEED_PUMP_FLOWRATE, Hydro_UnitsType_LiqFlowRate_LitersPerMin));
    }
    #endif
    #if SETUP_WATER_HEATER_PIN >= 0
    {   auto heater = hydroController.addWaterHeaterRelay(SETUP_WATER_HEATER_PIN, SETUP_WATER_HEATER_PINCHNL);
        heater->setParentRail(acRelayPower);
        heater->setParentReservoir(feedReservoir);
    }
    #endif
    #if SETUP_WATER_SPRAYER_PIN >= 0
    {   auto sprayer = hydroController.addWaterSprayerRelay(SETUP_WATER_SPRAYER_PIN, SETUP_WATER_SPRAYER_PINCHNL);
        sprayer->setParentRail(acRelayPower);
        sprayer->setParentReservoir(feedReservoir);
    }
    #endif
    #if SETUP_FAN_EXHAUST_PIN >= 0
    if (checkPinIsPWMOutput(SETUP_FAN_EXHAUST_PIN)) {
        auto fanExhaust = hydroController.addAnalogFanExhaust(SETUP_FAN_EXHAUST_PIN, ADC_RESOLUTION,
#ifdef ESP32
                                                              SETUP_FAN_EXHAUST_ESP_CHN,
#endif
#ifdef ESP_PLATFORM
                                                              SETUP_FAN_EXHAUST_ESP_FRQ,
#endif
                                                              SETUP_FAN_EXHAUST_PINCHNL);
        fanExhaust->setParentRail(dcRelayPower);            // PWM fans use DC relay
        fanExhaust->setParentReservoir(feedReservoir);
    } else {
        auto fanExhaust = hydroController.addFanExhaustRelay(SETUP_FAN_EXHAUST_PIN, SETUP_FAN_EXHAUST_PINCHNL);
        fanExhaust->setParentRail(acRelayPower);
        fanExhaust->setParentReservoir(feedReservoir);
    }
    #endif

    // DC-Based Peristaltic Pumps
    #if SETUP_NUTRIENT_MIX_PIN >= 0
    {   auto nutrientMix = hydroController.addFluidReservoir(Hydro_ReservoirType_NutrientPremix, 1, true);
        auto nutrientPump = hydroController.addPeristalticPumpRelay(SETUP_NUTRIENT_MIX_PIN, SETUP_NUTRIENT_MIX_PINCHNL);
        nutrientPump->setParentRail(dcRelayPower);
        nutrientPump->setSourceReservoir(nutrientMix);
        nutrientPump->setDestinationReservoir(feedReservoir);
        nutrientPump->setContinuousFlowRate(HydroSingleMeasurement(SETUP_PERI_PUMP_FLOWRATE, Hydro_UnitsType_LiqFlowRate_LitersPerMin));
    }
    #endif
    #if SETUP_FRESH_WATER_PIN >= 0
    {   auto freshWater = hydroController.addFluidReservoir(Hydro_ReservoirType_FreshWater, 1, true);
        auto dilutionPump = hydroController.addPeristalticPumpRelay(SETUP_FRESH_WATER_PIN, SETUP_FRESH_WATER_PINCHNL);
        dilutionPump->setParentRail(dcRelayPower);
        dilutionPump->setSourceReservoir(freshWater);
        dilutionPump->setDestinationReservoir(feedReservoir);
        dilutionPump->setContinuousFlowRate(HydroSingleMeasurement(SETUP_PERI_PUMP_FLOWRATE, Hydro_UnitsType_LiqFlowRate_LitersPerMin));
    }
    #endif
    #if SETUP_PH_UP_PIN >= 0
    {   auto phUpSolution = hydroController.addFluidReservoir(Hydro_ReservoirType_PhUpSolution, 1, true);
        auto pHUpPump = hydroController.addPeristalticPumpRelay(SETUP_PH_UP_PIN, SETUP_PH_UP_PINCHNL);
        pHUpPump->setParentRail(dcRelayPower);
        pHUpPump->setSourceReservoir(phUpSolution);
        pHUpPump->setDestinationReservoir(feedReservoir);
        pHUpPump->setContinuousFlowRate(HydroSingleMeasurement(SETUP_PERI_PUMP_FLOWRATE, Hydro_UnitsType_LiqFlowRate_LitersPerMin));
    }
    #endif
    #if SETUP_PH_DOWN_PIN >= 0
    {   auto phDownSolution = hydroController.addFluidReservoir(Hydro_ReservoirType_PhDownSolution, 1, true);
        auto pHDownPump = hydroController.addPeristalticPumpRelay(SETUP_PH_DOWN_PIN, SETUP_PH_DOWN_PINCHNL);
        pHDownPump->setParentRail(dcRelayPower);
        pHDownPump->setSourceReservoir(phDownSolution);
        pHDownPump->setDestinationReservoir(feedReservoir);
        pHDownPump->setContinuousFlowRate(HydroSingleMeasurement(SETUP_PERI_PUMP_FLOWRATE, Hydro_UnitsType_LiqFlowRate_LitersPerMin));
    }
    #endif
}

inline void setupUI()
{
    #if defined(HYDRO_USE_GUI) && (NOT_SETUP_AS(SETUP_CONTROL_IN_MODE, Disabled) || NOT_SETUP_AS(SETUP_DISPLAY_OUT_MODE, Disabled)) && NOT_SETUP_AS(SETUP_SYS_UI_MODE, Disabled)
        UIControlSetup uiCtrlSetup;
        UIDisplaySetup uiDispSetup;
        #if SETUP_UI_IS_DFROBOTSHIELD
            uiCtrlSetup = UIControlSetup::usingDFRobotShield();
            uiDispSetup = UIDisplaySetup::usingDFRobotShield();
        #else
            switch (hydroController.getControlInputMode()) {
                case Hydro_ControlInputMode_RotaryEncoderOk:
                case Hydro_ControlInputMode_RotaryEncoderOkLR:
                    uiCtrlSetup = UIControlSetup(RotaryControlSetup(JOIN(Hydro_EncoderSpeed,SETUP_UI_ENC_ROTARY_SPEED)));
                    break;
                case Hydro_ControlInputMode_UpDownButtonsOk:
                case Hydro_ControlInputMode_UpDownButtonsOkLR:
                    uiCtrlSetup = UIControlSetup(ButtonsControlSetup(SETUP_UI_KEY_REPEAT_SPEED));
                    break;
                case Hydro_ControlInputMode_UpDownESP32TouchOk:
                case Hydro_ControlInputMode_UpDownESP32TouchOkLR:
                    uiCtrlSetup = UIControlSetup(ESP32TouchControlSetup(SETUP_UI_KEY_REPEAT_SPEED, SETUP_UI_ESP32TOUCH_SWITCH, JOIN(Hydro_ESP32Touch_HighRef,SETUP_UI_ESP32TOUCH_HVOLTS), JOIN(Hydro_ESP32Touch_LowRef,SETUP_UI_ESP32TOUCH_LVOLTS), JOIN(Hydro_ESP32Touch_HighRefAtten,SETUP_UI_ESP32TOUCH_HVATTEN)));
                    break;
                case Hydro_ControlInputMode_AnalogJoystickOk:
                    uiCtrlSetup = UIControlSetup(JoystickControlSetup(SETUP_UI_KEY_REPEAT_DELAY, SETUP_UI_JS_ACCELERATION));
                    break;
                case Hydro_ControlInputMode_Matrix2x2UpDownButtonsOkL:
                case Hydro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk:
                case Hydro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR:
                case Hydro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk:
                case Hydro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR:
                    uiCtrlSetup = UIControlSetup(MatrixControlSetup(SETUP_UI_KEY_REPEAT_DELAY, SETUP_UI_KEY_REPEAT_INTERVAL, JOIN(Hydro_EncoderSpeed,SETUP_UI_ENC_ROTARY_SPEED)));
                    break;
                case Hydro_ControlInputMode_ResistiveTouch:
                case Hydro_ControlInputMode_TouchScreen:
                case Hydro_ControlInputMode_TFTTouch:
                    #ifndef HYDRO_UI_ENABLE_XPT2046TS
                        uiCtrlSetup = UIControlSetup(TouchscreenSetup(JOIN(Hydro_TouchscreenOrientation,SETUP_UI_TOUCHSCREEN_ORIENT)));
                    #else
                        uiCtrlSetup = UIControlSetup(TouchscreenSetup(JOIN(Hydro_TouchscreenOrientation,SETUP_UI_TOUCHSCREEN_ORIENT), &SETUP_UI_TOUCHSCREEN_SPI));
                    #endif
                    break;
                default: break;
            }
            switch (hydroController.getDisplayOutputMode()) {
                case Hydro_DisplayOutputMode_LCD16x2_EN:
                case Hydro_DisplayOutputMode_LCD16x2_RS:
                case Hydro_DisplayOutputMode_LCD20x4_EN:
                case Hydro_DisplayOutputMode_LCD20x4_RS:
                    uiDispSetup = UIDisplaySetup(LCDDisplaySetup(JOIN(Hydro_BacklightMode,SETUP_UI_GFX_BACKLIGHT_MODE)));
                    break;
                case Hydro_DisplayOutputMode_SSD1305:
                case Hydro_DisplayOutputMode_SSD1305_x32Ada:
                case Hydro_DisplayOutputMode_SSD1305_x64Ada:
                case Hydro_DisplayOutputMode_SSD1306:
                case Hydro_DisplayOutputMode_SH1106:
                case Hydro_DisplayOutputMode_CustomOLED:
                case Hydro_DisplayOutputMode_SSD1607:
                case Hydro_DisplayOutputMode_IL3820:
                case Hydro_DisplayOutputMode_IL3820_V2:
                case Hydro_DisplayOutputMode_ST7735:
                case Hydro_DisplayOutputMode_ST7789:
                case Hydro_DisplayOutputMode_ILI9341:
                    uiDispSetup = UIDisplaySetup(PixelDisplaySetup(JOIN(Hydro_DisplayRotation,SETUP_UI_GFX_ROTATION), SETUP_UI_GFX_DC_PIN, SETUP_UI_GFX_RESET_PIN, SETUP_UI_GFX_BACKLIGHT_PIN, JOIN(Hydro_BacklightMode,SETUP_UI_GFX_BACKLIGHT_MODE), DAC_RESOLUTION,
#ifdef ESP32
                                                                   SETUP_UI_GFX_BACKLIGHT_ESP_CHN,
#endif
#ifdef ESP_PLATFORM
                                                                   SETUP_UI_GFX_BACKLIGHT_ESP_FRQ,
#endif
#if IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, ST7735)
                                                                   JOIN(Hydro_ST7735Tag,SETUP_UI_GFX_ST7735_TAG)
#elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, ST7789)
                                                                   JOIN(Hydro_ST7789Res,SETUP_UI_GFX_ST7789_RES)
#else
                                                                   Hydro_ST77XXKind_Undefined
#endif
                    ));
                    break;

                case Hydro_DisplayOutputMode_TFT:
                    uiDispSetup = UIDisplaySetup(TFTDisplaySetup(JOIN(Hydro_DisplayRotation,SETUP_UI_GFX_ROTATION), SETUP_UI_GFX_BACKLIGHT_PIN, JOIN(Hydro_BacklightMode,SETUP_UI_GFX_BACKLIGHT_MODE), DAC_RESOLUTION,
#ifdef ESP32
                                                                 SETUP_UI_GFX_BACKLIGHT_ESP_CHN,
#endif
#ifdef ESP_PLATFORM
                                                                 SETUP_UI_GFX_BACKLIGHT_ESP_FRQ,
#endif
#if IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, ST7735)
                                                                 JOIN(Hydro_ST7735Tag,SETUP_UI_GFX_ST7735_TAG)
#else
                                                                 Hydro_ST77XXKind_Undefined
#endif
                    ));
                    break;
                default: break;
            }
        #endif
        HydruinoUI *ui = new HydruinoUI(String(F(SETUP_UI_DEVICE_UUID)), uiCtrlSetup, uiDispSetup, SETUP_UI_LOGIC_LEVEL, SETUP_UI_ALLOW_INTERRUPTS, SETUP_UI_USE_TCUNICODE_FONTS, SETUP_UI_USE_BUFFERED_VRAM);
        HYDRO_SOFT_ASSERT(ui, SFP(HStr_Err_AllocationFailure));

        if (ui) {
            #if IS_SETUP_AS(SETUP_SYS_UI_MODE, Minimal)
                #if IS_SETUP_AS(SETUP_CONTROL_IN_MODE, RotaryEncoderOk) || IS_SETUP_AS(SETUP_CONTROL_IN_MODE, RotaryEncoderOkLR) ||\
                    IS_SETUP_AS(SETUP_CONTROL_IN_MODE, UpDownButtons) || IS_SETUP_AS(SETUP_CONTROL_IN_MODE, UpDownButtonsLR) ||\
                    IS_SETUP_AS(SETUP_CONTROL_IN_MODE, AnalogJoystickOk) || IS_SETUP_AS(SETUP_CONTROL_IN_MODE, Matrix2x2UpDownButtonsOkL) ||\
                    IS_SETUP_AS(SETUP_CONTROL_IN_MODE, Matrix3x4Keyboard_OptRotEncOk) || IS_SETUP_AS(SETUP_CONTROL_IN_MODE, Matrix3x4Keyboard_OptRotEncOkLR) ||\
                    IS_SETUP_AS(SETUP_CONTROL_IN_MODE, Matrix4x4Keyboard_OptRotEncOk) || IS_SETUP_AS(SETUP_CONTROL_IN_MODE, Matrix4x4Keyboard_OptRotEncOkLR) ||\
                    SETUP_UI_IS_DFROBOTSHIELD
                    ui->allocateStandardControls();
                #elif IS_SETUP_AS(SETUP_CONTROL_IN_MODE, UpDownESP32Touch) || IS_SETUP_AS(SETUP_CONTROL_IN_MODE, UpDownESP32TouchLR)
                    ui->allocateESP32TouchControl();
                #endif
                #if IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, LCD16x2_EN) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, LCD16x2_RS) ||\
                    IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, LCD20x4_EN) || IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, LCD20x4_RS) ||\
                    SETUP_UI_IS_DFROBOTSHIELD
                    ui->allocateLCDDisplay();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SSD1305)
                    ui->allocateSSD1305Display();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SSD1305_x32Ada)
                    ui->allocateSSD1305x32AdaDisplay();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SSD1305_x64Ada)
                    ui->allocateSSD1305x64AdaDisplay();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SSD1306)
                    ui->allocateSSD1306Display();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SH1106)
                    ui->allocateSH1106Display();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, CustomOLED)
                    ui->allocateCustomOLEDDisplay();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, SSD1607)
                    ui->allocateSSD1607Display();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, IL3820)
                    ui->allocateIL3820Display();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, IL3820_V2)
                    ui->allocateIL3820V2Display();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, ST7735)
                    ui->allocateST7735Display();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, ST7789)
                    ui->allocateST7789Display();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, ILI9341)
                    ui->allocateILI9341Display();
                #elif IS_SETUP_AS(SETUP_DISPLAY_OUT_MODE, TFT)
                    ui->allocateTFTDisplay();
                #endif
                #if IS_SETUP_AS(SETUP_CONTROL_IN_MODE, TouchScreen)
                    ui->allocateTouchscreenControl();
                #elif IS_SETUP_AS(SETUP_CONTROL_IN_MODE, ResistiveTouch)
                    ui->allocateResistiveTouchControl();
                #elif IS_SETUP_AS(SETUP_CONTROL_IN_MODE, TFTTouch)
                    ui->allocateTFTTouchControl();
                #endif

                #if IS_SETUP_AS(SETUP_UI_REMOTE1_TYPE, Serial) || IS_SETUP_AS(SETUP_UI_REMOTE1_TYPE, UART)
                    ui->addSerialRemote(UARTDeviceSetup(&SETUP_UI_REMOTE1_UART));
                #elif IS_SETUP_AS(SETUP_UI_REMOTE1_TYPE, Simhub)
                    ui->addSimhubRemote(UARTDeviceSetup(&SETUP_UI_REMOTE1_UART));
                #elif IS_SETUP_AS(SETUP_UI_REMOTE1_TYPE, WiFi)
                    ui->addWiFiRemote(SETUP_UI_RC_NETWORKING_PORT);
                #elif IS_SETUP_AS(SETUP_UI_REMOTE1_TYPE, Ethernet)
                    ui->addEthernetRemote(SETUP_UI_RC_NETWORKING_PORT);
                #endif
                #if IS_SETUP_AS(SETUP_UI_REMOTE2_TYPE, Serial) || IS_SETUP_AS(SETUP_UI_REMOTE2_TYPE, UART)
                    ui->addSerialRemote(UARTDeviceSetup(&SETUP_UI_REMOTE2_UART));
                #elif IS_SETUP_AS(SETUP_UI_REMOTE2_TYPE, Simhub)
                    ui->addSimhubRemote(UARTDeviceSetup(&SETUP_UI_REMOTE2_UART));
                #elif IS_SETUP_AS(SETUP_UI_REMOTE2_TYPE, WiFi)
                    ui->addWiFiRemote(SETUP_UI_RC_NETWORKING_PORT);
                #elif IS_SETUP_AS(SETUP_UI_REMOTE2_TYPE, Ethernet)
                    ui->addEthernetRemote(SETUP_UI_RC_NETWORKING_PORT);
                #endif
            #else // Full
                #if NOT_SETUP_AS(SETUP_UI_REMOTE1_TYPE, Disabled)
                    ui->addRemote(JOIN(Hydro_RemoteControl,SETUP_UI_REMOTE1_TYPE), UARTDeviceSetup(&SETUP_UI_REMOTE1_UART), SETUP_UI_RC_NETWORKING_PORT);
                #endif
                #if NOT_SETUP_AS(SETUP_UI_REMOTE2_TYPE, Disabled)
                    ui->addRemote(JOIN(Hydro_RemoteControl,SETUP_UI_REMOTE2_TYPE), UARTDeviceSetup(&SETUP_UI_REMOTE2_UART), SETUP_UI_RC_NETWORKING_PORT);
                #endif
            #endif
            #ifdef SETUP_UI_USE_CLOCK_FONT
                ui->setOverviewClockFont(&SETUP_UI_USE_CLOCK_FONT);
            #endif
            #ifdef SETUP_UI_USE_DETAIL_FONT
                ui->setOverviewDetailFont(&SETUP_UI_USE_DETAIL_FONT);
            #endif
            #ifdef SETUP_UI_USE_OVERVIEW_FONT
                ui->setOverviewFont(&SETUP_UI_USE_OVERVIEW_FONT);
            #endif
            #ifdef SETUP_UI_USE_ITEM_FONT
                ui->setMenuItemFont(&SETUP_UI_USE_ITEM_FONT);
            #endif
            #ifdef SETUP_UI_USE_TITLE_FONT
                ui->setMenuTitleFont(&SETUP_UI_USE_TITLE_FONT);
            #endif
            #ifdef SETUP_UI_USE_MENU_FONT
                ui->setMenuFont(&SETUP_UI_USE_MENU_FONT);
            #endif
            hydroController.enableUI(ui);
        }
    #endif
}

void setup() {
    // Setup base interfaces
    #ifdef HYDRO_ENABLE_DEBUG_OUTPUT
        Serial.begin(115200);           // Begin USB Serial interface
        while (!Serial) { ; }           // Wait for USB Serial to connect
    #endif
    #if defined(ESP_PLATFORM)
        SETUP_I2C_WIRE.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL); // Begin i2c Wire for ESP
    #endif

    // Begin external data storage devices for various library data.
    #if SETUP_EXTDATA_EEPROM_ENABLE
        beginStringsFromEEPROM(SETUP_EEPROM_STRINGS_ADDR);
        #ifdef HYDRO_USE_GUI
            beginUIStringsFromEEPROM(SETUP_EEPROM_UIDSTRS_ADDR);
        #endif
        hydroCropsLib.beginCropsLibraryFromEEPROM(SETUP_EEPROM_CROPSLIB_ADDR);
    #endif
    #if SETUP_EXTDATA_SD_ENABLE
    {   String libPrefix(F(SETUP_EXTDATA_SD_LIB_PREFIX));
        beginStringsFromSDCard(libPrefix);
        #ifdef HYDRO_USE_GUI
            beginUIStringsFromSDCard(libPrefix);
        #endif
        hydroCropsLib.beginCropsLibraryFromSDCard(libPrefix);
    }
    #endif

    // Sets system config name used in any of the following inits.
    #if (defined(HYDRO_USE_WIFI_STORAGE) && NOT_SETUP_AS(SETUP_SAVES_WIFISTORAGE_MODE, Disabled)) || \
        (SETUP_SD_CARD_SPI_CS >= 0 && NOT_SETUP_AS(SETUP_SAVES_SD_CARD_MODE, Disabled))
        hydroController.setSystemConfigFilename(F(SETUP_SAVES_CONFIG_FILE));
    #endif
    // Sets the EEPROM memory address for system data.
    #if NOT_SETUP_AS(SETUP_EEPROM_DEVICE_TYPE, None) && NOT_SETUP_AS(SETUP_SAVES_EEPROM_MODE, Disabled)
        hydroController.setSystemDataAddress(SETUP_EEPROM_SYSDATA_ADDR);
    #endif

    setupPinChannels();

    // Initializes controller with first initialization method that successfully returns.
    if (!(false
        #if defined(HYDRO_USE_WIFI_STORAGE) && IS_SETUP_AS(SETUP_SAVES_WIFISTORAGE_MODE, Primary)
            || hydroController.initFromWiFiStorage()
        #elif SETUP_SD_CARD_SPI_CS >= 0 && IS_SETUP_AS(SETUP_SAVES_SD_CARD_MODE, Primary)
            || hydroController.initFromSDCard()
        #elif NOT_SETUP_AS(SETUP_EEPROM_DEVICE_TYPE, None) && IS_SETUP_AS(SETUP_SAVES_EEPROM_MODE, Primary)
            || hydroController.initFromEEPROM()
        #endif
        #if defined(HYDRO_USE_WIFI_STORAGE) && IS_SETUP_AS(SETUP_SAVES_WIFISTORAGE_MODE, Fallback)
            || hydroController.initFromWiFiStorage()
        #elif SETUP_SD_CARD_SPI_CS >= 0 && IS_SETUP_AS(SETUP_SAVES_SD_CARD_MODE, Fallback)
            || hydroController.initFromSDCard()
        #elif NOT_SETUP_AS(SETUP_EEPROM_DEVICE_TYPE, None) && IS_SETUP_AS(SETUP_SAVES_EEPROM_MODE, Fallback)
            || hydroController.initFromEEPROM()
        #endif
        )) {
        // First time running controller, set up default initial empty environment.
        hydroController.init(JOIN(Hydro_SystemMode,SETUP_SYSTEM_MODE),
                             JOIN(Hydro_MeasurementMode,SETUP_MEASURE_MODE),
                             JOIN(Hydro_DisplayOutputMode,SETUP_DISPLAY_OUT_MODE),
                             JOIN(Hydro_ControlInputMode,SETUP_CONTROL_IN_MODE));

        setupOnce();
        setupUI();
        setupAlways();
        setupObjects();
    } else {
        setupUI();
        setupAlways();
    }

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
