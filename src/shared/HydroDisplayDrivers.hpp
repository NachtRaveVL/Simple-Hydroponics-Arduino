/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Display Drivers
*/

#include "Hydruino.h"
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"

template <class T>
HydroDisplayAdafruitGFX<T>::HydroDisplayAdafruitGFX(SPIDeviceSetup dispSetup, Hydro_DisplayOrientation displayOrientation, pintype_t dcPin, pintype_t resetPin)
    : HydroDisplayDriver(displayOrientation),
    #ifndef ESP8266
          _gfx(dispSetup.spi, dcPin, dispSetup.cs, resetPin),
      #else
          _gfx(dispSetup.cs, dcPin, resetPin),
      #endif
      _gfxDrawable(&_gfx, 0),
      _renderer(HYDRO_UI_RENDERER_BUFFERSIZE, applicationInfo.name, &_gfxDrawable)
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
