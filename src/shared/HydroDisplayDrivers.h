/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Display Drivers
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroDisplayDrivers_H
#define HydroDisplayDrivers_H

class HydroDisplayDriver;
class HydroDisplayLiquidCrystalIO;
class HydroDisplayU8g2lib;
template <class T> class HydroDisplayAdafruitGFX;
class HydroDisplayTFTeSPI;

#include "HydruinoUI.h"

class HydroDisplayDriver {
public:
    HydroDisplayDriver(Hydro_DisplayOrientation displayOrientation = Hydro_DisplayOrientation_Undefined);
    virtual ~HydroDisplayDriver() = default;

    void commonInit(uint8_t updatesPerSec, Hydro_DisplayTheme displayTheme, bool analogSlider = false, bool utf8Fonts = false);

    virtual void init() = 0;
    virtual void begin() = 0;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const = 0;
    virtual BaseMenuRenderer *getBaseRenderer() = 0;
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() = 0;

    inline Hydro_DisplayOrientation getDisplayOrientation() const { return _displayOri; }
    inline Hydro_DisplayTheme getDisplayTheme() const { return _displayTheme; }

protected:
    const Hydro_DisplayOrientation _displayOri;
    Hydro_DisplayTheme _displayTheme;
};

class HydroDisplayLiquidCrystalIO : public HydroDisplayDriver {
public:
    HydroDisplayLiquidCrystalIO(Hydro_DisplayOutputMode displayMode, I2CDeviceSetup lcdSetup, bool bitInversion = false, LiquidCrystal::BackLightPinMode backlightPinMode = LiquidCrystal::BACKLIGHT_NORMAL);
    // Special constructor for DFRobotShield /w 16x2 LCD (isDFRobotShield_unused tossed, only used for constructor resolution)
    HydroDisplayLiquidCrystalIO(bool isDFRobotShield_unused, I2CDeviceSetup lcdSetup, bool bitInversion = false, LiquidCrystal::BackLightPinMode backlightPinMode = LiquidCrystal::BACKLIGHT_NORMAL);
    virtual ~HydroDisplayLiquidCrystalIO() = default;

    virtual void init() override;
    virtual void begin() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair((uint16_t)_screenSize[0], (uint16_t)_screenSize[1]); }
    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return nullptr; }

    inline LiquidCrystal &getLCD() { return _lcd; }

protected:
    uint8_t _screenSize[2];
    LiquidCrystal _lcd;
    LiquidCrystalRenderer _renderer;
};

class HydroDisplayU8g2lib : public HydroDisplayDriver {
public:
    HydroDisplayU8g2lib(Hydro_DisplayOutputMode displayMode, DeviceSetup lcdSetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin);
    virtual ~HydroDisplayU8g2lib();

    virtual void init() override;
    virtual void begin() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair(_screenSize[0], _screenSize[1]); }
    virtual BaseMenuRenderer *getBaseRenderer() override { return _renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return _renderer; }

    inline U8G2 *getOLED() { return _gfx; }

protected:
    uint16_t _screenSize[2];
    U8G2 *_gfx;
    U8g2Drawable *_gfxDrawable;
    GraphicsDeviceRenderer *_renderer;
};

template <class T>
class HydroDisplayAdafruitGFX : public HydroDisplayDriver {
public:
    HydroDisplayAdafruitGFX(SPIDeviceSetup lcdSetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin);
    virtual ~HydroDisplayAdafruitGFX() = default;

    virtual void init() override;
    virtual void begin() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair((uint16_t)320, (uint16_t)240); }
    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return &_renderer; }

    inline T &getGfx() { return _gfx; }

protected:
    T _gfx;
    AdafruitDrawable<T> _gfxDrawable;
    GraphicsDeviceRenderer _renderer;
};

template <>
class HydroDisplayAdafruitGFX<Adafruit_ST7735> : public HydroDisplayDriver {
public:
    HydroDisplayAdafruitGFX(SPIDeviceSetup lcdSetup, Hydro_DisplayOrientation displayOrientation, Hydro_ST7735Tab tabColor, pintype_t dcPin, pintype_t resetPin);
    virtual ~HydroDisplayAdafruitGFX() = default;

    virtual void init() override;
    virtual void begin() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair((uint16_t)320, (uint16_t)240); }
    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return &_renderer; }

    inline Adafruit_ST7735 &getGfx() { return _gfx; }

protected:
    const Hydro_ST7735Tab _tab;
    Adafruit_ST7735 _gfx;
    AdafruitDrawable<Adafruit_ST7735> _gfxDrawable;
    GraphicsDeviceRenderer _renderer;
};

template <>
class HydroDisplayAdafruitGFX<Adafruit_ST7789> : public HydroDisplayDriver {
public:
    HydroDisplayAdafruitGFX(SPIDeviceSetup lcdSetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin);
    virtual ~HydroDisplayAdafruitGFX() = default;

    virtual void init() override;
    virtual void begin() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair((uint16_t)320, (uint16_t)240); }
    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return &_renderer; }

    inline Adafruit_ST7789 &getGfx() { return _gfx; }

protected:
    Adafruit_ST7789 _gfx;
    AdafruitDrawable<Adafruit_ST7789> _gfxDrawable;
    GraphicsDeviceRenderer _renderer;
};

template <>
class HydroDisplayAdafruitGFX<Adafruit_PCD8544> : public HydroDisplayDriver {
public:
    HydroDisplayAdafruitGFX(SPIDeviceSetup lcdSetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin);
    virtual ~HydroDisplayAdafruitGFX() = default;

    virtual void init() override;
    virtual void begin() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair((uint16_t)320, (uint16_t)240); }
    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return &_renderer; }

    inline Adafruit_PCD8544 &getGfx() { return _gfx; }

protected:
    Adafruit_PCD8544 _gfx;
    AdafruitDrawable<Adafruit_PCD8544> _gfxDrawable;
    GraphicsDeviceRenderer _renderer;
};

class HydroDisplayTFTeSPI : public HydroDisplayDriver {
public:
    HydroDisplayTFTeSPI(SPIDeviceSetup lcdSetup, Hydro_DisplayOrientation displayOrientation, uint16_t screenWidth, uint16_t screenHeight);
    virtual ~HydroDisplayTFTeSPI() = default;

    virtual void init() override;
    virtual void begin() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair(_screenSize[0], _screenSize[1]); }
    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return &_renderer; }

    inline TFT_eSPI &getGfx() { return _gfx; }

protected:
    const uint16_t _screenSize[2];
    TFT_eSPI _gfx;
    TfteSpiDrawable _gfxDrawable;
    GraphicsDeviceRenderer _renderer;
};

#endif // /ifndef HydroDisplayDrivers_H
#endif
