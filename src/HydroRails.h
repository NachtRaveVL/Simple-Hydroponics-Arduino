/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Power Rails
*/

#ifndef HydroRails_H
#define HydroRails_H

class HydroRail;
class HydroSimpleRail;
class HydroRegulatedRail;

struct HydroRailData;
struct HydroSimpleRailData;
struct HydroRegulatedRailData;

#include "Hydruino.h"

// Creates rail object from passed rail data (return ownership transfer - user code *must* delete returned object)
extern HydroRail *newRailObjectFromData(const HydroRailData *dataIn);


// Power Rail Base
// This is the base class for all power rails, which defines how the rail is identified,
// where it lives, what's attached to it, and who can activate under it.
class HydroRail : public HydroObject, public HydroRailObjectInterface, public HydroPowerUnitsInterface {
public:
    const enum : signed char { Simple, Regulated, Unknown = -1 } classType; // Power rail class (custom RTTI)
    inline bool isSimpleClass() const { return classType == Simple; }
    inline bool isRegulatedClass() const { return classType == Regulated; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroRail(Hydro_RailType railType,
              hposi_t railIndex,
              int classType = Unknown);
    HydroRail(const HydroRailData *dataIn);
    virtual ~HydroRail();

    virtual void update() override;

    virtual bool addLinkage(HydroObject *obj) override;
    virtual bool removeLinkage(HydroObject *obj) override;

    virtual bool canActivate(HydroActuator *actuator) = 0;
    virtual float getCapacity(bool poll = false) = 0;

    inline Hydro_RailType getRailType() const { return _id.objTypeAs.railType; }
    inline hposi_t getRailIndex() const { return _id.posIndex; }
    inline float getRailVoltage() const { return getRailVoltageFromType(getRailType()); }

    Signal<HydroRail *, HYDRO_RAIL_SIGNAL_SLOTS> &getCapacitySignal();

protected:
    Hydro_TriggerState _limitState;                         // Current limit state

    Signal<HydroRail *, HYDRO_RAIL_SIGNAL_SLOTS> _capacitySignal; // Capacity changed signal

    virtual HydroData *allocateData() const override;
    virtual void saveToData(HydroData *dataOut) override;

    void handleLimit(Hydro_TriggerState limitState);
    friend class HydroRegulatedRail;
};

// Simple Power Rail
// Basic power rail that tracks # of devices turned on, with a limit to how many
// can be on at the same time. Crude, but effective, especially when all devices
// along the rail will use about the same amount of power anyways.
class HydroSimpleRail : public HydroRail {
public:
    HydroSimpleRail(Hydro_RailType railType,
                    hposi_t railIndex,
                    int maxActiveAtOnce = 2,
                    int classType = Simple);
    HydroSimpleRail(const HydroSimpleRailData *dataIn);

    virtual bool canActivate(HydroActuator *actuator) override;
    virtual float getCapacity(bool poll = false) override;

    virtual void setPowerUnits(Hydro_UnitsType powerUnits) override;

    inline int getActiveCount() { return _activeCount; }

protected:
    int _activeCount;                                       // Current active count
    int _maxActiveAtOnce;                                   // Max active count

    virtual void saveToData(HydroData *dataOut) override;

    void handleActivation(HydroActuator *actuator);
    friend class HydroRail;
};

// Regulated Power Rail
// Power rail that has a max power rating and power sensor that can track power
// usage, with limit trigger for over-power state limiting actuator activation.
class HydroRegulatedRail : public HydroRail,  public HydroPowerUsageSensorAttachmentInterface, public HydroLimitTriggerAttachmentInterface {
public:
    HydroRegulatedRail(Hydro_RailType railType,
                       hposi_t railIndex,
                       float maxPower,
                       int classType = Regulated);
    HydroRegulatedRail(const HydroRegulatedRailData *dataIn);

    virtual void update() override;
    virtual void handleLowMemory() override;

    virtual bool canActivate(HydroActuator *actuator) override;
    virtual float getCapacity(bool poll = false) override;

    virtual void setPowerUnits(Hydro_UnitsType powerUnits) override;

    virtual HydroSensorAttachment &getPowerUsage() override;

    virtual HydroTriggerAttachment &getLimit() override;

    inline float getMaxPower() const { return _maxPower; }

protected:
    float _maxPower;                                        // Maximum power
    HydroSensorAttachment _powerUsage;                      // Power usage sensor attachment
    HydroTriggerAttachment _limitTrigger;                   // Power limit trigger attachment

    virtual void saveToData(HydroData *dataOut) override;

    void handleActivation(HydroActuator *actuator);
    friend class HydroRail;

    void handlePower(const HydroMeasurement *measurement);
};


// Rail Serialization Data
struct HydroRailData : public HydroObjectData
{
    Hydro_UnitsType powerUnits;

    HydroRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Simple Rail Serialization Data
struct HydroSimpleRailData : public HydroRailData
{
    int maxActiveAtOnce;

    HydroSimpleRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Regulated Rail Serialization Data
struct HydroRegulatedRailData : public HydroRailData
{
    float maxPower;
    char powerSensor[HYDRO_NAME_MAXSIZE];
    HydroTriggerSubData limitTrigger;

    HydroRegulatedRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroRails_H
