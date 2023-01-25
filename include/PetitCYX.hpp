#ifndef PETITCYX_HPP
#define PETITCYX_HPP

#include "main.hpp"
#include "Offsets.hpp"

#define CYX__COLORVER_NOCOLOR   1

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
        
        
        /*
        @brief Color wrapper for @ref CYX::isPTCVersionValid()
        @param[in] ver  Version number
        @param[in] ok   Color to return when version is valid
        @param[in] ng   Color to return when version is invalid
        @retval The color string 
        */
        static std::string ColorPTCVerValid(u32 ver, u32 ok, u32 ng);
        
        /*
        @brief Sets the colors in the menus for dark themes
        */
        static void SetDarkMenuPalette();

        /*
        @brief The version SmileBASIC originally is
        */
        static u32 currentVersion;
    };
}

#endif