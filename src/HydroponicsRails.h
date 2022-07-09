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
class HydroponicsRail : public HydroponicsObject, public HydroponicsRailObjectInterface, public HydroponicsActuatorAttachmentsInterface, public HydroponicsSensorAttachmentsInterface {
public:
    const enum { Simple, Regulated, Unknown = -1 } classType; // Power rail class (custom RTTI)
    inline bool isSimpleClass() const { return classType == Simple; }
    inline bool isRegulatedClass() const { return classType == Regulated; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    HydroponicsRail(Hydroponics_RailType railType,
                    Hydroponics_PositionIndex railIndex,
                    int classType = Unknown);
    HydroponicsRail(const HydroponicsRailData *dataIn);
    virtual ~HydroponicsRail();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool canActivate(HydroponicsActuator *actuator) = 0;
    virtual float getCapacity() const = 0;

    virtual bool addActuator(HydroponicsActuator *actuator) override;
    virtual bool removeActuator(HydroponicsActuator *actuator) override;
    virtual bool hasActuator(HydroponicsActuator *actuator) const override;
    virtual arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> getActuators() const override;

    virtual bool addSensor(HydroponicsSensor *sensor) override;
    virtual bool removeSensor(HydroponicsSensor *sensor) override;
    virtual bool hasSensor(HydroponicsSensor *sensor) const override;
    virtual arx::map<Hydroponics_KeyType, HydroponicsObject *, HYDRUINO_OBJ_LINKS_MAXSIZE> getSensors() const override;

    Hydroponics_RailType getRailType() const;
    Hydroponics_PositionIndex getRailIndex() const;

    Signal<HydroponicsRail *> &getCapacitySignal();

protected:
    Signal<HydroponicsRail *> _capacitySignal;              // Capacity changed signal

    virtual HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) override;
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
    virtual ~HydroponicsSimpleRail();

    virtual bool canActivate(HydroponicsActuator *actuator) override;
    virtual float getCapacity() const override;

    virtual bool addActuator(HydroponicsActuator *actuator) override;
    virtual bool removeActuator(HydroponicsActuator *actuator) override;

    int getActiveCount();

protected:
    int _activeCount;                                       // Current active count
    int _maxActiveAtOnce;                                   // Max active count

    virtual void saveToData(HydroponicsData *dataOut) override;

    void handleActivation(HydroponicsActuator *actuator);
};

// Regulated Power Rail
// Power rail that has a max power rating and power sensor that can track power
// usage, with limit trigger for over-power state limiting actuator activation.
class HydroponicsRegulatedRail : public HydroponicsRail, public HydroponicsPowerAwareInterface {
public:
    HydroponicsRegulatedRail(Hydroponics_RailType railType,
                             Hydroponics_PositionIndex railIndex,
                             float maxPower,
                             int classType = Regulated);
    HydroponicsRegulatedRail(const HydroponicsRegulatedRailData *dataIn);
    virtual ~HydroponicsRegulatedRail();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool canActivate(HydroponicsActuator *actuator) override;
    virtual float getCapacity() const override;

    void setPowerUnits(Hydroponics_UnitsType powerUnits);
    Hydroponics_UnitsType getPowerUnits() const;

    virtual void setPowerSensor(HydroponicsIdentity powerSensorId) override;
    virtual void setPowerSensor(shared_ptr<HydroponicsSensor> powerSensor) override;
    virtual shared_ptr<HydroponicsSensor> getPowerSensor() override;

    virtual void setPowerDraw(float powerDraw, Hydroponics_UnitsType powerDrawUnits = Hydroponics_UnitsType_Undefined) override;
    virtual void setPowerDraw(HydroponicsSingleMeasurement powerDraw) override;
    virtual const HydroponicsSingleMeasurement &getPowerDraw() override;

    void setLimitTrigger(HydroponicsTrigger *limitTrigger);
    const HydroponicsTrigger *getLimitTrigger() const;

    float getMaxPower() const;

protected:
    float _maxPower;                                        // Maximum power
    Hydroponics_UnitsType _powerUnits;                      // Power units preferred
    HydroponicsDLinkObject<HydroponicsSensor> _powerSensor; // Power sensor linkage
    bool _needsPowerUpdate;                                 // Needs power draw update tracking flag
    HydroponicsSingleMeasurement _powerDraw;                // Current power draw
    HydroponicsTrigger *_limitTrigger;                      // Power limit trigger (owned)

    virtual void saveToData(HydroponicsData *dataOut) override;

    void attachPowerSensor();
    void detachPowerSensor();
    void handlePowerMeasure(const HydroponicsMeasurement *measurement);
    void handleLimitTrigger(Hydroponics_TriggerState triggerState);
};


// Rail Serialization Data
struct HydroponicsRailData : public HydroponicsObjectData
{
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
    Hydroponics_UnitsType powerUnits;
    char powerSensorName[HYDRUINO_NAME_MAXSIZE];
    HydroponicsTriggerSubData limitTrigger;

    HydroponicsRegulatedRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsRails_H
