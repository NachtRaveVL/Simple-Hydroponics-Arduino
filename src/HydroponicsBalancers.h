
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Balancers
*/

#ifndef HydroponicsBalancers_H
#define HydroponicsBalancers_H

class HydroponicsBalancer;

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

    HydroponicsBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, int measurementRow = 0, int type = Unknown);
    virtual ~HydroponicsBalancer();

    virtual void setTargetSetpoint(float targetSetpoint) override;
    virtual Hydroponics_BalancerState getBalancerState() const override;
    inline bool getIsBalanced() const { return getBalancerState() == Hydroponics_BalancerState_Balanced; }

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    void setTargetUnits(Hydroponics_UnitsType targetUnits);
    Hydroponics_UnitsType getTargetUnits() const;

    void setIncrementActuators(const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> &incActuators);
    void setDecrementActuators(const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> &decActuators);
    const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> &getIncrementActuators();
    const arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> &getDecrementActuators();

    void setEnabled(bool enabled);
    bool getIsEnabled() const;

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

    arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> _incActuators; // Increment actuators
    arx::map<Hydroponics_KeyType, arx::pair<shared_ptr<HydroponicsActuator>, float>, HYDRUINO_BAL_ACTUATORS_MAXSIZE> _decActuators; // Decrement actuators

    void disableIncActuators();
    void disableDecActuators();

    void attachRangeTrigger();
    void detachRangeTrigger();
    void handleRangeTrigger(Hydroponics_TriggerState triggerState);
};


// TODO
class HydroponicsLinearEdgeBalancer : public HydroponicsBalancer {
public:
    HydroponicsLinearEdgeBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, float edgeOffset = 0, float edgeLength = 0, int measurementRow = 0);
    virtual ~HydroponicsLinearEdgeBalancer();

    virtual void update() override;

    float getEdgeOffset() const;
    float getEdgeLength() const;

protected:
    float _edgeOffset;
    float _edgeLength;
};


// TODO
class HydroponicsTimedDosingBalancer : public HydroponicsBalancer {
public:
    HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, time_t baseDosingMillis, unsigned int mixTimeMins, int measurementRow = 0);
    HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor, float targetSetpoint, float targetRange, float reservoirVolume, Hydroponics_UnitsType volumeUnits, int measurementRow = 0);
    virtual ~HydroponicsTimedDosingBalancer();

    virtual void update() override;

    time_t getBaseDosingMillis() const;
    unsigned int getMixTimeMins() const;

protected:
    uint8_t _mixTimeMins;
    time_t _baseDosingMillis;

    time_t _lastDosingTime;
    float _lastDosingValue;
    time_t _lastDosingMillis;
    Hydroponics_BalancerState _lastDosingDir;
    int8_t _nextDosingActuatorIndex;

    void performDosing();
};

#endif // /ifndef HydroponicsBalancers_H
