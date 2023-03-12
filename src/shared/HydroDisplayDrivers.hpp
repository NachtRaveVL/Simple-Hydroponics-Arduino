/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Display Drivers
*/

#include "Hydruino.h"
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"

static inline const u8g2_cb_t *dispRotToU8g2Rot(Hydro_DisplayOrientation displayOrientation)
{
    switch (displayOrientation) {
        case Hydro_DisplayOrientation_R0: return U8G2_R0;
        case Hydro_DisplayOrientation_R1: return U8G2_R1;
        case Hydro_DisplayOrientation_R2: return U8G2_R2;
        case Hydro_DisplayOrientation_R3: return U8G2_R3;
        case Hydro_DisplayOrientation_HorzMirror: return U8G2_MIRROR;
        case Hydro_DisplayOrientation_VertMirror: return U8G2_MIRROR_VERTICAL;
        default: return U8G2_R0;
    }
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1305SPI(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 32, new U8G2_SSD1305_128X32_NONAME_F_4W_HW_SPI(dispRotToU8g2Rot(displayOrientation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1305I2C(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!HYDRO_USE_WIRE || displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 32, new U8G2_SSD1305_128X32_NONAME_F_HW_I2C(dispRotToU8g2Rot(displayOrientation), resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1305I2C2(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!HYDRO_USE_WIRE || (HYDRO_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1)), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 32, new U8G2_SSD1305_128X32_NONAME_F_2ND_HW_I2C(dispRotToU8g2Rot(displayOrientation), resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1305x32AdaSPI(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 32, new U8G2_SSD1305_128X32_ADAFRUIT_F_4W_HW_SPI(dispRotToU8g2Rot(displayOrientation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1305x32AdaI2C(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!HYDRO_USE_WIRE || displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 32, new U8G2_SSD1305_128X32_ADAFRUIT_F_HW_I2C(dispRotToU8g2Rot(displayOrientation), resetPin));
}
inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1305x32AdaI2C2(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!HYDRO_USE_WIRE || (HYDRO_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1)), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 32, new U8G2_SSD1305_128X32_ADAFRUIT_F_2ND_HW_I2C(dispRotToU8g2Rot(displayOrientation), resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1305x64AdaSPI(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 64, new U8G2_SSD1305_128X64_ADAFRUIT_F_4W_HW_SPI(dispRotToU8g2Rot(displayOrientation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1305x64AdaI2C(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!HYDRO_USE_WIRE || displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 64, new U8G2_SSD1305_128X64_ADAFRUIT_F_HW_I2C(dispRotToU8g2Rot(displayOrientation), resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1305x64AdaI2C2(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!HYDRO_USE_WIRE || (HYDRO_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1)), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 64, new U8G2_SSD1305_128X64_ADAFRUIT_F_2ND_HW_I2C(dispRotToU8g2Rot(displayOrientation), resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1306SPI(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 64, new U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI(dispRotToU8g2Rot(displayOrientation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1306I2C(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!HYDRO_USE_WIRE || displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 64, new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(dispRotToU8g2Rot(displayOrientation), resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1306I2C2(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!HYDRO_USE_WIRE || (HYDRO_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1)), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 64, new U8G2_SSD1306_128X64_NONAME_F_2ND_HW_I2C(dispRotToU8g2Rot(displayOrientation), resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSH1106SPI(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 64, new U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI(dispRotToU8g2Rot(displayOrientation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSH1106I2C(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!HYDRO_USE_WIRE || displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 64, new U8G2_SH1106_128X64_NONAME_F_HW_I2C(dispRotToU8g2Rot(displayOrientation), resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSH1106I2C2(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::I2CSetup && (!HYDRO_USE_WIRE || (HYDRO_USE_WIRE1 && displaySetup.cfgAs.i2c.wire == HYDRO_USE_WIRE1)), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 128, 64, new U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C(dispRotToU8g2Rot(displayOrientation), resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1607GDSPI(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(displaySetup.cfgType == DeviceSetup::SPISetup && (!HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI), SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 200, 200, new U8G2_SSD1607_GD_200X200_F_4W_HW_SPI(dispRotToU8g2Rot(displayOrientation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateSSD1607WSSPI(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(!HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 200, 200, new U8G2_SSD1607_WS_200X200_F_4W_HW_SPI(dispRotToU8g2Rot(displayOrientation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateIL3820SPI(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(!HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 296, 128, new U8G2_IL3820_296X128_F_4W_HW_SPI(dispRotToU8g2Rot(displayOrientation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}

inline HydroDisplayU8g2lib *HydroDisplayU8g2lib::allocateIL3820V2SPI(DeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
{
    HYDRO_SOFT_ASSERT(!HYDRO_USE_SPI || displaySetup.cfgAs.spi.spi == HYDRO_USE_SPI, SFP(HStr_Err_InvalidParameter));
    return new HydroDisplayU8g2lib(displaySetup, displayOrientation, 296, 128, new U8G2_IL3820_V2_296X128_F_4W_HW_SPI(dispRotToU8g2Rot(displayOrientation), displaySetup.cfgAs.spi.cs, dcPin, resetPin));
}


template <class T>
HydroDisplayAdafruitGFX<T>::HydroDisplayAdafruitGFX(SPIDeviceSetup displaySetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayOrientation),
    #ifndef ESP8266
          _gfx(displaySetup.spi, dcPin, displaySetup.cs, resetPin),
      #else
          _gfx(displaySetup.cs, dcPin, resetPin),
      #endif
      _gfxDrawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, getController()->getSystemNameChars(), &_gfxDrawable)
{
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

template <class T>
void HydroDisplayAdafruitGFX<T>::init()
{
    getBaseUI()->init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN3(Hydro_DisplayTheme, HYDRO_UI_GFX_DISP_THEME_BASE, HYDRO_UI_GFX_DISP_THEME_SMLMED)), HYDRO_UI_GFXTFT_USES_AN_SLIDER);
}

template <class T>
void HydroDisplayAdafruitGFX<T>::begin()
{
    _gfx.begin();
    _gfx.setRotation((uint8_t)_displayOri);
}

#endif
