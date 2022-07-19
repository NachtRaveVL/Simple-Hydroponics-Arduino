/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Strings
*/

#include "Hydroponics.h"

const PROGMEM char HS_Count[] = {"Count"};
const PROGMEM char HS_Disabled[] = {"Disabled"};
const PROGMEM char HS_Null[] = {"null"};
const PROGMEM char HS_Undefined[] = {"Undefined"};

const PROGMEM char HS_Err_AllocationFailure[] = {"Allocation failure"};
const PROGMEM char HS_Err_AlreadyInitialized[] = {"Already initialized"};
const PROGMEM char HS_Err_ExportFailure[] = {"Export failure"};
const PROGMEM char HS_Err_ImportFailure[] = {"Import failure"};
const PROGMEM char HS_Err_InitializationFailure[] = {"Initialization failure"};
const PROGMEM char HS_Err_InvalidParameter[] = {"Invalid parameter"};
const PROGMEM char HS_Err_InvalidPinOrType[] = {"Invalid pin or type"};
const PROGMEM char HS_Err_MeasurementFailure[] = {"Measurement failure"};
const PROGMEM char HS_Err_MissingLinkage[] = {"Missing or no linkage"};
const PROGMEM char HS_Err_NoPositionsAvailable[] = {"No positions available"};
const PROGMEM char HS_Err_NotYetInitialized[] = {"Not yet initialized"};
const PROGMEM char HS_Err_OperationFailure[] = {"Operation failure"};
const PROGMEM char HS_Err_ParameterMismatch[] = {"Parameter mismatch"};
const PROGMEM char HS_Err_UnsupportedOperation[] = {"Unsupported operation"};

const PROGMEM char HS_Key_ActiveLow[] = {"activeLow"};
const PROGMEM char HS_Key_AdditiveName[] = {"additiveName"};
const PROGMEM char HS_Key_AirTempRange[] = {"airTempRange"};
const PROGMEM char HS_Key_AirTempSensorName[] = {"airTempSensorName"};
const PROGMEM char HS_Key_AlwaysFilled[] = {"alwaysFilled"};
const PROGMEM char HS_Key_BaseFeedMultiplier[] = {"baseFeedMultiplier"};
const PROGMEM char HS_Key_Bloom[] = {"bloom"};
const PROGMEM char HS_Key_CO2Levels[] = {"co2Levels"};
const PROGMEM char HS_Key_CO2SensorName[] = {"co2SensorName"};
const PROGMEM char HS_Key_CalibUnits[] = {"calibUnits"};
const PROGMEM char HS_Key_ComputeHeatIndex[] = {"computeHeatIndex"};
const PROGMEM char HS_Key_ContFlowRate[] = {"contFlowRate"};
const PROGMEM char HS_Key_ContPowerDraw[] = {"contPowerDraw"};
const PROGMEM char HS_Key_CropName[] = {"cropName"};
const PROGMEM char HS_Key_CropType[] = {"cropType"};
const PROGMEM char HS_Key_CtrlInMode[] = {"ctrlInMode"};
const PROGMEM char HS_Key_DHTType[] = {"dhtType"};
const PROGMEM char HS_Key_DailyLightHours[] = {"dailyLightHours"};
const PROGMEM char HS_Key_DetriggerTolerance[] = {"detriggerTolerance"};
const PROGMEM char HS_Key_DispOutMode[] = {"dispOutMode"};
const PROGMEM char HS_Key_EmptyTrigger[] = {"emptyTrigger"};
const PROGMEM char HS_Key_FeedReservoirName[] = {"feedReservoirName"};
const PROGMEM char HS_Key_FeedTimingMins[] = {"feedTimingMins"};
const PROGMEM char HS_Key_FeedingTrigger[] = {"feedingTrigger"};
const PROGMEM char HS_Key_FeedingWeight[] = {"feedingWeight"};
const PROGMEM char HS_Key_FilledTrigger[] = {"filledTrigger"};
const PROGMEM char HS_Key_Flags[] = {"flags"};
const PROGMEM char HS_Key_FlowRateSensorName[] = {"flowRateSensorName"};
const PROGMEM char HS_Key_FlowRateUnits[] = {"flowRateUnits"};
const PROGMEM char HS_Key_Grow[] = {"grow"};
const PROGMEM char HS_Key_Id[] = {"id"};
const PROGMEM char HS_Key_InputBitRes[] = {"inputBitRes"};
const PROGMEM char HS_Key_InputInversion[] = {"inputInversion"};
const PROGMEM char HS_Key_InputPin[] = {"inputPin"};
const PROGMEM char HS_Key_Invasive[] = {"invasive"};
const PROGMEM char HS_Key_Large[] = {"large"};
const PROGMEM char HS_Key_LastChangeDate[] = {"lastChangeDate"};
const PROGMEM char HS_Key_LastFeedingDate[] = {"lastFeedingDate"};
const PROGMEM char HS_Key_LastPruningDate[] = {"lastPruningDate"};
const PROGMEM char HS_Key_LifeCycleWeeks[] = {"lifeCycleWeeks"};
const PROGMEM char HS_Key_LimitTrigger[] = {"limitTrigger"};
const PROGMEM char HS_Key_MaxActiveAtOnce[] = {"maxActiveAtOnce"};
const PROGMEM char HS_Key_MaxPower[] = {"maxPower"};
const PROGMEM char HS_Key_MaxVolume[] = {"maxVolume"};
const PROGMEM char HS_Key_Max[] = {"max"};
const PROGMEM char HS_Key_MeasureMode[] = {"measureMode"};
const PROGMEM char HS_Key_MeasurementRow[] = {"measurementRow"};
const PROGMEM char HS_Key_MeasurementUnits[] = {"measurementUnits"};
const PROGMEM char HS_Key_Min[] = {"min"};
const PROGMEM char HS_Key_MoistureSensorName[] = {"moistureSensorName"};
const PROGMEM char HS_Key_MoistureUnits[] = {"moistureUnits"};
const PROGMEM char HS_Key_Multiplier[] = {"multiplier"};
const PROGMEM char HS_Key_NightlyFeedMultiplier[] = {"nightlyFeedMultiplier"};
const PROGMEM char HS_Key_NumFeedingsToday[] = {"numFeedingsToday"};
const PROGMEM char HS_Key_Offset[] = {"offset"};
const PROGMEM char HS_Key_OutputBitRes[] = {"outputBitRes"};
const PROGMEM char HS_Key_OutputPin[] = {"outputPin"};
const PROGMEM char HS_Key_OutputReservoirName[] = {"outputReservoirName"};
const PROGMEM char HS_Key_PHRange[] = {"phRange"};
const PROGMEM char HS_Key_PHSensorName[] = {"phSensorName"};
const PROGMEM char HS_Key_Perennial[] = {"perennial"};
const PROGMEM char HS_Key_PhaseDurationWeeks[] = {"phaseDurationWeeks"};
const PROGMEM char HS_Key_PollingInterval[] = {"pollingInterval"};
const PROGMEM char HS_Key_PowerSensorName[] = {"powerSensorName"};
const PROGMEM char HS_Key_PowerUnits[] = {"powerUnits"};
const PROGMEM char HS_Key_PreFeedAeratorMins[] = {"preFeedAeratorMins"};
const PROGMEM char HS_Key_PreLightSprayMins[] = {"preLightSprayMins"};
const PROGMEM char HS_Key_Pruning[] = {"pruning"};
const PROGMEM char HS_Key_PullupPin[] = {"pullupPin"};
const PROGMEM char HS_Key_RailName[] = {"railName"};
const PROGMEM char HS_Key_ReservoirName[] = {"reservoirName"};
const PROGMEM char HS_Key_ReservoirType[] = {"reservoirType"};
const PROGMEM char HS_Key_Revision[] = {"_rev"};
const PROGMEM char HS_Key_Scheduler[] = {"scheduler"};
const PROGMEM char HS_Key_Seed[] = {"seed"};
const PROGMEM char HS_Key_SensorName[] = {"sensorName"};
const PROGMEM char HS_Key_SowDate[] = {"sowDate"};
const PROGMEM char HS_Key_Spraying[] = {"spraying"};
const PROGMEM char HS_Key_State[] = {"state"};
const PROGMEM char HS_Key_StdDosingRates[] = {"stdDosingRates"};
const PROGMEM char HS_Key_SubstrateType[] = {"substrateType"};
const PROGMEM char HS_Key_SystemMode[] = {"systemMode"};
const PROGMEM char HS_Key_SystemName[] = {"systemName"};
const PROGMEM char HS_Key_TDSRange[] = {"tdsRange"};
const PROGMEM char HS_Key_TDSSensorName[] = {"tdsSensorName"};
const PROGMEM char HS_Key_TDSUnits[] = {"tdsUnits"};
const PROGMEM char HS_Key_TempSensorName[] = {"tempSensorName"};
const PROGMEM char HS_Key_TempUnits[] = {"tempUnits"};
const PROGMEM char HS_Key_TimeZoneOffset[] = {"timeZoneOffset"};
const PROGMEM char HS_Key_Timestamp[] = {"timestamp"};
const PROGMEM char HS_Key_ToleranceHigh[] = {"toleranceHigh"};
const PROGMEM char HS_Key_ToleranceLow[] = {"toleranceLow"};
const PROGMEM char HS_Key_ToleranceUnits[] = {"toleranceUnits"};
const PROGMEM char HS_Key_Tolerance[] = {"tolerance"};
const PROGMEM char HS_Key_TotalFeedingsDay[] = {"totalFeedingsDay"};
const PROGMEM char HS_Key_TotalGrowWeeks[] = {"totalGrowWeeks"};
const PROGMEM char HS_Key_Toxic[] = {"toxic"};
const PROGMEM char HS_Key_TriggerBelow[] = {"triggerBelow"};
const PROGMEM char HS_Key_TriggerOutside[] = {"triggerOutside"};
const PROGMEM char HS_Key_Type[] = {"type"};
const PROGMEM char HS_Key_Unit[] = {"unit"};
const PROGMEM char HS_Key_Units[] = {"units"};
const PROGMEM char HS_Key_UsingISR[] = {"usingISR"};
const PROGMEM char HS_Key_Value[] = {"value"};
const PROGMEM char HS_Key_Values[] = {"values"};
const PROGMEM char HS_Key_Version[] = {"_ver"};
const PROGMEM char HS_Key_Viner[] = {"viner"};
const PROGMEM char HS_Key_VolumeSensorName[] = {"volumeSensorName"};
const PROGMEM char HS_Key_VolumeUnits[] = {"volumeUnits"};
const PROGMEM char HS_Key_WaterTempRange[] = {"waterTempRange"};
const PROGMEM char HS_Key_WaterTempSensorName[] = {"waterTempSensorName"};
const PROGMEM char HS_Key_WeeklyDosingRates[] = {"weeklyDosingRates"};
const PROGMEM char HS_Key_WiFiPasswordSeed[] = {"wifiPasswordSeed"};
const PROGMEM char HS_Key_WiFiPassword[] = {"wifiPassword"};
const PROGMEM char HS_Key_WiFiSSID[] = {"wifiSSID"};
const PROGMEM char HS_Key_WireDevAddress[] = {"wireDevAddress"};
const PROGMEM char HS_Key_WirePosIndex[] = {"wirePosIndex"};

String stringFromPGM(const char *str)
{
    String retVal; retVal.reserve(strlen_P(str)+1);
    char readChar = pgm_read_byte(str++);
    while (readChar) {
        retVal.concat(readChar);
        readChar = pgm_read_byte(str++);
    }
    return retVal;
}
