/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Base  UI
*/

#include "Hydruino.h"
#include "HydruinoUI.h"

HydruinoBaseUI::HydruinoBaseUI(UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts)
    : _appInfo{0}, _uiCtrlSetup(uiControlSetup), _uiDispSetup(uiDisplaySetup),
      _isActiveLow(isActiveLowIO), _allowISR(allowInterruptableIO), _utf8Fonts(enableTcUnicodeFonts),
      _gfxOrTFT(getController() && getController()->getDisplayOutputMode() >= Hydro_DisplayOutputMode_ST7735 && getController()->getDisplayOutputMode() <= Hydro_DisplayOutputMode_TFT),
      _menuRoot(nullptr), _input(nullptr), _display(nullptr), _remoteServer(nullptr), _backlight(nullptr)
{
    if (getController()) { strncpy(_appInfo.name, getController()->getSystemNameChars(), 30); }
    String uuid(F("dfa1e3a9-a13a-4af3-9133-956a6221615b")); // todo, name->hash
    strncpy(_appInfo.uuid, uuid.c_str(), 38);
    pintype_t ledPin = uiDisplaySetup.getBacklightPin();
    if (uiDisplaySetup.dispCfgType != UIDisplaySetup::LCD && isValidPin(ledPin)) { // LCD has its own backlight
        switch (uiDisplaySetup.getBacklightMode()) {
            case Hydro_BacklightMode_Inverted:
                _backlight = new HydroDigitalPin(ledPin, OUTPUT, ACT_LOW);
                break;
            case Hydro_BacklightMode_PWM:
                _backlight = new HydroAnalogPin(ledPin, OUTPUT); // todo dflt ESP channel/freq
                break;
            default: // Normal
                _backlight = new HydroDigitalPin(ledPin, OUTPUT, ACT_HIGH);
                break;
        }
        HYDRO_SOFT_ASSERT(_backlight, SFP(HStr_Err_AllocationFailure));

        if (_backlight) { _backlight->init(); }
    }
}

HydruinoBaseUI::~HydruinoBaseUI()
{
    while (_remotes.size()) {
        delete (*_remotes.begin());
        _remotes.erase(_remotes.begin());
    }
    if (_input) { delete _input; }
    if (_display) { delete _display; }
    if (_remoteServer) { delete _remoteServer; }
    if (_backlight) { delete _backlight; }
}

void HydruinoBaseUI::init(uint8_t updatesPerSec, Hydro_DisplayTheme displayTheme, bool analogSlider)
{
    SwitchInterruptMode isrMode(SWITCHES_POLL_EVERYTHING);
    if (_allowISR) {
        bool allInputInterruptable = false; // todo
        bool nonKeysInterruptable = false; // todo
        isrMode = (allInputInterruptable ? SWITCHES_NO_POLLING : (nonKeysInterruptable ? SWITCHES_POLL_KEYS_ONLY : SWITCHES_POLL_EVERYTHING));
    }

    switches.init(_input && _input->getIoAbstraction() ? _input->getIoAbstraction() : internalDigitalIo(), isrMode, _isActiveLow);

    if (_display) { _display->commonInit(updatesPerSec, displayTheme, analogSlider, _utf8Fonts); }
}

void HydruinoBaseUI::init()
{
    const HydroUIData *uiData = nullptr; // todo
    if (uiData) {
        init(uiData->updatesPerSec, uiData->displayTheme, _gfxOrTFT ? HYDRO_UI_GFXTFT_USES_AN_SLIDER : false);
    } else if (_display) {
        _display->initBaseUIFromDefaults(); // calls back into above init with default settings for display
    } else {
        init(HYDRO_UI_UPDATE_SPEED, Hydro_DisplayTheme_Undefined);
    }
}

RENDERING_CALLBACK_NAME_INVOKE(fnTesting123RtCall, textItemRenderFn, "Testing123", -1, NO_CALLBACK) // tbr
TextMenuItem menuTesting123(fnTesting123RtCall, "", 1, 5, NULL);

bool HydruinoBaseUI::begin()
{
    if (_display) { _display->begin(); }
    if (_input) { _input->begin(_display ? _display->getBaseRenderer() : nullptr, (_menuRoot = &menuTesting123)); }
    else { menuMgr.initWithoutInput(_display ? _display->getBaseRenderer() : nullptr, (_menuRoot = &menuTesting123)); }

    reset();
    setBacklightEnable(true);

    return (_display && (_input || _remotes.size())) || _remotes.size();
}

void HydruinoBaseUI::setNeedsLayout()
{ ; } // todo

void HydruinoBaseUI::setBacklightEnable(bool enabled)
{
    if (_uiDispSetup.dispCfgType != UIDisplaySetup::LCD && _backlight) {
        if (enabled) {
            if (_backlight->isDigitalType()) {
                ((HydroDigitalPin *)_backlight)->activate();
            } else {
                ((HydroAnalogPin *)_backlight)->analogWrite(1.0f);
            }
        } else {
            if (_backlight->isDigitalType()) {
                ((HydroDigitalPin *)_backlight)->deactivate();
            } else {
                ((HydroAnalogPin *)_backlight)->analogWrite(0.0f); // todo: nice backlight-out anim
            }
        }
    } else if (_uiDispSetup.dispCfgType == UIDisplaySetup::LCD && _display) {
        if (enabled) {
            if (_uiDispSetup.getBacklightMode() != Hydro_BacklightMode_PWM) {
                ((HydroDisplayLiquidCrystalIO *)_display)->getLCD().backlight();
            } else {
                ((HydroDisplayLiquidCrystalIO *)_display)->getLCD().setBacklight(255);
            }
        } else {
            if (_uiDispSetup.getBacklightMode() != Hydro_BacklightMode_PWM) {
                ((HydroDisplayLiquidCrystalIO *)_display)->getLCD().noBacklight();
            } else {
                ((HydroDisplayLiquidCrystalIO *)_display)->getLCD().setBacklight(0); // todo: nice backlight-out anim
            }
        }
    }
}

void HydruinoBaseUI::started(BaseMenuRenderer *currentRenderer)
{
    // overview screen started
    if (_display) {
        _display->clearScreen(); // tbr
        // todo
    }
}

void HydruinoBaseUI::reset()
{
    // user interaction timeout
    if (_display) {
        _display->getBaseRenderer()->takeOverDisplay();
    }
}

void HydruinoBaseUI::renderLoop(unsigned int currentValue, RenderPressMode userClick)
{
    // render overview screen until key interruption
    if (_display) {
        if (userClick == RPRESS_NONE) {
            // todo: custom render loop
        } else {
            _display->getBaseRenderer()->giveBackDisplay();
            setBacklightEnable(true);
        }
    }
}


HydroUIData::HydroUIData()
    : HydroData('H','U','I','D', 1),
      updatesPerSec(HYDRO_UI_UPDATE_SPEED), displayTheme(Hydro_DisplayTheme_Undefined), joystickCalib{0.5f,0.5f,0.05f}
{
    _size = sizeof(*this);
}

void HydroUIData::toJSONObject(JsonObject &objectOut) const
{
    HydroData::toJSONObject(objectOut);

    if (updatesPerSec != HYDRO_UI_UPDATE_SPEED) { objectOut[SFP(HStr_Key_UpdatesPerSec)] = updatesPerSec; }
    if (displayTheme != Hydro_DisplayTheme_Undefined) { objectOut[SFP(HStr_Key_DisplayTheme)] = displayTheme; }
    if (!isFPEqual(joystickCalib[0], 0.5f) || !isFPEqual(joystickCalib[1], 0.5f) || !isFPEqual(joystickCalib[2], 0.05f)) { objectOut[SFP(HStr_Key_JoystickCalib)] = commaStringFromArray(joystickCalib, 3); }
}

void HydroUIData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroData::fromJSONObject(objectIn);

    updatesPerSec = objectIn[SFP(HStr_Key_UpdatesPerSec)] | updatesPerSec;
    displayTheme = objectIn[SFP(HStr_Key_DisplayTheme)] | displayTheme;
    JsonVariantConst joystickCalibVar = objectIn[SFP(HStr_Key_JoystickCalib)];
    commaStringToArray(joystickCalibVar, joystickCalib, 3);
}
