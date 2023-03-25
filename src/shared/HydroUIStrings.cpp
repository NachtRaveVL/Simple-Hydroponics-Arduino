/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino UI Strings/Prototypes
*/

#include "HydruinoUI.h"
#ifdef HYDRO_USE_GUI

static uint16_t _uiStrDataAddress((uint16_t)-1);
void beginUIStringsFromEEPROM(uint16_t uiDataAddress)
{
    _uiStrDataAddress = uiDataAddress;
}

static String _uiStrDataFilePrefix;
void beginUIStringsFromSDCard(String uiDataFilePrefix)
{
    _uiStrDataFilePrefix = uiDataFilePrefix;
}

inline String getUIStringsFilename()
{
    String filename; filename.reserve(_uiStrDataFilePrefix.length() + 11 + 1);
    filename.concat(_uiStrDataFilePrefix);
    filename.concat('u'); // Cannot use SFP here so have to do it the long way
    filename.concat('i');
    filename.concat('d');
    filename.concat('s');
    filename.concat('t');
    filename.concat('r');
    filename.concat('s');
    filename.concat('.');
    filename.concat('d');
    filename.concat('a');
    filename.concat('t');
    return filename;
}

String stringFromPGM(HydroUI_String strNum)
{
    static HydroUI_String _lookupStrNum = (HydroUI_String)-1; // Simple LRU cache reduces a lot of lookup access
    static String _lookupCachedRes;
    if (strNum == _lookupStrNum) { return _lookupCachedRes; }
    else { _lookupStrNum = strNum; } // _lookupCachedRes set below

    if (_uiStrDataAddress != (uint16_t)-1) {
        auto eeprom = getController()->getEEPROM();

        if (eeprom) {
            uint16_t lookupOffset = 0;
            eeprom->readBlock(_uiStrDataAddress + (sizeof(uint16_t) * ((int)strNum + 1)), // +1 for initial total size word
                              (uint8_t *)&lookupOffset, sizeof(lookupOffset));

            {   String retVal;
                char buffer[HYDRO_STRING_BUFFER_SIZE] = {0};
                uint16_t bytesRead = eeprom->readBlock(lookupOffset, (uint8_t *)&buffer[0], HYDRO_STRING_BUFFER_SIZE);
                retVal.concat(charsToString(buffer, bytesRead));

                while (strnlen(buffer, HYDRO_STRING_BUFFER_SIZE) == HYDRO_STRING_BUFFER_SIZE) {
                    lookupOffset += HYDRO_STRING_BUFFER_SIZE;
                    bytesRead = eeprom->readBlock(lookupOffset, (uint8_t *)&buffer[0], HYDRO_STRING_BUFFER_SIZE);
                    if (bytesRead) { retVal.concat(charsToString(buffer, bytesRead)); }
                }

                if (retVal.length()) {
                    return (_lookupCachedRes = retVal);
                }
            }
        }
    }

    if (_uiStrDataFilePrefix.length()) {
        #if HYDRO_SYS_LEAVE_FILES_OPEN
            static
        #endif
        auto sd = getController()->getSDCard();

        if (sd) {
            String retVal;
            #if HYDRO_SYS_LEAVE_FILES_OPEN
                static
            #endif
            auto file = sd->open(getUIStringsFilename().c_str(), FILE_READ);

            if (file) {
                uint16_t lookupOffset = 0;
                file.seek(sizeof(uint16_t) * (int)strNum);
                #if defined(ARDUINO_ARCH_RP2040) || defined(ESP_PLATFORM)
                    file.readBytes((char *)&lookupOffset, sizeof(lookupOffset));
                #else
                    file.readBytes((uint8_t *)&lookupOffset, sizeof(lookupOffset));
                #endif

                {   char buffer[HYDRO_STRING_BUFFER_SIZE];
                    file.seek(lookupOffset);
                    auto bytesRead = file.readBytesUntil('\000', buffer, HYDRO_STRING_BUFFER_SIZE);
                    retVal.concat(charsToString(buffer, bytesRead));

                    while (strnlen(buffer, HYDRO_STRING_BUFFER_SIZE) == HYDRO_STRING_BUFFER_SIZE) {
                        bytesRead = file.readBytesUntil('\000', buffer, HYDRO_STRING_BUFFER_SIZE);
                        if (bytesRead) { retVal.concat(charsToString(buffer, bytesRead)); }
                    }
                }

                #if !HYDRO_SYS_LEAVE_FILES_OPEN
                    file.close();
                #endif
            }

            #if !HYDRO_SYS_LEAVE_FILES_OPEN
                getController()->endSDCard(sd);
            #endif
            if (retVal.length()) {
                return (_lookupCachedRes = retVal);
            }
        }
    }

    #ifndef HYDRO_DISABLE_BUILTIN_DATA
        return (_lookupCachedRes = stringFromPGMAddr(pgmAddrForStr(strNum)));
    #else
        return (_lookupCachedRes = String());
    #endif
}

#ifndef HYDRO_DISABLE_BUILTIN_DATA

const char *pgmAddrForStr(HydroUI_String strNum)
{
    switch(strNum) {
        case HUIStr_Keys_MatrixActions: {
            static const char flashUIStr_Keys_MatrixActions[] PROGMEM = {HYDRO_UI_MATRIX_ACTIONS};
            return flashUIStr_Keys_MatrixActions;
        } break;

        case HUIStr_Item_Actuators: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Actuators = { "Actuators", 20, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Actuators;
        } break;
        case HUIStr_Item_AddNew: {
            static const char flashUIStr_Item_AddNew[] PROGMEM = {"Add New"};
            return flashUIStr_Item_AddNew;
        } break;
        case HUIStr_Item_Additives: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Additives = { "Additives", 41, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Additives;
        } break;
        case HUIStr_Item_Alerts: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Alerts = { "Alerts", 1, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Alerts;
        } break;
        case HUIStr_Item_AllowRemoteCtrl: {
            static const char flashUIStr_Item_AllowRemoteCtrl[] PROGMEM = {"Allow Remote Ctrl"};
            return flashUIStr_Item_AllowRemoteCtrl;
        } break;
        case HUIStr_Item_Altitude: {
            static const char flashUIStr_Item_Altitude[] PROGMEM = {"Altitude"};
            return flashUIStr_Item_Altitude;
        } break;
        case HUIStr_Item_AssignByDHCP: {
            static const char flashUIStr_Item_AssignByDHCP[] PROGMEM = {"Assign by DHCP"};
            return flashUIStr_Item_AssignByDHCP;
        } break;
        case HUIStr_Item_AssignByHostname: {
            static const char flashUIStr_Item_AssignByHostname[] PROGMEM = {"Assign by Hostname"};
            return flashUIStr_Item_AssignByHostname;
        } break;
        case HUIStr_Item_AutosavePrimary: {
            static const char flashUIStr_Item_AutosavePrimary[] PROGMEM = {"Autosave Primary"};
            return flashUIStr_Item_AutosavePrimary;
        } break;
        case HUIStr_Item_AutosaveSecondary: {
            static const char flashUIStr_Item_AutosaveSecondary[] PROGMEM = {"Autosave Secondary"};
            return flashUIStr_Item_AutosaveSecondary;
        } break;
        case HUIStr_Item_BackToOverview: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_BackToOverview = { "Back to Overview", 7, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_BackToOverview;
        } break;
        case HUIStr_Item_BatteryFailure: {
            static const char flashUIStr_Item_BatteryFailure[] PROGMEM = {"Battery Failure"};
            return flashUIStr_Item_BatteryFailure;
        } break;
        case HUIStr_Item_Board: {
            static const char flashUIStr_Item_Board[] PROGMEM = {"Board"};
            return flashUIStr_Item_Board;
        } break;
        case HUIStr_Item_BrokerPort: {
            static const char flashUIStr_Item_BrokerPort[] PROGMEM = {"Broker Port"};
            return flashUIStr_Item_BrokerPort;
        } break;
        case HUIStr_Item_Browse: {
            static const char flashUIStr_Item_Browse[] PROGMEM = {"Browse"};
            return flashUIStr_Item_Browse;
        } break;
        case HUIStr_Item_Calibrations: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Calibrations = { "Calibrations", 42, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Calibrations;
        } break;
        case HUIStr_Item_ControlMode: {
            static const char flashUIStr_Item_ControlMode[] PROGMEM = {"Control Mode"};
            return flashUIStr_Item_ControlMode;
        } break;
        case HUIStr_Item_ControllerIP: {
            static const char flashUIStr_Item_ControllerIP[] PROGMEM = {"Controller IP"};
            return flashUIStr_Item_ControllerIP;
        } break;
        case HUIStr_Item_Controls: {
            static const char flashUIStr_Item_Controls[] PROGMEM = {"Controls"};
            return flashUIStr_Item_Controls;
        } break;
        case HUIStr_Item_Crops: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Crops = { "Crops", 22, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Crops;
        } break;
        case HUIStr_Item_CropsLib: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_CropsLib = { "Crops", 40, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_CropsLib;
        } break;
        case HUIStr_Item_DSTAddHour: {
            static const char flashUIStr_Item_DSTAddHour[] PROGMEM = {"DST Add Hour"};
            return flashUIStr_Item_DSTAddHour;
        } break;
        case HUIStr_Item_DataPolling: {
            static const char flashUIStr_Item_DataPolling[] PROGMEM = {"Data Polling"};
            return flashUIStr_Item_DataPolling;
        } break;
        case HUIStr_Item_Date: {
            static const char flashUIStr_Item_Date[] PROGMEM = {"Date"};
            return flashUIStr_Item_Date;
        } break;
        case HUIStr_Item_Debug: {
            static const PROGMEM SubMenuInfo flashUIStr_Item_Debug = { "Debug", 6, NO_ADDRESS, 0, NO_CALLBACK };
            return (const char *)&flashUIStr_Item_Debug;
        } break;
        case HUIStr_Item_DisplayMode: {
            static const char flashUIStr_Item_DisplayMode[] PROGMEM = {"Display Mode"};
            return flashUIStr_Item_DisplayMode;
        } break;
        case HUIStr_Item_EEPROM: {
            static const char flashUIStr_Item_EEPROM[] PROGMEM = {"EEPROM"};
            return flashUIStr_Item_EEPROM;
        } break;
        case HUIStr_Item_Firmware: {
            static const char flashUIStr_Item_Firmware[] PROGMEM = {"Firmware"};
            return flashUIStr_Item_Firmware;
        } break;
        case HUIStr_Item_FreeMemory: {
            static const char flashUIStr_Item_FreeMemory[] PROGMEM = {"Free Memory"};
            return flashUIStr_Item_FreeMemory;
        } break;
        case HUIStr_Item_GPSPolling: {
            static const char flashUIStr_Item_GPSPolling[] PROGMEM = {"GPS Polling"};
            return flashUIStr_Item_GPSPolling;
        } break;
        case HUIStr_Item_General: {
            static const char flashUIStr_Item_General[] PROGMEM = {"General"};
            return flashUIStr_Item_General;
        } break;
        case HUIStr_Item_Information: {
            static const PROGMEM BooleanMenuInfo flashUIStr_Item_Information = { "Information", 5, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Information;
        } break;
        case HUIStr_Item_JoystickXMid: {
            static const char flashUIStr_Item_JoystickXMid[] PROGMEM = {"Joystick X Mid"};
            return flashUIStr_Item_JoystickXMid;
        } break;
        case HUIStr_Item_JoystickXTol: {
            static const char flashUIStr_Item_JoystickXTol[] PROGMEM = {"Joystick X Tol"};
            return flashUIStr_Item_JoystickXTol;
        } break;
        case HUIStr_Item_JoystickYMid: {
            static const char flashUIStr_Item_JoystickYMid[] PROGMEM = {"Joystick Y Mid"};
            return flashUIStr_Item_JoystickYMid;
        } break;
        case HUIStr_Item_JoystickYTol: {
            static const char flashUIStr_Item_JoystickYTol[] PROGMEM = {"Joystick Y Tol"};
            return flashUIStr_Item_JoystickYTol;
        } break;
        case HUIStr_Item_LatDegrees: {
            static const char flashUIStr_Item_LatDegrees[] PROGMEM = {"Lat Degrees"};
            return flashUIStr_Item_LatDegrees;
        } break;
        case HUIStr_Item_Library: {
            static const PROGMEM SubMenuInfo flashUIStr_Item_Library = { "Library", 4, NO_ADDRESS, 0, NO_CALLBACK };
            return (const char *)&flashUIStr_Item_Library;
        } break;
        case HUIStr_Item_LocalTime: {
            static const char flashUIStr_Item_LocalTime[] PROGMEM = {"Local Time"};
            return flashUIStr_Item_LocalTime;
        } break;
        case HUIStr_Item_Location: {
            static const char flashUIStr_Item_Location[] PROGMEM = {"Location"};
            return flashUIStr_Item_Location;
        } break;
        case HUIStr_Item_LongMinutes: {
            static const char flashUIStr_Item_LongMinutes[] PROGMEM = {"Long Minutes"};
            return flashUIStr_Item_LongMinutes;
        } break;
        case HUIStr_Item_MACAddr0x: {
            static const char flashUIStr_Item_MACAddr0x[] PROGMEM = {"MAC Addr 0x"};
            return flashUIStr_Item_MACAddr0x;
        } break;
        case HUIStr_Item_MQTTBroker: {
            static const char flashUIStr_Item_MQTTBroker[] PROGMEM = {"MQTT Broker"};
            return flashUIStr_Item_MQTTBroker;
        } break;
        case HUIStr_Item_Measurements: {
            static const char flashUIStr_Item_Measurements[] PROGMEM = {"Measurements"};
            return flashUIStr_Item_Measurements;
        } break;
        case HUIStr_Item_Name: {
            static const char flashUIStr_Item_Name[] PROGMEM = {"Name"};
            return flashUIStr_Item_Name;
        } break;
        case HUIStr_Item_Networking: {
            static const char flashUIStr_Item_Networking[] PROGMEM = {"Networking"};
            return flashUIStr_Item_Networking;
        } break;
        case HUIStr_Item_PowerRails: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_PowerRails = { "Power Rails", 24, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_PowerRails;
        } break;
        case HUIStr_Item_RTC: {
            static const char flashUIStr_Item_RTC[] PROGMEM = {"RTC"};
            return flashUIStr_Item_RTC;
        } break;
        case HUIStr_Item_RemoteCtrlPort: {
            static const char flashUIStr_Item_RemoteCtrlPort[] PROGMEM = {"Remote Ctrl Port"};
            return flashUIStr_Item_RemoteCtrlPort;
        } break;
        case HUIStr_Item_Reservoirs: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Reservoirs = { "Reservoirs", 23, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Reservoirs;
        } break;
        case HUIStr_Item_SDCard: {
            static const char flashUIStr_Item_SDCard[] PROGMEM = {"SD Card"};
            return flashUIStr_Item_SDCard;
        } break;
        case HUIStr_Item_Scheduling: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Scheduling = { "Scheduling", 25, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Scheduling;
        } break;
        case HUIStr_Item_Sensors: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Sensors = { "Sensors", 21, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Sensors;
        } break;
        case HUIStr_Item_Settings: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_Settings = { "Settings", 3, NO_ADDRESS, 0, gotoScreen };
            return (const char *)&flashUIStr_Item_Settings;
        } break;
        case HUIStr_Item_Size: {
            static const char flashUIStr_Item_Size[] PROGMEM = {"Size"};
            return flashUIStr_Item_Size;
        } break;
        case HUIStr_Item_System: {
            static const PROGMEM SubMenuInfo flashUIStr_Item_System = { "System", 2, NO_ADDRESS, 0, NO_CALLBACK };
            return (const char *)&flashUIStr_Item_System;
        } break;
        case HUIStr_Item_SystemMode: {
            static const char flashUIStr_Item_SystemMode[] PROGMEM = {"System Mode"};
            return flashUIStr_Item_SystemMode;
        } break;
        case HUIStr_Item_SystemName: {
            static const char flashUIStr_Item_SystemName[] PROGMEM = {"System Name"};
            return flashUIStr_Item_SystemName;
        } break;
        case HUIStr_Item_Time: {
            static const char flashUIStr_Item_Time[] PROGMEM = {"Time"};
            return flashUIStr_Item_Time;
        } break;
        case HUIStr_Item_TimeZone: {
            static const char flashUIStr_Item_TimeZone[] PROGMEM = {"Time Zone"};
            return flashUIStr_Item_TimeZone;
        } break;
        case HUIStr_Item_ToggleBadConn: {
            static const PROGMEM BooleanMenuInfo flashUIStr_Item_ToggleBadConn = { "Toggle BadConn", 65, NO_ADDRESS, 1, debugAction, NAMING_ON_OFF };
            return (const char *)&flashUIStr_Item_ToggleBadConn;
        } break;
        case HUIStr_Item_ToggleFastTime: {
            static const PROGMEM BooleanMenuInfo flashUIStr_Item_ToggleFastTime = { "Toggle FastTime", 64, NO_ADDRESS, 1, debugAction, NAMING_ON_OFF };
            return (const char *)&flashUIStr_Item_ToggleFastTime;
        } break;
        case HUIStr_Item_TriggerAutosave: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_TriggerAutosave = { "Trigger Autosave", 60, NO_ADDRESS, 0, debugAction };
            return (const char *)&flashUIStr_Item_TriggerAutosave;
        } break;
        case HUIStr_Item_TriggerLowMem: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_TriggerLowMem = { "Trigger LowMem", 61, NO_ADDRESS, 0, debugAction };
            return (const char *)&flashUIStr_Item_TriggerLowMem;
        } break;
        case HUIStr_Item_TriggerSDCleanup: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_TriggerSDCleanup = { "Trigger SDCleanup", 62, NO_ADDRESS, 0, debugAction };
            return (const char *)&flashUIStr_Item_TriggerSDCleanup;
        } break;
        case HUIStr_Item_TriggerSigTime: {
            static const PROGMEM AnyMenuInfo flashUIStr_Item_TriggerSigTime = { "Trigger SigTime", 63, NO_ADDRESS, 0, debugAction };
            return (const char *)&flashUIStr_Item_TriggerSigTime;
        } break;
        case HUIStr_Item_Uptime: {
            static const char flashUIStr_Item_Uptime[] PROGMEM = {"Uptime"};
            return flashUIStr_Item_Uptime;
        } break;
        case HUIStr_Item_WiFiPass: {
            static const char flashUIStr_Item_WiFiPass[] PROGMEM = {"WiFi Pass"};
            return flashUIStr_Item_WiFiPass;
        } break;
        case HUIStr_Item_WiFiSSID: {
            static const char flashUIStr_Item_WiFiSSID[] PROGMEM = {"WiFi SSID"};
            return flashUIStr_Item_WiFiSSID;
        } break;

        case HUIStr_Enum_Autosave: {
            // SD Card000000
            // EEPROM0000000
            // WiFi Storage0
            static const char flashUIStr_Enum_Autosave[] PROGMEM = {"SD Card\000\000\000\000\000\000EEPROM\000\000\000\000\000\000\000WiFi Storage"};
            return flashUIStr_Enum_Autosave;
        } break;
        case HUIStr_Enum_DataPolling: {
            // 1 second000
            // 2 seconds00
            // 5 seconds00
            // 10 seconds0
            // 15 seconds0
            // 30 seconds0
            // 45 seconds0
            // 60 seconds0
            static const char flashUIStr_Enum_DataPolling[] PROGMEM = {"1 second\000\000\0002 seconds\000\0005 seconds\000\00010 seconds\00015 seconds\00030 seconds\00045 seconds\00060 seconds"};
            return flashUIStr_Enum_DataPolling;
        } break;
        case HUIStr_Enum_GPSPolling: {
            // 1 second000
            // 2 seconds00
            // 5 seconds00
            // 10 seconds0
            static const char flashUIStr_Enum_GPSPolling[] PROGMEM = {"1 second\000\000\0002 seconds\000\0005 seconds\000\00010 seconds"};
            return flashUIStr_Enum_GPSPolling;
        } break;
        case HUIStr_Enum_Measurements: {
            // Imperial000
            // Metric00000
            // Scientific0
            static const char flashUIStr_Enum_Measurements[] PROGMEM = {"Imperial\000\000\000Metric\000\000\000\000\000Scientific"};
            return flashUIStr_Enum_Measurements;
        } break;
        case HUIStr_Enum_SystemMode: {
            // Recycling0000
            // DrainToWaste0
            static const char flashUIStr_Enum_SystemMode[] PROGMEM = {"Recycling\000\000\000\000DrainToWaste"};
            return flashUIStr_Enum_SystemMode;
        } break;
        case HUIStr_Enum_TimeZone: {
            // UTC -12****
            // UTC -11****
            // UTC -10****
            // UTC -9*****
            // UTC -9:30**
            // UTC -8*****
            // UTC -7*****
            // UTC -6*****
            // UTC -5*****
            // UTC -4*****
            // UTC -3:30**
            // UTC -3*****
            // UTC -2:30**
            // UTC -2*****
            // UTC -1*****
            // UTC +0*****
            // UTC +1*****
            // UTC +2*****
            // UTC +3*****
            // UTC +3:30**
            // UTC +4*****
            // UTC +4:30**
            // UTC +5*****
            // UTC +5:30**
            // UTC +5:45**
            // UTC +6*****
            // UTC +6:30**
            // UTC +7*****
            // UTC +8*****
            // UTC +8:30**
            // UTC +8:45**
            // UTC +9*****
            // UTC +9:30**
            // UTC +10****
            // UTC +10:30*
            // UTC +11****
            // UTC +12****
            // UTC +12:45*
            // UTC +13****
            // UTC +13:45*
            // UTC +14****
            static const char flashUIStr_Enum_TimeZone[] PROGMEM = {"UTC -12\000\000\000\000UTC -11\000\000\000\000UTC -10\000\000\000\000UTC -9\000\000\000\000\000UTC -9:30\000\000UTC -8\000\000\000\000\000UTC -7\000\000\000\000\000UTC -6\000\000\000\000\000UTC -5\000\000\000\000\000UTC -4\000\000\000\000\000UTC -3:30\000\000UTC -3\000\000\000\000\000UTC -2:30\000\000UTC -2\000\000\000\000\000UTC -1\000\000\000\000\000UTC +0\000\000\000\000\000UTC +1\000\000\000\000\000UTC +2\000\000\000\000\000UTC +3\000\000\000\000\000UTC +3:30\000\000UTC +4\000\000\000\000\000UTC +4:30\000\000UTC +5\000\000\000\000\000UTC +5:30\000\000UTC +5:45\000\000UTC +6\000\000\000\000\000UTC +6:30\000\000UTC +7\000\000\000\000\000UTC +8\000\000\000\000\000UTC +8:30\000\000UTC +8:45\000\000UTC +9\000\000\000\000\000UTC +9:30\000\000UTC +10\000\000\000\000UTC +10:30\000UTC +11\000\000\000\000UTC +12\000\000\000\000UTC +12:45\000UTC +13\000\000\000\000UTC +13:45\000UTC +14\000\000\000"};
            return flashUIStr_Enum_TimeZone;
        } break;

        case HUIStr_Unit_MSL: {
            static const char flashUIStr_Unit_MSL[] PROGMEM = {"msl"};
            return flashUIStr_Unit_MSL;
        } break;

        case HUIStr_Count: break;
    }
    return nullptr;
}

#endif // /ifndef HYDRO_DISABLE_BUILTIN_DATA

size_t enumListPitch(const char *enumData)
{
    size_t size = 0;
    while (get_info_char(enumData) != '\000' && size < 64) {
        ++size; ++enumData;
    }
    while (get_info_char(enumData) == '\000' && size < 64) {
        ++size; ++enumData;
    }
    return size;
}

#endif
