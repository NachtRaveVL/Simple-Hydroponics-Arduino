/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Reservoirs
*/

#ifndef HydroponicsReservoirs_H
#define HydroponicsReservoirs_H

class HydroponicsReservoir;
class HydroponicsFluidReservoir;
class HydroponicsInfiniteReservoir;

struct HydroponicsReservoirData;
struct HydroponicsFluidReservoirData;
struct HydroponicsInfiniteReservoirData;

#include "Hydroponics.h"

// Creates reservoir object from passed reservoir data (return ownership transfer - user code *must* delete returned object)
extern HydroponicsReservoir *newReservoirObjectFromData(const HydroponicsReservoirData *dataIn);


// Hydroponics Reservoir Base
// This is the base class for all reservoirs, which defines how the reservoir is
// identified, where it lives, what's attached to it, if it is full or empty, and
// who can activate under it.
class HydroponicsReservoir : public HydroponicsObject, public HydroponicsReservoirObjectInterface, public HydroponicsActuatorAttachmentsInterface, public HydroponicsSensorAttachmentsInterface, public HydroponicsCropAttachmentsInterface {
public:
    const enum { Fluid, Pipe, Unknown = -1 } classType;     // Reservoir class type (custom RTTI)
    inline bool isFluidClass() { return classType == Fluid; }
    inline bool isPipeClass() { return classType == Pipe; }
    inline bool isUnknownClass() { return classType <= Unknown; }

    HydroponicsReservoir(Hydroponics_ReservoirType reservoirType,
                         Hydroponics_PositionIndex reservoirIndex,
                         int classType = Unknown);
    HydroponicsReservoir(const HydroponicsReservoirData *dataIn);
    virtual ~HydroponicsReservoir();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool canActivate(HydroponicsActuator *actuator) const = 0;
    virtual bool getIsFull() const = 0;
    virtual bool getIsEmpty() const = 0;

    virtual bool addActuator(HydroponicsActuator *actuator) override;
    virtual bool removeActuator(HydroponicsActuator *actuator) override;
    bool hasActuator(HydroponicsActuator *actuator) const override;
    arx::map<Hydroponics_KeyType, HydroponicsActuator *> getActuators() const override;

    virtual bool addSensor(HydroponicsSensor *sensor) override;
    virtual bool removeSensor(HydroponicsSensor *sensor) override;
    bool hasSensor(HydroponicsSensor *sensor) const override;
    arx::map<Hydroponics_KeyType, HydroponicsSensor *> getSensors() const override;

    virtual bool addCrop(HydroponicsCrop *crop) override;
    virtual bool removeCrop(HydroponicsCrop *crop) override;
    bool hasCrop(HydroponicsCrop *crop) const override;
    arx::map<Hydroponics_KeyType, HydroponicsCrop *> getCrops() const override;

    Hydroponics_ReservoirType getReservoirType() const;
    Hydroponics_PositionIndex getReservoirIndex() const;

    Signal<HydroponicsReservoir *> &getFilledSignal();
    Signal<HydroponicsReservoir *> &getEmptySignal();

protected:
    Signal<HydroponicsReservoir *> _filledSignal;           // Filled state signal
    Signal<HydroponicsReservoir *> _emptySignal;            // Empty state signal

    HydroponicsData *allocateData() const override;
    virtual void saveToData(HydroponicsData *dataOut) const override;
};


// Simple Fluid Reservoir
// Basic fluid reservoir that contains a volume of liquid and the ability to track
// such. Crude, but effective.
// Optional channel number is used mainly to track specific feed water reservoirs
// but is usually (but not always) aliasing reservoir type position index.
class HydroponicsFluidReservoir : public HydroponicsReservoir, public HydroponicsVolumeAwareInterface {
public:
    HydroponicsFluidReservoir(Hydroponics_ReservoirType reservoirType,
                              Hydroponics_PositionIndex reservoirIndex,
                              float maxVolume,
                              int channel = -1,
                              int classType = Fluid);
    HydroponicsFluidReservoir(const HydroponicsFluidReservoirData *dataIn);
    virtual ~HydroponicsFluidReservoir();

    void update() override;
    void resolveLinks() override;
    void handleLowMemory() override;

    bool canActivate(HydroponicsActuator *actuator) const override;

    bool getIsFull() const override;
    bool getIsEmpty() const override;

    void setVolumeUnits(Hydroponics_UnitsType volumeUnits);
    Hydroponics_UnitsType getVolumeUnits() const;

    void setChannelNumber(int channel);
    int getChannelNumber() const;

    void setVolumeSensor(HydroponicsIdentity volumeSensorId) override;
    void setVolumeSensor(shared_ptr<HydroponicsSensor> volumeSensor) override;
    shared_ptr<HydroponicsSensor> getVolumeSensor() override;

    void setLiquidVolume(float liquidVolume, Hydroponics_UnitsType liquidVolumeUnits = Hydroponics_UnitsType_Undefined) override;
    void setLiquidVolume(HydroponicsSingleMeasurement liquidVolume) override;
    const HydroponicsSingleMeasurement &getLiquidVolume() const override;

    void setFilledTrigger(HydroponicsTrigger *filledTrigger);
    const HydroponicsTrigger *getFilledTrigger() const;

    void setEmptyTrigger(HydroponicsTrigger *emptyTrigger);
    const HydroponicsTrigger *getEmptyTrigger() const;

protected:
    float _maxVolume;                                       // Maximum volume
    Hydroponics_UnitsType _volumeUnits;                     // Preferred volume units (else default)
    int _channel;                                           // Channel # (-1 if unset)
    HydroponicsDLinkObject<HydroponicsSensor> _volumeSensor; // Volume sensor linkage
    HydroponicsSingleMeasurement _volume;                   // Last volume measure
    HydroponicsTrigger *_filledTrigger;                     // Filled trigger (owned)
    HydroponicsTrigger *_emptyTrigger;                      // Empty trigger (owned)

    void saveToData(HydroponicsData *dataOut) const override;

    void attachVolumeSensor();
    void detachVolumeSensor();
    void handleVolumeMeasure(HydroponicsMeasurement *measurement);
};


// Infinite Pipe Reservoir
// An infinite pipe reservoir is like your standard water main - it's not technically
// unlimited, but you can act like it is. Used for reservoirs that should behave typeAs
// alwaysFilled (e.g. water mains) or not (e.g. drainage pipes).
class HydroponicsInfiniteReservoir : public HydroponicsReservoir {
public:
    HydroponicsInfiniteReservoir(Hydroponics_ReservoirType reservoirType,
                                 Hydroponics_PositionIndex reservoirIndex,
                                 bool alwaysFilled = true,
                                 int classType = Pipe);
    HydroponicsInfiniteReservoir(const HydroponicsInfiniteReservoirData *dataIn);
    virtual ~HydroponicsInfiniteReservoir();

    bool canActivate(HydroponicsActuator *actuator) const override;

    bool getIsFull() const override;
    bool getIsEmpty() const override;

protected:
    bool _alwaysFilled;                                     // Always filled flag

    void saveToData(HydroponicsData *dataOut) const override;
};


// Reservoir Serialization Data
struct HydroponicsReservoirData : public HydroponicsObjectData {
    HydroponicsReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Fluid Reservoir Serialization Data
struct HydroponicsFluidReservoirData : public HydroponicsReservoirData {
    float maxVolume;
    Hydroponics_UnitsType volumeUnits;
    int channel;
    char volumeSensorName[HYDRUINO_NAME_MAXSIZE];
    HydroponicsTriggerSubData filledTrigger;
    HydroponicsTriggerSubData emptyTrigger;

    HydroponicsFluidReservoirData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Infinite Pipe Reservoir Serialization Data
struct HydroponicsInfiniteReservoirData : public HydroponicsReservoirData {
    bool alwaysFilled;

    HydroponicsInfiniteReservoirData();
    void toJSONObject(JsonObject &objectOut) const override;
    void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroponicsReservoirs_H
