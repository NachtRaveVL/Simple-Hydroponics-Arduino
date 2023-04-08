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
    HydruinoFullUI(String deviceUUID,                                       // Device UUID hex string for remote controllability
                   UIControlSetup uiControlSetup = UIControlSetup(),        // UI control input setup, from controller initialization
                   UIDisplaySetup uiDisplaySetup = UIDisplaySetup(),        // UI display output setup, from controller initialization
                   bool isActiveLowIO = true,                               // Signaling logic level usage for I/O control/display devices
                   bool allowInterruptableIO = true,                        // Allows interruptable pins to interrupt, else forces polling
                   bool enableTcUnicodeFonts = false,                       // Enables tcUnicode fonts usage over GFXfont (Adafruit) fonts
                   bool enableBufferedVRAM = false);                        // Enables sprite-sized buffered video RAM for smooth animations
    virtual ~HydruinoFullUI();

    void addRemote(Hydro_RemoteControl rcType,                              // Type of remote control
                   UARTDeviceSetup rcSetup = UARTDeviceSetup(),             // Remote control serial setup (if serial based), else ignored
                   uint16_t rcServerPort = HYDRO_UI_REMOTESERVER_PORT);     // Remote control server listening port (if networking based), else ignored

    virtual bool isFullUI() override;

protected:
};

#endif // /ifndef HydroUI_H
#endif
