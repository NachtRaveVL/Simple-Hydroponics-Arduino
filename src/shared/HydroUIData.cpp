/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino UI Data
*/

#include "Hydruino.h"
#include "HydroUIData.h"

HydroUIData::HydroUIData()
    : HydroData('H','U','I','D', 1),
      updatesPerSec(HYDRO_UI_UPDATE_SPEED), displayTheme(Hydro_DisplayTheme_Undefined), joystickCalib{0.5f,0.5f,0.05f}
{
    _size = sizeof(*this);
}

void HydroUIData::toJSONObject(JsonObject &objectOut) const
{
    HydroData::toJSONObject(objectOut);

    if (updatesPerSec != HYDRO_UI_UPDATE_SPEED) { objectOut[SFP(HStr_Key_UpdatesPerSec)] = updatesPerSec; }
    if (displayTheme != Hydro_DisplayTheme_Undefined) { objectOut[SFP(HStr_Key_DisplayTheme)] = displayTheme; }
    if (!isFPEqual(joystickCalib[0], 0.5f) || !isFPEqual(joystickCalib[1], 0.5f) || !isFPEqual(joystickCalib[2], 0.05f)) { objectOut[SFP(HStr_Key_JoystickCalib)] = commaStringFromArray(joystickCalib, 3); }
}

void HydroUIData::fromJSONObject(JsonObjectConst &objectIn)
{
    HydroData::fromJSONObject(objectIn);

    updatesPerSec = objectIn[SFP(HStr_Key_UpdatesPerSec)] | updatesPerSec;
    displayTheme = objectIn[SFP(HStr_Key_DisplayTheme)] | displayTheme;
    JsonVariantConst joystickCalibVar = objectIn[SFP(HStr_Key_JoystickCalib)];
    commaStringToArray(joystickCalibVar, joystickCalib, 3);
}
