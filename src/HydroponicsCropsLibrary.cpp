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
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"AloeVera\",\"cropName\":\"Aloe Vera\",\"phRange\":\"7,8.5\",\"tdsRange\":\"1.8,2.5\",\"flags\":\"invasive,perennial,toxic\"}")));
             case Hydroponics_CropType_Anise:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Anise\",\"cropName\":\"Anise\",\"phRange\":\"5.8,6.4\",\"tdsRange\":\"0.9,1.4\"}")));
             case Hydroponics_CropType_Artichoke:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Artichoke\",\"cropName\":\"Artichoke\",\"phRange\":\"6.5,7.5\",\"tdsRange\":\"0.8,1.8\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_Arugula:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Arugula\",\"cropName\":\"Arugula\",\"phRange\":\"6,7.5\",\"tdsRange\":\"0.8,1.8\"}")));
             case Hydroponics_CropType_Asparagus:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Asparagus\",\"cropName\":\"Asparagus\",\"phRange\":\"6,6.8\",\"tdsRange\":\"1.4,1.8\",\"flags\":\"perennial,pruning\"}")));
             case Hydroponics_CropType_Basil:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Basil\",\"cropName\":\"Basil\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"flags\":\"pruning\"}")));
             case Hydroponics_CropType_Bean:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Bean\",\"cropName\":\"Bean (common)\",\"flags\":\"pruning\"}")));
             case Hydroponics_CropType_BeanBroad:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"BeanBroad\",\"cropName\":\"Bean (broad)\",\"phRange\":\"6,6.5\",\"flags\":\"pruning\"}")));
             case Hydroponics_CropType_Beetroot:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Beetroot\",\"cropName\":\"Beetroot\",\"phRange\":\"6,6.5\",\"tdsRange\":\"0.8,5\"}")));
             case Hydroponics_CropType_BlackCurrant:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"BlackCurrant\",\"cropName\":\"Black Currant\",\"tdsRange\":\"1.4,1.8\"}")));
             case Hydroponics_CropType_Blueberry:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Blueberry\",\"cropName\":\"Blueberry\",\"phRange\":\"4,5\",\"tdsRange\":\"1.8,2\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_BokChoi:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"BokChoi\",\"cropName\":\"Bok-choi\",\"phRange\":\"6,7\",\"tdsRange\":\"1.5,2.5\"}")));
             case Hydroponics_CropType_Broccoli:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Broccoli\",\"cropName\":\"Broccoli\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2.8,3.5\"}")));
             case Hydroponics_CropType_BrusselsSprout:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"BrusselsSprout\",\"cropName\":\"Brussell Sprouts\",\"phRange\":\"6.5,7.5\",\"tdsRange\":\"2.5,3\"}")));
             case Hydroponics_CropType_Cabbage:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Cabbage\",\"cropName\":\"Cabbage\",\"phRange\":\"6.5,7\",\"tdsRange\":\"2.5,3\"}")));
             case Hydroponics_CropType_Cannabis:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Cannabis\",\"cropName\":\"Cannabis (generic)\",\"phRange\":\"5.5,6.1\",\"tdsRange\":\"1,2.5\",\"flags\":\"large\"}")));
             case Hydroponics_CropType_Capsicum:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Capsicum\",\"cropName\":\"Capsicum\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.8,2.2\"}")));
             case Hydroponics_CropType_Carrots:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Carrots\",\"cropName\":\"Carrots\",\"phRange\":6.3,\"tdsRange\":\"1.6,2\"}")));
             case Hydroponics_CropType_Catnip:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Catnip\",\"cropName\":\"Catnip\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\"}")));
             case Hydroponics_CropType_Cauliflower:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Cauliflower\",\"cropName\":\"Cauliflower\",\"phRange\":\"6,7\",\"tdsRange\":\"0.5,2\"}")));
             case Hydroponics_CropType_Celery:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Celery\",\"cropName\":\"Celery\",\"phRange\":\"6.3,6.7\"}")));
             case Hydroponics_CropType_Chamomile:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Chamomile\",\"cropName\":\"Chamomile\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"flags\":\"toxic\"}")));
             case Hydroponics_CropType_Chicory:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Chicory\",\"cropName\":\"Chicory\",\"phRange\":\"5.5,6\",\"tdsRange\":\"2,2.4\"}")));
             case Hydroponics_CropType_Chives:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Chives\",\"cropName\":\"Chives\",\"phRange\":\"6,6.5\",\"flags\":\"perennial,toxic\"}")));
             case Hydroponics_CropType_Cilantro:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Cilantro\",\"cropName\":\"Cilantro\",\"phRange\":\"6.5,6.7\",\"tdsRange\":\"1.3,1.8\"}")));
             case Hydroponics_CropType_Coriander:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Coriander\",\"cropName\":\"Coriander\",\"phRange\":\"5.8,6.4\",\"tdsRange\":\"1.2,1.8\"}")));
             case Hydroponics_CropType_CornSweet:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"CornSweet\",\"cropName\":\"Corn (sweet)\",\"tdsRange\":\"1.6,2.4\",\"flags\":\"large,toxic\"}")));
             case Hydroponics_CropType_Cucumber:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Cucumber\",\"cropName\":\"Cucumber\",\"phRange\":\"5.8,6\",\"tdsRange\":\"1.7,2.5\",\"flags\":\"pruning\"}")));
             case Hydroponics_CropType_Dill:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Dill\",\"cropName\":\"Dill\",\"phRange\":\"5.5,6.4\",\"tdsRange\":\"1,1.6\"}")));
             case Hydroponics_CropType_Eggplant:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Eggplant\",\"cropName\":\"Eggplant\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"2.5,3.5\",\"flags\":\"pruning\"}")));
             case Hydroponics_CropType_Endive:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Endive\",\"cropName\":\"Endive\",\"phRange\":5.5,\"tdsRange\":\"2,2.4\"}")));
             case Hydroponics_CropType_Fennel:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Fennel\",\"cropName\":\"Fennel\",\"phRange\":\"6.4,6.8\",\"tdsRange\":\"1,1.4\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_Fodder:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Fodder\",\"cropName\":\"Fodder\",\"tdsRange\":\"1.8,2\"}")));
             case Hydroponics_CropType_Flowers:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Flowers\",\"cropName\":\"Flowers (generic)\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1.5,2.5\",\"flags\":\"toxic,pruning\"}")));
             case Hydroponics_CropType_Garlic:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Garlic\",\"cropName\":\"Garlic\",\"tdsRange\":\"1.4,1.8\",\"flags\":\"perennial,toxic\"}")));
             case Hydroponics_CropType_Ginger:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Ginger\",\"cropName\":\"Ginger\",\"phRange\":\"5.8,6\",\"tdsRange\":\"2,2.5\"}")));
             case Hydroponics_CropType_Kale:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Kale\",\"cropName\":\"Kale\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1.25,1.5\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_Lavender:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Lavender\",\"cropName\":\"Lavender\",\"phRange\":\"6.4,6.8\",\"tdsRange\":\"1,1.4\",\"flags\":\"perennial,toxic\"}")));
             case Hydroponics_CropType_Leek:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Leek\",\"cropName\":\"Leek\",\"phRange\":\"6.5,7\",\"tdsRange\":\"1.4,1.8\",\"flags\":\"toxic\"}")));
             case Hydroponics_CropType_LemonBalm:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"LemonBalm\",\"cropName\":\"Lemon Balm\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_Lettuce:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Lettuce\",\"cropName\":\"Lettuce\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"0.8,1.2\"}")));
             case Hydroponics_CropType_Marrow:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Marrow\",\"cropName\":\"Marrow\"}")));
             case Hydroponics_CropType_Melon:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Melon\",\"cropName\":\"Melon\",\"phRange\":\"5.5,6\",\"tdsRange\":\"2,2.5\",\"flags\":\"large\"}")));
             case Hydroponics_CropType_Mint:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Mint\",\"cropName\":\"Mint\",\"phRange\":\"5.5,6\",\"tdsRange\":\"2,2.4\",\"flags\":\"invasive,perennial,toxic\"}")));
             case Hydroponics_CropType_MustardCress:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"MustardCress\",\"cropName\":\"Mustard Cress\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.2,2.4\"}")));
             case Hydroponics_CropType_Okra:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Okra\",\"cropName\":\"Okra\",\"phRange\":6.5,\"tdsRange\":\"2,2.4\"}")));
             case Hydroponics_CropType_Onions:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Onions\",\"cropName\":\"Onions\",\"phRange\":\"6,6.7\",\"tdsRange\":\"1.4,1.8\",\"flags\":\"perennial,toxic\"}")));
             case Hydroponics_CropType_Oregano:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Oregano\",\"cropName\":\"Oregano\",\"phRange\":\"6,7\",\"tdsRange\":\"1.8,2.3\",\"flags\":\"perennial,toxic\"}")));
             case Hydroponics_CropType_PakChoi:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"PakChoi\",\"cropName\":\"Pak-choi\",\"phRange\":7,\"tdsRange\":\"1.5,2\"}")));
             case Hydroponics_CropType_Parsley:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Parsley\",\"cropName\":\"Parsley\",\"phRange\":\"5.5,6\",\"tdsRange\":\"0.8,1.8\",\"flags\":\"perennial,toxic\"}")));
             case Hydroponics_CropType_Parsnip:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Parsnip\",\"cropName\":\"Parsnip\",\"tdsRange\":\"1.4,1.8\"}")));
             case Hydroponics_CropType_Pea:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Pea\",\"cropName\":\"Pea (common)\",\"phRange\":\"6,7\",\"tdsRange\":\"0.8,1.8\"}")));
             case Hydroponics_CropType_PeaSugar:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"PeaSugar\",\"cropName\":\"Pea (sugar)\",\"phRange\":\"6,6.8\",\"tdsRange\":\"0.8,1.9\",\"flags\":\"toxic\"}")));
             case Hydroponics_CropType_Pepino:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Pepino\",\"cropName\":\"Pepino\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2,5\"}")));
             case Hydroponics_CropType_PeppersBell:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"PeppersBell\",\"cropName\":\"Peppers (bell)\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2,2.5\",\"flags\":\"pruning\"}")));
             case Hydroponics_CropType_PeppersHot:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"PeppersHot\",\"cropName\":\"Peppers (hot)\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2,3.5\",\"flags\":\"pruning\"}")));
             case Hydroponics_CropType_Potato:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Potato\",\"cropName\":\"Potato (common)\",\"phRange\":\"5,6\",\"tdsRange\":\"2,2.5\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_PotatoSweet:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"PotatoSweet\",\"cropName\":\"Potato (sweet)\",\"phRange\":\"5,6\",\"tdsRange\":\"2,2.5\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_Pumpkin:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Pumpkin\",\"cropName\":\"Pumpkin\",\"phRange\":\"5.5,7.5\",\"flags\":\"large,pruning\"}")));
             case Hydroponics_CropType_Radish:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Radish\",\"cropName\":\"Radish\",\"phRange\":\"6,7\",\"tdsRange\":\"1.6,2.2\"}")));
             case Hydroponics_CropType_Rhubarb:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Rhubarb\",\"cropName\":\"Rhubarb\",\"phRange\":\"5,6\",\"tdsRange\":\"1.6,2\",\"flags\":\"perennial,toxic\"}")));
             case Hydroponics_CropType_Rosemary:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Rosemary\",\"cropName\":\"Rosemary\",\"phRange\":\"5.5,6\",\"tdsRange\":\"1,1.6\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_Sage:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Sage\",\"cropName\":\"Sage\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_Silverbeet:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Silverbeet\",\"cropName\":\"Silverbeet\",\"phRange\":\"6,7\",\"tdsRange\":\"1.8,2.3\"}")));
             case Hydroponics_CropType_Spinach:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Spinach\",\"cropName\":\"Spinach\",\"phRange\":\"5.5,6.6\",\"tdsRange\":\"1.8,2.3\"}")));
             case Hydroponics_CropType_Squash:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Squash\",\"cropName\":\"Squash\",\"phRange\":\"5,6.5\",\"flags\":\"large,pruning\"}")));
             case Hydroponics_CropType_Sunflower:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Sunflower\",\"cropName\":\"Sunflower\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1.2,1.8\"}")));
             case Hydroponics_CropType_Strawberries:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Strawberries\",\"cropName\":\"Strawberries\",\"phRange\":\"5,5.5\",\"tdsRange\":\"1,1.4\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_SwissChard:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"SwissChard\",\"cropName\":\"Swiss Chard\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.8,2.3\"}")));
             case Hydroponics_CropType_Taro:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Taro\",\"cropName\":\"Taro\",\"phRange\":\"5,5.5\",\"tdsRange\":\"2.5,3\",\"flags\":\"toxic\"}")));
             case Hydroponics_CropType_Tarragon:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Tarragon\",\"cropName\":\"Tarragon\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.8\",\"flags\":\"toxic\"}")));
             case Hydroponics_CropType_Thyme:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Thyme\",\"cropName\":\"Thyme\",\"phRange\":\"5,7\",\"tdsRange\":\"0.8,1.6\",\"flags\":\"perennial\"}")));
             case Hydroponics_CropType_Tomato:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Tomato\",\"cropName\":\"Tomato\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"2,5\",\"flags\":\"toxic,pruning\"}")));
             case Hydroponics_CropType_Turnip:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Turnip\",\"cropName\":\"Turnip\",\"phRange\":\"6,6.5\"}")));
             case Hydroponics_CropType_Watercress:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Watercress\",\"cropName\":\"Watercress\",\"phRange\":\"6.5,6.8\",\"tdsRange\":\"0.4,1.8\",\"flags\":\"perennial,toxic\"}")));
             case Hydroponics_CropType_Watermelon:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Watermelon\",\"cropName\":\"Watermelon\",\"phRange\":5.8,\"tdsRange\":\"1.5,2.4\",\"flags\":\"large\"}")));
             case Hydroponics_CropType_Zucchini:
                 return new HydroponicsCropsLibraryBook(String(F("{\"type\":\"HCLD\",\"id\":\"Zucchini\",\"cropName\":\"Zucchini\",\"flags\":\"large\"}")));
            default: break;
        }
    #endif // /ifndef HYDRUINO_DISABLE_BUILT_IN_CROPS_LIBRARY
    return nullptr;
}
