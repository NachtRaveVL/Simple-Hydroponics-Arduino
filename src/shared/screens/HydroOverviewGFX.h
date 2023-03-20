/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino AdafruitGFX Overview Screen
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroOverviewGFX_H
#define HydroOverviewGFX_H

#include "../HydruinoUI.h"

// AdafruitGFX Overview Screen
// Overview screen built for AdafruitGFX displays.
template<class T>
class HydroOverviewGFX : public HydroOverview {
public:
    HydroOverviewGFX(HydroDisplayAdafruitGFX<T> *display);
    virtual ~HydroOverviewGFX();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    T &_gfx;                                                // Graphics (strong)
    AdafruitDrawable<T> &_drawable;                         // Drawable (strong)
};

#endif // /ifndef HydroOverviewGFX_H
#endif
