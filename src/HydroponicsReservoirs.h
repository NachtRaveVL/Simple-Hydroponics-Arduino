/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Reservoirs
*/

#ifndef HydroponicsReservoirs_H
#define HydroponicsReservoirs_H

class HydroponicsReservoir;
class HydroponicsWaterReservoir;
class HydroponicsFluidSolution;
class HydroponicsDrainagePipe;

#include "Hydroponics.h"

// Hydroponics Reservoir Base
// This is the base class for all reservoirs, which defines how the reservoir is
// identified, where it lives, what's attached to it, if it is full/empty, and who
// can activate under it.
class HydroponicsReservoir : public HydroponicsObject {
public:
    const enum { Fluid, Pipe, Unknown = -1 } classType;     // Reservoir class type (custom RTTI)
    inline bool isFluidClass() { return classType == Fluid; }
    inline bool isPipeClass() { return classType == Pipe; }
    inline bool isUnknownClass() { return classType == Unknown; }

    HydroponicsReservoir(Hydroponics_ReservoirType reservoirType,
                         Hydroponics_PositionIndex reservoirIndex,
                         int classType = Unknown);
    virtual ~HydroponicsReservoir();

    virtual Hydroponics_TriggerState getFilledState() const = 0;
    virtual Hydroponics_TriggerState getEmptyState() const = 0;
    virtual Signal<Hydroponics_TriggerState> *getFilledSignal() = 0;
    virtual Signal<Hydroponics_TriggerState> *getEmptySignal() = 0;

    virtual bool canActivate(shared_ptr<HydroponicsActuator> actuator) = 0;

    virtual bool addActuator(HydroponicsActuator *actuator);
    virtual bool removeActuator(HydroponicsActuator *actuator);
    inline bool hasActuator(HydroponicsActuator *actuator) { return hasLinkage(actuator); }
    arx::map<Hydroponics_KeyType, HydroponicsActuator *> getActuators() const;

    virtual bool addSensor(HydroponicsSensor *sensor);
    virtual bool removeSensor(HydroponicsSensor *sensor);
    inline bool hasSensor(HydroponicsSensor *sensor) { return hasLinkage(sensor); }
    arx::map<Hydroponics_KeyType, HydroponicsSensor *> getSensors() const;

    virtual bool addCrop(HydroponicsCrop *crop);
    virtual bool removeCrop(HydroponicsCrop *crop);
    inline bool hasCrop(HydroponicsCrop *crop) { return hasLinkage(crop); }
    arx::map<Hydroponics_KeyType, HydroponicsCrop *> getCrops() const;

    Hydroponics_ReservoirType getReservoirType() const;
    Hydroponics_PositionIndex getReservoirIndex() const;

protected:
};


// Simple Fluid Reservoir
// Basic fluid reservoir that contains a volume of liquid and the ability to attach
// filled and empty triggers for filled/empty tracking. Crude, but effective.
// Optional channel number is used mainly to track specific feed water reservoirs
// but is usually (but not always) aliasing reservoir type position index.
class HydroponicsFluidReservoir : public HydroponicsReservoir {
public:
    HydroponicsFluidReservoir(Hydroponics_ReservoirType reservoirType,
                              Hydroponics_PositionIndex reservoirIndex,
                              float maxVolume,
                              int channel = -1,
                              int classType = Fluid);
    virtual ~HydroponicsFluidReservoir();

    void update() override;
    void resolveLinks() override;

    Hydroponics_TriggerState getFilledState() const override;
    Hydroponics_TriggerState getEmptyState() const override;
    Signal<Hydroponics_TriggerState> *getFilledSignal() override;
    Signal<Hydroponics_TriggerState> *getEmptySignal() override;

    bool canActivate(shared_ptr<HydroponicsActuator> actuator) override;

    void setVolumeUnits(Hydroponics_UnitsType volumeUnits);
    Hydroponics_UnitsType getVolumeUnits() const;

    void setFilledTrigger(HydroponicsTrigger *filledTrigger);
    const HydroponicsTrigger *getFilledTrigger() const;

    void setEmptyTrigger(HydroponicsTrigger *emptyTrigger);
    const HydroponicsTrigger *getEmptyTrigger() const;

    void setChannelNumber(int channel);
    int getChannelNumber() const;

    arx::map<Hydroponics_KeyType, HydroponicsPumpObjectInterface *> getInputPumpActuators() const;
    arx::map<Hydroponics_KeyType, HydroponicsPumpObjectInterface *> getOutputPumpActuators() const;

protected:
    float _currVolume;                                      // Current volume (likely estimated)
    float _maxVolume;                                       // Maximum volume
    Hydroponics_UnitsType _volumeUnits;                     // Preferred volume units (else default)
    HydroponicsTrigger *_filledTrigger;                     // Filled trigger (owned)
    HydroponicsTrigger *_emptyTrigger;                      // Empty trigger (owned)
    int _channel;                                           // Channel # (-1 if unset)
};


// Infinite Pipe Reservoir
// An infinite pipe reservoir is like your standard water main - it's not technically
// unlimited, but you can act like it is. Used for reservoirs that should behave as
// alwaysFilled (e.g. water mains) or not (e.g. drainage pipes).
class HydroponicsInfiniteReservoir : public HydroponicsReservoir {
public:
    HydroponicsInfiniteReservoir(Hydroponics_ReservoirType reservoirType,
                                 Hydroponics_PositionIndex reservoirIndex,
                                 bool alwaysFilled = true,
                                 int classType = Pipe);
    virtual ~HydroponicsInfiniteReservoir();

    Hydroponics_TriggerState getFilledState() const override;
    Hydroponics_TriggerState getEmptyState() const override;
    Signal<Hydroponics_TriggerState> *getFilledSignal() override;
    Signal<Hydroponics_TriggerState> *getEmptySignal() override;

    bool canActivate(shared_ptr<HydroponicsActuator> actuator) override;

protected:
    bool _alwaysFilled;                                     // Always filled flag
};

#endif // /ifndef HydroponicsReservoirs_H
