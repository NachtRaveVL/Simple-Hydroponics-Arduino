/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Display Drivers
*/

#include "Hydruino.h"
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"

template <class T>
HydroDisplayAdafruitGFX<T>::HydroDisplayAdafruitGFX(SPIDeviceSetup lcdSetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayOrientation),
    #ifndef ESP8266
          _gfx(lcdSetup.spi, dcPin, lcdSetup.cs, resetPin),
      #else
          _gfx(lcdSetup.cs, dcPin, resetPin),
      #endif
      _gfxDrawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, applicationInfo.name, &_gfxDrawable)
{
    _renderer.setTitleMode(BaseGraphicalRenderer::TITLE_ALWAYS);
}

template <class T>
void HydroDisplayAdafruitGFX<T>::init()
{
    HydroDisplayDriver::init(HYDRO_UI_UPDATE_SPEED, definedThemeElse(getDisplayTheme(), JOIN(JOIN(Hydro_DisplayTheme, HYDRO_UI_DISPLAYTHEME_GFX), SM)));
}

template <class T>
void HydroDisplayAdafruitGFX<T>::begin()
{
    _gfx.begin();
    _gfx.setRotation((uint8_t)_displayOri);
}

#endif
