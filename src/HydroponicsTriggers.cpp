/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Triggers
*/

#include "Hydroponics.h"

// Creates trigger object from passed trigger data
HydroponicsTrigger *newTriggerObjectFromSubData(const HydroponicsTriggerSubData *dataIn)
{
    if (dataIn && dataIn->type == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->type >= 0, F("Invalid data"));

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


HydroponicsTrigger::HydroponicsTrigger(HydroponicsIdentity sensorId, int measurementRow, int typeIn)
    : type((typeof(type))typeIn), _sensor(sensorId), _measurementRow(measurementRow), _attached(false),
      _toleranceUnits(Hydroponics_UnitsType_Undefined), _triggerState(Hydroponics_TriggerState_Disabled)
{ ; }

HydroponicsTrigger::HydroponicsTrigger(shared_ptr<HydroponicsSensor> sensor, int measurementRow, int typeIn)
    : type((typeof(type))typeIn), _sensor(sensor), _measurementRow(measurementRow), _attached(false),
      _toleranceUnits(Hydroponics_UnitsType_Undefined), _triggerState(Hydroponics_TriggerState_Disabled)
{ ; }

HydroponicsTrigger::HydroponicsTrigger(const HydroponicsTriggerSubData *dataIn)
    : type((typeof(type))(dataIn->type)), _sensor(dataIn->sensorName), _measurementRow(dataIn->measurementRow), _attached(false),
      _toleranceUnits(dataIn->toleranceUnits), _triggerState(Hydroponics_TriggerState_Disabled)
{ ; }

HydroponicsTrigger::~HydroponicsTrigger()
{
    //discardFromTaskManager(&_triggerSignal);
}

void HydroponicsTrigger::saveToData(HydroponicsTriggerSubData *dataOut) const
{
    ((HydroponicsTriggerSubData *)dataOut)->type = (int8_t)type;
    if (_sensor.getId()) {
        strncpy(((HydroponicsTriggerSubData *)dataOut)->sensorName, _sensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    ((HydroponicsTriggerSubData *)dataOut)->measurementRow = _measurementRow;
    ((HydroponicsTriggerSubData *)dataOut)->toleranceUnits = _toleranceUnits;
}

void HydroponicsTrigger::update()
{
    if (!_attached) { attachTrigger(); }
}

void HydroponicsTrigger::resolveLinks()
{
    _sensor.resolveIfNeeded();
    if (!_attached) { attachTrigger(); }
}

void HydroponicsTrigger::handleLowMemory()
{ ; }

Hydroponics_TriggerState HydroponicsTrigger::getTriggerState() const
{
    return _triggerState;
}

void HydroponicsTrigger::setToleranceUnits(Hydroponics_UnitsType units)
{
    _toleranceUnits = units;
}

Hydroponics_UnitsType HydroponicsTrigger::getToleranceUnits() const
{
    return _toleranceUnits;
}

shared_ptr<HydroponicsSensor> HydroponicsTrigger::getSensor()
{
    return _sensor.getObj();
}

int HydroponicsTrigger::getMeasurementRow() const
{
    return _measurementRow;
}

Signal<Hydroponics_TriggerState> &HydroponicsTrigger::getTriggerSignal()
{
    return _triggerSignal;
}


HydroponicsMeasurementValueTrigger::HydroponicsMeasurementValueTrigger(HydroponicsIdentity sensorId, float tolerance, bool triggerBelow, float detriggerTolerance, int measurementRow)
    : HydroponicsTrigger(sensorId, measurementRow, MeasureValue),
      _triggerTolerance(tolerance), _detriggerTolerance(detriggerTolerance), _triggerBelow(triggerBelow)
{ ; }

HydroponicsMeasurementValueTrigger::HydroponicsMeasurementValueTrigger(shared_ptr<HydroponicsSensor> sensor, float tolerance, bool triggerBelow, float detriggerTolerance, int measurementRow)
    : HydroponicsTrigger(sensor, measurementRow, MeasureValue),
      _triggerTolerance(tolerance), _detriggerTolerance(detriggerTolerance), _triggerBelow(triggerBelow)
{ ; }

HydroponicsMeasurementValueTrigger::HydroponicsMeasurementValueTrigger(const HydroponicsTriggerSubData *dataIn)
    : HydroponicsTrigger(dataIn),
      _triggerTolerance(dataIn->dataAs.measureValue.tolerance), _detriggerTolerance(dataIn->detriggerTolerance),
      _triggerBelow(dataIn->dataAs.measureValue.triggerBelow)
{ ; }

HydroponicsMeasurementValueTrigger::~HydroponicsMeasurementValueTrigger()
{
    if (_attached) { detachTrigger(); }
}

void HydroponicsMeasurementValueTrigger::saveToData(HydroponicsTriggerSubData *dataOut) const
{
    HydroponicsTrigger::saveToData(dataOut);

    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureValue.tolerance = _triggerTolerance;
    ((HydroponicsTriggerSubData *)dataOut)->detriggerTolerance = _detriggerTolerance;
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureValue.triggerBelow = _triggerBelow;
}

void HydroponicsMeasurementValueTrigger::attachTrigger()
{
    if (!_attached) {
        auto methodSlot = MethodSlot<HydroponicsMeasurementValueTrigger, const HydroponicsMeasurement *>(this, &handleSensorMeasure);
        _sensor->getMeasurementSignal().attach(methodSlot);
        _attached = true;
    }
}

void HydroponicsMeasurementValueTrigger::detachTrigger()
{
    if (_attached) {
        auto methodSlot = MethodSlot<HydroponicsMeasurementValueTrigger, const HydroponicsMeasurement *>(this, &handleSensorMeasure);
        _sensor->getMeasurementSignal().detach(methodSlot);
        _attached = false;
    }
}

void HydroponicsMeasurementValueTrigger::setTriggerTolerance(float tolerance)
{
    if (!isFPEqual(_triggerTolerance, tolerance)) {
        _triggerTolerance = tolerance;

        if (_triggerState != Hydroponics_TriggerState_Disabled && getSensor()) {
            handleSensorMeasure(getSensor()->getLatestMeasurement());
        }
    }
}

float HydroponicsMeasurementValueTrigger::getTriggerTolerance() const
{
    return _triggerTolerance;
}

float HydroponicsMeasurementValueTrigger::getDetriggerTolerance() const
{
    return _detriggerTolerance;
}

bool HydroponicsMeasurementValueTrigger::getTriggerBelow() const
{
    return _triggerBelow;
}

void HydroponicsMeasurementValueTrigger::handleSensorMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement) {
        bool fromDisabled = (_triggerState == Hydroponics_TriggerState_Disabled);
        bool newState = (_triggerState == Hydroponics_TriggerState_Triggered);

        if (measurement->isBinaryType()) {
            newState = ((HydroponicsBinaryMeasurement *)measurement)->state != _triggerBelow;
        } else {
            auto measurementValue = measurementValueAt(measurement, _measurementRow);
            auto measurementUnits = measurementUnitsAt(measurement, _measurementRow);

            if (_toleranceUnits != Hydroponics_UnitsType_Undefined && measurementUnits != _toleranceUnits) {
                convertStdUnits(&measurementValue, &measurementUnits, _toleranceUnits);
                HYDRUINO_SOFT_ASSERT(measurementUnits == _toleranceUnits, F("Failure converting measurement value to tolerance units"));
            }

            if (_toleranceUnits == Hydroponics_UnitsType_Undefined || measurementUnits == _toleranceUnits) {
                float tolAdditive = (_triggerState == Hydroponics_TriggerState_Triggered ? _detriggerTolerance : 0);
                newState = (_triggerBelow ? measurementValue <= _triggerTolerance + tolAdditive + FLT_EPSILON
                                          : measurementValue >= _triggerTolerance - tolAdditive - FLT_EPSILON);
            }
        }

        if (newState != (_triggerState == Hydroponics_TriggerState_Triggered) || fromDisabled) {
            _triggerState = newState ? Hydroponics_TriggerState_Triggered : Hydroponics_TriggerState_NotTriggered;

            if (!fromDisabled) {
                scheduleSignalFireOnce<Hydroponics_TriggerState>(_triggerSignal, _triggerState);
            }
        }
    }
}


HydroponicsMeasurementRangeTrigger::HydroponicsMeasurementRangeTrigger(HydroponicsIdentity sensorId, float toleranceLow, float toleranceHigh, bool triggerOutside, float detriggerTolerance, int measurementRow)
    : HydroponicsTrigger(sensorId, measurementRow, MeasureRange),
      _triggerToleranceLow(toleranceLow), _triggerToleranceHigh(toleranceHigh), _detriggerTolerance(detriggerTolerance),
      _triggerOutside(triggerOutside)
{ ; }

HydroponicsMeasurementRangeTrigger::HydroponicsMeasurementRangeTrigger(shared_ptr<HydroponicsSensor> sensor, float toleranceLow, float toleranceHigh, bool triggerOutside, float detriggerTolerance, int measurementRow)
    : HydroponicsTrigger(sensor, measurementRow, MeasureRange),
      _triggerToleranceLow(toleranceLow), _triggerToleranceHigh(toleranceHigh), _detriggerTolerance(detriggerTolerance),
      _triggerOutside(triggerOutside)
{ ; }

HydroponicsMeasurementRangeTrigger::HydroponicsMeasurementRangeTrigger(const HydroponicsTriggerSubData *dataIn)
    : HydroponicsTrigger(dataIn),
      _triggerToleranceLow(dataIn->dataAs.measureRange.toleranceLow),
      _triggerToleranceHigh(dataIn->dataAs.measureRange.toleranceHigh),
      _detriggerTolerance(dataIn->detriggerTolerance),
      _triggerOutside(dataIn->dataAs.measureRange.triggerOutside)
{ ; }

HydroponicsMeasurementRangeTrigger::~HydroponicsMeasurementRangeTrigger()
{
    if (_attached) { detachTrigger(); }
}

void HydroponicsMeasurementRangeTrigger::saveToData(HydroponicsTriggerSubData *dataOut) const
{
    HydroponicsTrigger::saveToData(dataOut);

    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureRange.toleranceLow = _triggerToleranceLow;
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureRange.toleranceHigh = _triggerToleranceHigh;
    ((HydroponicsTriggerSubData *)dataOut)->detriggerTolerance = _detriggerTolerance;
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureRange.triggerOutside = _triggerOutside;
}

void HydroponicsMeasurementRangeTrigger::attachTrigger()
{
    if (!_attached) {
        auto methodSlot = MethodSlot<HydroponicsMeasurementRangeTrigger, const HydroponicsMeasurement *>(this, &handleSensorMeasure);
        _sensor->getMeasurementSignal().attach(methodSlot);
        _attached = true;
    }
}

void HydroponicsMeasurementRangeTrigger::detachTrigger()
{
    if (_attached) {
        auto methodSlot = MethodSlot<HydroponicsMeasurementRangeTrigger, const HydroponicsMeasurement *>(this, &handleSensorMeasure);
        _sensor->getMeasurementSignal().detach(methodSlot);
        _attached = false;
    }
}

void HydroponicsMeasurementRangeTrigger::setTriggerToleranceMid(float toleranceMid)
{
    float toleranceRangeHalf = (_triggerToleranceHigh - _triggerToleranceLow) * 0.5f;

    if (!isFPEqual(_triggerToleranceLow, toleranceMid - toleranceRangeHalf)) {
        _triggerToleranceLow = toleranceMid - toleranceRangeHalf;
        _triggerToleranceHigh = toleranceMid + toleranceRangeHalf;

        if (_triggerState != Hydroponics_TriggerState_Disabled && getSensor()) {
            handleSensorMeasure(getSensor()->getLatestMeasurement());
        }
    }
}

float HydroponicsMeasurementRangeTrigger::getTriggerToleranceLow() const
{
    return _triggerToleranceLow;
}

float HydroponicsMeasurementRangeTrigger::getTriggerToleranceHigh() const
{
    return _triggerToleranceHigh;
}

float HydroponicsMeasurementRangeTrigger::getDetriggerTolerance() const
{
    return _detriggerTolerance;
}

bool HydroponicsMeasurementRangeTrigger::getTriggerOutside() const
{
    return _triggerOutside;
}

void HydroponicsMeasurementRangeTrigger::handleSensorMeasure(const HydroponicsMeasurement *measurement)
{
    if (measurement) {
        bool fromDisabled = (_triggerState == Hydroponics_TriggerState_Disabled);
        bool newState = (_triggerState == Hydroponics_TriggerState_Triggered);
        auto measurementValue = measurementValueAt(measurement, _measurementRow);
        auto measurementUnits = measurementUnitsAt(measurement, _measurementRow);

        if (_toleranceUnits != Hydroponics_UnitsType_Undefined && measurementUnits != _toleranceUnits) {
            convertStdUnits(&measurementValue, &measurementUnits, _toleranceUnits);
            HYDRUINO_SOFT_ASSERT(measurementUnits == _toleranceUnits, F("Failure converting measurement value to tolerance units"));
        }

        if (_toleranceUnits == Hydroponics_UnitsType_Undefined || measurementUnits == _toleranceUnits) {
            float tolAdditive = (_triggerState == Hydroponics_TriggerState_Triggered ? _detriggerTolerance : 0);

            if (!_triggerOutside) {
                newState = (measurementValue >= _triggerToleranceLow - tolAdditive - FLT_EPSILON &&
                            measurementValue <= _triggerToleranceHigh + tolAdditive + FLT_EPSILON);
            } else {
                newState = (measurementValue <= _triggerToleranceLow + tolAdditive + FLT_EPSILON &&
                            measurementValue >= _triggerToleranceHigh - tolAdditive - FLT_EPSILON);
            }
        }

        if (newState != (_triggerState == Hydroponics_TriggerState_Triggered) || fromDisabled) {
            _triggerState = newState ? Hydroponics_TriggerState_Triggered : Hydroponics_TriggerState_NotTriggered;

            if (!fromDisabled) {
                scheduleSignalFireOnce<Hydroponics_TriggerState>(_triggerSignal, _triggerState);
            }
        }
    }
}


HydroponicsTriggerSubData::HydroponicsTriggerSubData()
    : HydroponicsSubData(), sensorName{0}, measurementRow(0), dataAs{.measureRange={0.0f,0.0f,false}}, detriggerTolerance(0), toleranceUnits(Hydroponics_UnitsType_Undefined)
{ ; }

void HydroponicsTriggerSubData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSubData::toJSONObject(objectOut);

    if (sensorName[0]) { objectOut[F("sensorName")] = stringFromChars(sensorName, HYDRUINO_NAME_MAXSIZE); }
    if (measurementRow > 0) { objectOut[F("measurementRow")] = measurementRow; }
    switch (type) {
        case 0: // MeasureValue
            objectOut[F("tolerance")] = dataAs.measureValue.tolerance;
            objectOut[F("triggerBelow")] = dataAs.measureValue.triggerBelow;
            break;
        case 1: // MeasureRange
            objectOut[F("toleranceLow")] = dataAs.measureRange.toleranceLow;
            objectOut[F("toleranceHigh")] = dataAs.measureRange.toleranceHigh;
            objectOut[F("triggerOutside")] = dataAs.measureRange.triggerOutside;
            break;
        default: break;
    }
    if (detriggerTolerance > 0) { objectOut[F("detriggerTolerance")] = detriggerTolerance; }
    if (toleranceUnits != Hydroponics_UnitsType_Undefined) { objectOut[F("toleranceUnits")] = toleranceUnits; }
}

void HydroponicsTriggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSubData::fromJSONObject(objectIn);

    const char *sensorNameStr = objectIn[F("sensorName")];
    if (sensorNameStr && sensorNameStr[0]) { strncpy(sensorName, sensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    measurementRow = objectIn[F("measurementRow")] | measurementRow;
    switch (type) {
        case 0: // MeasureValue
            dataAs.measureValue.tolerance = objectIn[F("tolerance")] | dataAs.measureValue.tolerance;
            dataAs.measureValue.triggerBelow = objectIn[F("triggerBelow")] | dataAs.measureValue.triggerBelow;
            break;
        case 1: // MeasureRange
            dataAs.measureRange.toleranceLow = objectIn[F("toleranceLow")] | dataAs.measureRange.toleranceLow;
            dataAs.measureRange.toleranceHigh = objectIn[F("toleranceHigh")] | dataAs.measureRange.toleranceHigh;
            dataAs.measureRange.triggerOutside = objectIn[F("triggerOutside")] | dataAs.measureRange.triggerOutside;
            break;
        default: break;
    }
    detriggerTolerance = objectIn[F("detriggerTolerance")] | detriggerTolerance;
    toleranceUnits = objectIn[F("toleranceUnits")] | toleranceUnits;
}
