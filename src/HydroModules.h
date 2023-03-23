/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Controller Modules
*/

#ifndef HydroModules_H
#define HydroModules_H

class HydroCalibrations;
class HydroAdditives;
class HydroObjectRegistration;
class HydroPinHandlers;

#include "Hydruino.h"
#include "HydroPins.h"

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
    Map<hkey_t, HydroCalibrationData *, HYDRO_CAL_CALIBS_MAXSIZE> _calibrationData; // Loaded user calibration data
};


// Additives Storage
// Stores custom user additive data, which is used to define feed nutrient dosing levels
// through the growing cycle.
class HydroAdditives {
public:
    // Sets custom additive data, returning success flag.
    bool setCustomAdditiveData(const HydroCustomAdditiveData *customAdditiveData);
    
    // Drops custom additive data, returning success flag.
    bool dropCustomAdditiveData(const HydroCustomAdditiveData *customAdditiveData);

    // Returns custom additive data (if any), else nullptr.
    const HydroCustomAdditiveData *getCustomAdditiveData(Hydro_ReservoirType reservoirType) const;

    // Returns if there are custom additives data stored
    inline bool hasCustomAdditives() const { return _additives.size(); }

protected:
    Map<Hydro_ReservoirType, HydroCustomAdditiveData *, Hydro_ReservoirType_CustomAdditiveCount> _additives; // Loaded custom additives data
};


// Object Registration Storage
// Stores objects in main system store, which is used for SharedPtr<> lookups as well as
// notifying appropriate modules upon entry-to/exit-from the system.
class HydroObjectRegistration {
public:
    // Adds object to system, returning success
    bool registerObject(SharedPtr<HydroObject> obj);
    // Removes object from system, returning success
    bool unregisterObject(SharedPtr<HydroObject> obj);

    // Searches for object by id key (nullptr return = no obj by that id, position index may use HYDRO_POS_SEARCH* defines)
    SharedPtr<HydroObject> objectById(HydroIdentity id) const;

    // Finds first position either open or taken, given the id type
    hposi_t firstPosition(HydroIdentity id, bool taken);
    // Finds first position taken, given the id type
    inline hposi_t firstPositionTaken(HydroIdentity id) { return firstPosition(id, true); }
    // Finds first position open, given the id type
    inline hposi_t firstPositionOpen(HydroIdentity id) { return firstPosition(id, false); }

protected:
    Map<hkey_t, SharedPtr<HydroObject>, HYDRO_SYS_OBJECTS_MAXSIZE> _objects; // Shared object collection, key'ed by HydroIdentity

    SharedPtr<HydroObject> objectById_Col(const HydroIdentity &id) const;
};


// Pin Handlers Storage
// Stores various pin-related system data on a shared pin # basis. Covers:
// - Pin locks: used for async shared resource management
// - Pin muxers: used for i/o pin multiplexing across a shared address bus
// - Pin expanders: used for i/o virtual pin expanding across an i2c interface
// - Pin OneWire: used for digital sensor pin's OneWire owner
class HydroPinHandlers {
public:
    // Attempts to get a lock on pin #, to prevent multi-device comm overlap (e.g. for OneWire comms).
    bool tryGetPinLock(pintype_t pin, millis_t wait = 150);
    // Returns a locked pin lock for the given pin. Only call if pin lock was successfully locked.
    inline void returnPinLock(pintype_t pin) { _pinLocks.erase(pin); }

    // Sets pin muxer for pin #.
    inline void setPinMuxer(pintype_t pin, SharedPtr<HydroPinMuxer> pinMuxer) { _pinMuxers[pin] = pinMuxer; }
    // Returns pin muxer for pin #.
    inline SharedPtr<HydroPinMuxer> getPinMuxer(pintype_t pin) { return _pinMuxers[pin]; }
    // Deactivates all pin muxers. All pin muxers are assumed to have a shared address bus.
    void deactivatePinMuxers();

    // Sets pin expander for index.
    inline void setPinExpander(hposi_t index, SharedPtr<HydroPinExpander> pinExpander) { _pinExpanders[index] = pinExpander; }
    // Returns expander for index.
    inline SharedPtr<HydroPinExpander> getPinExpander(hposi_t index) { return _pinExpanders[index]; }

    // OneWire instance for given pin (lazily instantiated)
    OneWire *getOneWireForPin(pintype_t pin);
    // Drops OneWire instance for given pin (if created)
    void dropOneWireForPin(pintype_t pin);

protected:
    Map<pintype_t, OneWire *, HYDRO_SYS_ONEWIRES_MAXSIZE> _pinOneWire; // Pin OneWire mapping
    Map<pintype_t, pintype_t, HYDRO_SYS_PINLOCKS_MAXSIZE> _pinLocks; // Pin locks mapping (existence = locked)
    Map<pintype_t, SharedPtr<HydroPinMuxer>, HYDRO_SYS_PINMUXERS_MAXSIZE> _pinMuxers; // Pin muxers mapping
    Map<hposi_t, SharedPtr<HydroPinExpander>, HYDRO_SYS_PINEXPANDERS_MAXSIZE> _pinExpanders; // Pin expanders mapping
};

#endif // /ifndef HydroModules_H
