// Crops Lib to CPP export script - mainly for dev purposes

#include <Hydroponics.h>

#define SETUP_EEPROM_DEVICE_SIZE    I2C_DEVICESIZE_24LC256
#define SETUP_SD_CARD_CS_PIN        SS
#define SETUP_EEPROM_I2C_ADDR       B000
#define SETUP_RTC_I2C_ADDR          B000
#define SETUP_I2C_WIRE_INST         Wire
#define SETUP_I2C_SPEED             400000U
#define SETUP_ESP_I2C_SDA           SDA
#define SETUP_ESP_I2C_SCL           SCL
#define SETUP_SD_CARD_SPI_SPEED     4000000U

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
    Serial.begin(115200);
    while(!Serial) { ; }
    #if defined(ESP32) || defined(ESP8266)
        SETUP_I2C_WIRE_INST.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL);
    #endif

    hydroController.init();

    getLoggerInstance()->logMessage(F("Writing crops library..."));

    String spacing(F("             "));
    for (int cropType = 0; cropType < Hydroponics_CropType_Count; ++cropType) {
        auto cropData = getCropsLibraryInstance()->checkoutCropsData((Hydroponics_CropType)cropType);

        if (cropData) {
            Serial.print(spacing);
            Serial.print(F("case Hydroponics_CropType_"));
            Serial.print(cropTypeToString((Hydroponics_CropType)cropType));
            Serial.println(':');
            Serial.print(spacing);
            Serial.print(F("    return new HydroponicsCropsLibraryBook(String(F(\""));

            {   StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;
                JsonObject jsonObject = doc.to<JsonObject>();
                cropData->toJSONObject(jsonObject);

                String giantStr;
                serializeJson(jsonObject, giantStr);
                giantStr.replace("\"", "\\\"");
                Serial.print(giantStr);
            }

            Serial.println(F("\")));"));

            getCropsLibraryInstance()->returnCropsData(cropData);
        }

        yield();
    }

    getLoggerInstance()->logMessage(F("Done!"));
}

void loop()
{ ; }
