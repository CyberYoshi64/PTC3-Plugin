#ifndef PETITCYX_HPP
#define PETITCYX_HPP

#include "main.hpp"
#include "Hooks.hpp"
#include "petitReimpl.hpp"
#include <stdio.h>

#define SBSERVER_DEFAULT_NAME1	"https://save.smilebasic.com"
#define SBSERVER_DEFAULT_NAME2	"https://load.smilebasic.com"

#define SBSERVER_SAVE_SAVE3			"/save3.php"
#define SBSERVER_SAVE_SHOW2			"/show2.php"
#define SBSERVER_SAVE_DELETE2		"/delete2.php"
#define SBSERVER_SAVE_SHOPLIST2		"/shoplist2.php"
#define SBSERVER_SAVE_PREPURCHASE2	"/prepurchase2.php"
#define SBSERVER_SAVE_PURCHASE2		"/purchase2.php"
#define SBSERVER_SAVE_LIST2			"/list2.php"
#define SBSERVER_LOAD_LOAD2			"/load2.php"
#define SBSERVER_LOAD_INFO2			"/info2.php"

#define SBSERVER_URL_MAXLEN		27

#define PTC_WORKSPACE_EXTDATANAME   "###"
#define PTC_WORKSPACE_CYXNAME       "$DEFAULT"

#define CYX__COLORVER_NOCOLOR   1
#define PRJSETFILEMAGIC     0x0154455325585943ULL
#define THREADVARS_MAGIC    0x21545624 // !TV$

typedef struct {
    u32 magic;          // Checked for, it's "SB3c"
    u16 smileToolPath[0x40]; // Null-terminated UTF-16 string
    u16 activeProject[0x10]; // Null-terminated UTF-16 string
    u32 commentColor;   // 0xAARRGGBB
    u32 commandColor;   // 0xAARRGGBB
    u32 stringColor;    // 0xAARRGGBB
    u32 labelColor;     // 0xAARRGGBB
    u32 numericColor;   // 0xAARRGGBB
    u32 textColor;      // 0xAARRGGBB
    u32 keybDelay;      // Key delay in frames
    u32 keybRepeat;     // Key repeat in frames
    u32 wordWrap;       // Editor word-wrap (boolean as u32)
    u8 pad[0x208];      // Unused / Padding?
    u32 blackListSize;  // Number of blocked users
    u32 unk1;           // Unknown
    struct {            // List of blocked creators
        u32 userID;         // User ID to block
        u32 unk;            // Padding?
    } blockedUsers[100];
    u32 ownUserID;      // Own user ID
    u32 magic2;         // Checked for, it's "DeCm"
    u32 functionColor;  // 0xAARRGGBB
    u32 backColor;      // 0xAARRGGBB
    u32 isGoldMember;   // Local-only badge
    u32 spotPassEnabled;// Not meaningful - visual only
} CTR_PACKED PTCConfig;

typedef struct {
    u32 state;
    int isPreparing;
    u32 reserved1;
    u32 playerCount;
    u16 passPhrase[16];
    u32 passPhraseSize;
    u32 reserved2;
    s32 hostID;
} PTCUDS;
// 0x01D0DEB4

enum PTCFileType : u16 {
    PTCFILETYPE_TXT = 0,
    PTCFILETYPE_DAT = 1,
    PTCFILETYPE_PRJ = 2,
};

enum PTCFileBitMask1 : u16 {
    PTCFILEBM_NONE = 0,             // No flag set
    PTCFILEBM_ZLIB = BIT(1),        // File is compressed with zLib
    PTCFILEBM_CLASSIC_IP = BIT(2),  // File is copy-protected (Classic IP)
};

typedef struct {
    u16 version;
    PTCFileType type;
    u16 bitmask1; // Refer to PTCFileBitMask1
    u16 icon;
    s32 dataSize;
    struct {
        s16 year;
        s8 month;
        s8 day;
        s8 hour;
        s8 minute;
        s8 second;
        s8 weekDay;
    } modDate;
    char creatorName[16];
    char uploaderName[16];
    u32 creatorUID;
    u32 uploaderUID;
    u64 creatorPID;
    u64 uploaderPID;
} PTCFileHeader;

typedef struct {
    char sign[4];       // should be "PCBN"
    char version[4];    // should be "0001"
    u16 contentType;    // s8/u8/s16/u16/s32/f64
    u16 dimCount;
    u32 dimensions[4];
} PTCBinaryHeader;

typedef struct {
    u32 dataSize;
    u32 fileCount;
} PTCPackedProjectHeader;

typedef struct {
    u32 size;
    char name[16];
} PTCPackedProjectEntry;

typedef struct { // EUR 3.6.0 @ 0x01D027CC
    u32 conClear; u32 conBlack;
    u32 conMaroon; u32 conRed;
    u32 conGreen; u32 conLime;
    u32 conOlive; u32 conYellow;
    u32 conNavy; u32 conBlue;
    u32 conPurple; u32 conMagenta;
    u32 conTeal; u32 conCyan;
    u32 conGray; u32 conWhite;
    u32 editComment; u32 editString;
    u32 editLabel; u32 editNumeric;
    u32 editKeywords; u32 editText;
    u32 editRuler; u32 editRulerSel;
    u32 editStatement; u32 editSelectFG;
} BASICTextPalette;

typedef struct { // EUR 3.6.0 @ 0x01B14B00
    u16 activeProject[15]; u16 currentProject[15];
    u16 unk1; u16 unk2;
} CTR_PACKED BASICActiveProject;

typedef struct {
    void* ptr1;
    u32 unk1; u32 unk2;
    u16* dispBuf; u16* workBuf;
    void* dispBuf2; void* workBuf2; // Bad pointer though...
    u32 unk3[5]; // Actively used, avoid writing to these.
    u32 displayedFormat; // Shown GSP format, should be 0x2 (RGBA5551)
    u32 __unk__sizeX; u32 __unk__sizeY;
    float dispScaleY; float dispScaleX; // Not a standard float/double...
    double unkDbl1; u32 unk8[5];
    u32 isResourceProtected; // Used by "Protected Resource" error
} CTR_PACKED BASICGraphicPage;

typedef struct { // EUR 3.6.0 @ 0x01D02A4C
    BASICGraphicPage grp[6];   // GRP0-GRP5
    BASICGraphicPage font;     // GRPF
    BASICGraphicPage system;   // SysUI / SysBASIC
    BASICGraphicPage sys_ctpk; // Textures from sys.ctpk
    BASICGraphicPage unk1;     // Unknown data (Test?)
} CTR_PACKED BASICGRPStructs;

typedef struct {
    u32 type; u32 unk1;
    u32 data; void* data2;
} BASICGenericVariable;

typedef struct {
    BASICGenericVariable* argc;
    u32 argv;
    BASICGenericVariable* outv;
    u32 outc;
    u32 unk[4];
    u32 stringPtr;
} BASICFunctionStack;

typedef struct {
    u16 text[1048576]; // UTF-16 content of slot
    u32 text_len;      // Length of slot content
    u32 text_og_len;   // Used to test if file is clean
    u32 chars_left;    // No. of characters left for this slot
    u16 file_name[14]; // File name (shown on OSK)
    u32 file_name_len; // Length of file name
    u32 unk1;
    u32 cursorPosition;// Used by editor to know where the cursor is
} CTR_PACKED BASICProgramSlot;

typedef struct {
    u32 offset; u32 lineLen;
    u32 lineNum; u32 always_one;
} CTR_PACKED BASICEditorLine;

typedef struct {
    u32 offset; u32 always_zero;
    u32 len; u16 data[0x1006];
} CTR_PACKED BASICUndoEntry;

typedef struct { // EUR @ 0x00F5DC9C
    u16 clipboardData[0x100000];
    u32 clipboardLength;
    void* ptr1;
    u32 cursorRaw;
    u32 cursorLineCount;
    u32 cursorLineOffset;
    BASICProgramSlot programSlot[4];
    u32 editorHeight;
    u32 editorWidth;
    BASICEditorLine editorLine[120];
    u32 lastShownLineOff;
    u32 EOFdistance;
    BASICUndoEntry undo[8];
    u32 undoIteration;
    u32 undoTotal;
    u32 undoCount;
} CTR_PACKED BASICEditorData;

typedef struct { // 0x31B570
    u8 appletType;
    u8 unk01;
    u8 unk02;
    u8 unk03;
    u8 unk04;
    u8 toHome01; // Not refreshed
    u8 quitApplication01;
    u8 unk07;
    u8 unk08;
    u8 quitApplication02; // used for nn::applet::WaitForStarting
    u8 unk0a;
    u8 unk0b;
    u8 unk0c;
    u8 toHome02;
} appletFlag;

enum ptcConsoleProp {
    PTCCON_OFF_FRONTCLR = 0,
    PTCCON_OFF_BACKCLR = 5,
    PTCCON_OFF_ATTR = 10,
    
    PTCCON_FLS_FRONTCLR = 0x1F,
    PTCCON_FLS_BACKCLR = 0x1F,
    PTCCON_FLS_ATTR = 0x0F,
};

#define PTCCON_MKPROP(fc,bc,at) ( \
    (fc & PTCCON_FLS_FRONTCLR)<<PTCCON_OFF_FRONTCLR | \
    (bc & PTCCON_FLS_BACKCLR)<<PTCCON_OFF_BACKCLR | \
    (at & PTCCON_FLS_ATTR)<<PTCCON_OFF_ATTR \
)

typedef struct {
    struct {
        u16 glyph;  // UTF-16 glyph
        u16 prop;   // Bitfield containing color & ATTR
        float z;    // z-Offset
    } slot[1500];
} ptcConsoleTiles;

typedef struct { // 0x315D74
    void* unk_0000;
    u32 unk_0004;
    u32 unk_0008;
    u32 unk_000c;
    void* unk_0010;
    void* unk_0014;
    u32 unk_0018;
    ptcConsoleTiles* tiles;
    u32 width; // Console width?
    u32 height; // Console height?
    u32 width2; // Console width?
    u32 height2; // Console height?
    u32 unk_0030;
    u32 fgColor;
    u32 bgColor;
    u32 attr;
    u32 csrx; // Horizontal cursor position (CSRX)
    u32 csry; // Vertical cursor position (CSRY)
    float csrz; // Cursor depth (CSRZ)
    u32 unk_004c;
    u32 unk_0050;
    u32 unk_0054;
    u32 unk_0058; // Vertical cursor position (input)
    void* unk_005c;
    u32 inputBufSize;
    u32 inputCursorPos;
    u32 unk_0068;
    u32 inputSelEnabledPos;
    u16 inputBuf[256];
    u32 unk_0270;
    u32 unk_0274;
    u32 unk_0278[128];
    u16 suggestion_buf[256][32];
    u32 suggestion_len[32];
    u32 suggestion_next; // Next offset to overwrite
    u32 suggestion_count; // If < 32, add, otherwise replace
    u32 unk_4504;
    u32 unk_4508;
    char general_buf[64]; // Used for error messages and FILES
    u32 unk_454c;
} ptcConsole;

typedef struct {
    void* ptr00;
    ptcConsole* console;
    u32 height;
    u32 width;
    void* unk10;
    void* unk14;
    BASICGraphicPage* dispGrpBuf;
    BASICGraphicPage* workGrpBuf;
    void* unk20;
    void* unk24;
    void* unk28;
    void* unk2c;
    u32 dispGrpIndex;
    u32 workGrpIndex;
    void* spritePtr;
    void* unk3c;
    void* unk40;
    void* unk44;
    u32 spriteCount;
    void* unk4c;
    void* bgPtr;
    void* unk54;
    void* unk58;
    void* unk5c;
    u32 bgCount;
    void* unk64;
    void* unk68;
    void* unk6c;
    u32 fadeColor;
    u32 fadeDuration;
    u32 fadeTimer;
    u32 fadeSrcColor;
    u32 fadeTargetColor;
    bool visibleCon;
    bool visibleGrp;
    bool visibleSp;
    bool visibleBg;
} ptcScreen;

#define SBVARRAW_INTEGER    0x305654
#define SBVARRAW_DOUBLE     0x3056B8
#define SBVARRAW_STRING     0x30571C
#define SBVARRAW_ARRAY      0x305780
#define SBVARRAW_NUMARRAY   0x3057E4
#define SBVARRAW_NULL       0x305910

enum SBVariableTypes {
    VARTYPE_NONE = 0, VARTYPE_INT,
    VARTYPE_DOUBLE, VARTYPE_STRING,
    VARTYPE_INTARRAY, VARTYPE_DBLARRAY,
    VARTYPE_STRARRAY,
};

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