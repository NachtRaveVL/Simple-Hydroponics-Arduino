/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Strings
*/

#include "Hydruino.h"

#ifndef HYDRO_DISABLE_BUILTIN_DATA
String stringFromPGMAddr(const char *flashStr);
const char *pgmAddrForStr(Hydro_String strNum);
#endif

static uint16_t _strDataAddress((uint16_t)-1);
void beginStringsFromEEPROM(uint16_t dataAddress)
{
    _strDataAddress = dataAddress;
}

static String _strDataFilePrefix;
void beginStringsFromSDCard(String dataFilePrefix)
{
    _strDataFilePrefix = dataFilePrefix;
}

inline String getStringsFilename()
{
    String filename; filename.reserve(_strDataFilePrefix.length() + 12);
    filename.concat(_strDataFilePrefix);
    filename.concat('s'); // Cannot use SFP here so have to do it the long way
    filename.concat('t');
    filename.concat('r');
    filename.concat('i');
    filename.concat('n');
    filename.concat('g');
    filename.concat('s');
    filename.concat('.');
    filename.concat('d');
    filename.concat('a');
    filename.concat('t');
    return filename;
}

String stringFromPGM(Hydro_String strNum)
{    
    static Hydro_String _lookupStrNum = (Hydro_String)-1; // Simple LRU cache reduces a lot of lookup access
    static String _lookupCachedRes;
    if (strNum == _lookupStrNum) { return _lookupCachedRes; }
    else { _lookupStrNum = strNum; } // _lookupCachedRes set below

    if (_strDataAddress != (uint16_t)-1) {
        auto eeprom = getController()->getEEPROM();

        if (eeprom) {
            uint16_t lookupOffset = 0;
            eeprom->readBlock(_strDataAddress + (sizeof(uint16_t) * ((int)strNum + 1)), // +1 for initial total size word
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

    if (_strDataFilePrefix.length()) {
        #if HYDRO_SYS_LEAVE_FILES_OPEN
            static
        #endif
        auto sd = getController()->getSDCard();

        if (sd) {
            String retVal;
            #if HYDRO_SYS_LEAVE_FILES_OPEN
                static
            #endif
            auto file = sd->open(getStringsFilename().c_str(), FILE_READ);

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
        return (_lookupCachedRes = stringFromPGMAddr(pgmAddrForStr(strNum)));
    #else
        return (_lookupCachedRes = String());
    #endif
}

String stringFromPGMAddr(const char *flashStr) {
    String retVal; retVal.reserve(strlen_P(flashStr) + 1);
    char buffer[HYDRO_STRING_BUFFER_SIZE] = {0};
    strncpy_P(buffer, flashStr, HYDRO_STRING_BUFFER_SIZE);
    retVal.concat(charsToString(buffer, HYDRO_STRING_BUFFER_SIZE));

    while (strnlen(buffer, HYDRO_STRING_BUFFER_SIZE) == HYDRO_STRING_BUFFER_SIZE) {
        flashStr += HYDRO_STRING_BUFFER_SIZE;
        strncpy_P(buffer, flashStr, HYDRO_STRING_BUFFER_SIZE);
        if (buffer[0]) { retVal.concat(charsToString(buffer, HYDRO_STRING_BUFFER_SIZE)); }
    }

    return retVal;
}

#ifndef HYDRO_DISABLE_BUILTIN_DATA

const char *pgmAddrForStr(Hydro_String strNum)
{
    switch(strNum) {
        case HStr_ColonSpace: {
            static const char flashStr_ColonSpace[] PROGMEM = {": "};
            return flashStr_ColonSpace;
        } break;
        case HStr_DoubleSpace: {
            static const char flashStr_DoubleSpace[] PROGMEM = {"  "};
            return flashStr_DoubleSpace;
        } break;
        case HStr_Count: {
            static const char flashStr_Count[] PROGMEM = {"Count"};
            return flashStr_Count;
        } break;
        case HStr_csv: {
            static const char flashStr_csv[] PROGMEM = {"csv"};
            return flashStr_csv;
        } break;
        case HStr_dat: {
            static const char flashStr_dat[] PROGMEM = {"dat"};
            return flashStr_dat;
        } break;
        case HStr_Disabled: {
            static const char flashStr_Disabled[] PROGMEM = {"Disabled"};
            return flashStr_Disabled;
        } break;
        case HStr_raw: {
            static const char flashStr_raw[] PROGMEM = {"raw"};
            return flashStr_raw;
        } break;
        case HStr_txt: {
            static const char flashStr_txt[] PROGMEM = {"txt"};
            return flashStr_txt;
        } break;
        case HStr_Undefined: {
            static const char flashStr_Undefined[] PROGMEM = {"Undefined"};
            return flashStr_Undefined;
        } break;
        case HStr_null: {
            static const char flashStr_null[] PROGMEM = {"null"};
            return flashStr_null;
        } break;

        case HStr_Default_SystemName: {
            static const char flashStr_Default_SystemName[] PROGMEM = {"Hydruino"};
            return flashStr_Default_SystemName;
        } break;
        case HStr_Default_ConfigFilename: {
            static const char flashStr_Default_ConfigFilename[] PROGMEM = {"hydruino.cfg"};
            return flashStr_Default_ConfigFilename;
        } break;

        case HStr_Err_AllocationFailure: {
            static const char flashStr_Err_AllocationFailure[] PROGMEM = {"Allocation failure"};
            return flashStr_Err_AllocationFailure;
        } break;
        case HStr_Err_AlreadyInitialized: {
            static const char flashStr_Err_AlreadyInitialized[] PROGMEM = {"Already initialized"};
            return flashStr_Err_AlreadyInitialized;
        } break;
        case HStr_Err_AssertionFailure: {
            static const char flashStr_Err_AssertionFailure[] PROGMEM = {"Assertion failure"};
            return flashStr_Err_AssertionFailure;
        } break;
        case HStr_Err_ExportFailure: {
            static const char flashStr_Err_ExportFailure[] PROGMEM = {"Export failure"};
            return flashStr_Err_ExportFailure;
        } break;
        case HStr_Err_ImportFailure: {
            static const char flashStr_Err_ImportFailure[] PROGMEM = {"Import failure"};
            return flashStr_Err_ImportFailure;
        } break;
        case HStr_Err_InitializationFailure: {
            static const char flashStr_Err_InitializationFailure[] PROGMEM = {"Initialization failure"};
            return flashStr_Err_InitializationFailure;
        } break;
        case HStr_Err_InvalidParameter: {
            static const char flashStr_Err_InvalidParameter[] PROGMEM = {"Invalid parameter"};
            return flashStr_Err_InvalidParameter;
        } break;
        case HStr_Err_InvalidPinOrType: {
            static const char flashStr_Err_InvalidPinOrType[] PROGMEM = {"Invalid pin or type"};
            return flashStr_Err_InvalidPinOrType;
        } break;
        case HStr_Err_MeasurementFailure: {
            static const char flashStr_Err_MeasurementFailure[] PROGMEM = {"Measurement failure"};
            return flashStr_Err_MeasurementFailure;
        } break;
        case HStr_Err_MissingLinkage: {
            static const char flashStr_Err_MissingLinkage[] PROGMEM = {"Missing or no linkage"};
            return flashStr_Err_MissingLinkage;
        } break;
        case HStr_Err_NoPositionsAvailable: {
            static const char flashStr_Err_NoPositionsAvailable[] PROGMEM = {"No positions available"};
            return flashStr_Err_NoPositionsAvailable;
        } break;
        case HStr_Err_NotYetInitialized: {
            static const char flashStr_Err_NotYetInitialized[] PROGMEM = {"Not yet initialized"};
            return flashStr_Err_NotYetInitialized;
        } break;
        case HStr_Err_OperationFailure: {
            static const char flashStr_Err_OperationFailure[] PROGMEM = {"Operation failure"};
            return flashStr_Err_OperationFailure;
        } break;
        case HStr_Err_UnsupportedOperation: {
            static const char flashStr_Err_UnsupportedOperation[] PROGMEM = {"Unsupported operation"};
            return flashStr_Err_UnsupportedOperation;
        } break;

        case HStr_Log_AirReport: {
            static const char flashStr_Log_AirReport[] PROGMEM = {" air report:"};
            return flashStr_Log_AirReport;
        } break;
        case HStr_Log_CalculatedPumping: {
            static const char flashStr_Log_CalculatedPumping[] PROGMEM = {" pumping request:"};
            return flashStr_Log_CalculatedPumping;
        } break;
        case HStr_Log_FeedingSequence: {
            static const char flashStr_Log_FeedingSequence[] PROGMEM = {" feeding sequence"};
            return flashStr_Log_FeedingSequence;
        } break;
        case HStr_Log_HasBegan: {
            static const char flashStr_Log_HasBegan[] PROGMEM = {" has began"};
            return flashStr_Log_HasBegan;
        } break;
        case HStr_Log_HasDisabled: {
            static const char flashStr_Log_HasDisabled[] PROGMEM = {" has disabled"};
            return flashStr_Log_HasDisabled;
        } break;
        case HStr_Log_HasEnabled: {
            static const char flashStr_Log_HasEnabled[] PROGMEM = {" has enabled"};
            return flashStr_Log_HasEnabled;
        } break;
        case HStr_Log_HasEnded: {
            static const char flashStr_Log_HasEnded[] PROGMEM = {" has ended"};
            return flashStr_Log_HasEnded;
        } break;
        case HStr_Log_LightingSequence: {
            static const char flashStr_Log_LightingSequence[] PROGMEM = {" lighting sequence"};
            return flashStr_Log_LightingSequence;
        } break;
        case HStr_Log_MeasuredPumping: {
            static const char flashStr_Log_MeasuredPumping[] PROGMEM = {" pumping result:"};
            return flashStr_Log_MeasuredPumping;
        } break;
        case HStr_Log_PreFeedBalancing: {
            static const char flashStr_Log_PreFeedBalancing[] PROGMEM = {" pre-feed balancing"};
            return flashStr_Log_PreFeedBalancing;
        } break;
        case HStr_Log_PreFeedTopOff: {
            static const char flashStr_Log_PreFeedTopOff[] PROGMEM = {" pre-feed top-off"};
            return flashStr_Log_PreFeedTopOff;
        } break;
        case HStr_Log_PreDawnSpraying: {
            static const char flashStr_Log_PreDawnSpraying[] PROGMEM = {" pre-dawn spraying"};
            return flashStr_Log_PreDawnSpraying;
        } break;
        case HStr_Log_RTCBatteryFailure: {
            static const char flashStr_Log_RTCBatteryFailure[] PROGMEM = {"RTC battery failure, time needs reset."};
            return flashStr_Log_RTCBatteryFailure;
        } break;
        case HStr_Log_SystemDataSaved: {
            static const char flashStr_Log_SystemDataSaved[] PROGMEM = {"System data saved"};
            return flashStr_Log_SystemDataSaved;
        } break;
        case HStr_Log_SystemUptime: {
            static const char flashStr_Log_SystemUptime[] PROGMEM = {"System uptime: "};
            return flashStr_Log_SystemUptime;
        } break;

        case HStr_Log_Prefix_Info: {
            static const char flashStr_Prefix_Info[] PROGMEM = {"[INFO] "};
            return flashStr_Prefix_Info;
        } break;
        case HStr_Log_Prefix_Warning: {
            static const char flashStr_Log_Prefix_Warning[] PROGMEM = {"[WARN] "};
            return flashStr_Log_Prefix_Warning;
        } break;
        case HStr_Log_Prefix_Error: {
            static const char flashStr_Log_Prefix_Error[] PROGMEM = {"[FAIL] "};
            return flashStr_Log_Prefix_Error;
        } break;

        case HStr_Log_Field_Aerator_Duration: {
            static const char flashStr_Log_Field_Aerator_Duration[] PROGMEM = {"  Aerator run time: "};
            return flashStr_Log_Field_Aerator_Duration;
        } break;
        case HStr_Log_Field_CO2_Measured: {
            static const char flashStr_Log_Field_CO2_Measured[] PROGMEM = {"  CO2 sensor: "};
            return flashStr_Log_Field_CO2_Measured;
        } break;
        case HStr_Log_Field_CO2_Setpoint: {
            static const char flashStr_Log_Field_CO2_Setpoint[] PROGMEM = {"  CO2 setpoint: "};
            return flashStr_Log_Field_CO2_Setpoint;
        } break;
        case HStr_Log_Field_Destination_Reservoir: {
            static const char flashStr_Log_Field_Destination_Reservoir[] PROGMEM = {"  To reservoir: "};
            return flashStr_Log_Field_Destination_Reservoir;
        } break;
        case HStr_Log_Field_Light_Duration: {
            static const char flashStr_Log_Field_Light_Duration[] PROGMEM = {"  Daylight hours: "};
            return flashStr_Log_Field_Light_Duration;
        } break;
        case HStr_Log_Field_MixTime_Duration: {
            static const char flashStr_Log_Field_MixTime_Duration[] PROGMEM = {"  Dosing mix time: "};
            return flashStr_Log_Field_MixTime_Duration;
        } break;
        case HStr_Log_Field_pH_Measured: {
            static const char flashStr_Log_Field_pH_Measured[] PROGMEM = {"  pH sensor: "};
            return flashStr_Log_Field_pH_Measured;
        } break;
        case HStr_Log_Field_pH_Setpoint: {
            static const char flashStr_Log_Field_pH_Setpoint[] PROGMEM = {"  ph setpoint: "};
            return flashStr_Log_Field_pH_Setpoint;
        } break;
        case HStr_Log_Field_Source_Reservoir: {
            static const char flashStr_Log_Field_Source_Reservoir[] PROGMEM = {"  From reservoir: "};
            return flashStr_Log_Field_Source_Reservoir;
        } break;
        case HStr_Log_Field_Sprayer_Duration: {
            static const char flashStr_Log_Field_Sprayer_Duration[] PROGMEM = {"  Sprayer run time: "};
            return flashStr_Log_Field_Sprayer_Duration;
        } break;
        case HStr_Log_Field_TDS_Measured: {
            static const char flashStr_Log_Field_TDS_Measured[] PROGMEM = {"  TDS sensor: "};
            return flashStr_Log_Field_TDS_Measured;
        } break;
        case HStr_Log_Field_TDS_Setpoint: {
            static const char flashStr_Log_Field_TDS_Setpoint[] PROGMEM = {"  TDS setpoint: "};
            return flashStr_Log_Field_TDS_Setpoint;
        } break;
        case HStr_Log_Field_Temp_Measured: {
            static const char flashStr_Log_Field_Temp_Measured[] PROGMEM = {"  Temp sensor: "};
            return flashStr_Log_Field_Temp_Measured;
        } break;
        case HStr_Log_Field_Temp_Setpoint: {
            static const char flashStr_Log_Field_Temp_Setpoint[] PROGMEM = {"  Temp setpoint: "};
            return flashStr_Log_Field_Temp_Setpoint;
        } break;
        case HStr_Log_Field_Time_Calculated: {
            static const char flashStr_Log_Field_Time_Calculated[] PROGMEM = {"  Pump run time: "};
            return flashStr_Log_Field_Time_Calculated;
        } break;
        case HStr_Log_Field_Time_Finish: {
            static const char flashStr_Log_Field_Time_Finish[] PROGMEM = {"  Finish time: "};
            return flashStr_Log_Field_Time_Finish;
        } break;
        case HStr_Log_Field_Time_Measured: {
            static const char flashStr_Log_Field_Time_Measured[] PROGMEM = {"  Elapsed time: "};
            return flashStr_Log_Field_Time_Measured;
        } break;
        case HStr_Log_Field_Time_Start: {
            static const char flashStr_Log_Field_Time_Start[] PROGMEM = {"  Start time: "};
            return flashStr_Log_Field_Time_Start;
        } break;
        case HStr_Log_Field_Vol_Calculated: {
            static const char flashStr_Log_Field_Vol_Calculated[] PROGMEM = {"  Est. pumped vol.: "};
            return flashStr_Log_Field_Vol_Calculated;
        } break;
        case HStr_Log_Field_Vol_Measured: {
            static const char flashStr_Log_Field_Vol_Measured[] PROGMEM = {"  Act. pumped vol.: "};
            return flashStr_Log_Field_Vol_Measured;
        } break;

        case HStr_Key_ActiveLow: {
            static const char flashStr_Key_ActiveLow[] PROGMEM = {"activeLow"};
            return flashStr_Key_ActiveLow;
        } break;
        case HStr_Key_AdditiveName: {
            static const char flashStr_Key_AdditiveName[] PROGMEM = {"additiveName"};
            return flashStr_Key_AdditiveName;
        } break;
        case HStr_Key_AirConcentrateUnits: {
            static const char flashStr_Key_AirConcentrateUnits[] PROGMEM = {"airConcentrateUnits"};
            return flashStr_Key_AirConcentrateUnits;
        } break;
        case HStr_Key_AirReportInterval: {
            static const char flashStr_Key_AirReportInterval[] PROGMEM = {"airReportInterval"};
            return flashStr_Key_AirReportInterval;
        } break;
        case HStr_Key_AirTempRange: {
            static const char flashStr_Key_AirTempRange[] PROGMEM = {"airTempRange"};
            return flashStr_Key_AirTempRange;
        } break;
        case HStr_Key_AirTemperatureSensor: {
            static const char flashStr_Key_AirTemperatureSensor[] PROGMEM = {"airTempSensor"};
            return flashStr_Key_AirTemperatureSensor;
        } break;
        case HStr_Key_Altitude: {
            static const char flashStr_Key_Altitude[] PROGMEM = {"altitude"};
            return flashStr_Key_Altitude;
        } break;
        case HStr_Key_AlwaysFilled: {
            static const char flashStr_Key_AlwaysFilled[] PROGMEM = {"alwaysFilled"};
            return flashStr_Key_AlwaysFilled;
        } break;
        case HStr_Key_AutosaveEnabled: {
            static const char flashStr_Key_AutosaveEnabled[] PROGMEM = {"autosaveEnabled"};
            return flashStr_Key_AutosaveEnabled;
        } break;
        case HStr_Key_AutosaveFallback: {
            static const char flashStr_Key_AutosaveFallback[] PROGMEM = {"autosaveFallback"};
            return flashStr_Key_AutosaveFallback;
        } break;
        case HStr_Key_AutosaveInterval: {
            static const char flashStr_Key_AutosaveInterval[] PROGMEM = {"autosaveInterval"};
            return flashStr_Key_AutosaveInterval;
        } break;
        case HStr_Key_BaseFeedMultiplier: {
            static const char flashStr_Key_BaseFeedMultiplier[] PROGMEM = {"baseFeedMultiplier"};
            return flashStr_Key_BaseFeedMultiplier;
        } break;
        case HStr_Key_BitRes: {
            static const char flashStr_Key_BitRes[] PROGMEM = {"bitRes"};
            return flashStr_Key_BitRes;
        } break;
        case HStr_Key_CalibrationUnits: {
            static const char flashStr_Key_CalibrationUnits[] PROGMEM = {"calibrationUnits"};
            return flashStr_Key_CalibrationUnits;
        } break;
        case HStr_Key_Channel: {
            static const char flashStr_Key_Channel[] PROGMEM = {"channel"};
            return flashStr_Key_Channel;
        } break;
        case HStr_Key_ChannelPins: {
            static const char flashStr_Key_ChannelPins[] PROGMEM = {"channelPins"};
            return flashStr_Key_ChannelPins;
        } break;
        case HStr_Key_ChipEnablePin: {
            static const char flashStr_Key_ChipEnablePin[] PROGMEM = {"chipEnablePin"};
            return flashStr_Key_ChipEnablePin;
        } break;
        case HStr_Key_ComputeHeatIndex: {
            static const char flashStr_Key_ComputeHeatIndex[] PROGMEM = {"computeHeatIndex"};
            return flashStr_Key_ComputeHeatIndex;
        } break;
        case HStr_Key_ConcentrateUnits: {
            static const char flashStr_Key_ConcentrateUnits[] PROGMEM = {"concentrateUnits"};
            return flashStr_Key_ConcentrateUnits;
        } break;
        case HStr_Key_ContinuousFlowRate: {
            static const char flashStr_Key_ContinuousFlowRate[] PROGMEM = {"contFlowRate"};
            return flashStr_Key_ContinuousFlowRate;
        } break;
        case HStr_Key_ContinuousPowerUsage: {
            static const char flashStr_Key_ContinuousPowerUsage[] PROGMEM = {"contPowerUsage"};
            return flashStr_Key_ContinuousPowerUsage;
        } break;
        case HStr_Key_CO2Levels: {
            static const char flashStr_Key_CO2Levels[] PROGMEM = {"co2Levels"};
            return flashStr_Key_CO2Levels;
        } break;
        case HStr_Key_CO2Sensor: {
            static const char flashStr_Key_CO2Sensor[] PROGMEM = {"co2Sensor"};
            return flashStr_Key_CO2Sensor;
        } break;
        case HStr_Key_CropName: {
            static const char flashStr_Key_CropName[] PROGMEM = {"cropName"};
            return flashStr_Key_CropName;
        } break;
        case HStr_Key_CtrlInMode: {
            static const char flashStr_Key_CtrlInMode[] PROGMEM = {"ctrlInMode"};
            return flashStr_Key_CtrlInMode;
        } break;
        case HStr_Key_DailyLightHours: {
            static const char flashStr_Key_DailyLightHours[] PROGMEM = {"dailyLightHours"};
            return flashStr_Key_DailyLightHours;
        } break;
        case HStr_Key_DataFilePrefix: {
            static const char flashStr_Key_DataFilePrefix[] PROGMEM = {"dataFilePrefix"};
            return flashStr_Key_DataFilePrefix;
        } break;
        case HStr_Key_DetriggerDelay: {
            static const char flashStr_Key_DetriggerDelay[] PROGMEM = {"detriggerDelay"};
            return flashStr_Key_DetriggerDelay;
        } break;
        case HStr_Key_DetriggerTol: {
            static const char flashStr_Key_DetriggerTol[] PROGMEM = {"detriggerTol"};
            return flashStr_Key_DetriggerTol;
        } break;
        case HStr_Key_DHTType: {
            static const char flashStr_Key_DHTType[] PROGMEM = {"dhtType"};
            return flashStr_Key_DHTType;
        } break;
        case HStr_Key_DispOutMode: {
            static const char flashStr_Key_DispOutMode[] PROGMEM = {"dispOutMode"};
            return flashStr_Key_DispOutMode;
        } break;
        case HStr_Key_EmptyTrigger: {
            static const char flashStr_Key_EmptyTrigger[] PROGMEM = {"emptyTrigger"};
            return flashStr_Key_EmptyTrigger;
        } break;
        case HStr_Key_EnableMode: {
            static const char flashStr_Key_EnableMode[] PROGMEM = {"enableMode"};
            return flashStr_Key_EnableMode;
        } break;
        case HStr_Key_FeedReservoir: {
            static const char flashStr_Key_FeedReservoir[] PROGMEM = {"feedReservoir"};
            return flashStr_Key_FeedReservoir;
        } break;
        case HStr_Key_FeedTimingMins: {
            static const char flashStr_Key_FeedTimingMins[] PROGMEM = {"feedTimingMins"};
            return flashStr_Key_FeedTimingMins;
        } break;
        case HStr_Key_FeedingTrigger: {
            static const char flashStr_Key_FeedingTrigger[] PROGMEM = {"feedingTrigger"};
            return flashStr_Key_FeedingTrigger;
        } break;
        case HStr_Key_FeedingWeight: {
            static const char flashStr_Key_FeedingWeight[] PROGMEM = {"feedingWeight"};
            return flashStr_Key_FeedingWeight;
        } break;
        case HStr_Key_FilledTrigger: {
            static const char flashStr_Key_FilledTrigger[] PROGMEM = {"filledTrigger"};
            return flashStr_Key_FilledTrigger;
        } break;
        case HStr_Key_Flags: {
            static const char flashStr_Key_Flags[] PROGMEM = {"flags"};
            return flashStr_Key_Flags;
        } break;
        case HStr_Key_FlowRateSensor: {
            static const char flashStr_Key_FlowRateSensor[] PROGMEM = {"flowRateSensor"};
            return flashStr_Key_FlowRateSensor;
        } break;
        case HStr_Key_FlowRateUnits: {
            static const char flashStr_Key_FlowRateUnits[] PROGMEM = {"flowRateUnits"};
            return flashStr_Key_FlowRateUnits;
        } break;
        case HStr_Key_Id: {
            static const char flashStr_Key_Id[] PROGMEM = {"id"};
            return flashStr_Key_Id;
        } break;
        case HStr_Key_InputInversion: {
            static const char flashStr_Key_InputInversion[] PROGMEM = {"inputInversion"};
            return flashStr_Key_InputInversion;
        } break;
        case HStr_Key_InputPin: {
            static const char flashStr_Key_InputPin[] PROGMEM = {"inputPin"};
            return flashStr_Key_InputPin;
        } break;
        case HStr_Key_Invasive: {
            static const char flashStr_Key_Invasive[] PROGMEM = {"invasive"};
            return flashStr_Key_Invasive;
        } break;
        case HStr_Key_Large: {
            static const char flashStr_Key_Large[] PROGMEM = {"large"};
            return flashStr_Key_Large;
        } break;
        case HStr_Key_LastChangeTime: {
            static const char flashStr_Key_LastChangeTime[] PROGMEM = {"lastChangeTime"};
            return flashStr_Key_LastChangeTime;
        } break;
        case HStr_Key_LastFeedingTime: {
            static const char flashStr_Key_LastFeedingTime[] PROGMEM = {"lastFeedingTime"};
            return flashStr_Key_LastFeedingTime;
        } break;
        case HStr_Key_LastPruningTime: {
            static const char flashStr_Key_LastPruningTime[] PROGMEM = {"lastPruningTime"};
            return flashStr_Key_LastPruningTime;
        } break;
        case HStr_Key_Latitude: {
            static const char flashStr_Key_Latitude[] PROGMEM = {"latitude"};
            return flashStr_Key_Latitude;
        } break;
        case HStr_Key_LimitTrigger: {
            static const char flashStr_Key_LimitTrigger[] PROGMEM = {"limitTrigger"};
            return flashStr_Key_LimitTrigger;
        } break;
        case HStr_Key_LogFilePrefix: {
            static const char flashStr_Key_LogFilePrefix[] PROGMEM = {"logFilePrefix"};
            return flashStr_Key_LogFilePrefix;
        } break;
        case HStr_Key_LogLevel: {
            static const char flashStr_Key_LogLevel[] PROGMEM = {"logLevel"};
            return flashStr_Key_LogLevel;
        } break;
        case HStr_Key_LogToSDCard: {
            static const char flashStr_Key_LogToSDCard[] PROGMEM = {"logToSDCard"};
            return flashStr_Key_LogToSDCard;
        } break;
        case HStr_Key_LogToWiFiStorage: {
            static const char flashStr_Key_LogToWiFiStorage[] PROGMEM = {"logToWiFiStorage"};
            return flashStr_Key_LogToWiFiStorage;
        } break;
        case HStr_Key_Logger: {
            static const char flashStr_Key_Logger[] PROGMEM = {"logger"};
            return flashStr_Key_Logger;
        } break;
        case HStr_Key_Longitude: {
            static const char flashStr_Key_Longitude[] PROGMEM = {"longitude"};
            return flashStr_Key_Longitude;
        } break;
        case HStr_Key_MACAddress: {
            static const char flashStr_Key_MACAddress[] PROGMEM = {"macAddress"};
            return flashStr_Key_MACAddress;
        } break;
        case HStr_Key_MaxActiveAtOnce: {
            static const char flashStr_Key_MaxActiveAtOnce[] PROGMEM = {"maxActiveAtOnce"};
            return flashStr_Key_MaxActiveAtOnce;
        } break;
        case HStr_Key_MaxPower: {
            static const char flashStr_Key_MaxPower[] PROGMEM = {"maxPower"};
            return flashStr_Key_MaxPower;
        } break;
        case HStr_Key_MaxVolume: {
            static const char flashStr_Key_MaxVolume[] PROGMEM = {"maxVolume"};
            return flashStr_Key_MaxVolume;
        } break;
        case HStr_Key_MeasureMode: {
            static const char flashStr_Key_MeasureMode[] PROGMEM = {"measureMode"};
            return flashStr_Key_MeasureMode;
        } break;
        case HStr_Key_MeasurementRow: {
            static const char flashStr_Key_MeasurementRow[] PROGMEM = {"measurementRow"};
            return flashStr_Key_MeasurementRow;
        } break;
        case HStr_Key_MeasurementUnits: {
            static const char flashStr_Key_MeasurementUnits[] PROGMEM = {"measurementUnits"};
            return flashStr_Key_MeasurementUnits;
        } break;
        case HStr_Key_Mode: {
            static const char flashStr_Key_Mode[] PROGMEM = {"mode"};
            return flashStr_Key_Mode;
        } break;
        case HStr_Key_MoistureSensor: {
            static const char flashStr_Key_MoistureSensor[] PROGMEM = {"moistureSensor"};
            return flashStr_Key_MoistureSensor;
        } break;
        case HStr_Key_Multiplier: {
            static const char flashStr_Key_Multiplier[] PROGMEM = {"multiplier"};
            return flashStr_Key_Multiplier;
        } break;
        case HStr_Key_NightlyFeedRate: {
            static const char flashStr_Key_NightlyFeedRate[] PROGMEM = {"nightlyFeedRate"};
            return flashStr_Key_NightlyFeedRate;
        } break;
        case HStr_Key_NumFeedingsToday: {
            static const char flashStr_Key_NumFeedingsToday[] PROGMEM = {"numFeedingsToday"};
            return flashStr_Key_NumFeedingsToday;
        } break;
        case HStr_Key_Offset: {
            static const char flashStr_Key_Offset[] PROGMEM = {"offset"};
            return flashStr_Key_Offset;
        } break;
        case HStr_Key_OutputPin: {
            static const char flashStr_Key_OutputPin[] PROGMEM = {"outputPin"};
            return flashStr_Key_OutputPin;
        } break;
        case HStr_Key_OutputReservoir: {
            static const char flashStr_Key_OutputReservoir[] PROGMEM = {"destReservoir"};
            return flashStr_Key_OutputReservoir;
        } break;
        case HStr_Key_Perennial: {
            static const char flashStr_Key_Perennial[] PROGMEM = {"perennial"};
            return flashStr_Key_Perennial;
        } break;
        case HStr_Key_PhaseDurationWeeks: {
            static const char flashStr_Key_PhaseDurationWeeks[] PROGMEM = {"phaseDurationWeeks"};
            return flashStr_Key_PhaseDurationWeeks;
        } break;
        case HStr_Key_PHRange: {
            static const char flashStr_Key_PHRange[] PROGMEM = {"phRange"};
            return flashStr_Key_PHRange;
        } break;
        case HStr_Key_PHSensor: {
            static const char flashStr_Key_PHSensor[] PROGMEM = {"phSensor"};
            return flashStr_Key_PHSensor;
        } break;
        case HStr_Key_Pin: {
            static const char flashStr_Key_Pin[] PROGMEM = {"pin"};
            return flashStr_Key_Pin;
        } break;
        case HStr_Key_PollingInterval: {
            static const char flashStr_Key_PollingInterval[] PROGMEM = {"pollingInterval"};
            return flashStr_Key_PollingInterval;
        } break;
        case HStr_Key_PowerSensor: {
            static const char flashStr_Key_PowerSensor[] PROGMEM = {"powerSensor"};
            return flashStr_Key_PowerSensor;
        } break;
        case HStr_Key_PowerUnits: {
            static const char flashStr_Key_PowerUnits[] PROGMEM = {"powerUnits"};
            return flashStr_Key_PowerUnits;
        } break;
        case HStr_Key_PreFeedAeratorMins: {
            static const char flashStr_Key_PreFeedAeratorMins[] PROGMEM = {"preFeedAeratorMins"};
            return flashStr_Key_PreFeedAeratorMins;
        } break;
        case HStr_Key_PreDawnSprayMins: {
            static const char flashStr_Key_PreDawnSprayMins[] PROGMEM = {"preDawnSprayMins"};
            return flashStr_Key_PreDawnSprayMins;
        } break;
        case HStr_Key_Pruning: {
            static const char flashStr_Key_Pruning[] PROGMEM = {"pruning"};
            return flashStr_Key_Pruning;
        } break;
        case HStr_Key_PublishToSDCard: {
            static const char flashStr_Key_PublishToSDCard[] PROGMEM = {"pubToSDCard"};
            return flashStr_Key_PublishToSDCard;
        } break;
        case HStr_Key_PublishToWiFiStorage: {
            static const char flashStr_Key_PublishToWiFiStorage[] PROGMEM = {"pubToWiFiStorage"};
            return flashStr_Key_PublishToWiFiStorage;
        } break;
        case HStr_Key_Publisher: {
            static const char flashStr_Key_Publisher[] PROGMEM = {"publisher"};
            return flashStr_Key_Publisher;
        } break;
        case HStr_Key_PullupPin: {
            static const char flashStr_Key_PullupPin[] PROGMEM = {"pullupPin"};
            return flashStr_Key_PullupPin;
        } break;
        case HStr_Key_PWMChannel: {
            static const char flashStr_Key_PWMChannel[] PROGMEM = {"pwmChannel"};
            return flashStr_Key_PWMChannel;
        } break;
        case HStr_Key_PWMFrequency: {
            static const char flashStr_Key_PWMFrequency[] PROGMEM = {"pwmFrequency"};
            return flashStr_Key_PWMFrequency;
        } break;
        case HStr_Key_RailName: {
            static const char flashStr_Key_RailName[] PROGMEM = {"railName"};
            return flashStr_Key_RailName;
        } break;
        case HStr_Key_ReservoirName: {
            static const char flashStr_Key_ReservoirName[] PROGMEM = {"reservoirName"};
            return flashStr_Key_ReservoirName;
        } break;
        case HStr_Key_Revision: {
            static const char flashStr_Key_Revision[] PROGMEM = {"revision"};
            return flashStr_Key_Revision;
        } break;
        case HStr_Key_Scheduler: {
            static const char flashStr_Key_Scheduler[] PROGMEM = {"scheduler"};
            return flashStr_Key_Scheduler;
        } break;
        case HStr_Key_SensorName: {
            static const char flashStr_Key_SensorName[] PROGMEM = {"sensorName"};
            return flashStr_Key_SensorName;
        } break;
        case HStr_Key_SignalPin: {
            static const char flashStr_Key_SignalPin[] PROGMEM = {"signalPin"};
            return flashStr_Key_SignalPin;
        } break;
        case HStr_Key_SowTime: {
            static const char flashStr_Key_SowTime[] PROGMEM = {"sowTime"};
            return flashStr_Key_SowTime;
        } break;
        case HStr_Key_Spraying: {
            static const char flashStr_Key_Spraying[] PROGMEM = {"spraying"};
            return flashStr_Key_Spraying;
        } break;
        case HStr_Key_State: {
            static const char flashStr_Key_State[] PROGMEM = {"state"};
            return flashStr_Key_State;
        } break;
        case HStr_Key_StdDosingRates: {
            static const char flashStr_Key_StdDosingRates[] PROGMEM = {"stdDosingRates"};
            return flashStr_Key_StdDosingRates;
        } break;
        case HStr_Key_SubstrateType: {
            static const char flashStr_Key_SubstrateType[] PROGMEM = {"substrateType"};
            return flashStr_Key_SubstrateType;
        } break;
        case HStr_Key_SystemMode: {
            static const char flashStr_Key_SystemMode[] PROGMEM = {"systemMode"};
            return flashStr_Key_SystemMode;
        } break;
        case HStr_Key_SystemName: {
            static const char flashStr_Key_SystemName[] PROGMEM = {"systemName"};
            return flashStr_Key_SystemName;
        } break;
        case HStr_Key_TDSRange: {
            static const char flashStr_Key_TDSRange[] PROGMEM = {"tdsRange"};
            return flashStr_Key_TDSRange;
        } break;
        case HStr_Key_TDSSensor: {
            static const char flashStr_Key_TDSSensor[] PROGMEM = {"tdsSensor"};
            return flashStr_Key_TDSSensor;
        } break;
        case HStr_Key_TemperatureUnits: {
            static const char flashStr_Key_TemperatureUnits[] PROGMEM = {"temperatureUnits"};
            return flashStr_Key_TemperatureUnits;
        } break;
        case HStr_Key_TemperatureSensor: {
            static const char flashStr_Key_TemperatureSensor[] PROGMEM = {"tempSensor"};
            return flashStr_Key_TemperatureSensor;
        } break;
        case HStr_Key_TimeZoneOffset: {
            static const char flashStr_Key_TimeZoneOffset[] PROGMEM = {"timeZoneOffset"};
            return flashStr_Key_TimeZoneOffset;
        } break;
        case HStr_Key_Timestamp: {
            static const char flashStr_Key_Timestamp[] PROGMEM = {"timestamp"};
            return flashStr_Key_Timestamp;
        } break;
        case HStr_Key_Tolerance: {
            static const char flashStr_Key_Tolerance[] PROGMEM = {"tolerance"};
            return flashStr_Key_Tolerance;
        } break;
        case HStr_Key_ToleranceHigh: {
            static const char flashStr_Key_ToleranceHigh[] PROGMEM = {"toleranceHigh"};
            return flashStr_Key_ToleranceHigh;
        } break;
        case HStr_Key_ToleranceLow: {
            static const char flashStr_Key_ToleranceLow[] PROGMEM = {"toleranceLow"};
            return flashStr_Key_ToleranceLow;
        } break;
        case HStr_Key_TotalFeedingsDay: {
            static const char flashStr_Key_TotalFeedingsDay[] PROGMEM = {"totalFeedingsDay"};
            return flashStr_Key_TotalFeedingsDay;
        } break;
        case HStr_Key_TotalGrowWeeks: {
            static const char flashStr_Key_TotalGrowWeeks[] PROGMEM = {"totalGrowWeeks"};
            return flashStr_Key_TotalGrowWeeks;
        } break;
        case HStr_Key_Toxic: {
            static const char flashStr_Key_Toxic[] PROGMEM = {"toxic"};
            return flashStr_Key_Toxic;
        } break;
        case HStr_Key_TriggerBelow: {
            static const char flashStr_Key_TriggerBelow[] PROGMEM = {"triggerBelow"};
            return flashStr_Key_TriggerBelow;
        } break;
        case HStr_Key_TriggerOutside: {
            static const char flashStr_Key_TriggerOutside[] PROGMEM = {"triggerOutside"};
            return flashStr_Key_TriggerOutside;
        } break;
        case HStr_Key_Type: {
            static const char flashStr_Key_Type[] PROGMEM = {"type"};
            return flashStr_Key_Type;
        } break;
        case HStr_Key_Units: {
            static const char flashStr_Key_Units[] PROGMEM = {"units"};
            return flashStr_Key_Units;
        } break;
        case HStr_Key_UsingISR: {
            static const char flashStr_Key_UsingISR[] PROGMEM = {"usingISR"};
            return flashStr_Key_UsingISR;
        } break;
        case HStr_Key_Value: {
            static const char flashStr_Key_Value[] PROGMEM = {"value"};
            return flashStr_Key_Value;
        } break;
        case HStr_Key_Version: {
            static const char flashStr_Key_Version[] PROGMEM = {"version"};
            return flashStr_Key_Version;
        } break;
        case HStr_Key_Viner: {
            static const char flashStr_Key_Viner[] PROGMEM = {"viner"};
            return flashStr_Key_Viner;
        } break;
        case HStr_Key_VolumeSensor: {
            static const char flashStr_Key_VolumeSensor[] PROGMEM = {"volumeSensor"};
            return flashStr_Key_VolumeSensor;
        } break;
        case HStr_Key_VolumeUnits: {
            static const char flashStr_Key_VolumeUnits[] PROGMEM = {"volumeUnits"};
            return flashStr_Key_VolumeUnits;
        } break;
        case HStr_Key_WaterConcentrateUnits: {
            static const char flashStr_Key_WaterConcentrateUnits[] PROGMEM = {"waterConcentrateUnits"};
            return flashStr_Key_WaterConcentrateUnits;
        } break;
        case HStr_Key_WaterTemperatureRange: {
            static const char flashStr_Key_WaterTemperatureRange[] PROGMEM = {"waterTempRange"};
            return flashStr_Key_WaterTemperatureRange;
        } break;
        case HStr_Key_WaterTemperatureSensor: {
            static const char flashStr_Key_WaterTemperatureSensor[] PROGMEM = {"waterTempSensor"};
            return flashStr_Key_WaterTemperatureSensor;
        } break;
        case HStr_Key_WeeklyDosingRates: {
            static const char flashStr_Key_WeeklyDosingRates[] PROGMEM = {"weeklyDosingRates"};
            return flashStr_Key_WeeklyDosingRates;
        } break;
        case HStr_Key_WiFiPassword: {
            static const char flashStr_Key_WiFiPassword[] PROGMEM = {"wifiPassword"};
            return flashStr_Key_WiFiPassword;
        } break;
        case HStr_Key_WiFiPasswordSeed: {
            static const char flashStr_Key_WiFiPasswordSeed[] PROGMEM = {"wifiPwSeed"};
            return flashStr_Key_WiFiPasswordSeed;
        } break;
        case HStr_Key_WiFiSSID: {
            static const char flashStr_Key_WiFiSSID[] PROGMEM = {"wifiSSID"};
            return flashStr_Key_WiFiSSID;
        } break;
        case HStr_Key_WireDevAddress: {
            static const char flashStr_Key_WireDevAddress[] PROGMEM = {"wireDevAddress"};
            return flashStr_Key_WireDevAddress;
        } break;
        case HStr_Key_WirePosIndex: {
            static const char flashStr_Key_WirePosIndex[] PROGMEM = {"wirePosIndex"};
            return flashStr_Key_WirePosIndex;
        } break;

        case HStr_Enum_16x2LCD: {
            static const char flashStr_Enum_16x2LCD[] PROGMEM = {"16x2LCD"};
            return flashStr_Enum_16x2LCD;
        } break;
        case HStr_Enum_16x2LCDSwapped: {
            static const char flashStr_Enum_16x2LCDSwapped[] PROGMEM = {"16x2LCDSwapped"};
            return flashStr_Enum_16x2LCDSwapped;
        } break;
        case HStr_Enum_20x4LCD: {
            static const char flashStr_Enum_20x4LCD[] PROGMEM = {"20x4LCD"};
            return flashStr_Enum_20x4LCD;
        } break;
        case HStr_Enum_20x4LCDSwapped: {
            static const char flashStr_Enum_20x4LCDSwapped[] PROGMEM = {"20x4LCDSwapped"};
            return flashStr_Enum_20x4LCDSwapped;
        } break;
        case HStr_Enum_2x2Matrix: {
            static const char flashStr_Enum_2x2Matrix[] PROGMEM = {"2x2Matrix"};
            return flashStr_Enum_2x2Matrix;
        } break;
        case HStr_Enum_4xButton: {
            static const char flashStr_Enum_4xButton[] PROGMEM = {"4xButton"};
            return flashStr_Enum_4xButton;
        } break;
        case HStr_Enum_6xButton: {
            static const char flashStr_Enum_6xButton[] PROGMEM = {"6xButton"};
            return flashStr_Enum_6xButton;
        } break;
        case HStr_Enum_AC110V: {
            static const char flashStr_Enum_AC110V[] PROGMEM = {"AC110V"};
            return flashStr_Enum_AC110V;
        } break;
        case HStr_Enum_AC220V: {
            static const char flashStr_Enum_AC220V[] PROGMEM = {"AC220V"};
            return flashStr_Enum_AC220V;
        } break;
        case HStr_Enum_AirCarbonDioxide: {
            static const char flashStr_Enum_AirCarbonDioxide[] PROGMEM = {"AirCO2"};
            return flashStr_Enum_AirCarbonDioxide;
        } break;
        case HStr_Enum_AirTemperatureHumidity: {
            static const char flashStr_Enum_AirTemperatureHumidity[] PROGMEM = {"AirTempHumidity"};
            return flashStr_Enum_AirTemperatureHumidity;
        } break;
        case HStr_Enum_Alkalinity: {
            static const char flashStr_Enum_Alkalinity[] PROGMEM = {"Alkalinity"};
            return flashStr_Enum_Alkalinity;
        } break;
        case HStr_Enum_AloeVera: {
            static const char flashStr_Enum_AloeVera[] PROGMEM = {"AloeVera"};
            return flashStr_Enum_AloeVera;
        } break;
        case HStr_Enum_AnalogInput: {
            static const char flashStr_Enum_AnalogInput[] PROGMEM = {"AnalogInput"};
            return flashStr_Enum_AnalogInput;
        } break;
        case HStr_Enum_AnalogOutput: {
            static const char flashStr_Enum_AnalogOutput[] PROGMEM = {"AnalogOutput"};
            return flashStr_Enum_AnalogOutput;
        } break;
        case HStr_Enum_Anise: {
            static const char flashStr_Enum_Anise[] PROGMEM = {"Anise"};
            return flashStr_Enum_Anise;
        } break;
        case HStr_Enum_Artichoke: {
            static const char flashStr_Enum_Artichoke[] PROGMEM = {"Artichoke"};
            return flashStr_Enum_Artichoke;
        } break;
        case HStr_Enum_Arugula: {
            static const char flashStr_Enum_Arugula[] PROGMEM = {"Arugula"};
            return flashStr_Enum_Arugula;
        } break;
        case HStr_Enum_AscOrder: {
            static const char flashStr_Enum_AscOrder[] PROGMEM = {"AscOrder"};
            return flashStr_Enum_AscOrder;
        } break;
        case HStr_Enum_Asparagus: {
            static const char flashStr_Enum_Asparagus[] PROGMEM = {"Asparagus"};
            return flashStr_Enum_Asparagus;
        } break;
        case HStr_Enum_Average: {
            static const char flashStr_Enum_Average[] PROGMEM = {"Average"};
            return flashStr_Enum_Average;
        } break;
        case HStr_Enum_Basil: {
            static const char flashStr_Enum_Basil[] PROGMEM = {"Basil"};
            return flashStr_Enum_Basil;
        } break;
        case HStr_Enum_Bean: {
            static const char flashStr_Enum_Bean[] PROGMEM = {"Bean"};
            return flashStr_Enum_Bean;
        } break;
        case HStr_Enum_BeanBroad: {
            static const char flashStr_Enum_BeanBroad[] PROGMEM = {"BeanBroad"};
            return flashStr_Enum_BeanBroad;
        } break;
        case HStr_Enum_Beetroot: {
            static const char flashStr_Enum_Beetroot[] PROGMEM = {"Beetroot"};
            return flashStr_Enum_Beetroot;
        } break;
        case HStr_Enum_BlackCurrant: {
            static const char flashStr_Enum_BlackCurrant[] PROGMEM = {"BlackCurrant"};
            return flashStr_Enum_BlackCurrant;
        } break;
        case HStr_Enum_Blueberry: {
            static const char flashStr_Enum_Blueberry[] PROGMEM = {"Blueberry"};
            return flashStr_Enum_Blueberry;
        } break;
        case HStr_Enum_BokChoi: {
            static const char flashStr_Enum_BokChoi[] PROGMEM = {"BokChoi"};
            return flashStr_Enum_BokChoi;
        } break;
        case HStr_Enum_Broccoli: {
            static const char flashStr_Enum_Broccoli[] PROGMEM = {"Broccoli"};
            return flashStr_Enum_Broccoli;
        } break;
        case HStr_Enum_BrusselsSprout: {
            static const char flashStr_Enum_BrusselsSprout[] PROGMEM = {"BrusselsSprout"};
            return flashStr_Enum_BrusselsSprout;
        } break;
        case HStr_Enum_Cabbage: {
            static const char flashStr_Enum_Cabbage[] PROGMEM = {"Cabbage"};
            return flashStr_Enum_Cabbage;
        } break;
        case HStr_Enum_Cannabis: {
            static const char flashStr_Enum_Cannabis[] PROGMEM = {"Cannabis"};
            return flashStr_Enum_Cannabis;
        } break;
        case HStr_Enum_Capsicum: {
            static const char flashStr_Enum_Capsicum[] PROGMEM = {"Capsicum"};
            return flashStr_Enum_Capsicum;
        } break;
        case HStr_Enum_Carrots: {
            static const char flashStr_Enum_Carrots[] PROGMEM = {"Carrots"};
            return flashStr_Enum_Carrots;
        } break;
        case HStr_Enum_Catnip: {
            static const char flashStr_Enum_Catnip[] PROGMEM = {"Catnip"};
            return flashStr_Enum_Catnip;
        } break;
        case HStr_Enum_Cauliflower: {
            static const char flashStr_Enum_Cauliflower[] PROGMEM = {"Cauliflower"};
            return flashStr_Enum_Cauliflower;
        } break;
        case HStr_Enum_Celery: {
            static const char flashStr_Enum_Celery[] PROGMEM = {"Celery"};
            return flashStr_Enum_Celery;
        } break;
        case HStr_Enum_Chamomile: {
            static const char flashStr_Enum_Chamomile[] PROGMEM = {"Chamomile"};
            return flashStr_Enum_Chamomile;
        } break;
        case HStr_Enum_Chicory: {
            static const char flashStr_Enum_Chicory[] PROGMEM = {"Chicory"};
            return flashStr_Enum_Chicory;
        } break;
        case HStr_Enum_Chives: {
            static const char flashStr_Enum_Chives[] PROGMEM = {"Chives"};
            return flashStr_Enum_Chives;
        } break;
        case HStr_Enum_Cilantro: {
            static const char flashStr_Enum_Cilantro[] PROGMEM = {"Cilantro"};
            return flashStr_Enum_Cilantro;
        } break;
        case HStr_Enum_ClayPebbles: {
            static const char flashStr_Enum_ClayPebbles[] PROGMEM = {"ClayPebbles"};
            return flashStr_Enum_ClayPebbles;
        } break;
        case HStr_Enum_CoconutCoir: {
            static const char flashStr_Enum_CoconutCoir[] PROGMEM = {"CoconutCoir"};
            return flashStr_Enum_CoconutCoir;
        } break;
        case HStr_Enum_Concentration: {
            static const char flashStr_Enum_Concentration[] PROGMEM = {"Concentration"};
            return flashStr_Enum_Concentration;
        } break;
        case HStr_Enum_Coriander: {
            static const char flashStr_Enum_Coriander[] PROGMEM = {"Coriander"};
            return flashStr_Enum_Coriander;
        } break;
        case HStr_Enum_CornSweet: {
            static const char flashStr_Enum_CornSweet[] PROGMEM = {"CornSweet"};
            return flashStr_Enum_CornSweet;
        } break;
        case HStr_Enum_Cucumber: {
            static const char flashStr_Enum_Cucumber[] PROGMEM = {"Cucumber"};
            return flashStr_Enum_Cucumber;
        } break;
        case HStr_Enum_CustomAdditive1: {
            static const char flashStr_Enum_CustomAdditive1[] PROGMEM = {"CustomAdditive1"};
            return flashStr_Enum_CustomAdditive1;
        } break;
        case HStr_Enum_CustomAdditive2: {
            static const char flashStr_Enum_CustomAdditive2[] PROGMEM = {"CustomAdditive2"};
            return flashStr_Enum_CustomAdditive2;
        } break;
        case HStr_Enum_CustomAdditive3: {
            static const char flashStr_Enum_CustomAdditive3[] PROGMEM = {"CustomAdditive3"};
            return flashStr_Enum_CustomAdditive3;
        } break;
        case HStr_Enum_CustomAdditive4: {
            static const char flashStr_Enum_CustomAdditive4[] PROGMEM = {"CustomAdditive4"};
            return flashStr_Enum_CustomAdditive4;
        } break;
        case HStr_Enum_CustomAdditive5: {
            static const char flashStr_Enum_CustomAdditive5[] PROGMEM = {"CustomAdditive5"};
            return flashStr_Enum_CustomAdditive5;
        } break;
        case HStr_Enum_CustomAdditive6: {
            static const char flashStr_Enum_CustomAdditive6[] PROGMEM = {"CustomAdditive6"};
            return flashStr_Enum_CustomAdditive6;
        } break;
        case HStr_Enum_CustomAdditive7: {
            static const char flashStr_Enum_CustomAdditive7[] PROGMEM = {"CustomAdditive7"};
            return flashStr_Enum_CustomAdditive7;
        } break;
        case HStr_Enum_CustomAdditive8: {
            static const char flashStr_Enum_CustomAdditive8[] PROGMEM = {"CustomAdditive8"};
            return flashStr_Enum_CustomAdditive8;
        } break;
        case HStr_Enum_CustomAdditive9: {
            static const char flashStr_Enum_CustomAdditive9[] PROGMEM = {"CustomAdditive9"};
            return flashStr_Enum_CustomAdditive9;
        } break;
        case HStr_Enum_CustomAdditive10: {
            static const char flashStr_Enum_CustomAdditive10[] PROGMEM = {"CustomAdditive10"};
            return flashStr_Enum_CustomAdditive10;
        } break;
        case HStr_Enum_CustomAdditive11: {
            static const char flashStr_Enum_CustomAdditive11[] PROGMEM = {"CustomAdditive11"};
            return flashStr_Enum_CustomAdditive11;
        } break;
        case HStr_Enum_CustomAdditive12: {
            static const char flashStr_Enum_CustomAdditive12[] PROGMEM = {"CustomAdditive12"};
            return flashStr_Enum_CustomAdditive12;
        } break;
        case HStr_Enum_CustomAdditive13: {
            static const char flashStr_Enum_CustomAdditive13[] PROGMEM = {"CustomAdditive13"};
            return flashStr_Enum_CustomAdditive13;
        } break;
        case HStr_Enum_CustomAdditive14: {
            static const char flashStr_Enum_CustomAdditive14[] PROGMEM = {"CustomAdditive14"};
            return flashStr_Enum_CustomAdditive14;
        } break;
        case HStr_Enum_CustomAdditive15: {
            static const char flashStr_Enum_CustomAdditive15[] PROGMEM = {"CustomAdditive15"};
            return flashStr_Enum_CustomAdditive15;
        } break;
        case HStr_Enum_CustomAdditive16: {
            static const char flashStr_Enum_CustomAdditive16[] PROGMEM = {"CustomAdditive16"};
            return flashStr_Enum_CustomAdditive16;
        } break;
        case HStr_Enum_CustomCrop1: {
            static const char flashStr_Enum_CustomCrop1[] PROGMEM = {"CustomCrop1"};
            return flashStr_Enum_CustomCrop1;
        } break;
        case HStr_Enum_CustomCrop2: {
            static const char flashStr_Enum_CustomCrop2[] PROGMEM = {"CustomCrop2"};
            return flashStr_Enum_CustomCrop2;
        } break;
        case HStr_Enum_CustomCrop3: {
            static const char flashStr_Enum_CustomCrop3[] PROGMEM = {"CustomCrop3"};
            return flashStr_Enum_CustomCrop3;
        } break;
        case HStr_Enum_CustomCrop4: {
            static const char flashStr_Enum_CustomCrop4[] PROGMEM = {"CustomCrop4"};
            return flashStr_Enum_CustomCrop4;
        } break;
        case HStr_Enum_CustomCrop5: {
            static const char flashStr_Enum_CustomCrop5[] PROGMEM = {"CustomCrop5"};
            return flashStr_Enum_CustomCrop5;
        } break;
        case HStr_Enum_CustomCrop6: {
            static const char flashStr_Enum_CustomCrop6[] PROGMEM = {"CustomCrop6"};
            return flashStr_Enum_CustomCrop6;
        } break;
        case HStr_Enum_CustomCrop7: {
            static const char flashStr_Enum_CustomCrop7[] PROGMEM = {"CustomCrop7"};
            return flashStr_Enum_CustomCrop7;
        } break;
        case HStr_Enum_CustomCrop8: {
            static const char flashStr_Enum_CustomCrop8[] PROGMEM = {"CustomCrop8"};
            return flashStr_Enum_CustomCrop8;
        } break;
        case HStr_Enum_DC12V: {
            static const char flashStr_Enum_DC12V[] PROGMEM = {"DC12V"};
            return flashStr_Enum_DC12V;
        } break;
        case HStr_Enum_DC24V: {
            static const char flashStr_Enum_DC24V[] PROGMEM = {"DC24V"};
            return flashStr_Enum_DC24V;
        } break;
        case HStr_Enum_DC3V3: {
            static const char flashStr_Enum_DC3V3[] PROGMEM = {"DC3V3"};
            return flashStr_Enum_DC3V3;
        } break;
        case HStr_Enum_DC48V: {
            static const char flashStr_Enum_DC48V[] PROGMEM = {"DC48V"};
            return flashStr_Enum_DC48V;
        } break;
        case HStr_Enum_DC5V: {
            static const char flashStr_Enum_DC5V[] PROGMEM = {"DC5V"};
            return flashStr_Enum_DC5V;
        } break;
        case HStr_Enum_DescOrder: {
            static const char flashStr_Enum_DescOrder[] PROGMEM = {"DescOrder"};
            return flashStr_Enum_DescOrder;
        } break;
        case HStr_Enum_DigitalInput: {
            static const char flashStr_Enum_DigitalInput[] PROGMEM = {"DigitalInput"};
            return flashStr_Enum_DigitalInput;
        } break;
        case HStr_Enum_DigitalInputPullDown: {
            static const char flashStr_Enum_DigitalInputPullDown[] PROGMEM = {"DigitalInputPullDown"};
            return flashStr_Enum_DigitalInputPullDown;
        } break;
        case HStr_Enum_DigitalInputPullUp: {
            static const char flashStr_Enum_DigitalInputPullUp[] PROGMEM = {"DigitalInputPullUp"};
            return flashStr_Enum_DigitalInputPullUp;
        } break;
        case HStr_Enum_DigitalOutput: {
            static const char flashStr_Enum_DigitalOutput[] PROGMEM = {"DigitalOutput"};
            return flashStr_Enum_DigitalOutput;
        } break;
        case HStr_Enum_DigitalOutputPushPull: {
            static const char flashStr_Enum_DigitalOutputPushPull[] PROGMEM = {"DigitalOutputPushPull"};
            return flashStr_Enum_DigitalOutputPushPull;
        } break;
        case HStr_Enum_Dill: {
            static const char flashStr_Enum_Dill[] PROGMEM = {"Dill"};
            return flashStr_Enum_Dill;
        } break;
        case HStr_Enum_Distance: {
            static const char flashStr_Enum_Distance[] PROGMEM = {"Distance"};
            return flashStr_Enum_Distance;
        } break;
        case HStr_Enum_DrainageWater: {
            static const char flashStr_Enum_DrainageWater[] PROGMEM = {"DrainageWater"};
            return flashStr_Enum_DrainageWater;
        } break;
        case HStr_Enum_DrainToWaste: {
            static const char flashStr_Enum_DrainToWaste[] PROGMEM = {"DrainToWaste"};
            return flashStr_Enum_DrainToWaste;
        } break;
        case HStr_Enum_Eggplant: {
            static const char flashStr_Enum_Eggplant[] PROGMEM = {"Eggplant"};
            return flashStr_Enum_Eggplant;
        } break;
        case HStr_Enum_Endive: {
            static const char flashStr_Enum_Endive[] PROGMEM = {"Endive"};
            return flashStr_Enum_Endive;
        } break;
        case HStr_Enum_FanExhaust: {
            static const char flashStr_Enum_FanExhaust[] PROGMEM = {"FanExhaust"};
            return flashStr_Enum_FanExhaust;
        } break;
        case HStr_Enum_FeedWater: {
            static const char flashStr_Enum_FeedWater[] PROGMEM = {"FeedWater"};
            return flashStr_Enum_FeedWater;
        } break;
        case HStr_Enum_Fennel: {
            static const char flashStr_Enum_Fennel[] PROGMEM = {"Fennel"};
            return flashStr_Enum_Fennel;
        } break;
        case HStr_Enum_Flowers: {
            static const char flashStr_Enum_Flowers[] PROGMEM = {"Flowers"};
            return flashStr_Enum_Flowers;
        } break;
        case HStr_Enum_Fodder: {
            static const char flashStr_Enum_Fodder[] PROGMEM = {"Fodder"};
            return flashStr_Enum_Fodder;
        } break;
        case HStr_Enum_FreshWater: {
            static const char flashStr_Enum_FreshWater[] PROGMEM = {"FreshWater"};
            return flashStr_Enum_FreshWater;
        } break;
        case HStr_Enum_Garlic: {
            static const char flashStr_Enum_Garlic[] PROGMEM = {"Garlic"};
            return flashStr_Enum_Garlic;
        } break;
        case HStr_Enum_Ginger: {
            static const char flashStr_Enum_Ginger[] PROGMEM = {"Ginger"};
            return flashStr_Enum_Ginger;
        } break;
        case HStr_Enum_GrowLights: {
            static const char flashStr_Enum_GrowLights[] PROGMEM = {"GrowLights"};
            return flashStr_Enum_GrowLights;
        } break;
        case HStr_Enum_Highest: {
            static const char flashStr_Enum_Highest[] PROGMEM = {"Highest"};
            return flashStr_Enum_Highest;
        } break;
        case HStr_Enum_Imperial: {
            static const char flashStr_Enum_Imperial[] PROGMEM = {"Imperial"};
            return flashStr_Enum_Imperial;
        } break;
        case HStr_Enum_InOrder: {
            static const char flashStr_Enum_InOrder[] PROGMEM = {"InOrder"};
            return flashStr_Enum_InOrder;
        } break;
        case HStr_Enum_Kale: {
            static const char flashStr_Enum_Kale[] PROGMEM = {"Kale"};
            return flashStr_Enum_Kale;
        } break;
        case HStr_Enum_Lavender: {
            static const char flashStr_Enum_Lavender[] PROGMEM = {"Lavender"};
            return flashStr_Enum_Lavender;
        } break;
        case HStr_Enum_Leek: {
            static const char flashStr_Enum_Leek[] PROGMEM = {"Leek"};
            return flashStr_Enum_Leek;
        } break;
        case HStr_Enum_LemonBalm: {
            static const char flashStr_Enum_LemonBalm[] PROGMEM = {"LemonBalm"};
            return flashStr_Enum_LemonBalm;
        } break;
        case HStr_Enum_Lettuce: {
            static const char flashStr_Enum_Lettuce[] PROGMEM = {"Lettuce"};
            return flashStr_Enum_Lettuce;
        } break;
        case HStr_Enum_LiqDilution: {
            static const char flashStr_Enum_LiqDilution[] PROGMEM = {"LiqDilution"};
            return flashStr_Enum_LiqDilution;
        } break;
        case HStr_Enum_LiqFlowRate: {
            static const char flashStr_Enum_LiqFlowRate[] PROGMEM = {"LiqFlowRate"};
            return flashStr_Enum_LiqFlowRate;
        } break;
        case HStr_Enum_LiqVolume: {
            static const char flashStr_Enum_LiqVolume[] PROGMEM = {"LiqVolume"};
            return flashStr_Enum_LiqVolume;
        } break;
        case HStr_Enum_Lowest: {
            static const char flashStr_Enum_Lowest[] PROGMEM = {"Lowest"};
            return flashStr_Enum_Lowest;
        } break;
        case HStr_Enum_Marrow: {
            static const char flashStr_Enum_Marrow[] PROGMEM = {"Marrow"};
            return flashStr_Enum_Marrow;
        } break;
        case HStr_Enum_Melon: {
            static const char flashStr_Enum_Melon[] PROGMEM = {"Melon"};
            return flashStr_Enum_Melon;
        } break;
        case HStr_Enum_Metric: {
            static const char flashStr_Enum_Metric[] PROGMEM = {"Metric"};
            return flashStr_Enum_Metric;
        } break;
        case HStr_Enum_Mint: {
            static const char flashStr_Enum_Mint[] PROGMEM = {"Mint"};
            return flashStr_Enum_Mint;
        } break;
        case HStr_Enum_Multiply: {
            static const char flashStr_Enum_Multiply[] PROGMEM = {"Multiply"};
            return flashStr_Enum_Multiply;
        } break;
        case HStr_Enum_MustardCress: {
            static const char flashStr_Enum_MustardCress[] PROGMEM = {"MustardCress"};
            return flashStr_Enum_MustardCress;
        } break;
        case HStr_Enum_NutrientPremix: {
            static const char flashStr_Enum_NutrientPremix[] PROGMEM = {"NutrientPremix"};
            return flashStr_Enum_NutrientPremix;
        } break;
        case HStr_Enum_Okra: {
            static const char flashStr_Enum_Okra[] PROGMEM = {"Okra"};
            return flashStr_Enum_Okra;
        } break;
        case HStr_Enum_Onions: {
            static const char flashStr_Enum_Onions[] PROGMEM = {"Onions"};
            return flashStr_Enum_Onions;
        } break;
        case HStr_Enum_Oregano: {
            static const char flashStr_Enum_Oregano[] PROGMEM = {"Oregano"};
            return flashStr_Enum_Oregano;
        } break;
        case HStr_Enum_PakChoi: {
            static const char flashStr_Enum_PakChoi[] PROGMEM = {"PakChoi"};
            return flashStr_Enum_PakChoi;
        } break;
        case HStr_Enum_Parsley: {
            static const char flashStr_Enum_Parsley[] PROGMEM = {"Parsley"};
            return flashStr_Enum_Parsley;
        } break;
        case HStr_Enum_Parsnip: {
            static const char flashStr_Enum_Parsnip[] PROGMEM = {"Parsnip"};
            return flashStr_Enum_Parsnip;
        } break;
        case HStr_Enum_Pea: {
            static const char flashStr_Enum_Pea[] PROGMEM = {"Pea"};
            return flashStr_Enum_Pea;
        } break;
        case HStr_Enum_PeaSugar: {
            static const char flashStr_Enum_PeaSugar[] PROGMEM = {"PeaSugar"};
            return flashStr_Enum_PeaSugar;
        } break;
        case HStr_Enum_Pepino: {
            static const char flashStr_Enum_Pepino[] PROGMEM = {"Pepino"};
            return flashStr_Enum_Pepino;
        } break;
        case HStr_Enum_PeppersBell: {
            static const char flashStr_Enum_PeppersBell[] PROGMEM = {"PeppersBell"};
            return flashStr_Enum_PeppersBell;
        } break;
        case HStr_Enum_PeppersHot: {
            static const char flashStr_Enum_PeppersHot[] PROGMEM = {"PeppersHot"};
            return flashStr_Enum_PeppersHot;
        } break;
        case HStr_Enum_Percentile: {
            static const char flashStr_Enum_Percentile[] PROGMEM = {"Percentile"};
            return flashStr_Enum_Percentile;
        } break;
        case HStr_Enum_PeristalticPump: {
            static const char flashStr_Enum_PeristalticPump[] PROGMEM = {"PeristalticPump"};
            return flashStr_Enum_PeristalticPump;
        } break;
        case HStr_Enum_PhDownSolution: {
            static const char flashStr_Enum_PhDownSolution[] PROGMEM = {"PhDownSolution"};
            return flashStr_Enum_PhDownSolution;
        } break;
        case HStr_Enum_PhUpSolution: {
            static const char flashStr_Enum_PhUpSolution[] PROGMEM = {"PhUpSolution"};
            return flashStr_Enum_PhUpSolution;
        } break;
        case HStr_Enum_Potato: {
            static const char flashStr_Enum_Potato[] PROGMEM = {"Potato"};
            return flashStr_Enum_Potato;
        } break;
        case HStr_Enum_PotatoSweet: {
            static const char flashStr_Enum_PotatoSweet[] PROGMEM = {"PotatoSweet"};
            return flashStr_Enum_PotatoSweet;
        } break;
        case HStr_Enum_Power: {
            static const char flashStr_Enum_Power[] PROGMEM = {"Power"};
            return flashStr_Enum_Power;
        } break;
        case HStr_Enum_PowerLevel: {
            static const char flashStr_Enum_PowerLevel[] PROGMEM = {"PowerLevel"};
            return flashStr_Enum_PowerLevel;
        } break;
        case HStr_Enum_PumpFlow: {
            static const char flashStr_Enum_PumpFlow[] PROGMEM = {"PumpFlow"};
            return flashStr_Enum_PumpFlow;
        } break;
        case HStr_Enum_Pumpkin: {
            static const char flashStr_Enum_Pumpkin[] PROGMEM = {"Pumpkin"};
            return flashStr_Enum_Pumpkin;
        } break;
        case HStr_Enum_Radish: {
            static const char flashStr_Enum_Radish[] PROGMEM = {"Radish"};
            return flashStr_Enum_Radish;
        } break;
        case HStr_Enum_Recycling: {
            static const char flashStr_Enum_Recycling[] PROGMEM = {"Recycling"};
            return flashStr_Enum_Recycling;
        } break;
        case HStr_Enum_RevOrder: {
            static const char flashStr_Enum_RevOrder[] PROGMEM = {"RevOrder"};
            return flashStr_Enum_RevOrder;
        } break;
        case HStr_Enum_Rhubarb: {
            static const char flashStr_Enum_Rhubarb[] PROGMEM = {"Rhubarb"};
            return flashStr_Enum_Rhubarb;
        } break;
        case HStr_Enum_Rockwool: {
            static const char flashStr_Enum_Rockwool[] PROGMEM = {"Rockwool"};
            return flashStr_Enum_Rockwool;
        } break;
        case HStr_Enum_Rosemary: {
            static const char flashStr_Enum_Rosemary[] PROGMEM = {"Rosemary"};
            return flashStr_Enum_Rosemary;
        } break;
        case HStr_Enum_RotaryEncoder: {
            static const char flashStr_Enum_RotaryEncoder[] PROGMEM = {"RotaryEncoder"};
            return flashStr_Enum_RotaryEncoder;
        } break;
        case HStr_Enum_Sage: {
            static const char flashStr_Enum_Sage[] PROGMEM = {"Sage"};
            return flashStr_Enum_Sage;
        } break;
        case HStr_Enum_Scientific: {
            static const char flashStr_Enum_Scientific[] PROGMEM = {"Scientific"};
            return flashStr_Enum_Scientific;
        } break;
        case HStr_Enum_Silverbeet: {
            static const char flashStr_Enum_Silverbeet[] PROGMEM = {"Silverbeet"};
            return flashStr_Enum_Silverbeet;
        } break;
        case HStr_Enum_SoilMoisture: {
            static const char flashStr_Enum_SoilMoisture[] PROGMEM = {"SoilMoisture"};
            return flashStr_Enum_SoilMoisture;
        } break;
        case HStr_Enum_Spinach: {
            static const char flashStr_Enum_Spinach[] PROGMEM = {"Spinach"};
            return flashStr_Enum_Spinach;
        } break;
        case HStr_Enum_Squash: {
            static const char flashStr_Enum_Squash[] PROGMEM = {"Squash"};
            return flashStr_Enum_Squash;
        } break;
        case HStr_Enum_Strawberries: {
            static const char flashStr_Enum_Strawberries[] PROGMEM = {"Strawberries"};
            return flashStr_Enum_Strawberries;
        } break;
        case HStr_Enum_Sunflower: {
            static const char flashStr_Enum_Sunflower[] PROGMEM = {"Sunflower"};
            return flashStr_Enum_Sunflower;
        } break;
        case HStr_Enum_SwissChard: {
            static const char flashStr_Enum_SwissChard[] PROGMEM = {"SwissChard"};
            return flashStr_Enum_SwissChard;
        } break;
        case HStr_Enum_Taro: {
            static const char flashStr_Enum_Taro[] PROGMEM = {"Taro"};
            return flashStr_Enum_Taro;
        } break;
        case HStr_Enum_Tarragon: {
            static const char flashStr_Enum_Tarragon[] PROGMEM = {"Tarragon"};
            return flashStr_Enum_Tarragon;
        } break;
        case HStr_Enum_Temperature: {
            static const char flashStr_Enum_Temperature[] PROGMEM = {"Temperature"};
            return flashStr_Enum_Temperature;
        } break;
        case HStr_Enum_Thyme: {
            static const char flashStr_Enum_Thyme[] PROGMEM = {"Thyme"};
            return flashStr_Enum_Thyme;
        } break;
        case HStr_Enum_Tomato: {
            static const char flashStr_Enum_Tomato[] PROGMEM = {"Tomato"};
            return flashStr_Enum_Tomato;
        } break;
        case HStr_Enum_Turnip: {
            static const char flashStr_Enum_Turnip[] PROGMEM = {"Turnip"};
            return flashStr_Enum_Turnip;
        } break;
        case HStr_Enum_WaterAerator: {
            static const char flashStr_Enum_WaterAerator[] PROGMEM = {"WaterAerator"};
            return flashStr_Enum_WaterAerator;
        } break;
        case HStr_Enum_WaterHeater: {
            static const char flashStr_Enum_WaterHeater[] PROGMEM = {"WaterHeater"};
            return flashStr_Enum_WaterHeater;
        } break;
        case HStr_Enum_WaterHeight: {
            static const char flashStr_Enum_WaterHeight[] PROGMEM = {"WaterHeight"};
            return flashStr_Enum_WaterHeight;
        } break;
        case HStr_Enum_WaterLevel: {
            static const char flashStr_Enum_WaterLevel[] PROGMEM = {"LevelIndicator"};
            return flashStr_Enum_WaterLevel;
        } break;
        case HStr_Enum_WaterPH: {
            static const char flashStr_Enum_WaterPH[] PROGMEM = {"WaterPH"};
            return flashStr_Enum_WaterPH;
        } break;
        case HStr_Enum_WaterPump: {
            static const char flashStr_Enum_WaterPump[] PROGMEM = {"WaterPump"};
            return flashStr_Enum_WaterPump;
        } break;
        case HStr_Enum_WaterSprayer: {
            static const char flashStr_Enum_WaterSprayer[] PROGMEM = {"WaterSprayer"};
            return flashStr_Enum_WaterSprayer;
        } break;
        case HStr_Enum_WaterTDS: {
            static const char flashStr_Enum_WaterTDS[] PROGMEM = {"WaterTDS"};
            return flashStr_Enum_WaterTDS;
        } break;
        case HStr_Enum_WaterTemperature: {
            static const char flashStr_Enum_WaterTemperature[] PROGMEM = {"WaterTemp"};
            return flashStr_Enum_WaterTemperature;
        } break;
        case HStr_Enum_Watercress: {
            static const char flashStr_Enum_Watercress[] PROGMEM = {"Watercress"};
            return flashStr_Enum_Watercress;
        } break;
        case HStr_Enum_Watermelon: {
            static const char flashStr_Enum_Watermelon[] PROGMEM = {"Watermelon"};
            return flashStr_Enum_Watermelon;
        } break;
        case HStr_Enum_Weight: {
            static const char flashStr_Enum_Weight[] PROGMEM = {"Weight"};
            return flashStr_Enum_Weight;
        } break;
        case HStr_Enum_Zucchini: {
            static const char flashStr_Enum_Zucchini[] PROGMEM = {"Zucchini"};
            return flashStr_Enum_Zucchini;
        } break;

        case HStr_Unit_Count: {
            static const char flashStr_Unit_Count[] PROGMEM = {"[qty]"};
            return flashStr_Unit_Count;
        } break;
        case HStr_Unit_Degree: {
            static const char flashStr_Unit_Degree[] PROGMEM = {"\xC2\xB0"};
            return flashStr_Unit_Degree;
        } break;
        case HStr_Unit_EC5: {
            static const char flashStr_Unit_EC5[] PROGMEM = {"EC(5)"};
            return flashStr_Unit_EC5;
        } break;
        case HStr_Unit_Feet: {
            static const char flashStr_Unit_Feet[] PROGMEM = {"ft"};
            return flashStr_Unit_Feet;
        } break;
        case HStr_Unit_Gallons: {
            static const char flashStr_Unit_Gallons[] PROGMEM = {"gal"};
            return flashStr_Unit_Gallons;
        } break;
        case HStr_Unit_Kilograms: {
            static const char flashStr_Unit_Kilograms[] PROGMEM = {"Kg"};
            return flashStr_Unit_Kilograms;
        } break;
        case HStr_Unit_MilliLiterPer: {
            static const char flashStr_Unit_MilliLiterPer[] PROGMEM = {"mL/"};
            return flashStr_Unit_MilliLiterPer;
        } break;
        case HStr_Unit_PerMinute: {
            static const char flashStr_Unit_PerMinute[] PROGMEM = {"/min"};
            return flashStr_Unit_PerMinute;
        } break;
        case HStr_Unit_pH14: {
            static const char flashStr_Unit_pH14[] PROGMEM = {"[pH(14)]"};
            return flashStr_Unit_pH14;
        } break;
        case HStr_Unit_Pounds: {
            static const char flashStr_Unit_Pounds[] PROGMEM = {"lbs"};
            return flashStr_Unit_Pounds;
        } break;
        case HStr_Unit_PPM500: {
            static const char flashStr_Unit_PPM500[] PROGMEM = {"ppm(500)"};
            return flashStr_Unit_PPM500;
        } break;
        case HStr_Unit_PPM640: {
            static const char flashStr_Unit_PPM640[] PROGMEM = {"ppm(640)"};
            return flashStr_Unit_PPM640;
        } break;
        case HStr_Unit_PPM700: {
            static const char flashStr_Unit_PPM700[] PROGMEM = {"ppm(700)"};
            return flashStr_Unit_PPM700;
        } break;
        case HStr_Unit_Undefined: {
            static const char flashStr_Unit_Undefined[] PROGMEM = {"[undef]"};
            return flashStr_Unit_Undefined;
        } break;
    }
    return nullptr;
}

#endif
