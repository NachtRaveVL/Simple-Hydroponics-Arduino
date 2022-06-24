/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#ifndef HydroponicsCrops_H
#define HydroponicsCrops_H

class HydroponicsCrop;
class HydroponicsSimpleCrop;

#include "Hydroponics.h"

// Hydroponic Crop Base
// This is the base class for all crops, which defines how the crop is identified, at
// what point it is in the growth cycle, which sensors are attached to it, what reservoir
// feeds it, etc.
class HydroponicsCrop : public HydroponicsObject {
public:
    const enum { Simple, Unknown = -1 } classType;          // Crop class type (custom RTTI)
    inline bool isSimpleClass() { return classType == Simple; }
    inline bool isUnknownClass() { return classType == Unknown; }

    HydroponicsCrop(Hydroponics_CropType cropType,
                    Hydroponics_PositionIndex cropIndex,
                    Hydroponics_SubstrateType substrateType,
                    time_t sowDate,
                    int classType = Unknown);
    ~HydroponicsCrop();

    virtual void update() override;
    virtual void resolveLinks() override;
    virtual void handleLowMemory() override;

    virtual bool addSensor(HydroponicsSensor *sensor);
    virtual bool removeSensor(HydroponicsSensor *sensor);
    inline bool hasSensor(HydroponicsSensor *sensor) { return hasLinkage(sensor); }
    arx::map<Hydroponics_KeyType, HydroponicsSensor *> getSensors();

    void setFeedReservoir(HydroponicsIdentity reservoirId);
    void setFeedReservoir(shared_ptr<HydroponicsReservoir> reservoir);
    shared_ptr<HydroponicsReservoir> getFeedReservoir();

    Hydroponics_CropType getCropType() const;
    Hydroponics_PositionIndex getCropIndex() const;
    Hydroponics_SubstrateType getSubstrateType() const;
    time_t getSowDate() const;

    const HydroponicsCropsLibData *getCropsLibData() const;
    int getGrowWeek() const;
    Hydroponics_CropPhase getCropPhase() const;

protected:
    Hydroponics_SubstrateType _substrateType;               // Substrate type
    time_t _sowDate;                                        // Sow date

    const HydroponicsCropsLibData *_cropsLibData;           // Crops library data (checked out iff !nullptr)
    int _growWeek;                                          // Current grow week
    Hydroponics_CropPhase _cropPhase;                       // Current crop phase

    arx::map<Hydroponics_KeyType, HydroponicsSensor *> _sensors; // Attached crop sensors (weak)
    HydroponicsDLinkObject<HydroponicsReservoir> _feedReservoir; // Feed reservoir linkage

    void recalcGrowWeekAndPhase();
    void checkoutCropsLibData();
    void returnCropsLibData();
};


// Simple Crop
// Basic crop that can manage TODO.
class HydroponicsSimpleCrop : public HydroponicsCrop {
public:
    HydroponicsSimpleCrop(Hydroponics_CropType cropType,
                          Hydroponics_PositionIndex cropIndex,
                          Hydroponics_SubstrateType substrateType,
                          time_t sowDate,
                          int classType = Simple);
    virtual ~HydroponicsSimpleCrop();

private:
};

#endif // /ifndef HydroponicsCrops_H
