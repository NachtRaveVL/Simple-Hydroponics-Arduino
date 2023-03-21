/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Display Drivers
*/

#include "HydruinoUI.h"
#ifdef HYDRO_USE_GUI
#include "BaseRenderers.h"
#include "graphics/BaseGraphicalRenderer.h"
#include "IoAbstractionWire.h"
#include "DfRobotInputAbstraction.h"

HydroDisplayDriver::HydroDisplayDriver(Hydro_DisplayRotation displayRotation)
    : _rotation(displayRotation), _displayTheme(Hydro_DisplayTheme_Undefined)
{ ; }

void HydroDisplayDriver::commonInit(uint8_t updatesPerSec, Hydro_DisplayTheme displayTheme, bool analogSlider, bool utf8Fonts, const void *itemFont, const void *titleFont, bool needEditingIcons)
{
    auto baseRenderer = getBaseRenderer();
    auto graphicsRenderer = getGraphicsRenderer();

    baseRenderer->setCustomDrawingHandler(getBaseUI());
    baseRenderer->setUpdatesPerSecond(updatesPerSec);
    if (graphicsRenderer) {
        if (getController() && getController()->getControlInputMode() >= Hydro_ControlInputMode_ResistiveTouch &&
            getController()->getControlInputMode() < Hydro_ControlInputMode_RemoteControl) {
            graphicsRenderer->setHasTouchInterface(true);
        }
        graphicsRenderer->setUseSliderForAnalog(analogSlider);
        if (utf8Fonts) { graphicsRenderer->enableTcUnicode(); }

        if (_displayTheme != displayTheme) {
            switch ((_displayTheme = displayTheme)) {
                case Hydro_DisplayTheme_CoolBlue_ML:
                    installCoolBlueModernTheme(*graphicsRenderer, MenuFontDef(itemFont, 1), MenuFontDef(titleFont, 1), needEditingIcons);
                    break;
                case Hydro_DisplayTheme_CoolBlue_SM:
                    installCoolBlueTraditionalTheme(*graphicsRenderer, MenuFontDef(itemFont, 1), MenuFontDef(titleFont, 1), needEditingIcons);
                    break;
                case Hydro_DisplayTheme_DarkMode_ML:
                    installDarkModeModernTheme(*graphicsRenderer, MenuFontDef(itemFont, 1), MenuFontDef(titleFont, 1), needEditingIcons);
                    break;
                case Hydro_DisplayTheme_DarkMode_SM:
                    installDarkModeTraditionalTheme(*graphicsRenderer, MenuFontDef(itemFont, 1), MenuFontDef(titleFont, 1), needEditingIcons);
                    break;
                case Hydro_DisplayTheme_MonoOLED:
                    installMonoBorderedTheme(*graphicsRenderer, MenuFontDef(itemFont, 1), MenuFontDef(titleFont, 1), needEditingIcons);
                    break;
                case Hydro_DisplayTheme_MonoOLED_Inv:
                    installMonoInverseTitleTheme(*graphicsRenderer, MenuFontDef(itemFont, 1), MenuFontDef(titleFont, 1), needEditingIcons);
                    break;
            }
        }
    }
}


HydroDisplayLiquidCrystal::HydroDisplayLiquidCrystal(Hydro_DisplayOutputMode displayMode, I2CDeviceSetup displaySetup, Hydro_BacklightMode ledMode)
    : _screenSize{displayMode < Hydro_DisplayOutputMode_LCD20x4_EN ? 16 : 20, displayMode < Hydro_DisplayOutputMode_LCD20x4_EN ? 2 : 4},
      _lcd(displayMode == Hydro_DisplayOutputMode_LCD16x2_EN || displayMode == Hydro_DisplayOutputMode_LCD20x4_EN ? 2 : 0, 1,
           displayMode == Hydro_DisplayOutputMode_LCD16x2_EN || displayMode == Hydro_DisplayOutputMode_LCD20x4_EN ? 0 : 2, 4, 5, 6, 7,
           ledMode == Hydro_BacklightMode_Normal ? LiquidCrystal::BACKLIGHT_NORMAL : ledMode == Hydro_BacklightMode_Inverted ? LiquidCrystal::BACKLIGHT_INVERTED : LiquidCrystal::BACKLIGHT_PWM,
           ioFrom8574(HYDRO_UI_I2C_LCD_BASEADDR | displaySetup.address, 0xff, displaySetup.wire, false)),
      _renderer(_lcd, _screenSize[0], _screenSize[1], getController()->getSystemNameChars())
{
    _lcd.configureBacklightPin(3);
    _renderer.setTitleRequired(false);
}

HydroDisplayLiquidCrystal::HydroDisplayLiquidCrystal(bool isDFRobotShield_unused, I2CDeviceSetup displaySetup, Hydro_BacklightMode ledMode)
    : _screenSize{16, 2},
      _lcd(8, 9, 4, 5, 6, 7,
           ledMode == Hydro_BacklightMode_Normal ? LiquidCrystal::BACKLIGHT_NORMAL : ledMode == Hydro_BacklightMode_Inverted ? LiquidCrystal::BACKLIGHT_INVERTED : LiquidCrystal::BACKLIGHT_PWM,
           ioFrom8574(HYDRO_UI_I2C_LCD_BASEADDR | displaySetup.address, 0xff, displaySetup.wire, false)),
      _renderer(_lcd, _screenSize[0], _screenSize[1], getController()->getSystemNameChars())
{
    _lcd.configureBacklightPin(10);
    _renderer.setTitleRequired(false);
}

void HydroDisplayLiquidCrystal::initBaseUIFromDefaults()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED);
}

void HydroDisplayLiquidCrystal::begin()
{
    _lcd.begin(_screenSize[0], _screenSize[1]);
}

HydroOverview *HydroDisplayLiquidCrystal::allocateOverview()
{
    return new HydroOverviewLCD(this);
}


HydroDisplayU8g2OLED::HydroDisplayU8g2OLED(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, U8G2 *gfx)
    : HydroDisplayDriver(displayRotation),
      _screenSize{gfx->getDisplayWidth(), gfx->getDisplayHeight()}, _gfx(gfx), _drawable(nullptr), _renderer(nullptr)
{
    HYDRO_SOFT_ASSERT(_gfx, SFP(HStr_Err_AllocationFailure));
    if (_gfx) {
        if (displaySetup.cfgType == DeviceSetup::I2CSetup) {
            _gfx->setI2CAddress(HYDRO_UI_I2C_OLED_BASEADDR | displaySetup.cfgAs.i2c.address);
            _drawable = new U8g2Drawable(_gfx, displaySetup.cfgAs.i2c.wire);
        } else {
            _drawable = new U8g2Drawable(_gfx);
        }
        HYDRO_SOFT_ASSERT(_drawable, SFP(HStr_Err_AllocationFailure));

        if (_drawable) {
            _renderer = new GraphicsDeviceRenderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), _drawable);
            HYDRO_SOFT_ASSERT(_renderer, SFP(HStr_Err_AllocationFailure));

            if (_renderer) { _renderer->setTitleMode(BaseGraphicalRenderer::TITLE_FIRST_ROW); }
        }
    }
}

HydroDisplayU8g2OLED::~HydroDisplayU8g2OLED()
{
    if (_renderer) { delete _renderer; }
    if (_drawable) { delete _drawable; }
    if (_gfx) { delete _gfx; }
}

void HydroDisplayU8g2OLED::initBaseUIFromDefaults()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), Hydro_DisplayTheme_MonoOLED));
}

void HydroDisplayU8g2OLED::begin()
{
    if (_gfx) { _gfx->begin(); }
    if (_renderer) { _renderer->setDisplayDimensions(getScreenSize().first, getScreenSize().second); }
}

HydroOverview *HydroDisplayU8g2OLED::allocateOverview()
{
    return new HydroOverviewOLED(this);
}


HydroDisplayAdafruitGFX<Adafruit_ST7735>::HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, Hydro_ST7735Tab tabColor, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayRotation), _tab(tabColor),
      #ifndef ESP8266
          _gfx(displaySetup.spi, intForPin(dcPin), intForPin(displaySetup.cs), intForPin(resetPin)),
      #else
          _gfx(intForPin(displaySetup.cs), intForPin(dcPin), intForPin(resetPin)),
      #endif
      _drawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), &_drawable)
{
    HYDRO_SOFT_ASSERT(_tab != Hydro_ST7735Tab_Undefined, SFP(HStr_Err_InvalidParameter));
    #ifdef ESP8266
        HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    #endif
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

void HydroDisplayAdafruitGFX<Adafruit_ST7735>::initBaseUIFromDefaults()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_SMLMED)), HYDRO_UI_GFX_VARS_USES_SLIDER);
}

void HydroDisplayAdafruitGFX<Adafruit_ST7735>::begin()
{
    if (_tab == Hydro_ST7735Tab_BModel) {
        _gfx.initB();
    } else {
        _gfx.initR((uint8_t)_tab);
    }
    _gfx.setRotation((uint8_t)_rotation);
    _renderer.setDisplayDimensions(getScreenSize().first, getScreenSize().second);
}

HydroOverview *HydroDisplayAdafruitGFX<Adafruit_ST7735>::allocateOverview()
{
    return new HydroOverviewGFX<Adafruit_ST7735>(this);
}


HydroDisplayAdafruitGFX<Adafruit_ST7789>::HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayRotation),
      #ifndef ESP8266
          _gfx(displaySetup.spi, intForPin(dcPin), intForPin(displaySetup.cs), intForPin(resetPin)),
      #else
          _gfx(intForPin(displaySetup.cs), intForPin(dcPin), intForPin(resetPin)),
      #endif
      _drawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), &_drawable)
{
    #ifdef ESP8266
        HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    #endif
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

void HydroDisplayAdafruitGFX<Adafruit_ST7789>::initBaseUIFromDefaults()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_SMLMED)), HYDRO_UI_GFX_VARS_USES_SLIDER);
}

void HydroDisplayAdafruitGFX<Adafruit_ST7789>::begin()
{
    _gfx.init(TFT_GFX_WIDTH, TFT_GFX_HEIGHT);
    _gfx.setRotation((uint8_t)_rotation);
    _renderer.setDisplayDimensions(getScreenSize().first, getScreenSize().second);
}

HydroOverview *HydroDisplayAdafruitGFX<Adafruit_ST7789>::allocateOverview()
{
    return new HydroOverviewGFX<Adafruit_ST7789>(this);
}


HydroDisplayAdafruitGFX<Adafruit_ILI9341>::HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayRotation),
      #ifndef ESP8266
          _gfx(displaySetup.spi, intForPin(dcPin), intForPin(displaySetup.cs), intForPin(resetPin)),
      #else
          _gfx(intForPin(displaySetup.cs), intForPin(dcPin), intForPin(resetPin)),
      #endif
      _drawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), &_drawable)
{
    #ifdef ESP8266
        HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    #endif
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

void HydroDisplayAdafruitGFX<Adafruit_ILI9341>::initBaseUIFromDefaults()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_SMLMED)), HYDRO_UI_GFX_VARS_USES_SLIDER);
}

void HydroDisplayAdafruitGFX<Adafruit_ILI9341>::begin()
{
    _gfx.begin(getController() ? getController()->getDisplaySetup().cfgAs.spi.speed : 0);
    _gfx.setRotation((uint8_t)_rotation);
    _renderer.setDisplayDimensions(getScreenSize().first, getScreenSize().second);
}

HydroOverview *HydroDisplayAdafruitGFX<Adafruit_ILI9341>::allocateOverview()
{
    return new HydroOverviewGFX<Adafruit_ILI9341>(this);
}


HydroDisplayTFTeSPI::HydroDisplayTFTeSPI(SPIDeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, uint16_t screenWidth, uint16_t screenHeight, Hydro_ST7735Tab tabColor)
    : HydroDisplayDriver(displayRotation),
      _screenSize{screenWidth, screenHeight}, _tabColor(tabColor),
      _gfx(screenWidth, screenHeight),
      _drawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), &_drawable)
{
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

void HydroDisplayTFTeSPI::initBaseUIFromDefaults()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_MEDLRG)), HYDRO_UI_GFX_VARS_USES_SLIDER);
}

void HydroDisplayTFTeSPI::begin()
{
    if (_tabColor == Hydro_ST7735Tab_BModel) {
        _gfx.begin();
    } else {
        _gfx.begin((uint8_t)_tabColor);
    }
    _gfx.setRotation((uint8_t)_rotation);
    _renderer.setDisplayDimensions(getScreenSize().first, getScreenSize().second);
}

HydroOverview *HydroDisplayTFTeSPI::allocateOverview()
{
    return new HydroOverviewTFT(this);
}

#endif
