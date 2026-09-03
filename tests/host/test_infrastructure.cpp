#include "Hydruino.h"
#include <cassert>
#include <iostream>

int main()
{
    Hydruino controller;
    controller.init();

    SharedPtr<HydroObject> nullObject;
    assert(!controller.registerObject(nullObject));
    assert(!controller.unregisterObject(nullObject));

    auto first = controller.addFeedWaterReservoir(1000.0f, false);
    auto second = controller.addFeedWaterReservoir(1000.0f, false);
    assert(first && second);
    assert(first->getId().isReservoirType());
    assert(first->getId().objTypeAs.reservoirType == Hydro_ReservoirType_FeedWater);
    assert(first->getId().posIndex != second->getId().posIndex);
    assert(first->getKey() != second->getKey());
    assert(controller.objectById(first->getId()).get() == first.get());
    assert(controller.unregisterObject(second));
    assert(!controller.objectById(second->getId()));
    assert(controller.registerObject(second));
    assert(controller.objectById(second->getId()).get() == second.get());

    HydroSingleMeasurement celsius(20.0f, Hydro_UnitsType_Temperature_Celsius, 100, 1);
    HydroSingleMeasurement fahrenheit = celsius.asUnits(Hydro_UnitsType_Temperature_Fahrenheit);
    assert(fahrenheit.isSet());
    assert(isFPEqual(fahrenheit.value, 68.0f));

    auto level = controller.addLevelIndicator(4, true);
    assert(level);

    HydroCalibrationData calibration(level->getId(), Hydro_UnitsType_Raw_1);
    calibration.setFromTwoPoints(0.0f, 0.0f, 1.0f, 1.0f);
    level->setUserCalibrationData(&calibration);
    assert(controller.hasUserCalibrations());
    assert(level->getUserCalibrationData() != nullptr);
    level->setUserCalibrationData(nullptr);
    assert(!controller.hasUserCalibrations());
    assert(level->getUserCalibrationData() == nullptr);

    level->setUserCalibrationData(&calibration);
    assert(controller.hasUserCalibrations());
    assert(level->getUserCalibrationData() != nullptr);
    controller.clearUserCalibrations();
    assert(!controller.hasUserCalibrations());
    assert(level->getUserCalibrationData() == nullptr);
    assert(controller.getUserCalibrationData(level->getKey()) == nullptr);

    HydroCustomAdditiveData additive;
    additive.reservoirType = Hydro_ReservoirType_CustomAdditive1;
    additive.weeklyDosingRates[0] = 0.25f;
    assert(controller.setCustomAdditiveData(&additive));
    assert(controller.hasCustomAdditives());
    assert(controller.getCustomAdditiveData(Hydro_ReservoirType_CustomAdditive1) != nullptr);
    controller.clearUserAdditives();
    assert(!controller.hasCustomAdditives());
    assert(controller.getCustomAdditiveData(Hydro_ReservoirType_CustomAdditive1) == nullptr);

    HydroData *saved = first->newSaveData();
    assert(saved && saved->isObjectData());
    HydroObject *restored = newObjectFromData(static_cast<HydroObjectData *>(saved));
    assert(restored);
    assert(restored->getId() == first->getId());
    delete restored;
    delete saved;

    std::cout << "PASS Hydruino infrastructure" << std::endl;
    return 0;
}
