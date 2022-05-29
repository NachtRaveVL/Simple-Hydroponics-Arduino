/*  Arduino Controller for Simple Hydroponics.
    Copyright (C) 2022 NachtRaveVL          <nachtravevl@gmail.com>
    Hydroponics Inlines
*/
#ifndef HydroponicsInlines_HPP
#define HydroponicsInlines_HPP

#include "Hydroponics.h"

static inline int roundUpVal16(int val) { return ((val + 15) & -16); }
static inline byte *roundUpPtr16(byte *ptr) { return ptr ? (byte *)(((uintptr_t)ptr + 15) & -16) : NULL; }
static inline byte *roundUpMalloc16(int size) { return (byte *)malloc((size_t)(size + 15)); }
static inline byte *roundUpRealloc16(byte *ptr, int size) { return (byte *)realloc((void *)ptr, (size_t)(size + 15)); }

#endif // /ifndef HydroponicsInlines_HPP
