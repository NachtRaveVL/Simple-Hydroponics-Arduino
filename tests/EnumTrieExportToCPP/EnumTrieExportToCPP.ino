// Enum to CPP export script - mainly for dev purposes

#include <Hydruino.h>

#ifdef HYDRO_DISABLE_BUILTIN_DATA
#error The HYDRO_DISABLE_BUILTIN_DATA flag is expected to be undefined in order to run this sketch
#endif

/// Pins & Class Instances
#define SETUP_PIEZO_BUZZER_PIN          -1              // Piezo buzzer pin, else -1
#define SETUP_EEPROM_DEVICE_TYPE        None            // EEPROM device type/size (24LC01, 24LC02, 24LC04, 24LC08, 24LC16, 24LC32, 24LC64, 24LC128, 24LC256, 24LC512, None)
#define SETUP_EEPROM_I2C_ADDR           B000            // EEPROM i2c address
#define SETUP_RTC_I2C_ADDR              B000            // RTC i2c address (only B000 can be used atm)
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
                         I2CDeviceSetup((uint8_t)SETUP_RTC_I2C_ADDR, &SETUP_I2C_WIRE, SETUP_I2C_SPEED),
                         SPIDeviceSetup((pintype_t)SETUP_SD_CARD_SPI_CS, &SETUP_SD_CARD_SPI, SETUP_SD_CARD_SPI_SPEED));

struct TreeNode;
static TreeNode *_root;

// Trie Encoding Tree
struct TreeNode {
    String piece;
    Map<char, TreeNode *> map;
    bool end;
    int typeIndex;

    TreeNode(String slice) : piece(slice), end(false) { ; }
    TreeNode(String slice, int typeIndexIn) : piece(slice), end(true), typeIndex(typeIndexIn) { ; }

    ~TreeNode() {
        for (auto iter = map.begin(); iter != map.end(); ++iter) {
            delete iter->second;
        }
    }

    // Inserts fragment into node with given parent (or null for root) and typeIndex
    void insert(TreeNode *parent, String fragment, int typeIndexIn) {
        if (fragment.length()) {
            for (int i = 0; i <= piece.length() && i <= fragment.length(); ++i) {
                if (i == piece.length()) {
                    if (!map[fragment[i]]) {
                        map[fragment[i]] = new TreeNode(fragment.substring(i), typeIndexIn);
                    } else {
                        map[fragment[i]]->insert(this, fragment.substring(i), typeIndexIn);
                    }
                    return;
                }
                if (i == fragment.length() || piece[i] != fragment[i]) {
                    String pieceSplit = piece.substring(i);

                    if (pieceSplit.length()) {
                        auto newParent = new TreeNode(piece.substring(0, i));
                        piece = pieceSplit;

                        if (parent) {
                            parent->map[newParent->piece[0]] = newParent;
                        } else {
                            _root = newParent;
                        }
                        newParent->map[piece[0]] = this;

                        newParent->insert(parent, fragment, typeIndexIn);

                        return;
                    } else {
                        end = true;
                        typeIndex = typeIndexIn;
                    }
                    return;
                }
            }
        } else {
            end = true;
            typeIndex = typeIndexIn;
        }
    }

    // Prints tab spaces in front
    void printSpacer(int level) {
        for (int i = 0; i < (level << 2); ++i) { Serial.print(' '); }
    }

    // Prints encoding tree out for debug
    void printDebug(int level) {
        printSpacer(level);
        Serial.print('-'); Serial.print(' '); Serial.println(piece);
        for (auto iter = map.begin(); iter != map.end(); ++iter) {
            iter->second->printDebug(level+1);
        }
    }

    // Prints encoding tree out in code
    void printCode(int level, const String &varName, const String &typeCast, int index = 0) {
        bool printedSwitch = false;

        if (end) {
            if (!map.size()) {
                printSpacer(level);
            } else {
                if (!printedSwitch) {
                    printedSwitch = true;
                    printSpacer(level);
                    Serial.print(F("switch (")); Serial.print(varName); Serial.print(F(".length() >= ")); Serial.print(index + 1); Serial.print(F(" ? ")); Serial.print(varName); Serial.print(F("[")); Serial.print(index); Serial.println(F("] : '\\0') {"));
                }
                printSpacer(level + 1);
                Serial.println(F("case '\\0':"));
                printSpacer(level + 2);
            }
            Serial.print(F("return ")); Serial.print(typeCast); Serial.print(typeIndex); Serial.println(F(";"));
        }

        for (auto iter = map.begin(); iter != map.end(); ++iter) {
            if (!printedSwitch) {
                printedSwitch = true;
                printSpacer(level);
                Serial.print(F("switch (")); Serial.print(varName); Serial.print(F(".length() >= ")); Serial.print(index + 1); Serial.print(F(" ? ")); Serial.print(varName); Serial.print(F("[")); Serial.print(index); Serial.println(F("] : '\\0') {"));
            }
            printSpacer(level + 1);
            Serial.print(F("case '")); Serial.print(iter->first); Serial.println(F("':"));

            iter->second->printCode(level + 2, varName, typeCast, index + iter->second->piece.length());

            if (!(iter->second->end && !iter->second->map.size())) {
                printSpacer(level + 2);
                Serial.println(F("break;"));
            }
        }

        if (printedSwitch) {
            for (int i = 0; i < (level << 2); ++i) { Serial.print(' '); }
            Serial.println(F("}"));
        }
    }
};

void printCropTypeTree() {
    String typeCast(F("(Hydro_CropType)"));
    String varName(F("cropTypeStr"));
    _root->printCode(1, varName, typeCast);
    delete _root; _root = new TreeNode("");
}

void buildCropTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");

    for (char charIndex = 'A'; charIndex <= 'Z'; ++charIndex) {
        if (freeMemory() < 2000 && _root->map.size()) { printCropTypeTree(); }
        for (int typeIndex = -1; typeIndex <= Hydro_CropType_Count; ++typeIndex) {
            String cropType = cropTypeToString((Hydro_CropType)typeIndex);
            if (toupper(cropType[0]) == charIndex) {
                _root->insert(nullptr, cropType, typeIndex);
            }
        }
    }

    for (int typeIndex = -1; typeIndex <= Hydro_CropType_Count; ++typeIndex) {
        String cropType = cropTypeToString((Hydro_CropType)typeIndex);
        if (toupper(cropType[0]) < 'A' || toupper(cropType[0]) > 'Z') {
            _root->insert(nullptr, cropType, typeIndex);
        }
    }

    if (_root->map.size()) { printCropTypeTree(); }
}

void buildSystemModeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydro_SystemMode_Count; ++typeIndex) {
        _root->insert(nullptr, systemModeToString((Hydro_SystemMode)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydro_SystemMode)"));
    String varName(F("systemModeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildMeasurementModeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydro_MeasurementMode_Count; ++typeIndex) {
        _root->insert(nullptr, measurementModeToString((Hydro_MeasurementMode)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydro_MeasurementMode)"));
    String varName(F("measurementModeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildDisplayOutputModeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydro_DisplayOutputMode_Count; ++typeIndex) {
        _root->insert(nullptr, displayOutputModeToString((Hydro_DisplayOutputMode)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydro_DisplayOutputMode)"));
    String varName(F("displayOutModeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildControlInputModeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydro_ControlInputMode_Count; ++typeIndex) {
        _root->insert(nullptr, controlInputModeToString((Hydro_ControlInputMode)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydro_ControlInputMode)"));
    String varName(F("controlInModeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildActuatorTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydro_ActuatorType_Count; ++typeIndex) {
        _root->insert(nullptr, actuatorTypeToString((Hydro_ActuatorType)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydro_ActuatorType)"));
    String varName(F("actuatorTypeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildSensorTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydro_SensorType_Count; ++typeIndex) {
        _root->insert(nullptr, sensorTypeToString((Hydro_SensorType)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydro_SensorType)"));
    String varName(F("sensorTypeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildSubstrateTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydro_SubstrateType_Count; ++typeIndex) {
        _root->insert(nullptr, substrateTypeToString((Hydro_SubstrateType)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydro_SubstrateType)"));
    String varName(F("substrateTypeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildReservoirTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydro_ReservoirType_Count; ++typeIndex) {
        _root->insert(nullptr, reservoirTypeToString((Hydro_ReservoirType)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydro_ReservoirType)"));
    String varName(F("reservoirTypeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildRailTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydro_RailType_Count; ++typeIndex) {
        _root->insert(nullptr, railTypeToString((Hydro_RailType)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydro_RailType)"));
    String varName(F("railTypeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildUnitsCategoryTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydro_UnitsCategory_Count; ++typeIndex) {
        _root->insert(nullptr, unitsCategoryToString((Hydro_UnitsCategory)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydro_UnitsCategory)"));
    String varName(F("unitsCategoryStr"));
    _root->printCode(1, varName, typeCast);
}

void printUnitsTypeTree() {
    String typeCast(F("(Hydro_UnitsType)"));
    String varName(F("unitsSymbolStr"));
    _root->printCode(1, varName, typeCast);
    delete _root; _root = new TreeNode("");
}

void buildUnitsTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");

    for (char charIndex = 'A'; charIndex <= 'Z'; ++charIndex) {
        if (freeMemory() < 2000 && _root->map.size()) { printUnitsTypeTree(); }
        for (int typeIndex = -1; typeIndex <= Hydro_UnitsType_Count; ++typeIndex) {
            String unitsType = unitsTypeToSymbol((Hydro_UnitsType)typeIndex);
            if (toupper(unitsType[0]) == charIndex) {
                _root->insert(nullptr, unitsType, typeIndex);
            }
        }
        // aliases
        if (charIndex == 'J') { _root->insert(nullptr, F("J/s"), Hydro_UnitsType_Power_Wattage); }
        if (charIndex == 'M') { _root->insert(nullptr, F("mS/cm"), Hydro_UnitsType_Concentration_EC); }
        if (charIndex == 'P') { _root->insert(nullptr, F("ppm"), Hydro_UnitsType_Concentration_PPM); }
        if (charIndex == 'T') { _root->insert(nullptr, F("TDS"), Hydro_UnitsType_Concentration_TDS); }
    }

    for (int typeIndex = -1; typeIndex <= Hydro_UnitsType_Count; ++typeIndex) {
        String unitsType = unitsTypeToSymbol((Hydro_UnitsType)typeIndex);
        if (toupper(unitsType[0]) < 'A' || toupper(unitsType[0]) > 'Z') {
            _root->insert(nullptr, unitsType, typeIndex);
        }
    }

    if (_root->map.size()) { printUnitsTypeTree(); }
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

    hydroController.init();

    getLoggerInstance()->logMessage(F("Writing enum decoding tree..."));

    getLoggerInstance()->logMessage(F("System mode tree:"));
    buildSystemModeTree();

    getLoggerInstance()->logMessage(F("Measurements tree:"));
    buildMeasurementModeTree();

    getLoggerInstance()->logMessage(F("Display output tree:"));
    buildDisplayOutputModeTree();

    getLoggerInstance()->logMessage(F("Control input tree:"));
    buildControlInputModeTree();

    getLoggerInstance()->logMessage(F("Actuator type tree:"));
    buildActuatorTypeTree();

    getLoggerInstance()->logMessage(F("Sensor type tree:"));
    buildSensorTypeTree();

    getLoggerInstance()->logMessage(F("Crop type tree: (multiple switches)"));
    buildCropTypeTree();

    getLoggerInstance()->logMessage(F("Substrate type tree:"));
    buildSubstrateTypeTree();

    getLoggerInstance()->logMessage(F("Reservoir type tree:"));
    buildReservoirTypeTree();

    getLoggerInstance()->logMessage(F("Rail type tree:"));
    buildRailTypeTree();

    getLoggerInstance()->logMessage(F("Units category tree:"));
    buildUnitsCategoryTree();

    getLoggerInstance()->logMessage(F("Units type tree: (multiple switches)"));
    buildUnitsTypeTree();

    getLoggerInstance()->logMessage(F("Done!"));
    if (_root) { delete _root; _root = nullptr; }
}

void loop()
{ ; }
