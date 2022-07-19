// JSON export tests

#include <Hydroponics.h>
#include <TaskScheduler.h>

Hydroponics hydroController;

#define JSONDocSize 256

void setup() {
    Serial.begin(115200);
    while(!Serial) { ; }
    Wire.setClock(hydroController.getI2CSpeed());

    hydroController.init();

    Serial.println(); Serial.println("=BEGIN=");

    Serial.println(); Serial.println("-- Actuators --");

    {   HydroponicsRelayActuator relay(Hydroponics_ActuatorType_GrowLights, 0, 7);
        relay.setRail(HydroponicsIdentity("ASDF"));
        relay.setReservoir(HydroponicsIdentity("JKL"));

        auto data = (HydroponicsRelayActuatorData *)(relay.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("RelayActuator: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(relay)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsPumpRelayActuator pumpRelay(Hydroponics_ActuatorType_WaterPump, 0, 7);
        pumpRelay.setRail(HydroponicsIdentity("ASDF"));
        pumpRelay.setReservoir(HydroponicsIdentity("JKL"));

        auto data = (HydroponicsPumpRelayActuatorData *)(pumpRelay.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("PumpRelayActuator: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(pumpRelay)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsPWMActuator pwmFan(Hydroponics_ActuatorType_FanExhaust, 0, 7);
        pwmFan.setRail(HydroponicsIdentity("ASDF"));
        pwmFan.setReservoir(HydroponicsIdentity("JKL"));

        auto data = (HydroponicsPWMActuatorData *)(pwmFan.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("PumpRelayActuator: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(pwmFan)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    Serial.println(); Serial.println("-- Sensors --");

    {   HydroponicsBinarySensor binarySensor(Hydroponics_SensorType_WaterLevelIndicator, 0, 0);
        binarySensor.setCrop(HydroponicsIdentity("ASDF"));
        binarySensor.setReservoir(HydroponicsIdentity("JKL"));

        auto data = (HydroponicsBinarySensorData *)(binarySensor.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("BinarySensor: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(binarySensor)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsAnalogSensor analogSensor(Hydroponics_SensorType_WaterHeightMeter, 0, 0);
        analogSensor.setCrop(HydroponicsIdentity("ASDF"));
        analogSensor.setReservoir(HydroponicsIdentity("JKL"));

        auto data = (HydroponicsAnalogSensorData *)(analogSensor.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("AnalogSensor: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(analogSensor)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsDHTTempHumiditySensor dhtSensor(0, 0);
        dhtSensor.setCrop(HydroponicsIdentity("ASDF"));
        dhtSensor.setReservoir(HydroponicsIdentity("JKL"));

        auto data = (HydroponicsDHTTempHumiditySensorData *)(dhtSensor.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("DHTTempHumiditySensor: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(dhtSensor)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsDSTemperatureSensor dsSensor(0, 0);
        dsSensor.setCrop(HydroponicsIdentity("ASDF"));
        dsSensor.setReservoir(HydroponicsIdentity("JKL"));

        auto data = (HydroponicsDSTemperatureSensorData *)(dsSensor.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("DSTemperatureSensor: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(dsSensor)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    Serial.println(); Serial.println("-- Crops --");

    {   HydroponicsTimedCrop timedCrop(Hydroponics_CropType_Lettuce, 0, Hydroponics_SubstrateType_ClayPebbles, DateTime((uint32_t)0));
        timedCrop.setFeedReservoir(HydroponicsIdentity("ASDF"));

        auto data = (HydroponicsTimedCropData *)(timedCrop.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("TimedCrop: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(timedCrop)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsAdaptiveCrop adaptiveCrop(Hydroponics_CropType_Lettuce, 0, Hydroponics_SubstrateType_ClayPebbles, DateTime((uint32_t)0));
        adaptiveCrop.setFeedReservoir(HydroponicsIdentity("ASDF"));
        adaptiveCrop.setMoistureSensor(HydroponicsIdentity("JKL"));

        auto data = (HydroponicsAdaptiveCropData *)(adaptiveCrop.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("AdaptiveCrop: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(adaptiveCrop)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    Serial.println(); Serial.println("-- Reservoirs --");

    {   HydroponicsFluidReservoir fluidRes(Hydroponics_ReservoirType_NutrientPremix, 0, 5);
        fluidRes.setVolumeSensor(HydroponicsIdentity("ASDF"));

        auto data = (HydroponicsFluidReservoirData *)(fluidRes.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("FluidReservoir: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(fluidRes)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsFeedReservoir feedRes(0, 5);
        feedRes.setVolumeSensor(HydroponicsIdentity("ASDF"));
        feedRes.setWaterPHSensor(HydroponicsIdentity("JKL"));
        feedRes.setWaterTDSSensor(HydroponicsIdentity("QWER"));
        feedRes.setWaterTempSensor(HydroponicsIdentity("UIOP"));
        feedRes.setAirTempSensor(HydroponicsIdentity("ZXCV"));
        feedRes.setAirCO2Sensor(HydroponicsIdentity("BNM"));

        auto data = (HydroponicsFeedReservoirData *)(feedRes.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("FeedReservoir: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(feedRes)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsInfiniteReservoir pipeRes(Hydroponics_ReservoirType_DrainageWater, 0);

        auto data = (HydroponicsInfiniteReservoirData *)(pipeRes.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("InfiniteReservoir: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(pipeRes)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    Serial.println(); Serial.println("-- Rails --");

    {   HydroponicsSimpleRail rail(Hydroponics_RailType_AC110V, 0);

        auto data = (HydroponicsSimpleRailData *)(rail.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("SimpleRail: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(rail)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    {   HydroponicsRegulatedRail regRail(Hydroponics_RailType_AC110V, 0, 15);
        regRail.setPowerSensor(HydroponicsIdentity("ASDF"));

        auto data = (HydroponicsRegulatedRailData *)(regRail.newSaveData());
        StaticJsonDocument<JSONDocSize> doc;
        JsonObject json = doc.to<JsonObject>();
        data->toJSONObject(json);
        Serial.println(); Serial.print("RegulatedRail: ");
        Serial.print(measureJsonPretty(json)); Serial.print("B data, ");
        Serial.print((int)sizeof(regRail)); Serial.println("B class");
        serializeJsonPretty(json, Serial);

        if (data) { delete data; }
        Serial.println(); Serial.flush(); yield();
    }

    Serial.println(); Serial.println("=FINISH=");
}

void loop()
{
    //hydroController.update();
}
