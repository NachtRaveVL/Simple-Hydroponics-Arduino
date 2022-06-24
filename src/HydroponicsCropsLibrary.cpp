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


bool HydroponicsCropsLibrary::_libraryBuilt = false; // To be removed in near future
HydroponicsCropsLibrary *HydroponicsCropsLibrary::_instance = nullptr;

HydroponicsCropsLibrary::HydroponicsCropsLibrary()
{
    buildLibrary();
}

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

void HydroponicsCropsLibrary::buildLibrary()
{
    {   HydroponicsCropsLibData cropData(Hydroponics_CropType_AloeVera);
        strncpy(cropData.cropName, "Aloe Vera", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 7.0; cropData.phRange[0][1] = 8.5;
        cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.5;
        cropData.isInvasiveOrViner = true;
        cropData.isPerennial = true;
        cropData.isToxicToPets = true;
        _cropsLibDataOld[Hydroponics_CropType_AloeVera] = cropData;
    }

    {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Anise);
        strncpy(cropData.cropName, "Anise", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 5.8; cropData.phRange[0][1] = 6.4;
        cropData.ecRange[0][0] = 0.9; cropData.ecRange[0][1] = 1.4;
        _cropsLibDataOld[Hydroponics_CropType_Anise] = cropData;
    }

    {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Artichoke);
        strncpy(cropData.cropName, "Artichoke", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 7.5;
        cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.8;
        cropData.isPerennial = true;
        _cropsLibDataOld[Hydroponics_CropType_Artichoke] = cropData;
    }

    {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Arugula);
        strncpy(cropData.cropName, "Arugula", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.5;
        cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.8;
        _cropsLibDataOld[Hydroponics_CropType_Arugula] = cropData;
    }

    {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Asparagus);
        strncpy(cropData.cropName, "Asparagus", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.8;
        cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
        cropData.isPerennial = true;
        cropData.isPruningRequired = true;
        _cropsLibDataOld[Hydroponics_CropType_Asparagus] = cropData;
    }

    {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Basil);
        strncpy(cropData.cropName, "Basil", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
        cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
        cropData.isPruningRequired = true;
        _cropsLibDataOld[Hydroponics_CropType_Basil] = cropData;
    }

    {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Bean);
        strncpy(cropData.cropName, "Bean (common)", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0;
        cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4; // alt: 2.0-4.0
        cropData.isPruningRequired = true;
        _cropsLibDataOld[Hydroponics_CropType_Bean] = cropData;
    }

    {   HydroponicsCropsLibData cropData(Hydroponics_CropType_BeanBroad);
        strncpy(cropData.cropName, "Bean (broad)", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
        cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4; // alt: 1.8-2.2
        cropData.isPruningRequired = true;
        _cropsLibDataOld[Hydroponics_CropType_BeanBroad] = cropData;
    }

    {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Beetroot);
        strncpy(cropData.cropName, "Beetroot", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
        cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 5.0;
        _cropsLibDataOld[Hydroponics_CropType_Beetroot] = cropData;
    }
  
    {   HydroponicsCropsLibData cropData(Hydroponics_CropType_BlackCurrant);
        strncpy(cropData.cropName, "Black Currant", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0;
        cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
        _cropsLibDataOld[Hydroponics_CropType_BlackCurrant] = cropData;
    }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Blueberry);
    //     strncpy(cropData.cropName, "Blueberry", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 4.0; cropData.phRange[0][1] = 5.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.0;
    //     cropData.isPerennial = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Blueberry] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_BokChoi);
    //     strncpy(cropData.cropName, "Bok-choi", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 1.5; cropData.ecRange[0][1] = 2.5;
    //     _cropsLibDataOld[Hydroponics_CropType_BokChoi] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Broccoli);
    //     strncpy(cropData.cropName, "Broccoli", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 2.8; cropData.ecRange[0][1] = 3.5;
    //     _cropsLibDataOld[Hydroponics_CropType_Broccoli] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_BrusselsSprout);
    //     strncpy(cropData.cropName, "Brussell Sprouts", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 7.5;
    //     cropData.ecRange[0][0] = 2.5; cropData.ecRange[0][1] = 3.0;
    //     _cropsLibDataOld[Hydroponics_CropType_BrusselsSprout] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Cabbage);
    //     strncpy(cropData.cropName, "Cabbage", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 2.5; cropData.ecRange[0][1] = 3.0;
    //     _cropsLibDataOld[Hydroponics_CropType_Cabbage] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Cannabis);
    //     strncpy(cropData.cropName, "Cannabis (generic)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.1;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 2.5;
    //     cropData.isLargePlant = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Cannabis] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Capsicum);
    //     strncpy(cropData.cropName, "Capsicum", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.2;
    //     _cropsLibDataOld[Hydroponics_CropType_Capsicum] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Carrots);
    //     strncpy(cropData.cropName, "Carrots", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.3;
    //     cropData.ecRange[0][0] = 1.6; cropData.ecRange[0][1] = 2.0;
    //     _cropsLibDataOld[Hydroponics_CropType_Carrots] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Catnip);
    //     strncpy(cropData.cropName, "Catnip", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     _cropsLibDataOld[Hydroponics_CropType_Catnip] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Cauliflower);
    //     strncpy(cropData.cropName, "Cauliflower", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 0.5; cropData.ecRange[0][1] = 2.0;
    //     _cropsLibDataOld[Hydroponics_CropType_Cauliflower] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Celery);
    //     strncpy(cropData.cropName, "Celery", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.3; cropData.phRange[0][1] = 6.7; // alt: 6.5
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     _cropsLibDataOld[Hydroponics_CropType_Celery] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Chamomile);
    //     strncpy(cropData.cropName, "Chamomile", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Chamomile] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Chicory);
    //     strncpy(cropData.cropName, "Chicory", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.4;
    //     _cropsLibDataOld[Hydroponics_CropType_Chicory] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Chives);
    //     strncpy(cropData.cropName, "Chives", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Chives] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Cilantro);
    //     strncpy(cropData.cropName, "Cilantro", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 6.7;
    //     cropData.ecRange[0][0] = 1.3; cropData.ecRange[0][1] = 1.8;
    //     _cropsLibDataOld[Hydroponics_CropType_Cilantro] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Coriander);
    //     strncpy(cropData.cropName, "Coriander", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.8; cropData.phRange[0][1] = 6.4;
    //     cropData.ecRange[0][0] = 1.2; cropData.ecRange[0][1] = 1.8;
    //     _cropsLibDataOld[Hydroponics_CropType_Coriander] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_CornSweet);
    //     strncpy(cropData.cropName, "Corn (sweet)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.6; cropData.ecRange[0][1] = 2.4;
    //     cropData.isLargePlant = true;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_CornSweet] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Cucumber);
    //     strncpy(cropData.cropName, "Cucumber", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.8; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 1.7; cropData.ecRange[0][1] = 2.5; // alt: 1.5-3.0
    //     cropData.isPruningRequired = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Cucumber] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Dill);
    //     strncpy(cropData.cropName, "Dill", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.4;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     _cropsLibDataOld[Hydroponics_CropType_Dill] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Eggplant);
    //     strncpy(cropData.cropName, "Eggplant", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 2.5; cropData.ecRange[0][1] = 3.5;
    //     cropData.isPruningRequired = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Eggplant] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Endive);
    //     strncpy(cropData.cropName, "Endive", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.4;
    //     _cropsLibDataOld[Hydroponics_CropType_Endive] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Fennel);
    //     strncpy(cropData.cropName, "Fennel", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.4; cropData.phRange[0][1] = 6.8;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.4;
    //     cropData.isPerennial = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Fennel] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Fodder);
    //     strncpy(cropData.cropName, "Fodder", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.0;
    //     _cropsLibDataOld[Hydroponics_CropType_Fodder] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Flowers);
    //     strncpy(cropData.cropName, "Flowers (generic)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.5; cropData.ecRange[0][1] = 2.5;
    //     cropData.isToxicToPets = true;
    //     cropData.isPruningRequired = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Flowers] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Garlic);
    //     strncpy(cropData.cropName, "Garlic", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Garlic] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Ginger);
    //     strncpy(cropData.cropName, "Ginger", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.8; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.5;
    //     _cropsLibDataOld[Hydroponics_CropType_Ginger] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Kale);
    //     strncpy(cropData.cropName, "Kale", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.25; cropData.ecRange[0][1] = 1.5;
    //     cropData.isPerennial = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Kale] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Lavender);
    //     strncpy(cropData.cropName, "Lavender", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.4; cropData.phRange[0][1] = 6.8;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.4;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Lavender] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Leek);
    //     strncpy(cropData.cropName, "Leek", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Leek] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_LemonBalm);
    //     strncpy(cropData.cropName, "Lemon Balm", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     cropData.isPerennial = true;
    //     _cropsLibDataOld[Hydroponics_CropType_LemonBalm] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Lettuce);
    //     strncpy(cropData.cropName, "Lettuce", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.2;
    //     _cropsLibDataOld[Hydroponics_CropType_Lettuce] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Marrow);
    //     strncpy(cropData.cropName, "Marrow", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     _cropsLibDataOld[Hydroponics_CropType_Marrow] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Melon);
    //     strncpy(cropData.cropName, "Melon", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.5;
    //     cropData.isLargePlant = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Melon] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Mint);
    //     strncpy(cropData.cropName, "Mint", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.4;
    //     cropData.isPerennial = true;
    //     cropData.isInvasiveOrViner = true;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Mint] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_MustardCress);
    //     strncpy(cropData.cropName, "Mustard Cress", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.2; cropData.ecRange[0][1] = 2.4;
    //     _cropsLibDataOld[Hydroponics_CropType_MustardCress] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Okra);
    //     strncpy(cropData.cropName, "Okra", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.4;
    //     _cropsLibDataOld[Hydroponics_CropType_Okra] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Onions);
    //     strncpy(cropData.cropName, "Onions", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.7;
    //     cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Onions] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Oregano);
    //     strncpy(cropData.cropName, "Oregano", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.3;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Oregano] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_PakChoi);
    //     strncpy(cropData.cropName, "Pak-choi", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 7.0;
    //     cropData.ecRange[0][0] = 1.5; cropData.ecRange[0][1] = 2.0;
    //     _cropsLibDataOld[Hydroponics_CropType_PakChoi] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Parsley);
    //     strncpy(cropData.cropName, "Parsley", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.8;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Parsley] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Parsnip);
    //     strncpy(cropData.cropName, "Parsnip", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
    //     _cropsLibDataOld[Hydroponics_CropType_Parsnip] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Pea);
    //     strncpy(cropData.cropName, "Pea (common)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.8;
    //     _cropsLibDataOld[Hydroponics_CropType_Pea] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_PeaSugar);
    //     strncpy(cropData.cropName, "Pea (sugar)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.8; // alt: 6.0-7.0
    //     cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.9; // alt: 0.8-1.8
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_PeaSugar] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Pepino);
    //     strncpy(cropData.cropName, "Pepino", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 5.0;
    //     _cropsLibDataOld[Hydroponics_CropType_Pepino] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_PeppersBell);
    //     strncpy(cropData.cropName, "Peppers (bell)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.5; // alt: 2.0-3.0
    //     cropData.isPruningRequired = true;
    //     _cropsLibDataOld[Hydroponics_CropType_PeppersBell] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_PeppersHot);
    //     strncpy(cropData.cropName, "Peppers (hot)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 3.5; // alt: 3.0-3.5
    //     cropData.isPruningRequired = true;
    //     _cropsLibDataOld[Hydroponics_CropType_PeppersHot] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Potato);
    //     strncpy(cropData.cropName, "Potato (common)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.5;
    //     cropData.isPerennial = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Potato] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_PotatoSweet);
    //     strncpy(cropData.cropName, "Potato (sweet)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.5;
    //     cropData.isPerennial = true;
    //     _cropsLibDataOld[Hydroponics_CropType_PotatoSweet] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Pumpkin);
    //     strncpy(cropData.cropName, "Pumpkin", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 7.5; // alt: 5.0-7.5
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     cropData.isLargePlant = true;
    //     cropData.isPruningRequired = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Pumpkin] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Radish);
    //     strncpy(cropData.cropName, "Radish", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 1.6; cropData.ecRange[0][1] = 2.2;
    //     _cropsLibDataOld[Hydroponics_CropType_Radish] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Rhubarb);
    //     strncpy(cropData.cropName, "Rhubarb", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 1.6; cropData.ecRange[0][1] = 2.0;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Rhubarb] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Rosemary);
    //     strncpy(cropData.cropName, "Rosemary", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     cropData.isPerennial = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Rosemary] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Sage);
    //     strncpy(cropData.cropName, "Sage", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     cropData.isPerennial = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Sage] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Silverbeet);
    //     strncpy(cropData.cropName, "Silverbeet", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.3;
    //     _cropsLibDataOld[Hydroponics_CropType_Silverbeet] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Spinach);
    //     strncpy(cropData.cropName, "Spinach", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.6; // alt: 6.0-7.0
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.3;
    //     _cropsLibDataOld[Hydroponics_CropType_Spinach] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Squash);
    //     strncpy(cropData.cropName, "Squash", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     cropData.isLargePlant = true;
    //     cropData.isPruningRequired = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Squash] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Sunflower);
    //     strncpy(cropData.cropName, "Sunflower", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.2; cropData.ecRange[0][1] = 1.8;
    //     _cropsLibDataOld[Hydroponics_CropType_Sunflower] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Strawberries);
    //     strncpy(cropData.cropName, "Strawberries", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 5.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.4;
    //     cropData.isPerennial = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Strawberries] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_SwissChard);
    //     strncpy(cropData.cropName, "Swiss Chard", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.3;
    //     _cropsLibDataOld[Hydroponics_CropType_SwissChard] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Taro);
    //     strncpy(cropData.cropName, "Taro", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 5.5;
    //     cropData.ecRange[0][0] = 2.5; cropData.ecRange[0][1] = 3.0;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Taro] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Tarragon);
    //     strncpy(cropData.cropName, "Tarragon", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.8;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Tarragon] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Thyme);
    //     strncpy(cropData.cropName, "Thyme", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.6;
    //     cropData.isPerennial = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Thyme] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Tomato);
    //     strncpy(cropData.cropName, "Tomato", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5; // alt: 5.5-6.0
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 5.0;
    //     cropData.isToxicToPets = true;
    //     cropData.isPruningRequired = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Tomato] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Turnip);
    //     strncpy(cropData.cropName, "Turnip", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     _cropsLibDataOld[Hydroponics_CropType_Turnip] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Watercress);
    //     strncpy(cropData.cropName, "Watercress", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 6.8;
    //     cropData.ecRange[0][0] = 0.4; cropData.ecRange[0][1] = 1.8;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Watercress] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Watermelon);
    //     strncpy(cropData.cropName, "Watermelon", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.8;
    //     cropData.ecRange[0][0] = 1.5; cropData.ecRange[0][1] = 2.4;
    //     cropData.isLargePlant = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Watermelon] = cropData;
    // }

    // {   HydroponicsCropsLibData cropData(Hydroponics_CropType_Zucchini);
    //     strncpy(cropData.cropName, "Zucchini", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     cropData.isLargePlant = true;
    //     _cropsLibDataOld[Hydroponics_CropType_Zucchini] = cropData;
    // }

    validateEntries();

    _libraryBuilt = true; // To be removed in near future
}

void HydroponicsCropsLibrary::validateEntries()
{
    for (Hydroponics_CropType cropTypeIndex = (Hydroponics_CropType)0; cropTypeIndex < Hydroponics_CropType_Count; cropTypeIndex = (Hydroponics_CropType)((int)cropTypeIndex + 1)) {
        Hydroponics_CropPhase cropPhaseIndex;

        // Ensure phase begin week is increasing
        for(cropPhaseIndex = (Hydroponics_CropPhase)1; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
            if (_cropsLibDataOld[cropTypeIndex].phaseBeginWeek[cropPhaseIndex] <= _cropsLibDataOld[cropTypeIndex].phaseBeginWeek[cropPhaseIndex-1]) {
                _cropsLibDataOld[cropTypeIndex].phaseBeginWeek[cropPhaseIndex] = _cropsLibDataOld[cropTypeIndex].phaseBeginWeek[cropPhaseIndex-1] + 1;
            }
        }

        // Check for empty pH/EC second value entries
        for(cropPhaseIndex = (Hydroponics_CropPhase)0; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
            if (_cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][0] > 0 &&
                _cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][1] == 0) {
                _cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][1] = _cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][0];
            }

            if (_cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][0] > 0 &&
                _cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][1] == 0) {
                _cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][1] = _cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][0];
            }
        }

        // Advance previous phase entries if later phase is empty
        for(cropPhaseIndex = (Hydroponics_CropPhase)1; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
            if (_cropsLibDataOld[cropTypeIndex].lightHoursPerDay[cropPhaseIndex] == 0 &&
                _cropsLibDataOld[cropTypeIndex].lightHoursPerDay[cropPhaseIndex-1] > 0) {
                _cropsLibDataOld[cropTypeIndex].lightHoursPerDay[cropPhaseIndex] = _cropsLibDataOld[cropTypeIndex].lightHoursPerDay[cropPhaseIndex-1];
            }

            if (_cropsLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex][0] == 0 &&
                _cropsLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex][1] == 0 &&
                _cropsLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][0] > 0 &&
                _cropsLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][1] > 0) {
                _cropsLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex][0] = _cropsLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][0];
                _cropsLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex][1] = _cropsLibDataOld[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][1];
            }

            if (_cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][0] == 0 &&
                _cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][1] == 0 &&
                _cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex-1][0] > 0 &&
                _cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex-1][1] > 0) {
                _cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][0] = _cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex-1][0];
                _cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex][1] = _cropsLibDataOld[cropTypeIndex].phRange[cropPhaseIndex-1][1];
            }

            if (_cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][0] == 0 &&
                _cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][1] == 0 &&
                _cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex-1][0] > 0 &&
                _cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex-1][1] > 0) {
                _cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][0] = _cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex-1][0];
                _cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex][1] = _cropsLibDataOld[cropTypeIndex].ecRange[cropPhaseIndex-1][1];
            }

            if (_cropsLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex][0] == 0 &&
                _cropsLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex][1] == 0 &&
                _cropsLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex-1][0] > 0 &&
                _cropsLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex-1][1] > 0) {
                _cropsLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex][0] = _cropsLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex-1][0];
                _cropsLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex][1] = _cropsLibDataOld[cropTypeIndex].waterTempRange[cropPhaseIndex-1][1];
            }

            if (_cropsLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex][0] == 0 &&
                _cropsLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex][1] == 0 &&
                _cropsLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex-1][0] > 0 &&
                _cropsLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex-1][1] > 0) {
                _cropsLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex][0] = _cropsLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex-1][0];
                _cropsLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex][1] = _cropsLibDataOld[cropTypeIndex].airTempRange[cropPhaseIndex-1][1];
            }
        }
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
            book->data = _cropsLibDataOld[cropType]; // remove after flash move
            //deserializeJson(doc, F("TODO"));
            //book->data.fromJSONElement(doc.as<JsonVariantConst>());
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
