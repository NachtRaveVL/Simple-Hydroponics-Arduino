/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Sensor Measurements
*/

#ifndef HydroMeasurements_H
#define HydroMeasurements_H

struct HydroMeasurement;
struct HydroSingleMeasurement;
struct HydroBinaryMeasurement;
struct HydroDoubleMeasurement;
struct HydroTripleMeasurement;

struct HydroMeasurementData;

#include "Hydruino.h"
#include "HydroData.h"

// Creates measurement object from passed trigger sub data (return ownership transfer - user code *must* delete returned object)
extern HydroMeasurement *newMeasurementObjectFromSubData(const HydroMeasurementData *dataIn);

// Gets the value of a measurement at a specified row (with optional binary true value).
extern float getMeasurementValue(const HydroMeasurement *measurement, uint8_t measurementRow = 0, float binTrue = 1.0f);
// Gets the units of a measurement at a specified row (with optional binary units).
extern Hydro_UnitsType getMeasurementUnits(const HydroMeasurement *measurement, uint8_t measurementRow = 0, Hydro_UnitsType binUnits = Hydro_UnitsType_Raw_0_1);
// Gets the number of rows of data that a measurement holds.
extern uint8_t getMeasurementRowCount(const HydroMeasurement *measurement);
// Gets the single measurement of a measurement (with optional binary true value / units).
extern HydroSingleMeasurement getAsSingleMeasurement(const HydroMeasurement *measurement, uint8_t measurementRow = 0, float binTrue = 1.0f, Hydro_UnitsType binUnits = Hydro_UnitsType_Raw_0_1);

// Sensor Data Measurement Base
struct HydroMeasurement {
    enum : signed char { Binary, Single, Double, Triple, Unknown = -1 } type; // Measurement type (custom RTTI)
    inline bool isBinaryType() const { return type == Binary; }
    inline bool isSingleType() const { return type == Single; }
    inline bool isDoubleType() const { return type == Double; }
    inline bool isTripleType() const { return type == Triple; }
    inline bool isUnknownType() const { return type <= Unknown; }

    time_t timestamp;                                       // Time event recorded (UTC)
    uint16_t frame;                                         // Polling frame #

    HydroMeasurement();
    HydroMeasurement(int type,
                     time_t timestamp = 0);
    HydroMeasurement(int type,
                     time_t timestamp,
                     uint16_t frame);
    HydroMeasurement(const HydroMeasurementData *dataIn);

    void saveToData(HydroMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline void updateTimestamp() { timestamp = unixNow(); }
    void updateFrame(unsigned int minFrame = 0);
    inline void setMinFrame(unsigned int minFrame = 0) { frame = max(minFrame, frame); }
};

// Single Value Sensor Data Measurement
struct HydroSingleMeasurement : public HydroMeasurement {
    float value;                                            // Polled value
    Hydro_UnitsType units;                                  // Units of value

    HydroSingleMeasurement();
    HydroSingleMeasurement(float value,
                           Hydro_UnitsType units,
                           time_t timestamp = unixNow());
    HydroSingleMeasurement(float value,
                           Hydro_UnitsType units,
                           time_t timestamp,
                           uint16_t frame);
    HydroSingleMeasurement(const HydroMeasurementData *dataIn);

    void saveToData(HydroMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;
};

// Binary Value Sensor Data Measurement
struct HydroBinaryMeasurement : public HydroMeasurement {
    bool state;                                             // Polled state

    HydroBinaryMeasurement();
    HydroBinaryMeasurement(bool state,
                           time_t timestamp = unixNow());
    HydroBinaryMeasurement(bool state,
                           time_t timestamp,
                           uint16_t frame);
    HydroBinaryMeasurement(const HydroMeasurementData *dataIn);

    void saveToData(HydroMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline HydroSingleMeasurement getAsSingleMeasurement(float binTrue = 1.0f, Hydro_UnitsType binUnits = Hydro_UnitsType_Raw_0_1) { return HydroSingleMeasurement(state ? binTrue : 0.0f, binUnits, timestamp, frame); }
};

// Double Value Sensor Data Measurement
struct HydroDoubleMeasurement : public HydroMeasurement {
    float value[2];                                         // Polled values
    Hydro_UnitsType units[2];                               // Units of values

    HydroDoubleMeasurement();
    HydroDoubleMeasurement(float value1,
                           Hydro_UnitsType units1, 
                           float value2,
                           Hydro_UnitsType units2, 
                           time_t timestamp = unixNow());
    HydroDoubleMeasurement(float value1,
                           Hydro_UnitsType units1, 
                           float value2,
                           Hydro_UnitsType units2, 
                           time_t timestamp,
                           uint16_t frame);
    HydroDoubleMeasurement(const HydroMeasurementData *dataIn);

    void saveToData(HydroMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline HydroSingleMeasurement getAsSingleMeasurement(uint8_t measurementRow) { return HydroSingleMeasurement(value[measurementRow], units[measurementRow], timestamp, frame); }
};

// Triple Value Sensor Data Measurement
struct HydroTripleMeasurement : public HydroMeasurement {
    float value[3];                                         // Polled values
    Hydro_UnitsType units[3];                               // Units of values

    HydroTripleMeasurement();
    HydroTripleMeasurement(float value1,
                           Hydro_UnitsType units1, 
                           float value2,
                           Hydro_UnitsType units2, 
                           float value3,
                           Hydro_UnitsType units3,
                           time_t timestamp = unixNow());
    HydroTripleMeasurement(float value1,
                           Hydro_UnitsType units1, 
                           float value2,
                           Hydro_UnitsType units2, 
                           float value3,
                           Hydro_UnitsType units3,
                           time_t timestamp,
                           uint16_t frame);
    HydroTripleMeasurement(const HydroMeasurementData *dataIn);

    void saveToData(HydroMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline HydroSingleMeasurement getAsSingleMeasurement(uint8_t measurementRow) { return HydroSingleMeasurement(value[measurementRow], units[measurementRow], timestamp, frame); }
    inline HydroDoubleMeasurement getAsDoubleMeasurement(uint8_t measurementRow1, uint8_t measurementRow2) { return HydroDoubleMeasurement(value[measurementRow1], units[measurementRow1], value[measurementRow2], units[measurementRow2], timestamp, frame); }
};


// Combined Measurement Serialization Sub Data
struct HydroMeasurementData : public HydroSubData {
    uint8_t measurementRow;                                 // Source measurement row index that data is from
    float value;                                            // Value
    Hydro_UnitsType units;                                  // Units of value
    time_t timestamp;                                       // Timestamp

    HydroMeasurementData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
    void fromJSONVariant(JsonVariantConst &variantIn);
};

#endif // /ifndef HydroMeasurements_H
