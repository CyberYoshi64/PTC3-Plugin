#ifndef PETITCYX_HPP
#define PETITCYX_HPP

#include "main.hpp"
#include "Offsets.hpp"

namespace CTRPluginFramework {
    class CYX {
    public:
        static void Initialize(void);
        static void LoadSettings(void);
        static void SaveSettings(void);
        static void ReplaceServerName(std::string& saveURL, std::string& loadURL);
        static void ChangeBootText(const char* text);
        static std::string PTCVersionString(u32 ver);
        static bool isPTCVersionValid(u32 ver);
        static std::string ColorPTCVerValid(u32 ver, u32 ok, u32 ng);
        static void SetDarkMenuPalette();

        static u32 currentVersion;
    };
}

#endif