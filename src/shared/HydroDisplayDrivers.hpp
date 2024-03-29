/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Display Drivers
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI

static inline const u8g2_cb_t *dispRotToU8g2Rot(Hydro_DisplayRotation displayRotation)
{
    switch (displayRotation) {
        case Hydro_DisplayRotation_R1: return U8G2_R1;
        case Hydro_DisplayRotation_R2: return U8G2_R2;
        case Hydro_DisplayRotation_R3: return U8G2_R3;
        case Hydro_DisplayRotation_HorzMirror: return U8G2_MIRROR;
        case Hydro_DisplayRotation_VertMirror: return U8G2_MIRROR_VERTICAL;
        case Hydro_DisplayRotation_R0: default: return U8G2_R0;
    }
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_NONAME_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_NONAME_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305Wire(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)HYDRO_USE_WIRE || displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_NONAME_F_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305Wire1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)HYDRO_USE_WIRE || ((bool)HYDRO_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1)), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_NONAME_F_2ND_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305x32AdaSPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_ADAFRUIT_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305x32AdaSPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_ADAFRUIT_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305x32AdaWire(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)HYDRO_USE_WIRE || displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_ADAFRUIT_F_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}
inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305x32AdaWire1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)HYDRO_USE_WIRE || ((bool)HYDRO_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1)), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X32_ADAFRUIT_F_2ND_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305x64AdaSPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X64_ADAFRUIT_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305x64AdaSPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X64_ADAFRUIT_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305x64AdaWire(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)HYDRO_USE_WIRE || displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X64_ADAFRUIT_F_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1305x64AdaWire1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)HYDRO_USE_WIRE || ((bool)HYDRO_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1)), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1305_128X64_ADAFRUIT_F_2ND_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1306SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1306SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1306_128X64_NONAME_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1306Wire(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)HYDRO_USE_WIRE || displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1306Wire1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)HYDRO_USE_WIRE || ((bool)HYDRO_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1)), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1306_128X64_NONAME_F_2ND_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSH1106SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSH1106SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SH1106_128X64_NONAME_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSH1106Wire(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)HYDRO_USE_WIRE || displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SH1106_128X64_NONAME_F_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSH1106Wire1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!(bool)HYDRO_USE_WIRE || ((bool)HYDRO_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1)), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateCustomOLEDI2C(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new HYDRO_UI_CUSTOM_OLED_I2C(dispRotToU8g2Rot(displayRotation), resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateCustomOLEDSPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new HYDRO_UI_CUSTOM_OLED_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1607SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1607_200X200_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateSSD1607SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_SSD1607_200X200_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateIL3820SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_IL3820_296X128_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateIL3820SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_IL3820_296X128_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateIL3820V2SPI(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_IL3820_V2_296X128_F_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2OLED *HydroDisplayU8g2OLED::allocateIL3820V2SPI1(DeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2OLED(displaySetup, displayRotation, new U8G2_IL3820_V2_296X128_F_2ND_4W_HW_SPI(dispRotToU8g2Rot(displayRotation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}


template <class T>
HydroDisplayAdafruitGFX<T>::HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayRotation displayRotation, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayRotation, _gfx.width(), _gfx.height()), // possibly incorrect until after begin
    #ifndef ESP8266
          _gfx(displaySetup.spi, intForPin(dcPin), intForPin(displaySetup.cs), intForPin(resetPin)),
      #else
          _gfx(intForPin(displaySetup.cs), intForPin(dcPin), intForPin(resetPin)),
      #endif
      _drawable(&_gfx, getBaseUI() ? getBaseUI()->getSpriteHeight() : 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, HydroDisplayDriver::getSystemName(), &_drawable)
{
    #ifdef ESP8266
        HYDRO_SOFT_ASSERT(!(bool)HYDRO_USE_SPI || displaySetup.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    #endif
}

template <class T>
void HydroDisplayAdafruitGFX<T>::initBaseUIFromDefaults()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_SMLMED)), Hydro_TitleMode_Always, HYDRO_UI_GFX_VARS_USES_SLIDER, HYDRO_UI_GFX_USE_EDITING_ICONS);
}

template <class T>
void HydroDisplayAdafruitGFX<T>::begin()
{
    _gfx.begin();
    _screenSize[0] = _gfx.width();
    _screenSize[1] = _gfx.height();
    _gfx.setRotation((uint8_t)_rotation);
}

template <class T>
HydroOverview *HydroDisplayAdafruitGFX<T>::allocateOverview(const void *clockFont, const void *detailFont)
{
    return new HydroOverviewGFX<T>(this, clockFont, detailFont);
}

#endif
