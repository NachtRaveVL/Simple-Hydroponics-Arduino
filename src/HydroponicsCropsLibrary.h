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
    // Returns the singleton instance of the library
    static HydroponicsCropsLibrary *getInstance();

    // Checks out the crop data for this crop from the library, created via the JSON from
    // PROGMEM if needed (nullptr return = failure). Increments crop data ref count by one.
    const HydroponicsCropsLibData *checkoutCropsData(Hydroponics_CropType cropType);

    // Returns crop data back to the library, to delete when no longer used. Decrements crop
    // data internal ref count by one, deleting on zero.
    void returnCropsData(const HydroponicsCropsLibData *cropData);

    // Adds/updates custom crop data to the library, returning success flag
    bool setCustomCropData(const HydroponicsCropsLibData *cropData);

    // Drops/removes custom crop data from the library, returning success flag
    bool dropCustomCropData(const HydroponicsCropsLibData *cropData);

    // Returns if there are custom crops in the library
    bool hasCustomCrops();

    // Signal when custom crops are added/updated in the library
    Signal<Hydroponics_CropType> &getCustomCropSignal();

protected:
    Map<Hydroponics_CropType, HydroponicsCropsLibraryBook *>::type _cropsData; // Loaded crop library data
    bool _hasCustomCrops = false;                           // Has custom crops flag
    Signal<Hydroponics_CropType> _cropDataSignal;           // Custom crop data updated signal    

    String jsonStringForCrop(Hydroponics_CropType cropType);
    bool updateHasCustom();

private:
    static HydroponicsCropsLibrary *_instance;              // Shared instance
    HydroponicsCropsLibrary() = default;                    // Private constructor to force singleton
    friend class Hydroponics;
};

// Crops Library Book
struct HydroponicsCropsLibraryBook {
    HydroponicsCropsLibraryBook();
    HydroponicsCropsLibraryBook(const HydroponicsCropsLibData &data);
    Hydroponics_CropType getKey() const;
    HydroponicsCropsLibData data;
    int count;
};

inline HydroponicsCropsLibrary *getCropsLibraryInstance() { return HydroponicsCropsLibrary::getInstance(); }

#endif // /ifndef HydroponicsCropsLibrary_H
