/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Strings
*/

#include "Hydroponics.h"

#ifndef HYDRUINO_ENABLE_EXTERNAL_DATA
String stringFromPGMAddr(const char *flashStr);
const char *pgmAddrForStr(Hydroponics_String strNum);
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

String stringFromPGM(Hydroponics_String strNum) {
    if (_strDataAddress != (size_t)-1) {
        auto eeprom = getHydroponicsInstance()->getEEPROM();

        if (eeprom) {
            uint16_t lookupOffset = 0;
            eeprom->readBlock(_strDataAddress + (sizeof(uint16_t) * ((int)strNum + 1)), // +1 for initial total size word
                              (byte *)&lookupOffset, sizeof(lookupOffset));

            {   char buffer[HYDRUINO_STRING_BUFFER_SIZE] = {0};
                eeprom->readBlock(lookupOffset, (byte *)&buffer[0], HYDRUINO_STRING_BUFFER_SIZE);
                return charsToString(buffer, HYDRUINO_STRING_BUFFER_SIZE);

                // The following will be needed in case we go over 32 bytes in a single string data
                // while (strnlen(buffer, HYDRUINO_STRING_BUFFER_SIZE) == HYDRUINO_STRING_BUFFER_SIZE) {
                //     lookupOffset += HYDRUINO_STRING_BUFFER_SIZE;
                //     eeprom->readBlock(lookupOffset, (byte *)&buffer[0], HYDRUINO_STRING_BUFFER_SIZE);
                //     retVal += charsToString(buffer, HYDRUINO_STRING_BUFFER_SIZE);
                // }
            }
        }
    }

    #ifndef HYDRUINO_ENABLE_EXTERNAL_DATA
        return stringFromPGMAddr(pgmAddrForStr(strNum));
    #endif

    if (_strDataFilePrefix.length()) {
        auto sd = getHydroponicsInstance()->getSDCard();

        if (sd) {
            String retVal;
            String filename = String(_strDataFilePrefix + String(F("strings.")) + SFP(HStr_dat));

            auto file = sd->open(filename, FILE_READ);
            if (file) {
                uint16_t lookupOffset = 0;
                file.seek(sizeof(uint16_t) * (int)strNum);
                file.readBytes((byte *)&lookupOffset, sizeof(lookupOffset));

                {   char buffer[HYDRUINO_STRING_BUFFER_SIZE] = {0};
                    file.seek(lookupOffset);
                    file.readBytesUntil('\0', buffer, HYDRUINO_STRING_BUFFER_SIZE);
                    retVal = charsToString(buffer, HYDRUINO_STRING_BUFFER_SIZE);

                    // The following will be needed in case we go over 32 bytes in a single string data
                    // while (strnlen(buffer, HYDRUINO_STRING_BUFFER_SIZE) == HYDRUINO_STRING_BUFFER_SIZE) {
                    //     file.readBytesUntil('\0', buffer, HYDRUINO_STRING_BUFFER_SIZE);
                    //     retVal.concat(charsToString(buffer, HYDRUINO_STRING_BUFFER_SIZE));
                    // }
                }

                file.close();
            }

            getHydroponicsInstance()->endSDCard(sd);
            if (retVal.length()) { return retVal; }
        }
    }

    return String();
}

#ifndef HYDRUINO_ENABLE_EXTERNAL_DATA

String stringFromPGMAddr(const char *flashStr) {
    if (flashStr) {
        char buffer[HYDRUINO_STRING_BUFFER_SIZE] = {0};
        strncpy_P(buffer, flashStr, HYDRUINO_STRING_BUFFER_SIZE);
        String retVal = charsToString(buffer, HYDRUINO_STRING_BUFFER_SIZE);

        // The following will be needed in case we go over 32 bytes in a single string data
        // while (strnlen(buffer, HYDRUINO_STRING_BUFFER_SIZE) == HYDRUINO_STRING_BUFFER_SIZE) {
        //     flashStr += HYDRUINO_STRING_BUFFER_SIZE;
        //     strncpy_P(buffer, flashStr, HYDRUINO_STRING_BUFFER_SIZE);
        //     retVal += charsToString(buffer, HYDRUINO_STRING_BUFFER_SIZE);
        // }
        
        return retVal;
    }
    return String();
}

const char *pgmAddrForStr(Hydroponics_String strNum)
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
        case HStr_Log_PreLightSpraying: {
            static const char flashStr_Log_PreLightSpraying[] PROGMEM = {" dawntime spraying"};
            return flashStr_Log_PreLightSpraying;
        } break;
        case HStr_Log_SystemDataSaved: {
            static const char flashStr_Log_SystemDataSaved[] PROGMEM = {"System data saved"};
            return flashStr_Log_SystemDataSaved;
        } break;
        case HStr_Log_SystemUptime: {
            static const char flashStr_Log_SystemUptime[] PROGMEM = {"System uptime: "};
            return flashStr_Log_SystemUptime;
        } break;

        case HSTR_Log_Field_Aerator_Duration: {
            static const char flashStr_Log_Field_Aerator_Duration[] PROGMEM = {"  Aerator duration: "};
            return flashStr_Log_Field_Aerator_Duration;
        } break;
        case HSTR_Log_Field_Light_Duration: {
            static const char flashStr_Log_Field_Light_Duration[] PROGMEM = {"  Daylight duration: "};
            return flashStr_Log_Field_Light_Duration;
        } break;
        case HSTR_Log_Field_Sprayer_Duration: {
            static const char flashStr_Log_Field_Sprayer_Duration[] PROGMEM = {"  Sprayer duration: "};
            return flashStr_Log_Field_Sprayer_Duration;
        } break;
        case HStr_Log_Field_pH_Setpoint: {
            static const char flashStr_Log_Field_pH_Setpoint[] PROGMEM = {"  ph setpoint: "};
            return flashStr_Log_Field_pH_Setpoint;
        } break;
        case HStr_Log_Field_TDS_Setpoint: {
            static const char flashStr_Log_Field_TDS_Setpoint[] PROGMEM = {"  TDS setpoint: "};
            return flashStr_Log_Field_TDS_Setpoint;
        } break;
        case HStr_Log_Field_Temp_Setpoint: {
            static const char flashStr_Log_Field_Temp_Setpoint[] PROGMEM = {"  Temp setpoint: "};
            return flashStr_Log_Field_Temp_Setpoint;
        } break;
        case HStr_Log_Field_CO2_Setpoint: {
            static const char flashStr_Log_Field_CO2_Setpoint[] PROGMEM = {"  CO2 setpoint: "};
            return flashStr_Log_Field_CO2_Setpoint;
        } break;
        case HStr_Log_Field_Time_Calculated: {
            static const char flashStr_Log_Field_Time_Calculated[] PROGMEM = {"  Pump time: "};
            return flashStr_Log_Field_Time_Calculated;
        } break;
        case HStr_Log_Field_Vol_Calculated: {
            static const char flashStr_Log_Field_Vol_Calculated[] PROGMEM = {"  Est. vol.: "};
            return flashStr_Log_Field_Vol_Calculated;
        } break;
        case HStr_Log_Field_pH_Measured: {
            static const char flashStr_Log_Field_pH_Measured[] PROGMEM = {"  ph sensor: "};
            return flashStr_Log_Field_pH_Measured;
        } break;
        case HStr_Log_Field_TDS_Measured: {
            static const char flashStr_Log_Field_TDS_Measured[] PROGMEM = {"  TDS sensor: "};
            return flashStr_Log_Field_TDS_Measured;
        } break;
        case HStr_Log_Field_Temp_Measured: {
            static const char flashStr_Log_Field_Temp_Measured[] PROGMEM = {"  Temp sensor: "};
            return flashStr_Log_Field_Temp_Measured;
        } break;
        case HStr_Log_Field_CO2_Measured: {
            static const char flashStr_Log_Field_CO2_Measured[] PROGMEM = {"  CO2 sensor: "};
            return flashStr_Log_Field_CO2_Measured;
        } break;
        case HStr_Log_Field_Time_Measured: {
            static const char flashStr_Log_Field_Time_Measured[] PROGMEM = {"  Elapsed time: "};
            return flashStr_Log_Field_Time_Measured;
        } break;
        case HStr_Log_Field_Vol_Measured: {
            static const char flashStr_Log_Field_Vol_Measured[] PROGMEM = {"  Pumped vol.: "};
            return flashStr_Log_Field_Vol_Measured;
        } break;
        case HStr_Log_Field_Time_Start: {
            static const char flashStr_Log_Field_Time_Start[] PROGMEM = {"  Start time: "};
            return flashStr_Log_Field_Time_Start;
        } break;
        case HStr_Log_Field_Time_Finish: {
            static const char flashStr_Log_Field_Time_Finish[] PROGMEM = {"  Finish time: "};
            return flashStr_Log_Field_Time_Finish;
        } break;

        case HStr_Key_ActiveLow: {
            static const char flashStr_Key_ActiveLow[] PROGMEM = {"activeLow"};
            return flashStr_Key_ActiveLow;
        } break;
        case HStr_Key_AdditiveName: {
            static const char flashStr_Key_AdditiveName[] PROGMEM = {"additiveName"};
            return flashStr_Key_AdditiveName;
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
        case HStr_Key_AlwaysFilled: {
            static const char flashStr_Key_AlwaysFilled[] PROGMEM = {"alwaysFilled"};
            return flashStr_Key_AlwaysFilled;
        } break;
        case HStr_Key_AutosaveEnabled: {
            static const char flashStr_Key_AutosaveEnabled[] PROGMEM = {"autosaveEnabled"};
            return flashStr_Key_AutosaveEnabled;
        } break;
        case HStr_Key_AutosaveInterval: {
            static const char flashStr_Key_AutosaveInterval[] PROGMEM = {"autosaveInterval"};
            return flashStr_Key_AutosaveInterval;
        } break;
        case HStr_Key_BaseFeedMultiplier: {
            static const char flashStr_Key_BaseFeedMultiplier[] PROGMEM = {"baseFeedMultiplier"};
            return flashStr_Key_BaseFeedMultiplier;
        } break;
        case HStr_Key_CO2Levels: {
            static const char flashStr_Key_CO2Levels[] PROGMEM = {"co2Levels"};
            return flashStr_Key_CO2Levels;
        } break;
        case HStr_Key_CO2Sensor: {
            static const char flashStr_Key_CO2Sensor[] PROGMEM = {"co2Sensor"};
            return flashStr_Key_CO2Sensor;
        } break;
        case HStr_Key_CalibUnits: {
            static const char flashStr_Key_CalibUnits[] PROGMEM = {"calibUnits"};
            return flashStr_Key_CalibUnits;
        } break;
        case HStr_Key_ComputeHeatIndex: {
            static const char flashStr_Key_ComputeHeatIndex[] PROGMEM = {"computeHeatIndex"};
            return flashStr_Key_ComputeHeatIndex;
        } break;
        case HStr_Key_ContFlowRate: {
            static const char flashStr_Key_ContFlowRate[] PROGMEM = {"contFlowRate"};
            return flashStr_Key_ContFlowRate;
        } break;
        case HStr_Key_ContPowerUsage: {
            static const char flashStr_Key_ContPowerUsage[] PROGMEM = {"contPowerUsage"};
            return flashStr_Key_ContPowerUsage;
        } break;
        case HStr_Key_CropName: {
            static const char flashStr_Key_CropName[] PROGMEM = {"cropName"};
            return flashStr_Key_CropName;
        } break;
        case HStr_Key_CropType: {
            static const char flashStr_Key_CropType[] PROGMEM = {"cropType"};
            return flashStr_Key_CropType;
        } break;
        case HStr_Key_CtrlInMode: {
            static const char flashStr_Key_CtrlInMode[] PROGMEM = {"ctrlInMode"};
            return flashStr_Key_CtrlInMode;
        } break;
        case HStr_Key_DHTType: {
            static const char flashStr_Key_DHTType[] PROGMEM = {"dhtType"};
            return flashStr_Key_DHTType;
        } break;
        case HStr_Key_DailyLightHours: {
            static const char flashStr_Key_DailyLightHours[] PROGMEM = {"dailyLightHours"};
            return flashStr_Key_DailyLightHours;
        } break;
        case HStr_Key_DataFilePrefix: {
            static const char flashStr_Key_DataFilePrefix[] PROGMEM = {"dataFilePrefix"};
            return flashStr_Key_DataFilePrefix;
        } break;
        case HStr_Key_DetriggerTol: {
            static const char flashStr_Key_DetriggerTol[] PROGMEM = {"detriggerTol"};
            return flashStr_Key_DetriggerTol;
        } break;
        case HStr_Key_DispOutMode: {
            static const char flashStr_Key_DispOutMode[] PROGMEM = {"dispOutMode"};
            return flashStr_Key_DispOutMode;
        } break;
        case HStr_Key_EmptyTrigger: {
            static const char flashStr_Key_EmptyTrigger[] PROGMEM = {"emptyTrigger"};
            return flashStr_Key_EmptyTrigger;
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
        case HStr_Key_InputBitRes: {
            static const char flashStr_Key_InputBitRes[] PROGMEM = {"inputBitRes"};
            return flashStr_Key_InputBitRes;
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
        case HStr_Key_LastChangeDate: {
            static const char flashStr_Key_LastChangeDate[] PROGMEM = {"lastChangeDate"};
            return flashStr_Key_LastChangeDate;
        } break;
        case HStr_Key_LastFeedingDate: {
            static const char flashStr_Key_LastFeedingDate[] PROGMEM = {"lastFeedingDate"};
            return flashStr_Key_LastFeedingDate;
        } break;
        case HStr_Key_LastPruningDate: {
            static const char flashStr_Key_LastPruningDate[] PROGMEM = {"lastPruningDate"};
            return flashStr_Key_LastPruningDate;
        } break;
        case HStr_Key_LifeCycleWeeks: {
            static const char flashStr_Key_LifeCycleWeeks[] PROGMEM = {"lifeCycleWeeks"};
            return flashStr_Key_LifeCycleWeeks;
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
        case HStr_Key_Logger: {
            static const char flashStr_Key_Logger[] PROGMEM = {"logger"};
            return flashStr_Key_Logger;
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
        case HStr_Key_MoistureSensor: {
            static const char flashStr_Key_MoistureSensor[] PROGMEM = {"moistureSensor"};
            return flashStr_Key_MoistureSensor;
        } break;
        case HStr_Key_MoistureUnits: {
            static const char flashStr_Key_MoistureUnits[] PROGMEM = {"moistureUnits"};
            return flashStr_Key_MoistureUnits;
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
        case HStr_Key_OutputBitRes: {
            static const char flashStr_Key_OutputBitRes[] PROGMEM = {"outputBitRes"};
            return flashStr_Key_OutputBitRes;
        } break;
        case HStr_Key_OutputPin: {
            static const char flashStr_Key_OutputPin[] PROGMEM = {"outputPin"};
            return flashStr_Key_OutputPin;
        } break;
        case HStr_Key_OutputReservoir: {
            static const char flashStr_Key_OutputReservoir[] PROGMEM = {"destReservoir"};
            return flashStr_Key_OutputReservoir;
        } break;
        case HStr_Key_PHRange: {
            static const char flashStr_Key_PHRange[] PROGMEM = {"phRange"};
            return flashStr_Key_PHRange;
        } break;
        case HStr_Key_PHSensor: {
            static const char flashStr_Key_PHSensor[] PROGMEM = {"phSensor"};
            return flashStr_Key_PHSensor;
        } break;
        case HStr_Key_Perennial: {
            static const char flashStr_Key_Perennial[] PROGMEM = {"perennial"};
            return flashStr_Key_Perennial;
        } break;
        case HStr_Key_PhaseDurationWeeks: {
            static const char flashStr_Key_PhaseDurationWeeks[] PROGMEM = {"phaseDurationWeeks"};
            return flashStr_Key_PhaseDurationWeeks;
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
        case HStr_Key_PreLightSprayMins: {
            static const char flashStr_Key_PreLightSprayMins[] PROGMEM = {"preLightSprayMins"};
            return flashStr_Key_PreLightSprayMins;
        } break;
        case HStr_Key_Pruning: {
            static const char flashStr_Key_Pruning[] PROGMEM = {"pruning"};
            return flashStr_Key_Pruning;
        } break;
        case HStr_Key_PublishToSDCard: {
            static const char flashStr_Key_PublishToSDCard[] PROGMEM = {"publishToSDCard"};
            return flashStr_Key_PublishToSDCard;
        } break;
        case HStr_Key_Publisher: {
            static const char flashStr_Key_Publisher[] PROGMEM = {"publisher"};
            return flashStr_Key_Publisher;
        } break;
        case HStr_Key_PullupPin: {
            static const char flashStr_Key_PullupPin[] PROGMEM = {"pullupPin"};
            return flashStr_Key_PullupPin;
        } break;
        case HStr_Key_RailName: {
            static const char flashStr_Key_RailName[] PROGMEM = {"railName"};
            return flashStr_Key_RailName;
        } break;
        case HStr_Key_ReservoirName: {
            static const char flashStr_Key_ReservoirName[] PROGMEM = {"reservoirName"};
            return flashStr_Key_ReservoirName;
        } break;
        case HStr_Key_ReservoirType: {
            static const char flashStr_Key_ReservoirType[] PROGMEM = {"reservoirType"};
            return flashStr_Key_ReservoirType;
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
        case HStr_Key_SowDate: {
            static const char flashStr_Key_SowDate[] PROGMEM = {"sowDate"};
            return flashStr_Key_SowDate;
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
        case HStr_Key_TDSUnits: {
            static const char flashStr_Key_TDSUnits[] PROGMEM = {"tdsUnits"};
            return flashStr_Key_TDSUnits;
        } break;
        case HStr_Key_TemperatureSensor: {
            static const char flashStr_Key_TemperatureSensor[] PROGMEM = {"tempSensor"};
            return flashStr_Key_TemperatureSensor;
        } break;
        case HStr_Key_TempUnits: {
            static const char flashStr_Key_TempUnits[] PROGMEM = {"tempUnits"};
            return flashStr_Key_TempUnits;
        } break;
        case HStr_Key_TimeZoneOffset: {
            static const char flashStr_Key_TimeZoneOffset[] PROGMEM = {"timeZoneOffset"};
            return flashStr_Key_TimeZoneOffset;
        } break;
        case HStr_Key_Timestamp: {
            static const char flashStr_Key_Timestamp[] PROGMEM = {"timestamp"};
            return flashStr_Key_Timestamp;
        } break;
        case HStr_Key_ToleranceHigh: {
            static const char flashStr_Key_ToleranceHigh[] PROGMEM = {"toleranceHigh"};
            return flashStr_Key_ToleranceHigh;
        } break;
        case HStr_Key_ToleranceLow: {
            static const char flashStr_Key_ToleranceLow[] PROGMEM = {"toleranceLow"};
            return flashStr_Key_ToleranceLow;
        } break;
        case HStr_Key_ToleranceUnits: {
            static const char flashStr_Key_ToleranceUnits[] PROGMEM = {"toleranceUnits"};
            return flashStr_Key_ToleranceUnits;
        } break;
        case HStr_Key_Tolerance: {
            static const char flashStr_Key_Tolerance[] PROGMEM = {"tolerance"};
            return flashStr_Key_Tolerance;
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
        case HStr_Key_WiFiPasswordSeed: {
            static const char flashStr_Key_WiFiPasswordSeed[] PROGMEM = {"wifiPwSeed"};
            return flashStr_Key_WiFiPasswordSeed;
        } break;
        case HStr_Key_WiFiPassword: {
            static const char flashStr_Key_WiFiPassword[] PROGMEM = {"wifiPassword"};
            return flashStr_Key_WiFiPassword;
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

        default:
            return nullptr;
    }
}

#endif
