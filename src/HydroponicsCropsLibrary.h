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
// which ones can unload. It is recommended to use the HydroponicsCropsLibData constructor
// if using a temporary, otherwise this checkout/return system. The returned crop lib data
// instance is guaranteed to stay unique for as long as it is allocated.
// Unless the HYDRUINO_DISABLE_BUILTIN_DATA define is defined, all crop data is
// internally stored as JSON strings in the Flash PROGMEM memory space. See the Data
// Writer Example sketch on how to program an EEPROM or SD Card with such data.
class HydroponicsCropsLibrary {
public:
    // Returns the singleton instance of the library
    static HydroponicsCropsLibrary *getInstance();

    // Begins crops library from external SD card library, with specified file prefix and data format.
    void beginCropsLibraryFromSDCard(String dataFilePrefix, bool jsonFormat = true);

    // Begins crops library from external EEPROM, with specified data begin address and data format.
    void beginCropsLibraryFromEEPROM(size_t dataAddress = 0, bool jsonFormat = false);

    // Checks out the crop data for this crop from the library, created via the JSON from
    // PROGMEM if needed (nullptr return -> failure). Increments crop data ref count by one.
    const HydroponicsCropsLibData *checkoutCropsData(Hydroponics_CropType cropType);

    // Returns crop data back to the library, to delete when no longer used. Decrements crop
    // data internal ref count by one, deleting on zero.
    void returnCropsData(const HydroponicsCropsLibData *cropData);

    // Adds/updates custom crop data to the library, returning success flag
    bool setUserCropData(const HydroponicsCropsLibData *cropData);

    // Drops/removes custom crop data from the library, returning success flag
    bool dropUserCropData(const HydroponicsCropsLibData *cropData);

    // Returns if there are custom crops in the library
    inline bool hasUserCrops() const { return _hasUserCrops; }

protected:
    Map<Hydroponics_CropType, HydroponicsCropsLibraryBook *, HYDRUINO_CROPSLIB_CROPS_MAXSIZE> _cropsData; // Loaded crops library data
    bool _hasUserCrops = false;                             // Has user crops flag

    String _libSDCropPrefix;                                // Library data files prefix for SD Card, else "" if unused
    bool _libSDJSONFormat = false;                          // Library SD Card data files JSON format tracking flag
    size_t _libEEPROMDataAddress = (size_t)-1;              // Library EEPROM data begin address, else -1 if unused
    bool _libEEPROMJSONFormat = false;                      // Library EEPROM data JSON format tracking flag

    HydroponicsCropsLibraryBook *newBookFromType(Hydroponics_CropType cropType);
    bool updateHasUserCrops();
    void updateCropsOfType(Hydroponics_CropType cropType);

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
