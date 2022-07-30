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

void HydroponicsDLinkObject::detachObject()
{
    if (_obj && !_keyStr) {
        auto id = getId();
        auto len = id.keyString.length();
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strncpy((char *)_keyStr, id.keyString.c_str(), len + 1);
        }
    }
    _obj = nullptr;
}

HydroponicsDLinkObject &HydroponicsDLinkObject::operator=(const HydroponicsDLinkObject &rhs)
{
    _key = rhs._key;
    _obj = rhs._obj;
    if (rhs._keyStr) {
        _keyStr = (const char *)malloc(strnlen(rhs._keyStr, HYDRUINO_NAME_MAXSIZE) + 1);
        strncpy((char *)_keyStr, rhs._keyStr, HYDRUINO_NAME_MAXSIZE);
    }
    return *this;
}

HydroponicsDLinkObject &HydroponicsDLinkObject::operator=(HydroponicsIdentity rhs)
{
    _key = rhs.key;
    _obj = nullptr;
    auto len = rhs.keyString.length();
    if (len) {
        _keyStr = (const char *)malloc(len + 1);
        strncpy((char *)_keyStr, rhs.keyString.c_str(), len + 1);
    }
    return *this;
}

HydroponicsDLinkObject &HydroponicsDLinkObject::operator=(const char *rhs)
{
    _key = stringHash(rhs);
    _obj = nullptr;
    auto len = strnlen(rhs, HYDRUINO_NAME_MAXSIZE);
    if (len) {
        _keyStr = (const char *)malloc(len + 1);
        strncpy((char *)_keyStr, rhs, len + 1);
    }
    return *this;
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
    _obj->removeLinkage((HydroponicsObject *)_parent);

    _obj.detachObject();
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

    if (_handleMethod) { _handleMethod(get()->getLatestMeasurement()); }
    else { handleMeasurement(get()->getLatestMeasurement()); }
}

void HydroponicsSensorAttachment::detachObject()
{
    HydroponicsSignalAttachment<const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>::detachObject();

    setNeedsMeasurement();
}

void HydroponicsSensorAttachment::updateIfNeeded(bool poll)
{
    if (resolve() && (_needsMeasurement || poll)) {
        if (_handleMethod) { _handleMethod(get()->getLatestMeasurement()); }
        else { handleMeasurement(get()->getLatestMeasurement()); }

        get()->takeMeasurement((_needsMeasurement || poll));
    }
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
    if (measurement && measurement->frame && resolve()) {
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
