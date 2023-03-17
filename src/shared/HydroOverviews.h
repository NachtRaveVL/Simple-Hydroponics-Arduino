/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Overview Screens
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroOverviews_H
#define HydroOverviews_H

class HydroOverview;
class HydroOverviewLCD;
class HydroOverviewOLED;
template<class T> class HydroOverviewAdaGfx;
class HydroOverviewTFT;

#include "HydruinoUI.h"

// Overview Screen Base
// Overview screen class that manages the default at-a-glance system overview.
// Meant to be able to be deleted on a moments notice to transition back into menu.
class HydroOverview {
public:
    HydroOverview(HydroDisplayDriver *display);
    virtual ~HydroOverview();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) = 0;

    inline void setNeedsFullRedraw() { _needsFullRedraw = true; }

protected:
    HydroDisplayDriver *_display;                           // Display (strong)
    bool _needsFullRedraw;                                  // Needs full redraw flag
};


// LCD Overview Screen
// Overview screen built for LCD displays.
class HydroOverviewLCD : public HydroOverview {
public:
    HydroOverviewLCD(HydroDisplayLiquidCrystal *display);
    virtual ~HydroOverviewLCD();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    LiquidCrystal &_lcd;                                    // LCD (strong)
};

// OLED Overview Screen
// Overview screen built for OLED displays.
class HydroOverviewOLED : public HydroOverview {
public:
    HydroOverviewOLED(HydroDisplayU8g2OLED *display);
    virtual ~HydroOverviewOLED();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    U8G2 &_gfx;                                             // Graphics (strong)
    U8g2Drawable &_drawable;                                // Drawable (strong)
};

// AdafruitGFX Overview Screen
// Overview screen built for AdafruitGFX displays.
template<class T>
class HydroOverviewAdaGfx : public HydroOverview {
public:
    HydroOverviewAdaGfx(HydroDisplayAdafruitGFX<T> *display);
    virtual ~HydroOverviewAdaGfx();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    T &_gfx;                                                // Graphics (strong)
    AdafruitDrawable<T> &_drawable;                         // Drawable (strong)

    DateTime _lastTime;                                     // Last time (local)

    void drawBackground(Coord pt, Coord sz, Pair<uint16_t, uint16_t> &screenSize);
};

// TFTe_SPI Overview Screen
// Overview screen built for TFTe_SPI displays.
class HydroOverviewTFT : public HydroOverview {
public:
    HydroOverviewTFT(HydroDisplayTFTeSPI *display);
    virtual ~HydroOverviewTFT();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    TFT_eSPI &_gfx;                                         // Graphics (strong)
    TfteSpiDrawable &_drawable;                             // Drawable (strong)
};

#endif // /ifndef HydroOverviews_H
#endif
