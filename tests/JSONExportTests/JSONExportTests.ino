// JSON export tests

#include <Hydruino.h>

Hydruino hydroController;

#define JSONDocSize 256

void testActuators()
{
    Serial.println(); Serial.println("-- Actuators --");

    {   auto relay = SharedPtr<HydroRelayActuator>(new HydroRelayActuator(Hydro_ActuatorType_GrowLights, 0, HydroDigitalPin(7, OUTPUT)));
        relay->setRail(HydroIdentity(String(F("ASDF"))));
        relay->setReservoir(HydroIdentity(String(F("JKL"))));

        auto data = (HydroActuatorData *)(relay->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("RelayActuator: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*relay)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto pumpRelay = SharedPtr<HydroRelayPumpActuator>(new HydroRelayPumpActuator(Hydro_ActuatorType_WaterPump, 0, HydroDigitalPin(7, OUTPUT)));
        pumpRelay->setRail(HydroIdentity(String(F("ASDF"))));
        pumpRelay->setReservoir(HydroIdentity(String(F("JKL"))));

        auto data = (HydroPumpActuatorData *)(pumpRelay->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("PumpRelayActuator: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*pumpRelay)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto pwmFan = SharedPtr<HydroVariableActuator>(new HydroVariableActuator(Hydro_ActuatorType_FanExhaust, 0, HydroAnalogPin(7, OUTPUT)));
        pwmFan->setRail(HydroIdentity(String(F("ASDF"))));
        pwmFan->setReservoir(HydroIdentity(String(F("JKL"))));

        auto data = (HydroActuatorData *)(pwmFan->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("PumpRelayActuator: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*pwmFan)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }
}

void testSensors()
{
    Serial.println(); Serial.println("-- Sensors --");

    {   auto binarySensor = SharedPtr<HydroBinarySensor>(new HydroBinarySensor(Hydro_SensorType_WaterLevel, 0, 0));
        binarySensor->setCrop(HydroIdentity(String(F("ASDF"))));
        binarySensor->setReservoir(HydroIdentity(String(F("JKL"))));

        auto data = (HydroBinarySensorData *)(binarySensor->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("BinarySensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*binarySensor)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto analogSensor = SharedPtr<HydroAnalogSensor>(new HydroAnalogSensor(Hydro_SensorType_WaterHeight, 0, 0));
        analogSensor->setCrop(HydroIdentity(String(F("ASDF"))));
        analogSensor->setReservoir(HydroIdentity(String(F("JKL"))));

        auto data = (HydroAnalogSensorData *)(analogSensor->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("AnalogSensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*analogSensor)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto dhtSensor = SharedPtr<HydroDHTTempHumiditySensor>(new HydroDHTTempHumiditySensor(0, 0, Hydro_DHTType_DHT12));
        dhtSensor->setCrop(HydroIdentity(String(F("ASDF"))));
        dhtSensor->setReservoir(HydroIdentity(String(F("JKL"))));

        auto data = (HydroDHTTempHumiditySensorData *)(dhtSensor->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("DHTTempHumiditySensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*dhtSensor)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto dsSensor = SharedPtr<HydroDSTemperatureSensor>(new HydroDSTemperatureSensor(0, 0));
        dsSensor->setCrop(HydroIdentity(String(F("ASDF"))));
        dsSensor->setReservoir(HydroIdentity(String(F("JKL"))));

        auto data = (HydroDSTemperatureSensorData *)(dsSensor->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("DSTemperatureSensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*dsSensor)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }
}

void testCrops()
{
    Serial.println(); Serial.println("-- Crops --");

    {   auto timedCrop = SharedPtr<HydroTimedCrop>(new HydroTimedCrop(Hydro_CropType_Lettuce, 0, Hydro_SubstrateType_ClayPebbles, DateTime()));
        timedCrop->setFeedReservoir(HydroIdentity(String(F("ASDF"))));

        auto data = (HydroTimedCropData *)(timedCrop->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("TimedCrop: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*timedCrop)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto adaptiveCrop = SharedPtr<HydroAdaptiveCrop>(new HydroAdaptiveCrop(Hydro_CropType_Lettuce, 0, Hydro_SubstrateType_ClayPebbles, DateTime()));
        adaptiveCrop->setFeedReservoir(HydroIdentity(String(F("ASDF"))));
        adaptiveCrop->setSoilMoistureSensor(HydroIdentity(String(F("JKL"))));

        auto data = (HydroAdaptiveCropData *)(adaptiveCrop->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("AdaptiveCrop: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*adaptiveCrop)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }
}

void testReservoirs()
{
    Serial.println(); Serial.println("-- Reservoirs --");

    {   auto fluidRes = SharedPtr<HydroFluidReservoir>(new HydroFluidReservoir(Hydro_ReservoirType_NutrientPremix, 0, 5));
        fluidRes->setWaterVolumeSensor(HydroIdentity(String(F("ASDF"))));

        auto data = (HydroFluidReservoirData *)(fluidRes->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("FluidReservoir: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*fluidRes)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto feedRes = SharedPtr<HydroFeedReservoir>(new HydroFeedReservoir(0, 5));
        feedRes->setWaterVolumeSensor(HydroIdentity(String(F("ASDF"))));
        feedRes->setWaterPHSensor(HydroIdentity(String(F("JKL"))));
        feedRes->setWaterTDSSensor(HydroIdentity(String(F("QWER"))));
        feedRes->setWaterTemperatureSensor(HydroIdentity(String(F("UIOP"))));
        feedRes->setAirTemperatureSensor(HydroIdentity(String(F("ZXCV"))));
        feedRes->setAirCO2Sensor(HydroIdentity(String(F("BNM"))));

        auto data = (HydroFeedReservoirData *)(feedRes->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("FeedReservoir: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*feedRes)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto pipeRes = SharedPtr<HydroInfiniteReservoir>(new HydroInfiniteReservoir(Hydro_ReservoirType_DrainageWater, 0));

        auto data = (HydroInfiniteReservoirData *)(pipeRes->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("InfiniteReservoir: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*pipeRes)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }
}

void testRails()
{
    Serial.println(); Serial.println("-- Rails --");

    {   auto rail = SharedPtr<HydroSimpleRail>(new HydroSimpleRail(Hydro_RailType_AC110V, 0));

        auto data = (HydroSimpleRailData *)(rail->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("SimpleRail: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*rail)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }

    {   auto regRail = SharedPtr<HydroRegulatedRail>(new HydroRegulatedRail(Hydro_RailType_AC110V, 0, 15));
        regRail->setPowerUsageSensor(HydroIdentity(String(F("ASDF"))));

        auto data = (HydroRegulatedRailData *)(regRail->newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("RegulatedRail: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(*regRail)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); flushYield();
    }
}

void setup() {
    Serial.begin(115200);           // Begin USB Serial interface
    while (!Serial) { ; }           // Wait for USB Serial to connect

    hydroController.init();

    getLogger()->logMessage(F("=BEGIN="));

    testActuators();
    testSensors();
    testCrops();
    testReservoirs();
    testRails();

    getLogger()->logMessage(F("=FINISH="));
}

void loop()
{
    //hydroController.update();
}
