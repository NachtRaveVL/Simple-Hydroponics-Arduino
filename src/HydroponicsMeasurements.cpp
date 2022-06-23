/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Sensor Measurements
*/

#include "Hydroponics.h"

HydroponicsMeasurement::HydroponicsMeasurement()
    : type(Unknown)
{
    auto hydroponics = getHydroponicsInstance();
    frame = (hydroponics ? hydroponics->getPollingFrameNumber() : 0);
    timestamp = now();
}

HydroponicsMeasurement::HydroponicsMeasurement(int typeIn, time_t timestampIn)
    : type((typeof(type))typeIn), timestamp(timestampIn)
{
    auto hydroponics = getHydroponicsInstance();
    frame = (hydroponics ? hydroponics->getPollingFrameNumber() : 0);
}

HydroponicsMeasurement::HydroponicsMeasurement(int typeIn, time_t timestampIn, uint32_t frameIn)
    : type((typeof(type))typeIn), timestamp(timestampIn), frame(frameIn)
{ ; }

HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement()
    : HydroponicsMeasurement(), state(false)
{ ; }

HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement(bool stateIn, time_t timestamp)
    : HydroponicsMeasurement((int)Binary, timestamp), state(stateIn)
{ ; }

HydroponicsBinaryMeasurement::HydroponicsBinaryMeasurement(bool stateIn, time_t timestamp, uint32_t frame)
    : HydroponicsMeasurement((int)Binary, timestamp, frame), state(stateIn)
{ ; }

HydroponicsSingleMeasurement::HydroponicsSingleMeasurement()
    : HydroponicsMeasurement((int)Single), value(0.0f), units(Hydroponics_UnitsType_Undefined)
{ ; }

HydroponicsSingleMeasurement::HydroponicsSingleMeasurement(float valueIn, Hydroponics_UnitsType unitsIn, time_t timestamp)
    : HydroponicsMeasurement((int)Single, timestamp), value(valueIn), units(unitsIn)
{ ; }

HydroponicsSingleMeasurement::HydroponicsSingleMeasurement(float valueIn, Hydroponics_UnitsType unitsIn, time_t timestamp, uint32_t frame)
    : HydroponicsMeasurement((int)Single, timestamp, frame), value(valueIn), units(unitsIn)
{ ; }

HydroponicsDoubleMeasurement::HydroponicsDoubleMeasurement()
    : HydroponicsMeasurement((int)Double), value{0}, units{Hydroponics_UnitsType_Undefined}
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

HydroponicsTripleMeasurement::HydroponicsTripleMeasurement()
    : HydroponicsMeasurement((int)Triple), value{0}, units{Hydroponics_UnitsType_Undefined}
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
