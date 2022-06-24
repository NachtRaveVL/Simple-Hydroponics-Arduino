/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops Library
*/

#include "Hydroponics.h"

struct HydroponicsCropsLibraryBook {
    HydroponicsCropsLibraryBook();
    Hydroponics_CropType getKey() const;
    HydroponicsCropsLibData data;
    int count;
};

HydroponicsCropsLibraryBook::HydroponicsCropsLibraryBook()
    : data(), count(1)
{ ; }

Hydroponics_CropType HydroponicsCropsLibraryBook::getKey() const
{
    return data.cropType;
}


HydroponicsCropsLibrary *HydroponicsCropsLibrary::_instance = nullptr;

HydroponicsCropsLibrary::HydroponicsCropsLibrary()
{ ; }

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

void HydroponicsCropsLibrary::validateEntries(HydroponicsCropsLibData *cropsLibData)
{
    Hydroponics_CropPhase cropPhaseIndex;

    // Ensure phase begin week is increasing
    for(cropPhaseIndex = (Hydroponics_CropPhase)1; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
        if (cropsLibData->phaseBeginWeek[cropPhaseIndex] <= cropsLibData->phaseBeginWeek[cropPhaseIndex-1]) {
            cropsLibData->phaseBeginWeek[cropPhaseIndex] = cropsLibData->phaseBeginWeek[cropPhaseIndex-1] + 1;
        }
    }

    // Check for empty pH/EC second value entries
    for(cropPhaseIndex = (Hydroponics_CropPhase)0; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
        if (cropsLibData->phRange[cropPhaseIndex][0] > 0 &&
            cropsLibData->phRange[cropPhaseIndex][1] == 0) {
            cropsLibData->phRange[cropPhaseIndex][1] = cropsLibData->phRange[cropPhaseIndex][0];
        }

        if (cropsLibData->ecRange[cropPhaseIndex][0] > 0 &&
            cropsLibData->ecRange[cropPhaseIndex][1] == 0) {
            cropsLibData->ecRange[cropPhaseIndex][1] = cropsLibData->ecRange[cropPhaseIndex][0];
        }
    }

    // Advance previous phase entries if later phase is empty
    for(cropPhaseIndex = (Hydroponics_CropPhase)1; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
        if (cropsLibData->lightHoursPerDay[cropPhaseIndex] == 0 &&
            cropsLibData->lightHoursPerDay[cropPhaseIndex-1] > 0) {
            cropsLibData->lightHoursPerDay[cropPhaseIndex] = cropsLibData->lightHoursPerDay[cropPhaseIndex-1];
        }

        if (cropsLibData->feedIntervalMins[cropPhaseIndex][0] == 0 &&
            cropsLibData->feedIntervalMins[cropPhaseIndex][1] == 0 &&
            cropsLibData->feedIntervalMins[cropPhaseIndex-1][0] > 0 &&
            cropsLibData->feedIntervalMins[cropPhaseIndex-1][1] > 0) {
            cropsLibData->feedIntervalMins[cropPhaseIndex][0] = cropsLibData->feedIntervalMins[cropPhaseIndex-1][0];
            cropsLibData->feedIntervalMins[cropPhaseIndex][1] = cropsLibData->feedIntervalMins[cropPhaseIndex-1][1];
        }

        if (cropsLibData->phRange[cropPhaseIndex][0] == 0 &&
            cropsLibData->phRange[cropPhaseIndex][1] == 0 &&
            cropsLibData->phRange[cropPhaseIndex-1][0] > 0 &&
            cropsLibData->phRange[cropPhaseIndex-1][1] > 0) {
            cropsLibData->phRange[cropPhaseIndex][0] = cropsLibData->phRange[cropPhaseIndex-1][0];
            cropsLibData->phRange[cropPhaseIndex][1] = cropsLibData->phRange[cropPhaseIndex-1][1];
        }

        if (cropsLibData->ecRange[cropPhaseIndex][0] == 0 &&
            cropsLibData->ecRange[cropPhaseIndex][1] == 0 &&
            cropsLibData->ecRange[cropPhaseIndex-1][0] > 0 &&
            cropsLibData->ecRange[cropPhaseIndex-1][1] > 0) {
            cropsLibData->ecRange[cropPhaseIndex][0] = cropsLibData->ecRange[cropPhaseIndex-1][0];
            cropsLibData->ecRange[cropPhaseIndex][1] = cropsLibData->ecRange[cropPhaseIndex-1][1];
        }

        if (cropsLibData->waterTempRange[cropPhaseIndex][0] == 0 &&
            cropsLibData->waterTempRange[cropPhaseIndex][1] == 0 &&
            cropsLibData->waterTempRange[cropPhaseIndex-1][0] > 0 &&
            cropsLibData->waterTempRange[cropPhaseIndex-1][1] > 0) {
            cropsLibData->waterTempRange[cropPhaseIndex][0] = cropsLibData->waterTempRange[cropPhaseIndex-1][0];
            cropsLibData->waterTempRange[cropPhaseIndex][1] = cropsLibData->waterTempRange[cropPhaseIndex-1][1];
        }

        if (cropsLibData->airTempRange[cropPhaseIndex][0] == 0 &&
            cropsLibData->airTempRange[cropPhaseIndex][1] == 0 &&
            cropsLibData->airTempRange[cropPhaseIndex-1][0] > 0 &&
            cropsLibData->airTempRange[cropPhaseIndex-1][1] > 0) {
            cropsLibData->airTempRange[cropPhaseIndex][0] = cropsLibData->airTempRange[cropPhaseIndex-1][0];
            cropsLibData->airTempRange[cropPhaseIndex][1] = cropsLibData->airTempRange[cropPhaseIndex-1][1];
        }
    }
}

String HydroponicsCropsLibrary::jsonStringForCrop(Hydroponics_CropType cropType)
{
    switch (cropType) {
        case Hydroponics_CropType_AloeVera:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"AloeVera\",\"cropName\":\"Aloe Vera\",\"phRange\":{\"min\":7,\"max\":8.5},\"ecRange\":{\"min\":1.8,\"max\":2.5},\"flags\":[\"invasive\",\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Anise:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Anise\",\"cropName\":\"Anise\",\"phRange\":{\"min\":5.8,\"max\":6.4},\"ecRange\":{\"min\":0.9,\"max\":1.4}}");
        case Hydroponics_CropType_Artichoke:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Artichoke\",\"cropName\":\"Artichoke\",\"phRange\":{\"min\":6.5,\"max\":7.5},\"ecRange\":{\"min\":0.8,\"max\":1.8},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Arugula:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Arugula\",\"cropName\":\"Arugula\",\"phRange\":{\"min\":6,\"max\":7.5},\"ecRange\":{\"min\":0.8,\"max\":1.8}}");
        case Hydroponics_CropType_Asparagus:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Asparagus\",\"cropName\":\"Asparagus\",\"phRange\":{\"min\":6,\"max\":6.8},\"ecRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"pruning\"]}");
        case Hydroponics_CropType_Basil:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Basil\",\"cropName\":\"Basil\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Bean:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Bean\",\"cropName\":\"Bean (common)\",\"phRange\":6,\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_BeanBroad:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"BeanBroad\",\"cropName\":\"Bean (broad)\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Beetroot:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Beetroot\",\"cropName\":\"Beetroot\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":0.8,\"max\":5}}");
        case Hydroponics_CropType_BlackCurrant:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"BlackCurrant\",\"cropName\":\"Black Currant\",\"phRange\":6,\"ecRange\":{\"min\":1.4,\"max\":1.8}}");
        case Hydroponics_CropType_Blueberry:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Blueberry\",\"cropName\":\"Blueberry\",\"phRange\":{\"min\":4,\"max\":5},\"ecRange\":{\"min\":1.8,\"max\":2},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_BokChoi:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"BokChoi\",\"cropName\":\"Bok-choi\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":1.5,\"max\":2.5}}");
        case Hydroponics_CropType_Broccoli:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Broccoli\",\"cropName\":\"Broccoli\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":2.8,\"max\":3.5}}");
        case Hydroponics_CropType_BrusselsSprout:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"BrusselsSprout\",\"cropName\":\"Brussell Sprouts\",\"phRange\":{\"min\":6.5,\"max\":7.5},\"ecRange\":{\"min\":2.5,\"max\":3}}");
        case Hydroponics_CropType_Cabbage:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Cabbage\",\"cropName\":\"Cabbage\",\"phRange\":{\"min\":6.5,\"max\":7},\"ecRange\":{\"min\":2.5,\"max\":3}}");
        case Hydroponics_CropType_Cannabis:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Cannabis\",\"cropName\":\"Cannabis (generic)\",\"phRange\":{\"min\":5.5,\"max\":6.1},\"ecRange\":{\"min\":1,\"max\":2.5},\"flags\":[\"large\"]}");
        case Hydroponics_CropType_Capsicum:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Capsicum\",\"cropName\":\"Capsicum\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.2}}");
        case Hydroponics_CropType_Carrots:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Carrots\",\"cropName\":\"Carrots\",\"phRange\":6.3,\"ecRange\":{\"min\":1.6,\"max\":2}}");
        case Hydroponics_CropType_Catnip:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Catnip\",\"cropName\":\"Catnip\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.6}}");
        case Hydroponics_CropType_Cauliflower:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Cauliflower\",\"cropName\":\"Cauliflower\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":0.5,\"max\":2}}");
        case Hydroponics_CropType_Celery:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Celery\",\"cropName\":\"Celery\",\"phRange\":{\"min\":6.3,\"max\":6.7},\"ecRange\":{\"min\":1.8,\"max\":2.4}}");
        case Hydroponics_CropType_Chamomile:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Chamomile\",\"cropName\":\"Chamomile\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Chicory:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Chicory\",\"cropName\":\"Chicory\",\"phRange\":{\"min\":5.5,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.4}}");
        case Hydroponics_CropType_Chives:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Chives\",\"cropName\":\"Chives\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Cilantro:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Cilantro\",\"cropName\":\"Cilantro\",\"phRange\":{\"min\":6.5,\"max\":6.7},\"ecRange\":{\"min\":1.3,\"max\":1.8}}");
        case Hydroponics_CropType_Coriander:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Coriander\",\"cropName\":\"Coriander\",\"phRange\":{\"min\":5.8,\"max\":6.4},\"ecRange\":{\"min\":1.2,\"max\":1.8}}");
        case Hydroponics_CropType_CornSweet:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"CornSweet\",\"cropName\":\"Corn (sweet)\",\"phRange\":6,\"ecRange\":{\"min\":1.6,\"max\":2.4},\"flags\":[\"large\",\"toxic\"]}");
        case Hydroponics_CropType_Cucumber:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Cucumber\",\"cropName\":\"Cucumber\",\"phRange\":{\"min\":5.8,\"max\":6},\"ecRange\":{\"min\":1.7,\"max\":2.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Dill:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Dill\",\"cropName\":\"Dill\",\"phRange\":{\"min\":5.5,\"max\":6.4},\"ecRange\":{\"min\":1,\"max\":1.6}}");
        case Hydroponics_CropType_Eggplant:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Eggplant\",\"cropName\":\"Eggplant\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":2.5,\"max\":3.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Endive:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Endive\",\"cropName\":\"Endive\",\"phRange\":5.5,\"ecRange\":{\"min\":2,\"max\":2.4}}");
        case Hydroponics_CropType_Fennel:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Fennel\",\"cropName\":\"Fennel\",\"phRange\":{\"min\":6.4,\"max\":6.8},\"ecRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Fodder:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Fodder\",\"cropName\":\"Fodder\",\"phRange\":6,\"ecRange\":{\"min\":1.8,\"max\":2}}");
        case Hydroponics_CropType_Flowers:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Flowers\",\"cropName\":\"Flowers (generic)\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1.5,\"max\":2.5},\"flags\":[\"pruning\",\"toxic\"]}");
        case Hydroponics_CropType_Garlic:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Garlic\",\"cropName\":\"Garlic\",\"phRange\":6,\"ecRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Ginger:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Ginger\",\"cropName\":\"Ginger\",\"phRange\":{\"min\":5.8,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.5}}");
        case Hydroponics_CropType_Kale:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Kale\",\"cropName\":\"Kale\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1.25,\"max\":1.5},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Lavender:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Lavender\",\"cropName\":\"Lavender\",\"phRange\":{\"min\":6.4,\"max\":6.8},\"ecRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Leek:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Leek\",\"cropName\":\"Leek\",\"phRange\":{\"min\":6.5,\"max\":7},\"ecRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_LemonBalm:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"LemonBalm\",\"cropName\":\"Lemon Balm\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Lettuce:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Lettuce\",\"cropName\":\"Lettuce\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":0.8,\"max\":1.2}}");
        case Hydroponics_CropType_Marrow:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Marrow\",\"cropName\":\"Marrow\",\"phRange\":6,\"ecRange\":{\"min\":1.8,\"max\":2.4}}");
        case Hydroponics_CropType_Melon:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Melon\",\"cropName\":\"Melon\",\"phRange\":{\"min\":5.5,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"large\"]}");
        case Hydroponics_CropType_Mint:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Mint\",\"cropName\":\"Mint\",\"phRange\":{\"min\":5.5,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.4},\"flags\":[\"invasive\",\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_MustardCress:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"MustardCress\",\"cropName\":\"Mustard Cress\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.2,\"max\":2.4}}");
        case Hydroponics_CropType_Okra:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Okra\",\"cropName\":\"Okra\",\"phRange\":6.5,\"ecRange\":{\"min\":2,\"max\":2.4}}");
        case Hydroponics_CropType_Onions:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Onions\",\"cropName\":\"Onions\",\"phRange\":{\"min\":6,\"max\":6.7},\"ecRange\":{\"min\":1.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Oregano:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Oregano\",\"cropName\":\"Oregano\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":1.8,\"max\":2.3},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_PakChoi:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"PakChoi\",\"cropName\":\"Pak-choi\",\"phRange\":7,\"ecRange\":{\"min\":1.5,\"max\":2}}");
        case Hydroponics_CropType_Parsley:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Parsley\",\"cropName\":\"Parsley\",\"phRange\":{\"min\":5.5,\"max\":6},\"ecRange\":{\"min\":0.8,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Parsnip:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Parsnip\",\"cropName\":\"Parsnip\",\"phRange\":6,\"ecRange\":{\"min\":1.4,\"max\":1.8}}");
        case Hydroponics_CropType_Pea:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Pea\",\"cropName\":\"Pea (common)\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":0.8,\"max\":1.8}}");
        case Hydroponics_CropType_PeaSugar:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"PeaSugar\",\"cropName\":\"Pea (sugar)\",\"phRange\":{\"min\":6,\"max\":6.8},\"ecRange\":{\"min\":0.8,\"max\":1.9},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Pepino:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Pepino\",\"cropName\":\"Pepino\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":2,\"max\":5}}");
        case Hydroponics_CropType_PeppersBell:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"PeppersBell\",\"cropName\":\"Peppers (bell)\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_PeppersHot:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"PeppersHot\",\"cropName\":\"Peppers (hot)\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":2,\"max\":3.5},\"flags\":[\"pruning\"]}");
        case Hydroponics_CropType_Potato:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Potato\",\"cropName\":\"Potato (common)\",\"phRange\":{\"min\":5,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_PotatoSweet:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"PotatoSweet\",\"cropName\":\"Potato (sweet)\",\"phRange\":{\"min\":5,\"max\":6},\"ecRange\":{\"min\":2,\"max\":2.5},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Pumpkin:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Pumpkin\",\"cropName\":\"Pumpkin\",\"phRange\":{\"min\":5.5,\"max\":7.5},\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\",\"pruning\"]}");
        case Hydroponics_CropType_Radish:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Radish\",\"cropName\":\"Radish\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":1.6,\"max\":2.2}}");
        case Hydroponics_CropType_Rhubarb:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Rhubarb\",\"cropName\":\"Rhubarb\",\"phRange\":{\"min\":5,\"max\":6},\"ecRange\":{\"min\":1.6,\"max\":2},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Rosemary:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Rosemary\",\"cropName\":\"Rosemary\",\"phRange\":{\"min\":5.5,\"max\":6},\"ecRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Sage:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Sage\",\"cropName\":\"Sage\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Silverbeet:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Silverbeet\",\"cropName\":\"Silverbeet\",\"phRange\":{\"min\":6,\"max\":7},\"ecRange\":{\"min\":1.8,\"max\":2.3}}");
        case Hydroponics_CropType_Spinach:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Spinach\",\"cropName\":\"Spinach\",\"phRange\":{\"min\":5.5,\"max\":6.6},\"ecRange\":{\"min\":1.8,\"max\":2.3}}");
        case Hydroponics_CropType_Squash:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Squash\",\"cropName\":\"Squash\",\"phRange\":{\"min\":5,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\",\"pruning\"]}");
        case Hydroponics_CropType_Sunflower:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Sunflower\",\"cropName\":\"Sunflower\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1.2,\"max\":1.8}}");
        case Hydroponics_CropType_Strawberries:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Strawberries\",\"cropName\":\"Strawberries\",\"phRange\":{\"min\":5,\"max\":5.5},\"ecRange\":{\"min\":1,\"max\":1.4},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_SwissChard:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"SwissChard\",\"cropName\":\"Swiss Chard\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.3}}");
        case Hydroponics_CropType_Taro:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Taro\",\"cropName\":\"Taro\",\"phRange\":{\"min\":5,\"max\":5.5},\"ecRange\":{\"min\":2.5,\"max\":3},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Tarragon:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Tarragon\",\"cropName\":\"Tarragon\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":1,\"max\":1.8},\"flags\":[\"toxic\"]}");
        case Hydroponics_CropType_Thyme:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Thyme\",\"cropName\":\"Thyme\",\"phRange\":{\"min\":5,\"max\":7},\"ecRange\":{\"min\":0.8,\"max\":1.6},\"flags\":[\"perennial\"]}");
        case Hydroponics_CropType_Tomato:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Tomato\",\"cropName\":\"Tomato\",\"phRange\":{\"min\":5.5,\"max\":6.5},\"ecRange\":{\"min\":2,\"max\":5},\"flags\":[\"pruning\",\"toxic\"]}");
        case Hydroponics_CropType_Turnip:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Turnip\",\"cropName\":\"Turnip\",\"phRange\":{\"min\":6,\"max\":6.5},\"ecRange\":{\"min\":1.8,\"max\":2.4}}");
        case Hydroponics_CropType_Watercress:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Watercress\",\"cropName\":\"Watercress\",\"phRange\":{\"min\":6.5,\"max\":6.8},\"ecRange\":{\"min\":0.4,\"max\":1.8},\"flags\":[\"perennial\",\"toxic\"]}");
        case Hydroponics_CropType_Watermelon:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Watermelon\",\"cropName\":\"Watermelon\",\"phRange\":5.8,\"ecRange\":{\"min\":1.5,\"max\":2.4},\"flags\":[\"large\"]}");
        case Hydroponics_CropType_Zucchini:
            return F("{\"_ident\":\"HCLD\",\"_version\":1,\"_revision\":1,\"cropType\":\"Zucchini\",\"cropName\":\"Zucchini\",\"phRange\":6,\"ecRange\":{\"min\":1.8,\"max\":2.4},\"flags\":[\"large\"]}");

        case Hydroponics_CropType_Custom1:
        case Hydroponics_CropType_Custom2:
        case Hydroponics_CropType_Custom3:
        case Hydroponics_CropType_Custom4:
        case Hydroponics_CropType_Custom5:
            // TODO
            break;

        default:
            return String();
    }
}

const HydroponicsCropsLibData *HydroponicsCropsLibrary::checkoutCropData(Hydroponics_CropType cropType)
{
    HydroponicsCropsLibraryBook *book = nullptr;
    auto iter = _cropsLibData.find(cropType);

    if (iter != _cropsLibData.end()) {
        book = iter->second;

        HYDRUINO_SOFT_ASSERT(book, F("Failure accessing crops lib book"));
        if (book) {
            book->count += 1;
        }
    } else {
        book = new HydroponicsCropsLibraryBook();

        HYDRUINO_SOFT_ASSERT(book, F("Failure allocating new crops lib book"));
        if (book) {
            {   DynamicJsonDocument doc(128);
                deserializeJson(doc, jsonStringForCrop(cropType));
                auto constVariantRef = doc.as<JsonVariantConst>();
                book->data.fromJSONElement(constVariantRef);
            }
            validateEntries(&(book->data));
            _cropsLibData.insert(cropType, book);
        }
    }

    return book ? &(book->data) : nullptr;
}

void HydroponicsCropsLibrary::returnCropData(const HydroponicsCropsLibData *cropData)
{
    HYDRUINO_SOFT_ASSERT(cropData, F("Invalid crop data"));

    if (cropData) {
        auto iter = _cropsLibData.find(cropData->cropType);
        HYDRUINO_SOFT_ASSERT(iter != _cropsLibData.end(), F("No check outs for crop type"));

        if (iter != _cropsLibData.end()) {
            auto book = iter->second;
            HYDRUINO_SOFT_ASSERT(book, F("Failure accessing crops lib book"));

            if (book) {
                book->count -= 1;

                if (book->count <= 0) {
                    _cropsLibData.erase(cropData->cropType);
                    delete book;
                }
            }
        }
    }
}
