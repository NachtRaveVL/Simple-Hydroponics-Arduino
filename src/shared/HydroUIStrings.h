/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino UI Strings/Prototypes
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroUIStrings_H
#define HydroUIStrings_H

// UI Strings Enumeration Table
enum HydroUI_String : unsigned short {
    HUIStr_Keys_MatrixActions,

    HUIStr_Item_Actuators,
    HUIStr_Item_AddNew,
    HUIStr_Item_Additives,
    HUIStr_Item_Alerts,
    HUIStr_Item_AllowRemoteCtrl,
    HUIStr_Item_Altitude,
    HUIStr_Item_AssignByDHCP,
    HUIStr_Item_AssignByHostname,
    HUIStr_Item_AutosavePrimary,
    HUIStr_Item_AutosaveSecondary,
    HUIStr_Item_BackToOverview,
    HUIStr_Item_BatteryFailure,
    HUIStr_Item_Board,
    HUIStr_Item_BrokerPort,
    HUIStr_Item_Browse,
    HUIStr_Item_Calibrations,
    HUIStr_Item_ControlMode,
    HUIStr_Item_ControllerIP,
    HUIStr_Item_Controls,
    HUIStr_Item_Crops,
    HUIStr_Item_CropsLib,
    HUIStr_Item_DSTAddHour,
    HUIStr_Item_DataPolling,
    HUIStr_Item_Date,
    HUIStr_Item_Debug,
    HUIStr_Item_DisplayMode,
    HUIStr_Item_EEPROM,
    HUIStr_Item_Firmware,
    HUIStr_Item_FreeMemory,
    HUIStr_Item_GPSPolling,
    HUIStr_Item_General,
    HUIStr_Item_Information,
    HUIStr_Item_JoystickXMid,
    HUIStr_Item_JoystickXTol,
    HUIStr_Item_JoystickYMid,
    HUIStr_Item_JoystickYTol,
    HUIStr_Item_LatDegrees,
    HUIStr_Item_Library,
    HUIStr_Item_LocalTime,
    HUIStr_Item_Location,
    HUIStr_Item_LongMinutes,
    HUIStr_Item_MACAddr0x,
    HUIStr_Item_MQTTBroker,
    HUIStr_Item_Measurements,
    HUIStr_Item_Name,
    HUIStr_Item_Networking,
    HUIStr_Item_PowerRails,
    HUIStr_Item_RTC,
    HUIStr_Item_RemoteCtrlPort,
    HUIStr_Item_Reservoirs,
    HUIStr_Item_Scheduling,
    HUIStr_Item_SDCard,
    HUIStr_Item_SimhubConnected,
    HUIStr_Item_Sensors,
    HUIStr_Item_Settings,
    HUIStr_Item_Size,
    HUIStr_Item_System,
    HUIStr_Item_SystemMode,
    HUIStr_Item_SystemName,
    HUIStr_Item_Time,
    HUIStr_Item_TimeZone,
    HUIStr_Item_ToggleBadConn,
    HUIStr_Item_ToggleFastTime,
    HUIStr_Item_TriggerAutosave,
    HUIStr_Item_TriggerLowMem,
    HUIStr_Item_TriggerSDCleanup,
    HUIStr_Item_TriggerSigTime,
    HUIStr_Item_TriggerSigLocation,
    HUIStr_Item_Uptime,
    HUIStr_Item_WiFiPass,
    HUIStr_Item_WiFiSSID,

    HUIStr_Enum_Autosave,
    HUIStr_Enum_DataPolling,
    HUIStr_Enum_GPSPolling,
    HUIStr_Enum_Measurements,
    HUIStr_Enum_SystemMode,
    HUIStr_Enum_TimeZone,

    HUIStr_Unit_MSL,
    HUIStr_Unit_Percent,

    HUIStr_Count
};

// Returns memory resident string from PROGMEM (Flash) UI string enumeration.
extern String stringFromPGM(HydroUI_String strNum);

// Makes UI Strings lookup go through EEPROM, with specified data begin address.
extern void beginUIStringsFromEEPROM(uint16_t uiDataAddress);

// Makes UI Strings lookup go through SD card strings file at file prefix.
extern void beginUIStringsFromSDCard(String uiDataFilePrefix);

#ifndef HYDRO_DISABLE_BUILTIN_DATA
// Returns PROGMEM (Flash) address pointer given UI string number.
const char *pgmAddrForStr(HydroUI_String strNum);
#endif

// Returns the pitch byte size for entries in a compressed enum list by parsing the initial item.
extern size_t enumListPitch(const char *enumData);
inline size_t enumListPitch(HydroUI_String strNum) { return enumListPitch(CFP(strNum)); }

#endif // /ifndef HydroUIStrings_H
#endif
