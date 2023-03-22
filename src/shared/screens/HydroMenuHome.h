/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Home Menu Screen
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroMenuHome_H
#define HydroMenuHome_H

class HydroHomeMenu;
#ifdef HYDRO_DISABLE_BUILTIN_DATA
struct HydroHomeMenuInfo;
#endif
struct HydroHomeMenuItems;

#include "../HydruinoUI.h"

class HydroHomeMenu : public HydroMenu
{
public:
    HydroHomeMenu();
    virtual ~HydroHomeMenu();

    virtual void loadMenu(MenuItem *addFrom = nullptr) override;
    virtual MenuItem *getRootItem() override;

    inline HydroHomeMenuItems &getItems() { return *_items; }

protected:
    HydroHomeMenuItems *_items;
};

#ifdef HYDRO_DISABLE_BUILTIN_DATA
struct HydroHomeMenuInfo {
    HydroHomeMenuInfo();

    AnyMenuInfo minfoBackToOverview;
#ifdef HYDRO_UI_ENABLE_DEBUG_MENU
    BooleanMenuInfo minfoToggleBadConn;
    BooleanMenuInfo minfoToggleFastTime;
    AnyMenuInfo minfoTriggerSigTime;
    AnyMenuInfo minfoTriggerSDCleanup;
    AnyMenuInfo minfoTriggerLowMem;
    AnyMenuInfo minfoTriggerAutosave;
    SubMenuInfo minfoDebug;
#endif // /ifdef HYDRO_UI_ENABLE_DEBUG_MENU
    AnyMenuInfo minfoInformation;
    AnyMenuInfo minfoCalibrations;
    AnyMenuInfo minfoAdditives;
    AnyMenuInfo minfoCropsLib;
    SubMenuInfo minfoLibrary;
    AnyMenuInfo minfoSettings;
    AnyMenuInfo minfoScheduling;
    AnyMenuInfo minfoPowerRails;
    AnyMenuInfo minfoReservoirs;
    AnyMenuInfo minfoCrops;
    AnyMenuInfo minfoSensors;
    AnyMenuInfo minfoActuators;
    SubMenuInfo minfoSystem;
    AnyMenuInfo minfoAlerts;
};
#endif // /ifdef HYDRO_DISABLE_BUILTIN_DATA

struct HydroHomeMenuItems {
    HydroHomeMenuItems();

#ifdef HYDRO_DISABLE_BUILTIN_DATA
    HydroHomeMenuInfo init;
#endif

    ActionMenuItem menuBackToOverview;
#ifdef HYDRO_UI_ENABLE_DEBUG_MENU
    BooleanMenuItem menuToggleBadConn;
    BooleanMenuItem menuToggleFastTime;
    ActionMenuItem menuTriggerSigTime;
    ActionMenuItem menuTriggerSDCleanup;
    ActionMenuItem menuTriggerLowMem;
    ActionMenuItem menuTriggerAutosave;
    BackMenuItem menuBackDebug;
    SubMenuItem menuDebug;
#endif // /ifdef HYDRO_UI_ENABLE_DEBUG_MENU
    ActionMenuItem menuInformation;
    ActionMenuItem menuCalibrations;
    ActionMenuItem menuAdditives;
    ActionMenuItem menuCropsLib;
    BackMenuItem menuBackLibrary;
    SubMenuItem menuLibrary;
    ActionMenuItem menuSettings;
    ActionMenuItem menuScheduling;
    ActionMenuItem menuPowerRails;
    ActionMenuItem menuReservoirs;
    ActionMenuItem menuCrops;
    ActionMenuItem menuSensors;
    ActionMenuItem menuActuators;
    BackMenuItem menuBackSystem;
    SubMenuItem menuSystem;
    ActionMenuItem menuAlerts;
};

#endif // /ifndef HydroMenuHome_H
#endif
