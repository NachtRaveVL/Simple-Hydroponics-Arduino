/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Crops
*/

#include "HydroponicsCrops.h"

HydroponicsCropData::HydroponicsCropData()
    : _ident{'H','C','D'}, _version(1),
      cropType(Hydroponics_CropType_Undefined), plantName{'\0'},
      growWeeksToHarvest(0), weeksBetweenHarvest(0),
      phaseBeginWeek{0}, lightHoursPerDay{0},
      isInvasiveOrViner(false), isLargePlant(false), isPerennial(false),
      isPrunningRequired(false), isToxicToPets(false)
{
    memset(feedIntervalMins, 0, sizeof(feedIntervalMins));
    memset(phRange, 0, sizeof(phRange));
    memset(ecRange, 0, sizeof(ecRange));
    memset(waterTempRange, 0, sizeof(waterTempRange));
    memset(airTempRange, 0, sizeof(airTempRange));
}

HydroponicsCropData::HydroponicsCropData(const Hydroponics_CropType cropTypeIn)
    : _ident{'H','C','D'}, _version(1),
      cropType(cropTypeIn), plantName{'\0'},
      growWeeksToHarvest(0), weeksBetweenHarvest(0),
      phaseBeginWeek{0}, lightHoursPerDay{0},
      isInvasiveOrViner(false), isLargePlant(false), isPerennial(false),
      isPrunningRequired(false), isToxicToPets(false)
{
    memset(feedIntervalMins, 0, sizeof(feedIntervalMins));
    memset(phRange, 0, sizeof(phRange));
    memset(ecRange, 0, sizeof(ecRange));
    memset(waterTempRange, 0, sizeof(waterTempRange));
    memset(airTempRange, 0, sizeof(airTempRange));

    if (HydroponicsCropsLibrary::_cropLibraryBuilt) {
        *this = *(HydroponicsCropsLibrary::getInstance()->getCropData(this->cropType));
    }
}


HydroponicsCropsLibrary *HydroponicsCropsLibrary::_instance = NULL;
bool HydroponicsCropsLibrary::_cropLibraryBuilt = false;

HydroponicsCropsLibrary::HydroponicsCropsLibrary()
{
    buildLibrary();
}

HydroponicsCropsLibrary *HydroponicsCropsLibrary::getInstance()
{
    if (!_instance) {
        _instance = new HydroponicsCropsLibrary();
    }
    return _instance;
}

void HydroponicsCropsLibrary::buildLibrary()
{   
    HydroponicsCropsLibrary::_cropLibraryBuilt = false;

    {   HydroponicsCropData cropData(Hydroponics_CropType_AloeVera);
        strncpy(&cropData.plantName[0], "Aloe Vera", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 7.0; cropData.phRange[0][1] = 8.5;
        cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.5;
        cropData.isPerennial = true;
        cropData.isToxicToPets = true;
        _cropData[Hydroponics_CropType_AloeVera] = cropData;
    }

    {   HydroponicsCropData cropData(Hydroponics_CropType_Anise);
        strncpy(&cropData.plantName[0], "Anise", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 5.8; cropData.phRange[0][1] = 6.4;
        cropData.ecRange[0][0] = 0.9; cropData.ecRange[0][1] = 1.4;
        _cropData[Hydroponics_CropType_Anise] = cropData;
    }

    {   HydroponicsCropData cropData(Hydroponics_CropType_Artichoke);
        strncpy(&cropData.plantName[0], "Artichoke", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 7.5;
        cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.8;
        cropData.isPerennial = true;
        _cropData[Hydroponics_CropType_Artichoke] = cropData;
    }

    {   HydroponicsCropData cropData(Hydroponics_CropType_Arugula);
        strncpy(&cropData.plantName[0], "Arugula", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.5;
        cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.8;
        _cropData[Hydroponics_CropType_Arugula] = cropData;
    }

    {   HydroponicsCropData cropData(Hydroponics_CropType_Asparagus);
        strncpy(&cropData.plantName[0], "Asparagus", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.8;
        cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
        cropData.isPerennial = true;
        cropData.isPrunningRequired = true;
        _cropData[Hydroponics_CropType_Asparagus] = cropData;
    }

    {   HydroponicsCropData cropData(Hydroponics_CropType_Basil);
        strncpy(&cropData.plantName[0], "Basil", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
        cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
        cropData.isPrunningRequired = true;
        _cropData[Hydroponics_CropType_Basil] = cropData;
    }

    {   HydroponicsCropData cropData(Hydroponics_CropType_Bean);
        strncpy(&cropData.plantName[0], "Bean (common)", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0;
        cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4; // alt: 2.0-4.0
        cropData.isPrunningRequired = true;
        _cropData[Hydroponics_CropType_Bean] = cropData;
    }

    {   HydroponicsCropData cropData(Hydroponics_CropType_BeanBroad);
        strncpy(&cropData.plantName[0], "Bean (broad)", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
        cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4; // alt: 1.8-2.2
        cropData.isPrunningRequired = true;
        _cropData[Hydroponics_CropType_BeanBroad] = cropData;
    }

    {   HydroponicsCropData cropData(Hydroponics_CropType_Beetroot);
        strncpy(&cropData.plantName[0], "Beetroot", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
        cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 5.0;
        _cropData[Hydroponics_CropType_Beetroot] = cropData;
    }

    {   HydroponicsCropData cropData(Hydroponics_CropType_BlackCurrant);
        strncpy(&cropData.plantName[0], "Black Currant", HYDRUINO_NAME_MAXSIZE);
        cropData.phRange[0][0] = 6.0;
        cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
        _cropData[Hydroponics_CropType_BlackCurrant] = cropData;
    }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Blueberry);
    //     strncpy(&cropData.plantName[0], "Blueberry", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 4.0; cropData.phRange[0][1] = 5.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.0;
    //     cropData.isPerennial = true;
    //     _cropData[Hydroponics_CropType_Blueberry] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_BokChoi);
    //     strncpy(&cropData.plantName[0], "Bok-choi", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 1.5; cropData.ecRange[0][1] = 2.5;
    //     _cropData[Hydroponics_CropType_BokChoi] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Broccoli);
    //     strncpy(&cropData.plantName[0], "Broccoli", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 2.8; cropData.ecRange[0][1] = 3.5;
    //     _cropData[Hydroponics_CropType_Broccoli] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_BrussellSprouts);
    //     strncpy(&cropData.plantName[0], "Brussell Sprouts", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 7.5;
    //     cropData.ecRange[0][0] = 2.5; cropData.ecRange[0][1] = 3.0;
    //     _cropData[Hydroponics_CropType_BrussellSprouts] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Cabbage);
    //     strncpy(&cropData.plantName[0], "Cabbage", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 2.5; cropData.ecRange[0][1] = 3.0;
    //     _cropData[Hydroponics_CropType_Cabbage] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Cannabis);
    //     strncpy(&cropData.plantName[0], "Cannabis (generic)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.1;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 2.5;
    //     cropData.isLargePlant = true;
    //     _cropData[Hydroponics_CropType_Cannabis] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Capiscum);
    //     strncpy(&cropData.plantName[0], "Capiscum", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.2;
    //     _cropData[Hydroponics_CropType_Capiscum] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Carrots);
    //     strncpy(&cropData.plantName[0], "Carrots", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.3;
    //     cropData.ecRange[0][0] = 1.6; cropData.ecRange[0][1] = 2.0;
    //     _cropData[Hydroponics_CropType_Carrots] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Catnip);
    //     strncpy(&cropData.plantName[0], "Catnip", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     _cropData[Hydroponics_CropType_Catnip] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Cauliflower);
    //     strncpy(&cropData.plantName[0], "Cauliflower", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 0.5; cropData.ecRange[0][1] = 2.0;
    //     _cropData[Hydroponics_CropType_Cauliflower] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Celery);
    //     strncpy(&cropData.plantName[0], "Celery", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.3; cropData.phRange[0][1] = 6.7; // alt: 6.5
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     _cropData[Hydroponics_CropType_Celery] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Chamomile);
    //     strncpy(&cropData.plantName[0], "Chamomile", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Chamomile] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Chickory);
    //     strncpy(&cropData.plantName[0], "Chickory", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.4;
    //     _cropData[Hydroponics_CropType_Chickory] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Chives);
    //     strncpy(&cropData.plantName[0], "Chives", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Chives] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Cilantro);
    //     strncpy(&cropData.plantName[0], "Cilantro", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 6.7;
    //     cropData.ecRange[0][0] = 1.3; cropData.ecRange[0][1] = 1.8;
    //     _cropData[Hydroponics_CropType_Cilantro] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Coriander);
    //     strncpy(&cropData.plantName[0], "Coriander", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.8; cropData.phRange[0][1] = 6.4;
    //     cropData.ecRange[0][0] = 1.2; cropData.ecRange[0][1] = 1.8;
    //     _cropData[Hydroponics_CropType_Coriander] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_CornSweet);
    //     strncpy(&cropData.plantName[0], "Corn (sweet)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.6; cropData.ecRange[0][1] = 2.4;
    //     cropData.isLargePlant = true;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_CornSweet] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Cucumber);
    //     strncpy(&cropData.plantName[0], "Cucumber", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.8; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 1.7; cropData.ecRange[0][1] = 2.5; // alt: 1.5-3.0
    //     cropData.isPrunningRequired = true;
    //     _cropData[Hydroponics_CropType_Cucumber] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Dill);
    //     strncpy(&cropData.plantName[0], "Dill", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.4;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     _cropData[Hydroponics_CropType_Dill] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Eggplant);
    //     strncpy(&cropData.plantName[0], "Eggplant", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 2.5; cropData.ecRange[0][1] = 3.5;
    //     cropData.isPrunningRequired = true;
    //     _cropData[Hydroponics_CropType_Eggplant] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Endive);
    //     strncpy(&cropData.plantName[0], "Endive", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.4;
    //     _cropData[Hydroponics_CropType_Endive] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Fennel);
    //     strncpy(&cropData.plantName[0], "Fennel", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.4; cropData.phRange[0][1] = 6.8;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.4;
    //     cropData.isPerennial = true;
    //     _cropData[Hydroponics_CropType_Fennel] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Fodder);
    //     strncpy(&cropData.plantName[0], "Fodder", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.0;
    //     _cropData[Hydroponics_CropType_Fodder] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Flowers);
    //     strncpy(&cropData.plantName[0], "Flowers (generic)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.5; cropData.ecRange[0][1] = 2.5;
    //     cropData.isToxicToPets = true;
    //     cropData.isPrunningRequired = true;
    //     _cropData[Hydroponics_CropType_Flowers] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Garlic);
    //     strncpy(&cropData.plantName[0], "Garlic", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Garlic] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Ginger);
    //     strncpy(&cropData.plantName[0], "Ginger", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.8; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.5;
    //     _cropData[Hydroponics_CropType_Ginger] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Kale);
    //     strncpy(&cropData.plantName[0], "Kale", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.25; cropData.ecRange[0][1] = 1.5;
    //     cropData.isPerennial = true;
    //     _cropData[Hydroponics_CropType_Kale] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Lavender);
    //     strncpy(&cropData.plantName[0], "Lavender", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.4; cropData.phRange[0][1] = 6.8;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.4;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Lavender] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Leek);
    //     strncpy(&cropData.plantName[0], "Leek", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Leek] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_LemonBalm);
    //     strncpy(&cropData.plantName[0], "Lemon Balm", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     cropData.isPerennial = true;
    //     _cropData[Hydroponics_CropType_LemonBalm] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Lettuce);
    //     strncpy(&cropData.plantName[0], "Lettuce", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.2;
    //     _cropData[Hydroponics_CropType_Lettuce] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Marrow);
    //     strncpy(&cropData.plantName[0], "Marrow", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     _cropData[Hydroponics_CropType_Marrow] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Melon);
    //     strncpy(&cropData.plantName[0], "Melon", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.5;
    //     cropData.isLargePlant = true;
    //     _cropData[Hydroponics_CropType_Melon] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Mint);
    //     strncpy(&cropData.plantName[0], "Mint", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.4;
    //     cropData.isPerennial = true;
    //     cropData.isInvasiveOrViner = true;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Mint] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_MustardCress);
    //     strncpy(&cropData.plantName[0], "Mustard Cress", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.2; cropData.ecRange[0][1] = 2.4;
    //     _cropData[Hydroponics_CropType_MustardCress] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Okra);
    //     strncpy(&cropData.plantName[0], "Okra", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.4;
    //     _cropData[Hydroponics_CropType_Okra] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Onions);
    //     strncpy(&cropData.plantName[0], "Onions", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.7;
    //     cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Onions] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Oregano);
    //     strncpy(&cropData.plantName[0], "Oregano", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.3;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Oregano] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_PakChoi);
    //     strncpy(&cropData.plantName[0], "Pak-choi", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 7.0;
    //     cropData.ecRange[0][0] = 1.5; cropData.ecRange[0][1] = 2.0;
    //     _cropData[Hydroponics_CropType_PakChoi] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Parsley);
    //     strncpy(&cropData.plantName[0], "Parsley", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.8;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Parsley] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Parsnip);
    //     strncpy(&cropData.plantName[0], "Parsnip", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.4; cropData.ecRange[0][1] = 1.8;
    //     _cropData[Hydroponics_CropType_Parsnip] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Pea);
    //     strncpy(&cropData.plantName[0], "Pea (common)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.8;
    //     _cropData[Hydroponics_CropType_Pea] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_PeaSugar);
    //     strncpy(&cropData.plantName[0], "Pea (sugar)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.8; // alt: 6.0-7.0
    //     cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.9; // alt: 0.8-1.8
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_PeaSugar] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Pepino);
    //     strncpy(&cropData.plantName[0], "Pepino", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 5.0;
    //     _cropData[Hydroponics_CropType_Pepino] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_PeppersBell);
    //     strncpy(&cropData.plantName[0], "Peppers (bell)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.5; // alt: 2.0-3.0
    //     cropData.isPrunningRequired = true;
    //     _cropData[Hydroponics_CropType_PeppersBell] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_PeppersHot);
    //     strncpy(&cropData.plantName[0], "Peppers (hot)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 3.5; // alt: 3.0-3.5
    //     cropData.isPrunningRequired = true;
    //     _cropData[Hydroponics_CropType_PeppersHot] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Potato);
    //     strncpy(&cropData.plantName[0], "Potato (common)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.5;
    //     cropData.isPerennial = true;
    //     _cropData[Hydroponics_CropType_Potato] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_PotatoSweet);
    //     strncpy(&cropData.plantName[0], "Potato (sweet)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 2.5;
    //     cropData.isPerennial = true;
    //     _cropData[Hydroponics_CropType_PotatoSweet] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Pumpkin);
    //     strncpy(&cropData.plantName[0], "Pumpkin", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 7.5; // alt: 5.0-7.5
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     cropData.isLargePlant = true;
    //     cropData.isPrunningRequired = true;
    //     _cropData[Hydroponics_CropType_Pumpkin] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Radish);
    //     strncpy(&cropData.plantName[0], "Radish", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 1.6; cropData.ecRange[0][1] = 2.2;
    //     _cropData[Hydroponics_CropType_Radish] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Rhubarb);
    //     strncpy(&cropData.plantName[0], "Rhubarb", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 1.6; cropData.ecRange[0][1] = 2.0;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Rhubarb] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Rosemary);
    //     strncpy(&cropData.plantName[0], "Rosemary", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.0;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     cropData.isPerennial = true;
    //     _cropData[Hydroponics_CropType_Rosemary] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Sage);
    //     strncpy(&cropData.plantName[0], "Sage", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.6;
    //     cropData.isPerennial = true;
    //     _cropData[Hydroponics_CropType_Sage] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Silverbeet);
    //     strncpy(&cropData.plantName[0], "Silverbeet", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.3;
    //     _cropData[Hydroponics_CropType_Silverbeet] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Spinach);
    //     strncpy(&cropData.plantName[0], "Spinach", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.6; // alt: 6.0-7.0
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.3;
    //     _cropData[Hydroponics_CropType_Spinach] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Squash);
    //     strncpy(&cropData.plantName[0], "Squash", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     cropData.isLargePlant = true;
    //     cropData.isPrunningRequired = true;
    //     _cropData[Hydroponics_CropType_Squash] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Sunflower);
    //     strncpy(&cropData.plantName[0], "Sunflower", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.2; cropData.ecRange[0][1] = 1.8;
    //     _cropData[Hydroponics_CropType_Sunflower] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Strawberries);
    //     strncpy(&cropData.plantName[0], "Strawberries", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 5.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.4;
    //     cropData.isPerennial = true;
    //     _cropData[Hydroponics_CropType_Strawberries] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_SwissChard);
    //     strncpy(&cropData.plantName[0], "Swiss Chard", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.3;
    //     _cropData[Hydroponics_CropType_SwissChard] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Taro);
    //     strncpy(&cropData.plantName[0], "Taro", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 5.5;
    //     cropData.ecRange[0][0] = 2.5; cropData.ecRange[0][1] = 3.0;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Taro] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Tarragon);
    //     strncpy(&cropData.plantName[0], "Tarragon", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.0; cropData.ecRange[0][1] = 1.8;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Tarragon] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Thyme);
    //     strncpy(&cropData.plantName[0], "Thyme", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.0; cropData.phRange[0][1] = 7.0;
    //     cropData.ecRange[0][0] = 0.8; cropData.ecRange[0][1] = 1.6;
    //     cropData.isPerennial = true;
    //     _cropData[Hydroponics_CropType_Thyme] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Tomato);
    //     strncpy(&cropData.plantName[0], "Tomato", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.5; cropData.phRange[0][1] = 6.5; // alt: 5.5-6.0
    //     cropData.ecRange[0][0] = 2.0; cropData.ecRange[0][1] = 5.0;
    //     cropData.isToxicToPets = true;
    //     cropData.isPrunningRequired = true;
    //     _cropData[Hydroponics_CropType_Tomato] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Turnip);
    //     strncpy(&cropData.plantName[0], "Turnip", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0; cropData.phRange[0][1] = 6.5;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     _cropData[Hydroponics_CropType_Turnip] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Watercress);
    //     strncpy(&cropData.plantName[0], "Watercress", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.5; cropData.phRange[0][1] = 6.8;
    //     cropData.ecRange[0][0] = 0.4; cropData.ecRange[0][1] = 1.8;
    //     cropData.isPerennial = true;
    //     cropData.isToxicToPets = true;
    //     _cropData[Hydroponics_CropType_Watercress] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Watermelon);
    //     strncpy(&cropData.plantName[0], "Watermelon (common)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.8;
    //     cropData.ecRange[0][0] = 1.5; cropData.ecRange[0][1] = 2.4;
    //     cropData.isLargePlant = true;
    //     _cropData[Hydroponics_CropType_Watermelon] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_WatermelonBaby);
    //     strncpy(&cropData.plantName[0], "Watermelon (baby)", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 5.8;
    //     cropData.ecRange[0][0] = 1.5; cropData.ecRange[0][1] = 2.4;
    //     _cropData[Hydroponics_CropType_WatermelonBaby] = cropData;
    // }

    // {   HydroponicsCropData cropData(Hydroponics_CropType_Zucchini);
    //     strncpy(&cropData.plantName[0], "Zucchini", HYDRUINO_NAME_MAXSIZE);
    //     cropData.phRange[0][0] = 6.0;
    //     cropData.ecRange[0][0] = 1.8; cropData.ecRange[0][1] = 2.4;
    //     cropData.isLargePlant = true;
    //     _cropData[Hydroponics_CropType_Zucchini] = cropData;
    // }

    validateEntries();

    HydroponicsCropsLibrary::_cropLibraryBuilt = true;
}

void HydroponicsCropsLibrary::validateEntries()
{
    for (Hydroponics_CropType cropTypeIndex = (Hydroponics_CropType)0; cropTypeIndex < Hydroponics_CropType_Count; cropTypeIndex = (Hydroponics_CropType)((int)cropTypeIndex + 1)) {
        Hydroponics_CropPhase cropPhaseIndex;

        // Ensure phase begin week is increasing
        for(cropPhaseIndex = (Hydroponics_CropPhase)1; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
            if (_cropData[cropTypeIndex].phaseBeginWeek[cropPhaseIndex] <= _cropData[cropTypeIndex].phaseBeginWeek[cropPhaseIndex-1]) {
                _cropData[cropTypeIndex].phaseBeginWeek[cropPhaseIndex] = _cropData[cropTypeIndex].phaseBeginWeek[cropPhaseIndex-1] + 1;
            }
        }

        // Check for empty pH/EC second value entries
        for(cropPhaseIndex = (Hydroponics_CropPhase)0; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
            if (_cropData[cropTypeIndex].phRange[cropPhaseIndex][0] > 0 &&
                _cropData[cropTypeIndex].phRange[cropPhaseIndex][1] == 0) {
                _cropData[cropTypeIndex].phRange[cropPhaseIndex][1] = _cropData[cropTypeIndex].phRange[cropPhaseIndex][0];
            }

            if (_cropData[cropTypeIndex].ecRange[cropPhaseIndex][0] > 0 &&
                _cropData[cropTypeIndex].ecRange[cropPhaseIndex][1] == 0) {
                _cropData[cropTypeIndex].ecRange[cropPhaseIndex][1] = _cropData[cropTypeIndex].ecRange[cropPhaseIndex][0];
            }
        }

        // Advance previous phase entries if later phase is empty
        for(cropPhaseIndex = (Hydroponics_CropPhase)1; cropPhaseIndex < Hydroponics_CropPhase_Count; cropPhaseIndex = (Hydroponics_CropPhase)((int)cropPhaseIndex + 1)) {
            if (_cropData[cropTypeIndex].lightHoursPerDay[cropPhaseIndex] == 0 &&
                _cropData[cropTypeIndex].lightHoursPerDay[cropPhaseIndex-1] > 0) {
                _cropData[cropTypeIndex].lightHoursPerDay[cropPhaseIndex] = _cropData[cropTypeIndex].lightHoursPerDay[cropPhaseIndex-1];
            }

            if (_cropData[cropTypeIndex].feedIntervalMins[cropPhaseIndex][0] == 0 &&
                _cropData[cropTypeIndex].feedIntervalMins[cropPhaseIndex][1] == 0 &&
                _cropData[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][0] > 0 &&
                _cropData[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][1] > 0) {
                _cropData[cropTypeIndex].feedIntervalMins[cropPhaseIndex][0] = _cropData[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][0];
                _cropData[cropTypeIndex].feedIntervalMins[cropPhaseIndex][1] = _cropData[cropTypeIndex].feedIntervalMins[cropPhaseIndex-1][1];
            }

            if (_cropData[cropTypeIndex].phRange[cropPhaseIndex][0] == 0 &&
                _cropData[cropTypeIndex].phRange[cropPhaseIndex][1] == 0 &&
                _cropData[cropTypeIndex].phRange[cropPhaseIndex-1][0] > 0 &&
                _cropData[cropTypeIndex].phRange[cropPhaseIndex-1][1] > 0) {
                _cropData[cropTypeIndex].phRange[cropPhaseIndex][0] = _cropData[cropTypeIndex].phRange[cropPhaseIndex-1][0];
                _cropData[cropTypeIndex].phRange[cropPhaseIndex][1] = _cropData[cropTypeIndex].phRange[cropPhaseIndex-1][1];
            }

            if (_cropData[cropTypeIndex].ecRange[cropPhaseIndex][0] == 0 &&
                _cropData[cropTypeIndex].ecRange[cropPhaseIndex][1] == 0 &&
                _cropData[cropTypeIndex].ecRange[cropPhaseIndex-1][0] > 0 &&
                _cropData[cropTypeIndex].ecRange[cropPhaseIndex-1][1] > 0) {
                _cropData[cropTypeIndex].ecRange[cropPhaseIndex][0] = _cropData[cropTypeIndex].ecRange[cropPhaseIndex-1][0];
                _cropData[cropTypeIndex].ecRange[cropPhaseIndex][1] = _cropData[cropTypeIndex].ecRange[cropPhaseIndex-1][1];
            }

            if (_cropData[cropTypeIndex].waterTempRange[cropPhaseIndex][0] == 0 &&
                _cropData[cropTypeIndex].waterTempRange[cropPhaseIndex][1] == 0 &&
                _cropData[cropTypeIndex].waterTempRange[cropPhaseIndex-1][0] > 0 &&
                _cropData[cropTypeIndex].waterTempRange[cropPhaseIndex-1][1] > 0) {
                _cropData[cropTypeIndex].waterTempRange[cropPhaseIndex][0] = _cropData[cropTypeIndex].waterTempRange[cropPhaseIndex-1][0];
                _cropData[cropTypeIndex].waterTempRange[cropPhaseIndex][1] = _cropData[cropTypeIndex].waterTempRange[cropPhaseIndex-1][1];
            }

            if (_cropData[cropTypeIndex].airTempRange[cropPhaseIndex][0] == 0 &&
                _cropData[cropTypeIndex].airTempRange[cropPhaseIndex][1] == 0 &&
                _cropData[cropTypeIndex].airTempRange[cropPhaseIndex-1][0] > 0 &&
                _cropData[cropTypeIndex].airTempRange[cropPhaseIndex-1][1] > 0) {
                _cropData[cropTypeIndex].airTempRange[cropPhaseIndex][0] = _cropData[cropTypeIndex].airTempRange[cropPhaseIndex-1][0];
                _cropData[cropTypeIndex].airTempRange[cropPhaseIndex][1] = _cropData[cropTypeIndex].airTempRange[cropPhaseIndex-1][1];
            }
        }
    }
}

const HydroponicsCropData *HydroponicsCropsLibrary::getCropData(Hydroponics_CropType cropType) const
{
    //assert((int)cropType >= 0 && cropType < Hydroponics_CropType_Count && "Invalid parameters");
    return &_cropData[cropType];
}

void HydroponicsCropsLibrary::setCustomCropData(const Hydroponics_CropType cropType, const HydroponicsCropData *cropData)
{
    //assert(cropType >= Hydroponics_CropType_Custom1 && cropType <= Hydroponics_CropType_Custom5 && "Invalid parameters");
    _cropData[cropType] = *cropData;
    validateEntries();
}


HydroponicsCrop::HydroponicsCrop(const Hydroponics_CropType cropType, const int positionIndex, const time_t sowDate)
    : _cropType(cropType), _positionIndex(positionIndex), _sowDate(sowDate),
      _cropData(NULL), _growWeek(0), _cropPhase(Hydroponics_CropPhase_Undefined)
{
    _cropData = HydroponicsCropsLibrary::getInstance()->getCropData(cropType);

    recalcGrowWeekAndPhase();
}

const Hydroponics_CropType HydroponicsCrop::getCropType() const
{
    return _cropType;
}

const HydroponicsCropData *HydroponicsCrop::getCropData() const
{
    return _cropData;
}

int HydroponicsCrop::getPositionIndex() const
{
    return _positionIndex;
}

time_t HydroponicsCrop::getSowDate() const
{
    return _sowDate;
}

int HydroponicsCrop::getGrowWeek() const
{
    return _growWeek;
}

Hydroponics_CropPhase HydroponicsCrop::getCropPhase() const
{
    return _cropPhase;
}

void HydroponicsCrop::update()
{
    // TODO
    recalcGrowWeekAndPhase();
}

void HydroponicsCrop::recalcGrowWeekAndPhase()
{
    // TODO
}
