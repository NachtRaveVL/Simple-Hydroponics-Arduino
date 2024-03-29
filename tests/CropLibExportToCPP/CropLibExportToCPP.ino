// Crops Lib to CPP export script - mainly for dev purposes

#include <Hydruino.h>

#ifdef HYDRO_DISABLE_BUILTIN_DATA
#error The HYDRO_DISABLE_BUILTIN_DATA flag is expected to be undefined in order to run this sketch
#endif

// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_TYPE        None            // EEPROM device type/size (AT24LC01, AT24LC02, AT24LC04, AT24LC08, AT24LC16, AT24LC32, AT24LC64, AT24LC128, AT24LC256, AT24LC512, None)
#define SETUP_EEPROM_I2C_ADDR           0b000           // EEPROM i2c address (A0-A2, bitwise or'ed with base address 0x50)
#define SETUP_RTC_DEVICE_TYPE           None            // RTC device type (DS1307, DS3231, PCF8523, PCF8563, None)
#define SETUP_SD_CARD_SPI               SPI             // SD card SPI class instance
#define SETUP_SD_CARD_SPI_CS            -1              // SD card CS pin, else -1
#define SETUP_SD_CARD_SPI_SPEED         F_SPD           // SD card SPI speed, in Hz (ignored on Teensy)
#define SETUP_I2C_WIRE                  Wire            // I2C wire class instance
#define SETUP_I2C_SPEED                 400000U         // I2C speed, in Hz
#define SETUP_ESP_I2C_SDA               SDA             // I2C SDA pin, if on ESP
#define SETUP_ESP_I2C_SCL               SCL             // I2C SCL pin, if on ESP

Hydruino hydroController((pintype_t)SETUP_PIEZO_BUZZER_PIN,
                         JOIN(Hydro_EEPROMType,SETUP_EEPROM_DEVICE_TYPE),
                         I2CDeviceSetup((uint8_t)SETUP_EEPROM_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
                         JOIN(Hydro_RTCType,SETUP_RTC_DEVICE_TYPE),
                         I2CDeviceSetup((uint8_t)0b000, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
                         SPIDeviceSetup((pintype_t)SETUP_SD_CARD_SPI_CS, &SETUP_SD_CARD_SPI, SETUP_SD_CARD_SPI_SPEED));

void setup() {
    // Setup base interfaces
    #ifdef HYDRO_ENABLE_DEBUG_OUTPUT
        Serial.begin(115200);           // Begin USB Serial interface
        while (!Serial) { ; }           // Wait for USB Serial to connect
    #endif
    #if defined(ESP_PLATFORM)
        SETUP_I2C_WIRE.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL); // Begin i2c Wire for ESP
    #endif

    hydroController.init();

    getLogger()->logMessage(F("Writing crops library..."));

    String spacing(F("            "));
    String indent = spacing + F("    ");
    for (int cropType = 0; cropType < Hydro_CropType_Count; ++cropType) {
        auto cropData = hydroCropsLib.checkoutCropsData((Hydro_CropType)cropType);

        if (cropData) {
            // case Hydro_CropType_AloeVera: {
            //     static const char flashStr_AloeVera[] PROGMEM = {"{\"type\":\"HCLD\"}"};
            //     progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_AloeVera);
            // } break;

            Serial.print(spacing);
            Serial.print(F("case Hydro_CropType_"));
            Serial.print(cropTypeToString((Hydro_CropType)cropType));
            Serial.println(F(": {"));

            Serial.print(indent);
            Serial.print(F("static const char flashStr_"));
            Serial.print(cropTypeToString((Hydro_CropType)cropType));
            Serial.print(F("[] PROGMEM = {\""));

            {   StaticJsonDocument<HYDRO_JSON_DOC_DEFSIZE> doc;
                JsonObject jsonObject = doc.to<JsonObject>();
                cropData->toJSONObject(jsonObject);

                String giantStr;
                serializeJson(jsonObject, giantStr);
                giantStr.replace("\"", "\\\"");
                Serial.print(giantStr);
            }

            Serial.println(F("\"};"));

            Serial.print(indent);
            Serial.print(F("progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_"));
            Serial.print(cropTypeToString((Hydro_CropType)cropType));
            Serial.println(F(");"));

            Serial.print(spacing);
            Serial.println(F("} break;"));

            hydroCropsLib.returnCropsData(cropData);
        }

        yield();
    }

    getLogger()->logMessage(F("Done!"));
}

void loop()
{ ; }
