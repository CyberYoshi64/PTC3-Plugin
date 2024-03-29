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
    ptcScreen* CYX::ptcScreens = NULL;
    FontOffFunc CYX::fontOff;
    PrintCharFunc CYX::printConFunc;
    BSAStringAllocFunc CYX::bsaAllocString;
    BSAErrorFunc CYX::bsaError;
    u16* CYX::basicFontMap = NULL;
    u32 CYX::patch_FontGetOffset[2] = {0};
    u32 CYX::patch_FontGetOffsetNew[2] = {0xE3A00001,0xE3A00001}; // asm "mov r0, #1"
    std::string CYX::g_currentProject = "";
    RT_HOOK CYX::clipboardFunc = {0};
    RT_HOOK CYX::basControllerFunc = {0};
    Hook CYX::sendKeyHook;
    Hook CYX::mainThreadEntryHook;
    Hook CYX::nnExitHook;
    Hook CYX::soundHook;
    Hook CYX::soundHook2;
    bool CYX::forceDisableSndOnPause = false;
    char CYX::introText[512] = "SmileBASIC-CYX - Ver. " STRING_VERSION "\n(C) 2022-2024 CyberYoshi64\n\n";
    char CYX::bytesFreeText[32] = " bytes free\n\n";
    bool CYX::provideCYXAPI = true;
    bool CYX::wasCYXAPIused = false;
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
    const char16_t* langNames[] = {
        u"ja", u"en", u"fr", u"de",
        u"it", u"es", u"zh", u"ko",
        u"nl", u"pt", u"ru", u"tw"
    };

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
        ptcScreens = (ptcScreen*)Hooks::offsets.screenBuf;
        basicFontMap = (u16*)Hooks::offsets.fontMapBuf;
        fontOff = (FontOffFunc)Hooks::offsets.funcFontGetOff;
        printConFunc = (PrintCharFunc)Hooks::offsets.funcPrintCon;
        bsaAllocString = (BSAStringAllocFunc)Hooks::offsets.funcBsaStringAlloc;
        bsaError = (BSAErrorFunc)Hooks::offsets.funcBSAError;
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

        soundHook.InitializeForMitm(Hooks::offsets.nnSndSoundThreadEntry1, (u32)SoundThreadHook);
        soundHook.SetFlags(USE_LR_TO_RETURN|MITM_MODE|EXECUTE_OI_AFTER_CB);
        soundHook.Enable();
        soundHook2.InitializeForMitm(Hooks::offsets.nnSndSoundThreadEntry2, (u32)SoundThreadHook2);
        soundHook2.SetFlags(USE_LR_TO_RETURN|MITM_MODE|EXECUTE_OI_AFTER_CB);
        soundHook2.Enable();
        sendKeyHook.InitializeForMitm(Hooks::offsets.funcSendKeyCode, (u32)sendKeyHookFunc);
        sendKeyHook.SetFlags(USE_LR_TO_RETURN|MITM_MODE|EXECUTE_OI_AFTER_CB);
        sendKeyHook.Enable();
        mainThreadEntryHook.InitializeForMitm(Hooks::offsets.funcMainEntry, (u32)ptcMainEntryHookFunc);
        mainThreadEntryHook.SetFlags(USE_LR_TO_RETURN|MITM_MODE|EXECUTE_OI_AFTER_CB);
        mainThreadEntryHook.Enable();
        nnExitHook.InitializeForMitm(Hooks::offsets.funcMainLoop1, (u32)nnExitHookFunc);
        nnExitHook.SetFlags(MITM_MODE|EXECUTE_OI_AFTER_CB);
        nnExitHook.Enable();

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

    void CYX::ValidateSaveData() {
        StringVector files, compromised;
        u32 index, size; File f;
        bool cancel = false;
        Directory dir, dir2;
        StringVector dv1, dv2;

        Process::Pause();
        OSD::Lock();

        Screen scr = OSD::GetTopScreen();
        Directory::Open(dir, EXTDATA_PATH);
        size = dir.ListDirectories(dv1);
        for (index = 0; index < size; index++) {
            scr.DrawRect(0, 0, 320, 64, Color::Black);
            scr.Draw(Utils::Format("Listing files to verify...(%4d/%4d)", index + 1, size), 4, 4);
            OSD::SwapBuffers();
            Directory::Open(dir2, EXTDATA_PATH"/" + dv1.at(index));
            dir2.ListFiles(dv2);
            for (u32 i = 0; i < dv2.size(); i++) {
                files.push_back(dv1.at(index) + "/" + dv2.at(i));
            }
            dir2.Close();
            dv2.clear();
        }
        dir.Close();
        dv1.clear();

        size = files.size();
        u8 digest[20], oldDigest[20];
        u32 fsize, fsize2, chunk;
        char* hmac = (char*)Hooks::offsets.serverHMACKey;
        int hmac_len = strlen(hmac);
        #define BUFFER_SIZE 262144
        void* buf = new u8[BUFFER_SIZE];
        if (!buf) abort();
        SHA1_HMAC::CTX hmacCtx;
        
        if (File::Exists(VERFAIL_PATH))
            File::Remove(VERFAIL_PATH);
        File::Create(VERFAIL_PATH);
        
        for (index = 0; index < size; index++) {
            scr.DrawRect(0, 0, 320, 64, Color::Black);
            scr.Draw(Utils::Format("Verifying file...(%5d/%5d)", index + 1, size), 4, 4);
            scr.Draw(files.at(index), 4, 16);
            if (compromised.size())
                scr.Draw(Utils::Format("%5d file(s) compromised", compromised.size()), 4, 32);
            scr.Draw(Utils::Format("Press X to cancel", compromised.size()), 4, 44);
            OSD::SwapBuffers();
            if (!File::Open(f, EXTDATA_PATH"/" + files.at(index), File::READ)) {
                SHA1_HMAC::Init(&hmacCtx, (u8*)hmac, hmac_len);
                fsize = fsize2 = f.GetSize()-20;
                f.Seek(-20, File::END);
                f.Read(oldDigest, 20);
                f.Seek(0, File::SET);
                while (fsize) {
                    Controller::Update();
                    if (Controller::GetKeysDown() & KEY_X) {
                        cancel = true;
                        break;
                    }

                    chunk = fsize > BUFFER_SIZE ? BUFFER_SIZE : fsize;
                    f.Read(buf, chunk);
                    SHA1_HMAC::Update(&hmacCtx, (u8*)buf, chunk);
                    fsize -= chunk;
                }
            }
            f.Close();
            if (cancel) break;
            SHA1_HMAC::Final(digest, &hmacCtx);
            if (memcmp(digest, oldDigest, 20)) {
                compromised.push_back(files.at(index));
                if (!File::Open(f, VERFAIL_PATH, File::WRITE|File::APPEND)) {
                    f.Write(files.at(index).c_str(), files.at(index).size());
                    f.Write("\n", 1);
                }
                f.Close();
            }
        }
        ::operator delete(buf);
        #undef BUFFER_SIZE

        scr.DrawRect(0, 0, 320, 64, Color::Black);
        OSD::SwapBuffers();
        scr.DrawRect(0, 0, 320, 64, Color::Black);
        scr.Draw(cancel ? "Cancelled." : "Done.", 4, 4);
        scr.Draw("You can view the list of compromised files in", 4, 16);
        scr.Draw("the following file:", 4, 26);
        scr.Draw("sdmc:" VERFAIL_PATH, 4, 38);
        OSD::SwapBuffers();
        Sleep(Seconds(3));
        OSD::Unlock();
        Process::Play();
    }

    void CYX::TrySave() {cyxSaveTimer = 0;}
    
    void CYX::MenuTick() {
        if (!isCYXenabled) return;
        std::string str;
        bool doUpdate = (!cyxSaveTimer);
        bool didUpdateManually = false;

        if (Config::Get().validateSaveData) {
            CYX::ValidateSaveData();
            Config::Get().validateSaveData = false;
            doUpdate = true;
        }
        
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
            if (!mirror.isInBasic) {
                doUpdate = true;
                mcuSetSleep(true);
            }
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
            if (System::IsCitra()) {
                volumeSliderValue = 63;
                rawBatteryLevel = 100;
                mcuSleep = false;
            } else {
                if R_SUCCEEDED(mcuHwcInit()) {
                    volumeSliderValue = 0;
                    rawBatteryLevel = 0;
                    MCUHWC_GetBatteryLevel((u8*)&rawBatteryLevel);
                    MCUHWC_GetSoundSliderLevel((u8*)&volumeSliderValue);
                    MCUHWC_ReadRegister(0x18, &mcuSleep, 1);
                    mcuSleep = !(mcuSleep & 0x6C);
                }
                mcuHwcExit();
            }
            cyxUpdateSDMCStats = 24;
        } else {
            cyxUpdateSDMCStats--;
        }
    }

    bool CYX::WouldOpenMenu() {
        if (!isCYXenabled)
            return true;
        
        //if (mirror.isInBasic) {
        //    SoundEngine::PlayMenuSound(SoundEngine::Event::DESELECT);
        //    return false;
        //}
        
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
    bool CYX::WasCYXAPIUsed() {
        return wasCYXAPIused;
    }
    void CYX::sendKeyHookFunc(u32* ptr1, u32 key) {
        if (key == 0x10023) return;
        // OSD::Notify(Utils::Format("%08X %08X", (u32)ptr1, key));
        
        __asm__ __volatile__ (
            "@ Set registers for original function"
            "ldr r0, %0 \n"
            "ldr r1, %1 \n"
            : "=m"(ptr1) : "m"(key)
        );
        ctrpfHook__ExecuteOriginalFunction();
    }
    void CYX::ptcMainEntryHookFunc(u32 r0) {
        extern LightEvent mainEvent1;
        LightEvent_Signal(&mainEvent1);

        if (R_SUCCEEDED(frdInit())) {
            FRD_UpdateGameModeDescription(Utils::Format(LANG("friendListHeader").c_str(), Utils::Format(LANG("friendListIdling").c_str(), PTC_WORKSPACE_CYXNAME).c_str()).c_str());
        }
        frdExit();

        __asm__ __volatile__ ("ldr r0, %0\n" : "=m"(r0));
        ctrpfHook__ExecuteOriginalFunction();
    }
    void CYX::nnExitHookFunc(u32 r0) {
        __asm__ __volatile__ ("ldr r0, %0\n" : "=m"(r0));
        ctrpfHook__ExecuteOriginalFunction();
    }

    int CYX::bsaGetInteger(BASICGenericVariable* a, int* out) {
        if (!a || !a->type) return 0;
        return a->type->getInteger(a, out);
    }

    int CYX::bsaGetDouble(BASICGenericVariable* a, double* out) {
        if (!a || !a->type) return 0;
        return a->type->getDouble(a, out);
    }

    int CYX::bsaGetFloat(BASICGenericVariable* a, float* out) {
        if (!a || !a->type) return 0;
        return a->type->getFloat(a, out);
    }

    u16* CYX::bsaGetString(BASICGenericVariable* a, int* len) {
        if (!a || !a->type) return NULL;
        u16* buf = a->type->getString(a, len);
        return buf;
    }

    int CYX::bsaSetInteger(BASICGenericVariable* a, int in) {
        if (!cyxApiOutc || !a || !a->type) return 0;
        return a->type->setInteger(a, in);
    }

    int CYX::bsaSetDouble(BASICGenericVariable* a, double in) {
        if (!cyxApiOutc || !a || !a->type) return 0;
        return a->type->setDouble(a, in);
    }

    int CYX::bsaSetString(BASICGenericVariable* a, const std::string& str) {
        if (!cyxApiOutc || !a || !a->type) return 0;
        void* ptr;
        string16 s16; Utils::ConvertUTF8ToUTF16(s16, str);
        if (!bsaAllocString(&ptr, s16.size(), (u16*)s16.c_str()))
            return bsaError(PTCERR_OUT_OF_MEMORY, -1, 0);
        return a->type->setString(a, &ptr);
    }

    int CYX::bsaSetString(BASICGenericVariable* a, const string16& str) {
        if (!cyxApiOutc || !a || !a->type) return 0;
        void* ptr;
        if (!bsaAllocString(&ptr, str.size(), (u16*)str.c_str()))
            return bsaError(PTCERR_OUT_OF_MEMORY, -1, 0);
        return a->type->setString(a, &ptr);
    }

    int CYX::bsaSetStringRaw(BASICGenericVariable* a, void* ptr) {
        if (!cyxApiOutc || !a || !a->type) return 0;
        return a->type->setString(a, &ptr);
    }

    // Function stub
    int CYX::stubBASICFunction(BSAFuncStack* stk) {
        return 0;
    }
    int CYX::controllerFuncHook(BSAFuncStack* stk) {
        if (stk->argc<1) return bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);
        if (!provideCYXAPI) {
            int type;
            if (bsaGetInteger(stk->argv, &type))
                return bsaError(PTCERR_TYPE_MISMATCH, 0, 0);
            if (stk->outc && bsaSetInteger(stk->outv, type == 0 ? 1 : 0)) // Just indicate N/C instead of "Incompatible statement"
                return 3;
        } else {
            cyxApiOutc = stk->outc;
            wasCYXAPIused = true;
            return BasicAPI::Parse(stk);
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