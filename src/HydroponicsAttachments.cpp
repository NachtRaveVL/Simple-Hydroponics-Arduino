/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>(
          parent, &HydroponicsSensor::getMeasurementSignal, MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::handleMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{ ; }

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, const HydroponicsIdentity &sensorId, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>(
          parent, sensorId, &HydroponicsSensor::getMeasurementSignal, MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::handleMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{ ; }

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, const char *sensorKeyStr, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>(
          parent, HydroponicsIdentity(sensorKeyStr), &HydroponicsSensor::getMeasurementSignal, MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::handleMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{ ; }

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, shared_ptr<HydroponicsSensor> sensor, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>(
          parent, sensor, &HydroponicsSensor::getMeasurementSignal, MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::handleMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{
    if (isResolved()) {
        handleMeasurement(_obj->getLatestMeasurement());
    }
}

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, const HydroponicsSensor *sensor, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>(
          parent, sensor, &HydroponicsSensor::getMeasurementSignal, MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::handleMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{
    if (isResolved()) {
        handleMeasurement(_obj->getLatestMeasurement());
    }
}

HydroponicsSensorAttachment::~HydroponicsSensorAttachment()
{ ; }

void HydroponicsSensorAttachment::attachObject()
{
    if (needsResolved()) {
        HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>::attachObject();

        if (isResolved()) {
            handleMeasurement(_obj->getLatestMeasurement());
        }
    }
}

void HydroponicsSensorAttachment::detachObject()
{
    if (isResolved()) {
        HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS>::detachObject();
        setNeedsMeasurement();
    }
}

void HydroponicsSensorAttachment::updateMeasurementIfNeeded(bool resolveIfNeeded)
{
    if (needsResolved() && resolveIfNeeded) { getObject(); }
    if (_needsMeasurement && _obj) {
        handleMeasurement(_obj->getLatestMeasurement());
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

    if (_updateMethod) { (_parent->*_updateMethod)(&_measurement); }
}

void HydroponicsSensorAttachment::setMeasurement(HydroponicsSingleMeasurement measurement)
{
    auto outUnits = definedUnitsElse(_measurement.units, measurement.units);
    _measurement = measurement;
    _measurement.setMinFrame(1);

    convertUnits(&_measurement, outUnits, _convertParam);
    _needsMeasurement = false;

    if (_updateMethod) { (_parent->*_updateMethod)(&_measurement); }
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
    _convertParam = convertParam;
    convertUnits(&_measurement, units, _convertParam);
    setNeedsMeasurement();
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
