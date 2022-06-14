/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#include "Hydroponics.h"

struct HydroponicsCropsLibraryBook {
    HydroponicsCropsLibraryBook();
    Hydroponics_CropType getKey() const;
    HydroponicsCropLibData data;
    int count;
};

HydroponicsCropsLibraryBook::HydroponicsCropsLibraryBook()
    : data(), count(1)
{ ; }

Hydroponics_CropType HydroponicsCropsLibraryBook::getKey() const
{
    return data.cropType;
}


bool HydroponicsCropsLibrary::_libraryBuilt = false; // To be removed in near future
HydroponicsCropsLibrary *HydroponicsCropsLibrary::_instance = nullptr;

HydroponicsCropsLibrary::HydroponicsCropsLibrary()
{
    buildLibrary();
    _libraryBuilt = true; // To be removed in near future
}

HydroponicsCropsLibrary *HydroponicsCropsLibrary::getInstance()
{
    if (_instance) { return _instance; }
    else {
        // TODO: static lock
        auto *instance = new HydroponicsCropsLibrary();
        if (!_instance) {
            _instance = instance; instance = nullptr;
        } else {
            delete instance; instance = nullptr;
        }
        return _instance;
    }
}

void HydroponicsCropsLibrary::buildLibrary()
{
    {   HydroponicsCropLibData plantData(Hydroponics_CropType_AloeVera);
        strncpy(&plantData.plantName[0], "Aloe Vera", HYDRUINO_NAME_MAXSIZE);
        plantData.phRange[0][0] = 7.0; plantData.phRange[0][1] = 8.5;
        plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.5;
        plantData.isInvasiveOrViner = true;
        plantData.isPerennial = true;
        plantData.isToxicToPets = true;
        _cropLibDataOld[Hydroponics_CropType_AloeVera] = plantData;
    }

    {   HydroponicsCropLibData plantData(Hydroponics_CropType_Anise);
        strncpy(&plantData.plantName[0], "Anise", HYDRUINO_NAME_MAXSIZE);
        plantData.phRange[0][0] = 5.8; plantData.phRange[0][1] = 6.4;
        plantData.ecRange[0][0] = 0.9; plantData.ecRange[0][1] = 1.4;
        _cropLibDataOld[Hydroponics_CropType_Anise] = plantData;
    }

    {   HydroponicsCropLibData plantData(Hydroponics_CropType_Artichoke);
        strncpy(&plantData.plantName[0], "Artichoke", HYDRUINO_NAME_MAXSIZE);
        plantData.phRange[0][0] = 6.5; plantData.phRange[0][1] = 7.5;
        plantData.ecRange[0][0] = 0.8; plantData.ecRange[0][1] = 1.8;
        plantData.isPerennial = true;
        _cropLibDataOld[Hydroponics_CropType_Artichoke] = plantData;
    }

    {   HydroponicsCropLibData plantData(Hydroponics_CropType_Arugula);
        strncpy(&plantData.plantName[0], "Arugula", HYDRUINO_NAME_MAXSIZE);
        plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 7.5;
        plantData.ecRange[0][0] = 0.8; plantData.ecRange[0][1] = 1.8;
        _cropLibDataOld[Hydroponics_CropType_Arugula] = plantData;
    }

    {   HydroponicsCropLibData plantData(Hydroponics_CropType_Asparagus);
        strncpy(&plantData.plantName[0], "Asparagus", HYDRUINO_NAME_MAXSIZE);
        plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.8;
        plantData.ecRange[0][0] = 1.4; plantData.ecRange[0][1] = 1.8;
        plantData.isPerennial = true;
        plantData.isPruningRequired = true;
        _cropLibDataOld[Hydroponics_CropType_Asparagus] = plantData;
    }

    {   HydroponicsCropLibData plantData(Hydroponics_CropType_Basil);
        strncpy(&plantData.plantName[0], "Basil", HYDRUINO_NAME_MAXSIZE);
        plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
        plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.6;
        plantData.isPruningRequired = true;
        _cropLibDataOld[Hydroponics_CropType_Basil] = plantData;
    }

    {   HydroponicsCropLibData plantData(Hydroponics_CropType_Bean);
        strncpy(&plantData.plantName[0], "Bean (common)", HYDRUINO_NAME_MAXSIZE);
        plantData.phRange[0][0] = 6.0;
        plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.4; // alt: 2.0-4.0
        plantData.isPruningRequired = true;
        _cropLibDataOld[Hydroponics_CropType_Bean] = plantData;
    }

    {   HydroponicsCropLibData plantData(Hydroponics_CropType_BeanBroad);
        strncpy(&plantData.plantName[0], "Bean (broad)", HYDRUINO_NAME_MAXSIZE);
        plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
        plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.4; // alt: 1.8-2.2
        plantData.isPruningRequired = true;
        _cropLibDataOld[Hydroponics_CropType_BeanBroad] = plantData;
    }

    {   HydroponicsCropLibData plantData(Hydroponics_CropType_Beetroot);
        strncpy(&plantData.plantName[0], "Beetroot", HYDRUINO_NAME_MAXSIZE);
        plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
        plantData.ecRange[0][0] = 0.8; plantData.ecRange[0][1] = 5.0;
        _cropLibDataOld[Hydroponics_CropType_Beetroot] = plantData;
    }
  
    {   HydroponicsCropLibData plantData(Hydroponics_CropType_BlackCurrant);
        strncpy(&plantData.plantName[0], "Black Currant", HYDRUINO_NAME_MAXSIZE);
        plantData.phRange[0][0] = 6.0;
        plantData.ecRange[0][0] = 1.4; plantData.ecRange[0][1] = 1.8;
        _cropLibDataOld[Hydroponics_CropType_BlackCurrant] = plantData;
    }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Blueberry);
    //     strncpy(&plantData.plantName[0], "Blueberry", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 4.0; plantData.phRange[0][1] = 5.0;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.0;
    //     plantData.isPerennial = true;
    //     _cropLibDataOld[Hydroponics_CropType_Blueberry] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_BokChoi);
    //     strncpy(&plantData.plantName[0], "Bok-choi", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 7.0;
    //     plantData.ecRange[0][0] = 1.5; plantData.ecRange[0][1] = 2.5;
    //     _cropLibDataOld[Hydroponics_CropType_BokChoi] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Broccoli);
    //     strncpy(&plantData.plantName[0], "Broccoli", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 2.8; plantData.ecRange[0][1] = 3.5;
    //     _cropLibDataOld[Hydroponics_CropType_Broccoli] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_BrusselsSprout);
    //     strncpy(&plantData.plantName[0], "Brussell Sprouts", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.5; plantData.phRange[0][1] = 7.5;
    //     plantData.ecRange[0][0] = 2.5; plantData.ecRange[0][1] = 3.0;
    //     _cropLibDataOld[Hydroponics_CropType_BrusselsSprout] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Cabbage);
    //     strncpy(&plantData.plantName[0], "Cabbage", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.5; plantData.phRange[0][1] = 7.0;
    //     plantData.ecRange[0][0] = 2.5; plantData.ecRange[0][1] = 3.0;
    //     _cropLibDataOld[Hydroponics_CropType_Cabbage] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Cannabis);
    //     strncpy(&plantData.plantName[0], "Cannabis (generic)", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.1;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 2.5;
    //     plantData.isLargePlant = true;
    //     _cropLibDataOld[Hydroponics_CropType_Cannabis] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Capsicum);
    //     strncpy(&plantData.plantName[0], "Capsicum", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.2;
    //     _cropLibDataOld[Hydroponics_CropType_Capsicum] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Carrots);
    //     strncpy(&plantData.plantName[0], "Carrots", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.3;
    //     plantData.ecRange[0][0] = 1.6; plantData.ecRange[0][1] = 2.0;
    //     _cropLibDataOld[Hydroponics_CropType_Carrots] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Catnip);
    //     strncpy(&plantData.plantName[0], "Catnip", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.6;
    //     _cropLibDataOld[Hydroponics_CropType_Catnip] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Cauliflower);
    //     strncpy(&plantData.plantName[0], "Cauliflower", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 7.0;
    //     plantData.ecRange[0][0] = 0.5; plantData.ecRange[0][1] = 2.0;
    //     _cropLibDataOld[Hydroponics_CropType_Cauliflower] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Celery);
    //     strncpy(&plantData.plantName[0], "Celery", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.3; plantData.phRange[0][1] = 6.7; // alt: 6.5
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.4;
    //     _cropLibDataOld[Hydroponics_CropType_Celery] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Chamomile);
    //     strncpy(&plantData.plantName[0], "Chamomile", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.6;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Chamomile] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Chicory);
    //     strncpy(&plantData.plantName[0], "Chicory", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.0;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 2.4;
    //     _cropLibDataOld[Hydroponics_CropType_Chicory] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Chives);
    //     strncpy(&plantData.plantName[0], "Chives", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.4;
    //     plantData.isPerennial = true;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Chives] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Cilantro);
    //     strncpy(&plantData.plantName[0], "Cilantro", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.5; plantData.phRange[0][1] = 6.7;
    //     plantData.ecRange[0][0] = 1.3; plantData.ecRange[0][1] = 1.8;
    //     _cropLibDataOld[Hydroponics_CropType_Cilantro] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Coriander);
    //     strncpy(&plantData.plantName[0], "Coriander", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.8; plantData.phRange[0][1] = 6.4;
    //     plantData.ecRange[0][0] = 1.2; plantData.ecRange[0][1] = 1.8;
    //     _cropLibDataOld[Hydroponics_CropType_Coriander] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_CornSweet);
    //     strncpy(&plantData.plantName[0], "Corn (sweet)", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0;
    //     plantData.ecRange[0][0] = 1.6; plantData.ecRange[0][1] = 2.4;
    //     plantData.isLargePlant = true;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_CornSweet] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Cucumber);
    //     strncpy(&plantData.plantName[0], "Cucumber", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.8; plantData.phRange[0][1] = 6.0;
    //     plantData.ecRange[0][0] = 1.7; plantData.ecRange[0][1] = 2.5; // alt: 1.5-3.0
    //     plantData.isPruningRequired = true;
    //     _cropLibDataOld[Hydroponics_CropType_Cucumber] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Dill);
    //     strncpy(&plantData.plantName[0], "Dill", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.4;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.6;
    //     _cropLibDataOld[Hydroponics_CropType_Dill] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Eggplant);
    //     strncpy(&plantData.plantName[0], "Eggplant", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 2.5; plantData.ecRange[0][1] = 3.5;
    //     plantData.isPruningRequired = true;
    //     _cropLibDataOld[Hydroponics_CropType_Eggplant] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Endive);
    //     strncpy(&plantData.plantName[0], "Endive", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 2.4;
    //     _cropLibDataOld[Hydroponics_CropType_Endive] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Fennel);
    //     strncpy(&plantData.plantName[0], "Fennel", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.4; plantData.phRange[0][1] = 6.8;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.4;
    //     plantData.isPerennial = true;
    //     _cropLibDataOld[Hydroponics_CropType_Fennel] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Fodder);
    //     strncpy(&plantData.plantName[0], "Fodder", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.0;
    //     _cropLibDataOld[Hydroponics_CropType_Fodder] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Flowers);
    //     strncpy(&plantData.plantName[0], "Flowers (generic)", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.5; plantData.ecRange[0][1] = 2.5;
    //     plantData.isToxicToPets = true;
    //     plantData.isPruningRequired = true;
    //     _cropLibDataOld[Hydroponics_CropType_Flowers] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Garlic);
    //     strncpy(&plantData.plantName[0], "Garlic", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0;
    //     plantData.ecRange[0][0] = 1.4; plantData.ecRange[0][1] = 1.8;
    //     plantData.isPerennial = true;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Garlic] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Ginger);
    //     strncpy(&plantData.plantName[0], "Ginger", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.8; plantData.phRange[0][1] = 6.0;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 2.5;
    //     _cropLibDataOld[Hydroponics_CropType_Ginger] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Kale);
    //     strncpy(&plantData.plantName[0], "Kale", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.25; plantData.ecRange[0][1] = 1.5;
    //     plantData.isPerennial = true;
    //     _cropLibDataOld[Hydroponics_CropType_Kale] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Lavender);
    //     strncpy(&plantData.plantName[0], "Lavender", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.4; plantData.phRange[0][1] = 6.8;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.4;
    //     plantData.isPerennial = true;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Lavender] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Leek);
    //     strncpy(&plantData.plantName[0], "Leek", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.5; plantData.phRange[0][1] = 7.0;
    //     plantData.ecRange[0][0] = 1.4; plantData.ecRange[0][1] = 1.8;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Leek] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_LemonBalm);
    //     strncpy(&plantData.plantName[0], "Lemon Balm", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.6;
    //     plantData.isPerennial = true;
    //     _cropLibDataOld[Hydroponics_CropType_LemonBalm] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Lettuce);
    //     strncpy(&plantData.plantName[0], "Lettuce", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 0.8; plantData.ecRange[0][1] = 1.2;
    //     _cropLibDataOld[Hydroponics_CropType_Lettuce] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Marrow);
    //     strncpy(&plantData.plantName[0], "Marrow", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.4;
    //     _cropLibDataOld[Hydroponics_CropType_Marrow] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Melon);
    //     strncpy(&plantData.plantName[0], "Melon", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.0;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 2.5;
    //     plantData.isLargePlant = true;
    //     _cropLibDataOld[Hydroponics_CropType_Melon] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Mint);
    //     strncpy(&plantData.plantName[0], "Mint", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.0;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 2.4;
    //     plantData.isPerennial = true;
    //     plantData.isInvasiveOrViner = true;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Mint] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_MustardCress);
    //     strncpy(&plantData.plantName[0], "Mustard Cress", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.2; plantData.ecRange[0][1] = 2.4;
    //     _cropLibDataOld[Hydroponics_CropType_MustardCress] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Okra);
    //     strncpy(&plantData.plantName[0], "Okra", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.5;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 2.4;
    //     _cropLibDataOld[Hydroponics_CropType_Okra] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Onions);
    //     strncpy(&plantData.plantName[0], "Onions", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.7;
    //     plantData.ecRange[0][0] = 1.4; plantData.ecRange[0][1] = 1.8;
    //     plantData.isPerennial = true;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Onions] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Oregano);
    //     strncpy(&plantData.plantName[0], "Oregano", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 7.0;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.3;
    //     plantData.isPerennial = true;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Oregano] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_PakChoi);
    //     strncpy(&plantData.plantName[0], "Pak-choi", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 7.0;
    //     plantData.ecRange[0][0] = 1.5; plantData.ecRange[0][1] = 2.0;
    //     _cropLibDataOld[Hydroponics_CropType_PakChoi] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Parsley);
    //     strncpy(&plantData.plantName[0], "Parsley", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.0;
    //     plantData.ecRange[0][0] = 0.8; plantData.ecRange[0][1] = 1.8;
    //     plantData.isPerennial = true;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Parsley] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Parsnip);
    //     strncpy(&plantData.plantName[0], "Parsnip", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0;
    //     plantData.ecRange[0][0] = 1.4; plantData.ecRange[0][1] = 1.8;
    //     _cropLibDataOld[Hydroponics_CropType_Parsnip] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Pea);
    //     strncpy(&plantData.plantName[0], "Pea (common)", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 7.0;
    //     plantData.ecRange[0][0] = 0.8; plantData.ecRange[0][1] = 1.8;
    //     _cropLibDataOld[Hydroponics_CropType_Pea] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_PeaSugar);
    //     strncpy(&plantData.plantName[0], "Pea (sugar)", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.8; // alt: 6.0-7.0
    //     plantData.ecRange[0][0] = 0.8; plantData.ecRange[0][1] = 1.9; // alt: 0.8-1.8
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_PeaSugar] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Pepino);
    //     strncpy(&plantData.plantName[0], "Pepino", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 5.0;
    //     _cropLibDataOld[Hydroponics_CropType_Pepino] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_PeppersBell);
    //     strncpy(&plantData.plantName[0], "Peppers (bell)", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 2.5; // alt: 2.0-3.0
    //     plantData.isPruningRequired = true;
    //     _cropLibDataOld[Hydroponics_CropType_PeppersBell] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_PeppersHot);
    //     strncpy(&plantData.plantName[0], "Peppers (hot)", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 3.5; // alt: 3.0-3.5
    //     plantData.isPruningRequired = true;
    //     _cropLibDataOld[Hydroponics_CropType_PeppersHot] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Potato);
    //     strncpy(&plantData.plantName[0], "Potato (common)", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.0; plantData.phRange[0][1] = 6.0;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 2.5;
    //     plantData.isPerennial = true;
    //     _cropLibDataOld[Hydroponics_CropType_Potato] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_PotatoSweet);
    //     strncpy(&plantData.plantName[0], "Potato (sweet)", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.0; plantData.phRange[0][1] = 6.0;
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 2.5;
    //     plantData.isPerennial = true;
    //     _cropLibDataOld[Hydroponics_CropType_PotatoSweet] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Pumpkin);
    //     strncpy(&plantData.plantName[0], "Pumpkin", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 7.5; // alt: 5.0-7.5
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.4;
    //     plantData.isLargePlant = true;
    //     plantData.isPruningRequired = true;
    //     _cropLibDataOld[Hydroponics_CropType_Pumpkin] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Radish);
    //     strncpy(&plantData.plantName[0], "Radish", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 7.0;
    //     plantData.ecRange[0][0] = 1.6; plantData.ecRange[0][1] = 2.2;
    //     _cropLibDataOld[Hydroponics_CropType_Radish] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Rhubarb);
    //     strncpy(&plantData.plantName[0], "Rhubarb", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.0; plantData.phRange[0][1] = 6.0;
    //     plantData.ecRange[0][0] = 1.6; plantData.ecRange[0][1] = 2.0;
    //     plantData.isPerennial = true;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Rhubarb] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Rosemary);
    //     strncpy(&plantData.plantName[0], "Rosemary", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.0;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.6;
    //     plantData.isPerennial = true;
    //     _cropLibDataOld[Hydroponics_CropType_Rosemary] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Sage);
    //     strncpy(&plantData.plantName[0], "Sage", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.6;
    //     plantData.isPerennial = true;
    //     _cropLibDataOld[Hydroponics_CropType_Sage] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Silverbeet);
    //     strncpy(&plantData.plantName[0], "Silverbeet", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 7.0;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.3;
    //     _cropLibDataOld[Hydroponics_CropType_Silverbeet] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Spinach);
    //     strncpy(&plantData.plantName[0], "Spinach", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.6; // alt: 6.0-7.0
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.3;
    //     _cropLibDataOld[Hydroponics_CropType_Spinach] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Squash);
    //     strncpy(&plantData.plantName[0], "Squash", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.0; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.4;
    //     plantData.isLargePlant = true;
    //     plantData.isPruningRequired = true;
    //     _cropLibDataOld[Hydroponics_CropType_Squash] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Sunflower);
    //     strncpy(&plantData.plantName[0], "Sunflower", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.2; plantData.ecRange[0][1] = 1.8;
    //     _cropLibDataOld[Hydroponics_CropType_Sunflower] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Strawberries);
    //     strncpy(&plantData.plantName[0], "Strawberries", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.0; plantData.phRange[0][1] = 5.5;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.4;
    //     plantData.isPerennial = true;
    //     _cropLibDataOld[Hydroponics_CropType_Strawberries] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_SwissChard);
    //     strncpy(&plantData.plantName[0], "Swiss Chard", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.3;
    //     _cropLibDataOld[Hydroponics_CropType_SwissChard] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Taro);
    //     strncpy(&plantData.plantName[0], "Taro", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.0; plantData.phRange[0][1] = 5.5;
    //     plantData.ecRange[0][0] = 2.5; plantData.ecRange[0][1] = 3.0;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Taro] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Tarragon);
    //     strncpy(&plantData.plantName[0], "Tarragon", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.0; plantData.ecRange[0][1] = 1.8;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Tarragon] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Thyme);
    //     strncpy(&plantData.plantName[0], "Thyme", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.0; plantData.phRange[0][1] = 7.0;
    //     plantData.ecRange[0][0] = 0.8; plantData.ecRange[0][1] = 1.6;
    //     plantData.isPerennial = true;
    //     _cropLibDataOld[Hydroponics_CropType_Thyme] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Tomato);
    //     strncpy(&plantData.plantName[0], "Tomato", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.5; plantData.phRange[0][1] = 6.5; // alt: 5.5-6.0
    //     plantData.ecRange[0][0] = 2.0; plantData.ecRange[0][1] = 5.0;
    //     plantData.isToxicToPets = true;
    //     plantData.isPruningRequired = true;
    //     _cropLibDataOld[Hydroponics_CropType_Tomato] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Turnip);
    //     strncpy(&plantData.plantName[0], "Turnip", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0; plantData.phRange[0][1] = 6.5;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.4;
    //     _cropLibDataOld[Hydroponics_CropType_Turnip] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Watercress);
    //     strncpy(&plantData.plantName[0], "Watercress", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.5; plantData.phRange[0][1] = 6.8;
    //     plantData.ecRange[0][0] = 0.4; plantData.ecRange[0][1] = 1.8;
    //     plantData.isPerennial = true;
    //     plantData.isToxicToPets = true;
    //     _cropLibDataOld[Hydroponics_CropType_Watercress] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Watermelon);
    //     strncpy(&plantData.plantName[0], "Watermelon", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 5.8;
    //     plantData.ecRange[0][0] = 1.5; plantData.ecRange[0][1] = 2.4;
    //     plantData.isLargePlant = true;
    //     _cropLibDataOld[Hydroponics_CropType_Watermelon] = plantData;
    // }

    // {   HydroponicsCropLibData plantData(Hydroponics_CropType_Zucchini);
    //     strncpy(&plantData.plantName[0], "Zucchini", HYDRUINO_NAME_MAXSIZE);
    //     plantData.phRange[0][0] = 6.0;
    //     plantData.ecRange[0][0] = 1.8; plantData.ecRange[0][1] = 2.4;
    //     plantData.isLargePlant = true;
    //     _cropLibDataOld[Hydroponics_CropType_Zucchini] = plantData;
    // }

    validateEntries();
}

void HydroponicsCropsLibrary::validateEntries()
{
    for (Hydroponics_CropType cropTypeIndex = (Hydroponics_CropType)0; cropTypeIndex < Hydroponics_CropType_Count; cropTypeIndex = (Hydroponics_CropType)((int)cropTypeIndex + 1)) {
        Hydroponics_CropPhase cropPhaseIndex;

        // Ensure phase begin week is increasing
        for(cropPhaseIndex = (Hydroponics_CropPhase)1; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
            if (_cropLibDataOld[cropTypeIndex].phaseBeginWeek[cropPhaseIndex] <= _cropLibDataOld[cropTypeIndex].phaseBeginWeek[cropPhaseIndex-1]) {
                _cropLibDataOld[cropTypeIndex].phaseBeginWeek[cropPhaseIndex] = _cropLibDataOld[cropTypeIndex].phaseBeginWeek[cropPhaseIndex-1] + 1;
            }
        }

        // Check for empty pH/EC second value entries
        for(cropPhaseIndex = (Hydroponics_CropPhase)0; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
            if (_cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][0] > 0 &&
                _cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][1] == 0) {
                _cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][1] = _cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][0];
            }

            if (_cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][0] > 0 &&
                _cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][1] == 0) {
                _cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][1] = _cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][0];
            }
        }

        // Advance previous phase entries if later phase is empty
        for(cropPhaseIndex = (Hydroponics_CropPhase)1; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
            if (_cropLibDataOld[cropTypeIndex].lightHoursPerDay[cropPhaseIndex] == 0 &&
                _cropLibDataOld[cropTypeIndex].lightHoursPerDay[cropPhaseIndex-1] > 0) {
                _cropLibDataOld[cropTypeIndex].lightHoursPerDay[cropPhaseIndex] = _cropLibDataOld[cropTypeIndex].lightHoursPerDay[cropPhaseIndex-1];
            }

            if (_cropLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex][0] == 0 &&
                _cropLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex][1] == 0 &&
                _cropLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][0] > 0 &&
                _cropLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][1] > 0) {
                _cropLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex][0] = _cropLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][0];
                _cropLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex][1] = _cropLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][1];
            }

            if (_cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][0] == 0 &&
                _cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][1] == 0 &&
                _cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex-1][0] > 0 &&
                _cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex-1][1] > 0) {
                _cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][0] = _cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex-1][0];
                _cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][1] = _cropLibDataOld[cropTypeIndex].phRange[cropPhaseIndex-1][1];
            }

            if (_cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][0] == 0 &&
                _cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][1] == 0 &&
                _cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex-1][0] > 0 &&
                _cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex-1][1] > 0) {
                _cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][0] = _cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex-1][0];
                _cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][1] = _cropLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex-1][1];
            }

            if (_cropLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex][0] == 0 &&
                _cropLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex][1] == 0 &&
                _cropLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex-1][0] > 0 &&
                _cropLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex-1][1] > 0) {
                _cropLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex][0] = _cropLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex-1][0];
                _cropLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex][1] = _cropLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex-1][1];
            }

            if (_cropLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex][0] == 0 &&
                _cropLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex][1] == 0 &&
                _cropLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex-1][0] > 0 &&
                _cropLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex-1][1] > 0) {
                _cropLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex][0] = _cropLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex-1][0];
                _cropLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex][1] = _cropLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex-1][1];
            }
        }
    }
}

const HydroponicsCropLibData *HydroponicsCropsLibrary::checkoutCropData(Hydroponics_CropType cropType)
{
    HydroponicsCropsLibraryBook *book = _plantData.getByKey(cropType);

    if (book) {
        book->count += 1;
    } else {
        // TODO: Create from Flash PROGMEM conversion
        HydroponicsCropsLibraryBook tempBook;

        memcpy(&(tempBook.data), &_cropLibDataOld[cropType], sizeof(HydroponicsCropLibData)); // remove after flash move
        //deserializeJson(doc, F("TODO"));
        //book.data.fromJSONDocument(doc);

        if (_plantData.add(tempBook)) {
            book = _plantData.getByKey(cropType);
        }
    }

    return book ? &(book->data) : nullptr;
}

void HydroponicsCropsLibrary::returnCropData(const HydroponicsCropLibData *plantData)
{
    //assert(plantData && "Invalid crop data");
    HydroponicsCropsLibraryBook *book = _plantData.getByKey(plantData->cropType);
    //assert(book && "No matching book for crop type");

    if (book) {
        book->count -= 1;
        if (book->count <= 0) {
            _plantData.removeByKey(book->data.cropType);
        }
    }
}


HydroponicsCrop::HydroponicsCrop(Hydroponics_CropType cropType,
                                 Hydroponics_PositionIndex cropIndex,
                                 Hydroponics_SubstrateType substrateType,
                                 time_t sowDate)
    : HydroponicsObject(HydroponicsIdentity(cropType, cropIndex)),
      _substrateType(substrateType), _sowDate(sowDate),
      _plantData(nullptr), _growWeek(0), _cropPhase(Hydroponics_CropPhase_Undefined)
{
    _plantData = HydroponicsCropsLibrary::getInstance()->checkoutCropData(cropType);
    recalcGrowWeekAndPhase();
}

HydroponicsCrop::~HydroponicsCrop()
{
    if (_plantData) { HydroponicsCropsLibrary::getInstance()->returnCropData(_plantData); _plantData = nullptr; }
}

Hydroponics_CropType HydroponicsCrop::getCropType() const
{
    return _id.as.cropType;
}

Hydroponics_PositionIndex HydroponicsCrop::getCropIndex() const
{
    return _id.posIndex;
}

Hydroponics_SubstrateType HydroponicsCrop::getSubstrateType() const
{
    return _substrateType;
}

time_t HydroponicsCrop::getSowDate() const
{
    return _sowDate;
}

const HydroponicsCropLibData *HydroponicsCrop::getCropData() const
{
    return _plantData;
}

int HydroponicsCrop::getGrowWeek() const
{
    return _growWeek;
}

Hydroponics_CropPhase HydroponicsCrop::getCropPhase() const
{
    return _cropPhase;
}

void HydroponicsCrop::recalcGrowWeekAndPhase()
{
    // TODO
}
