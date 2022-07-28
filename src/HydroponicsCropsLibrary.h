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
// Unless the HYDRUINO_DISABLE_BUILT_IN_CROPS_LIBRARY define is defined, all crop data is
// internally stored as JSON strings in the Flash PROGMEM memory space. See the Crop Library
// Upload example on how to program an EEPROM or SD Card with such data.
class HydroponicsCropsLibrary {
public:
    // Returns the singleton instance of the library
    static HydroponicsCropsLibrary *getInstance();

    // Begins crops library from external SD card library, with specified prefix and data format.
    void beginCropsLibraryFromSDCard(String libraryCropPrefix, bool jsonFormat = true);

    // Begins crops library from external EEPROM, with specified begin address and data format.
    void beginCropsLibraryFromEEPROM(size_t dataAddress = 0, bool jsonFormat = false);

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
    inline bool hasCustomCrops() const { return _hasCustomCrops; }

    // Signal when custom crops are added/updated in the library
    Signal<Hydroponics_CropType> &getCustomCropSignal();

protected:
    Map<Hydroponics_CropType, HydroponicsCropsLibraryBook *>::type _cropsData; // Loaded crop library data
    bool _hasCustomCrops = false;                           // Has custom crops flag

    String _libSDCropPrefix;                                // Library data files prefix for SD Card, else "" if unused
    bool _libSDJSONFormat = false;                          // Library SD Card data files JSON format tracking flag
    size_t _libEEPROMDataAddress = (size_t)-1;              // Library EEPROM data begin address, else -1 if unused
    bool _libEEPROMJSONFormat = false;                      // Library EEPROM data JSON format tracking flag

    Signal<Hydroponics_CropType> _cropDataSignal;           // Custom crop data updated signal    

    HydroponicsCropsLibraryBook *newBookFromType(Hydroponics_CropType cropType);
    bool updateHasCustom();

private:
    static HydroponicsCropsLibrary *_instance;              // Shared instance
    HydroponicsCropsLibrary() = default;                    // Private constructor to force singleton
    friend class Hydroponics;
};

// Crops Library Book
struct HydroponicsCropsLibraryBook {
    HydroponicsCropsLibData data;                           // Crop library data
    int count;                                              // Reference count
    bool userSet;                                           // If data was user set (not read from device)

    HydroponicsCropsLibraryBook();
    HydroponicsCropsLibraryBook(String jsonStringIn);
    HydroponicsCropsLibraryBook(Stream &streamIn, bool jsonFormat);
    HydroponicsCropsLibraryBook(const HydroponicsCropsLibData &dataIn);
    inline Hydroponics_CropType getKey() const { return data.cropType; }
};

inline HydroponicsCropsLibrary *getCropsLibraryInstance() { return HydroponicsCropsLibrary::getInstance(); }

#endif // /ifndef HydroponicsCropsLibrary_H
