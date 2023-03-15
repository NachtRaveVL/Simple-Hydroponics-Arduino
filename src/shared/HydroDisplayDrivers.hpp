/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Display Drivers
*/

#include "Hydruino.h"
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"

static inline const u8g2_cb_t *dispRotToU8g2Rot(Hydro_DisplayRotation displayRotation)
{
    switch (displayRotation) {
        case Hydro_DisplayRotation_R0: return U8G2_R0;
        case Hydro_DisplayRotation_R1: return U8G2_R1;
        case Hydro_DisplayRotation_R2: return U8G2_R2;
        case Hydro_DisplayRotation_R3: return U8G2_R3;
        case Hydro_DisplayRotation_HorzMirror: return U8G2_MIRROR;
        case Hydro_DisplayRotation_VertMirror: return U8G2_MIRROR_VERTICAL;
        default: return U8G2_R0;
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
    : HydroDisplayDriver(displayRotation),
    #ifndef ESP8266
          _gfx(displaySetup.spi, dcPin, displaySetup.cs, resetPin),
      #else
          _gfx(displaySetup.cs, dcPin, resetPin),
      #endif
      _drawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), &_drawable)
{
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

template <class T>
void HydroDisplayAdafruitGFX<T>::initBaseUIFromDefaults()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_SMLMED)), HYDRO_UI_GFXTFT_USES_SLIDER);
}

template <class T>
void HydroDisplayAdafruitGFX<T>::begin()
{
    _gfx.begin();
    _gfx.setRotation((uint8_t)_rotation);
}

template <class T>
HydroOverview *HydroDisplayAdafruitGFX<T>::createOverview()
{
    // Unknown if type inherits from Adafruit_SPITFT or not, so Gfx used for compatibility
    return new HydroOverviewAdaGfx<T>(this);
}

#endif
