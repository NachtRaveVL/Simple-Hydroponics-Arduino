/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Defines
*/

#ifndef HydroponicsDefines_H
#define HydroponicsDefines_H

#ifndef FLT_EPSILON
#define FLT_EPSILON                         0.00001f                // Single-precision floating point error tolerance
#endif
#ifndef FLT_UNDEF
#define FLT_UNDEF                           __FLT_MAX__             // A floating point value to stand in for "undefined"
#endif
#ifndef DBL_EPSILON
#define DBL_EPSILON                         0.0000000000001         // Double-precision floating point error tolerance
#endif
#ifndef ENABLED
#define ENABLED                             0x1                     // Enabled define (convenience)
#endif
#ifndef DISABLED
#define DISABLED                            0x0                     // Disabled define (convenience)
#endif
#ifndef RANDOM_MAX
#define RANDOM_MAX RAND_MAX
#endif
#if defined(ESP8266) || defined(ESP32)
#define min _min
#define max _max
#endif

typedef int8_t Hydroponics_PositionIndex;                           // Position indexing type
typedef uint32_t Hydroponics_KeyType;                               // Key type, for hashing

#define HYDRUINO_NAME_MAXSIZE               24                      // Naming character maximum size (system name, crop name, etc.)
#define HYDRUINO_POS_MAXSIZE                32                      // Position indicies maximum size (max # of objs of same type)
#define HYDRUINO_CTRLINPINMAP_MAXSIZE       8                       // Maximum array size for control input pinmap (max # of ribbon pins)
#define HYDRUINO_JSON_DOC_SYSSIZE           384                     // JSON document chunk size for reading in main system data (serialization buffer size)
#define HYDRUINO_JSON_DOC_DEFSIZE           256                     // Default JSON document chunk size (serialization buffer size)
// The following maxsizes only matter for architectures that do not have STL support
#define HYDRUINO_SYS_OBJECTS_MAXSIZE        24                      // Maximum array size for system objects (max # of objects in system)
#define HYDRUINO_OBJ_LINKS_MAXSIZE          12                      // Maximum array size for object linkage list, per obj (max # of linked objects)
#define HYDRUINO_OBJ_LINKSFILTER_DEFSIZE    8                       // Default array size for object linkage filtering
#define HYDRUINO_BAL_INCACTUATORS_MAXSIZE   8                       // Maximum array size for balancer increment actuators list (max # of increment autodosers/actuators used during balancing)
#define HYDRUINO_BAL_DECACTUATORS_MAXSIZE   4                       // Maximum array size for balancer decrement actuators list (max # of decrement autodosers/actuators used during balancing)
#define HYDRUINO_SCH_REQACTUATORS_MAXSIZE   4                       // Maximum array size for scheduler required actuators list (max # of actuators active per process stage)
#define HYDRUINO_SCH_FEEDRES_MAXSIZE        4                       // Maximum array size for scheduler feeding/lighting process lists (max # of feed reservoirs)
#define HYDRUINO_SYS_ONEWIRE_MAXSIZE        4                       // Maximum array size for pin->OneWire instances list (max # of OneWire comm pins)
#define HYDRUINO_SYS_PINLOCKS_MAXSIZE       4                       // Maximum array size for pin locks list (max # of simultaneous locks)

#define HYDRUINO_CONTROL_LOOP_INTERVAL      100                     // Run interval of main control loop, in milliseconds
#define HYDRUINO_DATA_LOOP_INTERVAL         2000                    // Default run interval of data loop, in milliseconds (customizable later)
#define HYDRUINO_MISC_LOOP_INTERVAL         250                     // Run interval of misc loop, in milliseconds

#define HYDRUINO_ACT_PUMPCALC_MINWRTMILLIS  250                     // Minimum time millis needing to pass before a pump reports/writes changed volume to reservoir (reduces error accumulation)
#define HYDRUINO_ACT_PUMPCALC_MINFLOWRATE   0.05f                   // What percentage of continuous flow rate an instantaneous flow rate sensor must achieve before it is used in pump/volume calculations (reduces near-zero error jitters)
#define HYDRUINO_ACT_PUMPCALC_MAXFRAMEDIFF  5                       // Maximum polling frames # away an instantaneous flow rate can be for it to be used in pump/volume calculations (so it uses only recent measurements)

#define HYDRUINO_CROP_NIGHT_BEGINHR         22                      // Hour of the day night begins (for night feeding multiplier)
#define HYDRUINO_CROP_NIGHT_ENDHR           6                       // Hour of the day night ends (for night feeding multiplier)
#define HYDRUINO_CROP_GROWWEEKS_MAX         16                      // Maximum grow weeks to support scheduling up to
#define HYDRUINO_CROP_GROWWEEKS_MIN         8                       // Minimum grow weeks to support scheduling up to

#define HYDRUINO_DOSETIME_FRACTION_MIN      0.5f                    // What percentage of base dosing time autodosers can scale down to, if estimated dosing time could exceed setpoint
#define HYDRUINO_DOSETIME_FRACTION_MAX      1.5f                    // What percentage of base dosing time autodosers can scale up to, if estimated dosing time remaining could fall short of setpoint

#define HYDRUINO_FEEDRES_FRACTION_EMPTY     0.2f                    // What fraction of a feed reservoir's volume is to be considered 'empty' during pumping/feedings (to account for pumps, heaters, etc. - only used for feed reservoirs with volume tracking but no filled/empty triggers)
#define HYDRUINO_FEEDRES_FRACTION_FILLED    0.9f                    // What fraction of a feed reservoir's volume to top-off to/considered 'filled' during pumping/feedings (rest will be used for balancing - only used for feed reservoirs with volume tracking but no filled/empty triggers)

#define HYDRUINO_POS_SEARCH_FROMBEG         -1                      // Search from beginning to end, 0 up to MAXSIZE-1
#define HYDRUINO_POS_SEARCH_FROMEND         HYDRUINO_POS_MAXSIZE    // Search from end to beginning, MAXSIZE-1 down to 0
#define HYDRUINO_POS_EXPORT_BEGFROM         1                       // Whenever exported/user-facing position indexing starts at 1 or 0 (aka display offset)

#define HYDRUINO_RANGE_PH_HALF              1.0f                    // How far to go, in either direction, to form a range when pH is expressed as a single number, in pH (note: this also controls auto-balancer ranges)
#define HYDRUINO_RANGE_EC_HALF              0.5f                    // How far to go, in either direction, to form a range when TDS/EC is expressed as a single number, in EC (note: this also controls auto-balancer ranges)
#define HYDRUINO_RANGE_TEMP_HALF            5.0f                    // How far to go, in either direction, to form a range when Temp is expressed as a single number, in C (note: this also controls auto-balancer ranges)
#define HYDRUINO_RANGE_CO2_HALF             100.0f                  // How far to go, in either direction, to form a range when CO2 is expressed as a single number, in PPM (note: this also controls auto-balancer ranges)

#define HYDRUINO_SCH_FEED_FRACTION          0.8f                    // What percentage of crops need to have their feeding signal turned on/off for scheduler to act on such as a whole
#define HYDRUINO_SCH_BALANCE_MINTIME        30                      // Minimum time, in seconds, that all balancers must register as balanced for until balancing is marked as completed

#define HYDRUINO_SENSOR_ANALOGREAD_SAMPLES  5                       // Number of samples to take for any analogRead call inside of a sensor's takeMeasurement call, or 0 to disable sampling (note: bitRes.maxValue * # of samples must fit inside a uint32_t)
#define HYDRUINO_SENSOR_ANALOGREAD_DELAY    0                       // Delay time between samples, or 0 to disable delay

#define HYDRUINO_SYS_AUTOSAVE_INTERVAL      120                     // Default autosave interval, in minutes
#define HYDRUINO_SYS_FREERAM_LOWBYTES       1024                    // How many bytes of free memory left spawns a handle low mem call to all objects
#define HYDRUINO_SYS_FREESPACE_INTERVAL     240                     // How many minutes should pass before checking attached file systems have enough disk space (performs cleanup if not)
#define HYDRUINO_SYS_FREESPACE_LOWSPACE     256                     // How many kilobytes of disk space remaining will force cleanup of oldest log/data files first
#define HYDRUINO_SYS_FREESPACE_DAYSBACK     180                     // How many days back log/data files are allowed to be stored up to (any beyond this are deleted during cleanup)
#define HYDRUINO_SYS_DELAYFINE_SPINMILLIS   20                      // How many milliseconds away from stop time fine delays can use yield() up to before using a blocking spin-lock (ensures fine dosing)


#if defined(__APPLE__) || defined(__APPLE) || defined(__unix__) || defined(__unix)
#define HYDRUINO_BLDPATH_SEPARATOR          '/'                     // Path separator for nix-based build machines
#else
#define HYDRUINO_BLDPATH_SEPARATOR          '\\'                    // Path separator for win-based build machines
#endif


// Crop Type
// Common crop types. Controls what pH, EC, etc. that a plant prefers.
enum Hydroponics_CropType {
    Hydroponics_CropType_AloeVera,                          // Aloe Vera crop
    Hydroponics_CropType_Anise,                             // Anise crop
    Hydroponics_CropType_Artichoke,                         // Artichoke crop
    Hydroponics_CropType_Arugula,                           // Arugula crop
    Hydroponics_CropType_Asparagus,                         // Asparagus crop
    Hydroponics_CropType_Basil,                             // Basil crop
    Hydroponics_CropType_Bean,                              // Bean (common) crop
    Hydroponics_CropType_BeanBroad,                         // Bean (broad) crop
    Hydroponics_CropType_Beetroot,                          // Beetroot crop
    Hydroponics_CropType_BlackCurrant,                      // Black Currant crop
    Hydroponics_CropType_Blueberry,                         // Blueberry crop
    Hydroponics_CropType_BokChoi,                           // Bok-choi crop
    Hydroponics_CropType_Broccoli,                          // Broccoli crop
    Hydroponics_CropType_BrusselsSprout,                    // Brussels Sprout crop
    Hydroponics_CropType_Cabbage,                           // Cabbage crop
    Hydroponics_CropType_Cannabis,                          // Cannabis (generic) crop
    Hydroponics_CropType_Capsicum,                          // Capsicum crop
    Hydroponics_CropType_Carrots,                           // Carrots crop
    Hydroponics_CropType_Catnip,                            // Catnip crop
    Hydroponics_CropType_Cauliflower,                       // Cauliflower crop
    Hydroponics_CropType_Celery,                            // Celery crop
    Hydroponics_CropType_Chamomile,                         // Chamomile crop
    Hydroponics_CropType_Chicory,                           // Chiccory crop
    Hydroponics_CropType_Chives,                            // Chives crop
    Hydroponics_CropType_Cilantro,                          // Cilantro crop
    Hydroponics_CropType_Coriander,                         // Coriander crop
    Hydroponics_CropType_CornSweet,                         // Corn (sweet) crop
    Hydroponics_CropType_Cucumber,                          // Cucumber crop
    Hydroponics_CropType_Dill,                              // Dill crop
    Hydroponics_CropType_Eggplant,                          // Eggplant crop
    Hydroponics_CropType_Endive,                            // Endive crop
    Hydroponics_CropType_Fennel,                            // Fennel crop
    Hydroponics_CropType_Fodder,                            // Fodder crop
    Hydroponics_CropType_Flowers,                           // Flowers (generic) crop
    Hydroponics_CropType_Garlic,                            // Garlic crop
    Hydroponics_CropType_Ginger,                            // Ginger crop
    Hydroponics_CropType_Kale,                              // Kale crop
    Hydroponics_CropType_Lavender,                          // Lavender crop
    Hydroponics_CropType_Leek,                              // Leek crop
    Hydroponics_CropType_LemonBalm,                         // Lemon Balm crop
    Hydroponics_CropType_Lettuce,                           // Lettuce crop
    Hydroponics_CropType_Marrow,                            // Marrow crop
    Hydroponics_CropType_Melon,                             // Melon crop
    Hydroponics_CropType_Mint,                              // Mint crop
    Hydroponics_CropType_MustardCress,                      // Mustard Cress crop
    Hydroponics_CropType_Okra,                              // Okra crop
    Hydroponics_CropType_Onions,                            // Onions crop
    Hydroponics_CropType_Oregano,                           // Oregano crop
    Hydroponics_CropType_PakChoi,                           // Pak-choi crop
    Hydroponics_CropType_Parsley,                           // Parsley crop
    Hydroponics_CropType_Parsnip,                           // Parsnip crop
    Hydroponics_CropType_Pea,                               // Pea (common) crop
    Hydroponics_CropType_PeaSugar,                          // Pea (sugar) crop
    Hydroponics_CropType_Pepino,                            // Pepino crop
    Hydroponics_CropType_PeppersBell,                       // Peppers (bell) crop
    Hydroponics_CropType_PeppersHot,                        // Peppers (hot) crop
    Hydroponics_CropType_Potato,                            // Potato (common) crop
    Hydroponics_CropType_PotatoSweet,                       // Potato (sweet) crop
    Hydroponics_CropType_Pumpkin,                           // Pumpkin crop
    Hydroponics_CropType_Radish,                            // Radish crop
    Hydroponics_CropType_Rhubarb,                           // Rhubarb crop
    Hydroponics_CropType_Rosemary,                          // Rosemary crop
    Hydroponics_CropType_Sage,                              // Sage crop
    Hydroponics_CropType_Silverbeet,                        // Silverbeet crop
    Hydroponics_CropType_Spinach,                           // Spinach crop
    Hydroponics_CropType_Squash,                            // Squash crop
    Hydroponics_CropType_Sunflower,                         // Sunflower crop
    Hydroponics_CropType_Strawberries,                      // Strawberries crop
    Hydroponics_CropType_SwissChard,                        // Swiss Chard crop
    Hydroponics_CropType_Taro,                              // Taro crop
    Hydroponics_CropType_Tarragon,                          // Tarragon crop
    Hydroponics_CropType_Thyme,                             // Thyme crop
    Hydroponics_CropType_Tomato,                            // Tomato crop
    Hydroponics_CropType_Turnip,                            // Turnip crop
    Hydroponics_CropType_Watercress,                        // Watercress crop
    Hydroponics_CropType_Watermelon,                        // Watermelon crop
    Hydroponics_CropType_Zucchini,                          // Zucchini crop

    // Custom crops allow customized parameters for crops to be programmed into the system.

    Hydroponics_CropType_CustomCrop1,                       // Custom crop 1
    Hydroponics_CropType_CustomCrop2,                       // Custom crop 2
    Hydroponics_CropType_CustomCrop3,                       // Custom crop 3
    Hydroponics_CropType_CustomCrop4,                       // Custom crop 4
    Hydroponics_CropType_CustomCrop5,                       // Custom crop 5
    Hydroponics_CropType_CustomCrop6,                       // Custom crop 6
    Hydroponics_CropType_CustomCrop7,                       // Custom crop 7
    Hydroponics_CropType_CustomCrop8,                       // Custom crop 8

    Hydroponics_CropType_Count,                             // Internal use only
    Hydroponics_CropType_CustomCropCount = 8,               // Internal use only
    Hydroponics_CropType_Undefined = -1                     // Internal use only
};

// Substrate Type
// Common substrate types. Influences feeding scheduling and environment control (TODO).
enum Hydroponics_SubstrateType {
    Hydroponics_SubstrateType_ClayPebbles,                  // Expanded clay pebbles substrate
    Hydroponics_SubstrateType_CoconutCoir,                  // Coconut coir (aka coco peat) substrate
    Hydroponics_SubstrateType_Rockwool,                     // Rockwool substrate

    Hydroponics_SubstrateType_Count,                        // Internal use only
    Hydroponics_SubstrateType_Undefined = -1                // Internal use only
};

// Crop Phase
// Common phases of crops. Influences feeding scheduling and environment control.
enum Hydroponics_CropPhase {
    Hydroponics_CropPhase_Seedling,                         // Initial seedling stage
    Hydroponics_CropPhase_Vegetative,                       // Vegetative stage
    Hydroponics_CropPhase_Blooming,                         // Flowering stage
    Hydroponics_CropPhase_Harvest,                          // Harvest stage

    Hydroponics_CropPhase_Count,                            // Internal use only
    Hydroponics_CropPhase_MainCount = 3,                    // Internal use only
    Hydroponics_CropPhase_Undefined = -1                    // Internal use only
};

// System Run Mode
// Specifies the general tank setup, fluid levels, and waste connection defaults.
enum Hydroponics_SystemMode {
    Hydroponics_SystemMode_Recycling,                       // System consistently recycles water in main feed water reservoir. Default setting, applicable to a wide range of NFT and DWC setups.
    Hydroponics_SystemMode_DrainToWaste,                    // System refills feed reservoir every time before feeding (with pH/feed premix topoff), and requires a drainage pipe (as feed pump output) or drainage pump (from feed reservoir to drainage pipe).

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
    Hydroponics_MeasurementMode_Default = Hydroponics_MeasurementMode_Metric // Default system measurement mode
};

// LCD/Display Output Mode
// Specifies what kind of visual output device is to be used.
// Currently, all ouput devices must ultimately be supported by tcMenu.
enum Hydroponics_DisplayOutputMode {
    Hydroponics_DisplayOutputMode_Disabled,                 // No display output
    Hydroponics_DisplayOutputMode_20x4LCD,                  // 20x4 i2c LCD (with layout: EN, RW, RS, BL, Data)
    Hydroponics_DisplayOutputMode_20x4LCD_Swapped,          // 20x4 i2c LCD (with EN<->RS swapped, layout: RS, RW, EN, BL, Data)
    Hydroponics_DisplayOutputMode_16x2LCD,                  // 16x2 i2c LCD (with layout: EN, RW, RS, BL, Data)
    Hydroponics_DisplayOutputMode_16x2LCD_Swapped,          // 16x2 i2c LCD (with EN<->RS swapped, layout: RS, RW, EN, BL, Data)

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
    Hydroponics_ActuatorType_WaterPump,                     // Water pump actuator (feed or drainage/main-water pipe reservoirs only)
    Hydroponics_ActuatorType_PeristalticPump,               // Peristaltic pump actuator (pH-up/down, nutrient, fresh water, or custom additive reservoirs only)
    Hydroponics_ActuatorType_WaterHeater,                   // Water heater actuator (feed reservoir only)
    Hydroponics_ActuatorType_WaterAerator,                  // Water aerator actuator (feed reservoir only)
    Hydroponics_ActuatorType_WaterSprayer,                  // Water sprayer actuator (feed reservoir only, uses crop linkages - assumes infinite water source)
    Hydroponics_ActuatorType_FanExhaust,                    // Fan exhaust/circulation relay actuator (feed reservoir only, uses crop linkages)

    Hydroponics_ActuatorType_Count,                         // Internal use only
    Hydroponics_ActuatorType_Undefined = -1                 // Internal use only
};

// Sensor Type
// Sensor device type. Specifies the various sensors and the kinds of things they measure.
enum Hydroponics_SensorType {
    Hydroponics_SensorType_PotentialHydrogen,               // pH sensor (analog/digital, feed reservoir only)
    Hydroponics_SensorType_TotalDissolvedSolids,            // TDS salts electrode sensor (analog/digital, feed reservoir only)
    Hydroponics_SensorType_SoilMoisture,                    // Soil moisture sensor (analog/digital)
    Hydroponics_SensorType_WaterTemperature,                // Submersible water sensor (analog/digital)
    Hydroponics_SensorType_WaterPumpFlowSensor,             // Water pump flow hall sensor (analog(PWM))
    Hydroponics_SensorType_WaterLevelIndicator,             // Water level indicator (binary)
    Hydroponics_SensorType_WaterHeightMeter,                // Water height meter (analog)
    Hydroponics_SensorType_AirTempHumidity,                 // Air temperature and humidity sensor (digital)
    Hydroponics_SensorType_AirCarbonDioxide,                // Air CO2 sensor (analog/digital)
    Hydroponics_SensorType_PowerUsageMeter,                 // Power usage meter (analog)

    Hydroponics_SensorType_Count,                           // Internal use only
    Hydroponics_SensorType_Undefined = -1                   // Internal use only
};

// Reservoir Type
// Common fluid containers. Specifies the various operational containers.
enum Hydroponics_ReservoirType {
    Hydroponics_ReservoirType_FeedWater,                    // Feed water
    Hydroponics_ReservoirType_DrainageWater,                // Drainage water
    Hydroponics_ReservoirType_NutrientPremix,               // Base nutrient premix (A or B iff mixed 1:1)
    Hydroponics_ReservoirType_FreshWater,                   // Fresh water
    Hydroponics_ReservoirType_PhUpSolution,                 // pH-Up solution
    Hydroponics_ReservoirType_PhDownSolution,               // pH-Down solution

    // Custom additives allow specialized weekly feeding schedules to be programmed into the system.

    Hydroponics_ReservoirType_CustomAdditive1,              // Custom additive 1 solution
    Hydroponics_ReservoirType_CustomAdditive2,              // Custom additive 2 solution
    Hydroponics_ReservoirType_CustomAdditive3,              // Custom additive 3 solution
    Hydroponics_ReservoirType_CustomAdditive4,              // Custom additive 4 solution
    Hydroponics_ReservoirType_CustomAdditive5,              // Custom additive 5 solution
    Hydroponics_ReservoirType_CustomAdditive6,              // Custom additive 6 solution
    Hydroponics_ReservoirType_CustomAdditive7,              // Custom additive 7 solution
    Hydroponics_ReservoirType_CustomAdditive8,              // Custom additive 8 solution
    Hydroponics_ReservoirType_CustomAdditive9,              // Custom additive 9 solution
    Hydroponics_ReservoirType_CustomAdditive10,             // Custom additive 10 solution
    Hydroponics_ReservoirType_CustomAdditive11,             // Custom additive 11 solution
    Hydroponics_ReservoirType_CustomAdditive12,             // Custom additive 12 solution
    Hydroponics_ReservoirType_CustomAdditive13,             // Custom additive 13 solution
    Hydroponics_ReservoirType_CustomAdditive14,             // Custom additive 14 solution
    Hydroponics_ReservoirType_CustomAdditive15,             // Custom additive 15 solution
    Hydroponics_ReservoirType_CustomAdditive16,             // Custom additive 16 solution

    Hydroponics_ReservoirType_Count,                        // Internal use only
    Hydroponics_ReservoirType_CustomAdditiveCount = 16,     // Internal use only
    Hydroponics_ReservoirType_Undefined = -1                // Internal use only
};

// Power Rail
// Common power rails. Specifies an isolated operational power rail unit.
enum Hydroponics_RailType {
    Hydroponics_RailType_AC110V,                            // 110~120V AC-based power rail, for pumps, lights, heaters, etc.
    Hydroponics_RailType_AC220V,                            // 110~120V AC-based power rail, for pumps, lights, heaters, etc.
    Hydroponics_RailType_DC5V,                              // 5v DC-based power rail, for dosing pumps, PWM fans, sensors, etc.
    Hydroponics_RailType_DC12V,                             // 12v DC-based power rail, for dosing pumps, PWM fans, sensors, etc.

    Hydroponics_RailType_Count,                             // Internal use only
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

// Balancing State
// Common balancing states. Specifies balance or which direction of imbalance.
enum Hydroponics_BalancerState {
    Hydroponics_BalancerState_TooLow,                       // Too low / needs incremented state
    Hydroponics_BalancerState_Balanced,                     // Balanced state
    Hydroponics_BalancerState_TooHigh,                      // Too high / needs decremented state

    Hydroponics_BalancerState_Count,                        // Internal use only
    Hydroponics_BalancerState_Undefined = -1                // Internal use only
};

// Units Category
// Unit of measurement category. Specifies the kind of unit.
enum Hydroponics_UnitsCategory {
    Hydroponics_UnitsCategory_Alkalinity,                   // Alkalinity based unit
    Hydroponics_UnitsCategory_DissolvedSolids,              // Dissolved solids based unit
    Hydroponics_UnitsCategory_SoilMoisture,                 // Soil moisture based unit
    Hydroponics_UnitsCategory_LiqTemperature,               // Liquid temperature based unit
    Hydroponics_UnitsCategory_LiqVolume,                    // Liquid volume based unit
    Hydroponics_UnitsCategory_LiqFlowRate,                  // Liquid flow rate based unit
    Hydroponics_UnitsCategory_LiqDilution,                  // Liquid dilution based unit
    Hydroponics_UnitsCategory_AirTemperature,               // Air temperature based unit
    Hydroponics_UnitsCategory_AirHumidity,                  // Air humidity based unit
    Hydroponics_UnitsCategory_AirHeatIndex,                 // Air heat index based unit
    Hydroponics_UnitsCategory_AirConcentration,             // Air particle concentration based unit
    Hydroponics_UnitsCategory_Distance,                     // Distance based unit
    Hydroponics_UnitsCategory_Weight,                       // Weight based unit
    Hydroponics_UnitsCategory_Power,                        // Power based unit

    Hydroponics_UnitsCategory_Count,                        // Internal use only
    Hydroponics_UnitsCategory_Undefined = -1                // Internal use only
};

// Units Type
// Unit of measurement type. Specifies the unit type associated with a measured value.
enum Hydroponics_UnitsType {
    Hydroponics_UnitsType_Raw_0_1,                          // Raw value [0.0,1.0] mode
    Hydroponics_UnitsType_Percentile_0_100,                 // Percentile [0.0,100.0] mode
    Hydroponics_UnitsType_Alkalinity_pH_0_14,               // pH value [0.0,14.0] alkalinity mode
    Hydroponics_UnitsType_Concentration_EC,                 // Siemens electrical conductivity mode
    Hydroponics_UnitsType_Temperature_Celsius,              // Celsius temperature mode
    Hydroponics_UnitsType_Temperature_Fahrenheit,           // Fahrenheit temperature mode
    Hydroponics_UnitsType_Temperature_Kelvin,               // Kelvin temperature mode
    Hydroponics_UnitsType_LiqVolume_Liters,                 // Liters liquid volume mode
    Hydroponics_UnitsType_LiqVolume_Gallons,                // Gallons liquid volume mode
    Hydroponics_UnitsType_LiqFlowRate_LitersPerMin,         // Liters per minute liquid flow rate mode
    Hydroponics_UnitsType_LiqFlowRate_GallonsPerMin,        // Gallons per minute liquid flow rate mode
    Hydroponics_UnitsType_LiqDilution_MilliLiterPerLiter,   // Milli liter per liter dilution mode
    Hydroponics_UnitsType_LiqDilution_MilliLiterPerGallon,  // Milli liter per gallon dilution mode
    Hydroponics_UnitsType_Concentration_PPM500,             // Parts-per-million 500 (NaCl) concentration mode (US)
    Hydroponics_UnitsType_Concentration_PPM640,             // Parts-per-million 640 concentration mode (EU)
    Hydroponics_UnitsType_Concentration_PPM700,             // Parts-per-million 700 (KCl) concentration mode (AU)
    Hydroponics_UnitsType_Distance_Meters,                  // Meters distance mode
    Hydroponics_UnitsType_Distance_Feet,                    // Feet distance mode
    Hydroponics_UnitsType_Weight_Kilogram,                  // Kilogram weight mode
    Hydroponics_UnitsType_Weight_Pounds,                    // Pounds weight mode
    Hydroponics_UnitsType_Power_Wattage,                    // Wattage power mode
    Hydroponics_UnitsType_Power_Amperage,                   // Amperage current power mode

    Hydroponics_UnitsType_Count,                            // Internal use only
    Hydroponics_UnitsType_Concentration_TDS = Hydroponics_UnitsType_Concentration_EC, // Standard TDS concentration mode alias
    Hydroponics_UnitsType_Concentration_PPM = Hydroponics_UnitsType_Concentration_PPM500, // Standard PPM concentration mode alias
    Hydroponics_UnitsType_Power_JoulesPerSecond = Hydroponics_UnitsType_Power_Wattage, // Joules per second power mode alias
    Hydroponics_UnitsType_Undefined = -1                    // Internal use only
};

class Hydroponics;
struct HydroponicsIdentity;
class HydroponicsObject;
class HydroponicsSubObject;
class HydroponicsActuator;
class HydroponicsSensor;
class HydroponicsCrop;
class HydroponicsReservoir;
class HydroponicsFeedReservoir;
class HydroponicsRail;
struct HydroponicsData;
struct HydroponicsObjectData;
struct HydroponicsSubData;
struct HydroponicsMeasurement;
struct HydroponicsSingleMeasurement;
class HydroponicsTrigger;
class HydroponicsBalancer;
class HydroponicsScheduler;
class HydroponicsLogger;
class HydroponicsPublisher;

#endif // /ifndef HydroponicsDefines_H
