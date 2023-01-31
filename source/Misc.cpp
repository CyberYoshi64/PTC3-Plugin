#include "Misc.hpp"
#include "main.hpp"

namespace CTRPluginFramework {

    void serverAdrChg(MenuEntry *entry){
        std::string srvName;
        Keyboard kbd("");
        kbd.Populate({"Reset to original", "Specify custom server", "\uE072 Back"});
        kbd.ChangeEntrySound(2, SoundEngine::Event::CANCEL);
        int kbdres = kbd.Open();
        if (kbdres < 0 || kbdres >= 2) return;
        if (kbdres == 0) {
            srvName = _DEFAULT_SERVERNAME_SAVE;
            std::string srvLoad = _DEFAULT_SERVERNAME_LOAD;
            CYX::ReplaceServerName(srvName, srvLoad);
            return;
        } else {
            Keyboard kbd("Enter the server name to which to connect to:\n\n The domain name must be " TOSTRING(_LEN_SERVERNAME_URL) " characters\n long or less.");
            kbd.SetMaxLength(_LEN_SERVERNAME_URL);
            kbdres = kbd.Open(srvName, "http://");
            if (kbdres < 0) return;
            if (srvName.compare(0, 4, "http")!=0) {
                MessageBox("The server name is not valid:\nThe name must start with \"http://\" or \"https://\".", DialogType::DialogOk, ClearScreen::Both)();
                return;
            }
            while (strlen(srvName.c_str())>_LEN_SERVERNAME_URL) srvName.resize(srvName.size()-1);
            MessageBox e("Server name changer", srvName+"\n\nIs this correct?", DialogType::DialogYesNo, ClearScreen::Both);
            if (e()){
                CYX::ReplaceServerName(srvName, srvName);
            } else {
                MessageBox("Cancelled operation", "No changes were made.");
            }
        }
    }

    void versionSpoof(MenuEntry *entry){
        u32 offsets[] = {0x0, JPN_VERSION_INT, USA_VERSION_INT, EUR_VERSION_INT};
        menu:
        u32 oldVersion = *(u32*)offsets[g_region], newVersion = 0;
        Keyboard mainsel(
            "Version spoofer\n\nCurrently set:\n" << SkipToPixel(48) <<
            CYX::ColorPTCVerValid(oldVersion, CYX__COLORVER_NOCOLOR, 0xFF0000FF) <<
            CYX::PTCVersionString(oldVersion) << SkipToPixel(200) <<
            Color(0x7F7F7FFF) <<
            Utils::Format("(0x%08X)\n", oldVersion) << ResetColor() <<
            "Original:\n" << SkipToPixel(48) <<
            CYX::PTCVersionString(CYX::currentVersion) << SkipToPixel(200) <<
            Color(0x7F7F7FFF) <<
            Utils::Format("(0x%08X)\n", CYX::currentVersion) <<
            ResetColor()
        );
        mainsel.Populate({"Reset to original", "Set value", "\uE072 Back"});
        mainsel.ChangeEntrySound(2, SoundEngine::Event::CANCEL);
        Keyboard kbd("");
        int kbdres = mainsel.Open();
        switch (kbdres) {
        case 0:
            *(u32*)offsets[g_region] = CYX::currentVersion;
            break;
        case 1:
            kbd.IsHexadecimal(true);
            if (kbd.Open(newVersion, oldVersion)==0) *(u32*)offsets[g_region] = newVersion;
            break;
        case -1:
        case -2:
        case 2:
            return;
        }
        goto menu;
    }
    void pluginDetails(MenuEntry *entry){
        MessageBox("Details",
            (
                Utils::Format("Base application: %s ",g_regionString)+
                CYX::PTCVersionString(CYX::currentVersion)+
                Utils::Format("\nCYX %s",STRING_VERSION)+"\n"+
                Utils::Format("\nEditor data @ 0x%08X",(u32)CYX::editorInstance)+
                Utils::Format("\nClipboard func @ 0x%08X",CYX::clipboardFunc.funcAddr)+
                Utils::Format("\nScrShot func @ 0x%08X",CYX::scrShotStub.funcAddr)
            ),
            DialogType::DialogOk, ClearScreen::Both)();
    }
    void clipboardHooking(MenuEntry* entry){
        if (!CYX::clipboardFunc.funcAddr) {
            MessageBox("The clipboard function is not hookable at this time. Check for an updated plugin.")();
            return;
        }
        bool hook = CYX::clipboardFunc.isEnabled;
        bool api = CYX::GetAPIClipboardAvailability();
        Keyboard kbd(""); int kres;
        std::string opt1, opt2;
        while (true){
            kbd.DisplayTopScreen = true;
            kbd.GetMessage() =
            "Disclaimer:\n"
            "This feature is experimental and may cause\n"
            "SmileBASIC to be unstable and/or enable BASIC\n"
            "programs to use out-of-scope features, such as\n"
            "accessing the SD Card or check the console type.";
            if (CYX::WasClipAPIUsed()){
                kbd.GetMessage() +=
                    Color(0xFF8800FF) << "\n\nThe CYX API has been utilized.\n" <<
                    "Disabling the hook and/or the API may cause\n" <<
                    "unexpected behaviour.";
            }
            opt1 =
                "Hook clipboard\u3000" + (hook ?
                Color::Lime << "⊂●":
                Color::Red << "●⊃");
            if (hook){
                opt2 = "Enable CYX API\u3000" + (api ?
                Color::Lime << "⊂●":
                Color::Red << "●⊃");
            } else {
                opt2 = Color::Gray << "Enable CYX API\u3000●⊃";
            }
            kbd.Populate(StringVector{opt1, opt2, "\uE072 Back" }, 0);
            kbd.ChangeEntrySound(0, SoundEngine::Event::DESELECT);
            kbd.ChangeEntrySound(1, SoundEngine::Event::DESELECT);
            kbd.ChangeEntrySound(2, SoundEngine::Event::CANCEL);
            kres = kbd.Open();
            switch (kres){
            case 0:
                if (hook){
                    rtDisableHook(&CYX::clipboardFunc);
                    CYX::SetAPIClipboardAvailability(api = false);
                    CYX::DiscardAPIUse();
                } else {
                    rtEnableHook(&CYX::clipboardFunc);
                }
                hook = !hook;
                break;
            case 1:
                CYX::SetAPIClipboardAvailability(api = !api & hook);
                break;
            default:
                return;
            }
        }
    }
}