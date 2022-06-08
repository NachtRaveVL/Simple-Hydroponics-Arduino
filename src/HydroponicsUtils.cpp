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
            // TODO
            break;

        case Hydroponics_UnitsType_Distance_Feet:
            // TODO
            break;

        case Hydroponics_UnitsType_Weight_Kilogram:
            // TODO
            break;

        case Hydroponics_UnitsType_Weight_Pounds:
            // TODO
            break;

        case Hydroponics_UnitsType_LiquidVolume_Liters:
            // TODO
            break;

        case Hydroponics_UnitsType_LiquidVolume_Gallons:
            // TODO
            break;

        case Hydroponics_UnitsType_LiquidFlow_LitersPerMin:
            // TODO
            break;

        case Hydroponics_UnitsType_LiquidFlow_GallonsPerMin:
            // TODO
            break;

        case Hydroponics_UnitsType_pHScale_0_14:
            // TODO
            break;

        case Hydroponics_UnitsType_Concentration_EC:
            // TODO
            break;

        case Hydroponics_UnitsType_Concentration_PPM:
            // TODO
            break;

        case Hydroponics_UnitsType_Percentile_0_100:
            // TODO
            break;

        case Hydroponics_UnitsType_Raw_0_1:
            // TODO
            break;

        default:
            break;
    }

    return false;
}

void convertAndAssign(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType unitsOut, int roundToDecPlaces = 2)
{
    if (tryConvertValue(*valueInOut, *unitsInOut, valueInOut, unitsOut)) {
        *unitsInOut = unitsOut;
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
