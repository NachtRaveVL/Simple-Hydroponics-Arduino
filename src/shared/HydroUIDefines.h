/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino UI Defines
*/

#ifndef HydroUIDefines_H
#define HydroUIDefines_H

#define XPT2046_RAW_MAX                 4096                // XPT2046 touch screen raw maximum value

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

#define HYDRO_UI_I2C_LCD_BASEADDR       0x20                // Base address of I2C LiquidCrystalIO LCDs (bitwise or'ed with passed address - technically base address of i2c expander in use)
#define HYDRO_UI_I2C_OLED_BASEADDR      0x78                // Base address of I2C U8g2 OLEDs (bitwise or'ed with passed address, some devices may use 0x7e)
#define HYDRO_UI_BACKLIGHT_TIMEOUT      5 * SECS_PER_MIN    // Backlight timeout, in seconds
#define HYDRO_UI_START_AT_OVERVIEW      false               // UI starts at overview screen (true), else menu screen (false)
#define HYDRO_UI_DEALLOC_AFTER_USE      defined(__AVR__)    // If screen data should be unloaded after use (true = lower memory usage, increased screens transition time), or stay memory-resident (false = higher memory usage, more instant screen transitions)
#define HYDRO_UI_GFX_VARS_USES_SLIDER   true                // Default analog slider usage for graphical displays displaying variable value ranges
#define HYDRO_UI_MENU_TITLE_MAG_LEVEL   2                   // Menu title font magnification level
#define HYDRO_UI_MENU_ITEM_MAG_LEVEL    2                   // Menu item font magnification level
#define HYDRO_UI_IOT_MONITOR_TEXT       "IoT Monitor"       // Menu IoT monitor item text
#define HYDRO_UI_AUTHENTICATOR_TEXT     "Authenticator"     // Menu authenticator item text

#define HYDRO_UI_KEYREPEAT_SPEED        20                  // Default key press repeat speed, in ticks (lower = faster)
#define HYDRO_UI_REMOTESERVER_PORT      3333                // Default remote control server's listening port
#define HYDRO_UI_2X2MATRIX_KEYS         "#BA*"              // 2x2 matrix keyboard keys (R/S1,D/S2,U/S3,L/S4)
#define HYDRO_UI_3X4MATRIX_KEYS         "123456789*0#"      // 3x4 matrix keyboard keys (123,456,789,*0#)
#define HYDRO_UI_4X4MATRIX_KEYS         "123A456B789C*0#D"  // 4x4 matrix keyboard keys (123A,456B,789C,*0#D)
#define HYDRO_UI_MATRIX_ACTIONS         "#*AB"              // Assigned enter/select char, delete/exit char, back char, and next char on keyboard
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
    Hydro_DisplayRotation_HorzMirror,                       // Horizontally mirrored (iff supported, touchscreen tuning orientation pass-through w/o rotation)
    Hydro_DisplayRotation_VertMirror,                       // Vertically mirrored (iff supported, touchscreen tuning orientation pass-through w/o rotation)

    Hydro_DisplayRotation_Count,                            // Placeholder
    Hydro_DisplayRotation_Undefined = -1                    // Placeholder
};

// Touchscreen Orientation
// Touchscreens can be attached differently than displays, so these allow finer touchscreen setup.
enum Hydro_TouchscreenOrientation : signed char {
    Hydro_TouchscreenOrientation_Same,                      // Apply same orientation as display rotation (converts display rotation to swapXY/invX/invY values)
    Hydro_TouchscreenOrientation_Plus1,                     // Apply same orientation as display rotation + R1, %4->[R0,R3] (converts display rotation + 90° to swapXY/invX/invY values)
    Hydro_TouchscreenOrientation_Plus2,                     // Apply same orientation as display rotation + R2, %4->[R0,R3] (converts display rotation + 180° to swapXY/invX/invY values)
    Hydro_TouchscreenOrientation_Plus3,                     // Apply same orientation as display rotation + R3, %4->[R0,R3] (converts display rotation + 270° to swapXY/invX/invY values)
    Hydro_TouchscreenOrientation_None,                      // No applied orientation (no invX, invY, or swapXY)
    Hydro_TouchscreenOrientation_InvertX,                   // Only invert X axis (no invY or swapXY)
    Hydro_TouchscreenOrientation_InvertY,                   // Only invert Y axis (no invX or swapXY)
    Hydro_TouchscreenOrientation_InvertXY,                  // Invert X & Y axis (no swapXY)
    Hydro_TouchscreenOrientation_SwapXY,                    // Only swap X/Y coordinates (aka transpose, no invX or invY)
    Hydro_TouchscreenOrientation_InvertX_SwapXY,            // Invert X axis, then swap X/Y coordinates (no invY)
    Hydro_TouchscreenOrientation_InvertY_SwapXY,            // Invert Y axis, then swap X/Y coordinates (no invX)
    Hydro_TouchscreenOrientation_InvertXY_SwapXY,           // Invert X & Y axis, then swap X/Y coordinates

    Hydro_TouchscreenOrientation_Count,                     // Placeholder
    Hydro_TouchscreenOrientation_Undefined = -1             // Placeholder
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
        
// ST77XX Device Kind
// Special device kind identifier for common ST7735 B/S/R color tags and common ST7789 screen resolutions.
enum Hydro_ST77XXKind : signed char {
    Hydro_ST7735Tag_B                   = (int8_t)0xff,     // ST7735B B model (128x160, 20480 pixels)
    Hydro_ST7735Tag_Green               = (int8_t)0x00,     // ST7735S Green tag (1.8" TFT /w offset such as WaveShare, 128x160, 20480 pixels)
    Hydro_ST7735Tag_Green18             = (int8_t)0x00,     // ST7735S 18Green tag (alias of Green, 128x160, 20480 pixels)
    Hydro_ST7735Tag_Red                 = (int8_t)0x01,     // ST7735R Red tag (128x160, 20480 pixels)
    Hydro_ST7735Tag_Red18               = (int8_t)0x01,     // ST7735R 18Red tag (alias of Red, 128x160, 20480 pixels)
    Hydro_ST7735Tag_Black               = (int8_t)0x02,     // ST7735S Black tag (1.8" TFT, 128x160, 20480 pixels)
    Hydro_ST7735Tag_Black18             = (int8_t)0x02,     // ST7735S 18Black tag (alias of Black, 128x160, 20480 pixels)
    Hydro_ST7735Tag_Green144            = (int8_t)0x01,     // ST7735R 144Green tag (1.44" TFT, 128x128, 16384 pixels)
    Hydro_ST7735Tag_Mini                = (int8_t)0x04,     // ST7735S Mini160x80 tag (0.96" TFT, 80x160, 12800 pixels - if inverted try Mini_Plugin)
    Hydro_ST7735Tag_Mini_Plugin         = (int8_t)0x06,     // ST7735S Mini160x80_Plugin tag (0.96" TFT /w plug-in FPC, 80x160, 12800 pixels)
    Hydro_ST7735Tag_Hallo_Wing          = (int8_t)0x05,     // ST7735R HalloWing tag (upside-down 144Green, 128x128, 16384 pixels)

    Hydro_ST7789Res_128x128             = (int8_t)0x10,     // ST7789 128x128 (0.85", 1.44" & 1.5" TFTs, 16384 pixels)
    Hydro_ST7789Res_135x240,                                // ST7789 135x240 (1.14" TFT, 32400 pixels)
    Hydro_ST7789Res_170x320,                                // ST7789 170x320 (1.9" TFT, 54400 pixels)
    Hydro_ST7789Res_172x320,                                // ST7789 172x320 (1.47" TFT, 55040 pixels)
    Hydro_ST7789Res_240x240,                                // ST7789 240x240 (1.3" & 1.54" TFTs, 57600 pixels)
    Hydro_ST7789Res_240x280,                                // ST7789 240x280 (1.69" TFT, 67200 pixels)
    Hydro_ST7789Res_240x320,                                // ST7789 240x320 (2", 2.4", & 2.8" TFTs, 76800 pixels)
    Hydro_ST7789Res_CustomTFT,                              // Custom ST7789 TFT resolution (defined statically by TFT_GFX_WIDTH & TFT_GFX_HEIGHT - override via build defines, or edit directly)

    Hydro_ST77XXKind_Undefined          = (int8_t)0xff,     // Placeholder  
    Hydro_ST7735Tag_Undefined           = (int8_t)0xff,     // Placeholder
    Hydro_ST7789Res_Undefined           = (int8_t)0xff,     // Placeholder
    Hydro_ST7789Res_Start               = Hydro_ST7789Res_128x128 // ST7789 enum start (alias of 128x128)
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
class HydroMenu;
class HydroHomeMenu;
class HydroOverview;
struct HydroUIData;


// tcMenu Callbacks
#define CALLBACK_FUNCTION
#define NO_ADDRESS                      0xffff              // No EEPROM address
extern void CALLBACK_FUNCTION gotoScreen(int id);
extern void CALLBACK_FUNCTION debugAction(int id);

#endif // /ifndef HydroUIDefines_H
