
/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
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
// This is the base class for all balancer objects, which are used to modify the external
// environment via a set of actuators that can affect a measured value. Balancers allow
// for a set-point to be used to drive such tasks, with different balancers specializing
// the manner in which they operate.
class HydroBalancer : public HydroSubObject,
                      public HydroBalancerObjectInterface,
                      public HydroMeasurementUnitsInterface,
                      public HydroSensorAttachmentInterface {
public:
    const enum : signed char { LinearEdge, TimedDosing, Unknown = -1 } type; // Balancer type (custom RTTI)
    inline bool isLinearEdgeType() const { return type == LinearEdge; }
    inline bool isTimedDosingType() const { return type == TimedDosing; }
    inline bool isUnknownType() const { return type <= Unknown; }

    HydroBalancer(SharedPtr<HydroSensor> sensor,
                  float targetSetpoint,
                  float targetRange,
                  uint8_t measurementRow,
                  int type = Unknown);
    virtual ~HydroBalancer();

    virtual void update();

    virtual void setTargetSetpoint(float targetSetpoint) override;
    virtual Hydro_BalancingState getBalancingState(bool poll = false) override;

    void setIncrementActuators(const Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> &incActuators);
    inline const Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> &getIncrementActuators() { return _incActuators; }

    void setDecrementActuators(const Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> &decActuators);
    inline const Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> &getDecrementActuators() { return _decActuators; }

    inline float getTargetSetpoint() const { return _targetSetpoint; }
    inline float getTargetRange() const { return _targetRange; }

    inline void setEnabled(bool enabled) { _enabled = enabled; }
    inline bool isEnabled() const { return _enabled; }

    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t = 0) override;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t = 0) const override;

    inline uint8_t getMeasurementRow() const { return _sensor.getMeasurementRow(); }
    inline float getMeasurementConvertParam() const { return _sensor.getMeasurementConvertParam(); }

    virtual HydroSensorAttachment &getSensorAttachment() override;

    Signal<Hydro_BalancingState, HYDRO_BALANCER_SIGNAL_SLOTS> &getBalancingSignal();

protected:
    HydroSensorAttachment _sensor;                          // Sensor attachment
    Hydro_BalancingState _balancingState;                   // Balancing state (last handled)
    float _targetSetpoint;                                  // Target set-point value
    float _targetRange;                                     // Target range value
    bool _enabled;                                          // Enabled flag

    Signal<Hydro_BalancingState, HYDRO_BALANCER_SIGNAL_SLOTS> _balancingSignal; // Balancing signal

    Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> _incActuators; // Increment actuator attachments
    Vector<HydroActuatorAttachment, HYDRO_BAL_ACTUATORS_MAXSIZE> _decActuators; // Decrement actuator attachments

    void disableAllActivations();

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
                             millis_t baseDosing,
                             time_t mixTime,
                             uint8_t measurementRow = 0);
    HydroTimedDosingBalancer(SharedPtr<HydroSensor> sensor,
                             float targetSetpoint,
                             float targetRange,
                             float reservoirVolume,
                             Hydro_UnitsType volumeUnits,
                             uint8_t measurementRow = 0);

    virtual void update() override;

    inline millis_t getBaseDosing() const { return _baseDosing; }
    inline unsigned int getMixTime() const { return _mixTime; }

protected:
    time_t _mixTime;                                        // Time allowance for mixing, in seconds
    millis_t _baseDosing;                                   // Base dosing time, in milliseconds

    time_t _lastDosingTime;                                 // Date dosing was last performed (UTC)
    float _lastDosingValue;                                 // Last used dosing value
    millis_t _dosing;                                       // Dosing millis for next runs
    Hydro_BalancingState _dosingDir;                        // Dosing direction for next runs
    int8_t _dosingActIndex;                                 // Next dosing actuator to run
};

#endif // /ifndef HydroBalancers_H
