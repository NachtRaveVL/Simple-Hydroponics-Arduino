/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

HydroponicsDLinkObject::HydroponicsDLinkObject()
    : _key((Hydroponics_KeyType)-1), _obj(nullptr), _keyStr(nullptr)
{ ; }

HydroponicsDLinkObject::HydroponicsDLinkObject(const HydroponicsDLinkObject &obj)
    : _key(obj._key), _obj(obj._obj), _keyStr(nullptr)
{
    if (obj._keyStr) {
        _keyStr = (const char *)malloc(strlen(obj._keyStr) + 1);
        strcpy((char *)_keyStr, obj._keyStr);
    }
}

HydroponicsDLinkObject::~HydroponicsDLinkObject()
{
    if (_keyStr) { free((void *)_keyStr); }
}

void HydroponicsDLinkObject::unresolve()
{
    if (_obj && !_keyStr) {
        auto id = _obj->getId();
        auto len = id.keyString.length();
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strcpy((char *)_keyStr, id.keyString.c_str());
        }
    }
    HYDRUINO_HARD_ASSERT(!_obj || _key == _obj->getKey(), SFP(HStr_Err_OperationFailure));
    _obj = nullptr;
}

shared_ptr<HydroponicsObjInterface> HydroponicsDLinkObject::_getObject()
{
    if (_obj) { return _obj; }
    if (_key == (Hydroponics_KeyType)-1) { return nullptr; }
    if (Hydroponics::_activeInstance) {
        _obj = static_pointer_cast<HydroponicsObjInterface>(Hydroponics::_activeInstance->_objects[_key]);
    }
    if (_obj && _keyStr) {
        free((void *)_keyStr); _keyStr = nullptr;
    }
    return _obj;
}

HydroponicsAttachment::HydroponicsAttachment(HydroponicsObjInterface *parent)
    : _parent(parent), _obj()
{
    HYDRUINO_HARD_ASSERT(_parent, SFP(HStr_Err_InvalidParameter));
}

HydroponicsAttachment::~HydroponicsAttachment()
{
    if (isResolved()) {
        _obj->removeLinkage((HydroponicsObject *)_parent);
    }
}

void HydroponicsAttachment::attachObject()
{
    _obj->addLinkage((HydroponicsObject *)_parent);
}

void HydroponicsAttachment::detachObject()
{
    if (isResolved()) {
        _obj->removeLinkage((HydroponicsObject *)_parent);
    }
    // note: used to set _obj to nullptr here, but found that it's best not to -> avoids additional operator= calls during typical detach scenarios
}

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObjInterface *parent, byte measurementRow)
    : HydroponicsSignalAttachment<const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>(
          parent, &HydroponicsSensor::getMeasurementSignal),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{
    setHandleMethod(&HydroponicsSensorAttachment::handleMeasurement);
}

void HydroponicsSensorAttachment::attachObject()
{
    HydroponicsSignalAttachment<const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>::attachObject();

    if (_handleMethod) { _handleMethod->operator()(get()->getLatestMeasurement()); }
    else { handleMeasurement(get()->getLatestMeasurement()); }
}

void HydroponicsSensorAttachment::detachObject()
{
    HydroponicsSignalAttachment<const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>::detachObject();

    setNeedsMeasurement();
}

void HydroponicsSensorAttachment::setMeasurement(float value, Hydroponics_UnitsType units)
{
    auto outUnits = definedUnitsElse(getMeasurementUnits(), units);
    _measurement.value = value;
    _measurement.units = units;
    _measurement.updateTimestamp();
    _measurement.updateFrame(1);

    convertUnits(&_measurement, outUnits, _convertParam);
    _needsMeasurement = false;
}

void HydroponicsSensorAttachment::setMeasurement(HydroponicsSingleMeasurement measurement)
{
    auto outUnits = definedUnitsElse(getMeasurementUnits(), measurement.units);
    _measurement = measurement;
    _measurement.setMinFrame(1);

    convertUnits(&_measurement, outUnits, _convertParam);
    _needsMeasurement = false;
}

void HydroponicsSensorAttachment::setMeasurementRow(byte measurementRow)
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
        setMeasurement(getAsSingleMeasurement(measurement, _measurementRow));
    }
}


HydroponicsTriggerAttachment::HydroponicsTriggerAttachment(HydroponicsObjInterface *parent)
    : HydroponicsSignalAttachment<Hydroponics_TriggerState, HYDRUINO_TRIGGER_STATE_SLOTS>(
        parent, &HydroponicsTrigger::getTriggerSignal)
{ ; }


HydroponicsBalancerAttachment::HydroponicsBalancerAttachment(HydroponicsObjInterface *parent)
    : HydroponicsSignalAttachment<Hydroponics_BalancerState, HYDRUINO_BALANCER_STATE_SLOTS>(
        parent, &HydroponicsBalancer::getBalancerSignal)
{ ; }
