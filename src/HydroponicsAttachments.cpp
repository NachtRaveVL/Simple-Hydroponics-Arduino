/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

HydroponicsDLinkObject::HydroponicsDLinkObject()
    : _id(), _obj(nullptr)
{ ; }

HydroponicsDLinkObject::HydroponicsDLinkObject(const HydroponicsIdentity &id)
    : _id(id), _obj(nullptr)
{ ; }

HydroponicsDLinkObject::HydroponicsDLinkObject(const char *idKeyStr)
    : _id(HydroponicsIdentity(idKeyStr)), _obj(nullptr)
{ ; }


HydroponicsAttachment::HydroponicsAttachment(HydroponicsObject *parent)
    : _parent(parent), _obj()
{
    HYDRUINO_HARD_ASSERT(_parent, SFP(HS_Err_InvalidParameter));
}

HydroponicsAttachment::~HydroponicsAttachment()
{
    if (isResolved() && !getId().isSubObject()) {
        _obj->removeLinkage(_parent);
    }
}

void HydroponicsAttachment::attachObject()
{
    if (!getId().isSubObject()) {
        _obj->addLinkage(_parent);
    }
}

void HydroponicsAttachment::detachObject()
{
    if (!getId().isSubObject()) {
        _obj->removeLinkage(_parent);
    }
    _obj = nullptr;
}


HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>(
          parent,
          &HydroponicsSensor::getMeasurementSignal,
          MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::handleMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true), _processMethod(nullptr), _updateMethod(nullptr)
{ ; }

void HydroponicsSensorAttachment::attachObject()
{
    HydroponicsSignalAttachment<const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>::attachObject();

    if (isResolved()) {
        handleMeasurement(get()->getLatestMeasurement());
    }
}

void HydroponicsSensorAttachment::detachObject()
{
    HydroponicsSignalAttachment<const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>::detachObject();

    setNeedsMeasurement();
}

void HydroponicsSensorAttachment::updateMeasurementIfNeeded(bool poll)
{
    if (resolve() && (_needsMeasurement || poll)) {
        handleMeasurement(get()->getLatestMeasurement());
        get()->takeMeasurement((_needsMeasurement || poll));
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
    : HydroponicsSignalAttachment<Hydroponics_TriggerState, HYDRUINO_TRIGGER_STATE_SLOTS>(
        parent,
        &HydroponicsTrigger::getTriggerSignal,
        MethodSlot<HydroponicsTriggerAttachment,Hydroponics_TriggerState>(this, &HydroponicsTriggerAttachment::handleTrigger)),
      _triggerState(Hydroponics_TriggerState_Undefined), _needsTriggerState(true), _processMethod(nullptr), _updateMethod(nullptr)
{ ; }

HydroponicsTriggerAttachment::~HydroponicsTriggerAttachment()
{
    if (isResolved()) { get()->detachTrigger(); }
}

void HydroponicsTriggerAttachment::attachObject()
{
    HydroponicsSignalAttachment<Hydroponics_TriggerState, HYDRUINO_TRIGGER_STATE_SLOTS>::attachObject();

    handleTrigger(get()->getTriggerState());
}

void HydroponicsTriggerAttachment::detachObject()
{
    if (isResolved()) { get()->detachTrigger(); }

    HydroponicsSignalAttachment<Hydroponics_TriggerState, HYDRUINO_TRIGGER_STATE_SLOTS>::detachObject();
}

Hydroponics_TriggerState HydroponicsTriggerAttachment::updateTriggerIfNeeded(bool poll)
{
    if (resolve() && (_needsTriggerState || poll)) {
        handleTrigger(get()->getTriggerState());

        if (get()->getSensor()) {
            get()->getSensor()->takeMeasurement((_needsTriggerState || poll));
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
    : HydroponicsSignalAttachment<Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS>(
        parent,
        &HydroponicsBalancer::getBalancerSignal,
        MethodSlot<HydroponicsBalancerAttachment,Hydroponics_BalancerState>(this, &HydroponicsBalancerAttachment::handleBalancer)),
      _balancerState(Hydroponics_BalancerState_Undefined), _needsBalancerState(true), _processMethod(nullptr), _updateMethod(nullptr)
{ ; }

HydroponicsBalancerAttachment::~HydroponicsBalancerAttachment()
{
    if (isResolved()) { get()->setEnabled(false); }
}

void HydroponicsBalancerAttachment::attachObject()
{
    HydroponicsSignalAttachment<Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS>::attachObject();

    handleBalancer(get()->getBalancerState());
}

void HydroponicsBalancerAttachment::detachObject()
{
    if (isResolved()) { get()->setEnabled(false); }

    HydroponicsSignalAttachment<Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS>::detachObject();
}

Hydroponics_BalancerState HydroponicsBalancerAttachment::updateBalancerIfNeeded(bool poll)
{
    if (resolve() && (_needsBalancerState || poll)) {
        handleBalancer(get()->getBalancerState());
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
