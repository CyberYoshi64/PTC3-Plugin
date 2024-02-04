#include "Hooks.hpp"

namespace CTRPluginFramework {
    Hooks::Offsets Hooks::offsets = {0};

    Result Hooks::Init() {
        u32 ret = 0;
        File f;
        if ((ret = File::Open(f, Utils::Format(HOOKFILE_PATH, g_regionString), File::READ))==0) {
            f.Read(&offsets, sizeof(offsets));
        }
        f.Close();
        if (offsets.mapMagic != HOOKFILE_MAGIC) {
            CYX::exitMessage = "The hook file is corrupted.";
            return 0xDEADBEE0;
        }
        if (offsets.mapVersion != HOOKFILE_VER) {
            CYX::exitMessage = "The hook file is outdated or corrupted.";
            return 0xDEADBEE1;
        }
        for (u32 i=2; i<sizeof(offsets)/sizeof(u32); i++) {
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
    int Hooks::ParseFuncMapFile(File &f, FuncMapFile* v) {
        char buf[32]; s32 count, len;
        v->ids.clear();
        v->humanNames.clear();
        f.Read(buf, 4);
        v->version = *(u32*)buf;
        if (v->version != HOOK_FUNCMAP_VER) return 1;
        f.Read(buf, 4);
        count = *(s32*)buf;
        if (count > 1023) return 2;
        for (int i = 0; i < count; i++) {
            f.Read(buf, 4);
            v->ids.push_back(*(s32*)buf);
            f.Read(buf, 2);
            len = *(u16*)buf;
            if (len >= sizeof(buf)) return 3;
            memset(buf, 0, sizeof(buf));
            f.Read(buf, len);
            buf[sizeof(buf)-1] = 0;
            v->humanNames.push_back(buf);
        }
        return 0;
    };
}