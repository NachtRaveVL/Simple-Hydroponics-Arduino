/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino UI Strings/Prototypes
*/

#include "HydroUIStrings.h"
#ifdef HYDRO_USE_GUI

static char _blank = '\0';
const char *HUIStr_Blank = &_blank;

#ifndef HYDRO_DISABLE_BUILTIN_DATA
String stringFromPGMAddr(const char *flashUIStr); // implemented in base system
const char *pgmAddrForUIStr(HydroUI_String strNum);
#endif

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
                    auto bytesRead = file.readBytesUntil('\0', buffer, HYDRO_STRING_BUFFER_SIZE);
                    retVal.concat(charsToString(buffer, bytesRead));

                    while (strnlen(buffer, HYDRO_STRING_BUFFER_SIZE) == HYDRO_STRING_BUFFER_SIZE) {
                        bytesRead = file.readBytesUntil('\0', buffer, HYDRO_STRING_BUFFER_SIZE);
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
        return (_lookupCachedRes = stringFromPGMAddr(pgmAddrForUIStr(strNum)));
    #else
        return (_lookupCachedRes = String());
    #endif
}

#ifndef HYDRO_DISABLE_BUILTIN_DATA

const char *pgmAddrForUIStr(HydroUI_String strNum)
{
    switch(strNum) {
        case HUIStr_MatrixActions: {
            static const char flashUIStr_MatrixActions[] PROGMEM = {HYDRO_UI_MATRIX_ACTIONS};
            return flashUIStr_MatrixActions;
        } break;
        case HUIStr_Matrix2x2Keys: {
            static const char flashUIStr_Matrix2x2Keys[] PROGMEM = {HYDRO_UI_2X2MATRIX_KEYS};
            return flashUIStr_Matrix2x2Keys;
        } break;
        case HUIStr_Matrix3x4Keys: {
            static const char flashUIStr_Matrix3x4Keys[] PROGMEM = {HYDRO_UI_3X4MATRIX_KEYS};
            return flashUIStr_Matrix3x4Keys;
        } break;
        case HUIStr_Matrix4x4Keys: {
            static const char flashUIStr_Matrix4x4Keys[] PROGMEM = {HYDRO_UI_4X4MATRIX_KEYS};
            return flashUIStr_Matrix4x4Keys;
        } break;

        case HUIStr_Item_Actuators: {
            static const char flashUIStr_Item_Actuators[] PROGMEM = {"Actuators"};
            return flashUIStr_Item_Actuators;
        } break;
        case HUIStr_Item_AddNew: {
            static const char flashUIStr_Item_AddNew[] PROGMEM = {"Add New"};
            return flashUIStr_Item_AddNew;
        } break;
        case HUIStr_Item_Additives: {
            static const char flashUIStr_Item_Additives[] PROGMEM = {"Additives"};
            return flashUIStr_Item_Additives;
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
            static const char flashUIStr_Item_BackToOverview[] PROGMEM = {"Back To Overview"};
            return flashUIStr_Item_BackToOverview;
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
            static const char flashUIStr_Item_Calibrations[] PROGMEM = {"Calibrations"};
            return flashUIStr_Item_Calibrations;
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
            static const char flashUIStr_Item_Crops[] PROGMEM = {"Crops"};
            return flashUIStr_Item_Crops;
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
            static const char flashUIStr_Item_Debug[] PROGMEM = {"Debug"};
            return flashUIStr_Item_Debug;
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
        case HUIStr_Item_Info: {
            static const char flashUIStr_Item_Info[] PROGMEM = {"Info"};
            return flashUIStr_Item_Info;
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
            static const char flashUIStr_Item_Library[] PROGMEM = {"Library"};
            return flashUIStr_Item_Library;
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
            static const char flashUIStr_Item_PowerRails[] PROGMEM = {"Power Rails"};
            return flashUIStr_Item_PowerRails;
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
            static const char flashUIStr_Item_Reservoirs[] PROGMEM = {"Reservoirs"};
            return flashUIStr_Item_Reservoirs;
        } break;
        case HUIStr_Item_SDCard: {
            static const char flashUIStr_Item_SDCard[] PROGMEM = {"SD Card"};
            return flashUIStr_Item_SDCard;
        } break;
        case HUIStr_Item_Scheduling: {
            static const char flashUIStr_Item_Scheduling[] PROGMEM = {"Scheduling"};
            return flashUIStr_Item_Scheduling;
        } break;
        case HUIStr_Item_Sensors: {
            static const char flashUIStr_Item_Sensors[] PROGMEM = {"Sensors"};
            return flashUIStr_Item_Sensors;
        } break;
        case HUIStr_Item_Settings: {
            static const char flashUIStr_Item_Settings[] PROGMEM = {"Settings"};
            return flashUIStr_Item_Settings;
        } break;
        case HUIStr_Item_Size: {
            static const char flashUIStr_Item_Size[] PROGMEM = {"Size"};
            return flashUIStr_Item_Size;
        } break;
        case HUIStr_Item_System: {
            static const char flashUIStr_Item_System[] PROGMEM = {"System"};
            return flashUIStr_Item_System;
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
            static const char flashUIStr_Item_ToggleBadConn[] PROGMEM = {"Toggle BadConn"};
            return flashUIStr_Item_ToggleBadConn;
        } break;
        case HUIStr_Item_ToggleFastTime: {
            static const char flashUIStr_Item_ToggleFastTime[] PROGMEM = {"Toggle FastTime"};
            return flashUIStr_Item_ToggleFastTime;
        } break;
        case HUIStr_Item_TriggerAutosave: {
            static const char flashUIStr_Item_TriggerAutosave[] PROGMEM = {"Trigger Autosave"};
            return flashUIStr_Item_TriggerAutosave;
        } break;
        case HUIStr_Item_TriggerLowMem: {
            static const char flashUIStr_Item_TriggerLowMem[] PROGMEM = {"Trigger LowMem"};
            return flashUIStr_Item_TriggerLowMem;
        } break;
        case HUIStr_Item_TriggerSDCleanup: {
            static const char flashUIStr_Item_TriggerSDCleanup[] PROGMEM = {"Trigger SDCleanup"};
            return flashUIStr_Item_TriggerSDCleanup;
        } break;
        case HUIStr_Item_TriggerSigTime: {
            static const char flashUIStr_Item_TriggerSigTime[] PROGMEM = {"Trigger SigTime"};
            return flashUIStr_Item_TriggerSigTime;
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

        case HUIStr_Enum_Autosave_0: {
            static const char flashUIStr_Enum_Autosave_0[] PROGMEM = {"SD Card"};
            return flashUIStr_Enum_Autosave_0;
        } break;
        case HUIStr_Enum_Autosave_1: {
            static const char flashUIStr_Enum_Autosave_1[] PROGMEM = {"EEPROM"};
            return flashUIStr_Enum_Autosave_1;
        } break;
        case HUIStr_Enum_Autosave_2: {
            static const char flashUIStr_Enum_Autosave_2[] PROGMEM = {"WiFi Storage"};
            return flashUIStr_Enum_Autosave_2;
        } break;
        case HUIStr_Enum_Autosave_3: {
            static const char flashUIStr_Enum_Autosave_3[] PROGMEM = {"Disabled"};
            return flashUIStr_Enum_Autosave_3;
        } break;
        case HUIStr_Enum_DataPolling_0: {
            static const char flashUIStr_Enum_DataPolling_0[] PROGMEM = {"1 second"};
            return flashUIStr_Enum_DataPolling_0;
        } break;
        case HUIStr_Enum_DataPolling_1: {
            static const char flashUIStr_Enum_DataPolling_1[] PROGMEM = {"2 seconds"};
            return flashUIStr_Enum_DataPolling_1;
        } break;
        case HUIStr_Enum_DataPolling_2: {
            static const char flashUIStr_Enum_DataPolling_2[] PROGMEM = {"5 seconds"};
            return flashUIStr_Enum_DataPolling_2;
        } break;
        case HUIStr_Enum_DataPolling_3: {
            static const char flashUIStr_Enum_DataPolling_3[] PROGMEM = {"10 seconds"};
            return flashUIStr_Enum_DataPolling_3;
        } break;
        case HUIStr_Enum_DataPolling_4: {
            static const char flashUIStr_Enum_DataPolling_4[] PROGMEM = {"15 seconds"};
            return flashUIStr_Enum_DataPolling_4;
        } break;
        case HUIStr_Enum_DataPolling_5: {
            static const char flashUIStr_Enum_DataPolling_5[] PROGMEM = {"30 seconds"};
            return flashUIStr_Enum_DataPolling_5;
        } break;
        case HUIStr_Enum_DataPolling_6: {
            static const char flashUIStr_Enum_DataPolling_6[] PROGMEM = {"45 seconds"};
            return flashUIStr_Enum_DataPolling_6;
        } break;
        case HUIStr_Enum_DataPolling_7: {
            static const char flashUIStr_Enum_DataPolling_7[] PROGMEM = {"60 seconds"};
            return flashUIStr_Enum_DataPolling_7;
        } break;
        case HUIStr_Enum_GPSPolling_0: {
            static const char flashUIStr_Enum_GPSPolling_0[] PROGMEM = {"1 second"};
            return flashUIStr_Enum_GPSPolling_0;
        } break;
        case HUIStr_Enum_GPSPolling_1: {
            static const char flashUIStr_Enum_GPSPolling_1[] PROGMEM = {"2 seconds"};
            return flashUIStr_Enum_GPSPolling_1;
        } break;
        case HUIStr_Enum_GPSPolling_2: {
            static const char flashUIStr_Enum_GPSPolling_2[] PROGMEM = {"5 seconds"};
            return flashUIStr_Enum_GPSPolling_2;
        } break;
        case HUIStr_Enum_GPSPolling_3: {
            static const char flashUIStr_Enum_GPSPolling_3[] PROGMEM = {"10 seconds"};
            return flashUIStr_Enum_GPSPolling_3;
        } break;
        case HUIStr_Enum_Measurements_0: {
            static const char flashUIStr_Enum_Measurements_0[] PROGMEM = {"Imperial"};
            return flashUIStr_Enum_Measurements_0;
        } break;
        case HUIStr_Enum_Measurements_1: {
            static const char flashUIStr_Enum_Measurements_1[] PROGMEM = {"Metric"};
            return flashUIStr_Enum_Measurements_1;
        } break;
        case HUIStr_Enum_Measurements_2: {
            static const char flashUIStr_Enum_Measurements_2[] PROGMEM = {"Scientific"};
            return flashUIStr_Enum_Measurements_2;
        } break;
        case HUIStr_Enum_SystemMode_0: {
            static const char flashUIStr_Enum_SystemMode_0[] PROGMEM = {"Recycling"};
            return flashUIStr_Enum_SystemMode_0;
        } break;
        case HUIStr_Enum_SystemMode_1: {
            static const char flashUIStr_Enum_SystemMode_1[] PROGMEM = {"DrainToWaste"};
            return flashUIStr_Enum_SystemMode_1;
        } break;
        case HUIStr_Enum_TimeZone_0: {
            static const char flashUIStr_Enum_TimeZone_0[] PROGMEM = {"UTC -12"};
            return flashUIStr_Enum_TimeZone_0;
        } break;
        case HUIStr_Enum_TimeZone_1: {
            static const char flashUIStr_Enum_TimeZone_1[] PROGMEM = {"UTC -11"};
            return flashUIStr_Enum_TimeZone_1;
        } break;
        case HUIStr_Enum_TimeZone_2: {
            static const char flashUIStr_Enum_TimeZone_2[] PROGMEM = {"UTC -10"};
            return flashUIStr_Enum_TimeZone_2;
        } break;
        case HUIStr_Enum_TimeZone_3: {
            static const char flashUIStr_Enum_TimeZone_3[] PROGMEM = {"UTC -9"};
            return flashUIStr_Enum_TimeZone_3;
        } break;
        case HUIStr_Enum_TimeZone_4: {
            static const char flashUIStr_Enum_TimeZone_4[] PROGMEM = {"UTC -9:30"};
            return flashUIStr_Enum_TimeZone_4;
        } break;
        case HUIStr_Enum_TimeZone_5: {
            static const char flashUIStr_Enum_TimeZone_5[] PROGMEM = {"UTC -8"};
            return flashUIStr_Enum_TimeZone_5;
        } break;
        case HUIStr_Enum_TimeZone_6: {
            static const char flashUIStr_Enum_TimeZone_6[] PROGMEM = {"UTC -7"};
            return flashUIStr_Enum_TimeZone_6;
        } break;
        case HUIStr_Enum_TimeZone_7: {
            static const char flashUIStr_Enum_TimeZone_7[] PROGMEM = {"UTC -6"};
            return flashUIStr_Enum_TimeZone_7;
        } break;
        case HUIStr_Enum_TimeZone_8: {
            static const char flashUIStr_Enum_TimeZone_8[] PROGMEM = {"UTC -5"};
            return flashUIStr_Enum_TimeZone_8;
        } break;
        case HUIStr_Enum_TimeZone_9: {
            static const char flashUIStr_Enum_TimeZone_9[] PROGMEM = {"UTC -4"};
            return flashUIStr_Enum_TimeZone_9;
        } break;
        case HUIStr_Enum_TimeZone_10: {
            static const char flashUIStr_Enum_TimeZone_10[] PROGMEM = {"UTC -3:30"};
            return flashUIStr_Enum_TimeZone_10;
        } break;
        case HUIStr_Enum_TimeZone_11: {
            static const char flashUIStr_Enum_TimeZone_11[] PROGMEM = {"UTC -3"};
            return flashUIStr_Enum_TimeZone_11;
        } break;
        case HUIStr_Enum_TimeZone_12: {
            static const char flashUIStr_Enum_TimeZone_12[] PROGMEM = {"UTC -2:30"};
            return flashUIStr_Enum_TimeZone_12;
        } break;
        case HUIStr_Enum_TimeZone_13: {
            static const char flashUIStr_Enum_TimeZone_13[] PROGMEM = {"UTC -2"};
            return flashUIStr_Enum_TimeZone_13;
        } break;
        case HUIStr_Enum_TimeZone_14: {
            static const char flashUIStr_Enum_TimeZone_14[] PROGMEM = {"UTC -1"};
            return flashUIStr_Enum_TimeZone_14;
        } break;
        case HUIStr_Enum_TimeZone_15: {
            static const char flashUIStr_Enum_TimeZone_15[] PROGMEM = {"UTC +0"};
            return flashUIStr_Enum_TimeZone_15;
        } break;
        case HUIStr_Enum_TimeZone_16: {
            static const char flashUIStr_Enum_TimeZone_16[] PROGMEM = {"UTC +1"};
            return flashUIStr_Enum_TimeZone_16;
        } break;
        case HUIStr_Enum_TimeZone_17: {
            static const char flashUIStr_Enum_TimeZone_17[] PROGMEM = {"UTC +2"};
            return flashUIStr_Enum_TimeZone_17;
        } break;
        case HUIStr_Enum_TimeZone_18: {
            static const char flashUIStr_Enum_TimeZone_18[] PROGMEM = {"UTC +3"};
            return flashUIStr_Enum_TimeZone_18;
        } break;
        case HUIStr_Enum_TimeZone_19: {
            static const char flashUIStr_Enum_TimeZone_19[] PROGMEM = {"UTC +3:30"};
            return flashUIStr_Enum_TimeZone_19;
        } break;
        case HUIStr_Enum_TimeZone_20: {
            static const char flashUIStr_Enum_TimeZone_20[] PROGMEM = {"UTC +4"};
            return flashUIStr_Enum_TimeZone_20;
        } break;
        case HUIStr_Enum_TimeZone_21: {
            static const char flashUIStr_Enum_TimeZone_21[] PROGMEM = {"UTC +4:30"};
            return flashUIStr_Enum_TimeZone_21;
        } break;
        case HUIStr_Enum_TimeZone_22: {
            static const char flashUIStr_Enum_TimeZone_22[] PROGMEM = {"UTC +5"};
            return flashUIStr_Enum_TimeZone_22;
        } break;
        case HUIStr_Enum_TimeZone_23: {
            static const char flashUIStr_Enum_TimeZone_23[] PROGMEM = {"UTC +5:30"};
            return flashUIStr_Enum_TimeZone_23;
        } break;
        case HUIStr_Enum_TimeZone_24: {
            static const char flashUIStr_Item_XXX[] PROGMEM = {"UTC +5:45"};
            return flashUIStr_Item_XXX;
        } break;
        case HUIStr_Enum_TimeZone_25: {
            static const char flashUIStr_Enum_TimeZone_25[] PROGMEM = {"UTC +6"};
            return flashUIStr_Enum_TimeZone_25;
        } break;
        case HUIStr_Enum_TimeZone_26: {
            static const char flashUIStr_Enum_TimeZone_26[] PROGMEM = {"UTC +6:30"};
            return flashUIStr_Enum_TimeZone_26;
        } break;
        case HUIStr_Enum_TimeZone_27: {
            static const char flashUIStr_Enum_TimeZone_27[] PROGMEM = {"UTC +7"};
            return flashUIStr_Enum_TimeZone_27;
        } break;
        case HUIStr_Enum_TimeZone_28: {
            static const char flashUIStr_Enum_TimeZone_28[] PROGMEM = {"UTC +8"};
            return flashUIStr_Enum_TimeZone_28;
        } break;
        case HUIStr_Enum_TimeZone_29: {
            static const char flashUIStr_Enum_TimeZone_29[] PROGMEM = {"UTC +8:30"};
            return flashUIStr_Enum_TimeZone_29;
        } break;
        case HUIStr_Enum_TimeZone_30: {
            static const char flashUIStr_Item_XXX[] PROGMEM = {"UTC +8:45"};
            return flashUIStr_Item_XXX;
        } break;
        case HUIStr_Enum_TimeZone_31: {
            static const char flashUIStr_Enum_TimeZone_31[] PROGMEM = {"UTC +9"};
            return flashUIStr_Enum_TimeZone_31;
        } break;
        case HUIStr_Enum_TimeZone_32: {
            static const char flashUIStr_Enum_TimeZone_32[] PROGMEM = {"UTC +9:30"};
            return flashUIStr_Enum_TimeZone_32;
        } break;
        case HUIStr_Enum_TimeZone_33: {
            static const char flashUIStr_Enum_TimeZone_33[] PROGMEM = {"UTC +10"};
            return flashUIStr_Enum_TimeZone_33;
        } break;
        case HUIStr_Enum_TimeZone_34: {
            static const char flashUIStr_Enum_TimeZone_34[] PROGMEM = {"UTC +10:30"};
            return flashUIStr_Enum_TimeZone_34;
        } break;
        case HUIStr_Enum_TimeZone_35: {
            static const char flashUIStr_Enum_TimeZone_35[] PROGMEM = {"UTC +11"};
            return flashUIStr_Enum_TimeZone_35;
        } break;
        case HUIStr_Enum_TimeZone_36: {
            static const char flashUIStr_Enum_TimeZone_36[] PROGMEM = {"UTC +12"};
            return flashUIStr_Enum_TimeZone_36;
        } break;
        case HUIStr_Enum_TimeZone_37: {
            static const char flashUIStr_Enum_TimeZone_37[] PROGMEM = {"UTC +12:45"};
            return flashUIStr_Enum_TimeZone_37;
        } break;
        case HUIStr_Enum_TimeZone_38: {
            static const char flashUIStr_Enum_TimeZone_38[] PROGMEM = {"UTC +13"};
            return flashUIStr_Enum_TimeZone_38;
        } break;
        case HUIStr_Enum_TimeZone_39: {
            static const char flashUIStr_Enum_TimeZone_39[] PROGMEM = {"UTC +13:45"};
            return flashUIStr_Enum_TimeZone_39;
        } break;
        case HUIStr_Enum_TimeZone_40: {
            static const char flashUIStr_Enum_TimeZone_40[] PROGMEM = {"UTC +14"};
            return flashUIStr_Enum_TimeZone_40;
        } break;

        case HUIStr_Unit_MSL: {
            static const char flashUIStr_Unit_MSL[] PROGMEM = {"msl"};
            return flashUIStr_Unit_MSL;
        } break;

        case HUIStr_Count: break;
    }
    return nullptr;
}

#endif
#endif
