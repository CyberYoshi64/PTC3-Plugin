#include "PetitCYX.hpp"
#include "main.hpp"

namespace CTRPluginFramework {
    void CYX::Initialize(void) {
        switch (g_region) {
            case REGION_JPN: currentVersion = *(u32*)JPN_VERSION_INT; break;
            case REGION_USA: currentVersion = *(u32*)USA_VERSION_INT; break;
            case REGION_EUR: currentVersion = *(u32*)EUR_VERSION_INT; break;
        }
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

    u32 CYX::currentVersion = 0;
    BASICEditorData* editorInstance = NULL;
}