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
        _keyStr = (const char *)malloc(strlen(obj._keyStr) + 1);
        strcpy((char *)_keyStr, obj._keyStr);
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
            strcpy((char *)_keyStr, id.keyString.c_str());
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
{
    HYDRO_HARD_ASSERT(_parent, SFP(HStr_Err_InvalidParameter));
}

HydroAttachment::~HydroAttachment()
{
    if (isResolved()) {
        _obj->removeLinkage((HydroObject *)_parent);
    }
}

void HydroAttachment::attachObject()
{
    _obj->addLinkage((HydroObject *)_parent);
}

void HydroAttachment::detachObject()
{
    if (isResolved()) {
        _obj->removeLinkage((HydroObject *)_parent);
    }
    // note: used to set _obj to nullptr here, but found that it's best not to -> avoids additional operator= calls during typical detach scenarios
}

HydroSensorAttachment::HydroSensorAttachment(HydroObjInterface *parent, uint8_t measurementRow)
    : HydroSignalAttachment<const HydroMeasurement *, HYDRO_SENSOR_MEASUREMENT_SLOTS>(
          parent, &HydroSensor::getMeasurementSignal),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{
    setHandleMethod(&HydroSensorAttachment::handleMeasurement);
}

void HydroSensorAttachment::attachObject()
{
    HydroSignalAttachment<const HydroMeasurement *, HYDRO_SENSOR_MEASUREMENT_SLOTS>::attachObject();

    if (_handleMethod) { _handleMethod->operator()(get()->getLatestMeasurement()); }
    else { handleMeasurement(get()->getLatestMeasurement()); }
}

void HydroSensorAttachment::detachObject()
{
    HydroSignalAttachment<const HydroMeasurement *, HYDRO_SENSOR_MEASUREMENT_SLOTS>::detachObject();

    setNeedsMeasurement();
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
    : HydroSignalAttachment<Hydro_TriggerState, HYDRO_TRIGGER_STATE_SLOTS>(
        parent, &HydroTrigger::getTriggerSignal)
{ ; }


HydroBalancerAttachment::HydroBalancerAttachment(HydroObjInterface *parent)
    : HydroSignalAttachment<Hydro_BalancerState, HYDRO_BALANCER_STATE_SLOTS>(
        parent, &HydroBalancer::getBalancerSignal)
{ ; }
