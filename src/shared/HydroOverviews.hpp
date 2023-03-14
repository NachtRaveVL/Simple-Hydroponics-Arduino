/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Overview Screens
*/

#include "Hydruino.h"
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"

template <class T>
HydroOverviewAdaGfx<T>::HydroOverviewAdaGfx(HydroDisplayAdafruitGFX<T> *display)
    : HydroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable())
{ ; }

template <class T>
HydroOverviewAdaGfx<T>::~HydroOverviewAdaGfx()
{ ; }

template <class T>
void HydroOverviewAdaGfx<T>::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
