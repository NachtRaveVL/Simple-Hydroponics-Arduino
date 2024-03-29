/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Sensor Measurements
*/

#include "Hydruino.h"

HydroMeasurement *newMeasurementObjectFromSubData(const HydroMeasurementData *dataIn)
{
    if (!dataIn || !isValidType(dataIn->type)) return nullptr;
    HYDRO_SOFT_ASSERT(dataIn && isValidType(dataIn->type), SFP(HStr_Err_InvalidParameter));

    if (dataIn) {
        switch (dataIn->type) {
            case (hid_t)HydroMeasurement::Binary:
                return new HydroBinaryMeasurement(dataIn);
            case (hid_t)HydroMeasurement::Single:
                return new HydroSingleMeasurement(dataIn);
            case (hid_t)HydroMeasurement::Double:
                return new HydroDoubleMeasurement(dataIn);
            case (hid_t)HydroMeasurement::Triple:
                return new HydroTripleMeasurement(dataIn);
            default: break;
        }
    }

    return nullptr;
}

float getMeasurementValue(const HydroMeasurement *measurement, uint8_t measurementRow, float binScale)
{
    if (measurement) {
        switch (measurement->type) {
            case HydroMeasurement::Binary:
                return ((HydroBinaryMeasurement *)measurement)->state ? binScale : 0.0f;
            case HydroMeasurement::Single:
                return ((HydroSingleMeasurement *)measurement)->value;
            case HydroMeasurement::Double:
                return ((HydroDoubleMeasurement *)measurement)->value[measurementRow];
            case HydroMeasurement::Triple:
                return ((HydroTripleMeasurement *)measurement)->value[measurementRow];
            default: break;
        }
    }
    return 0.0f;
}

Hydro_UnitsType getMeasurementUnits(const HydroMeasurement *measurement, uint8_t measurementRow, Hydro_UnitsType binUnits)
{
    if (measurement) {
        switch (measurement->type) {
            case HydroMeasurement::Binary:
                return binUnits;
            case HydroMeasurement::Single:
                return ((HydroSingleMeasurement *)measurement)->units;
            case HydroMeasurement::Double:
                return ((HydroDoubleMeasurement *)measurement)->units[measurementRow];
            case HydroMeasurement::Triple:
                return ((HydroTripleMeasurement *)measurement)->units[measurementRow];
            default: break;
        }
    }
    return Hydro_UnitsType_Undefined;
}

uint8_t getMeasurementRowCount(const HydroMeasurement *measurement)
{
    return measurement ? max(1, (int)(measurement->type)) : 0;
}

HydroSingleMeasurement getAsSingleMeasurement(const HydroMeasurement *measurement, uint8_t measurementRow, float binScale, Hydro_UnitsType binUnits)
{
    if (measurement) {
        switch (measurement->type) {
            case HydroMeasurement::Binary:
                return ((HydroBinaryMeasurement *)measurement)->getAsSingleMeasurement(binScale, binUnits);
            case HydroMeasurement::Single:
                return *((const HydroSingleMeasurement *)measurement);
            case HydroMeasurement::Double:
                return ((HydroDoubleMeasurement *)measurement)->getAsSingleMeasurement(measurementRow);
            case HydroMeasurement::Triple:
                return ((HydroTripleMeasurement *)measurement)->getAsSingleMeasurement(measurementRow);
            default: break;
        }
    }
    HydroSingleMeasurement retVal;
    retVal.frame = hframe_none; // meant to fail frame checks
    return retVal;
}


HydroMeasurement::HydroMeasurement(int classType, time_t timestampIn)
    : type((typeof(type))classType), timestamp(timestampIn)
{
    updateFrame();
}

HydroMeasurement::HydroMeasurement(const HydroMeasurementData *dataIn)
    : type((typeof(type))(dataIn->type)), timestamp(dataIn->timestamp)
{
    updateFrame(1);
}

void HydroMeasurement::saveToData(HydroMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    dataOut->type = (int8_t)type;
    dataOut->measurementRow = measurementRow;
    dataOut->timestamp = timestamp;
}

void HydroMeasurement::updateFrame(hframe_t minFrame)
{
    frame = max(minFrame, getController() ? getController()->getPollingFrame() : 0);
}


HydroBinaryMeasurement::HydroBinaryMeasurement()
    : HydroMeasurement(), state(false)
{ ; }

HydroBinaryMeasurement::HydroBinaryMeasurement(bool stateIn, time_t timestamp)
    : HydroMeasurement((int)Binary, timestamp), state(stateIn)
{ ; }

HydroBinaryMeasurement::HydroBinaryMeasurement(bool stateIn, time_t timestamp, hframe_t frame)
    : HydroMeasurement((int)Binary, timestamp, frame), state(stateIn)
{ ; }

HydroBinaryMeasurement::HydroBinaryMeasurement(const HydroMeasurementData *dataIn)
    : HydroMeasurement(dataIn),
      state(dataIn->measurementRow == 0 && dataIn->value >= 0.5f - FLT_EPSILON)
{ ; }

void HydroBinaryMeasurement::saveToData(HydroMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    HydroMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow == 0 && state ? 1.0f : 0.0f;
    dataOut->units = measurementRow == 0 ? Hydro_UnitsType_Raw_1 : Hydro_UnitsType_Undefined;
}


HydroSingleMeasurement::HydroSingleMeasurement()
    : HydroMeasurement((int)Single), value(0.0f), units(Hydro_UnitsType_Undefined)
{ ; }

HydroSingleMeasurement::HydroSingleMeasurement(float valueIn, Hydro_UnitsType unitsIn, time_t timestamp)
    : HydroMeasurement((int)Single, timestamp), value(valueIn), units(unitsIn)
{ ; }

HydroSingleMeasurement::HydroSingleMeasurement(float valueIn, Hydro_UnitsType unitsIn, time_t timestamp, hframe_t frame)
    : HydroMeasurement((int)Single, timestamp, frame), value(valueIn), units(unitsIn)
{ ; }

HydroSingleMeasurement::HydroSingleMeasurement(const HydroMeasurementData *dataIn)
    : HydroMeasurement(dataIn),
      value(dataIn->measurementRow == 0 ? dataIn->value : 0.0f),
      units(dataIn->measurementRow == 0 ? dataIn->units : Hydro_UnitsType_Undefined)
{ ; }

void HydroSingleMeasurement::saveToData(HydroMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    HydroMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow == 0 ? roundForExport(value, additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow == 0 ? units : Hydro_UnitsType_Undefined;
}


HydroDoubleMeasurement::HydroDoubleMeasurement()
    : HydroMeasurement((int)Double), value{0}, units{Hydro_UnitsType_Undefined,Hydro_UnitsType_Undefined}
{ ; }

HydroDoubleMeasurement::HydroDoubleMeasurement(float value1, Hydro_UnitsType units1,
                                               float value2, Hydro_UnitsType units2,
                                               time_t timestamp)
    : HydroMeasurement((int)Double, timestamp), value{value1,value2}, units{units1,units2}
{ ; }

HydroDoubleMeasurement::HydroDoubleMeasurement(float value1, Hydro_UnitsType units1,
                                               float value2, Hydro_UnitsType units2,
                                               time_t timestamp, hframe_t frame)
    : HydroMeasurement((int)Double, timestamp, frame), value{value1,value2}, units{units1,units2}
{ ; }

HydroDoubleMeasurement::HydroDoubleMeasurement(const HydroMeasurementData *dataIn)
    : HydroMeasurement(dataIn),
      value{dataIn->measurementRow == 0 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 1 ? dataIn->value : 0.0f
      },
      units{dataIn->measurementRow == 0 ? dataIn->units : Hydro_UnitsType_Undefined,
            dataIn->measurementRow == 1 ? dataIn->units : Hydro_UnitsType_Undefined
      }
{ ; }

void HydroDoubleMeasurement::saveToData(HydroMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    HydroMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow >= 0 && measurementRow < 2 ? roundForExport(value[measurementRow], additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow >= 0 && measurementRow < 2 ? units[measurementRow] : Hydro_UnitsType_Undefined;
}


HydroTripleMeasurement::HydroTripleMeasurement()
    : HydroMeasurement((int)Triple), value{0}, units{Hydro_UnitsType_Undefined,Hydro_UnitsType_Undefined,Hydro_UnitsType_Undefined}
{ ; }

HydroTripleMeasurement::HydroTripleMeasurement(float value1, Hydro_UnitsType units1,
                                               float value2, Hydro_UnitsType units2,
                                               float value3, Hydro_UnitsType units3,
                                               time_t timestamp)
    : HydroMeasurement((int)Triple, timestamp), value{value1,value2,value3}, units{units1,units2,units3}
{ ; }

HydroTripleMeasurement::HydroTripleMeasurement(float value1, Hydro_UnitsType units1,
                                               float value2, Hydro_UnitsType units2,
                                               float value3, Hydro_UnitsType units3,
                                               time_t timestamp, hframe_t frame)
    : HydroMeasurement((int)Triple, timestamp, frame), value{value1,value2,value3}, units{units1,units2,units3}
{ ; }

HydroTripleMeasurement::HydroTripleMeasurement(const HydroMeasurementData *dataIn)
    : HydroMeasurement(dataIn),
      value{dataIn->measurementRow == 0 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 1 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 2 ? dataIn->value : 0.0f,
      },
      units{dataIn->measurementRow == 0 ? dataIn->units : Hydro_UnitsType_Undefined,
            dataIn->measurementRow == 1 ? dataIn->units : Hydro_UnitsType_Undefined,
            dataIn->measurementRow == 2 ? dataIn->units : Hydro_UnitsType_Undefined,
      }
{ ; }

void HydroTripleMeasurement::saveToData(HydroMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    HydroMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow >= 0 && measurementRow < 3 ? roundForExport(value[measurementRow], additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow >= 0 && measurementRow < 3 ? units[measurementRow] : Hydro_UnitsType_Undefined;
}


HydroMeasurementData::HydroMeasurementData()
    : HydroSubData(), measurementRow(0), value(0.0f), units(Hydro_UnitsType_Undefined), timestamp(0)
{
    type = 0; // no type differentiation
}

void HydroMeasurementData::toJSONObject(JsonObject &objectOut) const
{
    //HydroSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    objectOut[SFP(HStr_Key_MeasurementRow)] = measurementRow;
    objectOut[SFP(HStr_Key_Value)] = value;
    objectOut[SFP(HStr_Key_Units)] = unitsTypeToSymbol(units);
    objectOut[SFP(HStr_Key_Timestamp)] = timestamp;
}

void HydroMeasurementData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    measurementRow = objectIn[SFP(HStr_Key_MeasurementRow)] | measurementRow;
    value = objectIn[SFP(HStr_Key_Value)] | value;
    units = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_Units)]);
    timestamp = objectIn[SFP(HStr_Key_Timestamp)] | timestamp;
}

void HydroMeasurementData::fromJSONVariant(JsonVariantConst &variantIn)
{
    if (variantIn.is<JsonObjectConst>()) {
        JsonObjectConst variantObj = variantIn;
        fromJSONObject(variantObj);
    } else if (variantIn.is<float>() || variantIn.is<int>()) {
        value = variantIn.as<float>();
    } else {
        HYDRO_SOFT_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
    }
}
