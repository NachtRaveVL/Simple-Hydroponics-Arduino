/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino UI Data
*/

#ifndef HydroUIData_H
#define HydroUIData_H

struct HydroUIData;

#include <Hydruino.h>
#include "HydroUIDefines.h"

// UI Serialization Data
// id: HUID. Hydruino UI data.
struct HydroUIData : public HydroData {
    uint8_t updatesPerSec;                                  // Updates per second (1-10, default: HYDRO_UI_UPDATE_SPEED)
    Hydro_DisplayTheme displayTheme;                        // Display theme (if supported)
    Hydro_TitleMode titleMode;                              // Title mode
    bool analogSlider;                                      // Use analog slider
    bool editingIcons;                                      // Use editing icons
    float joystickCalib[3];                                 // Joystick calibration ({midX,midY,zeroTol}, default: {0.5,0.5,0.05})
    uint16_t touchscreenCalib[4];                           // Touchscreen calibration ({x0,y0,x1,y1}), default: {0,0,0,0})

    HydroUIData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef HydroUIData_H
