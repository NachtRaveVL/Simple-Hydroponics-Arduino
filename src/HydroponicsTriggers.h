/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Triggers
*/

#ifndef HydroponicsTriggers_H
#define HydroponicsTriggers_H

class HydroponicsTrigger;
class HydroponicsMeasurementValueTrigger;
class HydroponicsMeasurementRangeTrigger;

struct HydroponicsTriggerSubData;

#include "Hydroponics.h"
#include "HydroponicsObject.h"

// Creates trigger object from passed trigger sub data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsTrigger *newTriggerObjectFromSubData(const HydroponicsTriggerSubData *dataIn);


// Hydroponics Trigger Base
// This is the base class for all triggers, which are used to alert the system
// to some change in a tracked property.
class HydroponicsTrigger : public HydroponicsSubObject, public HydroponicsTriggerObjectInterface {
public:
    const enum { MeasureValue, MeasureRange, Unknown = -1 } type; // Trigger type (custom RTTI)
    inline bool isMeasureValueType() const { return type == MeasureValue; }
    inline bool isMeasureRangeType() const { return type == MeasureRange; }
    inline bool isUnknownType() const { return type <= Unknown; }

    HydroponicsTrigger(HydroponicsIdentity sensorId,
                       byte measurementRow = 0,
                       int type = Unknown);
    HydroponicsTrigger(shared_ptr<HydroponicsSensor> sensor,
                       byte measurementRow = 0,
                       int type = Unknown);
    HydroponicsTrigger(const HydroponicsTriggerSubData *dataIn);
    virtual ~HydroponicsTrigger();

    virtual void saveToData(HydroponicsTriggerSubData *dataOut) const;

    virtual void update() override;
    virtual void handleLowMemory() override;

    virtual void attachTrigger() = 0;
    virtual void detachTrigger() = 0;
    virtual Hydroponics_TriggerState getTriggerState() const override;

    void setToleranceUnits(Hydroponics_UnitsType toleranceUnits);
    inline Hydroponics_UnitsType getToleranceUnits() const { return _toleranceUnits; }

    inline shared_ptr<HydroponicsSensor> getSensor() { return _sensor.getObject(); }
    inline byte getMeasurementRow() const { return _measurementRow; }

    Signal<Hydroponics_TriggerState> &getTriggerSignal();

protected:
    HydroponicsDLinkObject<HydroponicsSensor> _sensor;      // Attached sensor
    byte _measurementRow;                                   // Measurement data row to check against
    bool _attached;                                         // Attached flag
    bool _needsSensorUpdate;                                // Needs sensor measure update call tracking flag
    Hydroponics_UnitsType _toleranceUnits;                  // Tolerance units (if set, else undef)
    Hydroponics_TriggerState _triggerState;                 // Current trigger state
    Signal<Hydroponics_TriggerState> _triggerSignal;        // Trigger signal

    virtual void handleSensorMeasure(const HydroponicsMeasurement *measurement) = 0;
};


// Sensor Data Measurement Value Trigger
// This trigger simply checks a measured value against a set tolerance value and is
// useful for simple comparisons that control triggering. Initializes as disabled
// until updated with first measurement, and with undefined units that compares
// directly to measured units, otherwise units can be explicitly set. Can also
// set an additive value that a measurement must go past in order to detrigger.
class HydroponicsMeasurementValueTrigger : public HydroponicsTrigger {
public:
    HydroponicsMeasurementValueTrigger(HydroponicsIdentity sensorId,
                                       float triggerTol,
                                       bool triggerBelow = true,
                                       float detriggerTol = 0,
                                       byte measurementRow = 0);
    HydroponicsMeasurementValueTrigger(shared_ptr<HydroponicsSensor> sensor,
                                       float triggerTol,
                                       bool triggerBelow = true,
                                       float detriggerTol = 0,
                                       byte measurementRow = 0);
    HydroponicsMeasurementValueTrigger(const HydroponicsTriggerSubData *dataIn);
    ~HydroponicsMeasurementValueTrigger();

    virtual void saveToData(HydroponicsTriggerSubData *dataOut) const override;

    virtual void attachTrigger() override;
    virtual void detachTrigger() override;

    void setTriggerTolerance(float tolerance);

    inline float getTriggerTolerance() const { return _triggerTol; }
    inline float getDetriggerTolerance() const { return _detriggerTol; }
    inline bool getTriggerBelow() const { return _triggerBelow; }

protected:
    float _triggerTol;                                      // Trigger tolerance limit
    float _detriggerTol;                                    // Detrigger tolerance additive
    bool _triggerBelow;                                     // Trigger below flag

    virtual void handleSensorMeasure(const HydroponicsMeasurement *measurement) override;
};


// Sensor Data Measurement Range Trigger
// This trigger checks a measured value against a set tolerance range and is
// useful for ranged measurements that need to stay inside of (or outside of) a
// known range before triggering. Initializes as disabled until updated with
// first measurement, and with undefined units that compares directly to measured
// units, otherwise units can be explicitly set. Can also set an additive value
// that a measurement must go past in order to detrigger.
class HydroponicsMeasurementRangeTrigger : public HydroponicsTrigger {
public:
    HydroponicsMeasurementRangeTrigger(HydroponicsIdentity sensorId,
                                       float toleranceLow,
                                       float toleranceHigh,
                                       bool triggerOutside = true,
                                       float detriggerTol = 0,
                                       byte measurementRow = 0);
    HydroponicsMeasurementRangeTrigger(shared_ptr<HydroponicsSensor> sensor,
                                       float toleranceLow,
                                       float toleranceHigh,
                                       bool triggerOutside = true,
                                       float detriggerTol = 0,
                                       byte measurementRow = 0);
    HydroponicsMeasurementRangeTrigger(const HydroponicsTriggerSubData *dataIn);
    ~HydroponicsMeasurementRangeTrigger();

    virtual void saveToData(HydroponicsTriggerSubData *dataOut) const override;

    virtual void attachTrigger() override;
    virtual void detachTrigger() override;

    void setTriggerToleranceMid(float toleranceMid);

    inline float getTriggerToleranceLow() const { return _triggerTolLow; }
    inline float getTriggerToleranceHigh() const { return _triggerTolHigh; }
    inline float getDetriggerTolerance() const { return _detriggerTol; }
    inline bool getTriggerOutside() const { return _triggerOutside; }

protected:
    float _triggerTolLow;                                   // Low value tolerance
    float _triggerTolHigh;                                  // High value tolerance
    float _detriggerTol;                                    // Detrigger tolerance additive
    bool _triggerOutside;                                   // Trigger on outside flag

    virtual void handleSensorMeasure(const HydroponicsMeasurement *measurement) override;
};


// Combined Trigger Serialization Sub Data
struct HydroponicsTriggerSubData : public HydroponicsSubData {
    char sensorName[HYDRUINO_NAME_MAXSIZE];
    int8_t measurementRow;
    union {
        struct {
            float tolerance;
            bool triggerBelow;
        } measureValue;
        struct {
            float toleranceLow;
            float toleranceHigh;
            bool triggerOutside;
        } measureRange;
    } dataAs;
    float detriggerTol;
    Hydroponics_UnitsType toleranceUnits;

    HydroponicsTriggerSubData();
    virtual void toJSONObject(JsonObject &objectOut) const;
    virtual void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroponicsTriggers_H
