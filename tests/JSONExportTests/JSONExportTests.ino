// JSON export tests

#include <Hydroponics.h>

Hydroponics hydroController;

#define JSONDocSize 256

void testActuators()
{
    Serial.println(); Serial.println("-- Actuators --");

    {   HydroponicsRelayActuator relay(Hydroponics_ActuatorType_GrowLights, 0, 7);
        relay.setRail(HydroponicsIdentity(String(F("ASDF"))));
        relay.setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsRelayActuatorData *)(relay.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("RelayActuator: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(relay)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsPumpRelayActuator pumpRelay(Hydroponics_ActuatorType_WaterPump, 0, 7);
        pumpRelay.setRail(HydroponicsIdentity(String(F("ASDF"))));
        pumpRelay.setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsPumpRelayActuatorData *)(pumpRelay.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("PumpRelayActuator: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(pumpRelay)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsPWMActuator pwmFan(Hydroponics_ActuatorType_FanExhaust, 0, 7);
        pwmFan.setRail(HydroponicsIdentity(String(F("ASDF"))));
        pwmFan.setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsPWMActuatorData *)(pwmFan.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("PumpRelayActuator: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(pwmFan)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }
}

void testSensors()
{
    Serial.println(); Serial.println("-- Sensors --");

    {   HydroponicsBinarySensor binarySensor(Hydroponics_SensorType_WaterLevelIndicator, 0, 0);
        binarySensor.setCrop(HydroponicsIdentity(String(F("ASDF"))));
        binarySensor.setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsBinarySensorData *)(binarySensor.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("BinarySensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(binarySensor)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsAnalogSensor analogSensor(Hydroponics_SensorType_WaterHeightMeter, 0, 0);
        analogSensor.setCrop(HydroponicsIdentity(String(F("ASDF"))));
        analogSensor.setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsAnalogSensorData *)(analogSensor.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("AnalogSensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(analogSensor)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsDHTTempHumiditySensor dhtSensor(0, 0);
        dhtSensor.setCrop(HydroponicsIdentity(String(F("ASDF"))));
        dhtSensor.setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsDHTTempHumiditySensorData *)(dhtSensor.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("DHTTempHumiditySensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(dhtSensor)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsDSTemperatureSensor dsSensor(0, 0);
        dsSensor.setCrop(HydroponicsIdentity(String(F("ASDF"))));
        dsSensor.setReservoir(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsDSTemperatureSensorData *)(dsSensor.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("DSTemperatureSensor: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(dsSensor)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }
}

void testCrops()
{
    Serial.println(); Serial.println("-- Crops --");

    {   HydroponicsTimedCrop timedCrop(Hydroponics_CropType_Lettuce, 0, Hydroponics_SubstrateType_ClayPebbles, DateTime((uint32_t)0));
        timedCrop.setFeedReservoir(HydroponicsIdentity(String(F("ASDF"))));

        auto data = (HydroponicsTimedCropData *)(timedCrop.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("TimedCrop: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(timedCrop)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsAdaptiveCrop adaptiveCrop(Hydroponics_CropType_Lettuce, 0, Hydroponics_SubstrateType_ClayPebbles, DateTime((uint32_t)0));
        adaptiveCrop.setFeedReservoir(HydroponicsIdentity(String(F("ASDF"))));
        adaptiveCrop.setMoistureSensor(HydroponicsIdentity(String(F("JKL"))));

        auto data = (HydroponicsAdaptiveCropData *)(adaptiveCrop.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("AdaptiveCrop: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(adaptiveCrop)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }
}

void testReservoirs()
{
    Serial.println(); Serial.println("-- Reservoirs --");

    {   HydroponicsFluidReservoir fluidRes(Hydroponics_ReservoirType_NutrientPremix, 0, 5);
        fluidRes.setVolumeSensor(HydroponicsIdentity(String(F("ASDF"))));

        auto data = (HydroponicsFluidReservoirData *)(fluidRes.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("FluidReservoir: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(fluidRes)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsFeedReservoir feedRes(0, 5);
        feedRes.setVolumeSensor(HydroponicsIdentity(String(F("ASDF"))));
        feedRes.setWaterPHSensor(HydroponicsIdentity(String(F("JKL"))));
        feedRes.setWaterTDSSensor(HydroponicsIdentity(String(F("QWER"))));
        feedRes.setWaterTempSensor(HydroponicsIdentity(String(F("UIOP"))));
        feedRes.setAirTempSensor(HydroponicsIdentity(String(F("ZXCV"))));
        feedRes.setAirCO2Sensor(HydroponicsIdentity(String(F("BNM"))));

        auto data = (HydroponicsFeedReservoirData *)(feedRes.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("FeedReservoir: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(feedRes)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsInfiniteReservoir pipeRes(Hydroponics_ReservoirType_DrainageWater, 0);

        auto data = (HydroponicsInfiniteReservoirData *)(pipeRes.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("InfiniteReservoir: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(pipeRes)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }
}

void testRails()
{
    Serial.println(); Serial.println("-- Rails --");

    {   HydroponicsSimpleRail rail(Hydroponics_RailType_AC110V, 0);

        auto data = (HydroponicsSimpleRailData *)(rail.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("SimpleRail: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(rail)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsRegulatedRail regRail(Hydroponics_RailType_AC110V, 0, 15);
        regRail.setPowerSensor(HydroponicsIdentity(String(F("ASDF"))));

        auto data = (HydroponicsRegulatedRailData *)(regRail.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print(F("RegulatedRail: "));
        Serial.print(measureJsonPretty(json)); Serial.print(F("B data, "));
        Serial.print((int)sizeof(regRail)); Serial.println(F("B class"));
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }
}

void setup() {
    Serial.begin(115200);
    while(!Serial) { ; }
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
