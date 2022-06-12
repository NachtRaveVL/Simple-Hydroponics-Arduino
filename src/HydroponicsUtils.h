/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Utilities
*/

#ifndef HydroponicsUtils_H
#define HydroponicsUtils_H

#include "Hydroponics.h"

extern time_t rtcNow();

extern bool tryConvertValue(float valueIn, Hydroponics_UnitsType unitsIn, float *valueOut, Hydroponics_UnitsType unitsOut);
extern void convertAndAssign(float *valueInOut, Hydroponics_UnitsType *unitsInOut, Hydroponics_UnitsType unitsOut, int roundToDecPlaces = -1);

extern bool checkInputPinIsAnalog(int pin);
extern bool checkOutputPinIsAnalog(int pin);
extern bool checkPinIsDigital(int pin);
extern bool checkPinIsPWM(int pin);
extern bool checkPinCanInterrupt(int pin);

extern String stringForActuatorType(Hydroponics_ActuatorType actuatorType, bool excludeSpecial = false);
extern String stringForSensorType(Hydroponics_SensorType sensorType, bool excludeSpecial = false);
extern String stringForCropType(Hydroponics_CropType cropType, bool excludeSpecial = false);
extern String stringForFluidReservoir(Hydroponics_FluidReservoir fluidReservoir, bool excludeSpecial = false);
extern String symbolForUnitType(Hydroponics_UnitsType unitType, bool excludeSpecial = false);

extern Hydroponics_ActuatorType actuatorTypeFromString(String actuatorType);
extern Hydroponics_SensorType sensorTypeFromString(String sensorType);
extern Hydroponics_CropType cropTypeFromString(String cropType);
extern Hydroponics_FluidReservoir fluidReservoirFromString(String fluidReservoir);
extern Hydroponics_UnitsType unitTypeFromSymbol(String unitSymbol);

#endif // /ifndef HydroponicsUtils_H
