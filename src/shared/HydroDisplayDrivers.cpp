/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Display Drivers
*/

#include "Hydruino.h"
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"
#include "BaseRenderers.h"
#include "graphics/BaseGraphicalRenderer.h"
#include "IoAbstractionWire.h"
#include "DfRobotInputAbstraction.h"

HydroDisplayDriver::HydroDisplayDriver(Hydro_DisplayOrientation displayOrientation)
    : _displayOri(displayOrientation), _displayTheme(Hydro_DisplayTheme_Undefined)
{ ; }

void HydroDisplayDriver::commonInit(uint8_t updatesPerSec, Hydro_DisplayTheme displayTheme, bool analogSlider, bool utf8Fonts)
{
    getBaseRenderer()->setUpdatesPerSecond(updatesPerSec);
    if (getGraphicsRenderer()) {
        getGraphicsRenderer()->setUseSliderForAnalog(analogSlider);
        if (utf8Fonts) { getGraphicsRenderer()->enableTcUnicode(); }

        if (_displayTheme != displayTheme) {
            switch ((_displayTheme = displayTheme)) {
                case Hydro_DisplayTheme_CoolBlue_ML:
                    installCoolBlueModernTheme(*getGraphicsRenderer(), MenuFontDef(nullptr, 1), MenuFontDef(nullptr, 1), true);
                    break;
                case Hydro_DisplayTheme_CoolBlue_SM:
                    installCoolBlueTraditionalTheme(*getGraphicsRenderer(), MenuFontDef(nullptr, 1), MenuFontDef(nullptr, 1), true);
                    break;
                case Hydro_DisplayTheme_DarkMode_ML:
                    installDarkModeModernTheme(*getGraphicsRenderer(), MenuFontDef(nullptr, 1), MenuFontDef(nullptr, 1), true);
                    break;
                case Hydro_DisplayTheme_DarkMode_SM:
                    installDarkModeTraditionalTheme(*getGraphicsRenderer(), MenuFontDef(nullptr, 1), MenuFontDef(nullptr, 1), true);
                    break;
                case Hydro_DisplayTheme_MonoOLED:
                    installMonoBorderedTheme(*getGraphicsRenderer(), MenuFontDef(nullptr, 1), MenuFontDef(nullptr, 1), true);
                    break;
                case Hydro_DisplayTheme_MonoOLED_Inv:
                    installMonoInverseTitleTheme(*getGraphicsRenderer(), MenuFontDef(nullptr, 1), MenuFontDef(nullptr, 1), true);
                    break;
            }
        }
    }
}


HydroDisplayLiquidCrystalIO::HydroDisplayLiquidCrystalIO(Hydro_DisplayOutputMode displayMode, I2CDeviceSetup displaySetup, Hydro_BacklightMode backlightMode)
    : _screenSize{displayMode < Hydro_DisplayOutputMode_LCD20x4_EN ? 16 : 20, displayMode < Hydro_DisplayOutputMode_LCD20x4_EN ? 2 : 4},
      _lcd(displayMode == Hydro_DisplayOutputMode_LCD16x2_EN || displayMode == Hydro_DisplayOutputMode_LCD20x4_EN ? 2 : 0, 1,
           displayMode == Hydro_DisplayOutputMode_LCD16x2_EN || displayMode == Hydro_DisplayOutputMode_LCD20x4_EN ? 0 : 2, 4, 5, 6, 7,
           backlightMode == Hydro_BacklightMode_Normal ? LiquidCrystal::BACKLIGHT_NORMAL : backlightMode == Hydro_BacklightMode_Inverted ? LiquidCrystal::BACKLIGHT_INVERTED : LiquidCrystal::BACKLIGHT_PWM,
           ioFrom8574(HYDRO_UI_I2CLCD_BASEADDR | displaySetup.address, 0xff, displaySetup.wire, false)),
      _renderer(_lcd, _screenSize[0], _screenSize[1], getController()->getSystemNameChars())
{
    _lcd.configureBacklightPin(3);
    _renderer.setTitleRequired(_screenSize[1] >= 4);
}

HydroDisplayLiquidCrystalIO::HydroDisplayLiquidCrystalIO(bool isDFRobotShield_unused, I2CDeviceSetup displaySetup, Hydro_BacklightMode backlightMode)
    : _screenSize{16, 2},
      _lcd(8, 9, 4, 5, 6, 7,
           backlightMode == Hydro_BacklightMode_Normal ? LiquidCrystal::BACKLIGHT_NORMAL : backlightMode == Hydro_BacklightMode_Inverted ? LiquidCrystal::BACKLIGHT_INVERTED : LiquidCrystal::BACKLIGHT_PWM,
           ioFrom8574(HYDRO_UI_I2CLCD_BASEADDR | displaySetup.address, 0xff, displaySetup.wire, false)),
      _renderer(_lcd, _screenSize[0], _screenSize[1], getController()->getSystemNameChars())
{
    _lcd.configureBacklightPin(10);
    _renderer.setTitleRequired(false);
}

void HydroDisplayLiquidCrystalIO::init()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED);
}

void HydroDisplayLiquidCrystalIO::begin()
{
    _lcd.begin(_screenSize[0], _screenSize[1]);
    _lcd.backlight(); // todo better backlight timeout handling
}


HydroDisplayU8g2lib::HydroDisplayU8g2lib(Hydro_DisplayOutputMode displayMode, DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayOrientation),
      _screenSize{0}, _gfx(nullptr), _gfxDrawable(nullptr), _renderer(nullptr)
{
    const u8g2_cb_t *u8g2Rotation = nullptr;
    switch (_displayOri) {
        case Hydro_DisplayOrientation_R0:
            u8g2Rotation = U8G2_R0;
            break;
        case Hydro_DisplayOrientation_R1:
            u8g2Rotation = U8G2_R1;
            break;
        case Hydro_DisplayOrientation_R2:
            u8g2Rotation = U8G2_R2;
            break;
        case Hydro_DisplayOrientation_R3:
            u8g2Rotation = U8G2_R3;
            break;
        case Hydro_DisplayOrientation_HorzMirror:
            u8g2Rotation = U8G2_MIRROR;
            break;
        case Hydro_DisplayOrientation_VertMirror:
            u8g2Rotation = U8G2_MIRROR_VERTICAL;
            break;
        default: break;
    }

    switch (displayMode) {
        case Hydro_DisplayOutputMode_SSD1305:
            _screenSize[0] = 128; _screenSize[1] = 32;
            if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                _gfx = new U8G2_SSD1305_128X32_NONAME_F_4W_HW_SPI(u8g2Rotation, displaySetup.cfgAs.spi.cs, dcPin, resetPin);
            } else if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
                _gfx = new U8G2_SSD1305_128X32_NONAME_F_HW_I2C(u8g2Rotation, resetPin);
            }
            break;
        case Hydro_DisplayOutputMode_SSD1305_x32Ada:
            _screenSize[0] = 128; _screenSize[1] = 32;
            if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                _gfx = new U8G2_SSD1305_128X32_ADAFRUIT_F_4W_HW_SPI(u8g2Rotation, displaySetup.cfgAs.spi.cs, dcPin, resetPin);
            } else if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
                _gfx = new U8G2_SSD1305_128X32_ADAFRUIT_F_HW_I2C(u8g2Rotation, resetPin);
            }
            break;
        case Hydro_DisplayOutputMode_SSD1305_x64Ada:
            _screenSize[0] = 128; _screenSize[1] = 64;
            if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                _gfx = new U8G2_SSD1305_128X64_ADAFRUIT_F_4W_HW_SPI(u8g2Rotation, displaySetup.cfgAs.spi.cs, dcPin, resetPin);
            } else if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
                _gfx = new U8G2_SSD1305_128X64_ADAFRUIT_F_HW_I2C(u8g2Rotation, resetPin);
            }
            break;
        case Hydro_DisplayOutputMode_SSD1306:
            _screenSize[0] = 128; _screenSize[1] = 64;
            if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                _gfx = new U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI(u8g2Rotation, displaySetup.cfgAs.spi.cs, dcPin, resetPin);
            } else if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
                _gfx = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(u8g2Rotation, resetPin);
            }
            break;
        case Hydro_DisplayOutputMode_SH1106:
            _screenSize[0] = 128; _screenSize[1] = 64;
            if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                _gfx = new U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI(u8g2Rotation, displaySetup.cfgAs.spi.cs, dcPin, resetPin);
            } else if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
                _gfx = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(u8g2Rotation, resetPin);
            }
            break;
        case Hydro_DisplayOutputMode_SSD1607_GD:
            _screenSize[0] = 200; _screenSize[1] = 200;
            if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                _gfx = new U8G2_SSD1607_GD_200X200_F_4W_HW_SPI(u8g2Rotation, displaySetup.cfgAs.spi.cs, dcPin, resetPin);
            }
            break;
        case Hydro_DisplayOutputMode_SSD1607_WS:
            _screenSize[0] = 200; _screenSize[1] = 200;
            if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                _gfx = new U8G2_SSD1607_WS_200X200_F_4W_HW_SPI(u8g2Rotation, displaySetup.cfgAs.spi.cs, dcPin, resetPin);
            }
            break;
        case Hydro_DisplayOutputMode_IL3820:
            _screenSize[0] = 296; _screenSize[1] = 128;
            if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                _gfx = new U8G2_IL3820_296X128_F_4W_HW_SPI(u8g2Rotation, displaySetup.cfgAs.spi.cs, dcPin, resetPin);
            }
            break;
        case Hydro_DisplayOutputMode_IL3820_V2:
            _screenSize[0] = 296; _screenSize[1] = 128;
            if (displaySetup.cfgType == DeviceSetup::SPISetup) {
                _gfx = new U8G2_IL3820_V2_296X128_F_4W_HW_SPI(u8g2Rotation, displaySetup.cfgAs.spi.cs, dcPin, resetPin);
            }
            break;
        default: break;
    }
    HYDRO_SOFT_ASSERT(_gfx, SFP(HStr_Err_AllocationFailure));

    if (_gfx) {
        _gfxDrawable = new U8g2Drawable(_gfx, displaySetup.cfgAs.i2c.wire);
        HYDRO_SOFT_ASSERT(_gfxDrawable, SFP(HStr_Err_AllocationFailure));

        if (_gfxDrawable) {
            _renderer = new GraphicsDeviceRenderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), _gfxDrawable);
            HYDRO_SOFT_ASSERT(_renderer, SFP(HStr_Err_AllocationFailure));

            if (_renderer) { _renderer->setTitleMode(BaseGraphicalRenderer::TITLE_FIRST_ROW); }
        }
    }
}

HydroDisplayU8g2lib::~HydroDisplayU8g2lib()
{
    if (_renderer) { delete _renderer; }
    if (_gfxDrawable) { delete _gfxDrawable; }
    if (_gfx) { delete _gfx; }
}

void HydroDisplayU8g2lib::init()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), Hydro_DisplayTheme_MonoOLED));
}

void HydroDisplayU8g2lib::begin()
{
    if (_gfx) { _gfx->begin(); }
}


HydroDisplayAdafruitGFX<Adafruit_ST7735>::HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, Hydro_ST7735Tab tabColor, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayOrientation), _tab(tabColor),
      #ifndef ESP8266
          _gfx(displaySetup.spi, dcPin, displaySetup.cs, resetPin),
      #else
          _gfx(displaySetup.cs, dcPin, resetPin),
      #endif
      _gfxDrawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), &_gfxDrawable)
{
    HYDRO_SOFT_ASSERT(_tab != Hydro_ST7735Tab_Undefined, SFP(HStr_Err_InvalidParameter));
    #ifdef ESP8266
        HYDRO_SOFT_ASSERT(displaySetup.spi == HYDRO_USE_SPI, SFP(HStr_Err_NotConfiguredProperly));
    #endif
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

void HydroDisplayAdafruitGFX<Adafruit_ST7735>::init()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_SMLMED)), HYDRO_UI_GFXTFT_USES_AN_SLIDER);
}

void HydroDisplayAdafruitGFX<Adafruit_ST7735>::begin()
{
    _gfx.initR(_tab);
    _gfx.setRotation((uint8_t)_displayOri);
}


HydroDisplayAdafruitGFX<Adafruit_ST7789>::HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayOrientation),
      #ifndef ESP8266
          _gfx(displaySetup.spi, dcPin, displaySetup.cs, resetPin),
      #else
          _gfx(displaySetup.cs, dcPin, resetPin),
      #endif
      _gfxDrawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), &_gfxDrawable)
{
    #ifdef ESP8266
        HYDRO_SOFT_ASSERT(displaySetup.spi == HYDRO_USE_SPI, SFP(HStr_Err_NotConfiguredProperly));
    #endif
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

void HydroDisplayAdafruitGFX<Adafruit_ST7789>::init()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_SMLMED)), HYDRO_UI_GFXTFT_USES_AN_SLIDER);
}

void HydroDisplayAdafruitGFX<Adafruit_ST7789>::begin()
{
    _gfx.init(320, 240);
    _gfx.setRotation((uint8_t)_displayOri);
}


HydroDisplayAdafruitGFX<Adafruit_PCD8544>::HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayOrientation),
      _gfx(dcPin, displaySetup.cs, resetPin, displaySetup.spi),
      _gfxDrawable(&_gfx),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), &_gfxDrawable)
{
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

void HydroDisplayAdafruitGFX<Adafruit_PCD8544>::init()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_SMLMED)), HYDRO_UI_GFXTFT_USES_AN_SLIDER);
}

void HydroDisplayAdafruitGFX<Adafruit_PCD8544>::begin()
{
    _gfx.begin();
    _gfx.setRotation((uint8_t)_displayOri);
}


HydroDisplayTFTeSPI::HydroDisplayTFTeSPI(SPIDeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, uint16_t screenWidth, uint16_t screenHeight)
    : HydroDisplayDriver(displayOrientation), _screenSize{screenWidth, screenHeight},
      _gfx(screenWidth, screenHeight),
      _gfxDrawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), &_gfxDrawable)
{
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

void HydroDisplayTFTeSPI::init()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_MEDLRG)), HYDRO_UI_GFXTFT_USES_AN_SLIDER);
}

void HydroDisplayTFTeSPI::begin()
{
    _gfx.begin();
    _gfx.setRotation((uint8_t)_displayOri);
}

#endif
