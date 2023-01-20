#include "Misc.hpp"
#include "main.hpp"

namespace CTRPluginFramework {

    void serverAdrChg(MenuEntry *entry){
        std::string srvName;
        Keyboard kbd("");
        kbd.Populate({"Reset to original", "Specify custom server", "\uE072 Back"});
        kbd.ChangeEntrySound(3, SoundEngine::Event::CANCEL);
        int kbdres = kbd.Open();
        if (kbdres < 0 || kbdres >= 3) return;
        if (kbdres == 0) {
            srvName = _DEFAULT_SERVERNAME_SAVE;
            std::string srvLoad = _DEFAULT_SERVERNAME_LOAD;
            CYX::ReplaceServerName(srvName, srvLoad);
            MessageBox("Server name was reset to the original.", DialogType::DialogOk, ClearScreen::Both)();
            return;
        } else {
            Keyboard kbd("Enter the server name to which to connect to:\n\nThe domain name must be " TOSTRING(_LEN_SERVERNAME_URL) "\ncharacers long or less.");
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
            "Version spoofer\n\nCurrently set:\n"+SkipToPixel(48)+
            CYX::ColorPTCVerValid(oldVersion, 1, 0xFF0000)+
            CYX::PTCVersionString(oldVersion)+SkipToPixel(200)+
            u8"\x1b\x7f\x7f\x7f"+Utils::Format("(0x%08X)\n", oldVersion)+"\x18"+
            "Original:\n"+SkipToPixel(48)+
            CYX::PTCVersionString(CYX::currentVersion)+SkipToPixel(200)+
            u8"\x1b\x7f\x7f\x7f"+Utils::Format("(0x%08X)\n", CYX::currentVersion)+"\x18"
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
                Utils::Format("\nCYX %s",STRING_VERSION)+
                "\n\nCreated by CyberYoshi64 from Germany in 2022-2023"
            ),
            DialogType::DialogOk, ClearScreen::Both)();
    }
}