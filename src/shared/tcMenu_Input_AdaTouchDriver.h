/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    tcMenu Adafruit FT2606 & XPT2046 Touchscreen
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"

/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/* Changelist:
 * - Refactored to have screen size given in init
 * - Split into proper header/source file
 * - Enclosed inside of #ifdef & reorg'ed for general inclusion
 */

/**
 * @file tcMenu_Input_AdaTouchDriver.h
 * 
 * Touch integration for libraries that are compatible with the Adafruit_FT2606 library interface. It has been tested
 * with both the XPT_2046 and FT_6206 libraries.
 * This file is a plugin file and should not be directly edited, it will be replaced each time the project
 * is built. If you want to edit this file in place, make sure to rename it first.
 */

#ifndef TCMENU_TOUCH_PLUGIN_H
#define TCMENU_TOUCH_PLUGIN_H

#ifndef HYDRO_UI_ENABLE_XPT2046TS
#include <Adafruit_FT6206.h>
#else
#include <XPT2046_Touchscreen.h>
#endif
#include <ResistiveTouchScreen.h>

namespace iotouch {

    /**
     * Implements the touch interrogator class, this purely gets the current reading from the device when requested.
     */
    class AdaLibTouchInterrogator : public iotouch::TouchInterrogator {
    private:
        #ifndef HYDRO_UI_ENABLE_XPT2046TS
            Adafruit_FT6206& theTouchDevice;
        #else
            XPT2046_Touchscreen& theTouchDevice;
        #endif
        uint16_t maxWidthDim;
        uint16_t maxHeightDim;
    public:
        #ifndef HYDRO_UI_ENABLE_XPT2046TS
            AdaLibTouchInterrogator(Adafruit_FT6206& touchLibRef);
        #else
            AdaLibTouchInterrogator(XPT2046_Touchscreen& touchLibRef);
        #endif

        inline void init(uint16_t xMax, uint16_t yMax) { maxWidthDim = xMax; maxHeightDim = yMax; theTouchDevice.begin(); }
        iotouch::TouchState internalProcessTouch(float *ptrX, float *ptrY, const iotouch::TouchOrientationSettings& rotation, const iotouch::CalibrationHandler& calib);
    };

}

#endif // TCMENU_TOUCH_PLUGIN_H
#endif
