/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino AdafruitGFX Overview Screen
*/

#include "../HydruinoUI.h"
#ifdef HYDRO_USE_GUI

template <class T>
HydroOverviewGFX<T>::HydroOverviewGFX(HydroDisplayAdafruitGFX<T> *display, const void *clockFont, const void *detailFont)
    : HydroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable()), _clockFont(clockFont), _detailFont(detailFont)
{ ; }

template <class T>
HydroOverviewGFX<T>::~HydroOverviewGFX()
{ ; }

template <class T>
void HydroOverviewGFX<T>::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
