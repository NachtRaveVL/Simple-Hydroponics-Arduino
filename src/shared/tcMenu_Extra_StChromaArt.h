/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    tcMenu STM32 LDTC Frame Buffer & BSP Touch
*/

#include <Hydruino.h>
#include "HydroUIDefines.h"
#if defined(HYDRO_USE_GUI) && (defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_STM32)) && (defined(HYDRO_UI_ENABLE_STM32_LDTC) || defined(HYDRO_UI_ENABLE_BSP_TOUCH))

/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/* Changelist:
 * - Enclosed inside of #ifdef & reorg'ed for general inclusion
 */

/**
 * @file tcMenu_Extra_StChromaArt.h
 * 
 * This renderer works with the ChromaART classes provided on many ST boards, it assumes you have a object that
 * represents the frame buffer.
 * 
 * LIBRARY REQUIREMENT
 * This library requires the u8g2 library available for download from your IDE library manager.
 */

#ifndef TCMENU_ST_CHROMA_ART_H
#define TCMENU_ST_CHROMA_ART_H

#include <tcUtil.h>
#include <graphics/BaseGraphicalRenderer.h>
#include <graphics/GraphicsDeviceRenderer.h>
#include <BaseDialog.h>
#include <tcUtil.h>
#include <ResistiveTouchScreen.h>
#include <tcUnicodeHelper.h>
#include "tcMenu_Extra_BspUserSettings.h"

using namespace tcgfx;

// some colour displays don't create this value
#ifndef BLACK
#define BLACK 0
#endif

// some colour displays don't create this value
#ifndef WHITE
#define WHITE 0xffff
#endif

/**
 * A renderer that can work with BSP_LCD displays for mbed such as the STM32F429 discovery display prototype. You
 * can use this as a starting point for a chroma art based display renderer. It provides some extended support
 * by providing font capabilities from the Adafruit Graphics libraries.
 */
class StChromaArtDrawable : public DeviceDrawable {
public:
    explicit StChromaArtDrawable();
    ~StChromaArtDrawable() override = default;

    DeviceDrawable* getSubDeviceFor(const Coord &where, const Coord &size, const color_t *palette, int paletteSize) override {return nullptr; }

    void internalDrawText(const Coord &where, const void *font, int mag, const char *text) override;
    void drawBitmap(const Coord &where, const DrawableIcon *icon, bool selected) override;
    void drawXBitmap(const Coord &where, const Coord &size, const uint8_t *data) override;
    void drawBox(const Coord &where, const Coord &size, bool filled) override;
    void drawCircle(const Coord &where, int radius, bool filled) override;
    void drawPolygon(const Coord *points, int numPoints, bool filled) override;

    Coord getDisplayDimensions() override;
    void transaction(bool isStarting, bool redrawNeeded) override;
    Coord internalTextExtents(const void *font, int mag, const char *text, int *baseline) override;
    void drawPixel(uint16_t x, uint16_t y) override { BSP_LCD_DrawPixel(x, y, drawColor); }
};

class StBspTouchInterrogator : public iotouch::TouchInterrogator {
private:
    int width, height;
public:
    StBspTouchInterrogator(int wid, int hei);
    iotouch::TouchState internalProcessTouch(float *ptrX, float *ptrY, const iotouch::TouchOrientationSettings& rotation,
                                             const iotouch::CalibrationHandler& calib) override;
};

#endif // TCMENU_ST_CHROMA_ART_H
#endif
