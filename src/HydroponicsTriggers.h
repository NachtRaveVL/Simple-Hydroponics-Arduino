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

// Creates trigger object from passed trigger sub data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsTrigger *newTriggerObjectFromSubData(const HydroponicsTriggerSubData *dataIn);


// Hydroponics Trigger Base
// This is the base class for all triggers, which are used to alert the system
// to some change in a tracked property.
class HydroponicsTrigger : public HydroponicsSubObject, public HydroponicsTriggerObjectInterface {
public:
    const enum { MeasureValue, MeasureRange, Unknown = -1 } type; // Trigger type (custom RTTI)
    inline bool isMeasureValueType() { return type == MeasureValue; }
    inline bool isMeasureRangeType() { return type == MeasureRange; }
    inline bool isUnknownType() { return type <= Unknown; }

    HydroponicsTrigger(int type = Unknown);
    HydroponicsTrigger(const HydroponicsTriggerSubData *dataIn);
    virtual ~HydroponicsTrigger();

    void saveToData(HydroponicsSubData *dataOut) const override;
    virtual void saveToData(HydroponicsTriggerSubData *dataOut) const;

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual void attachTrigger() = 0;
    virtual void detachTrigger() = 0;
    virtual Hydroponics_TriggerState getTriggerState() const override;

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
    HydroponicsMeasurementValueTrigger(HydroponicsIdentity sensorId, float tolerance, bool triggerBelow, int measurementRow = 0);
    HydroponicsMeasurementValueTrigger(shared_ptr<HydroponicsSensor> sensor, float tolerance, bool triggerBelow, int measurementRow = 0);
    HydroponicsMeasurementValueTrigger(const HydroponicsTriggerSubData *dataIn);
    ~HydroponicsMeasurementValueTrigger();

    void saveToData(HydroponicsTriggerSubData *dataOut) const override;

    void resolveLinks() override;

    void attachTrigger() override;
    void detachTrigger() override;

    void setToleranceUnits(Hydroponics_UnitsType units);
    Hydroponics_UnitsType getToleranceUnits() const;

    shared_ptr<HydroponicsSensor> getSensor();
    float getTolerance() const;
    bool getTriggerBelow() const;

protected:
    HydroponicsDLinkObject<HydroponicsSensor> _sensor;      // Attached sensor
    float _tolerance;                                       // Tolerance limit
    Hydroponics_UnitsType _toleranceUnits;                  // Tolerance units (if set, else undef)
    bool _triggerBelow;                                     // Trigger below flag
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
    HydroponicsMeasurementRangeTrigger(HydroponicsIdentity sensorId, float toleranceLow, float toleranceHigh, bool triggerOutside = true, int measurementRow = 0);
    HydroponicsMeasurementRangeTrigger(shared_ptr<HydroponicsSensor> sensor, float toleranceLow, float toleranceHigh, bool triggerOutside = true, int measurementRow = 0);
    HydroponicsMeasurementRangeTrigger(const HydroponicsTriggerSubData *dataIn);
    ~HydroponicsMeasurementRangeTrigger();

    void saveToData(HydroponicsTriggerSubData *dataOut) const override;

    void resolveLinks() override;

    void attachTrigger() override;
    void detachTrigger() override;

    void setToleranceUnits(Hydroponics_UnitsType units);
    Hydroponics_UnitsType getToleranceUnits() const;

    shared_ptr<HydroponicsSensor> getSensor();
    float getToleranceLow() const;
    float getToleranceHigh() const;

protected:
    HydroponicsDLinkObject<HydroponicsSensor> _sensor;      // Attached sensor
    float _toleranceLow;                                    // Low value tolerance
    float _toleranceHigh;                                   // High value tolerance
    Hydroponics_UnitsType _toleranceUnits;                  // Tolerance units (if set, else undef)
    bool _triggerOutside;                                   // Trigger on outside flag
    int8_t _measurementRow;                                 // Measurement data row to check against

    void handleSensorMeasure(HydroponicsMeasurement *measurement);
};


// Combined Trigger Serialization Sub Data
struct HydroponicsTriggerSubData : public HydroponicsSubData {
    char sensorName[HYDRUINO_NAME_MAXSIZE];
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
    Hydroponics_UnitsType toleranceUnits;
    int8_t measurementRow;

    HydroponicsTriggerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroponicsTriggers_H
