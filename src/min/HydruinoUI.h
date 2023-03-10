/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Minimal/RO UI
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroUI_H
#define HydroUI_H

class HydruinoMinUI;
typedef HydruinoMinUI HydruinoUI;

#include "..\shared\HydruinoUI.h"

class HydruinoMinUI : public HydruinoBaseUI {
public:
    HydruinoMinUI(UIControlSetup uiControlSetup = UIControlSetup(),         // UI control input setup
                  UIDisplaySetup uiDisplaySetup = UIDisplaySetup(),         // UI display output setup 
                  bool isActiveLowIO = true,                                // Logic level usage for control & display IO pins
                  bool allowInterruptableIO = true,                         // Allows interruptable pins to interrupt, else forces polling
                  bool enableTcUnicodeFonts = true);                        // Enables tcUnicode UTF8 fonts usage instead of library fonts
    virtual ~HydruinoMinUI();

    virtual bool isFullUI() override;

protected:
};

#endif // /ifndef HydroUI_H
#endif
