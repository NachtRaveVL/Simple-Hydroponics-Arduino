/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Data Objects
*/

#ifndef HydroponicsDatas_H
#define HydroponicsDatas_H

struct HydroponicsData;
struct HydroponicsSystemData;
struct HydroponicsCalibrationData;
struct HydroponicsCropsLibData;

#include "Hydroponics.h"

// Creates a new hydroponics data object corresponding to an input binary stream (ownership transfer - user code *must* delete returned data).
extern HydroponicsData *dataFromBinaryStream(Stream *streamIn);

// Creates a new hydroponics data object corresponding to an input JSON element (ownership transfer - user code *must* delete returned data).
extern HydroponicsData *dataFromJSONElement(JsonVariantConst &elementIn);

// Hydroponics Data Base Class
// Base class for serializable (JSON+Binary) storage data, used to define the base
// header of all data stored internally.
// NOTE: NON-CONST VALUE TYPES ONLY. All data *MUST* be memcpy'able.
struct HydroponicsData : public HydroponicsJSONSerializableInterface, public HydroponicsBinarySerializableInterface {
    HydroponicsData();                                          // Default constructor
    HydroponicsData(const char *ident,                          // 4-char identifier
                    uint16_t version = 1,                       // Data structure version #
                    uint16_t revision = 1);                     // Stored data revision #
    HydroponicsData(int16_t idType,                             // ID type enum value
                    int16_t classType,                          // Class type enum value
                    uint16_t version = 1,                       // Data structure version #
                    uint16_t revision = 1);                     // Stored data revision #

    virtual void toBinaryStream(Print *streamOut) const override;
    virtual void fromBinaryStream(Stream *streamIn) override;
    virtual void toJSONElement(JsonVariant &elementOut) const override;
    virtual void fromJSONElement(JsonVariantConst &elementIn) override;

    union {
        char chars[4];                                          // Data structure 4-char identifier
        struct {
          int16_t idType;                                       // Object type enum value (e.g. actuator, sensor, etc.)
          int16_t classType;                                    // Object class type enum value (e.g. pump, dht1w, etc.)
        } object;
    } _ident;                                                   // Identifier union
    uint16_t _size;                                             // The size (in bytes) of the data
    uint16_t _version;                                          // Version # of data container
    uint16_t _revision;                                         // Revision # of stored data
    bool _modified;                                             // Flag tracking modified status

    inline void _bumpRev() { _revision += 1; _setModded(); }
    inline void _bumpRevIfNotAlreadyModded() { if (!_modified) { _bumpRev(); } } // Should be called before modifying data
    inline void _setModded() { _modified = true; }              // Should be called after modifying any data
    inline void _unsetModded() { _modified = false; }           // Should be called after save-out

    inline void _copyFrom(const HydroponicsData *otherData) { memcpy(this, otherData, min(_size, otherData->_size)); _setModded(); }
    inline void _copyTo(HydroponicsData *otherData) { if (otherData) { otherData->_copyFrom(this); } }

    inline bool isSystemData() { return strncasecmp(_ident.chars, "HSYS", 4) == 0; }
    inline bool isCalibrationData() { return strncasecmp(_ident.chars, "HSYS", 4) == 0; }
    inline bool isCropsLibData() { return strncasecmp(_ident.chars, "HCLD", 4) == 0; }
    inline bool isUnknownData() { return !isSystemData() && !isCalibrationData() && !isCropsLibData(); }
};

// User System Setup Data
// _ident: HSYS. Hydroponic system setup data.
struct HydroponicsSystemData : public HydroponicsData {
    HydroponicsSystemData();                                    // Default constructor

    void toBinaryStream(Print *streamOut) const override;
    void fromBinaryStream(Stream *streamIn) override;
    void toJSONElement(JsonVariant &elementOut) const override;
    void fromJSONElement(JsonVariantConst &elementIn) override;

    Hydroponics_SystemMode systemMode;                          // System type mode
    Hydroponics_MeasurementMode measureMode;                    // System measurement mode
    Hydroponics_DisplayOutputMode dispOutMode;                  // System display output mode
    Hydroponics_ControlInputMode ctrlInMode;                    // System control input mode 

    char systemName[HYDRUINO_NAME_MAXSIZE];                     // System name
    int8_t timeZoneOffset;                                      // Timezone offset
    uint32_t pollingIntMs;                                      // Sensor polling interval (milliseconds)
};

// Sensor Calibration Data
// _ident: HCAL. Hydroponic sensor calibration data.
// This class essentially controls a custom unit conversion mapping, and is used in
// converting raw sensor data to more useful value and units for doing science with.
// To convert from raw values to calibrated values, use transform(). To convert back to
// raw values from calibrated values, use inverseTransform(). The setFrom* methods
// allow you to easily set calibrated data using more user friendly of input.
struct HydroponicsCalibrationData : public HydroponicsData {
    HydroponicsCalibrationData();
    HydroponicsCalibrationData(HydroponicsIdentity sensorId, Hydroponics_UnitsType calibUnits = Hydroponics_UnitsType_Undefined);

    void toBinaryStream(Print *streamOut) const override;
    void fromBinaryStream(Stream *streamIn) override;
    void toJSONElement(JsonVariant &elementOut) const override;
    void fromJSONElement(JsonVariantConst &elementIn) override;

    // Transforms value from raw (or initial) value into calibrated (or transformed) value.
    inline float transform(float rawValue) const { return (rawValue * multiplier) + offset; }

    // Inverse transforms value from calibrated (or transformed) value back into raw (or initial) value.
    inline float inverseTransform(float calibratedValue) const { return (calibratedValue - offset) / multiplier; }

    // Sets linear calibration curvature from two points.
    // Measured normalized raw values should be between 0.0 and 1.0, and represents
    // the normalized voltage signal measurement from the analogRead() function (after
    // taking into account appropiate bit resolution conversion). Calibrated-to values
    // are what each measurement-at value should map out to.
    // For example, if your sensor should treat 0v (aka 0.0) as a pH of 2 and treat 5v
    // (aka 1.0, or MCU max voltage) as a pH of 10, you would pass 0.0, 2.0, 1.0, 10.0.
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

    HydroponicsIdentity sensorId;                               // Sensor Id this calibration belongs to
    Hydroponics_UnitsType calibUnits;                           // Calibration output units
    float multiplier, offset;                                   // Ax + B value transform
};

// Crop Library Data
// _ident: HCLD. Hydroponic crop library data.
struct HydroponicsCropsLibData : public HydroponicsData {
    HydroponicsCropsLibData();                                   // Default constructor
    HydroponicsCropsLibData(Hydroponics_CropType cropType);      // Convenience constructor, checks out data from Crop Library then returns, good for temporary objects.

    void toBinaryStream(Print *streamOut) const override;
    void fromBinaryStream(Stream *streamIn) override;
    void toJSONElement(JsonVariant &elementOut) const override;
    void fromJSONElement(JsonVariantConst &elementIn) override;

    Hydroponics_CropType cropType;                              // Crop type
    char cropName[HYDRUINO_NAME_MAXSIZE];                       // Name of crop
    byte growWeeksToHarvest;                                    // How long it takes to grow before harvestable
    byte weeksBetweenHarvest;                                   // How long it takes between harvests, if applicable
    byte phaseBeginWeek[Hydroponics_CropPhase_Count];           // Which week the plating phase generally begins
    byte lightHoursPerDay[Hydroponics_CropPhase_Count];         // How many light hours is needed per each day (in # hours)
    byte feedIntervalMins[Hydroponics_CropPhase_Count][2];      // Feeding interval (on time / off time, or cooldown time) (in # mins)
    float phRange[Hydroponics_CropPhase_Count][2];              // pH range acceptable ( min / max , or mid )
    float ecRange[Hydroponics_CropPhase_Count][2];              // EC range ( min / max , or mid )
    float waterTempRange[Hydroponics_CropPhase_Count][2];       // Water temperature range (in F)
    float airTempRange[Hydroponics_CropPhase_Count][2];         // Air temperature range (in F)
    bool isInvasiveOrViner;                                     // Flag indicating plant is invasive, will vine, and/or take over other plants
    bool isLargePlant;                                          // Flag indicating plant requires proper supports
    bool isPerennial;                                           // Flag indicating plant grows back year after year
    bool isPruningRequired;                                     // Flag indicating plant benefits from active pruning
    bool isToxicToPets;                                         // Flag indicating plant toxicity to common house pets (cats + dogs)
};

#endif // /ifndef HydroponicsDatas_H
