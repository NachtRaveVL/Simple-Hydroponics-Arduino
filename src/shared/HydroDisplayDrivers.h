/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Display Drivers
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroDisplayDrivers_H
#define HydroDisplayDrivers_H

class HydroDisplayDriver;
class HydroDisplayLiquidCrystal;
class HydroDisplayU8g2OLED;
template <class T> class HydroDisplayAdafruitGFX;
class HydroDisplayTFTeSPI;

#include "HydruinoUI.h"

// Driver Display Base
// Base display driver class that manages display output mode selection.
class HydroDisplayDriver {
public:
    HydroDisplayDriver(Hydro_DisplayRotation displayRotation = Hydro_DisplayRotation_Undefined);
    virtual ~HydroDisplayDriver() = default;

    void commonInit(uint8_t updatesPerSec, bool analogSlider = false, bool utf8Fonts = false);
    void installTheme(Hydro_DisplayTheme displayTheme, const void *itemFont = nullptr, const void *titleFont = nullptr, bool needEditingIcons = false);

    virtual void initBaseUIFromDefaults() = 0;
    virtual void begin() = 0;

    virtual HydroOverview *allocateOverview() = 0;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const = 0;
    virtual bool isLandscape() const = 0;
    inline bool isPortrait() const { return !isLandscape(); }
    virtual uint8_t getScreenBits() const = 0;
    inline bool isMonochrome() const { return getScreenBits() == 1; }
    inline bool isColor() const { return getScreenBits() > 1; }
    inline bool is16BitColor() const { return getScreenBits() == 16; }
    inline bool isFullColor() const { return getScreenBits() == 24; }

    virtual BaseMenuRenderer *getBaseRenderer() = 0;
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() = 0;

    inline Hydro_DisplayRotation getRotation() const { return _rotation; }
    inline Hydro_DisplayTheme getDisplayTheme() const { return _displayTheme; }

protected:
    const Hydro_DisplayRotation _rotation;
    Hydro_DisplayTheme _displayTheme;
};


// Liquid Crystal Display Driver
// Display driver for text-only monochrome LCD, typically ones that talk through a PCF8574 i2c driver or similar.
// Note: Parallel 6800/8080 raw data connections are not supported at this time.
class HydroDisplayLiquidCrystal : public HydroDisplayDriver {
public:
    HydroDisplayLiquidCrystal(Hydro_DisplayOutputMode displayMode, I2CDeviceSetup displaySetup, Hydro_BacklightMode ledMode = Hydro_BacklightMode_Normal);
    // Special constructor for DFRobotShield /w 16x2 LCD (isDFRobotShield_unused tossed, only used for constructor resolution)
    HydroDisplayLiquidCrystal(bool isDFRobotShield_unused, I2CDeviceSetup displaySetup, Hydro_BacklightMode ledMode = Hydro_BacklightMode_Normal);
    virtual ~HydroDisplayLiquidCrystal() = default;

    virtual void initBaseUIFromDefaults() override;
    virtual void begin() override;

    virtual HydroOverview *allocateOverview() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return isLandscape() ? make_pair((uint16_t)max(_screenSize[0],_screenSize[1]), (uint16_t)min(_screenSize[0],_screenSize[1]))
                                                                                          : make_pair((uint16_t)min(_screenSize[0],_screenSize[1]), (uint16_t)max(_screenSize[0],_screenSize[1])); }
    virtual bool isLandscape() const override { return _screenSize[0] >= _screenSize[1]; }
    virtual uint8_t getScreenBits() const override { return 1; }

    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return nullptr; }

    inline LiquidCrystal &getLCD() { return _lcd; }

protected:
    uint8_t _screenSize[2];
    LiquidCrystal _lcd;
    LiquidCrystalRenderer _renderer;
};


// Monochrome OLED Display Driver
// Display driver for monochrome OLED LCDs.
// Note: Uses HYDRO_UI_CUSTOM_OLED_* defines for custom OLED support.
class HydroDisplayU8g2OLED : public HydroDisplayDriver {
public:
    HydroDisplayU8g2OLED(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, U8G2 *gfx);
    virtual ~HydroDisplayU8g2OLED();

    virtual void initBaseUIFromDefaults() override;
    virtual void begin() override;

    virtual HydroOverview *allocateOverview() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return isLandscape() ? make_pair(max(_screenSize[0],_screenSize[1]), min(_screenSize[0],_screenSize[1]))
                                                                                          : make_pair(min(_screenSize[0],_screenSize[1]), max(_screenSize[0],_screenSize[1])); }
    virtual bool isLandscape() const override { return _screenSize[0] >= _screenSize[1] ? !(_rotation == Hydro_DisplayRotation_R1 || _rotation == Hydro_DisplayRotation_R3)
                                                                                        : (_rotation == Hydro_DisplayRotation_R1 || _rotation == Hydro_DisplayRotation_R3); }
    virtual uint8_t getScreenBits() const override { return 1; }

    virtual BaseMenuRenderer *getBaseRenderer() override { return _renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return _renderer; }

    inline U8G2 &getGfx() { return *_gfx; }
    inline U8g2Drawable &getDrawable() { return *_drawable; }

protected:
    uint16_t _screenSize[2];
    U8G2 *_gfx;
    U8g2Drawable *_drawable;
    GraphicsDeviceRenderer *_renderer;

public:
    static inline HydroDisplayU8g2OLED *allocateSSD1305SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305Wire(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305Wire1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305x32AdaSPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305x32AdaSPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305x32AdaWire(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305x32AdaWire1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305x64AdaSPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305x64AdaSPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305x64AdaWire(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1305x64AdaWire1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1306SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1306SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1306Wire(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1306Wire1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSH1106SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSH1106SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSH1106Wire(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSH1106Wire1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateCustomOLEDI2C(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateCustomOLEDSPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1607SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateSSD1607SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateIL3820SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateIL3820SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateIL3820V2SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    static inline HydroDisplayU8g2OLED *allocateIL3820V2SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
};


// Generic AdafruitGFX Display Driver
// A generic base AdafruitGFX driver that serves as a template for all specialized drivers.
// Note: It is likely that the initializer/begin calls used will not be correct for any given AdafruitGFX class used. Consider always specializing.
template <class T>
class HydroDisplayAdafruitGFX : public HydroDisplayDriver {
public:
    HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    virtual ~HydroDisplayAdafruitGFX() = default;

    virtual void initBaseUIFromDefaults() override;
    virtual void begin() override;

    virtual HydroOverview *allocateOverview() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair((uint16_t)_gfx.width(), (uint16_t)_gfx.height()); }
    virtual bool isLandscape() const override { return _gfx.width() >= _gfx.height(); }
    virtual uint8_t getScreenBits() const override { return 16; }

    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return &_renderer; }

    inline T &getGfx() { return _gfx; }
    inline AdafruitDrawable<T> &getDrawable() { return _drawable; }

protected:
    T _gfx;
    AdafruitDrawable<T> _drawable;
    GraphicsDeviceRenderer _renderer;
};


// ST7735 AdafruitSPITFT Display Driver
// Advanced color display.
// Note: Class requires proper ST7735 tab color.
template <>
class HydroDisplayAdafruitGFX<Adafruit_ST7735> : public HydroDisplayDriver {
public:
    HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, Hydro_ST7735Tab tabColor, pintype_t dcPin, pintype_t resetPin);
    virtual ~HydroDisplayAdafruitGFX() = default;

    virtual void initBaseUIFromDefaults() override;
    virtual void begin() override;

    virtual HydroOverview *allocateOverview() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair((uint16_t)_gfx.width(), (uint16_t)_gfx.height()); }
    virtual bool isLandscape() const override { return _gfx.width() >= _gfx.height(); }
    virtual uint8_t getScreenBits() const override { return 16; }

    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return &_renderer; }

    inline Adafruit_ST7735 &getGfx() { return _gfx; }
    inline AdafruitDrawable<Adafruit_ST7735> &getDrawable() { return _drawable; }

protected:
    const Hydro_ST7735Tab _tab;
    Adafruit_ST7735 _gfx;
    AdafruitDrawable<Adafruit_ST7735> _drawable;
    GraphicsDeviceRenderer _renderer;
};


// ST7789 AdafruitSPITFT Display Driver
// Advanced color display.
template <>
class HydroDisplayAdafruitGFX<Adafruit_ST7789> : public HydroDisplayDriver {
public:
    HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    virtual ~HydroDisplayAdafruitGFX() = default;

    virtual void initBaseUIFromDefaults() override;
    virtual void begin() override;

    virtual HydroOverview *allocateOverview() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair((uint16_t)_gfx.width(), (uint16_t)_gfx.height()); }
    virtual bool isLandscape() const override { return _gfx.width() >= _gfx.height(); }
    virtual uint8_t getScreenBits() const override { return 16; }

    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return &_renderer; }

    inline Adafruit_ST7789 &getGfx() { return _gfx; }
    inline AdafruitDrawable<Adafruit_ST7789> &getDrawable() { return _drawable; }

protected:
    Adafruit_ST7789 _gfx;
    AdafruitDrawable<Adafruit_ST7789> _drawable;
    GraphicsDeviceRenderer _renderer;
};


// ILI9341 AdafruitSPITFT Display Driver
// Most modern widely-available TFT displays on market are typically a variant of this particular type.
template <>
class HydroDisplayAdafruitGFX<Adafruit_ILI9341> : public HydroDisplayDriver {
public:
    HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin);
    virtual ~HydroDisplayAdafruitGFX() = default;

    virtual void initBaseUIFromDefaults() override;
    virtual void begin() override;

    virtual HydroOverview *allocateOverview() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return make_pair((uint16_t)_gfx.width(), (uint16_t)_gfx.height()); }
    virtual bool isLandscape() const override { return _gfx.width() >= _gfx.height(); }
    virtual uint8_t getScreenBits() const override { return 16; }

    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return &_renderer; }

    inline Adafruit_ILI9341 &getGfx() { return _gfx; }
    inline AdafruitDrawable<Adafruit_ILI9341> &getDrawable() { return _drawable; }

protected:
    Adafruit_ILI9341 _gfx;
    AdafruitDrawable<Adafruit_ILI9341> _drawable;
    GraphicsDeviceRenderer _renderer;
};


// TFT_eSPI Display Driver
// Fastest library for working with TFT displays, but requires additional setup.
// Note: Usage of TFT_eSPI requires library setup via its TFT_eSPI\User_Setup.h.
class HydroDisplayTFTeSPI : public HydroDisplayDriver {
public:
    HydroDisplayTFTeSPI(SPIDeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, uint16_t screenWidth, uint16_t screenHeight, Hydro_ST7735Tab tabColor = Hydro_ST7735Tab_Undefined);
    virtual ~HydroDisplayTFTeSPI() = default;

    virtual void initBaseUIFromDefaults() override;
    virtual void begin() override;

    virtual HydroOverview *allocateOverview() override;

    virtual Pair<uint16_t,uint16_t> getScreenSize() const override { return isLandscape() ? make_pair(max(_screenSize[0],_screenSize[1]), min(_screenSize[0],_screenSize[1]))
                                                                                          : make_pair(min(_screenSize[0],_screenSize[1]), max(_screenSize[0],_screenSize[1])); }
    virtual bool isLandscape() const override { return _screenSize[0] >= _screenSize[1] ? !(_rotation == Hydro_DisplayRotation_R1 || _rotation == Hydro_DisplayRotation_R3)
                                                                                        : (_rotation == Hydro_DisplayRotation_R1 || _rotation == Hydro_DisplayRotation_R3); }
    virtual uint8_t getScreenBits() const override { return 24; }

    virtual BaseMenuRenderer *getBaseRenderer() override { return &_renderer; }
    virtual GraphicsDeviceRenderer *getGraphicsRenderer() override { return &_renderer; }

    inline TFT_eSPI &getGfx() { return _gfx; }
    inline TfteSpiDrawable &getDrawable() { return _drawable; }

protected:
    const uint16_t _screenSize[2];
    const Hydro_ST7735Tab _tabColor;
    TFT_eSPI _gfx;
    TfteSpiDrawable _drawable;
    GraphicsDeviceRenderer _renderer;
};

#endif // /ifndef HydroDisplayDrivers_H
#endif
