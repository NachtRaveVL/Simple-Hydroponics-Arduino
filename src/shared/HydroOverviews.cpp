/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Overview Screens
*/

#include "Hydruino.h"
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"

HydroOverview::HydroOverview(HydroDisplayDriver *display)
    : _display(display), _needsFullRedraw(true)
{ ; }

HydroOverview::~HydroOverview()
{ ; }


HydroOverviewLCD::HydroOverviewLCD(HydroDisplayLiquidCrystal *display)
    : HydroOverview(display), _lcd(display->getLCD())
{ ; }

HydroOverviewLCD::~HydroOverviewLCD()
{ ; }

void HydroOverviewLCD::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}


HydroOverviewOLED::HydroOverviewOLED(HydroDisplayU8g2OLED *display)
    : HydroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable())
{ ; }

HydroOverviewOLED::~HydroOverviewOLED()
{ ; }

void HydroOverviewOLED::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}


HydroOverviewTFT::HydroOverviewTFT(HydroDisplayTFTeSPI *display)
    : HydroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable())
{ ; }

HydroOverviewTFT::~HydroOverviewTFT()
{ ; }

void HydroOverviewTFT::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}


#endif
