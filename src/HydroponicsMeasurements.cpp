/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensor Measurements
*/

#include "Hydroponics.h"

HydroponicsMeasurement *newMeasurementObjectFromSubData(const HydroponicsMeasurementData *dataIn)
{
    if (dataIn && dataIn->type == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->type >= 0, SFP(HStr_Err_InvalidParameter));

    if (dataIn) {
        switch (dataIn->type) {
            case 0: // Binary
                return new HydroponicsBinaryMeasurement(dataIn);
            case 1: // Single
                return new HydroponicsSingleMeasurement(dataIn);
            case 2: // Double
                return new HydroponicsDoubleMeasurement(dataIn);
            case 3: // Triple
                return new HydroponicsTripleMeasurement(dataIn);
            default: break;
        }
    }

    return nullptr;
}

float getMeasurementValue(const HydroponicsMeasurement *measurement, uint8_t measurementRow, float binTrue)
{
    if (measurement) {
        switch (measurement->type) {
            case 0: // Binary
                return ((HydroponicsBinaryMeasurement *)measurement)->state ? binTrue : 0.0f;
            case 1: // Single
                return ((HydroponicsSingleMeasurement *)measurement)->value;
            case 2: // Double
                return ((HydroponicsDoubleMeasurement *)measurement)->value[measurementRow];
            case 3: // Triple
                return ((HydroponicsTripleMeasurement *)measurement)->value[measurementRow];
            default: break;
        }
    }
    return 0.0f;
}

Hydroponics_UnitsType getMeasurementUnits(const HydroponicsMeasurement *measurement, uint8_t measurementRow, Hydroponics_UnitsType binUnits)
{
    if (measurement) {
        switch (measurement->type) {
            case 0: // Binary
                return binUnits;
            case 1: // Single
                return ((HydroponicsSingleMeasurement *)measurement)->units;
            case 2: // Double
                return ((HydroponicsDoubleMeasurement *)measurement)->units[measurementRow];
            case 3: // Triple
                return ((HydroponicsTripleMeasurement *)measurement)->units[measurementRow];
            default: break;
        }
    }
    return Hydroponics_UnitsType_Undefined;
}

uint8_t getMeasurementRowCount(const HydroponicsMeasurement *measurement)
{
    return measurement ? max(1, (int)(measurement->type)) : 0;
}

HydroponicsSingleMeasurement getAsSingleMeasurement(const HydroponicsMeasurement *measurement, uint8_t measurementRow, float binTrue, Hydroponics_UnitsType binUnits)
{
    if (measurement) {
        switch (measurement->type) {
            case 0: // Binary
                return ((HydroponicsBinaryMeasurement *)measurement)->getAsSingleMeasurement(binTrue, binUnits);
            case 1: // Single
                return *((const HydroponicsSingleMeasurement *)measurement);
            case 2: // Double
                return ((HydroponicsDoubleMeasurement *)measurement)->getAsSingleMeasurement(measurementRow);
            case 3: // Triple
                return ((HydroponicsTripleMeasurement *)measurement)->getAsSingleMeasurement(measurementRow);
            default: break;
        }
    }
    {   HydroponicsSingleMeasurement retVal;
        retVal.frame = 0; // force fails frame checks
        return retVal;
    }
}


HydroponicsMeasurement::HydroponicsMeasurement()
    : type(Unknown), frame(0), timestamp(unixNow())
{ ; }

HydroponicsMeasurement::HydroponicsMeasurement(int typeIn, time_t timestampIn)
    : type((typeof(type))typeIn), timestamp(timestampIn)
{
    updateFrame();
}

HydroponicsMeasurement::HydroponicsMeasurement(int typeIn, time_t timestampIn, uint16_t frameIn)
    : type((typeof(type))typeIn), timestamp(timestampIn), frame(frameIn)
{ ; }

HydroponicsMeasurement::HydroponicsMeasurement(const HydroponicsMeasurementData *dataIn)
    : type((typeof(type))(dataIn->type)), timestamp(dataIn->timestamp)
{
    updateFrame(1);
}

void HydroponicsMeasurement::saveToData(HydroponicsMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    dataOut->type = (int8_t)type;
    dataOut->measurementRow = measurementRow;
    dataOut->timestamp = timestamp;
}

void HydroponicsMeasurement::updateFrame(unsigned int minFrame)
{
    auto hydroponics = getHydroponicsInstance();
    frame = max(minFrame, hydroponics ? hydroponics->getPollingFrame() : 0);
}


HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement()
    : HydroponicsMeasurement(), state(false)
{ ; }

HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement(bool stateIn, time_t timestamp)
    : HydroponicsMeasurement((int)Binary, timestamp), state(stateIn)
{ ; }

HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement(bool stateIn, time_t timestamp, uint16_t frame)
    : HydroponicsMeasurement((int)Binary, timestamp, frame), state(stateIn)
{ ; }

HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement(const HydroponicsMeasurementData *dataIn)
    : HydroponicsMeasurement(dataIn),
      state(dataIn->measurementRow == 0 && dataIn->value >= 0.5f - FLT_EPSILON)
{ ; }

void HydroponicsBinaryMeasurement::saveToData(HydroponicsMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    HydroponicsMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow == 0 && state ? 1.0f : 0.0f;
    dataOut->units = measurementRow == 0 ? Hydroponics_UnitsType_Raw_0_1 : Hydroponics_UnitsType_Undefined;
}


HydroponicsSingleMeasurement::HydroponicsSingleMeasurement()
    : HydroponicsMeasurement((int)Single), value(0.0f), units(Hydroponics_UnitsType_Undefined)
{ ; }

HydroponicsSingleMeasurement::HydroponicsSingleMeasurement(float valueIn, Hydroponics_UnitsType unitsIn, time_t timestamp)
    : HydroponicsMeasurement((int)Single, timestamp), value(valueIn), units(unitsIn)
{ ; }

HydroponicsSingleMeasurement::HydroponicsSingleMeasurement(float valueIn, Hydroponics_UnitsType unitsIn, time_t timestamp, uint16_t frame)
    : HydroponicsMeasurement((int)Single, timestamp, frame), value(valueIn), units(unitsIn)
{ ; }

HydroponicsSingleMeasurement::HydroponicsSingleMeasurement(const HydroponicsMeasurementData *dataIn)
    : HydroponicsMeasurement(dataIn),
      value(dataIn->measurementRow == 0 ? dataIn->value : 0.0f),
      units(dataIn->measurementRow == 0 ? dataIn->units : Hydroponics_UnitsType_Undefined)
{ ; }

void HydroponicsSingleMeasurement::saveToData(HydroponicsMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    HydroponicsMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow == 0 ? roundForExport(value, additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow == 0 ? units : Hydroponics_UnitsType_Undefined;
}


HydroponicsDoubleMeasurement::HydroponicsDoubleMeasurement()
    : HydroponicsMeasurement((int)Double), value{0}, units{Hydroponics_UnitsType_Undefined,Hydroponics_UnitsType_Undefined}
{ ; }

HydroponicsDoubleMeasurement::HydroponicsDoubleMeasurement(float value1, Hydroponics_UnitsType units1,
                                                           float value2, Hydroponics_UnitsType units2,
                                                           time_t timestamp)
    : HydroponicsMeasurement((int)Double, timestamp), value{value1,value2}, units{units1,units2}
{ ; }

HydroponicsDoubleMeasurement::HydroponicsDoubleMeasurement(float value1, Hydroponics_UnitsType units1,
                                                           float value2, Hydroponics_UnitsType units2,
                                                           time_t timestamp, uint16_t frame)
    : HydroponicsMeasurement((int)Double, timestamp, frame), value{value1,value2}, units{units1,units2}
{ ; }

HydroponicsDoubleMeasurement::HydroponicsDoubleMeasurement(const HydroponicsMeasurementData *dataIn)
    : HydroponicsMeasurement(dataIn),
      value{dataIn->measurementRow == 0 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 1 ? dataIn->value : 0.0f
      },
      units{dataIn->measurementRow == 0 ? dataIn->units : Hydroponics_UnitsType_Undefined,
            dataIn->measurementRow == 1 ? dataIn->units : Hydroponics_UnitsType_Undefined
      }
{ ; }

void HydroponicsDoubleMeasurement::saveToData(HydroponicsMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    HydroponicsMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow >= 0 && measurementRow < 2 ? roundForExport(value[measurementRow], additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow >= 0 && measurementRow < 2 ? units[measurementRow] : Hydroponics_UnitsType_Undefined;
}


HydroponicsTripleMeasurement::HydroponicsTripleMeasurement()
    : HydroponicsMeasurement((int)Triple), value{0}, units{Hydroponics_UnitsType_Undefined,Hydroponics_UnitsType_Undefined,Hydroponics_UnitsType_Undefined}
{ ; }

HydroponicsTripleMeasurement::HydroponicsTripleMeasurement(float value1, Hydroponics_UnitsType units1,
                                                           float value2, Hydroponics_UnitsType units2,
                                                           float value3, Hydroponics_UnitsType units3,
                                                           time_t timestamp)
    : HydroponicsMeasurement((int)Triple, timestamp), value{value1,value2,value3}, units{units1,units2,units3}
{ ; }

HydroponicsTripleMeasurement::HydroponicsTripleMeasurement(float value1, Hydroponics_UnitsType units1,
                                                           float value2, Hydroponics_UnitsType units2,
                                                           float value3, Hydroponics_UnitsType units3,
                                                           time_t timestamp, uint16_t frame)
    : HydroponicsMeasurement((int)Triple, timestamp, frame), value{value1,value2,value3}, units{units1,units2,units3}
{ ; }

HydroponicsTripleMeasurement::HydroponicsTripleMeasurement(const HydroponicsMeasurementData *dataIn)
    : HydroponicsMeasurement(dataIn),
      value{dataIn->measurementRow == 0 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 1 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 2 ? dataIn->value : 0.0f,
      },
      units{dataIn->measurementRow == 0 ? dataIn->units : Hydroponics_UnitsType_Undefined,
            dataIn->measurementRow == 1 ? dataIn->units : Hydroponics_UnitsType_Undefined,
            dataIn->measurementRow == 2 ? dataIn->units : Hydroponics_UnitsType_Undefined,
      }
{ ; }

void HydroponicsTripleMeasurement::saveToData(HydroponicsMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    HydroponicsMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow >= 0 && measurementRow < 3 ? roundForExport(value[measurementRow], additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow >= 0 && measurementRow < 3 ? units[measurementRow] : Hydroponics_UnitsType_Undefined;
}


HydroponicsMeasurementData::HydroponicsMeasurementData()
    : HydroponicsSubData(), measurementRow(0), value(0.0f), units(Hydroponics_UnitsType_Undefined), timestamp(0)
{
    type = 0; // no type differentiation
}

void HydroponicsMeasurementData::toJSONObject(JsonObject &objectOut) const
{
    //HydroponicsSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    objectOut[SFP(HStr_Key_MeasurementRow)] = measurementRow;
    objectOut[SFP(HStr_Key_Value)] = value;
    objectOut[SFP(HStr_Key_Units)] = unitsTypeToSymbol(units);
    objectOut[SFP(HStr_Key_Timestamp)] = timestamp;
}

void HydroponicsMeasurementData::fromJSONObject(JsonObjectConst &objectIn)
{
    //HydroponicsSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    measurementRow = objectIn[SFP(HStr_Key_MeasurementRow)] | measurementRow;
    value = objectIn[SFP(HStr_Key_Value)] | value;
    units = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_Units)]);
    timestamp = objectIn[SFP(HStr_Key_Timestamp)] | timestamp;
}

void HydroponicsMeasurementData::fromJSONVariant(JsonVariantConst &variantIn)
{
    if (variantIn.is<JsonObjectConst>()) {
        JsonObjectConst variantObj = variantIn;
        fromJSONObject(variantObj);
    } else if (variantIn.is<float>() || variantIn.is<int>()) {
        value = variantIn.as<float>();
    } else {
        HYDRUINO_SOFT_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
    }
}
