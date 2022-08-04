/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Calibrations Store
*/

#ifndef HydroponicsCalibrationsStore_H
#define HydroponicsCalibrationsStore_H

class HydroponicsCalibrationsStore;

#include "Hydroponics.h"

// Hydroponics Calibrations Store
// TODO
class HydroponicsCalibrationsStore {
public:
    // Returns the singleton instance of the store
    static HydroponicsCalibrationsStore *getInstance();

    // Returns user calibration data instance in store
    const HydroponicsCalibrationData *getUserCalibrationData(Hydroponics_KeyType key) const;

    // Adds/updates user calibration data to the store, returning success flag
    bool setUserCalibrationData(const HydroponicsCalibrationData *calibrationData);

    // Drops/removes user calibration data from the store, returning success flag
    bool dropUserCalibrationData(const HydroponicsCalibrationData *calibrationData);

    // Returns if there are user calibrations in the store
    bool hasUserCalibrations() const;

    // Signal when user calibrations are added/updated in the store
    Signal<Hydroponics_KeyType> &getUserCalibrationSignal();

protected:
    Map<Hydroponics_KeyType, HydroponicsCalibrationData *, HYDRUINO_CALSTORE_CALIBS_MAXSIZE> _calibrationData; // Loaded user calibration data
    Signal<Hydroponics_KeyType> _calibrationSignal;             // User calibration data updated signal    

private:
    static HydroponicsCalibrationsStore *_instance;             // Shared instance
    HydroponicsCalibrationsStore() = default;                   // Private constructor to force singleton
    friend class Hydroponics;
};

inline HydroponicsCalibrationsStore *getCalibrationsStoreInstance() { return HydroponicsCalibrationsStore::getInstance(); }

#endif // /ifndef HydroponicsCalibrationsStore_H
