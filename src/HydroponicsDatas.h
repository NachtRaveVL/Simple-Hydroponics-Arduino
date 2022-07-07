/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Datas
*/

#ifndef HydroponicsDatas_H
#define HydroponicsDatas_H

struct HydroponicsSystemData;
struct HydroponicsCalibrationData;
struct HydroponicsCropsLibData;
struct HydroponicsCustomAdditiveData;

#include "Hydroponics.h"
#include "HydroponicsData.h"
#include "HydroponicsScheduler.h"

// User System Setup Data
// id: HSYS. Hydroponic user system setup data.
struct HydroponicsSystemData : public HydroponicsData {
    Hydroponics_SystemMode systemMode;                          // System type mode
    Hydroponics_MeasurementMode measureMode;                    // System measurement mode
    Hydroponics_DisplayOutputMode dispOutMode;                  // System display output mode
    Hydroponics_ControlInputMode ctrlInMode;                    // System control input mode 
    char systemName[HYDRUINO_NAME_MAXSIZE];                     // System name
    int8_t timeZoneOffset;                                      // Timezone offset
    uint32_t pollingInterval;                                   // Sensor polling interval, in milliseconds
    HydroponicsSchedulerSubData scheduler;                      // Scheduler subdata

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
                                   float analogRefVoltage) {    // aRef: Value of aRef pin (if not connected uses default of 5 for 5v MCUs, 3.3 for 3.3v MCUs)
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
    float tdsRange[2];                                          // Base acceptable TDS/EC range (min,max or mid - default: 1.8,2.4)
    float nightlyFeedMultiplier;                                // Nightly feed multiplier, if crop uses a lower TDS/EC at night (default: 1)
    float waterTempRange[2];                                    // Water temperature range (in C, min,max or mid - default: 25)
    float airTempRange[2];                                      // Air temperature range (in C, min,max or mid - default: 25)
    bool isInvasiveOrViner;                                     // Flag indicating plant is invasive, will vine, and/or take over other plants (default: false)
    bool isLargePlant;                                          // Flag indicating plant requires proper supports (default: false)
    bool isPerennial;                                           // Flag indicating plant grows back year after year (default: false)
    bool isToxicToPets;                                         // Flag indicating plant toxicity to common house pets (cats+dogs - default: false)
    bool isPruningNeeded;                                       // Flag indicating plant benefits from active pruning (default: false)
    bool isSprayingNeeded;                                      // Flag indicating plant benefits from spraying in the morning (default: false)

    HydroponicsCropsLibData();
    HydroponicsCropsLibData(Hydroponics_CropType cropType);     // Convenience constructor, checks out data from Crop Library then returns, good for temporary objects.
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};


// Custom Additive Data
// id: HADD. Hydroponic custom additive data.
struct HydroponicsCustomAdditiveData : public HydroponicsData {
    Hydroponics_ReservoirType reservoirType;                    // Reservoir type (must be CustomAdditive*)
    char additiveName[HYDRUINO_NAME_MAXSIZE];                   // Name of additive
    float weeklyDosingRates[HYDRUINO_CROP_GROWEEKS_MAX];        // Weekly dosing rate percentages (default: 0)

    HydroponicsCustomAdditiveData();
    HydroponicsCustomAdditiveData(Hydroponics_ReservoirType reservoirType); // Convenience constructor, copies data from Hydroponics system then returns, good for temporary objects.
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};


// Internal use, but must contain all ways for all data types to be new'ed
extern HydroponicsData *_allocateDataFromBaseDecode(const HydroponicsData &baseDecode);
extern HydroponicsData *_allocateDataForObjType(int8_t idType, int8_t classType);

#endif // /ifndef HydroponicsDatas_H
