/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Triggers
*/

#ifndef HydroTriggers_H
#define HydroTriggers_H

class HydroTrigger;
class HydroMeasurementValueTrigger;
class HydroMeasurementRangeTrigger;

struct HydroTriggerSubData;

#include "Hydruino.h"
#include "HydroObject.h"
#include "HydroSensors.h"

// Creates trigger object from passed trigger sub data (return ownership transfer - user code *must* delete returned object)
extern HydroTrigger *newTriggerObjectFromSubData(const HydroTriggerSubData *dataIn);


// Trigger Base
// This is the base class for all triggers, which are used to alert the system
// to some change in a tracked property.
class HydroTrigger : public HydroSubObject, public HydroTriggerObjectInterface, public HydroMeasurementUnitsInterface, public HydroSensorAttachmentInterface {
public:
    const enum : signed char { MeasureValue, MeasureRange, Unknown = -1 } type; // Trigger type (custom RTTI)
    inline bool isMeasureValueType() const { return type == MeasureValue; }
    inline bool isMeasureRangeType() const { return type == MeasureRange; }
    inline bool isUnknownType() const { return type <= Unknown; }

    HydroTrigger(HydroIdentity sensorId,
                 uint8_t measurementRow = 0,
                 int type = Unknown);
    HydroTrigger(SharedPtr<HydroSensor> sensor,
                 uint8_t measurementRow = 0,
                 int type = Unknown);
    HydroTrigger(const HydroTriggerSubData *dataIn);

    virtual void saveToData(HydroTriggerSubData *dataOut) const;

    virtual void update();
    virtual void handleLowMemory();

    virtual Hydro_TriggerState getTriggerState(bool poll = false) override;

    virtual void setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Hydro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

    inline uint8_t getMeasurementRow() const { return _sensor.getMeasurementRow(); }
    inline float getMeasurementConvertParam() const { return _sensor.getMeasurementConvertParam(); }

    virtual HydroSensorAttachment &getSensorAttachment() override;

    Signal<Hydro_TriggerState, HYDRO_TRIGGER_SIGNAL_SLOTS> &getTriggerSignal();

protected:
    HydroSensorAttachment _sensor;                          // Sensor attachment
    Hydro_TriggerState _triggerState;                       // Trigger state (last handled)
    Signal<Hydro_TriggerState, HYDRO_TRIGGER_SIGNAL_SLOTS> _triggerSignal; // Trigger signal

    virtual void handleMeasurement(const HydroMeasurement *measurement) = 0;
};


// Sensor Data Measurement Value Trigger
// This trigger simply checks a measured value against a set tolerance value and is
// useful for simple comparisons that control triggering. Initializes as disabled
// until updated with first measurement, and with undefined units that compares
// directly to measured units, otherwise units can be explicitly set. Can also
// set an additive value that a measurement must go past in order to detrigger.
class HydroMeasurementValueTrigger : public HydroTrigger {
public:
    HydroMeasurementValueTrigger(HydroIdentity sensorId,
                                 float triggerTol,
                                 bool triggerBelow = true,
                                 float detriggerTol = 0,
                                 uint8_t measurementRow = 0);
    HydroMeasurementValueTrigger(SharedPtr<HydroSensor> sensor,
                                 float triggerTol,
                                 bool triggerBelow = true,
                                 float detriggerTol = 0,
                                 uint8_t measurementRow = 0);
    HydroMeasurementValueTrigger(const HydroTriggerSubData *dataIn);

    virtual void saveToData(HydroTriggerSubData *dataOut) const override;

    void setTriggerTolerance(float tolerance);

    inline float getTriggerTolerance() const { return _triggerTol; }
    inline float getDetriggerTolerance() const { return _detriggerTol; }
    inline bool getTriggerBelow() const { return _triggerBelow; }

protected:
    float _triggerTol;                                      // Trigger tolerance limit
    float _detriggerTol;                                    // Detrigger tolerance additive
    bool _triggerBelow;                                     // Trigger below flag

    virtual void handleMeasurement(const HydroMeasurement *measurement) override;
    friend class HydroTrigger;
};


// Sensor Data Measurement Range Trigger
// This trigger checks a measured value against a set tolerance range and is
// useful for ranged measurements that need to stay inside of (or outside of) a
// known range before triggering. Initializes as disabled until updated with
// first measurement, and with undefined units that compares directly to measured
// units, otherwise units can be explicitly set. Can also set an additive value
// that a measurement must go past in order to detrigger.
class HydroMeasurementRangeTrigger : public HydroTrigger {
public:
    HydroMeasurementRangeTrigger(HydroIdentity sensorId,
                                 float toleranceLow,
                                 float toleranceHigh,
                                 bool triggerOutside = true,
                                 float detriggerTol = 0,
                                 uint8_t measurementRow = 0);
    HydroMeasurementRangeTrigger(SharedPtr<HydroSensor> sensor,
                                 float toleranceLow,
                                 float toleranceHigh,
                                 bool triggerOutside = true,
                                 float detriggerTol = 0,
                                 uint8_t measurementRow = 0);
    HydroMeasurementRangeTrigger(const HydroTriggerSubData *dataIn);

    virtual void saveToData(HydroTriggerSubData *dataOut) const override;

    void updateTriggerMidpoint(float toleranceMid);

    inline float getTriggerToleranceLow() const { return _triggerTolLow; }
    inline float getTriggerToleranceHigh() const { return _triggerTolHigh; }
    inline float getDetriggerTolerance() const { return _detriggerTol; }
    inline bool getTriggerOutside() const { return _triggerOutside; }

protected:
    float _triggerTolLow;                                   // Low value tolerance
    float _triggerTolHigh;                                  // High value tolerance
    float _detriggerTol;                                    // Detrigger tolerance additive
    bool _triggerOutside;                                   // Trigger on outside flag

    virtual void handleMeasurement(const HydroMeasurement *measurement) override;
    friend class HydroTrigger;
};


// Combined Trigger Serialization Sub Data
struct HydroTriggerSubData : public HydroSubData {
    char sensorName[HYDRO_NAME_MAXSIZE];
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
    Hydro_UnitsType measurementUnits;

    HydroTriggerSubData();
    virtual void toJSONObject(JsonObject &objectOut) const;
    virtual void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef HydroTriggers_H
