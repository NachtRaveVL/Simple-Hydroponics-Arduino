/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Calibrations Store
*/

#ifndef HydroCalibrationsStore_H
#define HydroCalibrationsStore_H

class HydroCalibrationsStore;

#include "Hydruino.h"

// Calibrations Store
// The Calibrations Store stores user calibration data, which calibrates the various
// sensors output to a usable input value.
class HydroCalibrationsStore {
public:
    // Adds/updates user calibration data to the store, returning success flag
    bool setUserCalibrationData(const HydroCalibrationData *calibrationData);

    // Drops/removes user calibration data from the store, returning success flag
    bool dropUserCalibrationData(const HydroCalibrationData *calibrationData);

    // Returns user calibration data instance in store
    const HydroCalibrationData *getUserCalibrationData(Hydro_KeyType key) const;

    // Returns if there are user calibrations in the store
    inline bool hasUserCalibrations() const { return _calibrationData.size(); };

protected:
    Map<Hydro_KeyType, HydroCalibrationData *, HYDRO_CALSTORE_CALIBS_MAXSIZE> _calibrationData; // Loaded user calibration data

    friend class Hydruino;
};

extern HydroCalibrationsStore hydroCalibrations;

#endif // /ifndef HydroCalibrationsStore_H
