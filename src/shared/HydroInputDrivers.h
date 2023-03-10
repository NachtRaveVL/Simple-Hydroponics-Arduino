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
class HydroInputJoystick;
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

class HydroInputDriver {
public:
    HydroInputDriver(Pair<uint8_t, const pintype_t *> controlPins);

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) = 0;

    virtual IoAbstractionRef getIoAbstraction() = 0;

    inline const Pair<uint8_t, const pintype_t *> &getPins() const { return _pins; }

protected:
    const Pair<uint8_t, const pintype_t *> _pins;
};

class HydroInputRotary : public HydroInputDriver {
public:
    HydroInputRotary(Pair<uint8_t, const pintype_t *> controlPins, Hydro_EncoderSpeed encoderSpeed);
    virtual ~HydroInputRotary() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline Hydro_EncoderSpeed getEncoderSpeed() const { return _encoderSpeed; }

protected:
    const Hydro_EncoderSpeed _encoderSpeed;
};

class HydroInputUpDownButtons : public HydroInputDriver {
public:
    HydroInputUpDownButtons(Pair<uint8_t, const pintype_t *> controlPins, uint16_t keyRepeatSpeed);
    // Special constructor for DFRobotShield /w buttons (isDFRobotShield_unused tossed, only used for constructor resolution)
    HydroInputUpDownButtons(bool isDFRobotShield_unused, uint16_t keyRepeatSpeed);
    virtual ~HydroInputUpDownButtons();

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual IoAbstractionRef getIoAbstraction() override { return _dfRobotIORef; }

    inline uint16_t getKeySpeed() const { return _keySpeed; }

protected:
    const uint16_t _keySpeed;
    IoAbstractionRef _dfRobotIORef;
};

class HydroInputJoystick : public HydroInputDriver {
public:
    HydroInputJoystick(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, float decreaseDivisor, float jsCenterX = 0.5f, float jsCenterY = 0.5f, float jsZeroTol = 0.05f);
    virtual ~HydroInputJoystick() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

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

class HydroInputMatrix3x4 : public HydroInputDriver {
public:
    HydroInputMatrix3x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Hydro_EncoderSpeed optEncoderSpeed);
    virtual ~HydroInputMatrix3x4();

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline MatrixKeyboardManager &getKeyboard() { return _keyboard; }
    inline HydroInputRotary *getRotaryEncoder() { return _rotaryEncoder; }

protected:
    MatrixKeyboardManager _keyboard;
    KeyboardLayout _keyboardLayout;
    MenuEditingKeyListener _tcMenuKeyListener;
    HydroInputRotary *_rotaryEncoder;
};

class HydroInputMatrix4x4 : public HydroInputDriver {
public:
    HydroInputMatrix4x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Hydro_EncoderSpeed optEncoderSpeed = Hydro_EncoderSpeed_HalfCycle);
    virtual ~HydroInputMatrix4x4();

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline MatrixKeyboardManager &getKeyboard() { return _keyboard; }
    inline HydroInputRotary *getRotaryEncoder() { return _rotaryEncoder; }

protected:
    MatrixKeyboardManager _keyboard;
    KeyboardLayout _keyboardLayout;
    MenuEditingKeyListener _tcMenuKeyListener;
    HydroInputRotary *_rotaryEncoder;
};

class HydroInputResistiveTouch : public HydroInputDriver {
public:
    HydroInputResistiveTouch(Pair<uint8_t, const pintype_t *> controlPins, HydroDisplayDriver *displayDriver);
    virtual ~HydroInputResistiveTouch() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline MenuTouchScreenManager &getTouchScreen() { return _touchScreen; }

protected:
    iotouch::ResistiveTouchInterrogator _touchInterrogator;
    iotouch::TouchOrientationSettings _touchOrientation;
    MenuTouchScreenManager _touchScreen;
};

class HydroInputTouchscreen : public HydroInputDriver {
public:
    HydroInputTouchscreen(Pair<uint8_t, const pintype_t *> controlPins, Hydro_DisplayOrientation displayOrientation);
    virtual ~HydroInputTouchscreen() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    #ifndef HYDRO_ENABLE_XPT2046TS
        inline Adafruit_FT6206 &getTouchScreen() { return _touchScreen; }
    #else
        inline XPT2046_Touchscreen &getTouchScreen() { return _touchScreen; }
    #endif
protected:
    #ifndef HYDRO_ENABLE_XPT2046TS
        Adafruit_FT6206 _touchScreen;
    #else
        XPT2046_Touchscreen _touchScreen;
    #endif
    iotouch::AdaLibTouchInterrogator _touchInterrogator;
    iotouch::TouchOrientationSettings _touchOrientation;
};

class HydroInputTFTTouch : public HydroInputDriver {
public:
    HydroInputTFTTouch(Pair<uint8_t, const pintype_t *> controlPins, HydroDisplayTFTeSPI *displayDriver, bool useRawTouch = false);
    virtual ~HydroInputTFTTouch() = default;

    virtual void begin(MenuRenderer *renderer, MenuItem *initialItem) override;

    virtual IoAbstractionRef getIoAbstraction() override { return nullptr; }

    inline MenuTouchScreenManager &getTouchScreen() { return _touchScreen; }

protected:
    iotouch::TftSpiTouchInterrogator _touchInterrogator;
    iotouch::TouchOrientationSettings _touchOrientation;
    MenuTouchScreenManager _touchScreen;
};

#endif // /ifndef HydroInputDrivers_H
#endif
