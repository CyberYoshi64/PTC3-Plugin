#include "PetitCYX.hpp"
#include "main.hpp"

/*
    TODO: Come up with a better name...
    CYX (CY64 Extension) seems too cocky
*/

namespace CTRPluginFramework {
    void CYX::Initialize(void) {
        switch (g_region) {
            case JPN: currentVersion = *(u32*)JPN_VERSION_INT; break;
            case USA: currentVersion = *(u32*)USA_VERSION_INT; break;
            case EUR: currentVersion = *(u32*)EUR_VERSION_INT; break;
        }
    }

    void CYX::ReplaceServerName(std::string& saveURL, std::string& loadURL){
        /*
            TODO: Make this function more ellegant.
            Works for now (TM)
        */
        u32 addresses[] = {
            JPN_SERVERNAME_LOAD2_1,JPN_SERVERNAME_LOAD2_2,JPN_SERVERNAME_SAVE3_1,JPN_SERVERNAME_SAVE3_2,JPN_SERVERNAME_SHOW2,JPN_SERVERNAME_LIST2,JPN_SERVERNAME_INFO2,JPN_SERVERNAME_DELETE2,JPN_SERVERNAME_SHOPLIST2,JPN_SERVERNAME_PREPURCHASE2,JPN_SERVERNAME_PURCHASE2,
            USA_SERVERNAME_LOAD2_1,USA_SERVERNAME_LOAD2_2,USA_SERVERNAME_SAVE3_1,USA_SERVERNAME_SAVE3_2,USA_SERVERNAME_SHOW2,USA_SERVERNAME_LIST2,USA_SERVERNAME_INFO2,USA_SERVERNAME_DELETE2,USA_SERVERNAME_SHOPLIST2,USA_SERVERNAME_PREPURCHASE2,USA_SERVERNAME_PURCHASE2,
            EUR_SERVERNAME_LOAD2_1,EUR_SERVERNAME_LOAD2_2,EUR_SERVERNAME_SAVE3_1, EUR_SERVERNAME_SAVE3_2, EUR_SERVERNAME_SHOW2, EUR_SERVERNAME_LIST2, EUR_SERVERNAME_INFO2, EUR_SERVERNAME_DELETE2, EUR_SERVERNAME_SHOPLIST2, EUR_SERVERNAME_PREPURCHASE2, EUR_SERVERNAME_PURCHASE2
        };
        char buf[64]; int off = (g_region-1)*11;
        if (off < 0 || off > 22) return;
        sprintf(buf, "%s" _NAME_SERVERNAME_LOAD_LOAD2, loadURL.c_str());
        memcpy((void*)addresses[off+0], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_LOAD_LOAD2, loadURL.c_str());
        memcpy((void*)addresses[off+1], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_SAVE3, saveURL.c_str());
        memcpy((void*)addresses[off+2], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_SAVE3, saveURL.c_str());
        memcpy((void*)addresses[off+3], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_SHOW2, saveURL.c_str());
        memcpy((void*)addresses[off+4], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_LIST2, saveURL.c_str() );
        memcpy((void*)addresses[off+5], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_LOAD_INFO2, loadURL.c_str());
        memcpy((void*)addresses[off+6], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_DELETE2, saveURL.c_str());
        memcpy((void*)addresses[off+7], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_SHOPLIST2, saveURL.c_str());
        memcpy((void*)addresses[off+8], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_PREPURCHASE2, saveURL.c_str());
        memcpy((void*)addresses[off+9], buf, strlen(buf)+1);
        sprintf(buf, "%s" _NAME_SERVERNAME_SAVE_PURCHASE2, saveURL.c_str());
        memcpy((void*)addresses[off+10], buf, strlen(buf)+1);
    }

    void CYX::ChangeBootText(const char* text){
        char str[BOOT_TEXT_LEN];
        sprintf(str, "%s", text);
        switch (g_region) {
        case JPN:
            memcpy((void*)JPN_BOOTTEXT, str, BOOT_TEXT_LEN); break;
        case USA:
            memcpy((void*)USA_BOOTTEXT, str, BOOT_TEXT_LEN); break;
        case EUR:
            memcpy((void*)EUR_BOOTTEXT, str, BOOT_TEXT_LEN); break;
        }
    }
    void CYX::SetDarkMenuPalette(){
        u32 eurPtr[]={EUR_COLOR_KEYBBG,EUR_COLOR_SEARCHBG,EUR_COLOR_FCREATORBG,EUR_COLOR_FDESCBG,EUR_COLOR_ACTPRJLBL,EUR_COLOR_SETSMBUTF,EUR_COLOR_SETKEYREP,EUR_COLOR_SETKEYTL};
        u32 usaPtr[]={USA_COLOR_KEYBBG,USA_COLOR_SEARCHBG,USA_COLOR_FCREATORBG,USA_COLOR_FDESCBG,USA_COLOR_ACTPRJLBL,USA_COLOR_SETSMBUTF,USA_COLOR_SETKEYREP,USA_COLOR_SETKEYTL};
        u32 jpnPtr[]={JPN_COLOR_KEYBBG,JPN_COLOR_SEARCHBG,JPN_COLOR_FCREATORBG,JPN_COLOR_FDESCBG,JPN_COLOR_ACTPRJLBL,JPN_COLOR_SETSMBUTF,JPN_COLOR_SETKEYREP,JPN_COLOR_SETKEYTL};
        u32* ptr;
        switch (g_region) {
            case JPN: ptr = jpnPtr; break;
            case USA: ptr = usaPtr; break;
            case EUR: ptr = eurPtr; break;
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
            if (ok==1){
                return ResetColor();
            } else {
                c[1]=MAX(1, (u8)(ok>>16));
                c[2]=MAX(1, (u8)(ok>>8));
                c[3]=MAX(1, (u8)ok);
            }
        } else {
            if (ng==1){
                return ResetColor();
            } else {
                c[1]=MAX(1, (u8)(ng>>16));
                c[2]=MAX(1, (u8)(ng>>8));
                c[3]=MAX(1, (u8)ng);
            }
        }
        return (c);
    }

    u32 CYX::currentVersion = 0;
}