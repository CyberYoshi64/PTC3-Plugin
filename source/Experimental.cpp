#include "Experimental.hpp"

namespace CTRPluginFramework {
    void experiment1(MenuEntry *entry){
        PLGSET(PLGFLG_EXPERIMENTS);
        PANIC("PANIC() Test", _FILENAME, __LINE__);
    }
    void testOSD(MenuEntry *entry){
        PLGSET(PLGFLG_EXPERIMENTS);
        std::string str;
        OSD::Lock();
        Screen top = OSD::GetTopScreen();
        CYX::UTF16toUTF8(str="", CYX::activeProject->activeProject);
        top.Draw(str, 0, 0);
        CYX::UTF16toUTF8(str="", CYX::activeProject->currentProject);
        top.Draw(str, 0, 10);
        top.Draw(CYX::g_currentProject, 0, 20);
        OSD::Unlock();
    }
}