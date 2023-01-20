#include "ExceptionHandler.hpp"

namespace CTRPluginFramework {
    Process::ExceptionCallbackState Exception::Handler(ERRF_ExceptionInfo *excep, CpuRegisters *regs) {
        OSD::Lock();
        Screen top = OSD::GetTopScreen();
        top.DrawRect(0, 0, 400, 240, Color());
        top.DrawSysfont("A fatal error has occured", 4, 4, Color::Red);
        top.DrawSysfont("SmileBASIC encountered an error from", 8, 30);
        top.DrawSysfont("which it can't recover from.", 8, 50);
        top.DrawSysfont("Press \uE002 to return to \uE073HOME Menu.", 8, 100);
        top.DrawSysfont("Press \uE003 to show exception details.", 8, 120);
        top.DrawSysfont("Press \uE001 to reboot.", 8, 140);
        Screen bot = OSD::GetBottomScreen();
        bot.DrawRect(0, 0, 320, 240, Color());
        bot.Draw("Crash details:",3,5);
        for (size_t i=0; i<13; i++)
            bot.Draw(Utils::Format("R%02D  : %08X", i,regs->r[i]),6+107*(i%3),20+(i/3)*10);
        bot.Draw(Utils::Format("FSR  : %08X",excep->fsr),113,60);
        bot.Draw(Utils::Format("FPINST %08X",excep->fpinst),220,60);
        bot.Draw(Utils::Format("CPSR : %08X",regs->cpsr),6,70);
        bot.Draw(Utils::Format("FAR  : %08X",excep->far),113,70);
        bot.Draw(Utils::Format("FPEXC: %08X",excep->fpexc),220,70);
        bot.Draw(Utils::Format("SP   : %08X",regs->sp),6,80);
        bot.Draw(Utils::Format("LR   : %08X",regs->lr),113,80);
        bot.Draw(Utils::Format("PC   : %08X",regs->pc),220,80);
        OSD::SwapBuffers();
        OSD::Unlock();
        
        // Write an exception file here soon
        // TODO: Find program slot names + project name
        // MAYBE: Look for common crashes to blame user
        
        Sleep(Seconds(1));
        while (true) {
            Controller::Update();
            switch (Controller::GetKeysPressed()) {
            case Key::X:
                return Process::EXCB_RETURN_HOME;
            case Key::Y:
                return Process::EXCB_DEFAULT_HANDLER;
            case Key::B:
                return Process::EXCB_REBOOT;
            }
            Sleep(Milliseconds(16));
        }
    }
}