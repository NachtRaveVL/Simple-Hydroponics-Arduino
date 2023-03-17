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

#ifndef COOKIE_CUT_MEMBUFFER_SIZE
#ifdef __AVR__
#define COOKIE_CUT_MEMBUFFER_SIZE 16
#else
#define COOKIE_CUT_MEMBUFFER_SIZE 32
#endif // AVR reduced size buffer
#endif // COOKIE_CUT_MEMBUFFER_SIZE

//
// helper functions
//

uint16_t memBuffer[COOKIE_CUT_MEMBUFFER_SIZE];

void drawCookieCutBitmap(Adafruit_SPITFT* gfx, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
                         int16_t h, int16_t totalWidth, int16_t xStart, int16_t yStart,
                         uint16_t fgColor, uint16_t bgColor) {
    // total width here is different to the width we are drawing, imagine rolling out a long
    // line of dough and cutting cookies from it. The cookie is the part of the image we want
    uint16_t byteWidth = (totalWidth + 7) / 8; // Bitmap scanline pad = whole byte
    uint16_t yEnd = h + yStart;
    uint16_t xEnd = w + xStart;
    uint8_t byte;

    gfx->startWrite();
    int next = 0;

    for (uint16_t j = yStart; j < yEnd; j++, y++) {
        byte = bitmap[size_t(((j * byteWidth) + xStart) / 8)];
        gfx->setAddrWindow(x, j, w, 1);
        for (uint16_t i = xStart; i < xEnd; i++) {
            if (i & 7U)
                byte <<= 1U;
            else
                byte = bitmap[size_t((j * byteWidth) + i / 8)];

            memBuffer[next] = (byte & 0x80U) ? fgColor : bgColor;
            next = next + 1;
            if(next == COOKIE_CUT_MEMBUFFER_SIZE) {
                gfx->writePixels(memBuffer, next);
                next = 0;
            }
        }
        if(next != 0) {
            gfx->writePixels(memBuffer, next);
            next = 0;
        }
    }

    gfx->endWrite();
}

void drawCookieCutBitmap(Adafruit_GFX* gfx, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
                         int16_t h, int16_t totalWidth, int16_t xStart, int16_t yStart,
                         uint16_t fgColor, uint16_t bgColor) {

    // total width here is different to the width we are drawing, imagine rolling out a long
    // line of dough and cutting cookies from it. The cookie is the part of the image we want
    uint16_t byteWidth = (totalWidth + 7) / 8; // Bitmap scanline pad = whole byte
    uint16_t yEnd = h + yStart;
    uint16_t xEnd = w + xStart;
    uint8_t byte;

    gfx->startWrite();

    for (uint16_t j = yStart; j < yEnd; j++, y++) {
        byte = bitmap[size_t(((j * byteWidth) + xStart) / 8)];
        for (uint16_t i = xStart; i < xEnd; i++) {
            if (i & 7U)
                byte <<= 1U;
            else
                byte = bitmap[size_t((j * byteWidth) + i / 8)];
            gfx->writePixel(x + (i - xStart), y, (byte & 0x80U) ? fgColor : bgColor);
        }
    }

    gfx->endWrite();
}

void drawCookieCutBitmap2bpp(Adafruit_SPITFT* gfx, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
                             int16_t h, int16_t totalWidth, int16_t xStart, int16_t yStart,
                             const uint16_t* palette) {
    // total width here is different to the width we are drawing, imagine rolling out a long
    // line of dough and cutting cookies from it. The cookie is the part of the image we want
    uint16_t byteWidth = (totalWidth + 3) / 4; // Bitmap scanline pad = whole byte
    uint16_t yEnd = h + yStart;
    uint16_t xEnd = w + xStart;
    uint8_t byte;

    gfx->startWrite();

    int next = 0;

    for (uint16_t j = yStart; j < yEnd; j++, y++) {
        byte = bitmap[(j * byteWidth) + (xStart / 4)];
        gfx->setAddrWindow(x, y, w, 1);
        for (uint16_t i = xStart; i < xEnd; i++) {
            if((i & 3) == 0) {
                byte = bitmap[(j * byteWidth) + (i / 4)];
            }
            auto offset = (i & 3) << 1;
            auto color = (byte >> offset) & 3;
            memBuffer[next] = palette[color];
            next = next + 1;
            if(next == COOKIE_CUT_MEMBUFFER_SIZE) {
                gfx->writePixels(memBuffer, next);
                next = 0;
            }
        }
        if(next != 0) {
            gfx->writePixels(memBuffer, next);
            next = 0;
        }
    }

    gfx->endWrite();
}

//
// TcGFXcanvas2 - the 2bit graphics canvas class
//

#define PIXELS_PER_BYTE 4
#define PIXELS_PER_BYTE_ROUNDING 3
#define POSITION_IN_BUFFER(x,y) (&buffer[((x) / PIXELS_PER_BYTE) + ((y) * ((WIDTH + PIXELS_PER_BYTE_ROUNDING) / PIXELS_PER_BYTE))])
#define SHIFT_PIXEL(x, c) (((c) & 3) << (((x) & 3) << 1))

uint8_t bitsOffMask[] = { 0xFc, 0xF3, 0xcF, 0x3f };

TcGFXcanvas2::TcGFXcanvas2(uint16_t w, uint16_t h): Adafruit_GFX((int16_t)w, (int16_t)h) {
    size_t byteCount = getByteCount();
    if ((buffer = new uint8_t[byteCount])) {
        memset(buffer, 0, byteCount);
        maxBytesAvailable = byteCount;
    }
}

TcGFXcanvas2::~TcGFXcanvas2() {
    delete[] buffer;
}

bool TcGFXcanvas2::reInitCanvas(int w, int h) {
    // first check we can allocate this buffer
    size_t bytesNeededForThisBuffer = (((w + 3) / 4) * h) * 2;
    if(bytesNeededForThisBuffer >= maxBytesAvailable) {
        return false;
    }

    // now reset the width and height to the new arrangements.
    WIDTH = w;
    HEIGHT = h;
    _width = WIDTH;
    _height = HEIGHT;
    rotation = 0;
    cursor_y = cursor_x = 0;
    textsize_x = textsize_y = 1;
    return true;
}

void TcGFXcanvas2::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if(!buffer) return;

    if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
        return;

    int16_t t;
    switch (rotation) {
        case 1:
            t = x;
            x = WIDTH - 1 - y;
            y = t;
            break;
        case 2:
            x = WIDTH - 1 - x;
            y = HEIGHT - 1 - y;
            break;
        case 3:
            t = x;
            x = y;
            y = HEIGHT - 1 - t;
            break;
    }

    uint8_t *ptr = POSITION_IN_BUFFER(x, y);
    int bitOffset = (x & 3);
    *ptr &= bitsOffMask[bitOffset];
    *ptr |= SHIFT_PIXEL(x, color);
}

uint8_t makeColorToByte(uint16_t color) {
    uint8_t col = (color & 3);

    uint8_t ret =  col;

    col = col << 2;
    ret |= col;

    col = col << 2;
    ret |= col;

    col = col << 2;
    ret |= col;

    return ret;
}

void TcGFXcanvas2::fillScreen(uint16_t color) {
    if (!buffer) return;
    uint8_t col = makeColorToByte(color);
    memset(buffer, col, getByteCount());
}

void TcGFXcanvas2::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (h < 0) { // Convert negative heights to positive equivalent
        h *= -1;
        y -= h - 1;
        if (y < 0) {
            h += y;
            y = 0;
        }
    }

    // Edge rejection (no-draw if totally off canvas)
    if ((x < 0) || (x >= width()) || (y >= height()) || ((y + h - 1) < 0)) {
        return;
    }

    if (y < 0) { // Clip top
        h += y;
        y = 0;
    }
    if (y + h > height()) { // Clip bottom
        h = height() - y;
    }

    if (getRotation() == 0) {
        drawFastRawVLine(x, y, h, color);
    } else if (getRotation() == 1) {
        int16_t t = x;
        x = WIDTH - 1 - y;
        y = t;
        x -= h - 1;
        drawFastRawHLine(x, y, h, color);
    } else if (getRotation() == 2) {
        x = WIDTH - 1 - x;
        y = HEIGHT - 1 - y;

        y -= h - 1;
        drawFastRawVLine(x, y, h, color);
    } else if (getRotation() == 3) {
        int16_t t = x;
        x = y;
        y = HEIGHT - 1 - t;
        drawFastRawHLine(x, y, h, color);
    }
}

void TcGFXcanvas2::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (w < 0) { // Convert negative widths to positive equivalent
        w *= -1;
        x -= w - 1;
        if (x < 0) {
            w += x;
            x = 0;
        }
    }

    // Edge rejection (no-draw if totally off canvas)
    if ((y < 0) || (y >= height()) || (x >= width()) || ((x + w - 1) < 0)) {
        return;
    }

    if (x < 0) { // Clip left
        w += x;
        x = 0;
    }
    if (x + w >= width()) { // Clip right
        w = width() - x;
    }

    if (getRotation() == 0) {
        drawFastRawHLine(x, y, w, color);
    } else if (getRotation() == 1) {
        int16_t t = x;
        x = WIDTH - 1 - y;
        y = t;
        drawFastRawVLine(x, y, w, color);
    } else if (getRotation() == 2) {
        x = WIDTH - 1 - x;
        y = HEIGHT - 1 - y;

        x -= w - 1;
        drawFastRawHLine(x, y, w, color);
    } else if (getRotation() == 3) {
        int16_t t = x;
        x = y;
        y = HEIGHT - 1 - t;
        y -= w - 1;
        drawFastRawVLine(x, y, w, color);
    }
}

uint8_t TcGFXcanvas2::getPixel(int16_t x, int16_t y) const {
    int16_t t;
    switch (rotation) {
        case 1:
            t = x;
            x = WIDTH - 1 - y;
            y = t;
            break;
        case 2:
            x = WIDTH - 1 - x;
            y = HEIGHT - 1 - y;
            break;
        case 3:
            t = x;
            x = y;
            y = HEIGHT - 1 - t;
            break;
    }
    return getRawPixel(x, y);
}

uint8_t TcGFXcanvas2::getRawPixel(int16_t x, int16_t y) const {
    if ((x < 0) || (y < 0) || (x >= WIDTH) || (y >= HEIGHT)) return 0;

    if (buffer) {
        uint8_t *ptr = POSITION_IN_BUFFER(x, y);
        auto col = *ptr >> ((x & 3) << 1);
        return col & 3;
    }
    return 0;
}

void TcGFXcanvas2::drawFastRawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    // x & y already in raw (rotation 0) coordinates, no need to transform.
    uint8_t *ptr = POSITION_IN_BUFFER(x, y);
    size_t rowBytes = (WIDTH + PIXELS_PER_BYTE_ROUNDING) / PIXELS_PER_BYTE;

    uint8_t bitMaskReset = bitsOffMask[x & 3];
    uint8_t colorBits = SHIFT_PIXEL(x, color);
    for (int16_t i = 0; i < h; i++) {
        *ptr &= bitMaskReset;
        *ptr |= colorBits;
        ptr += rowBytes;
    }
}

void TcGFXcanvas2::drawFastRawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    // x & y already in raw (rotation 0) coordinates, no need to transform.
    uint8_t *ptr = POSITION_IN_BUFFER(x, y);
    size_t remainingWidthBits = w;

    // check to see if first byte needs to be partially filled
    if ((x & 3) > 0) {
        // create bit mask for first byte
        uint8_t startByteBitReset = 0xFF;
        uint8_t startByteBitSet = 0x00;

        for (auto i = int8_t(x & 3); ((i < 4) && (remainingWidthBits > 0)); i++) {
            startByteBitReset &= bitsOffMask[i];
            startByteBitSet = SHIFT_PIXEL(x, color);
            remainingWidthBits--;
        }
        *ptr &= startByteBitReset;
        *ptr |= startByteBitSet;
        ptr++;
    }

    // do the next remainingWidthBits bits
    if (remainingWidthBits > 0) {
        size_t remainingWholeBytes = remainingWidthBits / 4;
        size_t lastByteBits = remainingWidthBits % 4;

        memset(ptr, makeColorToByte(color), remainingWholeBytes);

        if (lastByteBits > 0) {
            uint8_t startByteBitReset = 0xFF;
            uint8_t startByteBitSet = 0x00;
            for (size_t i = 0; i < lastByteBits; i++) {
                startByteBitReset &= bitsOffMask[3 - i];
                startByteBitSet = SHIFT_PIXEL((3 - i), color);
            }
            ptr += remainingWholeBytes;

            *ptr &= ~startByteBitReset;
            *ptr |= startByteBitSet;
        }
    }
}

#endif
