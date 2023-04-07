/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino AdafruitGFX Overview Screen
*/

#include "../HydruinoUI.h"
#ifdef HYDRO_USE_GUI

float skyEaseInOut(float x) {
    return x < 0.5f ? 2.0f * x * x : 1.0f - ((-2.0f * x + 2.0f) * (-2.0f * x + 2.0f) * 0.5f);
}

void randomStarColor(uint8_t* r, uint8_t* g, uint8_t* b) {
    switch(random(20)) {
        case 0:
            *r = 155; *g = 176; *b = 255;
            break;
        case 1:
            *r = 170; *g = 191; *b = 255;
            break;
        case 2:
            *r = 202; *g = 215; *b = 255;
            break;
        case 3: case 4:
            *r = 248; *g = 247; *b = 255;
            break;
        case 5: case 6: case 7:
            *r = 255; *g = 244; *b = 234;
            break;
        case 8: case 9: case 10: case 11:
            *r = 255; *g = 210; *b = 161;
            break;
        default:
            *r = 255; *g = 204; *b = 111;
            break;
    }
    int randVals[3] = {(int)random(20),(int)random(25),(int)random(20)};
    *r = constrain((int)(*r) + (-10 + randVals[0]), 0, 255);
    *g = constrain((int)(*g) + (-15 + randVals[1]), 0, 255);
    *b = constrain((int)(*b) + (-10 + randVals[2]), 0, 255);
}

#endif
