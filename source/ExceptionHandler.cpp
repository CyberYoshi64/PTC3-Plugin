#include "ExceptionHandler.hpp"
#include <cmath>

#define EXCEPTIONSCREEN_DELAY   30.0
#define EXCEPTIONRESCUEWRITEBUF 4096

namespace CTRPluginFramework {
    ExceptionSettings Exception::excepSet;

    void ExceptionBuildRescueScreen(u8 mode, u32 i, u32 j, std::string& s2){
        std::string s;
        switch (mode){
            case 0: s = Utils::Format("Saving asset %d/%d ...", i+1, j); break;
        }
        
        OSD::Lock();
        Screen bot = OSD::GetBottomScreen();
        bot.DrawRect(0,0,320,240,Color(0x301820FF));
        if (!mode){
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

    void Exception::RescueIfRequired(){
        std::string fname = Utils::Format(DUMP_PATH"/%016X.cyxdmp",osGetTime());
        File outf;
        if (!Directory::IsExists(DUMP_PATH)) Directory::Create(DUMP_PATH);
        if (File::Open(outf, fname, File::Mode::RWC | File::Mode::SYNC)==0){
            u32 currentBlob = 0; u8 blobCat = 0, blobCatIdx = 0;
            u32 fcount =
                (bool)(excepSet.rescue & EXCEPRESCUE_PROGRAM)*4+
                (bool)(excepSet.rescue & EXCEPRESCUE_GRAPHICS)*6+
                (bool)(excepSet.rescue & EXCEPRESCUE_CLIPBOARD)*1;
            CYXDumpHeader hdr = mkHeader(excepSet.rescue, fcount);
            while ((currentBlob < fcount) && (blobCat < 3)){
                if (!(excepSet.rescue & (1<<blobCat))){
                    blobCatIdx=0; blobCat++; continue;
                }
                switch (blobCat) {
                case 0: // PRG
                    if (blobCatIdx >= 4){
                        blobCatIdx=0; blobCat++; continue;
                    }
                    sprintf(hdr.blobName[currentBlob], "PRG%d-%s", blobCatIdx, CYX::GetProgramSlotFileName(blobCatIdx).c_str());
                    hdr.blobBufSize[currentBlob] = sizeof(CYX::editorInstance->programSlot[blobCatIdx].text);
                    hdr.blobDataLen[currentBlob] = CYX::editorInstance->programSlot[blobCatIdx].text_len;
                    break;
                case 1: // CLIP
                    if (blobCatIdx){
                        blobCatIdx=0; blobCat++; continue;
                    }
                    sprintf(hdr.blobName[currentBlob], "CLP0");
                    hdr.blobBufSize[currentBlob] = sizeof(CYX::editorInstance->clipboardData);
                    hdr.blobDataLen[currentBlob] = CYX::editorInstance->clipboardLength;
                    break;
                case 2: // GRP
                    if (blobCatIdx >= 6){
                        blobCatIdx=0; blobCat++; continue;
                    }
                    sprintf(hdr.blobName[currentBlob], "GRP%d", blobCatIdx);
                    hdr.blobBufSize[currentBlob] = (512 * 512 * 2);
                    hdr.blobDataLen[currentBlob] = (512 * 512 * 2);
                    break;
                }
                blobCatIdx++;
                currentBlob++;
            }
            outf.Write(&hdr, sizeof(hdr));
            currentBlob = 0, blobCat = 0, blobCatIdx = 0;
            while ((currentBlob < fcount) && (blobCat < 3)){
                std::string s = hdr.blobName[currentBlob];
                ExceptionBuildRescueScreen(0, currentBlob, fcount, s);
                if (!(excepSet.rescue & (1<<blobCat))){
                    blobCatIdx=0; blobCat++;
                    continue;
                }
                switch (blobCat) {
                case 0: // PRG
                    if (blobCatIdx >= 4){
                        blobCatIdx=0; blobCat++; continue;
                    }
                    outf.Write(CYX::editorInstance->programSlot[blobCatIdx].text, hdr.blobBufSize[currentBlob]);
                    break;
                case 1: // CLIP
                    if (blobCatIdx){
                        blobCatIdx=0; blobCat++; continue;
                    }
                    outf.Write(CYX::editorInstance->clipboardData, hdr.blobBufSize[currentBlob]);
                    break;
                case 2: // GRP
                    if (blobCatIdx >= 6){
                        blobCatIdx=0; blobCat++; continue;
                    }
                    outf.Write(CYX::GraphicPage->grp[blobCatIdx].workBuf, hdr.blobBufSize[currentBlob]);
                }
                blobCatIdx++;
                currentBlob++;
            }
        }
        outf.Close();
        if (excepSet.rescue){
            ExceptionBuildRescueScreen(1, 0, 0, fname);
            Sleep(Seconds(3));
        }
    }

    void ExceptionBuildScreen(Screen& top, Screen& bot, u64 timer){
        bool flag;
        if (timer < EXCEPTIONSCREEN_DELAY){
            if (timer < 2){
                top.Fade(.5); bot.Fade(.5);
            }
            double f = sin((timer / EXCEPTIONSCREEN_DELAY) * 1.5707963267948966);
            top.DrawRect(0, 0, 400, 240*f, Color::Black);
            bot.DrawRect(0, 241-240*f, 320, 240*f, Color::Black);
        } else {
            top.DrawRect(0, 0, 400, 240, Color::Black);
            bot.DrawRect(0, 0, 320, 240, Color::Black);
            top.DrawSysfont("A fatal error has occured", 5, 4, Color::Red);
            top.DrawSysfont("A fatal error has occured", 4, 4, Color::Red);
            top.DrawSysfont("SmileBASIC has encountered a problem and has", 8, 30);
            top.DrawSysfont("to be shut down.", 8, 50);
            top.DrawSysfont("Press \uE001 to return to \uE073HOME Menu.", 8, 100);
            top.DrawSysfont("Press \uE003 to show exception details.", 8, 120);
            top.DrawSysfont("Press \uE002 to reboot.", 8, 140);

            flag = Exception::excepSet.rescue & EXCEPRESCUE_PROGRAM;
            bot.DrawRect(8, 180, 16, 16, flag ? Color::ForestGreen : Color::Gray, flag);
            bot.Draw("Rescue program slots", 26, 184);
            flag = Exception::excepSet.rescue & EXCEPRESCUE_GRAPHICS;
            bot.DrawRect(8, 200, 16, 16, flag ? Color::ForestGreen : Color::Gray, flag);
            bot.Draw("Rescue graphic pages", 26, 204);
            flag = Exception::excepSet.rescue & EXCEPRESCUE_CLIPBOARD;
            bot.DrawRect(8, 220, 16, 16, flag ? Color::ForestGreen : Color::Gray, flag);
            bot.Draw("Rescue clipboard content", 26, 224);
        }
    }
    bool excep__isTouching(UIntVector t, u16 x, u16 y, u16 w, u16 h){
        return ((t.x >= x) && (t.y >= y) && (t.x < x+w) && (t.y < y+h));
    }
    Process::ExceptionCallbackState Exception::Handler(ERRF_ExceptionInfo *excep, CpuRegisters *regs) {
        mcuSetSleep(0);
        excepSet.version = EXCEPTSET_VERSION;
        u32 kMask = Key::B | Key::X | Key::Y;
        u32 keyP;
        SoundEngine::Event sndSel = SoundEngine::Event::SELECT;
        SoundEngine::Event sndDesel = SoundEngine::Event::DESELECT;
        u64 timer = 0;
        Screen top = OSD::GetTopScreen();
        Screen bot = OSD::GetBottomScreen();
        bool touchP, oldTouchP;
        
        // Write an exception file here soon
        // TODO: Find program slot names + project name
        // MAYBE: Look for common crashes to blame user
        
        while (true) {
            Controller::Update();
            keyP = Controller::GetKeysPressed();
            UIntVector t = Touch::GetPosition();
            oldTouchP = touchP; touchP = Touch::IsDown();
            if (timer > EXCEPTIONSCREEN_DELAY){
                if (!oldTouchP && touchP){
                    if (excep__isTouching(t, 0, 178, 160, 20)){
                        excepSet.rescue ^= EXCEPRESCUE_PROGRAM;
                        SoundEngine::PlayMenuSound(
                            excepSet.rescue & EXCEPRESCUE_PROGRAM ? sndSel : sndDesel
                        );
                    }
                    if (excep__isTouching(t, 0, 198, 160, 20)){
                        excepSet.rescue ^= EXCEPRESCUE_GRAPHICS;
                        SoundEngine::PlayMenuSound(
                            excepSet.rescue & EXCEPRESCUE_GRAPHICS ? sndSel : sndDesel
                        );
                    }
                    if (excep__isTouching(t, 0, 218, 160, 20)){
                        excepSet.rescue ^= EXCEPRESCUE_CLIPBOARD;
                        SoundEngine::PlayMenuSound(
                            excepSet.rescue & EXCEPRESCUE_CLIPBOARD ? sndSel : sndDesel
                        );
                    }
                }
                if (keyP & kMask){
                    Exception::RescueIfRequired();
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
            ExceptionBuildScreen(top, bot, timer++);
            OSD::SwapBuffers();
            OSD::Unlock();
        }
    }
    CYXDumpHeader Exception::mkHeader(u16 type, u16 cnt){
        CYXDumpHeader d;
        memset(&d,0,sizeof(d));
        d.magic = *(u64*)CYXDMPHDR_MAGIC;
        d.version = 1;
        d.blobCount = cnt;
        d.contType = type;
        return d;
    }
}