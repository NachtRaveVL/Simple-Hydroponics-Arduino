/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino UI Common Inlines
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroUIInlines_HPP
#define HydroUIInlines_HPP

struct LCDDisplaySetup;
struct PixelDisplaySetup;
struct ST7735DisplaySetup;
struct TFTDisplaySetup;
struct UIDisplaySetup;

struct RotaryControlSetup;
struct ButtonsControlSetup;
struct ESP32TouchControlSetup;
struct JoystickControlSetup;
struct MatrixControlSetup;
struct UIControlSetup;

#include "HydruinoUI.h"

// Returns the active base UI instance. Not guaranteed to be non-null.
inline HydruinoBaseUI *getBaseUI() { return reinterpret_cast<HydruinoBaseUI *>(getUI()); }

// Returns the first theme in parameter list that isn't undefined, allowing defaulting chains to be nicely defined.
inline Hydro_DisplayTheme definedThemeElse(Hydro_DisplayTheme theme1, Hydro_DisplayTheme theme2) {
    return theme1 != Hydro_DisplayTheme_Undefined ? theme1 : theme2;
}


// LCD Display Setup
struct LCDDisplaySetup {
    Hydro_BacklightMode backlightMode;  // Backlight/LED/BL pin mode (default: Hydro_BacklightMode_Normal)
    bool isDFRobotShield;               // Using DF robot shield

    inline LCDDisplaySetup(Hydro_BacklightMode backlightModeIn = Hydro_BacklightMode_Normal, bool isDFRobotShieldIn = false) : backlightMode(backlightModeIn), isDFRobotShield(isDFRobotShieldIn) { ; }

    static inline LCDDisplaySetup usingDFRobotShield() { return LCDDisplaySetup(Hydro_BacklightMode_Normal, true); }
};

// Standard Pixel Display Setup
struct PixelDisplaySetup {
    Hydro_DisplayRotation rotation;     // Display orientation/rotation (default: R0)
    pintype_t dcPin;                    // DC/RS pin (if using SPI), else -1 (default: -1)
    pintype_t ledPin;                   // Optional backlight/LED/BL pin (if using SPI), else -1 (default: -1, Note: Unused backlight pin can optionally be tied typically to HIGH for always-on)
    pintype_t resetPin;                 // Optional reset/RST pin, else -1 (default: -1, Note: Unused reset pin typically needs tied to HIGH for display to function)
    Hydro_BacklightMode backlightMode;  // Backlight/LED/BL pin mode (default: Hydro_BacklightMode_Normal)

    inline PixelDisplaySetup(Hydro_DisplayRotation rotationIn = Hydro_DisplayRotation_R0, pintype_t dcPinIn = -1, pintype_t ledPinIn = -1, pintype_t resetPinIn = -1, Hydro_BacklightMode backlightModeIn = Hydro_BacklightMode_Normal) : rotation(rotationIn), dcPin(dcPinIn), ledPin(ledPinIn), resetPin(resetPinIn), backlightMode(backlightModeIn) { ; }
};

// ST7735 Pixel Display Setup
struct ST7735DisplaySetup {
    Hydro_DisplayRotation rotation;     // Display orientation/rotation (default: R0)
    Hydro_ST7735Tab tabColor;           // ST7735 tab color (default: undef/-1)
    pintype_t dcPin;                    // DC/RS pin (if using SPI), else -1 (default: -1)
    pintype_t ledPin;                   // Optional backlight/LED/BL pin (if using SPI), else -1 (default: -1, Note: Unused backlight pin can optionally be tied typically to HIGH for always-on)
    pintype_t resetPin;                 // Optional reset/RST pin, else -1 (default: -1, Note: Unused reset pin typically needs tied to HIGH for display to function)
    Hydro_BacklightMode backlightMode;  // Backlight/LED/BL pin mode (default: Hydro_BacklightMode_Normal)

    inline ST7735DisplaySetup(Hydro_DisplayRotation rotationIn = Hydro_DisplayRotation_R0, Hydro_ST7735Tab tabColorIn = Hydro_ST7735Tab_Undefined, pintype_t dcPinIn = -1, pintype_t ledPinIn = -1, pintype_t resetPinIn = -1, Hydro_BacklightMode backlightModeIn = Hydro_BacklightMode_Normal) : rotation(rotationIn), tabColor(tabColorIn), dcPin(dcPinIn), ledPin(ledPinIn), resetPin(resetPinIn), backlightMode(backlightModeIn) { ; }
};

// TFT Display Setup
struct TFTDisplaySetup {
    Hydro_DisplayRotation rotation;     // Display orientation/rotation (default: R0)
    uint16_t screenWidth;               // TFT screen width (default: 320)
    uint16_t screenHeight;              // TFT screen height (default: 240)

    inline TFTDisplaySetup(Hydro_DisplayRotation rotationIn = Hydro_DisplayRotation_R0, pintype_t screenWidthIn = 320, pintype_t screenHeightIn = 240) : rotation(rotationIn), screenWidth(screenWidthIn), screenHeight(screenHeightIn) { ; }
};

// Combined UI Display Setup
// A union of the various UI display setup structures, to assist with user display output settings.
struct UIDisplaySetup {
    enum : signed char { None, LCD, Pixel, ST7735, TFT } dispCfgType; // Display config type
    union {
        LCDDisplaySetup lcd;            // LCD display setup
        PixelDisplaySetup gfx;          // Pixel display setup
        ST7735DisplaySetup st7735;      // ST7735 display setup
        TFTDisplaySetup tft;            // TFT display setup
    } dispCfgAs;                        // Display config data

    inline UIDisplaySetup() : dispCfgType(None), dispCfgAs{} { ; }
    inline UIDisplaySetup(LCDDisplaySetup displaySetup) : dispCfgType(LCD), dispCfgAs{.lcd=displaySetup} { ; }
    inline UIDisplaySetup(PixelDisplaySetup displaySetup) : dispCfgType(Pixel), dispCfgAs{.gfx=displaySetup} { ; }
    inline UIDisplaySetup(ST7735DisplaySetup displaySetup) : dispCfgType(ST7735), dispCfgAs{.st7735=displaySetup} { ; }
    inline UIDisplaySetup(TFTDisplaySetup displaySetup) : dispCfgType(TFT), dispCfgAs{.tft=displaySetup} { ; }

    static inline UIDisplaySetup usingDFRobotShield() { return UIDisplaySetup(LCDDisplaySetup::usingDFRobotShield()); }

    inline Hydro_DisplayRotation getDisplayRotation() const { return dispCfgType == Pixel ? dispCfgAs.gfx.rotation : dispCfgType == ST7735 ? dispCfgAs.st7735.rotation : dispCfgType == TFT ? dispCfgAs.tft.rotation : Hydro_DisplayRotation_R0; }
    inline Hydro_BacklightMode getBacklightMode() const { return dispCfgType == LCD ? dispCfgAs.lcd.backlightMode : dispCfgType == Pixel ? dispCfgAs.gfx.backlightMode : dispCfgType == ST7735 ? dispCfgAs.st7735.backlightMode : Hydro_BacklightMode_Normal; }
    inline pintype_t getBacklightPin() const { return dispCfgType == LCD ? (pintype_t)(dispCfgAs.lcd.isDFRobotShield ? 10 : 3) : dispCfgType == Pixel ? dispCfgAs.gfx.ledPin : dispCfgType == ST7735 ? dispCfgAs.st7735.ledPin : (pintype_t)-1; }
};


// Rotary Encoder Input Setup
struct RotaryControlSetup {
    Hydro_EncoderSpeed encoderSpeed;    // Encoder cycling speed (detent freq)

    inline RotaryControlSetup(Hydro_EncoderSpeed encoderSpeedIn = Hydro_EncoderSpeed_HalfCycle) : encoderSpeed(encoderSpeedIn) { ; }
};

// Up/Down Buttons Input Setup
struct ButtonsControlSetup {
    uint8_t repeatSpeed;                // Key repeat speed, in ticks
    bool isDFRobotShield;               // Using DF robot shield

    inline ButtonsControlSetup(uint8_t repeatSpeedIn = HYDRO_UI_KEYREPEAT_SPEED, bool isDFRobotShieldIn = false) : repeatSpeed(repeatSpeedIn), isDFRobotShield(isDFRobotShieldIn) { ; }

    static inline ButtonsControlSetup usingDFRobotShield() { return ButtonsControlSetup(HYDRO_UI_KEYREPEAT_SPEED, true); }
};

// ESP32 Touch Keys Input Setup
struct ESP32TouchControlSetup {
    uint8_t repeatSpeed;                        // Key repeat speed, in ticks
    uint16_t switchThreshold;                   // Switch activation threshold (default: 800)
    Hydro_ESP32Touch_HighRef highVoltage;       // High reference voltage (default: V_2V7)
    Hydro_ESP32Touch_LowRef lowVoltage;         // Low reference voltage (default: V_0V5)
    Hydro_ESP32Touch_HighRefAtten attenuation;  // High reference voltage attention (default: V_1V)

    inline ESP32TouchControlSetup(uint8_t repeatSpeedIn = HYDRO_UI_KEYREPEAT_SPEED, uint16_t switchThresholdIn = 800, Hydro_ESP32Touch_HighRef highVoltageIn = Hydro_ESP32Touch_HighRef_V_2V7, Hydro_ESP32Touch_LowRef lowVoltageIn = Hydro_ESP32Touch_LowRef_V_0V5, Hydro_ESP32Touch_HighRefAtten attenuationIn = Hydro_ESP32Touch_HighRefAtten_V_1V) : repeatSpeed(repeatSpeedIn), switchThreshold(switchThresholdIn), highVoltage(highVoltageIn), lowVoltage(lowVoltageIn), attenuation(attenuationIn) { ; }
};

// Analog Joystick Input Setup
struct JoystickControlSetup {
    millis_t repeatDelay;               // Repeat delay, in milliseconds (default: 750)
    float decreaseDivisor;              // Repeat decrease divisor

    inline JoystickControlSetup(millis_t repeatDelayIn = 750, float decreaseDivisorIn = 3.0f) : repeatDelay(repeatDelayIn), decreaseDivisor(decreaseDivisorIn) { ; }
};

// Display Matrix Input Setup
struct MatrixControlSetup {
    millis_t repeatDelay;               // Repeat delay, in milliseconds
    millis_t repeatInterval;            // Repeat interval, in milliseconds
    Hydro_EncoderSpeed encoderSpeed;    // Encoder cycling speed (optional)

    inline MatrixControlSetup(millis_t repeatDelayIn = 850, millis_t repeatIntervalIn = 350, Hydro_EncoderSpeed encoderSpeedIn = Hydro_EncoderSpeed_HalfCycle) : repeatDelay(repeatDelayIn), repeatInterval(repeatIntervalIn), encoderSpeed(encoderSpeedIn) { ; }
};

// Combined UI Control Setup
// A union of the various UI control setup structures, to assist with user control input settings.
struct UIControlSetup {
    enum : signed char { None, Encoder, Buttons, ESP32Touch, Joystick, Matrix } ctrlCfgType; // Control config type
    union {
        RotaryControlSetup encoder;     // Rotary encoder setup
        ButtonsControlSetup buttons;    // Up/Down buttons setup
        ESP32TouchControlSetup touch;   // ESP32 touch keys setup
        JoystickControlSetup joystick;  // Analog joystick setup
        MatrixControlSetup matrix;      // Matrix keyboard setup
    } ctrlCfgAs;

    inline UIControlSetup() : ctrlCfgType(None), ctrlCfgAs{} { ; }
    inline UIControlSetup(RotaryControlSetup ctrlSetup) : ctrlCfgType(Encoder), ctrlCfgAs{.encoder=ctrlSetup} { ; }
    inline UIControlSetup(ButtonsControlSetup ctrlSetup) : ctrlCfgType(Buttons), ctrlCfgAs{.buttons=ctrlSetup} { ; }
    inline UIControlSetup(ESP32TouchControlSetup ctrlSetup) : ctrlCfgType(ESP32Touch), ctrlCfgAs{.touch=ctrlSetup} { ; }
    inline UIControlSetup(JoystickControlSetup ctrlSetup) : ctrlCfgType(Joystick), ctrlCfgAs{.joystick=ctrlSetup} { ; }
    inline UIControlSetup(MatrixControlSetup ctrlSetup) : ctrlCfgType(Matrix), ctrlCfgAs{.matrix=ctrlSetup} { ; }

    static inline UIControlSetup usingDFRobotShield() { return UIControlSetup(ButtonsControlSetup::usingDFRobotShield()); }
};

#endif // /ifndef HydroUIInlines_HPP
#endif
