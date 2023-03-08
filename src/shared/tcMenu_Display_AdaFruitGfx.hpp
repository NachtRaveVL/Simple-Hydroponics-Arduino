/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    tcMenu Adafruit GFX Display
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI

/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * Adafruit_GFX renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 * 
 * LIBRARY REQUIREMENT
 * This library requires the AdaGfx library along with a suitable driver.
 */

#include "tcMenu_Display_AdaFruitGfx.h"
#include <ScrollChoiceMenuItem.h>

template <class T>
void AdafruitDrawable<T>::transaction(bool isStarting, bool redrawNeeded) {
#if DISPLAY_HAS_MEMBUFFER == true
    if(!isStarting && redrawMeeded) getGfx()->display();
#endif
}

template <class T>
void AdafruitDrawable<T>::internalDrawText(const Coord &where, const void *font, int mag, const char *sz) {
    graphics->setTextWrap(false);
    int baseline=0;
    Coord exts = textExtents(font, mag, "(;y", &baseline);
    int yCursor = font ? (where.y + (exts.y - baseline)) : where.y;
    graphics->setCursor(where.x, yCursor);
    graphics->setTextColor(drawColor);
    graphics->print(sz);
}

template <class T>
void AdafruitDrawable<T>::drawBitmap(const Coord &where, const DrawableIcon *icon, bool selected) {
    if(icon->getIconType() == DrawableIcon::ICON_XBITMAP) {
        graphics->fillRect(where.x, where.y, icon->getDimensions().x, icon->getDimensions().y, backgroundColor);
        graphics->drawXBitmap(where.x, where.y, icon->getIcon(selected), icon->getDimensions().x, icon->getDimensions().y, drawColor);
    }
    else if(icon->getIconType() == DrawableIcon::ICON_NATIVE) {
        graphics->drawRGBBitmap(where.x, where.y, (const uint16_t*)icon->getIcon(selected), icon->getDimensions().x, icon->getDimensions().y);
    }
    else if(icon->getIconType() == DrawableIcon::ICON_MONO) {
        graphics->drawBitmap(where.x, where.y, icon->getIcon(selected), icon->getDimensions().x, icon->getDimensions().y, drawColor, backgroundColor);
    }
}

template <class T>
void AdafruitDrawable<T>::drawXBitmap(const Coord &where, const Coord &size, const uint8_t *data) {
    graphics->fillRect(where.x, where.y, size.x, size.y, backgroundColor);
    graphics->drawXBitmap(where.x, where.y, data, size.x, size.y, drawColor);
}

template <class T>
void AdafruitDrawable<T>::drawBox(const Coord &where, const Coord &size, bool filled) {
    if(filled) {
        graphics->fillRect(where.x, where.y, size.x, size.y, drawColor);
    }
    else {
        graphics->drawRect(where.x, where.y, size.x, size.y, drawColor);
    }
}

template <class T>
void AdafruitDrawable<T>::drawCircle(const Coord& where, int radius, bool filled) {
    if(filled) {
        graphics->fillCircle(where.x, where.y, radius, drawColor);
    }
    else {
        graphics->drawCircle(where.x, where.y, radius, drawColor);
    }
}

template <class T>
void AdafruitDrawable<T>::drawPolygon(const Coord points[], int numPoints, bool filled) {
    if(numPoints == 2) {
        graphics->drawLine(points[0].x, points[0].y, points[1].x, points[1].y, drawColor);
    }
    else if(numPoints == 3) {
        if(filled) {
            graphics->fillTriangle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, drawColor);
        }
        else {
            graphics->drawTriangle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, drawColor);
        }
    }
}

template <class T>
Coord AdafruitDrawable<T>::internalTextExtents(const void *f, int mag, const char *text, int *baseline) {
    graphics->setFont(static_cast<const GFXfont *>(f));
    graphics->setTextSize(mag);
    auto* font = (GFXfont *) f;
    int16_t x1, y1;
    uint16_t w, h;
    graphics->getTextBounds((char*)text, 3, font?30:2, &x1, &y1, &w, &h);

    if(font == nullptr) {
        // for the default font, the starting offset is 0, and we calculate the height.
        if(baseline) *baseline = 0;
        return Coord(w, h);
    }
    else {
        // we need to work out the biggest glyph and maximum extent beyond the baseline, we use 'Ay(' for this
        const char sz[] = "AgyjK(";
        int height = 0;
        int bl = 0;
        const char* current = sz;
        auto fontLast = pgm_read_word(&font->last);
        auto fontFirst = pgm_read_word(&font->first);
        while(*current && (*current < fontLast)) {
            size_t glIdx = *current - fontFirst;
            auto allGlyphs = (GFXglyph*)pgm_read_ptr(&font->glyph);
            unsigned char glyphHeight = pgm_read_byte(&allGlyphs[glIdx].height);
            if (glyphHeight > height) height = glyphHeight;
            bl = glyphHeight + (signed char)pgm_read_byte(&allGlyphs[glIdx].yOffset);
            current++;
        }
        if(baseline) *baseline = bl;
        return Coord((int)w, height);
    }
}

template <class T>
void AdafruitDrawable<T>::drawPixel(uint16_t x, uint16_t y) {
    graphics->writePixel(x, y, drawColor);
}

template <class T>
UnicodeFontHandler *AdafruitDrawable<T>::createFontHandler() {
    return new UnicodeFontHandler(graphics, ENCMODE_UTF8);
}

template <class T>
DeviceDrawable *AdafruitDrawable<T>::getSubDeviceFor(const Coord &where, const Coord& size, const color_t *palette, int paletteSize) {
    if(spriteHeight != 0 && canvasDrawable == nullptr) canvasDrawable = new AdafruitCanvasDrawable2bpp<T>(this, graphics->width(), spriteHeight);
    if(!canvasDrawable) return nullptr;

    return (canvasDrawable->initSprite(where, size, palette, paletteSize)) ? canvasDrawable : nullptr;
}

template <class T>
AdafruitCanvasDrawable2bpp<T>::AdafruitCanvasDrawable2bpp(AdafruitDrawable<T>* root,  int width, int height) : root(root),
            sizeMax({width, height}), sizeCurrent(), palette{} {
    canvas = new TcGFXcanvas2(width, height);
    AdafruitDrawable<T>::setGraphics(canvas);
}

template <class T>
void AdafruitCanvasDrawable2bpp<T>::transaction(bool isStarting, bool redrawNeeded) {
    if (!isStarting) {
        // if it's ending, we push the canvas onto the display.
        drawCookieCutBitmap2bpp((Adafruit_SPITFT*)root->getGfx(), where.x, where.y, canvas->getBuffer(), sizeCurrent.x, sizeCurrent.y,
                                canvas->width(), 0, 0, palette);
    }
}

template <class T>
bool AdafruitCanvasDrawable2bpp<T>::initSprite(const Coord& spriteWhere, const Coord& spriteSize, const color_t* colPalette, size_t paletteSize) {
    if(!canvas->reInitCanvas(spriteSize.x, spriteSize.y)) {
        return false;
    }
    where = spriteWhere;
    sizeCurrent = spriteSize;
    if(paletteSize > 4) paletteSize = 4;
    for(size_t i=0; i<paletteSize; i++) {
        palette[i] = colPalette[i];
    }

    if(root->isTcUnicodeEnabled()) {
        this->enableTcUnicode();
    }
    return true;
}

template <class T>
color_t AdafruitCanvasDrawable2bpp<T>::getUnderlyingColor(color_t col) {
    for(int i=0; i<4; i++) {
        if(palette[i] == col) return i;
    }

    return 0;
}

template <class T>
DeviceDrawable *AdafruitCanvasDrawable2bpp<T>::getSubDeviceFor(const Coord&, const Coord&, const color_t *, int) {
    return nullptr; // don't allow further nesting.
}

#endif
