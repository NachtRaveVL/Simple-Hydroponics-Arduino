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
#define HYDRUINO_CALIB_MAXSIZE              8                       // Analog calibration data maximum size
#define HYDRUINO_EEPROM_MEMORYSIZE          I2C_DEVICESIZE_24LC256  // Memory size for EEPROM (256kb)
#define HYDRUINO_CTRLINPINMAP_MAXSIZE       8                       // Control input pinmap maximum size

// Crop Type
enum Hydroponics_CropType {
    Hydroponics_CropType_AloeVera,          // Aloe Vera crop.
    Hydroponics_CropType_Anise,             // Anise crop.
    Hydroponics_CropType_Artichoke,         // Artichoke crop.
    Hydroponics_CropType_Arugula,           // Arugula crop.
    Hydroponics_CropType_Asparagus,         // Asparagus crop.
    Hydroponics_CropType_Basil,             // Basil crop.
    Hydroponics_CropType_Bean,              // Bean (common) crop.
    Hydroponics_CropType_BeanBroad,         // Bean (broad) crop.
    Hydroponics_CropType_Beetroot,          // Beetroot crop.
    Hydroponics_CropType_BlackCurrant,      // Black Currant crop.
    Hydroponics_CropType_Blueberry,         // Blueberry crop.
    Hydroponics_CropType_BokChoi,           // Bok-choi crop.
    Hydroponics_CropType_Broccoli,          // Broccoli crop.
    Hydroponics_CropType_BrussellSprouts,   // BrussellSprouts crop.
    Hydroponics_CropType_Cabbage,           // Cabbage crop.
    Hydroponics_CropType_Cannabis,          // Cannabis (generic) crop.
    Hydroponics_CropType_Capiscum,          // Capiscum crop.
    Hydroponics_CropType_Carrots,           // Carrots crop.
    Hydroponics_CropType_Catnip,            // Catnip crop.
    Hydroponics_CropType_Cauliflower,       // Cauliflower crop.
    Hydroponics_CropType_Celery,            // Celery crop.
    Hydroponics_CropType_Chamomile,         // Chamomile crop.
    Hydroponics_CropType_Chickory,          // Chickory crop.
    Hydroponics_CropType_Chives,            // Chives crop.
    Hydroponics_CropType_Cilantro,          // Cilantro crop.
    Hydroponics_CropType_Coriander,         // Coriander crop.
    Hydroponics_CropType_CornSweet,         // Corn (sweet) crop.
    Hydroponics_CropType_Cucumber,          // Cucumber crop.
    Hydroponics_CropType_Dill,              // Dill crop.
    Hydroponics_CropType_Eggplant,          // Eggplant crop.
    Hydroponics_CropType_Endive,            // Endive crop.
    Hydroponics_CropType_Fennel,            // Fennel crop.
    Hydroponics_CropType_Fodder,            // Fodder crop.
    Hydroponics_CropType_Flowers,           // Flowers (generic) crop.
    Hydroponics_CropType_Garlic,            // Garlic crop.
    Hydroponics_CropType_Ginger,            // Ginger crop.
    Hydroponics_CropType_Kale,              // Kale crop.
    Hydroponics_CropType_Lavender,          // Lavender crop.
    Hydroponics_CropType_Leek,              // Leek crop.
    Hydroponics_CropType_LemonBalm,         // Lemon Balm crop.
    Hydroponics_CropType_Lettuce,           // Lettuce crop.
    Hydroponics_CropType_Marrow,            // Marrow crop.
    Hydroponics_CropType_Melon,             // Melon crop.
    Hydroponics_CropType_Mint,              // Mint crop.
    Hydroponics_CropType_MustardCress,      // Mustard Cress crop.
    Hydroponics_CropType_Okra,              // Okra crop.
    Hydroponics_CropType_Onions,            // Onions crop.
    Hydroponics_CropType_Oregano,           // Oregano crop.
    Hydroponics_CropType_PakChoi,           // Pak-choi crop.
    Hydroponics_CropType_Parsley,           // Parsley crop.
    Hydroponics_CropType_Parsnip,           // Parsnip crop.
    Hydroponics_CropType_Pea,               // Pea (common) crop.
    Hydroponics_CropType_PeaSugar,          // Pea (sugar) crop.
    Hydroponics_CropType_Pepino,            // Pepino crop.
    Hydroponics_CropType_PeppersBell,       // Peppers (bell) crop.
    Hydroponics_CropType_PeppersHot,        // Peppers (hot) crop.
    Hydroponics_CropType_Potato,            // Potato (common) crop.
    Hydroponics_CropType_PotatoSweet,       // Potato (sweet) crop.
    Hydroponics_CropType_Pumpkin,           // Pumpkin crop.
    Hydroponics_CropType_Radish,            // Radish crop.
    Hydroponics_CropType_Rhubarb,           // Rhubarb crop.
    Hydroponics_CropType_Rosemary,          // Rosemary crop.
    Hydroponics_CropType_Sage,              // Sage crop.
    Hydroponics_CropType_Silverbeet,        // Silverbeet crop.
    Hydroponics_CropType_Spinach,           // Spinach crop.
    Hydroponics_CropType_Squash,            // Squash crop.
    Hydroponics_CropType_Sunflower,         // Sunflower crop.
    Hydroponics_CropType_Strawberries,      // Strawberries crop.
    Hydroponics_CropType_SwissChard,        // Swiss chard crop.
    Hydroponics_CropType_Taro,              // Taro crop.
    Hydroponics_CropType_Tarragon,          // Tarragon crop.
    Hydroponics_CropType_Thyme,             // Thyme crop.
    Hydroponics_CropType_Tomato,            // Tomato crop.
    Hydroponics_CropType_Turnip,            // Turnip crop.
    Hydroponics_CropType_Watercress,        // Watercress crop.
    Hydroponics_CropType_Watermelon,        // Watermelon crop.
    Hydroponics_CropType_Zucchini,          // Zucchini crop.
    Hydroponics_CropType_Custom1,           // Custom 1 crop.
    Hydroponics_CropType_Custom2,           // Custom 2 crop.
    Hydroponics_CropType_Custom3,           // Custom 3 crop.
    Hydroponics_CropType_Custom4,           // Custom 4 crop.
    Hydroponics_CropType_Custom5,           // Custom 5 crop.

    // FIXME: Need to change storage of crop data to PROGMEM. Capping CropType count until fixed.
    Hydroponics_CropType_Count = 10,        // Internal use only
    Hydroponics_CropType_CustomCount = 5,   // Internal use only
    Hydroponics_CropType_Undefined = -1     // Internal use only
};

// Crop Phase
enum Hydroponics_CropPhase {
    Hydroponics_CropPhase_Seedling,         // Initial seedling stage
    Hydroponics_CropPhase_Vegetative,       // Vegetative stage
    Hydroponics_CropPhase_Flowering,        // Flowering stage
    Hydroponics_CropPhase_Fruiting,         // Fruiting stage
    Hydroponics_CropPhase_Harvest,          // Harvest stage

    Hydroponics_CropPhase_Count,            // Internal use only
    Hydroponics_CropPhase_Undefined = -1    // Internal use only
};

// Relay Rail
enum Hydroponics_RelayRail {
    Hydroponics_RelayRail_ACRail,           // AC relay rail
    Hydroponics_RelayRail_DCRail,           // DC relay rail
    Hydroponics_RelayRail_Custom1,          // Custom 1 relay rail
    Hydroponics_RelayRail_Custom2,          // Custom 2 relay rail

    Hydroponics_RelayRail_Count,            // Internal use only
    Hydroponics_RelayRail_CustomCount,      // Internal use only
    Hydroponics_RelayRail_Undefined = -1    // Internal use only
};

// Fluid Reservoir
enum Hydroponics_FluidReservoir {
    Hydroponics_FluidReservoir_FeedWater,           // Feed water reservoir (aka main water reservoir)
    Hydroponics_FluidReservoir_DrainageWater,       // Drainage water reservoir
    Hydroponics_FluidReservoir_NutrientPremix,      // Nutrient premix reservoir/source
    Hydroponics_FluidReservoir_FreshWater,          // Fresh water reservoir/source
    Hydroponics_FluidReservoir_PhUpSolution,        // pH-Up solution
    Hydroponics_FluidReservoir_PhDownSolution,      // pH-Down solution

    Hydroponics_FluidReservoir_Count,               // Internal use only
    Hydroponics_FluidReservoir_Undefined = -1       // Internal use only
};

// System Run Mode
enum Hydroponics_SystemMode {
    Hydroponics_SystemMode_Recycling,               // System consistently recycles water in main feed water reservoir. Default setting.
    Hydroponics_SystemMode_DrainToWaste,            // System fills feed reservoir before feeding (with pH/feed premix topoff prior), expects runoff (waste) to drain (unless drainage reservoir defined).

    Hydroponics_SystemMode_Count,                   // Internal use only
    Hydroponics_SystemMode_Undefined = -1           // Internal use only
};

// Measurement Units Mode
enum Hydroponics_MeasurementMode {
    Hydroponics_MeasurementMode_Imperial,           // Imperial measurement mode (default setting, °F Ft Gal Lbs M-D-Y Val.X etc)
    Hydroponics_MeasurementMode_Metric,             // Metric measurement mode (°C M L Kg Y-M-D Val.X etc)
    Hydroponics_MeasurementMode_Scientific,         // Scientific measurement mode (°K M L Kg Y-M-D Val.XX etc)

    Hydroponics_MeasurementMode_Count,              // Internal use only
    Hydroponics_MeasurementMode_Undefined = -1,     // Internal use only
    Hydroponics_MeasurementMode_Default = Hydroponics_MeasurementMode_Imperial // If you must change it
};

// LCD Output Mode
enum Hydroponics_LCDOutputMode {
    Hydroponics_LCDOutputMode_Disabled,             // No LCD output
    Hydroponics_LCDOutputMode_20x4LCD,              // 20x4 i2c LCD (with layout EN, RW, RS, BL, Data)
    Hydroponics_LCDOutputMode_20x4LCD_Swapped,      // 20x4 i2c LCD (with EN<->RS swapped layout RS, RW, EN, BL, Data)
    Hydroponics_LCDOutputMode_16x2LCD,              // 16x2 i2c LCD (with layout EN, RW, RS, BL, Data)
    Hydroponics_LCDOutputMode_16x2LCD_Swapped,      // 16x2 i2c LCD (with EN<->RS swapped layout RS, RW, EN, BL, Data)

    Hydroponics_LCDOutputMode_Count,                // Internal use only
    Hydroponics_LCDOutputMode_Undefined = -1        // Internal use only
};

// Control Input Mode
enum Hydroponics_ControlInputMode {
    Hydroponics_ControlInputMode_Disabled,          // No control input
    Hydroponics_ControlInputMode_2x2Matrix,         // 2x2 directional keyboard matrix button array, ribbon: {L1,L2,R1,R2} (L1 = pin 1)
    Hydroponics_ControlInputMode_4xButton,          // 4x standard momentary buttons, ribbon: {U,D,L,R} (U = pin 1)
    Hydroponics_ControlInputMode_6xButton,          // 6x standard momentary buttons, ribbon: {TODO} (X = pin 1)
    Hydroponics_ControlInputMode_RotaryEncoder,     // Rotary encoder, ribbon: {A,B,OK,L,R} (A = pin 1)

    Hydroponics_ControlInputMode_Count,             // Internal use only
    Hydroponics_ControlInputMode_Undefined = -1     // Internal use only
};

// Actuator Type
enum Hydroponics_ActuatorType {
    Hydroponics_ActuatorType_GrowLightsRelay,       // Grow lights relay actuator
    Hydroponics_ActuatorType_WaterPumpRelay,        // Water pump relay actuator (feed or drainage reservoir only)
    Hydroponics_ActuatorType_PeristalticPumpRelay,  // Peristaltic pump relay actuator (pH-up/down, nutrient, or fresh water reservoirs only)
    Hydroponics_ActuatorType_WaterHeaterRelay,      // Water heater relay actuator (feed reservoir only)
    Hydroponics_ActuatorType_WaterAeratorRelay,     // Water aerator relay actuator (feed reservoir only)
    Hydroponics_ActuatorType_FanExhaustRelay,       // Fan exhaust relay actuator
    Hydroponics_ActuatorType_FanExhaustPWM,         // Fan exhaust PWM actuator

    Hydroponics_ActuatorType_Count,                 // Internal use only
    Hydroponics_ActuatorType_Undefined = -1         // Internal use only
};

// Sensor Type
enum Hydroponics_SensorType {
    Hydroponics_SensorType_AirTempHumidity,         // Air temperature and humidity sensor (digital, front-panel)
    Hydroponics_SensorType_AirCarbonDioxide,        // Air CO2 sensor (analog->binary/binary)
    Hydroponics_SensorType_PotentialHydrogen,       // pH sensor (analog, signal pin sometimes labeled as 'Po', feed reservoir only)
    Hydroponics_SensorType_TotalDissolvedSolids,    // TDS salts electrode sensor (analog, feed reservoir only)
    Hydroponics_SensorType_WaterTemperature,        // DallasTemperature DS18* submersible sensor (analog)
    Hydroponics_SensorType_WaterPumpFlowSensor,     // Water pump flow hall sensor (PWM)
    Hydroponics_SensorType_LowWaterLevelIndicator,  // Low water level indicator (binary)
    Hydroponics_SensorType_HighWaterLevelIndicator, // High water level indicator (binary)
    Hydroponics_SensorType_LowWaterHeightMeter,     // Low water height meter (analog->binary)
    Hydroponics_SensorType_HighWaterHeightMeter,    // High water height meter (analog->binary)

    Hydroponics_SensorType_Count,                   // Internal use only
    Hydroponics_SensorType_Undefined = -1           // Internal use only
};

// Units Type
enum Hydroponics_UnitsType {
    Hydroponics_UnitsType_Temperature_Celsius,      // Celsius temperature mode.
    Hydroponics_UnitsType_Temperature_Fahrenheit,   // Fahrenheit temperature mode.
    Hydroponics_UnitsType_Temperature_Kelvin,       // Kelvin temperature mode.
    Hydroponics_UnitsType_Distance_Meters,          // Meters distance mode.
    Hydroponics_UnitsType_Distance_Feet,            // Feet distance mode.
    Hydroponics_UnitsType_Weight_Kilogram,          // Kilogram weight mode.
    Hydroponics_UnitsType_Weight_Pounds,            // Pounds weight mode.
    Hydroponics_UnitsType_LiquidVolume_Liters,      // Liters liquid volume mode.
    Hydroponics_UnitsType_LiquidVolume_Gallons,     // Gallons liquid volume mode.
    Hydroponics_UnitsType_LiquidFlow_LitersPerMin,  // Liters per minute liquid flow mode.
    Hydroponics_UnitsType_LiquidFlow_GallonsPerMin, // Gallons per minute liquid flow mode.
    Hydroponics_UnitsType_pHScale_0_14,             // pH scale [0.0,14.0] mode.
    Hydroponics_UnitsType_Concentration_EC,         // Electrical conductivity concentration mode.
    Hydroponics_UnitsType_Concentration_PPM,        // Parts-per-million concentration mode.
    Hydroponics_UnitsType_Percentile_0_100,         // Percentile [0.0,100.0] mode.
    Hydroponics_UnitsType_Raw_0_1,                  // Raw value [0.0,1.0] mode.

    Hydroponics_UnitsType_Count,                    // Internal use only
    Hydroponics_UnitsType_Undefined = -1            // Internal use only
};


// Serializable Interface
struct HydroponicsSerializableInterface {
    // Given a JSON document to fill in, writes self to JSON format.
    virtual void toJSONDocument(JsonDocument &docOut) const = 0;

    // Given a JSON document to read from, reads JSON overtop self.
    virtual void fromJSONDocument(const JsonDocument &docIn) = 0;
};

// Base class for serializable (JSON+Binary) storage data.
// NOTE: NO REFERENCES OR CONSTS ALLOWED. All data *MUST* be fully memcpy'able.
struct HydroponicsData : public HydroponicsSerializableInterface {
    // Default constructor
    HydroponicsData(const char *ident,
                    uint16_t version = 1, uint16_t revision = 1);

    char _ident[4];                                             // Data structure identifier
    uint16_t _version;                                          // Version # of data container
    uint16_t _revision;                                         // Revision # of stored data
};

// User System Setup Data
struct HydroponicsSystemData : public HydroponicsData {         // _ident: HSYS
    HydroponicsSystemData();                                    // Default constructor
    HydroponicsSystemData(const JsonDocument *docIn);           // From JSON convenience constructor

    // Overrides
    virtual void toJSONDocument(JsonDocument &docOut) const;
    virtual void fromJSONDocument(const JsonDocument &docIn);

    char systemName[HYDRUINO_NAME_MAXSIZE];                     // System name
    int8_t timeZoneOffset;                                      // Timezone offset (TODO)
    Hydroponics_SystemMode systemMode;                          // System type mode
    Hydroponics_MeasurementMode measurementMode;                // System measurement mode
    Hydroponics_LCDOutputMode lcdOutMode;                       // System LCD mode
    Hydroponics_ControlInputMode ctrlInMode;                    // System control input mode 
    byte cropPositionsCount;                                    // Total number of crop positions
    byte maxActiveRelayCount[Hydroponics_RelayRail_Count];      // Total active relays on same rail per rail
    float reservoirSize[Hydroponics_FluidReservoir_Count];      // Total reservoir size of each
    Hydroponics_UnitsType reservoirSizeUnits;                   // Units of reservoir size
    float pumpFlowRate[Hydroponics_FluidReservoir_Count];       // Pump flow rate for each reservoir
    Hydroponics_UnitsType pumpFlowRateUnits;                    // Units of pump flow rate

    // Other TODO stuff not used yet
    float phDriftDataTODO;                                      // TODO
    float ecDriftDataTODO;                                      // TODO
};

// Sensor Calibration Data
struct HydroponicsCalibrationData : public HydroponicsData {    // _ident: HCAL
    HydroponicsCalibrationData();
    HydroponicsCalibrationData(Hydroponics_SensorType sensor, Hydroponics_FluidReservoir reservoir);

    // Overrides
    virtual void toJSONDocument(JsonDocument &docOut) const;
    virtual void fromJSONDocument(const JsonDocument &docIn);

    Hydroponics_SensorType sensor;                              // Sensor type this applies to
    Hydroponics_FluidReservoir reservoir;                       // Reservoir type this applies to
    float multiplier, offset;                                   // Ax + B curve correction
};

// Crop Library Data
struct HydroponicsCropData : public HydroponicsData {           // _ident: HCRP
    HydroponicsCropData();                                      // Default constructor
    HydroponicsCropData(Hydroponics_CropType cropType);         // Convenience constructor, checks out data from Crop Library then returns, good for temporary objects.

    // Overrides
    virtual void toJSONDocument(JsonDocument &docOut) const;
    virtual void fromJSONDocument(const JsonDocument &docIn);

    Hydroponics_CropType cropType;                              // Crop type
    char plantName[HYDRUINO_NAME_MAXSIZE];                      // Name of plant
    byte growWeeksToHarvest;                                    // How long it takes to grow before harvestable
    byte weeksBetweenHarvest;                                   // How long it takes between harvests, if applicable
    byte phaseBeginWeek[Hydroponics_CropPhase_Count];           // Which week the plating phase generally begins.
    byte lightHoursPerDay[Hydroponics_CropPhase_Count];         // How many light hours is needed per each day.
    byte feedIntervalMins[Hydroponics_CropPhase_Count][2];      // How much time between feedings (on time / off time)
    float phRange[Hydroponics_CropPhase_Count][2];              // pH range acceptable ( min / max, or avg )
    float ecRange[Hydroponics_CropPhase_Count][2];              // EC range ( min / max, or avg )
    float waterTempRange[Hydroponics_CropPhase_Count][2];       // Water temperature range (C) (maybe?)
    float airTempRange[Hydroponics_CropPhase_Count][2];         // Air temperature range (C)
    bool isInvasiveOrViner;                                     // Flag indicating plant will vine and/or take over other plants
    bool isLargePlant;                                          // Flag indicating plant requires proper supports
    bool isPerennial;                                           // Flag indicating plant grows back year after year
    bool isPrunningRequired;                                    // Flag indicating plant benefits from active prunning
    bool isToxicToPets;                                         // Flag indicating plant toxicity to common house pets (cats + dogs)
};


// Loggable Data Interface TODO
struct HydroponicsLoggableDataInterface {
    // virtual todo toMQTTPublish(todo *mqttBroker) = 0;
    // virtual todo toSDCardCSVAppend(todo *csvFile) = 0;
    // virtual todo toWebServerDBInsert(todo *dbInterface) = 0;
    // virtual todo toWebServerAPIService(todo *apiInterface) = 0;
};

// Sensor Data Measurement
struct HydroponicsSensorMeasurement : public HydroponicsLoggableDataInterface
{
    time_t timestamp;                       // Time event recorded
};

// Analog Sensor Data Measurement
struct HydroponicsAnalogSensorMeasurement : public HydroponicsSensorMeasurement
{
    float value;                            // Analog value
    Hydroponics_UnitsType units;            // Units of value
};

// DHT Sensor Data Measurement
struct HydroponicsDHTSensorMeasurement : public HydroponicsSensorMeasurement
{
    float temperature;                      // Temperature
    Hydroponics_UnitsType temperatureUnits; // Units of temperature
    float humidity;                         // Hunidity
    Hydroponics_UnitsType humidityUnits;    // Units of humidity
    float heatIndex;                        // Heat index
    Hydroponics_UnitsType heatIndexUnits;   // Units of heat index
};

// Binary Sensor Data Measurement
struct HydroponicsBinarySensorMeasurement : public HydroponicsSensorMeasurement
{
    bool state;                             // Trigger state
};

// Binary Analog Sensor Data Measurement
struct HydroponicsBinaryAnalogSensorMeasurement : public HydroponicsSensorMeasurement
{
    float value;                            // Analog value
    Hydroponics_UnitsType units;            // Units of value
    bool state;                             // Trigger state
};

#endif // /ifndef HydroponicsDefines_H
