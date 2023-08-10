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
        if (offsets.mapMagic != HOOKFILE_MAGIC) return 0xDEADBEE0;
        if (offsets.mapVersion != HOOKFILE_VER) return 0xDEADBEE1;
        for (u32 i=2; i<sizeof(offsets)/sizeof(u32); i++){
            if (!((u32*)&offsets)[i]) return 0xDEADBEE2;
        }
        return ret;
    }
}