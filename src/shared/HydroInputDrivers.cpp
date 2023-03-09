/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Input Drivers
*/

#include "Hydruino.h"
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"


HydroInputDriver::HydroInputDriver(Pair<uint8_t, const pintype_t *> controlPins)
    : _pins(controlPins)
{ ; }


HydroInputRotary::HydroInputRotary(Pair<uint8_t, const pintype_t *> controlPins, EncoderType encoderSpeed)
    : HydroInputDriver(controlPins), _encoderSpeed(encoderSpeed)
{ ; }

void HydroInputRotary::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    menuMgr.initForEncoder(renderer, initialItem, _pins.second[0], _pins.second[1], _pins.second[2], _encoderSpeed);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) menuMgr.setBackButton(_pins.second[3]);
    if (_pins.first > 4 && isValidPin(_pins.second[4])) menuMgr.setNextButton(_pins.second[4]);    
}


HydroInputUpDownButtons::HydroInputUpDownButtons(Pair<uint8_t, const pintype_t *> controlPins, uint16_t keyRepeatSpeed)
    : HydroInputDriver(controlPins), _keySpeed(keyRepeatSpeed)
{ ; }

void HydroInputUpDownButtons::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    menuMgr.initForUpDownOk(renderer, initialItem, _pins.second[1], _pins.second[0], _pins.second[2], _keySpeed);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) menuMgr.setBackButton(_pins.second[3]);
    if (_pins.first > 4 && isValidPin(_pins.second[4])) menuMgr.setNextButton(_pins.second[4]);
}


HydroInputJoystick::HydroInputJoystick(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, float decreaseDivisor, float jsCenterX, float jsCenterY, float jsZeroTol)
    : HydroInputDriver(controlPins), _repeatDelay(repeatDelay), _decreaseDivisor(decreaseDivisor),
      _joystickCalib{jsCenterX, jsCenterY, jsZeroTol},
      _joystickMultiIo(200),
      _joystickIoXAxis(internalAnalogIo(), controlPins.second[0], jsCenterX)
{
    multiIoAddExpander(&_joystickMultiIo, &_joystickIoXAxis, controlPins.first);
}

static void menuMgrOnMenuSelect(pinid_t /*key*/, bool held)
{
    menuMgr.onMenuSelect(held);
}

static void menuMgrPerformDirectionMoveTrue(pinid_t /*key*/, bool held)
{
    menuMgr.performDirectionMove(true);
}

static void menuMgrPerformDirectionMoveFalse(pinid_t /*key*/, bool held)
{
    menuMgr.performDirectionMove(false);
}

static void menuMgrValueChanged(int val)
{
    menuMgr.valueChanged(val);
}

void HydroInputJoystick::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    if (isValidPin(_pins.second[2])) {
        switches.addSwitch(_pins.second[2], NULL);
        switches.onRelease(_pins.second[2], &menuMgrOnMenuSelect);
    }
    if (isValidPin(_pins.second[0])) {
        switches.addSwitch(200, &menuMgrPerformDirectionMoveTrue);
        switches.addSwitch(201, &menuMgrPerformDirectionMoveFalse);
    }
    if (isValidPin(_pins.second[1])) {
        setupAnalogJoystickEncoder(internalAnalogIo(), _pins.second[1], &menuMgrValueChanged);
    }
    if (switches.getEncoder()) {
        reinterpret_cast<JoystickSwitchInput*>(switches.getEncoder())->setTolerance(_joystickCalib[1], _joystickCalib[2]);
        reinterpret_cast<JoystickSwitchInput*>(switches.getEncoder())->setAccelerationParameters(_repeatDelay, _decreaseDivisor);
    }

    menuMgr.initWithoutInput(renderer, initialItem);
}


static String kb3x4(F(HYDRO_UI_3X4MATRIX_KEYS));

HydroInput3x4Matrix::HydroInput3x4Matrix(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, EncoderType optEncoderSpeed)
    : HydroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(4,3,kb3x4.c_str()),
      _tcMenuKeyListener(HYDRO_UI_NX4MATRIX_ACTIONS),
      _rotaryEncoder(nullptr)
{
    // todo expander setup
    _keyboardLayout.setRowPin(0, controlPins.second[0]);
    _keyboardLayout.setRowPin(1, controlPins.second[1]);
    _keyboardLayout.setRowPin(2, controlPins.second[2]);
    _keyboardLayout.setRowPin(3, controlPins.second[3]);
    _keyboardLayout.setColPin(0, controlPins.second[4]);
    _keyboardLayout.setColPin(1, controlPins.second[5]);
    _keyboardLayout.setColPin(2, controlPins.second[6]);
    _keyboard.initialise(internalDigitalIo(), &_keyboardLayout, &_tcMenuKeyListener, false);
    _keyboard.setRepeatKeyMillis(repeatDelay, repeatInterval);

    if (isValidPin(controlPins.second[7])) {
        _rotaryEncoder = new HydroInputRotary(make_pair((uint8_t)(controlPins.first - 7), &controlPins.second[7]), optEncoderSpeed);
    }
}

HydroInput3x4Matrix::~HydroInput3x4Matrix()
{
    if (_rotaryEncoder) { delete _rotaryEncoder; }
}

void HydroInput3x4Matrix::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    if (_rotaryEncoder) { _rotaryEncoder->begin(renderer, initialItem); }
}


static String kb4x4(F(HYDRO_UI_4X4MATRIX_KEYS));

HydroInput4x4Matrix::HydroInput4x4Matrix(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, EncoderType optEncoderSpeed)
    : HydroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(4,4,kb4x4.c_str()),
      _tcMenuKeyListener(HYDRO_UI_NX4MATRIX_ACTIONS),
      _rotaryEncoder()
{
    // todo expander setup
    _keyboardLayout.setRowPin(0, controlPins.second[0]);
    _keyboardLayout.setRowPin(1, controlPins.second[1]);
    _keyboardLayout.setRowPin(2, controlPins.second[2]);
    _keyboardLayout.setRowPin(3, controlPins.second[3]);
    _keyboardLayout.setColPin(0, controlPins.second[4]);
    _keyboardLayout.setColPin(1, controlPins.second[5]);
    _keyboardLayout.setColPin(2, controlPins.second[6]);
    _keyboardLayout.setColPin(3, controlPins.second[7]);
    _keyboard.initialise(internalDigitalIo(), &_keyboardLayout, &_tcMenuKeyListener, false);
    _keyboard.setRepeatKeyMillis(repeatDelay, repeatInterval);

    if (isValidPin(controlPins.second[8])) {
        _rotaryEncoder = new HydroInputRotary(make_pair((uint8_t)(controlPins.first - 8), &controlPins.second[8]), optEncoderSpeed);
    }
}

HydroInput4x4Matrix::~HydroInput4x4Matrix()
{
    if (_rotaryEncoder) { delete _rotaryEncoder; }
}

void HydroInput4x4Matrix::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    if (_rotaryEncoder) { _rotaryEncoder->begin(renderer, initialItem); }
}


HydroInputResistiveTouch::HydroInputResistiveTouch(Pair<uint8_t, const pintype_t *> controlPins, HydroDisplayDriver *displayDriver)
    : HydroInputDriver(controlPins),
      _touchInterrogator(controlPins.second[0], controlPins.second[1], controlPins.second[2], controlPins.second[3]),
      _touchOrientation(
         /*swap*/ displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R1 || displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R3,
         /*invX*/ displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R1 || displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R2 || displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_HorzMirror,
         /*invY*/ displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R3 || displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R2 || displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_VertMirror
      ),
      _touchScreen(&_touchInterrogator, displayDriver->getGraphicsRenderer(), _touchOrientation)
{ ; }

void HydroInputResistiveTouch::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    _touchScreen.start();
    menuMgr.initWithoutInput(renderer, initialItem);
}


HydroInputTouchscreen::HydroInputTouchscreen(Pair<uint8_t, const pintype_t *> controlPins, Hydro_DisplayOrientation displayOrientation)
    : HydroInputDriver(controlPins),
      _touchScreen(),
      _touchInterrogator(_touchScreen),
      _touchOrientation(
         /*swap*/ displayOrientation == Hydro_DisplayOrientation_R1 || displayOrientation == Hydro_DisplayOrientation_R3,
         /*invX*/ displayOrientation == Hydro_DisplayOrientation_R1 || displayOrientation == Hydro_DisplayOrientation_R2 || displayOrientation == Hydro_DisplayOrientation_HorzMirror,
         /*invY*/ displayOrientation == Hydro_DisplayOrientation_R3 || displayOrientation == Hydro_DisplayOrientation_R2 || displayOrientation == Hydro_DisplayOrientation_VertMirror
      )
{ ; }

void HydroInputTouchscreen::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    _touchInterrogator.init(); // begins touch device
    menuMgr.initWithoutInput(renderer, initialItem);
}


HydroInputTFTTouch::HydroInputTFTTouch(Pair<uint8_t, const pintype_t *> controlPins, HydroDisplayTFTeSPI *displayDriver, bool useRawTouch)
    : HydroInputDriver(controlPins),
      _touchInterrogator(&displayDriver->getGfx(), displayDriver->getScreenSize().first, displayDriver->getScreenSize().second, useRawTouch),
      _touchOrientation(
         /*swap*/ displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R1 || displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R3,
         /*invX*/ displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R1 || displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R2 || displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_HorzMirror,
         /*invY*/ displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R3 || displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_R2 || displayDriver->getDisplayOrientation() == Hydro_DisplayOrientation_VertMirror
      ),
      _touchScreen(&_touchInterrogator, displayDriver->getGraphicsRenderer(), _touchOrientation)
{ ; }

void HydroInputTFTTouch::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    _touchScreen.start();
    menuMgr.initWithoutInput(renderer, initialItem);
}

#endif
