/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    tcMenu Adafruit FT2606 Touchscreen
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI

/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/* Changelist:
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

#ifndef HYDRO_ENABLE_XPT2046TS
#include <Adafruit_FT6206.h>
#else
#include <XPT2046_Touchscreen.h>
#endif
#include <ResistiveTouchScreen.h>

//
// This is the known absolute maximum value of the touch unit before calibration. It will become 1.0F
//
#ifndef HYDRO_ENABLE_XPT2046TS
#define KNOWN_DEVICE_TOUCH_RANGE_X 240.0F
#define KNOWN_DEVICE_TOUCH_RANGE_Y 320.0F
#else
#define KNOWN_DEVICE_TOUCH_RANGE_X 4096.0F
#define KNOWN_DEVICE_TOUCH_RANGE_Y 4096.0F
#endif

namespace iotouch {

    /**
     * Implements the touch interrogator class, this purely gets the current reading from the device when requested.
     */
    class AdaLibTouchInterrogator : public iotouch::TouchInterrogator {
    private:
        #ifndef HYDRO_ENABLE_XPT2046TS
            Adafruit_FT6206& theTouchDevice;
        #else
            XPT2046_Touchscreen& theTouchDevice;
        #endif
    public:
        #ifndef HYDRO_ENABLE_XPT2046TS
            AdaLibTouchInterrogator(Adafruit_FT6206& touchLibRef) : theTouchDevice(touchLibRef) {}
        #else
            AdaLibTouchInterrogator(XPT2046_Touchscreen& touchLibRef) : theTouchDevice(touchLibRef) {}
        #endif

        void init() {
            theTouchDevice.begin();
        }

        iotouch::TouchState internalProcessTouch(float *ptrX, float *ptrY, const iotouch::TouchOrientationSettings& rotation, const iotouch::CalibrationHandler& calib) {
            if(theTouchDevice.touched() == 0) return iotouch::NOT_TOUCHED;

            TS_Point pt = theTouchDevice.getPoint();
            //serdebugF3("point at ", pt.x, pt.y);

            *ptrX = calib.calibrateX(float(pt.x) / KNOWN_DEVICE_TOUCH_RANGE_X, rotation.isXInverted());
            *ptrY = calib.calibrateY(float(pt.y) / KNOWN_DEVICE_TOUCH_RANGE_Y, rotation.isYInverted());
            return iotouch::TOUCHED;
        }
    };

}

#endif // TCMENU_TOUCH_PLUGIN_H
#endif
