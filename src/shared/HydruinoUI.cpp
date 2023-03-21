/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Base UI
*/

#include "HydruinoUI.h"
#ifdef HYDRO_USE_GUI

HydruinoBaseUI::HydruinoBaseUI(UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts)
    : _appInfo{0}, _uiCtrlSetup(uiControlSetup), _uiDispSetup(uiDisplaySetup),
      _isActiveLow(isActiveLowIO), _allowISR(allowInterruptableIO), _isUnicodeFonts(enableTcUnicodeFonts),
      _input(nullptr), _display(nullptr), _remoteServer(nullptr), _backlight(nullptr), _blTimeout(0),
      _overview(nullptr), _homeMenu(nullptr)
{
    if (getController()) { strncpy(_appInfo.name, getController()->getSystemNameChars(), 30); }
    String uuid(F("dfa1e3a9-a13a-4af3-9133-956a6221615b")); // todo, name->hash
    strncpy(_appInfo.uuid, uuid.c_str(), 38);
    pintype_t ledPin = uiDisplaySetup.getBacklightPin();
    if (uiDisplaySetup.dispCfgType != UIDisplaySetup::LCD && isValidPin(ledPin)) { // LCD has its own backlight
        switch (uiDisplaySetup.getBacklightMode()) {
            case Hydro_BacklightMode_Inverted:
                _backlight = new HydroDigitalPin(ledPin, OUTPUT, ACT_LOW, hpinchnl_none);
                break;
            case Hydro_BacklightMode_PWM:
                _backlight = new HydroAnalogPin(ledPin, OUTPUT, uiDisplaySetup.getBacklightBitRes(),
#ifdef ESP32
                                                uiDisplaySetup.getBacklightChannel(),
#endif
#ifdef ESP_PLATFORM
                                                uiDisplaySetup.getBacklightFrequency(),
#endif
                                                hpinchnl_none);
                break;
            default: // Normal
                _backlight = new HydroDigitalPin(ledPin, OUTPUT, ACT_HIGH, hpinchnl_none);
                break;
        }
        HYDRO_SOFT_ASSERT(_backlight, SFP(HStr_Err_AllocationFailure));

        if (_backlight) { _backlight->init(); }
    }
}

HydruinoBaseUI::~HydruinoBaseUI()
{
    if (_overview) { delete _overview; }
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
        bool mainPinsInterruptable = _input && _input->areMainPinsInterruptable();
        bool allPinsInterruptable = mainPinsInterruptable && _input->areAllPinsInterruptable();
        isrMode = (allPinsInterruptable ? SWITCHES_NO_POLLING : (mainPinsInterruptable ? SWITCHES_POLL_KEYS_ONLY : SWITCHES_POLL_EVERYTHING));
    }

    switches.init(_input && _input->getIoAbstraction() ? _input->getIoAbstraction() : internalDigitalIo(), isrMode, _isActiveLow);

    if (_display) { _display->commonInit(updatesPerSec, displayTheme, analogSlider, _isUnicodeFonts); }
    #if !HYDRO_UI_START_AT_OVERVIEW
        if (!_homeMenu) {
            _homeMenu = new HydroHomeMenu();
            HYDRO_SOFT_ASSERT(_homeMenu, SFP(HStr_Err_AllocationFailure));
        }
    #endif
}

void HydruinoBaseUI::init(HydroUIData *uiData)
{
    if ((_uiData = uiData)) {
        init(uiData->updatesPerSec, uiData->displayTheme, _display && _display->isColor() ? HYDRO_UI_GFX_VARS_USES_SLIDER : false);
    } else if (_display) {
        _display->initBaseUIFromDefaults(); // calls back into above init with default settings for display
    } else {
        init(HYDRO_UI_UPDATE_SPEED, Hydro_DisplayTheme_Undefined);
    }
}

bool HydruinoBaseUI::begin()
{
    if (_display) { _display->begin(); }
    if (_input) { _input->begin(_display ? _display->getBaseRenderer() : nullptr, _homeMenu ? _homeMenu->getRootItem() : nullptr); }
    else { menuMgr.initWithoutInput(_display ? _display->getBaseRenderer() : nullptr, _homeMenu ? _homeMenu->getRootItem() : nullptr); }

    #if HYDRO_UI_START_AT_OVERVIEW
        reset();
    #endif
    setBacklightEnable(true);

    return (_display && (_input || _remotes.size())) || _remotes.size();
}

void HydruinoBaseUI::setNeedsRedraw()
{
    if (_overview) { _overview->setNeedsFullRedraw(); }
    else { menuMgr.notifyStructureChanged(); }
}

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
            _blTimeout = 0;
        }
    } else if (_uiDispSetup.dispCfgType == UIDisplaySetup::LCD && _display) {
        if (enabled) {
            if (_uiDispSetup.getBacklightMode() != Hydro_BacklightMode_PWM) {
                ((HydroDisplayLiquidCrystal *)_display)->getLCD().backlight();
            } else {
                ((HydroDisplayLiquidCrystal *)_display)->getLCD().setBacklight(255);
            }
        } else {
            if (_uiDispSetup.getBacklightMode() != Hydro_BacklightMode_PWM) {
                ((HydroDisplayLiquidCrystal *)_display)->getLCD().noBacklight();
            } else {
                ((HydroDisplayLiquidCrystal *)_display)->getLCD().setBacklight(0); // todo: nice backlight-out anim
            }
            _blTimeout = 0;
        }
    }
}

void HydruinoBaseUI::started(BaseMenuRenderer *currentRenderer)
{
    // overview screen started
    if (_display) {
        #if HYDRO_UI_DEALLOC_AFTER_USE
            if (_homeMenu) { delete _homeMenu; _homeMenu = nullptr; menuMgr.changeMenu(); }
        #endif

        if (!_overview) {
            _overview = _display->allocateOverview();
            HYDRO_SOFT_ASSERT(_overview, SFP(HStr_Err_AllocationFailure));
        }
    }
}

void HydruinoBaseUI::reset()
{
    // menu interaction timeout
    if (_display) {
        _display->getBaseRenderer()->takeOverDisplay();
        _blTimeout = (_backlight || (_uiDispSetup.dispCfgType == UIDisplaySetup::LCD && _display)) ? unixNow() + HYDRO_UI_BACKLIGHT_TIMEOUT : 0;
    }
}

void HydruinoBaseUI::renderLoop(unsigned int currentValue, RenderPressMode userClick)
{
    // render overview screen until key interruption
    if (_display) {
        if (userClick == RPRESS_NONE) {
            if (_overview) { _overview->renderOverview(_display->isLandscape(), _display->getScreenSize()); }

            if (_blTimeout && unixNow() >= _blTimeout) {
                setBacklightEnable(false);
            }
        } else {
            #if HYDRO_UI_DEALLOC_AFTER_USE
                if (_overview) { delete _overview; _overview = nullptr; }
            #endif
            if (!_homeMenu) {
                _homeMenu = new HydroHomeMenu();
                HYDRO_SOFT_ASSERT(_homeMenu, SFP(HStr_Err_AllocationFailure));

                if (_homeMenu) {
                    menuMgr.changeMenu(_homeMenu->getRootItem());
                }
            }
            _display->getBaseRenderer()->giveBackDisplay();
            setBacklightEnable(true);
            _blTimeout = 0;
        }
    }
}

#endif
