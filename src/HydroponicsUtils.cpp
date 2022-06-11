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
