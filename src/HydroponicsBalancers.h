
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

// Hydroponics Balancer Base
// This is the base class for all balancers, which are used to modify the external
// environment via increment / decrement actuators that can increase or decrease a
// measured value. Balancers allow for a setpoint to be used to drive such devices.
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

    virtual void update() override;
    virtual void handleLowMemory() override;

    virtual void setTargetSetpoint(float targetSetpoint) override;
    virtual Hydroponics_BalancerState getBalancerState() const override;

    void setTargetUnits(Hydroponics_UnitsType targetUnits) { _sensor.setMeasurementUnits(targetUnits); }
    inline Hydroponics_UnitsType getTargetUnits() const { return _sensor.getMeasurementUnits(); }

    void setIncrementActuators(const Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type &incActuators);
    void setDecrementActuators(const Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type &decActuators);
    inline const Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type &getIncrementActuators() { return _incActuators; }
    inline const Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type &getDecrementActuators() { return _decActuators; }

    inline void setEnabled(bool enabled) { _enabled = enabled; }
    inline bool isEnabled() const { return _enabled; }

    inline float getTargetSetpoint() const { return _targetSetpoint; }
    inline float getTargetRange() const { return _targetRange; }

    inline shared_ptr<HydroponicsSensor> getSensor(bool poll = false) { _sensor.updateIfNeeded(poll); return _sensor.getObject(); }
    inline byte getMeasurementRow() const { return _sensor.getMeasurementRow(); }

    Signal<Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS> &getBalancerSignal();

protected:
    HydroponicsSensorAttachment _sensor;                    // Sensor attachment
    Hydroponics_BalancerState _balancerState;               // Current balancer state
    float _targetSetpoint;                                  // Target setpoint value
    float _targetRange;                                     // Target range value
    bool _enabled;                                          // Enabled flag

    Signal<Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS> _balancerSignal; // Balancer signal

    Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_INCACTUATORS_MAXSIZE>::type _incActuators; // Increment actuators
    Vector<Pair<shared_ptr<HydroponicsActuator>, float>::type, HYDRUINO_BAL_DECACTUATORS_MAXSIZE>::type _decActuators; // Decrement actuators

    void disableAllActuators();

    void handleMeasurement(const HydroponicsMeasurement *measurement);
};


// Linear Edge Balancer
// A linear edge balancer is a balancer that provides the ability to form high and low
// areas of actuator control either by a vertical edge or a linear-gradient edge that
// interpolates along an edge's length. A vertical edge in this case can be thought of as
// an edge with zero length, which is the default. Useful for fans, heaters, and others.
class HydroponicsLinearEdgeBalancer : public HydroponicsBalancer {
public:
    HydroponicsLinearEdgeBalancer(shared_ptr<HydroponicsSensor> sensor,
                                  float targetSetpoint,
                                  float targetRange,
                                  float edgeOffset = 0,
                                  float edgeLength = 0,
                                  byte measurementRow = 0);

    virtual void update() override;

    inline float getEdgeOffset() const { return _edgeOffset; }
    inline float getEdgeLength() const { return _edgeLength; }

protected:
    float _edgeOffset;                                      // Edge offset
    float _edgeLength;                                      // Length of edge (0 for non-linear)
};


// Timed Auto-Dosing Balancer
// Auto-doser that dispenses liquids from another fluid reservoirs via pumping to
// achieve a certain environment condition, with mixing wait time between dosing.
// Dosing rates (treated as a percentage of dose-time) can be configured via Scheduler.
// After first dosing in either direction the system can become more or less aggressive
// in subsequent dispensing to help speed up the balancing process.
class HydroponicsTimedDosingBalancer : public HydroponicsBalancer {
public:
    HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor,
                                   float targetSetpoint,
                                   float targetRange,
                                   time_t baseDosingMillis,
                                   time_t mixTime,
                                   byte measurementRow = 0);
    HydroponicsTimedDosingBalancer(shared_ptr<HydroponicsSensor> sensor,
                                   float targetSetpoint,
                                   float targetRange,
                                   float reservoirVolume,
                                   Hydroponics_UnitsType volumeUnits,
                                   byte measurementRow = 0);

    virtual void update() override;

    inline time_t getBaseDosingMillis() const { return _baseDosingMillis; }
    inline unsigned int getMixTime() const { return _mixTime; }

protected:
    time_t _mixTime;                                        // Time allowance for mixing, in seconds
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
