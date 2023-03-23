/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Strings
*/

#ifndef HydroStrings_H
#define HydroStrings_H

// Strings Enumeration Table
enum Hydro_String : unsigned short {
    HStr_ColonSpace,
    HStr_DoubleSpace,
    HStr_csv,
    HStr_dat,
    HStr_Disabled,
    HStr_raw,
    HStr_txt,
    HStr_Undefined,
    HStr_null,

    HStr_Default_SystemName,
    HStr_Default_ConfigFilename,

    HStr_Err_AllocationFailure,
    HStr_Err_AlreadyInitialized,
    HStr_Err_AssertionFailure,
    HStr_Err_ExportFailure,
    HStr_Err_ImportFailure,
    HStr_Err_InitializationFailure,
    HStr_Err_InvalidParameter,
    HStr_Err_InvalidPinOrType,
    HStr_Err_MeasurementFailure,
    HStr_Err_MissingLinkage,
    HStr_Err_NoPositionsAvailable,
    HStr_Err_NotConfiguredProperly,
    HStr_Err_NotYetInitialized,
    HStr_Err_OperationFailure,
    HStr_Err_UnsupportedOperation,

    HStr_Log_AirReport,
    HStr_Log_CalculatedPumping,
    HStr_Log_FeedingSequence,
    HStr_Log_HasBegan,
    HStr_Log_HasDisabled,
    HStr_Log_HasEnabled,
    HStr_Log_HasEnded,
    HStr_Log_LightingSequence,
    HStr_Log_MeasuredPumping,
    HStr_Log_NatLightingSequence,
    HStr_Log_PreFeedBalancing,
    HStr_Log_PreFeedTopOff,
    HStr_Log_PreDawnSpraying,
    HStr_Log_RTCBatteryFailure,
    HStr_Log_SystemDataSaved,
    HStr_Log_SystemUptime,

    HStr_Log_Prefix_Info,
    HStr_Log_Prefix_Warning,
    HStr_Log_Prefix_Error,

    HStr_Log_Field_Aerator_Duration,
    HStr_Log_Field_CO2_Measured,
    HStr_Log_Field_CO2_Setpoint,
    HStr_Log_Field_Destination_Reservoir,
    HStr_Log_Field_Light_Duration,
    HStr_Log_Field_MixTime_Duration,
    HStr_Log_Field_pH_Measured,
    HStr_Log_Field_pH_Setpoint,
    HStr_Log_Field_Source_Reservoir,
    HStr_Log_Field_Sprayer_Duration,
    HStr_Log_Field_TDS_Measured,
    HStr_Log_Field_TDS_Setpoint,
    HStr_Log_Field_Temp_Measured,
    HStr_Log_Field_Temp_Setpoint,
    HStr_Log_Field_Time_Calculated,
    HStr_Log_Field_Time_Finish,
    HStr_Log_Field_Time_Measured,
    HStr_Log_Field_Time_Start,
    HStr_Log_Field_Vol_Calculated,
    HStr_Log_Field_Vol_Measured,

    HStr_Key_ActiveLow,
    HStr_Key_AdditiveName,
    HStr_Key_AirConcentrateUnits,
    HStr_Key_AirReportInterval,
    HStr_Key_AirTempRange,
    HStr_Key_AirTemperatureSensor,
    HStr_Key_AlwaysFilled,
    HStr_Key_AutosaveEnabled,
    HStr_Key_AutosaveFallback,
    HStr_Key_AutosaveInterval,
    HStr_Key_BaseFeedMultiplier,
    HStr_Key_BitRes,
    HStr_Key_CalibrationUnits,
    HStr_Key_Channel,
    HStr_Key_ComputeHeatIndex,
    HStr_Key_ConcentrateUnits,
    HStr_Key_ContinuousFlowRate,
    HStr_Key_ContinuousPowerUsage,
    HStr_Key_CO2Levels,
    HStr_Key_CO2Sensor,
    HStr_Key_CropName,
    HStr_Key_CtrlInMode,
    HStr_Key_DailyLightHours,
    HStr_Key_DataFilePrefix,
    HStr_Key_DetriggerDelay,
    HStr_Key_DetriggerTol,
    HStr_Key_DHTType,
    HStr_Key_DisplayTheme,
    HStr_Key_DispOutMode,
    HStr_Key_EmptyTrigger,
    HStr_Key_EnableMode,
    HStr_Key_FeedReservoir,
    HStr_Key_FeedTimingMins,
    HStr_Key_FeedingTrigger,
    HStr_Key_FeedingWeight,
    HStr_Key_FilledTrigger,
    HStr_Key_Flags,
    HStr_Key_FlowRateSensor,
    HStr_Key_FlowRateUnits,
    HStr_Key_Id,
    HStr_Key_InputInversion,
    HStr_Key_InputPin,
    HStr_Key_Invasive,
    HStr_Key_JoystickCalib,
    HStr_Key_Large,
    HStr_Key_LastChangeTime,
    HStr_Key_LastFeedingTime,
    HStr_Key_LastPruningTime,
    HStr_Key_LimitTrigger,
    HStr_Key_Location,
    HStr_Key_LogFilePrefix,
    HStr_Key_LogLevel,
    HStr_Key_LogToSDCard,
    HStr_Key_LogToWiFiStorage,
    HStr_Key_Logger,
    HStr_Key_MACAddress,
    HStr_Key_MaxActiveAtOnce,
    HStr_Key_MaxPower,
    HStr_Key_MaxVolume,
    HStr_Key_MeasureMode,
    HStr_Key_MeasurementRow,
    HStr_Key_MeasurementUnits,
    HStr_Key_Mode,
    HStr_Key_MoistureSensor,
    HStr_Key_Multiplier,
    HStr_Key_NaturalLightOffsetMins,
    HStr_Key_NightlyFeedRate,
    HStr_Key_NumFeedingsToday,
    HStr_Key_Offset,
    HStr_Key_OutputPin,
    HStr_Key_OutputReservoir,
    HStr_Key_Perennial,
    HStr_Key_PhaseDurationWeeks,
    HStr_Key_PHRange,
    HStr_Key_PHSensor,
    HStr_Key_Pin,
    HStr_Key_PollingInterval,
    HStr_Key_PowerSensor,
    HStr_Key_PowerUnits,
    HStr_Key_PreFeedAeratorMins,
    HStr_Key_PreDawnSprayMins,
    HStr_Key_Pruning,
    HStr_Key_PublishToSDCard,
    HStr_Key_PublishToWiFiStorage,
    HStr_Key_Publisher,
    HStr_Key_PullupPin,
    HStr_Key_PWMChannel,
    HStr_Key_PWMFrequency,
    HStr_Key_RailName,
    HStr_Key_ReservoirName,
    HStr_Key_Revision,
    HStr_Key_Scheduler,
    HStr_Key_SensorName,
    HStr_Key_SowTime,
    HStr_Key_Spraying,
    HStr_Key_State,
    HStr_Key_StdDosingRates,
    HStr_Key_SubstrateType,
    HStr_Key_SystemMode,
    HStr_Key_SystemName,
    HStr_Key_TDSRange,
    HStr_Key_TDSSensor,
    HStr_Key_TemperatureUnits,
    HStr_Key_TemperatureSensor,
    HStr_Key_TimeZoneOffset,
    HStr_Key_Timestamp,
    HStr_Key_Tolerance,
    HStr_Key_ToleranceHigh,
    HStr_Key_ToleranceLow,
    HStr_Key_TotalFeedingsPerDay,
    HStr_Key_TotalGrowWeeks,
    HStr_Key_Toxic,
    HStr_Key_TriggerBelow,
    HStr_Key_TriggerOutside,
    HStr_Key_Type,
    HStr_Key_Units,
    HStr_Key_UpdatesPerSec,
    HStr_Key_UsingISR,
    HStr_Key_Value,
    HStr_Key_Version,
    HStr_Key_Viner,
    HStr_Key_VolumeSensor,
    HStr_Key_VolumeUnits,
    HStr_Key_WaterConcentrateUnits,
    HStr_Key_WaterTemperatureRange,
    HStr_Key_WaterTemperatureSensor,
    HStr_Key_WeeklyDosingRates,
    HStr_Key_WiFiPassword,
    HStr_Key_WiFiPasswordSeed,
    HStr_Key_WiFiSSID,
    HStr_Key_WireDevAddress,
    HStr_Key_WirePosIndex,

    HStr_Enum_AC110V,
    HStr_Enum_AC220V,
    HStr_Enum_AirCarbonDioxide,
    HStr_Enum_AirTemperatureHumidity,
    HStr_Enum_Alkalinity,
    HStr_Enum_AloeVera,
    HStr_Enum_AnalogInput,
    HStr_Enum_AnalogJoystick,
    HStr_Enum_AnalogOutput,
    HStr_Enum_Anise,
    HStr_Enum_Artichoke,
    HStr_Enum_Arugula,
    HStr_Enum_AscOrder,
    HStr_Enum_Asparagus,
    HStr_Enum_Average,
    HStr_Enum_Basil,
    HStr_Enum_Bean,
    HStr_Enum_BeanBroad,
    HStr_Enum_Beetroot,
    HStr_Enum_BlackCurrant,
    HStr_Enum_Blueberry,
    HStr_Enum_BokChoi,
    HStr_Enum_Broccoli,
    HStr_Enum_BrusselsSprout,
    HStr_Enum_Cabbage,
    HStr_Enum_Cannabis,
    HStr_Enum_Capsicum,
    HStr_Enum_Carrots,
    HStr_Enum_Catnip,
    HStr_Enum_Cauliflower,
    HStr_Enum_Celery,
    HStr_Enum_Chamomile,
    HStr_Enum_Chicory,
    HStr_Enum_Chives,
    HStr_Enum_Cilantro,
    HStr_Enum_ClayPebbles,
    HStr_Enum_CoconutCoir,
    HStr_Enum_Concentration,
    HStr_Enum_Coriander,
    HStr_Enum_CornSweet,
    HStr_Enum_Cucumber,
    HStr_Enum_CustomAdditive1,
    HStr_Enum_CustomAdditive2,
    HStr_Enum_CustomAdditive3,
    HStr_Enum_CustomAdditive4,
    HStr_Enum_CustomAdditive5,
    HStr_Enum_CustomAdditive6,
    HStr_Enum_CustomAdditive7,
    HStr_Enum_CustomAdditive8,
    HStr_Enum_CustomAdditive9,
    HStr_Enum_CustomAdditive10,
    HStr_Enum_CustomAdditive11,
    HStr_Enum_CustomAdditive12,
    HStr_Enum_CustomAdditive13,
    HStr_Enum_CustomAdditive14,
    HStr_Enum_CustomAdditive15,
    HStr_Enum_CustomAdditive16,
    HStr_Enum_CustomCrop1,
    HStr_Enum_CustomCrop2,
    HStr_Enum_CustomCrop3,
    HStr_Enum_CustomCrop4,
    HStr_Enum_CustomCrop5,
    HStr_Enum_CustomCrop6,
    HStr_Enum_CustomCrop7,
    HStr_Enum_CustomCrop8,
    HStr_Enum_CustomOLED,
    HStr_Enum_DC12V,
    HStr_Enum_DC24V,
    HStr_Enum_DC3V3,
    HStr_Enum_DC48V,
    HStr_Enum_DC5V,
    HStr_Enum_DescOrder,
    HStr_Enum_DigitalInput,
    HStr_Enum_DigitalInputPullDown,
    HStr_Enum_DigitalInputPullUp,
    HStr_Enum_DigitalOutput,
    HStr_Enum_DigitalOutputPushPull,
    HStr_Enum_Dill,
    HStr_Enum_Distance,
    HStr_Enum_DrainageWater,
    HStr_Enum_DrainToWaste,
    HStr_Enum_Eggplant,
    HStr_Enum_Endive,
    HStr_Enum_FanExhaust,
    HStr_Enum_FeedWater,
    HStr_Enum_Fennel,
    HStr_Enum_Flowers,
    HStr_Enum_Fodder,
    HStr_Enum_FreshWater,
    HStr_Enum_Garlic,
    HStr_Enum_Ginger,
    HStr_Enum_GrowLights,
    HStr_Enum_Highest,
    HStr_Enum_IL3820,
    HStr_Enum_IL3820V2,
    HStr_Enum_ILI9341,
    HStr_Enum_Imperial,
    HStr_Enum_InOrder,
    HStr_Enum_Kale,
    HStr_Enum_Lavender,
    HStr_Enum_LCD16x2,
    HStr_Enum_LCD20x4,
    HStr_Enum_Leek,
    HStr_Enum_LemonBalm,
    HStr_Enum_Lettuce,
    HStr_Enum_LiqDilution,
    HStr_Enum_LiqFlowRate,
    HStr_Enum_LiqVolume,
    HStr_Enum_Lowest,
    HStr_Enum_Marrow,
    HStr_Enum_Matrix2x2,
    HStr_Enum_Matrix3x4,
    HStr_Enum_Matrix4x4,
    HStr_Enum_Melon,
    HStr_Enum_Metric,
    HStr_Enum_Mint,
    HStr_Enum_Multiply,
    HStr_Enum_MustardCress,
    HStr_Enum_NutrientPremix,
    HStr_Enum_Okra,
    HStr_Enum_Onions,
    HStr_Enum_Oregano,
    HStr_Enum_PakChoi,
    HStr_Enum_Parsley,
    HStr_Enum_Parsnip,
    HStr_Enum_Pea,
    HStr_Enum_PeaSugar,
    HStr_Enum_Pepino,
    HStr_Enum_PeppersBell,
    HStr_Enum_PeppersHot,
    HStr_Enum_Percentile,
    HStr_Enum_PeristalticPump,
    HStr_Enum_PhDownSolution,
    HStr_Enum_PhUpSolution,
    HStr_Enum_Potato,
    HStr_Enum_PotatoSweet,
    HStr_Enum_Power,
    HStr_Enum_PowerLevel,
    HStr_Enum_PumpFlow,
    HStr_Enum_Pumpkin,
    HStr_Enum_Radish,
    HStr_Enum_Recycling,
    HStr_Enum_RemoteControl,
    HStr_Enum_ResistiveTouch,
    HStr_Enum_RevOrder,
    HStr_Enum_Rhubarb,
    HStr_Enum_Rockwool,
    HStr_Enum_Rosemary,
    HStr_Enum_RotaryEncoder,
    HStr_Enum_Sage,
    HStr_Enum_Scientific,
    HStr_Enum_SH1106,
    HStr_Enum_Silverbeet,
    HStr_Enum_SoilMoisture,
    HStr_Enum_Spinach,
    HStr_Enum_Squash,
    HStr_Enum_SSD1305,
    HStr_Enum_SSD1305x32Ada,
    HStr_Enum_SSD1305x64Ada,
    HStr_Enum_SSD1306,
    HStr_Enum_SSD1607,
    HStr_Enum_ST7735,
    HStr_Enum_ST7789,
    HStr_Enum_Strawberries,
    HStr_Enum_Sunflower,
    HStr_Enum_SwissChard,
    HStr_Enum_Taro,
    HStr_Enum_Tarragon,
    HStr_Enum_Temperature,
    HStr_Enum_TFTTouch,
    HStr_Enum_Thyme,
    HStr_Enum_Tomato,
    HStr_Enum_TouchScreen,
    HStr_Enum_Turnip,
    HStr_Enum_UpDownButtons,
    HStr_Enum_UpDownESP32Touch,
    HStr_Enum_WaterAerator,
    HStr_Enum_WaterHeater,
    HStr_Enum_WaterHeight,
    HStr_Enum_WaterLevel,
    HStr_Enum_WaterPH,
    HStr_Enum_WaterPump,
    HStr_Enum_WaterSprayer,
    HStr_Enum_WaterTDS,
    HStr_Enum_WaterTemperature,
    HStr_Enum_Watercress,
    HStr_Enum_Watermelon,
    HStr_Enum_Weight,
    HStr_Enum_Zucchini,

    HStr_Unit_Count,
    HStr_Unit_Degree,
    HStr_Unit_EC5,
    HStr_Unit_Feet,
    HStr_Unit_Gallons,
    HStr_Unit_Kilograms,
    HStr_Unit_MilliLiterPer,
    HStr_Unit_PerMinute,
    HStr_Unit_pH14,
    HStr_Unit_Pounds,
    HStr_Unit_PPM500,
    HStr_Unit_PPM640,
    HStr_Unit_PPM700,
    HStr_Unit_Undefined,

    HStr_Count
};

// Blank string ("")
extern const char *HStr_Blank;

// Returns memory resident string from PROGMEM (Flash) string number.
extern String stringFromPGM(Hydro_String strNum);
#define SFP(strNum) stringFromPGM((strNum))

// Returns memory resident string from PROGMEM (Flash) string address.
String stringFromPGMAddr(const char *flashStr);

// Makes Strings lookup go through EEPROM, with specified data begin address.
extern void beginStringsFromEEPROM(uint16_t dataAddress);

// Makes Strings lookup go through SD card strings file at file prefix.
extern void beginStringsFromSDCard(String dataFilePrefix);

#ifndef HYDRO_DISABLE_BUILTIN_DATA
// Returns string from given PROGMEM (Flash) string address.
String stringFromPGMAddr(const char *flashStr);

// Returns PROGMEM (Flash) address pointer given string number.
const char *pgmAddrForStr(Hydro_String strNum);
#define CFP(strNum) pgmAddrForStr(strNum)
#else
#define CFP(strNum) SFP(strNum).c_str()
#endif

#endif // /ifndef HydroStrings_H
