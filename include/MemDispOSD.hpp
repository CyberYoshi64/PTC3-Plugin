#ifndef MEMDISPOSD_HPP
#define MEMDISPOSD_HPP

#include "main.hpp"

namespace CTRPluginFramework {
    class MemDisplayOSD {
    public:
        static void setup(MenuEntry *entry);
        static void OSDFunc(MenuEntry* entry);
    private:
        static u32 memStart;
        static u32 length;
        static u32 divider;
        static bool isValid;
    };
}


#endif