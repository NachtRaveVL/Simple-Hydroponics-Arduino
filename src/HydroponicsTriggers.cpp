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


HydroponicsTrigger::HydroponicsTrigger(int typeIn)
    : type((typeof(type))typeIn), _triggerState(Hydroponics_TriggerState_Disabled), _attached(false)
{ ; }

HydroponicsTrigger::HydroponicsTrigger(const HydroponicsTriggerSubData *dataIn)
    : type((typeof(type))(dataIn->type)), _triggerState(Hydroponics_TriggerState_Disabled), _attached(false)
{ ; }

HydroponicsTrigger::~HydroponicsTrigger()
{
    //discardFromTaskManager(&_triggerSignal);
    if (_attached) { detachTrigger(); }
}

void HydroponicsTrigger::saveToData(HydroponicsSubData *dataOut) const
{
    return saveToData((HydroponicsTriggerSubData *)dataOut);
}

void HydroponicsTrigger::saveToData(HydroponicsTriggerSubData *dataOut) const
{
    dataOut->type = (int8_t)type;
}

void HydroponicsTrigger::update()
{
    if (!_attached) { attachTrigger(); }
}

void HydroponicsTrigger::resolveLinks()
{
    if (!_attached) { attachTrigger(); }
}

void HydroponicsTrigger::handleLowMemory()
{ ; }

Hydroponics_TriggerState HydroponicsTrigger::getTriggerState() const
{
    return _triggerState;
}

Signal<Hydroponics_TriggerState> &HydroponicsTrigger::getTriggerSignal()
{
    return _triggerSignal;
}

HydroponicsMeasurementValueTrigger::HydroponicsMeasurementValueTrigger(HydroponicsIdentity sensorId, float tolerance, bool triggerBelow, int measurementRow)
    : HydroponicsTrigger(MeasureValue), _sensor(sensorId),
      _tolerance(tolerance), _toleranceUnits(Hydroponics_UnitsType_Undefined),
      _triggerBelow(triggerBelow), _measurementRow((int8_t)measurementRow)
{ ; }

HydroponicsMeasurementValueTrigger::HydroponicsMeasurementValueTrigger(shared_ptr<HydroponicsSensor> sensor, float tolerance, bool triggerBelow, int measurementRow)
    : HydroponicsTrigger(MeasureValue), _sensor(sensor),
      _tolerance(tolerance), _toleranceUnits(Hydroponics_UnitsType_Undefined),
      _triggerBelow(triggerBelow), _measurementRow((int8_t)measurementRow)
{ ; }

HydroponicsMeasurementValueTrigger::HydroponicsMeasurementValueTrigger(const HydroponicsTriggerSubData *dataIn)
    : HydroponicsTrigger(dataIn), _sensor(dataIn->sensorName),
      _tolerance(dataIn->dataAs.measureValue.tolerance), _toleranceUnits(dataIn->toleranceUnits),
      _triggerBelow(dataIn->dataAs.measureValue.triggerBelow), _measurementRow(dataIn->measurementRow)
{ ; }

HydroponicsMeasurementValueTrigger::~HydroponicsMeasurementValueTrigger()
{ ; }

void HydroponicsMeasurementValueTrigger::saveToData(HydroponicsTriggerSubData *dataOut) const
{
    HydroponicsTrigger::saveToData(dataOut);

    if (_sensor.getId()) {
        strncpy(((HydroponicsTriggerSubData *)dataOut)->sensorName, _sensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureValue.tolerance = _tolerance;
    ((HydroponicsTriggerSubData *)dataOut)->toleranceUnits = _toleranceUnits;
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureValue.triggerBelow = _triggerBelow;
    ((HydroponicsTriggerSubData *)dataOut)->measurementRow = _measurementRow;
}

void HydroponicsMeasurementValueTrigger::resolveLinks()
{
    HydroponicsTrigger::resolveLinks();

    _sensor.resolveIfNeeded();
}

void HydroponicsMeasurementValueTrigger::attachTrigger()
{
    if (!_attached) {
        auto methodSlot = MethodSlot<HydroponicsMeasurementValueTrigger, HydroponicsMeasurement *>(this, &handleSensorMeasure);
        _sensor->getMeasurementSignal().attach(methodSlot);
        _attached = true;
    }
}

void HydroponicsMeasurementValueTrigger::detachTrigger()
{
    if (_attached) {
        auto methodSlot = MethodSlot<HydroponicsMeasurementValueTrigger, HydroponicsMeasurement *>(this, &handleSensorMeasure);
        _sensor->getMeasurementSignal().detach(methodSlot);
        _attached = false;
    }
}

void HydroponicsMeasurementValueTrigger::setToleranceUnits(Hydroponics_UnitsType units)
{
    _toleranceUnits = units;
}

float HydroponicsMeasurementValueTrigger::getTolerance() const
{
    return _tolerance;
}

Hydroponics_UnitsType HydroponicsMeasurementValueTrigger::getToleranceUnits() const
{
    return _toleranceUnits;
}

bool HydroponicsMeasurementValueTrigger::getTriggerBelow() const
{
    return _triggerBelow;
}

shared_ptr<HydroponicsSensor> HydroponicsMeasurementValueTrigger::getSensor()
{
    return _sensor.getObj();
}

void HydroponicsMeasurementValueTrigger::handleSensorMeasure(HydroponicsMeasurement *measurement)
{
    if (measurement) {
        bool fromDisabled = (_triggerState == Hydroponics_TriggerState_Disabled);
        bool newState = (_triggerState == Hydroponics_TriggerState_Triggered);

        if (measurement->isBinaryType()) {
            newState = ((HydroponicsBinaryMeasurement *)measurement)->state != _triggerBelow;
        } else {
            float measurementValue = 0.0f;
            Hydroponics_UnitsType measurementUnits = Hydroponics_UnitsType_Undefined;

            if (measurement->isSingleType()) {
                measurementValue = ((HydroponicsSingleMeasurement *)measurement)->value;
                measurementUnits = ((HydroponicsSingleMeasurement *)measurement)->units;
            } else if (measurement->isDoubleType()) {
                measurementValue = ((HydroponicsDoubleMeasurement *)measurement)->value[_measurementRow];
                measurementUnits = ((HydroponicsDoubleMeasurement *)measurement)->units[_measurementRow];
            } else if (measurement->isTripleType()) {
                measurementValue = ((HydroponicsTripleMeasurement *)measurement)->value[_measurementRow];
                measurementUnits = ((HydroponicsTripleMeasurement *)measurement)->units[_measurementRow];
            }

            if (_toleranceUnits != Hydroponics_UnitsType_Undefined && measurementUnits != _toleranceUnits) {
                convertStdUnits(&measurementValue, &measurementUnits, _toleranceUnits);
                HYDRUINO_SOFT_ASSERT(measurementUnits == _toleranceUnits, F("Failure converting measurement value to tolerance units"));
            }

            if (_toleranceUnits == Hydroponics_UnitsType_Undefined || measurementUnits == _toleranceUnits) {
                newState = (_triggerBelow ? measurementValue <= _tolerance + FLT_EPSILON
                                          : measurementValue >= _tolerance - FLT_EPSILON);
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


HydroponicsMeasurementRangeTrigger::HydroponicsMeasurementRangeTrigger(HydroponicsIdentity sensorId, float toleranceLow, float toleranceHigh, bool triggerOutside, int measurementRow)
    : HydroponicsTrigger(MeasureRange), _sensor(sensorId),
      _toleranceLow(toleranceLow), _toleranceHigh(toleranceHigh),
      _triggerOutside(triggerOutside), _measurementRow(measurementRow)
{ ; }

HydroponicsMeasurementRangeTrigger::HydroponicsMeasurementRangeTrigger(shared_ptr<HydroponicsSensor> sensor, float toleranceLow, float toleranceHigh, bool triggerOutside, int measurementRow)
    : HydroponicsTrigger(MeasureRange), _sensor(sensor),
      _toleranceLow(toleranceLow), _toleranceHigh(toleranceHigh),
      _triggerOutside(triggerOutside), _measurementRow(measurementRow)
{ ; }

HydroponicsMeasurementRangeTrigger::HydroponicsMeasurementRangeTrigger(const HydroponicsTriggerSubData *dataIn)
    : HydroponicsTrigger(MeasureRange), _sensor(dataIn->sensorName),
      _toleranceLow(dataIn->dataAs.measureRange.toleranceLow),
      _toleranceHigh(dataIn->dataAs.measureRange.toleranceHigh),
      _toleranceUnits(dataIn->toleranceUnits),
      _triggerOutside(dataIn->dataAs.measureRange.triggerOutside),
      _measurementRow(dataIn->measurementRow)
{ ; }

HydroponicsMeasurementRangeTrigger::~HydroponicsMeasurementRangeTrigger()
{ ; }

void HydroponicsMeasurementRangeTrigger::saveToData(HydroponicsTriggerSubData *dataOut) const
{
    HydroponicsTrigger::saveToData(dataOut);

    if (_sensor.getId()) {
        strncpy(((HydroponicsTriggerSubData *)dataOut)->sensorName, _sensor.getId().keyStr.c_str(), HYDRUINO_NAME_MAXSIZE);
    }
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureRange.toleranceLow = _toleranceLow;
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureRange.toleranceHigh = _toleranceHigh;
    ((HydroponicsTriggerSubData *)dataOut)->toleranceUnits = _toleranceUnits;
    ((HydroponicsTriggerSubData *)dataOut)->dataAs.measureRange.triggerOutside = _triggerOutside;
    ((HydroponicsTriggerSubData *)dataOut)->measurementRow = _measurementRow;
}

void HydroponicsMeasurementRangeTrigger::resolveLinks()
{
    HydroponicsTrigger::resolveLinks();

    _sensor.resolveIfNeeded();
}

void HydroponicsMeasurementRangeTrigger::attachTrigger()
{
    if (!_attached) {
        auto methodSlot = MethodSlot<HydroponicsMeasurementRangeTrigger, HydroponicsMeasurement *>(this, &handleSensorMeasure);
        _sensor->getMeasurementSignal().attach(methodSlot);
        _attached = true;
    }
}

void HydroponicsMeasurementRangeTrigger::detachTrigger()
{
    if (_attached) {
        auto methodSlot = MethodSlot<HydroponicsMeasurementRangeTrigger, HydroponicsMeasurement *>(this, &handleSensorMeasure);
        _sensor->getMeasurementSignal().detach(methodSlot);
        _attached = false;
    }
}

void HydroponicsMeasurementRangeTrigger::setToleranceUnits(Hydroponics_UnitsType units)
{
    _toleranceUnits = units;
}

shared_ptr<HydroponicsSensor> HydroponicsMeasurementRangeTrigger::getSensor()
{
    return _sensor.getObj();
}

float HydroponicsMeasurementRangeTrigger::getToleranceLow() const
{
    return _toleranceLow;
}

float HydroponicsMeasurementRangeTrigger::getToleranceHigh() const
{
    return _toleranceHigh;
}

Hydroponics_UnitsType HydroponicsMeasurementRangeTrigger::getToleranceUnits() const
{
    return _toleranceUnits;
}

void HydroponicsMeasurementRangeTrigger::handleSensorMeasure(HydroponicsMeasurement *measurement)
{
    if (measurement) {
        bool fromDisabled = (_triggerState == Hydroponics_TriggerState_Disabled);
        bool newState = (_triggerState == Hydroponics_TriggerState_Triggered);
        float measurementValue = 0.0f;
        Hydroponics_UnitsType measurementUnits = Hydroponics_UnitsType_Undefined;

        if (measurement->isBinaryType()) {
            measurementValue = ((HydroponicsBinaryMeasurement *)measurement)->state * 1.0f;
            measurementUnits = Hydroponics_UnitsType_Raw_0_1;
        } else if (measurement->isSingleType()) {
            measurementValue = ((HydroponicsSingleMeasurement *)measurement)->value;
            measurementUnits = ((HydroponicsSingleMeasurement *)measurement)->units;
        } else if (measurement->isDoubleType()) {
            measurementValue = ((HydroponicsDoubleMeasurement *)measurement)->value[_measurementRow];
            measurementUnits = ((HydroponicsDoubleMeasurement *)measurement)->units[_measurementRow];
        } else if (measurement->isTripleType()) {
            measurementValue = ((HydroponicsTripleMeasurement *)measurement)->value[_measurementRow];
            measurementUnits = ((HydroponicsTripleMeasurement *)measurement)->units[_measurementRow];
        }

        if (_toleranceUnits != Hydroponics_UnitsType_Undefined && measurementUnits != _toleranceUnits) {
            convertStdUnits(&measurementValue, &measurementUnits, _toleranceUnits);
            HYDRUINO_SOFT_ASSERT(measurementUnits == _toleranceUnits, F("Failure converting measurement value to tolerance units"));
        }

        if (_toleranceUnits == Hydroponics_UnitsType_Undefined || measurementUnits == _toleranceUnits) {
            bool isInside = (measurementValue >= _toleranceLow - FLT_EPSILON &&
                             measurementValue <= _toleranceHigh + FLT_EPSILON);
            newState = _triggerOutside ? !isInside : isInside;
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
    : HydroponicsSubData(), sensorName{0}, dataAs{.measureRange={0.0f,0.0f,false}}, toleranceUnits(Hydroponics_UnitsType_Undefined), measurementRow(0)
{ ; }

void HydroponicsTriggerSubData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSubData::toJSONObject(objectOut);

    objectOut[F("sensorName")] = stringFromChars(sensorName, HYDRUINO_NAME_MAXSIZE);
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
    if (toleranceUnits != Hydroponics_UnitsType_Undefined) {
        objectOut[F("toleranceUnits")] = toleranceUnits;
    }
    if (measurementRow > 0) {
        objectOut[F("measurementRow")] = measurementRow;
    }
}

void HydroponicsTriggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSubData::fromJSONObject(objectIn);

    const char *sensorNameStr = objectIn[F("sensorName")];
    if (sensorNameStr && sensorNameStr[0]) { strncpy(sensorName, sensorNameStr, HYDRUINO_NAME_MAXSIZE); }
    switch (type) {
        case 0: // MeasureValue
            dataAs.measureValue.tolerance = objectIn[F("tolerance")];
            dataAs.measureValue.triggerBelow = objectIn[F("triggerBelow")];
            break;
        case 1: // MeasureRange
            dataAs.measureRange.toleranceLow = objectIn[F("toleranceLow")];
            dataAs.measureRange.toleranceHigh = objectIn[F("toleranceHigh")];
            dataAs.measureRange.triggerOutside = objectIn[F("triggerOutside")];
            break;
        default: break;
    }
    toleranceUnits = objectIn[F("toleranceUnits")];
    measurementRow = objectIn[F("measurementRow")];
}
