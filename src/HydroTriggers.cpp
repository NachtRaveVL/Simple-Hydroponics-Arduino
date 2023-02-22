/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Triggers
*/

#include "Hydruino.h"

// Creates trigger object from passed trigger data
HydroTrigger *newTriggerObjectFromSubData(const HydroTriggerSubData *dataIn)
{
    if (!dataIn || !isValidType(dataIn->type)) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && isValidType(dataIn->type), SFP(HStr_Err_InvalidParameter));

    if (dataIn) {
        switch (dataIn->type) {
            case (hid_t)HydroTrigger::MeasureValue:
                return new HydroMeasurementValueTrigger(dataIn);
            case (hid_t)HydroTrigger::MeasureRange:
                return new HydroMeasurementRangeTrigger(dataIn);
            default: break;
        }
    }

    return nullptr;
}


HydroTrigger::HydroTrigger(HydroIdentity sensorId, uint8_t measurementRow, int typeIn)
    : type((typeof(type))typeIn), _sensor(this), _triggerState(Hydro_TriggerState_Disabled)
{
    _sensor.setMeasurementRow(measurementRow);
    _sensor.setObject(sensorId);
}

HydroTrigger::HydroTrigger(SharedPtr<HydroSensor> sensor, uint8_t measurementRow, int typeIn)
    : type((typeof(type))typeIn), _sensor(this), _triggerState(Hydro_TriggerState_Disabled)
{
    _sensor.setMeasurementRow(measurementRow);
    _sensor.setObject(sensor);
}

HydroTrigger::HydroTrigger(const HydroTriggerSubData *dataIn)
    : type((typeof(type))(dataIn->type)), _sensor(this), _triggerState(Hydro_TriggerState_Disabled)
{
    _sensor.setMeasurementRow(dataIn->measurementRow);
    _sensor.setMeasurementUnits(dataIn->measurementUnits);
    _sensor.setObject(dataIn->sensorName);
}

void HydroTrigger::saveToData(HydroTriggerSubData *dataOut) const
{
    ((HydroTriggerSubData *)dataOut)->type = (int8_t)type;
    if (_sensor.isSet()) {
        strncpy(((HydroTriggerSubData *)dataOut)->sensorName, _sensor.getKeyString().c_str(), HYDRO_NAME_MAXSIZE);
    }
    ((HydroTriggerSubData *)dataOut)->measurementRow = getMeasurementRow();
    ((HydroTriggerSubData *)dataOut)->measurementUnits = getMeasurementUnits();
}

void HydroTrigger::update()
{
    _sensor.updateIfNeeded(true);
}

void HydroTrigger::handleLowMemory()
{ ; }

Hydro_TriggerState HydroTrigger::getTriggerState(bool poll)
{
    _sensor.updateIfNeeded(poll);
    return _triggerState;
}

void HydroTrigger::setMeasurementUnits(Hydro_UnitsType measurementUnits, uint8_t measurementRow)
{
    _sensor.setMeasurementUnits(measurementUnits);
}

Hydro_UnitsType HydroTrigger::getMeasurementUnits(uint8_t measurementRow) const
{
    return _sensor.getMeasurementUnits();
}

Signal<Hydro_TriggerState, HYDRO_TRIGGER_SIGNAL_SLOTS> &HydroTrigger::getTriggerSignal()
{
    return _triggerSignal;
}


HydroMeasurementValueTrigger::HydroMeasurementValueTrigger(HydroIdentity sensorId, float tolerance, bool triggerBelow, float detriggerTol, uint8_t measurementRow)
    : HydroTrigger(sensorId, measurementRow, MeasureValue),
      _triggerTol(tolerance), _detriggerTol(detriggerTol), _triggerBelow(triggerBelow)
{
    _sensor.setHandleMethod(&HydroMeasurementValueTrigger::handleMeasurement);
}

HydroMeasurementValueTrigger::HydroMeasurementValueTrigger(SharedPtr<HydroSensor> sensor, float tolerance, bool triggerBelow, float detriggerTol, uint8_t measurementRow)
    : HydroTrigger(sensor, measurementRow, MeasureValue),
      _triggerTol(tolerance), _detriggerTol(detriggerTol), _triggerBelow(triggerBelow)
{
    _sensor.setHandleMethod(&HydroMeasurementValueTrigger::handleMeasurement);
}

HydroMeasurementValueTrigger::HydroMeasurementValueTrigger(const HydroTriggerSubData *dataIn)
    : HydroTrigger(dataIn),
      _triggerTol(dataIn->dataAs.measureValue.tolerance), _detriggerTol(dataIn->detriggerTol),
      _triggerBelow(dataIn->dataAs.measureValue.triggerBelow)
{
    _sensor.setHandleMethod(&HydroMeasurementValueTrigger::handleMeasurement);
}

void HydroMeasurementValueTrigger::saveToData(HydroTriggerSubData *dataOut) const
{
    HydroTrigger::saveToData(dataOut);

    ((HydroTriggerSubData *)dataOut)->dataAs.measureValue.tolerance = _triggerTol;
    ((HydroTriggerSubData *)dataOut)->detriggerTol = _detriggerTol;
    ((HydroTriggerSubData *)dataOut)->dataAs.measureValue.triggerBelow = _triggerBelow;
}

void HydroMeasurementValueTrigger::setTriggerTolerance(float tolerance)
{
    if (!isFPEqual(_triggerTol, tolerance)) {
        _triggerTol = tolerance;

        _sensor.setNeedsMeasurement();
    }
}

void HydroMeasurementValueTrigger::handleMeasurement(const HydroMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        bool nextState = triggerStateToBool(_triggerState);

        if (measurement->isBinaryType()) {
            nextState = ((HydroBinaryMeasurement *)measurement)->state != _triggerBelow;
            _sensor.setMeasurement(getAsSingleMeasurement(measurement, getMeasurementRow()));
        } else {
            auto measure = getAsSingleMeasurement(measurement, getMeasurementRow());
            convertUnits(&measure, getMeasurementUnits(), getMeasurementConvertParam());
            _sensor.setMeasurement(measure);

            float tolAdditive = (nextState ? _detriggerTol : 0);
            nextState = (_triggerBelow ? measure.value <= _triggerTol + tolAdditive + FLT_EPSILON
                                       : measure.value >= _triggerTol - tolAdditive - FLT_EPSILON);
        }

        if (_triggerState == Hydro_TriggerState_Disabled ||
            nextState != triggerStateToBool(_triggerState)) {
            _triggerState = triggerStateFromBool(nextState);

            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<Hydro_TriggerState>(_triggerSignal, _triggerState);
            #else
                _triggerSignal.fire(_triggerState);
            #endif
        }
    }
}


HydroMeasurementRangeTrigger::HydroMeasurementRangeTrigger(HydroIdentity sensorId, float toleranceLow, float toleranceHigh, bool triggerOutside, float detriggerTol, uint8_t measurementRow)
    : HydroTrigger(sensorId, measurementRow, MeasureRange),
      _triggerTolLow(toleranceLow), _triggerTolHigh(toleranceHigh), _detriggerTol(detriggerTol),
      _triggerOutside(triggerOutside)
{
    _sensor.setHandleMethod(&HydroMeasurementRangeTrigger::handleMeasurement);
}

HydroMeasurementRangeTrigger::HydroMeasurementRangeTrigger(SharedPtr<HydroSensor> sensor, float toleranceLow, float toleranceHigh, bool triggerOutside, float detriggerTol, uint8_t measurementRow)
    : HydroTrigger(sensor, measurementRow, MeasureRange),
      _triggerTolLow(toleranceLow), _triggerTolHigh(toleranceHigh), _detriggerTol(detriggerTol),
      _triggerOutside(triggerOutside)
{
    _sensor.setHandleMethod(&HydroMeasurementRangeTrigger::handleMeasurement);
}

HydroMeasurementRangeTrigger::HydroMeasurementRangeTrigger(const HydroTriggerSubData *dataIn)
    : HydroTrigger(dataIn),
      _triggerTolLow(dataIn->dataAs.measureRange.toleranceLow),
      _triggerTolHigh(dataIn->dataAs.measureRange.toleranceHigh),
      _detriggerTol(dataIn->detriggerTol),
      _triggerOutside(dataIn->dataAs.measureRange.triggerOutside)
{
    _sensor.setHandleMethod(&HydroMeasurementRangeTrigger::handleMeasurement);
}

void HydroMeasurementRangeTrigger::saveToData(HydroTriggerSubData *dataOut) const
{
    HydroTrigger::saveToData(dataOut);

    ((HydroTriggerSubData *)dataOut)->dataAs.measureRange.toleranceLow = _triggerTolLow;
    ((HydroTriggerSubData *)dataOut)->dataAs.measureRange.toleranceHigh = _triggerTolHigh;
    ((HydroTriggerSubData *)dataOut)->detriggerTol = _detriggerTol;
    ((HydroTriggerSubData *)dataOut)->dataAs.measureRange.triggerOutside = _triggerOutside;
}

void HydroMeasurementRangeTrigger::updateTriggerMidpoint(float toleranceMid)
{
    float toleranceRangeHalf = (_triggerTolHigh - _triggerTolLow) * 0.5f;

    if (!isFPEqual(_triggerTolLow, toleranceMid - toleranceRangeHalf)) {
        _triggerTolLow = toleranceMid - toleranceRangeHalf;
        _triggerTolHigh = toleranceMid + toleranceRangeHalf;

        _sensor.setNeedsMeasurement();
    }
}

void HydroMeasurementRangeTrigger::handleMeasurement(const HydroMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        bool nextState = triggerStateToBool(_triggerState);

        auto measure = getAsSingleMeasurement(measurement, getMeasurementRow());
        convertUnits(&measure, getMeasurementUnits(), getMeasurementConvertParam());
        _sensor.setMeasurement(measure);

        float tolAdditive = (nextState ? _detriggerTol : 0);

        if (!_triggerOutside) {
            nextState = (measure.value >= _triggerTolLow - tolAdditive - FLT_EPSILON &&
                         measure.value <= _triggerTolHigh + tolAdditive + FLT_EPSILON);
        } else {
            nextState = (measure.value <= _triggerTolLow + tolAdditive + FLT_EPSILON &&
                         measure.value >= _triggerTolHigh - tolAdditive - FLT_EPSILON);
        }

        if (_triggerState == Hydro_TriggerState_Disabled ||
            nextState != triggerStateToBool(_triggerState)) {
            _triggerState = triggerStateFromBool(nextState);

            #ifdef HYDRO_USE_MULTITASKING
                scheduleSignalFireOnce<Hydro_TriggerState>(_triggerSignal, _triggerState);
            #else
                _triggerSignal.fire(_triggerState);
            #endif
        }
    }
}


HydroTriggerSubData::HydroTriggerSubData()
    : HydroSubData(), sensorName{0}, measurementRow(0), dataAs{.measureRange={0.0f,0.0f,false}}, detriggerTol(0), measurementUnits(Hydro_UnitsType_Undefined)
{ ; }

void HydroTriggerSubData::toJSONObject(JsonObject &objectOut) const
{
    HydroSubData::toJSONObject(objectOut);

    if (sensorName[0]) { objectOut[SFP(HStr_Key_SensorName)] = charsToString(sensorName, HYDRO_NAME_MAXSIZE); }
    if (measurementRow > 0) { objectOut[SFP(HStr_Key_MeasurementRow)] = measurementRow; }
    switch (type) {
        case (hid_t)HydroTrigger::MeasureValue:
            objectOut[SFP(HStr_Key_Tolerance)] = dataAs.measureValue.tolerance;
            objectOut[SFP(HStr_Key_TriggerBelow)] = dataAs.measureValue.triggerBelow;
            break;
        case (hid_t)HydroTrigger::MeasureRange:
            objectOut[SFP(HStr_Key_ToleranceLow)] = dataAs.measureRange.toleranceLow;
            objectOut[SFP(HStr_Key_ToleranceHigh)] = dataAs.measureRange.toleranceHigh;
            objectOut[SFP(HStr_Key_TriggerOutside)] = dataAs.measureRange.triggerOutside;
            break;
        default: break;
    }
    if (detriggerTol > 0) { objectOut[SFP(HStr_Key_DetriggerTol)] = detriggerTol; }
    if (measurementUnits != Hydro_UnitsType_Undefined) { objectOut[SFP(HStr_Key_MeasurementUnits)] = unitsTypeToSymbol(measurementUnits); }
}

void HydroTriggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroSubData::fromJSONObject(objectIn);

    const char *sensorNameStr = objectIn[SFP(HStr_Key_SensorName)];
    if (sensorNameStr && sensorNameStr[0]) { strncpy(sensorName, sensorNameStr, HYDRO_NAME_MAXSIZE); }
    measurementRow = objectIn[SFP(HStr_Key_MeasurementRow)] | measurementRow;
    switch (type) {
        case (hid_t)HydroTrigger::MeasureValue:
            dataAs.measureValue.tolerance = objectIn[SFP(HStr_Key_Tolerance)] | dataAs.measureValue.tolerance;
            dataAs.measureValue.triggerBelow = objectIn[SFP(HStr_Key_TriggerBelow)] | dataAs.measureValue.triggerBelow;
            break;
        case (hid_t)HydroTrigger::MeasureRange:
            dataAs.measureRange.toleranceLow = objectIn[SFP(HStr_Key_ToleranceLow)] | dataAs.measureRange.toleranceLow;
            dataAs.measureRange.toleranceHigh = objectIn[SFP(HStr_Key_ToleranceHigh)] | dataAs.measureRange.toleranceHigh;
            dataAs.measureRange.triggerOutside = objectIn[SFP(HStr_Key_TriggerOutside)] | dataAs.measureRange.triggerOutside;
            break;
        default: break;
    }
    detriggerTol = objectIn[SFP(HStr_Key_DetriggerTol)] | detriggerTol;
    measurementUnits = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_MeasurementUnits)]);
}
