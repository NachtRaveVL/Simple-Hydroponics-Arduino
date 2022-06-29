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
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (!_instance) {
                _instance = new HydroponicsCropsLibrary();
            }
        }
        return _instance;
    }
}

const HydroponicsCropsLibData *HydroponicsCropsLibrary::checkoutCropData(Hydroponics_CropType cropType)
{
    HydroponicsCropsLibraryBook *book = nullptr;
    auto iter = _cropsData.find(cropType);

    if (iter != _cropsData.end()) {
        book = iter->second;

        HYDRUINO_SOFT_ASSERT(book, F("Failure accessing crops lib book"));
        if (book) {
            book->count += 1;
        }
    } else if (cropType < Hydroponics_CropType_Custom1) { // don't allocate custom
        book = new HydroponicsCropsLibraryBook();

        HYDRUINO_SOFT_ASSERT(book, F("Failure allocating new crops lib book"));
        if (book) {
            {   StaticJsonDocument<HYDRUINO_JSON_DOC_MAXSIZE> doc;
                deserializeJson(doc, jsonStringForCrop(cropType));
                auto cropLibDataObj = doc.as<JsonObjectConst>();
                book->data.fromJSONObject(cropLibDataObj);
            }
            _cropsData.insert(cropType, book);
        }
    }

    return book ? &(book->data) : nullptr;
}

void HydroponicsCropsLibrary::returnCropData(const HydroponicsCropsLibData *cropData)
{
    HYDRUINO_SOFT_ASSERT(cropData, F("Invalid crop data"));

    if (cropData) {
        auto iter = _cropsData.find(cropData->cropType);
        HYDRUINO_SOFT_ASSERT(iter != _cropsData.end(), F("No check outs for crop type"));

        if (iter != _cropsData.end()) {
            auto book = iter->second;
            HYDRUINO_SOFT_ASSERT(book, F("Failure accessing crops lib book"));

            if (book) {
                book->count -= 1;

                if (book->data.cropType < Hydroponics_CropType_Custom1 && // don't delete custom
                    book->count <= 0) {
                    _cropsData.erase(cropData->cropType);
                    delete book;
                }
            }
        }
    }
}

bool HydroponicsCropsLibrary::setCustomCropData(const HydroponicsCropsLibData *cropData)
{
    HYDRUINO_SOFT_ASSERT(cropData && cropData->cropType >= (int)Hydroponics_CropType_Custom1, F("Invalid custom crop data"));

    if (cropData && cropData->cropType >= (int)Hydroponics_CropType_Custom1) {
        Hydroponics_CropType cropType = cropData->cropType;
        auto iter = _cropsData.find(cropType);
        bool retVal = false;

        if (iter == _cropsData.end()) {
            auto book = new HydroponicsCropsLibraryBook(*cropData);

            HYDRUINO_SOFT_ASSERT(book, F("Failure allocating new crop lib book"));
            if (book) {
                retVal = _cropsData.insert(cropData->cropType, book).second;
            }
        } else if (&(iter->second->data) != cropData) {
            iter->second->data = *cropData;
            retVal = true;
        }

        if (retVal) {
            _hasCustomCrops = true;
            scheduleSignalFireOnce<Hydroponics_CropType>(_cropDataSignal, cropType);
            return true;
        }
    }
    return false;
}

bool HydroponicsCropsLibrary::dropCustomCropData(const HydroponicsCropsLibData *cropData)
{
    HYDRUINO_SOFT_ASSERT(cropData && cropData->cropType >= (int)Hydroponics_CropType_Custom1, F("Invalid custom crop data"));

    if (cropData && cropData->cropType >= (int)Hydroponics_CropType_Custom1) {
        Hydroponics_CropType cropType = cropData->cropType;
        auto iter = _cropsData.find(cropType);

        if (iter != _cropsData.end()) {
            delete iter->second;
            _cropsData.erase(iter);

            updateHasCustom();
            scheduleSignalFireOnce<Hydroponics_CropType>(_cropDataSignal, cropType);
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
        if (iter->first >= Hydroponics_CropType_Custom1) {
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
            return F("{\"id\":\"HCLD\",\"cropType\":\"AloeVera\",\"cropName\":\"Aloe Vera\",\"phRange\":{\"min\":7,\"max\":8.5},\"ecRange\":{\"min\":1.8,\"max\":2.5},\"flags\":[\"invasive\",\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Anise:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Anise\",\"cropName\":\"Anise\",\"phRange\":{\"min\":5.8,\"max\":6.4},\"ecRange\":{\"min\":0.9,\"max\":1.4}}");
        case Hydroponics_CropType_Artichoke:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Artichoke\",\"cropName\":\"Artichoke\",\"phRange\":{\"min\":6.5,\"max\":7.5},\"ecRange\":{\"min\":0.8,\"max\":1.8},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Arugula:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Arugula\",\"cropName\":\"Arugula\",\"phRange\":{\"min\":6,\"max\":7.5},\"ecRange\":{\"min\":0.8,\"max\":1.8}}");
        case Hydroponics_CropType_Asparagus:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Asparagus\",\"cropName\":\"Asparagus\",\"phRange\":{\"min\":6,\"max\":6.8},\"ecRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"pruning\"]}");
        case Hydroponics_CropType_Basil:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Basil\",\"cropName\":\"Basil\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Bean:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Bean\",\"cropName\":\"Bean (common)\",\"phRange\":6,\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_BeanBroad:
            return F("{\"id\":\"HCLD\",\"cropType\":\"BeanBroad\",\"cropName\":\"Bean (broad)\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Beetroot:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Beetroot\",\"cropName\":\"Beetroot\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":0.8,\"max\":5}}");
        case Hydroponics_CropType_BlackCurrant:
            return F("{\"id\":\"HCLD\",\"cropType\":\"BlackCurrant\",\"cropName\":\"Black Currant\",\"phRange\":6,\"ecRange\":{\"min\":1.4,\"max\":1.8}}");
        case Hydroponics_CropType_Blueberry:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Blueberry\",\"cropName\":\"Blueberry\",\"phRange\":{\"min\":4,\"max\":5},\"ecRange\":{\"min\":1.8,\"max\":2},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_BokChoi:
            return F("{\"id\":\"HCLD\",\"cropType\":\"BokChoi\",\"cropName\":\"Bok-choi\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":1.5,\"max\":2.5}}");
        case Hydroponics_CropType_Broccoli:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Broccoli\",\"cropName\":\"Broccoli\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":2.8,\"max\":3.5}}");
        case Hydroponics_CropType_BrusselsSprout:
            return F("{\"id\":\"HCLD\",\"cropType\":\"BrusselsSprout\",\"cropName\":\"Brussell Sprouts\",\"phRange\":{\"min\":6.5,\"max\":7.5},\"ecRange\":{\"min\":2.5,\"max\":3}}");
        case Hydroponics_CropType_Cabbage:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Cabbage\",\"cropName\":\"Cabbage\",\"phRange\":{\"min\":6.5,\"max\":7},\"ecRange\":{\"min\":2.5,\"max\":3}}");
        case Hydroponics_CropType_Cannabis:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Cannabis\",\"cropName\":\"Cannabis (generic)\",\"phRange\":{\"min\":5.5,\"max\":6.1},\"ecRange\":{\"min\":1,\"max\":2.5},\"flags\":[\"large\"]}");
        case Hydroponics_CropType_Capsicum:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Capsicum\",\"cropName\":\"Capsicum\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.2}}");
        case Hydroponics_CropType_Carrots:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Carrots\",\"cropName\":\"Carrots\",\"phRange\":6.3,\"ecRange\":{\"min\":1.6,\"max\":2}}");
        case Hydroponics_CropType_Catnip:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Catnip\",\"cropName\":\"Catnip\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.6}}");
        case Hydroponics_CropType_Cauliflower:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Cauliflower\",\"cropName\":\"Cauliflower\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":0.5,\"max\":2}}");
        case Hydroponics_CropType_Celery:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Celery\",\"cropName\":\"Celery\",\"phRange\":{\"min\":6.3,\"max\":6.7},\"ecRange\":{\"min\":1.8,\"max\":2.4}}");
        case Hydroponics_CropType_Chamomile:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Chamomile\",\"cropName\":\"Chamomile\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Chicory:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Chicory\",\"cropName\":\"Chicory\",\"phRange\":{\"min\":5.5,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.4}}");
        case Hydroponics_CropType_Chives:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Chives\",\"cropName\":\"Chives\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Cilantro:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Cilantro\",\"cropName\":\"Cilantro\",\"phRange\":{\"min\":6.5,\"max\":6.7},\"ecRange\":{\"min\":1.3,\"max\":1.8}}");
        case Hydroponics_CropType_Coriander:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Coriander\",\"cropName\":\"Coriander\",\"phRange\":{\"min\":5.8,\"max\":6.4},\"ecRange\":{\"min\":1.2,\"max\":1.8}}");
        case Hydroponics_CropType_CornSweet:
            return F("{\"id\":\"HCLD\",\"cropType\":\"CornSweet\",\"cropName\":\"Corn (sweet)\",\"phRange\":6,\"ecRange\":{\"min\":1.6,\"max\":2.4},\"flags\":[\"large\",\"toxic\"]}");
        case Hydroponics_CropType_Cucumber:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Cucumber\",\"cropName\":\"Cucumber\",\"phRange\":{\"min\":5.8,\"max\":6},\"ecRange\":{\"min\":1.7,\"max\":2.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Dill:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Dill\",\"cropName\":\"Dill\",\"phRange\":{\"min\":5.5,\"max\":6.4},\"ecRange\":{\"min\":1,\"max\":1.6}}");
        case Hydroponics_CropType_Eggplant:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Eggplant\",\"cropName\":\"Eggplant\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":2.5,\"max\":3.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Endive:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Endive\",\"cropName\":\"Endive\",\"phRange\":5.5,\"ecRange\":{\"min\":2,\"max\":2.4}}");
        case Hydroponics_CropType_Fennel:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Fennel\",\"cropName\":\"Fennel\",\"phRange\":{\"min\":6.4,\"max\":6.8},\"ecRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Fodder:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Fodder\",\"cropName\":\"Fodder\",\"phRange\":6,\"ecRange\":{\"min\":1.8,\"max\":2}}");
        case Hydroponics_CropType_Flowers:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Flowers\",\"cropName\":\"Flowers (generic)\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1.5,\"max\":2.5},\"flags\":[\"pruning\",\"toxic\"]}");
        case Hydroponics_CropType_Garlic:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Garlic\",\"cropName\":\"Garlic\",\"phRange\":6,\"ecRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Ginger:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Ginger\",\"cropName\":\"Ginger\",\"phRange\":{\"min\":5.8,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.5}}");
        case Hydroponics_CropType_Kale:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Kale\",\"cropName\":\"Kale\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1.25,\"max\":1.5},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Lavender:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Lavender\",\"cropName\":\"Lavender\",\"phRange\":{\"min\":6.4,\"max\":6.8},\"ecRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Leek:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Leek\",\"cropName\":\"Leek\",\"phRange\":{\"min\":6.5,\"max\":7},\"ecRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_LemonBalm:
            return F("{\"id\":\"HCLD\",\"cropType\":\"LemonBalm\",\"cropName\":\"Lemon Balm\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Lettuce:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Lettuce\",\"cropName\":\"Lettuce\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":0.8,\"max\":1.2}}");
        case Hydroponics_CropType_Marrow:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Marrow\",\"cropName\":\"Marrow\",\"phRange\":6,\"ecRange\":{\"min\":1.8,\"max\":2.4}}");
        case Hydroponics_CropType_Melon:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Melon\",\"cropName\":\"Melon\",\"phRange\":{\"min\":5.5,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"large\"]}");
        case Hydroponics_CropType_Mint:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Mint\",\"cropName\":\"Mint\",\"phRange\":{\"min\":5.5,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.4},\"flags\":[\"invasive\",\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_MustardCress:
            return F("{\"id\":\"HCLD\",\"cropType\":\"MustardCress\",\"cropName\":\"Mustard Cress\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.2,\"max\":2.4}}");
        case Hydroponics_CropType_Okra:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Okra\",\"cropName\":\"Okra\",\"phRange\":6.5,\"ecRange\":{\"min\":2,\"max\":2.4}}");
        case Hydroponics_CropType_Onions:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Onions\",\"cropName\":\"Onions\",\"phRange\":{\"min\":6,\"max\":6.7},\"ecRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Oregano:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Oregano\",\"cropName\":\"Oregano\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":1.8,\"max\":2.3},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_PakChoi:
            return F("{\"id\":\"HCLD\",\"cropType\":\"PakChoi\",\"cropName\":\"Pak-choi\",\"phRange\":7,\"ecRange\":{\"min\":1.5,\"max\":2}}");
        case Hydroponics_CropType_Parsley:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Parsley\",\"cropName\":\"Parsley\",\"phRange\":{\"min\":5.5,\"max\":6},\"ecRange\":{\"min\":0.8,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Parsnip:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Parsnip\",\"cropName\":\"Parsnip\",\"phRange\":6,\"ecRange\":{\"min\":1.4,\"max\":1.8}}");
        case Hydroponics_CropType_Pea:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Pea\",\"cropName\":\"Pea (common)\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":0.8,\"max\":1.8}}");
        case Hydroponics_CropType_PeaSugar:
            return F("{\"id\":\"HCLD\",\"cropType\":\"PeaSugar\",\"cropName\":\"Pea (sugar)\",\"phRange\":{\"min\":6,\"max\":6.8},\"ecRange\":{\"min\":0.8,\"max\":1.9},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Pepino:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Pepino\",\"cropName\":\"Pepino\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":2,\"max\":5}}");
        case Hydroponics_CropType_PeppersBell:
            return F("{\"id\":\"HCLD\",\"cropType\":\"PeppersBell\",\"cropName\":\"Peppers (bell)\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_PeppersHot:
            return F("{\"id\":\"HCLD\",\"cropType\":\"PeppersHot\",\"cropName\":\"Peppers (hot)\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":2,\"max\":3.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Potato:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Potato\",\"cropName\":\"Potato (common)\",\"phRange\":{\"min\":5,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_PotatoSweet:
            return F("{\"id\":\"HCLD\",\"cropType\":\"PotatoSweet\",\"cropName\":\"Potato (sweet)\",\"phRange\":{\"min\":5,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Pumpkin:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Pumpkin\",\"cropName\":\"Pumpkin\",\"phRange\":{\"min\":5.5,\"max\":7.5},\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\",\"pruning\"]}");
        case Hydroponics_CropType_Radish:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Radish\",\"cropName\":\"Radish\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":1.6,\"max\":2.2}}");
        case Hydroponics_CropType_Rhubarb:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Rhubarb\",\"cropName\":\"Rhubarb\",\"phRange\":{\"min\":5,\"max\":6},\"ecRange\":{\"min\":1.6,\"max\":2},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Rosemary:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Rosemary\",\"cropName\":\"Rosemary\",\"phRange\":{\"min\":5.5,\"max\":6},\"ecRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Sage:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Sage\",\"cropName\":\"Sage\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Silverbeet:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Silverbeet\",\"cropName\":\"Silverbeet\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":1.8,\"max\":2.3}}");
        case Hydroponics_CropType_Spinach:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Spinach\",\"cropName\":\"Spinach\",\"phRange\":{\"min\":5.5,\"max\":6.6},\"ecRange\":{\"min\":1.8,\"max\":2.3}}");
        case Hydroponics_CropType_Squash:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Squash\",\"cropName\":\"Squash\",\"phRange\":{\"min\":5,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\",\"pruning\"]}");
        case Hydroponics_CropType_Sunflower:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Sunflower\",\"cropName\":\"Sunflower\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1.2,\"max\":1.8}}");
        case Hydroponics_CropType_Strawberries:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Strawberries\",\"cropName\":\"Strawberries\",\"phRange\":{\"min\":5,\"max\":5.5},\"ecRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_SwissChard:
            return F("{\"id\":\"HCLD\",\"cropType\":\"SwissChard\",\"cropName\":\"Swiss Chard\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.3}}");
        case Hydroponics_CropType_Taro:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Taro\",\"cropName\":\"Taro\",\"phRange\":{\"min\":5,\"max\":5.5},\"ecRange\":{\"min\":2.5,\"max\":3},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Tarragon:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Tarragon\",\"cropName\":\"Tarragon\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.8},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Thyme:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Thyme\",\"cropName\":\"Thyme\",\"phRange\":{\"min\":5,\"max\":7},\"ecRange\":{\"min\":0.8,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Tomato:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Tomato\",\"cropName\":\"Tomato\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":2,\"max\":5},\"flags\":[\"pruning\",\"toxic\"]}");
        case Hydroponics_CropType_Turnip:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Turnip\",\"cropName\":\"Turnip\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.4}}");
        case Hydroponics_CropType_Watercress:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Watercress\",\"cropName\":\"Watercress\",\"phRange\":{\"min\":6.5,\"max\":6.8},\"ecRange\":{\"min\":0.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Watermelon:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Watermelon\",\"cropName\":\"Watermelon\",\"phRange\":5.8,\"ecRange\":{\"min\":1.5,\"max\":2.4},\"flags\":[\"large\"]}");
        case Hydroponics_CropType_Zucchini:
            return F("{\"id\":\"HCLD\",\"cropType\":\"Zucchini\",\"cropName\":\"Zucchini\",\"phRange\":6,\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\"]}");

        default:
            return String();
    }
}
