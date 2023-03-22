/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino AdafruitGFX Overview Screen
*/

#include "../HydruinoUI.h"
#ifdef HYDRO_USE_GUI

static float skyEaseInOut(float x) {
    return x < 0.5f ? 2.0f * x * x : 1.0f - ((-2.0f * x + 2.0f) * (-2.0f * x + 2.0f) * 0.5f);
}

static inline void randomStarColor(uint8_t* r, uint8_t* g, uint8_t* b) {
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
    int randVals[3] = {random(20),random(25),random(20)};
    *r = constrain((int)(*r) + (-10 + randVals[0]), 0, 255);
    *g = constrain((int)(*g) + (-15 + randVals[1]), 0, 255);
    *b = constrain((int)(*b) + (-10 + randVals[2]), 0, 255);
}

HydroOverviewGFX<Adafruit_ILI9341>::HydroOverviewGFX(HydroDisplayAdafruitGFX<Adafruit_ILI9341> *display, const void *clockFont, const void *detailFont)
    : HydroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable()), _clockFont(clockFont), _detailFont(detailFont),
      _skyBlue(255), _skyRed(0), _timeMag(1), _dateMag(1), _lastTime((uint32_t)0), _timeHeight(0), _dateHeight(0)
{
    const auto screenSize = display->getScreenSize();
    DateTime scaleTest(2099, 12, 31, 23, 59, 59);

    String timestamp = scaleTest.timestamp(DateTime::TIMESTAMP_TIME);
    for (int i = 2; i < 10; ++i) {
        auto extents = _drawable.textExtents(_clockFont, i, timestamp.c_str());
        if (screenSize.first - extents.x > screenSize.first / 4) {
            _timeMag = i;
        } else { break; }
    }

    timestamp = scaleTest.timestamp(DateTime::TIMESTAMP_DATE);
    for (int i = 2; i < 10; ++i) {
        auto extents = _drawable.textExtents(_clockFont, i, timestamp.c_str());
        if (screenSize.first - extents.x > screenSize.first / 2) {
            _dateMag = i;
        } else { break; }
    }

    randomSeed(unixNow());

    for (int i = 0; i < HYDRO_UI_STARFIELD_MAXSIZE; ++i) {
        uint16_t randY = random(screenSize.second);
        while (_stars.find(randY) != _stars.end()) { randY = random(screenSize.second); }
        uint8_t starRGB[3] = {0};
        randomStarColor(&starRGB[0], &starRGB[1], &starRGB[2]);
        _stars[randY] = make_pair((uint16_t)random(screenSize.first),
                                  (uint16_t)_gfx.color565(starRGB[0], starRGB[1], starRGB[2]));
    }
}

HydroOverviewGFX<Adafruit_ILI9341>::~HydroOverviewGFX()
{ ; }

void HydroOverviewGFX<Adafruit_ILI9341>::drawBackground(Coord pt, Coord sz, Pair<uint16_t, uint16_t> &screenSize)
{
    pt.x = constrain(pt.x, 0, screenSize.first);
    sz.x = constrain(sz.x, 0, screenSize.first - pt.x);
    pt.y = constrain(pt.y, 0, screenSize.second);
    sz.y = constrain(sz.y, 0, screenSize.second - pt.y);

    _gfx.startWrite();
    int maxY = pt.y + sz.y;
    for (int y = pt.y; y < maxY; ++y) {
        int skyBlue = constrain(y - (screenSize.second - _skyBlue - 10), max(10,_skyBlue >> 2), max(10,_skyBlue));
        uint16_t skyColor = _gfx.color565(_skyRed, (skyBlue * 7)/8, skyBlue);
        _gfx.setAddrWindow(pt.x, y, sz.x, 1);
        _gfx.writeColor(skyColor, (uint32_t)sz.x);

        if (skyBlue + skyBlue + (int)_skyRed < 255) {
            auto starIter = _stars.find(y);
            if (starIter != _stars.end() && (*starIter).second.first >= pt.x && (*starIter).second.first <= pt.x + sz.x) {
                int skyT = skyBlue + skyBlue + (int)_skyRed;
                int starT = 255 - skyT;
                uint16_t star565 = (*starIter).second.second;
                uint8_t starR = (star565 >> 11) & 0x1F; starR = (starR << 3) | (starR >> 2);
                uint8_t starG = (star565 >> 5) & 0x3F; starG = (starG << 2) | (starG >> 4);
                uint8_t starB = star565 & 0x1F; starB = (starB << 3) | (starB >> 2);

                _gfx.writePixel((*starIter).second.first, y,
                    _gfx.color565((((int)_skyRed * skyT) / 255) + (((int)starR * starT) / 255),
                                  ((((skyBlue * 7)/8) * skyT) / 255) + (((int)starG * starT) / 255),
                                  ((skyBlue * skyT) / 255) + (((int)starB * starT) / 255)));
            }
        }
    }
    _gfx.endWrite();
}

void HydroOverviewGFX<Adafruit_ILI9341>::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    auto currTime = localNow();

    {   auto sunrise = (getScheduler() ? getScheduler()->getDailyTwilight() : Twilight()).getSunriseLocalTime();
        auto sunset = (getScheduler() ? getScheduler()->getDailyTwilight() : Twilight()).getSunsetLocalTime();
        uint8_t skyBlue, skyRed;

        if (currTime.unixtime() < sunrise.unixtime() - (SECS_PER_HOUR >> 1) ||
            currTime.unixtime() > sunset.unixtime() + (SECS_PER_HOUR >> 1)) { skyBlue = 0; skyRed = 0; }
        else if (currTime.unixtime() >= sunrise.unixtime() + (SECS_PER_HOUR >> 1) &&
             currTime.unixtime() <= sunset.unixtime() - (SECS_PER_HOUR >> 1)) { skyBlue = 255; skyRed = 0; }
        else {
            float x = (currTime.unixtime() - ((currTime.hour() < 12 ? sunrise.unixtime() : sunset.unixtime()) - (SECS_PER_HOUR >> 1))) / (float)SECS_PER_HOUR; //(float)SECS_PER_HOUR;
            x = constrain(x, 0.0f, 1.0f);
            skyBlue = roundf(skyEaseInOut(currTime.hour() < 12 ? x : 1.0f - x) * 255.0f);
            skyBlue = constrain(skyBlue, 0, 255);
            if (currTime.hour() > 12) { x = constrain(x * 1.5f, 0.0f, 1.0f); }
            else { x = constrain(((x - 0.25f) * 1.5f), 0.0f, 1.0f); }
            skyRed = (-300.0f * (x * x)) + (300.0f * x);
            skyRed = constrain(skyRed, 0, 255);
        }

        _needsFullRedraw = _needsFullRedraw || _skyBlue != skyBlue || _skyRed != skyRed;
        _skyBlue = skyBlue; _skyRed = skyRed;
    }

    if (_needsFullRedraw) {
        uint16_t yOffset = 10;

        String timestamp = currTime.timestamp(DateTime::TIMESTAMP_TIME);
        auto extents = _drawable.textExtents(_clockFont, _timeMag, timestamp.c_str());
        _timeHeight = extents.y;

        drawBackground(Coord(0,0), Coord(screenSize.first,yOffset + _timeHeight + 5), screenSize);
        _drawable.setDrawColor(TFT_WHITE);
        _drawable.drawText(Coord((screenSize.first - extents.x) >> 1, yOffset), _clockFont, _timeMag, timestamp.c_str());

        yOffset += _timeHeight + 5;

        timestamp = currTime.timestamp(DateTime::TIMESTAMP_DATE);
        extents = _drawable.textExtents(_clockFont, _dateMag, timestamp.c_str());
        _dateHeight = extents.y;

        drawBackground(Coord(0,yOffset), Coord(screenSize.first,yOffset + _dateHeight + 5), screenSize);
        _drawable.setDrawColor(TFT_WHITE);
        _drawable.drawText(Coord((screenSize.first - extents.x) >> 1, yOffset), _clockFont, _dateMag, timestamp.c_str());

        yOffset += _dateHeight + 5;

        drawBackground(Coord(0,yOffset), Coord(screenSize.first,screenSize.second - yOffset), screenSize);

        _needsFullRedraw = false;
    } else {
        bool needsTimeRedraw = _lastTime.unixtime() != currTime.unixtime();
        bool needsDateRedraw = _lastTime.day() != currTime.day() || _lastTime.month() != currTime.month() || _lastTime.year() != currTime.year();
        uint16_t yOffset = 10;

        if (needsTimeRedraw) {
            String lastTimestamp = _lastTime.timestamp(DateTime::TIMESTAMP_TIME);
            String currTimestamp = currTime.timestamp(DateTime::TIMESTAMP_TIME);
            auto fullExtents = _drawable.textExtents(_clockFont, _timeMag, currTimestamp.c_str());

            for (int i = 0; i < lastTimestamp.length() && i < currTimestamp.length(); ++i) {
                if (i == lastTimestamp.length() || i == currTimestamp.length() || lastTimestamp[i] != currTimestamp[i]) {
                    auto partExtents = _drawable.textExtents(_clockFont, _timeMag, currTimestamp.c_str() + i);
                    Coord partStart = Coord(((screenSize.first - fullExtents.x) >> 1) + fullExtents.x - partExtents.x, yOffset);

                    drawBackground(partStart, partExtents, screenSize);

                    _drawable.setDrawColor(TFT_WHITE);
                    _drawable.drawText(partStart, _clockFont, _timeMag, currTimestamp.c_str() + i);

                    break;
                }
            }
        }

        yOffset += _timeHeight + 5;

        if (needsDateRedraw) {
            String lastTimestamp = _lastTime.timestamp(DateTime::TIMESTAMP_DATE);
            String currTimestamp = currTime.timestamp(DateTime::TIMESTAMP_DATE);
            auto fullExtents = _drawable.textExtents(_clockFont, _dateMag, currTimestamp.c_str());

            for (int i = 0; i < lastTimestamp.length() && i < currTimestamp.length(); ++i) {
                if (i == lastTimestamp.length() || i == currTimestamp.length() || lastTimestamp[i] != currTimestamp[i]) {
                    auto partExtents = _drawable.textExtents(_clockFont, _dateMag, currTimestamp.c_str() + i);
                    Coord partStart = Coord(((screenSize.first - fullExtents.x) >> 1) + fullExtents.x - partExtents.x, yOffset);

                    drawBackground(partStart, partExtents, screenSize);

                    _drawable.setDrawColor(TFT_WHITE);
                    _drawable.drawText(partStart, _clockFont, _dateMag, currTimestamp.c_str() + i);

                    break;
                }
            }
        }

        yOffset += _dateHeight + 5;
    }

    _lastTime = currTime;
}

#endif
