#ifndef HOOKINIT_HPP
#define HOOKINIT_HPP

#include "main.hpp"

#define HOOKFILE_PATH   RESOURCES_PATH"/map_%s.cyxmap"
#define HOOKFILE_MAGIC  *(u32*)"CY$X"
#define HOOKFILE_VER    2

#define HOOK_FUNCMAP_VER    1

namespace CTRPluginFramework {
    class Hooks {
        typedef struct Offsets {
            u32 mapMagic;                   // Map file signature — not for use
            u32 mapVersion;                 // Map file version — not for use

            u32 versionInt;                 // Version integer (VERSION)
            u32 bootText;                   // Pointer to boot text
            u32 configBuf;                  // Config save buffer
            u32 editorData;                 // Editor data structure
            u32 graphicStructs;             // Graphic Page structs
            u32 activeProjStr;              // Active Project structure
            u32 helpPagePal;                // Help page palette pointer
            u32 helpPageDefCol;             // Help page default color
            u32 helpPageDefPal;             // Help page default palette
            u32 consoleTextPal;             // Console text palette
            u32 funcController;             // CONTROLLER function
            u32 basicInterpretRun;          // Interpreter is running
            u32 basicCommandRun;            // A command/program is executed
            u32 basicIsDirect;              // Direct Mode / Play Mode
            u32 fontMapBuf;                 // Font Map buffer
            u32 funcFontGetOff;             // Function to rule FONTDEF definitions
            u32 colorKeybBack;              // BASIC Keyboard background color
            u32 colorSearchBack;            // Search background color
            u32 colorFileCreatorBack;       // Browser File Creator Color
            u32 colorFileDescBack;          // Browser File Description Color
            u32 colorActiveProjLbl;         // TOP MENU Active Project Color
            u32 colorSetSmToolLbl;          // Settings SmileTool Color
            u32 colorSetKeyRep;             // Settings Key Repeat Color
            u32 colorSetKeyTL;              // Settings Key TL Color
            u32 serverLoad2[2];             // Server URL to load2.php
            u32 serverSave3[2];             // Server URL to save3.php
            u32 serverShow2;                // Server URL to show2.php
            u32 serverList2;                // Server URL to list2.php
            u32 serverInfo2;                // Server URL to info2.php
            u32 serverDelete2;              // Server URL to delete2.php
            u32 serverShopList2;            // Server URL to shoplist2.php (won't work but why not?)
            u32 serverPrepurchase2;         // Server URL to prepurchase2.php (won't work but why not?)
            u32 serverPurchase2;            // Server URL to purchase2.php (won't work but why not?)
            u32 serverHMACKey;              // Server HMAC Signature Key
            u32 petcSessionTokenFunc;       // Function that provides server session token
            u32 petcAccountToken;           // Server Account Token (typically generated by ACT/MINT)
            u32 nnActConnectRequired;       // Value of nn::act::IsConnectRequired()
            u32 nnActNetworkTimeValidated;  // Value of nn::act::NetworkTimeValidated()
            u32 nnActIsNetworkAccountFunc;  // Function nn::act::IsNetworkAccount()
            u32 nnSndSoundThreadEntry1;     // Sound Thread #1 Entry Point
            u32 nnSndSoundThreadEntry2;     // Sound Thread #2 Entry Point
        } Offsets;

    public:
        typedef struct FuncMapFile_s {
            u32 version;
            std::vector<s32> ids;
            std::vector<std::string> humanNames;
        } FuncMapFile;
        static Offsets offsets; // Offset structure
        static Result Init();
        static int ParseFuncMapFile(File &f, FuncMapFile* v);
    };
}

#endif