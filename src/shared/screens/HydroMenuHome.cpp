/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Home Menu Screen
*/

#include "../HydruinoUI.h"
#ifdef HYDRO_USE_GUI

const PROGMEM char pgmStrIoTMonitorText[] = { "IoT Monitor" };
const PROGMEM char pgmStrAuthenticatorText[] = { "Authenticator" };

void CALLBACK_FUNCTION allowRemoteChanged(int id) { } // todo
void CALLBACK_FUNCTION altChanged(int id) { } // todo
void CALLBACK_FUNCTION backToOverview(int id) { getBaseUI()->reset(); }
void CALLBACK_FUNCTION brokerByChanged(int id) { } // todo
void CALLBACK_FUNCTION brokerChanged(int id) { } // todo
void CALLBACK_FUNCTION brokerPortChanged(int id) { } // todo
void CALLBACK_FUNCTION dateChanged(int id) { } // todo
void CALLBACK_FUNCTION dstChanged(int id) { } // todo
void CALLBACK_FUNCTION ipByChanged(int id) { } // todo
void CALLBACK_FUNCTION ipChanged(int id) { } // todo
void CALLBACK_FUNCTION jsConfigChanged(int id) { } // todo
void CALLBACK_FUNCTION latChanged(int id) { } // todo
void CALLBACK_FUNCTION longChanged(int id) { } // todo
void CALLBACK_FUNCTION macChanged(int id) { } // todo
void CALLBACK_FUNCTION passChanged(int id) { } // todo
void CALLBACK_FUNCTION pollingChanged(int id) { } // todo
void CALLBACK_FUNCTION pollingDTChanged(int id) { } // todo
void CALLBACK_FUNCTION primaryAutosaveChanged(int id) { } // todo
void CALLBACK_FUNCTION remotePortChanged(int id) { } // todo
void CALLBACK_FUNCTION secondaryAutosaveChanged(int id) { } // todo
void CALLBACK_FUNCTION ssidChanged(int id) { } // todo
void CALLBACK_FUNCTION sysMeasureChanged(int id) { } // todo
void CALLBACK_FUNCTION sysModeChanged(int id) { } // todo
void CALLBACK_FUNCTION sysNameChanged(int id) { } // todo
void CALLBACK_FUNCTION timeChanged(int id) { } // todo
void CALLBACK_FUNCTION tzChanged(int id) { } // todo

H_RENDERING_CALLBACK_NAME_INVOKE(fnEEPROMSizeRtCall, textItemRenderFn, HUIStr_Item_Size, -1, NO_CALLBACK)
H_RENDERING_CALLBACK_NAME_INVOKE(fnSDNameRtCall, textItemRenderFn, HUIStr_Item_Name, -1, NO_CALLBACK)
H_RENDERING_CALLBACK_NAME_INVOKE(fnDisplayModeRtCall, textItemRenderFn, HUIStr_Item_DisplayMode, -1, NO_CALLBACK)
H_RENDERING_CALLBACK_NAME_INVOKE(fnControlModeRtCall, textItemRenderFn, HUIStr_Item_ControlMode, -1, NO_CALLBACK)
H_RENDERING_CALLBACK_NAME_INVOKE(fnFreeMemoryRtCall, textItemRenderFn, HUIStr_Item_FreeMemory, -1, NO_CALLBACK)
H_RENDERING_CALLBACK_NAME_INVOKE(fnUptimeRtCall, textItemRenderFn, HUIStr_Item_Uptime, -1, NO_CALLBACK)
H_RENDERING_CALLBACK_NAME_INVOKE(fnFirmwareRtCall, textItemRenderFn, HUIStr_Item_Firmware, -1, NO_CALLBACK)
H_RENDERING_CALLBACK_NAME_INVOKE(fnBoardRtCall, textItemRenderFn, HUIStr_Item_Board, -1, NO_CALLBACK)
H_RENDERING_CALLBACK_NAME_INVOKE(fnJoystickYTolRtCall, largeNumItemRenderFn, HUIStr_Item_JoystickYTol, -1, jsConfigChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnJoystickYMidRtCall, largeNumItemRenderFn, HUIStr_Item_JoystickYMid, -1, jsConfigChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnJoystickXTolRtCall, largeNumItemRenderFn, HUIStr_Item_JoystickXTol, -1, jsConfigChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnJoystickXMidRtCall, largeNumItemRenderFn, HUIStr_Item_JoystickXMid, -1, jsConfigChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnLongitudeMinRtCall, largeNumItemRenderFn, HUIStr_Item_LongMinutes, -1, longChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnLatitudeDegRtCall, largeNumItemRenderFn, HUIStr_Item_LatDegrees, -1, latChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnMQTTBrokerRtCall, ipAddressRenderFn, HUIStr_Item_MQTTBroker, -1, brokerChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnWiFiPasswordRtCall, textItemRenderFn, HUIStr_Item_WiFiPass, -1, passChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnWiFiSSIDRtCall, textItemRenderFn, HUIStr_Item_WiFiSSID, -1, ssidChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnMACAddressRtCall, textItemRenderFn, HUIStr_Item_MACAddr0x, -1, macChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnControllerIPRtCall, ipAddressRenderFn, HUIStr_Item_ControllerIP, -1, ipChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnLocalTimeRtCall, timeItemRenderFn, HUIStr_Item_LocalTime, -1, timeChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnDateRtCall, dateItemRenderFn, HUIStr_Item_Date, -1, dateChanged)
H_RENDERING_CALLBACK_NAME_INVOKE(fnSystemNameRtCall, textItemRenderFn, HUIStr_Item_SystemName, -1, sysNameChanged)


HydroHomeMenu::HydroHomeMenu()
    : HydroMenu(), _items(nullptr)
{ ; }

HydroHomeMenu::~HydroHomeMenu()
{
    if (_items) { delete _items; }
}

void HydroHomeMenu::loadMenu(MenuItem *addFrom)
{
    if (!_items) {
        _loaded = (bool)(_items = new HydroHomeMenuItems());
        HYDRO_SOFT_ASSERT(_items, SFP(HStr_Err_AllocationFailure));
    }
}

MenuItem *HydroHomeMenu::getRootItem()
{
    if (!_loaded) { loadMenu(); }
    return _loaded && _items ? &_items->menuSystem : nullptr;
}

HydroHomeMenuInfo::HydroHomeMenuInfo()
{
    #ifdef HELIO_DISABLE_BUILTIN_DATA
        enumStrGPSPolling[0] = (_enumStrGPSPolling[0] = SFP(HUIStr_Enum_GPSPolling_0)).c_str();
        enumStrGPSPolling[1] = (_enumStrGPSPolling[1] = SFP(HUIStr_Enum_GPSPolling_1)).c_str();
        enumStrGPSPolling[2] = (_enumStrGPSPolling[2] = SFP(HUIStr_Enum_GPSPolling_2)).c_str();
        enumStrGPSPolling[3] = (_enumStrGPSPolling[3] = SFP(HUIStr_Enum_GPSPolling_3)).c_str();
        enumStrTimeZone[0] = (_enumStrTimeZone[0] = SFP(HUIStr_Enum_TimeZone_0)).c_str();
        enumStrTimeZone[1] = (_enumStrTimeZone[1] = SFP(HUIStr_Enum_TimeZone_1)).c_str();
        enumStrTimeZone[2] = (_enumStrTimeZone[2] = SFP(HUIStr_Enum_TimeZone_2)).c_str();
        enumStrTimeZone[3] = (_enumStrTimeZone[3] = SFP(HUIStr_Enum_TimeZone_3)).c_str();
        enumStrTimeZone[4] = (_enumStrTimeZone[4] = SFP(HUIStr_Enum_TimeZone_4)).c_str();
        enumStrTimeZone[5] = (_enumStrTimeZone[5] = SFP(HUIStr_Enum_TimeZone_5)).c_str();
        enumStrTimeZone[6] = (_enumStrTimeZone[6] = SFP(HUIStr_Enum_TimeZone_6)).c_str();
        enumStrTimeZone[7] = (_enumStrTimeZone[7] = SFP(HUIStr_Enum_TimeZone_7)).c_str();
        enumStrTimeZone[8] = (_enumStrTimeZone[8] = SFP(HUIStr_Enum_TimeZone_8)).c_str();
        enumStrTimeZone[9] = (_enumStrTimeZone[9] = SFP(HUIStr_Enum_TimeZone_9)).c_str();
        enumStrTimeZone[10] = (_enumStrTimeZone[10] = SFP(HUIStr_Enum_TimeZone_10)).c_str();
        enumStrTimeZone[11] = (_enumStrTimeZone[11] = SFP(HUIStr_Enum_TimeZone_11)).c_str();
        enumStrTimeZone[12] = (_enumStrTimeZone[12] = SFP(HUIStr_Enum_TimeZone_12)).c_str();
        enumStrTimeZone[13] = (_enumStrTimeZone[13] = SFP(HUIStr_Enum_TimeZone_13)).c_str();
        enumStrTimeZone[14] = (_enumStrTimeZone[14] = SFP(HUIStr_Enum_TimeZone_14)).c_str();
        enumStrTimeZone[15] = (_enumStrTimeZone[15] = SFP(HUIStr_Enum_TimeZone_15)).c_str();
        enumStrTimeZone[16] = (_enumStrTimeZone[16] = SFP(HUIStr_Enum_TimeZone_16)).c_str();
        enumStrTimeZone[17] = (_enumStrTimeZone[17] = SFP(HUIStr_Enum_TimeZone_17)).c_str();
        enumStrTimeZone[18] = (_enumStrTimeZone[18] = SFP(HUIStr_Enum_TimeZone_18)).c_str();
        enumStrTimeZone[19] = (_enumStrTimeZone[19] = SFP(HUIStr_Enum_TimeZone_19)).c_str();
        enumStrTimeZone[20] = (_enumStrTimeZone[20] = SFP(HUIStr_Enum_TimeZone_20)).c_str();
        enumStrTimeZone[21] = (_enumStrTimeZone[21] = SFP(HUIStr_Enum_TimeZone_21)).c_str();
        enumStrTimeZone[22] = (_enumStrTimeZone[22] = SFP(HUIStr_Enum_TimeZone_22)).c_str();
        enumStrTimeZone[23] = (_enumStrTimeZone[23] = SFP(HUIStr_Enum_TimeZone_23)).c_str();
        enumStrTimeZone[24] = (_enumStrTimeZone[24] = SFP(HUIStr_Enum_TimeZone_24)).c_str();
        enumStrTimeZone[25] = (_enumStrTimeZone[25] = SFP(HUIStr_Enum_TimeZone_25)).c_str();
        enumStrTimeZone[26] = (_enumStrTimeZone[26] = SFP(HUIStr_Enum_TimeZone_26)).c_str();
        enumStrTimeZone[27] = (_enumStrTimeZone[27] = SFP(HUIStr_Enum_TimeZone_27)).c_str();
        enumStrTimeZone[28] = (_enumStrTimeZone[28] = SFP(HUIStr_Enum_TimeZone_28)).c_str();
        enumStrTimeZone[29] = (_enumStrTimeZone[29] = SFP(HUIStr_Enum_TimeZone_29)).c_str();
        enumStrTimeZone[30] = (_enumStrTimeZone[30] = SFP(HUIStr_Enum_TimeZone_30)).c_str();
        enumStrTimeZone[31] = (_enumStrTimeZone[31] = SFP(HUIStr_Enum_TimeZone_31)).c_str();
        enumStrTimeZone[32] = (_enumStrTimeZone[32] = SFP(HUIStr_Enum_TimeZone_32)).c_str();
        enumStrTimeZone[33] = (_enumStrTimeZone[33] = SFP(HUIStr_Enum_TimeZone_33)).c_str();
        enumStrTimeZone[34] = (_enumStrTimeZone[34] = SFP(HUIStr_Enum_TimeZone_34)).c_str();
        enumStrTimeZone[35] = (_enumStrTimeZone[35] = SFP(HUIStr_Enum_TimeZone_35)).c_str();
        enumStrTimeZone[36] = (_enumStrTimeZone[36] = SFP(HUIStr_Enum_TimeZone_36)).c_str();
        enumStrTimeZone[37] = (_enumStrTimeZone[37] = SFP(HUIStr_Enum_TimeZone_37)).c_str();
        enumStrTimeZone[38] = (_enumStrTimeZone[38] = SFP(HUIStr_Enum_TimeZone_38)).c_str();
        enumStrTimeZone[39] = (_enumStrTimeZone[39] = SFP(HUIStr_Enum_TimeZone_39)).c_str();
        enumStrTimeZone[40] = (_enumStrTimeZone[40] = SFP(HUIStr_Enum_TimeZone_40)).c_str();
        enumStrDataPolling[0] = (_enumStrDataPolling[0] = SFP(HUIStr_Enum_DataPolling_0)).c_str();
        enumStrDataPolling[1] = (_enumStrDataPolling[1] = SFP(HUIStr_Enum_DataPolling_1)).c_str();
        enumStrDataPolling[2] = (_enumStrDataPolling[2] = SFP(HUIStr_Enum_DataPolling_2)).c_str();
        enumStrDataPolling[3] = (_enumStrDataPolling[3] = SFP(HUIStr_Enum_DataPolling_3)).c_str();
        enumStrDataPolling[4] = (_enumStrDataPolling[4] = SFP(HUIStr_Enum_DataPolling_4)).c_str();
        enumStrDataPolling[5] = (_enumStrDataPolling[5] = SFP(HUIStr_Enum_DataPolling_5)).c_str();
        enumStrDataPolling[6] = (_enumStrDataPolling[6] = SFP(HUIStr_Enum_DataPolling_6)).c_str();
        enumStrDataPolling[7] = (_enumStrDataPolling[7] = SFP(HUIStr_Enum_DataPolling_7)).c_str();
        enumStrAutosave[0] = (_enumStrAutosave[0] = SFP(HUIStr_Enum_Autosave_0)).c_str();
        enumStrAutosave[1] = (_enumStrAutosave[1] = SFP(HUIStr_Enum_Autosave_1)).c_str();
        enumStrAutosave[2] = (_enumStrAutosave[2] = SFP(HUIStr_Enum_Autosave_2)).c_str();
        enumStrAutosave[3] = (_enumStrAutosave[3] = SFP(HUIStr_Enum_Autosave_3)).c_str();
        enumStrMeasurements[0] = (_enumStrMeasurements[0] = SFP(HUIStr_Enum_Measurements_0)).c_str();
        enumStrMeasurements[1] = (_enumStrMeasurements[1] = SFP(HUIStr_Enum_Measurements_1)).c_str();
        enumStrMeasurements[2] = (_enumStrMeasurements[2] = SFP(HUIStr_Enum_Measurements_2)).c_str();
        enumStrSystemMode[0] = (_enumStrSystemMode[0] = SFP(HUIStr_Enum_SystemMode_0)).c_str();
        enumStrSystemMode[1] = (_enumStrSystemMode[1] = SFP(HUIStr_Enum_SystemMode_1)).c_str();
    #else
        enumStrGPSPolling[0] = CFP(HUIStr_Enum_GPSPolling_0);
        enumStrGPSPolling[1] = CFP(HUIStr_Enum_GPSPolling_1);
        enumStrGPSPolling[2] = CFP(HUIStr_Enum_GPSPolling_2);
        enumStrGPSPolling[3] = CFP(HUIStr_Enum_GPSPolling_3);
        enumStrTimeZone[0] = CFP(HUIStr_Enum_TimeZone_0);
        enumStrTimeZone[1] = CFP(HUIStr_Enum_TimeZone_1);
        enumStrTimeZone[2] = CFP(HUIStr_Enum_TimeZone_2);
        enumStrTimeZone[3] = CFP(HUIStr_Enum_TimeZone_3);
        enumStrTimeZone[4] = CFP(HUIStr_Enum_TimeZone_4);
        enumStrTimeZone[5] = CFP(HUIStr_Enum_TimeZone_5);
        enumStrTimeZone[6] = CFP(HUIStr_Enum_TimeZone_6);
        enumStrTimeZone[7] = CFP(HUIStr_Enum_TimeZone_7);
        enumStrTimeZone[8] = CFP(HUIStr_Enum_TimeZone_8);
        enumStrTimeZone[9] = CFP(HUIStr_Enum_TimeZone_9);
        enumStrTimeZone[10] = CFP(HUIStr_Enum_TimeZone_10);
        enumStrTimeZone[11] = CFP(HUIStr_Enum_TimeZone_11);
        enumStrTimeZone[12] = CFP(HUIStr_Enum_TimeZone_12);
        enumStrTimeZone[13] = CFP(HUIStr_Enum_TimeZone_13);
        enumStrTimeZone[14] = CFP(HUIStr_Enum_TimeZone_14);
        enumStrTimeZone[15] = CFP(HUIStr_Enum_TimeZone_15);
        enumStrTimeZone[16] = CFP(HUIStr_Enum_TimeZone_16);
        enumStrTimeZone[17] = CFP(HUIStr_Enum_TimeZone_17);
        enumStrTimeZone[18] = CFP(HUIStr_Enum_TimeZone_18);
        enumStrTimeZone[19] = CFP(HUIStr_Enum_TimeZone_19);
        enumStrTimeZone[20] = CFP(HUIStr_Enum_TimeZone_20);
        enumStrTimeZone[21] = CFP(HUIStr_Enum_TimeZone_21);
        enumStrTimeZone[22] = CFP(HUIStr_Enum_TimeZone_22);
        enumStrTimeZone[23] = CFP(HUIStr_Enum_TimeZone_23);
        enumStrTimeZone[24] = CFP(HUIStr_Enum_TimeZone_24);
        enumStrTimeZone[25] = CFP(HUIStr_Enum_TimeZone_25);
        enumStrTimeZone[26] = CFP(HUIStr_Enum_TimeZone_26);
        enumStrTimeZone[27] = CFP(HUIStr_Enum_TimeZone_27);
        enumStrTimeZone[28] = CFP(HUIStr_Enum_TimeZone_28);
        enumStrTimeZone[29] = CFP(HUIStr_Enum_TimeZone_29);
        enumStrTimeZone[30] = CFP(HUIStr_Enum_TimeZone_30);
        enumStrTimeZone[31] = CFP(HUIStr_Enum_TimeZone_31);
        enumStrTimeZone[32] = CFP(HUIStr_Enum_TimeZone_32);
        enumStrTimeZone[33] = CFP(HUIStr_Enum_TimeZone_33);
        enumStrTimeZone[34] = CFP(HUIStr_Enum_TimeZone_34);
        enumStrTimeZone[35] = CFP(HUIStr_Enum_TimeZone_35);
        enumStrTimeZone[36] = CFP(HUIStr_Enum_TimeZone_36);
        enumStrTimeZone[37] = CFP(HUIStr_Enum_TimeZone_37);
        enumStrTimeZone[38] = CFP(HUIStr_Enum_TimeZone_38);
        enumStrTimeZone[39] = CFP(HUIStr_Enum_TimeZone_39);
        enumStrTimeZone[40] = CFP(HUIStr_Enum_TimeZone_40);
        enumStrDataPolling[0] = CFP(HUIStr_Enum_DataPolling_0);
        enumStrDataPolling[1] = CFP(HUIStr_Enum_DataPolling_1);
        enumStrDataPolling[2] = CFP(HUIStr_Enum_DataPolling_2);
        enumStrDataPolling[3] = CFP(HUIStr_Enum_DataPolling_3);
        enumStrDataPolling[4] = CFP(HUIStr_Enum_DataPolling_4);
        enumStrDataPolling[5] = CFP(HUIStr_Enum_DataPolling_5);
        enumStrDataPolling[6] = CFP(HUIStr_Enum_DataPolling_6);
        enumStrDataPolling[7] = CFP(HUIStr_Enum_DataPolling_7);
        enumStrAutosave[0] = CFP(HUIStr_Enum_Autosave_0);
        enumStrAutosave[1] = CFP(HUIStr_Enum_Autosave_1);
        enumStrAutosave[2] = CFP(HUIStr_Enum_Autosave_2);
        enumStrAutosave[3] = CFP(HUIStr_Enum_Autosave_3);
        enumStrMeasurements[0] = CFP(HUIStr_Enum_Measurements_0);
        enumStrMeasurements[1] = CFP(HUIStr_Enum_Measurements_1);
        enumStrMeasurements[2] = CFP(HUIStr_Enum_Measurements_2);
        enumStrSystemMode[0] = CFP(HUIStr_Enum_SystemMode_0);
        enumStrSystemMode[1] = CFP(HUIStr_Enum_SystemMode_1);
    #endif

    InitAnyMenuInfo(minfoOverview, HUIStr_Item_BackToOverview, 7, NO_ADDRESS, 0, backToOverview);
    InitBooleanMenuInfo(minfoToggleBadConn, HUIStr_Item_ToggleBadConn, 65, NO_ADDRESS, 1, NO_CALLBACK, NAMING_ON_OFF);
    InitBooleanMenuInfo(minfoToggleFastTime, HUIStr_Item_ToggleFastTime, 64, NO_ADDRESS, 1, NO_CALLBACK, NAMING_ON_OFF);
    InitAnyMenuInfo(minfoTriggerSigTime, HUIStr_Item_TriggerSigTime, 63, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoTriggerSDCleanup, HUIStr_Item_TriggerSDCleanup, 62, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoTriggerLowMem, HUIStr_Item_TriggerLowMem, 61, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoTriggerAutosave, HUIStr_Item_TriggerAutosave, 60, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoDebug, HUIStr_Item_Debug, 6, NO_ADDRESS, 0, NO_CALLBACK);
    InitBooleanMenuInfo(minfoBatteryFailure, HUIStr_Item_BatteryFailure, 530, NO_ADDRESS, 1, NO_CALLBACK, NAMING_YES_NO);
    InitSubMenuInfo(minfoRTC, HUIStr_Item_RTC, 53, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoEEPROM, HUIStr_Item_EEPROM, 52, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoSDCard, HUIStr_Item_SDCard, 51, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoSystem, HUIStr_Item_System, 50, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoInfo, HUIStr_Item_Info, 5, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoAddNewCalibration, HUIStr_Item_AddNew, 421, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoBrowseCalibrations, HUIStr_Item_Browse, 420, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoCalibrations, HUIStr_Item_Calibrations, 42, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoAddNewAdditive, HUIStr_Item_AddNew, 411, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoBrowseAdditives, HUIStr_Item_Browse, 410, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoAdditives, HUIStr_Item_Additives, 41, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoAddNewCropLib, HUIStr_Item_AddNew, 401, NO_ADDRESS, 0, NO_CALLBACK);
    InitAnyMenuInfo(minfoBrowseCropsLib, HUIStr_Item_Browse, 400, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoCropsLib, HUIStr_Item_Crops, 40, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoLibrary, HUIStr_Item_Library, 4, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoControls, HUIStr_Item_Controls, 34, NO_ADDRESS, 0, NO_CALLBACK);
    InitEnumMenuInfo(minfoGPSPolling, HUIStr_Item_GPSPolling, 333, NO_ADDRESS, 3, pollingChanged, enumStrGPSPolling);
    InitAnalogMenuInfoUnits(minfoAltitude, HUIStr_Item_Altitude, 332, NO_ADDRESS, 10000, altChanged, 0, 1, HUIStr_Unit_MSL);
    InitSubMenuInfo(minfoLocation, HUIStr_Item_Location, 33, NO_ADDRESS, 0, NO_CALLBACK);
    InitBooleanMenuInfo(minfoAllowRemoteCtrl, HUIStr_Item_AllowRemoteCtrl, 329, NO_ADDRESS, 1, allowRemoteChanged, NAMING_CHECKBOX);
    InitAnalogMenuInfo(minfoRemoteCtrlPort, HUIStr_Item_RemoteCtrlPort, 328, NO_ADDRESS, 65535, remotePortChanged, 0, 1);
    InitAnalogMenuInfo(minfoBrokerPort, HUIStr_Item_BrokerPort, 327, NO_ADDRESS, 65535, brokerPortChanged, 0, 1);
    InitBooleanMenuInfo(minfoAssignByHostname, HUIStr_Item_AssignByHostname, 326, NO_ADDRESS, 1, brokerByChanged, NAMING_CHECKBOX);
    InitBooleanMenuInfo(minfoAssignByDHCP, HUIStr_Item_AssignByDHCP, 321, NO_ADDRESS, 1, ipByChanged, NAMING_CHECKBOX);
    InitSubMenuInfo(minfoNetworking, HUIStr_Item_Networking, 32, NO_ADDRESS, 0, NO_CALLBACK);
    InitBooleanMenuInfo(minfoDSTAddHour, HUIStr_Item_DSTAddHour, 313, NO_ADDRESS, 1, dstChanged, NAMING_CHECKBOX);
    InitEnumMenuInfo(minfoTimeZone, HUIStr_Item_TimeZone, 312, NO_ADDRESS, 40, tzChanged, enumStrTimeZone);
    InitSubMenuInfo(minfoTime, HUIStr_Item_Time, 31, NO_ADDRESS, 0, NO_CALLBACK);
    InitEnumMenuInfo(minfoDataPolling, HUIStr_Item_DataPolling, 305, NO_ADDRESS, 7, pollingDTChanged, enumStrDataPolling);
    InitEnumMenuInfo(minfoAutosaveSeconday, HUIStr_Item_AutosaveSecondary, 304, NO_ADDRESS, 3, secondaryAutosaveChanged, enumStrAutosave);
    InitEnumMenuInfo(minfoAutosavePrimary, HUIStr_Item_AutosavePrimary, 303, NO_ADDRESS, 3, primaryAutosaveChanged, enumStrAutosave);
    InitEnumMenuInfo(minfoMeasurements, HUIStr_Item_Measurements, 302, NO_ADDRESS, 2, sysMeasureChanged, enumStrMeasurements);
    InitEnumMenuInfo(minfoSystemMode, HUIStr_Item_SystemMode, 301, NO_ADDRESS, 1, sysModeChanged, enumStrSystemMode);
    InitSubMenuInfo(minfoGeneral, HUIStr_Item_General, 30, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoSettings, HUIStr_Item_Settings, 3, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoScheduling, HUIStr_Item_Scheduling, 25, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoPowerRails, HUIStr_Item_PowerRails, 24, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoReservoirs, HUIStr_Item_Reservoirs, 23, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoCrops, HUIStr_Item_Crops, 22, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoSensors, HUIStr_Item_Sensors, 21, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoActuators, HUIStr_Item_Actuators, 20, NO_ADDRESS, 0, NO_CALLBACK);
    InitSubMenuInfo(minfoSystem, HUIStr_Item_System, 2, NO_ADDRESS, 0, NO_CALLBACK);
}

HydroHomeMenuItems::HydroHomeMenuItems()
    : init(),
      menuOverview(&init.minfoOverview, NULL, INFO_LOCATION_RAM),
      menuToggleBadConn(&init.minfoToggleBadConn, false, NULL, INFO_LOCATION_RAM),
      menuToggleFastTime(&init.minfoToggleFastTime, false, &menuToggleBadConn, INFO_LOCATION_RAM),
      menuTriggerSigTime(&init.minfoTriggerSigTime, &menuToggleFastTime, INFO_LOCATION_RAM),
      menuTriggerSDCleanup(&init.minfoTriggerSDCleanup, &menuTriggerSigTime, INFO_LOCATION_RAM),
      menuTriggerLowMem(&init.minfoTriggerLowMem, &menuTriggerSDCleanup, INFO_LOCATION_RAM),
      menuTriggerAutosave(&init.minfoTriggerAutosave, &menuTriggerLowMem, INFO_LOCATION_RAM),
      menuBackDebug(&init.minfoDebug, &menuTriggerAutosave, INFO_LOCATION_RAM),
      menuDebug(&init.minfoDebug, &menuBackDebug, &menuOverview, INFO_LOCATION_RAM),
      menuIoTMonitor(pgmStrIoTMonitorText, 54, NULL),
      menuBatteryFailure(&init.minfoBatteryFailure, false, NULL, INFO_LOCATION_RAM),
      menuBackRTC(&init.minfoRTC, &menuBatteryFailure, INFO_LOCATION_RAM),
      menuRTC(&init.minfoRTC, &menuBackRTC, &menuIoTMonitor, INFO_LOCATION_RAM),
      menuEEPROMSize(fnEEPROMSizeRtCall, HStr_Blank, 520, 16, NULL),
      menuBackEEPROM(&init.minfoEEPROM, &menuEEPROMSize, INFO_LOCATION_RAM),
      menuEEPROM(&init.minfoEEPROM, &menuBackEEPROM, &menuRTC, INFO_LOCATION_RAM),
      menuSDName(fnSDNameRtCall, HStr_Blank, 510, 16, NULL),
      menuBackSDCard(&init.minfoSDCard, &menuSDName, INFO_LOCATION_RAM),
      menuSDCard(&init.minfoSDCard, &menuBackSDCard, &menuEEPROM, INFO_LOCATION_RAM),
      menuDisplayMode(fnDisplayModeRtCall, HStr_Blank, 505, 16, NULL),
      menuControlMode(fnControlModeRtCall, HStr_Blank, 504, 16, &menuDisplayMode),
      menuFreeMemory(fnFreeMemoryRtCall, HStr_Blank, 503, 16, &menuControlMode),
      menuUptime(fnUptimeRtCall, HStr_Blank, 502, 16, &menuFreeMemory),
      menuFirmware(fnFirmwareRtCall, HStr_Blank, 501, 16, &menuUptime),
      menuBoard(fnBoardRtCall, HStr_Blank, 500, 16, &menuFirmware),
      menuBackSystemInfo(&init.minfoSystemInfo, &menuBoard, INFO_LOCATION_RAM),
      menuSystemInfo(&init.minfoSystemInfo, &menuBackSystem, &menuSDCard, INFO_LOCATION_RAM),
      menuBackInfo(&init.minfoInfo, &menuSystem, INFO_LOCATION_RAM),
      menuInfo(&init.minfoInfo, &menuBackInfo, &menuDebug, INFO_LOCATION_RAM),
      minfoAddNewCalibration(&init.minfoAddNewCalibration, NULL, INFO_LOCATION_RAM),
      menuBrowseCalibrations(&init.minfoBrowseCalibrations, &minfoAddNewCalibration, INFO_LOCATION_RAM),
      menuBackCalibrations(&init.minfoCalibrations, &menuBrowseCalibrations, INFO_LOCATION_RAM),
      menuCalibrations(&init.minfoCalibrations, &menuBackCalibrations, NULL, INFO_LOCATION_RAM),
      menuAddNewAdditives(&init.minfoAddNewAdditive, NULL, INFO_LOCATION_RAM),
      menuBrowseAdditives(&init.minfoBrowseAdditives, &menuAddNewAdditives, INFO_LOCATION_RAM),
      menuBackAdditives(&init.minfoAdditives, &menuBrowseAdditives, INFO_LOCATION_RAM),
      menuAdditives(&init.minfoAdditives, &menuBackAdditives, &menuCalibrations, INFO_LOCATION_RAM),
      menuAddNewCropsLib(&init.minfoAddNewCropLib, NULL, INFO_LOCATION_RAM),
      menuBrowseCropsLib(&init.minfoBrowseCropsLib, &menuAddNewCropsLib, INFO_LOCATION_RAM),
      menuBackCropsLib(&init.minfoCropsLib, &menuBrowseCropsLib, INFO_LOCATION_RAM),
      menuCropsLib(&init.minfoCropsLib, &menuBackCrops, &menuAdditives, INFO_LOCATION_RAM),
      menuBackLibrary(&init.minfoLibrary, &menuCrops, INFO_LOCATION_RAM),
      menuLibrary(&init.minfoLibrary, &menuBackLibrary, &menuInfo, INFO_LOCATION_RAM),
      menuAuthenticator(pgmStrAuthenticatorText, NO_CALLBACK, 35, NULL),
      menuJoystickYTol(fnJoystickYTolRtCall, LargeFixedNumber(12, 4, 0U, 500U, false), 346, false, NULL),
      menuJoystickYMid(fnJoystickYMidRtCall, LargeFixedNumber(12, 4, 0U, 5000U, false), 345, false, &menuJoystickYTol),
      menuJoystickXTol(fnJoystickXTolRtCall, LargeFixedNumber(12, 4, 0U, 500U, false), 344, false, &menuJoystickYMid),
      menuJoystickXMid(fnJoystickXMidRtCall, LargeFixedNumber(12, 4, 0U, 5000U, false), 343, false, &menuJoystickXTol),
      menuBackControls(&init.minfoControls, &menuJoystickXMid, INFO_LOCATION_RAM),
      menuControls(&init.minfoControls, &menuBackControls, &menuAuthenticator, INFO_LOCATION_RAM),
      menuGPSPolling(&init.minfoGPSPolling, 0, NULL, INFO_LOCATION_RAM),
      menuAltitude(&init.minfoAltitude, 0, &menuGPSPolling, INFO_LOCATION_RAM),
      menuLongitudeMin(fnLongitudeMinRtCall, LargeFixedNumber(12, 4, 0U, 0U, false), 331, true, &menuAltitude),
      menuLatitudeDeg(fnLatitudeDegRtCall, LargeFixedNumber(12, 6, 0U, 0U, false), 330, true, &menuLongitudeMin),
      menuBackLocation(&init.minfoLocation, &menuLatitudeDeg, INFO_LOCATION_RAM),
      menuLocation(&init.minfoLocation, &menuBackLocation, &menuControls, INFO_LOCATION_RAM),
      menuAllowRemoteCtrl(&init.minfoAllowRemoteCtrl, true, NULL, INFO_LOCATION_RAM),
      menuRemoteCtrlPort(&init.minfoRemoteCtrlPort, 3333, &menuAllowRemoteCtrl, INFO_LOCATION_RAM),
      menuBrokerPort(&init.minfoBrokerPort, 1883, &menuRemoteCtrlPort, INFO_LOCATION_RAM),
      menuAssignByHostname(&init.minfoAssignByHostname, false, &menuBrokerPort, INFO_LOCATION_RAM),
      menuMQTTBroker(fnMQTTBrokerRtCall, IpAddressStorage(127, 0, 0, 1), 325, &menuAssignByHostname),
      menuWiFiPassword(fnWiFiPasswordRtCall, HStr_Blank, 324, 24, &menuMQTTBroker),
      menuWiFiSSID(fnWiFiSSIDRtCall, HStr_Blank, 323, 24, &menuWiFiPassword),
      menuMACAddress(fnMACAddressRtCall, HStr_Blank, 322, 12, &menuWiFiSSID),
      menuAssignByDHCP(&init.minfoAssignByDHCP, true, &menuMACAddress, INFO_LOCATION_RAM),
      menuControllerIP(fnControllerIPRtCall, IpAddressStorage(127, 0, 0, 1), 320, &menuAssignByDHCP),
      menuBackNetworking(&init.minfoNetworking, &menuControllerIP, INFO_LOCATION_RAM),
      menuNetworking(&init.minfoNetworking, &menuBackNetworking, &menuLocation, INFO_LOCATION_RAM),
      menuDSTAddHour(&init.minfoDSTAddHour, false, NULL, INFO_LOCATION_RAM),
      menuTimeZone(&init.minfoTimeZone, 15, &menuDSTAddHour, INFO_LOCATION_RAM),
      menuLocalTime(fnLocalTimeRtCall, TimeStorage(localNow().hour(), localNow().minute(), localNow().second(), 0), 311, (MultiEditWireType)2, &menuTimeZone),
      menuDate(fnDateRtCall, DateStorage(localNow().day(), localNow().month(), localNow().year()), 310, &menuLocalTime),
      menuBackTime(&init.minfoTime, &menuDate, INFO_LOCATION_RAM),
      menuTime(&init.minfoTime, &menuBackTime, &menuNetworking, INFO_LOCATION_RAM),
      menuDataPolling(&init.minfoDataPolling, 1, NULL, INFO_LOCATION_RAM),
      menuAutosaveSeconday(&init.minfoAutosaveSeconday, 3, &menuDataPolling, INFO_LOCATION_RAM),
      menuAutosavePrimary(&init.minfoAutosavePrimary, 3, &menuAutosaveSeconday, INFO_LOCATION_RAM),
      menuMeasurements(&init.minfoMeasurements, 1, &menuAutosavePrimary, INFO_LOCATION_RAM),
      menuSystemMode(&init.minfoSystemMode, 0, &menuMeasurements, INFO_LOCATION_RAM),
      menuSystemName(fnSystemNameRtCall, getController() ? getController()->getSystemNameChars() : HStr_Blank, 300, 24, &menuSystemMode),
      menuBackGeneral(&init.minfoGeneral, &menuSystemName, INFO_LOCATION_RAM),
      menuGeneral(&init.minfoGeneral, &menuBackGeneral, &menuTime, INFO_LOCATION_RAM),
      menuBackSettings(&init.minfoSettings, &menuGeneral, INFO_LOCATION_RAM),
      menuSettings(&init.minfoSettings, &menuBackSettings, &menuLibrary, INFO_LOCATION_RAM),
      menuBackScheduling(&init.minfoScheduling, NULL, INFO_LOCATION_RAM),
      menuScheduling(&init.minfoScheduling, &menuBackScheduling, NULL, INFO_LOCATION_RAM),
      menuBackPowerRails(&init.minfoPowerRails, NULL, INFO_LOCATION_RAM),
      menuPowerRails(&init.minfoPowerRails, &menuBackPowerRails, &menuScheduling, INFO_LOCATION_RAM),
      menuBackReservoirs(&init.minfoReservoirs, NULL, INFO_LOCATION_RAM),
      menuReservoirs(&init.minfoReservoirs, &menuBackReservoirs, &menuPowerRails, INFO_LOCATION_RAM),
      menuBackCrops(&init.minfoCrops, NULL, INFO_LOCATION_RAM),
      menuCrops(&init.minfoCrops, &menuBackCrops, &menuReservoirs, INFO_LOCATION_RAM),
      menuBackSensors(&init.minfoSensors, NULL, INFO_LOCATION_RAM),
      menuSensors(&init.minfoSensors, &menuBackSensors, &menuCrops, INFO_LOCATION_RAM),
      menuBackActuators(&init.minfoActuators, NULL, INFO_LOCATION_RAM),
      menuActuators(&init.minfoActuators, &menuBackActuators, &menuSensors, INFO_LOCATION_RAM),
      menuBackSystem(&init.minfoSystem, &menuActuators, INFO_LOCATION_RAM),
      menuSystem(&init.minfoSystem, &menuBackSystem, &menuSettings, INFO_LOCATION_RAM)
{
    menuControlMode.setReadOnly(true);
    menuAutosavePrimary.setReadOnly(true);
    menuAssignByDHCP.setReadOnly(true);
    menuFreeMemory.setReadOnly(true);
    menuAutosaveSeconday.setReadOnly(true);
    menuEEPROMSize.setReadOnly(true);
    menuAssignByHostname.setReadOnly(true);
    menuRemoteCtrlPort.setReadOnly(true);
    menuBrokerPort.setReadOnly(true);
    menuBoard.setReadOnly(true);
    menuMQTTBroker.setReadOnly(true);
    menuFirmware.setReadOnly(true);
    menuControllerIP.setReadOnly(true);
    menuOverview.setReadOnly(true);
    menuDisplayMode.setReadOnly(true);
    menuBatteryFailure.setReadOnly(true);
    menuSDName.setReadOnly(true);
    menuUptime.setReadOnly(true);
    menuAuthenticator.setLocalOnly(true);
    menuIoTMonitor.setLocalOnly(true);
    menuJoystickYMid.setVisible(false);
    menuWiFiPassword.setVisible(false);
    menuJoystickXTol.setVisible(false);
    menuJoystickYTol.setVisible(false);
    menuMACAddress.setVisible(false);
    menuAssignByHostname.setVisible(false);
    menuBrokerPort.setVisible(false);
    menuJoystickXMid.setVisible(false);
    menuWiFiSSID.setVisible(false);
    menuMQTTBroker.setVisible(false);
    menuAltitude.setStep(100);

    if (getBaseUI() && getBaseUI()->getRemoteServer()) {
        menuIoTMonitor.setRemoteServer(*(getBaseUI()->getRemoteServer()));
    }
    menuAuthenticator.init();
}

#endif
