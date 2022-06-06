/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#ifndef HydroponicsCrops_H
#define HydroponicsCrops_H

class HydroponicsCropsLibrary;
class HydroponicsCrop;

#include "Hydroponics.h"

class HydroponicsCropsLibrary {
public:
    static HydroponicsCropsLibrary *getInstance();

    const HydroponicsCropData *getCropData(Hydroponics_CropType cropType) const;
    void setCustomCropData(const Hydroponics_CropType cropType, const HydroponicsCropData *cropData);

    // TODO maybe?
    //void loadCustomCropData();
    //void saveCustomCropData();

protected:
    static HydroponicsCropsLibrary *_instance;
    static bool _cropLibraryBuilt;
    HydroponicsCropData _cropData[Hydroponics_CropType_Count];      // Crop data (CUSTOM* saved to EEPROM)

    friend HydroponicsCropData::HydroponicsCropData(const Hydroponics_CropType);
    HydroponicsCropsLibrary();

    void buildLibrary();
    void validateEntries();
};

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
    const Hydroponics_CropType _cropType;
    const HydroponicsCropData *_cropData; 
    int _positionIndex;
    time_t _sowDate;
    int _growWeek;
    Hydroponics_CropPhase _cropPhase;

    void recalcGrowWeekAndPhase();
};

#endif // /ifndef HydroponicsCrops_H
