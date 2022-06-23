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

#include "Hydroponics.h"

// Sensor Data Measurement Base
struct HydroponicsMeasurement {
    enum { Binary, Single, Double, Triple, Unknown = -1 } type; // Measurement type (custom RTTI)
    inline bool isBinaryType() const { return type == Binary; }
    inline bool isSingleType() const { return type == Single; }
    inline bool isDoubleType() const { return type == Double; }
    inline bool isTripleType() const { return type == Triple; }
    inline bool isUnknownType() const { return type >= Unknown; }

    HydroponicsMeasurement();
    HydroponicsMeasurement(int type, time_t timestamp = -1);
    HydroponicsMeasurement(int type, time_t timestamp, uint32_t frame);

    time_t timestamp;                                           // Time event recorded (UTC unix time)
    uint32_t frame;                                             // Polling frame #
};

// Single Sensor Data Measurement
struct HydroponicsSingleMeasurement : public HydroponicsMeasurement {
    HydroponicsSingleMeasurement();
    HydroponicsSingleMeasurement(float value, Hydroponics_UnitsType units,
                                 time_t timestamp);
    HydroponicsSingleMeasurement(float value, Hydroponics_UnitsType units,
                                 time_t timestamp, uint32_t frame);

    float value;                                                // Polled value
    Hydroponics_UnitsType units;                                // Units of value
};

// Binary Sensor Data Measurement
struct HydroponicsBinaryMeasurement : public HydroponicsMeasurement {
    HydroponicsBinaryMeasurement();
    HydroponicsBinaryMeasurement(bool state,
                                 time_t timestamp);
    HydroponicsBinaryMeasurement(bool state,
                                 time_t timestamp, uint32_t frame);

    bool state;                                                 // Polled state

    inline HydroponicsSingleMeasurement asSingleMeasurement(int row) { return HydroponicsSingleMeasurement(state ? 1.0f : 0.0f, Hydroponics_UnitsType_Raw_0_1, timestamp, frame); }
};

// Double Sensor Data Measurement
struct HydroponicsDoubleMeasurement : public HydroponicsMeasurement {
    HydroponicsDoubleMeasurement();
    HydroponicsDoubleMeasurement(float value1, Hydroponics_UnitsType units1, 
                                 float value2, Hydroponics_UnitsType units2, 
                                 time_t timestamp);
    HydroponicsDoubleMeasurement(float value1, Hydroponics_UnitsType units1, 
                                 float value2, Hydroponics_UnitsType units2, 
                                 time_t timestamp, uint32_t frame);

    float value[2];                                             // Polled values
    Hydroponics_UnitsType units[2];                             // Units of values

    inline HydroponicsSingleMeasurement asSingleMeasurement(int row) { return HydroponicsSingleMeasurement(value[row], units[row], timestamp, frame); }
};

// Triple Sensor Data Measurement
struct HydroponicsTripleMeasurement : public HydroponicsMeasurement {
    HydroponicsTripleMeasurement();
    HydroponicsTripleMeasurement(float value1, Hydroponics_UnitsType units1, 
                                 float value2, Hydroponics_UnitsType units2, 
                                 float value3, Hydroponics_UnitsType units3,
                                 time_t timestamp);
    HydroponicsTripleMeasurement(float value1, Hydroponics_UnitsType units1, 
                                 float value2, Hydroponics_UnitsType units2, 
                                 float value3, Hydroponics_UnitsType units3,
                                 time_t timestamp, uint32_t frame);

    float value[3];                                             // Polled values
    Hydroponics_UnitsType units[3];                             // Units of values

    inline HydroponicsSingleMeasurement asSingleMeasurement(int row) { return HydroponicsSingleMeasurement(value[row], units[row], timestamp, frame); }
    inline HydroponicsDoubleMeasurement asDoubleMeasurement(int row1, int row2) { return HydroponicsDoubleMeasurement(value[row1], units[row1], value[row2], units[row2], timestamp, frame); }
};

#endif // /ifndef HydroponicsMeasurements_H
