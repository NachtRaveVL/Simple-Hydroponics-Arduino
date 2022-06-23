/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Defines
*/

#ifndef HydroponicsDefines_H
#define HydroponicsDefines_H

#ifndef FLT_EPSILON
#define FLT_EPSILON                     0.00001f            // Floating point error tolerance
#endif
#ifndef ENABLED
#define ENABLED                         0x1                 // Enabled define (convenience)
#endif
#ifndef DISABLED
#define DISABLED                        0x0                 // Disabled define (convenience)
#endif
#if defined(ESP8266) || defined(ESP32)
#define min _min
#define max _max
#endif

#define HYDRUINO_NAME_MAXSIZE               32                      // Naming character maximum size
#define HYDRUINO_ATPOS_MAXSIZE              16                      // Position indicies maximum size
#define HYDRUINO_CTRLINPINMAP_MAXSIZE       8                       // Control input pinmap maximum size
#define HYDRUINO_LINKAGE_BASESIZE           4                       // Base growth factor for linkage list
#define HYDRUINO_EEPROM_MEMORYSIZE          I2C_DEVICESIZE_24LC256  // Memory size for EEPROM (default: 256kb)
// TODO: Move HYDRUINO_EEPROM_MEMORYSIZE to input param on Hydroponics constructor.

typedef int8_t Hydroponics_PositionIndex;                           // Position indexing type
typedef uint32_t Hydroponics_KeyType;                               // Key type

#define HYDRUINO_ATPOS_SEARCH_FROMBEG       -1                      // Search from beginning to end, 0 up to MAXSIZE-1
#define HYDRUINO_ATPOS_SEARCH_FROMEND       HYDRUINO_ATPOS_MAXSIZE  // Search from end to beginning, MAXSIZE-1 down to 0
#define HYDRUINO_ATPOS_BEGFROM              1                       // Whenever position indexing starts at 1 or 0 (offset)

// Crop Type
// Common crop types. Controls what pH, EC, etc. that a plant prefers.
enum Hydroponics_CropType {
    Hydroponics_CropType_AloeVera,                          // Aloe Vera crop.
    Hydroponics_CropType_Anise,                             // Anise crop.
    Hydroponics_CropType_Artichoke,                         // Artichoke crop.
    Hydroponics_CropType_Arugula,                           // Arugula crop.
    Hydroponics_CropType_Asparagus,                         // Asparagus crop.
    Hydroponics_CropType_Basil,                             // Basil crop.
    Hydroponics_CropType_Bean,                              // Bean (common) crop.
    Hydroponics_CropType_BeanBroad,                         // Bean (broad) crop.
    Hydroponics_CropType_Beetroot,                          // Beetroot crop.
    Hydroponics_CropType_BlackCurrant,                      // Black Currant crop.
    Hydroponics_CropType_Blueberry,                         // Blueberry crop.
    Hydroponics_CropType_BokChoi,                           // Bok-choi crop.
    Hydroponics_CropType_Broccoli,                          // Broccoli crop.
    Hydroponics_CropType_BrusselsSprout,                    // Brussels Sprout crop.
    Hydroponics_CropType_Cabbage,                           // Cabbage crop.
    Hydroponics_CropType_Cannabis,                          // Cannabis (generic) crop.
    Hydroponics_CropType_Capsicum,                          // Capsicum crop.
    Hydroponics_CropType_Carrots,                           // Carrots crop.
    Hydroponics_CropType_Catnip,                            // Catnip crop.
    Hydroponics_CropType_Cauliflower,                       // Cauliflower crop.
    Hydroponics_CropType_Celery,                            // Celery crop.
    Hydroponics_CropType_Chamomile,                         // Chamomile crop.
    Hydroponics_CropType_Chicory,                           // Chiccory crop.
    Hydroponics_CropType_Chives,                            // Chives crop.
    Hydroponics_CropType_Cilantro,                          // Cilantro crop.
    Hydroponics_CropType_Coriander,                         // Coriander crop.
    Hydroponics_CropType_CornSweet,                         // Corn (sweet) crop.
    Hydroponics_CropType_Cucumber,                          // Cucumber crop.
    Hydroponics_CropType_Dill,                              // Dill crop.
    Hydroponics_CropType_Eggplant,                          // Eggplant crop.
    Hydroponics_CropType_Endive,                            // Endive crop.
    Hydroponics_CropType_Fennel,                            // Fennel crop.
    Hydroponics_CropType_Fodder,                            // Fodder crop.
    Hydroponics_CropType_Flowers,                           // Flowers (generic) crop.
    Hydroponics_CropType_Garlic,                            // Garlic crop.
    Hydroponics_CropType_Ginger,                            // Ginger crop.
    Hydroponics_CropType_Kale,                              // Kale crop.
    Hydroponics_CropType_Lavender,                          // Lavender crop.
    Hydroponics_CropType_Leek,                              // Leek crop.
    Hydroponics_CropType_LemonBalm,                         // Lemon Balm crop.
    Hydroponics_CropType_Lettuce,                           // Lettuce crop.
    Hydroponics_CropType_Marrow,                            // Marrow crop.
    Hydroponics_CropType_Melon,                             // Melon crop.
    Hydroponics_CropType_Mint,                              // Mint crop.
    Hydroponics_CropType_MustardCress,                      // Mustard Cress crop.
    Hydroponics_CropType_Okra,                              // Okra crop.
    Hydroponics_CropType_Onions,                            // Onions crop.
    Hydroponics_CropType_Oregano,                           // Oregano crop.
    Hydroponics_CropType_PakChoi,                           // Pak-choi crop.
    Hydroponics_CropType_Parsley,                           // Parsley crop.
    Hydroponics_CropType_Parsnip,                           // Parsnip crop.
    Hydroponics_CropType_Pea,                               // Pea (common) crop.
    Hydroponics_CropType_PeaSugar,                          // Pea (sugar) crop.
    Hydroponics_CropType_Pepino,                            // Pepino crop.
    Hydroponics_CropType_PeppersBell,                       // Peppers (bell) crop.
    Hydroponics_CropType_PeppersHot,                        // Peppers (hot) crop.
    Hydroponics_CropType_Potato,                            // Potato (common) crop.
    Hydroponics_CropType_PotatoSweet,                       // Potato (sweet) crop.
    Hydroponics_CropType_Pumpkin,                           // Pumpkin crop.
    Hydroponics_CropType_Radish,                            // Radish crop.
    Hydroponics_CropType_Rhubarb,                           // Rhubarb crop.
    Hydroponics_CropType_Rosemary,                          // Rosemary crop.
    Hydroponics_CropType_Sage,                              // Sage crop.
    Hydroponics_CropType_Silverbeet,                        // Silverbeet crop.
    Hydroponics_CropType_Spinach,                           // Spinach crop.
    Hydroponics_CropType_Squash,                            // Squash crop.
    Hydroponics_CropType_Sunflower,                         // Sunflower crop.
    Hydroponics_CropType_Strawberries,                      // Strawberries crop.
    Hydroponics_CropType_SwissChard,                        // Swiss chard crop.
    Hydroponics_CropType_Taro,                              // Taro crop.
    Hydroponics_CropType_Tarragon,                          // Tarragon crop.
    Hydroponics_CropType_Thyme,                             // Thyme crop.
    Hydroponics_CropType_Tomato,                            // Tomato crop.
    Hydroponics_CropType_Turnip,                            // Turnip crop.
    Hydroponics_CropType_Watercress,                        // Watercress crop.
    Hydroponics_CropType_Watermelon,                        // Watermelon crop.
    Hydroponics_CropType_Zucchini,                          // Zucchini crop.
    Hydroponics_CropType_Custom1,                           // Custom 1 crop.
    Hydroponics_CropType_Custom2,                           // Custom 2 crop.
    Hydroponics_CropType_Custom3,                           // Custom 3 crop.
    Hydroponics_CropType_Custom4,                           // Custom 4 crop.
    Hydroponics_CropType_Custom5,                           // Custom 5 crop.

    // FIXME: Need to change storage of crop data to PROGMEM. Capping CropType count until fixed.
    Hydroponics_CropType_Count = 10,                        // Internal use only
    Hydroponics_CropType_CustomCount = 5,                   // Internal use only
    Hydroponics_CropType_Undefined = -1                     // Internal use only
};

// Substrate Type
// Common substrate types. Influences feeding scheduling and environment control.
enum Hydroponics_SubstrateType {
    Hydroponics_SubstrateType_ClayPebbles,                  // Expanded clay pebbles substrate
    Hydroponics_SubstrateType_CoconutCoir,                  // Coconut coir substrate
    Hydroponics_SubstrateType_Rockwool,                     // Rockwool substrate

    Hydroponics_SubstrateType_Count,                        // Internal use only
    Hydroponics_SubstrateType_Undefined = -1                // Internal use only
};

// Crop Phase
// Common phases of crops. Influences feeding scheduling and environment control.
enum Hydroponics_CropPhase {
    Hydroponics_CropPhase_Seedling,                         // Initial seedling stage
    Hydroponics_CropPhase_Vegetative,                       // Vegetative stage
    Hydroponics_CropPhase_Flowering,                        // Flowering stage
    Hydroponics_CropPhase_Fruiting,                         // Fruiting stage
    Hydroponics_CropPhase_Harvest,                          // Harvest stage

    Hydroponics_CropPhase_Count,                            // Internal use only
    Hydroponics_CropPhase_Undefined = -1                    // Internal use only
};

// System Run Mode
// Specifies the general tank setup and waste connections.
enum Hydroponics_SystemMode {
    Hydroponics_SystemMode_Recycling,                       // System consistently recycles water in main feed water reservoir. Default setting.
    Hydroponics_SystemMode_DrainToWaste,                    // System fills feed reservoir before feeding (with pH/feed premix topoff prior), expects runoff (waste) to drain (unless drainage reservoir defined).

    Hydroponics_SystemMode_Count,                           // Internal use only
    Hydroponics_SystemMode_Undefined = -1                   // Internal use only
};

// Measurement Units Mode
// Specifies the standard of measurement style that units will use.
enum Hydroponics_MeasurementMode {
    Hydroponics_MeasurementMode_Imperial,                   // Imperial measurement mode (default setting, °F Ft Gal Lbs M-D-Y Val.X etc)
    Hydroponics_MeasurementMode_Metric,                     // Metric measurement mode (°C M L Kg Y-M-D Val.X etc)
    Hydroponics_MeasurementMode_Scientific,                 // Scientific measurement mode (°K M L Kg Y-M-D Val.XX etc)

    Hydroponics_MeasurementMode_Count,                      // Internal use only
    Hydroponics_MeasurementMode_Undefined = -1,             // Internal use only
    Hydroponics_MeasurementMode_Default = Hydroponics_MeasurementMode_Imperial // If you must change it
};

// LCD/Display Output Mode
// Specifies what kind of visual output device is to be used.
// Currently, all ouput devices must ultimately be supported by tcMenu.
enum Hydroponics_DisplayOutputMode {
    Hydroponics_DisplayOutputMode_Disabled,                 // No display output
    Hydroponics_DisplayOutputMode_20x4LCD,                  // 20x4 i2c LCD (with layout EN, RW, RS, BL, Data)
    Hydroponics_DisplayOutputMode_20x4LCD_Swapped,          // 20x4 i2c LCD (with EN<->RS swapped layout RS, RW, EN, BL, Data)
    Hydroponics_DisplayOutputMode_16x2LCD,                  // 16x2 i2c LCD (with layout EN, RW, RS, BL, Data)
    Hydroponics_DisplayOutputMode_16x2LCD_Swapped,          // 16x2 i2c LCD (with EN<->RS swapped layout RS, RW, EN, BL, Data)

    Hydroponics_DisplayOutputMode_Count,                    // Internal use only
    Hydroponics_DisplayOutputMode_Undefined = -1            // Internal use only
};

// Control Input Mode
// Specifies what kind of control input mode is to be used.
// Currently, all input devices must ultimately be supported by tcMenu.
enum Hydroponics_ControlInputMode {
    Hydroponics_ControlInputMode_Disabled,                  // No control input
    Hydroponics_ControlInputMode_2x2Matrix,                 // 2x2 directional keyboard matrix button array, ribbon: {L1,L2,R1,R2} (L1 = pin 1)
    Hydroponics_ControlInputMode_4xButton,                  // 4x standard momentary buttons, ribbon: {U,D,L,R} (U = pin 1)
    Hydroponics_ControlInputMode_6xButton,                  // 6x standard momentary buttons, ribbon: {TODO} (X = pin 1)
    Hydroponics_ControlInputMode_RotaryEncoder,             // Rotary encoder, ribbon: {A,B,OK,L,R} (A = pin 1)

    Hydroponics_ControlInputMode_Count,                     // Internal use only
    Hydroponics_ControlInputMode_Undefined = -1             // Internal use only
};

// Actuator Type
// Control actuator type. Specifies the various controllable equipment and their usage.
enum Hydroponics_ActuatorType {
    Hydroponics_ActuatorType_GrowLights,                    // Grow lights actuator
    Hydroponics_ActuatorType_WaterPump,                     // Water pump actuator (feed or drainage reservoir only)
    Hydroponics_ActuatorType_PeristalticPump,               // Peristaltic pump actuator (pH-up/down, nutrient, or fresh water reservoirs only)
    Hydroponics_ActuatorType_WaterHeater,                   // Water heater actuator (feed reservoir only)
    Hydroponics_ActuatorType_WaterAerator,                  // Water aerator actuator (feed reservoir only)
    Hydroponics_ActuatorType_FanExhaust,                    // Fan exhaust relay actuator

    Hydroponics_ActuatorType_Count,                         // Internal use only
    Hydroponics_ActuatorType_Undefined = -1                 // Internal use only
};

// Sensor Type
// Sensor device type. Specifies the various sensors and the kinds of things they measure.
enum Hydroponics_SensorType {
    Hydroponics_SensorType_AirTempHumidity,                 // Air temperature and humidity sensor (digital, front-panel)
    Hydroponics_SensorType_AirCarbonDioxide,                // Air CO2 sensor (analog, signal pin sometimes labeled as 'Ao')
    Hydroponics_SensorType_PotentialHydrogen,               // pH sensor (analog, signal pin sometimes labeled as 'Po', feed reservoir only)
    Hydroponics_SensorType_TotalDissolvedSolids,            // TDS salts electrode sensor (analog, feed reservoir only)
    Hydroponics_SensorType_WaterTemperature,                // DallasTemperature DS18* submersible sensor (analog)
    Hydroponics_SensorType_SoilMoisture,                    // Soil moisture sensor (analog)
    Hydroponics_SensorType_WaterPumpFlowSensor,             // Water pump flow hall sensor (PWM/analog)
    Hydroponics_SensorType_WaterLevelIndicator,             // Water level indicator (binary)
    Hydroponics_SensorType_WaterHeightMeter,                // Water height meter (analog)

    Hydroponics_SensorType_Count,                           // Internal use only
    Hydroponics_SensorType_Undefined = -1                   // Internal use only
};

// Reservoir Type
// Common fluid containers. Specifies the various operational containers.
enum Hydroponics_ReservoirType {
    Hydroponics_ReservoirType_FeedWater,                    // Feed water reservoir (aka main water reservoir)
    Hydroponics_ReservoirType_DrainageWater,                // Drainage water reservoir
    Hydroponics_ReservoirType_NutrientPremix,               // Nutrient premix reservoir/source
    Hydroponics_ReservoirType_FreshWater,                   // Fresh water reservoir/source
    Hydroponics_ReservoirType_PhUpSolution,                 // pH-Up solution
    Hydroponics_ReservoirType_PhDownSolution,               // pH-Down solution

    Hydroponics_ReservoirType_Count,                        // Internal use only
    Hydroponics_ReservoirType_Undefined = -1                // Internal use only
};

// Relay Power Rail
// Common powered relay rails. Specifies an isolated operational power rail.
enum Hydroponics_RailType {
    Hydroponics_RailType_ACPower,                           // AC-based power rail, for pumps, lights, heaters, etc.
    Hydroponics_RailType_DCPower,                           // DC-based power rail, for dosing pumps, PWM fans, sensors, etc.

    Hydroponics_RailType_Count,                             // Internal use only
    Hydroponics_RailType_CustomCount,                       // Internal use only
    Hydroponics_RailType_Undefined = -1                     // Internal use only
};

// Trigger Status
// Common trigger statuses. Specifies enablement and tripped state.
enum Hydroponics_TriggerState {
    Hydroponics_TriggerState_Disabled,                      // Triggers disabled (not hooked up)
    Hydroponics_TriggerState_NotTriggered,                  // Not triggered
    Hydroponics_TriggerState_Triggered,                     // Triggered

    Hydroponics_TriggerState_Count,                         // Internal use only
    Hydroponics_TriggerState_Undefined = -1                 // Internal use only
};


// Units Type
// Unit of measurement type. Specifies the unit type associated with a measured value.
enum Hydroponics_UnitsType {
    Hydroponics_UnitsType_Temperature_Celsius,              // Celsius temperature mode.
    Hydroponics_UnitsType_Temperature_Fahrenheit,           // Fahrenheit temperature mode.
    Hydroponics_UnitsType_Temperature_Kelvin,               // Kelvin temperature mode.
    Hydroponics_UnitsType_Distance_Meters,                  // Meters distance mode.
    Hydroponics_UnitsType_Distance_Feet,                    // Feet distance mode.
    Hydroponics_UnitsType_Weight_Kilogram,                  // Kilogram weight mode.
    Hydroponics_UnitsType_Weight_Pounds,                    // Pounds weight mode.
    Hydroponics_UnitsType_LiquidVolume_Liters,              // Liters liquid volume mode.
    Hydroponics_UnitsType_LiquidVolume_Gallons,             // Gallons liquid volume mode.
    Hydroponics_UnitsType_LiquidFlow_LitersPerMin,          // Liters per minute liquid flow mode.
    Hydroponics_UnitsType_LiquidFlow_GallonsPerMin,         // Gallons per minute liquid flow mode.
    Hydroponics_UnitsType_pHScale_0_14,                     // pH scale [0.0,14.0] mode.
    Hydroponics_UnitsType_Concentration_EC,                 // Siemens electrical conductivity mode.
    Hydroponics_UnitsType_Concentration_PPM500,             // Parts-per-million 500 (NaCl) concentration mode (US).
    Hydroponics_UnitsType_Concentration_PPM640,             // Parts-per-million 640 concentration mode (EU).
    Hydroponics_UnitsType_Concentration_PPM700,             // Parts-per-million 700 (KCl) concentration mode (AU).
    Hydroponics_UnitsType_Percentile_0_100,                 // Percentile [0.0,100.0] mode.
    Hydroponics_UnitsType_Raw_0_1,                          // Raw value [0.0,1.0] mode.

    Hydroponics_UnitsType_Count,                            // Internal use only
    Hydroponics_UnitsType_Concentration_PPMTDS = Hydroponics_UnitsType_Concentration_PPM500, // TDS PPM concentration mode alias.
    Hydroponics_UnitsType_Undefined = -1                    // Internal use only
};

class Hydroponics;
struct HydroponicsIdentity;
class HydroponicsObject;
class HydroponicsActuator;
class HydroponicsSensor;
class HydroponicsCrop;
class HydroponicsReservoir;
class HydroponicsRail;
struct HydroponicsData;
struct HydroponicsMeasurement;
struct HydroponicsSingleMeasurement;
class HydroponicsTrigger;
class HydroponicsCropsLibrary;

#endif // /ifndef HydroponicsDefines_H
