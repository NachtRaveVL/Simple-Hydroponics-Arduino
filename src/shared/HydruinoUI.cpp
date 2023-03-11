/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Base  UI
*/

#include "Hydruino.h"
#include "HydruinoUI.h"

const ConnectorLocalInfo applicationInfo = { "Simple-Hydroponics-Arduino", "dfa1e3a9-a13a-4af3-9133-956a6221615b" };

HydruinoBaseUI::HydruinoBaseUI(UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts)
    : _uiCtrlSetup(uiControlSetup), _uiDispSetup(uiDisplaySetup),
      _isActiveLow(isActiveLowIO), _allowISR(allowInterruptableIO), _utf8Fonts(enableTcUnicodeFonts),
      _gfxOrTFT(getController() && getController()->getDisplayOutputMode() >= Hydro_DisplayOutputMode_ST7735 && getController()->getDisplayOutputMode() <= Hydro_DisplayOutputMode_TFT),
      _menuRoot(nullptr), _input(nullptr), _display(nullptr), _remoteServer(nullptr)
{ ; }

HydruinoBaseUI::~HydruinoBaseUI()
{
    while (_remotes.size()) {
        delete (*_remotes.begin());
        _remotes.erase(_remotes.begin());
    }
    if (_input) { delete _input; }
    if (_display) { delete _display; }
    if (_remoteServer) { delete _remoteServer; }
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
        _display->init(); // calls back into above init with default settings for display
    } else {
        init(HYDRO_UI_UPDATE_SPEED, Hydro_DisplayTheme_Undefined);
    }
}

bool HydruinoBaseUI::begin()
{
    if (_display) { _display->begin(); }
    if (_input) { _input->begin(_display ? _display->getBaseRenderer() : nullptr, _menuRoot); }
    else { menuMgr.initWithoutInput(_display ? _display->getBaseRenderer() : nullptr, _menuRoot); }

    return (_display && (_input || _remotes.size())) || _remotes.size();
}

void HydruinoBaseUI::setNeedsLayout()
{ ; } // todo


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
