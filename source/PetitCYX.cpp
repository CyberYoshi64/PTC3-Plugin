#include "PetitCYX.hpp"
#include "BasicAPI.hpp"

namespace CTRPluginFramework {
    u32 CYX::currentVersion = 0;
    CYX::Offsets CYX::offsets = {0};
    CYX::MirroredVars CYX::mirror = {0};
    std::tuple<u32, u32*, u32> CYX::soundThreadsInfo[1] = { std::tuple<u32, u32*, u32>(0xFFFFFFFF, nullptr, 0) };
    BASICEditorData* CYX::editorInstance = NULL;
    BASICGRPStructs* CYX::GraphicPage = NULL;
    BASICTextPalette* CYX::textPalette = NULL;
    BASICActiveProject* CYX::activeProject = NULL;
    std::string CYX::g_currentProject = "";
    RT_HOOK CYX::clipboardFunc = {0};
    RT_HOOK CYX::basControllerFunc = {0};
    RT_HOOK CYX::scrShotStub = {0};
    Hook CYX::soundHook;
    char CYX::introText[512] = "SmileBASIC-CYX " STRING_VERSION "\nBuild " STRING_BUILD "\n\n2022-2023 CyberYoshi64\n\n";
    char CYX::bytesFreeText[32] = " bytes free\n\n";
    bool CYX::provideCYXAPI = false;
    bool CYX::wasClipAPIused = false;
    u32 CYX::cyxApiOutType = 0;
    string16 CYX::cyxApiTextOut;
    double CYX::cyxApiFloatOut = 0;
    s32 CYX::cyxApiIntOut = 0;

    Result CYX::Initialize(void) {
        switch (g_region) {
            case REGION_JPN:
                offsets.activeProjectStrings    = JPN_ACTPROJ_STR;
                offsets.basicInterpreterFlag    = JPN_BASIC_INT_FLAG;
                offsets.basicIsDirectMode       = JPN_BASIC_DIRECT_MODE;
                offsets.basicIsRunning          = JPN_BASIC_CMD_FLAG;
                offsets.bootText                = JPN_BOOTTEXT;
                offsets.clipboardFuncAddr       = JPN_CLIPBOARDFUNC;
                offsets.consoleTextPalette      = JPN_CONTXTPAL;
                offsets.controllerFuncAddr      = JPN_CONTROLLERFUNC;
                offsets.editorData              = JPN_EDITORDATA;
                offsets.grpStructs              = JPN_GRPSTRUCTS;
                offsets.helpPageDefaultColors   = JPN_HELPPAGE_DEF;
                offsets.helpPagePalette         = JPN_HELPPAGE_PAL;
                offsets.versionInt              = JPN_VERSION_INT;
                break;
            case REGION_USA:
                offsets.activeProjectStrings    = USA_ACTPROJ_STR;
                offsets.basicInterpreterFlag    = USA_BASIC_INT_FLAG;
                offsets.basicIsDirectMode       = USA_BASIC_DIRECT_MODE;
                offsets.basicIsRunning          = USA_BASIC_CMD_FLAG;
                offsets.bootText                = USA_BOOTTEXT;
                offsets.clipboardFuncAddr       = USA_CLIPBOARDFUNC;
                offsets.consoleTextPalette      = USA_CONTXTPAL;
                offsets.controllerFuncAddr      = USA_CONTROLLERFUNC;
                offsets.editorData              = USA_EDITORDATA;
                offsets.grpStructs              = USA_GRPSTRUCTS;
                offsets.helpPageDefaultColors   = USA_HELPPAGE_DEF;
                offsets.helpPagePalette         = USA_HELPPAGE_PAL;
                offsets.versionInt              = USA_VERSION_INT;
                break;
            case REGION_EUR:
                offsets.activeProjectStrings    = EUR_ACTPROJ_STR;
                offsets.basicInterpreterFlag    = EUR_BASIC_INT_FLAG;
                offsets.basicIsDirectMode       = EUR_BASIC_DIRECT_MODE;
                offsets.basicIsRunning          = EUR_BASIC_CMD_FLAG;
                offsets.bootText                = EUR_BOOTTEXT;
                offsets.clipboardFuncAddr       = EUR_CLIPBOARDFUNC;
                offsets.consoleTextPalette      = EUR_CONTXTPAL;
                offsets.controllerFuncAddr      = EUR_CONTROLLERFUNC;
                offsets.editorData              = EUR_EDITORDATA;
                offsets.grpStructs              = EUR_GRPSTRUCTS;
                offsets.helpPageDefaultColors   = EUR_HELPPAGE_DEF;
                offsets.helpPagePalette         = EUR_HELPPAGE_PAL;
                offsets.versionInt              = EUR_VERSION_INT;
                /*soundHook.InitializeForMitm(0x12A318, (u32)SoundThreadHook);
                soundHook.SetFlags(USE_LR_TO_RETURN|MITM_MODE|EXECUTE_OI_AFTER_CB);
                soundHook.Enable();*/
                break;
        }
        if(!offsets.versionInt) return BIT(31)|1;
        if(!offsets.bootText) return BIT(31)|2;
        if(!offsets.controllerFuncAddr) return BIT(31)|3;
        if(!offsets.clipboardFuncAddr) return BIT(31)|4;
        if(!offsets.editorData) return BIT(31)|5;
        if(!offsets.grpStructs) return BIT(31)|6;
        if(!offsets.activeProjectStrings) return BIT(31)|7;
        if(!offsets.basicInterpreterFlag) return BIT(31)|8;
        if(!offsets.basicIsDirectMode) return BIT(31)|9;
        if(!offsets.basicIsRunning) return BIT(31)|10;
        if(!offsets.consoleTextPalette) return BIT(31)|11;
        if(!offsets.helpPageDefaultColors) return BIT(31)|12;
        if(!offsets.helpPagePalette) return BIT(31)|13;

        currentVersion = *(u32*)offsets.versionInt;
        editorInstance = (BASICEditorData*)offsets.editorData;
        GraphicPage = (BASICGRPStructs*)offsets.grpStructs;
        textPalette = (BASICTextPalette*)offsets.consoleTextPalette;
        activeProject = (BASICActiveProject*)offsets.activeProjectStrings;
        rtInitHook(&basControllerFunc, offsets.controllerFuncAddr, (u32)CYX::controllerFuncHook);
        rtEnableHook(&basControllerFunc);
        *(char**)offsets.bootText = introText;
        *(char**)(offsets.bootText+4) = bytesFreeText;
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
            p.apiUsed = provideCYXAPI;
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
                provideCYXAPI = false;
                BasicAPI::flags = APIFLAG_DEFAULT;
            } else {
                provideCYXAPI = p.apiUsed;
                BasicAPI::flags = p.apiFlags;
            }
        }
        if (BasicAPI::flags & APIFLAG_FS_ACC_SAFE) CreateHomeFolder();
    }
    void CYX::MenuTick(){
        UpdateMirror();
        if (mirror.diff.currentProject || !g_currentProject.size()){
            SaveProjectSettings();
            UTF16toUTF8(g_currentProject="", activeProject->currentProject);
            if (g_currentProject==""||g_currentProject=="###") g_currentProject="$DEFAULT";
            OSD::Notify(g_currentProject);
            LoadProjectSettings();
        }
        if (mirror.diff.isDirectMode){
            OSD::Notify(Utils::Format("isDirectMode: %d", mirror.isDirectMode));
        }
        if (mirror.isInBasic && mirror.isBasicRunningTime == 3 && mirror.isBasicRunning2 != mirror.isBasicRunning){
            mirror.isBasicRunning2 = mirror.isBasicRunning;
            OSD::Notify(Utils::Format("isBasicRunning: %d", mirror.isBasicRunning));
        }
        if (mirror.diff.isInBasic){
            OSD::Notify(Utils::Format("isInBasic: %d", mirror.isInBasic));
        }
    }
    void CYX::UpdateMirror(){
        mirror.diff.currentProject = memcmp(activeProject->currentProject, mirror.currentProject, sizeof(mirror.currentProject))!=0;
        memcpy(mirror.currentProject, activeProject->currentProject, sizeof(mirror.currentProject));
        mirror.diff.isDirectMode = mirror.isDirectMode != *(u8*)offsets.basicIsDirectMode;
        mirror.isDirectMode = *(u8*)offsets.basicIsDirectMode;
        mirror.diff.isInBasic = mirror.isInBasic != *(u8*)offsets.basicInterpreterFlag;
        mirror.isInBasic = *(u8*)offsets.basicInterpreterFlag;
        mirror.diff.isBasicRunning = mirror.isBasicRunning != *(u8*)offsets.basicIsRunning;
        mirror.isBasicRunning = *(u8*)offsets.basicIsRunning;
        ++mirror.isBasicRunningTime *= !(mirror.diff.isBasicRunning);
    }

    void CYX::SetAPIAvailability(bool enabled){
        provideCYXAPI = enabled;
    }
    bool CYX::GetAPIAvailability(){
        return provideCYXAPI;
    }
    void CYX::DiscardAPIUse(){
        wasClipAPIused = false;
    }
    void CYX::SetAPIUse(bool enabled){
        wasClipAPIused = enabled;
    }
    bool CYX::WasCYXAPIUsed(){
        return wasClipAPIused;
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
                outv->data = 0x10000001;
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
        u32 jpnPtr[]={JPN_SERVERNAME_LOAD2_1, JPN_SERVERNAME_LOAD2_2, JPN_SERVERNAME_SAVE3_1, JPN_SERVERNAME_SAVE3_2, JPN_SERVERNAME_SHOW2, JPN_SERVERNAME_LIST2, JPN_SERVERNAME_INFO2, JPN_SERVERNAME_DELETE2, JPN_SERVERNAME_SHOPLIST2, JPN_SERVERNAME_PREPURCHASE2, JPN_SERVERNAME_PURCHASE2};
        u32 usaPtr[]={USA_SERVERNAME_LOAD2_1, USA_SERVERNAME_LOAD2_2, USA_SERVERNAME_SAVE3_1, USA_SERVERNAME_SAVE3_2, USA_SERVERNAME_SHOW2, USA_SERVERNAME_LIST2, USA_SERVERNAME_INFO2, USA_SERVERNAME_DELETE2, USA_SERVERNAME_SHOPLIST2, USA_SERVERNAME_PREPURCHASE2, USA_SERVERNAME_PURCHASE2};
        u32 eurPtr[]={EUR_SERVERNAME_LOAD2_1, EUR_SERVERNAME_LOAD2_2, EUR_SERVERNAME_SAVE3_1, EUR_SERVERNAME_SAVE3_2, EUR_SERVERNAME_SHOW2, EUR_SERVERNAME_LIST2, EUR_SERVERNAME_INFO2, EUR_SERVERNAME_DELETE2, EUR_SERVERNAME_SHOPLIST2, EUR_SERVERNAME_PREPURCHASE2, EUR_SERVERNAME_PURCHASE2};
        u32* ptr;
        switch (g_region) {
            case REGION_JPN: ptr = jpnPtr; break;
            case REGION_USA: ptr = usaPtr; break;
            case REGION_EUR: ptr = eurPtr; break;
            default: return;
        }
        char buf[50];
        sprintf(buf, "%s" _NAME_SERVERNAME_LOAD_LOAD2, loadURL.c_str());
        memcpy((void*)ptr[0], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_LOAD_LOAD2, loadURL.c_str());
        memcpy((void*)ptr[1], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_SAVE3, saveURL.c_str());
        memcpy((void*)ptr[2], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_SAVE3, saveURL.c_str());
        memcpy((void*)ptr[3], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_SHOW2, saveURL.c_str());
        memcpy((void*)ptr[4], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_LIST2, saveURL.c_str() );
        memcpy((void*)ptr[5], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_LOAD_INFO2, loadURL.c_str());
        memcpy((void*)ptr[6], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_DELETE2, saveURL.c_str());
        memcpy((void*)ptr[7], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_SHOPLIST2, saveURL.c_str());
        memcpy((void*)ptr[8], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_PREPURCHASE2, saveURL.c_str());
        memcpy((void*)ptr[9], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_PURCHASE2, saveURL.c_str());
        memcpy((void*)ptr[10], buf, strlen(buf)+1);
    }

    void CYX::ChangeBootText(const char* text, const char* bytfre){
        if (!text || strlen(text)>1530) return;
        sprintf(introText, "%s", text);
        if (!bytfre || strlen(bytfre)>1530) return;
        sprintf(bytesFreeText, "%s", bytfre);
    }
    void CYX::SetDarkMenuPalette(){
        u32 eurPtr[]={EUR_COLOR_KEYBBG,EUR_COLOR_SEARCHBG,EUR_COLOR_FCREATORBG,EUR_COLOR_FDESCBG,EUR_COLOR_ACTPRJLBL,EUR_COLOR_SETSMBUTF,EUR_COLOR_SETKEYREP,EUR_COLOR_SETKEYTL};
        u32 usaPtr[]={USA_COLOR_KEYBBG,USA_COLOR_SEARCHBG,USA_COLOR_FCREATORBG,USA_COLOR_FDESCBG,USA_COLOR_ACTPRJLBL,USA_COLOR_SETSMBUTF,USA_COLOR_SETKEYREP,USA_COLOR_SETKEYTL};
        u32 jpnPtr[]={JPN_COLOR_KEYBBG,JPN_COLOR_SEARCHBG,JPN_COLOR_FCREATORBG,JPN_COLOR_FDESCBG,JPN_COLOR_ACTPRJLBL,JPN_COLOR_SETSMBUTF,JPN_COLOR_SETKEYREP,JPN_COLOR_SETKEYTL};
        u32* ptr;
        switch (g_region) {
            case REGION_JPN: ptr = jpnPtr; break;
            case REGION_USA: ptr = usaPtr; break;
            case REGION_EUR: ptr = eurPtr; break;
            default: return;
        }
        *(u32*)(ptr[0]) = 0xFF100800;
        *(u32*)(ptr[1]) = 0xFFFFA000;
        *(u32*)(ptr[2]) = 0xFFC0FF80;
        *(u32*)(ptr[3]) = 0xFF6868A0;
        *(u32*)(ptr[4]) = 0xFF40C000;
        *(u32*)(ptr[5]) = 0xFF40C000;
        *(u32*)(ptr[6]) = 0xFF80D8FF;
        *(u32*)(ptr[7]) = 0xFFFFA800;
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