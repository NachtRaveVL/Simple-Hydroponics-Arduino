/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Input Drivers
*/

#include "Hydruino.h"
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"
#include <DfRobotInputAbstraction.h>


HydroInputDriver::HydroInputDriver(Pair<uint8_t, const pintype_t *> controlPins)
    : _pins(controlPins)
{ ; }


HydroInputRotary::HydroInputRotary(Pair<uint8_t, const pintype_t *> controlPins, Hydro_EncoderSpeed encoderSpeed)
    : HydroInputDriver(controlPins), _encoderSpeed(encoderSpeed)
{ ; }

void HydroInputRotary::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    menuMgr.initForEncoder(renderer, initialItem, _pins.second[0], _pins.second[1], _pins.second[2], _encoderSpeed == Hydro_EncoderSpeed_FullCycle ? FULL_CYCLE : _encoderSpeed == Hydro_EncoderSpeed_HalfCycle ? HALF_CYCLE : QUARTER_CYCLE);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) menuMgr.setBackButton(_pins.second[3]);
    if (_pins.first > 4 && isValidPin(_pins.second[4])) menuMgr.setNextButton(_pins.second[4]);    
}


HydroInputUpDownButtons::HydroInputUpDownButtons(Pair<uint8_t, const pintype_t *> controlPins, uint16_t keyRepeatSpeed)
    : HydroInputDriver(controlPins), _keySpeed(keyRepeatSpeed), _dfRobotIORef(nullptr)
{ ; }

HydroInputUpDownButtons::HydroInputUpDownButtons(bool isDFRobotShield_unused, uint16_t keyRepeatSpeed)
    : HydroInputDriver(make_pair((uint8_t)5, (const pintype_t *)(new pintype_t[5]))), _keySpeed(keyRepeatSpeed), _dfRobotIORef(inputFromDfRobotShield())
{
    pintype_t *pins = const_cast<pintype_t *>(_pins.second);
    HYDRO_SOFT_ASSERT(pins, SFP(HStr_Err_AllocationFailure));

    if (pins) {
        pins[0] = (pintype_t)DF_KEY_UP;
        pins[1] = (pintype_t)DF_KEY_DOWN;
        pins[2] = (pintype_t)DF_KEY_SELECT;
        pins[3] = (pintype_t)DF_KEY_LEFT;
        pins[4] = (pintype_t)DF_KEY_RIGHT;
    }
    #if NUM_ANALOG_INPUTS > 0
        pinMode(A0, INPUT);
    #endif
}

HydroInputUpDownButtons::~HydroInputUpDownButtons()
{
    if (_dfRobotIORef) {
        pintype_t *pins = const_cast<pintype_t *>(_pins.second);
        if (pins) { delete [] pins; }
    }
}

void HydroInputUpDownButtons::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    menuMgr.initForUpDownOk(renderer, initialItem, _pins.second[1], _pins.second[0], _pins.second[2], _keySpeed);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) menuMgr.setBackButton(_pins.second[3]);
    if (_pins.first > 4 && isValidPin(_pins.second[4])) menuMgr.setNextButton(_pins.second[4]);
}

HydroInputESP32TouchKeys::HydroInputESP32TouchKeys(Pair<uint8_t, const pintype_t *> controlPins, uint16_t keyRepeatSpeed, uint16_t switchThreshold, Hydro_ESP32Touch_HighRef highVoltage, Hydro_ESP32Touch_LowRef lowVoltage, Hydro_ESP32Touch_HighRefAtten attenuation)
    : HydroInputDriver(controlPins), _keySpeed(keyRepeatSpeed)
#ifdef ESP32
      , _esp32Touch(switchThreshold,
                    highVoltage == Hydro_ESP32Touch_HighRef_V_2V4 ? TOUCH_HVOLT_2V4 : highVoltage == Hydro_ESP32Touch_HighRef_V_2V5 ? TOUCH_HVOLT_2V5 :
                    highVoltage == Hydro_ESP32Touch_HighRef_V_2V6 ? TOUCH_HVOLT_2V6 : highVoltage == Hydro_ESP32Touch_HighRef_V_2V7 ? TOUCH_HVOLT_2V7 : 
                    highVoltage == Hydro_ESP32Touch_HighRef_Max ? TOUCH_HVOLT_MAX : TOUCH_HVOLT_KEEP,
                    lowVoltage == Hydro_ESP32Touch_LowRef_V_0V5 ? TOUCH_LVOLT_0V5 : lowVoltage == Hydro_ESP32Touch_LowRef_V_0V6 ? TOUCH_LVOLT_0V6 :
                    lowVoltage == Hydro_ESP32Touch_LowRef_V_0V7 ? TOUCH_LVOLT_0V7 : lowVoltage == Hydro_ESP32Touch_LowRef_V_0V8 ? TOUCH_LVOLT_0V8 :
                    lowVoltage == Hydro_ESP32Touch_LowRef_Max ? TOUCH_LVOLT_MAX : TOUCH_LVOLT_KEEP,
                    attenuation == Hydro_ESP32Touch_HighRefAtten_V_0V ? TOUCH_HVOLT_ATTEN_0V : attenuation == Hydro_ESP32Touch_HighRefAtten_V_0V5 ? TOUCH_HVOLT_ATTEN_0V5 :
                    attenuation == Hydro_ESP32Touch_HighRefAtten_V_1V ? TOUCH_HVOLT_ATTEN_1V : attenuation == Hydro_ESP32Touch_HighRefAtten_V_1V5 ? TOUCH_HVOLT_ATTEN_1V5 :
                    attenuation == Hydro_ESP32Touch_HighRefAtten_Max ? TOUCH_HVOLT_ATTEN_MAX : TOUCH_HVOLT_ATTEN_KEEP)
#endif
{ ; }

void HydroInputESP32TouchKeys::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    menuMgr.initForUpDownOk(renderer, initialItem, _pins.second[1], _pins.second[0], _pins.second[2], _keySpeed);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) menuMgr.setBackButton(_pins.second[3]);
    if (_pins.first > 4 && isValidPin(_pins.second[4])) menuMgr.setNextButton(_pins.second[4]);
    #ifdef ESP32
        _esp32Touch.ensureInterruptRegistered();
    #endif
}


HydroInputJoystick::HydroInputJoystick(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, float decreaseDivisor, float jsCenterX, float jsCenterY, float jsZeroTol)
    : HydroInputDriver(controlPins), _repeatDelay(repeatDelay), _decreaseDivisor(decreaseDivisor),
      _joystickCalib{jsCenterX, jsCenterY, jsZeroTol},
      _joystickMultiIo(200),
      _joystickIoXAxis(internalAnalogIo(), controlPins.second[0], jsCenterX)
{
    multiIoAddExpander(&_joystickMultiIo, &_joystickIoXAxis, 5);
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


HydroInputMatrix2x2::HydroInputMatrix2x2(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval)
    : HydroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(2,2,SFP(HStr_UI_MatrixActions).c_str()),
      _tcMenuKeyListener(SFP(HStr_UI_MatrixActions)[0], SFP(HStr_UI_MatrixActions)[1], SFP(HStr_UI_MatrixActions)[2], SFP(HStr_UI_MatrixActions)[3])
{
    // todo expander setup
    _keyboardLayout.setRowPin(0, controlPins.second[0]);
    _keyboardLayout.setRowPin(1, controlPins.second[1]);
    _keyboardLayout.setColPin(0, controlPins.second[2]);
    _keyboardLayout.setColPin(1, controlPins.second[3]);
    _keyboard.setRepeatKeyMillis(repeatDelay, repeatInterval);
}

void HydroInputMatrix2x2::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    _keyboard.initialise(internalDigitalIo(), &_keyboardLayout, &_tcMenuKeyListener, false);
}


static String getMatrix3x4KBKeys()
{
    static const String kb3x4(F(HYDRO_UI_3X4MATRIX_KEYS));
    return kb3x4;
}

HydroInputMatrix3x4::HydroInputMatrix3x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Hydro_EncoderSpeed encoderSpeed)
    : HydroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(4,3,SFP(HStr_UI_Matrix3x4Keys).c_str()),
      _tcMenuKeyListener(SFP(HStr_UI_MatrixActions)[0], SFP(HStr_UI_MatrixActions)[1], SFP(HStr_UI_MatrixActions)[2], SFP(HStr_UI_MatrixActions)[3]),
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
    _keyboard.setRepeatKeyMillis(repeatDelay, repeatInterval);

    if (isValidPin(controlPins.second[7])) {
        _rotaryEncoder = new HydroInputRotary(make_pair((uint8_t)(controlPins.first - 7), &controlPins.second[7]), encoderSpeed);
    }
}

HydroInputMatrix3x4::~HydroInputMatrix3x4()
{
    if (_rotaryEncoder) { delete _rotaryEncoder; }
}

void HydroInputMatrix3x4::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    _keyboard.initialise(internalDigitalIo(), &_keyboardLayout, &_tcMenuKeyListener, false);
    if (_rotaryEncoder) { _rotaryEncoder->begin(renderer, initialItem); }
}


static String getMatrix4x4KBKeys()
{
    static const String kb4x4(F(HYDRO_UI_4X4MATRIX_KEYS));
    return kb4x4;
}

HydroInputMatrix4x4::HydroInputMatrix4x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Hydro_EncoderSpeed encoderSpeed)
    : HydroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(4,4,SFP(HStr_UI_Matrix4x4Keys).c_str()),
      _tcMenuKeyListener(SFP(HStr_UI_MatrixActions)[0], SFP(HStr_UI_MatrixActions)[1], SFP(HStr_UI_MatrixActions)[2], SFP(HStr_UI_MatrixActions)[3]),
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
    _keyboardLayout.setColPin(3, controlPins.second[7]);
    _keyboard.setRepeatKeyMillis(repeatDelay, repeatInterval);

    if (isValidPin(controlPins.second[8])) {
        _rotaryEncoder = new HydroInputRotary(make_pair((uint8_t)(controlPins.first - 8), &controlPins.second[8]), encoderSpeed);
    }
}

HydroInputMatrix4x4::~HydroInputMatrix4x4()
{
    if (_rotaryEncoder) { delete _rotaryEncoder; }
}

void HydroInputMatrix4x4::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    _keyboard.initialise(internalDigitalIo(), &_keyboardLayout, &_tcMenuKeyListener, false);
    if (_rotaryEncoder) { _rotaryEncoder->begin(renderer, initialItem); }
}


HydroInputResistiveTouch::HydroInputResistiveTouch(Pair<uint8_t, const pintype_t *> controlPins, HydroDisplayDriver *displayDriver)
    : HydroInputDriver(controlPins),
      _touchInterrogator(controlPins.second[0], controlPins.second[1], controlPins.second[2], controlPins.second[3]),
      _touchOrientation(
         /*swap*/ displayDriver->getRotation() == Hydro_DisplayOrientation_R1 || displayDriver->getRotation() == Hydro_DisplayOrientation_R3,
         /*invX*/ displayDriver->getRotation() == Hydro_DisplayOrientation_R1 || displayDriver->getRotation() == Hydro_DisplayOrientation_R2 || displayDriver->getRotation() == Hydro_DisplayOrientation_HorzMirror,
         /*invY*/ displayDriver->getRotation() == Hydro_DisplayOrientation_R3 || displayDriver->getRotation() == Hydro_DisplayOrientation_R2 || displayDriver->getRotation() == Hydro_DisplayOrientation_VertMirror
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
      #ifndef HYDRO_ENABLE_XPT2046TS
          _touchScreen(),
      #else
          _touchScreen(controlPins.second[0], controlPins.second[1]),
      #endif
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
         /*swap*/ displayDriver->getRotation() == Hydro_DisplayOrientation_R1 || displayDriver->getRotation() == Hydro_DisplayOrientation_R3,
         /*invX*/ displayDriver->getRotation() == Hydro_DisplayOrientation_R1 || displayDriver->getRotation() == Hydro_DisplayOrientation_R2 || displayDriver->getRotation() == Hydro_DisplayOrientation_HorzMirror,
         /*invY*/ displayDriver->getRotation() == Hydro_DisplayOrientation_R3 || displayDriver->getRotation() == Hydro_DisplayOrientation_R2 || displayDriver->getRotation() == Hydro_DisplayOrientation_VertMirror
      ),
      _touchScreen(&_touchInterrogator, displayDriver->getGraphicsRenderer(), _touchOrientation)
{ ; }

void HydroInputTFTTouch::begin(MenuRenderer *renderer, MenuItem *initialItem)
{
    _touchScreen.start();
    menuMgr.initWithoutInput(renderer, initialItem);
}

#endif
