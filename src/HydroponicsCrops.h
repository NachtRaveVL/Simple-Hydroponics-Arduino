/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#ifndef HydroponicsCrops_H
#define HydroponicsCrops_H

class HydroponicsCropsLibrary;
struct HydroponicsCropsLibraryBook;
class HydroponicsCrop;

#include "Hydroponics.h"

// Hydroponics Crops Library
// Crop data is vast and most microcontrollers don't have the memory to load all the crop
// data at once. The crops library uses a library book like checkout and return system,
// in which case reference counting is performed to see which crops need to be loaded and
// which ones can unload. It is recommended to use the HydroponicsCropData's constructor
// method if using a temporary, otherwise this checkout/return system. The returned crop
// data instance is garuanteed to stay unique for as long as it is allocated.
// All crop data is internally stored as JSON strings in the Flash PROGMEM memory space.
class HydroponicsCropsLibrary {
public:
    // Returns the singleton instance of this library.
    static HydroponicsCropsLibrary *getInstance();

    // Checks out the crop data for this crop, created via the JSON from PROGMEM if needed (NULL return = failure). Increments ref count by one.
    const HydroponicsCropData *checkoutCropData(Hydroponics_CropType cropType);

    // Returns crop data back to system, to delete when no longer used. Decrements internal ref count by one, deleting on zero.
    void returnCropData(const HydroponicsCropData *cropData);

    // TODO
    // TODO: Handle the case where custom data gets updated and existing stores are out of date (integrate into object updates/signaling)
    //const HydroponicsCropData *resyncCropData(const HydroponicsCropData *cropData);
    //void setCustomCropData(const Hydroponics_CropType cropType, const HydroponicsCropData *cropData);
    // Signal<CropType> *signalForCropUpdate();

protected:
    static HydroponicsCropsLibrary *_instance;              // Shared instance
    BtreeList<Hydroponics_CropType,
              HydroponicsCropsLibraryBook> _cropData;       // Loaded crop library data

    HydroponicsCropData _cropDataOld[Hydroponics_CropType_Count]; // TBR

    HydroponicsCropsLibrary();                              // Private constructor to force singleton

    void buildLibrary(); // TBR
    void validateEntries(); // TBD
};


// Hydroponic Crop Base Instance
class HydroponicsCrop {
public:
    HydroponicsCrop(const Hydroponics_CropType cropType, const int positionIndex, const time_t sowDate);
    ~HydroponicsCrop();

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
