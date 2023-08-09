#ifndef HOOKINIT_HPP
#define HOOKINIT_HPP

#include "main.hpp"

#define HOOKFILE_PATH   RESOURCES_PATH"/map_%s.cyxmap"
#define HOOKFILE_MAGIC  *(u32*)"CY$X"
#define HOOKFILE_VER    1

namespace CTRPluginFramework {
    class Hooks {
        typedef struct Offsets {
            u32 mapMagic;
            u32 mapVersion;

            u32 versionInt;
            u32 bootText;
            u32 configBuf;
            u32 editorData;
            u32 graphicStructs;
            u32 activeProjStr;
            u32 helpPagePal;
            u32 helpPageDef;
            u32 consoleTextPal;
            u32 funcController;
            u32 basicInterpretRun;
            u32 basicCommandRun;
            u32 basicIsDirect;
            u32 fontMapBuf;
            u32 funcFontGetOff;
            u32 colorKeybBack;
            u32 colorSearchBack;
            u32 colorFileCreatorBack;
            u32 colorFileDescBack;
            u32 colorActiveProjLbl;
            u32 colorSetSmToolLbl;
            u32 colorSetKeyRep;
            u32 colorSetKeyTL;
            u32 serverLoad2[2];
            u32 serverSave3[2];
            u32 serverShow2;
            u32 serverList2;
            u32 serverInfo2;
            u32 serverDelete2;
            u32 serverShopList2;
            u32 serverPrepurchase2;
            u32 serverPurchase2;
        } Offsets;
    public:
        static Offsets offsets;
        static Result Init();
    };
}

#endif