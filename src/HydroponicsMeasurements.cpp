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

float measurementValueAt(const HydroponicsMeasurement *measurementIn, int rowIndex, float binTrue)
{
    if (measurementIn) {
        switch (measurementIn->type) {
            case 0: // Binary
                if (rowIndex == 0) { return ((HydroponicsBinaryMeasurement *)measurementIn)->state ? binTrue : 0.0f; }
            case 1: // Single
                if (rowIndex == 0) { return ((HydroponicsSingleMeasurement *)measurementIn)->value; }
            case 2: // Double
                if (rowIndex > 0 && rowIndex < 2) { return ((HydroponicsDoubleMeasurement *)measurementIn)->value[rowIndex]; }
            case 3: // Triple
                if (rowIndex > 0 && rowIndex < 3) { return ((HydroponicsTripleMeasurement *)measurementIn)->value[rowIndex]; }
            default: break;
        }
    }
    return 0.0f;
}

Hydroponics_UnitsType measurementUnitsAt(const HydroponicsMeasurement *measurementIn, int rowIndex, Hydroponics_UnitsType binUnits)
{
    if (measurementIn) {
        switch (measurementIn->type) {
            case 0: // Binary
                if (rowIndex == 0) { return binUnits; }
            case 1: // Single
                if (rowIndex == 0) { return ((HydroponicsSingleMeasurement *)measurementIn)->units; }
            case 2: // Double
                if (rowIndex > 0 && rowIndex < 2) { return ((HydroponicsDoubleMeasurement *)measurementIn)->units[rowIndex]; }
            case 3: // Triple
                if (rowIndex > 0 && rowIndex < 3) { return ((HydroponicsTripleMeasurement *)measurementIn)->units[rowIndex]; }
            default: break;
        }
    }
    return Hydroponics_UnitsType_Undefined;
}

HydroponicsSingleMeasurement singleMeasurementAt(const HydroponicsMeasurement *measurementIn, int rowIndex, float binTrue, Hydroponics_UnitsType binUnits)
{
    if (measurementIn) {
        switch (measurementIn->type) {
            case 0: // Binary
                if (rowIndex == 0) { return ((HydroponicsBinaryMeasurement *)measurementIn)->asSingleMeasurement(binTrue, binUnits); }
            case 1: // Single
                if (rowIndex == 0) { return HydroponicsSingleMeasurement(*((HydroponicsSingleMeasurement *)measurementIn)); }
            case 2: // Double
                if (rowIndex > 0 && rowIndex < 2) { return ((HydroponicsDoubleMeasurement *)measurementIn)->asSingleMeasurement(rowIndex); }
            case 3: // Triple
                if (rowIndex > 0 && rowIndex < 3) { return ((HydroponicsTripleMeasurement *)measurementIn)->asSingleMeasurement(rowIndex); }
            default: break;
        }
    }
    {   HydroponicsSingleMeasurement retVal;
        retVal.frame = 0; // force fails frame checks
        return retVal;
    }
}


HydroponicsMeasurement::HydroponicsMeasurement()
    : type(Unknown), frame(0), timestamp(now())
{ ; }

HydroponicsMeasurement::HydroponicsMeasurement(int typeIn, time_t timestampIn)
    : type((typeof(type))typeIn), timestamp(timestampIn)
{
    updateFrame();
}

HydroponicsMeasurement::HydroponicsMeasurement(int typeIn, time_t timestampIn, uint32_t frameIn)
    : type((typeof(type))typeIn), timestamp(timestampIn), frame(frameIn)
{ ; }

HydroponicsMeasurement::HydroponicsMeasurement(const HydroponicsMeasurementData *dataIn)
    : type((typeof(type))(dataIn->type)), timestamp(dataIn->timestamp), frame(1)
{ ; }

void HydroponicsMeasurement::saveToData(HydroponicsMeasurementData *dataOut) const
{
    dataOut->type = (int8_t)type;
    dataOut->timestamp = timestamp;
}

void HydroponicsMeasurement::updateFrame(int minFrame)
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
            objectOut[F("values")] = commaStringFromArray(dataAs.doubleMeasure.value, 2);
            if (dataAs.doubleMeasure.units[0] != dataAs.doubleMeasure.units[1]) {
                objectOut[F("units")] = commaStringFromArray(dataAs.doubleMeasure.units, 2);
            } else {
                objectOut[F("units")] = dataAs.doubleMeasure.units[0];
            }
        } break;
        case 3: { // Triple
            objectOut[F("values")] = commaStringFromArray(dataAs.tripleMeasure.value, 3);
            if (dataAs.tripleMeasure.units[0] != dataAs.tripleMeasure.units[1] ||
                dataAs.tripleMeasure.units[0] != dataAs.tripleMeasure.units[2]) {
                objectOut[F("units")] = commaStringFromArray(dataAs.tripleMeasure.units, 3);
            } else {
                objectOut[F("units")] = dataAs.tripleMeasure.units[0];
            }
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
            fromJSONObject(objectIn, 0);
            break;
        default: {
            JsonVariantConst valuesVar = objectIn[F("values")] | objectIn[F("vals")];
            if (valuesVar.is<JsonArrayConst>()) { JsonArrayConst valuesArray = valuesVar; fromJSONValuesArray(valuesArray); }
            else { fromJSONValuesString(valuesVar); }

            JsonVariantConst unitsVar = objectIn[F("units")] | objectIn[F("unit")];
            if (unitsVar.is<JsonArrayConst>()) { JsonArrayConst unitsArray = unitsVar; fromJSONUnitsArray(unitsArray); }
            else { fromJSONUnitsString(unitsVar); }
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
    } else if (variantIn.is<const char *>()) {
        const char *valuesIn = variantIn.as<const char*>();
        type = occurrencesInString(valuesIn, ',') + 1;
        fromJSONValuesString(valuesIn);
    } else if (variantIn.is<float>() || variantIn.is<int>()) {
        type = 1;
        setValue(variantIn, 0);
    } else {
        HYDRUINO_SOFT_ASSERT(false, F("Unsupported measurement JSON"));
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

void HydroponicsMeasurementData::fromJSONValuesString(const char *valuesIn)
{
    float values[3]; commaStringToArray(valuesIn, values, type);

    for (int rowIndex = 0; rowIndex < type; ++rowIndex) {
        setValue(values[rowIndex], rowIndex);
    }
}

void HydroponicsMeasurementData::fromJSONUnitsArray(JsonArrayConst &unitsIn)
{
    for (int rowIndex = 0; rowIndex < type; ++rowIndex) {
        setUnits(unitsIn[rowIndex], rowIndex);
    }
}

void HydroponicsMeasurementData::fromJSONUnitsString(const char *unitsIn)
{
    int units[3]; commaStringToArray(unitsIn, units, type);

    for (int rowIndex = 0; rowIndex < type; ++rowIndex) {
        setUnits((Hydroponics_UnitsType)units[rowIndex], rowIndex);
    }
}

void HydroponicsMeasurementData::setValue(float value, int rowIndex)
{
    switch (type) {
        case 0: // Binary
            if (rowIndex == 0) { dataAs.binaryMeasure.state = value; }
            break;
        case 1: // Single
            if (rowIndex == 0) { dataAs.singleMeasure.value = value; }
            break;
        case 2: // Double
            if (rowIndex >= 0 && rowIndex < 2) { dataAs.doubleMeasure.value[rowIndex] = value; }
            break;
        case 3: // Triple
            if (rowIndex >= 0 && rowIndex < 3) { dataAs.tripleMeasure.value[rowIndex] = value; }
            break;
        default: break;
    }
}

void HydroponicsMeasurementData::setUnits(Hydroponics_UnitsType units, int rowIndex)
{
    switch (type) {
        case 1: // Single
            if (rowIndex == 0) { dataAs.singleMeasure.units = units; }
            break;
        case 2: // Double
            if (rowIndex >= 0 && rowIndex < 2) { dataAs.doubleMeasure.units[rowIndex] = units; }
            break;
        case 3: // Triple
            if (rowIndex >= 0 && rowIndex < 3) { dataAs.tripleMeasure.units[rowIndex] = units; }
            break;
        default: break;
    }
}
