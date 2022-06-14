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
// data up at once. The crops library uses a library book like checkout and return system,
// in which case reference counting is performed to see which crops need to be loaded and
// which ones can unload. It is recommended to use the HydroponicsCropLibData's constructor
// method if using a temporary, otherwise this checkout/return system. The returned crop
// data instance is guaranteed to stay unique for as long as it is allocated.
// All crop data is internally stored as JSON strings in the Flash PROGMEM memory space.
class HydroponicsCropsLibrary {
public:
    // Returns the singleton instance of this library.
    static HydroponicsCropsLibrary *getInstance();

    // Checks out the crop data for this crop, created via the JSON from PROGMEM if needed
    // (nullptr return = failure). Increments crop data ref count by one.
    const HydroponicsCropLibData *checkoutCropData(Hydroponics_CropType cropType);

    // Returns crop data back to system, to delete when no longer used. Decrements crop
    // data internal ref count by one, deleting on zero.
    void returnCropData(const HydroponicsCropLibData *plantData);

    // TODO
    //void setCustomCropData(const Hydroponics_CropType cropType, const HydroponicsCropLibData *plantData);
    // CallBack<CropUpdateData> *registerCallbackForCropUpdate();
    static bool _libraryBuilt; // To be removed in near future

protected:
    static HydroponicsCropsLibrary *_instance;              // Shared instance
    BtreeList<Hydroponics_CropType,
              HydroponicsCropsLibraryBook> _plantData;       // Loaded crop library data

    HydroponicsCropLibData _cropLibDataOld[Hydroponics_CropType_Count]; // To be removed in near future

    HydroponicsCropsLibrary();                              // Private constructor to force singleton

    void buildLibrary(); // To be removed in near future
    void validateEntries(); // To be removed in near future
};


// Hydroponic Crop Base
// This is the base class for all crops, which defines how the crop is identified, at
// what point in the grow cycle it is, etc. It can perform basic operations for most
// crop types, but more advanced crops can specialize the specific logic used.
class HydroponicsCrop : public HydroponicsObject {
public:
    HydroponicsCrop(Hydroponics_CropType cropType,
                    Hydroponics_PositionIndex cropIndex,
                    Hydroponics_SubstrateType substrateType,
                    time_t sowDate);
    ~HydroponicsCrop();

    Hydroponics_CropType getCropType() const;
    Hydroponics_PositionIndex getCropIndex() const;
    Hydroponics_SubstrateType getSubstrateType() const;
    time_t getSowDate() const;

    const HydroponicsCropLibData *getCropData() const;
    int getGrowWeek() const;
    Hydroponics_CropPhase getCropPhase() const;

protected:
    Hydroponics_SubstrateType _substrateType;               // Substrate type
    time_t _sowDate;                                        // Sow date

    const HydroponicsCropLibData *_plantData;               // Crop data (checked out iff !nullptr)
    int _growWeek;                                          // Current grow week
    Hydroponics_CropPhase _cropPhase;                       // Current crop phase

    void recalcGrowWeekAndPhase();
};

#endif // /ifndef HydroponicsCrops_H
