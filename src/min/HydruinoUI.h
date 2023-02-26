/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Minimal UI
*/

class HydruinoMinUI;

#include "Hydruino.h"

class HydruinoMinUI : HydruinoUIInterface {
public:
    virtual bool begin() override;

    virtual void setNeedsLayout() override;

protected:
};
