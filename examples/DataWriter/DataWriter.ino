// Simple-Hydroponics-Arduino Data Writer Example
// In this example we program an SD card or EEPROM to hold onto the various data items
// that normally are built into the produced binary. However, this isn't possible on
// memory constrained devices (e.g. ATMega2560), thus offloading the data is needed.
// Since dead code is stripped out of the final binary on most Arduino-like build
// processes, we can take advantage of that fact to create an "empty" system that only
// does one thing: programs the SD card or EEPROM attached to it. This sketch can be
// easily compiled to such constrained device to "prep" said storage, and thus we can
// offload any program constant data, like Crops Library and strings data. Endianness
// is also not a concern this way since the same device does both writing & reading.
//
// For EEPROM device writing, keep track of the produced output that defines the various
// data locations written to the EEPROM chip, as these will be needed to correctly set
// the various data address locations (e.g. CROP_ADDR, STR_ADDR, etc.).
//
// Make sure that any EEPROM Write-Protect jumpers are disabled, and that you have not
// defined HYDRO_DISABLE_BUILTIN_DATA so that the full data is built into the onboard
// Flash. You may also enable Serial log output by defining HYDRO_ENABLE_DEBUG_OUTPUT.
// You may refer to: https://forum.arduino.cc/index.php?topic=602603.0 on how to define
// custom build flags manually via modifying platform[.local].txt.
//
// In Hydruino.h:
// 
// // Uncomment or -D this define to enable external data storage (SD card or EEPROM) to save on sketch size. Required for constrained devices.
// //#define HYDRO_DISABLE_BUILTIN_DATA              // Disables library data existing in Flash, instead relying solely on external storage.
// 
// // Uncomment or -D this define to enable debug output (treats Serial output as attached to serial monitor).
// #define HYDRO_ENABLE_DEBUG_OUTPUT
//
// Alternatively, in platform[.local].txt:
// build.extra_flags=-DHYDRO_ENABLE_DEBUG_OUTPUT

#include <Hydruino.h>

// Compiler flag checks
#ifdef HYDRO_DISABLE_BUILTIN_DATA
#error The HYDRO_DISABLE_BUILTIN_DATA flag is expected to be undefined in order to run this sketch
#endif

// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_TYPE        None            // EEPROM device type/size (24LC01, 24LC02, 24LC04, 24LC08, 24LC16, 24LC32, 24LC64, 24LC128, 24LC256, 24LC512, None)
#define SETUP_EEPROM_I2C_ADDR           0b000           // EEPROM i2c address
#define SETUP_RTC_I2C_ADDR              0b000           // RTC i2c address (only 0b000 can be used atm)
#define SETUP_RTC_DEVICE_TYPE           None            // RTC device type (DS1307, DS3231, PCF8523, PCF8563, None)
#define SETUP_SD_CARD_SPI               SPI             // SD card SPI class instance
#define SETUP_SD_CARD_SPI_CS            -1              // SD card CS pin, else -1
#define SETUP_SD_CARD_SPI_SPEED         F_SPD           // SD card SPI speed, in Hz (ignored on Teensy)
#define SETUP_I2C_WIRE                  Wire            // I2C wire class instance
#define SETUP_I2C_SPEED                 400000U         // I2C speed, in Hz
#define SETUP_ESP_I2C_SDA               SDA             // I2C SDA pin, if on ESP
#define SETUP_ESP_I2C_SCL               SCL             // I2C SCL pin, if on ESP

// External Data Settings
#define SETUP_EXTDATA_SD_ENABLE         true            // If data should be written to an external SD card
#define SETUP_EXTDATA_SD_LIB_PREFIX     "lib/"          // Library data folder/data file prefix (appended with {type}##.dat)
#define SETUP_EXTDATA_EEPROM_ENABLE     true            // If data should be written to an external EEPROM
#define SETUP_EXTDATA_EEPROM_BEG_ADDR   0               // Start data address for data to be written to EEPROM

Hydruino hydroController((pintype_t)SETUP_PIEZO_BUZZER_PIN,
                         JOIN(Hydro_EEPROMType,SETUP_EEPROM_DEVICE_TYPE),
                         I2CDeviceSetup((uint8_t)SETUP_EEPROM_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
                         JOIN(Hydro_RTCType,SETUP_RTC_DEVICE_TYPE),
                         I2CDeviceSetup((uint8_t)SETUP_RTC_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
                         SPIDeviceSetup((pintype_t)SETUP_SD_CARD_SPI_CS, &SETUP_SD_CARD_SPI, SETUP_SD_CARD_SPI_SPEED));

// Wraps a formatted address as appended pseudo alt text, e.g. " (0xADDR)"
String altAddressToString(uint16_t addr)
{
    String retVal;
    retVal.concat(' '); retVal.concat('('); 
    retVal.concat(addressToString(addr));
    retVal.concat(')');
    return retVal;
}

void setup() {
    // Setup base interfaces
    #ifdef HYDRO_ENABLE_DEBUG_OUTPUT
        Serial.begin(115200);           // Begin USB Serial interface
        while (!Serial) { ; }           // Wait for USB Serial to connect
    #endif
    #if defined(ESP_PLATFORM)
        SETUP_I2C_WIRE.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL); // Begin i2c Wire for ESP
    #endif

    // Just a lone initializer is all that's needed since we won't actually be using the full controller.
    hydroController.init();

    // Right here would be the place to program in any custom crop data that you want made available for later.
    //HydroCropsLibData customCrop1(Hydro_CropType_CustomCrop1);
    //strncpy(customCrop1.cropName, "Custom name", HYDRO_NAME_MAXSIZE);
    //hydroCropsLib.setUserCropData(&customCrop1);

    getLogger()->logMessage(F("Writing external data..."));

    #if SETUP_EXTDATA_SD_ENABLE
    {   auto sd = getController()->getSDCard();

        if (sd) {
            getLogger()->logMessage(F("=== Writing Crops Library data to SD card ==="));

            for (int cropType = 0; cropType < Hydro_CropType_Count; ++cropType) {
                auto cropData = hydroCropsLib.checkoutCropsData((Hydro_CropType)cropType);
                String filename = getNNFilename(String(F(SETUP_EXTDATA_SD_LIB_PREFIX)) + String(F("crop")), cropType, SFP(HStr_dat));

                if (cropData && cropData->cropName[0]) {
                    getLogger()->logMessage(F("Writing Crop: "), charsToString(cropData->cropName, HYDRO_NAME_MAXSIZE));
                    getLogger()->logMessage(F("... to file: "), filename);

                    createDirectoryFor(sd, filename);
                    if (sd->exists(filename.c_str())) {
                        sd->remove(filename.c_str());
                    }
                    auto file = sd->open(filename.c_str(), FILE_WRITE); // Creates/resets file for writing
                    if (file) {
                        StaticJsonDocument<HYDRO_JSON_DOC_DEFSIZE> doc;
                        JsonObject jsonObject = doc.to<JsonObject>();
                        cropData->toJSONObject(jsonObject);
                        uint16_t bytesWritten = serializeJsonPretty(jsonObject, file); // Could also write out in binary but we have acres of cheap SD storage

                        if (bytesWritten) {
                            getLogger()->logMessage(F("Wrote: "), String(bytesWritten), F(" bytes"));
                        } else {
                            getLogger()->logError(F("Failure writing to crops lib data file!"));
                        }

                        file.flush();
                        file.close();
                    } else {
                        getLogger()->logError(F("Failure opening crops lib data file for writing!"));
                    }

                    hydroCropsLib.returnCropsData(cropData);
                } else if (sd->exists(filename.c_str())) {
                    sd->remove(filename.c_str());
                }

                yield();
            }

            {   getLogger()->logMessage(F("=== Writing string data to SD card ==="));

                uint16_t lookupTable[Hydro_Strings_Count];

                // Initializes lookup table with proper locations
                {   uint16_t writeAddr = sizeof(lookupTable);

                    for (int stringNum = 0; stringNum < Hydro_Strings_Count; ++stringNum) {
                        String string = SFP((Hydro_String)stringNum);
                        lookupTable[stringNum] = writeAddr;
                        writeAddr += string.length() + 1;
                    }
                }

                getLogger()->logMessage(F("Writing Strings"));
                String filename = String(String(F(SETUP_EXTDATA_SD_LIB_PREFIX)) + String(F("strings.")) + SFP(HStr_dat));
                getLogger()->logMessage(F("... to file: "), filename);

                createDirectoryFor(sd, filename);
                if (sd->exists(filename.c_str())) {
                    sd->remove(filename.c_str());
                }
                auto file = sd->open(filename.c_str(), FILE_WRITE);
                if (file) { // Strings data goes into a single file as binary
                    uint16_t bytesWritten = 0;

                    // Lookup table constructed first to avoid random seeking
                    bytesWritten += file.write((const uint8_t *)lookupTable, sizeof(lookupTable));

                    for (int stringNum = 0; stringNum < Hydro_Strings_Count; ++stringNum) {
                        String string = SFP((Hydro_String)stringNum);
                        bytesWritten += file.write((const uint8_t *)string.c_str(), string.length() + 1); // +1 to also write out null terminator
                    }

                    if (bytesWritten) {
                        getLogger()->logMessage(F("Wrote: "), String(bytesWritten), F(" bytes"));
                    } else {
                        getLogger()->logError(F("Failure writing to strings data file!"));
                    }

                    file.flush();
                    file.close();
                } else {
                    getLogger()->logError(F("Failure opening strings data file for writing!"));
                }

                yield();
            }

            getController()->endSDCard(sd);
        } else {
            getLogger()->logWarning(F("Could not find SD card device. Check that you have it set up properly."));
        }

        getLogger()->flush();
    }
    #endif

    #if SETUP_EXTDATA_EEPROM_ENABLE
    {   auto eeprom = getController()->getEEPROM();

        if (eeprom) {
            uint16_t cropsLibBegAddr = SETUP_EXTDATA_EEPROM_BEG_ADDR;
            uint16_t stringsBegAddr = (uint16_t)-1;
            uint16_t sysDataBegAddr = (uint16_t)-1;

            {   getLogger()->logMessage(F("=== Writing Crops Library data to EEPROM ==="));

                // A lookup table similar to uint16_t lookupTable[Hydro_Strings_Count] is created
                // manually here, which is used for crop data lookup. The first uint16_t value will be
                // reserved for the total chunk size (hence the +1).
                uint16_t writeAddr = cropsLibBegAddr + ((Hydro_CropType_Count + 1) * sizeof(uint16_t));

                for (int cropType = 0; cropType < Hydro_CropType_Count; ++cropType) {
                    auto cropData = hydroCropsLib.checkoutCropsData((Hydro_CropType)cropType);

                    if (cropData && cropData->cropName[0]) {
                        getLogger()->logMessage(F("Writing Crop: "), charsToString(cropData->cropName, HYDRO_NAME_MAXSIZE));
                        getLogger()->logMessage(F("... to byte offset: "), String(writeAddr), altAddressToString(writeAddr));

                        auto eepromStream = HydroEEPROMStream(writeAddr, sizeof(HydroCropsLibData));
                        size_t bytesWritten = serializeDataToBinaryStream(cropData, &eepromStream); // Could also write out in JSON, but is space inefficient

                        // After writing data out, write location out to lookup table
                        if (bytesWritten && eeprom->updateBlockVerify(cropsLibBegAddr + ((cropType + 1) * sizeof(uint16_t)),
                                                                      (const uint8_t *)&writeAddr, sizeof(uint16_t))) {
                            writeAddr += bytesWritten;

                            getLogger()->logMessage(F("Wrote: "), String(bytesWritten), F(" bytes"));
                        } else {
                            getLogger()->logError(F("Failure writing crops lib data to EEPROM!"));
                        }

                        hydroCropsLib.returnCropsData(cropData);
                    } else if (!eeprom->setBlockVerify(cropsLibBegAddr + ((cropType + 1) * sizeof(uint16_t)), 0, sizeof(uint16_t))) {
                        getLogger()->logError(F("Failure writing crops lib table data to EEPROM!"));
                    }

                    yield();
                }

                // Write out total crops lib size to first position, as long as something was at least written out
                stringsBegAddr = cropsLibBegAddr;
                if (writeAddr > cropsLibBegAddr + ((Hydro_CropType_Count + 1) * sizeof(uint16_t))) {
                    uint16_t totalBytesWritten = writeAddr - cropsLibBegAddr;

                    if (eeprom->updateBlockVerify(cropsLibBegAddr, (const uint8_t *)&totalBytesWritten, sizeof(uint16_t))) {
                        stringsBegAddr = writeAddr;
                        getLogger()->logMessage(F("Successfully wrote: "), String(totalBytesWritten), F(" bytes"));
                    } else {
                        getLogger()->logError(F("Failure writing total crops lib data size to EEPROM!"));
                    }
                }
            }

            {   getLogger()->logMessage(F("=== Writing strings data to EEPROM ==="));

                // Similar to above, same deal with a lookup table.
                uint16_t writeAddr = stringsBegAddr + ((Hydro_Strings_Count + 1) * sizeof(uint16_t));

                for (int stringNum = 0; stringNum < Hydro_Strings_Count; ++stringNum) {
                    String string = SFP((Hydro_String)stringNum);

                    getLogger()->logMessage(F("Writing String: #"), String(stringNum) + String(F(" \"")), string + String(F("\"")));
                    getLogger()->logMessage(F("... to byte offset: "), String(writeAddr), altAddressToString(writeAddr));

                    if(eeprom->updateBlockVerify(writeAddr, (const uint8_t *)string.c_str(), string.length() + 1) &&
                       eeprom->updateBlockVerify(stringsBegAddr + ((stringNum + 1) * sizeof(uint16_t)),
                                                 (const uint8_t *)&writeAddr, sizeof(uint16_t))) {
                        writeAddr += string.length() + 1;
                        getLogger()->logMessage(F("Wrote: "), String(string.length() + 1), F(" bytes"));
                    } else {
                        getLogger()->logError(F("Failure writing strings data to EEPROM!"));
                    }

                    yield();
                }

                sysDataBegAddr = stringsBegAddr;
                if (writeAddr > stringsBegAddr + ((Hydro_Strings_Count + 1) * sizeof(uint16_t))) {
                    uint16_t totalBytesWritten = writeAddr - stringsBegAddr;

                    if (eeprom->updateBlockVerify(stringsBegAddr, (const uint8_t *)&totalBytesWritten, sizeof(uint16_t))) {
                        getLogger()->logMessage(F("Successfully wrote: "), String(totalBytesWritten), F(" bytes"));
                        sysDataBegAddr = writeAddr;
                    } else {
                        getLogger()->logError(F("Failure writing total strings data size to EEPROM!"));
                    }
                }
            }

            getLogger()->logMessage(F("Total EEPROM usage: "), String(sysDataBegAddr), F(" bytes"));
            getLogger()->logMessage(F("EEPROM capacity used: "), String(((float)sysDataBegAddr / eeprom->getDeviceSize()) * 100.0f) + String(F("% of ")), String(eeprom->getDeviceSize()) + String(F(" bytes")));
            getLogger()->logMessage(F("Use the following EEPROM setup defines in your sketch:"));
            Serial.print(F("#define SETUP_EEPROM_SYSDATA_ADDR       "));
            Serial.println(addressToString(sysDataBegAddr));
            Serial.print(F("#define SETUP_EEPROM_CROPSLIB_ADDR      "));
            Serial.println(addressToString(cropsLibBegAddr));
            Serial.print(F("#define SETUP_EEPROM_STRINGS_ADDR       "));
            Serial.println(addressToString(stringsBegAddr));
        } else {
            getLogger()->logWarning(F("Could not find EEPROM device. Check that you have it set up properly."));
        }

        getLogger()->flush();
    }
    #endif

    getLogger()->logMessage(F("Done!"));
}

void loop()
{ ; }
