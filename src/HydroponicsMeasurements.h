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
extern float measurementValueAt(const HydroponicsMeasurement *measurementIn, int rowIndex = 0, float binTrue = 1.0f);
// Gets the units of a measurement at a specified row (with optional binary units).
extern Hydroponics_UnitsType measurementUnitsAt(const HydroponicsMeasurement *measurementIn, int rowIndex = 0, Hydroponics_UnitsType binUnits = Hydroponics_UnitsType_Raw_0_1);


// Sensor Data Measurement Base
struct HydroponicsMeasurement {
    enum { Binary, Single, Double, Triple, Unknown = -1 } type; // Measurement type (custom RTTI)
    inline bool isBinaryType() const { return type == Binary; }
    inline bool isSingleType() const { return type == Single; }
    inline bool isDoubleType() const { return type == Double; }
    inline bool isTripleType() const { return type == Triple; }
    inline bool isUnknownType() const { return type <= Unknown; }

    time_t timestamp;                                           // Time event recorded (UTC unix time)
    uint32_t frame;                                             // Polling frame #

    HydroponicsMeasurement();
    HydroponicsMeasurement(int type, time_t timestamp = -1);
    HydroponicsMeasurement(int type, time_t timestamp, uint32_t frame);
    HydroponicsMeasurement(const HydroponicsMeasurementData *dataIn);

    void saveToData(HydroponicsMeasurementData *dataOut) const;
};

// Single Sensor Data Measurement
struct HydroponicsSingleMeasurement : public HydroponicsMeasurement {
    float value;                                                // Polled value
    Hydroponics_UnitsType units;                                // Units of value

    HydroponicsSingleMeasurement();
    HydroponicsSingleMeasurement(float value, Hydroponics_UnitsType units,
                                 time_t timestamp);
    HydroponicsSingleMeasurement(float value, Hydroponics_UnitsType units,
                                 time_t timestamp, uint32_t frame);
    HydroponicsSingleMeasurement(const HydroponicsMeasurementData *dataIn);

    void saveToData(HydroponicsMeasurementData *dataOut) const;
};

// Binary Sensor Data Measurement
struct HydroponicsBinaryMeasurement : public HydroponicsMeasurement {
    bool state;                                                 // Polled state

    HydroponicsBinaryMeasurement();
    HydroponicsBinaryMeasurement(bool state,
                                 time_t timestamp);
    HydroponicsBinaryMeasurement(bool state,
                                 time_t timestamp, uint32_t frame);
    HydroponicsBinaryMeasurement(const HydroponicsMeasurementData *dataIn);

    void saveToData(HydroponicsMeasurementData *dataOut) const;

    inline HydroponicsSingleMeasurement asSingleMeasurement(float onValue = 1.0f, Hydroponics_UnitsType valueUnits = Hydroponics_UnitsType_Raw_0_1) { return HydroponicsSingleMeasurement(state ? onValue : 0.0f, valueUnits, timestamp, frame); }
};

// Double Sensor Data Measurement
struct HydroponicsDoubleMeasurement : public HydroponicsMeasurement {
    float value[2];                                             // Polled values
    Hydroponics_UnitsType units[2];                             // Units of values

    HydroponicsDoubleMeasurement();
    HydroponicsDoubleMeasurement(float value1, Hydroponics_UnitsType units1, 
                                 float value2, Hydroponics_UnitsType units2, 
                                 time_t timestamp);
    HydroponicsDoubleMeasurement(float value1, Hydroponics_UnitsType units1, 
                                 float value2, Hydroponics_UnitsType units2, 
                                 time_t timestamp, uint32_t frame);
    HydroponicsDoubleMeasurement(const HydroponicsMeasurementData *dataIn);

    void saveToData(HydroponicsMeasurementData *dataOut) const;

    inline HydroponicsSingleMeasurement asSingleMeasurement(int row) { return HydroponicsSingleMeasurement(value[row], units[row], timestamp, frame); }
};

// Triple Sensor Data Measurement
struct HydroponicsTripleMeasurement : public HydroponicsMeasurement {
    float value[3];                                             // Polled values
    Hydroponics_UnitsType units[3];                             // Units of values

    HydroponicsTripleMeasurement();
    HydroponicsTripleMeasurement(float value1, Hydroponics_UnitsType units1, 
                                 float value2, Hydroponics_UnitsType units2, 
                                 float value3, Hydroponics_UnitsType units3,
                                 time_t timestamp);
    HydroponicsTripleMeasurement(float value1, Hydroponics_UnitsType units1, 
                                 float value2, Hydroponics_UnitsType units2, 
                                 float value3, Hydroponics_UnitsType units3,
                                 time_t timestamp, uint32_t frame);
    HydroponicsTripleMeasurement(const HydroponicsMeasurementData *dataIn);

    void saveToData(HydroponicsMeasurementData *dataOut) const;

    inline HydroponicsSingleMeasurement asSingleMeasurement(int row) { return HydroponicsSingleMeasurement(value[row], units[row], timestamp, frame); }
    inline HydroponicsDoubleMeasurement asDoubleMeasurement(int row1, int row2) { return HydroponicsDoubleMeasurement(value[row1], units[row1], value[row2], units[row2], timestamp, frame); }
};


// Combined Measurement Serialization Sub Data
struct HydroponicsMeasurementData : public HydroponicsSubData {
    union {
        struct {
            bool state;
        } binaryMeasure;
        struct {
            float value;
            Hydroponics_UnitsType units;
        } singleMeasure;
        struct {
            float value[2];
            Hydroponics_UnitsType units[2];
        } doubleMeasure;
        struct {
            float value[3];
            Hydroponics_UnitsType units[3];
        } tripleMeasure;
    } dataAs;
    time_t timestamp;

    HydroponicsMeasurementData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
    void fromJSONVariant(JsonVariantConst &variantIn);

private:
    void fromJSONArray(JsonArrayConst &arrayIn);
    void fromJSONObjectsArray(JsonArrayConst &objectsIn);
    void fromJSONObject(JsonObjectConst &objectIn, int rowIndex);
    void fromJSONValuesArray(JsonArrayConst &valuesIn);
    void fromJSONValuesString(const char *valuesIn);
    void fromJSONUnitsArray(JsonArrayConst &unitsIn);
    void fromJSONUnitsString(const char *unitsIn);
    void setValue(float value, int rowIndex);
    void setUnits(Hydroponics_UnitsType units, int rowIndex);
};

#endif // /ifndef HydroponicsMeasurements_H
