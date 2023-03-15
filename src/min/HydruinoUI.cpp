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
                #ifdef HYDRO_ENABLE_XPT2046TS
                    HYDRO_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && isValidPin(ctrlInPins.second[0]), SFP(HStr_Err_InvalidPinOrType));
                #endif
                _input = new HydroInputTouchscreen(ctrlInPins, _uiDispSetup.getDisplayRotation());
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
                HYDRO_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && isValidPin(ctrlInPins.second[0]), SFP(HStr_Err_InvalidPinOrType));
                #ifdef TOUCH_CS
                    HYDRO_SOFT_ASSERT(ctrlInPins.first && ctrlInPins.second && ctrlInPins.second[0] == TOUCH_CS, SFP(HStr_Err_NotConfiguredProperly));
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
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Hydro_DisplayOutputMode_LCD20x4_RS) || displaySetup.cfgType == DeviceSetup::I2CSetup, SFP(HStr_Err_InvalidParameter));
        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_LCD16x2_EN:
            case Hydro_DisplayOutputMode_LCD16x2_RS:
            case Hydro_DisplayOutputMode_LCD20x4_EN:
            case Hydro_DisplayOutputMode_LCD20x4_RS:
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::LCD, SFP(HStr_Err_InvalidParameter));
                if (!_uiDispSetup.dispCfgAs.lcd.isDFRobotShield) {
                    _display = new HydroDisplayLiquidCrystal(dispOutMode, displaySetup.cfgAs.i2c, _uiDispSetup.dispCfgAs.lcd.ledMode);
                } else {
                    _display = new HydroDisplayLiquidCrystal(true, displaySetup.cfgAs.i2c, _uiDispSetup.dispCfgAs.lcd.ledMode);
                }
                break;
            default: break;
        }
        HYDRO_SOFT_ASSERT(!(dispOutMode >= Hydro_DisplayOutputMode_LCD16x2_EN && dispOutMode <= Hydro_DisplayOutputMode_LCD20x4_RS) || _display, SFP(HStr_Err_AllocationFailure));
    }
}

void HydruinoMinUI::allocateSSD1305Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_SSD1305: {
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305Wire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305Wire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI1) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateSSD1305x32AdaDisplay()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_SSD1305_x32Ada: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305x32AdaWire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305x32AdaWire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305x32AdaSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI1) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305x32AdaSPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateSSD1305x64AdaDisplay()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_SSD1305_x64Ada: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305x64AdaWire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305x64AdaWire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305x64AdaSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI1) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1305x64AdaSPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateSSD1306Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_SSD1306: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1306Wire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1306Wire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1306SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI1) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1306SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateSH1106Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_SH1106: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE) {
                    _display = HydroDisplayU8g2OLED::allocateSH1106Wire(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::I2CSetup && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1) {
                    _display = HydroDisplayU8g2OLED::allocateSH1106Wire1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2OLED::allocateSH1106SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup && displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI1) {
                    _display = HydroDisplayU8g2OLED::allocateSH1106SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateCustomOLEDDisplay()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_CustomOLED: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
                    _display = HydroDisplayU8g2OLED::allocateCustomOLEDI2C(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                    _display = HydroDisplayU8g2OLED::allocateCustomOLEDSPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateSSD1607Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_SSD1607: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                if (displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1607SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI1) {
                    _display = HydroDisplayU8g2OLED::allocateSSD1607SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateIL3820Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_IL3820: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                if (displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2OLED::allocateIL3820SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI1) {
                    _display = HydroDisplayU8g2OLED::allocateIL3820SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateIL3820V2Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_IL3820_V2: {
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                if (displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI) {
                    _display = HydroDisplayU8g2OLED::allocateIL3820V2SPI(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else if (displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI1) {
                    _display = HydroDisplayU8g2OLED::allocateIL3820V2SPI1(displaySetup, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                } else {
                    HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_InvalidParameter));
                }
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateST7735Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_ST7735: {
                HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgAs.gfx.tabColor != Hydro_ST7735Tab_Undefined, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_ST7735>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.tabColor, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateST7789Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        // Display driver setup
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_ST7789: {
                HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_ST7789>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateILI9341Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_ILI9341: {
                HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_ILI9341>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocatePCD8544Display()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_PCD8544: {
                HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::Pixel, SFP(HStr_Err_InvalidParameter));
                _display = new HydroDisplayAdafruitGFX<Adafruit_PCD8544>(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.gfx.rotation, _uiDispSetup.dispCfgAs.gfx.dcPin, _uiDispSetup.dispCfgAs.gfx.resetPin);
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
    }
}

void HydruinoMinUI::allocateTFTDisplay()
{
    auto controller = getController();
    HYDRO_HARD_ASSERT(controller, SFP(HStr_Err_InitializationFailure));
    HYDRO_SOFT_ASSERT(!_display, SFP(HStr_Err_AlreadyInitialized));

    if (controller && !_display) {
        auto dispOutMode = controller->getDisplayOutputMode();
        auto displaySetup = controller->getDisplaySetup();

        switch (dispOutMode) {
            case Hydro_DisplayOutputMode_TFT: {
                HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(_uiDispSetup.dispCfgType == UIDisplaySetup::TFT, SFP(HStr_Err_InvalidParameter));
                HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
                #ifdef TFT_CS
                    HYDRO_SOFT_ASSERT(displaySetup.cfgAs.spi.cs == TFT_CS, SFP(HStr_Err_NotConfiguredProperly));
                #else
                    HYDRO_HARD_ASSERT(false, SFP(HStr_Err_NotConfiguredProperly));
                #endif
                _display = new HydroDisplayTFTeSPI(displaySetup.cfgAs.spi, _uiDispSetup.dispCfgAs.tft.rotation, _uiDispSetup.dispCfgAs.tft.screenWidth, _uiDispSetup.dispCfgAs.tft.screenHeight, _uiDispSetup.dispCfgAs.tft.tabColor);
                HYDRO_SOFT_ASSERT(_display, SFP(HStr_Err_AllocationFailure));
            } break;
            default: break;
        }
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
