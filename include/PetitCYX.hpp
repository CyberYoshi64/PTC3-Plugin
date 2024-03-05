#ifndef PETITCYX_HPP
#define PETITCYX_HPP

#include "CTRPluginFramework.hpp"
#include "constants/main.h"
#include "constants/ptcImpl.h"
#include "Hooks.hpp"
#include "ExceptionHandler.hpp"
#include "Config.hpp"
#include "petitReimpl.hpp"
#include <stdio.h>

#define SBSERVER_URL_MAXLEN		27

#define PTC_WORKSPACE_EXTDATANAME   "###"
#define PTC_WORKSPACE_CYXNAME       "$DEFAULT"

#define CYX__COLORVER_NOCOLOR   1
#define PRJSETFILEMAGIC     0x0154455325585943ULL
#define THREADVARS_MAGIC    0x21545624 // !TV$

enum BasicAPI_Flags { // Project flags
    APIFLAG_NONE            = 0,
    APIFLAG_FS_ACC_SAFE     = BIT(16), APIFLAG_BIT_FS_ACC_SAFE     = 16, // Allow use of a private folder (/homefs/[project name])
    APIFLAG_FS_ACC_XREF_RO  = BIT(17), APIFLAG_BIT_FS_ACC_XREF_RO  = 17, // Access files cross-project (read-only)
    APIFLAG_FS_ACC_XREF_RW  = BIT(18), APIFLAG_BIT_FS_ACC_XREF_RW  = 18, // Access files cross-project (with write permissions)
    APIFLAG_FS_ACC_SD_RO    = BIT(19), APIFLAG_BIT_FS_ACC_SD_RO    = 19, // Access the entire SD Card (read-only)
    APIFLAG_FS_ACC_SD_RW    = BIT(20), APIFLAG_BIT_FS_ACC_SD_RW    = 20, // Access the entire SD Card (with write permissions)
    APIFLAG_ALLOW_TOGGLE    = BIT(23), APIFLAG_BIT_ALLOW_TOGGLE    = 23, // Allow BASIC programs to toggle the project flags with CFGSET
    
    // All flags set
    APIFLAG_ADMIN = BIT(24)-1,

    APIFLAG_FS_ACCESS_XREF = (APIFLAG_FS_ACC_XREF_RO|APIFLAG_FS_ACC_XREF_RW),   // Flags depicting permission to other project files
    APIFLAG_FS_ACCESS_SD = (APIFLAG_FS_ACC_SD_RO|APIFLAG_FS_ACC_SD_RW),         // Flags depicting permission to SD Card access

    // Default flag
    APIFLAG_DEFAULT     = APIFLAG_ALLOW_TOGGLE
};

namespace CTRPluginFramework {
    using FontOffFunc = bool(*)(int c, int* y, int* x);
    using PrintCharFunc = int(*)(ptcConsole* con, u16 chr, u32 type);
    class CYX {
        typedef struct MirroredVars {
            u8 isDirectMode;
            u8 isInBasic;
            u8 isBasicRunning;
            u8 isBasicRunning2;
            u16 currentProject[15];
            struct diff {
                bool isDirectMode;
                bool isInBasic;
                bool isBasicRunning;
                bool currentProject;
            } diff;
            u32 isBasicRunningTime;
        } MirroredVars;
        
        typedef struct ProjectSettings {
            u64 magic;
            u32 apiFlags;
        } CTR_PACKED ProjectSettings;
    public:

        /**
         * @brief Initialize plugin hooks and structure references and loads settings
         */
        static Result Initialize(void);

        /**
         * @brief Remove plugin hooks and structure references and saves settings
         */
        static void Finalize(void);

        /**
         * @brief Wrapper for buffered strings
         */
        static void UTF16toUTF8(std::string& out, u16* str);
        static void UTF16toUTF8(std::string& out, u16* str, u32 len);
        
        /**
         * @brief Enable/Disable CYX API
         * @param[in] enabled Whether to enable or not
         */
        static void SetAPIAvailability(bool enabled);

        /**
         * @brief Gets CYX API status
         * @return true if API is available, false otherwise
         */
        static bool GetAPIAvailability();

        /**
         * @brief [Internal] Removes the API use flag
         */
        static void DiscardAPIUse();

        /**
         * @brief [Internal] Set the API Use flag
         * @param[in] enabled Whether to enable or not
         */
        static void SetAPIUse(bool enabled);

        /**
         * @brief [Internal] Get the API Use flag
         * @return true if API was used, false otherwise
         */
        static bool WasCYXAPIUsed();

        /**
         * @brief Replace server names
         * 
         * @note
         * Custom servers very likely only use a single domain, but change my mind.
         * 
         * @param saveURL Replacement for save.smilebasic.com
         * @param loadURL Replacement for load.smilebasic.com
         */
        static void ReplaceServerName(const std::string& saveURL, const std::string& loadURL);

        /**
         * @brief Change SmileBASIC Direct Mode boot text
         * 
         * @param text Header text
         * @param bytfre Text after the free byte count
         */
        static void ChangeBootText(const char* text, const char* bytfre);

        /**
         * @brief Get the File Name of a Program Slot
         * 
         * @param slot PRG slot (0 - 3)
         * @return File name (blank if no file is loaded)
         */
        static std::string GetProgramSlotFileName(u8 slot);

        /**
         * @brief Format SmileBASIC version as string
         * 
         * @param ver Version integer
         * @return Formatted string
         */
        static std::string PTCVersionString(u32 ver);

        /**
         * @brief Validate SmileBASIC version
         * 
         * @param ver Version integer
         * @return `true`, if valid. `false` otherwise. 
         */
        static bool isPTCVersionValid(u32 ver);
        
        static u8 getSBVariableType(u32 rawType);
        
        /// @brief Key Send Hook - used to filter out screenshot
        static void sendKeyHookFunc(u32* ptr1, u32 key);

        /// @brief CONTROLLER stub - implements CYX API beside the main functionality
        static int controllerFuncHook(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv);
        static int stubBASICFunction(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv);

        static void TrySave();
        static void MenuTick();
        static bool WouldOpenMenu();
        static void UpdateMirror();

        static s32 argGetInteger(BASICGenericVariable* arg);
        static void argGetString(string16& out, BASICGenericVariable* arg);
        static void argGetString(u16** ptr, u32* len, BASICGenericVariable* arg);
        static double argGetFloat(BASICGenericVariable* arg);
        static u32 apiGetFlags(void);
        static void apiToggleFlag(u32 flag);
        static void apiEnableFlag(u32 flag);
        static void apiDisableFlag(u32 flag);

        static void CYXAPI_Out(BASICGenericVariable* out);
        static void CYXAPI_Out(BASICGenericVariable* out, s32 i);
        static void CYXAPI_Out(BASICGenericVariable* out, double f);
        static void CYXAPI_Out(BASICGenericVariable* out, const char* s);
        static void CYXAPI_Out(BASICGenericVariable* out, const std::string& s);

        static void CreateHomeFolder(const std::string& s);
        static void CreateHomeFolder();
        static std::string GetExtDataFolderName();
        static std::string GetHomeFolder();
        static std::string GetHomeFolder(std::string project);

        static void LoadProjectSettings();
        static void SaveProjectSettings();

        static std::string ColorPTCVerValid(u32 ver, u32 ok, u32 ng);
        static void SetDarkMenuPalette();
        static void SetFontGetAddressStrictness(bool on);
        static void RestoreRescueDump(const std::string& path);

        static std::tuple<u32, u32*, u32> soundThreadsInfo[];
	    static void playMusicAlongCTRPF(bool playMusic);
        static void SoundThreadHook();
        static void SoundThreadHook2();
        static void ResetServerLoginState();
        static int petcTokenHookFunc();
        static int nnActIsNetworkAccountStub();
        static void ptcMainEntryHookFunc(u32 r0);
        static void nnExitHookFunc(void);
        static void ValidateSaveData();

        static u32 currentVersion;
        static BASICEditorData* editorInstance;     // Editor instance
        static BASICActiveProject* activeProject;   // Active Project strings
        static BASICGRPStructs* GraphicPage;        // Graphic pages
        static BASICTextPalette* textPalette;       // Text console palette
        static PTCConfig* ptcConfig;                // BASIC config struct
        static PTCUDS* ptcUds;                      // Multiplayer Struct
        static ptcScreen* ptcScreens;               // Screens
        static std::string g_currentProject;
        static RT_HOOK clipboardFunc;
        static RT_HOOK basControllerFunc;
        static Hook sendKeyHook;
        static Hook mainThreadEntryHook;            // MitM hook for main entry
        static Hook nnExitHook;               // Replace nnWebbrsTick
        static Hook soundHook;
        static Hook soundHook2;
        static bool forceDisableSndOnPause;
        static string16 cyxApiTextOut;
        static u32 cyxApiOutc, cyxApiLastOutv;
        static u16* basicFontMap;                   // BASIC font map
        static u32 patch_FontGetOffset[];
        static u32 patch_FontGetOffsetNew[];
        static MirroredVars mirror;
        static FontOffFunc fontOff;                 // FONTDEF remap function
        static PrintCharFunc printConFunc;          // Function to plot glyphs to the console
        static u64 sdmcFreeSpace;
        static u64 sdmcTotalSpace;
        static s32 volumeSliderValue;
        static s32 rawBatteryLevel;
        static u8 mcuSleep;
        static u64 askQuickRestore;
        static bool wouldExit;
        static std::string exitMessage;
        static u32 helpPageColors[];
        static RT_HOOK petcServiceTokenHook;        // Hook for nn::act::GetIndependentServiceToken()
        static RT_HOOK nnActIsNetworkAccountHook;   // Hook for nn::act::IsNetworkAccount()
    private:
        static bool provideCYXAPI;
        static bool wasCYXAPIused;
        static u32 cyxSaveTimer;
        static u32 cyxUpdateSDMCStats;
        static char introText[];
        static char bytesFreeText[];
    };
}

#endif