#include "Misc.hpp"
#include "BasicAPI.hpp"

namespace CTRPluginFramework {

    void* generalPointer[8];
    u32 generalInt;

    void experiment1(MenuEntry* entry) {
        PANIC("experiment1(): PANIC() Test", __FILE, __LINE);
    }
    void experiment2(MenuEntry* entry) {
        ERROR_F("experiment2(): ERROR_F() Test", __FILE, __LINE);
    }
    void experiment3(MenuEntry* entry) {
        DANG("experiment3(): DANG() Test", __FILE, __LINE);
    }
    void experiment4(MenuEntry* entry) {
        DANG("experiment4(): There is no experiment.", __FILE, __LINE);
    }

    void restoreRescueDump(MenuEntry *entry) {
        StringVector s;
        Directory d;
        Directory::Open(d, DUMP_PATH);
        d.ListFiles(s, ".cyxdmp");
        d.Close();

        Keyboard k("Select a CYX rescue dump to restore.", s);

        int res = k.Open();

        if (res < 0) return;
        CYX::RestoreRescueDump(DUMP_PATH "/" + s[res]);
    }

    void serverAdrChg(MenuEntry *entry){
        std::string srvName;
        Keyboard kbd("");
        kbd.Populate({
            "Reset to original",
            "Specify custom server",
            "\uE072 Back"
        });
        kbd.ChangeEntrySound(2, SoundEngine::Event::CANCEL);
        int kbdres = kbd.Open();
        if (kbdres < 0 || kbdres >= 2) return;
        PLGSET(PLGFLG_EXPERIMENTS);
        if (kbdres == 0) {
            Config::Get().cyx.set.server.serverType = Config::Enums::ServerType::VANILLA;
            CYX::ReplaceServerName(SBSERVER_DEFAULT_NAME1, SBSERVER_DEFAULT_NAME2);
            return;
        } else {
            Keyboard kbd("Enter the server name to which to connect to:\n\n The domain name must be " TOSTRING(SBSERVER_URL_MAXLEN) " characters\n long or less.");
            kbd.SetMaxLength(SBSERVER_URL_MAXLEN);
            kbdres = kbd.Open(srvName, "http://");
            if (kbdres < 0) return;
            while (srvName.length() && srvName.back()=='/') srvName.pop_back();
            for (u32 i=0; i<srvName.size(); i++){
                if (srvName[i] < '%' || srvName[i] > 'z'){
                    MessageBox("The server name is not valid:\nThe name contains invalid characters.", DialogType::DialogOk, ClearScreen::Both)();
                    return;
                }
            }
            if (srvName.length()<=10){
                MessageBox("The server name is not valid:\nThe server name is too short.", DialogType::DialogOk, ClearScreen::Both)();
                return;
            }
            if (srvName.compare(0, 7, "http://")!=0&&srvName.compare(0, 8, "https://")!=0) {
                MessageBox("The server name is not valid:\nThe name must start with \"http://\" or \"https://\".", DialogType::DialogOk, ClearScreen::Both)();
                return;
            }
            while (strlen(srvName.c_str())>SBSERVER_URL_MAXLEN) srvName.resize(srvName.size()-1);
            if (MessageBox("Server name changer", srvName+"\n\nIs this correct?", DialogType::DialogYesNo, ClearScreen::Both)()){
                sprintf(Config::Get().cyx.set.server.serverName, "%s", srvName.c_str());
                Config::Get().cyx.set.server.serverType = Config::Enums::ServerType::GENERIC;
                CYX::ReplaceServerName(srvName, srvName);
            } else {
                MessageBox("Cancelled operation", "No changes were made.")();
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
    void pluginDisclaimer(MenuEntry *entry){
        std::string disclSet;
        u32 monthID = StringArchive::GetID("dateJanuary");
        u16 plgDisclVer;
        
        StringArchive::Get(disclSet, "plgDisclVer");
        plgDisclVer = *(u16*)&disclSet[0];
        
        int res = MessageBox(
            Utils::Format(StringArchive::Get("plgDisclTitle").c_str(), StringArchive::Get(monthID + disclSet[2]).c_str(), 2000 + disclSet[3]),
            StringArchive::Get("plgDiscl"),
            DialogType::DialogYesNo, ClearScreen::Both
        )();
        if (!res) {
            CYX::Finalize();
            Process::ReturnToHomeMenu();
        }
        Config::Get().pluginDisclAgreed = plgDisclVer;
    }
    void cyxAPItoggle(MenuEntry* entry){
        if (!CYX::basControllerFunc.funcAddr) {
            MessageBox("The CONTROLLER function was not hooked. Check for an updated plugin.")();
            return;
        }
        bool api = CYX::GetAPIAvailability();
        std::string staticMsg =
            ToggleDrawMode(Render::UNDERLINE) +
            "CYX API settings for \""+CYX::g_currentProject+"\"" +
            ToggleDrawMode(Render::UNDERLINE) +
            "\n\nDisclaimer:\n"
            "The API features are experimental and may\n"
            "cause instability or modify other projects' data.\n\n"
            "For project-specific flags, refer to the 'CYX'\n"
            "section of the SmileTool and/or the CFGSET API\n"
            "function.";
        Keyboard kbd(""); int kres;
        StringVector opt; opt.resize(2);
        opt[1] = "\uE072 Back";
        std::string _2w[3] = {"●⊃", Color::Red << "●⊃", Color::Lime << "⊂●"};
        std::string _3w[5] = {"●\u3000⊃", Color::Red << "●\u3000⊃", Color::Orange << "⊂●⊃", "", Color::Lime << "⊂\u3000●"};
        std::string message = "";
        bool w[1] = {0};
        u8 b[1] = {0};
        u8 __tmp;
        while (true){
            kbd.DisplayTopScreen = true;
            kbd.GetMessage() = staticMsg;
            opt[0] = "Enable API  " + _2w[1+api];
            /*
            opt[1] = "Allow BASIC toggle  ";
            if (api){
                for (int i=1; i<2; i++){
                    opt[i] = opt[i] + (
                        w[i] ? _3w[1+((BasicAPI::flags>>b[i])&3)] : _2w[1+((BasicAPI::flags>>b[i])&1)]
                    );
                }
            } else {
                for (int i=1; i<2; i++){
                    opt[i] = Color::Gray << opt[i] + (
                        w[i] ? _3w[0] : _2w[0]
                    );
                }
            }
            */
            kbd.Populate(opt, 0);
            for (int i=0; i<1; i++)
                kbd.ChangeEntrySound(i, SoundEngine::Event::DESELECT);
            kbd.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
            kres = kbd.Open();
            if (kres == 0) {
                Config::Get().cyx.enableAPI = __tmp = api = !api;
                CYX::SetAPIAvailability(api);
                CYX::DiscardAPIUse();
            } else if (kres > 0 && kres < 1) {
                if (api){
                    if (w[kres]){
                        __tmp = (BasicAPI::flags>>b[kres])&3;
                        __tmp = (__tmp + 1 + (__tmp==1)) & 3;
                        BasicAPI::flags &= ~(3<<b[kres]);
                        BasicAPI::flags |= (__tmp<<b[kres]);
                        
                    } else {
                        __tmp = (BasicAPI::flags>>b[kres])&1;
                        BasicAPI::flags &= ~(1<<b[kres]);
                        BasicAPI::flags |= (__tmp<<b[kres]);
                    }
                }
            } else {
                return;
            }
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
        u32 sline = 0, scol = 0;
        std::string t; StringVector s;
        switch (ev.type){
            case ev.SelectionChanged:
                if (ev.selectedIndex < 0 || ev.selectedIndex > 7) return;
                generalInt = ev.selectedIndex;
                break;
            case ev.FrameTop:
                s = {
                    "\005REM\001 Look, \0061\001 sample\006.",
                    "\003@I_hope\002's good...",
                    "\004CONTROLLER\006 0\002 '\uFF3C\uE008\uFF0F",
                    "?\007\"サイバーヨッシー６４ rules, right?\"",
                    "Currently in \005use\001:",
                    ">> "+(*(StringVector*)generalPointer[1])[generalInt]
                };
                kbd.GetMessage() = "Select the color scheme to use.";
                c = CYX::textPalette->editSelectFG;
                c0 = Color(SWITCHEND32(c));
                c = ((u32**)generalPointer[0])[generalInt][0];
                c1 = Color(SWITCHEND32(c));
                c = ((u32**)generalPointer[0])[generalInt][1];
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
                    r->DrawSysString(t, 100-c, 59+sline*16, Color::Black);
                    for (u32 si1=0; si1 < s[si0].size(); si1++){
                        t = s[si0].at(si1);
                        if ((c = t[0]) < ' '){
                            switch (c){
                                case 0: // Cut off
                                    si1 = 0x7FFFDEAD;
                                    break;
                                case 1: // Reset color
                                    c3 = Color(SWITCHEND32(CYX::textPalette->editText));
                                    break;
                                case 2: // Comment
                                    c3 = Color(SWITCHEND32(CYX::textPalette->editComment));
                                    break;
                                case 3: // Label
                                    c3 = Color(SWITCHEND32(CYX::textPalette->editLabel));
                                    break;
                                case 4: // Statement
                                    c3 = Color(SWITCHEND32(CYX::textPalette->editStatement));
                                    break;
                                case 5: // Keyword
                                    c3 = Color(SWITCHEND32(CYX::textPalette->editKeywords));
                                    break;
                                case 6: // Numeric
                                    c3 = Color(SWITCHEND32(CYX::textPalette->editNumeric));
                                    break;
                                case 7: // String
                                    c3 = Color(SWITCHEND32(CYX::textPalette->editString));
                                    break;
                            }
                            continue;
                        }
                        if (c = (t[0]>=0xC0)+(t[0]>=0xE0)+(t[0]>=0xF0)) {
                            t = s[si0].substr(si1, c+1);
                            si1 += c;
                        }
                        c = Render::GetTextWidth(t)/2;
                        r->DrawSysString(t, 115+scol*12-c, 59+sline*16, c3, 360);
                        if (scol++ >= 20) {
                            scol = 0; sline++;
                        }
                    }
                    c3 = Color(SWITCHEND32(CYX::textPalette->editText));
                    scol = 0; sline++;
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
        StringVector str = {
            "Original",
            "Inverted",
            "Aqua",
            "Jungle",
            "Dark 1",
            "Dark 2",
            "Light",
        };
        u32* ptr[] = {pal0,pal1,pal2,pal3,pal4,pal5,pal6};
        generalPointer[0] = ptr;
        generalPointer[1] = &str;
        generalInt = 0;
        
        Keyboard kbd;
        kbd.OnKeyboardEvent(editorRulerPalCallback);
        kbd.Populate(str, true);
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
        } else if (kres >= 0) {
            Config::Get().cyx.fontdefStrict = kres;
            CYX::SetFontGetAddressStrictness(kres);
        }
    }
    void validateFile(MenuEntry* entry){
        #define BUFFER_SIZE 262144
        char* hmac = (char*)Hooks::offsets.serverHMACKey;
        int hmac_len = strlen(hmac), res = 0;
        std::string out = "/"+CYX::GetExtDataFolderName();
        std::string headerString = "Validating...";
        Screen sc1 = OSD::GetBottomScreen();
        StringVector strings; bool inFolder = true;
        Keyboard k; u32 entries;
        Directory d; Directory::Open(d, SAVEDATA_PATH+out);
        entries = d.ListFiles(strings);
        d.Close();
        strings.push_back("\uE072 Back");
        k.Populate(strings);
        k.DisplayTopScreen = true;

        while (true){
            if (inFolder){
                k.GetMessage() = "Select a file to validate in\n\"ptc:"+out+"\".";
            } else {
                k.GetMessage() = "Select a project to validate a file of.\n";
            }
            k.GetMessage() += "\n\n\uE07D Move cursor\n\uE000 Select\n\uE001 Back";
            k.ChangeEntrySound(entries, SoundEngine::Event::CANCEL);
            res = k.Open();
            if (res == entries) res = -1;
            if (res < -1) return;
            if (res == -1){
                if (inFolder){
                    inFolder = false;
                    out = "";
                    strings.clear();
                    Directory::Open(d, SAVEDATA_PATH);
                    entries = d.ListDirectories(strings);
                    d.Close();
                    strings.push_back("\uE072 Back");
                    k.Populate(strings);
                } else
                    return;
            } else {
                out += "/"+strings[res];
                if (inFolder){
                    break;
                } else {
                    inFolder = true;
                    strings.clear();
                    Directory::Open(d, SAVEDATA_PATH+out);
                    entries = d.ListFiles(strings);
                    d.Close();
                    strings.push_back("\uE072 Back");
                    k.Populate(strings);
                }
            }
        }
        out = SAVEDATA_PATH + out;

        OSD::Lock();
        for (u32 b=0; b<2; b++){
            OSD::GetTopScreen().DrawRect(33, 23, 334, 194, Color::Black, true);
            sc1.DrawRect(23, 23, 274, 194, Color::Black, true);
            sc1.DrawSysfont(headerString, 160 - Render::GetTextWidth(headerString)/2, 80);
            sc1.DrawRect(90, 125, 140, 25, Color::White, false);
            OSD::SwapBuffers();
        }
        OSD::Unlock();
        File f;
        if ((res = File::Open(f, out, File::RW))){
            MessageBox("An error has occured while opening the file.\n\nStatus code: "+Utils::Format("%08X", res), DialogType::DialogOk, ClearScreen::Both)();
            return;
        }
        u8 digest[20], oldDigest[20];
        u16 waitIc$[2] = {0};
        u32 waitIcT;
        void* buf = new u8[BUFFER_SIZE];
        if (!buf) abort();
        bool aborted = false;
        u32 size, size2, chunk;
        u32 perc = 0, operc;
        size = size2 = f.GetSize()-20;
        f.Seek(-20, File::END);
        if (res = f.Read(oldDigest, 20)){
            MessageBox("An error occured while reading the existing signature.\nStatus code: "+Utils::Format("%08X", res), DialogType::DialogOk, ClearScreen::Both)();
            f.Close();
            return;
        }
        f.Seek(0, File::SET);

        SHA1_HMAC::CTX hmacCtx;
        SHA1_HMAC::Init(&hmacCtx, (u8*)hmac, hmac_len);
        OSD::Lock();
        while (size && !aborted){
            operc = perc;
            perc = (1.f - (float)size / (float)size2) * 1000;
            waitIc$[0] = 0xE020 + ((waitIcT++ / 4) & 7);
            headerString = Utils::Format("%.1f %%", perc/10.f);
            sc1.DrawRect(91, 126, 138, 23, Color::Black);
            sc1.DrawRect(91, 126, 138.f * (perc / 1000.f), 23, Color::Teal);
            sc1.DrawSysfont(headerString, 160 - Render::GetTextWidth(headerString)/2, 128, Color::White);
            headerString.clear();
            Utils::ConvertUTF16ToUTF8(headerString, waitIc$);
            sc1.DrawRect(145, 100, 30, 20, Color::Black);
            sc1.DrawSysfont(headerString, 160 - Render::GetTextWidth(headerString)/2, 100, Color::Cyan);
            OSD::SwapBuffers();
            Controller::Update();
            if (Controller::GetKeysDown() & KEY_X) {
                aborted = true;
                break;
            }
            chunk = size > BUFFER_SIZE ? BUFFER_SIZE : size;
            if (res = f.Read(buf, chunk)){
                OSD::Unlock();
                MessageBox("An error occured while reading the file.\nStatus code: "+Utils::Format("%08X", res), DialogType::DialogOk, ClearScreen::Both)();
                f.Close();
                return;
            }
            SHA1_HMAC::Update(&hmacCtx, (u8*)buf, chunk);
            size -= chunk;
        }
        ::operator delete(buf);
        OSD::Unlock();
        
        if (!aborted){
            SHA1_HMAC::Final(digest, &hmacCtx);

            if (memcmp(digest, oldDigest, 20)) {
                f.Seek(-20, File::END);
                if (res = f.Write(digest, 20)){
                    MessageBox("An error occured while writing the new signature.\nStatus code: "+Utils::Format("%08X", res), DialogType::DialogOk, ClearScreen::Both)();
                } else {
                    MessageBox("The signature of the file has been corrected.", DialogType::DialogOk, ClearScreen::Both)();
                }
            } else {
                MessageBox("The signature of the file was valid. No changes were made.", DialogType::DialogOk, ClearScreen::Both)();
            }
        } else {
            MessageBox("The operation was cancelled. No changes were made.", DialogType::DialogOk, ClearScreen::Both)();
        }
        f.Close();
        #undef BUFFER_SIZE

    }

    void tokenHooker(MenuEntry* entry){
        std::string m;
        Keyboard k(
            ToggleDrawMode(Render::FontDrawMode::BOLD | Render::FontDrawMode::UNDERLINE) +
            "Server session token - Hook Test" +
            ToggleDrawMode(Render::FontDrawMode::BOLD | Render::FontDrawMode::UNDERLINE) +
            "\n\nIf enabled, the session token used with\nthe SmileBASIC server will be a dummy token.\n\n"
            "You may not use this with the official server\nas it will be rejected immediately.\n\n"
            "State: " + (StringVector){
                Color::Red<<"Disabled",
                Color::Lime<<"Enabled"
            }[CYX::petcServiceTokenHook.isEnabled] + ResetColor(),
            
            (StringVector){
                "Hook function",
                "Revert to original"
            }
        );
        int res = k.Open();
        if (res < 0) return;

        CYX::ResetServerLoginState();
        if (res == 0 || res == 1) {
            if (res==0){
                m = "The hook has been enabled.\nThis is for a rough test with a custom server and will not be useful on the official servers.";
                rtEnableHook(&CYX::petcServiceTokenHook);
                rtEnableHook(&CYX::nnActIsNetworkAccountHook);
            } else if (res==1) {
                m = "The hook has been disabled.\nPlease sign back into Nintendo Network to obtain a valid session token.";
                rtDisableHook(&CYX::petcServiceTokenHook);
                rtDisableHook(&CYX::nnActIsNetworkAccountHook);
            }

            Config::Get().cyx.set.server.serverType |= Config::Enums::ServerType::STUB_TOKEN;
            if (!res)
                Config::Get().cyx.set.server.serverType ^= Config::Enums::ServerType::STUB_TOKEN;
        }
        MessageBox(m, DialogType::DialogOk, ClearScreen::Both)();
    }
}