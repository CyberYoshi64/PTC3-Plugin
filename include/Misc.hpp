#ifndef MISC_HPP
#define MISC_HPP

#include "main.hpp"
#include "Hooks.hpp"

namespace CTRPluginFramework {
    void serverAdrChg(MenuEntry *entry);
    void versionSpoof(MenuEntry *entry);
    void pluginDisclaimer(MenuEntry *entry);
    void cyxAPItoggle(MenuEntry* entry);
    void grpFixMe(MenuEntry* entry);
    void grpFreeMe(MenuEntry* entry);
    void editorRulerPalette(MenuEntry* entry);
    void fontGetAddrPatch(MenuEntry* entry);
    
    void validateFile(MenuEntry* entry);
    
    void tokenHooker(MenuEntry* entry);
}

#endif