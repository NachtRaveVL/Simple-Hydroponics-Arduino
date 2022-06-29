/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensor Measurements
*/

#include "Hydroponics.h"

HydroponicsMeasurement *newMeasurementObjectFromSubData(const HydroponicsMeasurementData *dataIn)
{
    if (dataIn && dataIn->type == -1) return nullptr;
    HYDRUINO_SOFT_ASSERT(dataIn && dataIn->type >= 0, F("Invalid data"));

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


HydroponicsMeasurement::HydroponicsMeasurement()
    : type(Unknown)
{
    auto hydroponics = getHydroponicsInstance();
    frame = (hydroponics ? hydroponics->getPollingFrame() : 0);
    timestamp = now();
}

HydroponicsMeasurement::HydroponicsMeasurement(int typeIn, time_t timestampIn)
    : type((typeof(type))typeIn), timestamp(timestampIn)
{
    auto hydroponics = getHydroponicsInstance();
    frame = (hydroponics ? hydroponics->getPollingFrame() : 0);
}

HydroponicsMeasurement::HydroponicsMeasurement(int typeIn, time_t timestampIn, uint32_t frameIn)
    : type((typeof(type))typeIn), timestamp(timestampIn), frame(frameIn)
{ ; }

HydroponicsMeasurement::HydroponicsMeasurement(const HydroponicsMeasurementData *dataIn)
    : type((typeof(type))(dataIn->type)), timestamp(dataIn->timestamp), frame(0)
{ ; }

void HydroponicsMeasurement::saveToData(HydroponicsMeasurementData *dataOut) const
{
    dataOut->type = (int8_t)type;
    dataOut->timestamp = timestamp;
}


HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement()
    : HydroponicsMeasurement(), state(false)
{ ; }

HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement(bool stateIn, time_t timestamp)
    : HydroponicsMeasurement((int)Binary, timestamp), state(stateIn)
{ ; }

HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement(bool stateIn, time_t timestamp, uint32_t frame)
    : HydroponicsMeasurement((int)Binary, timestamp, frame), state(stateIn)
{ ; }

HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement(const HydroponicsMeasurementData *dataIn)
    : HydroponicsMeasurement(dataIn), state(dataIn->dataAs.binaryMeasure.state)
{ ; }

void HydroponicsBinaryMeasurement::saveToData(HydroponicsMeasurementData *dataOut) const
{
    HydroponicsMeasurement::saveToData(dataOut);

    dataOut->dataAs.binaryMeasure.state = state;
}


HydroponicsSingleMeasurement::HydroponicsSingleMeasurement()
    : HydroponicsMeasurement((int)Single), value(0.0f), units(Hydroponics_UnitsType_Undefined)
{ ; }

HydroponicsSingleMeasurement::HydroponicsSingleMeasurement(float valueIn, Hydroponics_UnitsType unitsIn, time_t timestamp)
    : HydroponicsMeasurement((int)Single, timestamp), value(valueIn), units(unitsIn)
{ ; }

HydroponicsSingleMeasurement::HydroponicsSingleMeasurement(float valueIn, Hydroponics_UnitsType unitsIn, time_t timestamp, uint32_t frame)
    : HydroponicsMeasurement((int)Single, timestamp, frame), value(valueIn), units(unitsIn)
{ ; }

HydroponicsSingleMeasurement::HydroponicsSingleMeasurement(const HydroponicsMeasurementData *dataIn)
    : HydroponicsMeasurement(dataIn), value(dataIn->dataAs.singleMeasure.value), units(dataIn->dataAs.singleMeasure.units)
{ ; }

void HydroponicsSingleMeasurement::saveToData(HydroponicsMeasurementData *dataOut) const
{
    HydroponicsMeasurement::saveToData(dataOut);

    dataOut->dataAs.singleMeasure.value = value;
    dataOut->dataAs.singleMeasure.units = units;
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
                                                           time_t timestamp, uint32_t frame)
    : HydroponicsMeasurement((int)Double, timestamp, frame), value{value1,value2}, units{units1,units2}
{ ; }

HydroponicsDoubleMeasurement::HydroponicsDoubleMeasurement(const HydroponicsMeasurementData *dataIn)
    : HydroponicsMeasurement(dataIn),
      value{dataIn->dataAs.doubleMeasure.value[0], dataIn->dataAs.doubleMeasure.value[1]},
      units{dataIn->dataAs.doubleMeasure.units[0], dataIn->dataAs.doubleMeasure.units[1]}
{ ; }

void HydroponicsDoubleMeasurement::saveToData(HydroponicsMeasurementData *dataOut) const
{
    HydroponicsMeasurement::saveToData(dataOut);

    dataOut->dataAs.doubleMeasure.value[0] = value[0];
    dataOut->dataAs.doubleMeasure.value[1] = value[1];
    dataOut->dataAs.doubleMeasure.units[0] = units[0];
    dataOut->dataAs.doubleMeasure.units[1] = units[1];
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
                                                           time_t timestamp, uint32_t frame)
    : HydroponicsMeasurement((int)Triple, timestamp, frame), value{value1,value2,value3}, units{units1,units2,units3}
{ ; }

HydroponicsTripleMeasurement::HydroponicsTripleMeasurement(const HydroponicsMeasurementData *dataIn)
    : HydroponicsMeasurement(dataIn),
      value{dataIn->dataAs.tripleMeasure.value[0], dataIn->dataAs.tripleMeasure.value[1], dataIn->dataAs.tripleMeasure.value[2]},
      units{dataIn->dataAs.tripleMeasure.units[0], dataIn->dataAs.tripleMeasure.units[1], dataIn->dataAs.tripleMeasure.units[2]}
{ ; }

void HydroponicsTripleMeasurement::saveToData(HydroponicsMeasurementData *dataOut) const
{
    HydroponicsMeasurement::saveToData(dataOut);

    dataOut->dataAs.tripleMeasure.value[0] = value[0];
    dataOut->dataAs.tripleMeasure.value[1] = value[1];
    dataOut->dataAs.tripleMeasure.value[2] = value[2];
    dataOut->dataAs.tripleMeasure.units[0] = units[0];
    dataOut->dataAs.tripleMeasure.units[1] = units[1];
    dataOut->dataAs.tripleMeasure.units[2] = units[2];
}


HydroponicsMeasurementData::HydroponicsMeasurementData()
    : HydroponicsSubData(), dataAs{.tripleMeasure={{0.0f,0.0f,0.0f},{Hydroponics_UnitsType_Undefined,Hydroponics_UnitsType_Undefined,Hydroponics_UnitsType_Undefined}}}, timestamp(0)
{ ; }

void HydroponicsMeasurementData::toJSONObject(JsonObject &objectOut) const
{
    HydroponicsSubData::toJSONObject(objectOut);

    switch (type) {
        case 0: // Binary
            objectOut[F("state")] = dataAs.binaryMeasure.state;
            break;
        case 1: // Single
            objectOut[F("value")] = dataAs.singleMeasure.value;
            objectOut[F("units")] = dataAs.singleMeasure.units;
            break;
        case 2: { // Double
            JsonArray valuesArray = objectOut.createNestedArray(F("values"));
            valuesArray[0] = dataAs.doubleMeasure.value[0];
            valuesArray[1] = dataAs.doubleMeasure.value[1];
            JsonArray unitsArray = objectOut.createNestedArray(F("units"));
            unitsArray[0] = dataAs.doubleMeasure.units[0];
            unitsArray[1] = dataAs.doubleMeasure.units[1];
        } break;
        case 3: { // Triple
            JsonArray valuesArray = objectOut.createNestedArray(F("values"));
            valuesArray[0] = dataAs.tripleMeasure.value[0];
            valuesArray[1] = dataAs.tripleMeasure.value[1];
            valuesArray[2] = dataAs.tripleMeasure.value[2];
            JsonArray unitsArray = objectOut.createNestedArray(F("units"));
            unitsArray[0] = dataAs.tripleMeasure.units[0];
            unitsArray[1] = dataAs.tripleMeasure.units[1];
            unitsArray[2] = dataAs.tripleMeasure.units[2];
        } break;
        default: break;
    }
    objectOut[F("timestamp")] = timestamp;
}

void HydroponicsMeasurementData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroponicsSubData::fromJSONObject(objectIn);

    switch (type) {
        case 0: // Binary
            dataAs.binaryMeasure.state = objectIn[F("state")] | false;
            break;
        case 1: // Single
            fromJSONObject(objectIn);
            break;
        default: {
            JsonArrayConst valuesArray = objectIn[F("values")];
            if (!valuesArray.isNull()) { fromJSONValuesArray(valuesArray); }
            JsonArrayConst unitsArray = objectIn[F("units")];
            if (!unitsArray.isNull()) { fromJSONUnitsArray(unitsArray); }
        } break;
    }
    timestamp = objectIn[F("timestamp")] | timestamp;
}

void HydroponicsMeasurementData::fromJSONVariant(JsonVariantConst &variantIn)
{
    if (variantIn.is<JsonObjectConst>()) {
        JsonObjectConst variantObj = variantIn;
        fromJSONObject(variantObj);
    } else if (variantIn.is<JsonArrayConst>()) {
        JsonArrayConst variantArray = variantIn;
        fromJSONArray(variantArray);
    } else {
        setValue(variantIn, 0);
    }
}

void HydroponicsMeasurementData::fromJSONArray(JsonArrayConst &arrayIn)
{
    type = arrayIn.size();
    if (arrayIn[0].is<JsonObjectConst>()) {
        fromJSONObjectsArray(arrayIn);
    } else {
        fromJSONValuesArray(arrayIn);
    }
}

void HydroponicsMeasurementData::fromJSONObjectsArray(JsonArrayConst &objectsIn)
{
    for (int rowIndex = 0; rowIndex < type; ++rowIndex) {
        JsonObjectConst objectObj = objectsIn[rowIndex];
        if (!objectObj.isNull()) { fromJSONObject(objectObj, rowIndex); }
    }
}

void HydroponicsMeasurementData::fromJSONObject(JsonObjectConst &objectIn, int rowIndex)
{
    setValue(objectIn[F("value")] | objectIn[F("val")] | objectIn[F("state")] | 0.0f, rowIndex);
    setUnits(objectIn[F("units")] | objectIn[F("unit")] | Hydroponics_UnitsType_Undefined, rowIndex);
}

void HydroponicsMeasurementData::fromJSONValuesArray(JsonArrayConst &valuesIn)
{
    for (int rowIndex = 0; rowIndex < type; ++rowIndex) {
        setValue(valuesIn[rowIndex], rowIndex);
    }
}

void HydroponicsMeasurementData::fromJSONUnitsArray(JsonArrayConst &unitsIn)
{
    for (int rowIndex = 0; rowIndex < type; ++rowIndex) {
        setUnits(unitsIn[rowIndex], rowIndex);
    }
}

void HydroponicsMeasurementData::setValue(float value, int rowIndex)
{
    switch (type) {
        case 0: // Binary
            dataAs.binaryMeasure.state = value;
            break;
        case 1: // Single
            dataAs.singleMeasure.value = value;
            break;
        case 2: // Double
            dataAs.doubleMeasure.value[rowIndex] = value;
            break;
        case 3: // Triple
            dataAs.tripleMeasure.value[rowIndex] = value;
            break;
        default: break;
    }
}

void HydroponicsMeasurementData::setUnits(Hydroponics_UnitsType units, int rowIndex)
{
    switch (type) {
        case 1: // Single
            dataAs.singleMeasure.units = units;
            break;
        case 2: // Double
            dataAs.doubleMeasure.units[rowIndex] = units;
            break;
        case 3: // Triple
            dataAs.tripleMeasure.units[rowIndex] = units;
            break;
        default: break;
    }
}
