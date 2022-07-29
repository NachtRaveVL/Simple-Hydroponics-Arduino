// Simple-Hydroponics-Arduino Data Writer Example
// In this example we program an SD Card or EEPROM to hold onto the various data items
// that normally are built into the produced binary. However, this isn't possible on
// memory constrained devices (e.g. ATMega2560), thus offloading the data is needed.
// Since dead code is stripped out of the final binary on most Arduino-like build
// processes, we can take advantage of that fact to create an "empty" system that only
// does one thing: programs the SD Card or EEPROM attached to it. This sketch can be
// easily compiled to such constrained device to "prep" said storage, and thus we can
// offload any program constant data, like Crops Library and Strings data. Endianness
// is also not a concern this way since the same device does both writing & reading.
//
// For EEPROM device writing, keep track of the produced output that defines the various
// data offsets written to the EEPROM chip, as these will be needed to correctly set
// the various data address offsets (e.g. CROP_ADDR, STR_ADDR, etc.).
//
// Make sure that any EEPROM Write-Protect jumpers are disabled, and that you have not
// defined HYDRUINO_ENABLE_EXTERNAL_DATA so that the full data is built into the onboard
// Flash. You may also enable Serial log output by defining HYDRUINO_ENABLE_DEBUG_OUTPUT.
// You may refer to: https://forum.arduino.cc/index.php?topic=602603.0 on how to define
// custom build flags manually via modifying platform[.local].txt.
//
// In Hydroponics.h:
// 
// // Uncomment or -D this define to enable external data storage (SD Card or EEPROM) to save on sketch size. Required for constrained devices.
// // #define HYDRUINO_ENABLE_EXTERNAL_DATA             // If enabled, disables built-in Crops Lib and String data, instead relying solely on external device.
// 
// // Uncomment or -D this define to enable debug output (treats Serial as attached to serial monitor).
// #define HYDRUINO_ENABLE_DEBUG_OUTPUT
//
// Alternatively, in platform[.local].txt:
// build.extra_flags=-DHYDRUINO_ENABLE_DEBUG_OUTPUT

#include <Hydroponics.h>

// Compiler flag check
#ifdef HYDRUINO_ENABLE_EXTERNAL_DATA
#error The HYDRUINO_ENABLE_EXTERNAL_DATA flag is expected to be disabled in order to run this sketch
#endif

// Pins & Class Instances
#define SETUP_EEPROM_DEVICE_SIZE        I2C_DEVICESIZE_24LC256 // EEPROM bit storage size (use I2C_DEVICESIZE_* defines), else 0
#define SETUP_SD_CARD_CS_PIN            SS              // SD card CS pin, else -1
#define SETUP_EEPROM_I2C_ADDR           B000            // EEPROM address
#define SETUP_RTC_I2C_ADDR              B000            // RTC i2c address (only B000 can be used atm)
#define SETUP_I2C_WIRE_INST             Wire            // I2C wire class instance
#define SETUP_I2C_SPEED                 400000U         // I2C speed, in Hz
#define SETUP_ESP_I2C_SDA               SDA             // I2C SDA pin, if on ESP
#define SETUP_ESP_I2C_SCL               SCL             // I2C SCL pin, if on ESP
#define SETUP_SD_CARD_SPI_SPEED         4000000U        // SD card SPI speed, in Hz (ignored if on Teensy)

// External Data Settings
#define SETUP_EXTDATA_SD_ENABLE         true            // If data should be written to an external SD Card
#define SETUP_EXTDATA_SD_LIB_PREFIX     "lib/"          // Library data folder/data file prefix (appended with {type}##.dat)
#define SETUP_EXTDATA_EEPROM_ENABLE     true            // If data should be written to an external EEPROM
#define SETUP_EXTDATA_EEPROM_BEG_ADDR   0               // Start address for data written to EEPROM

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
    Serial.begin(115200);               // Begin USB Serial interface
    while(!Serial) { ; }                // Wait for USB Serial to connect
    #if defined(ESP32) || defined(ESP8266)
        SETUP_I2C_WIRE_INST.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL); // Begin i2c Wire for ESP
    #endif

    // Just a lone initializer is all that's needed since we won't actually be using the full controller.
    hydroController.init();

    // Right here would be the place to program in any custom crop data that you want made available for later.
    //HydroponicsCropsLibData customCrop1(Hydroponics_CropType_CustomCrop1);
    //strncpy(customCrop1.cropName, "Custom name", HYDRUINO_NAME_MAXSIZE);
    //getCropsLibraryInstance()->setCustomCropData(&customCrop1);

    getLoggerInstance()->logMessage(F("Writing external data..."));

    #if SETUP_EXTDATA_SD_ENABLE
    {   auto sd = getHydroponicsInstance()->getSDCard();

        if (sd) {
            getLoggerInstance()->logMessage(F("=== Writing Crops Library data to SD Card ==="));

            for (int cropType = 0; cropType < Hydroponics_CropType_Count; ++cropType) {
                auto cropData = getCropsLibraryInstance()->checkoutCropsData((Hydroponics_CropType)cropType);

                if (cropData) {
                    getLoggerInstance()->logMessage(F("Writing Crop: "), charsToString(cropData->cropName, HYDRUINO_NAME_MAXSIZE));
                    String filename = getNNFilename(String(F(SETUP_EXTDATA_SD_LIB_PREFIX)) + String(F("crop")), cropType, SFP(HStr_dat));
                    getLoggerInstance()->logMessage(F("... to file: "), filename);

                    auto file = sd->open(filename, O_WRITE | O_CREAT | O_TRUNC); // Creates/resets file for writing
                    if (file.availableForWrite()) {
                        StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;
                        JsonObject jsonObject = doc.to<JsonObject>();
                        uint16_t bytesWritten = serializeJsonPretty(jsonObject, file); // Could also write out in binary but we have acres of cheap storage

                        if (bytesWritten) {
                            getLoggerInstance()->logMessage(F("Wrote: "), String(bytesWritten), F(" bytes"));
                        } else {
                            getLoggerInstance()->logError(F("Failure writing to crops lib data file!"));
                        }
                        file.close();
                    } else {
                        if (file) { file.close(); }
                        getLoggerInstance()->logError(F("Failure opening crops lib data file for writing!"));
                    }

                    getCropsLibraryInstance()->returnCropsData(cropData);
                }

                yield();
            }

            {   getLoggerInstance()->logMessage(F("=== Writing String data to SD Card ==="));

                uint16_t lookupTable[Hydroponics_Strings_Count];

                // Initializes lookup table with proper offsets
                {   uint16_t writeOffset = sizeof(lookupTable);

                    for (int stringNum = 0; stringNum < Hydroponics_Strings_Count; ++stringNum) {
                        String string = SFP((Hydroponics_String)stringNum);
                        lookupTable[stringNum] = writeOffset;
                        writeOffset += string.length() + 1;
                    }
                }

                getLoggerInstance()->logMessage(F("Writing Strings"));
                String filename = String(String(F(SETUP_EXTDATA_SD_LIB_PREFIX)) + String(F("strings.")) + SFP(HStr_dat));
                getLoggerInstance()->logMessage(F("... to file: "), filename);

                auto file = sd->open(filename, O_WRITE | O_CREAT | O_TRUNC); // Creates/resets file for writing
                if (file.availableForWrite()) { // Strings data goes into a single file as binary
                    uint16_t bytesWritten = 0;

                    // Lookup table constructed first to avoid random seeking
                    bytesWritten += file.write((const byte *)lookupTable, sizeof(lookupTable));

                    for (int stringNum = 0; stringNum < Hydroponics_Strings_Count; ++stringNum) {
                        String string = SFP((Hydroponics_String)stringNum);
                        bytesWritten += file.write(string.c_str(), string.length() + 1); // +1 to also write out null terminator
                    }

                    if (bytesWritten) {
                        getLoggerInstance()->logMessage(F("Wrote: "), String(bytesWritten), F(" bytes"));
                    } else {
                        getLoggerInstance()->logError(F("Failure writing to strings data file!"));
                    }
                    file.close();
                } else {
                    if (file) { file.close(); }
                    getLoggerInstance()->logError(F("Failure opening strings data file for writing!"));
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

    #if SETUP_EXTDATA_EEPROM_ENABLE
    {   auto eeprom = getHydroponicsInstance()->getEEPROM();

        if (eeprom) {
            uint16_t cropsLibBegAddr = SETUP_EXTDATA_EEPROM_BEG_ADDR;
            uint16_t stringsBegAddr = (uint16_t)-1;
            uint16_t sysDataBegAddr = (uint16_t)-1;

            {   getLoggerInstance()->logMessage(F("=== Writing Crops Library data to EEPROM ==="));

                // A lookup table similar to uint16_t lookupTable[Hydroponics_Strings_Count] is created
                // manually here, which is used for crop data lookup. The first uint16_t value will be
                // reserved for the total chunk size (hence the +1).
                uint16_t writeOffset = cropsLibBegAddr + (sizeof(uint16_t) * Hydroponics_CropType_Count + 1);

                for (int cropType = 0; cropType < Hydroponics_CropType_Count; ++cropType) {
                    auto cropData = getCropsLibraryInstance()->checkoutCropsData((Hydroponics_CropType)cropType);

                    if (cropData) {
                        getLoggerInstance()->logMessage(F("Writing Crop: "), charsToString(cropData->cropName, HYDRUINO_NAME_MAXSIZE));
                        getLoggerInstance()->logMessage(F("... to offset: "), String(writeOffset));

                        auto eepromStream = HydroponicsEEPROMStream(writeOffset, sizeof(HydroponicsCropsLibData));
                        size_t bytesWritten = serializeDataToBinaryStream(cropData, &eepromStream); // Could also write out in JSON, but is space inefficient

                        // After writing data out, write offset out to lookup table
                        if (bytesWritten && eeprom->updateBlockVerify(cropsLibBegAddr + (sizeof(uint16_t) * (cropType + 1)),
                                                                      (const byte *)&writeOffset, sizeof(uint16_t))) {
                            writeOffset += bytesWritten;
                        } else {
                            getLoggerInstance()->logError(F("Failure writing crops lib data to EEPROM!"));
                        }

                        getCropsLibraryInstance()->returnCropsData(cropData);
                    } else if (!eeprom->setBlockVerify(cropsLibBegAddr + (sizeof(uint16_t) * (cropType + 1)), 0, sizeof(uint16_t))) {
                        getLoggerInstance()->logError(F("Failure writing crops lib table data to EEPROM!"));
                    }

                    yield();
                }

                // Write out total crops lib size to first position, as long as something was at least written out
                stringsBegAddr = cropsLibBegAddr;
                if (writeOffset > cropsLibBegAddr + (sizeof(uint16_t) * Hydroponics_CropType_Count + 1)) {
                    uint16_t totalBytesWritten = writeOffset - cropsLibBegAddr;

                    if (eeprom->updateBlockVerify(cropsLibBegAddr, (const byte *)&totalBytesWritten, sizeof(uint16_t))) {
                        stringsBegAddr = writeOffset;
                        getLoggerInstance()->logMessage(F("Successfully wrote: "), String(totalBytesWritten), F(" bytes"));
                    } else {
                        getLoggerInstance()->logError(F("Failure writing total crops lib data size to EEPROM!"));
                    }
                }
            }

            {   getLoggerInstance()->logMessage(F("=== Writing String data to EEPROM ==="));

                // Similar to above, same deal with a lookup table.
                uint16_t writeOffset = stringsBegAddr + (sizeof(uint16_t) * (Hydroponics_Strings_Count + 1));

                for (int stringNum = 0; stringNum < Hydroponics_Strings_Count; ++stringNum) {
                    String string = SFP((Hydroponics_String)stringNum);

                    getLoggerInstance()->logMessage(F("Writing String: #"), String(stringNum) + String(F(" \"")), string + String(F("\"")));
                    getLoggerInstance()->logMessage(F("... to offset: "), String(writeOffset));

                    if(eeprom->updateBlockVerify(writeOffset, (const byte *)string.c_str(), string.length() + 1) &&
                       eeprom->updateBlockVerify(stringsBegAddr + (sizeof(uint16_t) * (stringNum + 1)),
                                                 (const byte *)&writeOffset, sizeof(uint16_t))) {
                        writeOffset += string.length() + 1;
                    } else {
                        getLoggerInstance()->logError(F("Failure writing strings data to EEPROM!"));
                    }

                    yield();
                }

                sysDataBegAddr = stringsBegAddr;
                if (writeOffset > stringsBegAddr + (sizeof(uint16_t) * (Hydroponics_Strings_Count + 1))) {
                    uint16_t totalBytesWritten = writeOffset - stringsBegAddr;

                    if (eeprom->updateBlockVerify(stringsBegAddr, (const byte *)&totalBytesWritten, sizeof(uint16_t))) {
                        getLoggerInstance()->logMessage(F("Successfully wrote: "), String(totalBytesWritten), F(" bytes"));
                        sysDataBegAddr = writeOffset;
                    } else {
                        getLoggerInstance()->logError(F("Failure writing total strings data size to EEPROM!"));
                    }
                }
            }

            getLoggerInstance()->logMessage(F("Total EEPROM usage: "), String(sysDataBegAddr), F(" bytes"));
            getLoggerInstance()->logMessage(F("EEPROM capacity used: "), String(((float)sysDataBegAddr / eeprom->getDeviceSize()) * 100.0f) + String(F("% of ")), String(eeprom->getDeviceSize()) + String(F(" bytes")));
            getLoggerInstance()->logMessage(F("Use the following EEPROM setup defines in your sketch:"));
            Serial.print(F("#define SETUP_EXTDATA_EEPROM_CROP_ADDR  0x"));
            Serial.println(String(cropsLibBegAddr != (uint16_t)-1 ? cropsLibBegAddr : 0, 16));
            Serial.print(F("#define SETUP_EXTDATA_EEPROM_STR_ADDR   0x"));
            Serial.println(String(stringsBegAddr != (uint16_t)-1 && stringsBegAddr != cropsLibBegAddr ? stringsBegAddr : 0, 16));
            Serial.print(F("#define SETUP_EEPROM_SYSDATA_ADDR       0x"));
            Serial.println(String(sysDataBegAddr != (uint16_t)-1 && sysDataBegAddr != stringsBegAddr ? sysDataBegAddr : 0, 16));

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
