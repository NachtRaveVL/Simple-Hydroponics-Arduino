// Simple-Hydroponics-Arduino Full System
//
// This sketch will build the entire library onto a device, while supporting all of its
// functionality, and thus has the highest cost. Not meant for constrained devices!
// Minimum Flash size: 512kB - 1MB+, depending on settings
//
// Under full UI mode the UI will allow you to modify, create, and destroy any objects.
// Other settings normally locked under the UI will be configurable (some may require
// reboot), making it so a re-build and re-upload isn't necessary.

#ifdef USE_SW_SERIAL
#include "SoftwareSerial.h"
SoftwareSerial SWSerial(RX, TX);                        // Replace with Rx/Tx pins of your choice
#define Serial1 SWSerial
#endif

#include <Hydruino.h>
#include "full/HydruinoUI.h"

// Compiler flag checks
#ifdef HYDRO_DISABLE_GUI
#error The HYDRO_DISABLE_GUI flag is expected to be undefined in order to run this sketch
#endif

// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_TYPE        None            // EEPROM device type/size (AT24LC01, AT24LC02, AT24LC04, AT24LC08, AT24LC16, AT24LC32, AT24LC64, AT24LC128, AT24LC256, AT24LC512, None)
#define SETUP_EEPROM_I2C_ADDR           0b000           // EEPROM i2c address (A0-A2, bitwise or'ed with base address 0x50)
#define SETUP_RTC_DEVICE_TYPE           None            // RTC device type (DS1307, DS3231, PCF8523, PCF8563, None)
#define SETUP_SD_CARD_SPI               SPI             // SD card SPI class instance
#define SETUP_SD_CARD_SPI_CS            -1              // SD card CS pin, else -1
#define SETUP_SD_CARD_SPI_SPEED         F_SPD           // SD card SPI speed, in Hz (ignored on Teensy)
#define SETUP_DISP_I2C_ADDR             0b000           // LCD/OLED i2c address (bitwise or'ed with base address - LCD: 0x20, OLED: 0x78, note: most LCDs typically use 0b111 => 0x27)
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
#define SETUP_SYS_STATIC_LAT            DBL_UNDEF       // System static latitude (if not using GPS/UI, else DBL_UNDEF), in degrees
#define SETUP_SYS_STATIC_LONG           DBL_UNDEF       // System static longitude (if not using GPS/UI, else DBL_UNDEF), in minutes
#define SETUP_SYS_STATIC_ALT            DBL_UNDEF       // System static altitude (if not using GPS/UI, else DBL_UNDEF), in meters above sea level (msl)

// System Settings
#define SETUP_SYSTEM_MODE               Recycling       // System run mode (Recycling, DrainToWaste)
#define SETUP_MEASURE_MODE              Default         // System measurement mode (Default, Imperial, Metric, Scientific)
#define SETUP_DISPLAY_OUT_MODE          Disabled        // System display output mode (Disabled, LCD16x2_EN, LCD16x2_RS, LCD20x4_EN, LCD20x4_RS, SSD1305, SSD1305_x32Ada, SSD1305_x64Ada, SSD1306, SH1106, CustomOLED, SSD1607, IL3820, IL3820_V2, ST7735, ST7789, ILI9341, TFT)
#define SETUP_CONTROL_IN_MODE           RemoteControl   // System control input mode (Disabled, RotaryEncoderOk, RotaryEncoderOkLR, UpDownButtonsOk, UpDownButtonsOkLR, UpDownESP32TouchOk, UpDownESP32TouchOkLR, AnalogJoystickOk, Matrix2x2UpDownButtonsOkL, Matrix3x4Keyboard_OptRotEncOk, Matrix3x4Keyboard_OptRotEncOkLR, Matrix4x4Keyboard_OptRotEncOk, Matrix4x4Keyboard_OptRotEncOkLR, ResistiveTouch, TouchScreen, TFTTouch, RemoteControl)
#define SETUP_SYS_NAME                  "Hydruino"      // System name
#define SETUP_SYS_TIMEZONE              +0              // System timezone offset, in hours (int or float)
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

#if defined(HYDRO_USE_WIFI)
WiFiClient netClient;
#elif defined(HYDRO_USE_ETHERNET)
EthernetClient netClient;
#endif
#ifdef HYDRO_USE_MQTT
MQTTClient mqttClient;
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
#else
                         I2CDeviceSetup((uint8_t)SETUP_DISP_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED)
#endif
                         );

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

inline void setupUI()
{
    #if defined(HYDRO_USE_GUI) && (NOT_SETUP_AS(SETUP_CONTROL_IN_MODE, Disabled) || NOT_SETUP_AS(SETUP_DISPLAY_OUT_MODE, Disabled))
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
            #if NOT_SETUP_AS(SETUP_UI_REMOTE1_TYPE, Disabled)
                ui->addRemote(JOIN(Hydro_RemoteControl,SETUP_UI_REMOTE1_TYPE), UARTDeviceSetup(&SETUP_UI_REMOTE1_UART), SETUP_UI_RC_NETWORKING_PORT);
            #endif
            #if NOT_SETUP_AS(SETUP_UI_REMOTE2_TYPE, Disabled)
                ui->addRemote(JOIN(Hydro_RemoteControl,SETUP_UI_REMOTE2_TYPE), UARTDeviceSetup(&SETUP_UI_REMOTE2_UART), SETUP_UI_RC_NETWORKING_PORT);
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
    }

    setupAlways();

    setupUI();

    // Launches controller into main operation.
    hydroController.launch();
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
