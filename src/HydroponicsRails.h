/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Power Rails
*/

#ifndef HydroponicsRails_H
#define HydroponicsRails_H

class HydroponicsRail;
class HydroponicsSimpleRail;
class HydroponicsRegulatedRail;

struct HydroponicsRailData;
struct HydroponicsSimpleRailData;
struct HydroponicsRegulatedRailData;

#include "Hydroponics.h"

// Creates rail object from passed rail data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsRail *newRailObjectFromData(const HydroponicsRailData *dataIn);


// Hydroponics Power Rail Base
// This is the base class for all power rails, which defines how the rail is identified,
// where it lives, what's attached to it, and who can activate under it.
class HydroponicsRail : public HydroponicsObject, public HydroponicsRailObjectInterface {
public:
    const enum : signed char { Simple, Regulated, Unknown = -1 } classType; // Power rail class (custom RTTI)
    inline bool isSimpleClass() const { return classType == Simple; }
    inline bool isRegulatedClass() const { return classType == Regulated; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroponicsRail(Hydroponics_RailType railType,
                    Hydroponics_PositionIndex railIndex,
                    int classType = Unknown);
    HydroponicsRail(const HydroponicsRailData *dataIn);
    virtual ~HydroponicsRail();

    virtual void update() override;

    virtual bool addLinkage(HydroponicsObject *obj) override;
    virtual bool removeLinkage(HydroponicsObject *obj) override;

    virtual bool canActivate(HydroponicsActuator *actuator) = 0;
    virtual float getCapacity() = 0;

    virtual void setPowerUnits(Hydroponics_UnitsType powerUnits) override;
    virtual Hydroponics_UnitsType getPowerUnits() const override;

    inline Hydroponics_RailType getRailType() const { return _id.objTypeAs.railType; }
    inline Hydroponics_PositionIndex getRailIndex() const { return _id.posIndex; }
    virtual float getRailVoltage() const override;

    Signal<HydroponicsRail *, HYDRUINO_CAPACITY_STATE_SLOTS> &getCapacitySignal();

protected:
    Hydroponics_UnitsType _powerUnits;                      // Power units preferred

    Hydroponics_TriggerState _limitState;                   // Current limit state

    Signal<HydroponicsRail *, HYDRUINO_CAPACITY_STATE_SLOTS> _capacitySignal; // Capacity changed signal

    virtual HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) override;

    void handleLimit(Hydroponics_TriggerState limitState);
    friend class HydroponicsRegulatedRail;
};

// Simple Power Rail
// Basic power rail that tracks # of devices turned on, with a limit to how many
// can be on at the same time. Crude, but effective, especially when all devices
// along the rail will use about the same amount of power anyways.
class HydroponicsSimpleRail : public HydroponicsRail {
public:
    HydroponicsSimpleRail(Hydroponics_RailType railType,
                          Hydroponics_PositionIndex railIndex,
                          int maxActiveAtOnce = 2,
                          int classType = Simple);
    HydroponicsSimpleRail(const HydroponicsSimpleRailData *dataIn);

    virtual bool canActivate(HydroponicsActuator *actuator) override;
    virtual float getCapacity() override;

    inline int getActiveCount() { return _activeCount; }

protected:
    int _activeCount;                                       // Current active count
    int _maxActiveAtOnce;                                   // Max active count

    virtual void saveToData(HydroponicsData *dataOut) override;

    void handleActivation(HydroponicsActuator *actuator);
    friend class HydroponicsRail;
};

// Regulated Power Rail
// Power rail that has a max power rating and power sensor that can track power
// usage, with limit trigger for over-power state limiting actuator activation.
class HydroponicsRegulatedRail : public HydroponicsRail, public HydroponicsPowerSensorAttachmentInterface {
public:
    HydroponicsRegulatedRail(Hydroponics_RailType railType,
                             Hydroponics_PositionIndex railIndex,
                             float maxPower,
                             int classType = Regulated);
    HydroponicsRegulatedRail(const HydroponicsRegulatedRailData *dataIn);

    virtual void update() override;
    virtual void handleLowMemory() override;

    virtual bool canActivate(HydroponicsActuator *actuator) override;
    virtual float getCapacity() override;

    virtual void setPowerUnits(Hydroponics_UnitsType powerUnits) override;

    virtual HydroponicsSensorAttachment &getPowerUsage(bool poll = false) override;

    void setLimitTrigger(HydroponicsTrigger *limitTrigger);
    const HydroponicsTrigger *getLimitTrigger() const;

    template<typename T> inline void setLimitTrigger(T limitTrigger) { _limitTrigger = limitTrigger; }
    inline SharedPtr<HydroponicsTrigger> getLimitTrigger() { return _limitTrigger.getObject(); }

    inline float getMaxPower() const { return _maxPower; }

protected:
    float _maxPower;                                        // Maximum power
    HydroponicsSensorAttachment _powerUsage;                // Power usage sensor attachment
    HydroponicsTriggerAttachment _limitTrigger;             // Power limit trigger attachment

    virtual void saveToData(HydroponicsData *dataOut) override;

    void handleActivation(HydroponicsActuator *actuator);
    friend class HydroponicsRail;

    void handlePower(const HydroponicsMeasurement *measurement);
};


// Rail Serialization Data
struct HydroponicsRailData : public HydroponicsObjectData
{
    Hydroponics_UnitsType powerUnits;

    HydroponicsRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Simple Rail Serialization Data
struct HydroponicsSimpleRailData : public HydroponicsRailData
{
    int maxActiveAtOnce;

    HydroponicsSimpleRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Regulated Rail Serialization Data
struct HydroponicsRegulatedRailData : public HydroponicsRailData
{
    float maxPower;
    char powerSensor[HYDRUINO_NAME_MAXSIZE];
    HydroponicsTriggerSubData limitTrigger;

    HydroponicsRegulatedRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsRails_H
