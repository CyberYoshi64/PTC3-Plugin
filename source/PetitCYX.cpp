#include "PetitCYX.hpp"
#include "BasicAPI.hpp"

namespace CTRPluginFramework {
    u32 CYX::currentVersion = 0;
    CYX::MirroredVars CYX::mirror = {0};
    std::tuple<u32, u32*, u32> CYX::soundThreadsInfo[2] = { std::tuple<u32, u32*, u32>(0xFFFFFFFF, nullptr, 0) };
    BASICEditorData* CYX::editorInstance = NULL;
    BASICGRPStructs* CYX::GraphicPage = NULL;
    BASICTextPalette* CYX::textPalette = NULL;
    BASICActiveProject* CYX::activeProject = NULL;
    PTCConfig* CYX::ptcConfig = NULL;
    FontOffFunc CYX::fontOff;
    u16* CYX::basicFontMap = NULL;
    u32 CYX::patch_FontGetOffset[2] = {0};
    u32 CYX::patch_FontGetOffsetNew[2] = {0xE3A00001,0xE3A00001}; // asm "mov r0, #1"
    std::string CYX::g_currentProject = "";
    RT_HOOK CYX::clipboardFunc = {0};
    RT_HOOK CYX::basControllerFunc = {0};
    RT_HOOK CYX::scrShotStub = {0};
    Hook CYX::soundHook;
    Hook CYX::soundHook2;
    bool CYX::forceDisableSndOnPause = false;
    char CYX::introText[512] = "SmileBASIC-CYX " STRING_VERSION "(" BUILD_DATE ")" "\n\n2022-2023 CyberYoshi64\n\n";
    char CYX::bytesFreeText[32] = " bytes free\n\n";
    bool CYX::provideCYXAPI = true;
    bool CYX::wasCYXAPIused = false;
    string16 CYX::cyxApiTextOut;
    u32 CYX::cyxApiOutc;
    u32 CYX::cyxApiLastOutv;
    u32 CYX::cyxSaveTimer = 0;
    u64 CYX::sdmcFreeSpace = 0;
    u64 CYX::sdmcTotalSpace = 0;
    s32 CYX::volumeSliderValue = 0;
    s32 CYX::rawBatteryLevel = 0;
    u8  CYX::mcuSleep = 0;
    u32 CYX::cyxUpdateSDMCStats = 0;
    u64 CYX::askQuickRestore = 0;
    bool CYX::wouldExit = false;
    std::string CYX::exitMessage;
    u32 CYX::helpPageColors[14] = {
        0xFFFFFFFF, 0xFF000818,
        0xFFFFFFFF, 0xFF0060D0,
        0xFFFFFFFF, 0xFF082098,
        0xFFFFFFFF, 0xFF042060,
        0xFFFFFFFF, 0xFF001030,
        0xFFFFFFFF, 0xFFC04000, // This is technically out of bounds in the original
        0xFF808080, 0xFF000000, // but I want some more colors in ma help pages! ;_;
    };
    RT_HOOK CYX::petcServiceTokenHook;
    RT_HOOK CYX::nnActIsNetworkAccountHook;

    void setCYXStuff(void) {
        CYX::SetAPIAvailability(Config::Get().cyx.enableAPI);
        CYX::SetFontGetAddressStrictness(Config::Get().cyx.fontdefStrict);
    }

    Result CYX::Initialize(void) {
        if (!isCYXenabled) {
            wouldExit = false;
            return 0;
        }
        Result hookErr; u32 res;
        wouldExit = true;
        if (R_FAILED(hookErr = Hooks::Init())) return hookErr;

        currentVersion = *(u32*)Hooks::offsets.versionInt;
        editorInstance = (BASICEditorData*)Hooks::offsets.editorData;
        GraphicPage = (BASICGRPStructs*)Hooks::offsets.graphicStructs;
        textPalette = (BASICTextPalette*)Hooks::offsets.consoleTextPal;
        activeProject = (BASICActiveProject*)Hooks::offsets.activeProjStr;
        ptcConfig = (PTCConfig*)Hooks::offsets.configBuf;
        basicFontMap = (u16*)Hooks::offsets.fontMapBuf;
        fontOff = (FontOffFunc)Hooks::offsets.funcFontGetOff;
        Process::Write32(Hooks::offsets.helpPagePal, (u32)helpPageColors);
        Process::Write32(Hooks::offsets.helpPageDefCol + 0, helpPageColors[1]); // Default text FG
        Process::Write32(Hooks::offsets.helpPageDefCol + 4, helpPageColors[0]); // Default help BG
        Process::CopyMemory(patch_FontGetOffset, (void*)(Hooks::offsets.funcFontGetOff+0x3c), 8);
        rtInitHook(&basControllerFunc, Hooks::offsets.funcController, (u32)CYX::controllerFuncHook);
        rtEnableHook(&basControllerFunc);
        rtInitHook(&petcServiceTokenHook, Hooks::offsets.petcSessionTokenFunc, (u32)CYX::petcTokenHookFunc);
        rtInitHook(&nnActIsNetworkAccountHook, Hooks::offsets.nnActIsNetworkAccountFunc, (u32)CYX::nnActIsNetworkAccountStub);
        rtEnableHook(&nnActIsNetworkAccountHook);
        *(char**)Hooks::offsets.bootText = introText;
        *(char**)(Hooks::offsets.bootText+4) = bytesFreeText;

        // These 2 hooks must be MitM, I just want their thread ID's for later
        soundHook.InitializeForMitm(Hooks::offsets.nnSndSoundThreadEntry1, (u32)SoundThreadHook);
        soundHook.SetFlags(USE_LR_TO_RETURN|MITM_MODE|EXECUTE_OI_AFTER_CB);
        soundHook.Enable();
        soundHook2.InitializeForMitm(Hooks::offsets.nnSndSoundThreadEntry2, (u32)SoundThreadHook2);
        soundHook2.SetFlags(USE_LR_TO_RETURN|MITM_MODE|EXECUTE_OI_AFTER_CB);
        soundHook2.Enable();
        
        // Load Config and set act appropriately
        Config::Load();
        if (Config::Get().clearCache) {
            Directory::Remove(CACHE_PATH);
            Config::Get().clearCache = false;
        }
        if (Config::Get().recoverFromException) {
            askQuickRestore = Config::Get().lastExcepDumpID;
            Config::Get().lastExcepDumpID = 0;
            Config::Get().recoverFromException = false;
        }
        if (Config::Get().cyx.set.server.serverType) {
            CYX::ReplaceServerName(Config::Get().cyx.set.server.serverName, Config::Get().cyx.set.server.serverName);
            if (Config::Get().cyx.set.server.serverType & Config::Enums::ServerType::STUB_TOKEN){
                rtEnableHook(&petcServiceTokenHook);
            }
        }
        setCYXStuff();

        // Other initializations
        BasicAPI::Initialize();
        if (res = StringArchive::Init()) {
            wouldExit = true;
            exitMessage = Utils::Format("StringArchive::Init() failed with status code 0x%08X", res);
            return res;
        }
        forceDisableSndOnPause = !(System::IsCitra()||System::IsNew3DS());
        wouldExit = false;
        return 0;
    }

    void CYX::Finalize() {
        if (!isCYXenabled) return;
        StringArchive::Exit();
        BasicAPI::Finalize();
        SaveProjectSettings();
        Config::Save();
    }
    
    void CYX::SaveProjectSettings() {
        if (g_currentProject=="") return;
        File f;
        std::string path=PROJECTSET_PATH"/P"+g_currentProject+".bin";
        if (!Directory::Exists(CONFIG_PATH)) Directory::Create(CONFIG_PATH);
        if (!Directory::Exists(PROJECTSET_PATH)) Directory::Create(PROJECTSET_PATH);
        if (File::Open(f, path, File::RWC | File::TRUNCATE) == File::SUCCESS) {
            ProjectSettings p; memset(&p, 0xFF, sizeof(p));
            p.magic = PRJSETFILEMAGIC;
            p.apiFlags = BasicAPI::flags;
            f.Write(&p, sizeof(p));
        }
        f.Close();
    }
    void CYX::LoadProjectSettings(){
        File f;
        std::string path=PROJECTSET_PATH"/P"+g_currentProject+".bin";
        if (File::Exists(path)){
            ProjectSettings p; memset(&p, 0xFF, sizeof(p));
            if (File::Open(f, path, File::READ) == File::SUCCESS)
                f.Read(&p, sizeof(p));
            f.Close();
            if (p.magic != PRJSETFILEMAGIC) {
                MessageBox(LANG("warn"), Utils::Format(LANG("cyxSettingsInvalid").c_str(), g_currentProject.c_str()))();
                File::Remove(path);
                BasicAPI::flags = APIFLAG_DEFAULT;
            } else {
                BasicAPI::flags = p.apiFlags;
            }
        }
        if (BasicAPI::flags & APIFLAG_FS_ACC_SAFE) CreateHomeFolder();
        CYXConfirmDlg::ResetUse();
    }
    void CYX::TrySave() {cyxSaveTimer = 0;}
    void CYX::MenuTick() {
        if (!isCYXenabled) return;
        std::string str;
        bool doUpdate = (!cyxSaveTimer);
        bool didUpdateManually = false;

        if (Config::Get().pluginDisclAgreed < PLUGINDISCLAIMERVER) {
            pluginDisclaimer(NULL);
            doUpdate = true;
        }
        
        if (wouldExit) {
            DANG(exitMessage, __FILE, __LINE);
            wouldExit = false;
        }

        if (askQuickRestore) {
            if (MessageBox(LANG("question"), LANG("cyxQuickRescueAsk"), DialogType::DialogYesNo)()) {
                RestoreRescueDump(Utils::Format(DUMP_PATH"/%016X.cyxdmp",askQuickRestore));
            }
            askQuickRestore = 0;
        }

        UpdateMirror();
        if (mirror.diff.currentProject || !g_currentProject.size()) {
            doUpdate = true; didUpdateManually = true;
            SaveProjectSettings();
            UTF16toUTF8(g_currentProject="", activeProject->currentProject);
            if (g_currentProject==""||g_currentProject==PTC_WORKSPACE_EXTDATANAME) g_currentProject=PTC_WORKSPACE_CYXNAME;
            LoadProjectSettings();
        }
        if (mirror.isInBasic && mirror.isBasicRunningTime == 3 && mirror.isBasicRunning2 != mirror.isBasicRunning) {
            mirror.isBasicRunning2 = mirror.isBasicRunning;
        }
        if (mirror.diff.isInBasic) {
            if (!mirror.isInBasic) doUpdate = true;
            if (R_SUCCEEDED(frdInit())) {
                if (mirror.isDirectMode) {
                    str = Utils::Format(LANG("friendListHeader").c_str(), Utils::Format(LANG("friendListEditing").c_str(), g_currentProject.c_str()).c_str());
                } else if (mirror.isInBasic) {
                    str = Utils::Format(LANG("friendListHeader").c_str(), Utils::Format(LANG("friendListPlaying").c_str(), g_currentProject.c_str()).c_str());
                } else {
                    str = Utils::Format(LANG("friendListHeader").c_str(), Utils::Format(LANG("friendListIdling").c_str(), g_currentProject.c_str()).c_str());
                }
                FRD_UpdateGameModeDescription(str.c_str());
            }
            frdExit();
        }
        if (doUpdate) {
            if (!didUpdateManually) {
                SaveProjectSettings();
            }
            cyxSaveTimer = 1800;
            Config::Save();
        } else {
           cyxSaveTimer--; 
        }
        if (!cyxUpdateSDMCStats) {
            FSUSER_GetArchiveResource(&g_sdmcArcRes, SYSTEM_MEDIATYPE_SD);
            sdmcFreeSpace = (float)g_sdmcArcRes.clusterSize * g_sdmcArcRes.freeClusters;
            sdmcTotalSpace = (float)g_sdmcArcRes.clusterSize * g_sdmcArcRes.totalClusters;
            mcuHwcInit();
            if (System::IsCitra()) {
                volumeSliderValue = 63;
                rawBatteryLevel = 100;
            } else {
                volumeSliderValue = 0;
                rawBatteryLevel = 0;
                MCUHWC_GetBatteryLevel((u8*)&rawBatteryLevel);
                MCUHWC_GetSoundSliderLevel((u8*)&volumeSliderValue);
            }
            MCUHWC_ReadRegister(0x18, &mcuSleep, 1);
            mcuSleep = !(mcuSleep & 0x6C);
            mcuHwcExit();
            cyxUpdateSDMCStats = 24;
        } else {
            cyxUpdateSDMCStats--;
        }
    }

    bool CYX::WouldOpenMenu() {
        if (!isCYXenabled) return true;
        if (mirror.isInBasic) {
            SoundEngine::PlayMenuSound(SoundEngine::Event::DESELECT);
            return false;
        }
        return true;
    }
    void CYX::UpdateMirror() {
        mirror.diff.currentProject = memcmp(activeProject->currentProject, mirror.currentProject, sizeof(mirror.currentProject))!=0;
        memcpy(mirror.currentProject, activeProject->currentProject, sizeof(mirror.currentProject));
        mirror.diff.isDirectMode = mirror.isDirectMode != *(u8*)Hooks::offsets.basicIsDirect;
        mirror.isDirectMode = *(u8*)Hooks::offsets.basicIsDirect;
        mirror.diff.isInBasic = mirror.isInBasic != *(u8*)Hooks::offsets.basicInterpretRun;
        mirror.isInBasic = *(u8*)Hooks::offsets.basicInterpretRun;
        mirror.diff.isBasicRunning = mirror.isBasicRunning != *(u8*)Hooks::offsets.basicCommandRun;
        mirror.isBasicRunning = *(u8*)Hooks::offsets.basicCommandRun;
        ++mirror.isBasicRunningTime *= !(mirror.diff.isBasicRunning);
    }

    void CYX::SetAPIAvailability(bool enabled) {
        provideCYXAPI = enabled;
    }
    bool CYX::GetAPIAvailability() {
        return provideCYXAPI;
    }
    void CYX::DiscardAPIUse() {
        wasCYXAPIused = false;
    }
    void CYX::SetAPIUse(bool enabled) {
        wasCYXAPIused = enabled;
    }
    bool CYX::WasCYXAPIUsed() {
        return wasCYXAPIused;
    }
    int CYX::scrShotStubFunc() { // Gnahh... I can't find this stinker in the code.
        return 0;
    }
    void CYX::CYXAPI_Out(BASICGenericVariable* out) {
        if (out >= (BASICGenericVariable*)cyxApiLastOutv) return;
        out->type = SBVARRAW_STRING;
        out->data = editorInstance->clipboardLength;
        out->data2 = editorInstance->clipboardData;
    }
    void CYX::CYXAPI_Out(BASICGenericVariable* out, s32 i) {
        if (out >= (BASICGenericVariable*)cyxApiLastOutv) return;
        out->type = SBVARRAW_INTEGER;
        out->data = i;
    }
    void CYX::CYXAPI_Out(BASICGenericVariable* out, double f) {
        if (out >= (BASICGenericVariable*)cyxApiLastOutv) return;
        out->type = SBVARRAW_DOUBLE;
        *(double*)&out->data = f;
    }
    void CYX::CYXAPI_Out(BASICGenericVariable* out, const char* s) {
        if (out >= (BASICGenericVariable*)cyxApiLastOutv) return;
        u32 o = cyxApiTextOut.size(), l;
        Utils::ConvertUTF8ToUTF16(CYX::cyxApiTextOut, s);
        l = cyxApiTextOut.size() - o;
        out->type = SBVARRAW_STRING;
        out->data = l;
        out->data2 = (void*)(cyxApiTextOut.c_str() + o);
    }
    void CYX::CYXAPI_Out(BASICGenericVariable* out, const std::string& s) {
        if (out >= (BASICGenericVariable*)cyxApiLastOutv) return;
        u32 o = cyxApiTextOut.size(), l;
        Utils::ConvertUTF8ToUTF16(CYX::cyxApiTextOut, s);
        l = cyxApiTextOut.size() - o;
        out->type = SBVARRAW_STRING;
        out->data = l;
        out->data2 = (void*)(cyxApiTextOut.c_str() + o);
    }

    s32 CYX::argGetInteger(BASICGenericVariable* arg) {
        if (!arg) return 0;
        if (!arg->type) return 0;
        switch (arg->type) {
            case SBVARRAW_INTEGER:
                return *(s32*)&arg->data;
            case SBVARRAW_DOUBLE:
                return (s32)(*(double*)&arg->data);
            case SBVARRAW_STRING:
                return 0;
        }
        return 0;
    }
    void CYX::argGetString(string16& out, BASICGenericVariable* arg) {
        out.clear();
        if (!arg) return;
        if (!arg->type) return;
        switch (arg->type) {
            case SBVARRAW_INTEGER:
                Utils::ConvertUTF8ToUTF16(out, Utils::Format("%ld",*(s32*)&arg->data));
                break;
            case SBVARRAW_DOUBLE:
                Utils::ConvertUTF8ToUTF16(out, Utils::ToString(*(double*)&arg->data));
                break;
            case SBVARRAW_STRING:
                out.append((u16*)arg->data2, arg->data);
                break;
        }
    }
    void CYX::argGetString(u16** ptr, u32* len, BASICGenericVariable* arg) {
        *ptr = NULL;
        *len = 0;
        if (!arg) return;
        if (!arg->type) return;
        if (arg->type == SBVARRAW_STRING) {
            *ptr = (u16*)arg->data2;
            *len = arg->data;
        }
    }
    double CYX::argGetFloat(BASICGenericVariable* arg) {
        if (!arg) return 0;
        if (!arg->type) return 0;
        switch (arg->type) {
            case SBVARRAW_INTEGER:
                return *(s32*)&arg->data;
            case SBVARRAW_DOUBLE:
                return *(double*)&arg->data;
            case SBVARRAW_STRING:
                return 0;
        }
        return 0;
    }
    u8 CYX::getSBVariableType(u32 rawType) {
        switch(rawType) {
            case SBVARRAW_INTEGER: return VARTYPE_INT;
            case SBVARRAW_DOUBLE: return VARTYPE_DOUBLE;
            case SBVARRAW_STRING: return VARTYPE_STRING;
            case SBVARRAW_ARRAY: return VARTYPE_DBLARRAY;
            //case SBVARRAW_STRARRAY: return VARTYPE_STRARRAY;
        }
        return VARTYPE_NONE;
    }
    // Function stub
    int CYX::stubBASICFunction(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv) {
        return 0;
    }
    int CYX::controllerFuncHook(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv) {
        u8 type; bool isCYX=false;
        if (argc<1) return 0;
        type = getSBVariableType(argv->type);
        if (!provideCYXAPI) {
            if (type != VARTYPE_NONE) {
                switch (type) {
                    case VARTYPE_STRING: isCYX=true; break;
                    case VARTYPE_INT:
                        if (argv->data != 0) return 3;
                        break;
                    case VARTYPE_DOUBLE:
                        if ((int)(*(double*)&argv->data) != 0) return 3;
                        break;
                }
            }
            if (outc){
                outv->type = SBVARRAW_INTEGER;
                outv->data = 1;
            }
        } else {
            cyxApiOutc = outc;
            cyxApiLastOutv = (u32)(outv+outc);
            CYX::cyxApiTextOut.clear();
            int res=BasicAPI::Parse(argv, argc, outv, outc);
            switch (res) {
                case 0: case 1: break;
                case 2: return 1; // Silent exit
                case 3: return 2; // Non-fatal quit
                default: return 3; // Fatal quit
            }
        }
        return 0;
    }

    u32 CYX::apiGetFlags(void) {
        return BasicAPI::flags;
    }
    void CYX::apiToggleFlag(u32 flag) {
        BasicAPI::flags ^= flag;
    }
    void CYX::apiEnableFlag(u32 flag) {
        BasicAPI::flags |= flag;
    }
    void CYX::apiDisableFlag(u32 flag) {
        BasicAPI::flags &= ~flag;
    }

    // Editor strings are merely buffers, so the string has to be crafted
    // with a given length parameter.
    void CYX::UTF16toUTF8(std::string& out, u16* str, u32 len) {
        string16 s16; s16.append(str, len);
        Utils::ConvertUTF16ToUTF8(out, s16);
    }
    void CYX::UTF16toUTF8(std::string& out, u16* str) {
        string16 s16 = str;
        Utils::ConvertUTF16ToUTF8(out, s16);
    }

    void CYX::ReplaceServerName(const  std::string& saveURL, const std::string& loadURL) {
        char buf[50] = {0};
        sprintf(buf, "%s" SBSERVER_LOAD_LOAD2, loadURL.c_str());
        strcpy((char*)Hooks::offsets.serverLoad2[0], buf);
        sprintf(buf, "%s" SBSERVER_LOAD_LOAD2, loadURL.c_str());
        strcpy((char*)Hooks::offsets.serverLoad2[1], buf);
        sprintf(buf, "%s" SBSERVER_SAVE_SAVE3, saveURL.c_str());
        strcpy((char*)Hooks::offsets.serverSave3[0], buf);
        sprintf(buf, "%s" SBSERVER_SAVE_SAVE3, saveURL.c_str());
        strcpy((char*)Hooks::offsets.serverSave3[1], buf);
        sprintf(buf, "%s" SBSERVER_SAVE_SHOW2, saveURL.c_str());
        strcpy((char*)Hooks::offsets.serverShow2, buf);
        sprintf(buf, "%s" SBSERVER_SAVE_LIST2, saveURL.c_str() );
        strcpy((char*)Hooks::offsets.serverList2, buf);
        sprintf(buf, "%s" SBSERVER_LOAD_INFO2, loadURL.c_str());
        strcpy((char*)Hooks::offsets.serverInfo2, buf);
        sprintf(buf, "%s" SBSERVER_SAVE_DELETE2, saveURL.c_str());
        strcpy((char*)Hooks::offsets.serverDelete2, buf);
        sprintf(buf, "%s" SBSERVER_SAVE_SHOPLIST2, saveURL.c_str());
        strcpy((char*)Hooks::offsets.serverShopList2, buf);
        sprintf(buf, "%s" SBSERVER_SAVE_PREPURCHASE2, saveURL.c_str());
        strcpy((char*)Hooks::offsets.serverPrepurchase2, buf);
        sprintf(buf, "%s" SBSERVER_SAVE_PURCHASE2, saveURL.c_str());
        strcpy((char*)Hooks::offsets.serverPurchase2, buf);
    }

    void CYX::ChangeBootText(const char* text, const char* bytfre) {
        if (!text || strlen(text)>sizeof(introText)) return;
        sprintf(introText, "%s", text);
        if (!bytfre || strlen(bytfre)>sizeof(bytesFreeText)) return;
        sprintf(bytesFreeText, "%s", bytfre);
    }
    
    // To be moved to Config for customizability
    void CYX::SetDarkMenuPalette() {
        *(u32*)(Hooks::offsets.colorKeybBack) = 0xFF100800;
        *(u32*)(Hooks::offsets.colorSearchBack) = 0xFFFFA000;
        *(u32*)(Hooks::offsets.colorFileCreatorBack) = 0xFFC0FF80;
        *(u32*)(Hooks::offsets.colorFileDescBack) = 0xFF6868A0;
        *(u32*)(Hooks::offsets.colorActiveProjLbl) = 0xFF40C000;
        *(u32*)(Hooks::offsets.colorSetSmToolLbl) = 0xFF40C000;
        *(u32*)(Hooks::offsets.colorSetKeyRep) = 0xFF80D8FF;
        *(u32*)(Hooks::offsets.colorSetKeyTL) = 0xFFFFA800;
    }

    std::string CYX::GetProgramSlotFileName(u8 slot) {
        if (slot >= 4) return "";
        std::string f;
        UTF16toUTF8(f, editorInstance->programSlot[slot].file_name, editorInstance->programSlot[slot].file_name_len);
        return f;
    }

    std::string CYX::PTCVersionString(u32 ver) {
        std::string out;
        out += Utils::Format("%d.",(ver>>24)&255);
        out += Utils::Format("%d.",(ver>>16)&255);
        out += Utils::Format("%d",(ver>>8)&255);
        if (ver&255) out += Utils::Format("-%d",ver&255);
        return out;
    }

    // Quite bold, however, I doubt SmileBoom can push out updates
    // so I'd say this strictness is granted.
    bool CYX::isPTCVersionValid(u32 ver) {
        if ((u8)(ver>>24) != 3) return false;
        if ((u8)(ver>>16) > 6) return false;
        if ((u8)(ver>>8) > 3) return false;
        if (ver&255) return false;
        return true;
    }

    std::string CYX::ColorPTCVerValid(u32 ver, u32 ok, u32 ng) {
        char c[5]={0}; c[0]=0x1b;
        if (CYX::isPTCVersionValid(ver)) {
            if (ok == CYX__COLORVER_NOCOLOR) {
                return ResetColor();
            } else {
                return ""<<Color(ok);
            }
        } else {
            if (ng == CYX__COLORVER_NOCOLOR) {
                return ResetColor();
            } else {
                return ""<<Color(ng);
            }
        }
        return (c);
    }

    void CYX::CreateHomeFolder(const std::string& s) {
        std::string path = HOMEFS_PATH"/P"+s;
        if (!Directory::Exists(HOMEFS_PATH)) Directory::Create(HOMEFS_PATH);
        if (!Directory::Exists(path)) Directory::Create(path);
    }
    void CYX::CreateHomeFolder() {
        CreateHomeFolder(g_currentProject);
    }
    std::string CYX::GetExtDataFolderName() {
        if (g_currentProject == PTC_WORKSPACE_CYXNAME) return PTC_WORKSPACE_EXTDATANAME;
        return g_currentProject;
    }
    std::string CYX::GetHomeFolder() {
        return HOMEFS_PATH"/P"+g_currentProject;
    }
    std::string CYX::GetHomeFolder(std::string project) {
        if (project == PTC_WORKSPACE_EXTDATANAME) return HOMEFS_PATH "/P" PTC_WORKSPACE_CYXNAME;
        return HOMEFS_PATH"/P"+project;
    }

    void CYX::SetFontGetAddressStrictness(bool on) {
        if (on)
            Process::CopyMemory((char*)(Hooks::offsets.funcFontGetOff+0x3c), patch_FontGetOffset, 8);
        else
            Process::CopyMemory((char*)(Hooks::offsets.funcFontGetOff+0x3c), patch_FontGetOffsetNew, 8);
        rtFlushInstructionCache((char*)Hooks::offsets.funcFontGetOff, 0x44);
    }

    void CYX::RestoreRescueDump(const std::string& path) {
        File f;
        if (File::Open(f, path, File::READ)) {
            MessageBox(LANG("error"), LANG("fileOpenFail"))();
            return;
        }
        CYXDumpHeader h;
        f.Read(&h, sizeof(h));
        if (h.magic != *(u64*)CYXDMPHDR_MAGIC) {
            MessageBox(LANG("error"), LANG("fileSignatureFail"))();
            return;
        }
        if (h.version != CYXDMPHDR_VERSION) {
            MessageBox(LANG("error"), LANG("fileVersionMismatch"))();
            return;
        }
        for (u32 i=0; i<h.blobCount; i++) {
            u32 index;
            if (strncmp(h.blobName[i],"PRG",3)==0) {
                index = h.blobName[i][3] - '0';
                f.Read(editorInstance->programSlot[index].text, h.blobBufSize[i]);
                editorInstance->programSlot[index].text_len = editorInstance->programSlot[index].text_og_len = h.blobDataLen[i] / 2;
                editorInstance->programSlot[index].chars_left = 1048576 - editorInstance->programSlot[index].text_len;
                Process::WriteString((u32)editorInstance->programSlot[index].file_name, h.blobName[i]+5, StringFormat::Utf16);
                editorInstance->programSlot[index].file_name_len = MAX(0, strlen(h.blobName[i]+5));
            } else if (strncmp(h.blobName[i],"CLP",3)==0) {
                index = h.blobName[i][3] - '0';
                f.Read(editorInstance->clipboardData, h.blobBufSize[i]);
                editorInstance->clipboardLength = h.blobDataLen[i] / 2;
            } else if (strncmp(h.blobName[i],"GRP",3)==0) {
                index = h.blobName[i][3] - '0';
                f.Read(GraphicPage->grp[index].workBuf, h.blobBufSize[i]);
                memcpy(GraphicPage->grp[index].dispBuf, GraphicPage->grp[index].workBuf, h.blobBufSize[i]);
            } else {
                OSD::Notify(Utils::Format("CYX::RestoreRescueDump() - IDK what '%s' is!", h.blobName[i]));
            }
        }
        f.Close();
        MessageBox(LANG("success"), LANG("operationGood"))();
    }

    // Thanks to the CTGP-7 Open Source Project!
    void CYX::SoundThreadHook(){ // SoundThreadImpl
        svcGetThreadId(&std::get<0>(soundThreadsInfo[0]), CUR_THREAD_HANDLE);
        std::get<1>(soundThreadsInfo[0]) = (u32*)getThreadLocalStorage();
        ctrpfHook__ExecuteOriginalFunction();
    }
    void CYX::SoundThreadHook2(){ // SoundThread2
        svcGetThreadId(&std::get<0>(soundThreadsInfo[1]), CUR_THREAD_HANDLE);
        std::get<1>(soundThreadsInfo[1]) = (u32*)getThreadLocalStorage();
        ctrpfHook__ExecuteOriginalFunction();
    }

	void CYX::playMusicAlongCTRPF(bool playMusic) {
		static bool isPlayMusic = false;
		if (forceDisableSndOnPause)
			playMusic = false;
		if (isPlayMusic == playMusic) return;
		isPlayMusic = playMusic;
		static u32 tlsBackup[2];
		static s32 prioBackup[2];
        for (u32 i = 0; i < 2; i++) {
            u32 soundThreadID = std::get<0>(soundThreadsInfo[i]);
            u32* soundThreadTls = std::get<1>(soundThreadsInfo[i]);
            Handle soundThreadHandle; bool perf=true;
            if (R_FAILED(svcOpenThread(&soundThreadHandle, CUR_PROCESS_HANDLE, soundThreadID))) perf=false;
            if (soundThreadID == 0xFFFFFFFF) perf=false;
            if (soundThreadHandle == 0) perf=false;
            if (perf) {
                if (playMusic) {
                    tlsBackup[i] = *soundThreadTls;
                    *soundThreadTls = THREADVARS_MAGIC;
                    svcGetThreadPriority(&prioBackup[i], soundThreadHandle);
                    svcSetThreadPriority(soundThreadHandle, FwkSettings::Get().ThreadPriority - 1);
                } else {
                    *soundThreadTls = tlsBackup[i];
                    svcSetThreadPriority(soundThreadHandle, prioBackup[i]);
                }
            }
            svcCloseHandle(soundThreadHandle);
        }
	}

    void CYX::ResetServerLoginState() {
        *(u8*)Hooks::offsets.nnActConnectRequired = 1;
        *(u8*)Hooks::offsets.nnActNetworkTimeValidated = 0;
        memset((char*)Hooks::offsets.petcAccountToken, 0, 512);
    }

    int CYX::petcTokenHookFunc() {
        *(u8*)Hooks::offsets.nnActConnectRequired = 0;
        *(u8*)Hooks::offsets.nnActNetworkTimeValidated = 1;

        memset((char*)Hooks::offsets.petcAccountToken, 0, 512); // Set a bogus token here
        sprintf((char*)Hooks::offsets.petcAccountToken,
            "QUFCQiwgb2ghIEhlbGxvIHRoZXJlLCBzaXIgb3IgbWFkYW0hIEkgaG9wZSBpIGFpbid0IGludHJ1ZGluZyB0aGUgcGVhY2UgaGVyZSAuLi4K"
        );
        
        return 0; // Indicate success
    }
    int CYX::nnActIsNetworkAccountStub() {
        return 1; // What do you expect? We spoof it to say "Yes".
    }

}