/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensor Measurements
*/

#ifndef HydroponicsMeasurements_H
#define HydroponicsMeasurements_H

struct HydroponicsMeasurement;
struct HydroponicsSingleMeasurement;
struct HydroponicsBinaryMeasurement;
struct HydroponicsDoubleMeasurement;
struct HydroponicsTripleMeasurement;

struct HydroponicsMeasurementData;

#include "Hydroponics.h"
#include "HydroponicsData.h"

// Creates measurement object from passed trigger sub data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsMeasurement *newMeasurementObjectFromSubData(const HydroponicsMeasurementData *dataIn);

// Gets the value of a measurement at a specified row (with optional binary true value).
extern float getMeasurementValue(const HydroponicsMeasurement *measurement, uint8_t measurementRow = 0, float binTrue = 1.0f);
// Gets the units of a measurement at a specified row (with optional binary units).
extern Hydroponics_UnitsType getMeasurementUnits(const HydroponicsMeasurement *measurement, uint8_t measurementRow = 0, Hydroponics_UnitsType binUnits = Hydroponics_UnitsType_Raw_0_1);
// Gets the number of rows of data that a measurement holds.
extern uint8_t getMeasurementRowCount(const HydroponicsMeasurement *measurement);
// Gets the single measurement of a measurement (with optional binary true value / units).
extern HydroponicsSingleMeasurement getAsSingleMeasurement(const HydroponicsMeasurement *measurement, uint8_t measurementRow = 0, float binTrue = 1.0f, Hydroponics_UnitsType binUnits = Hydroponics_UnitsType_Raw_0_1);

// Sensor Data Measurement Base
struct HydroponicsMeasurement {
    enum : signed char { Binary, Single, Double, Triple, Unknown = -1 } type; // Measurement type (custom RTTI)
    inline bool isBinaryType() const { return type == Binary; }
    inline bool isSingleType() const { return type == Single; }
    inline bool isDoubleType() const { return type == Double; }
    inline bool isTripleType() const { return type == Triple; }
    inline bool isUnknownType() const { return type <= Unknown; }

    time_t timestamp;                                           // Time event recorded (UTC)
    uint16_t frame;                                             // Polling frame #

    HydroponicsMeasurement();
    HydroponicsMeasurement(int type,
                           time_t timestamp = 0);
    HydroponicsMeasurement(int type,
                           time_t timestamp,
                           uint16_t frame);
    HydroponicsMeasurement(const HydroponicsMeasurementData *dataIn);

    void saveToData(HydroponicsMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline void updateTimestamp() { timestamp = unixNow(); }
    void updateFrame(unsigned int minFrame = 0);
    inline void setMinFrame(unsigned int minFrame = 0) { frame = max(minFrame, frame); }
};

// Single Value Sensor Data Measurement
struct HydroponicsSingleMeasurement : public HydroponicsMeasurement {
    float value;                                                // Polled value
    Hydroponics_UnitsType units;                                // Units of value

    HydroponicsSingleMeasurement();
    HydroponicsSingleMeasurement(float value,
                                 Hydroponics_UnitsType units,
                                 time_t timestamp = unixNow());
    HydroponicsSingleMeasurement(float value,
                                 Hydroponics_UnitsType units,
                                 time_t timestamp,
                                 uint16_t frame);
    HydroponicsSingleMeasurement(const HydroponicsMeasurementData *dataIn);

    void saveToData(HydroponicsMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;
};

// Binary Value Sensor Data Measurement
struct HydroponicsBinaryMeasurement : public HydroponicsMeasurement {
    bool state;                                                 // Polled state

    HydroponicsBinaryMeasurement();
    HydroponicsBinaryMeasurement(bool state,
                                 time_t timestamp = unixNow());
    HydroponicsBinaryMeasurement(bool state,
                                 time_t timestamp,
                                 uint16_t frame);
    HydroponicsBinaryMeasurement(const HydroponicsMeasurementData *dataIn);

    void saveToData(HydroponicsMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline HydroponicsSingleMeasurement getAsSingleMeasurement(float binTrue = 1.0f, Hydroponics_UnitsType binUnits = Hydroponics_UnitsType_Raw_0_1) { return HydroponicsSingleMeasurement(state ? binTrue : 0.0f, binUnits, timestamp, frame); }
};

// Double Value Sensor Data Measurement
struct HydroponicsDoubleMeasurement : public HydroponicsMeasurement {
    float value[2];                                             // Polled values
    Hydroponics_UnitsType units[2];                             // Units of values

    HydroponicsDoubleMeasurement();
    HydroponicsDoubleMeasurement(float value1,
                                 Hydroponics_UnitsType units1, 
                                 float value2,
                                 Hydroponics_UnitsType units2, 
                                 time_t timestamp = unixNow());
    HydroponicsDoubleMeasurement(float value1,
                                 Hydroponics_UnitsType units1, 
                                 float value2,
                                 Hydroponics_UnitsType units2, 
                                 time_t timestamp,
                                 uint16_t frame);
    HydroponicsDoubleMeasurement(const HydroponicsMeasurementData *dataIn);

    void saveToData(HydroponicsMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline HydroponicsSingleMeasurement getAsSingleMeasurement(uint8_t measurementRow) { return HydroponicsSingleMeasurement(value[measurementRow], units[measurementRow], timestamp, frame); }
};

// Triple Value Sensor Data Measurement
struct HydroponicsTripleMeasurement : public HydroponicsMeasurement {
    float value[3];                                             // Polled values
    Hydroponics_UnitsType units[3];                             // Units of values

    HydroponicsTripleMeasurement();
    HydroponicsTripleMeasurement(float value1,
                                 Hydroponics_UnitsType units1, 
                                 float value2,
                                 Hydroponics_UnitsType units2, 
                                 float value3,
                                 Hydroponics_UnitsType units3,
                                 time_t timestamp = unixNow());
    HydroponicsTripleMeasurement(float value1,
                                 Hydroponics_UnitsType units1, 
                                 float value2,
                                 Hydroponics_UnitsType units2, 
                                 float value3,
                                 Hydroponics_UnitsType units3,
                                 time_t timestamp,
                                 uint16_t frame);
    HydroponicsTripleMeasurement(const HydroponicsMeasurementData *dataIn);

    void saveToData(HydroponicsMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline HydroponicsSingleMeasurement getAsSingleMeasurement(uint8_t measurementRow) { return HydroponicsSingleMeasurement(value[measurementRow], units[measurementRow], timestamp, frame); }
    inline HydroponicsDoubleMeasurement getAsDoubleMeasurement(uint8_t measurementRow1, uint8_t measurementRow2) { return HydroponicsDoubleMeasurement(value[measurementRow1], units[measurementRow1], value[measurementRow2], units[measurementRow2], timestamp, frame); }
};


// Combined Measurement Serialization Sub Data
struct HydroponicsMeasurementData : public HydroponicsSubData {
    uint8_t measurementRow;                                 // Source measurement row index that data is from
    float value;                                            // Value
    Hydroponics_UnitsType units;                            // Units of value
    time_t timestamp;                                       // Timestamp

    HydroponicsMeasurementData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
    void fromJSONVariant(JsonVariantConst &variantIn);
};

#endif // /ifndef HydroponicsMeasurements_H
