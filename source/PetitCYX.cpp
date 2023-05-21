#include "PetitCYX.hpp"
#include "BasicAPI.hpp"

namespace CTRPluginFramework {
    u32 CYX::currentVersion = 0;
    std::tuple<u32, u32*, u32> CYX::soundThreadsInfo[1] = { std::tuple<u32, u32*, u32>(0xFFFFFFFF, nullptr, 0) };
    BASICEditorData* CYX::editorInstance = NULL;
    BASICGRPStructs* CYX::GraphicPage = NULL;
    BASICTextPalette* CYX::textPalette = NULL;
    BASICActiveProject* CYX::activeProject = NULL;
    RT_HOOK CYX::clipboardFunc = {0};
    RT_HOOK CYX::basControllerFunc = {0};
    RT_HOOK CYX::scrShotStub = {0};
    Hook CYX::soundHook;
    char CYX::introText[512] = "SmileBASIC-CYX " STRING_VERSION "\nBuild " STRING_BUILD "\n\n2022-2023 CyberYoshi64\n\n";
    char CYX::bytesFreeText[32] = " bytes free\n\n";
    bool CYX::provideClipAPI = false;
    bool CYX::wasClipAPIused = false;
    u32 CYX::cyxApiOutType = 0;
    string16 CYX::cyxApiTextOut;
    double CYX::cyxApiFloatOut = 0;
    s32 CYX::cyxApiIntOut = 0;

    void CYX::Initialize(void) {
        switch (g_region) {
            case REGION_JPN:
                currentVersion = *(u32*)JPN_VERSION_INT;
                editorInstance = (BASICEditorData*)JPN_EDITORDATA;
                GraphicPage = (BASICGRPStructs*)JPN_GRPSTRUCTS;
                textPalette = (BASICTextPalette*)JPN_CONTXTPAL;
                activeProject = (BASICActiveProject*)JPN_ACTPROJ_STR;
                rtInitHook(&basControllerFunc, JPN_BASICCONTROLLERFUNC, (u32)CYX::controllerFuncHook);
                *(char**)JPN_BOOTTEXT = introText;
                *(char**)(JPN_BOOTTEXT+4) = bytesFreeText;
                break;
            case REGION_USA:
                currentVersion = *(u32*)USA_VERSION_INT;
                editorInstance = (BASICEditorData*)USA_EDITORDATA;
                GraphicPage = (BASICGRPStructs*)USA_GRPSTRUCTS;
                textPalette = (BASICTextPalette*)USA_CONTXTPAL;
                activeProject = (BASICActiveProject*)USA_ACTPROJ_STR;
                rtInitHook(&basControllerFunc, USA_BASICCONTROLLERFUNC, (u32)CYX::controllerFuncHook);
                *(char**)USA_BOOTTEXT = introText;
                *(char**)(USA_BOOTTEXT+4) = bytesFreeText;
                break;
            case REGION_EUR:
                currentVersion = *(u32*)EUR_VERSION_INT;
                editorInstance = (BASICEditorData*)EUR_EDITORDATA;
                GraphicPage = (BASICGRPStructs*)EUR_GRPSTRUCTS;
                textPalette = (BASICTextPalette*)EUR_CONTXTPAL;
                activeProject = (BASICActiveProject*)EUR_ACTPROJ_STR;
                rtInitHook(&basControllerFunc, EUR_BASICCONTROLLERFUNC, (u32)CYX::controllerFuncHook);
                /*soundHook.InitializeForMitm(0x12A318, (u32)SoundThreadHook);
                soundHook.SetFlags(USE_LR_TO_RETURN|MITM_MODE|EXECUTE_OI_AFTER_CB);
                soundHook.Enable();*/
                *(char**)EUR_BOOTTEXT = introText;
                *(char**)(EUR_BOOTTEXT+4) = bytesFreeText;
                break;
        }
        BasicAPI::Initialize();
        if (basControllerFunc.funcAddr) rtEnableHook(&basControllerFunc);
    }

    void CYX::Finalize(){
        BasicAPI::Finalize();
        CYX::SaveSettings();
    }

    void CYX::SetAPIClipboardAvailability(bool enabled){
        provideClipAPI = enabled;
    }
    bool CYX::GetAPIClipboardAvailability(){
        return provideClipAPI;
    }
    void CYX::DiscardAPIUse(){
        wasClipAPIused = false;
    }
    void CYX::SetAPIUse(bool enabled){
        wasClipAPIused = enabled;
    }
    bool CYX::WasClipAPIUsed(){
        return wasClipAPIused;
    }
    int CYX::scrShotStubFunc() {
        return 0;
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
        if ((argc < 1) || (outc != 1)) return 3;
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
        if (isCYX){
            CYX::cyxApiTextOut.clear();
            int res=BasicAPI::Parse(argv, argc);
            switch (res){
                case 0: case 1: break;
                case 2: return 1; // Silent exit
                case 3: return 2; // Non-fatal quit
                default: return 3; // Fatal quit
            }
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
            }
        } else {
            outv->type = SBVARRAW_INTEGER;
            outv->data = 0x10000001;
        }
        return 0;
    }

    /*int CYX::clipboardFuncHook(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv){
        BASICGenericVariable* var; int type;
        bool outToClip = (argc==1); int apiOut = 1;
        std::string newClipData = "";

        newClipData = Utils::Format("OUT %p (%d), IN %p (%d)", outv, outc, argv, argc);
        OSD::Notify(newClipData);
        
        newClipData = "";
        for (u32 i=0; i<argc; i++){
            var = argv++;
            type = getSBVariableType(var->type);
            if (type != VARTYPE_NONE){
                if (i) newClipData += "\x1f";
                switch (type){
                    case VARTYPE_STRING: UTF16toUTF8(newClipData, (u16*)var->data2, var->data); break;
                    case VARTYPE_INT: newClipData += Utils::Format("%d", var->data); break;
                    case VARTYPE_DOUBLE: newClipData += Utils::ToString(*(double*)&var->data,4); break;
                }
                if (strlen_utf8(newClipData) > 1048576) return 3;
            }
        }
        OSD::Notify(newClipData);
        if (provideClipAPI) apiOut = BasicAPI::ParseClipAPI(newClipData);
        if (newClipData.length() > 1048576) newClipData.resize(1048576);
        if (outToClip || apiOut == 0){
            Process::WriteString((u32)&editorInstance->clipboardData, newClipData, StringFormat::Utf16);
            editorInstance->clipboardLength = strlen_utf8(newClipData);
        }
        switch (apiOut){
            case 0: case 1: break;
            case 2: return 1; // Silent exit
            case 3: return 2; // Non-fatal quit
            default: return 3; // Fatal quit
        }
        for (u32 i=0; i<outc; i++){
            var = outv++;
            if (!var->type) break;
            var->type = SBVARRAW_STRING;
            var->data = editorInstance->clipboardLength;
            var->data2 = &editorInstance->clipboardData;
        }
        return 0;
    }*/

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

    void CYX::ReplaceServerName(std::string& saveURL, std::string& loadURL){
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