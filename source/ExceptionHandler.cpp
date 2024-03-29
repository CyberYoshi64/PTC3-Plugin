#include "ExceptionHandler.hpp"
#include "BasicAPI.hpp"

#define EXCEPTIONSCREEN_DELAY   30.0
#define EXCEPTIONSCREENSAD_INT  450
#define EXCEPTIONRESCUEWRITEBUF 4096

extern "C" char* __textend;

namespace CTRPluginFramework {
    ExceptionSettings Exception::excepSet;
    std::string Exception::panicString = "";
    std::vector<std::vector<bool>> Exception::qrModule;
    u32 Exception::screenSadMessageIndex = 0;
    u32 Exception::state = 0;
    u32 Exception::timer = 0;

    bool Exception::dumpAsText = false;

    u8 Exception::dataBuffer[160]={0};
    u16 Exception::dataLength = 0;

    void Exception::Panic(std::string s) {
        panicString = s; dumpAsText = true;
        s.clear();
        
        PLGSET(PLGFLG_PANIC);
        *(u32*)1337 = 0xBEEF; // Trigger an exception intentionally
    }

    void Exception::Error(std::string s, const char* f, s32 line) {
        if (f != NULL)
            s += Utils::Format("\n(Called by %s:%d)",f,line);
        StringVector vec = {
            "Rescue programs",
            "Rescue GRP pages",
            "Rescue clipboard",
            "\uE071 Return to \uE073HOME Menu"
        };
        std::string _2w[2] = {Color::Red << "\u3000●⊃", Color::Lime << "\u3000⊂●"};
        StringVector kv;
        Keyboard k(s);

        mcuSetSleep(false);
        while (true) {
            kv = {
                vec[0] + _2w[(bool)(Exception::excepSet.rescue & EXCEPRESCUE_PROGRAM)],
                vec[1] + _2w[(bool)(Exception::excepSet.rescue & EXCEPRESCUE_GRAPHICS)],
                vec[2] + _2w[(bool)(Exception::excepSet.rescue & EXCEPRESCUE_CLIPBOARD)],
                "",
                vec[3]
            };
            k.Populate(kv);
            k.ChangeEntrySound(0, SoundEngine::Event::SELECT);
            k.ChangeEntrySound(1, SoundEngine::Event::SELECT);
            k.ChangeEntrySound(2, SoundEngine::Event::SELECT);
            k.ChangeEntrySound(4, SoundEngine::Event::CANCEL);
            int res = k.Open();
            if (res > 2) break;
            switch (res) {
                case 0:
                    Exception::excepSet.rescue ^= EXCEPRESCUE_PROGRAM;
                    break;
                case 1:
                    Exception::excepSet.rescue ^= EXCEPRESCUE_GRAPHICS;
                    break;
                case 2:
                    Exception::excepSet.rescue ^= EXCEPRESCUE_CLIPBOARD;
                    break;
            }
        }
        Config::Get().recoverFromException = true;
        Config::Get().wasExceptionFatal = false;
        Config::Get().clearCache = true;
        Config::Get().validateSaveData = true;
        Config::Get().lastExcepDumpID = 0;
        if (excepSet.rescue) Exception::RescueIfRequired();
        CYX::Finalize();
        mcuSetSleep(true);
        Process::ReturnToHomeMenu();
    }

    void Exception::BuildRescueScreen(u8 mode, u32 i, u32 j, std::string& s2) {
        std::string s;
        switch (mode) {
            case 0: s = Utils::Format("Saving asset %d/%d ...", i+1, j); break;
        }
        
        OSD::Lock();
        Screen bot = OSD::GetBottomScreen();
        bot.DrawRect(0,0,320,240,Color(0x301820FF));
        if (!mode) {
            bot.DrawSysfont(s, (320-Render::GetTextWidth(s))/2, 112);
            bot.Draw(s2, (320-(s2.length()*6))/2, 144, Color::Gray, Color(0x301820FF));
        } else {
            bot.DrawSysfont("Success!", 12, 12, Color::Lime);
            bot.DrawSysfont("The asset dump can be found here:", 16, 48);
            bot.Draw(s2, 20, 68);
            bot.DrawSysfont("Continuing in 3 seconds...", 16, 100);
        }
        OSD::SwapBuffers();
        OSD::Unlock();
    }

    void Exception::RescueIfRequired() {
        u64 dumpNum = osGetTime()/1000;
        std::string fname = Utils::Format(DUMP_PATH"/%016X.cyxdmp",dumpNum);
        File outf; std::string s1;
        if (!Directory::Exists(DUMP_PATH)) Directory::Create(DUMP_PATH);
        if (File::Open(outf, fname, File::Mode::RWC | File::Mode::SYNC)==0) {
            u32 currentBlob = 0; u8 blobCat = 0, blobCatIdx = 0;
            u32 fcount =
                (bool)(excepSet.rescue & EXCEPRESCUE_PROGRAM)*4+
                (bool)(excepSet.rescue & EXCEPRESCUE_GRAPHICS)*6+
                (bool)(excepSet.rescue & EXCEPRESCUE_CLIPBOARD)*1;
            CYXDumpHeader hdr = mkHeader(excepSet.rescue, fcount);
            while ((currentBlob < fcount) && (blobCat < 3)) {
                if (!(excepSet.rescue & (1<<blobCat))) {
                    blobCatIdx=0; blobCat++; continue;
                }
                switch (blobCat) {
                case 0: // PRG
                    if (blobCatIdx >= 4){
                        blobCatIdx=0; blobCat++; continue;
                    }
                    s1 = CYX::GetProgramSlotFileName(blobCatIdx);
                    if (s1=="")
                        sprintf(hdr.blobName[currentBlob], "PRG%d", blobCatIdx);
                    else
                        sprintf(hdr.blobName[currentBlob], "PRG%d-%s", blobCatIdx, s1.c_str());
                    hdr.blobBufSize[currentBlob] = sizeof(CYX::editorInstance->programSlot[blobCatIdx].text);
                    hdr.blobDataLen[currentBlob] = CYX::editorInstance->programSlot[blobCatIdx].text_len * 2;
                    break;
                case 1: // CLIP
                    if (blobCatIdx) {
                        blobCatIdx=0; blobCat++; continue;
                    }
                    sprintf(hdr.blobName[currentBlob], "CLP0");
                    hdr.blobBufSize[currentBlob] = sizeof(CYX::editorInstance->clipboardData);
                    hdr.blobDataLen[currentBlob] = CYX::editorInstance->clipboardLength * 2;
                    break;
                case 2: // GRP
                    if (blobCatIdx >= 6) {
                        blobCatIdx=0; blobCat++; continue;
                    }
                    sprintf(hdr.blobName[currentBlob], "GRP%d", blobCatIdx);
                    hdr.blobBufSize[currentBlob] = hdr.blobDataLen[currentBlob] = 524288;
                    break;
                }
                blobCatIdx++;
                currentBlob++;
            }
            outf.Write(&hdr, sizeof(hdr));
            currentBlob = 0, blobCat = 0, blobCatIdx = 0;
            while ((currentBlob < fcount) && (blobCat < 3)) {
                std::string s = hdr.blobName[currentBlob];
                BuildRescueScreen(0, currentBlob, fcount, s);
                if (!(excepSet.rescue & (1<<blobCat))) {
                    blobCatIdx=0; blobCat++;
                    continue;
                }
                switch (blobCat) {
                case 0: // PRG
                    if (blobCatIdx >= 4) {
                        blobCatIdx=0; blobCat++; continue;
                    }
                    outf.Write(CYX::editorInstance->programSlot[blobCatIdx].text, hdr.blobBufSize[currentBlob]);
                    break;
                case 1: // CLIP
                    if (blobCatIdx) {
                        blobCatIdx=0; blobCat++; continue;
                    }
                    outf.Write(CYX::editorInstance->clipboardData, hdr.blobBufSize[currentBlob]);
                    break;
                case 2: // GRP
                    if (blobCatIdx >= 6) {
                        blobCatIdx=0; blobCat++; continue;
                    }
                    outf.Write(CYX::GraphicPage->grp[blobCatIdx].workBuf, hdr.blobBufSize[currentBlob]);
                }
                blobCatIdx++;
                currentBlob++;
            }
            Config::Get().lastExcepDumpID = dumpNum;
        }
        outf.Close();
        BuildRescueScreen(1, 0, 0, fname);
        Sleep(Seconds(3));
    }

    const char* Exception::SadMessage[] = {
        "That's a shame.",
        "What have you done!?",
        "Kinda embarrassing.",
        "How dare you, Cyber!",
        "The sand can be eaten.",
        "Unterhaltungselektronik, na?",
        "I don't give a gold coin!",
        "Help me, Doctor Hakase!",
        "Wan-Paku has messed up.",
        "Syntax Error!",
        "I hope you got backups…",
        "How unfortunate.",
    };

    void Exception::BuildScreen(Screen& top, Screen& bot) {
        
        bool flag; Color ccol;
        double startAnimFlt;
        float qrTexScale, px, _py, py;
        u32 qrSize, iw, ih, x, y, qrViewPort;

        switch (state) {
        case 1:
            startAnimFlt = sin((timer / EXCEPTIONSCREEN_DELAY) * 1.5707963267948966);
            ccol = Color((88-startAnimFlt*88),(64-startAnimFlt*64),(72-startAnimFlt*72));
            top.DrawRect(0, 0, 400, 240*startAnimFlt, ccol);
            bot.DrawRect(0, 241-240*startAnimFlt, 320, 240*startAnimFlt, ccol);
            if (timer >= EXCEPTIONSCREEN_DELAY) state++;
            break;
        case 2: case 3: // Render twice because double-buffer
            top.DrawRect(0, 0, 400, 240, Color::Black);
            bot.DrawRect(0, 0, 320, 240, Color::Black);
            top.DrawSysfont("A fatal error has occured", 4, 3, Color::Red);
            top.DrawSysfont("A fatal error has occured", 3, 3, Color::Red);
            top.DrawSysfont("SmileBASIC was shut down.", 6, 20);

            top.DrawSysfont("This QR is no link, but", 6, 60);
            top.DrawSysfont("contains the info for the", 6, 75);
            top.DrawSysfont("crash. If this keeps happening,", 6, 90);
            top.DrawSysfont("join CY64's Discord and", 6, 105);
            top.DrawSysfont("ask for help with this code.", 6, 120);
            
            top.DrawSysfont("\uE001 Return to \uE073HOME Menu", 4, 145);
            top.DrawSysfont("\uE002 Reboot", 4, 160);
            top.DrawSysfont("\uE003 Show details", 4, 175, Color(255, 160, 144));
            
            x = 208;
            y = 0;
            qrViewPort = 192;
            qrSize = qrModule.size();
            top.DrawRect(x, y, qrViewPort, qrViewPort, Color::White);
            qrTexScale = (qrViewPort-10) / (float)qrSize;
            if (qrTexScale >= 2) qrTexScale = (u32)qrTexScale;
            px = x+(qrViewPort - (qrSize*qrTexScale))/2;
            _py = y+(qrViewPort - (qrSize*qrTexScale))/2;
            for (u16 ix=0; ix<qrSize; ix++) {
                py=_py;
                for (u16 iy=0; iy<qrSize; iy++) {
                    
                    iw = (u32)(px+qrTexScale) - (u32)px;
                    ih = (u32)(py+qrTexScale) - (u32)py;
                    top.DrawRect(px, py, iw, ih, qrModule.at(iy).at(ix) ? Color::Black : Color::White);
                    py += qrTexScale;
                }
                px += qrTexScale;
            }
            
            if (PLGGET(PLGFLG_PANIC)) {
                Color bg = Color(80, 40, 16);
                top.DrawRect(0, 192, 400, 48, bg);
                top.Draw("The plugin panicked. This is the context given:", 4, 196, Color::Orange, bg);
                top.Draw(panicString, 10, 210, Color::White, bg);
            } else {
                Color bg = Color(40, 16, 80);
                top.DrawRect(0, 192, 400, 48, bg);
                top.Draw("The crash is within SmileBASIC itself. The backtrace may help", 4, 196, Color::Magenta, bg);
                top.Draw("diagnose the issue.", 4, 206, Color::Magenta, bg);
                if (PLGGET(PLGFLG_EXPERIMENTS)) {
                    top.Draw("You have used Experiments. They might have caused the problem.", 6, 220, Color::Orange, bg);
                }
            }

            bot.DrawSysfont("If the QR code somehow fails, a crash dump",5,5);
            bot.DrawSysfont("file is provided at:",5,20);
            bot.DrawSysfont("sdmc:" EXCEPTION_SYSDUMP_PATH, 10,40, Color::Turquoise);

            bot.DrawSysfont("Contact info, such as the Discord invite link",5,60);
            bot.DrawSysfont("can be found at:",5,75);
            bot.DrawSysfont("https://cyberyoshi64.github.io", 10,95, Color::DeepSkyBlue);

            bot.DrawSysfont("Try rescuing...", 4, 160);
            bot.DrawRect(4, 176, 110, 1, Color::White);
            bot.Draw("Program slots", 24, 184);
            bot.Draw("Graphic pages", 24, 204);
            bot.Draw("Clipboard data", 24, 224);
            state++;
            break;
        case 4: // Only render parts that could change
            top.DrawRect(6, 35, 202, 18, Color::Black);
            top.DrawSysfont(SadMessage[screenSadMessageIndex], 6, 35);
            
            bot.DrawRect(0, 180, 22, 60, Color::Black);
            flag = Exception::excepSet.rescue & EXCEPRESCUE_PROGRAM;
            bot.DrawRect(5, 180, 16, 16, flag ? Color::ForestGreen : Color::Gray, flag);
            flag = Exception::excepSet.rescue & EXCEPRESCUE_GRAPHICS;
            bot.DrawRect(5, 200, 16, 16, flag ? Color::ForestGreen : Color::Gray, flag);
            flag = Exception::excepSet.rescue & EXCEPRESCUE_CLIPBOARD;
            bot.DrawRect(5, 220, 16, 16, flag ? Color::ForestGreen : Color::Gray, flag);
            break;
        }
    }
    void Exception::BuildExceptionData(ERRF_ExceptionInfo *excep, CpuRegisters *regs) {
        if (dumpAsText) {
            ExceptionSysDumpTxt d = {0};
            dataLength = sizeof(ExceptionSysDumpTxt);
            std::string s = panicString;
            s.resize(sizeof(d.err));
            sprintf(d.err, "%s", s.c_str());
            d.lang = EXCEP_LANG;
            d.plgVer = VER_INTEGER;
            d.plgFlg = PLGFLAGS;
            d.ptcverOrig = CYX::currentVersion;
            d.magic = EXCSYSDMPHDR_MAGIC;
            d.version = EXCSYSDMPTXT_VER;
            d.type = 0xFF;
            memcpy(dataBuffer, &d, dataLength);
        } else {
            ExceptionSysDump d = {0};
            dataLength = sizeof(ExceptionSysDump);
            d.lang = EXCEP_LANG;
            d.plgVer = VER_INTEGER;
            d.plgFlg = PLGFLAGS;
            d.ptcverOrig = CYX::currentVersion;
            d.magic = EXCSYSDMPHDR_MAGIC;
            d.version = EXCSYSDMPDAT_VER;
            for (u8 i=0; i<4; i++) sprintf(d.prgn[i],"%s",CYX::GetProgramSlotFileName(i).c_str());
            d.lr = regs->lr;
            d.pc = regs->pc;
            d.far = excep->far;
            d.type = (int)excep->type;

            // Call stack trace code is from the
            // CTGP-7 Open Source Project:
            // https://github.com/PabloMK7/CTGP-7_Open_Source

            u32 sp, soff = 0;
            sp = d.sp = regs->sp;
            
            u32 plgTextBeg = 0x07000100;
            
            u32 plgTextEnd = (u32)&__textend;
            u32 appTextBeg = 0x100000;
            u32 appTextEnd = appTextBeg + Process::GetTextSize();
            
            for (u32 i=0; i < (sizeof(d.callStack)/sizeof(u32)); i++) {
                u32 v;
                while (Process::Read32(sp+soff, v) && soff<0x7000) {
                    soff += 4;
                    if (v&0x3FF == 0) continue; // Most likely a length property of a sort
                    if ((v>=appTextBeg&&v<appTextEnd)||(v>=plgTextBeg&&v<plgTextEnd)) {
                        d.callStack[i] = v;
                        break;
                    }
                }
            }
            d.cyxApiFlags = BasicAPI::flags;
            strcpyu16u8(CYX::activeProject->activeProject, d.activeProject, 15);
            strcpyu16u8(CYX::activeProject->currentProject, d.currentProject, 15);
            memcpy(dataBuffer, &d, dataLength);
        }

        char t[20]={0}; dateTimeToString(t,osGetTime());
        File f; std::string fn = EXCEPTION_SYSDUMP_PATH;
        u16 md = File::RWC | File::TRUNCATE;
        if (File::Open(f, fn, md)==0) {
            f.Write(dataBuffer, dataLength);
        }
        f.Close();

    }
    bool excep__isTouching(UIntVector t, u16 x, u16 y, u16 w, u16 h) {
        return ((t.x >= x) && (t.y >= y) && (t.x < x+w) && (t.y < y+h));
    }
    Process::ExceptionCallbackState Exception::Handler(ERRF_ExceptionInfo *excep, CpuRegisters *regs) {
        u32 kMask = Key::B | Key::X | Key::Y;
        u32 keyP;
        SoundEngine::Event sndSel = SoundEngine::Event::SELECT;
        SoundEngine::Event sndDesel = SoundEngine::Event::DESELECT;
        Screen top = OSD::GetTopScreen();
        Screen bot = OSD::GetBottomScreen();
        static bool touchP, oldTouchP;

        if (!state) {
            mcuSetSleep(0);
            excepSet.version = EXCEPTSET_VERSION;
            
            Config::Get().recoverFromException = true;
            Config::Get().wasExceptionFatal = true;
            Config::Get().clearCache = true;
            Config::Get().lastExcepDumpID = 0;
            Config::Get().validateSaveData = true;
            BuildExceptionData(excep, regs);

            qrcodegen::QrCode __q = qrcodegen::QrCode::encodeText(Base64::Encode((u8*)dataBuffer, dataLength).c_str(), qrcodegen::QrCode::Ecc::MEDIUM);
            qrModule = __q.getRawModules();
            state++;
        } else {
            Controller::Update();
            if (!(timer % EXCEPTIONSCREENSAD_INT))
                screenSadMessageIndex = ((u32)rand()) % (sizeof(SadMessage)/sizeof(const char*));
            keyP = Controller::GetKeysPressed();
            UIntVector t = Touch::GetPosition();
            oldTouchP = touchP; touchP = Touch::IsDown();
            if (state > 3) {
                if (!oldTouchP && touchP){
                    if (excep__isTouching(t, 0, 178, 160, 20)) {
                        excepSet.rescue ^= EXCEPRESCUE_PROGRAM;
                        SoundEngine::PlayMenuSound(
                            excepSet.rescue & EXCEPRESCUE_PROGRAM ? sndSel : sndDesel
                        );
                    }
                    if (excep__isTouching(t, 0, 198, 160, 20)) {
                        excepSet.rescue ^= EXCEPRESCUE_GRAPHICS;
                        SoundEngine::PlayMenuSound(
                            excepSet.rescue & EXCEPRESCUE_GRAPHICS ? sndSel : sndDesel
                        );
                    }
                    if (excep__isTouching(t, 0, 218, 160, 20)) {
                        excepSet.rescue ^= EXCEPRESCUE_CLIPBOARD;
                        SoundEngine::PlayMenuSound(
                            excepSet.rescue & EXCEPRESCUE_CLIPBOARD ? sndSel : sndDesel
                        );
                    }
                }
                if (keyP & kMask) {
                    if (excepSet.rescue) Exception::RescueIfRequired();
                    CYX::Finalize();
                    switch (keyP) {
                    case Key::B:
                        mcuSetSleep(1);
                        return Process::EXCB_RETURN_HOME;
                    case Key::Y:
                        return Process::EXCB_DEFAULT_HANDLER;
                    case Key::X:
                        return Process::EXCB_REBOOT;
                    }
                }
            }
            OSD::Lock();
            BuildScreen(top, bot);
            timer++;
            OSD::SwapBuffers();
            OSD::Unlock();
        }
        return Process::EXCB_LOOP;
    }
    CYXDumpHeader Exception::mkHeader(u16 type, u16 cnt) {
        CYXDumpHeader d;
        memset(&d,0,sizeof(d));
        d.magic = *(u64*)CYXDMPHDR_MAGIC;
        d.version = CYXDMPHDR_VERSION;
        d.blobCount = cnt;
        d.contType = type;
        return d;
    }
}