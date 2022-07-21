/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops Library
*/

#include "Hydroponics.h"

HydroponicsCropsLibraryBook::HydroponicsCropsLibraryBook()
    : data(), count(1)
{ ; }

HydroponicsCropsLibraryBook::HydroponicsCropsLibraryBook(const HydroponicsCropsLibData &dataIn)
    : data(dataIn), count(1)
{ ; }

Hydroponics_CropType HydroponicsCropsLibraryBook::getKey() const
{
    return data.cropType;
}


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

const HydroponicsCropsLibData *HydroponicsCropsLibrary::checkoutCropsData(Hydroponics_CropType cropType)
{
    HYDRUINO_SOFT_ASSERT((int)cropType >= 0 && cropType < Hydroponics_CropType_CustomCropCount, SFP(HS_Err_InvalidParameter));

    HydroponicsCropsLibraryBook *book = nullptr;

    if ((int)cropType >= 0 && cropType < Hydroponics_CropType_CustomCropCount) {
        auto iter = _cropsData.find(cropType);

        if (iter != _cropsData.end()) {
            book = iter->second;
            if (book) {
                book->count += 1;
            }
        } else if (cropType < Hydroponics_CropType_CustomCrop1) { // don't allocate custom
            book = new HydroponicsCropsLibraryBook();

            HYDRUINO_SOFT_ASSERT(book, SFP(HS_Err_AllocationFailure));
            if (book) {
                {   StaticJsonDocument<HYDRUINO_JSON_DOC_DEFSIZE> doc;
                    deserializeJson(doc, jsonStringForCrop(cropType));
                    auto cropsLibDataObj = doc.as<JsonObjectConst>();
                    book->data.fromJSONObject(cropsLibDataObj);
                }
                _cropsData[cropType] = book;
                if (_cropsData.find(cropType) == _cropsData.end()) {
                    delete book; book = nullptr;
                }
            }
        }
    }

    return book ? &(book->data) : nullptr;
}

void HydroponicsCropsLibrary::returnCropsData(const HydroponicsCropsLibData *cropData)
{
    HYDRUINO_SOFT_ASSERT(cropData, SFP(HS_Err_InvalidParameter));

    if (cropData) {
        auto iter = _cropsData.find(cropData->cropType);
        HYDRUINO_SOFT_ASSERT(iter != _cropsData.end(), F("No check outs for crop type"));

        if (iter != _cropsData.end()) {
            auto book = iter->second;
            if (book) {
                book->count -= 1;

                if (book->data.cropType < Hydroponics_CropType_CustomCrop1 && // don't delete custom
                    book->count <= 0) {
                    if (iter->second) { delete iter->second; }
                    _cropsData.erase(iter);
                }
            }
        }
    }
}

bool HydroponicsCropsLibrary::setCustomCropData(const HydroponicsCropsLibData *cropData)
{
    HYDRUINO_SOFT_ASSERT(cropData, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!cropData || (cropData->cropType >= Hydroponics_CropType_CustomCrop1 &&
                                       cropData->cropType < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount), SFP(HS_Err_InvalidParameter));

    if (cropData && cropData->cropType >= Hydroponics_CropType_CustomCrop1 &&
        cropData->cropType < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount) {
        Hydroponics_CropType cropType = cropData->cropType;
        auto iter = _cropsData.find(cropType);
        bool retVal = false;

        if (iter == _cropsData.end()) {
            auto book = new HydroponicsCropsLibraryBook(*cropData);

            HYDRUINO_SOFT_ASSERT(book, SFP(HS_Err_AllocationFailure));
            if (book) {
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
    HYDRUINO_SOFT_ASSERT(cropData, SFP(HS_Err_InvalidParameter));
    HYDRUINO_SOFT_ASSERT(!cropData || (cropData->cropType >= Hydroponics_CropType_CustomCrop1 &&
                         cropData->cropType < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount), SFP(HS_Err_InvalidParameter));

    if (cropData && cropData->cropType >= Hydroponics_CropType_CustomCrop1 &&
        cropData->cropType < Hydroponics_CropType_CustomCrop1 + Hydroponics_CropType_CustomCropCount) {
        Hydroponics_CropType cropType = cropData->cropType;
        auto iter = _cropsData.find(cropType);

        if (iter != _cropsData.end()) {
            if (iter->second) { delete iter->second; }
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

bool HydroponicsCropsLibrary::hasCustomCrops()
{
    return _hasCustomCrops;
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

String HydroponicsCropsLibrary::jsonStringForCrop(Hydroponics_CropType cropType)
{
    switch (cropType) {
        case Hydroponics_CropType_AloeVera:
            return F("{\"type\":\"HCLD\",\"cropType\":\"AloeVera\",\"cropName\":\"Aloe Vera\",\"phRange\":{\"min\":7,\"max\":8.5},\"tdsRange\":{\"min\":1.8,\"max\":2.5},\"flags\":[\"invasive\",\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Anise:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Anise\",\"cropName\":\"Anise\",\"phRange\":{\"min\":5.8,\"max\":6.4},\"tdsRange\":{\"min\":0.9,\"max\":1.4}}");
        case Hydroponics_CropType_Artichoke:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Artichoke\",\"cropName\":\"Artichoke\",\"phRange\":{\"min\":6.5,\"max\":7.5},\"tdsRange\":{\"min\":0.8,\"max\":1.8},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Arugula:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Arugula\",\"cropName\":\"Arugula\",\"phRange\":{\"min\":6,\"max\":7.5},\"tdsRange\":{\"min\":0.8,\"max\":1.8}}");
        case Hydroponics_CropType_Asparagus:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Asparagus\",\"cropName\":\"Asparagus\",\"phRange\":{\"min\":6,\"max\":6.8},\"tdsRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"pruning\"]}");
        case Hydroponics_CropType_Basil:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Basil\",\"cropName\":\"Basil\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Bean:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Bean\",\"cropName\":\"Bean (common)\",\"phRange\":6,\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_BeanBroad:
            return F("{\"type\":\"HCLD\",\"cropType\":\"BeanBroad\",\"cropName\":\"Bean (broad)\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Beetroot:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Beetroot\",\"cropName\":\"Beetroot\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":0.8,\"max\":5}}");
        case Hydroponics_CropType_BlackCurrant:
            return F("{\"type\":\"HCLD\",\"cropType\":\"BlackCurrant\",\"cropName\":\"Black Currant\",\"phRange\":6,\"tdsRange\":{\"min\":1.4,\"max\":1.8}}");
        case Hydroponics_CropType_Blueberry:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Blueberry\",\"cropName\":\"Blueberry\",\"phRange\":{\"min\":4,\"max\":5},\"tdsRange\":{\"min\":1.8,\"max\":2},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_BokChoi:
            return F("{\"type\":\"HCLD\",\"cropType\":\"BokChoi\",\"cropName\":\"Bok-choi\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":1.5,\"max\":2.5}}");
        case Hydroponics_CropType_Broccoli:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Broccoli\",\"cropName\":\"Broccoli\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":2.8,\"max\":3.5}}");
        case Hydroponics_CropType_BrusselsSprout:
            return F("{\"type\":\"HCLD\",\"cropType\":\"BrusselsSprout\",\"cropName\":\"Brussell Sprouts\",\"phRange\":{\"min\":6.5,\"max\":7.5},\"tdsRange\":{\"min\":2.5,\"max\":3}}");
        case Hydroponics_CropType_Cabbage:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Cabbage\",\"cropName\":\"Cabbage\",\"phRange\":{\"min\":6.5,\"max\":7},\"tdsRange\":{\"min\":2.5,\"max\":3}}");
        case Hydroponics_CropType_Cannabis:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Cannabis\",\"cropName\":\"Cannabis (generic)\",\"phRange\":{\"min\":5.5,\"max\":6.1},\"tdsRange\":{\"min\":1,\"max\":2.5},\"flags\":[\"large\"]}");
        case Hydroponics_CropType_Capsicum:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Capsicum\",\"cropName\":\"Capsicum\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.2}}");
        case Hydroponics_CropType_Carrots:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Carrots\",\"cropName\":\"Carrots\",\"phRange\":6.3,\"tdsRange\":{\"min\":1.6,\"max\":2}}");
        case Hydroponics_CropType_Catnip:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Catnip\",\"cropName\":\"Catnip\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.6}}");
        case Hydroponics_CropType_Cauliflower:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Cauliflower\",\"cropName\":\"Cauliflower\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":0.5,\"max\":2}}");
        case Hydroponics_CropType_Celery:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Celery\",\"cropName\":\"Celery\",\"phRange\":{\"min\":6.3,\"max\":6.7},\"tdsRange\":{\"min\":1.8,\"max\":2.4}}");
        case Hydroponics_CropType_Chamomile:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Chamomile\",\"cropName\":\"Chamomile\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Chicory:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Chicory\",\"cropName\":\"Chicory\",\"phRange\":{\"min\":5.5,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.4}}");
        case Hydroponics_CropType_Chives:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Chives\",\"cropName\":\"Chives\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Cilantro:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Cilantro\",\"cropName\":\"Cilantro\",\"phRange\":{\"min\":6.5,\"max\":6.7},\"tdsRange\":{\"min\":1.3,\"max\":1.8}}");
        case Hydroponics_CropType_Coriander:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Coriander\",\"cropName\":\"Coriander\",\"phRange\":{\"min\":5.8,\"max\":6.4},\"tdsRange\":{\"min\":1.2,\"max\":1.8}}");
        case Hydroponics_CropType_CornSweet:
            return F("{\"type\":\"HCLD\",\"cropType\":\"CornSweet\",\"cropName\":\"Corn (sweet)\",\"phRange\":6,\"tdsRange\":{\"min\":1.6,\"max\":2.4},\"flags\":[\"large\",\"toxic\"]}");
        case Hydroponics_CropType_Cucumber:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Cucumber\",\"cropName\":\"Cucumber\",\"phRange\":{\"min\":5.8,\"max\":6},\"tdsRange\":{\"min\":1.7,\"max\":2.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Dill:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Dill\",\"cropName\":\"Dill\",\"phRange\":{\"min\":5.5,\"max\":6.4},\"tdsRange\":{\"min\":1,\"max\":1.6}}");
        case Hydroponics_CropType_Eggplant:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Eggplant\",\"cropName\":\"Eggplant\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":2.5,\"max\":3.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Endive:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Endive\",\"cropName\":\"Endive\",\"phRange\":5.5,\"tdsRange\":{\"min\":2,\"max\":2.4}}");
        case Hydroponics_CropType_Fennel:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Fennel\",\"cropName\":\"Fennel\",\"phRange\":{\"min\":6.4,\"max\":6.8},\"tdsRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Fodder:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Fodder\",\"cropName\":\"Fodder\",\"phRange\":6,\"tdsRange\":{\"min\":1.8,\"max\":2}}");
        case Hydroponics_CropType_Flowers:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Flowers\",\"cropName\":\"Flowers (generic)\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1.5,\"max\":2.5},\"flags\":[\"pruning\",\"toxic\"]}");
        case Hydroponics_CropType_Garlic:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Garlic\",\"cropName\":\"Garlic\",\"phRange\":6,\"tdsRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Ginger:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Ginger\",\"cropName\":\"Ginger\",\"phRange\":{\"min\":5.8,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.5}}");
        case Hydroponics_CropType_Kale:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Kale\",\"cropName\":\"Kale\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1.25,\"max\":1.5},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Lavender:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Lavender\",\"cropName\":\"Lavender\",\"phRange\":{\"min\":6.4,\"max\":6.8},\"tdsRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Leek:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Leek\",\"cropName\":\"Leek\",\"phRange\":{\"min\":6.5,\"max\":7},\"tdsRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_LemonBalm:
            return F("{\"type\":\"HCLD\",\"cropType\":\"LemonBalm\",\"cropName\":\"Lemon Balm\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Lettuce:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Lettuce\",\"cropName\":\"Lettuce\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":0.8,\"max\":1.2}}");
        case Hydroponics_CropType_Marrow:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Marrow\",\"cropName\":\"Marrow\",\"phRange\":6,\"tdsRange\":{\"min\":1.8,\"max\":2.4}}");
        case Hydroponics_CropType_Melon:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Melon\",\"cropName\":\"Melon\",\"phRange\":{\"min\":5.5,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"large\"]}");
        case Hydroponics_CropType_Mint:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Mint\",\"cropName\":\"Mint\",\"phRange\":{\"min\":5.5,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.4},\"flags\":[\"invasive\",\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_MustardCress:
            return F("{\"type\":\"HCLD\",\"cropType\":\"MustardCress\",\"cropName\":\"Mustard Cress\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.2,\"max\":2.4}}");
        case Hydroponics_CropType_Okra:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Okra\",\"cropName\":\"Okra\",\"phRange\":6.5,\"tdsRange\":{\"min\":2,\"max\":2.4}}");
        case Hydroponics_CropType_Onions:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Onions\",\"cropName\":\"Onions\",\"phRange\":{\"min\":6,\"max\":6.7},\"tdsRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Oregano:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Oregano\",\"cropName\":\"Oregano\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":1.8,\"max\":2.3},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_PakChoi:
            return F("{\"type\":\"HCLD\",\"cropType\":\"PakChoi\",\"cropName\":\"Pak-choi\",\"phRange\":7,\"tdsRange\":{\"min\":1.5,\"max\":2}}");
        case Hydroponics_CropType_Parsley:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Parsley\",\"cropName\":\"Parsley\",\"phRange\":{\"min\":5.5,\"max\":6},\"tdsRange\":{\"min\":0.8,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Parsnip:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Parsnip\",\"cropName\":\"Parsnip\",\"phRange\":6,\"tdsRange\":{\"min\":1.4,\"max\":1.8}}");
        case Hydroponics_CropType_Pea:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Pea\",\"cropName\":\"Pea (common)\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":0.8,\"max\":1.8}}");
        case Hydroponics_CropType_PeaSugar:
            return F("{\"type\":\"HCLD\",\"cropType\":\"PeaSugar\",\"cropName\":\"Pea (sugar)\",\"phRange\":{\"min\":6,\"max\":6.8},\"tdsRange\":{\"min\":0.8,\"max\":1.9},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Pepino:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Pepino\",\"cropName\":\"Pepino\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":2,\"max\":5}}");
        case Hydroponics_CropType_PeppersBell:
            return F("{\"type\":\"HCLD\",\"cropType\":\"PeppersBell\",\"cropName\":\"Peppers (bell)\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_PeppersHot:
            return F("{\"type\":\"HCLD\",\"cropType\":\"PeppersHot\",\"cropName\":\"Peppers (hot)\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":2,\"max\":3.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Potato:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Potato\",\"cropName\":\"Potato (common)\",\"phRange\":{\"min\":5,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_PotatoSweet:
            return F("{\"type\":\"HCLD\",\"cropType\":\"PotatoSweet\",\"cropName\":\"Potato (sweet)\",\"phRange\":{\"min\":5,\"max\":6},\"tdsRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Pumpkin:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Pumpkin\",\"cropName\":\"Pumpkin\",\"phRange\":{\"min\":5.5,\"max\":7.5},\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\",\"pruning\"]}");
        case Hydroponics_CropType_Radish:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Radish\",\"cropName\":\"Radish\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":1.6,\"max\":2.2}}");
        case Hydroponics_CropType_Rhubarb:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Rhubarb\",\"cropName\":\"Rhubarb\",\"phRange\":{\"min\":5,\"max\":6},\"tdsRange\":{\"min\":1.6,\"max\":2},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Rosemary:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Rosemary\",\"cropName\":\"Rosemary\",\"phRange\":{\"min\":5.5,\"max\":6},\"tdsRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Sage:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Sage\",\"cropName\":\"Sage\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Silverbeet:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Silverbeet\",\"cropName\":\"Silverbeet\",\"phRange\":{\"min\":6,\"max\":7},\"tdsRange\":{\"min\":1.8,\"max\":2.3}}");
        case Hydroponics_CropType_Spinach:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Spinach\",\"cropName\":\"Spinach\",\"phRange\":{\"min\":5.5,\"max\":6.6},\"tdsRange\":{\"min\":1.8,\"max\":2.3}}");
        case Hydroponics_CropType_Squash:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Squash\",\"cropName\":\"Squash\",\"phRange\":{\"min\":5,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\",\"pruning\"]}");
        case Hydroponics_CropType_Sunflower:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Sunflower\",\"cropName\":\"Sunflower\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1.2,\"max\":1.8}}");
        case Hydroponics_CropType_Strawberries:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Strawberries\",\"cropName\":\"Strawberries\",\"phRange\":{\"min\":5,\"max\":5.5},\"tdsRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_SwissChard:
            return F("{\"type\":\"HCLD\",\"cropType\":\"SwissChard\",\"cropName\":\"Swiss Chard\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.3}}");
        case Hydroponics_CropType_Taro:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Taro\",\"cropName\":\"Taro\",\"phRange\":{\"min\":5,\"max\":5.5},\"tdsRange\":{\"min\":2.5,\"max\":3},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Tarragon:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Tarragon\",\"cropName\":\"Tarragon\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":1,\"max\":1.8},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Thyme:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Thyme\",\"cropName\":\"Thyme\",\"phRange\":{\"min\":5,\"max\":7},\"tdsRange\":{\"min\":0.8,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Tomato:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Tomato\",\"cropName\":\"Tomato\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"tdsRange\":{\"min\":2,\"max\":5},\"flags\":[\"pruning\",\"toxic\"]}");
        case Hydroponics_CropType_Turnip:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Turnip\",\"cropName\":\"Turnip\",\"phRange\":{\"min\":6,\"max\":6.5},\"tdsRange\":{\"min\":1.8,\"max\":2.4}}");
        case Hydroponics_CropType_Watercress:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Watercress\",\"cropName\":\"Watercress\",\"phRange\":{\"min\":6.5,\"max\":6.8},\"tdsRange\":{\"min\":0.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Watermelon:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Watermelon\",\"cropName\":\"Watermelon\",\"phRange\":5.8,\"tdsRange\":{\"min\":1.5,\"max\":2.4},\"flags\":[\"large\"]}");
        case Hydroponics_CropType_Zucchini:
            return F("{\"type\":\"HCLD\",\"cropType\":\"Zucchini\",\"cropName\":\"Zucchini\",\"phRange\":6,\"tdsRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\"]}");

        default:
            return String();
    }
}
