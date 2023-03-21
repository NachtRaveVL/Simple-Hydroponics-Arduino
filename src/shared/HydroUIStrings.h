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
    HUIStr_Keys_Matrix2x2Keys,
    HUIStr_Keys_Matrix3x4Keys,
    HUIStr_Keys_Matrix4x4Keys,

    HUIStr_Item_Actuators,
    HUIStr_Item_AddNew,
    HUIStr_Item_Additives,
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
    HUIStr_Item_Info,
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
    HUIStr_Item_SDCard,
    HUIStr_Item_Scheduling,
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
    HUIStr_Item_Uptime,
    HUIStr_Item_WiFiPass,
    HUIStr_Item_WiFiSSID,

    HUIStr_Enum_Autosave_0,
    HUIStr_Enum_Autosave_1,
    HUIStr_Enum_Autosave_2,
    HUIStr_Enum_Autosave_3,
    HUIStr_Enum_DataPolling_0,
    HUIStr_Enum_DataPolling_1,
    HUIStr_Enum_DataPolling_2,
    HUIStr_Enum_DataPolling_3,
    HUIStr_Enum_DataPolling_4,
    HUIStr_Enum_DataPolling_5,
    HUIStr_Enum_DataPolling_6,
    HUIStr_Enum_DataPolling_7,
    HUIStr_Enum_GPSPolling_0,
    HUIStr_Enum_GPSPolling_1,
    HUIStr_Enum_GPSPolling_2,
    HUIStr_Enum_GPSPolling_3,
    HUIStr_Enum_Measurements_0,
    HUIStr_Enum_Measurements_1,
    HUIStr_Enum_Measurements_2,
    HUIStr_Enum_SystemMode_0,
    HUIStr_Enum_SystemMode_1,
    HUIStr_Enum_TimeZone_0,
    HUIStr_Enum_TimeZone_1,
    HUIStr_Enum_TimeZone_2,
    HUIStr_Enum_TimeZone_3,
    HUIStr_Enum_TimeZone_4,
    HUIStr_Enum_TimeZone_5,
    HUIStr_Enum_TimeZone_6,
    HUIStr_Enum_TimeZone_7,
    HUIStr_Enum_TimeZone_8,
    HUIStr_Enum_TimeZone_9,
    HUIStr_Enum_TimeZone_10,
    HUIStr_Enum_TimeZone_11,
    HUIStr_Enum_TimeZone_12,
    HUIStr_Enum_TimeZone_13,
    HUIStr_Enum_TimeZone_14,
    HUIStr_Enum_TimeZone_15,
    HUIStr_Enum_TimeZone_16,
    HUIStr_Enum_TimeZone_17,
    HUIStr_Enum_TimeZone_18,
    HUIStr_Enum_TimeZone_19,
    HUIStr_Enum_TimeZone_20,
    HUIStr_Enum_TimeZone_21,
    HUIStr_Enum_TimeZone_22,
    HUIStr_Enum_TimeZone_23,
    HUIStr_Enum_TimeZone_24,
    HUIStr_Enum_TimeZone_25,
    HUIStr_Enum_TimeZone_26,
    HUIStr_Enum_TimeZone_27,
    HUIStr_Enum_TimeZone_28,
    HUIStr_Enum_TimeZone_29,
    HUIStr_Enum_TimeZone_30,
    HUIStr_Enum_TimeZone_31,
    HUIStr_Enum_TimeZone_32,
    HUIStr_Enum_TimeZone_33,
    HUIStr_Enum_TimeZone_34,
    HUIStr_Enum_TimeZone_35,
    HUIStr_Enum_TimeZone_36,
    HUIStr_Enum_TimeZone_37,
    HUIStr_Enum_TimeZone_38,
    HUIStr_Enum_TimeZone_39,
    HUIStr_Enum_TimeZone_40,

    HUIStr_Unit_MSL,

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

#endif // /ifndef HydroUIStrings_H
#endif
