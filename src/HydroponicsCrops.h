/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#ifndef HydroponicsCrops_H
#define HydroponicsCrops_H

class HydroponicsCropsLibrary;
class HydroponicsCrop;

#include "Hydroponics.h"

// TODO
class HydroponicsCropsLibrary {
public:
    static HydroponicsCropsLibrary *getInstance();

    const HydroponicsCropData *getCropData(Hydroponics_CropType cropType) const;
    void setCustomCropData(const Hydroponics_CropType cropType, const HydroponicsCropData *cropData);

    // TODO maybe?
    //void loadCustomCropData();
    //void saveCustomCropData();

protected:
    static HydroponicsCropsLibrary *_instance;              // TODO
    static bool _cropLibraryBuilt;                          // TODO
    HydroponicsCropData _cropData[Hydroponics_CropType_Count]; // Crop data (CUSTOM* saved to EEPROM)

    friend HydroponicsCropData::HydroponicsCropData(const Hydroponics_CropType);
    HydroponicsCropsLibrary();

    void buildLibrary();
    void validateEntries();
};


// TODO
class HydroponicsCrop {
public:
    HydroponicsCrop(const Hydroponics_CropType cropType, const int positionIndex, const time_t sowDate);

    void update();

    const Hydroponics_CropType getCropType() const;
    const HydroponicsCropData *getCropData() const;
    int getPositionIndex() const;
    time_t getSowDate() const;
    int getGrowWeek() const;
    Hydroponics_CropPhase getCropPhase() const;

protected:
    const Hydroponics_CropType _cropType;                   // TODO
    const HydroponicsCropData *_cropData;                   // TODO
    int _positionIndex;                                     // TODO
    time_t _sowDate;                                        // TODO
    int _growWeek;                                          // TODO
    Hydroponics_CropPhase _cropPhase;                       // TODO

    void recalcGrowWeekAndPhase();
};

#endif // /ifndef HydroponicsCrops_H
