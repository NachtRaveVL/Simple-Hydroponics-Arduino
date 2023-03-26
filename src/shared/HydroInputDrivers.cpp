/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Input Drivers
*/

#include "HydruinoUI.h"
#ifdef HYDRO_USE_GUI
#include <DfRobotInputAbstraction.h>

static const char _matrix2x2Keys[] PROGMEM = {HYDRO_UI_2X2MATRIX_KEYS};
static const char _matrix3x4Keys[] PROGMEM = {HYDRO_UI_3X4MATRIX_KEYS};
static const char _matrix4x4Keys[] PROGMEM = {HYDRO_UI_4X4MATRIX_KEYS};


bool HydroInputDriver::areAllPinsInterruptable() const
{
    for (int i = 0; i < _pins.first; ++i) {
        if (!(isValidPin(_pins.second[i]) && checkPinCanInterrupt(_pins.second[i]))) {
            return false;
        }
    }
    return true;
}


HydroInputRotary::HydroInputRotary(Pair<uint8_t, const pintype_t *> controlPins, Hydro_EncoderSpeed encoderSpeed)
    : HydroInputDriver(controlPins), _encoderSpeed(encoderSpeed)
{ ; }

void HydroInputRotary::begin(HydroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    #ifndef HYDRO_DISABLE_MULTITASKING
        auto expander = getController() && _pins.first > 0 && isValidPin(_pins.second[0]) && _pins.second[0] >= hpin_virtual ? getController()->getPinExpander(expanderForPinNumber(_pins.second[0])) : nullptr;
        switches.init(expander && expander->getIoAbstraction() ? expander->getIoAbstraction() : (getIoAbstraction() ?: internalDigitalIo()),
                      getBaseUI() ? getBaseUI()->getISRMode() : SWITCHES_POLL_EVERYTHING,
                      getBaseUI() && getBaseUI()->isActiveLow());
    #else
        switches.init(getIoAbstraction() ?: internalDigitalIo(),
                      getBaseUI() ? getBaseUI()->getISRMode() : SWITCHES_POLL_EVERYTHING,
                      getBaseUI() && getBaseUI()->isActiveLow());
    #endif

    menuMgr.initForEncoder(displayDriver->getBaseRenderer(), initialItem, _pins.second[0], _pins.second[1], _pins.second[2],
                           _encoderSpeed == Hydro_EncoderSpeed_FullCycle ? FULL_CYCLE : _encoderSpeed == Hydro_EncoderSpeed_HalfCycle ? HALF_CYCLE : QUARTER_CYCLE);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) { menuMgr.setBackButton(_pins.second[3]); }
    if (_pins.first > 4 && isValidPin(_pins.second[4])) { menuMgr.setNextButton(_pins.second[4]); }
}

bool HydroInputRotary::areMainPinsInterruptable() const
{
    return _pins.first >= 3 &&
           isValidPin(_pins.second[0]) && checkPinCanInterrupt(_pins.second[0]) &&
           isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]) &&
           isValidPin(_pins.second[2]) && checkPinCanInterrupt(_pins.second[2]);
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

void HydroInputUpDownButtons::begin(HydroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    if (_dfRobotIORef) {
        switches.initialise(_dfRobotIORef, getBaseUI() && getBaseUI()->isActiveLow());
    } else {
        #ifndef HYDRO_DISABLE_MULTITASKING
            auto expander = getController() && _pins.first > 0 && isValidPin(_pins.second[0]) && _pins.second[0] >= hpin_virtual ? getController()->getPinExpander(expanderForPinNumber(_pins.second[0])) : nullptr;
            switches.init(expander && expander->getIoAbstraction() ? expander->getIoAbstraction() : (getIoAbstraction() ?: internalDigitalIo()),
                          getBaseUI() ? getBaseUI()->getISRMode() : SWITCHES_POLL_EVERYTHING,
                          getBaseUI() && getBaseUI()->isActiveLow());
        #else
            switches.init(getIoAbstraction() ?: internalDigitalIo(),
                          getBaseUI() ? getBaseUI()->getISRMode() : SWITCHES_POLL_EVERYTHING,
                          getBaseUI() && getBaseUI()->isActiveLow());
        #endif
    }

    menuMgr.initForUpDownOk(displayDriver->getBaseRenderer(), initialItem, _pins.second[1], _pins.second[0], _pins.second[2], _keySpeed);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) { menuMgr.setBackButton(_pins.second[3]); }
    if (_pins.first > 4 && isValidPin(_pins.second[4])) { menuMgr.setNextButton(_pins.second[4]); }
}

bool HydroInputUpDownButtons::areMainPinsInterruptable() const
{
    return _pins.first >= 3 &&
           isValidPin(_pins.second[0]) && checkPinCanInterrupt(_pins.second[0]) &&
           isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]) &&
           isValidPin(_pins.second[2]) && checkPinCanInterrupt(_pins.second[2]);
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

void HydroInputESP32TouchKeys::begin(HydroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    switches.init(getIoAbstraction() ?: internalDigitalIo(),
                  getBaseUI() ? getBaseUI()->getISRMode() : SWITCHES_POLL_EVERYTHING,
                  getBaseUI() && getBaseUI()->isActiveLow());

    menuMgr.initForUpDownOk(displayDriver->getBaseRenderer(), initialItem, _pins.second[1], _pins.second[0], _pins.second[2], _keySpeed);
    if (_pins.first > 3 && isValidPin(_pins.second[3])) { menuMgr.setBackButton(_pins.second[3]); }
    if (_pins.first > 4 && isValidPin(_pins.second[4])) { menuMgr.setNextButton(_pins.second[4]); }
    #ifdef ESP32
        _esp32Touch.ensureInterruptRegistered();
    #endif
}

bool HydroInputESP32TouchKeys::areMainPinsInterruptable() const
{
    return _pins.first >= 3 &&
           isValidPin(_pins.second[0]) && checkPinCanInterrupt(_pins.second[0]) &&
           isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]) &&
           isValidPin(_pins.second[2]) && checkPinCanInterrupt(_pins.second[2]);
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

void HydroInputJoystick::begin(HydroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    switches.init(getIoAbstraction() ?: internalDigitalIo(),
                  getBaseUI() ? getBaseUI()->getISRMode() : SWITCHES_POLL_EVERYTHING,
                  getBaseUI() && getBaseUI()->isActiveLow());

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

    menuMgr.initWithoutInput(displayDriver->getBaseRenderer(), initialItem);
}

bool HydroInputJoystick::areMainPinsInterruptable() const
{
    return _pins.first >= 2 &&
           isValidPin(_pins.second[0]) && checkPinCanInterrupt(_pins.second[0]) &&
           isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]);
}


HydroInputMatrix2x2::HydroInputMatrix2x2(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval)
    : HydroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(2, 2, _matrix2x2Keys),
      _tcMenuKeyListener(SFP(HUIStr_Keys_MatrixActions)[0], SFP(HUIStr_Keys_MatrixActions)[1], SFP(HUIStr_Keys_MatrixActions)[2], SFP(HUIStr_Keys_MatrixActions)[3])
{
    _keyboardLayout.setRowPin(0, controlPins.second[0]);
    _keyboardLayout.setRowPin(1, controlPins.second[1]);
    _keyboardLayout.setColPin(0, controlPins.second[2]);
    _keyboardLayout.setColPin(1, controlPins.second[3]);
    _keyboard.setRepeatKeyMillis(repeatDelay, repeatInterval);
}

void HydroInputMatrix2x2::begin(HydroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    #ifndef HYDRO_DISABLE_MULTITASKING
        auto expander = getController() && _pins.first > 0 && isValidPin(_pins.second[0]) && _pins.second[0] >= hpin_virtual ? getController()->getPinExpander(expanderForPinNumber(_pins.second[0])) : nullptr;
        _keyboard.initialise(expander && expander->getIoAbstraction() ? expander->getIoAbstraction() : (getIoAbstraction() ?: internalDigitalIo()),
                             &_keyboardLayout, &_tcMenuKeyListener,
                             (!getBaseUI() || getBaseUI()->allowingISR()) && areRowPinsInterruptable());
    #else
        _keyboard.initialise(getIoAbstraction() ?: internalDigitalIo(),
                             &_keyboardLayout, &_tcMenuKeyListener,
                             (!getBaseUI() || getBaseUI()->allowingISR()) && areRowPinsInterruptable());
    #endif
}

bool HydroInputMatrix2x2::areRowPinsInterruptable() const
{
    return _pins.first >= 2 &&
           isValidPin(_pins.second[0]) && checkPinCanInterrupt(_pins.second[0]) &&
           isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]);
}

bool HydroInputMatrix2x2::areMainPinsInterruptable() const
{
    return areRowPinsInterruptable();
}


HydroInputMatrix3x4::HydroInputMatrix3x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Hydro_EncoderSpeed encoderSpeed)
    : HydroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(4, 3, _matrix3x4Keys),
      _tcMenuKeyListener(SFP(HUIStr_Keys_MatrixActions)[0], SFP(HUIStr_Keys_MatrixActions)[1], SFP(HUIStr_Keys_MatrixActions)[2], SFP(HUIStr_Keys_MatrixActions)[3]),
      _rotaryEncoder(nullptr)
{
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

void HydroInputMatrix3x4::begin(HydroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    #ifndef HYDRO_DISABLE_MULTITASKING
        auto expander = getController() && _pins.first > 0 && isValidPin(_pins.second[0]) && _pins.second[0] >= hpin_virtual ? getController()->getPinExpander(expanderForPinNumber(_pins.second[0])) : nullptr;
        _keyboard.initialise(expander && expander->getIoAbstraction() ? expander->getIoAbstraction() : (getIoAbstraction() ?: internalDigitalIo()),
                             &_keyboardLayout, &_tcMenuKeyListener,
                             (!getBaseUI() || getBaseUI()->allowingISR()) && areRowPinsInterruptable());
    #else
        _keyboard.initialise(getIoAbstraction() ?: internalDigitalIo(),
                             &_keyboardLayout, &_tcMenuKeyListener,
                             (!getBaseUI() || getBaseUI()->allowingISR()) && areRowPinsInterruptable());
    #endif

    if (_rotaryEncoder) { _rotaryEncoder->begin(displayDriver, initialItem); }
}

bool HydroInputMatrix3x4::areRowPinsInterruptable() const
{
    return _pins.first >= 4 &&
           isValidPin(_pins.second[0]) && checkPinCanInterrupt(_pins.second[0]) &&
           isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]) &&
           isValidPin(_pins.second[2]) && checkPinCanInterrupt(_pins.second[2]) &&
           isValidPin(_pins.second[3]) && checkPinCanInterrupt(_pins.second[3]);
}

bool HydroInputMatrix3x4::areMainPinsInterruptable() const
{
    return areRowPinsInterruptable() && (!_rotaryEncoder || _rotaryEncoder->areMainPinsInterruptable());
}


HydroInputMatrix4x4::HydroInputMatrix4x4(Pair<uint8_t, const pintype_t *> controlPins, millis_t repeatDelay, millis_t repeatInterval, Hydro_EncoderSpeed encoderSpeed)
    : HydroInputDriver(controlPins),
      _keyboard(),
      _keyboardLayout(4, 4, _matrix4x4Keys),
      _tcMenuKeyListener(SFP(HUIStr_Keys_MatrixActions)[0], SFP(HUIStr_Keys_MatrixActions)[1], SFP(HUIStr_Keys_MatrixActions)[2], SFP(HUIStr_Keys_MatrixActions)[3]),
      _rotaryEncoder(nullptr)
{
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

void HydroInputMatrix4x4::begin(HydroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    #ifndef HYDRO_DISABLE_MULTITASKING
        auto expander = getController() && _pins.first > 0 && isValidPin(_pins.second[0]) && _pins.second[0] >= hpin_virtual ? getController()->getPinExpander(expanderForPinNumber(_pins.second[0])) : nullptr;
        _keyboard.initialise(expander && expander->getIoAbstraction() ? expander->getIoAbstraction() : (getIoAbstraction() ?: internalDigitalIo()),
                             &_keyboardLayout, &_tcMenuKeyListener,
                             (!getBaseUI() || getBaseUI()->allowingISR()) && areRowPinsInterruptable());
    #else
        _keyboard.initialise(getIoAbstraction() ?: internalDigitalIo(),
                             &_keyboardLayout, &_tcMenuKeyListener,
                             (!getBaseUI() || getBaseUI()->allowingISR()) && areRowPinsInterruptable());
    #endif

    if (_rotaryEncoder) { _rotaryEncoder->begin(displayDriver, initialItem); }
}

bool HydroInputMatrix4x4::areRowPinsInterruptable() const
{
    return _pins.first >= 4 &&
           isValidPin(_pins.second[0]) && checkPinCanInterrupt(_pins.second[0]) &&
           isValidPin(_pins.second[1]) && checkPinCanInterrupt(_pins.second[1]) &&
           isValidPin(_pins.second[2]) && checkPinCanInterrupt(_pins.second[2]) &&
           isValidPin(_pins.second[3]) && checkPinCanInterrupt(_pins.second[3]);
}

bool HydroInputMatrix4x4::areMainPinsInterruptable() const
{
    return areRowPinsInterruptable() && (!_rotaryEncoder || _rotaryEncoder->areMainPinsInterruptable());
}


HydroInputResistiveTouch::HydroInputResistiveTouch(Pair<uint8_t, const pintype_t *> controlPins, HydroDisplayDriver *displayDriver, Hydro_DisplayRotation displayRotation, Hydro_TouchscreenOrientation touchOrient)
    : HydroInputDriver(controlPins),
      _touchInterrogator(controlPins.second[0], controlPins.second[1], controlPins.second[2], controlPins.second[3]),
      _touchOrientation(
         /*swap*/ (touchOrient == Hydro_TouchscreenOrientation_Same && (displayRotation == Hydro_DisplayRotation_R1 || displayRotation == Hydro_DisplayRotation_R3))
                  || (touchOrient == Hydro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY_SwapXY || touchOrient == Hydro_TouchscreenOrientation_SwapXY),
         /*invX*/ (touchOrient == Hydro_TouchscreenOrientation_Same && (displayRotation == Hydro_DisplayRotation_R1 || displayRotation == Hydro_DisplayRotation_R2 || displayRotation == Hydro_DisplayRotation_HorzMirror))
                  || (touchOrient == Hydro_TouchscreenOrientation_InvertX || touchOrient == Hydro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY_SwapXY),
         /*invY*/ (touchOrient == Hydro_TouchscreenOrientation_Same && (displayRotation == Hydro_DisplayRotation_R3 || displayRotation == Hydro_DisplayRotation_R2 || displayRotation == Hydro_DisplayRotation_VertMirror))
                  || (touchOrient == Hydro_TouchscreenOrientation_InvertY || touchOrient == Hydro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY_SwapXY)
      ),
      _touchScreen(&_touchInterrogator, displayDriver->getGraphicsRenderer(), _touchOrientation)
{ ; }

void HydroInputResistiveTouch::begin(HydroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    _touchScreen.start();
    menuMgr.initWithoutInput(displayDriver->getBaseRenderer(), initialItem);
}


HydroInputTouchscreen::HydroInputTouchscreen(Pair<uint8_t, const pintype_t *> controlPins, HydroDisplayDriver *displayDriver, Hydro_DisplayRotation displayRotation, Hydro_TouchscreenOrientation touchOrient)
    : HydroInputDriver(controlPins),
      #ifdef HYDRO_UI_ENABLE_XPT2046TS
          _touchScreen(controlPins.second[0], getBaseUI() && getBaseUI()->allowingISR() ? controlPins.second[1] : (uint8_t)0xff),
      #else
          _touchScreen(),
      #endif
      #ifdef HYDRO_UI_ENABLE_BSP_TOUCH
          _touchInterrogator(),
      #else
          _touchInterrogator(_touchScreen),
      #endif
      _touchOrientation(
         /*swap*/ (touchOrient == Hydro_TouchscreenOrientation_Same && (displayRotation == Hydro_DisplayRotation_R1 || displayRotation == Hydro_DisplayRotation_R3))
                  || (touchOrient == Hydro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY_SwapXY || touchOrient == Hydro_TouchscreenOrientation_SwapXY),
         /*invX*/ (touchOrient == Hydro_TouchscreenOrientation_Same && (displayRotation == Hydro_DisplayRotation_R1 || displayRotation == Hydro_DisplayRotation_R2 || displayRotation == Hydro_DisplayRotation_HorzMirror))
                  || (touchOrient == Hydro_TouchscreenOrientation_InvertX || touchOrient == Hydro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY_SwapXY),
         /*invY*/ (touchOrient == Hydro_TouchscreenOrientation_Same && (displayRotation == Hydro_DisplayRotation_R3 || displayRotation == Hydro_DisplayRotation_R2 || displayRotation == Hydro_DisplayRotation_VertMirror))
                  || (touchOrient == Hydro_TouchscreenOrientation_InvertY || touchOrient == Hydro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY_SwapXY)
      )
{ ; }

void HydroInputTouchscreen::begin(HydroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    #ifndef HYDRO_UI_ENABLE_XPT2046TS
        _touchInterrogator.init(displayDriver->getScreenSize(false).first, displayDriver->getScreenSize(false).second);
    #else
        _touchInterrogator.init(getBaseUI() ? getBaseUI()->getControlSetup().ctrlCfgAs.touchscreen.spiClass : nullptr);
    #endif
    menuMgr.initWithoutInput(displayDriver->getBaseRenderer(), initialItem);
    #ifdef HYDRO_UI_ENABLE_XPT2046TS
        _touchScreen.setRotation(getBaseUI()  ? (uint8_t)getBaseUI()->getDisplaySetup().getDisplayRotation() : 0);
    #endif
}


HydroInputTFTTouch::HydroInputTFTTouch(Pair<uint8_t, const pintype_t *> controlPins, HydroDisplayTFTeSPI *displayDriver, Hydro_DisplayRotation displayRotation, Hydro_TouchscreenOrientation touchOrient, bool useRawTouch)
    : HydroInputDriver(controlPins),
      _touchInterrogator(&displayDriver->getGfx(), useRawTouch),
      _touchOrientation(
         /*swap*/ (touchOrient == Hydro_TouchscreenOrientation_Same && (displayRotation == Hydro_DisplayRotation_R1 || displayRotation == Hydro_DisplayRotation_R3))
                  || (touchOrient == Hydro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY_SwapXY || touchOrient == Hydro_TouchscreenOrientation_SwapXY),
         /*invX*/ (touchOrient == Hydro_TouchscreenOrientation_Same && (displayRotation == Hydro_DisplayRotation_R1 || displayRotation == Hydro_DisplayRotation_R2 || displayRotation == Hydro_DisplayRotation_HorzMirror))
                  || (touchOrient == Hydro_TouchscreenOrientation_InvertX || touchOrient == Hydro_TouchscreenOrientation_InvertX_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY_SwapXY),
         /*invY*/ (touchOrient == Hydro_TouchscreenOrientation_Same && (displayRotation == Hydro_DisplayRotation_R3 || displayRotation == Hydro_DisplayRotation_R2 || displayRotation == Hydro_DisplayRotation_VertMirror))
                  || (touchOrient == Hydro_TouchscreenOrientation_InvertY || touchOrient == Hydro_TouchscreenOrientation_InvertY_SwapXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY || touchOrient == Hydro_TouchscreenOrientation_InvertXY_SwapXY)
      ),
      _touchScreen(&_touchInterrogator, displayDriver->getGraphicsRenderer(), _touchOrientation)
{ ; }

void HydroInputTFTTouch::begin(HydroDisplayDriver *displayDriver, MenuItem *initialItem)
{
    _touchInterrogator.init(displayDriver->getScreenSize(false).first, displayDriver->getScreenSize(false).second);
    _touchScreen.start();
    menuMgr.initWithoutInput(displayDriver->getBaseRenderer(), initialItem);
}

#endif
