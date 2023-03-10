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

    void allocateStandardControls();                                        // Allocates/pulls-into-build any standard tcMenu controls, from encoders to matrices
    void allocateResistiveControls();                                       // Allocates/pulls-into-build any resistive touchscreen controls, must be called after display allocation
    void allocateTouchscreenControl();                                      // Allocates/pulls-into-build any Touchscreen controls (using FT6206/XPT2046)
    void allocateTFTTouchControl();                                         // Allocates/pulls-into-build any TFTTouch controls (using TFTe_SPI), must be called after TFT display allocation

    void allocateLCDDisplay();                                              // Allocates/pulls-into-build any LCD displays (using LiquidCrystalIO)
    void allocateU8G2Display();                                             // Allocates/pulls-into-build any U8G2 OLED pixel displays (using U8G2)
    void allocateAdaGFXDisplay();                                           // Allocates/pulls-into-build any AdafruitGFX color pixel displays (using AdafruitGFX)
    void allocateTFTDisplay();                                              // Allocates/pulls-into-build any TFTe_SPI color pixel displays (using TFTe_SPI)

    void addSerialRemote(UARTDeviceSetup rcSetup = UARTDeviceSetup());      // Adds/pulls-into-build remote control by Serial or Bluetooth AT
    void addSimhubRemote(UARTDeviceSetup rcSetup = UARTDeviceSetup());      // Adds/pulls-into-build remote control by Simhub serial connector, requires UART setup
    void addWiFiRemote(uint16_t rcServerPort = HYDRO_UI_REMOTESERVER_PORT); // Adds/pulls-into-build remote control by WiFi, requires enabled WiFi
    void addEthernetRemote(uint16_t rcServerPort = HYDRO_UI_REMOTESERVER_PORT); // Adds/pulls-into-build remote control by Ethernet, requires enabled Ethernet

    virtual bool isFullUI() override;

protected:
};

#endif // /ifndef HydroUI_H
#endif
