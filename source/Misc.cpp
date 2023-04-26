#include "Misc.hpp"
#include "main.hpp"

namespace CTRPluginFramework {

    void* generalPointer;
    u32 generalInt;

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
            PLGSET(PLGFLG_SPOOFED_VER);
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
                    PLGSET(PLGFLG_EXPERIMENTS);
                }
                hook = !hook;
                break;
            case 1:
                CYX::SetAPIClipboardAvailability(api = !api & hook);
                PLGSET(PLGFLG_EXPERIMENTS);
                PLGSET(PLGFLG_CYX_API);
                break;
            default:
                return;
            }
        }
    }
    void grpCorruptor(MenuEntry* entry){
        if (entry->IsActivated()){
            PLGSET(PLGFLG_EXPERIMENTS);
            u32 o = Utils::Random(0, 512*512 - 4096);
            for (u16 i=0; i<4096; i++){
                CYX::GraphicPage->grp[0].dispBuf[o+i] = (u16)Utils::Random();
                CYX::GraphicPage->grp[1].dispBuf[o+i] = (u16)Utils::Random();
                CYX::GraphicPage->grp[2].dispBuf[o+i] = (u16)Utils::Random();
                CYX::GraphicPage->grp[3].dispBuf[o+i] = (u16)Utils::Random();
                CYX::GraphicPage->grp[4].dispBuf[o+i] = (u16)Utils::Random();
                CYX::GraphicPage->grp[5].dispBuf[o+i] = (u16)Utils::Random();
                CYX::GraphicPage->font.dispBuf[o+i] = (u16)Utils::Random();
                CYX::GraphicPage->system.dispBuf[o+i] = (u16)Utils::Random();
            }
            CYX::GraphicPage->grp[0].displayedFormat = Utils::Random(0,11);
            CYX::GraphicPage->grp[1].displayedFormat = Utils::Random(0,11);
            CYX::GraphicPage->grp[2].displayedFormat = Utils::Random(0,11);
            CYX::GraphicPage->grp[3].displayedFormat = Utils::Random(0,11);
            CYX::GraphicPage->grp[4].displayedFormat = Utils::Random(0,11);
            CYX::GraphicPage->grp[5].displayedFormat = Utils::Random(0,11);
            CYX::GraphicPage->font.displayedFormat = Utils::Random(0,11);
            CYX::GraphicPage->system.displayedFormat = Utils::Random(0,11);
        }
    }
    void grpFixMe(MenuEntry* entry){
        if (entry->IsActivated()){
            PLGSET(PLGFLG_EXPERIMENTS);
            u32 dspX = 0xBB000000, dspY = 0x3B000000;
            for (u8 i=0; i<6; i++){
                CYX::GraphicPage->grp[i].displayedFormat = 2;
                CYX::GraphicPage->grp[i].__unk__sizeX = 0x200;
                CYX::GraphicPage->grp[i].__unk__sizeY = 0x200;
                CYX::GraphicPage->grp[i].dispScaleX = *(float*)&dspX;
                CYX::GraphicPage->grp[i].dispScaleY = *(float*)&dspY;
            }
            CYX::GraphicPage->font.displayedFormat = 2;
            CYX::GraphicPage->system.displayedFormat = 2;
        }
        entry->Disable();
    }

    void editorRulerPalCallback(Keyboard& kbd, KeyboardEvent& ev){
        u32 c; CTRPluginFramework::Render::Interface* r;
        Color c0, c1, c2, c3;
        std::string t;
        StringVector s = {
            "This is a sample.",
            "It's sorta accurate.",
            "\uFF3C\uE008\uFF0F",
            "?\"サイバーヨッシー６４\""
        };
        switch (ev.type){
            case ev.SelectionChanged:
                if (ev.selectedIndex > 7) return;
                generalInt = ev.selectedIndex;
                break;
            case ev.FrameTop:
                kbd.GetMessage() = "Select the color scheme to use.";
                c = CYX::textPalette->editSelectFG;
                c0 = Color(SWITCHEND32(c));
                c = ((u32**)generalPointer)[generalInt][0];
                c1 = Color(SWITCHEND32(c));
                c = ((u32**)generalPointer)[generalInt][1];
                c2 = Color(SWITCHEND32(c));
                c = CYX::textPalette->editText;
                c3 = Color(SWITCHEND32(c));
                r = ev.renderInterface;
                r->DrawRect(IntRect(40,50,320,160), c0);
                r->DrawRect(IntRect(42,188,316,15), c1);
                r->DrawRect(IntRect(110,188,12,15), c2);
                r->DrawRect(IntRect(42,60,64,15), c1);
                r->DrawRect(IntRect(42,76,64,15), c2);
                r->DrawRect(IntRect(42,92,64,15), c1);
                r->DrawRect(IntRect(42,108,64,15), c1);
                r->DrawRect(IntRect(42,124,64,15), c1);
                r->DrawRect(IntRect(42,140,64,15), c1);
                r->DrawRect(IntRect(42,156,64,15), c1);
                r->DrawRect(IntRect(42,172,64,15), c1);
                for (u32 si0=0; si0 < s.size(); si0++){
                    t = Utils::Format("%d",si0+1);
                    c = Render::GetTextWidth(t);
                    r->DrawSysString(t, 100-c, 59+si0*16, Color::Black);
                    u32 si2=0;
                    for (u32 si1=0; si1 < s[si0].size(); si1++){
                        t = s[si0].at(si1);
                        if (t[0]>=0x80) {
                            t = s[si0].substr(si1, 3);
                            si1+=2;
                        }
                        c = Render::GetTextWidth(t)/2;
                        r->DrawSysString(t, 115+si2*12-c, 59+si0*16, c3, 400);
                        si2++;
                    }
                }
                break;
        }
    }

    void editorRulerPalette(MenuEntry* entry){
        u32 pal0[] = {0xFFC0C0C0, 0xFF20DCE0}; // Original
        u32 pal1[] = {0xFF404040, 0xFFE03010}; // Inverted
        u32 pal2[] = {0xFF808000, 0xFFFFFF40}; // Aqua
        u32 pal3[] = {0xFF003860, 0xFF00FF80}; // Jungle
        u32 pal4[] = {0xFF484040, 0xFF80FF00}; // Dark 1
        u32 pal5[] = {0xFF000000, 0xFFFF7000}; // Dark 2
        u32 pal6[] = {0xFFFFFFFF, 0xFFFF5000}; // Light
        u32* ptr[] = {pal0,pal1,pal2,pal3,pal4,pal5,pal6};
        generalPointer = ptr; generalInt = 0;
        
        Keyboard kbd;
        kbd.OnKeyboardEvent(editorRulerPalCallback);
        kbd.Populate({
            "Original",
            "Inverted",
            "Aqua",
            "Jungle",
            "Dark 1",
            "Dark 2",
            "Light",
        }, true);
        kbd.DisplayTopScreen = true;
        int kres = kbd.Open();
        if (kres < 0) return;
        CYX::textPalette->editRuler = ptr[kres][0];
        CYX::textPalette->editRulerSel = ptr[kres][1];
    }
}