/*  Hydruino: Simple automation controller for hydroponic grow systems.
    Copyright (C) 2022-2023 NachtRaveVL     <nachtravevl@gmail.com>
    Hydruino Base UI
*/

#include <Hydruino.h>
#ifdef HYDRO_USE_GUI
#ifndef HydroBaseUI_H
#define HydroBaseUI_H

class HydruinoBaseUI : public HydoUIInterface {
public:
    virtual bool begin() override;

    virtual void setNeedsLayout() override;

protected:
};

#endif // /ifndef HydroBaseUI_H
#endif
