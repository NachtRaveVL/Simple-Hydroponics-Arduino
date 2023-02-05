/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2003 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Attachment Points
*/

#include "Hydruino.h"

HydroDLinkObject::HydroDLinkObject()
    : _key((Hydro_KeyType)-1), _obj(nullptr), _keyStr(nullptr)
{ ; }

HydroDLinkObject::HydroDLinkObject(const HydroDLinkObject &obj)
    : _key(obj._key), _obj(obj._obj), _keyStr(nullptr)
{
    if (obj._keyStr) {
        auto len = strnlen(obj._keyStr, HYDRO_NAME_MAXSIZE);
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strncpy((char *)_keyStr, obj._keyStr, len + 1);
        }
    }
}

HydroDLinkObject::~HydroDLinkObject()
{
    if (_keyStr) { free((void *)_keyStr); }
}

void HydroDLinkObject::unresolve()
{
    if (_obj && !_keyStr) {
        auto id = _obj->getId();
        auto len = id.keyString.length();
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strncpy((char *)_keyStr, id.keyString.c_str(), len + 1);
        }
    }
    HYDRO_HARD_ASSERT(!_obj || _key == _obj->getKey(), SFP(HStr_Err_OperationFailure));
    _obj = nullptr;
}

SharedPtr<HydroObjInterface> HydroDLinkObject::_getObject()
{
    if (_obj) { return _obj; }
    if (_key == (Hydro_KeyType)-1) { return nullptr; }
    if (Hydruino::_activeInstance) {
        _obj = static_pointer_cast<HydroObjInterface>(Hydruino::_activeInstance->_objects[_key]);
    }
    if (_obj && _keyStr) {
        free((void *)_keyStr); _keyStr = nullptr;
    }
    return _obj;
}


HydroAttachment::HydroAttachment(HydroObjInterface *parent)
    : _parent(parent), _obj()
{ ; }

HydroAttachment::HydroAttachment(const HydroAttachment &attachment)
    : _parent(attachment._parent), _obj()
{
    setObject(attachment);
}

HydroAttachment::~HydroAttachment()
{
    if (_parent && isResolved()) {
        _obj->removeLinkage((HydroObject *)_parent);
    }
}

void HydroAttachment::attachObject()
{
    if (resolve() && _parent) { // purposeful resolve in front
        _obj->addLinkage((HydroObject *)_parent);
    }
}

void HydroAttachment::detachObject()
{
    if (_parent && isResolved()) {
        _obj->removeLinkage((HydroObject *)_parent);
    }
    // note: used to set _obj to nullptr here, but found that it's best not to -> avoids additional operator= calls during typical detach scenarios
}

void HydroAttachment::updateIfNeeded(bool poll)
{
    // intended to be overridden by derived classes, but not an error if left not implemented
}

void HydroAttachment::setParent(HydroObjInterface *parent)
{
    if (_parent != parent) {
        if (isResolved() && _parent) { _obj->removeLinkage((HydroObject *)_parent); }
        _parent = parent;
        if (isResolved() && _parent) { _obj->addLinkage((HydroObject *)_parent); }
    }
}


HydroSensorAttachment::HydroSensorAttachment(HydroObjInterface *parent, uint8_t measurementRow)
    : HydroSignalAttachment<const HydroMeasurement *, HYDRO_SENSOR_SIGNAL_SLOTS>(
          parent, &HydroSensor::getMeasurementSignal),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{
    setHandleMethod(&HydroSensorAttachment::handleMeasurement);
}

HydroSensorAttachment::HydroSensorAttachment(const HydroSensorAttachment &attachment)
    : HydroSignalAttachment<const HydroMeasurement *, HYDRO_SENSOR_SIGNAL_SLOTS>(attachment),
      _measurement(attachment._measurement), _measurementRow(attachment._measurementRow),
      _convertParam(attachment._convertParam), _needsMeasurement(attachment._needsMeasurement)
{
    setHandleSlot(*attachment._handleSlot);
}

HydroSensorAttachment::~HydroSensorAttachment()
{ ; }

void HydroSensorAttachment::attachObject()
{
    HydroSignalAttachment<const HydroMeasurement *, HYDRO_SENSOR_SIGNAL_SLOTS>::attachObject();

    if (_handleSlot) { _handleSlot->operator()(get()->getLatestMeasurement()); }
    else { handleMeasurement(get()->getLatestMeasurement()); }
}

void HydroSensorAttachment::detachObject()
{
    HydroSignalAttachment<const HydroMeasurement *, HYDRO_SENSOR_SIGNAL_SLOTS>::detachObject();

    setNeedsMeasurement();
}

void HydroSensorAttachment::updateIfNeeded(bool poll)
{
    if (resolve() && (_needsMeasurement || poll)) {
        if (_handleSlot) { _handleSlot->operator()(get()->getLatestMeasurement()); }
        else { handleMeasurement(get()->getLatestMeasurement()); }

        get()->takeMeasurement((_needsMeasurement || poll)); // purposeful recheck
    }
}

void HydroSensorAttachment::setMeasurement(HydroSingleMeasurement measurement)
{
    auto outUnits = definedUnitsElse(getMeasurementUnits(), measurement.units);
    _measurement = measurement;
    _measurement.setMinFrame(1);

    convertUnits(&_measurement, outUnits, _convertParam);
    _needsMeasurement = false;
}

void HydroSensorAttachment::setMeasurementRow(uint8_t measurementRow)
{
    if (_measurementRow != measurementRow) {
        _measurementRow = measurementRow;

        setNeedsMeasurement();
    }
}

void HydroSensorAttachment::setMeasurementUnits(Hydro_UnitsType units, float convertParam)
{
    if (_measurement.units != units || !isFPEqual(_convertParam, convertParam)) {
        _convertParam = convertParam;
        convertUnits(&_measurement, units, _convertParam);

        setNeedsMeasurement();
    }
}

void HydroSensorAttachment::handleMeasurement(const HydroMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        setMeasurement(getAsSingleMeasurement(measurement, _measurementRow));
    }
}


HydroTriggerAttachment::HydroTriggerAttachment(HydroObjInterface *parent)
    : HydroSignalAttachment<Hydro_TriggerState, HYDRO_TRIGGER_SIGNAL_SLOTS>(
        parent, &HydroTrigger::getTriggerSignal)
{ ; }

HydroTriggerAttachment::HydroTriggerAttachment(const HydroTriggerAttachment &attachment)
    : HydroSignalAttachment<Hydro_TriggerState, HYDRO_TRIGGER_SIGNAL_SLOTS>(attachment)
{ ; }

HydroTriggerAttachment::~HydroTriggerAttachment()
{ ; }

void HydroTriggerAttachment::updateIfNeeded(bool poll)
{
    if (resolve()) { get()->update(); }
}


HydroBalancerAttachment::HydroBalancerAttachment(HydroObjInterface *parent)
    : HydroSignalAttachment<Hydro_BalancingState, HYDRO_BALANCER_SIGNAL_SLOTS>(
        parent, &HydroBalancer::getBalancingSignal)
{ ; }

HydroBalancerAttachment::HydroBalancerAttachment(const HydroBalancerAttachment &attachment)
    : HydroSignalAttachment<Hydro_BalancingState, HYDRO_BALANCER_SIGNAL_SLOTS>(attachment)
{ ; }

HydroBalancerAttachment::~HydroBalancerAttachment()
{ ; }

void HydroBalancerAttachment::updateIfNeeded(bool poll)
{
    if (resolve()) { get()->update(); }
}


HydroActuatorAttachment::HydroActuatorAttachment(HydroObjInterface *parent)
    :  HydroSignalAttachment<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS>(
        parent, &HydroActuator::getActivationSignal),
       _actuatorHandle(), _updateSlot(nullptr), _rateMultiplier(1.0f)
{ ; }

HydroActuatorAttachment::HydroActuatorAttachment(const HydroActuatorAttachment &attachment)
    : HydroSignalAttachment<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS>(attachment),
      _updateSlot(attachment._updateSlot ? attachment._updateSlot->clone() : nullptr),
      _actuatorHandle(attachment._actuatorHandle), _rateMultiplier(attachment._rateMultiplier)
{ ; }

HydroActuatorAttachment::~HydroActuatorAttachment()
{
    if (_updateSlot) { delete _updateSlot; _updateSlot = nullptr; }
}

void HydroActuatorAttachment::updateIfNeeded(bool poll = false)
{
    if (isEnabled()) {
        _actuatorHandle.elapseBy(millis() - _actuatorHandle.checkTime);
        if (_updateSlot) {
            _updateSlot->operator()(&_actuatorHandle);
        }
    }
}

void HydroActuatorAttachment::setUpdateSlot(const Slot<HydroActivationHandle *> &updateSlot)
{
    if (!_updateSlot || !_updateSlot->operator==(&updateSlot)) {
        if (_updateSlot) { delete _updateSlot; _updateSlot = nullptr; }
        _updateSlot = updateSlot.clone();
    }
}
