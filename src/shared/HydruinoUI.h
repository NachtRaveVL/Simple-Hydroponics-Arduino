/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Base UI
*/

// Library UI Setup

// NOTE: It is recommended to use custom build flags instead of editing this file directly.

// Uncomment or -D this define to enable usage of the XPT2046_Touchscreen library, in place of the Adafruit FT6206 library.
//#define HYDRO_UI_ENABLE_XPT2046TS               // https://github.com/PaulStoffregen/XPT2046_Touchscreen

// Uncomment or -D this define to enable usage of the StChromaArt LDTC framebuffer capable canvas in place of default U8g2Drawable canvas (STM32/mbed only, note: requires advanced setup)
//#define HYDRO_UI_ENABLE_STCHROMA_LDTC

// Uncomment or -D this define to enable usage of the StChromaArt BSP touch screen interrogator in place of the default AdaLibTouchInterrogator (STM32/mbed only, note: requires advanced setup, see tcMenu_Extra_BspUserSettings.h)
//#define HYDRO_UI_ENABLE_BSP_TOUCH

// Uncomment or -D this define to enable usage of the debug menu 
//#define HYDRO_UI_ENABLE_DEBUG_MENU


#ifdef NDEBUG
#ifdef HYDRO_UI_ENABLE_DEBUG_MENU
#undef HYDRO_UI_ENABLE_DEBUG_MENU
#endif
#endif // /ifdef NDEBUG

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroBaseUI_H
#define HydroBaseUI_H

#include "HydroUIDefines.h"
#include "HydroUIInlines.hh"
#include "HydroUIData.h"
#include "HydroUIStrings.h"

// tcMenu Plugin Adaptations
#include "tcMenu_Display_AdaFruitGfx.h"
#include "tcMenu_Display_LiquidCrystal.h"
#include "tcMenu_Display_TfteSpi.h"
#include "tcMenu_Display_U8g2.h"
#include "tcMenu_Extra_StChromaArt.h"
#include "tcMenu_Input_AdaTouchDriver.h"
#include "tcMenu_Input_ESP32TouchKeysAbstraction.h"
#include "tcMenu_Remote_EthernetTransport.h"
#include "tcMenu_Remote_SerialTransport.h"
#include "tcMenu_Remote_SimhubConnector.h"
#include "tcMenu_Remote_WiFiTransport.h"
#include "tcMenu_Theme_CoolBlueModern.h"
#include "tcMenu_Theme_CoolBlueTraditional.h"
#include "tcMenu_Theme_DarkModeModern.h"
#include "tcMenu_Theme_DarkModeTraditional.h"
#include "tcMenu_Theme_MonoBordered.h"
#include "tcMenu_Theme_MonoInverse.h"

#include "HydroDisplayDrivers.h"
#include "HydroInputDrivers.h"
#include "HydroRemoteControls.h"
#include "HydroMenus.h"
#include "HydroOverviews.h"

// Base UI
// The base class that manages interaction with the tcMenu UI system. Font setup should
// precede initialization. Overview & menu screens not guaranteed to be allocated.
class HydruinoBaseUI : public HydroUIInterface, public CustomDrawing {
public:
    // Base UI constructor.
    HydruinoBaseUI(String deviceUUID,                                       // Device UUID hex string for remote controllability
                   UIControlSetup uiControlSetup = UIControlSetup(),        // UI control input setup, from controller initialization
                   UIDisplaySetup uiDisplaySetup = UIDisplaySetup(),        // UI display output setup, from controller initialization
                   bool isActiveLowIO = true,                               // Signaling logic level usage for I/O control/display devices
                   bool allowInterruptableIO = true,                        // Allows interruptable pins to interrupt, else forces polling
                   bool enableTcUnicodeFonts = false,                       // Enables tcUnicode fonts usage over GFXfont (Adafruit) fonts
                   bool enableBufferedVRAM = false);                        // Enables sprite-sized buffered video RAM for smooth animations
    virtual ~HydruinoBaseUI();

    // Sets up optional overview clock font (expected setup before init), else uses internal default
    inline void setupOverviewClockFont(const void *clockFont) { _clockFont = clockFont; }
    // Sets up optional overview detail font (expected setup before init), else uses internal default
    inline void setupOverviewDetailFont(const void *detailFont) { _detailFont = detailFont; }
    // Sets up both optional overview clock and detail font (expected setup before init), else uses internal default
    inline void setupOverviewFont(const void *overviewFont) { _clockFont = overviewFont; _detailFont = overviewFont; }
    // Sets up optional menu item font (expected setup before init), else uses internal default
    inline void setupMenuItemFont(const void *itemFont) { _itemFont = itemFont; }
    // Sets up optional menu title font (expected setup before init), else uses internal default
    inline void setupMenuTitleFont(const void *titleFont) { _titleFont = titleFont; }
    // Sets up both optional menu item and title font (expected setup before init), else uses internal default
    inline void setupMenuFont(const void *menuFont) { _itemFont = menuFont; _titleFont = menuFont; }

    // Initializes UI from passed UIData object, returning new UIData object or original
    // as backing model object to export upon save. Attempts to use the display driver's
    // default init mode settings if given a null data object. Otherwise will initialize
    // based on remote control defaults. Designated/recommended initializer.
    virtual HydroUIData *init(HydroUIData *uiData = nullptr) override;      // UIData instance

    // Initializes UI from passed parameters. Typically called by designated initializer,
    // but also able to be called directly for custom defaults.
    void init(uint8_t updatesPerSec,                                        // Updates per second (1 to 10)
              Hydro_DisplayTheme displayTheme,                              // Display theme to apply
              Hydro_TitleMode titleMode,                                    // Title mode
              bool analogSlider = false,                                    // Slider usage for analog items
              bool editingIcons = false);                                   // Editing icons usage

    // Begins the display and input drivers, bringing their devices online. 
    virtual void begin() override;

    // Sets redraw needed flag for full UI screen redraw.
    virtual void setNeedsRedraw() override;

    // Determines the ISR mode to use for switches/keys, based on allowed ISR setting
    // and control input pins specified. If input controller does not allow main pins
    // to be interruptable then it will not check for all pins being interruptable.
    SwitchInterruptMode getISRMode() const;

    // Determines the number of buffered video RAM rows to use in SRAM, based on enabled
    // buffering setting and graphics device (else 0 if disabled). To reduce the flicker
    // associated with common non-buffered drawing code, the number of rows of buffered
    // VRAM is set to the largest pixel height of sprite that will need to be rendered,
    // and is supplied to tcMenu's Drawable-derived classes (used in screen rendering).
    // Note: VRAM buffering can be memory intensive - only use if SRAM is plentiful.
    inline int getVRAMBufferRows() const { return _isBufferedVRAM && (_display && _display->getGraphicsRenderer()) ? HYDRO_UI_SPRITE_MAXYSIZE : 0; }

    // Determines if running in full-UI mode (true) or not (false).
    virtual bool isFullUI() = 0;
    // Determines if running in minimal-UI mode (true) or not (false).
    inline bool isMinUI() { return !isFullUI(); }

    // App info accessor
    inline const ConnectorLocalInfo &getApplicationInfo() const { return _appInfo; }
    // Control setup accessor
    inline const UIControlSetup &getControlSetup() const { return _uiCtrlSetup; }
    // Display setup accessor
    inline const UIDisplaySetup &getDisplaySetup() const { return _uiDispSetup; }
    // Active low signaling logic accessor
    inline bool isActiveLow() const { return _isActiveLow; }
    // Allowing interruptable I/O accessor
    inline bool allowingISR() const { return _allowISR; }
    // Using tcUnicode fonts over GFXfont accessor
    inline bool isTcUnicodeFonts() const { return _isTcUnicodeFonts; }
    // Using buffered VRAM for non-flickering sprite animations accessor
    inline bool isBufferedVRAM() const { return _isBufferedVRAM; }

    // UIData accessor
    inline HydroUIData *getUIData() { return _uiData; }
    // Input driver accessor
    inline HydroInputDriver *getInput() { return _input; }
    // Display driver accessor
    inline HydroDisplayDriver *getDisplay() { return _display; }
    // Remote server accessor
    inline TcMenuRemoteServer *getRemoteServer() { return _remoteServer; }
    // Overview screen accessor
    inline HydroOverview *getOverview() { return _overview; }
    // Home menu accessor
    inline HydroHomeMenu *getHomeMenu() { return _homeMenu; }

protected:
    ConnectorLocalInfo _appInfo;                            // Application info for remote connections
    const UIControlSetup _uiCtrlSetup;                      // Control setup, from controller initialization
    const UIDisplaySetup _uiDispSetup;                      // Display setup, from controller initialization
    const bool _isActiveLow;                                // I/O pins use of active-low signaling logic (where LOW -> active)
    const bool _allowISR;                                   // Perform ISR checks for eligibility (for faster I/O response timings)
    const bool _isTcUnicodeFonts;                           // Using tcUnicode library fonts over GFXfont (Adafruit) fonts
    const bool _isBufferedVRAM;                             // Using sprite-sized video RAM buffering (high SRAM usage for smooth anims)

    HydroUIData *_uiData;                                   // Hydro UI data (strong)
    HydroInputDriver *_input;                               // Input driver (owned)
    HydroDisplayDriver *_display;                           // Display driver (owned)
    TcMenuRemoteServer *_remoteServer;                      // Remote control server (owned)
    Vector<HydroRemoteControl *, HYDRO_UI_REMOTECONTROLS_MAXSIZE> _remotes; // Remote controls list (owned)
    HydroPin *_backlight;                                   // Backlight control (owned)
    time_t _blTimeout;                                      // Backlight timeout (UTC)
    HydroOverview *_overview;                               // Overview screen (owned)
    HydroHomeMenu *_homeMenu;                               // Home menu screen (owned)
    const void *_clockFont;                                 // Overview clock font (strong), when gfx display
    const void *_detailFont;                                // Overview detail font (strong), when gfx display
    const void *_itemFont;                                  // Menu item font (strong), when gfx display
    const void *_titleFont;                                 // Menu title font (strong), when gfx display

    void setBacklightEnable(bool enabled);

public: // consider protected
    virtual void started(BaseMenuRenderer* currentRenderer) override;
    virtual void reset() override;
    virtual void renderLoop(unsigned int currentValue, RenderPressMode userClick) override;
};

#include "tcMenu_Display_AdaFruitGfx.hpp"
#include "HydroDisplayDrivers.hpp"
#include "HydroOverviews.hpp"

#endif // /ifndef HydroBaseUI_H
#endif
