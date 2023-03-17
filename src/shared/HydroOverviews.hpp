/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Overview Screens
*/

#include "Hydruino.h"
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"

template <class T>
HydroOverviewAdaGfx<T>::HydroOverviewAdaGfx(HydroDisplayAdafruitGFX<T> *display)
    : HydroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable()),
      _lastTime((uint32_t)0)
{ ; }

template <class T>
HydroOverviewAdaGfx<T>::~HydroOverviewAdaGfx()
{ ; }

template <class T>
void HydroOverviewAdaGfx<T>::drawBackground(Coord pt, Coord sz, Pair<uint16_t, uint16_t> &screenSize)
{
    uint16_t &width(screenSize.first); uint16_t &height(screenSize.second);
    pt.x = constrain(pt.x, 0, width);
    sz.x = constrain(sz.x, 0, width - pt.x);
    pt.y = constrain(pt.y, 0, height);
    sz.y = constrain(sz.y, 0, height - pt.y);

    _gfx.startWrite();
    int maxY = pt.y + sz.y;
    for (int y = pt.y; y < maxY; ++y) {
        uint16_t skyColor = _gfx.color565(0, constrain(y - (height - 255), 20, 255), constrain(y - (height - 255), 20, 255));
        _gfx.writeFillRectPreclipped(pt.x, y, sz.x, 1, skyColor);
    }
    _gfx.endWrite();
}

template <class T>
void HydroOverviewAdaGfx<T>::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
