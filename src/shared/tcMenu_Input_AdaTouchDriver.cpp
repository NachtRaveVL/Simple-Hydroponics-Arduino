/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    tcMenu Adafruit FT2606 & XPT2046 Touchscreen
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI

/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu_Input_AdaTouchDriver.h"

#ifndef HYDRO_UI_ENABLE_XPT2046TS
iotouch::AdaLibTouchInterrogator::AdaLibTouchInterrogator(Adafruit_FT6206& touchLibRef)
    : theTouchDevice(touchLibRef), maxWidthDim(0), maxHeightDim(0) {}
#else
iotouch::AdaLibTouchInterrogator::AdaLibTouchInterrogator(XPT2046_Touchscreen& touchLibRef)
    : theTouchDevice(touchLibRef) {}
#endif

iotouch::TouchState iotouch::AdaLibTouchInterrogator::internalProcessTouch(float *ptrX, float *ptrY, const iotouch::TouchOrientationSettings& rotation, const iotouch::CalibrationHandler& calib) {
    if(theTouchDevice.touched() == 0) return iotouch::NOT_TOUCHED;

    TS_Point pt = theTouchDevice.getPoint();
    //serdebugF3("point at ", pt.x, pt.y);

    #ifndef HYDRO_UI_ENABLE_XPT2046TS
        *ptrX = calib.calibrateX(float(pt.x) / maxWidthDim, rotation.isXInverted());
        *ptrY = calib.calibrateY(float(pt.y) / maxHeightDim, rotation.isYInverted());
    #else
        *ptrX = calib.calibrateX(float(pt.x) / XPT2046_RAW_MAX, rotation.isXInverted());
        *ptrY = calib.calibrateY(float(pt.y) / XPT2046_RAW_MAX, rotation.isYInverted());
    #endif
    return iotouch::TOUCHED;
}

#endif