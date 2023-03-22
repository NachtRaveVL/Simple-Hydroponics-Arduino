/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Full/RW UI
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroUI_H
#define HydroUI_H

class HydruinoFullUI;
typedef HydruinoFullUI HydruinoUI;

#include "../shared/HydruinoUI.h"

class HydruinoFullUI : public HydruinoBaseUI {
public:
    HydruinoFullUI(UIControlSetup uiControlSetup = UIControlSetup(),        // UI control input setup
                   UIDisplaySetup uiDisplaySetup = UIDisplaySetup(),        // UI display output setup 
                   bool isActiveLowIO = true,                               // Logic level usage for control & display IO pins
                   bool allowInterruptableIO = true,                        // Allows interruptable pins to interrupt, else forces polling
                   bool enableTcUnicodeFonts = true);                       // Enables tcUnicode UTF8 fonts usage instead of library fonts
    virtual ~HydruinoFullUI();

    void addRemote(Hydro_RemoteControl rcType,                              // Type of remote control
                   UARTDeviceSetup rcSetup = UARTDeviceSetup(),             // Remote control serial setup (if serial based), else ignored
                   uint16_t rcServerPort = HYDRO_UI_REMOTESERVER_PORT);     // Remote control server listening port (if networking based), else ignored

    virtual bool isFullUI() override;

protected:
};

#endif // /ifndef HydroUI_H
#endif
