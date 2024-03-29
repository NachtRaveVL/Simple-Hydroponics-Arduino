/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino U8g2 OLED Overview Screen
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroOverviewOLED_H
#define HydroOverviewOLED_H

class HydroOverviewOLED;

#include "../HydruinoUI.h"

// OLED Overview Screen
// Overview screen built for OLED displays.
class HydroOverviewOLED : public HydroOverview {
public:
    HydroOverviewOLED(HydroDisplayU8g2OLED *display, const void *clockFont, const void *detailFont);
    virtual ~HydroOverviewOLED();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    U8G2 &_gfx;                                             // Graphics (strong)
    #ifdef HYDRO_UI_ENABLE_STCHROMA_LDTC
        StChromaArtDrawable &_drawable;                     // Drawable (strong)
    #else
        U8g2Drawable &_drawable;                            // Drawable (strong)
    #endif
    const void *_clockFont;                                 // Overview clock font (strong)
    const void *_detailFont;                                // Overview detail font (strong)
};

#endif // /ifndef HydroOverviewOLED_H
#endif
