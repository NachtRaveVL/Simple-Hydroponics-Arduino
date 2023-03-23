/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino TFT_eSPI Overview Screen
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroOverviewTFT_H
#define HydroOverviewTFT_H

class HydroOverviewTFT;

#include "../HydruinoUI.h"

// TFT_eSPI Overview Screen
// Overview screen built for TFT_eSPI displays.
class HydroOverviewTFT : public HydroOverview {
public:
    HydroOverviewTFT(HydroDisplayTFTeSPI *display);
    virtual ~HydroOverviewTFT();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    TFT_eSPI &_gfx;                                         // Graphics (strong)
    TfteSpiDrawable &_drawable;                             // Drawable (strong)
};

#endif // /ifndef HydroOverviewTFT_H
#endif
