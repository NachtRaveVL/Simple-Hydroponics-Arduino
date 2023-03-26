/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    tcMenu Liquid Crystal Display
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#include "HydruinoUI.h"

/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/* Changelist:
 * - Changed from application.name to constructed appTitle parameter
 * - Put length limits into renderTitle & now using internal appTitle
 * - Enclosed inside of #ifdef & reorg'ed for general inclusion
 */

/**
 * @file tcMenu_Display_LiquidCrystal.h
 * 
 * LiquidCrystalIO renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 * 
 * LIBRARY REQUIREMENT
 * This renderer is designed for use with this library: https://github.com/davetcc/LiquidCrystalIO
 */

#ifndef _TCMENU_LIQUID_CRYSTAL_H
#define _TCMENU_LIQUID_CRYSTAL_H

#include <LiquidCrystalIO.h>

#include "tcMenu.h"
#include "BaseRenderers.h"
#include <BaseDialog.h>

/**
 * A renderer that can renderer onto a LiquidCrystal display and supports the concept of single level
 * sub menus, active items and editing.
 */
class LiquidCrystalRenderer : public BaseMenuRenderer {
private:
    LiquidCrystal* lcd;
    uint8_t dimY;
    const char *appTitle;
    uint8_t backChar;
    uint8_t forwardChar;
    uint8_t editChar;
    bool drewTitleThisTime;
    bool titleRequired;
    uint8_t lcdEditorCursorX = 0xFF;
    uint8_t lcdEditorCursorY = 0xFF;
public:

    LiquidCrystalRenderer(LiquidCrystal& lcd, uint8_t dimX, uint8_t dimY, const char *appTitle);
    virtual ~LiquidCrystalRenderer();
    void render() override;
    void initialise() override;
    void setTitleRequired(bool titleRequired) { this->titleRequired = titleRequired; }

    void setEditorChars(char back, char forward, char edit);

    uint8_t getRows() {return dimY;}
    LiquidCrystal* getLCD() {return lcd;}
    BaseDialog* getDialog() override;
private:
    void renderTitle(bool forceDraw);
    void renderMenuItem(uint8_t row, MenuItem* item);
    void renderActionItem(uint8_t row, MenuItem* item);
    void renderBackItem(uint8_t row, MenuItem* item);
    void renderList();
    void setupEditorPlacement(int32_t x, int32_t y);
};

class LiquidCrystalDialog : public BaseDialog {
public:
    LiquidCrystalDialog(LiquidCrystalRenderer* renderer) {
        bitWrite(flags, DLG_FLAG_SMALLDISPLAY, (renderer->getRows() <= 2));
    }
protected:
    void internalRender(int currentValue) override;
};

/**
 * This method constructs an instance of a liquid crystal renderer.
 */
inline MenuRenderer* liquidCrystalRenderer(LiquidCrystal& lcd, uint8_t dimX, uint8_t dimY, const char *appTitle) {
    return new LiquidCrystalRenderer(lcd, dimX, dimY, appTitle);
}

#endif // _TCMENU_LIQUID_CRYSTAL_H
#endif
