/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops Library
*/

#include "Hydroponics.h"

HydroponicsCropsLibraryBook::HydroponicsCropsLibraryBook()
    : data(), count(1), userSet(false)
{ ; }

HydroponicsCropsLibraryBook::HydroponicsCropsLibraryBook(String jsonStringIn)
    : data(), count(1), userSet(false)
{
    StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;
    deserializeJson(doc, jsonStringIn);
    auto cropsLibDataObj = doc.as<JsonObjectConst>();
    data.fromJSONObject(cropsLibDataObj);
}

HydroponicsCropsLibraryBook::HydroponicsCropsLibraryBook(Stream &streamIn, bool jsonFormat)
    : data(), count(1), userSet(false)
{
    if (jsonFormat) {
        StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;
        deserializeJson(doc, streamIn);
        auto cropsLibDataObj = doc.as<JsonObjectConst>();
        data.fromJSONObject(cropsLibDataObj);
    } else {
        deserializeDataFromBinaryStream(&data, &streamIn);
    }
}

HydroponicsCropsLibraryBook::HydroponicsCropsLibraryBook(const HydroponicsCropsLibData &dataIn)
    : data(dataIn), count(1), userSet(false)
{ ; }


HydroponicsCropsLibrary *HydroponicsCropsLibrary::_instance = nullptr;

HydroponicsCropsLibrary *HydroponicsCropsLibrary::getInstance()
{
    if (_instance) { return _instance; }
    else {
        CRITICAL_SECTION {
            if (!_instance) {
                _instance = new HydroponicsCropsLibrary();
            }
        }
        return _instance;
    }
}

void HydroponicsCropsLibrary::beginCropsLibraryFromSDCard(String libraryCropPrefix, bool jsonFormat)
{
    _libSDCropPrefix = libraryCropPrefix;
    _libSDJSONFormat = jsonFormat;
}

void HydroponicsCropsLibrary::beginCropsLibraryFromEEPROM(size_t dataAddress, bool jsonFormat)
{
    _libEEPROMDataAddress = dataAddress;
    _libEEPROMJSONFormat = dataAddress;
}

const HydroponicsCropsLibData *HydroponicsCropsLibrary::checkoutCropsData(Hydroponics_CropType cropType)
{
    HYDRUINO_SOFT_ASSERT((int)cropType >= 0 && cropType < Hydroponics_CropType_CustomCropCount, SFP(HS_Err_InvalidParameter));

    HydroponicsCropsLibraryBook *book = nullptr;
    auto iter = _cropsData.find(cropType);

    if (iter != _cropsData.end()) {
        book = iter->second;
        if (book) {
            book->count += 1;
        }
    } else if (cropType < Hydroponics_CropType_CustomCrop1 || (_libSDCropPrefix.length() || _libEEPROMDataAddress != (size_t)-1)) {
        book = newBookFromType(cropType);

        HYDRUINO_SOFT_ASSERT(book || cropType >= Hydroponics_CropType_CustomCrop1, SFP(HS_Err_AllocationFailure));
        if (book) {
            _cropsData[cropType] = book;
            HYDRUINO_HARD_ASSERT(_cropsData.find(cropType) != _cropsData.end(), SFP(HS_Err_OperationFailure));
        }
    }

    return book ? &(book->data) : nullptr;
}

void HydroponicsCropsLibrary::returnCropsData(const HydroponicsCropsLibData *cropData)
{
    HYDRUINO_HARD_ASSERT(cropData, SFP(HS_Err_InvalidParameter));

    auto iter = _cropsData.find(cropData->cropType);
    HYDRUINO_SOFT_ASSERT(iter != _cropsData.end(), F("No check outs for crop type"));

    if (iter != _cropsData.end()) {
        auto book = iter->second;
        if (book) {
            book->count -= 1;

            if (book->count <= 0 && // delete on 0 count
               (book->data.cropType < Hydroponics_CropType_CustomCrop1 || !book->userSet)) { // don't delete custom unless not user set
                if (iter->second) { delete iter->second; }
                _cropsData.erase(iter);
            }
        }
    }
}

bool HydroponicsCropsLibrary::setCustomCropData(const HydroponicsCropsLibData *cropData)
{
    HYDRUINO_HARD_ASSERT(cropData, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!cropData || (cropData->cropType >= Hydroponics_CropType_CustomCrop1 &&
                                       cropData->cropType < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount), SFP(HS_Err_InvalidParameter));

    if (cropData->cropType >= Hydroponics_CropType_CustomCrop1 &&
        cropData->cropType < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount) {
        Hydroponics_CropType cropType = cropData->cropType;
        auto iter = _cropsData.find(cropType);
        bool retVal = false;

        if (iter == _cropsData.end()) {
            auto book = new HydroponicsCropsLibraryBook(*cropData);

            HYDRUINO_SOFT_ASSERT(book, SFP(HS_Err_AllocationFailure));
            if (book) {
                book->userSet = true;
                _cropsData[cropData->cropType] = book;
                retVal = (_cropsData.find(cropData->cropType) != _cropsData.end());
            }
        } else {
            iter->second->data = *cropData;
            retVal = true;
        }

        if (retVal) {
            _hasCustomCrops = true;

            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<Hydroponics_CropType>(_cropDataSignal, cropType);
            #else
                _cropDataSignal.fire(cropType);
            #endif

            return true;
        }
    }
    return false;
}

bool HydroponicsCropsLibrary::dropCustomCropData(const HydroponicsCropsLibData *cropData)
{
    HYDRUINO_HARD_ASSERT(cropData, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!cropData || (cropData->cropType >= Hydroponics_CropType_CustomCrop1 &&
                                       cropData->cropType < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount), SFP(HS_Err_InvalidParameter));

    if (cropData->cropType >= Hydroponics_CropType_CustomCrop1 &&
        cropData->cropType < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount) {
        Hydroponics_CropType cropType = cropData->cropType;
        auto iter = _cropsData.find(cropType);

        if (iter != _cropsData.end()) {
            delete iter->second;
            _cropsData.erase(iter);

            updateHasCustom();

            #ifndef HYDRUINO_DISABLE_MULTITASKING
                scheduleSignalFireOnce<Hydroponics_CropType>(_cropDataSignal, cropType);
            #else
                _cropDataSignal.fire(cropType);
            #endif

            return true;
        }
    }

    return false;
}

bool HydroponicsCropsLibrary::updateHasCustom()
{
    for (auto iter = _cropsData.begin(); iter != _cropsData.end(); ++iter) {
        if (iter->first >= Hydroponics_CropType_CustomCrop1) {
            return (_hasCustomCrops = true);
        }
    }
    return (_hasCustomCrops = false);
}

Signal<Hydroponics_CropType> &HydroponicsCropsLibrary::getCustomCropSignal()
{
    return _cropDataSignal;
}

HydroponicsCropsLibraryBook * HydroponicsCropsLibrary::newBookFromType(Hydroponics_CropType cropType)
{
    if (_libSDCropPrefix.length()) {
        HydroponicsCropsLibraryBook *retVal = nullptr;
        auto sd = getHydroponicsInstance()->getSDCard();

        if (sd) {
            String filename = getNNFilename(_libSDCropPrefix, (unsigned int)cropType, SFP(HS_dat));

            auto file = sd->open(filename, FILE_READ);
            if (file) {
                retVal = new HydroponicsCropsLibraryBook(file, _libSDJSONFormat);
                file.close();
            }

            getHydroponicsInstance()->endSDCard(sd);
        }

        if (retVal) { return retVal; }
    }

    if (_libEEPROMDataAddress != (size_t)-1) {
        HydroponicsCropsLibraryBook *retVal = nullptr;
        auto eeprom = getHydroponicsInstance()->getEEPROM();

        if (eeprom) {
            uint16_t lookupOffset = 0;
            eeprom->readBlock(_libEEPROMDataAddress + (sizeof(uint16_t) * (int)(cropType + 1)), // +1 for initial total size word
                              (byte *)&lookupOffset, sizeof(lookupOffset));
            auto eepromStream = HydroponicsEEPROMStream(_libEEPROMDataAddress + lookupOffset, sizeof(HydroponicsCropsLibData));
            retVal = new HydroponicsCropsLibraryBook(eepromStream, _libEEPROMJSONFormat);
        }

        if (retVal) { return retVal; }
    }

    #ifndef HYDRUINO_DISABLE_BUILT_IN_CROPS_LIBRARY
        switch (cropType) {
            case Hydroponics_CropType_AloeVera:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"AloeVera\",\"cropName\":\"Aloe Vera\",\"phRange\":{\"min\":7,\"max\":8.5},\"tdsRange\":{\"min\":1.8,\"max\":2.5},\"flags\":[\"invasive\",\"perennial\",\"toxic\"]}")));
            case Hydroponics_CropType_Anise:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Anise\",\"cropName\":\"Anise\",\"phRange\":{\"min\":5.8,\"max\":6.4},\"tdsRange\":{\"min\":0.9,\"max\":1.4}}")));
            case Hydroponics_CropType_Artichoke:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Artichoke\",\"cropName\":\"Artichoke\",\"phRange\":{\"min\":6.5,\"max\":7.5},\"tdsRange\":{\"min\":0.8,\"max\":1.8},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_Arugula:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Arugula\",\"cropName\":\"Arugula\",\"phRange\":{\"min\":6,\"max\":7.5},\"tdsRange\":{\"min\":0.8,\"max\":1.8}}")));
            case Hydroponics_CropType_Asparagus:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Asparagus\",\"cropName\":\"Asparagus\",\"phRange\":{\"min\":6,\"max\":6.8},\"tdsRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"pruning\"]}")));
            case Hydroponics_CropType_Basil:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Basil\",\"cropName\":\"Basil\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"pruning\"]}")));
            case Hydroponics_CropType_Bean:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Bean\",\"cropName\":\"Bean (common)\",\"phRange\":6,\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"pruning\"]}")));
            case Hydroponics_CropType_BeanBroad:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"BeanBroad\",\"cropName\":\"Bean (broad)\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"pruning\"]}")));
            case Hydroponics_CropType_Beetroot:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Beetroot\",\"cropName\":\"Beetroot\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":0.8,\"max\":5}}")));
            case Hydroponics_CropType_BlackCurrant:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"BlackCurrant\",\"cropName\":\"Black Currant\",\"phRange\":6,\"tdsRange\":{\"min\":1.4,\"max\":1.8}}")));
            case Hydroponics_CropType_Blueberry:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Blueberry\",\"cropName\":\"Blueberry\",\"phRange\":{\"min\":4,\"max\":5},\"tdsRange\":{\"min\":1.8,\"max\":2},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_BokChoi:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"BokChoi\",\"cropName\":\"Bok-choi\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":1.5,\"max\":2.5}}")));
            case Hydroponics_CropType_Broccoli:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Broccoli\",\"cropName\":\"Broccoli\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":2.8,\"max\":3.5}}")));
            case Hydroponics_CropType_BrusselsSprout:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"BrusselsSprout\",\"cropName\":\"Brussell Sprouts\",\"phRange\":{\"min\":6.5,\"max\":7.5},\"tdsRange\":{\"min\":2.5,\"max\":3}}")));
            case Hydroponics_CropType_Cabbage:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Cabbage\",\"cropName\":\"Cabbage\",\"phRange\":{\"min\":6.5,\"max\":7},\"tdsRange\":{\"min\":2.5,\"max\":3}}")));
            case Hydroponics_CropType_Cannabis:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Cannabis\",\"cropName\":\"Cannabis (generic)\",\"phRange\":{\"min\":5.5,\"max\":6.1},\"tdsRange\":{\"min\":1,\"max\":2.5},\"flags\":[\"large\"]}")));
            case Hydroponics_CropType_Capsicum:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Capsicum\",\"cropName\":\"Capsicum\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.2}}")));
            case Hydroponics_CropType_Carrots:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Carrots\",\"cropName\":\"Carrots\",\"phRange\":6.3,\"tdsRange\":{\"min\":1.6,\"max\":2}}")));
            case Hydroponics_CropType_Catnip:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Catnip\",\"cropName\":\"Catnip\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.6}}")));
            case Hydroponics_CropType_Cauliflower:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Cauliflower\",\"cropName\":\"Cauliflower\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":0.5,\"max\":2}}")));
            case Hydroponics_CropType_Celery:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Celery\",\"cropName\":\"Celery\",\"phRange\":{\"min\":6.3,\"max\":6.7},\"tdsRange\":{\"min\":1.8,\"max\":2.4}}")));
            case Hydroponics_CropType_Chamomile:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Chamomile\",\"cropName\":\"Chamomile\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"toxic\"]}")));
            case Hydroponics_CropType_Chicory:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Chicory\",\"cropName\":\"Chicory\",\"phRange\":{\"min\":5.5,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.4}}")));
            case Hydroponics_CropType_Chives:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Chives\",\"cropName\":\"Chives\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"perennial\",\"toxic\"]}")));
            case Hydroponics_CropType_Cilantro:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Cilantro\",\"cropName\":\"Cilantro\",\"phRange\":{\"min\":6.5,\"max\":6.7},\"tdsRange\":{\"min\":1.3,\"max\":1.8}}")));
            case Hydroponics_CropType_Coriander:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Coriander\",\"cropName\":\"Coriander\",\"phRange\":{\"min\":5.8,\"max\":6.4},\"tdsRange\":{\"min\":1.2,\"max\":1.8}}")));
            case Hydroponics_CropType_CornSweet:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"CornSweet\",\"cropName\":\"Corn (sweet)\",\"phRange\":6,\"tdsRange\":{\"min\":1.6,\"max\":2.4},\"flags\":[\"large\",\"toxic\"]}")));
            case Hydroponics_CropType_Cucumber:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Cucumber\",\"cropName\":\"Cucumber\",\"phRange\":{\"min\":5.8,\"max\":6},\"tdsRange\":{\"min\":1.7,\"max\":2.5},\"flags\":[\"pruning\"]}")));
            case Hydroponics_CropType_Dill:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Dill\",\"cropName\":\"Dill\",\"phRange\":{\"min\":5.5,\"max\":6.4},\"tdsRange\":{\"min\":1,\"max\":1.6}}")));
            case Hydroponics_CropType_Eggplant:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Eggplant\",\"cropName\":\"Eggplant\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":2.5,\"max\":3.5},\"flags\":[\"pruning\"]}")));
            case Hydroponics_CropType_Endive:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Endive\",\"cropName\":\"Endive\",\"phRange\":5.5,\"tdsRange\":{\"min\":2,\"max\":2.4}}")));
            case Hydroponics_CropType_Fennel:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Fennel\",\"cropName\":\"Fennel\",\"phRange\":{\"min\":6.4,\"max\":6.8},\"tdsRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_Fodder:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Fodder\",\"cropName\":\"Fodder\",\"phRange\":6,\"tdsRange\":{\"min\":1.8,\"max\":2}}")));
            case Hydroponics_CropType_Flowers:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Flowers\",\"cropName\":\"Flowers (generic)\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1.5,\"max\":2.5},\"flags\":[\"pruning\",\"toxic\"]}")));
            case Hydroponics_CropType_Garlic:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Garlic\",\"cropName\":\"Garlic\",\"phRange\":6,\"tdsRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}")));
            case Hydroponics_CropType_Ginger:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Ginger\",\"cropName\":\"Ginger\",\"phRange\":{\"min\":5.8,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.5}}")));
            case Hydroponics_CropType_Kale:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Kale\",\"cropName\":\"Kale\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1.25,\"max\":1.5},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_Lavender:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Lavender\",\"cropName\":\"Lavender\",\"phRange\":{\"min\":6.4,\"max\":6.8},\"tdsRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\",\"toxic\"]}")));
            case Hydroponics_CropType_Leek:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Leek\",\"cropName\":\"Leek\",\"phRange\":{\"min\":6.5,\"max\":7},\"tdsRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"toxic\"]}")));
            case Hydroponics_CropType_LemonBalm:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"LemonBalm\",\"cropName\":\"Lemon Balm\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_Lettuce:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Lettuce\",\"cropName\":\"Lettuce\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":0.8,\"max\":1.2}}")));
            case Hydroponics_CropType_Marrow:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Marrow\",\"cropName\":\"Marrow\",\"phRange\":6,\"tdsRange\":{\"min\":1.8,\"max\":2.4}}")));
            case Hydroponics_CropType_Melon:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Melon\",\"cropName\":\"Melon\",\"phRange\":{\"min\":5.5,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"large\"]}")));
            case Hydroponics_CropType_Mint:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Mint\",\"cropName\":\"Mint\",\"phRange\":{\"min\":5.5,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.4},\"flags\":[\"invasive\",\"perennial\",\"toxic\"]}")));
            case Hydroponics_CropType_MustardCress:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"MustardCress\",\"cropName\":\"Mustard Cress\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.2,\"max\":2.4}}")));
            case Hydroponics_CropType_Okra:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Okra\",\"cropName\":\"Okra\",\"phRange\":6.5,\"tdsRange\":{\"min\":2,\"max\":2.4}}")));
            case Hydroponics_CropType_Onions:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Onions\",\"cropName\":\"Onions\",\"phRange\":{\"min\":6,\"max\":6.7},\"tdsRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}")));
            case Hydroponics_CropType_Oregano:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Oregano\",\"cropName\":\"Oregano\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":1.8,\"max\":2.3},\"flags\":[\"perennial\",\"toxic\"]}")));
            case Hydroponics_CropType_PakChoi:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"PakChoi\",\"cropName\":\"Pak-choi\",\"phRange\":7,\"tdsRange\":{\"min\":1.5,\"max\":2}}")));
            case Hydroponics_CropType_Parsley:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Parsley\",\"cropName\":\"Parsley\",\"phRange\":{\"min\":5.5,\"max\":6},\"tdsRange\":{\"min\":0.8,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}")));
            case Hydroponics_CropType_Parsnip:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Parsnip\",\"cropName\":\"Parsnip\",\"phRange\":6,\"tdsRange\":{\"min\":1.4,\"max\":1.8}}")));
            case Hydroponics_CropType_Pea:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Pea\",\"cropName\":\"Pea (common)\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":0.8,\"max\":1.8}}")));
            case Hydroponics_CropType_PeaSugar:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"PeaSugar\",\"cropName\":\"Pea (sugar)\",\"phRange\":{\"min\":6,\"max\":6.8},\"tdsRange\":{\"min\":0.8,\"max\":1.9},\"flags\":[\"toxic\"]}")));
            case Hydroponics_CropType_Pepino:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Pepino\",\"cropName\":\"Pepino\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":2,\"max\":5}}")));
            case Hydroponics_CropType_PeppersBell:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"PeppersBell\",\"cropName\":\"Peppers (bell)\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"pruning\"]}")));
            case Hydroponics_CropType_PeppersHot:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"PeppersHot\",\"cropName\":\"Peppers (hot)\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":2,\"max\":3.5},\"flags\":[\"pruning\"]}")));
            case Hydroponics_CropType_Potato:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Potato\",\"cropName\":\"Potato (common)\",\"phRange\":{\"min\":5,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_PotatoSweet:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"PotatoSweet\",\"cropName\":\"Potato (sweet)\",\"phRange\":{\"min\":5,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_Pumpkin:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Pumpkin\",\"cropName\":\"Pumpkin\",\"phRange\":{\"min\":5.5,\"max\":7.5},\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\",\"pruning\"]}")));
            case Hydroponics_CropType_Radish:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Radish\",\"cropName\":\"Radish\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":1.6,\"max\":2.2}}")));
            case Hydroponics_CropType_Rhubarb:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Rhubarb\",\"cropName\":\"Rhubarb\",\"phRange\":{\"min\":5,\"max\":6},\"tdsRange\":{\"min\":1.6,\"max\":2},\"flags\":[\"perennial\",\"toxic\"]}")));
            case Hydroponics_CropType_Rosemary:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Rosemary\",\"cropName\":\"Rosemary\",\"phRange\":{\"min\":5.5,\"max\":6},\"tdsRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_Sage:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Sage\",\"cropName\":\"Sage\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_Silverbeet:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Silverbeet\",\"cropName\":\"Silverbeet\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":1.8,\"max\":2.3}}")));
            case Hydroponics_CropType_Spinach:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Spinach\",\"cropName\":\"Spinach\",\"phRange\":{\"min\":5.5,\"max\":6.6},\"tdsRange\":{\"min\":1.8,\"max\":2.3}}")));
            case Hydroponics_CropType_Squash:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Squash\",\"cropName\":\"Squash\",\"phRange\":{\"min\":5,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\",\"pruning\"]}")));
            case Hydroponics_CropType_Sunflower:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Sunflower\",\"cropName\":\"Sunflower\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1.2,\"max\":1.8}}")));
            case Hydroponics_CropType_Strawberries:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Strawberries\",\"cropName\":\"Strawberries\",\"phRange\":{\"min\":5,\"max\":5.5},\"tdsRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_SwissChard:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"SwissChard\",\"cropName\":\"Swiss Chard\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.3}}")));
            case Hydroponics_CropType_Taro:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Taro\",\"cropName\":\"Taro\",\"phRange\":{\"min\":5,\"max\":5.5},\"tdsRange\":{\"min\":2.5,\"max\":3},\"flags\":[\"toxic\"]}")));
            case Hydroponics_CropType_Tarragon:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Tarragon\",\"cropName\":\"Tarragon\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.8},\"flags\":[\"toxic\"]}")));
            case Hydroponics_CropType_Thyme:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Thyme\",\"cropName\":\"Thyme\",\"phRange\":{\"min\":5,\"max\":7},\"tdsRange\":{\"min\":0.8,\"max\":1.6},\"flags\":[\"perennial\"]}")));
            case Hydroponics_CropType_Tomato:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Tomato\",\"cropName\":\"Tomato\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":2,\"max\":5},\"flags\":[\"pruning\",\"toxic\"]}")));
            case Hydroponics_CropType_Turnip:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Turnip\",\"cropName\":\"Turnip\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.4}}")));
            case Hydroponics_CropType_Watercress:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Watercress\",\"cropName\":\"Watercress\",\"phRange\":{\"min\":6.5,\"max\":6.8},\"tdsRange\":{\"min\":0.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}")));
            case Hydroponics_CropType_Watermelon:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Watermelon\",\"cropName\":\"Watermelon\",\"phRange\":5.8,\"tdsRange\":{\"min\":1.5,\"max\":2.4},\"flags\":[\"large\"]}")));
            case Hydroponics_CropType_Zucchini:
                return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"cropType\":\"Zucchini\",\"cropName\":\"Zucchini\",\"phRange\":6,\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\"]}")));
            default: break;
        }
    #endif // /ifndef HYDRUINO_DISABLE_BUILT_IN_CROPS_LIBRARY
    return nullptr;
}
