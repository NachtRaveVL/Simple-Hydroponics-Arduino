/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino LCD Overview Screen
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroOverviewLCD_H
#define HydroOverviewLCD_H

#include "../HydruinoUI.h"

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

#endif // /ifndef HydroOverviewLCD_H
#endif
