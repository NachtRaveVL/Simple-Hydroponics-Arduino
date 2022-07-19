/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Strings
*/

#include "Hydroponics.h"

const char PROGMEM HS_Count[] = {"Count"};
const char PROGMEM HS_Default[] = {"Default"};
const char PROGMEM HS_Disabled[] = {"Disabled"};
const char PROGMEM HS_Null[] = {"null"};
const char PROGMEM HS_Undefined[] = {"Undefined"};

const char PROGMEM HS_Err_AllocationFailure[] = {"Allocation failure"};
const char PROGMEM HS_Err_AssertionFailure[] = {"Assertion failure"};
const char PROGMEM HS_Err_DataAlreadyInitialized[] = {"Data already initialized"};
const char PROGMEM HS_Err_DataNotYetInitialized[] = {"Data not yet initialized"};
const char PROGMEM HS_Err_ExportFailure[] = {"Export failure"};
const char PROGMEM HS_Err_ImportFailure[] = {"Import failure"};
const char PROGMEM HS_Err_InitializationFailure[] = {"Initialization failure"};
const char PROGMEM HS_Err_InvalidParameter[] = {"Invalid parameter"};
const char PROGMEM HS_Err_InvalidPin[] = {"Invalid pin or type"};
const char PROGMEM HS_Err_MeasurementFailure[] = {"Measurement failure"};
const char PROGMEM HS_Err_MissingLinkage[] = {"Missing or no linkage"};
const char PROGMEM HS_Err_NoPositionsAvailable[] = {"No positions available"};
const char PROGMEM HS_Err_OperationFailure[] = {"Operation failure"};
const char PROGMEM HS_Err_ParameterMismatch[] = {"Parameter mismatch"};
const char PROGMEM HS_Err_UnsupportedOperation[] = {"Unsupported operation"};

const char PROGMEM HS_Key_ActiveLow[] = {"activeLow"};
const char PROGMEM HS_Key_AdditiveName[] = {"additiveName"};
const char PROGMEM HS_Key_AirTempRange[] = {"airTempRange"};
const char PROGMEM HS_Key_AirTempSensorName[] = {"airTempSensorName"};
const char PROGMEM HS_Key_AlwaysFilled[] = {"alwaysFilled"};
const char PROGMEM HS_Key_BaseFeedMultiplier[] = {"baseFeedMultiplier"};
const char PROGMEM HS_Key_Bloom[] = {"bloom"};
const char PROGMEM HS_Key_CO2Levels[] = {"co2Levels"};
const char PROGMEM HS_Key_CO2SensorName[] = {"co2SensorName"};
const char PROGMEM HS_Key_CalibUnits[] = {"calibUnits"};
const char PROGMEM HS_Key_ComputeHeatIndex[] = {"computeHeatIndex"};
const char PROGMEM HS_Key_ContFlowRate[] = {"contFlowRate"};
const char PROGMEM HS_Key_ContPowerDraw[] = {"contPowerDraw"};
const char PROGMEM HS_Key_CropName[] = {"cropName"};
const char PROGMEM HS_Key_CropType[] = {"cropType"};
const char PROGMEM HS_Key_CtrlInMode[] = {"ctrlInMode"};
const char PROGMEM HS_Key_DHTType[] = {"dhtType"};
const char PROGMEM HS_Key_DailyLightHours[] = {"dailyLightHours"};
const char PROGMEM HS_Key_DetriggerTolerance[] = {"detriggerTolerance"};
const char PROGMEM HS_Key_DispOutMode[] = {"dispOutMode"};
const char PROGMEM HS_Key_EmptyTrigger[] = {"emptyTrigger"};
const char PROGMEM HS_Key_FeedReservoirName[] = {"feedReservoirName"};
const char PROGMEM HS_Key_FeedTimingMins[] = {"feedTimingMins"};
const char PROGMEM HS_Key_FeedingTrigger[] = {"feedingTrigger"};
const char PROGMEM HS_Key_FeedingWeight[] = {"feedingWeight"};
const char PROGMEM HS_Key_FilledTrigger[] = {"filledTrigger"};
const char PROGMEM HS_Key_Flags[] = {"flags"};
const char PROGMEM HS_Key_FlowRateSensorName[] = {"flowRateSensorName"};
const char PROGMEM HS_Key_FlowRateUnits[] = {"flowRateUnits"};
const char PROGMEM HS_Key_Grow[] = {"grow"};
const char PROGMEM HS_Key_Id[] = {"id"};
const char PROGMEM HS_Key_InputBitRes[] = {"inputBitRes"};
const char PROGMEM HS_Key_InputInversion[] = {"inputInversion"};
const char PROGMEM HS_Key_InputPin[] = {"inputPin"};
const char PROGMEM HS_Key_Invasive[] = {"invasive"};
const char PROGMEM HS_Key_Large[] = {"large"};
const char PROGMEM HS_Key_LastChangeDate[] = {"lastChangeDate"};
const char PROGMEM HS_Key_LastFeedingDate[] = {"lastFeedingDate"};
const char PROGMEM HS_Key_LastPruningDate[] = {"lastPruningDate"};
const char PROGMEM HS_Key_LifeCycleWeeks[] = {"lifeCycleWeeks"};
const char PROGMEM HS_Key_LimitTrigger[] = {"limitTrigger"};
const char PROGMEM HS_Key_MaxActiveAtOnce[] = {"maxActiveAtOnce"};
const char PROGMEM HS_Key_MaxPower[] = {"maxPower"};
const char PROGMEM HS_Key_MaxVolume[] = {"maxVolume"};
const char PROGMEM HS_Key_Max[] = {"max"};
const char PROGMEM HS_Key_MeasureMode[] = {"measureMode"};
const char PROGMEM HS_Key_MeasurementRow[] = {"measurementRow"};
const char PROGMEM HS_Key_MeasurementUnits[] = {"measurementUnits"};
const char PROGMEM HS_Key_Min[] = {"min"};
const char PROGMEM HS_Key_MoistureSensorName[] = {"moistureSensorName"};
const char PROGMEM HS_Key_MoistureUnits[] = {"moistureUnits"};
const char PROGMEM HS_Key_Multiplier[] = {"multiplier"};
const char PROGMEM HS_Key_NightlyFeedMultiplier[] = {"nightlyFeedMultiplier"};
const char PROGMEM HS_Key_NumFeedingsToday[] = {"numFeedingsToday"};
const char PROGMEM HS_Key_Offset[] = {"offset"};
const char PROGMEM HS_Key_OutputBitRes[] = {"outputBitRes"};
const char PROGMEM HS_Key_OutputPin[] = {"outputPin"};
const char PROGMEM HS_Key_OutputReservoirName[] = {"outputReservoirName"};
const char PROGMEM HS_Key_PHRange[] = {"phRange"};
const char PROGMEM HS_Key_PHSensorName[] = {"phSensorName"};
const char PROGMEM HS_Key_Perennial[] = {"perennial"};
const char PROGMEM HS_Key_PhaseDurationWeeks[] = {"phaseDurationWeeks"};
const char PROGMEM HS_Key_PollingInterval[] = {"pollingInterval"};
const char PROGMEM HS_Key_PowerSensorName[] = {"powerSensorName"};
const char PROGMEM HS_Key_PowerUnits[] = {"powerUnits"};
const char PROGMEM HS_Key_PreFeedAeratorMins[] = {"preFeedAeratorMins"};
const char PROGMEM HS_Key_PreLightSprayMins[] = {"preLightSprayMins"};
const char PROGMEM HS_Key_Pruning[] = {"pruning"};
const char PROGMEM HS_Key_PullupPin[] = {"pullupPin"};
const char PROGMEM HS_Key_RailName[] = {"railName"};
const char PROGMEM HS_Key_ReservoirName[] = {"reservoirName"};
const char PROGMEM HS_Key_ReservoirType[] = {"reservoirType"};
const char PROGMEM HS_Key_Revision[] = {"_rev"};
const char PROGMEM HS_Key_Scheduler[] = {"scheduler"};
const char PROGMEM HS_Key_Seed[] = {"seed"};
const char PROGMEM HS_Key_SensorName[] = {"sensorName"};
const char PROGMEM HS_Key_SowDate[] = {"sowDate"};
const char PROGMEM HS_Key_Spraying[] = {"spraying"};
const char PROGMEM HS_Key_State[] = {"state"};
const char PROGMEM HS_Key_StdDosingRates[] = {"stdDosingRates"};
const char PROGMEM HS_Key_SubstrateType[] = {"substrateType"};
const char PROGMEM HS_Key_SystemMode[] = {"systemMode"};
const char PROGMEM HS_Key_SystemName[] = {"systemName"};
const char PROGMEM HS_Key_TDSRange[] = {"tdsRange"};
const char PROGMEM HS_Key_TDSSensorName[] = {"tdsSensorName"};
const char PROGMEM HS_Key_TDSUnits[] = {"tdsUnits"};
const char PROGMEM HS_Key_TempSensorName[] = {"tempSensorName"};
const char PROGMEM HS_Key_TempUnits[] = {"tempUnits"};
const char PROGMEM HS_Key_TimeZoneOffset[] = {"timeZoneOffset"};
const char PROGMEM HS_Key_Timestamp[] = {"timestamp"};
const char PROGMEM HS_Key_ToleranceHigh[] = {"toleranceHigh"};
const char PROGMEM HS_Key_ToleranceLow[] = {"toleranceLow"};
const char PROGMEM HS_Key_ToleranceUnits[] = {"toleranceUnits"};
const char PROGMEM HS_Key_Tolerance[] = {"tolerance"};
const char PROGMEM HS_Key_TotalFeedingsDay[] = {"totalFeedingsDay"};
const char PROGMEM HS_Key_TotalGrowWeeks[] = {"totalGrowWeeks"};
const char PROGMEM HS_Key_Toxic[] = {"toxic"};
const char PROGMEM HS_Key_TriggerBelow[] = {"triggerBelow"};
const char PROGMEM HS_Key_TriggerOutside[] = {"triggerOutside"};
const char PROGMEM HS_Key_Type[] = {"type"};
const char PROGMEM HS_Key_Unit[] = {"unit"};
const char PROGMEM HS_Key_Units[] = {"units"};
const char PROGMEM HS_Key_UsingISR[] = {"usingISR"};
const char PROGMEM HS_Key_Value[] = {"value"};
const char PROGMEM HS_Key_Values[] = {"values"};
const char PROGMEM HS_Key_Version[] = {"_ver"};
const char PROGMEM HS_Key_Viner[] = {"viner"};
const char PROGMEM HS_Key_VolumeSensorName[] = {"volumeSensorName"};
const char PROGMEM HS_Key_VolumeUnits[] = {"volumeUnits"};
const char PROGMEM HS_Key_WaterTempRange[] = {"waterTempRange"};
const char PROGMEM HS_Key_WaterTempSensorName[] = {"waterTempSensorName"};
const char PROGMEM HS_Key_WeeklyDosingRates[] = {"weeklyDosingRates"};
const char PROGMEM HS_Key_WiFiPasswordSeed[] = {"wifiPasswordSeed"};
const char PROGMEM HS_Key_WiFiPassword[] = {"wifiPassword"};
const char PROGMEM HS_Key_WiFiSSID[] = {"wifiSSID"};
const char PROGMEM HS_Key_WireDevAddress[] = {"wireDevAddress"};
const char PROGMEM HS_Key_WirePosIndex[] = {"wirePosIndex"};

String stringFromPGM(const char PROGMEM *str)
{
    String retVal; retVal.reserve(strlen_P(str));
    char readChar = pgm_read_byte(str++);
    while (readChar) {
        retVal.concat(readChar);
        readChar = pgm_read_byte(str++);
    }
    return retVal;
}
