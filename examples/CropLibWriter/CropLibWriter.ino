// Simple-Hydroponics-Arduino Crop Library Writer Example
// In this example we program an SD Card or EEPROM to hold onto the crop library data.

#include <Hydroponics.h>

// Pins & Class Instances
#define SETUP_EEPROM_DEVICE_SIZE    I2C_DEVICESIZE_24LC256 // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
#define SETUP_SD_CARD_CS_PIN        SS              // SD card CS pin, else -1
#define SETUP_EEPROM_I2C_ADDR       B000            // EEPROM address
#define SETUP_RTC_I2C_ADDR          B000            // RTC i2c address (only B000 can be used atm)
#define SETUP_I2C_WIRE_INST         Wire            // I2C wire class instance
#define SETUP_I2C_SPEED             400000U         // I2C speed, in Hz
#define SETUP_SD_CARD_SPI_SPEED     4000000U        // SD card SPI speed, in Hz (ignored if on Teensy)

// External Crops Library Data Settings
#define SETUP_EXTCROPLIB_SD_ENABLE true            // If crops library should be written to an external SD card
#define SETUP_EXTCROPLIB_SD_PREFIX "lib/crop"      // Crop data SD data file prefix (appended with ##.dat)
#define SETUP_EXTCROPLIB_EEPROM_ENABLE  true       // If crops library should be written to an external EEPROM
#define SETUP_EXTCROPLIB_EEPROM_ADDRESS 0          // Crop data EEPROM data begin address


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

    // Just a lone initializer is all that's needed since we won't be actually using the controller.
    hydroController.init();

    String writingCrop = F("Writing Crop: ");
    String dotDotDot = F("...");

    // Right here would be the place to program in any custom crop data.
    HydroponicsCropsLibData customCrop1(Hydroponics_CropType_CustomCrop1);
    strncpy(customCrop1.cropName, "Custom name", HYDRUINO_NAME_MAXSIZE);

    getCropsLibraryInstance()->setCustomCropData(&customCrop1);

    getLoggerInstance()->logMessage(F("Writing crops library..."));

    #if SETUP_EXTCROPLIB_SD_ENABLE
    {   auto sd = getHydroponicsInstance()->getSDCard();

        if (sd) {
            getLoggerInstance()->logMessage(F("... Writing SD Card data..."));

            for (int cropType = 0; cropType < Hydroponics_CropType_Count; ++cropType) {
                auto cropData = getCropsLibraryInstance()->checkoutCropsData((Hydroponics_CropType)cropType);

                if (cropData) {
                    getLoggerInstance()->logMessage(writingCrop, String(cropData->cropName), dotDotDot);
                    String filename = getNNFilename(F(SETUP_EXTCROPLIB_SD_PREFIX), cropType, SFP(HS_dat));
                    getLoggerInstance()->logMessage(F("... to file: "), filename, dotDotDot);

                    auto file = sd->open(filename, O_WRITE | O_CREAT | O_TRUNC); // Creates/resets file for writing
                    if (file.availableForWrite()) {
                        StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;
                        JsonObject jsonObject = doc.to<JsonObject>();
                        if (!serializeJsonPretty(jsonObject, file)) { // Could also write out in binary but don't bother
                            getLoggerInstance()->logError(F("Failure writing to file!"));
                        }
                        file.close();
                    } else {
                        if (file) { file.close(); }
                        getLoggerInstance()->logError(F("Failure opening file for writing!"));
                    }

                    getCropsLibraryInstance()->returnCropsData(cropData);
                }
            }

            getHydroponicsInstance()->endSDCard(sd);
        }
    }
    #endif

    #if SETUP_EXTCROPLIB_EEPROM_ENABLE
    {   auto eeprom = getHydroponicsInstance()->getEEPROM();

        if (eeprom) {
            getLoggerInstance()->logMessage(F("... Writing EEPROM data..."));

            // A lookup table of uint16_t[Hydroponics_CropType_Count] is created to aid in offset lookup
            uint16_t writeOffset = SETUP_EXTCROPLIB_EEPROM_ADDRESS + (sizeof(uint16_t) * Hydroponics_CropType_Count);

            for (int cropType = 0; cropType < Hydroponics_CropType_Count; ++cropType) {
                auto cropData = getCropsLibraryInstance()->checkoutCropsData((Hydroponics_CropType)cropType);

                if (cropData) {
                    getLoggerInstance()->logMessage(writingCrop, String(cropData->cropName), dotDotDot);
                    getLoggerInstance()->logMessage(F("... to offset: "), String(writeOffset), dotDotDot);

                    auto eepromStream = HydroponicsEEPROMStream(writeOffset, sizeof(HydroponicsCropsLibData));
                    size_t bytesWritten = serializeDataToBinaryStream(cropData, &eepromStream); // Could also write out in JSON but why

                    // After writing data out, write offset out to lookup table
                    if (bytesWritten && eeprom->updateBlockVerify(SETUP_EXTCROPLIB_EEPROM_ADDRESS + (sizeof(uint16_t) * cropType),
                                                                  (const byte *)&writeOffset, sizeof(uint16_t))) {
                        writeOffset += bytesWritten;
                    } else {
                        getLoggerInstance()->logError(F("Failure writing EEPROM data!"));
                    }
                } else { // Unused crop data
                    
                    eeprom->setBlockVerify(SETUP_EXTCROPLIB_EEPROM_ADDRESS + (sizeof(uint16_t) * cropType), 0, sizeof(uint16_t));
                }
            }
        }
    }
    #endif

    getLoggerInstance()->logMessage(F("Done!"));
}

void loop()
{
    // Hydruino will manage most updates for us.
    hydroController.update();
}
