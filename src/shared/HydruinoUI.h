/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Base UI
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroBaseUI_H
#define HydroBaseUI_H

#include "HydroUIDefines.h"

// tcMenu Plugin Adaptations
#include "tcMenu_Display_AdaFruitGfx.h"
#include "tcMenu_Display_LiquidCrystal.h"
#include "tcMenu_Display_TfteSpi.h"
#include "tcMenu_Display_U8g2.h"
#include "tcMenu_Input_AdaTouchDriver.h"
#include "tcMenu_Remote_EthernetTransport.h"
#include "tcMenu_Remote_SerialTransport.h"
#include "tcMenu_Remote_SimhubConnector.h"
#include "tcMenu_Remote_WiFiTransport.h"
#include "tcMenu_Theme_CoolBlueModern.h"
#include "tcMenu_Theme_CoolBlueTraditional.h"
#include "tcMenu_Theme_DarkModeModern.h"
#include "tcMenu_Theme_DarkModeTraditional.h"
#include "tcMenu_Theme_MonoBordered.h"
#include "tcMenu_Theme_MonoInverse.h"

class HydruinoBaseUI;
struct HydroUIData;

#include "HydroDisplayDrivers.h"
#include "HydroInputDrivers.h"
#include "HydroRemoteControls.h"

// LCD Display Setup
struct LCDDisplaySetup {
    bool bitInversion;                  // Bit logic inversion (inverts b/w, default: false)
    LiquidCrystal::BackLightPinMode backlitMode; // Backlight pin mode (default: LiquidCrystal::BACKLIGHT_NORMAL)
    bool isDFRobotShield;               // DFRobot shield usage (default: false)

    inline LCDDisplaySetup(bool bitInversionIn = false, LiquidCrystal::BackLightPinMode backlitModeIn = LiquidCrystal::BACKLIGHT_NORMAL, bool isDFRobotShieldIn = false) : bitInversion(bitInversionIn), backlitMode(backlitModeIn), isDFRobotShield(isDFRobotShieldIn) { ; }
};

// Standard Pixel Display Setup
struct PixelDisplaySetup {
    Hydro_DisplayOrientation dispOrient; // Display orientation (default: R0)
    pintype_t dcPin;                    // DC pin (if using SPI), else -1 (default: -1)
    pintype_t resetPin;                 // Reset pin (optional), else -1 (default: -1)

    inline PixelDisplaySetup(Hydro_DisplayOrientation dispOrientIn = Hydro_DisplayOrientation_R0, pintype_t dcPinIn = -1, pintype_t resetPinIn = -1) : dispOrient(dispOrientIn), dcPin(dcPinIn), resetPin(resetPinIn) { ; }
};

// Special ST7735 Display Setup
struct ST7735DisplaySetup {
    Hydro_DisplayOrientation dispOrient; // Display orientation (default: R0)
    Hydro_ST7735Tab tabColor;           // ST7735 tab color (default: undef/-1)
    pintype_t dcPin;                    // DC pin (if using SPI), else -1 (default: -1)
    pintype_t resetPin;                 // Reset pin (optional), else -1 (default: -1)

    inline ST7735DisplaySetup(Hydro_DisplayOrientation dispOrientIn = Hydro_DisplayOrientation_R0, Hydro_ST7735Tab tabColorIn = Hydro_ST7735Tab_Undefined, pintype_t dcPinIn = -1, pintype_t resetPinIn = -1) : dispOrient(dispOrientIn), tabColor(tabColorIn), dcPin(dcPinIn), resetPin(resetPinIn) { ; }
};

// TFT Display Setup
struct TFTDisplaySetup {
    Hydro_DisplayOrientation dispOrient; // Display orientation
    uint16_t screenWidth;               // TFT screen width (default: 320)
    uint16_t screenHeight;              // TFT screen height (default: 240)

    inline TFTDisplaySetup(Hydro_DisplayOrientation dispOrientIn = Hydro_DisplayOrientation_R0, pintype_t screenWidthIn = 320, pintype_t screenHeightIn = 240) : dispOrient(dispOrientIn), screenWidth(screenWidthIn), screenHeight(screenHeightIn) { ; }
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
    inline UIDisplaySetup(LCDDisplaySetup dispSetup) : dispCfgType(LCD), dispCfgAs{.lcd=dispSetup} { ; }
    inline UIDisplaySetup(PixelDisplaySetup dispSetup) : dispCfgType(Pixel), dispCfgAs{.gfx=dispSetup} { ; }
    inline UIDisplaySetup(ST7735DisplaySetup dispSetup) : dispCfgType(ST7735), dispCfgAs{.st7735=dispSetup} { ; }
    inline UIDisplaySetup(TFTDisplaySetup dispSetup) : dispCfgType(TFT), dispCfgAs{.tft=dispSetup} { ; }

    inline Hydro_DisplayOrientation getDisplayOrientation() const { return dispCfgType == Pixel ? dispCfgAs.gfx.dispOrient : dispCfgType == ST7735 ? dispCfgAs.st7735.dispOrient : dispCfgType == TFT ? dispCfgAs.tft.dispOrient : Hydro_DisplayOrientation_R0; }
};

// Rotary Encoder Input Setup
struct RotaryInputSetup {
    EncoderType encoderSpeed;           // Encoder cycling speed

    inline RotaryInputSetup(EncoderType encoderSpeedIn = HALF_CYCLE) : encoderSpeed(encoderSpeedIn) { ; }
};

// Up/Down Buttons Input Setup
struct ButtonsInputSetup {
    uint8_t repeatSpeed;                // Key repeat speed, in milliseconds

    inline ButtonsInputSetup(uint8_t repeatSpeedIn = HYDRO_UI_KEYREPEAT_SPEED) : repeatSpeed(repeatSpeedIn) { ; }
};

// Analog Joystick Input Setup
struct JoystickInputSetup {
    millis_t repeatDelay;               // Repeat delay, in milliseconds (default: 750)
    float decreaseDivisor;              // Repeat decrease divisor

    inline JoystickInputSetup(millis_t repeatDelayIn = 750, float decreaseDivisorIn = 3.0f) : repeatDelay(repeatDelayIn), decreaseDivisor(decreaseDivisorIn) { ; }
};

// Display Matrix Input Setup
struct MatrixInputSetup {
    millis_t repeatDelay;               // Repeat delay, in milliseconds
    millis_t repeatInterval;            // Repeat interval, in milliseconds
    EncoderType encoderSpeed;           // Encoder cycling speed (optional)

    inline MatrixInputSetup(millis_t repeatDelayIn = 850, millis_t repeatIntervalIn = 350, EncoderType encoderSpeedIn = HALF_CYCLE) : repeatDelay(repeatDelayIn), repeatInterval(repeatIntervalIn), encoderSpeed(encoderSpeedIn) { ; }
};

// Combined UI Control Setup
// A union of the various UI control setup structures, to assist with user control input settings.
struct UIControlSetup {
    enum : signed char { None, Encoder, Buttons, Joystick, Matrix } ctrlCfgType; // Control config type
    union {
        RotaryInputSetup encoder;       // Rotary encoder setup
        ButtonsInputSetup buttons;      // Up/Down buttons setup
        JoystickInputSetup joystick;    // Analog joystick setup
        MatrixInputSetup matrix;        // Matrix keyboard setup
    } ctrlCfgAs;

    inline UIControlSetup() : ctrlCfgType(None), ctrlCfgAs{} { ; }
    inline UIControlSetup(RotaryInputSetup ctrlSetup) : ctrlCfgType(Encoder), ctrlCfgAs{.encoder=ctrlSetup} { ; }
    inline UIControlSetup(ButtonsInputSetup ctrlSetup) : ctrlCfgType(Buttons), ctrlCfgAs{.buttons=ctrlSetup} { ; }
    inline UIControlSetup(JoystickInputSetup ctrlSetup) : ctrlCfgType(Joystick), ctrlCfgAs{.joystick=ctrlSetup} { ; }
    inline UIControlSetup(MatrixInputSetup ctrlSetup) : ctrlCfgType(Matrix), ctrlCfgAs{.matrix=ctrlSetup} { ; }
};

// Base UI
// The base class that manages interaction with the tcMenu UI system.
class HydruinoBaseUI : public HydroUIInterface {
public:
    HydruinoBaseUI(UIDisplaySetup uiDisplaySetup = UIDisplaySetup(),        // UI display output setup 
                   UIControlSetup uiControlSetup = UIControlSetup(),        // UI control input setup
                   bool isActiveLowIO = true,                               // Logic level usage for control & display IO pins
                   bool allowInterruptableIO = true,                        // Allows interruptable pins to interrupt, else forces polling
                   bool enableTcUnicodeFonts = true);                       // Enables tcUnicode UTF8 fonts usage instead of library fonts
    virtual ~HydruinoBaseUI();

    void init(uint8_t updatesPerSec,                                        // Updates per second (1 to 10)
              Hydro_DisplayTheme displayTheme,                              // Display theme to apply
              bool analogSlider = false);                                   // Slider usage for analog items
    void init();                                                            // Standard initializer

    void addRemote(Hydro_RemoteControl rcType,                              // Type of remote control
                   UARTDeviceSetup rcSetup = UARTDeviceSetup(),             // Remote control serial setup (if serial based), else ignored
                   uint16_t rcServerPort = HYDRO_UI_REMOTESERVER_PORT,      // Remote control server listening port (if networking based), else ignored
                   menuid_t statusMenuId = -1);                             // Status menu item (if simhub based), else ignored

    virtual bool begin() override;                                          // Begins UI

    virtual void setNeedsLayout() override;

    virtual bool isFullUI() = 0;
    inline bool isMinUI() { return !isFullUI(); }

protected:
    const bool _isActiveLow;                                // IO pins use active-low signaling logic
    const bool _allowISR;                                   // Perform ISR checks to determine ISR eligibility
    const bool _utf8Fonts;                                  // Using tcUnicode library fonts
    const bool _gfxOrTFT;                                   // Display is Adafruit_GFX or TFT_eSPI

    MenuItem *_menuRoot;                                    // Menu root item (strong)
    HydroInputDriver *_input;                               // Input driver (owned)
    HydroDisplayDriver *_display;                           // Display driver (owned)
    TcMenuRemoteServer *_remoteServer;                      // Remote control server (owned)
    Vector<HydroRemoteControl *, HYDRO_UI_REMOTECONTROLS_MAXSIZE> _remotes; // Remote controls list (owned)
};


// UI Serialization Data
struct HydroUIData : public HydroData {
    uint8_t updatesPerSec;                                  // Updates per second (1-10, default: HYDRO_UI_UPDATE_SPEED)
    Hydro_DisplayTheme displayTheme;                        // Display theme (if supported)
    float joystickCalib[3];                                 // Joystick calibration ({midX,midY,zeroTol}, default: {0.5,0.5,0.05})

    HydroUIData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#include "tcMenu_Display_AdaFruitGfx.hpp"
#include "HydroDisplayDrivers.hpp"

#endif // /ifndef HydroBaseUI_H
#endif
