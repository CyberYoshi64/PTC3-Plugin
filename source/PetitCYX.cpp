#include "PetitCYX.hpp"
#include "main.hpp"

namespace CTRPluginFramework {
    u32 CYX::currentVersion = 0;
    BASICEditorData* CYX::editorInstance = NULL;
    RT_HOOK CYX::clipboardFunc = {0};
    RT_HOOK CYX::scrShotStub = {0};
    bool CYX::provideClipAPI = false;
    bool CYX::wasClipAPIused = false;

    void CYX::Initialize(void) {
        switch (g_region) {
            case REGION_JPN:
                currentVersion = *(u32*)JPN_VERSION_INT;
                editorInstance = (BASICEditorData*)JPN_EDITORDATA;
                break;
            case REGION_USA:
                currentVersion = *(u32*)USA_VERSION_INT;
                editorInstance = (BASICEditorData*)USA_EDITORDATA;
                break;
            case REGION_EUR:
                currentVersion = *(u32*)EUR_VERSION_INT;
                editorInstance = (BASICEditorData*)EUR_EDITORDATA;
                //rtInitHook(&scrShotStub, 0x1A7B58, (u32)CYX::scrShotStubFunc);
                //rtEnableHook(&scrShotStub);
                rtInitHook(&clipboardFunc, EUR_CLIPBOARDFUNC, (u32)CYX::clipboardFuncHook);
                break;
        }
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
    bool CYX::WasClipAPIUsed(){
        return wasClipAPIused;
    }
    int CYX::scrShotStubFunc() {
        return 0;
    }
    int CYX::getSBVariableType(u32 rawType){
        switch(rawType){
            case SBVARRAW_INTEGER: return VARTYPE_INT;
            case SBVARRAW_DOUBLE: return VARTYPE_DOUBLE;
            case SBVARRAW_STRING: return VARTYPE_STRING;
            case SBVARRAW_INTARRAY: return VARTYPE_INTARRAY;
            case SBVARRAW_ARRAY: return VARTYPE_DBLARRAY;
            //case SBVARRAW_STRARRAY: return VARTYPE_STRARRAY;
        }
        return VARTYPE_NONE;
    }
    
    int CYX::clipboardFuncHook(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u8 outc, void* a4, u8 argc, BASICGenericVariable* argv){
        BASICGenericVariable* var; int type;
        bool outToClip = true; int apiOut;
        std::string newClipData;
        for (u32 i=0; i<argc; i++){
            newClipData = ""; var = argv++;
            type = getSBVariableType(var->type);
            if (type != VARTYPE_NONE){
                switch (type){
                    case VARTYPE_STRING: UTF16toUTF8(newClipData, (u16*)var->data2, var->data); break;
                    case VARTYPE_INT: newClipData = Utils::Format("&H%08X", var->data); break;
                    case VARTYPE_DOUBLE: newClipData = Utils::ToString(*(double*)&var->data,4); break;
                }
                apiOut = 1;
                if (strlen_utf8(newClipData) > 1048576) return 3;
                if (provideClipAPI) apiOut = ParseClipAPI(newClipData);
                if (newClipData.length() > 1048576) newClipData.resize(1048576);
                if (outToClip || apiOut == 0){
                    Process::WriteString((u32)&editorInstance->clipboardData, newClipData, StringFormat::Utf16);
                    editorInstance->clipboardLength = strlen_utf8(newClipData);
                    outToClip = false;
                }
                switch (apiOut){
                    case 0: case 1: break;
                    case 2: return 1; // Silent exit
                    case 3: return 2; // Non-fatal quit
                    default: return 3; // Fatal quit
                }
            }
        }
        for (u32 i=0; i<outc; i++){
            var = outv++;
            if (!var->type) break;
            var->type = SBVARRAW_STRING;
            var->data = editorInstance->clipboardLength;
            var->data2 = &editorInstance->clipboardData;
        }
        return 0;
    }

    int CYX::ParseClipAPI(std::string& data){
        if (data.compare(0, 4, "CYX:")!=0) return 1;
        if (!wasClipAPIused) wasClipAPIused = true;
        StringVector arglist;
        u32 index = 4; u32 newidx;
        while (index && index < data.length()){
            newidx = data.find(":", index);
            arglist.push_back(data.substr(index, newidx-index));
            index = ++newidx;
        }
        data = "";
        if (arglist.size()) {
            strupper(arglist[0]);
            if (arglist[0] == "OSD") {
                int type; data = "";
                for (u32 i=1; i<arglist.size(); i++) {
                    if (i == 1 && arglist[i].length()==1){
                        if (arglist[i] == "W"){
                            type = 1; continue;
                        } else if (arglist[i] == "N"){
                            type = 2; continue;
                        } else if (arglist[i] == "E"){
                            type = 3; continue;
                        } else if (arglist[i] == "S"){
                            type = 4; continue;
                        } else if (arglist[i] == "M"){
                            type = 0; continue;
                        }
                    }
                    data += arglist.at(i);
                    if (i+1 != arglist.size()) data += ":";
                }
                switch (type){
                case 1:
                    OSD::Notify(data, Color::White, Color::Olive);
                    break;
                case 2:
                    OSD::Notify(data, Color::Cyan, Color::Navy);
                    break;
                case 3:
                    OSD::Notify(data, Color::White, Color::Maroon);
                    break;
                case 4:
                    OSD::Notify(data, Color::White, Color::Green);
                    break;
                default:
                    OSD::Notify(data, Color::White, Color::Teal);
                    break;
                }
                return 0;
            } else if (arglist[0] == "FILES") {
                if (arglist.size()<2) {
                    data = "Error: Missing arguments for FILES";
                    return 1;
                }
                Directory dir(arglist[1]);
                StringVector vec;
                dir.ListDirectories(vec);
                for (u32 i=0; i<vec.size(); i++) data += ":/"+vec[i];
                vec.clear();
                dir.ListFiles(vec);
                for (u32 i=0; i<vec.size(); i++) data += ":*"+vec[i];
                vec.clear();
                dir.Close();
                data.erase(0, 1);
                return 1;
            } else {
                data = "Error: Unknown API call \""+arglist[0]+"\"";
            }
        }
        
        return 0;
    }

    // Editor strings are merely buffers, so the string has to be crafted
    // with a given length parameter.
    void CYX::UTF16toUTF8(std::string& out, u16* str, u32 len){
        string16 s16 = str; s16.resize(len);
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

    void CYX::ChangeBootText(const char* text){
        char str[BOOT_TEXT_LEN];
        sprintf(str, "%s", text);
        switch (g_region) {
        case REGION_JPN: memcpy((void*)JPN_BOOTTEXT, str, BOOT_TEXT_LEN); break;
        case REGION_USA: memcpy((void*)USA_BOOTTEXT, str, BOOT_TEXT_LEN); break;
        case REGION_EUR: memcpy((void*)EUR_BOOTTEXT, str, BOOT_TEXT_LEN); break;
        }
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
}