#include "PetitCYX.hpp"
#include "BasicAPI.hpp"

namespace CTRPluginFramework {
    u32 CYX::currentVersion = 0;
    CYX::MirroredVars CYX::mirror = {0};
    std::tuple<u32, u32*, u32> CYX::soundThreadsInfo[1] = { std::tuple<u32, u32*, u32>(0xFFFFFFFF, nullptr, 0) };
    BASICEditorData* CYX::editorInstance = NULL;
    BASICGRPStructs* CYX::GraphicPage = NULL;
    BASICTextPalette* CYX::textPalette = NULL;
    BASICActiveProject* CYX::activeProject = NULL;
    CYX::PTCConfig* CYX::ptcConfig = NULL;
    FontOffFunc CYX::fontOff;
    u16* CYX::basicFontMap = NULL;
    u32 CYX::patch_FontGetOffset[2] = {0};
    u32 CYX::patch_FontGetOffsetNew[2] = {0xE3A00001,0xE3A00001};
    std::string CYX::g_currentProject = "";
    RT_HOOK CYX::clipboardFunc = {0};
    RT_HOOK CYX::basControllerFunc = {0};
    RT_HOOK CYX::scrShotStub = {0};
    Hook CYX::soundHook;
    char CYX::introText[512] = "SmileBASIC-CYX " STRING_VERSION "\nBuild " STRING_BUILD "\n\n2022-2023 CyberYoshi64\n\n";
    char CYX::bytesFreeText[32] = " bytes free\n\n";
    bool CYX::provideCYXAPI = true;
    bool CYX::wasCYXAPIused = false;
    u32 CYX::cyxApiOutType = 0;
    string16 CYX::cyxApiTextOut;
    double CYX::cyxApiFloatOut = 0;
    s32 CYX::cyxApiIntOut = 0;

    Result CYX::Initialize(void) {
        Result hookErr;
        if (R_FAILED(hookErr = Hooks::Init())) return hookErr;

        currentVersion = *(u32*)Hooks::offsets.versionInt;
        editorInstance = (BASICEditorData*)Hooks::offsets.editorData;
        GraphicPage = (BASICGRPStructs*)Hooks::offsets.graphicStructs;
        textPalette = (BASICTextPalette*)Hooks::offsets.consoleTextPal;
        activeProject = (BASICActiveProject*)Hooks::offsets.activeProjStr;
        ptcConfig = (PTCConfig*)Hooks::offsets.configBuf;
        basicFontMap = (u16*)Hooks::offsets.fontMapBuf;
        fontOff = (FontOffFunc)Hooks::offsets.funcFontGetOff;
        Process::CopyMemory(patch_FontGetOffset, (void*)(Hooks::offsets.funcFontGetOff+0x3c), 8);
        rtInitHook(&basControllerFunc, Hooks::offsets.funcController, (u32)CYX::controllerFuncHook);
        rtEnableHook(&basControllerFunc);
        *(char**)Hooks::offsets.bootText = introText;
        *(char**)(Hooks::offsets.bootText+4) = bytesFreeText;
        LoadSettings();
        BasicAPI::Initialize();

        return 0;
    }

    void CYX::Finalize(){
        BasicAPI::Finalize();
        SaveProjectSettings();
        SaveSettings();
    }
    void CYX::SaveProjectSettings(){
        if (g_currentProject=="") return;
        File f;
        std::string path=PROJECTSET_PATH"/P"+g_currentProject+".bin";
        if (!Directory::IsExists(CONFIG_PATH)) Directory::Create(CONFIG_PATH);
        if (!Directory::IsExists(PROJECTSET_PATH)) Directory::Create(PROJECTSET_PATH);
        if (File::Open(f, path, File::RWC | File::TRUNCATE) == File::SUCCESS){
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
            if (p.magic != PRJSETFILEMAGIC){
                MessageBox("The project settings for "+g_currentProject+" is invalid.\n\nThe settings will be reset for this project.")();
                File::Remove(path);
                BasicAPI::flags = APIFLAG_DEFAULT;
            } else {
                BasicAPI::flags = p.apiFlags;
            }
        }
        if (BasicAPI::flags & APIFLAG_FS_ACC_SAFE) CreateHomeFolder();
    }
    void CYX::MenuTick(){
        char str[128]={0};
        UpdateMirror();
        if (mirror.diff.currentProject || !g_currentProject.size()){
            SaveProjectSettings();
            UTF16toUTF8(g_currentProject="", activeProject->currentProject);
            if (g_currentProject==""||g_currentProject=="###") g_currentProject="$DEFAULT";
            //OSD::Notify(g_currentProject);
            if (R_SUCCEEDED(frdInit())){
                sprintf(str,"Using ＣＹＸ %s\nIn %s", STRING_VERSION, g_currentProject.c_str());
                FRD_UpdateGameModeDescription(str);
            }
            frdExit();
            LoadProjectSettings();
        }
        if (mirror.diff.isDirectMode){
            //OSD::Notify(Utils::Format("isDirectMode: %d", mirror.isDirectMode));
        }
        if (mirror.isInBasic && mirror.isBasicRunningTime == 3 && mirror.isBasicRunning2 != mirror.isBasicRunning){
            mirror.isBasicRunning2 = mirror.isBasicRunning;
            //OSD::Notify(Utils::Format("isBasicRunning: %d", mirror.isBasicRunning));
        }
        if (mirror.diff.isInBasic){
            if (!mirror.isInBasic) SaveProjectSettings();
            //OSD::Notify(Utils::Format("isInBasic: %d", mirror.isInBasic));
        }
    }
    bool CYX::WouldOpenMenu(){
        return !mirror.isInBasic;
    }
    void CYX::UpdateMirror(){
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

    void CYX::SetAPIAvailability(bool enabled){
        provideCYXAPI = enabled;
    }
    bool CYX::GetAPIAvailability(){
        return provideCYXAPI;
    }
    void CYX::DiscardAPIUse(){
        wasCYXAPIused = false;
    }
    void CYX::SetAPIUse(bool enabled){
        wasCYXAPIused = enabled;
    }
    bool CYX::WasCYXAPIUsed(){
        return wasCYXAPIused;
    }
    int CYX::scrShotStubFunc() {
        return 0;
    }
    void CYX::CYXAPI_Out(){
        cyxApiOutType = 3;
        cyxApiTextOut.clear();
    }
    void CYX::CYXAPI_Out(s32 i){
        cyxApiOutType = 2;
        cyxApiTextOut.clear();
        cyxApiIntOut = i;
    }
    void CYX::CYXAPI_Out(double f){
        cyxApiOutType = 1;
        cyxApiTextOut.clear();
        cyxApiFloatOut = f;
    }
    void CYX::CYXAPI_Out(const char* s){
        cyxApiOutType = 0;
        cyxApiTextOut.clear();
        Utils::ConvertUTF8ToUTF16(cyxApiTextOut, s);
    }
    void CYX::CYXAPI_Out(const std::string& s){
        cyxApiOutType = 0;
        CYX::cyxApiTextOut.clear();
        Utils::ConvertUTF8ToUTF16(CYX::cyxApiTextOut, s);
    }

    s32 CYX::argGetInteger(BASICGenericVariable* arg){
        if (!arg) return 0;
        if (!arg->type) return 0;
        switch (arg->type){
            case SBVARRAW_INTEGER:
                return *(s32*)&arg->data;
            case SBVARRAW_DOUBLE:
                return (s32)(*(double*)&arg->data);
            case SBVARRAW_STRING:
                CYXAPI_Out("W: Can't convert string to s32"); break;
        }
        return 0;
    }
    void CYX::argGetString(string16& out, BASICGenericVariable* arg){
        out.clear();
        if (!arg) return;
        if (!arg->type) return;
        switch (arg->type){
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
    void CYX::argGetString(u16** ptr, u32* len, BASICGenericVariable* arg){
        *ptr = NULL;
        *len = 0;
        if (!arg) return;
        if (!arg->type) return;
        if (arg->type == SBVARRAW_STRING){
            *ptr = (u16*)arg->data2;
            *len = arg->data;
        }
    }
    double CYX::argGetFloat(BASICGenericVariable* arg){
        if (!arg) return 0;
        if (!arg->type) return 0;
        switch (arg->type){
            case SBVARRAW_INTEGER:
                return *(s32*)&arg->data;
            case SBVARRAW_DOUBLE:
                return *(double*)&arg->data;
            case SBVARRAW_STRING:
                CYXAPI_Out("W: Can't convert string to s32"); break;
        }
        return 0;
    }
    u8 CYX::getSBVariableType(u32 rawType){
        switch(rawType){
            case SBVARRAW_INTEGER: return VARTYPE_INT;
            case SBVARRAW_DOUBLE: return VARTYPE_DOUBLE;
            case SBVARRAW_STRING: return VARTYPE_STRING;
            case SBVARRAW_ARRAY: return VARTYPE_DBLARRAY;
            //case SBVARRAW_STRARRAY: return VARTYPE_STRARRAY;
        }
        return VARTYPE_NONE;
    }
    // Function stub
    int CYX::stubBASICFunction(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv){
        DEBUG("CYX: Function %08X (%x/%x) %p", selfPtr, argc, outc, a4);
        return 0;
    }
    int CYX::controllerFuncHook(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv){
        u8 type; bool isCYX=false;
        if (argc<1) return 0;
        type = getSBVariableType(argv->type);
        if (type != VARTYPE_NONE){
            switch (type){
                case VARTYPE_STRING: isCYX=true; break;
                case VARTYPE_INT:
                    if (argv->data != 0) return 3;
                    break;
                case VARTYPE_DOUBLE:
                    if ((int)(*(double*)&argv->data) != 0) return 3;
                    break;
            }
        }
        if (provideCYXAPI && isCYX){
            CYX::cyxApiTextOut.clear();
            int res=BasicAPI::Parse(argv, argc);
            switch (res){
                case 0: case 1: break;
                case 2: return 1; // Silent exit
                case 3: return 2; // Non-fatal quit
                default: return 3; // Fatal quit
            }
            if (outc){
                switch (cyxApiOutType){
                case 0:
                    outv->type = SBVARRAW_STRING;
                    outv->data = cyxApiTextOut.size();
                    outv->data2 = (void*)cyxApiTextOut.c_str();
                    break;
                case 1:
                    outv->type = SBVARRAW_DOUBLE;
                    *(double*)&outv->data = cyxApiFloatOut;
                    break;
                case 2:
                    outv->type = SBVARRAW_INTEGER;
                    outv->data = cyxApiIntOut;
                    break;
                case 3:
                    outv->type = SBVARRAW_STRING;
                    outv->data = editorInstance->clipboardLength;
                    outv->data2 = editorInstance->clipboardData;
                    break;
                }
            }
        } else {
            if (outc){
                outv->type = SBVARRAW_INTEGER;
                outv->data = 1 | (provideCYXAPI<<28);
            }
        }
        return 0;
    }

    // Editor strings are merely buffers, so the string has to be crafted
    // with a given length parameter.
    void CYX::UTF16toUTF8(std::string& out, u16* str, u32 len){
        string16 s16; s16.append(str, len);
        Utils::ConvertUTF16ToUTF8(out, s16);
    }
    void CYX::UTF16toUTF8(std::string& out, u16* str){
        string16 s16 = str;
        Utils::ConvertUTF16ToUTF8(out, s16);
    }

    void CYX::LoadSettings(){} // To be used soon
    void CYX::SaveSettings(){} // Not needed for now

    void CYX::ReplaceServerName(const  std::string& saveURL, const std::string& loadURL){
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

    void CYX::ChangeBootText(const char* text, const char* bytfre){
        if (!text || strlen(text)>sizeof(introText)) return;
        sprintf(introText, "%s", text);
        if (!bytfre || strlen(bytfre)>sizeof(bytesFreeText)) return;
        sprintf(bytesFreeText, "%s", bytfre);
    }
    void CYX::SetDarkMenuPalette(){
        *(u32*)(Hooks::offsets.colorKeybBack) = 0xFF100800;
        *(u32*)(Hooks::offsets.colorSearchBack) = 0xFFFFA000;
        *(u32*)(Hooks::offsets.colorFileCreatorBack) = 0xFFC0FF80;
        *(u32*)(Hooks::offsets.colorFileDescBack) = 0xFF6868A0;
        *(u32*)(Hooks::offsets.colorActiveProjLbl) = 0xFF40C000;
        *(u32*)(Hooks::offsets.colorSetSmToolLbl) = 0xFF40C000;
        *(u32*)(Hooks::offsets.colorSetKeyRep) = 0xFF80D8FF;
        *(u32*)(Hooks::offsets.colorSetKeyTL) = 0xFFFFA800;
    }

    std::string CYX::GetProgramSlotFileName(u8 slot){
        if (slot >= 4) return "";
        std::string f;
        UTF16toUTF8(f, editorInstance->programSlot[slot].file_name, editorInstance->programSlot[slot].file_name_len);
        return f;
    }

    std::string CYX::PTCVersionString(u32 ver){
        std::string out;
        out += Utils::Format("%d.",(ver>>24)&255);
        out += Utils::Format("%d.",(ver>>16)&255);
        out += Utils::Format("%d",(ver>>8)&255);
        if (ver&255) out += Utils::Format("-%d",ver&255);
        return out;
    }

    bool CYX::isPTCVersionValid(u32 ver){
        if ((u8)(ver>>24) != 3) return false;
        if ((u8)(ver>>16) > 6) return false;
        if ((u8)(ver>>8) > 3) return false;
        if (ver&255) return false;
        return true;
    }

    std::string CYX::ColorPTCVerValid(u32 ver, u32 ok, u32 ng){
        char c[5]={0}; c[0]=0x1b;
        if (CYX::isPTCVersionValid(ver)){
            if (ok == CYX__COLORVER_NOCOLOR){
                return ResetColor();
            } else {
                return ""<<Color(ok);
            }
        } else {
            if (ng == CYX__COLORVER_NOCOLOR){
                return ResetColor();
            } else {
                return ""<<Color(ng);
            }
        }
        return (c);
    }

    void CYX::CreateHomeFolder(const std::string& s){
        std::string path = HOMEFS_PATH"/P"+s;
        if (!Directory::IsExists(HOMEFS_PATH)) Directory::Create(HOMEFS_PATH);
        if (!Directory::IsExists(path)) Directory::Create(path);
    }
    void CYX::CreateHomeFolder(){
        CreateHomeFolder(g_currentProject);
    }
    std::string CYX::GetHomeFolder(){
        return HOMEFS_PATH"/P"+g_currentProject;
    }
    std::string CYX::GetHomeFolder(std::string project){
        return HOMEFS_PATH"/P"+project;
    }

    void CYX::SetFontGetAddressStrictness(bool on){
        if (on)
            Process::CopyMemory((char*)(Hooks::offsets.funcFontGetOff+0x3c), patch_FontGetOffset, 8);
        else
            Process::CopyMemory((char*)(Hooks::offsets.funcFontGetOff+0x3c), patch_FontGetOffsetNew, 8);
        rtFlushInstructionCache((char*)Hooks::offsets.funcFontGetOff, 0x44);
    }

    void CYX::SoundThreadHook(){
        svcGetThreadId(&std::get<0>(soundThreadsInfo[0]), CUR_THREAD_HANDLE);
        std::get<1>(soundThreadsInfo[0]) = (u32*)getThreadLocalStorage();
        ctrpfHook__ExecuteOriginalFunction();
    }

	void CYX::playMusicAlongCTRPF(bool playMusic) {
		static bool isPlayMusic = false;
		//if (forceDisableSndOnPause)
		//	playMusic = false;
		if (isPlayMusic == playMusic) return;
		isPlayMusic = playMusic;
		static u32 tlsBackup;
		static s32 prioBackup;
        u32 soundThreadID = std::get<0>(soundThreadsInfo[0]);
        u32* soundThreadTls = std::get<1>(soundThreadsInfo[0]);
        Handle soundThreadHandle; bool perf=true;
        if (R_FAILED(svcOpenThread(&soundThreadHandle, CUR_PROCESS_HANDLE, soundThreadID))) perf=false;
        if (soundThreadID == 0xFFFFFFFF) perf=false;
        if (soundThreadHandle == 0) perf=false;
        if (perf){
            if (playMusic) {
                tlsBackup = *soundThreadTls;
                *soundThreadTls = THREADVARS_MAGIC;
                svcGetThreadPriority(&prioBackup, soundThreadHandle);
                svcSetThreadPriority(soundThreadHandle, FwkSettings::Get().ThreadPriority - 1);
            }
            else {
                *soundThreadTls = tlsBackup;
                svcSetThreadPriority(soundThreadHandle, prioBackup);
            }
        }
        svcCloseHandle(soundThreadHandle);
	}
}