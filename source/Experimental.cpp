#include "Experimental.hpp"

namespace CTRPluginFramework {
    void experiment1(MenuEntry *entry){
        PLGSET(PLGFLG_EXPERIMENTS);
        PANIC("PANIC() Test", _FILENAME, __LINE__);
    }
}