/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Calibrations
*/

#ifndef HydroCalibrations_H
#define HydroCalibrations_H

class HydroCalibrations;

#include "Hydruino.h"

// Calibrations Storage
// Stores user calibration data, which calibrates the various sensors output to
// an usable input value.
class HydroCalibrations {
public:
    // Adds/updates user calibration data to the store, returning success flag
    bool setUserCalibrationData(const HydroCalibrationData *calibrationData);

    // Drops/removes user calibration data from the store, returning success flag
    bool dropUserCalibrationData(const HydroCalibrationData *calibrationData);

    // Returns user calibration data instance in store
    const HydroCalibrationData *getUserCalibrationData(hkey_t key) const;

    // Returns if there are user calibrations in the store
    inline bool hasUserCalibrations() const { return _calibrationData.size(); };

protected:
    Map<hkey_t, HydroCalibrationData *, HYDRO_CAL_CALIBSTORE_MAXSIZE> _calibrationData; // Loaded user calibration data

    friend class Hydruino;
};

extern HydroCalibrations hydroCalibrations;

#endif // /ifndef HydroCalibrations_H
