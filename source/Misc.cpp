#include "Misc.hpp"
#include "BasicAPI.hpp"

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
        PLGSET(PLGFLG_EXPERIMENTS);
        PLGSET(PLGFLG_SBSERVER);
        if (kbdres == 0) {
            CYX::ReplaceServerName(SBSERVER_DEFAULT_NAME1, SBSERVER_DEFAULT_NAME2);
            return;
        } else {
            Keyboard kbd("Enter the server name to which to connect to:\n\n The domain name must be " TOSTRING(_LEN_SERVERNAME_URL) " characters\n long or less.");
            kbd.SetMaxLength(SBSERVER_URL_MAXLEN);
            kbdres = kbd.Open(srvName, "http://");
            if (kbdres < 0) return;
            if (srvName.compare(0, 4, "http")!=0) {
                MessageBox("The server name is not valid:\nThe name must start with \"http://\" or \"https://\".", DialogType::DialogOk, ClearScreen::Both)();
                return;
            }
            while (strlen(srvName.c_str())>SBSERVER_URL_MAXLEN) srvName.resize(srvName.size()-1);
            MessageBox e("Server name changer", srvName+"\n\nIs this correct?", DialogType::DialogYesNo, ClearScreen::Both);
            if (e()){
                CYX::ReplaceServerName(srvName, srvName);
            } else {
                MessageBox("Cancelled operation", "No changes were made.");
            }
        }
    }

    void versionSpoof(MenuEntry *entry){
        menu:
        u32 oldVersion = *(u32*)Hooks::offsets.versionInt, newVersion = 0;
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
            *(u32*)Hooks::offsets.versionInt = CYX::currentVersion;
            break;
        case 1:
            kbd.IsHexadecimal(true);
            PLGSET(PLGFLG_SPOOFED_VER);
            if (kbd.Open(newVersion, oldVersion)==0) *(u32*)Hooks::offsets.versionInt = newVersion;
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
                Utils::Format("\nCONTROLLER func @ 0x%08X",CYX::basControllerFunc.funcAddr)
            ),
            DialogType::DialogOk, ClearScreen::Both)();
    }
    u8 cyxAPItoggle_handleSeverity(int index, u8 mode){
        switch (index){
        case 1: // SysInfo
            break;
        case 2: // FWInfo
            break;
        case 3: // HWInfo
            break;
        case 4: // SafeDir
            break;
        case 5: // XREF
            if (mode == 3 && !MessageBox("You're about to enable write access to other projects.\nOnly enable this setting if you trust the programs in this project.\n\nContinue anyway?", DialogType::DialogYesNo, ClearScreen::Both)())
                return 0;
            break;
        case 6: // SDAccess
            if (mode == 3 && !MessageBox("You're about to enable write access to the SD Card.\nOnly enable this setting if you trust the programs in this project.\n\nContinue anyway?", DialogType::DialogYesNo, ClearScreen::Both)())
                return 0;
            break;
        }
        return mode;
    }
    void cyxAPItoggle_getMsg(std::string& out, int index, u8 mode){
        switch (index){
        case -1:
            out = "";
            break;
        case 0:
            switch (mode){
                case 0: out="Disabled access to the CYX API."; break;
                case 1: out="Enabled access to the CYX API."; break;
            }
            break;
        case 1:
            switch (mode){
                case 0: out="Disabled access to system info."; break;
                case 1: out="Enabled access to system info."; break;
            }
            break;
        case 2:
            switch (mode){
                case 0: out="Disabled access to firmware info."; break;
                case 1: out="Enabled access to firmware info."; break;
            }
            break;
        case 3:
            switch (mode){
                case 0: out="Disabled access to hardware info."; break;
                case 1: out="Enabled access to hardware info."; break;
            }
            break;
        case 4:
            switch (mode){
                case 0: out="Disabled access to the current project's safe folder."; break;
                case 1:
                    CYX::CreateHomeFolder();
                    out="Enabled access to the current project's safe folder.";
                    break;
            }
            break;
        case 5:
            switch (mode){
                case 0: out="Disabled access to other projects."; break;
                case 1: out="Enabled read access to other projects."; break;
                case 3: out="Enabled read/write access to other projects."; break;
            }
            break;
        case 6:
            switch (mode){
                case 0: out="Disabled access to the SD Card."; break;
                case 1: out="Enabled read access to the SD Card."; break;
                case 3: out="Enabled read/write access to the SD Card."; break;
            }
            break;
        }
    }
    void cyxAPItoggle(MenuEntry* entry){
        if (!CYX::basControllerFunc.funcAddr) {
            MessageBox("The CONTROLLER function was not hooked. Check for an updated plugin.")();
            return;
        }
        bool api = CYX::GetAPIAvailability();
        std::string staticMsg =
            "CYX API settings for \""+CYX::g_currentProject+
            "\"\nDisclaimer:\n"
            "This feature is experimental and may cause\n"
            "instability.\nSelect the below options with care.";
        Keyboard kbd(""); int kres;
        StringVector opt; opt.resize(8);
        opt[7] = "\uE072 Back";
        std::string _2w[3] = {"●⊃", Color::Red << "●⊃", Color::Lime << "⊂●"};
        std::string _3w[5] = {"●\u3000⊃", Color::Red << "●\u3000⊃", Color::Orange << "⊂●⊃", "", Color::Lime << "⊂\u3000●"};
        std::string message = "";
        bool w[7] = {0,0,0,0,0,1,1};
        u8 b[7] = {0,0,1,2,16,17,19};
        u8 __tmp;
        while (true){
            kbd.DisplayTopScreen = true;
            kbd.GetMessage() = staticMsg + "\n\n" << Color(0x60FF00FF) << message;
            if (CYX::WasCYXAPIUsed()){
                kbd.GetMessage() +=
                    Color(0xFF8800FF) << "\n\nThe CYX API has been utilized.\n" <<
                    "Disabling the hook and/or the API may cause\n" <<
                    "unexpected behaviour." + ResetColor();
            }
            opt[0] = "Enable API  " + _2w[1+api];
            opt[1] = "Read SysInfo  ";
            opt[2] = "Read Firmware Info  ";
            opt[3] = "Read Hardware Info  ";
            opt[4] = "Allow safe dir  ";
            opt[5] = "Cross-project access  ";
            opt[6] = "SD Card access  ";
            if (api){
                for (int i=1; i<7; i++){
                    opt[i] = opt[i] + (
                        w[i] ? _3w[1+((BasicAPI::flags>>b[i])&3)] : _2w[1+((BasicAPI::flags>>b[i])&1)]
                    );
                }
            } else {
                for (int i=1; i<7; i++){
                    opt[i] = Color::Gray << opt[i] + (
                        w[i] ? _3w[0] : _2w[0]
                    );
                }
            }
            kbd.Populate(opt, 0);
            for (int i=0; i<7; i++)
                kbd.ChangeEntrySound(i, SoundEngine::Event::DESELECT);
            kbd.ChangeEntrySound(7, SoundEngine::Event::CANCEL);
            kres = kbd.Open();
            switch (kres){
            case 0:
                CYX::SetAPIAvailability(__tmp = api = !api);
                CYX::DiscardAPIUse();
                break;
            case 1: case 2: case 3: case 4:
            case 5: case 6:
                if (api){
                    if (w[kres]){
                        __tmp = (BasicAPI::flags>>b[kres])&3;
                        __tmp = (__tmp + 1 + (__tmp==1)) & 3;
                        __tmp = cyxAPItoggle_handleSeverity(kres, __tmp);
                        BasicAPI::flags &= ~(3<<b[kres]);
                        BasicAPI::flags |= (__tmp<<b[kres]);
                        
                    } else {
                        __tmp = (BasicAPI::flags>>b[kres])&1;
                        __tmp = cyxAPItoggle_handleSeverity(kres, !__tmp);
                        BasicAPI::flags &= ~(1<<b[kres]);
                        BasicAPI::flags |= (__tmp<<b[kres]);
                    }
                }
                break;
            default:
                return;
            }
            if (!api) kres=-1;
            cyxAPItoggle_getMsg(message, kres, __tmp);
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
    void grpFreeMe(MenuEntry* entry){
        if (entry->IsActivated()){
            PLGSET(PLGFLG_EXPERIMENTS);
            for (u8 i=0; i<6; i++){
                CYX::GraphicPage->grp[i].isResourceProtected = 0;
            }
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

    void fontGetAddrPatch(MenuEntry* entry){
        Keyboard kbd;
        u16 codePoint; int yOff, xOff; bool r;
        kbd.GetMessage() = "Do you want to allow FONTDEF to edit\nthe [X] character?";
        kbd.Populate((StringVector){
            "Allow", "Restrict", "Test"
        }, true);
        kbd.DisplayTopScreen = true;
        int kres = kbd.Open();
        if (kres == 2){
            kbd.GetMessage() = "Test FONTDEF strictness\n\nPlease enter a codepoint (hex) to test.";
            kbd.IsHexadecimal(true); // U+XXXX
            if (kbd.Open(codePoint)>=0){
                r = CYX::fontOff(codePoint, &yOff, &xOff); // Call into PTC's code
                kbd.GetMessage() = Utils::Format(
                    "Test FONTDEF strictness\n\n"
                    "The codepoint U+%04X returned\n\n"
                    "X: %ld\nY: %ld\n\n"
                    "FONTDEF allowed: %s",
                    codePoint, xOff, 504-yOff, r ? "Yes" : "No"
                );
                kbd.Populate((StringVector){"Okay"});
                kbd.Open();
            }
        } else if (kres >= 0) CYX::SetFontGetAddressStrictness(kres);
    }
}