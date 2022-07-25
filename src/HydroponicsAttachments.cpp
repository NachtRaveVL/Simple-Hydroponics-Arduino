/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Attachment Points
*/

#include "Hydroponics.h"

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, const HydroponicsIdentity &sensorId, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS, HydroponicsSensorAttachment>(
          parent, sensorId, &HydroponicsSensor::getMeasurementSignal, MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::setMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{ ; }

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, const char *sensorKeyStr, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS, HydroponicsSensorAttachment>(
          parent, HydroponicsIdentity(sensorKeyStr), &HydroponicsSensor::getMeasurementSignal, MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::setMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{ ; }

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, shared_ptr<HydroponicsSensor> sensor, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS, HydroponicsSensorAttachment>(
          parent, sensor, &HydroponicsSensor::getMeasurementSignal, MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::setMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{
    if (isResolved()) {
        setMeasurement(_obj->getLatestMeasurement());
    }
}

HydroponicsSensorAttachment::HydroponicsSensorAttachment(HydroponicsObject *parent, const HydroponicsSensor *sensor, Hydroponics_PositionIndex measurementRow)
    : HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS, HydroponicsSensorAttachment>(
          parent, sensor, &HydroponicsSensor::getMeasurementSignal, MethodSlot<HydroponicsSensorAttachment, const HydroponicsMeasurement *>(this, &HydroponicsSensorAttachment::setMeasurement)),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{
    if (isResolved()) {
        setMeasurement(_obj->getLatestMeasurement());
    }
}

HydroponicsSensorAttachment::~HydroponicsSensorAttachment()
{ ; }

void HydroponicsSensorAttachment::attachObject()
{
    if (needsResolved()) {
        HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS, HydroponicsSensorAttachment>::attachObject();

        if (isResolved()) {
            setMeasurement(_obj->getLatestMeasurement());
        }
    }
}

void HydroponicsSensorAttachment::detachObject()
{
    if (isResolved()) {
        HydroponicsSignalAttachment<HydroponicsSensor, const HydroponicsMeasurement *, HYDRUINO_SENSOR_MEASUREMENT_SLOTS, HydroponicsSensorAttachment>::detachObject();

        if (!isResolved()) {
            _needsMeasurement = true;
        }
    }
}

void HydroponicsSensorAttachment::updateMeasurementIfNeeded(bool force)
{
    if (_needsMeasurement && (force ? getObject() : _obj)) {
        setMeasurement(_obj->getLatestMeasurement());
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
}

void HydroponicsSensorAttachment::setMeasurement(HydroponicsSingleMeasurement measurement)
{
    auto outUnits = definedUnitsElse(_measurement.units, measurement.units);
    _measurement = measurement;
    _measurement.setMinFrame(1);

    convertUnits(&_measurement, outUnits, _convertParam);
    _needsMeasurement = false;
}

void HydroponicsSensorAttachment::setMeasurement(const HydroponicsMeasurement *measurement)
{
    setMeasurement(getAsSingleMeasurement(measurement, _measurementRow));
}

void HydroponicsSensorAttachment::setMeasurementUnits(Hydroponics_UnitsType units, float convertParam)
{
    _convertParam = convertParam;
    convertUnits(&_measurement, units, _convertParam);
}
