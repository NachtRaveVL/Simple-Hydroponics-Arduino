/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Triggers
*/

#ifndef HydroponicsTriggers_H
#define HydroponicsTriggers_H

class HydroponicsTrigger;
class HydroponicsMeasurementValueTrigger;
class HydroponicsMeasurementRangeTrigger;
class HydroponicsEstimatedVolumeTrigger;

#include "Hydroponics.h"

// Hydroponics Trigger Base
// This is the base class for all triggers, which are used to alert the system
// to some change in a tracked property.
class HydroponicsTrigger
{
public:
    const enum { MeasureValue, MeasureRange, EstimatedVol, Unknown = -1 } type;     // Trigger type (custom RTTI)
    inline bool isMeasureValueType() { return type == MeasureValue; }
    inline bool isMeasureRangeType() { return type == MeasureRange; }
    inline bool isEstimatedVolType() { return type == EstimatedVol; }
    inline bool isUnknownType() { return type == Unknown; }

    HydroponicsTrigger(int type = Unknown);
    virtual ~HydroponicsTrigger();

    virtual void update();

    virtual void attach() = 0;
    virtual void detach() = 0;

    Hydroponics_TriggerState getTriggerState() const;
    Signal<Hydroponics_TriggerState> &getTriggerSignal();

protected:
    Hydroponics_TriggerState _triggerState;                 // Trigger state
    Signal<Hydroponics_TriggerState> _triggerSignal;        // Trigger signal
    bool _attached;                                         // Attached flag
};


// Sensor Data Measurement Value Trigger
// This trigger simply checks a measured value against a set tolerance value and is
// useful for simple comparisons that control triggering. Initializes as disabled
// until updated with first measurement, and with undefined units that compares
// directly to measured units, otherwise units can be explicitly set.
class HydroponicsMeasurementValueTrigger : public HydroponicsTrigger {
public:
    HydroponicsMeasurementValueTrigger(shared_ptr<HydroponicsSensor> sensor, float tolerance, bool activeBelow, int measurementRow = 0);
    ~HydroponicsMeasurementValueTrigger();

    void attach() override;
    void detach() override;

    void setToleranceUnits(Hydroponics_UnitsType units);
    Hydroponics_UnitsType getToleranceUnits() const;

    shared_ptr<HydroponicsSensor> getSensor() const;
    float getTolerance() const;
    bool getActiveBelow() const;

protected:
    shared_ptr<HydroponicsSensor> _sensor;                  // Attached sensor
    float _tolerance;                                       // Tolerance limit
    Hydroponics_UnitsType _toleranceUnits;                  // Tolerance units (if set, else undef)
    bool _activeBelow;                                      // Active below flag
    int8_t _measurementRow;                                 // Measurement data row to check against

    void handleSensorMeasure(HydroponicsMeasurement *measurement);
};


// Sensor Data Measurement Range Trigger
// This trigger checks a measured value against a set tolerance range and is
// useful for ranged measurements that need to stay inside of (or outside of) a
// known range before triggering. Initializes as disabled until updated with
// first measurement, and with undefined units that compares directly to measured
// units, otherwise units can be explicitly set.
class HydroponicsMeasurementRangeTrigger : public HydroponicsTrigger {
public:
    HydroponicsMeasurementRangeTrigger(shared_ptr<HydroponicsSensor> sensor, float toleranceLow, float toleranceHigh, bool triggerOnOutside = true, int measurementRow = 0);
    ~HydroponicsMeasurementRangeTrigger();

    void attach() override;
    void detach() override;

    void setToleranceUnits(Hydroponics_UnitsType units);
    Hydroponics_UnitsType getToleranceUnits() const;

    shared_ptr<HydroponicsSensor> getSensor() const;
    float getToleranceLow() const;
    float getToleranceHigh() const;

protected:
    float _toleranceLow;                                    // Low value tolerance
    float _toleranceHigh;                                   // High value tolerance
    Hydroponics_UnitsType _toleranceUnits;                  // Tolerance units (if set, else undef)
    bool _triggerOnOutside;                                 // Trigger on outside flag
    int8_t _measurementRow;                                 // Measurement data row to check against
    shared_ptr<HydroponicsSensor> _sensor;                  // Attached sensor

    void handleSensorMeasure(HydroponicsMeasurement *measurement);
};


// Reservoir Estimated Volume From Pump Flow Trigger
// This trigger attempts to estimate the time it would take for a pump to drain or
// fill a known volume of liquid with a known constant (or sensed) flow rate. It
// isn't as precise as a filled/empty indicator, but it's better for systems that
// avoid having to include such and instead can operate fine just using estimations.
// Better if equipped with  It can be set up as either triggering on empty or filled
// status, and triggering on input or output reservoir relative to the pump. Utilized
// reservoir must have explicitly set a known initial volume, and pump actuator must
// have explicitly set a constant flow rate or have a sensed flow rate.
/* TODO
class HydroponicsEstimatedVolumeTrigger : public HydroponicsTrigger
{
public:
    HydroponicsEstimatedVolumeTrigger(shared_ptr<HydroponicsActuator> pumpActuator, bool triggerOnEmpty = true, bool triggerOnInput = true);
    ~HydroponicsEstimatedVolumeTrigger();

    void update() override;

    void attach() override;
    void detach() override;

    shared_ptr<HydroponicsActuator> getPumpActuator() const;
    bool getTriggerOnEmpty() const;
    bool getTriggerOnInput() const;
    shared_ptr<HydroponicsSensor> getFlowRateSensor() const;
    shared_ptr<HydroponicsReservoir> getInputReservoir() const;
    shared_ptr<HydroponicsReservoir> getOutputReservoir() const;

protected:
    shared_ptr<HydroponicsActuator> _pumpActuator;          // Attached pump actuator
    bool _triggerOnEmpty;                                   // Trigger on empty flag
    bool _triggerOnInput;                                   // Trigger on input flag

    void handlePumpActivation(HydroponicsActuator *actuator);
    void handleFlowRateMeasure(HydroponicsMeasurement *measurement);
};*/

#endif // /ifndef HydroponicsTriggers_H
