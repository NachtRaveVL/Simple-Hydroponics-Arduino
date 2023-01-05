/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydroponics Calibrations Store
*/

#ifndef HydroponicsCalibrationsStore_H
#define HydroponicsCalibrationsStore_H

class HydroponicsCalibrationsStore;

#include "Hydroponics.h"

// Hydroponics Calibrations Store
// The Calibrations Store stores user calibration data, which calibrates the various
// sensors output to a usable input value.
class HydroponicsCalibrationsStore {
public:
    // Adds/updates user calibration data to the store, returning success flag
    bool setUserCalibrationData(const HydroponicsCalibrationData *calibrationData);

    // Drops/removes user calibration data from the store, returning success flag
    bool dropUserCalibrationData(const HydroponicsCalibrationData *calibrationData);

    // Returns user calibration data instance in store
    const HydroponicsCalibrationData *getUserCalibrationData(Hydroponics_KeyType key) const;

    // Returns if there are user calibrations in the store
    inline bool hasUserCalibrations() const { return _calibrationData.size(); };

protected:
    Map<Hydroponics_KeyType, HydroponicsCalibrationData *, HYDRUINO_CALSTORE_CALIBS_MAXSIZE> _calibrationData; // Loaded user calibration data

    friend class Hydroponics;
};

extern HydroponicsCalibrationsStore hydroCalibrations;

#endif // /ifndef HydroponicsCalibrationsStore_H
