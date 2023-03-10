/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Base  UI
*/

#include "Hydruino.h"
#include "HydruinoUI.h"

const ConnectorLocalInfo applicationInfo = { "Simple-Hydroponics-Arduino", "dfa1e3a9-a13a-4af3-9133-956a6221615b" };

HydruinoBaseUI::HydruinoBaseUI(UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts)
    : _isActiveLow(isActiveLowIO), _allowISR(allowInterruptableIO), _utf8Fonts(enableTcUnicodeFonts),
      _gfxOrTFT(getController() && getController()->getDisplayOutputMode() >= Hydro_DisplayOutputMode_ST7735 && getController()->getDisplayOutputMode() <= Hydro_DisplayOutputMode_TFT),
      _menuRoot(nullptr), _input(nullptr), _display(nullptr), _remoteServer(nullptr)
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    const HydroUIData *uiData = nullptr; // todo

    if (controller) {
        // Input driver setup
        auto ctrlInMode = controller->getControlInputMode();
        auto ctrlInPins = controller->getControlInputPins();
        switch (ctrlInMode) {
            case Hydro_ControlInputMode_RotaryEncoderOk:
            case Hydro_ControlInputMode_RotaryEncoderOk_LR:
                HYDRO_SOFT_ASSERT(uiControlSetup.ctrlCfgType == UIControlSetup::Encoder, SFP(HStr_Err_InvalidParameter));
                _input = new HydroInputRotary(ctrlInPins, uiControlSetup.ctrlCfgAs.encoder.encoderSpeed);
                break;

            case Hydro_ControlInputMode_UpDownOkButtons:
            case Hydro_ControlInputMode_UpDownOkButtons_LR:
                HYDRO_SOFT_ASSERT(uiControlSetup.ctrlCfgType == UIControlSetup::Buttons, SFP(HStr_Err_InvalidParameter));
                if (!uiControlSetup.ctrlCfgAs.buttons.isDFRobotShield) {
                    _input = new HydroInputUpDownButtons(ctrlInPins, uiControlSetup.ctrlCfgAs.buttons.repeatSpeed);
                } else {
                    _input = new HydroInputUpDownButtons(true, uiControlSetup.ctrlCfgAs.buttons.repeatSpeed);
                }
                break;

            case Hydro_ControlInputMode_AnalogJoystickOk:
                HYDRO_SOFT_ASSERT(uiControlSetup.ctrlCfgType == UIControlSetup::Joystick, SFP(HStr_Err_InvalidParameter));
                if (uiData) {
                    _input = new HydroInputJoystick(ctrlInPins, uiControlSetup.ctrlCfgAs.joystick.repeatDelay, uiControlSetup.ctrlCfgAs.joystick.decreaseDivisor,
                                                    uiData->joystickCalib[0], uiData->joystickCalib[1], uiData->joystickCalib[2]);
                } else {
                    _input = new HydroInputJoystick(ctrlInPins, uiControlSetup.ctrlCfgAs.joystick.repeatDelay, uiControlSetup.ctrlCfgAs.joystick.decreaseDivisor);
                }
                break;

            case Hydro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk:
            case Hydro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR:
                HYDRO_SOFT_ASSERT(uiControlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(HStr_Err_InvalidParameter));
                _input = new HydroInputMatrix3x4(ctrlInPins, uiControlSetup.ctrlCfgAs.matrix.repeatDelay, uiControlSetup.ctrlCfgAs.matrix.repeatInterval,
                                                 uiControlSetup.ctrlCfgAs.matrix.encoderSpeed);
                break;

            case Hydro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk:
            case Hydro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR:
                HYDRO_SOFT_ASSERT(uiControlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(HStr_Err_InvalidParameter));
                _input = new HydroInputMatrix4x4(ctrlInPins, uiControlSetup.ctrlCfgAs.matrix.repeatDelay, uiControlSetup.ctrlCfgAs.matrix.repeatInterval,
                                                 uiControlSetup.ctrlCfgAs.matrix.encoderSpeed);
                break;

            case Hydro_ControlInputMode_TouchScreen:
                _input = new HydroInputTouchscreen(ctrlInPins, uiDisplaySetup.getDisplayOrientation());
                break;

            default: break;
        }
        HYDRO_SOFT_ASSERT(!(ctrlInMode >= Hydro_ControlInputMode_RotaryEncoderOk && ctrlInMode != Hydro_ControlInputMode_ResistiveTouch && ctrlInMode <= Hydro_ControlInputMode_TouchScreen) || _input, SFP(HStr_Err_AllocationFailure));

        // Display driver setup
        auto dispOutMode = controller->getDisplayOutputMode();
        auto lcdSetup = controller->getLCDSetup();

        // LiquidCrystalIO supports only i2c
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_LCD16x2 && dispOutMode <= Hydro_DisplayOutputMode_LCD20x4_Swapped) || lcdSetup.cfgType == DeviceSetup::I2CSetup, SFP(HStr_Err_InvalidParameter));
        // U8g2 supports either i2c or SPI
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_SSD1305 && dispOutMode <= Hydro_DisplayOutputMode_IL3820_V2) || (lcdSetup.cfgType == DeviceSetup::I2CSetup || lcdSetup.cfgType == DeviceSetup::SPISetup), SFP(HStr_Err_InvalidParameter));
        // AdafruitGFX supports only SPI
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_ST7735 && dispOutMode <= Hydro_DisplayOutputMode_PCD8544) || (lcdSetup.cfgType == DeviceSetup::SPISetup), SFP(HStr_Err_InvalidParameter));
        // TFTe_SPI supports only SPI
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_ST7735 && dispOutMode <= Hydro_DisplayOutputMode_PCD8544) || (lcdSetup.cfgType == DeviceSetup::SPISetup), SFP(HStr_Err_InvalidParameter));

        switch (dispOutMode) {
            // LiquidCrystalIO
            case Hydro_DisplayOutputMode_LCD16x2:
            case Hydro_DisplayOutputMode_LCD16x2_Swapped:
            case Hydro_DisplayOutputMode_LCD20x4:
            case Hydro_DisplayOutputMode_LCD20x4_Swapped:
                HYDRO_SOFT_ASSERT(uiDisplaySetup.dispCfgType == UIDisplaySetup::LCD, SFP(HStr_Err_InvalidParameter));
                if (!uiDisplaySetup.dispCfgAs.lcd.isDFRobotShield) {
                    _display = new HydroDisplayLiquidCrystalIO(dispOutMode, lcdSetup.cfgAs.i2c, uiDisplaySetup.dispCfgAs.lcd.bitInversion, uiDisplaySetup.dispCfgAs.lcd.backlitMode);
                } else {
                    _display = new HydroDisplayLiquidCrystalIO(true, lcdSetup.cfgAs.i2c, uiDisplaySetup.dispCfgAs.lcd.bitInversion, uiDisplaySetup.dispCfgAs.lcd.backlitMode);
                }
                break;

            // U8g2lib
            case Hydro_DisplayOutputMode_SSD1305:
            case Hydro_DisplayOutputMode_SSD1305_x32Ada:
            case Hydro_DisplayOutputMode_SSD1305_x64Ada:
            case Hydro_DisplayOutputMode_SSD1306:
            case Hydro_DisplayOutputMode_SH1106:
            case Hydro_DisplayOutputMode_SSD1607_GD:
            case Hydro_DisplayOutputMode_SSD1607_WS:
            case Hydro_DisplayOutputMode_IL3820:
            case Hydro_DisplayOutputMode_IL3820_V2:
                HYDRO_SOFT_ASSERT(uiDisplaySetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayU8g2lib(dispOutMode, lcdSetup, uiDisplaySetup.dispCfgAs.gfx.dispOrient, uiDisplaySetup.dispCfgAs.gfx.dcPin, uiDisplaySetup.dispCfgAs.gfx.resetPin);
                break;

            // AdafruitGFX
            case Hydro_DisplayOutputMode_ST7735:
                HYDRO_SOFT_ASSERT(uiDisplaySetup.dispCfgType == UIDisplaySetup::ST7735, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(uiDisplaySetup.dispCfgAs.st7735.tabColor != Hydro_ST7735Tab_Undefined, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_ST7735>(lcdSetup.cfgAs.spi, uiDisplaySetup.dispCfgAs.st7735.dispOrient, uiDisplaySetup.dispCfgAs.st7735.tabColor, uiDisplaySetup.dispCfgAs.st7735.dcPin, uiDisplaySetup.dispCfgAs.st7735.resetPin);
                break;
            case Hydro_DisplayOutputMode_ST7789:
                HYDRO_SOFT_ASSERT(uiDisplaySetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_ST7789>(lcdSetup.cfgAs.spi, uiDisplaySetup.dispCfgAs.gfx.dispOrient, uiDisplaySetup.dispCfgAs.gfx.dcPin, uiDisplaySetup.dispCfgAs.gfx.resetPin);
                break;
            case Hydro_DisplayOutputMode_ILI9341:
                HYDRO_SOFT_ASSERT(uiDisplaySetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_ILI9341>(lcdSetup.cfgAs.spi, uiDisplaySetup.dispCfgAs.gfx.dispOrient, uiDisplaySetup.dispCfgAs.gfx.dcPin, uiDisplaySetup.dispCfgAs.gfx.resetPin);
                break;
            case Hydro_DisplayOutputMode_PCD8544:
                HYDRO_SOFT_ASSERT(uiDisplaySetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_PCD8544>(lcdSetup.cfgAs.spi, uiDisplaySetup.dispCfgAs.gfx.dispOrient, uiDisplaySetup.dispCfgAs.gfx.dcPin, uiDisplaySetup.dispCfgAs.gfx.resetPin);
                break;

            // TFT_eSPI
            case Hydro_DisplayOutputMode_TFT:
                HYDRO_SOFT_ASSERT(uiDisplaySetup.dispCfgType == UIDisplaySetup::TFT, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(lcdSetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_NotConfiguredProperly));
                #ifdef TFT_CS
                    HYDRO_SOFT_ASSERT(lcdSetup.cfgAs.spi.cs == TFT_CS, SFP(HStr_Err_NotConfiguredProperly));
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
                _display = new HydroDisplayTFTeSPI(lcdSetup.cfgAs.spi, uiDisplaySetup.dispCfgAs.tft.dispOrient, uiDisplaySetup.dispCfgAs.tft.screenWidth, uiDisplaySetup.dispCfgAs.tft.screenHeight);
                break;

            default: break;
        }
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_LCD16x2 && dispOutMode <= Hydro_DisplayOutputMode_TFT) || _display, SFP(HStr_Err_AllocationFailure));

        if (_display) {
            // Late input driver setup
            switch (ctrlInMode) {
                case Hydro_ControlInputMode_ResistiveTouch:
                    _input = new HydroInputResistiveTouch(ctrlInPins, _display);
                    break;

                // TFT_eSPI
                case Hydro_ControlInputMode_TFTTouch:
                    HYDRO_SOFT_ASSERT(dispOutMode == Hydro_DisplayOutputMode_TFT, SFP(HStr_Err_InvalidParameter));
                    #ifdef TOUCH_CS
                        HYDRO_SOFT_ASSERT(ctrlInPins.second[0] == TOUCH_CS, SFP(HStr_Err_NotConfiguredProperly));
                    #else
                        HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                    #endif
                    _input = new HydroInputTFTTouch(ctrlInPins, (HydroDisplayTFTeSPI *)_display, HYDRO_UI_TFTTOUCH_USES_RAW);
                    break;

                default: break;
            }
            HYDRO_SOFT_ASSERT(!(ctrlInMode == Hydro_ControlInputMode_ResistiveTouch || ctrlInMode == Hydro_ControlInputMode_TFTTouch) || _input, SFP(HStr_Err_AllocationFailure));
        }
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
}

void HydruinoBaseUI::init(uint8_t updatesPerSec, Hydro_DisplayTheme displayTheme, bool analogSlider)
{
    SwitchInterruptMode isrMode(SWITCHES_POLL_EVERYTHING);
    if (_allowISR) {
        bool allInputInterruptable = false; // todo
        bool nonKeysInterruptable = false; // todo
        isrMode = (allInputInterruptable ? SWITCHES_NO_POLLING : (nonKeysInterruptable ? SWITCHES_POLL_KEYS_ONLY : SWITCHES_POLL_EVERYTHING));
    }

    switches.init(_input->getIoAbstraction() ?: internalDigitalIo(), isrMode, _isActiveLow);

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

void HydruinoBaseUI::addRemote(Hydro_RemoteControl rcType, UARTDeviceSetup rcSetup, uint16_t rcServerPort)
{
    HydroRemoteControl *remoteControl = nullptr;
    menuid_t statusMenuId = -1; // todo

    switch (rcType) {
        case Hydro_RemoteControl_Serial:
            remoteControl = new HydroRemoteSerialControl(rcSetup);
            HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));
            break;

        case Hydro_RemoteControl_Simhub:
            remoteControl = new HydroRemoteSimhubControl(rcSetup, statusMenuId);
            HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));
            break;

        case Hydro_RemoteControl_WiFi:
            #ifdef HYDRO_USE_WIFI
                remoteControl = new HydroRemoteWiFiControl(rcServerPort);
                HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));
            #endif
            break;

        case Hydro_RemoteControl_Ethernet:
            #ifdef HYDRO_USE_ETHERNET
                remoteControl = new HydroRemoteEthernetControl(rcServerPort);
                HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));
            #endif
            break;

        default: break;
    }

    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(applicationInfo); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else {
        if (remoteControl) { delete remoteControl; }
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
