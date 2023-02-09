/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Datas
*/

#ifndef HydroDatas_H
#define HydroDatas_H

struct HydroSystemData;
struct HydroCalibrationData;
struct HydroCropsLibData;
struct HydroCustomAdditiveData;

#include "Hydruino.h"
#include "HydroData.h"
#include "HydroScheduler.h"
#include "HydroPublisher.h"
#include "HydroLogger.h"

// Autosave Enumeration
enum Hydro_Autosave : signed char {
    Hydro_Autosave_EnabledToSDCardJson,                     // Autosave to SD card in Json
    Hydro_Autosave_EnabledToSDCardRaw,                      // Autosave to SD card in binary
    Hydro_Autosave_EnabledToEEPROMJson,                     // Autosave to EEPROM in Json
    Hydro_Autosave_EnabledToEEPROMRaw,                      // Autosave to EEPROM in binary
    Hydro_Autosave_EnabledToWiFiStorageJson,                // Autosave to WiFiStorage in Json
    Hydro_Autosave_EnabledToWiFiStorageRaw,                 // Autosave to WiFiStorage in binary
    Hydro_Autosave_Disabled = -1                            // Autosave disabled
};

// User System Setup Data
// id: HSYS. Hydruino user system setup data.
struct HydroSystemData : public HydroData {
    Hydro_SystemMode systemMode;                            // System type mode
    Hydro_MeasurementMode measureMode;                      // System measurement mode
    Hydro_DisplayOutputMode dispOutMode;                    // System display output mode
    Hydro_ControlInputMode ctrlInMode;                      // System control input mode 
    char systemName[HYDRO_NAME_MAXSIZE];                    // System name
    int8_t timeZoneOffset;                                  // Timezone offset
    uint16_t pollingInterval;                               // Sensor polling interval, in milliseconds
    Hydro_Autosave autosaveEnabled;                         // Autosave enabled
    Hydro_Autosave autosaveFallback;                        // Autosave fallback
    uint16_t autosaveInterval;                              // Autosave interval, in minutes
    char wifiSSID[HYDRO_NAME_MAXSIZE];                      // WiFi SSID
    uint8_t wifiPassword[HYDRO_NAME_MAXSIZE];               // WiFi password (xor encrypted)
    uint32_t wifiPasswordSeed;                              // Seed for WiFi password one-time pad
    uint8_t macAddress[6];                                  // Ethernet MAC address
    double latitude;                                        // System latitude
    double longitude;                                       // System longitude
    double altitude;                                        // System altitude

    HydroSchedulerSubData scheduler;                        // Scheduler subdata
    HydroLoggerSubData logger;                              // Logger subdata
    HydroPublisherSubData publisher;                        // Publisher subdata

    HydroSystemData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};


// Calibration Data
// id: HCAL. Hydruino linear calibration data.
// This class essentially controls a simple Ax+B linear transformation mapping, and is
// used to 'convert' values from one coordinate system into another, or in our case used
// for storing custom user curve/offset correction/mapping data.
// See setFrom* methods to set calibrated data in various formats.
struct HydroCalibrationData : public HydroData {
    char ownerName[HYDRO_NAME_MAXSIZE];                     // Owner object name this calibration belongs to (actuator/sensor)
    Hydro_UnitsType calibUnits;                             // Calibration output units
    float multiplier, offset;                               // Ax + B value transform coefficients

    HydroCalibrationData();
    HydroCalibrationData(HydroIdentity ownerId,
                         Hydro_UnitsType calibUnits = Hydro_UnitsType_Undefined);

    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;

    // Transforms value from raw (or initial) value into calibrated (or transformed) value.
    inline float transform(float value) const { return (value * multiplier) + offset; }
    // Transforms value in-place from raw (or initial) value into calibrated (or transformed) value, with optional units write out.
    inline void transform(float *valueInOut, Hydro_UnitsType *unitsOut = nullptr) const { *valueInOut = transform(*valueInOut);
                                                                                          if (unitsOut) { *unitsOut = calibUnits; } }
    // Transforms measurement from raw (or initial) measurement into calibrated (or transformed) measurement.
    inline HydroSingleMeasurement transform(HydroSingleMeasurement measurement) { return HydroSingleMeasurement(transform(measurement.value), calibUnits, measurement.timestamp, measurement.frame); }
    // Transforms measurement in-place from raw (or initial) measurement into calibrated (or transformed) measurement.
    inline void transform(HydroSingleMeasurement *measurementInOut) const { transform(&measurementInOut->value, &measurementInOut->units); }

    // Inverse transforms value from calibrated (or transformed) value back into raw (or initial) value.
    inline float inverseTransform(float value) const { return (value - offset) / multiplier; }
    // Inverse transforms value in-place from calibrated (or transformed) value back into raw (or initial) value, with optional units write out.
    inline void inverseTransform(float *valueInOut, Hydro_UnitsType *unitsOut = nullptr) const { *valueInOut = inverseTransform(*valueInOut);
                                                                                                 if (unitsOut) { *unitsOut = Hydro_UnitsType_Raw_0_1; } }
    // Inverse transforms measurement from calibrated (or transformed) measurement back into raw (or initial) measurement.
    inline HydroSingleMeasurement inverseTransform(HydroSingleMeasurement measurement) { return HydroSingleMeasurement(inverseTransform(measurement.value), calibUnits, measurement.timestamp, measurement.frame); }
    // Inverse transforms measurement in-place from calibrated (or transformed) measurement back into raw (or initial) measurement.
    inline void inverseTransform(HydroSingleMeasurement *measurementInOut) const { inverseTransform(&measurementInOut->value, &measurementInOut->units); }

    // Sets linear calibration curvature from two points.
    // Measured normalized raw values should be between 0.0 and 1.0, and represents
    // the normalized voltage signal measurement from the analogRead() function (after
    // taking into account appropiate bit resolution conversion). Calibrated-to values
    // are what each measurement-at value should map out to.
    // For example, if your sensor should treat 0v (aka 0.0) as a value of 2 and treat 5v
    // (aka 1.0, or MCU max voltage) as a value of 10, you would pass 0.0, 2.0, 1.0, 10.0.
    // The final calculated curvature transform, for this example, would be y = 8x + 2.
    void setFromTwoPoints(float point1RawMeasuredAt,        // What normalized value point 1 measured in at [0.0,1.0]
                          float point1CalibratedTo,         // What value point 1 should be mapped to
                          float point2RawMeasuredAt,        // What normalized value point 2 measured in at [0.0,1.0]
                          float point2CalibratedTo);        // What value point 2 should be mapped to

    // Sets linear calibration curvature from two voltages.
    // Wrapper to setFromTwoPoints, used when raw voltage values are easier to work with.
    inline void setFromTwoVoltages(float point1VoltsAt,     // What raw voltage value point 1 measured in at [0.0,aRef]
                                   float point1CalibTo,     // What value point 1 should be mapped to
                                   float point2VoltsAt,     // What raw voltage value point 2 measured in at [0.0,aRef]
                                   float point2CalibTo,     // What value point 2 should be mapped to
                                   float analogRefVolts) {  // aRef: Value of aRef pin (use 5 for 5v MCUs, 3.3 for 3.3v MCUs)
        setFromTwoPoints(point1VoltsAt / analogRefVolts, point1CalibTo,
                         point2VoltsAt / analogRefVolts, point2CalibTo);
    }

    // Sets linear calibration curvature from known output range.
    // Wrapper to setFromTwoPoints, used when data uses the entire intensity range with a known min/max value at each end.
    // E.g. will map 0v (aka 0.0) to min value and 5v (aka 1.0, or MCU max voltage) to max value.
    inline void setFromRange(float min, float max) { setFromTwoPoints(0.0, min, 1.0, max); }

    // Sets linear calibration curvature from known output scale.
    // Similar to setFromTwoPoints, but when data has a known max intensity.
    // E.g. will map 0v to 0 and 5v (aka 1.0, or MCU max voltage) to scale value.
    inline void setFromScale(float scale) { setFromRange(0.0, scale); }

    // Sets linear calibration curvature from typical servo ranges.
    // Wrapper to setFromTwoPoints, used for specifying servo degree operation ranges using the typical 2.5% and 12.5% phase lengths that hobbyist servos operate at.
    // E.g. will map 2.5% (servo min/neg position/speed) to minDegrees and 12.5% (servo max/pos position/speed) to maxDegrees.
    inline void setFromServo(float minDegrees, float maxDegrees) { setFromTwoPoints(0.025f, minDegrees, 0.125f, maxDegrees); }
};


// Crops Data Flags
enum Hydro_CropsDataFlag : unsigned char {
    Hydro_CropsDataFlag_Invasive        = 0x01,             // Flag indicating plant is invasive and will take over other plants (default: false)
    Hydro_CropsDataFlag_Viner           = 0x02,             // Flag indicating plant is a viner and will require a stick for support (default: false)
    Hydro_CropsDataFlag_Large           = 0x04,             // Flag indicating plant grows large and will require proper support (default: false)
    Hydro_CropsDataFlag_Perennial       = 0x08,             // Flag indicating plant grows back year after year (default: false)
    Hydro_CropsDataFlag_Toxic           = 0x10,             // Flag indicating plant toxicity to common house pets (cats+dogs - default: false)
    Hydro_CropsDataFlag_Pruning         = 0x20,             // Flag indicating plant benefits from active pruning (default: false)
    Hydro_CropsDataFlag_Spraying        = 0x40,             // Flag indicating plant benefits from spraying in the morning (default: false)
    Hydro_CropsDataFlag_None            = 0x00              // Placeholder
};

// Crops Library Data
// id: HCLD. Hydruino crops library data.
struct HydroCropsLibData : public HydroData {
    Hydro_CropType cropType;                                // Crop type
    char cropName[HYDRO_NAME_MAXSIZE];                      // Name of crop
    uint8_t totalGrowWeeks;                                 // How long it takes to grow until harvestable, in weeks (default: 14)
    uint8_t lifeCycleWeeks;                                 // How long a perennials life cycle lasts, in weeks (default: 0)
    uint8_t phaseDurationWeeks[Hydro_CropPhase_MainCount];  // How many weeks each main crop phase lasts (seed,veg,bloom&> - default: 2,4,8)
    uint8_t dailyLightHours[Hydro_CropPhase_MainCount];     // How many light hours per day is needed per main stages (seed,veg,bloom&> or all - default: 20,18,12)
    float phRange[2];                                       // Ideal pH range (min,max or mid - default: 6)
    float tdsRange[2];                                      // Ideal TDS/EC range (min,max or mid - default: 1.8,2.4)
    float nightlyFeedRate;                                  // Nightly feed multiplier, if crop uses a lower TDS/EC at night (default: 1)
    float waterTempRange[2];                                // Ideal water temperature range, in Celsius (min,max or mid - default: 25)
    float airTempRange[2];                                  // Ideal air temperature range, in Celsius (min,max or mid - default: 25)
    float co2Levels[2];                                     // Ideal CO2 levels per <=veg/>=bloom stages, in PPM (seed&veg,bloom&> or all - default: 700, 1400)
    Hydro_CropsDataFlag flags;                              // Crop data flags

    HydroCropsLibData();
    HydroCropsLibData(Hydro_CropType cropType);             // Convenience constructor, checks out data from Crops Library then returns, good for temporary objects.
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;

    inline void setIsInvasive() { flags = (Hydro_CropsDataFlag)(flags | Hydro_CropsDataFlag_Invasive); }
    inline void setIsViner() { flags = (Hydro_CropsDataFlag)(flags | Hydro_CropsDataFlag_Viner); }
    inline void setIsLarge() { flags = (Hydro_CropsDataFlag)(flags | Hydro_CropsDataFlag_Large); }
    inline void setIsPerennial() { flags = (Hydro_CropsDataFlag)(flags | Hydro_CropsDataFlag_Perennial); }
    inline void setIsToxicToPets() { flags = (Hydro_CropsDataFlag)(flags | Hydro_CropsDataFlag_Toxic); }
    inline void setNeedsPrunning() { flags = (Hydro_CropsDataFlag)(flags | Hydro_CropsDataFlag_Pruning); }
    inline void setNeedsSpraying() { flags = (Hydro_CropsDataFlag)(flags | Hydro_CropsDataFlag_Spraying); }

    inline bool isInvasive() const { return flags & Hydro_CropsDataFlag_Invasive; }
    inline bool isViner() const { return flags & Hydro_CropsDataFlag_Viner; }
    inline bool isLarge() const { return flags & Hydro_CropsDataFlag_Large; }
    inline bool isPerennial() const { return flags & Hydro_CropsDataFlag_Perennial; }
    inline bool isToxicToPets() const { return flags & Hydro_CropsDataFlag_Toxic; }
    inline bool needsPrunning() const { return flags & Hydro_CropsDataFlag_Pruning; }
    inline bool needsSpraying() const { return flags & Hydro_CropsDataFlag_Spraying; }
};


// Custom Additive Data
// id: HADD. Hydruino custom additive data.
struct HydroCustomAdditiveData : public HydroData {
    Hydro_ReservoirType reservoirType;                      // Reservoir type (must be CustomAdditive*)
    char additiveName[HYDRO_NAME_MAXSIZE];                  // Name of additive
    float weeklyDosingRates[HYDRO_CROPS_GROWWEEKS_MAX];     // Weekly dosing rate percentages (default: 0)

    HydroCustomAdditiveData();
    HydroCustomAdditiveData(Hydro_ReservoirType reservoirType); // Convenience constructor, copies data from Hydruino system then returns, good for temporary objects.
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};


// Internal use, but must contain all ways for all data types to be new'ed
extern HydroData *_allocateDataFromBaseDecode(const HydroData &baseDecode);
extern HydroData *_allocateDataForObjType(int8_t idType, int8_t classType);

#endif // /ifndef HydroDatas_H
