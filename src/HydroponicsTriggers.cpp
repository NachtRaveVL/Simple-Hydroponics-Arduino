/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Triggers
*/

#include "Hydroponics.h"

// Creates trigger object from passed trigger data
HydroponicsTrigger *newTriggerObjectFromSubData(const HydroponicsTriggerSubData *dataIn)
{
    if (dataIn && dataIn->type == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->type >= 0, SFP(HS_Err_InvalidParameter));

    if (dataIn) {
        switch (dataIn->type) {
            case 0: // MeasureValue
                return new HydroponicsMeasurementValueTrigger(dataIn);
            case 1: // MeasureRange
                return new HydroponicsMeasurementRangeTrigger(dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroponicsTrigger::HydroponicsTrigger(HydroponicsIdentity sensorId, byte measurementRow, int typeIn)
    : type((typeof(type))typeIn), _sensor((HydroponicsObject *)this), _attached(false),
      _toleranceUnits(Hydroponics_UnitsType_Undefined), _triggerState(Hydroponics_TriggerState_Disabled)
{
    _sensor = sensorId;

    HYDRUINO_HARD_ASSERT(isMeasureValueType() || isMeasureRangeType(), HS_Err_OperationFailure);
    if (isMeasureValueType()) {
        _sensor.setProcessMethod(&HydroponicsMeasurementValueTrigger::handleMeasurement);
    } else if (isMeasureRangeType()) {
        _sensor.setProcessMethod(&HydroponicsMeasurementRangeTrigger::handleMeasurement);
    }
}

HydroponicsTrigger::HydroponicsTrigger(shared_ptr<HydroponicsSensor> sensor, byte measurementRow, int typeIn)
    : type((typeof(type))typeIn), _sensor((HydroponicsObject *)this), _attached(false),
      _toleranceUnits(Hydroponics_UnitsType_Undefined), _triggerState(Hydroponics_TriggerState_Disabled)
{
    _sensor = sensor;

    HYDRUINO_HARD_ASSERT(isMeasureValueType() || isMeasureRangeType(), HS_Err_OperationFailure);
    if (isMeasureValueType()) {
        _sensor.setProcessMethod(&HydroponicsMeasurementValueTrigger::handleMeasurement);
    } else if (isMeasureRangeType()) {
        _sensor.setProcessMethod(&HydroponicsMeasurementRangeTrigger::handleMeasurement);
    }
}

HydroponicsTrigger::HydroponicsTrigger(const HydroponicsTriggerSubData *dataIn)
    : type((typeof(type))(dataIn->type)), _sensor((HydroponicsObject *)this), _attached(false),
      _toleranceUnits(dataIn->toleranceUnits), _triggerState(Hydroponics_TriggerState_Disabled)
{
    _sensor = dataIn->sensorName;
    setToleranceUnits(dataIn->toleranceUnits);

    HYDRUINO_HARD_ASSERT(isMeasureValueType() || isMeasureRangeType(), HS_Err_OperationFailure);
    if (isMeasureValueType()) {
        _sensor.setProcessMethod(&HydroponicsMeasurementValueTrigger::handleMeasurement);
    } else if (isMeasureRangeType()) {
        _sensor.setProcessMethod(&HydroponicsMeasurementRangeTrigger::handleMeasurement);
    }
}

void HydroponicsTrigger::saveToData(HydroponicsTriggerSubData *dataOut) const
{
    ((HydroponicsTriggerSubData *)dataOut)->type = (int8_t)type;
    if (_sensor.getId()) {
        strncpy(((HydroponicsTriggerSubData *)dataOut)->sensorName, _sensor.getKeyString().c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    ((HydroponicsTriggerSubData *)dataOut)->measurementRow = _sensor.getMeasurementRow();
    ((HydroponicsTriggerSubData *)dataOut)->toleranceUnits = _toleranceUnits;
}

void HydroponicsTrigger::update()
{
    if (!_attached) { attachTrigger(); }

    _sensor.updateMeasurementIfNeeded();
}

void HydroponicsTrigger::handleLowMemory()
{ ; }

Hydroponics_TriggerState HydroponicsTrigger::getTriggerState() const
{
    return _triggerState;
}

void HydroponicsTrigger::setToleranceUnits(Hydroponics_UnitsType toleranceUnits)
{
    if (_toleranceUnits != toleranceUnits) {
        _toleranceUnits = toleranceUnits;

        _sensor.setMeasurementUnits(getToleranceUnits());
    }
}

Signal<Hydroponics_TriggerState, HYDRUINO_TRIGGER_STATE_SLOTS> &HydroponicsTrigger::getTriggerSignal()
{
    return _triggerSignal;
}


HydroponicsMeasurementValueTrigger::HydroponicsMeasurementValueTrigger(HydroponicsIdentity sensorId, float tolerance, bool triggerBelow, float detriggerTol, byte measurementRow)
    : HydroponicsTrigger(sensorId, measurementRow, MeasureValue),
      _triggerTol(tolerance), _detriggerTol(detriggerTol), _triggerBelow(triggerBelow)
{ ; }

HydroponicsMeasurementValueTrigger::HydroponicsMeasurementValueTrigger(shared_ptr<HydroponicsSensor> sensor, float tolerance, bool triggerBelow, float detriggerTol, byte measurementRow)
    : HydroponicsTrigger(sensor, measurementRow, MeasureValue),
      _triggerTol(tolerance), _detriggerTol(detriggerTol), _triggerBelow(triggerBelow)
{ ; }

HydroponicsMeasurementValueTrigger::HydroponicsMeasurementValueTrigger(const HydroponicsTriggerSubData *dataIn)
    : HydroponicsTrigger(dataIn),
      _triggerTol(dataIn->dataAs.measureValue.tolerance), _detriggerTol(dataIn->detriggerTol),
      _triggerBelow(dataIn->dataAs.measureValue.triggerBelow)
{ ; }

void HydroponicsMeasurementValueTrigger::saveToData(HydroponicsTriggerSubData *dataOut) const
{
    HydroponicsTrigger::saveToData(dataOut);

    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureValue.tolerance = _triggerTol;
    ((HydroponicsTriggerSubData *)dataOut)->detriggerTol = _detriggerTol;
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureValue.triggerBelow = _triggerBelow;
}

void HydroponicsMeasurementValueTrigger::attachTrigger()
{
    if (!_attached) {
        _sensor.attachObject();
        _attached = _sensor.isResolved();
    }
}

void HydroponicsMeasurementValueTrigger::detachTrigger()
{
    if (_attached) {
        _sensor.detachObject();
        _attached = _sensor.isResolved();
    }
}

void HydroponicsMeasurementValueTrigger::setTriggerTolerance(float tolerance)
{
    if (!isFPEqual(_triggerTol, tolerance)) {
        _triggerTol = tolerance;

        _sensor.setNeedsMeasurement();
    }
}

void HydroponicsMeasurementValueTrigger::handleMeasurement(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        bool nextState = triggerStateToBool(_triggerState);

        if (measurement->isBinaryType()) {
            nextState = ((HydroponicsBinaryMeasurement *)measurement)->state != _triggerBelow;

            _sensor.setMeasurement(getAsSingleMeasurement(measurement, _sensor.getMeasurementRow()));
        } else {
            auto measure = getAsSingleMeasurement(measurement, _sensor.getMeasurementRow());

            convertUnits(&measure, getToleranceUnits(), _sensor.getMeasurementConvertParam());

            _sensor.setMeasurement(measure);

            float tolAdditive = (_triggerState == Hydroponics_TriggerState_Triggered ? _detriggerTol : 0);
            nextState = (_triggerBelow ? measure.value <= _triggerTol + tolAdditive + FLT_EPSILON
                                       : measure.value >= _triggerTol - tolAdditive - FLT_EPSILON);
        }

        if ((_triggerState == Hydroponics_TriggerState_Disabled) ||
            (nextState != (_triggerState == Hydroponics_TriggerState_Triggered))) {
            _triggerState = triggerStateFromBool(nextState);

            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<Hydroponics_TriggerState>(_triggerSignal, _triggerState);
            #else
                _triggerSignal.fire(_triggerState);
            #endif
        }
    }
}


HydroponicsMeasurementRangeTrigger::HydroponicsMeasurementRangeTrigger(HydroponicsIdentity sensorId, float toleranceLow, float toleranceHigh, bool triggerOutside, float detriggerTol, byte measurementRow)
    : HydroponicsTrigger(sensorId, measurementRow, MeasureRange),
      _triggerTolLow(toleranceLow), _triggerTolHigh(toleranceHigh), _detriggerTol(detriggerTol),
      _triggerOutside(triggerOutside)
{ ; }

HydroponicsMeasurementRangeTrigger::HydroponicsMeasurementRangeTrigger(shared_ptr<HydroponicsSensor> sensor, float toleranceLow, float toleranceHigh, bool triggerOutside, float detriggerTol, byte measurementRow)
    : HydroponicsTrigger(sensor, measurementRow, MeasureRange),
      _triggerTolLow(toleranceLow), _triggerTolHigh(toleranceHigh), _detriggerTol(detriggerTol),
      _triggerOutside(triggerOutside)
{ ; }

HydroponicsMeasurementRangeTrigger::HydroponicsMeasurementRangeTrigger(const HydroponicsTriggerSubData *dataIn)
    : HydroponicsTrigger(dataIn),
      _triggerTolLow(dataIn->dataAs.measureRange.toleranceLow),
      _triggerTolHigh(dataIn->dataAs.measureRange.toleranceHigh),
      _detriggerTol(dataIn->detriggerTol),
      _triggerOutside(dataIn->dataAs.measureRange.triggerOutside)
{ ; }

void HydroponicsMeasurementRangeTrigger::saveToData(HydroponicsTriggerSubData *dataOut) const
{
    HydroponicsTrigger::saveToData(dataOut);

    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureRange.toleranceLow = _triggerTolLow;
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureRange.toleranceHigh = _triggerTolHigh;
    ((HydroponicsTriggerSubData *)dataOut)->detriggerTol = _detriggerTol;
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureRange.triggerOutside = _triggerOutside;
}

void HydroponicsMeasurementRangeTrigger::attachTrigger()
{
    if (!_attached) {
        _sensor.attachObject();
        _attached = _sensor.isResolved();
    }
}

void HydroponicsMeasurementRangeTrigger::detachTrigger()
{
    if (_attached) {
        _sensor.detachObject();
        _attached = _sensor.isResolved();
    }
}

void HydroponicsMeasurementRangeTrigger::updateTriggerMidpoint(float toleranceMid)
{
    float toleranceRangeHalf = (_triggerTolHigh - _triggerTolLow) * 0.5f;

    if (!isFPEqual(_triggerTolLow, toleranceMid - toleranceRangeHalf)) {
        _triggerTolLow = toleranceMid - toleranceRangeHalf;
        _triggerTolHigh = toleranceMid + toleranceRangeHalf;

        _sensor.setNeedsMeasurement();
    }
}

void HydroponicsMeasurementRangeTrigger::handleMeasurement(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        bool nextState = triggerStateToBool(_triggerState);
        auto measure = getAsSingleMeasurement(measurement, _sensor.getMeasurementRow());

        convertUnits(&measure, getToleranceUnits(), _sensor.getMeasurementConvertParam());

        _sensor.setMeasurement(measure);

        float tolAdditive = (_triggerState == Hydroponics_TriggerState_Triggered ? _detriggerTol : 0);

        if (!_triggerOutside) {
            nextState = (measure.value >= _triggerTolLow - tolAdditive - FLT_EPSILON &&
                         measure.value <= _triggerTolHigh + tolAdditive + FLT_EPSILON);
        } else {
            nextState = (measure.value <= _triggerTolLow + tolAdditive + FLT_EPSILON &&
                         measure.value >= _triggerTolHigh - tolAdditive - FLT_EPSILON);
        }

        if ((_triggerState == Hydroponics_TriggerState_Disabled) ||
            (nextState != (_triggerState == Hydroponics_TriggerState_Triggered))) {
            _triggerState = triggerStateFromBool(nextState);

            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<Hydroponics_TriggerState>(_triggerSignal, _triggerState);
            #else
                _triggerSignal.fire(_triggerState);
            #endif
        }
    }
}


HydroponicsTriggerSubData::HydroponicsTriggerSubData()
    : HydroponicsSubData(), sensorName{0}, measurementRow(0), dataAs{.measureRange={0.0f,0.0f,false}}, detriggerTol(0), toleranceUnits(Hydroponics_UnitsType_Undefined)
{ ; }

void HydroponicsTriggerSubData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSubData::toJSONObject(objectOut);

    if (sensorName[0]) { objectOut[SFP(HS_Key_SensorName)] = charsToString(sensorName, HYDRUINO_NAME_MAXSIZE); }
    if (measurementRow > 0) { objectOut[SFP(HS_Key_MeasurementRow)] = measurementRow; }
    switch (type) {
        case 0: // MeasureValue
            objectOut[SFP(HS_Key_Tolerance)] = dataAs.measureValue.tolerance;
            objectOut[SFP(HS_Key_TriggerBelow)] = dataAs.measureValue.triggerBelow;
            break;
        case 1: // MeasureRange
            objectOut[SFP(HS_Key_ToleranceLow)] = dataAs.measureRange.toleranceLow;
            objectOut[SFP(HS_Key_ToleranceHigh)] = dataAs.measureRange.toleranceHigh;
            objectOut[SFP(HS_Key_TriggerOutside)] = dataAs.measureRange.triggerOutside;
            break;
        default: break;
    }
    if (detriggerTol > 0) { objectOut[SFP(HS_Key_DetriggerTol)] = detriggerTol; }
    if (toleranceUnits != Hydroponics_UnitsType_Undefined) { objectOut[SFP(HS_Key_ToleranceUnits)] = unitsTypeToSymbol(toleranceUnits); }
}

void HydroponicsTriggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSubData::fromJSONObject(objectIn);

    const char *sensorNameStr = objectIn[SFP(HS_Key_SensorName)];
    if (sensorNameStr && sensorNameStr[0]) { strncpy(sensorName, sensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    measurementRow = objectIn[SFP(HS_Key_MeasurementRow)] | measurementRow;
    switch (type) {
        case 0: // MeasureValue
            dataAs.measureValue.tolerance = objectIn[SFP(HS_Key_Tolerance)] | dataAs.measureValue.tolerance;
            dataAs.measureValue.triggerBelow = objectIn[SFP(HS_Key_TriggerBelow)] | dataAs.measureValue.triggerBelow;
            break;
        case 1: // MeasureRange
            dataAs.measureRange.toleranceLow = objectIn[SFP(HS_Key_ToleranceLow)] | dataAs.measureRange.toleranceLow;
            dataAs.measureRange.toleranceHigh = objectIn[SFP(HS_Key_ToleranceHigh)] | dataAs.measureRange.toleranceHigh;
            dataAs.measureRange.triggerOutside = objectIn[SFP(HS_Key_TriggerOutside)] | dataAs.measureRange.triggerOutside;
            break;
        default: break;
    }
    detriggerTol = objectIn[SFP(HS_Key_DetriggerTol)] | detriggerTol;
    toleranceUnits = unitsTypeFromSymbol(objectIn[SFP(HS_Key_ToleranceUnits)]);
}
