#include "Hydruino.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void testSystemData()
{
    HydroSystemData data;
    strncpy(data.systemName, "Greenhouse Controller", sizeof(data.systemName) - 1);
    data.systemName[sizeof(data.systemName) - 1] = '\0';
    data.systemMode = Hydro_SystemMode_Recycling;
    data.measureMode = Hydro_MeasurementMode_Imperial;
    data.timeZoneOffset = -7.0f;
    data.pollingInterval = 500;
    data.latitude = 49.2827;
    data.longitude = -123.1207;
    data.altitude = 70.0;
    data.scheduler.baseFeedMultiplier = 1.25f;
    data.scheduler.totalFeedingsPerDay = 4;
    data.scheduler.airReportInterval = 3600;
    data.logger.logLevel = Hydro_LogLevel_Warnings;

    StaticJsonDocument<2048> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);

    HydroSystemData decoded;
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();
    decoded.fromJSONObject(objectConst);
    assert(decoded.isSystemData());
    assert(strcmp(decoded.systemName, data.systemName) == 0);
    assert(decoded.systemMode == data.systemMode);
    assert(decoded.measureMode == data.measureMode);
    assert(isFPEqual(decoded.timeZoneOffset, data.timeZoneOffset));
    assert(decoded.pollingInterval == data.pollingInterval);
    assert(isFPEqual(decoded.latitude, data.latitude));
    assert(isFPEqual(decoded.longitude, data.longitude));
    assert(isFPEqual(decoded.altitude, data.altitude));
    assert(isFPEqual(decoded.scheduler.baseFeedMultiplier, data.scheduler.baseFeedMultiplier));
    assert(decoded.scheduler.totalFeedingsPerDay == data.scheduler.totalFeedingsPerDay);
    assert(decoded.scheduler.airReportInterval == data.scheduler.airReportInterval);
    assert(decoded.logger.logLevel == data.logger.logLevel);

    HydroData *allocated = newDataFromJSONObject(objectConst);
    assert(allocated && allocated->isSystemData());
    delete allocated;
}

static void testCalibrationData()
{
    HydroCalibrationData data(HydroIdentity(Hydro_SensorType_WaterTemperature, 1), Hydro_UnitsType_Temperature_Celsius);
    data.setFromTwoPoints(0.1f, -10.0f, 0.9f, 30.0f);
    assert(isFPEqual(data.transform(0.5f), 10.0f));
    assert(isFPEqual(data.inverseTransform(10.0f), 0.5f));

    StaticJsonDocument<256> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    HydroCalibrationData decoded;
    decoded.fromJSONObject(objectConst);
    assert(decoded.isCalibrationData());
    assert(strcmp(decoded.ownerName, data.ownerName) == 0);
    assert(decoded.calibrationUnits == data.calibrationUnits);
    assert(isFPEqual(decoded.multiplier, data.multiplier));
    assert(isFPEqual(decoded.offset, data.offset));
}

static void testActuatorData()
{
    HydroActuatorData data;
    data.id.object.idType = HydroIdentity::Actuator;
    data.id.object.objType = Hydro_ActuatorType_GrowLights;
    data.id.object.posIndex = 2;
    data.id.object.classType = HydroActuator::Relay;
    data.enableMode = Hydro_EnableMode_InOrder;
    HydroDigitalPin(8, Hydro_PinMode_Digital_Output_PushPull, false).saveToData(&data.outputPin);
    strncpy(data.railName, "AC110V #0", sizeof(data.railName) - 1);
    data.railName[sizeof(data.railName) - 1] = '\0';
    strncpy(data.reservoirName, "FeedWater #0", sizeof(data.reservoirName) - 1);
    data.reservoirName[sizeof(data.reservoirName) - 1] = '\0';

    StaticJsonDocument<512> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    HydroData *allocated = newDataFromJSONObject(objectConst);
    assert(allocated && allocated->isObjectData());
    HydroActuatorData *decoded = static_cast<HydroActuatorData *>(allocated);
    assert(decoded->id.object.idType == HydroIdentity::Actuator);
    assert(decoded->id.object.objType == Hydro_ActuatorType_GrowLights);
    assert(decoded->id.object.posIndex == 2);
    assert(decoded->id.object.classType == HydroActuator::Relay);
    assert(decoded->enableMode == Hydro_EnableMode_InOrder);
    assert(decoded->outputPin.pin == 8);
    assert(!decoded->outputPin.dataAs.digitalPin.activeLow);
    assert(strcmp(decoded->railName, data.railName) == 0);
    assert(strcmp(decoded->reservoirName, data.reservoirName) == 0);
    delete allocated;
}

static void testBinarySensorData()
{
    HydroBinarySensorData data;
    data.id.object.idType = HydroIdentity::Sensor;
    data.id.object.objType = Hydro_SensorType_WaterLevel;
    data.id.object.posIndex = 1;
    data.id.object.classType = HydroSensor::Binary;
    HydroDigitalPin(14, Hydro_PinMode_Digital_Input_PullUp, true).saveToData(&data.inputPin);
    data.usingISR = false;
    data.stateStableTimeMs = 75;
    strncpy(data.reservoirName, "FeedWater #0", sizeof(data.reservoirName) - 1);
    data.reservoirName[sizeof(data.reservoirName) - 1] = '\0';

    StaticJsonDocument<512> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    HydroData *allocated = newDataFromJSONObject(objectConst);
    assert(allocated && allocated->isObjectData());
    HydroBinarySensorData *decoded = static_cast<HydroBinarySensorData *>(allocated);
    assert(decoded->id.object.classType == HydroSensor::Binary);
    assert(decoded->inputPin.pin == 14);
    assert(decoded->inputPin.dataAs.digitalPin.activeLow);
    assert(decoded->stateStableTimeMs == 75);
    assert(strcmp(decoded->reservoirName, data.reservoirName) == 0);

    HydroSensor *sensor = newSensorObjectFromData(decoded);
    assert(sensor && sensor->isBinaryClass() && !sensor->isDigitalClass());
    delete sensor;
    delete allocated;
}

static void testTriggerSubData()
{
    HydroTriggerSubData data;
    data.type = HydroTrigger::MeasureValue;
    strncpy(data.sensorName, "WaterTemperature #1", sizeof(data.sensorName) - 1);
    data.sensorName[sizeof(data.sensorName) - 1] = '\0';
    data.measurementRow = 0;
    data.measurementUnits = Hydro_UnitsType_Temperature_Celsius;
    data.detriggerTol = 0.5f;
    data.detriggerDelay = 1000;
    data.dataAs.measureValue.tolerance = 5.0f;
    data.dataAs.measureValue.triggerBelow = true;

    StaticJsonDocument<384> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    HydroTriggerSubData decoded;
    decoded.fromJSONObject(objectConst);
    assert(decoded.type == data.type);
    assert(strcmp(decoded.sensorName, data.sensorName) == 0);
    assert(decoded.measurementRow == data.measurementRow);
    assert(decoded.measurementUnits == data.measurementUnits);
    assert(isFPEqual(decoded.detriggerTol, data.detriggerTol));
    assert(decoded.detriggerDelay == data.detriggerDelay);
    assert(isFPEqual(decoded.dataAs.measureValue.tolerance, data.dataAs.measureValue.tolerance));
    assert(decoded.dataAs.measureValue.triggerBelow == data.dataAs.measureValue.triggerBelow);
}

int main()
{
    testSystemData();
    testCalibrationData();
    testActuatorData();
    testBinarySensorData();
    testTriggerSubData();
    puts("PASS serialization");
    return 0;
}
