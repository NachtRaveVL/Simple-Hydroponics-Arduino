/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops Library
*/

#ifndef HydroponicsCropsLibrary_H
#define HydroponicsCropsLibrary_H

class HydroponicsCropsLibrary;
struct HydroponicsCropsLibraryBook;

#include "Hydroponics.h"

// Hydroponics Crops Library
// Crop data is vast and most microcontrollers don't have the memory to load all the crop
// data up at once. The crops library uses a library book like checkout and return system,
// in which case reference counting is performed to see which crops need to be loaded and
// which ones can unload. It is recommended to use the HydroponicsCropsLubData class if
// using a temporary, otherwise this checkout/return system. The returned crop lib data
// instance is guaranteed to stay unique for as long as it is allocated.
// All crop data is internally stored as JSON strings in the Flash PROGMEM memory space.
class HydroponicsCropsLibrary {
public:
    // Returns the singleton instance of this library.
    static HydroponicsCropsLibrary *getInstance();

    // Checks out the crop data for this crop, created via the JSON from PROGMEM if needed
    // (nullptr return = failure). Increments crop data ref count by one.
    const HydroponicsCropsLibData *checkoutCropData(Hydroponics_CropType cropType);

    // Returns crop data back to system, to delete when no longer used. Decrements crop
    // data internal ref count by one, deleting on zero.
    void returnCropData(const HydroponicsCropsLibData *cropData);

    // TODO
    //void setCustomCropData(const Hydroponics_CropType cropType, const HydroponicsCropsLibData *cropData);
    // CallBack<CropUpdateData> *registerCallbackForCropUpdate();
    static bool _libraryBuilt; // To be removed in near future

protected:
    static HydroponicsCropsLibrary *_instance;              // Shared instance
    arx::map<Hydroponics_CropType, HydroponicsCropsLibraryBook *> _cropsLibData; // Loaded crop library data

    HydroponicsCropsLibData _cropsLibDataOld[Hydroponics_CropType_Count]; // To be removed in near future

    HydroponicsCropsLibrary();                              // Private constructor to force singleton

    void buildLibrary(); // To be removed in near future
    void validateEntries(); // To be removed in near future
};

#endif // /ifndef HydroponicsCrops_H
