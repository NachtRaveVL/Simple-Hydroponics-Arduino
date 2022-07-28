// Simple-Hydroponics-Arduino Crop Library Writer Example
// In this example we program an SD Card or EEPROM to hold onto the crop library data.
// Since dead code is stripped out of the final binary on most Arduino-like build
// processes, we can take advantage of that fact to create an "empty" system that only
// does one thing: programs the SD Card or EEPROM attached to it. This sketch can be
// ran on a device to "prep" such storage, and thus can easily offload any program data.
// Endianness is not a concern this war since the same device does both writing & reading.
// 
// Make sure that any EEPROM Write-Protect jumpers are disabled, and that you have not
// defined HYDRUINO_DISABLE_BUILT_IN_CROPS_LIBRARY so that the full crops library
// is built onto onboard Flash. You may also enable Serial log output by defining
// HYDRUINO_ENABLE_DEBUG_OUTPUT. You may refer to: https://forum.arduino.cc/index.php?topic=602603.0
// on how to define custom build flags manually via modifying platform[.local].txt.
//
// In Hydroponics.h:
// 
// // Uncomment or -D this define to disable building-in of Crops Library data (note: saves considerable size on sketch). Required for constrained devices.
// //#define HYDRUINO_DISABLE_BUILT_IN_CROPS_LIBRARY   // If enabled, must use external device (such as SD Card or EEPROM) for Crops Library support.
// 
// // Uncomment or -D this define to enable debug output (treats Serial as attached to serial monitor).
// #define HYDRUINO_ENABLE_DEBUG_OUTPUT
//
// Alternatively, in platform[.local].txt:
// build.extra_flags=-DHYDRUINO_ENABLE_DEBUG_OUTPUT

#include <Hydroponics.h>

// Pins & Class Instances
#define SETUP_EEPROM_DEVICE_SIZE    I2C_DEVICESIZE_24LC256 // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
#define SETUP_SD_CARD_CS_PIN        SS              // SD card CS pin, else -1
#define SETUP_EEPROM_I2C_ADDR       B000            // EEPROM address
#define SETUP_RTC_I2C_ADDR          B000            // RTC i2c address (only B000 can be used atm)
#define SETUP_I2C_WIRE_INST         Wire            // I2C wire class instance
#define SETUP_I2C_SPEED             400000U         // I2C speed, in Hz
#define SETUP_ESP_I2C_SDA           SDA             // I2C SDA pin, if on ESP
#define SETUP_ESP_I2C_SCL           SCL             // I2C SCL pin, if on ESP
#define SETUP_SD_CARD_SPI_SPEED     4000000U        // SD card SPI speed, in Hz (ignored if on Teensy)

// External Crops Library Data Settings
#define SETUP_EXTCROPLIB_SD_ENABLE  true            // If crops library should be written to an external SD card
#define SETUP_EXTCROPLIB_SD_PREFIX  "lib/crop"      // Crop data SD data file prefix (appended with ##.dat)
#define SETUP_EXTCROPLIB_EEPROM_ENABLE  true        // If crops library should be written to an external EEPROM
#define SETUP_EXTCROPLIB_EEPROM_ADDRESS 0           // Crop data EEPROM data begin address


Hydroponics hydroController(-1,
                            SETUP_EEPROM_DEVICE_SIZE,
                            SETUP_SD_CARD_CS_PIN,
                            -1,
                            SETUP_EEPROM_I2C_ADDR,
                            SETUP_RTC_I2C_ADDR,
                            0,
                            SETUP_I2C_WIRE_INST,
                            SETUP_I2C_SPEED,
                            SETUP_SD_CARD_SPI_SPEED);

void setup() {
    #ifdef HYDRUINO_ENABLE_DEBUG_OUTPUT
        Serial.begin(115200);           // Begin USB Serial interface
        while(!Serial) { ; }            // Wait for USB Serial to connect (remove in production)
    #endif
    #if defined(ESP32) || defined(ESP8266)
        SETUP_I2C_WIRE_INST.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL); // Begin i2c Wire for ESP
    #endif

    // Just a lone initializer is all that's needed since we won't actually be using the controller.
    hydroController.init();

    // Right here would be the place to program in any custom crop data that you want available.
    //HydroponicsCropsLibData customCrop1(Hydroponics_CropType_CustomCrop1);
    //strncpy(customCrop1.cropName, "Custom name", HYDRUINO_NAME_MAXSIZE);
    //getCropsLibraryInstance()->setCustomCropData(&customCrop1);

    getLoggerInstance()->logMessage(F("Writing crops library..."));

    #if SETUP_EXTCROPLIB_SD_ENABLE
    {   auto sd = getHydroponicsInstance()->getSDCard();

        if (sd) {
            getLoggerInstance()->logMessage(F("... Writing SD Card data..."));

            for (int cropType = 0; cropType < Hydroponics_CropType_Count; ++cropType) {
                auto cropData = getCropsLibraryInstance()->checkoutCropsData((Hydroponics_CropType)cropType);

                if (cropData) {
                    getLoggerInstance()->logMessage(F("Writing Crop: "), charsToString(cropData->cropName, HYDRUINO_NAME_MAXSIZE));
                    String filename = getNNFilename(F(SETUP_EXTCROPLIB_SD_PREFIX), cropType, SFP(HS_dat));
                    getLoggerInstance()->logMessage(F("... to file: "), filename);

                    auto file = sd->open(filename, O_WRITE | O_CREAT | O_TRUNC); // Creates/resets file for writing
                    if (file.availableForWrite()) {
                        StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;
                        JsonObject jsonObject = doc.to<JsonObject>();
                        if (!serializeJsonPretty(jsonObject, file)) { // Could also write out in binary but we have acres of cheap storage
                            getLoggerInstance()->logError(F("Failure writing to crop data file!"));
                        }
                        file.close();
                    } else {
                        if (file) { file.close(); }
                        getLoggerInstance()->logError(F("Failure opening crop data file for writing!"));
                    }

                    getCropsLibraryInstance()->returnCropsData(cropData);
                }

                yield();
            }

            getHydroponicsInstance()->endSDCard(sd);
        } else {
            getLoggerInstance()->logWarning(F("Could not find SD Card device. Check that you have it set up properly."));
        }

        getLoggerInstance()->flush();
    }
    #endif

    #if SETUP_EXTCROPLIB_EEPROM_ENABLE
    {   auto eeprom = getHydroponicsInstance()->getEEPROM();

        if (eeprom) {
            getLoggerInstance()->logMessage(F("... Writing EEPROM data..."));

            // A lookup table of uint16_t[Hydroponics_CropType_Count] is created to aid in offset lookup,
            // which is positioned right after an initial uint16_t total size value (hence the +1).
            uint16_t writeOffset = SETUP_EXTCROPLIB_EEPROM_ADDRESS + (sizeof(uint16_t) * Hydroponics_CropType_Count + 1);
            uint16_t totalSize = 0;

            for (int cropType = 0; cropType < Hydroponics_CropType_Count; ++cropType) {
                auto cropData = getCropsLibraryInstance()->checkoutCropsData((Hydroponics_CropType)cropType);

                if (cropData) {
                    getLoggerInstance()->logMessage(F("Writing Crop: "), charsToString(cropData->cropName, HYDRUINO_NAME_MAXSIZE));
                    getLoggerInstance()->logMessage(F("... to offset: "), String(writeOffset));

                    auto eepromStream = HydroponicsEEPROMStream(writeOffset, sizeof(HydroponicsCropsLibData));
                    size_t bytesWritten = serializeDataToBinaryStream(cropData, &eepromStream); // Could also write out in JSON but inefficient

                    // After writing data out, write offset out to lookup table
                    if (bytesWritten && eeprom->updateBlockVerify(SETUP_EXTCROPLIB_EEPROM_ADDRESS + (sizeof(uint16_t) * cropType),
                                                                  (const byte *)&writeOffset, sizeof(uint16_t))) {
                        writeOffset += bytesWritten;
                        totalSize += bytesWritten;
                    } else {
                        getLoggerInstance()->logError(F("Failure writing crop data to EEPROM!"));
                    }

                    getCropsLibraryInstance()->returnCropsData(cropData);
                } else if (!eeprom->setBlockVerify(SETUP_EXTCROPLIB_EEPROM_ADDRESS + (sizeof(uint16_t) * cropType), 0, sizeof(uint16_t))) {
                    getLoggerInstance()->logError(F("Failure writing table data to EEPROM!"));
                }

                yield();
            }

            if (totalSize) {
                totalSize += (sizeof(uint16_t) * Hydroponics_CropType_Count + 1);
                if (eeprom->updateBlockVerify(SETUP_EXTCROPLIB_EEPROM_ADDRESS, (const byte *)&totalSize, sizeof(uint16_t))) {
                    getLoggerInstance()->logMessage(F("Total bytes written to EEPROM: "), String(totalSize));
                } else {
                    getLoggerInstance()->logError(F("Failure writing total size to EEPROM!"));
                }
            }
        } else {
            getLoggerInstance()->logWarning(F("Could not find EEPROM device. Check that you have it set up properly."));
        }

        getLoggerInstance()->flush();
    }
    #endif

    getLoggerInstance()->logMessage(F("Done!"));
}

void loop()
{ ; }
