/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Attachment Points
*/

#include "Hydruino.h"

HydroDLinkObject::HydroDLinkObject()
    : _key(hkey_none), _obj(nullptr), _keyStr(nullptr)
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
    if (_key == hkey_none) { return nullptr; }
    if (Hydruino::_activeInstance) {
        _obj = static_pointer_cast<HydroObjInterface>(Hydruino::_activeInstance->_objects[_key]);
    }
    if (_obj && _keyStr) {
        free((void *)_keyStr); _keyStr = nullptr;
    }
    return _obj;
}


HydroAttachment::HydroAttachment(HydroObjInterface *parent)
    : HydroSubObject(parent), _obj()
{ ; }

HydroAttachment::HydroAttachment(const HydroAttachment &attachment)
    : HydroSubObject(attachment._parent), _obj()
{
    setObject(attachment._obj);
}

HydroAttachment::~HydroAttachment()
{
    if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<HydroObject>()->removeLinkage((HydroObject *)_parent);
    }
}

void HydroAttachment::attachObject()
{
    if (resolve() && _obj->isObject() && _parent && _parent->isObject()) { // purposeful resolve in front
        _obj.get<HydroObject>()->addLinkage((HydroObject *)_parent);
    }
}

void HydroAttachment::detachObject()
{
    if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<HydroObject>()->removeLinkage((HydroObject *)_parent);
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
        if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) { _obj.get<HydroObject>()->removeLinkage((HydroObject *)_parent); }

        _parent = parent;

        if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) { _obj.get<HydroObject>()->addLinkage((HydroObject *)_parent); }
    }
}


HydroActuatorAttachment::HydroActuatorAttachment(HydroObjInterface *parent)
    :  HydroSignalAttachment<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS>(parent, &HydroActuator::getActivationSignal),
       _actHandle(), _actSetup(), _updateSlot(nullptr), _rateMultiplier(1.0f), _calledLastUpdate(false)
{ ; }

HydroActuatorAttachment::HydroActuatorAttachment(const HydroActuatorAttachment &attachment)
    : HydroSignalAttachment<HydroActuator *, HYDRO_ACTUATOR_SIGNAL_SLOTS>(attachment),
      _actHandle(attachment._actHandle), _actSetup(attachment._actSetup),
      _updateSlot(attachment._updateSlot ? attachment._updateSlot->clone() : nullptr),
      _rateMultiplier(attachment._rateMultiplier), _calledLastUpdate(false)
{ ; }

HydroActuatorAttachment::~HydroActuatorAttachment()
{
    if (_updateSlot) { delete _updateSlot; _updateSlot = nullptr; }
}

void HydroActuatorAttachment::updateIfNeeded(bool poll)
{
    if (_actHandle.isValid()) {
        if (isActivated()) {
            _actHandle.elapseTo();
            if (_updateSlot) { _updateSlot->operator()(this); }
            _calledLastUpdate = _actHandle.isDone();
        } else if (_actHandle.isDone() && !_calledLastUpdate) {
            if (_updateSlot) { _updateSlot->operator()(this); }
            _calledLastUpdate = true;
        }
    }
}

void HydroActuatorAttachment::setupActivation(float value, millis_t duration, bool force)
{
    if (resolve()) {
        value = get()->calibrationInvTransform(value);

        if (get()->isDirectionalType()) {
            setupActivation(HydroActivation(value > FLT_EPSILON ? Hydro_DirectionMode_Forward : value < -FLT_EPSILON ? Hydro_DirectionMode_Reverse : Hydro_DirectionMode_Stop, fabsf(value), duration, (force ? Hydro_ActivationFlags_Forced : Hydro_ActivationFlags_None)));
            return;
        }
    }

    setupActivation(HydroActivation(Hydro_DirectionMode_Forward, value, duration, (force ? Hydro_ActivationFlags_Forced : Hydro_ActivationFlags_None)));
}

void HydroActuatorAttachment::enableActivation()
{
    if (!_actHandle.actuator && _actSetup.isValid() && resolve()) {
        if (_actHandle.isDone()) { applySetup(); } // repeats existing setup
        _calledLastUpdate = false;
        _actHandle = getObject();
    }
}

void HydroActuatorAttachment::setUpdateSlot(const Slot<HydroActuatorAttachment *> &updateSlot)
{
    if (!_updateSlot || !_updateSlot->operator==(&updateSlot)) {
        if (_updateSlot) { delete _updateSlot; _updateSlot = nullptr; }
        _updateSlot = updateSlot.clone();
    }
}

void HydroActuatorAttachment::applySetup()
{
    if (_actSetup.isValid()) {
        if (!isFPEqual(_rateMultiplier, 1.0f)) {
            _actHandle.activation.direction = _actSetup.direction;
            _actHandle.activation.flags = _actSetup.flags;

            if (resolve() && get()->isAnyBinaryClass()) { // Duration based change for rate multiplier
                _actHandle.activation.intensity = _actSetup.intensity;
                if (!_actHandle.isUntimed()) {
                    _actHandle.activation.duration = _actSetup.duration * _rateMultiplier;
                } else {
                    _actHandle.activation.duration = _actSetup.duration;
                }
            } else { // Intensity based change for rate multiplier
                _actHandle.activation.intensity = _actSetup.intensity * _rateMultiplier;
                _actHandle.activation.duration = _actSetup.duration;
            }
        } else {
            _actHandle.activation = _actSetup;
        }

        if (isActivated() && resolve()) { get()->setNeedsUpdate(); }
    }
}


HydroSensorAttachment::HydroSensorAttachment(HydroObjInterface *parent, uint8_t measurementRow)
    : HydroSignalAttachment<const HydroMeasurement *, HYDRO_SENSOR_SIGNAL_SLOTS>(parent, &HydroSensor::getMeasurementSignal),
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
    : HydroSignalAttachment<Hydro_TriggerState, HYDRO_TRIGGER_SIGNAL_SLOTS>(parent, &HydroTrigger::getTriggerSignal)
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
    : HydroSignalAttachment<Hydro_BalancingState, HYDRO_BALANCER_SIGNAL_SLOTS>(parent, &HydroBalancer::getBalancingSignal)
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
