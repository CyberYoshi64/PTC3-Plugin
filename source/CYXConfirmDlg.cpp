#include "CYXConfirmDlg.hpp"

namespace CTRPluginFramework {
    u32 CYXConfirmDlg::coolDown = 0;
    u32 CYXConfirmDlg::useCount = 0;
    u32 CYXConfirmDlg::useTimer = 1;

    void CYXConfirmDlg::DoTheThing(){
        if (useTimer) {
            if (!--useTimer)
                CYX::apiEnableFlag(APIFLAG_ALLOW_TOGGLE);
        }
        if (coolDown){
            coolDown--;
            return;
        } else if (___confirmWaiting){
            switch (___confirmID){
            case CYXCONFIRM_BASICAPI_XREF_RW:
                ___confirmRes = BasicAPI_XREF_RW();
                break;
            case CYXCONFIRM_BASICAPI_SD_RW:
                ___confirmRes = BasicAPI_SD_RW();
                break;
            case CYXCONFIRM_BASICAPI_TOHOME:
                ___confirmRes = BasicAPI_ToHOME();
                break;
            default:
                break;
            }
            ++useCount *= (!!useTimer);
            if (useCount >= 5) {
                CYX::apiDisableFlag(APIFLAG_ALLOW_TOGGLE);
                OSD::Notify("Too many requests! Please wait for 30 sec to request again.", Color::White, Color::Maroon);
            }
            useTimer = 1800;
            coolDown = 60;
            ___confirmWaiting = false;
        }
    }

    void CYXConfirmDlg::ResetUse() {
        useTimer = 1;
        useCount = 0;
        CYX::apiEnableFlag(APIFLAG_ALLOW_TOGGLE);
    }

    int CYXConfirmDlg::BasicAPI_XREF_RW(){
        return MessageBox(
            "A program wants to enable write access to other projects.\n\nOnly grant this permission if you trust the programs running in this project.\nContinue?\n(If you decline, this project can still read data from other projects.)",
            DialogType::DialogYesNo
        )();
    }
    int CYXConfirmDlg::BasicAPI_SD_RW(){
        return MessageBox(
            "A program wants to enable write access to the SD Card.\n" << Color::Orange <<
            "This will supersede permissions for the safe directory and access to other projects.\n" << Color::Red <<
            "This also allows modifying critical files for CFW and homebrew applications." + ResetColor() +
            "\n\nOnly grant this permission if you trust the programs running in this project.\nContinue?\n(If you decline, this project can still read data from the SD Card.)",
            DialogType::DialogYesNo
        )();
    }
    int CYXConfirmDlg::BasicAPI_ToHOME(){
        return MessageBox(
            "A program wants to close SmileBASIC.\nDo you want to continue?\n\n(You may lose unsaved changes.)",
            DialogType::DialogYesNo
        )();
    }

}