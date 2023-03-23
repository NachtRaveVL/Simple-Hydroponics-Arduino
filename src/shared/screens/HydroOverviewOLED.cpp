/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino U8g2 OLED Overview Screen
*/

#include "../HydruinoUI.h"
#ifdef HYDRO_USE_GUI

HydroOverviewOLED::HydroOverviewOLED(HydroDisplayU8g2OLED *display)
    : HydroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable())
{ ; }

HydroOverviewOLED::~HydroOverviewOLED()
{ ; }

void HydroOverviewOLED::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
