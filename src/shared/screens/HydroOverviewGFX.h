/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino AdafruitGFX Overview Screen
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroOverviewGFX_H
#define HydroOverviewGFX_H

template<class T> class HydroOverviewGFX;

#include "../HydruinoUI.h"

// AdafruitGFX Overview Screen
// Overview screen built for AdafruitGFX displays.
template<class T>
class HydroOverviewGFX : public HydroOverview {
public:
    HydroOverviewGFX(HydroDisplayAdafruitGFX<T> *display, const void *clockFont = nullptr, const void *detailFont = nullptr);
    virtual ~HydroOverviewGFX();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    T &_gfx;                                                // Graphics (strong)
    AdafruitDrawable<T> &_drawable;                         // Drawable (strong)
    const void *_clockFont;                                 // Overview clock font (strong)
    const void *_detailFont;                                // Overview detail font (strong)

    uint8_t _skyBlue, _skyRed;                              // Sky color
    Map<uint16_t,Pair<uint16_t,uint16_t>,HYDRO_UI_STARFIELD_MAXSIZE> _stars; // Starfield
    int _timeMag, _dateMag;                                 // Time/date mag level
    DateTime _lastTime;                                     // Last time (local)
    uint16_t _timeHeight, _dateHeight;                      // Pixel height

    void drawBackground(Coord pt, Coord sz, Pair<uint16_t, uint16_t> &screenSize);
};

#endif // /ifndef HydroOverviewGFX_H
#endif
