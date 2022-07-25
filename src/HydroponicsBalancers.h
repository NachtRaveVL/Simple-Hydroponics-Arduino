
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Balancers
*/

#ifndef HydroponicsBalancers_H
#define HydroponicsBalancers_H

class HydroponicsBalancer;
class HydroponicsLinearEdgeBalancer;
class HydroponicsTimedDosingBalancer;

#include "Hydroponics.h"
#include "HydroponicsObject.h"
#include "HydroponicsTriggers.h"

// TODO
class HydroponicsBalancer : public HydroponicsSubObject, public HydroponicsBalancerObjectInterface {
public:
    const enum { LinearEdge, TimedDosing, Unknown = -1 } type; // Balancer type (custom RTTI)
    inline bool isLinearEdgeType() const { return type == LinearEdge; }
    inline bool isTimedDosingType() const { return type == TimedDosing; }
    inline bool isUnknownType() const { return type <= Unknown; }

    HydroponicsBalancer(shared_ptr<HydroponicsSensor> sensor,
                        float targetSetpoint,
                        float targetRange,
                        byte measurementRow = 0,
                        int type = Unknown);
    virtual ~HydroponicsBalancer();

    virtual void setTargetSetpoint(float targetSetpoint) override;
    virtual Hydroponics_BalancerState getBalancerState() const override;
    inline bool isBalanced() const { return getBalancerState() == Hydroponics_BalancerState_Balanced; }

    virtual void update() override;
    virtual void handleLowMemory() override;

    void setTargetUnits(Hydroponics_UnitsType targetUnits);
    Hydroponics_UnitsType getTargetUnits() const;

    void setIncrementActuators(const Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type &incActuators);
    void setDecrementActuators(const Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type &decActuators);
    const Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type &getIncrementActuators();
    const Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type &getDecrementActuators();

    void setEnabled(bool enabled);
    bool isEnabled() const;

    float getTargetSetpoint() const;
    float getTargetRange() const;

    Signal<Hydroponics_BalancerState> &getBalancerSignal();

protected:
    HydroponicsMeasurementRangeTrigger *_rangeTrigger;      // Target range trigger
    float _targetSetpoint;                                  // Target setpoint value
    float _targetRange;                                     // Target range value
    bool _enabled;                                          // Enabled flag
    bool _needsTriggerUpdate;                               // Needs trigger update tracking flag
    Hydroponics_UnitsType _targetUnits;                     // Target units
    Hydroponics_BalancerState _balancerState;               // Current balancer state
    Signal<Hydroponics_BalancerState> _balancerSignal;      // Balancer signal

    Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type _incActuators; // Increment actuators
    Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type _decActuators; // Decrement actuators

    void disableIncActuators();
    void disableDecActuators();

    void attachRangeTrigger();
    void detachRangeTrigger();
    void handleRangeTrigger(Hydroponics_TriggerState triggerState);
};


// TODO
class HydroponicsLinearEdgeBalancer : public HydroponicsBalancer {
public:
    HydroponicsLinearEdgeBalancer(shared_ptr<HydroponicsSensor> sensor,
                                  float targetSetpoint,
                                  float targetRange,
                                  float edgeOffset = 0,
                                  float edgeLength = 0,
                                  byte measurementRow = 0);
    virtual ~HydroponicsLinearEdgeBalancer();

    virtual void update() override;

    float getEdgeOffset() const;
    float getEdgeLength() const;

protected:
    float _edgeOffset;                                      // Edge offset
    float _edgeLength;                                      // Length of edge (0 for non-linear)
};


// TODO
class HydroponicsTimedDosingBalancer : public HydroponicsBalancer {
public:
    HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor,
                                   float targetSetpoint,
                                   float targetRange,
                                   time_t baseDosingMillis,
                                   unsigned int mixTimeMins,
                                   byte measurementRow = 0);
    HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor,
                                   float targetSetpoint,
                                   float targetRange,
                                   float reservoirVolume,
                                   Hydroponics_UnitsType volumeUnits,
                                   byte measurementRow = 0);
    virtual ~HydroponicsTimedDosingBalancer();

    virtual void update() override;

    time_t getBaseDosingMillis() const;
    unsigned int getMixTimeMins() const;

protected:
    uint8_t _mixTimeMins;                                   // Time allowance for mixing, in minutes
    time_t _baseDosingMillis;                               // Base dosing time, in milliseconds

    time_t _lastDosingTime;                                 // Date dosing was last performed (UTC)
    float _lastDosingValue;                                 // Last used dosing value
    time_t _dosingMillis;                                   // Dosing missis for next runs
    Hydroponics_BalancerState _dosingDir;                   // Dosing direction for next runs
    int8_t _dosingActIndex;                                 // Next dosing actuator to run

    void performDosing();
    void performDosing(shared_ptr<HydroponicsActuator> actuator, time_t timeMillis);
};

#endif // /ifndef HydroponicsBalancers_H
