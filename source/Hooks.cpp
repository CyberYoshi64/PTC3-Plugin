#include "Hooks.hpp"

namespace CTRPluginFramework {
    Hooks::Offsets Hooks::offsets = {0};

    Result Hooks::Init(){
        u32 ret = 0;
        File f;
        if ((ret = File::Open(f, Utils::Format(HOOKFILE_PATH, g_regionString), File::READ))==0){
            f.Read(&offsets, sizeof(offsets));
        }
        f.Close();
        if (offsets.mapMagic != HOOKFILE_MAGIC) {
            CYX::exitMessage = "The hook file is corrupted.";
            return 0xDEADBEE0;
        }
        if (offsets.mapVersion != HOOKFILE_VER){
            CYX::exitMessage = "The hook file is outdated or corrupted.";
            return 0xDEADBEE1;
        }
        for (u32 i=2; i<sizeof(offsets)/sizeof(u32); i++){
            u32 v = ((u32*)&offsets)[i];
            if (v < 0x100000 || v > 0x1E80000) {
                CYX::exitMessage = "The hook file contains an invalid address.";
                return 0xDEADBEE1;
            }
        }
        if (*(u32*)offsets.helpPagePal != offsets.helpPageDefPal) {
            CYX::exitMessage = Utils::Format("Bad value: %08X ≠ %08X", *(u32*)offsets.helpPagePal, offsets.helpPageDefPal);
            return 0xDEADBF00;
        }
        return ret;
    }
}