#include "MemDispOSD.hpp"

namespace CTRPluginFramework {
    u32 MemDisplayOSD::memStart = 0;
    u32 MemDisplayOSD::length = 0;
    u32 MemDisplayOSD::divider = 1;
    bool MemDisplayOSD::isValid = false;
    void MemDisplayOSD::setup(MenuEntry *entry){
        Keyboard k1(""), k2(""); int kres;
        bool run = true; u32 j;
        k1.Populate({"Change offset","Change length","Change divider","Close"});
        k1.ChangeEntrySound(3, SoundEngine::Event::CANCEL);
        k1.DisplayTopScreen = k2.DisplayTopScreen = true;
        while (run){
            j = memStart + length - 1;

            isValid =
                ((memStart >= 0x100000 && memStart < 0x1D4F000) ||
                (memStart >= 0x8000000 && memStart < 0xC8EE000) ||
                (memStart >= 0xFFC0000 && memStart < 0x10000000)) &&
                ((j >= 0x100000 && j < 0x1D4F000) ||
                (j >= 0x8000000 && j < 0xC8EE000) ||
                (j >= 0xFFC0000 && j < 0x10000000)) &&
                (divider > 0 && divider <= 32) &&
                (length > 0 && length <= 4096);
            
            if (isValid) PLGSET(PLGFLG_EXPERIMENTS);
            
            k1.GetMessage() =
                "Memory Display Settings\n\n"+
                Utils::Format(
                    "Start offset: 0x%08X\n"
                    "Length: %d (0x%08X)\n"
                    "Divider: %d",
                    memStart, length, memStart+length,
                    divider
                );
            if (!isValid){
                k1.GetMessage() += "" << Color::Red <<
                "\n\nThe parameters are invalid.\nPlease check if they're correct." <<
                ResetColor();
            }
            kres = k1.Open();
            switch (kres){
                case 0:
                    k2.GetMessage() = "Please enter the start address.\n\n"+Utils::Format("Currently: %08X",memStart);
                    k2.IsHexadecimal(true);
                    k2.Open(memStart);
                    break;
                case 1:
                    k2.GetMessage() = "Please enter the length.\n\n"+Utils::Format("Currently: %d",length);
                    k2.IsHexadecimal(false);
                    k2.Open(length);
                    length = (length+3)&~3;
                    break;
                case 2:
                    k2.GetMessage() = "Please enter the divider size.\n\n"+Utils::Format("Currently: %d",divider);
                    k2.IsHexadecimal(false);
                    k2.Open(divider);
                    divider = (divider+3)&~3;
                    break;
                case 3:
                    run = false;
                    break;
            }
        }
    }
    void MemDisplayOSD::OSDFunc(MenuEntry *entry){
        if (entry->WasJustActivated() && !isValid){
            MessageBox("The settings for the memory display are invalid.\n\nPlease check if they're correct.")();
            entry->Disable();
            return;
        }
        StringVector s; std::string b;
        u32 i = memStart, j = 0, l = length; u8 m;

        s.push_back(Utils::Format("MEM %08X-%08X / div%d", memStart, memStart+length, divider));
        while (l--){
            if (!j){
                if (b.length()) s.push_back(b);
                b = Utils::Format("%08X: ", i);
            }
            m=*(u8*)i++;
            j = (j + 1) % divider;
            b += Utils::Format("%02X",m);
            if (!(j&3)) b+=" ";
        }
        if (b.length()) s.push_back(b);
        
        OSD::Lock();
        Screen top = OSD::GetTopScreen();
        for (i=0; i<s.size(); i++) top.Draw(s[i],0,i*10);
        OSD::Unlock();
    }
}