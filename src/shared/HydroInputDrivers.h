/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Input Drivers
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroInputDrivers_H
#define HydroInputDrivers_H

class HydroInputDriver;
class HydroInputRotary;
class HydroInputUpDownButtons;
class HydroInputESP32TouchKeys;
class HydroInputJoystick;
class HydroInputMatrix2x2;
class HydroInputMatrix3x4;
class HydroInputMatrix4x4;
class HydroInputResistiveTouch;
class HydroInputTouchscreen;
class HydroInputTFTTouch;

#include "HydruinoUI.h"
#include "KeyboardManager.h"
#include "tcMenuKeyboard.h"
#include "graphics\MenuTouchScreenEncoder.h"
#include "JoystickSwitchInput.h"

// Input Driver Base
// Base input driver class that manages control input mode selection.
class HydroInputDriver {
public:
    HydroInputDriver(Pair<uint8_t, const pintype_t *> controlPins);

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) = 0;

    bool areAllPinsInterruptable() const;
    virtual bool areMainPinsInterruptable() const = 0;

    virtual IoAbstractionRef getIoAbstraction() = 0;

    inline const Pair<uint8_t, const pintype_t *> &getPins() const { return _pins; }

protected:
    const Pair<uint8_t, const pintype_t *> _pins;
};


// Rotary Encoder Input Driver
// Rotary encoder that uses a twisting motion, along with momentary push-down.
class HydroInputRotary : public HydroInputDriver {
public:
    HydroInputRotary(Pair<uint8_t, const pintype_t *> controlPins, Hydro_EncoderSpeed encoderSpeed);
    virtual ~HydroInputRotary() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override;

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline Hydro_EncoderSpeed getEncoderSpeed() const { return _encoderSpeed; }

protected:
    const Hydro_EncoderSpeed _encoderSpeed;
};


// Up/Down Buttons Input Driver
// Standard momentary buttons input.
class HydroInputUpDownButtons : public HydroInputDriver {
public:
    HydroInputUpDownButtons(Pair<uint8_t, const pintype_t *> controlPins, uint16_t keyRepeatSpeed);
    // Special constructor for DFRobotShield /w buttons (isDFRobotShield_unused tossed, only used for constructor resolution)
    HydroInputUpDownButtons(bool isDFRobotShield_unused, uint16_t keyRepeatSpeed);
    virtual ~HydroInputUpDownButtons();

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override;

    virtual IoAbstractionRef getIoAbstraction() override { return _dfRobotIORef; }

    inline uint16_t getKeySpeed() const { return _keySpeed; }

protected:
    const uint16_t _keySpeed;
    IoAbstractionRef _dfRobotIORef;
};


// ESP32 ESPTouch Keys Input Driver
// For ESP32 only, uses integrated touch keys library.
class HydroInputESP32TouchKeys : public HydroInputDriver {
public:
    HydroInputESP32TouchKeys(Pair<uint8_t, const pintype_t *> controlPins, uint16_t keyRepeatSpeed, uint16_t switchThreshold, Hydro_ESP32Touch_HighRef highVoltage, Hydro_ESP32Touch_LowRef lowVoltage, Hydro_ESP32Touch_HighRefAtten attenuation);
    virtual ~HydroInputESP32TouchKeys() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override { return false; }

    #ifdef ESP32
        virtual IoAbstractionRef getIoAbstraction() override { return &_esp32Touch; }
    #else
        virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }
    #endif

    inline uint16_t getKeySpeed() const { return _keySpeed; }

protected:
    const uint16_t _keySpeed;
    #ifdef ESP32
        ESP32TouchKeysAbstraction _esp32Touch;
    #endif
};


// Analog Joystick Input Driver
// Analog joystick control with momentary press button.
class HydroInputJoystick : public HydroInputDriver {
public:
    HydroInputJoystick(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, float decreaseDivisor, float jsCenterX = 0.5f, float jsCenterY = 0.5f, float jsZeroTol = 0.05f);
    virtual ~HydroInputJoystick() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override { return false; }

    virtual IoAbstractionRef getIoAbstraction() override { return &_joystickMultiIo; }

    inline millis_t getRepeatDelay() const { return _repeatDelay; }
    inline float getDecreaseDivisor() const { return _decreaseDivisor; }
    inline const float *getJoystickCalib() const { return _joystickCalib; }

protected:
    const millis_t _repeatDelay;
    const float _decreaseDivisor;
    const float _joystickCalib[3];
    MultiIoAbstraction _joystickMultiIo;
    AnalogJoystickToButtons _joystickIoXAxis;
};


// 2x2 Button Matrix Input Driver
// For matrix-style input with 2 rows and 2 columns of cross-tied momentary buttons.
class HydroInputMatrix2x2 : public HydroInputDriver {
public:
    HydroInputMatrix2x2(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval);
    virtual ~HydroInputMatrix2x2();

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override { return false; }

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline MatrixKeyboardManager &getKeyboard() { return _keyboard; }

protected:
    MatrixKeyboardManager _keyboard;
    KeyboardLayout _keyboardLayout;
    MenuEditingKeyListener _tcMenuKeyListener;
};


// 3x4 Button Matrix Input Driver
// For matrix-style numeric input with 4 rows and 3 columns of cross-tied momentary buttons.
class HydroInputMatrix3x4 : public HydroInputDriver {
public:
    HydroInputMatrix3x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Hydro_EncoderSpeed encoderSpeed);
    virtual ~HydroInputMatrix3x4();

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override { return _rotaryEncoder ? _rotaryEncoder->areMainPinsInterruptable() : false; }

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline MatrixKeyboardManager &getKeyboard() { return _keyboard; }
    inline HydroInputRotary *getRotaryEncoder() { return _rotaryEncoder; }

protected:
    MatrixKeyboardManager _keyboard;
    KeyboardLayout _keyboardLayout;
    MenuEditingKeyListener _tcMenuKeyListener;
    HydroInputRotary *_rotaryEncoder;
};


// 4x4 Button Matrix Input Driver
// For matrix-style alpha-numeric input with 4 rows and 4 columns of cross-tied momentary buttons.
class HydroInputMatrix4x4 : public HydroInputDriver {
public:
    HydroInputMatrix4x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Hydro_EncoderSpeed encoderSpeed = Hydro_EncoderSpeed_HalfCycle);
    virtual ~HydroInputMatrix4x4();

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override { return _rotaryEncoder ? _rotaryEncoder->areMainPinsInterruptable() : false; }

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline MatrixKeyboardManager &getKeyboard() { return _keyboard; }
    inline HydroInputRotary *getRotaryEncoder() { return _rotaryEncoder; }

protected:
    MatrixKeyboardManager _keyboard;
    KeyboardLayout _keyboardLayout;
    MenuEditingKeyListener _tcMenuKeyListener;
    HydroInputRotary *_rotaryEncoder;
};


// Resistive Touch Screen Input Driver
// A style of touch screen that uses resistive measurements for touch detection.
class HydroInputResistiveTouch : public HydroInputDriver {
public:
    HydroInputResistiveTouch(Pair<uint8_t, const pintype_t *> controlPins, HydroDisplayDriver *displayDriver);
    virtual ~HydroInputResistiveTouch() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override { return false; }

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline MenuTouchScreenManager &getTouchScreen() { return _touchScreen; }

protected:
    iotouch::ResistiveTouchInterrogator _touchInterrogator;
    iotouch::TouchOrientationSettings _touchOrientation;
    MenuTouchScreenManager _touchScreen;
};


// Touch Screen Input Driver
// Standard touch screen driver using FT6206 (i2c based) or XPT2046 (SPI based).
// BSP touch interrogator uses TFT_GFX_WIDTH/HEIGHT for screen device width/height.
class HydroInputTouchscreen : public HydroInputDriver {
public:
    HydroInputTouchscreen(Pair<uint8_t, const pintype_t *> controlPins, Hydro_DisplayRotation displayRotation);
    virtual ~HydroInputTouchscreen() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override { return false; }

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    #ifdef HYDRO_UI_ENABLE_XPT2046TS
        inline XPT2046_Touchscreen &getTouchScreen() { return _touchScreen; }
    #else
        inline Adafruit_FT6206 &getTouchScreen() { return _touchScreen; }
    #endif
protected:
    #ifdef HYDRO_UI_ENABLE_XPT2046TS
        XPT2046_Touchscreen _touchScreen;
    #else
        Adafruit_FT6206 _touchScreen;
    #endif
    #ifdef HYDRO_UI_ENABLE_BSP_TOUCH
        StBspTouchInterrogator _touchInterrogator;
    #else
        iotouch::AdaLibTouchInterrogator _touchInterrogator;
    #endif
    iotouch::TouchOrientationSettings _touchOrientation;
};


// TFT Touch Screen Input Driver
// Standard XPT2046 touch screen, but using TFT_eSPI library. Must be paired with TFTeSPI display driver.
class HydroInputTFTTouch : public HydroInputDriver {
public:
    HydroInputTFTTouch(Pair<uint8_t, const pintype_t *> controlPins, HydroDisplayTFTeSPI *displayDriver, bool useRawTouch = false);
    virtual ~HydroInputTFTTouch() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual bool areMainPinsInterruptable() const override { return false; }

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline MenuTouchScreenManager &getTouchScreen() { return _touchScreen; }

protected:
    iotouch::TftSpiTouchInterrogator _touchInterrogator;
    iotouch::TouchOrientationSettings _touchOrientation;
    MenuTouchScreenManager _touchScreen;
};

#endif // /ifndef HydroInputDrivers_H
#endif
