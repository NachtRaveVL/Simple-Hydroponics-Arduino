/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Strings
*/

#ifndef HydroponicsStrings_H
#define HydroponicsStrings_H

extern const PROGMEM char HS_Count[];
extern const PROGMEM char HS_csv[];
extern const PROGMEM char HS_Disabled[];
extern const PROGMEM char HS_txt[];
extern const PROGMEM char HS_Undefined[];
extern const PROGMEM char HS_null[];

extern const PROGMEM char HS_Err_AllocationFailure[];
extern const PROGMEM char HS_Err_AlreadyInitialized[];
extern const PROGMEM char HS_Err_ExportFailure[];
extern const PROGMEM char HS_Err_ImportFailure[];
extern const PROGMEM char HS_Err_InitializationFailure[];
extern const PROGMEM char HS_Err_InvalidParameter[];
extern const PROGMEM char HS_Err_InvalidPinOrType[];
extern const PROGMEM char HS_Err_MeasurementFailure[];
extern const PROGMEM char HS_Err_MissingLinkage[];
extern const PROGMEM char HS_Err_NoPositionsAvailable[];
extern const PROGMEM char HS_Err_NotYetInitialized[];
extern const PROGMEM char HS_Err_OperationFailure[];
extern const PROGMEM char HS_Err_ParameterMismatch[];
extern const PROGMEM char HS_Err_UnsupportedOperation[];

extern const PROGMEM char HS_Log_EstimatedRunTime[];
extern const PROGMEM char HS_Log_FeedingSequence[];
extern const PROGMEM char HS_Log_HasBegan[];
extern const PROGMEM char HS_Log_HasDisabled[];
extern const PROGMEM char HS_Log_HasEnabled[];
extern const PROGMEM char HS_Log_HasEnded[];
extern const PROGMEM char HS_Log_LightingSequence[];
extern const PROGMEM char HS_Log_PollingFrame[];
extern const PROGMEM char HS_Log_SystemDataSaved[];
extern const PROGMEM char HS_Log_SystemUptime[];

extern const PROGMEM char HS_Key_ActiveLow[];
extern const PROGMEM char HS_Key_AdditiveName[];
extern const PROGMEM char HS_Key_AirTempRange[];
extern const PROGMEM char HS_Key_AirTempSensor[];
extern const PROGMEM char HS_Key_AlwaysFilled[];
extern const PROGMEM char HS_Key_AutosaveEnabled[];
extern const PROGMEM char HS_Key_AutosaveInterval[];
extern const PROGMEM char HS_Key_BaseFeedMultiplier[];
extern const PROGMEM char HS_Key_CO2Levels[];
extern const PROGMEM char HS_Key_CO2Sensor[];
extern const PROGMEM char HS_Key_CalibUnits[];
extern const PROGMEM char HS_Key_ComputeHeatIndex[];
extern const PROGMEM char HS_Key_ContFlowRate[];
extern const PROGMEM char HS_Key_ContPowerDraw[];
extern const PROGMEM char HS_Key_Crop[];
extern const PROGMEM char HS_Key_CropType[];
extern const PROGMEM char HS_Key_CtrlInMode[];
extern const PROGMEM char HS_Key_DHTType[];
extern const PROGMEM char HS_Key_DailyLightHours[];
extern const PROGMEM char HS_Key_DataFilePrefix[];
extern const PROGMEM char HS_Key_DetriggerTolerance[];
extern const PROGMEM char HS_Key_DispOutMode[];
extern const PROGMEM char HS_Key_EmptyTrigger[];
extern const PROGMEM char HS_Key_FeedReservoir[];
extern const PROGMEM char HS_Key_FeedTimingMins[];
extern const PROGMEM char HS_Key_FeedingTrigger[];
extern const PROGMEM char HS_Key_FeedingWeight[];
extern const PROGMEM char HS_Key_FilledTrigger[];
extern const PROGMEM char HS_Key_Flags[];
extern const PROGMEM char HS_Key_FlowRateSensor[];
extern const PROGMEM char HS_Key_FlowRateUnits[];
extern const PROGMEM char HS_Key_Id[];
extern const PROGMEM char HS_Key_InputBitRes[];
extern const PROGMEM char HS_Key_InputInversion[];
extern const PROGMEM char HS_Key_InputPin[];
extern const PROGMEM char HS_Key_Invasive[];
extern const PROGMEM char HS_Key_Large[];
extern const PROGMEM char HS_Key_LastChangeDate[];
extern const PROGMEM char HS_Key_LastFeedingDate[];
extern const PROGMEM char HS_Key_LastPruningDate[];
extern const PROGMEM char HS_Key_LifeCycleWeeks[];
extern const PROGMEM char HS_Key_LimitTrigger[];
extern const PROGMEM char HS_Key_LogFilePrefix[];
extern const PROGMEM char HS_Key_LogLevel[];
extern const PROGMEM char HS_Key_LogToSDCard[];
extern const PROGMEM char HS_Key_Logger[];
extern const PROGMEM char HS_Key_MaxActiveAtOnce[];
extern const PROGMEM char HS_Key_MaxPower[];
extern const PROGMEM char HS_Key_MaxVolume[];
extern const PROGMEM char HS_Key_Max[];
extern const PROGMEM char HS_Key_MeasureMode[];
extern const PROGMEM char HS_Key_MeasurementRow[];
extern const PROGMEM char HS_Key_MeasurementUnits[];
extern const PROGMEM char HS_Key_Min[];
extern const PROGMEM char HS_Key_MoistureSensor[];
extern const PROGMEM char HS_Key_MoistureUnits[];
extern const PROGMEM char HS_Key_Multiplier[];
extern const PROGMEM char HS_Key_NightlyFeedMultiplier[];
extern const PROGMEM char HS_Key_NumFeedingsToday[];
extern const PROGMEM char HS_Key_Offset[];
extern const PROGMEM char HS_Key_OutputBitRes[];
extern const PROGMEM char HS_Key_OutputPin[];
extern const PROGMEM char HS_Key_OutputReservoir[];
extern const PROGMEM char HS_Key_PHRange[];
extern const PROGMEM char HS_Key_PHSensor[];
extern const PROGMEM char HS_Key_Perennial[];
extern const PROGMEM char HS_Key_PhaseDurationWeeks[];
extern const PROGMEM char HS_Key_PollingInterval[];
extern const PROGMEM char HS_Key_PowerSensor[];
extern const PROGMEM char HS_Key_PowerUnits[];
extern const PROGMEM char HS_Key_PreFeedAeratorMins[];
extern const PROGMEM char HS_Key_PreLightSprayMins[];
extern const PROGMEM char HS_Key_Pruning[];
extern const PROGMEM char HS_Key_PublishToSDCard[];
extern const PROGMEM char HS_Key_Publisher[];
extern const PROGMEM char HS_Key_PullupPin[];
extern const PROGMEM char HS_Key_Rail[];
extern const PROGMEM char HS_Key_Reservoir[];
extern const PROGMEM char HS_Key_ReservoirType[];
extern const PROGMEM char HS_Key_Revision[];
extern const PROGMEM char HS_Key_Scheduler[];
extern const PROGMEM char HS_Key_Sensor[];
extern const PROGMEM char HS_Key_SowDate[];
extern const PROGMEM char HS_Key_Spraying[];
extern const PROGMEM char HS_Key_State[];
extern const PROGMEM char HS_Key_StdDosingRates[];
extern const PROGMEM char HS_Key_SubstrateType[];
extern const PROGMEM char HS_Key_SystemMode[];
extern const PROGMEM char HS_Key_SystemName[];
extern const PROGMEM char HS_Key_TDSRange[];
extern const PROGMEM char HS_Key_TDSSensor[];
extern const PROGMEM char HS_Key_TDSUnits[];
extern const PROGMEM char HS_Key_TempSensor[];
extern const PROGMEM char HS_Key_TempUnits[];
extern const PROGMEM char HS_Key_TimeZoneOffset[];
extern const PROGMEM char HS_Key_Timestamp[];
extern const PROGMEM char HS_Key_ToleranceHigh[];
extern const PROGMEM char HS_Key_ToleranceLow[];
extern const PROGMEM char HS_Key_ToleranceUnits[];
extern const PROGMEM char HS_Key_Tolerance[];
extern const PROGMEM char HS_Key_TotalFeedingsDay[];
extern const PROGMEM char HS_Key_TotalGrowWeeks[];
extern const PROGMEM char HS_Key_Toxic[];
extern const PROGMEM char HS_Key_TriggerBelow[];
extern const PROGMEM char HS_Key_TriggerOutside[];
extern const PROGMEM char HS_Key_Type[];
extern const PROGMEM char HS_Key_Units[];
extern const PROGMEM char HS_Key_UsingISR[];
extern const PROGMEM char HS_Key_Value[];
extern const PROGMEM char HS_Key_Version[];
extern const PROGMEM char HS_Key_Viner[];
extern const PROGMEM char HS_Key_VolumeSensor[];
extern const PROGMEM char HS_Key_VolumeUnits[];
extern const PROGMEM char HS_Key_WaterTempRange[];
extern const PROGMEM char HS_Key_WaterTempSensor[];
extern const PROGMEM char HS_Key_WeeklyDosingRates[];
extern const PROGMEM char HS_Key_WiFiPasswordSeed[];
extern const PROGMEM char HS_Key_WiFiPassword[];
extern const PROGMEM char HS_Key_WiFiSSID[];
extern const PROGMEM char HS_Key_WireDevAddress[];
extern const PROGMEM char HS_Key_WirePosIndex[];

extern String stringFromPGM(const char *str);
#define SFP(HStr) stringFromPGM((HStr))

#endif // /ifndef HydroponicsStrings_H
