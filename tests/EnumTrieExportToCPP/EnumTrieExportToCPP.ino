// Crops Lib to CPP export script - mainly for dev purposes

#include <Hydroponics.h>

#ifdef HYDRUINO_DISABLE_BUILTIN_DATA
#error The HYDRUINO_DISABLE_BUILTIN_DATA flag is expected to be undefined in order to run this sketch
#endif

#define SETUP_PIEZO_BUZZER_PIN          11
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
                            SETUP_SD_CARD_CS_PIN,
                            nullptr,
                            SETUP_EEPROM_I2C_ADDR,
                            SETUP_RTC_I2C_ADDR,
                            0,
                            SETUP_I2C_WIRE_INST,
                            SETUP_I2C_SPEED,
                            SETUP_SD_CARD_SPI_SPEED);
struct TreeNode;
static TreeNode *_root;

// Trie Encoding Tree
struct TreeNode {
    String piece;
    Map<char, TreeNode *>::type map;
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
    String typeCast(F("(Hydroponics_CropType)"));
    String varName(F("cropTypeStr"));
    _root->printCode(1, varName, typeCast);
    delete _root; _root = new TreeNode("");
}

void buildCropTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");

    for (char charIndex = 'A'; charIndex <= 'Z'; ++charIndex) {
        if (freeMemory() < 2000 && _root->map.size()) { printCropTypeTree(); }
        for (int typeIndex = -1; typeIndex <= Hydroponics_CropType_Count; ++typeIndex) {
            String cropType = cropTypeToString((Hydroponics_CropType)typeIndex);
            if (toupper(cropType[0]) == charIndex) {
                _root->insert(nullptr, cropType, typeIndex);
            }
        }
    }

    for (int typeIndex = -1; typeIndex <= Hydroponics_CropType_Count; ++typeIndex) {
        String cropType = cropTypeToString((Hydroponics_CropType)typeIndex);
        if (toupper(cropType[0]) < 'A' || toupper(cropType[0]) > 'Z') {
            _root->insert(nullptr, cropType, typeIndex);
        }
    }

    if (_root->map.size()) { printCropTypeTree(); }
}

void buildSystemModeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydroponics_SystemMode_Count; ++typeIndex) {
        _root->insert(nullptr, systemModeToString((Hydroponics_SystemMode)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydroponics_SystemMode)"));
    String varName(F("systemModeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildMeasurementModeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydroponics_MeasurementMode_Count; ++typeIndex) {
        _root->insert(nullptr, measurementModeToString((Hydroponics_MeasurementMode)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydroponics_MeasurementMode)"));
    String varName(F("measurementModeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildDisplayOutputModeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydroponics_DisplayOutputMode_Count; ++typeIndex) {
        _root->insert(nullptr, displayOutputModeToString((Hydroponics_DisplayOutputMode)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydroponics_DisplayOutputMode)"));
    String varName(F("displayOutModeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildControlInputModeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydroponics_ControlInputMode_Count; ++typeIndex) {
        _root->insert(nullptr, controlInputModeToString((Hydroponics_ControlInputMode)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydroponics_ControlInputMode)"));
    String varName(F("controlInModeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildActuatorTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydroponics_ActuatorType_Count; ++typeIndex) {
        _root->insert(nullptr, actuatorTypeToString((Hydroponics_ActuatorType)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydroponics_ActuatorType)"));
    String varName(F("actuatorTypeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildSensorTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydroponics_SensorType_Count; ++typeIndex) {
        _root->insert(nullptr, sensorTypeToString((Hydroponics_SensorType)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydroponics_SensorType)"));
    String varName(F("sensorTypeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildSubstrateTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydroponics_SubstrateType_Count; ++typeIndex) {
        _root->insert(nullptr, substrateTypeToString((Hydroponics_SubstrateType)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydroponics_SubstrateType)"));
    String varName(F("substrateTypeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildReservoirTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydroponics_ReservoirType_Count; ++typeIndex) {
        _root->insert(nullptr, reservoirTypeToString((Hydroponics_ReservoirType)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydroponics_ReservoirType)"));
    String varName(F("reservoirTypeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildRailTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydroponics_RailType_Count; ++typeIndex) {
        _root->insert(nullptr, railTypeToString((Hydroponics_RailType)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydroponics_RailType)"));
    String varName(F("railTypeStr"));
    _root->printCode(1, varName, typeCast);
}

void buildUnitsCategoryTree() {
    if (_root) { delete _root; } _root = new TreeNode("");
    for (int typeIndex = -1; typeIndex <= Hydroponics_UnitsCategory_Count; ++typeIndex) {
        _root->insert(nullptr, unitsCategoryToString((Hydroponics_UnitsCategory)typeIndex), typeIndex);
    }
    String typeCast(F("(Hydroponics_UnitsCategory)"));
    String varName(F("unitsCategoryStr"));
    _root->printCode(1, varName, typeCast);
}

void printUnitsTypeTree() {
    String typeCast(F("(Hydroponics_UnitsType)"));
    String varName(F("unitsSymbolStr"));
    _root->printCode(1, varName, typeCast);
    delete _root; _root = new TreeNode("");
}

void buildUnitsTypeTree() {
    if (_root) { delete _root; } _root = new TreeNode("");

    for (char charIndex = 'A'; charIndex <= 'Z'; ++charIndex) {
        if (freeMemory() < 2000 && _root->map.size()) { printUnitsTypeTree(); }
        for (int typeIndex = -1; typeIndex <= Hydroponics_UnitsType_Count; ++typeIndex) {
            String unitsType = unitsTypeToSymbol((Hydroponics_UnitsType)typeIndex);
            if (toupper(unitsType[0]) == charIndex) {
                _root->insert(nullptr, unitsType, typeIndex);
            }
        }
        // aliases
        if (charIndex == 'J') { _root->insert(nullptr, F("J/s"), Hydroponics_UnitsType_Power_Wattage); }
        if (charIndex == 'M') { _root->insert(nullptr, F("mS/cm"), Hydroponics_UnitsType_Concentration_EC); }
        if (charIndex == 'P') { _root->insert(nullptr, F("ppm"), Hydroponics_UnitsType_Concentration_PPM); }
        if (charIndex == 'T') { _root->insert(nullptr, F("TDS"), Hydroponics_UnitsType_Concentration_TDS); }
    }

    for (int typeIndex = -1; typeIndex <= Hydroponics_UnitsType_Count; ++typeIndex) {
        String unitsType = unitsTypeToSymbol((Hydroponics_UnitsType)typeIndex);
        if (toupper(unitsType[0]) < 'A' || toupper(unitsType[0]) > 'Z') {
            _root->insert(nullptr, unitsType, typeIndex);
        }
    }

    if (_root->map.size()) { printUnitsTypeTree(); }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }
    #if defined(ESP32) || defined(ESP8266)
        SETUP_I2C_WIRE_INST.begin(SETUP_ESP_I2C_SDA, SETUP_ESP_I2C_SCL);
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
