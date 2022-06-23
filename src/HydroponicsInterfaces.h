/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Interfaces
*/

#ifndef HydroponicsInterfaces_H
#define HydroponicsInterfaces_H

struct HydroponicsSerializableInterface;
struct HydroponicsJSONSerializableInterface;

#include "Hydroponics.h"



// Binary Serializable Interface
struct HydroponicsBinarySerializableInterface {
    // Given a byte stream to write to, writes self to binary format.
    virtual void toBinaryStream(Print *streamOut) const = 0;

    // Given a byte stream to read from, reads overtop self from binary format.
    virtual void fromBinaryStream(Stream *streamIn) = 0;
};

// JSON Serializable Interface
struct HydroponicsJSONSerializableInterface {
    // Given a JSON element to fill in, writes self to JSON format.
    virtual void toJSONElement(JsonVariant &elementOut) const = 0;

    // Given a JSON element to read from, reads overtop self from JSON format.
    virtual void fromJSONElement(JsonVariantConst &elementIn) = 0;
};

// Pump Object Interface
class HydroponicsPumpObjectInterface {
public:
    virtual void setContinuousFlowRate(float flowRate, Hydroponics_UnitsType flowRateUnits = Hydroponics_UnitsType_Undefined) = 0;
    virtual void setContinuousFlowRate(HydroponicsSingleMeasurement flowRate) = 0;
    virtual const HydroponicsSingleMeasurement &getContinuousFlowRate() const = 0;

    virtual void setFlowRateSensor(HydroponicsIdentity flowRateSensorId) = 0;
    virtual void setFlowRateSensor(shared_ptr<HydroponicsSensor> flowRateSensor) = 0;
    virtual shared_ptr<HydroponicsSensor> getFlowRateSensor() = 0;
    virtual const HydroponicsSingleMeasurement &getInstantaneousFlowRate() const = 0;

    virtual void setOutputReservoir(HydroponicsIdentity outputReservoirId) = 0;
    virtual void setOutputReservoir(shared_ptr<HydroponicsReservoir> outputReservoir) = 0;
    virtual shared_ptr<HydroponicsReservoir> getOutputReservoir() = 0;
};

#endif // /ifndef HydroponicsInterfaces_H
