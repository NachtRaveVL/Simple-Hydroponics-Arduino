
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Balancers
*/

#ifndef HydroBalancers_H
#define HydroBalancers_H

class HydroBalancer;
class HydroLinearEdgeBalancer;
class HydroTimedDosingBalancer;

#include "Hydruino.h"
#include "HydroObject.h"
#include "HydroTriggers.h"

// Balancer Base
// This is the base class for all balancers, which are used to modify the external
// environment via increment / decrement actuators that can increase or decrease a
// measured value. Balancers allow for a setpoint to be used to drive such devices.
class HydroBalancer : public HydroSubObject, public HydroBalancerObjectInterface {
public:
    const enum : signed char { LinearEdge, TimedDosing, Unknown = -1 } type; // Balancer type (custom RTTI)
    inline bool isLinearEdgeType() const { return type == LinearEdge; }
    inline bool isTimedDosingType() const { return type == TimedDosing; }
    inline bool isUnknownType() const { return type <= Unknown; }

    HydroBalancer(SharedPtr<HydroSensor> sensor,
                  float targetSetpoint,
                  float targetRange,
                  uint8_t measurementRow = 0,
                  int type = Unknown);
    virtual ~HydroBalancer();

    virtual void update();

    virtual void setTargetSetpoint(float targetSetpoint) override;
    virtual Hydro_BalancerState getBalancerState() const override;

    void setTargetUnits(Hydro_UnitsType targetUnits) { _sensor.setMeasurementUnits(targetUnits); }
    inline Hydro_UnitsType getTargetUnits() const { return _sensor.getMeasurementUnits(); }

    void setIncrementActuators(const Vector<Pair<SharedPtr<HydroActuator>, float>, HYDRO_BAL_INCACTUATORS_MAXSIZE> &incActuators);
    void setDecrementActuators(const Vector<Pair<SharedPtr<HydroActuator>, float>, HYDRO_BAL_DECACTUATORS_MAXSIZE> &decActuators);
    inline const Vector<Pair<SharedPtr<HydroActuator>, float>, HYDRO_BAL_INCACTUATORS_MAXSIZE> &getIncrementActuators() { return _incActuators; }
    inline const Vector<Pair<SharedPtr<HydroActuator>, float>, HYDRO_BAL_DECACTUATORS_MAXSIZE> &getDecrementActuators() { return _decActuators; }

    inline void setEnabled(bool enabled) { _enabled = enabled; }
    inline bool isEnabled() const { return _enabled; }

    inline float getTargetSetpoint() const { return _targetSetpoint; }
    inline float getTargetRange() const { return _targetRange; }

    inline SharedPtr<HydroSensor> getSensor(bool poll = false) { _sensor.updateIfNeeded(poll); return _sensor.getObject(); }
    inline uint8_t getMeasurementRow() const { return _sensor.getMeasurementRow(); }

    Signal<Hydro_BalancerState, HYDRO_BALANCER_STATE_SLOTS> &getBalancerSignal();

protected:
    HydroSensorAttachment _sensor;                          // Sensor attachment
    Hydro_BalancerState _balancerState;                     // Current balancer state
    float _targetSetpoint;                                  // Target setpoint value
    float _targetRange;                                     // Target range value
    bool _enabled;                                          // Enabled flag

    Signal<Hydro_BalancerState, HYDRO_BALANCER_STATE_SLOTS> _balancerSignal; // Balancer signal

    Vector<Pair<SharedPtr<HydroActuator>, float>, HYDRO_BAL_INCACTUATORS_MAXSIZE> _incActuators; // Increment actuators
    Vector<Pair<SharedPtr<HydroActuator>, float>, HYDRO_BAL_DECACTUATORS_MAXSIZE> _decActuators; // Decrement actuators

    void disableAllActuators();

    void handleMeasurement(const HydroMeasurement *measurement);
};


// Linear Edge Balancer
// A linear edge balancer is a balancer that provides the ability to form high and low
// areas of actuator control either by a vertical edge or a linear-gradient edge that
// interpolates along an edge's length. A vertical edge in this case can be thought of as
// an edge with zero length, which is the default. Useful for fans, heaters, and others.
class HydroLinearEdgeBalancer : public HydroBalancer {
public:
    HydroLinearEdgeBalancer(SharedPtr<HydroSensor> sensor,
                            float targetSetpoint,
                            float targetRange,
                            float edgeOffset = 0,
                            float edgeLength = 0,
                            uint8_t measurementRow = 0);

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
class HydroTimedDosingBalancer : public HydroBalancer {
public:
    HydroTimedDosingBalancer(SharedPtr<HydroSensor> sensor,
                             float targetSetpoint,
                             float targetRange,
                             time_t baseDosingMillis,
                             time_t mixTime,
                             uint8_t measurementRow = 0);
    HydroTimedDosingBalancer(SharedPtr<HydroSensor> sensor,
                             float targetSetpoint,
                             float targetRange,
                             float reservoirVolume,
                             Hydro_UnitsType volumeUnits,
                             uint8_t measurementRow = 0);

    virtual void update() override;

    inline time_t getBaseDosingMillis() const { return _baseDosingMillis; }
    inline unsigned int getMixTime() const { return _mixTime; }

protected:
    time_t _mixTime;                                        // Time allowance for mixing, in seconds
    time_t _baseDosingMillis;                               // Base dosing time, in milliseconds

    time_t _lastDosingTime;                                 // Date dosing was last performed (UTC)
    float _lastDosingValue;                                 // Last used dosing value
    time_t _dosingMillis;                                   // Dosing millis for next runs
    Hydro_BalancerState _dosingDir;                         // Dosing direction for next runs
    int8_t _dosingActIndex;                                 // Next dosing actuator to run

    void performDosing();
    void performDosing(SharedPtr<HydroActuator> &actuator, time_t timeMillis);
};

#endif // /ifndef HydroBalancers_H
