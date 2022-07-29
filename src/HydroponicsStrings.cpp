/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Strings
*/

#include "Hydroponics.h"

#ifndef HYDRUINO_ENABLE_EXTERNAL_DATA
String stringFromPGMAddr(const char *flashStr);
const char *pgmAddrForStr(Hydroponics_String strNum);
#endif

String stringFromPGM(Hydroponics_String strNum) {
    #ifdef HYDRUINO_ENABLE_EXTERNAL_DATA
        
    #else
        return stringFromPGMAddr(pgmAddrForStr(strNum));
    #endif
}

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

#ifndef HYDRUINO_ENABLE_EXTERNAL_DATA

String stringFromPGMAddr(const char *flashStr) {
    if (flashStr) {
        char buffer[32] = {0};
        strncpy_P(buffer, flashStr, 32);
        return String(buffer);
    }
    return String();
}

const char *pgmAddrForStr(Hydroponics_String strNum)
{
    switch(strNum) {
        case HS_ColonSpace: {
            static const char flashStr_ColonSpace[] PROGMEM = {": "};
            return flashStr_ColonSpace;
        } break;
        case HS_DoubleSpace: {
            static const char flashStr_DoubleSpace[] PROGMEM = {"  "};
            return flashStr_DoubleSpace;
        } break;
        case HS_Count: {
            static const char flashStr_Count[] PROGMEM = {"Count"};
            return flashStr_Count;
        } break;
        case HS_csv: {
            static const char flashStr_csv[] PROGMEM = {"csv"};
            return flashStr_csv;
        } break;
        case HS_dat: {
            static const char flashStr_dat[] PROGMEM = {"dat"};
            return flashStr_dat;
        } break;
        case HS_Disabled: {
            static const char flashStr_Disabled[] PROGMEM = {"Disabled"};
            return flashStr_Disabled;
        } break;
        case HS_txt: {
            static const char flashStr_txt[] PROGMEM = {"txt"};
            return flashStr_txt;
        } break;
        case HS_Undefined: {
            static const char flashStr_Undefined[] PROGMEM = {"Undefined"};
            return flashStr_Undefined;
        } break;
        case HS_null: {
            static const char flashStr_null[] PROGMEM = {"null"};
            return flashStr_null;
        } break;

        case HS_Err_AllocationFailure: {
            static const char flashStr_Err_AllocationFailure[] PROGMEM = {"Allocation failure"};
            return flashStr_Err_AllocationFailure;
        } break;
        case HS_Err_AlreadyInitialized: {
            static const char flashStr_Err_AlreadyInitialized[] PROGMEM = {"Already initialized"};
            return flashStr_Err_AlreadyInitialized;
        } break;
        case HS_Err_AssertionFailure: {
            static const char flashStr_Err_AssertionFailure[] PROGMEM = {"Assertion failure"};
            return flashStr_Err_AssertionFailure;
        } break;
        case HS_Err_ExportFailure: {
            static const char flashStr_Err_ExportFailure[] PROGMEM = {"Export failure"};
            return flashStr_Err_ExportFailure;
        } break;
        case HS_Err_ImportFailure: {
            static const char flashStr_Err_ImportFailure[] PROGMEM = {"Import failure"};
            return flashStr_Err_ImportFailure;
        } break;
        case HS_Err_InitializationFailure: {
            static const char flashStr_Err_InitializationFailure[] PROGMEM = {"Initialization failure"};
            return flashStr_Err_InitializationFailure;
        } break;
        case HS_Err_InvalidParameter: {
            static const char flashStr_Err_InvalidParameter[] PROGMEM = {"Invalid parameter"};
            return flashStr_Err_InvalidParameter;
        } break;
        case HS_Err_InvalidPinOrType: {
            static const char flashStr_Err_InvalidPinOrType[] PROGMEM = {"Invalid pin or type"};
            return flashStr_Err_InvalidPinOrType;
        } break;
        case HS_Err_MeasurementFailure: {
            static const char flashStr_Err_MeasurementFailure[] PROGMEM = {"Measurement failure"};
            return flashStr_Err_MeasurementFailure;
        } break;
        case HS_Err_MissingLinkage: {
            static const char flashStr_Err_MissingLinkage[] PROGMEM = {"Missing or no linkage"};
            return flashStr_Err_MissingLinkage;
        } break;
        case HS_Err_NoPositionsAvailable: {
            static const char flashStr_Err_NoPositionsAvailable[] PROGMEM = {"No positions available"};
            return flashStr_Err_NoPositionsAvailable;
        } break;
        case HS_Err_NotYetInitialized: {
            static const char flashStr_Err_NotYetInitialized[] PROGMEM = {"Not yet initialized"};
            return flashStr_Err_NotYetInitialized;
        } break;
        case HS_Err_OperationFailure: {
            static const char flashStr_Err_OperationFailure[] PROGMEM = {"Operation failure"};
            return flashStr_Err_OperationFailure;
        } break;
        case HS_Err_UnsupportedOperation: {
            static const char flashStr_Err_UnsupportedOperation[] PROGMEM = {"Unsupported operation"};
            return flashStr_Err_UnsupportedOperation;
        } break;

        case HS_Log_AirReport: {
            static const char flashStr_Log_AirReport[] PROGMEM = {" air report:"};
            return flashStr_Log_AirReport;
        } break;
        case HS_Log_CalculatedPumping: {
            static const char flashStr_Log_CalculatedPumping[] PROGMEM = {" pumping request:"};
            return flashStr_Log_CalculatedPumping;
        } break;
        case HS_Log_FeedingSequence: {
            static const char flashStr_Log_FeedingSequence[] PROGMEM = {" feeding sequence"};
            return flashStr_Log_FeedingSequence;
        } break;
        case HS_Log_HasBegan: {
            static const char flashStr_Log_HasBegan[] PROGMEM = {" has began"};
            return flashStr_Log_HasBegan;
        } break;
        case HS_Log_HasDisabled: {
            static const char flashStr_Log_HasDisabled[] PROGMEM = {" has disabled"};
            return flashStr_Log_HasDisabled;
        } break;
        case HS_Log_HasEnabled: {
            static const char flashStr_Log_HasEnabled[] PROGMEM = {" has enabled"};
            return flashStr_Log_HasEnabled;
        } break;
        case HS_Log_HasEnded: {
            static const char flashStr_Log_HasEnded[] PROGMEM = {" has ended"};
            return flashStr_Log_HasEnded;
        } break;
        case HS_Log_LightingSequence: {
            static const char flashStr_Log_LightingSequence[] PROGMEM = {" lighting sequence"};
            return flashStr_Log_LightingSequence;
        } break;
        case HS_Log_MeasuredPumping: {
            static const char flashStr_Log_MeasuredPumping[] PROGMEM = {" pumping result:"};
            return flashStr_Log_MeasuredPumping;
        } break;
        case HS_Log_PreFeedBalancing: {
            static const char flashStr_Log_PreFeedBalancing[] PROGMEM = {" pre-feed balancing"};
            return flashStr_Log_PreFeedBalancing;
        } break;
        case HS_Log_PreFeedTopOff: {
            static const char flashStr_Log_PreFeedTopOff[] PROGMEM = {" pre-feed top-off"};
            return flashStr_Log_PreFeedTopOff;
        } break;
        case HS_Log_PreLightSpraying: {
            static const char flashStr_Log_PreLightSpraying[] PROGMEM = {" pre-light spraying"};
            return flashStr_Log_PreLightSpraying;
        } break;
        case HS_Log_SystemDataSaved: {
            static const char flashStr_Log_SystemDataSaved[] PROGMEM = {"System data saved"};
            return flashStr_Log_SystemDataSaved;
        } break;
        case HS_Log_SystemUptime: {
            static const char flashStr_Log_SystemUptime[] PROGMEM = {"System uptime: "};
            return flashStr_Log_SystemUptime;
        } break;

        case HS_Log_Field_pH_Setpoint: {
            static const char flashStr_Log_Field_pH_Setpoint[] PROGMEM = {"  ph Setpoint: "};
            return flashStr_Log_Field_pH_Setpoint;
        } break;
        case HS_Log_Field_TDS_Setpoint: {
            static const char flashStr_Log_Field_TDS_Setpoint[] PROGMEM = {"  TDS Setpoint: "};
            return flashStr_Log_Field_TDS_Setpoint;
        } break;
        case HS_Log_Field_Temp_Setpoint: {
            static const char flashStr_Log_Field_Temp_Setpoint[] PROGMEM = {"  Temp Setpoint: "};
            return flashStr_Log_Field_Temp_Setpoint;
        } break;
        case HS_Log_Field_CO2_Setpoint: {
            static const char flashStr_Log_Field_CO2_Setpoint[] PROGMEM = {"  CO2 Setpoint: "};
            return flashStr_Log_Field_CO2_Setpoint;
        } break;
        case HS_Log_Field_Time_Calculated: {
            static const char flashStr_Log_Field_Time_Calculated[] PROGMEM = {"  Pump Time: "};
            return flashStr_Log_Field_Time_Calculated;
        } break;
        case HS_Log_Field_Vol_Calculated: {
            static const char flashStr_Log_Field_Vol_Calculated[] PROGMEM = {"  Est. Vol: "};
            return flashStr_Log_Field_Vol_Calculated;
        } break;
        case HS_Log_Field_pH_Measured: {
            static const char flashStr_Log_Field_pH_Measured[] PROGMEM = {"  ph Sensor: "};
            return flashStr_Log_Field_pH_Measured;
        } break;
        case HS_Log_Field_TDS_Measured: {
            static const char flashStr_Log_Field_TDS_Measured[] PROGMEM = {"  TDS Sensor: "};
            return flashStr_Log_Field_TDS_Measured;
        } break;
        case HS_Log_Field_Temp_Measured: {
            static const char flashStr_Log_Field_Temp_Measured[] PROGMEM = {"  Temp Sensor: "};
            return flashStr_Log_Field_Temp_Measured;
        } break;
        case HS_Log_Field_CO2_Measured: {
            static const char flashStr_Log_Field_CO2_Measured[] PROGMEM = {"  CO2 Sensor: "};
            return flashStr_Log_Field_CO2_Measured;
        } break;
        case HS_Log_Field_Time_Measured: {
            static const char flashStr_Log_Field_Time_Measured[] PROGMEM = {"  Elapsed Time: "};
            return flashStr_Log_Field_Time_Measured;
        } break;
        case HS_Log_Field_Vol_Measured: {
            static const char flashStr_Log_Field_Vol_Measured[] PROGMEM = {"  Pumped Vol: "};
            return flashStr_Log_Field_Vol_Measured;
        } break;
        case HS_Log_Field_Time_Start: {
            static const char flashStr_Log_Field_Time_Start[] PROGMEM = {"  Start Time: "};
            return flashStr_Log_Field_Time_Start;
        } break;
        case HS_Log_Field_Time_Finish: {
            static const char flashStr_Log_Field_Time_Finish[] PROGMEM = {"  Finish Time: "};
            return flashStr_Log_Field_Time_Finish;
        } break;

        case HS_Key_ActiveLow: {
            static const char flashStr_Key_ActiveLow[] PROGMEM = {"activeLow"};
            return flashStr_Key_ActiveLow;
        } break;
        case HS_Key_AdditiveName: {
            static const char flashStr_Key_AdditiveName[] PROGMEM = {"additiveName"};
            return flashStr_Key_AdditiveName;
        } break;
        case HS_Key_AirReportInterval: {
            static const char flashStr_Key_AirReportInterval[] PROGMEM = {"airReportInterval"};
            return flashStr_Key_AirReportInterval;
        } break;
        case HS_Key_AirTempRange: {
            static const char flashStr_Key_AirTempRange[] PROGMEM = {"airTempRange"};
            return flashStr_Key_AirTempRange;
        } break;
        case HS_Key_AirTemperatureSensor: {
            static const char flashStr_Key_AirTemperatureSensor[] PROGMEM = {"airTempSensor"};
            return flashStr_Key_AirTemperatureSensor;
        } break;
        case HS_Key_AlwaysFilled: {
            static const char flashStr_Key_AlwaysFilled[] PROGMEM = {"alwaysFilled"};
            return flashStr_Key_AlwaysFilled;
        } break;
        case HS_Key_AutosaveEnabled: {
            static const char flashStr_Key_AutosaveEnabled[] PROGMEM = {"autosaveEnabled"};
            return flashStr_Key_AutosaveEnabled;
        } break;
        case HS_Key_AutosaveInterval: {
            static const char flashStr_Key_AutosaveInterval[] PROGMEM = {"autosaveInterval"};
            return flashStr_Key_AutosaveInterval;
        } break;
        case HS_Key_BaseFeedMultiplier: {
            static const char flashStr_Key_BaseFeedMultiplier[] PROGMEM = {"baseFeedMultiplier"};
            return flashStr_Key_BaseFeedMultiplier;
        } break;
        case HS_Key_CO2Levels: {
            static const char flashStr_Key_CO2Levels[] PROGMEM = {"co2Levels"};
            return flashStr_Key_CO2Levels;
        } break;
        case HS_Key_CO2Sensor: {
            static const char flashStr_Key_CO2Sensor[] PROGMEM = {"co2Sensor"};
            return flashStr_Key_CO2Sensor;
        } break;
        case HS_Key_CalibUnits: {
            static const char flashStr_Key_CalibUnits[] PROGMEM = {"calibUnits"};
            return flashStr_Key_CalibUnits;
        } break;
        case HS_Key_ComputeHeatIndex: {
            static const char flashStr_Key_ComputeHeatIndex[] PROGMEM = {"computeHeatIndex"};
            return flashStr_Key_ComputeHeatIndex;
        } break;
        case HS_Key_ContFlowRate: {
            static const char flashStr_Key_ContFlowRate[] PROGMEM = {"contFlowRate"};
            return flashStr_Key_ContFlowRate;
        } break;
        case HS_Key_ContPowerUsage: {
            static const char flashStr_Key_ContPowerUsage[] PROGMEM = {"contPowerUsage"};
            return flashStr_Key_ContPowerUsage;
        } break;
        case HS_Key_CropName: {
            static const char flashStr_Key_CropName[] PROGMEM = {"cropName"};
            return flashStr_Key_CropName;
        } break;
        case HS_Key_CropType: {
            static const char flashStr_Key_CropType[] PROGMEM = {"cropType"};
            return flashStr_Key_CropType;
        } break;
        case HS_Key_CtrlInMode: {
            static const char flashStr_Key_CtrlInMode[] PROGMEM = {"ctrlInMode"};
            return flashStr_Key_CtrlInMode;
        } break;
        case HS_Key_DHTType: {
            static const char flashStr_Key_DHTType[] PROGMEM = {"dhtType"};
            return flashStr_Key_DHTType;
        } break;
        case HS_Key_DailyLightHours: {
            static const char flashStr_Key_DailyLightHours[] PROGMEM = {"dailyLightHours"};
            return flashStr_Key_DailyLightHours;
        } break;
        case HS_Key_DataFilePrefix: {
            static const char flashStr_Key_DataFilePrefix[] PROGMEM = {"dataFilePrefix"};
            return flashStr_Key_DataFilePrefix;
        } break;
        case HS_Key_DetriggerTol: {
            static const char flashStr_Key_DetriggerTol[] PROGMEM = {"detriggerTol"};
            return flashStr_Key_DetriggerTol;
        } break;
        case HS_Key_DispOutMode: {
            static const char flashStr_Key_DispOutMode[] PROGMEM = {"dispOutMode"};
            return flashStr_Key_DispOutMode;
        } break;
        case HS_Key_EmptyTrigger: {
            static const char flashStr_Key_EmptyTrigger[] PROGMEM = {"emptyTrigger"};
            return flashStr_Key_EmptyTrigger;
        } break;
        case HS_Key_FeedReservoir: {
            static const char flashStr_Key_FeedReservoir[] PROGMEM = {"feedReservoir"};
            return flashStr_Key_FeedReservoir;
        } break;
        case HS_Key_FeedTimingMins: {
            static const char flashStr_Key_FeedTimingMins[] PROGMEM = {"feedTimingMins"};
            return flashStr_Key_FeedTimingMins;
        } break;
        case HS_Key_FeedingTrigger: {
            static const char flashStr_Key_FeedingTrigger[] PROGMEM = {"feedingTrigger"};
            return flashStr_Key_FeedingTrigger;
        } break;
        case HS_Key_FeedingWeight: {
            static const char flashStr_Key_FeedingWeight[] PROGMEM = {"feedingWeight"};
            return flashStr_Key_FeedingWeight;
        } break;
        case HS_Key_FilledTrigger: {
            static const char flashStr_Key_FilledTrigger[] PROGMEM = {"filledTrigger"};
            return flashStr_Key_FilledTrigger;
        } break;
        case HS_Key_Flags: {
            static const char flashStr_Key_Flags[] PROGMEM = {"flags"};
            return flashStr_Key_Flags;
        } break;
        case HS_Key_FlowRateSensor: {
            static const char flashStr_Key_FlowRateSensor[] PROGMEM = {"flowRateSensor"};
            return flashStr_Key_FlowRateSensor;
        } break;
        case HS_Key_FlowRateUnits: {
            static const char flashStr_Key_FlowRateUnits[] PROGMEM = {"flowRateUnits"};
            return flashStr_Key_FlowRateUnits;
        } break;
        case HS_Key_Id: {
            static const char flashStr_Key_Id[] PROGMEM = {"id"};
            return flashStr_Key_Id;
        } break;
        case HS_Key_InputBitRes: {
            static const char flashStr_Key_InputBitRes[] PROGMEM = {"inputBitRes"};
            return flashStr_Key_InputBitRes;
        } break;
        case HS_Key_InputInversion: {
            static const char flashStr_Key_InputInversion[] PROGMEM = {"inputInversion"};
            return flashStr_Key_InputInversion;
        } break;
        case HS_Key_InputPin: {
            static const char flashStr_Key_InputPin[] PROGMEM = {"inputPin"};
            return flashStr_Key_InputPin;
        } break;
        case HS_Key_Invasive: {
            static const char flashStr_Key_Invasive[] PROGMEM = {"invasive"};
            return flashStr_Key_Invasive;
        } break;
        case HS_Key_Large: {
            static const char flashStr_Key_Large[] PROGMEM = {"large"};
            return flashStr_Key_Large;
        } break;
        case HS_Key_LastChangeDate: {
            static const char flashStr_Key_LastChangeDate[] PROGMEM = {"lastChangeDate"};
            return flashStr_Key_LastChangeDate;
        } break;
        case HS_Key_LastFeedingDate: {
            static const char flashStr_Key_LastFeedingDate[] PROGMEM = {"lastFeedingDate"};
            return flashStr_Key_LastFeedingDate;
        } break;
        case HS_Key_LastPruningDate: {
            static const char flashStr_Key_LastPruningDate[] PROGMEM = {"lastPruningDate"};
            return flashStr_Key_LastPruningDate;
        } break;
        case HS_Key_LifeCycleWeeks: {
            static const char flashStr_Key_LifeCycleWeeks[] PROGMEM = {"lifeCycleWeeks"};
            return flashStr_Key_LifeCycleWeeks;
        } break;
        case HS_Key_LimitTrigger: {
            static const char flashStr_Key_LimitTrigger[] PROGMEM = {"limitTrigger"};
            return flashStr_Key_LimitTrigger;
        } break;
        case HS_Key_LogFilePrefix: {
            static const char flashStr_Key_LogFilePrefix[] PROGMEM = {"logFilePrefix"};
            return flashStr_Key_LogFilePrefix;
        } break;
        case HS_Key_LogLevel: {
            static const char flashStr_Key_LogLevel[] PROGMEM = {"logLevel"};
            return flashStr_Key_LogLevel;
        } break;
        case HS_Key_LogToSDCard: {
            static const char flashStr_Key_LogToSDCard[] PROGMEM = {"logToSDCard"};
            return flashStr_Key_LogToSDCard;
        } break;
        case HS_Key_Logger: {
            static const char flashStr_Key_Logger[] PROGMEM = {"logger"};
            return flashStr_Key_Logger;
        } break;
        case HS_Key_MaxActiveAtOnce: {
            static const char flashStr_Key_MaxActiveAtOnce[] PROGMEM = {"maxActiveAtOnce"};
            return flashStr_Key_MaxActiveAtOnce;
        } break;
        case HS_Key_MaxPower: {
            static const char flashStr_Key_MaxPower[] PROGMEM = {"maxPower"};
            return flashStr_Key_MaxPower;
        } break;
        case HS_Key_MaxVolume: {
            static const char flashStr_Key_MaxVolume[] PROGMEM = {"maxVolume"};
            return flashStr_Key_MaxVolume;
        } break;
        case HS_Key_Max: {
            static const char flashStr_Key_Max[] PROGMEM = {"max"};
            return flashStr_Key_Max;
        } break;
        case HS_Key_MeasureMode: {
            static const char flashStr_Key_MeasureMode[] PROGMEM = {"measureMode"};
            return flashStr_Key_MeasureMode;
        } break;
        case HS_Key_MeasurementRow: {
            static const char flashStr_Key_MeasurementRow[] PROGMEM = {"measurementRow"};
            return flashStr_Key_MeasurementRow;
        } break;
        case HS_Key_MeasurementUnits: {
            static const char flashStr_Key_MeasurementUnits[] PROGMEM = {"measurementUnits"};
            return flashStr_Key_MeasurementUnits;
        } break;
        case HS_Key_Min: {
            static const char flashStr_Key_Min[] PROGMEM = {"min"};
            return flashStr_Key_Min;
        } break;
        case HS_Key_MoistureSensor: {
            static const char flashStr_Key_MoistureSensor[] PROGMEM = {"moistureSensor"};
            return flashStr_Key_MoistureSensor;
        } break;
        case HS_Key_MoistureUnits: {
            static const char flashStr_Key_MoistureUnits[] PROGMEM = {"moistureUnits"};
            return flashStr_Key_MoistureUnits;
        } break;
        case HS_Key_Multiplier: {
            static const char flashStr_Key_Multiplier[] PROGMEM = {"multiplier"};
            return flashStr_Key_Multiplier;
        } break;
        case HS_Key_NightlyFeedRate: {
            static const char flashStr_Key_NightlyFeedRate[] PROGMEM = {"nightlyFeedRate"};
            return flashStr_Key_NightlyFeedRate;
        } break;
        case HS_Key_NumFeedingsToday: {
            static const char flashStr_Key_NumFeedingsToday[] PROGMEM = {"numFeedingsToday"};
            return flashStr_Key_NumFeedingsToday;
        } break;
        case HS_Key_Offset: {
            static const char flashStr_Key_Offset[] PROGMEM = {"offset"};
            return flashStr_Key_Offset;
        } break;
        case HS_Key_OutputBitRes: {
            static const char flashStr_Key_OutputBitRes[] PROGMEM = {"outputBitRes"};
            return flashStr_Key_OutputBitRes;
        } break;
        case HS_Key_OutputPin: {
            static const char flashStr_Key_OutputPin[] PROGMEM = {"outputPin"};
            return flashStr_Key_OutputPin;
        } break;
        case HS_Key_OutputReservoir: {
            static const char flashStr_Key_OutputReservoir[] PROGMEM = {"destReservoir"};
            return flashStr_Key_OutputReservoir;
        } break;
        case HS_Key_PHRange: {
            static const char flashStr_Key_PHRange[] PROGMEM = {"phRange"};
            return flashStr_Key_PHRange;
        } break;
        case HS_Key_PHSensor: {
            static const char flashStr_Key_PHSensor[] PROGMEM = {"phSensor"};
            return flashStr_Key_PHSensor;
        } break;
        case HS_Key_Perennial: {
            static const char flashStr_Key_Perennial[] PROGMEM = {"perennial"};
            return flashStr_Key_Perennial;
        } break;
        case HS_Key_PhaseDurationWeeks: {
            static const char flashStr_Key_PhaseDurationWeeks[] PROGMEM = {"phaseDurationWeeks"};
            return flashStr_Key_PhaseDurationWeeks;
        } break;
        case HS_Key_PollingInterval: {
            static const char flashStr_Key_PollingInterval[] PROGMEM = {"pollingInterval"};
            return flashStr_Key_PollingInterval;
        } break;
        case HS_Key_PowerSensor: {
            static const char flashStr_Key_PowerSensor[] PROGMEM = {"powerSensor"};
            return flashStr_Key_PowerSensor;
        } break;
        case HS_Key_PowerUnits: {
            static const char flashStr_Key_PowerUnits[] PROGMEM = {"powerUnits"};
            return flashStr_Key_PowerUnits;
        } break;
        case HS_Key_PreFeedAeratorMins: {
            static const char flashStr_Key_PreFeedAeratorMins[] PROGMEM = {"preFeedAeratorMins"};
            return flashStr_Key_PreFeedAeratorMins;
        } break;
        case HS_Key_PreLightSprayMins: {
            static const char flashStr_Key_PreLightSprayMins[] PROGMEM = {"preLightSprayMins"};
            return flashStr_Key_PreLightSprayMins;
        } break;
        case HS_Key_Pruning: {
            static const char flashStr_Key_Pruning[] PROGMEM = {"pruning"};
            return flashStr_Key_Pruning;
        } break;
        case HS_Key_PublishToSDCard: {
            static const char flashStr_Key_PublishToSDCard[] PROGMEM = {"publishToSDCard"};
            return flashStr_Key_PublishToSDCard;
        } break;
        case HS_Key_Publisher: {
            static const char flashStr_Key_Publisher[] PROGMEM = {"publisher"};
            return flashStr_Key_Publisher;
        } break;
        case HS_Key_PullupPin: {
            static const char flashStr_Key_PullupPin[] PROGMEM = {"pullupPin"};
            return flashStr_Key_PullupPin;
        } break;
        case HS_Key_RailName: {
            static const char flashStr_Key_RailName[] PROGMEM = {"railName"};
            return flashStr_Key_RailName;
        } break;
        case HS_Key_ReservoirName: {
            static const char flashStr_Key_ReservoirName[] PROGMEM = {"reservoirName"};
            return flashStr_Key_ReservoirName;
        } break;
        case HS_Key_ReservoirType: {
            static const char flashStr_Key_ReservoirType[] PROGMEM = {"reservoirType"};
            return flashStr_Key_ReservoirType;
        } break;
        case HS_Key_Revision: {
            static const char flashStr_Key_Revision[] PROGMEM = {"revision"};
            return flashStr_Key_Revision;
        } break;
        case HS_Key_Scheduler: {
            static const char flashStr_Key_Scheduler[] PROGMEM = {"scheduler"};
            return flashStr_Key_Scheduler;
        } break;
        case HS_Key_SensorName: {
            static const char flashStr_Key_SensorName[] PROGMEM = {"sensorName"};
            return flashStr_Key_SensorName;
        } break;
        case HS_Key_SowDate: {
            static const char flashStr_Key_SowDate[] PROGMEM = {"sowDate"};
            return flashStr_Key_SowDate;
        } break;
        case HS_Key_Spraying: {
            static const char flashStr_Key_Spraying[] PROGMEM = {"spraying"};
            return flashStr_Key_Spraying;
        } break;
        case HS_Key_State: {
            static const char flashStr_Key_State[] PROGMEM = {"state"};
            return flashStr_Key_State;
        } break;
        case HS_Key_StdDosingRates: {
            static const char flashStr_Key_StdDosingRates[] PROGMEM = {"stdDosingRates"};
            return flashStr_Key_StdDosingRates;
        } break;
        case HS_Key_SubstrateType: {
            static const char flashStr_Key_SubstrateType[] PROGMEM = {"substrateType"};
            return flashStr_Key_SubstrateType;
        } break;
        case HS_Key_SystemMode: {
            static const char flashStr_Key_SystemMode[] PROGMEM = {"systemMode"};
            return flashStr_Key_SystemMode;
        } break;
        case HS_Key_SystemName: {
            static const char flashStr_Key_SystemName[] PROGMEM = {"systemName"};
            return flashStr_Key_SystemName;
        } break;
        case HS_Key_TDSRange: {
            static const char flashStr_Key_TDSRange[] PROGMEM = {"tdsRange"};
            return flashStr_Key_TDSRange;
        } break;
        case HS_Key_TDSSensor: {
            static const char flashStr_Key_TDSSensor[] PROGMEM = {"tdsSensor"};
            return flashStr_Key_TDSSensor;
        } break;
        case HS_Key_TDSUnits: {
            static const char flashStr_Key_TDSUnits[] PROGMEM = {"tdsUnits"};
            return flashStr_Key_TDSUnits;
        } break;
        case HS_Key_TemperatureSensor: {
            static const char flashStr_Key_TemperatureSensor[] PROGMEM = {"tempSensor"};
            return flashStr_Key_TemperatureSensor;
        } break;
        case HS_Key_TempUnits: {
            static const char flashStr_Key_TempUnits[] PROGMEM = {"tempUnits"};
            return flashStr_Key_TempUnits;
        } break;
        case HS_Key_TimeZoneOffset: {
            static const char flashStr_Key_TimeZoneOffset[] PROGMEM = {"timeZoneOffset"};
            return flashStr_Key_TimeZoneOffset;
        } break;
        case HS_Key_Timestamp: {
            static const char flashStr_Key_Timestamp[] PROGMEM = {"timestamp"};
            return flashStr_Key_Timestamp;
        } break;
        case HS_Key_ToleranceHigh: {
            static const char flashStr_Key_ToleranceHigh[] PROGMEM = {"toleranceHigh"};
            return flashStr_Key_ToleranceHigh;
        } break;
        case HS_Key_ToleranceLow: {
            static const char flashStr_Key_ToleranceLow[] PROGMEM = {"toleranceLow"};
            return flashStr_Key_ToleranceLow;
        } break;
        case HS_Key_ToleranceUnits: {
            static const char flashStr_Key_ToleranceUnits[] PROGMEM = {"toleranceUnits"};
            return flashStr_Key_ToleranceUnits;
        } break;
        case HS_Key_Tolerance: {
            static const char flashStr_Key_Tolerance[] PROGMEM = {"tolerance"};
            return flashStr_Key_Tolerance;
        } break;
        case HS_Key_TotalFeedingsDay: {
            static const char flashStr_Key_TotalFeedingsDay[] PROGMEM = {"totalFeedingsDay"};
            return flashStr_Key_TotalFeedingsDay;
        } break;
        case HS_Key_TotalGrowWeeks: {
            static const char flashStr_Key_TotalGrowWeeks[] PROGMEM = {"totalGrowWeeks"};
            return flashStr_Key_TotalGrowWeeks;
        } break;
        case HS_Key_Toxic: {
            static const char flashStr_Key_Toxic[] PROGMEM = {"toxic"};
            return flashStr_Key_Toxic;
        } break;
        case HS_Key_TriggerBelow: {
            static const char flashStr_Key_TriggerBelow[] PROGMEM = {"triggerBelow"};
            return flashStr_Key_TriggerBelow;
        } break;
        case HS_Key_TriggerOutside: {
            static const char flashStr_Key_TriggerOutside[] PROGMEM = {"triggerOutside"};
            return flashStr_Key_TriggerOutside;
        } break;
        case HS_Key_Type: {
            static const char flashStr_Key_Type[] PROGMEM = {"type"};
            return flashStr_Key_Type;
        } break;
        case HS_Key_Units: {
            static const char flashStr_Key_Units[] PROGMEM = {"units"};
            return flashStr_Key_Units;
        } break;
        case HS_Key_UsingISR: {
            static const char flashStr_Key_UsingISR[] PROGMEM = {"usingISR"};
            return flashStr_Key_UsingISR;
        } break;
        case HS_Key_Value: {
            static const char flashStr_Key_Value[] PROGMEM = {"value"};
            return flashStr_Key_Value;
        } break;
        case HS_Key_Version: {
            static const char flashStr_Key_Version[] PROGMEM = {"version"};
            return flashStr_Key_Version;
        } break;
        case HS_Key_Viner: {
            static const char flashStr_Key_Viner[] PROGMEM = {"viner"};
            return flashStr_Key_Viner;
        } break;
        case HS_Key_VolumeSensor: {
            static const char flashStr_Key_VolumeSensor[] PROGMEM = {"volumeSensor"};
            return flashStr_Key_VolumeSensor;
        } break;
        case HS_Key_VolumeUnits: {
            static const char flashStr_Key_VolumeUnits[] PROGMEM = {"volumeUnits"};
            return flashStr_Key_VolumeUnits;
        } break;
        case HS_Key_WaterTemperatureRange: {
            static const char flashStr_Key_WaterTemperatureRange[] PROGMEM = {"waterTempRange"};
            return flashStr_Key_WaterTemperatureRange;
        } break;
        case HS_Key_WaterTemperatureSensor: {
            static const char flashStr_Key_WaterTemperatureSensor[] PROGMEM = {"waterTempSensor"};
            return flashStr_Key_WaterTemperatureSensor;
        } break;
        case HS_Key_WeeklyDosingRates: {
            static const char flashStr_Key_WeeklyDosingRates[] PROGMEM = {"weeklyDosingRates"};
            return flashStr_Key_WeeklyDosingRates;
        } break;
        case HS_Key_WiFiPasswordSeed: {
            static const char flashStr_Key_WiFiPasswordSeed[] PROGMEM = {"wifiPwSeed"};
            return flashStr_Key_WiFiPasswordSeed;
        } break;
        case HS_Key_WiFiPassword: {
            static const char flashStr_Key_WiFiPassword[] PROGMEM = {"wifiPassword"};
            return flashStr_Key_WiFiPassword;
        } break;
        case HS_Key_WiFiSSID: {
            static const char flashStr_Key_WiFiSSID[] PROGMEM = {"wifiSSID"};
            return flashStr_Key_WiFiSSID;
        } break;
        case HS_Key_WireDevAddress: {
            static const char flashStr_Key_WireDevAddress[] PROGMEM = {"wireDevAddress"};
            return flashStr_Key_WireDevAddress;
        } break;
        case HS_Key_WirePosIndex: {
            static const char flashStr_Key_WirePosIndex[] PROGMEM = {"wirePosIndex"};
            return flashStr_Key_WirePosIndex;
        } break;

        default:
            return nullptr;
    }
}

#endif
