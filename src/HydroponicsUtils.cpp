/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#include "HydroponicsUtils.h"
#include <pins_arduino.h>

bool tryConvertValue(float valueIn, Hydroponics_UnitsType unitsIn, float *valueOut, Hydroponics_UnitsType unitsOut)
{
    switch (unitsIn) {
        case Hydroponics_UnitsType_Temperature_Celsius:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Temperature_Fahrenheit:
                    if (valueOut) {
                        *valueOut = valueIn * 1.8 + 32.0;
                        return true;
                    }
                    break;

                case Hydroponics_UnitsType_Temperature_Kelvin:
                    if (valueOut) {
                        *valueOut = valueIn + 273.15;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Temperature_Fahrenheit:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Temperature_Celsius:
                    if (valueOut) {
                        *valueOut = (valueIn - 32.0) / 1.8;
                        return true;
                    }
                    break;

                case Hydroponics_UnitsType_Temperature_Kelvin:
                    if (valueOut) {
                        *valueOut = ((valueIn + 459.67f) * 5.0f) / 9.0f;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Temperature_Kelvin:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Temperature_Celsius:
                    if (valueOut) {
                        *valueOut = valueIn - 273.15;
                        return true;
                    }
                    break;

                case Hydroponics_UnitsType_Temperature_Fahrenheit:
                    if (valueOut) {
                        *valueOut = ((valueIn * 9.0) / 5.0) - 459.67;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Distance_Meters:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Distance_Feet:
                    if (valueOut) {
                        *valueOut = valueIn * 3.28084f;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Distance_Feet:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Distance_Meters:
                    if (valueOut) {
                        *valueOut = valueIn * 0.3048;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Weight_Kilogram:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Weight_Pounds:
                    if (valueOut) {
                        *valueOut = valueIn * 2.20462f;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Weight_Pounds:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Weight_Kilogram:
                    if (valueOut) {
                        *valueOut = valueIn * 0.453592f;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiquidVolume_Liters:
        case Hydroponics_UnitsType_LiquidFlow_LitersPerMin:
            switch(unitsOut) {
                case Hydroponics_UnitsType_LiquidVolume_Gallons:
                case Hydroponics_UnitsType_LiquidFlow_GallonsPerMin:
                    if (valueOut) {
                        *valueOut = valueIn * 0.264172f;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_LiquidVolume_Gallons:
        case Hydroponics_UnitsType_LiquidFlow_GallonsPerMin:
            switch(unitsOut) {
                case Hydroponics_UnitsType_LiquidVolume_Liters:
                case Hydroponics_UnitsType_LiquidFlow_LitersPerMin:
                    if (valueOut) {
                        *valueOut = valueIn * 3.78541f;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_pHScale_0_14:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    if (valueOut) {
                        *valueOut = valueIn / 14.0f; // FIXME no idea if this is correct
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Concentration_EC:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    if (valueOut) {
                        *valueOut = valueIn / 5.0f; // FIXME no idea if this is correct
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Concentration_PPM:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    if (valueOut) {
                        *valueOut = valueIn / 1000.0f; // FIXME no idea if this is correct
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Percentile_0_100:
            switch(unitsOut) {
                case Hydroponics_UnitsType_Raw_0_1:
                    if (valueOut) {
                        *valueOut = valueIn / 100.0f;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case Hydroponics_UnitsType_Raw_0_1:
            switch(unitsOut) {
                case Hydroponics_UnitsType_pHScale_0_14:
                    if (valueOut) {
                        *valueOut = valueIn * 14.0f; // FIXME no idea if this is correct
                        return true;
                    }
                    break;

                case Hydroponics_UnitsType_Concentration_EC:
                    if (valueOut) {
                        *valueOut = valueIn * 5.0f; // FIXME no idea if this is correct
                        return true;
                    }
                    break;

                case Hydroponics_UnitsType_Concentration_PPM:
                    if (valueOut) {
                        *valueOut = valueIn * 1000.0f; // FIXME no idea if this is correct
                        return true;
                    }
                    break;

                case Hydroponics_UnitsType_Percentile_0_100:
                    if (valueOut) {
                        *valueOut = valueIn * 100.0f;
                        return true;
                    }
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return false;
}

void convertAndAssign(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType unitsOut, int roundToDecPlaces)
{
    if (tryConvertValue(*valueInOut, *unitsInOut, valueInOut, unitsOut)) {
        *unitsInOut = unitsOut;
        if (roundToDecPlaces >= 0) {
            float shiftScaler = powf(10.0f, roundToDecPlaces);
            *valueInOut = roundf(*valueInOut * shiftScaler) / shiftScaler;
        }
    }
}

bool checkInputPinIsAnalog(int pin)
{
    #if !defined(NUM_ANALOG_INPUTS) || NUM_ANALOG_INPUTS == 0
        return false;
    #else
        switch (pin) {
            #if NUM_ANALOG_INPUTS > 0
                case (int)A0:
            #endif
            #if NUM_ANALOG_INPUTS > 1
                case (int)A1:
            #endif
            #if NUM_ANALOG_INPUTS > 2
                case (int)A2:
            #endif
            #if NUM_ANALOG_INPUTS > 3
                case (int)A3:
            #endif
            #if NUM_ANALOG_INPUTS > 4
                case (int)A4:
            #endif
            #if NUM_ANALOG_INPUTS > 5
                case (int)A5:
            #endif
            #if NUM_ANALOG_INPUTS > 6
                case (int)A6:
            #endif
            #if NUM_ANALOG_INPUTS > 7
                case (int)A7:
            #endif
            #if NUM_ANALOG_INPUTS > 8
                case (int)A8:
            #endif
            #if NUM_ANALOG_INPUTS > 9
                case (int)A9:
            #endif
            #if NUM_ANALOG_INPUTS > 10
                case (int)A10:
            #endif
            #if NUM_ANALOG_INPUTS > 11
                case (int)A11:
            #endif
            #if NUM_ANALOG_INPUTS > 12
                case (int)A12:
            #endif
            #if NUM_ANALOG_INPUTS > 13
                case (int)A13:
            #endif
            #if NUM_ANALOG_INPUTS > 14
                case (int)A14:
            #endif
            #if NUM_ANALOG_INPUTS > 15
                case (int)A15:
            #endif
                return true;

            default:
                return false;
        }
    #endif
}

bool checkOutputPinIsAnalog(int pin)
{
    #if !defined(NUM_ANALOG_OUTPUTS) || NUM_ANALOG_OUTPUTS == 0
        return false;
    #else
        switch (pin) {
            #if NUM_ANALOG_OUTPUTS > 0
                case (int)DAC0:
            #endif
            #if NUM_ANALOG_OUTPUTS > 1
                case (int)DAC1:
            #endif
            #if NUM_ANALOG_OUTPUTS > 2
                case (int)DAC2:
            #endif
            #if NUM_ANALOG_OUTPUTS > 3
                case (int)DAC3:
            #endif
            #if NUM_ANALOG_OUTPUTS > 4
                case (int)DAC4:
            #endif
            #if NUM_ANALOG_OUTPUTS > 5
                case (int)DAC5:
            #endif
            #if NUM_ANALOG_OUTPUTS > 6
                case (int)DAC6:
            #endif
            #if NUM_ANALOG_OUTPUTS > 7
                case (int)DAC7:
            #endif
                return true;

            default:
                return false;
        }
    #endif
}

bool checkPinIsDigital(int pin)
{
    return !checkInputPinIsAnalog(pin) && !checkOutputPinIsAnalog(pin);
}

bool checkPinIsPWM(int pin)
{
    return digitalPinHasPWM(pin);
}

bool checkPinCanInterrupt(int pin)
{
    // TODO no idea how to check for this :S
    return false;
}

String stringForActuatorType(Hydroponics_ActuatorType actuatorType, bool excludeSpecial)
{
    switch (actuatorType) {
        case Hydroponics_ActuatorType_GrowLightsRelay:
            return F("GrowLights");
        case Hydroponics_ActuatorType_WaterPumpRelay:
            return F("WaterPump");
        case Hydroponics_ActuatorType_PeristalticPumpRelay:
            return F("PeristalticPump");
        case Hydroponics_ActuatorType_WaterHeaterRelay:
            return F("WaterHeater");
        case Hydroponics_ActuatorType_WaterAeratorRelay:
            return F("WaterAerator");
        case Hydroponics_ActuatorType_FanExhaustRelay:
            return F("FanExhaust");
        case Hydroponics_ActuatorType_FanExhaustPWM:
            return F("VarFanExhaust");
        case Hydroponics_ActuatorType_Count:
            return !excludeSpecial ? F("Count") : F("");
        case Hydroponics_ActuatorType_Undefined:
            break;
    }
    return !excludeSpecial ? F("Undefined") : F("");
}

String stringForSensorType(Hydroponics_SensorType sensorType, bool excludeSpecial)
{
    switch (sensorType) {
        case Hydroponics_SensorType_AirTempHumidity:
            return F("AirTempHumid");
        case Hydroponics_SensorType_AirCarbonDioxide:
            return F("AirCO2");
        case Hydroponics_SensorType_PotentialHydrogen:
            return F("PHMeter");
        case Hydroponics_SensorType_TotalDissolvedSolids:
            return F("TDSMeter");
        case Hydroponics_SensorType_WaterTemperature:
            return F("WaterTemp");
        case Hydroponics_SensorType_WaterPumpFlowSensor:
            return F("PumpFlow");
        case Hydroponics_SensorType_LowWaterLevelIndicator:
            return F("LowLevel");
        case Hydroponics_SensorType_HighWaterLevelIndicator:
            return F("HighLevel");
        case Hydroponics_SensorType_LowWaterHeightMeter:
            return F("LowHeight");
        case Hydroponics_SensorType_HighWaterHeightMeter:
            return F("HighHeight");
        case Hydroponics_SensorType_Count:
            return !excludeSpecial ? F("Count") : F("");
        case Hydroponics_SensorType_Undefined:
            break;
    }
    return !excludeSpecial ? F("Undefined") : F("");
}

String stringForCropType(Hydroponics_CropType cropType, bool excludeSpecial)
{
    switch (cropType) {
        case Hydroponics_CropType_AloeVera:
            return F("AloeVera");
        case Hydroponics_CropType_Anise:
            return F("Anise");
        case Hydroponics_CropType_Artichoke:
            return F("Artichoke");
        case Hydroponics_CropType_Arugula:
            return F("Arugula");
        case Hydroponics_CropType_Asparagus:
            return F("Asparagus");
        case Hydroponics_CropType_Basil:
            return F("Basil");
        case Hydroponics_CropType_Bean:
            return F("Bean");
        case Hydroponics_CropType_BeanBroad:
            return F("BeanBroad");
        case Hydroponics_CropType_Beetroot:
            return F("Beetroot");
        case Hydroponics_CropType_BlackCurrant:
            return F("BlackCurrant");
        case Hydroponics_CropType_Blueberry:
            return F("Blueberry");
        case Hydroponics_CropType_BokChoi:
            return F("BokChoi");
        case Hydroponics_CropType_Broccoli:
            return F("Broccoli");
        case Hydroponics_CropType_BrussellSprouts:
            return F("BrussellSprouts");
        case Hydroponics_CropType_Cabbage:
            return F("Cabbage");
        case Hydroponics_CropType_Cannabis:
            return F("Cannabis");
        case Hydroponics_CropType_Capiscum:
            return F("Capiscum");
        case Hydroponics_CropType_Carrots:
            return F("Carrots");
        case Hydroponics_CropType_Catnip:
            return F("Catnip");
        case Hydroponics_CropType_Cauliflower:
            return F("Cauliflower");
        case Hydroponics_CropType_Celery:
            return F("Celery");
        case Hydroponics_CropType_Chamomile:
            return F("Chamomile");
        case Hydroponics_CropType_Chickory:
            return F("Chickory");
        case Hydroponics_CropType_Chives:
            return F("Chives");
        case Hydroponics_CropType_Cilantro:
            return F("Cilantro");
        case Hydroponics_CropType_Coriander:
            return F("Coriander");
        case Hydroponics_CropType_CornSweet:
            return F("CornSweet");
        case Hydroponics_CropType_Cucumber:
            return F("Cucumber");
        case Hydroponics_CropType_Dill:
            return F("Dill");
        case Hydroponics_CropType_Eggplant:
            return F("Eggplant");
        case Hydroponics_CropType_Endive:
            return F("Endive");
        case Hydroponics_CropType_Fennel:
            return F("Fennel");
        case Hydroponics_CropType_Fodder:
            return F("Fodder");
        case Hydroponics_CropType_Flowers:
            return F("Flowers");
        case Hydroponics_CropType_Garlic:
            return F("Garlic");
        case Hydroponics_CropType_Ginger:
            return F("Ginger");
        case Hydroponics_CropType_Kale:
            return F("Kale");
        case Hydroponics_CropType_Lavender:
            return F("Lavender");
        case Hydroponics_CropType_Leek:
            return F("Leek");
        case Hydroponics_CropType_LemonBalm:
            return F("LemonBalm");
        case Hydroponics_CropType_Lettuce:
            return F("Lettuce");
        case Hydroponics_CropType_Marrow:
            return F("Marrow");
        case Hydroponics_CropType_Melon:
            return F("Melon");
        case Hydroponics_CropType_Mint:
            return F("Mint");
        case Hydroponics_CropType_MustardCress:
            return F("MustardCress");
        case Hydroponics_CropType_Okra:
            return F("Okra");
        case Hydroponics_CropType_Onions:
            return F("Onions");
        case Hydroponics_CropType_Oregano:
            return F("Oregano");
        case Hydroponics_CropType_PakChoi:
            return F("PakChoi");
        case Hydroponics_CropType_Parsley:
            return F("Parsley");
        case Hydroponics_CropType_Parsnip:
            return F("Parsnip");
        case Hydroponics_CropType_Pea:
            return F("Pea");
        case Hydroponics_CropType_PeaSugar:
            return F("PeaSugar");
        case Hydroponics_CropType_Pepino:
            return F("Pepino");
        case Hydroponics_CropType_PeppersBell:
            return F("PeppersBell");
        case Hydroponics_CropType_PeppersHot:
            return F("PeppersHot");
        case Hydroponics_CropType_Potato:
            return F("Potato");
        case Hydroponics_CropType_PotatoSweet:
            return F("PotatoSweet");
        case Hydroponics_CropType_Pumpkin:
            return F("Pumpkin");
        case Hydroponics_CropType_Radish:
            return F("Radish");
        case Hydroponics_CropType_Rhubarb:
            return F("Rhubarb");
        case Hydroponics_CropType_Rosemary:
            return F("Rosemary");
        case Hydroponics_CropType_Sage:
            return F("Sage");
        case Hydroponics_CropType_Silverbeet:
            return F("Silverbeet");
        case Hydroponics_CropType_Spinach:
            return F("Spinach");
        case Hydroponics_CropType_Squash:
            return F("Squash");
        case Hydroponics_CropType_Sunflower:
            return F("Sunflower");
        case Hydroponics_CropType_Strawberries:
            return F("Strawberries");
        case Hydroponics_CropType_SwissChard:
            return F("SwissChard");
        case Hydroponics_CropType_Taro:
            return F("Taro");
        case Hydroponics_CropType_Tarragon:
            return F("Tarragon");
        case Hydroponics_CropType_Thyme:
            return F("Thyme");
        case Hydroponics_CropType_Tomato:
            return F("Tomato");
        case Hydroponics_CropType_Turnip:
            return F("Turnip");
        case Hydroponics_CropType_Watercress:
            return F("Watercress");
        case Hydroponics_CropType_Watermelon:
            return F("Watermelon");
        case Hydroponics_CropType_Zucchini:
            return F("Zucchini");
        case Hydroponics_CropType_Custom1:
            return F("Custom1");
        case Hydroponics_CropType_Custom2:
            return F("Custom2");
        case Hydroponics_CropType_Custom3:
            return F("Custom3");
        case Hydroponics_CropType_Custom4:
            return F("Custom4");
        case Hydroponics_CropType_Custom5:
            return F("Custom5");
        case (Hydroponics_CropType)82://Hydroponics_CropType_Count:
            return !excludeSpecial ? F("Count") : F("");
        case Hydroponics_CropType_Undefined:
            break;
    }
    return !excludeSpecial ? F("Undefined") : F("");
}

String stringForFluidReservoir(Hydroponics_FluidReservoir fluidReservoir, bool excludeSpecial)
{
    switch (fluidReservoir) {
        case Hydroponics_FluidReservoir_FeedWater:
            return F("Feed");
        case Hydroponics_FluidReservoir_DrainageWater:
            return F("Drainage");
        case Hydroponics_FluidReservoir_NutrientPremix:
            return F("Nutrient");
        case Hydroponics_FluidReservoir_FreshWater:
            return F("Fresh");
        case Hydroponics_FluidReservoir_PhUpSolution:
            return F("phUp");
        case Hydroponics_FluidReservoir_PhDownSolution:
            return F("pHDown");
        case Hydroponics_FluidReservoir_Count:
            return !excludeSpecial ? F("Count") : F("");
        case Hydroponics_FluidReservoir_Undefined:
            break;
    }
    return !excludeSpecial ? F("Undefined") : F("");
}

String symbolForUnitType(Hydroponics_UnitsType unitType, bool excludeSpecial)
{
    switch (unitType) {
        case Hydroponics_UnitsType_Temperature_Celsius:
            return F("°C");
        case Hydroponics_UnitsType_Temperature_Fahrenheit:
            return F("°F");
        case Hydroponics_UnitsType_Temperature_Kelvin:
            return F("°K");
        case Hydroponics_UnitsType_Distance_Meters:
            return F("m");
        case Hydroponics_UnitsType_Distance_Feet:
            return F("ft");
        case Hydroponics_UnitsType_Weight_Kilogram:
            return F("kg");
        case Hydroponics_UnitsType_Weight_Pounds:
            return F("lbs");
        case Hydroponics_UnitsType_LiquidVolume_Liters:
            return F("l");
        case Hydroponics_UnitsType_LiquidVolume_Gallons:
            return F("g");
        case Hydroponics_UnitsType_LiquidFlow_LitersPerMin:
            return F("l/m");
        case Hydroponics_UnitsType_LiquidFlow_GallonsPerMin:
            return F("g/m");
        case Hydroponics_UnitsType_pHScale_0_14:
            return F("pH");
        case Hydroponics_UnitsType_Concentration_EC:
            return F("EC");
        case Hydroponics_UnitsType_Concentration_PPM:
            return F("ppm");
        case Hydroponics_UnitsType_Percentile_0_100:
            return F("%");
        case Hydroponics_UnitsType_Raw_0_1:
            return F("");
        case Hydroponics_UnitsType_Count:
            return !excludeSpecial ? F("qty") : F("");
        case Hydroponics_UnitsType_Undefined:
            break;
    }
    return !excludeSpecial ? F("undef") : F("");
}

Hydroponics_ActuatorType actuatorTypeFromString(String actuatorType)
{
    for (int typeIndex = 0; typeIndex <= (int)Hydroponics_ActuatorType_Count; ++typeIndex) {
        if (actuatorType == stringForActuatorType((Hydroponics_ActuatorType)typeIndex)) {
            return (Hydroponics_ActuatorType)typeIndex;
        }
    }
    return Hydroponics_ActuatorType_Undefined;
}

Hydroponics_SensorType sensorTypeFromString(String sensorType)
{
    for (int typeIndex = 0; typeIndex <= (int)Hydroponics_SensorType_Count; ++typeIndex) {
        if (sensorType == stringForSensorType((Hydroponics_SensorType)typeIndex)) {
            return (Hydroponics_SensorType)typeIndex;
        }
    }
    return Hydroponics_SensorType_Undefined;
}

Hydroponics_CropType cropTypeFromString(String cropType)
{
    for (int typeIndex = 0; typeIndex <= (int)Hydroponics_CropType_Count; ++typeIndex) {
        if (cropType == stringForCropType((Hydroponics_CropType)typeIndex)) {
            return (Hydroponics_CropType)typeIndex;
        }
    }
    return Hydroponics_CropType_Undefined;
}

Hydroponics_FluidReservoir fluidReservoirFromString(String fluidReservoir)
{
    for (int typeIndex = 0; typeIndex <= (int)Hydroponics_FluidReservoir_Count; ++typeIndex) {
        if (fluidReservoir == stringForFluidReservoir((Hydroponics_FluidReservoir)typeIndex)) {
            return (Hydroponics_FluidReservoir)typeIndex;
        }
    }
    return Hydroponics_FluidReservoir_Undefined;
}

Hydroponics_UnitsType unitTypeFromSymbol(String unitSymbol)
{
    for (int typeIndex = 0; typeIndex <= (int)Hydroponics_UnitsType_Count; ++typeIndex) {
        if (unitSymbol == symbolForUnitType((Hydroponics_UnitsType)typeIndex)) {
            return (Hydroponics_UnitsType)typeIndex;
        }
    }
    return Hydroponics_UnitsType_Undefined;
}
