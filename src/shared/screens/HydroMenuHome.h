/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Home Menu Screen
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroMenuHome_H
#define HydroMenuHome_H

class HydroHomeMenu;
struct HydroHomeMenuInfo;
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

struct HydroHomeMenuInfo {
    HydroHomeMenuInfo();

#ifdef HELIO_DISABLE_BUILTIN_DATA
    String _enumStrGPSPolling[4];
    String _enumStrTimeZone[41];
    String _enumStrDataPolling[8];
    String _enumStrAutosave[4];
    String _enumStrMeasurements[3];
    String _enumStrSystemMode[2];
#endif

    const char *enumStrGPSPolling[4];
    const char *enumStrTimeZone[41];
    const char *enumStrDataPolling[8];
    const char *enumStrAutosave[4];
    const char *enumStrMeasurements[3];
    const char *enumStrSystemMode[2];

    AnyMenuInfo minfoOverview;
    BooleanMenuInfo minfoToggleBadConn;
    BooleanMenuInfo minfoToggleFastTime;
    AnyMenuInfo minfoTriggerSigTime;
    AnyMenuInfo minfoTriggerSDCleanup;
    AnyMenuInfo minfoTriggerLowMem;
    AnyMenuInfo minfoTriggerAutosave;
    SubMenuInfo minfoDebug;
    BooleanMenuInfo minfoBatteryFailure;
    SubMenuInfo minfoRTC;
    SubMenuInfo minfoEEPROM;
    SubMenuInfo minfoSDCard;
    SubMenuInfo minfoSystemInfo;
    SubMenuInfo minfoInfo;
    AnyMenuInfo minfoAddNewCalibration;
    AnyMenuInfo minfoBrowseCalibrations;
    SubMenuInfo minfoCalibrations;
    AnyMenuInfo minfoAddNewAdditive;
    AnyMenuInfo minfoBrowseAdditives;
    SubMenuInfo minfoAdditives;
    AnyMenuInfo minfoAddNewCropLib;
    AnyMenuInfo minfoBrowseCropsLib;
    SubMenuInfo minfoCropsLib;
    SubMenuInfo minfoLibrary;
    SubMenuInfo minfoControls;
    EnumMenuInfo minfoGPSPolling;
    AnalogMenuInfo minfoAltitude;
    SubMenuInfo minfoLocation;
    BooleanMenuInfo minfoAllowRemoteCtrl;
    AnalogMenuInfo minfoRemoteCtrlPort;
    AnalogMenuInfo minfoBrokerPort;
    BooleanMenuInfo minfoAssignByHostname;
    BooleanMenuInfo minfoAssignByDHCP;
    SubMenuInfo minfoNetworking;
    BooleanMenuInfo minfoDSTAddHour;
    EnumMenuInfo minfoTimeZone;
    SubMenuInfo minfoTime;
    EnumMenuInfo minfoDataPolling;
    EnumMenuInfo minfoAutosaveSeconday;
    EnumMenuInfo minfoAutosavePrimary;
    EnumMenuInfo minfoMeasurements;
    EnumMenuInfo minfoSystemMode;
    SubMenuInfo minfoGeneral;
    SubMenuInfo minfoSettings;
    SubMenuInfo minfoScheduling;
    SubMenuInfo minfoPowerRails;
    SubMenuInfo minfoReservoirs;
    SubMenuInfo minfoCrops;
    SubMenuInfo minfoSensors;
    SubMenuInfo minfoActuators;
    SubMenuInfo minfoSystem;
};

struct HydroHomeMenuItems {
    HydroHomeMenuItems();

    HydroHomeMenuInfo init;

    ActionMenuItem menuOverview;
    BooleanMenuItem menuToggleBadConn;
    BooleanMenuItem menuToggleFastTime;
    ActionMenuItem menuTriggerSigTime;
    ActionMenuItem menuTriggerSDCleanup;
    ActionMenuItem menuTriggerLowMem;
    ActionMenuItem menuTriggerAutosave;
    BackMenuItem menuBackDebug;
    SubMenuItem menuDebug;
    RemoteMenuItem menuIoTMonitor;
    BooleanMenuItem menuBatteryFailure;
    BackMenuItem menuBackRTC;
    SubMenuItem menuRTC;
    TextMenuItem menuEEPROMSize;
    BackMenuItem menuBackEEPROM;
    SubMenuItem menuEEPROM;
    TextMenuItem menuSDName;
    BackMenuItem menuBackSDCard;
    SubMenuItem menuSDCard;
    TextMenuItem menuDisplayMode;
    TextMenuItem menuControlMode;
    TextMenuItem menuFreeMemory;
    TextMenuItem menuUptime;
    TextMenuItem menuFirmware;
    TextMenuItem menuBoard;
    BackMenuItem menuBackSystemInfo;
    SubMenuItem menuSystemInfo;
    BackMenuItem menuBackInfo;
    SubMenuItem menuInfo;
    ActionMenuItem minfoAddNewCalibration;
    ActionMenuItem menuBrowseCalibrations;
    BackMenuItem menuBackCalibrations;
    SubMenuItem menuCalibrations;
    ActionMenuItem menuAddNewAdditives;
    ActionMenuItem menuBrowseAdditives;
    BackMenuItem menuBackAdditives;
    SubMenuItem menuAdditives;
    ActionMenuItem menuAddNewCropsLib;
    ActionMenuItem menuBrowseCropsLib;
    BackMenuItem menuBackCropsLib;
    SubMenuItem menuCropsLib;
    BackMenuItem menuBackLibrary;
    SubMenuItem menuLibrary;
    EepromAuthenticationInfoMenuItem menuAuthenticator;
    EditableLargeNumberMenuItem menuJoystickYTol;
    EditableLargeNumberMenuItem menuJoystickYMid;
    EditableLargeNumberMenuItem menuJoystickXTol;
    EditableLargeNumberMenuItem menuJoystickXMid;
    BackMenuItem menuBackControls;
    SubMenuItem menuControls;
    EnumMenuItem menuGPSPolling;
    AnalogMenuItem menuAltitude;
    EditableLargeNumberMenuItem menuLongitudeMin;
    EditableLargeNumberMenuItem menuLatitudeDeg;
    BackMenuItem menuBackLocation;
    SubMenuItem menuLocation;
    BooleanMenuItem menuAllowRemoteCtrl;
    AnalogMenuItem menuRemoteCtrlPort;
    AnalogMenuItem menuBrokerPort;
    BooleanMenuItem menuAssignByHostname;
    IpAddressMenuItem menuMQTTBroker;
    TextMenuItem menuWiFiPassword;
    TextMenuItem menuWiFiSSID;
    TextMenuItem menuMACAddress;
    BooleanMenuItem menuAssignByDHCP;
    IpAddressMenuItem menuControllerIP;
    BackMenuItem menuBackNetworking;
    SubMenuItem menuNetworking;
    BooleanMenuItem menuDSTAddHour;
    EnumMenuItem menuTimeZone;
    TimeFormattedMenuItem menuLocalTime;
    DateFormattedMenuItem menuDate;
    BackMenuItem menuBackTime;
    SubMenuItem menuTime;
    EnumMenuItem menuDataPolling;
    EnumMenuItem menuAutosaveSeconday;
    EnumMenuItem menuAutosavePrimary;
    EnumMenuItem menuMeasurements;
    EnumMenuItem menuSystemMode;
    TextMenuItem menuSystemName;
    BackMenuItem menuBackGeneral;
    SubMenuItem menuGeneral;
    BackMenuItem menuBackSettings;
    SubMenuItem menuSettings;
    BackMenuItem menuBackScheduling;
    SubMenuItem menuScheduling;
    BackMenuItem menuBackPowerRails;
    SubMenuItem menuPowerRails;
    BackMenuItem menuBackReservoirs;
    SubMenuItem menuReservoirs;
    BackMenuItem menuBackCrops;
    SubMenuItem menuCrops;
    BackMenuItem menuBackSensors;
    SubMenuItem menuSensors;
    BackMenuItem menuBackActuators;
    SubMenuItem menuActuators;
    BackMenuItem menuBackSystem;
    SubMenuItem menuSystem;
};

#endif // /ifndef HydroMenuHome_H
#endif
