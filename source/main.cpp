#include <CTRPluginFramework.hpp>
#include "main.hpp"
#include "BasicAPI.hpp"
#include "save.hpp"
#include "petitReimpl.hpp"

const char* pluginSerial = "3GX-H-CYXZ";

// Patterns of functions to try replace with custom ones
u8 fsMountArchivePat1[] = {0x10, 0x00, 0x97, 0xE5, 0xD8, 0x20, 0xCD, 0xE1, 0x00, 0x00, 0x8D};
u8 fsMountArchivePat2[] = {0x28, 0xD0, 0x4D, 0xE2, 0x00, 0x40, 0xA0, 0xE1, 0xA8, 0x60, 0x9F, 0xE5, 0x01, 0xC0, 0xA0, 0xE3};
u8 fsRegArchivePat[] = {0xB4, 0x44, 0x20, 0xC8, 0x59, 0x46, 0x60, 0xD8};
u8 userFsTryOpenFilePat1[] = {0x0D, 0x10, 0xA0, 0xE1, 0x00, 0xC0, 0x90, 0xE5, 0x04, 0x00, 0xA0, 0xE1, 0x3C, 0xFF, 0x2F, 0xE1};
u8 userFsTryOpenFilePat2[] = {0x10, 0x10, 0x8D, 0xE2, 0x00, 0xC0, 0x90, 0xE5, 0x05, 0x00, 0xA0, 0xE1, 0x3C, 0xFF, 0x2F, 0xE1};
u8 openArchivePat[] = {0xF0, 0x81, 0xBD, 0xE8, 0xC2, 0x00, 0x0C, 0x08};
u8 formatSavePat[] = {0xF0, 0x9F, 0xBD, 0xE8, 0x42, 0x02, 0x4C, 0x08};
u8 fsSetThisSecValPat[] = {0xC0, 0x00, 0x6E, 0x08};
u8 fsObsSetThisSecValPat[] = {0x40, 0x01, 0x65, 0x08};
u8 fsSetSecValPat[] = {0x80, 0x01, 0x75, 0x08};
u8 fsCheckPermsPat[] = {0x04, 0x10, 0x12, 0x00, 0x76, 0x46, 0x00, 0xD9};

namespace CTRPluginFramework {
    LightEvent mainEvent1;
    bool isCYXenabled;
    Region g_region;
    FS_ArchiveResource g_sdmcArcRes;
    u32 ___pluginFlags;
    char g_regionString[4];
    u8 g_systemModel;
    u8 g_systemRegion;
    u32 parentalControlFlag;
    char g_systemRegionString[4];
    u32 *fileOperations[NUMBER_FILE_OP] = {nullptr};
    RT_HOOK fileOpHooks[NUMBER_FILE_OP] = {0};
    RT_HOOK regArchiveHook = {0};
    RT_HOOK openArchiveHook = {0};
    RT_HOOK formatSaveHook = {0};
    RT_HOOK fsSetThisSecValHook = {0};
    RT_HOOK fsObsSetThisSecValHook = {0};
    RT_HOOK fsSetSecValHook = {0};
    u32 fsMountArchive = 0;
    char g_ProcessTID[17];
    vu32 ___confirmID;
    volatile int ___confirmRes = 0;
    volatile bool ___confirmWaiting = false;

    u32 g_osFirmVer, g_osKernelVer;
    OS_VersionBin g_osNVer, g_osCVer;
    char g_osSysVer[16];
    u32 cyxres;

    void setCTRPFConfirm(u32 id, int defaultRes = 0) {
        ___confirmID = id;
        ___confirmRes = defaultRes;
        ___confirmWaiting = true;
    }

    int waitCTRPFConfirm() {
        while (___confirmWaiting) Sleep(Milliseconds(16));
        return ___confirmRes;
    }

    void mcuSetSleep(bool on) {
        u8 reg;
        if (R_FAILED(mcuHwcInit())) return;
        MCUHWC_ReadRegister(0x18, &reg, 1);
        if (on)
            reg &= ~0x6C;
        else
            reg |= 0x6C;
        MCUHWC_WriteRegister(0x18, &reg, 1);
        mcuHwcExit();
        return;
    }

    bool mcuIsSleepEnabled() {
        u8 reg;
        if (R_FAILED(mcuHwcInit())) return false;
        MCUHWC_ReadRegister(0x18, &reg, 1);
        mcuHwcExit();
        return !(reg & 0x6C);
    }

    void deleteSecureVal() {
        DEBUG("NOTE: This game uses a secure value, ");
        Result res; u8 out;
        u64 secureValue = ((u64)SECUREVALUE_SLOT_SD << 32) | (((u32)Process::GetTitleID() >> 8) << 8);
        res = FSUSER_ControlSecureSave(SECURESAVE_ACTION_DELETE, &secureValue, 8, &out, 1);
        if (res) {
            DEBUG(" fsControlSecureSave returned: 0x%08X, proceeding to patch fs", res);
            Handle prochand;
            res = svcOpenProcess(&prochand, 0); // fs processID
            if (res) {
                DEBUG(", svcOpenProcess returned: 0x%08X, aborting.\n", res);
                customBreak(0xAB047, 1, 0, 0);
            }
            s64 info;
            res = svcGetProcessInfo(&info, prochand, 0x10005); // get start of .text
            if (res) {
                DEBUG(", svcGetProcessInfo 0x10005 returned: 0x%08X, aborting.\n", res);
                customBreak(0xAB047, 1, 0, 0);
            }
            u32 *addr = (u32 *)info;
            res = svcGetProcessInfo(&info, prochand, 0x10002); // get .text size
            if (res) {
                DEBUG(", svcGetProcessInfo 0x10002 returned: 0x%08X, aborting.\n", res);
                customBreak(0xAB047, 1, 0, 0);
            }
            res = svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x08000000, prochand, (u32)addr, (u32)info);
            if (res) {
                DEBUG(", svcMapProcessMemoryEx returned: 0x%08X, aborting.\n", res);
                customBreak(0xAB047, 1, 0, 0);
            }
            addr = (u32 *)0x08000000;
            u32 *endAddr = (u32 *)((u32)addr + (u32)info);
            std::vector<u32 *> backup;
            DEBUG(" (patched : ");
            bool first = true;
            while (addr < endAddr) {
                if (memcmp(addr, fsCheckPermsPat, sizeof(fsCheckPermsPat)) == 0) {
                    backup.push_back(addr);
                    *addr = 0x80; // SD access patched by Luma3DS
                    if (first) {
                        DEBUG("0x%08X", (u32)addr);
                        first = false;
                    } else
                        DEBUG(", 0x%08X", (u32)addr);
                }
                addr++;
            }
            DEBUG("), ");
            svcInvalidateEntireInstructionCache();
            res = FSUSER_ControlSecureSave(SECURESAVE_ACTION_DELETE, &secureValue, 8, &out, 1);
            if (res) {
                DEBUG("patched fsControlSecureSave returned: 0x%08X, abort.\n", res);
                customBreak(0xAB047, 1, 0, 0);
            } else {
                DEBUG("patch succeeded, ");
            }
            for (u32 *addrRest : backup)
                *addrRest = 0x121004;
            svcInvalidateEntireInstructionCache();
            svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x08000000, (u32)info);
            svcCloseHandle(prochand);
        }
        if (out) {
            DEBUG("secure value has been deleted.\n");
        } else {
            DEBUG("but there was no secure value stored.\n");
        }
    }

    void dummyEntry(MenuEntry *entry) {}

    u32 *findNearestSTMFD(u32 *newaddr) {
        for (u32 i=0; i<1024; i++) {
            newaddr--;
            i++;
            if (*((u16*)newaddr + 1) == 0xE92D) return newaddr;
        }
        return 0;
    }

    static inline u32 decodeARMBranch(const u32 *src) {
        s32 off = (*src & 0xFFFFFF) << 2;
        off = (off << 6) >> 6; // sign extend
        return (u32)src + 8 + off;
    }

    void storeAddrByOffset(u32 *addr, u16 offset) {
        if (offset % 4 != 0)
            return;
        offset >>= 2;
        if (ENABLE_DEBUG) {
            const char *funcstr = "";
            char buf[10];
            switch (offset) {
            case 0:
                funcstr = "fsOpenFile";
                break;
            case 1:
                funcstr = "fsOpenDirectory";
                break;
            case 2:
                funcstr = "fsDeleteFile";
                break;
            case 3:
                funcstr = "fsRenameFile";
                break;
            case 4:
                funcstr = "fsDeleteDirectory";
                break;
            case 5:
                funcstr = "fsDeleteDirectoryRecursively";
                break;
            case 6:
                funcstr = "fsCreateFile";
                break;
            case 7:
                funcstr = "fsCreateDirectory";
                break;
            case 8:
                funcstr = "fsRenameDirectory";
                break;
            default:
                snprintf(buf, sizeof(buf), "%d", offset);
                funcstr = buf;
            }
            DEBUG("> %s found at 0x%08X\n", funcstr, (u32)addr);
        }
        if (offset < NUMBER_FILE_OP)
            fileOperations[offset] = addr;
    }

    void processFileSystemOperations(u32 *funct, u32 *endAddr) {
        DEBUG("\nStarting to process fs functions...\n");
        int i;
        for (i = 0; i < 0x20; i++) { // Search for the closest BL, this BL will branch to getArchObj
            if ((*(funct + i) & 0xFF000000) == 0xEB000000) {
                funct += i;
                break;
            }
        }
        u32 funcAddr;
        u32 *addr;
        int ctr = 1;
        if (i >= 0x20) { // If there are no branches, the function couldn't be found.
            DEBUG("> ERROR: Couldn't find getArchObj\n");
            ctr = 0;
            goto exit;
        }
        funcAddr = decodeARMBranch(funct); // Get the address of getArchObj
        DEBUG("> getArchObj found at 0x%08X\n", funcAddr);
        addr = (u32 *)0x100000;
        while (addr < endAddr) { // Scan the text section of the code for the fs functions
            if ((*addr & 0xFF000000) == 0xEB000000 && (decodeARMBranch(addr) == funcAddr)) { // If a branch to getArchObj if found analyze it.
                u8 regId = 0xFF;
                for (i = 0; i < 1024; i++) { // Scan forwards for the closest BLX, and get the register it is branching to
                    int currinst = addr[i];
                    if (*((u16 *)(addr + i) + 1) == 0xE92D)
                        break; // Stop if STMFD is found (no BLX in this function)
                    if ((currinst & ~0xF) == 0xE12FFF30) { // BLX
                        regId = currinst & 0xF;
                        break;
                    }
                }
                if (regId != 0xFF) { // If a BLX is found, scan backwards for the nearest LDR to the BLX register.
                    int j = i;
                    for (; i > 0; i--) {
                        if (((addr[i] & 0xFFF00000) == 0xE5900000) && (((addr[i] & 0xF000) >> 12) == regId)) {																// If it is a LDR and to the BLX register
                            storeAddrByOffset(findNearestSTMFD(addr), addr[i] & 0xFFF); // It is a fs function, store it based on the LDR offset. (This LDR gets the values from the archive object vtable, by checking the vtable offset it is possible to know which function it is)
                            break;
                        }
                    }
                    addr += j; // Continue the analysis from the BLX
                }
            }
            addr++;
        }
        for (int i = 0; i < NUMBER_FILE_OP; i++) {
            if (fileOperations[i] == nullptr)
                continue;
            ctr++;
            rtInitHook(&fileOpHooks[i], (u32)fileOperations[i], fileOperationFuncs[i]);
            rtEnableHook(&fileOpHooks[i]);
        }
    exit:
        DEBUG("Finished processing fs functions: %d/%d found.\n\n", ctr, NUMBER_FILE_OP + 1);
    }

    void initOnionFSHooks(u32 textSize) {
        u32 *addr = (u32 *)0x100000;
        u32 *endAddr = (u32 *)(0x100000 + textSize);
        bool contOpen = true, contMount = true, contReg = true, contArch = true, contDelete = true, contSetThis = true, contSetObs = true, contSet = true;
        while (addr < endAddr && (contOpen || contMount || contReg || contArch || contDelete || contSetThis || contSetObs || contSet)) {
            if (contOpen && memcmp(addr, userFsTryOpenFilePat1, sizeof(userFsTryOpenFilePat1)) == 0 || memcmp(addr, userFsTryOpenFilePat2, sizeof(userFsTryOpenFilePat2)) == 0) {
                u32 *fndaddr = findNearestSTMFD(addr);
                DEBUG("tryOpenFile found at 0x%08X\n", (u32)fndaddr);
                contOpen = false;
                processFileSystemOperations(fndaddr, endAddr);
            }
            if (contMount && memcmp(addr, fsMountArchivePat1, sizeof(fsMountArchivePat1)) == 0 || memcmp(addr, fsMountArchivePat2, sizeof(fsMountArchivePat2)) == 0) {
                u32 *fndaddr = findNearestSTMFD(addr);
                DEBUG("mountArchive found at 0x%08X\n", (u32)fndaddr);
                contMount = false;
                fsMountArchive = (u32)fndaddr;
            }
            if (contReg && memcmp(addr, fsRegArchivePat, sizeof(fsRegArchivePat)) == 0) {
                contReg = false;
                u32 *fndaddr = findNearestSTMFD(addr);
                DEBUG("registerArchive found at 0x%08X\n", (u32)fndaddr);
                rtInitHook(&regArchiveHook, (u32)fndaddr, (u32)fsRegArchiveCallback);
                rtEnableHook(&regArchiveHook);
            }
            if (contArch && memcmp(addr, openArchivePat, sizeof(openArchivePat)) == 0) {
                contArch = false;
                u32 *fndaddr = findNearestSTMFD(addr);
                DEBUG("openArchive found at 0x%08X\n", (u32)fndaddr);
                rtInitHook(&openArchiveHook, (u32)fndaddr, (u32)fsOpenArchiveFunc);
                rtEnableHook(&openArchiveHook);
            }
            if (contDelete && memcmp(addr, formatSavePat, sizeof(formatSavePat)) == 0) {
                contDelete = false;
                u32 *fndaddr = findNearestSTMFD(addr);
                DEBUG("formatSaveData found at 0x%08X\n", (u32)fndaddr);
                rtInitHook(&formatSaveHook, (u32)fndaddr, (u32)fsFormatSaveData);
                rtEnableHook(&formatSaveHook);
            }
            if (contSetThis && memcmp(addr, fsSetThisSecValPat, sizeof(fsSetThisSecValPat)) == 0) {
                contSetThis = false;
                u32 *fndaddr = findNearestSTMFD(addr);
                DEBUG("fsSetThisSaveDataSecureValue found at 0x%08X\n", (u32)fndaddr);
                rtInitHook(&fsSetThisSecValHook, (u32)fndaddr, (u32)fsSetThisSaveDataSecureValue);
                rtEnableHook(&fsSetThisSecValHook);
            }
            if (contSetObs && memcmp(addr, fsObsSetThisSecValPat, sizeof(fsObsSetThisSecValPat)) == 0) {
                contSetObs = false;
                u32 *fndaddr = findNearestSTMFD(addr);
                DEBUG("Obsoleted_5_0_fsSetSaveDataSecureValue found at 0x%08X\n", (u32)fndaddr);
                rtInitHook(&fsObsSetThisSecValHook, (u32)fndaddr, (u32)Obsoleted_5_0_fsSetSaveDataSecureValue);
                rtEnableHook(&fsObsSetThisSecValHook);
            }
            if (contSet && memcmp(addr, fsSetSecValPat, sizeof(fsSetSecValPat)) == 0) {
                contSet = false;
                u32 *fndaddr = findNearestSTMFD(addr);
                DEBUG("fsSetSaveDataSecureValue found at 0x%08X\n", (u32)fndaddr);
                rtInitHook(&fsSetSecValHook, (u32)fndaddr, (u32)fsSetSaveDataSecureValue);
                rtEnableHook(&fsSetSecValHook);
            }
            addr++;
        }
        if (fsSetThisSecValHook.isEnabled || fsObsSetThisSecValHook.isEnabled || fsSetSecValHook.isEnabled)
            deleteSecureVal();

        if (!(openArchiveHook.isEnabled && regArchiveHook.isEnabled)) {
            DEBUG("ERROR: Some hooks couldn't be initialized, aborting.\n");
            customBreak(0xab047, 0, 0, 0);
        }
    }

    // This patch the NFC disabling the touchscreen when scanning an amiibo, which prevents ctrpf to be used
    static void ToggleTouchscreenForceOn(void) {
        static u32 original = 0;
        static u32 *patchAddress = nullptr;

        if (patchAddress && original) {
            *patchAddress = original;
            return;
        }

        static const std::vector<u32> pattern = {
            0xE59F10C0, 0xE5840004, 0xE5841000, 0xE5DD0000,
            0xE5C40008, 0xE28DD03C, 0xE8BD80F0, 0xE5D51001,
            0xE1D400D4, 0xE3510003, 0x159F0034, 0x1A000003
        };

        Result res;
        Handle processHandle;
        s64 textTotalSize = 0, startAddress = 0;
        u32 *found;

        if (R_FAILED(svcOpenProcess(&processHandle, 16)))
            return;

        svcGetProcessInfo(&textTotalSize, processHandle, 0x10002);
        svcGetProcessInfo(&startAddress, processHandle, 0x10005);
        if (R_FAILED(svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, processHandle, (u32)startAddress, textTotalSize)))
            goto exit;

        found = (u32 *)Utils::Search<u32>(0x14000000, (u32)textTotalSize, pattern);

        if (found != nullptr) {
            original = found[13];
            patchAddress = (u32 *)PA_FROM_VA((found + 13));
            found[13] = 0xE1A00000;
        }

        svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, textTotalSize);
    exit:
        svcCloseHandle(processHandle);
    }

    void CheckRegion(void)
    {
        g_region = REGION_NONE;
        u64 tid = Process::GetTitleID();

        // Get current game's region
        switch (tid) {
        case 0x0004000000117200:
            g_region = REGION_JPN;
            sprintf(g_regionString, "JPN");
            break;
        case 0x000400000016DE00:
            g_region = REGION_USA;
            sprintf(g_regionString, "USA");
            break;
        case 0x00040000001A1C00:
            g_region = REGION_EUR;
            sprintf(g_regionString, "EUR");
            break;
        }
    }
    static bool CheckRevision() {
        switch (g_region) {
        case REGION_JPN:
            if (CYX::currentVersion != 0x03060300)
                return false;
            break;
        case REGION_USA:
        case REGION_EUR:
            if (CYX::currentVersion != 0x03060000)
                return false;
            break;
        }
        return true;
    }

    // Patch pre-start of main game code
    void PatchProcess(FwkSettings &settings) {
        settings.BackgroundBorderColor = Color::DeepSkyBlue;
        settings.CustomKeyboard.BackgroundBorder = Color::DeepSkyBlue;
        settings.BackgroundSecondaryColor = Color(0x001010FF);
        settings.CustomKeyboard.BackgroundSecondary = Color(0x001010FF);
        settings.MainTextColor = Color(0xD0D0D0FF);
        sprintf(g_ProcessTID, "%016lX", Process::GetTitleID());
        CFG_GetParentalControlMask(&parentalControlFlag);
        CFGU_GetSystemModel(&g_systemModel);
        CFGU_SecureInfoGetRegion(&g_systemRegion);
        memcpy(g_systemRegionString, "JPNUSAEURAUSKORCHNTWNUNK" + (3 * MIN(g_systemRegion, 7)), 3);
        g_systemRegionString[3] = 0;
        
        CheckRegion();
        isCYXenabled = (g_region > REGION_NONE) && (g_region < REGION_MAX);
        
        if (File::Exists(CONFIG_PATH "/disableCYX.flag"))
            isCYXenabled = false;
        
        if (isCYXenabled) {
            if(!Directory::Exists(TOP_DIR)) Directory::Create(TOP_DIR);
            if(!Directory::Exists(RESOURCES_PATH)) Directory::Create(RESOURCES_PATH);
            Directory::ChangeWorkingDirectory(RESOURCES_PATH);
#if EXTDATA_PATH_SEPERATE
            if(!Directory::Exists(EXTDATA_PATH)) Directory::Create(EXTDATA_PATH);
#endif
            if(!Directory::Exists(SAVEDATA_PATH)) Directory::Create(SAVEDATA_PATH);
            if(!Directory::Exists(HOMEFS_PATH)) Directory::Create(HOMEFS_PATH);
            if(!Directory::Exists(HOMEFS_SHARED_PATH)) Directory::Create(HOMEFS_SHARED_PATH);
        }
        ToggleTouchscreenForceOn();
        LightLock_Init(&regLock);
        LightLock_Init(&openLock);
        OnionSave::initDebug();
        if (isCYXenabled) {
            OnionSave::setupPackPaths();
            FSUSER_GetArchiveResource(&g_sdmcArcRes, SYSTEM_MEDIATYPE_SD);
            DEBUG("\n--- Initializing hooks:\n\n");
            initOnionFSHooks(Process::GetTextSize());
            DEBUG("\nLoading CYX...\n\n");
            if (g_region != REGION_NONE) {
                if R_SUCCEEDED(cyxres = CYX::Initialize()) {
                    LightEvent_Init(&mainEvent1, RESET_ONESHOT);
                    Process::OnPauseResume = [](bool isGoingToPause) {
                        CYX::playMusicAlongCTRPF(isGoingToPause);
                    };
                    if (CheckRevision()) {
                        if (File::Exists(CONFIG_PATH"/darkPalette.flag") > 0)
                            CYX::SetDarkMenuPalette();
                        DEBUG("\nCYX initialized, starting game.\n---\n\n");
                    } else {
                        g_region = REGION_MAX;
                        DEBUG("\nCYX initialized but the version is invalid; bailing out!\n\n");
                    }
                } else {
                    g_region = REGION_MAX2;
                    DEBUG("\nCYX failed to initialize with error code %08X; bailing out!\n\n", cyxres);
                }
            }
        }
    }

    // Stall plugin thread until game starts for plugin menu to safely initialize
    // Here done by waiting for a hooked function to signal a wakeup event
    void StallProcess(void) {
        Controller::Update();
        //if (Controller::GetKeysDown() & KEY_X)
        //    customBreak(1,2,3,4);
        if (isCYXenabled) {
            LightEvent_Wait(&mainEvent1);
            LightEvent_Clear(&mainEvent1);
        } else
            Sleep(Seconds(5)); // Since we've replaced it in pluginInit.cpp, we do it here
    }

    // Called when process ends
    void OnProcessExit(void) {
        CYX::Finalize();
        ToggleTouchscreenForceOn();
    }

    void InitMenu(PluginMenu &menu) {
        if (isCYXenabled) {
            menu += new MenuFolder(LANG("menuMiscellaneous"), std::vector<MenuEntry *>({
                new MenuEntry("Change server location", nullptr, serverAdrChg, "Change the server address to be connected to for the NETWORK MENU."),
                new MenuEntry("Spoof VERSION system variable", nullptr, versionSpoof, "[Useless but whatever] Modify the VERSION system variable to fool BASIC programs imposing a version blocker."),
            }));
            menu += new MenuFolder(LANG("menuExperiments"),
            "These features are freshly implemented or are still work in progress. Use these at your own risk.",
            std::vector<MenuEntry *>({
                new MenuEntry(
                    "Set FONTDEF strictness",
                    nullptr, fontGetAddrPatch,
                    "Allow FONTDEF to modify the [X] character (or help simplify using a custom non-standard font map)"
                ),
                new MenuEntry(
                    "Change editor ruler color",
                    nullptr, editorRulerPalette,
                    "Choose from one of a few palettes for the editor ruler."
                ),
                new MenuEntry(
                    "Restore CYX rescue dump",
                    nullptr, restoreRescueDump,
                    "Restore a CYX rescue dump that is obtained when the plugin closes abnormally, i.e. during a critical error or an exception."
                ),
                new MenuEntry(
                    "Correct file HMAC",
                    nullptr, validateFile,
                    "Select a file to fix its HMAC signature footer. Fixing the signature will make the file eligible for upload to the SmileBASIC server."
                ),
                new MenuEntry(
                    "Server session token hooking",
                    nullptr, tokenHooker,
                    "This hook will replace the obtainment of the NNID-based session token for the SmileBASIC server with a dummy one. This option is only for use as a test with custom servers and is discouraged to be used for the official server."
                ),
                new MenuEntry(
                    "———————————",
                    nullptr, nullptr
                ),
                new MenuEntry(
                    "experiment1",
                    nullptr, [](MenuEntry* entry) {
                        PLGSET(PLGFLG_EXPERIMENTS);
                        PANIC("experiment1(): PANIC() Test", __FILE, __LINE);
                    }
                ),
                new MenuEntry("experiment2",
                    nullptr, [](MenuEntry* entry) {
                        PLGSET(PLGFLG_EXPERIMENTS);
                        ERROR_F("experiment2(): ERROR_F() Test", __FILE, __LINE);
                    }
                ),
                new MenuEntry("experiment3",
                    nullptr, [](MenuEntry* entry) {
                        PLGSET(PLGFLG_EXPERIMENTS);
                        DANG("experiment3(): DANG() Test", __FILE, __LINE);
                    }
                ),
                new MenuEntry("experiment4",
                    nullptr, [](MenuEntry* entry) {
                        MessageBox("You tried to use this function but nothing happened.")();
                    }
                ),
            }));
            menu += new MenuEntry(
                LANG("menuCYXAPISet"),
                nullptr, cyxAPItoggle,
                "The CYX API adds various features to BASIC."
            );
            menu += new MenuEntry(
                LANG("menuPluginDiscl"),
                nullptr, pluginDisclaimer,
                "General details about this plugin"
            );
        }
        else
            menu += new MenuEntry(
                "Dummy entry",
                nullptr, nullptr,
                "This plugin is a dummy"
            );

        menu += new MenuEntry(
            "About CYX",
            nullptr, [](MenuEntry* e) {
                std::string s = "";
                s += Utils::Format("CYX Version: %s [%s]\n", STRING_VERSION, pluginSerial);
                s += Utils::Format("Built %s (Commit %07X)\n", BUILD_DATE, COMMIT_HASH);

                if (CYX::currentVersion && g_region>=REGION_JPN && g_region<=REGION_EUR)
                    s += Utils::Format("SmileBASIC Version: %s %s\n", g_regionString, CYX::PTCVersionString(CYX::currentVersion).c_str());
                
                s += Utils::Format("Current Console: %s", System::IsCitra() ? "Citra" : (System::IsNew3DS() ? "New 3DS" : "Old 3DS"));

                MessageBox("About CYX Plugin", s, DialogType::DialogOk, ClearScreen::Both)();
            },
            "General details about this plugin"
        );
#if DBG_QKSET
        menu += new MenuEntry(
            "Quick Exit",
            nullptr, [](MenuEntry* e) {
                Process::ReturnToHomeMenu();
            }
        );
#endif
    }

    void warnIfSDTooBig(void) {
        if (System::IsCitra()) return; // Citra stubs this, so I don't care what it gives
        
        double sdmcSize = (float)g_sdmcArcRes.clusterSize * g_sdmcArcRes.totalClusters;
        
        if (((u64)sdmcSize>>30) > 58 || (((u64)sdmcSize>>30) > 32 && g_sdmcArcRes.clusterSize < 65536)){
            MessageBox(
                Color::Yellow << "Warning" + ResetColor(),
                "\nThe SD Card is either too big or not ideally formatted for use with this system.\n" +
                Utils::Format(
                    "Detected size: %.2f GiB (%d KiB clusters)\n\n",
                    sdmcSize / 1073741824.0, g_sdmcArcRes.clusterSize/1024
                ) +
                "CyberYoshi64 is not responsible for slowdowns and corruption as a result of using this plugin with this SD Card."
            )();
        }
    }

    void menuSetScreenShotSettings(PluginMenu* menu, bool doEnable, u32 setHotKey) {
        bool* enabled;
        u32* hotkey;
        menu->ScreenshotSettings(&enabled, &hotkey);
        *enabled = doEnable;
        *hotkey = setHotKey;
        menu->ScreenshotFilePrefix() = SCRCAP_FPREFIX;
        menu->ScreenshotPath() = SCRCAP_PATH;
        if (!Directory::Exists(SCRCAP_PATH))
            Directory::Create(SCRCAP_PATH);
        menu->ScreenshotUpdatePaths();
    }

    void menuSetScreenShotSettings(bool doEnable, u32 setHotKey) {
        menuSetScreenShotSettings(PluginMenu::GetRunningInstance(), doEnable, setHotKey);
    }

    bool menuOpen(void) {
        return CYX::WouldOpenMenu();
    }

    const std::string about =
        "SmileBASIC-CYX\n"
        "© 2022-2024 CyberYoshi64\n"
        "Made in Germany\n\n"
        "Refer to its page on my website:\n" +
        ToggleDrawMode(Render::UNDERLINE) +
        "https://cyberyoshi64.github.io/prj/sb/cyx" +
        ToggleDrawMode(Render::UNDERLINE) + SetShake(true, true, 2) +
        "\n\nThank you for downloading! <3";

    int main(void) {
        if (isCYXenabled) {
            if (g_region != REGION_NONE && g_region < REGION_MAX) {
                Process::exceptionCallback = Exception::Handler;
                g_osFirmVer = osGetFirmVersion();
                g_osKernelVer = osGetKernelVersion();
                osGetSystemVersionDataString(&g_osNVer, &g_osCVer, g_osSysVer, sizeof(g_osSysVer));
            } else {
                
                if (CYX::wouldExit)
                    MessageBox("An error occured while setting up CYX.\n\n"+CYX::exitMessage+Utils::Format("\n(Error code %08X)", cyxres))();
                else
                    MessageBox("This application is not supported and will be closed.")();
                
                Process::ReturnToHomeMenu();
                return 1;
            }
            
            if (
                !System::IsCitra() &&
                (g_sdmcArcRes.clusterSize * g_sdmcArcRes.freeClusters) < 67108864 &&
                !MessageBox(
                    "Warning — SD Card running out of space",
                    "There's less than 64 MiB free on the SD Card.\n"
                    "It is not recommended to continue using the SD Card without freeing some space.\n\n"
                    "Do you want to risk data corruption by proceeding using this plugin?\n"
                    "(Declining will close the game immediately.)",
                    DialogType::DialogOkCancel
                )()
            ){
                Process::ReturnToHomeMenu();
                return 1;
            }

            warnIfSDTooBig();
        }

        PluginMenu *menu = new PluginMenu("CYX", about, isCYXenabled);
        menu->SynchronizeWithFrame(true);
        menu->ShowWelcomeMessage(true);
        menu->SetHexEditorState(DBG_QKSET);
        menuSetScreenShotSettings(menu, false, 0);

        if (isCYXenabled) {
            menu->Callback([](){
                BasicAPI::MenuTick();
                CYX::MenuTick();
                CYXConfirmDlg::DoTheThing();
            });

            menu->OnOpening = [](){
                return CYX::WouldOpenMenu();
            };

            menu->OnClosing = [](){
                CYX::TrySave();
            };
        }

        InitMenu(*menu);
        menu->Run();
        delete menu;
        return 0;
    }
}
