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

#include "../shared/HydruinoUI.h"

class HydruinoMinUI : public HydruinoBaseUI {
public:
    HydruinoMinUI(UIControlSetup uiControlSetup = UIControlSetup(),         // UI control input setup
                  UIDisplaySetup uiDisplaySetup = UIDisplaySetup(),         // UI display output setup 
                  bool isActiveLowIO = true,                                // Logic level usage for control & display IO pins
                  bool allowInterruptableIO = true,                         // Allows interruptable pins to interrupt, else forces polling
                  bool enableTcUnicodeFonts = true);                        // Enables tcUnicode UTF8 fonts usage instead of library fonts
    virtual ~HydruinoMinUI();

    void allocateStandardControls();                                        // Allocates/pulls-into-build any standard tcMenu controls, from encoders to matrices
    void allocateESP32TouchControl();                                       // Allocates/pulls-into-build an ESP32 Touch key control (only for ESP32)
    void allocateResistiveTouchControl();                                   // Allocates/pulls-into-build a resistive touchscreen control, must be called after display allocation
    void allocateTouchscreenControl();                                      // Allocates/pulls-into-build a Touchscreen control (using FT6206/XPT2046), must be called after display allocation
    void allocateTFTTouchControl();                                         // Allocates/pulls-into-build a TFTTouch control (using TFT_eSPI), must be called after TFT display allocation

    void allocateLCDDisplay();                                              // Allocates/pulls-into-build a LCD display (using LiquidCrystalIO)

    void allocateSSD1305Display();                                          // Allocates/pulls-into-build a SSD1305 OLED pixel display (using U8g2)
    void allocateSSD1305x32AdaDisplay();                                    // Allocates/pulls-into-build a SSD1305 (ADAFRUIT_128X32) OLED pixel display (using U8g2)
    void allocateSSD1305x64AdaDisplay();                                    // Allocates/pulls-into-build a SSD1305 (ADAFRUIT_128x64) OLED pixel display (using U8g2)
    void allocateSSD1306Display();                                          // Allocates/pulls-into-build a SSD1306 OLED pixel display (using U8g2)
    void allocateSH1106Display();                                           // Allocates/pulls-into-build a SH1106 OLED pixel display (using U8g2)
    void allocateCustomOLEDDisplay();                                       // Allocates/pulls-into-build a custom OLED pixel display as statically defined (using U8g2)
    void allocateSSD1607Display();                                          // Allocates/pulls-into-build a SSD1607 OLED pixel display (using U8g2)
    void allocateIL3820Display();                                           // Allocates/pulls-into-build an IL3820 OLED pixel display (using U8g2)
    void allocateIL3820V2Display();                                         // Allocates/pulls-into-build an IL3820 V2 OLED pixel display (using U8g2)

    void allocateST7735Display();                                           // Allocates/pulls-into-build a ST7735 color pixel display (using AdafruitGFX)
    void allocateST7789Display();                                           // Allocates/pulls-into-build a ST7789 color pixel display (using AdafruitGFX)
    void allocateILI9341Display();                                          // Allocates/pulls-into-build an ILI9341 pixel display (using AdafruitGFX)
    void allocateTFTDisplay();                                              // Allocates/pulls-into-build a TFT_eSPI color pixel display (using TFT_eSPI)

    void addSerialRemote(UARTDeviceSetup rcSetup = UARTDeviceSetup());      // Adds/pulls-into-build a remote control by Serial or Bluetooth AT
    void addSimhubRemote(UARTDeviceSetup rcSetup = UARTDeviceSetup());      // Adds/pulls-into-build a remote control by Simhub serial connector, requires UART setup
    void addWiFiRemote(uint16_t rcServerPort = HYDRO_UI_REMOTESERVER_PORT); // Adds/pulls-into-build a remote control by WiFi, requires enabled WiFi
    void addEthernetRemote(uint16_t rcServerPort = HYDRO_UI_REMOTESERVER_PORT); // Adds/pulls-into-build a remote control by Ethernet, requires enabled Ethernet

    virtual bool isFullUI() override;

protected:
};

#endif // /ifndef HydroUI_H
#endif
