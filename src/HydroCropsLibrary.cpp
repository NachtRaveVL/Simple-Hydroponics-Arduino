/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Crops Library
*/

#include "Hydruino.h"

HydroCropsLibraryBook::HydroCropsLibraryBook()
    : data(), count(1), userSet(false)
{ ; }

HydroCropsLibraryBook::HydroCropsLibraryBook(String jsonStringIn)
    : data(), count(1), userSet(false)
{
    StaticJsonDocument<HYDRO_JSON_DOC_DEFSIZE> doc;
    deserializeJson(doc, jsonStringIn);
    auto cropsLibDataObj = doc.as<JsonObjectConst>();
    data.fromJSONObject(cropsLibDataObj);
}

HydroCropsLibraryBook::HydroCropsLibraryBook(Stream &streamIn, bool jsonFormat)
    : data(), count(1), userSet(false)
{
    if (jsonFormat) {
        StaticJsonDocument<HYDRO_JSON_DOC_DEFSIZE> doc;
        deserializeJson(doc, streamIn);
        auto cropsLibDataObj = doc.as<JsonObjectConst>();
        data.fromJSONObject(cropsLibDataObj);
    } else {
        deserializeDataFromBinaryStream(&data, &streamIn);
    }
}

HydroCropsLibraryBook::HydroCropsLibraryBook(const HydroCropsLibData &dataIn)
    : data(dataIn), count(1), userSet(false)
{ ; }


HydroCropsLibrary hydroCropsLib;

void HydroCropsLibrary::beginCropsLibraryFromSDCard(String dataFilePrefix, bool jsonFormat)
{
    _libSDCropPrefix = dataFilePrefix;
    _libSDJSONFormat = jsonFormat;
}

void HydroCropsLibrary::beginCropsLibraryFromEEPROM(size_t dataAddress, bool jsonFormat)
{
    _libEEPROMDataAddress = dataAddress;
    _libEEPROMJSONFormat = jsonFormat;
}

const HydroCropsLibData *HydroCropsLibrary::checkoutCropsData(Hydro_CropType cropType)
{
    HYDRO_SOFT_ASSERT((int)cropType >= 0 && cropType < Hydro_CropType_Count, SFP(HStr_Err_InvalidParameter));

    HydroCropsLibraryBook *book = nullptr;
    auto iter = _cropsData.find(cropType);

    if (iter != _cropsData.end()) {
        book = iter->second;
        if (book) {
            book->count += 1;
        }
    } else {
        book = newBookFromType(cropType);

        HYDRO_SOFT_ASSERT(book || cropType >= Hydro_CropType_CustomCrop1, SFP(HStr_Err_AllocationFailure));
        if (book) {
            _cropsData[cropType] = book;
            HYDRO_HARD_ASSERT(_cropsData.find(cropType) != _cropsData.end(), SFP(HStr_Err_OperationFailure));
        }
    }

    return book ? &(book->data) : nullptr;
}

void HydroCropsLibrary::returnCropsData(const HydroCropsLibData *cropData)
{
    HYDRO_HARD_ASSERT(cropData, SFP(HStr_Err_InvalidParameter));

    auto iter = _cropsData.find(cropData->cropType);
    HYDRO_SOFT_ASSERT(iter != _cropsData.end(), F("No check outs for crop type"));

    if (iter != _cropsData.end()) {
        auto book = iter->second;
        if (book) {
            book->count--;

            if (book->count <= 0 && (!book->userSet || !book->data.isModified())) {
                delete iter->second;
                _cropsData.erase(iter);
            }
        }
    }
}

bool HydroCropsLibrary::setUserCropData(const HydroCropsLibData *cropData)
{
    HYDRO_HARD_ASSERT(cropData, SFP(HStr_Err_InvalidParameter));

    auto iter = _cropsData.find(cropData->cropType);
    bool retVal = false;

    if (iter == _cropsData.end()) {
        auto book = new HydroCropsLibraryBook(*cropData);
        HYDRO_HARD_ASSERT(book, SFP(HStr_Err_AllocationFailure));

        book->userSet = true;
        _cropsData[cropData->cropType] = book;
        retVal = (_cropsData.find(cropData->cropType) != _cropsData.end());
    } else {
        iter->second->data = *cropData;
        iter->second->userSet = true;
        retVal = true;
    }

    if (retVal) {
        _hasUserCrops = true;
        updateCropsOfType(cropData->cropType);
        return true;
    }
    return false;
}

bool HydroCropsLibrary::dropUserCropData(const HydroCropsLibData *cropData)
{
    HYDRO_HARD_ASSERT(cropData, SFP(HStr_Err_InvalidParameter));

    auto iter = _cropsData.find(cropData->cropType);

    if (iter != _cropsData.end()) {
        delete iter->second;
        _cropsData.erase(iter);

        updateHasUserCrops();
        updateCropsOfType(cropData->cropType);
        return true;
    }

    return false;
}

bool HydroCropsLibrary::updateHasUserCrops()
{
    for (auto iter = _cropsData.begin(); iter != _cropsData.end(); ++iter) {
        if (iter->second->userSet) {
            return (_hasUserCrops = true);
        }
    }
    return (_hasUserCrops = false);
}

void HydroCropsLibrary::updateCropsOfType(Hydro_CropType cropType)
{
    if (Hydruino::_activeInstance) {
        for (auto iter = Hydruino::_activeInstance->_objects.begin(); iter != Hydruino::_activeInstance->_objects.end(); ++iter) {
            if (iter->second->isCropType()) {
                auto crop = static_pointer_cast<HydroCrop>(iter->second);
                if (crop->getCropType() == cropType) {
                    bool incCount = false;
                    if (_cropsData.find(cropType) != _cropsData.end()) {
                        _cropsData[cropType]->count++; // prevents auto-deletion of underlying data
                        incCount = true;
                    }

                    crop->returnCropsLibData(); // forces new data checkout
                    crop->recalcGrowthParams();

                    if (incCount) {
                        _cropsData[cropType]->count--;
                    }
                }
            }
        }
    }
}

HydroCropsLibraryBook *HydroCropsLibrary::newBookFromType(Hydro_CropType cropType)
{
    if (_libSDCropPrefix.length()) {
        HydroCropsLibraryBook *retVal = nullptr;
        auto sd = getController()->getSDCard();

        if (sd) {
            String filename = getNNFilename(_libSDCropPrefix, (unsigned int)cropType, SFP(HStr_dat));

            if (sd->exists(filename.c_str())) {
                auto file = sd->open(filename.c_str(), FILE_READ);
                if (file) {
                    retVal = new HydroCropsLibraryBook(file, _libSDJSONFormat);
                    file.close();
                }
            }

            getController()->endSDCard(sd);
        }

        if (retVal) { return retVal; }
    }

    if (_libEEPROMDataAddress != (size_t)-1) {
        HydroCropsLibraryBook *retVal = nullptr;
        auto eeprom = getController()->getEEPROM();

        if (eeprom) {
            uint16_t lookupOffset = 0;
            eeprom->readBlock(_libEEPROMDataAddress + (((int)cropType + 1) * sizeof(uint16_t)), // +1 for initial total size word
                              (uint8_t *)&lookupOffset, sizeof(uint16_t));

            if (lookupOffset) {
                auto eepromStream = HydroEEPROMStream(lookupOffset, sizeof(HydroCropsLibData));
                retVal = new HydroCropsLibraryBook(eepromStream, _libEEPROMJSONFormat);
            }
        }

        if (retVal) { return retVal; }
    }

    #ifndef HYDRO_DISABLE_BUILTIN_DATA
    {   HydroPROGMEMStream progmemStream(0, 0);
        switch (cropType) {
            case Hydro_CropType_AloeVera: {
                static const char flashStr_AloeVera[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"AloeVera\",\"cropName\":\"Aloe Vera\",\"phRange\":\"7,8.5\",\"tdsRange\":\"1.8,2.5\",\"flags\":\"invasive,perennial,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_AloeVera);
            } break;
            case Hydro_CropType_Anise: {
                static const char flashStr_Anise[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Anise\",\"cropName\":\"Anise\",\"phRange\":\"5.8,6.4\",\"tdsRange\":\"0.9,1.4\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Anise);
            } break;
            case Hydro_CropType_Artichoke: {
                static const char flashStr_Artichoke[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Artichoke\",\"cropName\":\"Artichoke\",\"phRange\":\"6.5,7.5\",\"tdsRange\":\"0.8,1.8\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Artichoke);
            } break;
            case Hydro_CropType_Arugula: {
                static const char flashStr_Arugula[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Arugula\",\"cropName\":\"Arugula\",\"phRange\":\"6,7.5\",\"tdsRange\":\"0.8,1.8\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Arugula);
            } break;
            case Hydro_CropType_Asparagus: {
                static const char flashStr_Asparagus[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Asparagus\",\"cropName\":\"Asparagus\",\"phRange\":\"6,6.8\",\"tdsRange\":\"1.4,1.8\",\"flags\":\"perennial,pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Asparagus);
            } break;
            case Hydro_CropType_Basil: {
                static const char flashStr_Basil[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Basil\",\"cropName\":\"Basil\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"flags\":\"pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Basil);
            } break;
            case Hydro_CropType_Bean: {
                static const char flashStr_Bean[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Bean\",\"cropName\":\"Bean (common)\",\"flags\":\"pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Bean);
            } break;
            case Hydro_CropType_BeanBroad: {
                static const char flashStr_BeanBroad[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"BeanBroad\",\"cropName\":\"Bean (broad)\",\"phRange\":\"6,6.5\",\"flags\":\"pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_BeanBroad);
            } break;
            case Hydro_CropType_Beetroot: {
                static const char flashStr_Beetroot[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Beetroot\",\"cropName\":\"Beetroot\",\"phRange\":\"6,6.5\",\"tdsRange\":\"0.8,5\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Beetroot);
            } break;
            case Hydro_CropType_BlackCurrant: {
                static const char flashStr_BlackCurrant[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"BlackCurrant\",\"cropName\":\"Black Currant\",\"tdsRange\":\"1.4,1.8\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_BlackCurrant);
            } break;
            case Hydro_CropType_Blueberry: {
                static const char flashStr_Blueberry[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Blueberry\",\"cropName\":\"Blueberry\",\"phRange\":\"4,5\",\"tdsRange\":\"1.8,2\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Blueberry);
            } break;
            case Hydro_CropType_BokChoi: {
                static const char flashStr_BokChoi[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"BokChoi\",\"cropName\":\"Bok-choi\",\"phRange\":\"6,7\",\"tdsRange\":\"1.5,2.5\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_BokChoi);
            } break;
            case Hydro_CropType_Broccoli: {
                static const char flashStr_Broccoli[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Broccoli\",\"cropName\":\"Broccoli\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2.8,3.5\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Broccoli);
            } break;
            case Hydro_CropType_BrusselsSprout: {
                static const char flashStr_BrusselsSprout[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"BrusselsSprout\",\"cropName\":\"Brussell Sprouts\",\"phRange\":\"6.5,7.5\",\"tdsRange\":\"2.5,3\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_BrusselsSprout);
            } break;
            case Hydro_CropType_Cabbage: {
                static const char flashStr_Cabbage[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Cabbage\",\"cropName\":\"Cabbage\",\"phRange\":\"6.5,7\",\"tdsRange\":\"2.5,3\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Cabbage);
            } break;
            case Hydro_CropType_Cannabis: {
                static const char flashStr_Cannabis[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Cannabis\",\"cropName\":\"Cannabis (generic)\",\"phRange\":\"5.5,6.1\",\"tdsRange\":\"1,2.5\",\"flags\":\"large\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Cannabis);
            } break;
            case Hydro_CropType_Capsicum: {
                static const char flashStr_Capsicum[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Capsicum\",\"cropName\":\"Capsicum\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.8,2.2\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Capsicum);
            } break;
            case Hydro_CropType_Carrots: {
                static const char flashStr_Carrots[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Carrots\",\"cropName\":\"Carrots\",\"phRange\":6.3,\"tdsRange\":\"1.6,2\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Carrots);
            } break;
            case Hydro_CropType_Catnip: {
                static const char flashStr_Catnip[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Catnip\",\"cropName\":\"Catnip\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Catnip);
            } break;
            case Hydro_CropType_Cauliflower: {
                static const char flashStr_Cauliflower[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Cauliflower\",\"cropName\":\"Cauliflower\",\"phRange\":\"6,7\",\"tdsRange\":\"0.5,2\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Cauliflower);
            } break;
            case Hydro_CropType_Celery: {
                static const char flashStr_Celery[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Celery\",\"cropName\":\"Celery\",\"phRange\":\"6.3,6.7\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Celery);
            } break;
            case Hydro_CropType_Chamomile: {
                static const char flashStr_Chamomile[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Chamomile\",\"cropName\":\"Chamomile\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"flags\":\"toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Chamomile);
            } break;
            case Hydro_CropType_Chicory: {
                static const char flashStr_Chicory[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Chicory\",\"cropName\":\"Chicory\",\"phRange\":\"5.5,6\",\"tdsRange\":\"2,2.4\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Chicory);
            } break;
            case Hydro_CropType_Chives: {
                static const char flashStr_Chives[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Chives\",\"cropName\":\"Chives\",\"phRange\":\"6,6.5\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Chives);
            } break;
            case Hydro_CropType_Cilantro: {
                static const char flashStr_Cilantro[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Cilantro\",\"cropName\":\"Cilantro\",\"phRange\":\"6.5,6.7\",\"tdsRange\":\"1.3,1.8\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Cilantro);
            } break;
            case Hydro_CropType_Coriander: {
                static const char flashStr_Coriander[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Coriander\",\"cropName\":\"Coriander\",\"phRange\":\"5.8,6.4\",\"tdsRange\":\"1.2,1.8\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Coriander);
            } break;
            case Hydro_CropType_CornSweet: {
                static const char flashStr_CornSweet[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"CornSweet\",\"cropName\":\"Corn (sweet)\",\"tdsRange\":\"1.6,2.4\",\"flags\":\"large,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_CornSweet);
            } break;
            case Hydro_CropType_Cucumber: {
                static const char flashStr_Cucumber[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Cucumber\",\"cropName\":\"Cucumber\",\"phRange\":\"5.8,6\",\"tdsRange\":\"1.7,2.5\",\"flags\":\"pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Cucumber);
            } break;
            case Hydro_CropType_Dill: {
                static const char flashStr_Dill[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Dill\",\"cropName\":\"Dill\",\"phRange\":\"5.5,6.4\",\"tdsRange\":\"1,1.6\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Dill);
            } break;
            case Hydro_CropType_Eggplant: {
                static const char flashStr_Eggplant[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Eggplant\",\"cropName\":\"Eggplant\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"2.5,3.5\",\"flags\":\"pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Eggplant);
            } break;
            case Hydro_CropType_Endive: {
                static const char flashStr_Endive[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Endive\",\"cropName\":\"Endive\",\"phRange\":5.5,\"tdsRange\":\"2,2.4\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Endive);
            } break;
            case Hydro_CropType_Fennel: {
                static const char flashStr_Fennel[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Fennel\",\"cropName\":\"Fennel\",\"phRange\":\"6.4,6.8\",\"tdsRange\":\"1,1.4\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Fennel);
            } break;
            case Hydro_CropType_Fodder: {
                static const char flashStr_Fodder[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Fodder\",\"cropName\":\"Fodder\",\"tdsRange\":\"1.8,2\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Fodder);
            } break;
            case Hydro_CropType_Flowers: {
                static const char flashStr_Flowers[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Flowers\",\"cropName\":\"Flowers (generic)\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1.5,2.5\",\"flags\":\"toxic,pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Flowers);
            } break;
            case Hydro_CropType_Garlic: {
                static const char flashStr_Garlic[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Garlic\",\"cropName\":\"Garlic\",\"tdsRange\":\"1.4,1.8\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Garlic);
            } break;
            case Hydro_CropType_Ginger: {
                static const char flashStr_Ginger[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Ginger\",\"cropName\":\"Ginger\",\"phRange\":\"5.8,6\",\"tdsRange\":\"2,2.5\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Ginger);
            } break;
            case Hydro_CropType_Kale: {
                static const char flashStr_Kale[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Kale\",\"cropName\":\"Kale\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1.25,1.5\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Kale);
            } break;
            case Hydro_CropType_Lavender: {
                static const char flashStr_Lavender[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Lavender\",\"cropName\":\"Lavender\",\"phRange\":\"6.4,6.8\",\"tdsRange\":\"1,1.4\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Lavender);
            } break;
            case Hydro_CropType_Leek: {
                static const char flashStr_Leek[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Leek\",\"cropName\":\"Leek\",\"phRange\":\"6.5,7\",\"tdsRange\":\"1.4,1.8\",\"flags\":\"toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Leek);
            } break;
            case Hydro_CropType_LemonBalm: {
                static const char flashStr_LemonBalm[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"LemonBalm\",\"cropName\":\"Lemon Balm\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_LemonBalm);
            } break;
            case Hydro_CropType_Lettuce: {
                static const char flashStr_Lettuce[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Lettuce\",\"cropName\":\"Lettuce\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"0.8,1.2\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Lettuce);
            } break;
            case Hydro_CropType_Marrow: {
                static const char flashStr_Marrow[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Marrow\",\"cropName\":\"Marrow\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Marrow);
            } break;
            case Hydro_CropType_Melon: {
                static const char flashStr_Melon[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Melon\",\"cropName\":\"Melon\",\"phRange\":\"5.5,6\",\"tdsRange\":\"2,2.5\",\"flags\":\"large\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Melon);
            } break;
            case Hydro_CropType_Mint: {
                static const char flashStr_Mint[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Mint\",\"cropName\":\"Mint\",\"phRange\":\"5.5,6\",\"tdsRange\":\"2,2.4\",\"flags\":\"invasive,perennial,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Mint);
            } break;
            case Hydro_CropType_MustardCress: {
                static const char flashStr_MustardCress[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"MustardCress\",\"cropName\":\"Mustard Cress\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.2,2.4\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_MustardCress);
            } break;
            case Hydro_CropType_Okra: {
                static const char flashStr_Okra[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Okra\",\"cropName\":\"Okra\",\"phRange\":6.5,\"tdsRange\":\"2,2.4\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Okra);
            } break;
            case Hydro_CropType_Onions: {
                static const char flashStr_Onions[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Onions\",\"cropName\":\"Onions\",\"phRange\":\"6,6.7\",\"tdsRange\":\"1.4,1.8\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Onions);
            } break;
            case Hydro_CropType_Oregano: {
                static const char flashStr_Oregano[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Oregano\",\"cropName\":\"Oregano\",\"phRange\":\"6,7\",\"tdsRange\":\"1.8,2.3\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Oregano);
            } break;
            case Hydro_CropType_PakChoi: {
                static const char flashStr_PakChoi[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"PakChoi\",\"cropName\":\"Pak-choi\",\"phRange\":7,\"tdsRange\":\"1.5,2\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_PakChoi);
            } break;
            case Hydro_CropType_Parsley: {
                static const char flashStr_Parsley[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Parsley\",\"cropName\":\"Parsley\",\"phRange\":\"5.5,6\",\"tdsRange\":\"0.8,1.8\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Parsley);
            } break;
            case Hydro_CropType_Parsnip: {
                static const char flashStr_Parsnip[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Parsnip\",\"cropName\":\"Parsnip\",\"tdsRange\":\"1.4,1.8\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Parsnip);
            } break;
            case Hydro_CropType_Pea: {
                static const char flashStr_Pea[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Pea\",\"cropName\":\"Pea (common)\",\"phRange\":\"6,7\",\"tdsRange\":\"0.8,1.8\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Pea);
            } break;
            case Hydro_CropType_PeaSugar: {
                static const char flashStr_PeaSugar[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"PeaSugar\",\"cropName\":\"Pea (sugar)\",\"phRange\":\"6,6.8\",\"tdsRange\":\"0.8,1.9\",\"flags\":\"toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_PeaSugar);
            } break;
            case Hydro_CropType_Pepino: {
                static const char flashStr_Pepino[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Pepino\",\"cropName\":\"Pepino\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2,5\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Pepino);
            } break;
            case Hydro_CropType_PeppersBell: {
                static const char flashStr_PeppersBell[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"PeppersBell\",\"cropName\":\"Peppers (bell)\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2,2.5\",\"flags\":\"pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_PeppersBell);
            } break;
            case Hydro_CropType_PeppersHot: {
                static const char flashStr_PeppersHot[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"PeppersHot\",\"cropName\":\"Peppers (hot)\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2,3.5\",\"flags\":\"pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_PeppersHot);
            } break;
            case Hydro_CropType_Potato: {
                static const char flashStr_Potato[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Potato\",\"cropName\":\"Potato (common)\",\"phRange\":\"5,6\",\"tdsRange\":\"2,2.5\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Potato);
            } break;
            case Hydro_CropType_PotatoSweet: {
                static const char flashStr_PotatoSweet[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"PotatoSweet\",\"cropName\":\"Potato (sweet)\",\"phRange\":\"5,6\",\"tdsRange\":\"2,2.5\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_PotatoSweet);
            } break;
            case Hydro_CropType_Pumpkin: {
                static const char flashStr_Pumpkin[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Pumpkin\",\"cropName\":\"Pumpkin\",\"phRange\":\"5.5,7.5\",\"flags\":\"large,pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Pumpkin);
            } break;
            case Hydro_CropType_Radish: {
                static const char flashStr_Radish[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Radish\",\"cropName\":\"Radish\",\"phRange\":\"6,7\",\"tdsRange\":\"1.6,2.2\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Radish);
            } break;
            case Hydro_CropType_Rhubarb: {
                static const char flashStr_Rhubarb[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Rhubarb\",\"cropName\":\"Rhubarb\",\"phRange\":\"5,6\",\"tdsRange\":\"1.6,2\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Rhubarb);
            } break;
            case Hydro_CropType_Rosemary: {
                static const char flashStr_Rosemary[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Rosemary\",\"cropName\":\"Rosemary\",\"phRange\":\"5.5,6\",\"tdsRange\":\"1,1.6\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Rosemary);
            } break;
            case Hydro_CropType_Sage: {
                static const char flashStr_Sage[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Sage\",\"cropName\":\"Sage\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Sage);
            } break;
            case Hydro_CropType_Silverbeet: {
                static const char flashStr_Silverbeet[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Silverbeet\",\"cropName\":\"Silverbeet\",\"phRange\":\"6,7\",\"tdsRange\":\"1.8,2.3\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Silverbeet);
            } break;
            case Hydro_CropType_Spinach: {
                static const char flashStr_Spinach[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Spinach\",\"cropName\":\"Spinach\",\"phRange\":\"5.5,6.6\",\"tdsRange\":\"1.8,2.3\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Spinach);
            } break;
            case Hydro_CropType_Squash: {
                static const char flashStr_Squash[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Squash\",\"cropName\":\"Squash\",\"phRange\":\"5,6.5\",\"flags\":\"large,pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Squash);
            } break;
            case Hydro_CropType_Sunflower: {
                static const char flashStr_Sunflower[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Sunflower\",\"cropName\":\"Sunflower\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1.2,1.8\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Sunflower);
            } break;
            case Hydro_CropType_Strawberries: {
                static const char flashStr_Strawberries[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Strawberries\",\"cropName\":\"Strawberries\",\"phRange\":\"5,5.5\",\"tdsRange\":\"1,1.4\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Strawberries);
            } break;
            case Hydro_CropType_SwissChard: {
                static const char flashStr_SwissChard[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"SwissChard\",\"cropName\":\"Swiss Chard\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.8,2.3\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_SwissChard);
            } break;
            case Hydro_CropType_Taro: {
                static const char flashStr_Taro[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Taro\",\"cropName\":\"Taro\",\"phRange\":\"5,5.5\",\"tdsRange\":\"2.5,3\",\"flags\":\"toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Taro);
            } break;
            case Hydro_CropType_Tarragon: {
                static const char flashStr_Tarragon[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Tarragon\",\"cropName\":\"Tarragon\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.8\",\"flags\":\"toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Tarragon);
            } break;
            case Hydro_CropType_Thyme: {
                static const char flashStr_Thyme[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Thyme\",\"cropName\":\"Thyme\",\"phRange\":\"5,7\",\"tdsRange\":\"0.8,1.6\",\"flags\":\"perennial\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Thyme);
            } break;
            case Hydro_CropType_Tomato: {
                static const char flashStr_Tomato[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Tomato\",\"cropName\":\"Tomato\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"2,5\",\"flags\":\"toxic,pruning\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Tomato);
            } break;
            case Hydro_CropType_Turnip: {
                static const char flashStr_Turnip[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Turnip\",\"cropName\":\"Turnip\",\"phRange\":\"6,6.5\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Turnip);
            } break;
            case Hydro_CropType_Watercress: {
                static const char flashStr_Watercress[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Watercress\",\"cropName\":\"Watercress\",\"phRange\":\"6.5,6.8\",\"tdsRange\":\"0.4,1.8\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Watercress);
            } break;
            case Hydro_CropType_Watermelon: {
                static const char flashStr_Watermelon[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Watermelon\",\"cropName\":\"Watermelon\",\"phRange\":5.8,\"tdsRange\":\"1.5,2.4\",\"flags\":\"large\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Watermelon);
            } break;
            case Hydro_CropType_Zucchini: {
                static const char flashStr_Zucchini[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Zucchini\",\"cropName\":\"Zucchini\",\"flags\":\"large\"}"};
                progmemStream = HydroPROGMEMStream((uintptr_t)flashStr_Zucchini);
            } break;
            default: break;
        }
        if (progmemStream.available()) { return new HydroCropsLibraryBook(progmemStream, true); }
    }
    #endif // /ifndef HYDRO_DISABLE_BUILTIN_DATA
    return nullptr;
}
