/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino TFT_eSPI Overview Screen
*/

#include "../HydruinoUI.h"
#ifdef HYDRO_USE_GUI

extern float skyEaseInOut(float x);
extern void randomStarColor(uint8_t* r, uint8_t* g, uint8_t* b);

HydroOverviewTFT::HydroOverviewTFT(HydroDisplayTFTeSPI *display, const void *clockFont, const void *detailFont)
    : HydroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable()), _clockFont(clockFont), _detailFont(detailFont)
{ ; }

HydroOverviewTFT::~HydroOverviewTFT()
{ ; }

void HydroOverviewTFT::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
