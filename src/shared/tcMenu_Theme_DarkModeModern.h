/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    tcMenu Dark Mode Modern/ML Theme
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI

/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/* Changelist:
 * - Variable name standardizations and keeping things in function scope.
 * - Enclosed inside of #ifdef & reorg'ed for general inclusion
 */

/**
 * Dark mode modern theme by TheCodersCorner.com. This is part of the standard themes shipped with TcMenu.
 * This file will not be updated by the designer, you can edit.
 */
#ifndef DARK_MODE_MODERN_THEME
#define DARK_MODE_MODERN_THEME

#include <graphics/BaseGraphicalRenderer.h>

#define ACTION_BORDER_WIDTH 0

inline void installDarkModeModernTheme(GraphicsDeviceRenderer& bgr, const MenuFontDef& itemFont, const MenuFontDef& titleFont, bool needEditingIcons) {
    const color_t darkModeMdrnTitlePalette[] = {RGB(255,255,255), RGB(43,43,43), RGB(192,192,192), RGB(0,133,255)};
    const color_t darkModeMdrnItemPalette[] = {RGB(255, 255, 255), RGB(0,0,0), RGB(43,43,43), RGB(65,65,65)};
    const color_t darkModeMdrnActionPalette[] = {RGB(255, 255, 255), RGB(35,35,35), RGB(20,45,110), RGB(192,192,192)};

    // here we get a reference to the drawable and then set the dimensions.
    auto* rootDrawable = bgr.getDeviceDrawable();
    bgr.setDisplayDimensions(rootDrawable->getDisplayDimensions().x, rootDrawable->getDisplayDimensions().y);

    // we need a reference to the factory object that we will use to configure the drawing.
    auto& factory = bgr.getGraphicsPropertiesFactory();

    // change the selected colours.
    factory.setSelectedColors(RGB(46, 66, 161), RGB(255, 255, 255));

    // for this theme we use the same size padding for each case, we need touchable items. We calculate the height too
    MenuPadding allPadding(4, 3, 4, 3);
    int titleHeight = bgr.heightForFontPadding(titleFont.fontData, titleFont.fontMag, allPadding);
    int itemHeight = bgr.heightForFontPadding(itemFont.fontData, itemFont.fontMag, allPadding);

    // now we configure the drawing for each item type
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, darkModeMdrnTitlePalette, allPadding, titleFont.fontData, titleFont.fontMag, 3, titleHeight,
                                        GridPosition::JUSTIFY_CENTER_WITH_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, darkModeMdrnItemPalette, allPadding, itemFont.fontData, itemFont.fontMag, 2, itemHeight,
                                        GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT , MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, darkModeMdrnActionPalette, allPadding, itemFont.fontData, itemFont.fontMag, 2, itemHeight,
                                        GridPosition::JUSTIFY_CENTER_WITH_VALUE, MenuBorder(ACTION_BORDER_WIDTH));

    // and lastly, whenever changing the configuration, we must refresh.
    tcgfx::ConfigurableItemDisplayPropertiesFactory::refreshCache();
}

#endif //DARK_MODE_MODERN_THEME
#endif
