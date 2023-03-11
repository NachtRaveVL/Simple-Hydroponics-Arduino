/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Minimal/RO UI
*/

#include "Hydruino.h"
#include "HydruinoUI.h"

HydruinoMinUI::HydruinoMinUI(UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts)
    : HydruinoBaseUI(uiControlSetup, uiDisplaySetup, isActiveLowIO, allowInterruptableIO, enableTcUnicodeFonts)
{ ; }

HydruinoMinUI::~HydruinoMinUI()
{ ; }

void HydruinoMinUI::allocateStandardControls()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_input, SFP(HStr_Err_AlreadyInitialized));
    const HydroUIData *uiData = nullptr; // todo

    if (controller && !_input) {
        auto ctrlInMode = controller->getControlInputMode();
        auto ctrlInPins = controller->getControlInputPins();
        switch (ctrlInMode) {
            case Hydro_ControlInputMode_RotaryEncoderOk:
            case Hydro_ControlInputMode_RotaryEncoderOkLR: {
                HYDRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Encoder, SFP(HStr_Err_InvalidParameter));
                _input = new HydroInputRotary(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.encoder.encoderSpeed);
            } break;

            case Hydro_ControlInputMode_UpDownButtonsOk:
            case Hydro_ControlInputMode_UpDownButtonsOkLR: {
                HYDRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Buttons, SFP(HStr_Err_InvalidParameter));
                if (!_uiCtrlSetup.ctrlCfgAs.buttons.isDFRobotShield) {
                    _input = new HydroInputUpDownButtons(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.buttons.repeatSpeed);
                } else {
                    _input = new HydroInputUpDownButtons(true, _uiCtrlSetup.ctrlCfgAs.buttons.repeatSpeed);
                }
            } break;

            case Hydro_ControlInputMode_AnalogJoystickOk: {
                HYDRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Joystick, SFP(HStr_Err_InvalidParameter));
                if (uiData) {
                    _input = new HydroInputJoystick(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.joystick.repeatDelay, _uiCtrlSetup.ctrlCfgAs.joystick.decreaseDivisor,
                                                    uiData->joystickCalib[0], uiData->joystickCalib[1], uiData->joystickCalib[2]);
                } else {
                    _input = new HydroInputJoystick(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.joystick.repeatDelay, _uiCtrlSetup.ctrlCfgAs.joystick.decreaseDivisor);
                }
            } break;

            case Hydro_ControlInputMode_Matrix2x2UpDownButtonsOkL: {
                HYDRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(HStr_Err_InvalidParameter));
                _input = new HydroInputMatrix2x2(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.matrix.repeatDelay, _uiCtrlSetup.ctrlCfgAs.matrix.repeatInterval);
            } break;

            case Hydro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk:
            case Hydro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR: {
                HYDRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(HStr_Err_InvalidParameter));
                _input = new HydroInputMatrix3x4(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.matrix.repeatDelay, _uiCtrlSetup.ctrlCfgAs.matrix.repeatInterval,
                                                 _uiCtrlSetup.ctrlCfgAs.matrix.encoderSpeed);
            } break;

            case Hydro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk:
            case Hydro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR: {
                HYDRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::Matrix, SFP(HStr_Err_InvalidParameter));
                _input = new HydroInputMatrix4x4(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.matrix.repeatDelay, _uiCtrlSetup.ctrlCfgAs.matrix.repeatInterval,
                                                 _uiCtrlSetup.ctrlCfgAs.matrix.encoderSpeed);
            } break;

            default: break;
        }
        HYDRO_SOFT_ASSERT(!(ctrlInMode >= Hydro_ControlInputMode_RotaryEncoderOk && ctrlInMode <= Hydro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR) || _input, SFP(HStr_Err_AllocationFailure));
    }
}

void HydruinoMinUI::allocateESP32TouchControl()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_input, SFP(HStr_Err_AlreadyInitialized));
    #ifndef ESP32
        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
    #endif

    if (controller && !_input) {
        auto ctrlInMode = controller->getControlInputMode();
        auto ctrlInPins = controller->getControlInputPins();
        switch (ctrlInMode) {
            case Hydro_ControlInputMode_UpDownESP32TouchOk:
            case Hydro_ControlInputMode_UpDownESP32TouchOkLR:
                HYDRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::ESP32Touch, SFP(HStr_Err_InvalidParameter));
                _input = new HydroInputESP32TouchKeys(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.touch.repeatSpeed, _uiCtrlSetup.ctrlCfgAs.touch.switchThreshold,
                                                      _uiCtrlSetup.ctrlCfgAs.touch.highVoltage, _uiCtrlSetup.ctrlCfgAs.touch.lowVoltage, _uiCtrlSetup.ctrlCfgAs.touch.attenuation);
                break;
            default: break;
        }
        HYDRO_SOFT_ASSERT(!(ctrlInMode >= Hydro_ControlInputMode_UpDownESP32TouchOk && ctrlInMode <= Hydro_ControlInputMode_UpDownESP32TouchOkLR) || _input, SFP(HStr_Err_AllocationFailure));
    }
}

void HydruinoMinUI::allocateResistiveTouchControl()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_input, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_input) {
        auto ctrlInMode = controller->getControlInputMode();
        auto ctrlInPins = controller->getControlInputPins();
        switch (ctrlInMode) {
            case Hydro_ControlInputMode_ResistiveTouch:
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_NotYetInitialized));
                _input = new HydroInputResistiveTouch(ctrlInPins, _display);
                break;
            default: break;
        }
        HYDRO_SOFT_ASSERT(ctrlInMode != Hydro_ControlInputMode_ResistiveTouch || _input, SFP(HStr_Err_AllocationFailure));
    }
}

void HydruinoMinUI::allocateTouchscreenControl()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_input, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_input) {
        auto ctrlInMode = controller->getControlInputMode();
        auto ctrlInPins = controller->getControlInputPins();
        switch (ctrlInMode) {
            case Hydro_ControlInputMode_TouchScreen:
                _input = new HydroInputTouchscreen(ctrlInPins, _uiDispSetup.getDisplayOrientation());
                break;
            default: break;
        }
        HYDRO_SOFT_ASSERT(ctrlInMode != Hydro_ControlInputMode_TouchScreen || _input, SFP(HStr_Err_AllocationFailure));
    }
}

void HydruinoMinUI::allocateTFTTouchControl()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_input, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_input) {
        auto ctrlInMode = controller->getControlInputMode();
        auto ctrlInPins = controller->getControlInputPins();
        switch (ctrlInMode) {
            case Hydro_ControlInputMode_TFTTouch:
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_NotYetInitialized));
                HYDRO_SOFT_ASSERT(controller->getDisplayOutputMode() == Hydro_DisplayOutputMode_TFT, SFP(HStr_Err_InvalidParameter));
                #ifdef TOUCH_CS
                    HYDRO_SOFT_ASSERT(ctrlInPins.second[0] == TOUCH_CS, SFP(HStr_Err_NotConfiguredProperly));
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
                _input = new HydroInputTFTTouch(ctrlInPins, (HydroDisplayTFTeSPI *)_display, HYDRO_UI_TFTTOUCH_USES_RAW);
                break;
            default: break;
        }
        HYDRO_SOFT_ASSERT(ctrlInMode != Hydro_ControlInputMode_TFTTouch || _input, SFP(HStr_Err_AllocationFailure));
    }
}

void HydruinoMinUI::allocateLCDDisplay()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        // Display driver setup
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        // LiquidCrystalIO supports only i2c
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Hydro_DisplayOutputMode_LCD20x4_RS) || displaySetup.cfgType == DeviceSetup::I2CSetup, SFP(HStr_Err_InvalidParameter));

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_LCD16x2_EN:
            case Hydro_DisplayOutputMode_LCD16x2_RS:
            case Hydro_DisplayOutputMode_LCD20x4_EN:
            case Hydro_DisplayOutputMode_LCD20x4_RS:
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::LCD, SFP(HStr_Err_InvalidParameter));
                if (!_uiDispSetup.dispCfgAs.lcd.isDFRobotShield) {
                    _display = new HydroDisplayLiquidCrystalIO(dispOutMode, displaySetup.cfgAs.i2c, _uiDispSetup.dispCfgAs.lcd.backlightMode);
                } else {
                    _display = new HydroDisplayLiquidCrystalIO(true, displaySetup.cfgAs.i2c, _uiDispSetup.dispCfgAs.lcd.backlightMode);
                }
                break;
            default: break;
        }
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Hydro_DisplayOutputMode_LCD20x4_RS) || _display, SFP(HStr_Err_AllocationFailure));
    }
}

void HydruinoMinUI::allocateU8G2Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        // Display driver setup
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        // U8g2 supports either i2c or SPI
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_SSD1305 && dispOutMode <= Hydro_DisplayOutputMode_IL3820_V2) || (displaySetup.cfgType == DeviceSetup::I2CSetup || displaySetup.cfgType == DeviceSetup::SPISetup), SFP(HStr_Err_InvalidParameter));

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_SSD1305:
            case Hydro_DisplayOutputMode_SSD1305_x32Ada:
            case Hydro_DisplayOutputMode_SSD1305_x64Ada:
            case Hydro_DisplayOutputMode_SSD1306:
            case Hydro_DisplayOutputMode_SH1106:
            case Hydro_DisplayOutputMode_SSD1607_GD:
            case Hydro_DisplayOutputMode_SSD1607_WS:
            case Hydro_DisplayOutputMode_IL3820:
            case Hydro_DisplayOutputMode_IL3820_V2:
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayU8g2lib(dispOutMode, displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                break;
            default: break;
        }
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_SSD1305 && dispOutMode <= Hydro_DisplayOutputMode_IL3820_V2) || _display, SFP(HStr_Err_AllocationFailure));
    }
}

void HydruinoMinUI::allocateAdaGFXDisplay()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        // Display driver setup
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        // AdafruitGFX supports only SPI
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_ST7735 && dispOutMode <= Hydro_DisplayOutputMode_PCD8544) || (displaySetup.cfgType == DeviceSetup::SPISetup), SFP(HStr_Err_InvalidParameter));

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_ST7735: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::ST7735, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgAs.st7735.tabColor != Hydro_ST7735Tab_Undefined, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_ST7735>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.st7735.dispOrient, _uiDispSetup.dispCfgAs.st7735.tabColor, _uiDispSetup.dispCfgAs.st7735.dcPin, _uiDispSetup.dispCfgAs.st7735.resetPin);
            } break;
            case Hydro_DisplayOutputMode_ST7789: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_ST7789>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Hydro_DisplayOutputMode_ILI9341: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_ILI9341>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Hydro_DisplayOutputMode_PCD8544: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_PCD8544>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            default: break;
        }
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_ST7735 && dispOutMode <= Hydro_DisplayOutputMode_PCD8544) || _display, SFP(HStr_Err_AllocationFailure));
    }
}

void HydruinoMinUI::allocateTFTDisplay()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        // Display driver setup
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        // TFTe_SPI supports only SPI
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_ST7735 && dispOutMode <= Hydro_DisplayOutputMode_PCD8544) || (displaySetup.cfgType == DeviceSetup::SPISetup), SFP(HStr_Err_InvalidParameter));

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_TFT: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::TFT, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_NotConfiguredProperly));
                #ifdef TFT_CS
                    HYDRO_SOFT_ASSERT(displaySetup.cfgAs.spi.cs == TFT_CS, SFP(HStr_Err_NotConfiguredProperly));
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
                _display = new HydroDisplayTFTeSPI(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.tft.dispOrient, _uiDispSetup.dispCfgAs.tft.screenWidth, _uiDispSetup.dispCfgAs.tft.screenHeight);
            } break;
            default: break;
        }
        HYDRO_SOFT_ASSERT(dispOutMode != Hydro_DisplayOutputMode_TFT || _display, SFP(HStr_Err_AllocationFailure));
    }
}

void HydruinoMinUI::addSerialRemote(UARTDeviceSetup rcSetup)
{
    HydroRemoteControl *remoteControl = new HydroRemoteSerialControl(rcSetup);
    HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));

    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else {
        if (remoteControl) { delete remoteControl; }
    }
}

void HydruinoMinUI::addSimhubRemote(UARTDeviceSetup rcSetup)
{
    menuid_t statusMenuId = -1; // todo
    HydroRemoteControl *remoteControl = new HydroRemoteSimhubControl(rcSetup, statusMenuId);
    HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));

    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else {
        if (remoteControl) { delete remoteControl; }
    }
}

void HydruinoMinUI::addWiFiRemote(uint16_t rcServerPort)
{
    HydroRemoteControl *remoteControl = 
    #ifdef HYDRO_USE_WIFI
        new HydroRemoteWiFiControl(rcServerPort);
        HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));
    #else
        nullptr;
        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
    #endif

    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else {
        if (remoteControl) { delete remoteControl; }
    }
}

void HydruinoMinUI::addEthernetRemote(uint16_t rcServerPort)
{
    HydroRemoteControl *remoteControl = 
    #ifdef HYDRO_USE_ETHERNET
        new HydroRemoteEthernetControl(rcServerPort);
        HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));
    #else
        nullptr;
        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
    #endif

    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else {
        if (remoteControl) { delete remoteControl; }
    }
}

bool HydruinoMinUI::isFullUI()
{
    return false;
}
