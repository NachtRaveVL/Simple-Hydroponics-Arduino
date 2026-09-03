#include "Hydruino.h"
#include <cassert>
#include <iostream>

int main()
{
    Hydruino controller;
    controller.init();

    auto pump = controller.addWaterPumpRelay(5);
    assert(pump);
    assert(pump->getOutputPin().isValid());

    HydroCalibrationData calibration(pump->getId(), Hydro_UnitsType_Raw_1);
    calibration.setFromTwoPoints(0.0f, 0.0f, 1.0f, 1.0f);
    pump->setUserCalibrationData(&calibration);
    assert(pump->getUserCalibrationData() != nullptr);
    controller.clearUserCalibrations();
    assert(pump->getUserCalibrationData() == nullptr);

    pump->setEnableMode(Hydro_EnableMode_Highest);
    HydroActivationHandle handle = pump->enableActuator(Hydro_DirectionMode_Forward, 0.5f, (millis_t)-1, true);
    pump->update();
    assert(pump->isEnabled());

    handle.unset();
    pump->update();
    assert(!pump->isEnabled());

    auto source = controller.addFreshWaterMain();
    auto destination = controller.addFeedWaterReservoir(1000.0f, false);
    assert(source && destination);

    pump->setSourceReservoir(source);
    pump->setDestinationReservoir(destination);
    assert(pump->getSourceReservoirAttachment().getKey() == source->getKey());
    assert(pump->getDestinationReservoirAttachment().getKey() == destination->getKey());

    std::cout << "PASS Hydruino hardware" << std::endl;
    return 0;
}
