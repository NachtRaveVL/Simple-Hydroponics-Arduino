/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino LCD Overview Screen
*/

#include "../HydruinoUI.h"
#ifdef HYDRO_USE_GUI

HydroOverviewLCD::HydroOverviewLCD(HydroDisplayLiquidCrystal *display)
    : HydroOverview(display), _lcd(display->getLCD())
{ ; }

HydroOverviewLCD::~HydroOverviewLCD()
{ ; }

void HydroOverviewLCD::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
