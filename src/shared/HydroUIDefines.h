/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino UI Defines
*/

#ifndef HydroUIDefines_H
#define HydroUIDefines_H

#if F_SPD >= 48000000                                       // Resolve an appropriate UI update speed (1-10)
#define HYDRO_UI_UPDATE_SPEED           10
#elif F_SPD >= 32000000
#define HYDRO_UI_UPDATE_SPEED           5
#elif F_SPD >= 16000000
#define HYDRO_UI_UPDATE_SPEED           2
#endif

// The following sizes apply to all architectures
#define HYDRO_UI_RENDERER_BUFFERSIZE    32                  // Buffer size for display renderers
#define HYDRO_UI_STARFIELD_MAXSIZE      16                  // Starfield map maxsize
// The following sizes only apply to architectures that do not have STL support (AVR/SAM)
#define HYDRO_UI_REMOTECONTROLS_MAXSIZE 2                   // Maximum array size for remote controls list (max # of remote controls)

// CustomOLED U8g2 device string
#ifndef HYDRO_UI_CUSTOM_OLED_I2C
#define HYDRO_UI_CUSTOM_OLED_I2C        U8G2_SSD1309_128X64_NONAME0_F_HW_I2C    // Custom OLED for i2c setup (must be _HW_I2C variant /w 2 init params: rotation, resetPin - Wire# not assertion checked since baked into define)
#endif
#ifndef HYDRO_UI_CUSTOM_OLED_SPI
#define HYDRO_UI_CUSTOM_OLED_SPI        U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI // Custom OLED for SPI setup (must be _4W_HW_SPI variant /w 4 init params: rotation, csPin, dcPin, resetPin - SPI# not assertion checked since baked into define)
#endif

#define HYDRO_UI_I2C_LCD_BASEADDR       0x20                // Base address of I2C LiquidCrystalIO LCDs (bitwise or'ed with passed address)
#define HYDRO_UI_I2C_OLED_BASEADDR      0x78                // Base address of I2C U8g2 OLEDs (bitwise or'ed with passed address, some devices may use 0x7e)
#define HYDRO_UI_GFXTFT_USES_SLIDER     true                // Default analog slider usage for Gfx/TFT displays
#define HYDRO_UI_BACKLIGHT_TIMEOUT      5 * SECS_PER_MIN    // Backlight timeout, in secs

#define HYDRO_UI_KEYREPEAT_SPEED        20                  // Default key press repeat speed
#define HYDRO_UI_REMOTESERVER_PORT      3333                // Default remote control server's listening port
#define HYDRO_UI_3X4MATRIX_KEYS         "123456789*0#"      // 3x4 matrix keyboard keys (123,456,789,*0#)
#define HYDRO_UI_4X4MATRIX_KEYS         "123A456B789C*0#D"  // 4x4 matrix keyboard keys (123A,456B,789C,*0#D)
#define HYDRO_UI_MATRIX_ACTIONS         "#*AB"              // Assigned enter/select char, delete/exit char, back char, and next char on keyboard (also 2x2 matrix keyboard keys)
#define HYDRO_UI_TFTTOUCH_USES_RAW      false               // Raw touch usage for TFTTouch

// Default graphical display theme base (CoolBlue, DarkMode)
#define HYDRO_UI_GFX_DISP_THEME_BASE    CoolBlue            
#define HYDRO_UI_GFX_DISP_THEME_SMLMED  SM
#define HYDRO_UI_GFX_DISP_THEME_MEDLRG  ML


// Remote Control
// Type of remote control.
enum Hydro_RemoteControl : signed char {
    Hydro_RemoteControl_Disabled,                           // Disabled remote control
    Hydro_RemoteControl_Serial,                             // Remote control by Serial/Bluetooth AT, requires UART setup
    Hydro_RemoteControl_Simhub,                             // Remote control by Simhub serial connector, requires UART setup
    Hydro_RemoteControl_WiFi,                               // Remote control by WiFi device, requires enabled WiFi
    Hydro_RemoteControl_Ethernet,                           // Remote control by Ethernet device, requires enabled Ethernet

    Hydro_RemoteControl_Count,                              // Placeholder
    Hydro_RemoteControl_Undefined = -1                      // Placeholder
};

// Display Rotation
// Amount of display rotation, or in some cases mirror'ing.
enum Hydro_DisplayRotation : signed char {
    Hydro_DisplayRotation_R0,                               // 0° clockwise display rotation (0° counter-clockwise device mounting)
    Hydro_DisplayRotation_R1,                               // 90° clockwise display rotation (90° counter-clockwise device mounting)
    Hydro_DisplayRotation_R2,                               // 180° clockwise display rotation (180° counter-clockwise device mounting)
    Hydro_DisplayRotation_R3,                               // 270° clockwise display rotation  (270° counter-clockwise device mounting)
    Hydro_DisplayRotation_HorzMirror,                       // Horizontally mirrored (iff supported)
    Hydro_DisplayRotation_VertMirror,                       // Vertically mirrored (iff supported)

    Hydro_DisplayRotation_Count,                            // Placeholder
    Hydro_DisplayRotation_Undefined = -1                    // Placeholder
};

// Display Theme
// General color theme and aesthetics.
enum Hydro_DisplayTheme : signed char {
    Hydro_DisplayTheme_CoolBlue_ML,                         // Cool blue theme for medium to large color displays (larger fonts/more padding)
    Hydro_DisplayTheme_CoolBlue_SM,                         // Cool blue theme for small to medium color displays (smaller fonts/less padding)
    Hydro_DisplayTheme_DarkMode_ML,                         // Dark mode theme for medium to large color displays (larger fonts/more padding)
    Hydro_DisplayTheme_DarkMode_SM,                         // Dark mode theme for small to medium color displays (smaller fonts/less padding)
    Hydro_DisplayTheme_MonoOLED,                            // Monochrome/OLED theme for small to medium monochrome displays, /w standard border
    Hydro_DisplayTheme_MonoOLED_Inv,                        // Monochrome/OLED theme for small to medium monochrome displays, /w inverted colors

    Hydro_DisplayTheme_Count,                               // Placeholder
    Hydro_DisplayTheme_Undefined = -1                       // Placeholder
};

// ST7735 Device Tab
// Special device tab identifier for ST7735 (B & R) devices.
enum Hydro_ST7735Tab : signed char {
    Hydro_ST7735Tab_BModel              = (int8_t)0xff,     // ST7735B model (no color tag)
    Hydro_ST7735Tab_Green               = 0x00,             // ST7735R Green tag
    Hydro_ST7735Tab_Green18             = 0x00,             // ST7735R 18Green tag (alias of Green)
    Hydro_ST7735Tab_Red                 = 0x01,             // ST7735R Red tag
    Hydro_ST7735Tab_Red18               = 0x01,             // ST7735R 18Red tag (alias of Red)
    Hydro_ST7735Tab_Black               = 0x02,             // ST7735R Black tag
    Hydro_ST7735Tab_Black18             = 0x02,             // ST7735R 18Black tag (alias of Black)
    Hydro_ST7735Tab_Green144            = 0x01,             // ST7735R 144Green tag (alias of Red)
    Hydro_ST7735Tab_Mini                = 0x04,             // ST7735R Mini160x80 tag
    Hydro_ST7735Tab_Hallowing           = 0x05,             // ST7735R Hallowing tag
    Hydro_ST7735Tab_Mini_Plugin         = 0x06,             // ST7735R Mini160x80_Plugin tag

    Hydro_ST7735Tab_Undefined           = (int8_t)0xff      // Placeholder
};

// Backlight Operation Mode
// How the backlight gets handled. Derived from LCD usage.
enum Hydro_BacklightMode : signed char {
    Hydro_BacklightMode_Normal,                             // The backlight is active HIGH, standard amongst most displays
    Hydro_BacklightMode_Inverted,                           // The backlight is active LOW, inverted ouput signal
    Hydro_BacklightMode_PWM,                                // The backlight uses analog PWM for variable intensity control

    Hydro_BacklightMode_Count,                              // Placeholder
    Hydro_BacklightMode_Undefined = -1                      // Placeholder
};

// Rotary Encoder Speed
// Essentially how far the rotary encoder must physically travel before the UI responds (selection change, scroll to prev/next, etc.).
// Note: Smaller cycle length = faster item selection/scroll speed, but more physical precision required (accessibility concern).
enum Hydro_EncoderSpeed : signed char {
    Hydro_EncoderSpeed_FullCycle,                           // Detent after every full cycle of both signals, A and B
    Hydro_EncoderSpeed_HalfCycle,                           // Detent on every position where A == B
    Hydro_EncoderSpeed_QuarterCycle,                        // Detent after every signal change, A or B

    Hydro_EncoderSpeed_Count,                               // Placeholder
    Hydro_EncoderSpeed_Undefined = -1                       // Placeholder
};

// ESP32 Touch Key High Reference Voltage
// High reference voltage for press detection.
enum Hydro_ESP32Touch_HighRef : signed char {
    Hydro_ESP32Touch_HighRef_Keep,                          // No change
    Hydro_ESP32Touch_HighRef_V_2V4,                         // 2.4v
    Hydro_ESP32Touch_HighRef_V_2V5,                         // 2.5v
    Hydro_ESP32Touch_HighRef_V_2V6,                         // 2.6v
    Hydro_ESP32Touch_HighRef_V_2V7,                         // 2.7v
    Hydro_ESP32Touch_HighRef_Max,                           // Max voltage

    Hydro_ESP32Touch_HighRef_Count,                         // Placeholder
    Hydro_ESP32Touch_HighRef_Undefined = -1                 // Placeholder
};

// ESP32 Touch Key Low Reference Voltage
// Low reference voltage for press detection.
enum Hydro_ESP32Touch_LowRef : signed char {
    Hydro_ESP32Touch_LowRef_Keep,                           // No change
    Hydro_ESP32Touch_LowRef_V_0V5,                          // 0.5v
    Hydro_ESP32Touch_LowRef_V_0V6,                          // 0.6v
    Hydro_ESP32Touch_LowRef_V_0V7,                          // 0.7v
    Hydro_ESP32Touch_LowRef_V_0V8,                          // 0.8v
    Hydro_ESP32Touch_LowRef_Max,                            // Max voltage

    Hydro_ESP32Touch_LowRef_Count,                          // Placeholder
    Hydro_ESP32Touch_LowRef_Undefined = -1                  // Placeholder
};

// ESP32 Touch Key High Ref Volt Attenuation
// High reference voltage attenuation for press detection.
enum Hydro_ESP32Touch_HighRefAtten : signed char {
    Hydro_ESP32Touch_HighRefAtten_Keep,                     // No change
    Hydro_ESP32Touch_HighRefAtten_V_1V5,                    // 1.5v
    Hydro_ESP32Touch_HighRefAtten_V_1V,                     // 1v
    Hydro_ESP32Touch_HighRefAtten_V_0V5,                    // 0.5v
    Hydro_ESP32Touch_HighRefAtten_V_0V,                     // 0v
    Hydro_ESP32Touch_HighRefAtten_Max,                      // Max voltage

    Hydro_ESP32Touch_HighRefAtten_Count,                    // Placeholder
    Hydro_ESP32Touch_HighRefAtten_Undefined = -1            // Placeholder
};


class HydruinoBaseUI;
class HydroDisplayDriver;
class HydroInputDriver;
class HydroRemoteControl;
class HydroOverview;
struct HydroUIData;

#endif // /ifndef HydroUIDefines_H
