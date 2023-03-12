/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Full/RW UI
*/

#include "Hydruino.h"
#include "HydruinoUI.h"

HydruinoFullUI::HydruinoFullUI(UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts)
    : HydruinoBaseUI(uiControlSetup, uiDisplaySetup, isActiveLowIO, allowInterruptableIO, enableTcUnicodeFonts)
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

            case Hydro_ControlInputMode_UpDownESP32TouchOk:
            case Hydro_ControlInputMode_UpDownESP32TouchOkLR: {
                HYDRO_SOFT_ASSERT(_uiCtrlSetup.ctrlCfgType == UIControlSetup::ESP32Touch, SFP(HStr_Err_InvalidParameter));
                _input = new HydroInputESP32TouchKeys(ctrlInPins, _uiCtrlSetup.ctrlCfgAs.touch.repeatSpeed, _uiCtrlSetup.ctrlCfgAs.touch.switchThreshold,
                                                      _uiCtrlSetup.ctrlCfgAs.touch.highVoltage, _uiCtrlSetup.ctrlCfgAs.touch.lowVoltage, _uiCtrlSetup.ctrlCfgAs.touch.attenuation);
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

            case Hydro_ControlInputMode_TouchScreen: {
                #ifdef HELIO_ENABLE_XPT2046TS
                    HYDRO_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && isValidPin(ctrlInPins.second[0]), SFP(HStr_Err_InvalidPinOrType));
                #endif
                _input = new HydroInputTouchscreen(ctrlInPins, _uiDispSetup.getDisplayOrientation());
            } break;

            default: break;
        }
        HYDRO_SOFT_ASSERT(!(ctrlInMode >= Hydro_ControlInputMode_RotaryEncoderOk && ctrlInMode != Hydro_ControlInputMode_ResistiveTouch && ctrlInMode <= Hydro_ControlInputMode_TouchScreen) || _input, SFP(HStr_Err_AllocationFailure));

        // Display driver setup
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        // LiquidCrystalIO supports only i2c, /w LCD setup
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Hydro_DisplayOutputMode_LCD20x4_RS) || displaySetup.cfgType == DeviceSetup::I2CSetup, SFP(HStr_Err_InvalidParameter));
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Hydro_DisplayOutputMode_LCD20x4_RS) || _uiDispSetup.dispCfgType == UIDisplaySetup::LCD, SFP(HStr_Err_InvalidParameter));
        // U8g2 supports either i2c or SPI, /w Pixel setup
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_SSD1305 && dispOutMode <= Hydro_DisplayOutputMode_SH1106) || (displaySetup.cfgType == DeviceSetup::I2CSetup || displaySetup.cfgType == DeviceSetup::SPISetup), SFP(HStr_Err_InvalidParameter));
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_SSD1607_GD && dispOutMode <= Hydro_DisplayOutputMode_IL3820_V2) || displaySetup.cfgType == DeviceSetup::SPISetup, SFP(HStr_Err_InvalidParameter));
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_SSD1305 && dispOutMode <= Hydro_DisplayOutputMode_IL3820_V2) || _uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
        // AdafruitGFX supports only SPI, /w ST7735/Pixel setup
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_ST7735 && dispOutMode <= Hydro_DisplayOutputMode_PCD8544) || displaySetup.cfgType == DeviceSetup::SPISetup, SFP(HStr_Err_InvalidParameter));
        HYDRO_SOFT_ASSERT(dispOutMode != Hydro_DisplayOutputMode_ST7735 || _uiDispSetup.dispCfgType == UIDisplaySetup::ST7735, SFP(HStr_Err_InvalidParameter));
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_ST7789 && dispOutMode <= Hydro_DisplayOutputMode_PCD8544) || _uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
        // TFTe_SPI supports only SPI, /w TFT setup
        HYDRO_SOFT_ASSERT(dispOutMode != Hydro_DisplayOutputMode_TFT || displaySetup.cfgType == DeviceSetup::SPISetup, SFP(HStr_Err_InvalidParameter));
        HYDRO_SOFT_ASSERT(dispOutMode != Hydro_DisplayOutputMode_TFT || _uiDispSetup.dispCfgType == UIDisplaySetup::TFT, SFP(HStr_Err_InvalidParameter));

        switch (dispOutMode) {
            // LiquidCrystalIO
            case Hydro_DisplayOutputMode_LCD16x2_EN:
            case Hydro_DisplayOutputMode_LCD16x2_RS:
            case Hydro_DisplayOutputMode_LCD20x4_EN:
            case Hydro_DisplayOutputMode_LCD20x4_RS: {
                if (!_uiDispSetup.dispCfgAs.lcd.isDFRobotShield) {
                    _display = new HydroDisplayLiquidCrystalIO(dispOutMode, displaySetup.cfgAs.i2c, _uiDispSetup.dispCfgAs.lcd.backlightMode);
                } else {
                    _display = new HydroDisplayLiquidCrystalIO(true, displaySetup.cfgAs.i2c, _uiDispSetup.dispCfgAs.lcd.backlightMode);
                }
            } break;

            // U8g2lib
            case Hydro_DisplayOutputMode_SSD1305: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE) {
                    _display = HydroDisplayU8g2lib::allocateSSD1305I2C(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1) {
                    _display = HydroDisplayU8g2lib::allocateSSD1305I2C2(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2lib::allocateSSD1305SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
            } break;
            case Hydro_DisplayOutputMode_SSD1305_x32Ada: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE) {
                    _display = HydroDisplayU8g2lib::allocateSSD1305x32AdaI2C(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1) {
                    _display = HydroDisplayU8g2lib::allocateSSD1305x32AdaI2C2(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2lib::allocateSSD1305x32AdaSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
            } break;
            case Hydro_DisplayOutputMode_SSD1305_x64Ada: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE) {
                    _display = HydroDisplayU8g2lib::allocateSSD1305x64AdaI2C(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1) {
                    _display = HydroDisplayU8g2lib::allocateSSD1305x64AdaI2C2(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2lib::allocateSSD1305x64AdaSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
            } break;
            case Hydro_DisplayOutputMode_SSD1306: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE) {
                    _display = HydroDisplayU8g2lib::allocateSSD1306I2C(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1) {
                    _display = HydroDisplayU8g2lib::allocateSSD1306I2C2(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2lib::allocateSSD1306SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
            } break;
            case Hydro_DisplayOutputMode_SH1106: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE) {
                    _display = HydroDisplayU8g2lib::allocateSH1106I2C(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1) {
                    _display = HydroDisplayU8g2lib::allocateSH1106I2C2(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2lib::allocateSH1106SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
            } break;
            case Hydro_DisplayOutputMode_SSD1607_GD: {
                _display = HydroDisplayU8g2lib::allocateSSD1607GDSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Hydro_DisplayOutputMode_SSD1607_WS: {
                _display = HydroDisplayU8g2lib::allocateSSD1607WSSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Hydro_DisplayOutputMode_IL3820: {
                _display = HydroDisplayU8g2lib::allocateIL3820SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Hydro_DisplayOutputMode_IL3820_V2: {
                _display = HydroDisplayU8g2lib::allocateIL3820V2SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;

            // AdafruitGFX
            case Hydro_DisplayOutputMode_ST7735: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgAs.st7735.tabColor != Hydro_ST7735Tab_Undefined, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_ST7735>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.st7735.dispOrient, _uiDispSetup.dispCfgAs.st7735.tabColor, _uiDispSetup.dispCfgAs.st7735.dcPin, _uiDispSetup.dispCfgAs.st7735.resetPin);
            } break;
            case Hydro_DisplayOutputMode_ST7789: {
                _display = new HydroDisplayAdafruitGFX<Adafruit_ST7789>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Hydro_DisplayOutputMode_ILI9341: {
                _display = new HydroDisplayAdafruitGFX<Adafruit_ILI9341>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;
            case Hydro_DisplayOutputMode_PCD8544: {
                _display = new HydroDisplayAdafruitGFX<Adafruit_PCD8544>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.dispOrient, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
            } break;

            // TFT_eSPI
            case Hydro_DisplayOutputMode_TFT: {
                HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
                #ifdef TFT_CS
                    HYDRO_SOFT_ASSERT(displaySetup.cfgAs.spi.cs == TFT_CS, SFP(HStr_Err_NotConfiguredProperly));
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
                _display = new HydroDisplayTFTeSPI(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.tft.dispOrient, _uiDispSetup.dispCfgAs.tft.screenWidth, _uiDispSetup.dispCfgAs.tft.screenHeight);
            } break;

            default: break;
        }
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Hydro_DisplayOutputMode_TFT) || _display, SFP(HStr_Err_AllocationFailure));

        // Late input driver setup
        switch (ctrlInMode) {
            case Hydro_ControlInputMode_ResistiveTouch: {
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_NotYetInitialized));
                _input = new HydroInputResistiveTouch(ctrlInPins, _display);
            } break;

            // TFT_eSPI
            case Hydro_ControlInputMode_TFTTouch: {
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_NotYetInitialized));
                HYDRO_SOFT_ASSERT(dispOutMode == Hydro_DisplayOutputMode_TFT, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && isValidPin(ctrlInPins.second[0]), SFP(HStr_Err_InvalidPinOrType));
                #ifdef TOUCH_CS
                    HYDRO_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && ctrlInPins.second[0] == TOUCH_CS, SFP(HStr_Err_NotConfiguredProperly));
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
                _input = new HydroInputTFTTouch(ctrlInPins, (HydroDisplayTFTeSPI *)_display, HYDRO_UI_TFTTOUCH_USES_RAW);
            } break;

            default: break;
        }
        HYDRO_SOFT_ASSERT(!(ctrlInMode == Hydro_ControlInputMode_ResistiveTouch || ctrlInMode == Hydro_ControlInputMode_TFTTouch) || _input, SFP(HStr_Err_AllocationFailure));
    }
}

HydruinoFullUI::~HydruinoFullUI()
{ ; }

void HydruinoFullUI::addRemote(Hydro_RemoteControl rcType, UARTDeviceSetup rcSetup, uint16_t rcServerPort)
{
    HydroRemoteControl *remoteControl = nullptr;
    menuid_t statusMenuId = -1; // todo

    switch (rcType) {
        case Hydro_RemoteControl_Serial: {
            remoteControl = new HydroRemoteSerialControl(rcSetup);
            HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));
        } break;

        case Hydro_RemoteControl_Simhub: {
            remoteControl = new HydroRemoteSimhubControl(rcSetup, statusMenuId);
            HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));
        } break;

        case Hydro_RemoteControl_WiFi: {
            #ifdef HYDRO_USE_WIFI
                remoteControl = new HydroRemoteWiFiControl(rcServerPort);
                HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));
            #endif
        } break;

        case Hydro_RemoteControl_Ethernet: {
            #ifdef HYDRO_USE_ETHERNET
                remoteControl = new HydroRemoteEthernetControl(rcServerPort);
                HYDRO_SOFT_ASSERT(remoteControl, SFP(HStr_Err_AllocationFailure));
            #endif
        } break;

        default: break;
    }

    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else {
        if (remoteControl) { delete remoteControl; }
    }
}

bool HydruinoFullUI::isFullUI()
{
    return true;
}
