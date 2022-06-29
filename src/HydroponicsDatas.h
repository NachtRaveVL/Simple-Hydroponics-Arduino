/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Data Objects
*/

#ifndef HydroponicsDatas_H
#define HydroponicsDatas_H

struct HydroponicsData;
struct HydroponicsSubData;
struct HydroponicsSystemData;
struct HydroponicsCalibrationData;
struct HydroponicsCropsLibData;

#include "Hydroponics.h"

// Serializes Hydroponics data structure to a binary output stream (essentially a memcpy), with optional skipBytes
extern size_t serializeDataToBinaryStream(HydroponicsData *data, Stream *streamOut, size_t skipBytes = sizeof(void*));
// Deserializes Hydroponics data structure from a binary input stream (essentially a memcpy), with optional skipBytes
extern size_t deserializeDataFromBinaryStream(HydroponicsData *data, Stream *streamIn, size_t skipBytes = sizeof(void*));

// Creates a new hydroponics data object corresponding to a binary input stream (return ownership transfer - user code *must* delete returned data)
extern HydroponicsData *newDataFromBinaryStream(Stream *streamIn);
// Creates a new hydroponics data object corresponding to an input JSON element (return ownership transfer - user code *must* delete returned data)
extern HydroponicsData *newDataFromJSONObject(JsonObjectConst &objectIn);

// Hydroponics Data Base
// Base class for serializable (JSON+Binary) storage data, used to define the base
// header of all data stored internally.
// NOTE: NON-CONST VALUE TYPES ONLY. All data *MUST* be directly memcpy'able.
struct HydroponicsData : public HydroponicsJSONSerializableInterface {
    union {
        char chars[4];                                          // Data structure 4-char identifier
        struct {
          int8_t idType;                                        // Object ID type enum value (e.g. actuator, sensor, etc.)
          int8_t objType;                                       // Object type enum value (e.g. actuatorType, sensorType, etc.)
          int8_t posIndex;                                      // Object position index # (zero-ordinal)
          int8_t classType;                                     // Object class type enum value (e.g. pump, dht1w, etc.)
        } object;
    } id;                                                       // Identifier union
    uint16_t _size;                                             // The size (in bytes) of the data
    uint8_t _version;                                           // Version # of data container
    uint8_t _revision;                                          // Revision # of stored data
    bool _modified;                                             // Flag tracking modified status

    inline bool isStdData() const { return strncasecmp(id.chars, "H", 1) == 0; } // Standalone data
    inline bool isSystemData() const { return strncasecmp(id.chars, "HSYS", 4) == 0; }
    inline bool isCalibrationData() const { return strncasecmp(id.chars, "HSYS", 4) == 0; }
    inline bool isCropsLibData() const { return strncasecmp(id.chars, "HCLD", 4) == 0; }
    inline bool isObjData() const { return !isStdData() && id.object.idType >= 0; }
    inline bool isUnknownData() const { return !isSystemData() && !isCalibrationData() && !isCropsLibData(); }

    HydroponicsData();                                          // Default constructor
    HydroponicsData(const char *id,                             // 4-char identifier
                    uint8_t version = 1,                        // Data structure version #
                    uint8_t revision = 1);                      // Stored data revision #
    HydroponicsData(int8_t idType,                              // ID type enum value
                    int8_t objType,                             // Object type enum value
                    int8_t posIndex,                            // Object position index #
                    int8_t classType,                           // Class type enum value
                    uint8_t version = 1,                        // Data structure version #
                    uint8_t revision = 1);                      // Stored data revision #
    HydroponicsData(const HydroponicsIdentity &id);             // Identity constructor

    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;

    inline void _bumpRev() { _revision += 1; _setModded(); }
    inline void _bumpRevIfNotAlreadyModded() { if (!_modified) { _bumpRev(); } } // Should be called before modifying data
    inline void _setModded() { _modified = true; }              // Should be called after modifying any data
    inline void _unsetModded() { _modified = false; }           // Should be called after save-out
};

// Hydroponics Sub Data Base
// Sub-data exists inside of regular data for smaller objects that don't require the
// entire data object hierarchy, useful for triggers, measurements, etc.
// NOTE: NON-CONST VALUE TYPES ONLY, NO VIRTUALS. All data *MUST* be directly memcpy'able.
struct HydroponicsSubData {
    int8_t type;

    HydroponicsSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

// User System Setup Data
// id: HSYS. Hydroponic system setup data.
struct HydroponicsSystemData : public HydroponicsData {
    Hydroponics_SystemMode systemMode;                          // System type mode
    Hydroponics_MeasurementMode measureMode;                    // System measurement mode
    Hydroponics_DisplayOutputMode dispOutMode;                  // System display output mode
    Hydroponics_ControlInputMode ctrlInMode;                    // System control input mode 
    char systemName[HYDRUINO_NAME_MAXSIZE];                     // System name
    byte ctrlInputPinMap[HYDRUINO_CTRLINPINMAP_MAXSIZE];        // Control input pinmap
    int8_t timeZoneOffset;                                      // Timezone offset
    uint32_t pollingInterval;                                   // Sensor polling interval, in milliseconds
    time_t lastWaterChangeTime;                                 // Last water change time (recycling systems only)

    HydroponicsSystemData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Sensor Calibration Data
// id: HCAL. Hydroponic sensor calibration data.
// This class essentially controls a custom unit conversion mapping, and is used in
// converting raw sensor data to more useful value and units for doing science with.
// To convert from raw values to calibrated values, use transform(). To convert back to
// raw values from calibrated values, use inverseTransform(). The setFrom* methods
// allow you to easily set calibrated data in various formats.
struct HydroponicsCalibrationData : public HydroponicsData {
    char sensorName[HYDRUINO_NAME_MAXSIZE];                     // Sensor name this calibration belongs to
    Hydroponics_UnitsType calibUnits;                           // Calibration output units
    float multiplier, offset;                                   // Ax + B value transform

    HydroponicsCalibrationData();
    HydroponicsCalibrationData(HydroponicsIdentity sensorId, Hydroponics_UnitsType calibUnits = Hydroponics_UnitsType_Undefined);

    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;

    // Transforms value from raw (or initial) value into calibrated (or transformed) value.
    inline float transform(float rawValue) const { return (rawValue * multiplier) + offset; }

    // Inverse transforms value from calibrated (or transformed) value back into raw (or initial) value.
    inline float inverseTransform(float calibratedValue) const { return (calibratedValue - offset) / multiplier; }

    // Sets linear calibration curvature from two points.
    // Measured normalized raw values should be between 0.0 and 1.0, and represents
    // the normalized voltage signal measurement from the analogRead() function (after
    // taking into account appropiate bit resolution conversion). Calibrated-to values
    // are what each measurement-at value should map out to.
    // For example, if your sensor should treat 0v (aka 0.0) typeAs a pH of 2 and treat 5v
    // (aka 1.0, or MCU max voltage) typeAs a pH of 10, you would pass 0.0, 2.0, 1.0, 10.0.
    // The final calculated curvature transform, for this example, would be y = 8x + 2.
    void setFromTwoPoints(float point1RawMeasuredAt,            // What normalized value point 1 measured in at [0.0,1.0]
                          float point1CalibratedTo,             // What value point 1 should be mapped to
                          float point2RawMeasuredAt,            // What normalized value point 2 measured in at [0.0,1.0]
                          float point2CalibratedTo);            // What value point 2 should be mapped to

    // Sets linear calibration curvature from two voltages.
    // Wrapper to setFromTwoPoints, used when raw voltage values are easier to work with.
    inline void setFromTwoVoltages(float point1VoltageAt,       // What raw voltage value point 1 measured in at [0.0,aRef]
                                   float point1CalibratedTo,    // What value point 1 should be mapped to
                                   float point2VoltageAt,       // What raw voltage value point 2 measured in at [0.0,aRef]
                                   float point2CalibratedTo,    // What value point 2 should be mapped to
                                   float analogRefVoltage) {    // aRef: 5.0 for 5v MCUs, otherwise 3.3 for 3.3v MCUs
        setFromTwoPoints(point1VoltageAt / analogRefVoltage, point1CalibratedTo,
                         point2VoltageAt / analogRefVoltage, point2CalibratedTo);
    }

    // Sets linear calibration curvature from known output range.
    // Wrapper to setFromTwoPoints, used when the sensor uses the entire voltage range
    // with a known min/max value at each end. This will map 0v (aka 0.0) to min value
    // and 5v (aka 1.0, or MCU max voltage) to max value.
    inline void setFromRange(float min, float max) { setFromTwoPoints(0.0, min, 1.0, max); }

    // Sets linear calibration curvature from known output scale.
    // Similar to setFromTwoPoints, but when the sensor has a known max scale.
    // This will map 0v to 0 and 5v (or MCU max voltage) to scale value.
    inline void setFromScale(float scale) { setFromRange(0.0, scale); }
};

// Crop Library Data
// id: HCLD. Hydroponic crop library data.
struct HydroponicsCropsLibData : public HydroponicsData {
    Hydroponics_CropType cropType;                              // Crop type
    char cropName[HYDRUINO_NAME_MAXSIZE];                       // Name of crop
    byte totalGrowWeeks;                                        // How long it takes to grow until harvestable, in weeks (default: 14)
    byte lifeCycleWeeks;                                        // How long a perennials life cycle lasts, in weeks (default: 0)
    byte phaseDurationWeeks[Hydroponics_CropPhase_MainCount];   // How many weeks each main crop phase lasts (seed,veg,bloom - default: 2,4,8)
    byte dailyLightHours[Hydroponics_CropPhase_MainCount];      // How many light hours per day is needed per main stages (seed,veg,bloom or all - default: 20,18,12)
    float phRange[2];                                           // Base acceptable pH range (min,max or mid - default: 6)
    float ecRange[2];                                           // Base acceptable EC range (min,max or mid - default: 1)
    float waterTempRange[2];                                    // Water temperature range (in C, min,max or mid - default: 25)
    float airTempRange[2];                                      // Air temperature range (in C, min,max or mid - default: 25)
    bool isInvasiveOrViner;                                     // Flag indicating plant is invasive, will vine, and/or take over other plants (default: false)
    bool isLargePlant;                                          // Flag indicating plant requires proper supports (default: false)
    bool isPerennial;                                           // Flag indicating plant grows back year after year (default: false)
    bool isPruningRequired;                                     // Flag indicating plant benefits from active pruning (default: false)
    bool isToxicToPets;                                         // Flag indicating plant toxicity to common house pets (cats+dogs - default: false)

    HydroponicsCropsLibData();
    HydroponicsCropsLibData(Hydroponics_CropType cropType);     // Convenience constructor, checks out data from Crop Library then returns, good for temporary objects.

    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Internal use, but must contain all ways for all data types to be new'ed
extern HydroponicsData *_allocateDataFromBaseDecode(const HydroponicsData &baseDecode);
extern HydroponicsData *_allocateDataForObjType(int8_t idType, int8_t classType);

#endif // /ifndef HydroponicsDatas_H
