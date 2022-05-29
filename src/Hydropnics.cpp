/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Main
*/

#include "Hydroponics.h"

static void uDelayMillisFuncDef(unsigned int timeout) {
#ifdef HYDRO_USE_SCHEDULER
    if (timeout > 0) {
        unsigned long currTime = millis();
        unsigned long endTime = currTime + (unsigned long)timeout;
        if (currTime < endTime) { // not overflowing
            while (millis() < endTime)
                Scheduler.yield();
        } else { // overflowing
            unsigned long begTime = currTime;
            while (currTime >= begTime || currTime < endTime) {
                Scheduler.yield();
                currTime = millis();
            }
        }
    } else
        Scheduler.yield();
#else
    delay(timeout);
#endif
}

static void uDelayMicrosFuncDef(unsigned int timeout) {
#ifdef HYDRO_USE_SCHEDULER
    if (timeout > 1000) {
        unsigned long currTime = micros();
        unsigned long endTime = currTime + (unsigned long)timeout;
        if (currTime < endTime) { // not overflowing
            while (micros() < endTime)
                Scheduler.yield();
        } else { // overflowing
            unsigned long begTime = currTime;
            while (currTime >= begTime || currTime < endTime) {
                Scheduler.yield();
                currTime = micros();
            }
        }
    } else if (timeout > 0)
        delayMicroseconds(timeout);
    else
        Scheduler.yield();
#else
    delayMicroseconds(timeout);
#endif
}

