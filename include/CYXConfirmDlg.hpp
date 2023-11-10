#ifndef CYXCONFIRMDLG_HPP
#define CYXCONFIRMDLG_HPP

#include "main.hpp"

namespace CTRPluginFramework {
    enum CYXConfirmID {
        CYXCONFIRM_NONE = 0,
        CYXCONFIRM_BASICAPI_XREF_RW,
        CYXCONFIRM_BASICAPI_SD_RW,
        CYXCONFIRM_BASICAPI_TOHOME,
    };
    class CYXConfirmDlg {
    public:
        static void DoTheThing(void);
        static void ResetUse(void);
    private:
        static u32 coolDown;
        static u32 useCount; // Per minute; used to disable CFGSET if spammed
        static u32 useTimer; // Per minute; used to disable CFGSET if spammed
        static int BasicAPI_XREF_RW(void);
        static int BasicAPI_SD_RW(void);
        static int BasicAPI_ToHOME(void);
    };
}
#endif