/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>(
          parent,
          &HydroponicsSensor::getMeasurementSignal,
          MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::handleMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true), _processMethod(nullptr), _updateMethod(nullptr)
{ ; }

void HydroponicsSensorAttachment::attachObject()
{
    HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>::attachObject();

    if (isResolved()) {
        handleMeasurement(_obj->getLatestMeasurement());
    }
}

void HydroponicsSensorAttachment::detachObject()
{
    HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>::detachObject();

    setNeedsMeasurement();
}

void HydroponicsSensorAttachment::updateMeasurementIfNeeded(bool force)
{
    if (getObject() && (_needsMeasurement || force)) {
        handleMeasurement(_obj->getLatestMeasurement());
        _obj->takeMeasurement((_needsMeasurement || force));
    }
}

void HydroponicsSensorAttachment::setMeasurement(float value, Hydroponics_UnitsType units)
{
    auto outUnits = definedUnitsElse(_measurement.units, units);
    _measurement.value = value;
    _measurement.units = units;
    _measurement.updateTimestamp();
    _measurement.updateFrame(1);

    convertUnits(&_measurement, outUnits, _convertParam);
    _needsMeasurement = false;

    if (_updateMethod) { (_parent->*_updateMethod)(_measurement); }
}

void HydroponicsSensorAttachment::setMeasurement(HydroponicsSingleMeasurement measurement)
{
    auto outUnits = definedUnitsElse(_measurement.units, measurement.units);
    _measurement = measurement;
    _measurement.setMinFrame(1);

    convertUnits(&_measurement, outUnits, _convertParam);
    _needsMeasurement = false;

    if (_updateMethod) { (_parent->*_updateMethod)(_measurement); }
}

void HydroponicsSensorAttachment::setMeasurementRow(Hydroponics_PositionIndex measurementRow)
{
    if (_measurementRow != measurementRow) {
        _measurementRow = measurementRow;
        setNeedsMeasurement();
    }
}

void HydroponicsSensorAttachment::setMeasurementUnits(Hydroponics_UnitsType units, float convertParam)
{
    if (_measurement.units != units || !isFPEqual(_convertParam, convertParam)) {
        _convertParam = convertParam;
        convertUnits(&_measurement, units, _convertParam);
        setNeedsMeasurement();
    }
}

void HydroponicsSensorAttachment::handleMeasurement(const HydroponicsMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        if (_processMethod) {
            (_parent->*_processMethod)(measurement);
        } else {
            setMeasurement(getAsSingleMeasurement(measurement, _measurementRow));
        }
    }
}


HydroponicsTriggerAttachment::HydroponicsTriggerAttachment(HydroponicsObject *parent)
    : HydroponicsSignalAttachment<HydroponicsTrigger, Hydroponics_TriggerState, HYDRUINO_TRIGGER_STATE_SLOTS>(
        parent,
        &HydroponicsTrigger::getTriggerSignal,
        MethodSlot<HydroponicsTriggerAttachment,Hydroponics_TriggerState>(this, &HydroponicsTriggerAttachment::handleTrigger)),
      _triggerState(Hydroponics_TriggerState_Undefined), _needsTriggerState(true), _processMethod(nullptr), _updateMethod(nullptr)
{ ; }

HydroponicsTriggerAttachment::~HydroponicsTriggerAttachment()
{
    if (isResolved()) { _obj->detachTrigger(); }
}

void HydroponicsTriggerAttachment::attachObject()
{
    HydroponicsSignalAttachment<HydroponicsTrigger, Hydroponics_TriggerState, HYDRUINO_TRIGGER_STATE_SLOTS>::attachObject();

    handleTrigger(_obj->getTriggerState());
}

void HydroponicsTriggerAttachment::detachObject()
{
    if (isResolved()) { _obj->detachTrigger(); }

    HydroponicsSignalAttachment<HydroponicsTrigger, Hydroponics_TriggerState, HYDRUINO_TRIGGER_STATE_SLOTS>::detachObject();
}

Hydroponics_TriggerState HydroponicsTriggerAttachment::updateTriggerIfNeeded(bool force)
{
    if (getObject() && (_needsTriggerState || force)) {
        handleTrigger(_obj->getTriggerState());

        if (_obj->getSensor()) {
            _obj->getSensor()->takeMeasurement((_needsTriggerState || force));
        }
    }
    return _triggerState;
}

void HydroponicsTriggerAttachment::setTriggerState(Hydroponics_TriggerState triggerState)
{
    _needsTriggerState = false;
    if (_triggerState != triggerState) {
        _triggerState = triggerState;

        if (_updateMethod) { (_parent->*_updateMethod)(triggerState); }
    }
}

void HydroponicsTriggerAttachment::handleTrigger(Hydroponics_TriggerState triggerState)
{
    if (_processMethod) {
        (_parent->*_processMethod)(triggerState);
    } else {
        setTriggerState(triggerState);
    }
}


HydroponicsBalancerAttachment::HydroponicsBalancerAttachment(HydroponicsObject *parent)
    : HydroponicsSignalAttachment<HydroponicsBalancer, Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS>(
        parent,
        &HydroponicsBalancer::getBalancerSignal,
        MethodSlot<HydroponicsBalancerAttachment,Hydroponics_BalancerState>(this, &HydroponicsBalancerAttachment::handleBalancer)),
      _balancerState(Hydroponics_BalancerState_Undefined), _needsBalancerState(true), _processMethod(nullptr), _updateMethod(nullptr)
{ ; }

HydroponicsBalancerAttachment::~HydroponicsBalancerAttachment()
{
    if (isResolved()) { _obj->setEnabled(false); }
}

void HydroponicsBalancerAttachment::attachObject()
{
    HydroponicsSignalAttachment<HydroponicsBalancer, Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS>::attachObject();

    handleBalancer(_obj->getBalancerState());
}

void HydroponicsBalancerAttachment::detachObject()
{
    if (isResolved()) { _obj->setEnabled(false); }

    HydroponicsSignalAttachment<HydroponicsBalancer, Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS>::detachObject();
}

Hydroponics_BalancerState HydroponicsBalancerAttachment::updateBalancerIfNeeded(bool force)
{
    if (getObject() && (_needsBalancerState || force)) {
        handleBalancer(_obj->getBalancerState());
    }
    return _balancerState;
}

void HydroponicsBalancerAttachment::setBalancerState(Hydroponics_BalancerState balancerState)
{
    _needsBalancerState = false;
    if (_balancerState != balancerState) {
        _balancerState = balancerState;

        if (_updateMethod) { (_parent->*_updateMethod)(balancerState); }
    }
}

void HydroponicsBalancerAttachment::handleBalancer(Hydroponics_BalancerState balancerState)
{
    if (_processMethod) {
        (_parent->*_processMethod)(balancerState);
    } else {
        setBalancerState(balancerState);
    }
}
