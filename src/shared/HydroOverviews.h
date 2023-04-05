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
template<class T> class HydroOverviewGFX;
class HydroOverviewTFT;

#include "HydruinoUI.h"

// Overview Screen Base
// Overview screen class that manages the default at-a-glance system overview.
// Meant to be able to be deleted on a moments notice to transition back into menu.
class HydroOverview {
public:
    inline HydroOverview(HydroDisplayDriver *display) : _display(display), _needsFullRedraw(true) { ; }
    virtual ~HydroOverview() = default;

    // Renders overview screen given current display orientation.
    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) = 0;

    inline void setNeedsFullRedraw() { _needsFullRedraw = true; }

protected:
    HydroDisplayDriver *_display;                           // Display (strong)
    bool _needsFullRedraw;                                  // Needs full redraw flag
};

#include "screens/HydroOverviewGFX.h"
#include "screens/HydroOverviewLCD.h"
#include "screens/HydroOverviewOLED.h"
#include "screens/HydroOverviewTFT.h"

#endif // /ifndef HydroOverviews_H
#endif
