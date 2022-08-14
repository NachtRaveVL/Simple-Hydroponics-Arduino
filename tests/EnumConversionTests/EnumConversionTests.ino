// Crops Lib to CPP export script - mainly for dev purposes

#include <Hydroponics.h>

#ifdef HYDRUINO_DISABLE_BUILTIN_DATA
#error The HYDRUINO_DISABLE_BUILTIN_DATA flag is expected to be undefined in order to run this sketch
#endif

#define SETUP_PIEZO_BUZZER_PIN          -1
#define SETUP_EEPROM_DEVICE_SIZE        I2C_DEVICESIZE_24LC256
#define SETUP_SD_CARD_CS_PIN            SS
#define SETUP_EEPROM_I2C_ADDR           B000
#define SETUP_RTC_I2C_ADDR              B000
#define SETUP_I2C_WIRE_INST             Wire
#define SETUP_I2C_SPEED                 400000U
#define SETUP_ESP_I2C_SDA               SDA
#define SETUP_ESP_I2C_SCL               SCL
#define SETUP_SD_CARD_SPI_SPEED         4000000U

Hydroponics hydroController(SETUP_PIEZO_BUZZER_PIN,
                            SETUP_EEPROM_DEVICE_SIZE,
                            SETUP_EEPROM_I2C_ADDR,
                            SETUP_RTC_I2C_ADDR,
                            SETUP_SD_CARD_CS_PIN,
                            SETUP_SD_CARD_SPI_SPEED,
                            nullptr,
                            0,
                            SETUP_I2C_WIRE_INST,
                            SETUP_I2C_SPEED);

void testSystemModeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_SystemMode_Count; ++typeIndex) {
        String typeString = systemModeToString((Hydroponics_SystemMode)typeIndex);
        int retTypeIndex = (int)systemModeFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testSystemModeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testMeasurementModeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_MeasurementMode_Count; ++typeIndex) {
        String typeString = measurementModeToString((Hydroponics_MeasurementMode)typeIndex);
        int retTypeIndex = (int)measurementModeFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testMeasurementModeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testDisplayOutputModeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_DisplayOutputMode_Count; ++typeIndex) {
        String typeString = displayOutputModeToString((Hydroponics_DisplayOutputMode)typeIndex);
        int retTypeIndex = (int)displayOutputModeFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testDisplayOutputModeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testControlInputModeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_ControlInputMode_Count; ++typeIndex) {
        String typeString = controlInputModeToString((Hydroponics_ControlInputMode)typeIndex);
        int retTypeIndex = (int)controlInputModeFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testControlInputModeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testActuatorTypeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_ActuatorType_Count; ++typeIndex) {
        String typeString = actuatorTypeToString((Hydroponics_ActuatorType)typeIndex);
        int retTypeIndex = (int)actuatorTypeFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testActuatorTypeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testSensorTypeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_SensorType_Count; ++typeIndex) {
        String typeString = sensorTypeToString((Hydroponics_SensorType)typeIndex);
        int retTypeIndex = (int)sensorTypeFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testSensorTypeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testCropTypeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_CropType_Count; ++typeIndex) {
        String typeString = cropTypeToString((Hydroponics_CropType)typeIndex);
        int retTypeIndex = (int)cropTypeFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testCropTypeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testSubstrateTypeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_SubstrateType_Count; ++typeIndex) {
        String typeString = substrateTypeToString((Hydroponics_SubstrateType)typeIndex);
        int retTypeIndex = (int)substrateTypeFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testSubstrateTypeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testReservoirTypeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_ReservoirType_Count; ++typeIndex) {
        String typeString = reservoirTypeToString((Hydroponics_ReservoirType)typeIndex);
        int retTypeIndex = (int)reservoirTypeFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testReservoirTypeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testRailTypeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_RailType_Count; ++typeIndex) {
        String typeString = railTypeToString((Hydroponics_RailType)typeIndex);
        int retTypeIndex = (int)railTypeFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testRailTypeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testUnitsCategoryEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_UnitsCategory_Count; ++typeIndex) {
        String typeString = unitsCategoryToString((Hydroponics_UnitsCategory)typeIndex);
        int retTypeIndex = (int)unitsCategoryFromString(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testUnitsCategoryEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void testUnitsTypeEnums()
{
    for (int typeIndex = -1; typeIndex <= Hydroponics_UnitsType_Count; ++typeIndex) {
        String typeString = unitsTypeToSymbol((Hydroponics_UnitsType)typeIndex);
        int retTypeIndex = (int)unitsTypeFromSymbol(typeString);
        if (typeIndex != retTypeIndex) {
            getLoggerInstance()->logError(F("testUnitsTypeEnums: Conversion failure: "), String(typeIndex));
            getLoggerInstance()->logError(F("  Invalid return: "), String(retTypeIndex), String(F(" (")) + typeString + String(F(")")));
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }
    #if defined(ESP_PLATFORM)
        SETUP_I2C_WIRE_INST.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL);
    #endif

    hydroController.init();

    getLoggerInstance()->logMessage(F("=BEGIN="));

    testSystemModeEnums();
    testMeasurementModeEnums();
    testDisplayOutputModeEnums();
    testControlInputModeEnums();
    testActuatorTypeEnums();
    testSensorTypeEnums();
    testCropTypeEnums();
    testSubstrateTypeEnums();
    testReservoirTypeEnums();
    testRailTypeEnums();
    testUnitsCategoryEnums();
    testUnitsTypeEnums();

    getLoggerInstance()->logMessage(F("=FINISH="));
}

void loop()
{ ; }
