// JSON export tests

#include <Hydroponics.h>

Hydroponics hydroController;

#define JSONDocSize 256

void testActuators()
{
    Serial.println(); Serial.println("-- Actuators --");

    {   auto relay = arx::stdx::make_shared<HydroponicsRelayActuator>(Hydroponics_ActuatorType_GrowLights, 0, 7);
        relay->setRail(HydroponicsIdentity(String(F("ASDF"))));
        relay->setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsRelayActuatorData *)(relay->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("RelayActuator: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*relay.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto pumpRelay = arx::stdx::make_shared<HydroponicsPumpRelayActuator>(Hydroponics_ActuatorType_WaterPump, 0, 7);
        pumpRelay->setRail(HydroponicsIdentity(String(F("ASDF"))));
        pumpRelay->setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsPumpRelayActuatorData *)(pumpRelay->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("PumpRelayActuator: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*pumpRelay.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto pwmFan = arx::stdx::make_shared<HydroponicsPWMActuator>(Hydroponics_ActuatorType_FanExhaust, 0, 7);
        pwmFan->setRail(HydroponicsIdentity(String(F("ASDF"))));
        pwmFan->setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsPWMActuatorData *)(pwmFan->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("PumpRelayActuator: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*pwmFan.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }
}

void testSensors()
{
    Serial.println(); Serial.println("-- Sensors --");

    {   auto binarySensor = arx::stdx::make_shared<HydroponicsBinarySensor>(Hydroponics_SensorType_WaterLevelIndicator, 0, 0);
        binarySensor->setCrop(HydroponicsIdentity(String(F("ASDF"))));
        binarySensor->setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsBinarySensorData *)(binarySensor->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("BinarySensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*binarySensor.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto analogSensor = arx::stdx::make_shared<HydroponicsAnalogSensor>(Hydroponics_SensorType_WaterHeightMeter, 0, 0);
        analogSensor->setCrop(HydroponicsIdentity(String(F("ASDF"))));
        analogSensor->setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsAnalogSensorData *)(analogSensor->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("AnalogSensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*analogSensor.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto dhtSensor = arx::stdx::make_shared<HydroponicsDHTTempHumiditySensor>(0, 0);
        dhtSensor->setCrop(HydroponicsIdentity(String(F("ASDF"))));
        dhtSensor->setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsDHTTempHumiditySensorData *)(dhtSensor->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("DHTTempHumiditySensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*dhtSensor.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto dsSensor = arx::stdx::make_shared<HydroponicsDSTemperatureSensor>(0, 0);
        dsSensor->setCrop(HydroponicsIdentity(String(F("ASDF"))));
        dsSensor->setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsDSTemperatureSensorData *)(dsSensor->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("DSTemperatureSensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*dsSensor.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }
}

void testCrops()
{
    Serial.println(); Serial.println("-- Crops --");

    {   auto timedCrop = arx::stdx::make_shared<HydroponicsTimedCrop>(Hydroponics_CropType_Lettuce, 0, Hydroponics_SubstrateType_ClayPebbles, DateTime());
        timedCrop->setFeedReservoir(HydroponicsIdentity(String(F("ASDF"))));

        auto data = (HydroponicsTimedCropData *)(timedCrop->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("TimedCrop: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*timedCrop.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto adaptiveCrop = arx::stdx::make_shared<HydroponicsAdaptiveCrop>(Hydroponics_CropType_Lettuce, 0, Hydroponics_SubstrateType_ClayPebbles, DateTime());
        adaptiveCrop->setFeedReservoir(HydroponicsIdentity(String(F("ASDF"))));
        adaptiveCrop->setSoilMoistureSensor(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsAdaptiveCropData *)(adaptiveCrop->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("AdaptiveCrop: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*adaptiveCrop.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }
}

void testReservoirs()
{
    Serial.println(); Serial.println("-- Reservoirs --");

    {   auto fluidRes = arx::stdx::make_shared<HydroponicsFluidReservoir>(Hydroponics_ReservoirType_NutrientPremix, 0, 5);
        fluidRes->setWaterVolumeSensor(HydroponicsIdentity(String(F("ASDF"))));

        auto data = (HydroponicsFluidReservoirData *)(fluidRes->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("FluidReservoir: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*fluidRes.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto feedRes = arx::stdx::make_shared<HydroponicsFeedReservoir>(0, 5);
        feedRes->setWaterVolumeSensor(HydroponicsIdentity(String(F("ASDF"))));
        feedRes->setWaterPHSensor(HydroponicsIdentity(String(F("JKL"))));
        feedRes->setWaterTDSSensor(HydroponicsIdentity(String(F("QWER"))));
        feedRes->setWaterTemperatureSensor(HydroponicsIdentity(String(F("UIOP"))));
        feedRes->setAirTemperatureSensor(HydroponicsIdentity(String(F("ZXCV"))));
        feedRes->setAirCO2Sensor(HydroponicsIdentity(String(F("BNM"))));

        auto data = (HydroponicsFeedReservoirData *)(feedRes->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("FeedReservoir: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*feedRes.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto pipeRes = arx::stdx::make_shared<HydroponicsInfiniteReservoir>(Hydroponics_ReservoirType_DrainageWater, 0);

        auto data = (HydroponicsInfiniteReservoirData *)(pipeRes->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("InfiniteReservoir: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*pipeRes.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }
}

void testRails()
{
    Serial.println(); Serial.println("-- Rails --");

    {   auto rail = arx::stdx::make_shared<HydroponicsSimpleRail>(Hydroponics_RailType_AC110V, 0);

        auto data = (HydroponicsSimpleRailData *)(rail->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("SimpleRail: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*rail.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto regRail = arx::stdx::make_shared<HydroponicsRegulatedRail>(Hydroponics_RailType_AC110V, 0, 15);
        regRail->setPowerUsageSensor(HydroponicsIdentity(String(F("ASDF"))));

        auto data = (HydroponicsRegulatedRailData *)(regRail->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("RegulatedRail: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*regRail.get())); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }
    Wire.setClock(hydroController.getI2CSpeed());

    hydroController.init();

    Serial.println(); Serial.println("=BEGIN=");

    testActuators();
    testSensors();
    testCrops();
    testReservoirs();
    testRails();

    Serial.println(); Serial.println(F("=FINISH="));
}

void loop()
{
    //hydroController.update();
}
