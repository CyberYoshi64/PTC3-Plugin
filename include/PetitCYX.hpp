#ifndef PETITCYX_HPP
#define PETITCYX_HPP

#include "main.hpp"
#include "Hooks.hpp"

namespace CTRPluginFramework {


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

    #define CYX__COLORVER_NOCOLOR   1
    #define PRJSETFILEMAGIC     0x0154455325585943ULL
    #define THREADVARS_MAGIC    0x21545624 // !TV$

    using FontOffFunc = bool(*)(int c, int* y, int* x);

    typedef struct BASICTextPalette_s { // EUR @ 0x01D027CC
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

    typedef struct BASICActiveProject_s { // EUR @ 0x01B14B00
        u16 activeProject[15]; u16 currentProject[15];
        u16 unk1; u16 unk2;
    } PACKED BASICActiveProject;

    typedef struct BASICGraphicPage_s {
        void* ptr1;
        u32 unk1; u32 unk2;
        u16* dispBuf; u16* workBuf;
        void* dispBuf2; void* workBuf2; // Bad pointer though...
        u32 unk3[5]; // Actively used, avoid writing to these.
        u32 displayedFormat; // Shown GSP format, should be 0x2 (RGBA5551)
        u32 __unk__sizeX; u32 __unk__sizeY;
        float dispScaleY; float dispScaleX; // Not a standard float/double...
        u32 unkDbl1[2]; u32 unk8[5];
        u32 isResourceProtected; // Used by "Protected Resource" error
    } PACKED BASICGraphicPage;
    
    typedef struct BASICGRPStructs_s { // EUR @ 0x01D02A4C
        BASICGraphicPage grp[6];   // GRP0-GRP5
        BASICGraphicPage font;     // GRPF
        BASICGraphicPage system;   // SysUI / SysBASIC
        BASICGraphicPage sys_ctpk; // Textures from sys.ctpk
        BASICGraphicPage unk1;     // Unknown data (Test?)
    } PACKED BASICGRPStructs;

    typedef struct BASICGenericVariable_s {
        u32 type; u32 unk1;
        u32 data; void* data2;
    } PACKED BASICGenericVariable;

    typedef struct BASICProgramSlot_s {
        u16 text[1048576]; // UTF-16 content of slot
        u32 text_len;      // Length of slot content
        u32 text_og_len;   // Used to test if file is clean
        u32 chars_left;    // No. of characters left for this slot
        u16 file_name[14]; // File name (shown on OSK)
        u32 file_name_len; // Length of file name
        u32 unk1;
        u32 cursorPosition;// Used by editor to know where the cursor is
    } PACKED BASICProgramSlot;

    typedef struct BASICEditorLine_s {
        u32 offset; u32 lineLen;
        u32 lineNum; u32 always_one;
    } PACKED BASICEditorLine;

    typedef struct BASICUndoEntry_s {
        u32 offset; u32 always_zero;
        u32 len; u16 data[0x1006];
    } PACKED BASICUndoEntry;

    typedef struct BASICEditorData_s { // EUR @ 0x00F5DC9C
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
    } PACKED BASICEditorData;

    enum ProgramSlotName {
        PRGSLOT_0 = 0, PRGSLOT_1,
        PRGSLOT_2, PRGSLOT_3, PRGSLOT_MAX,
    };

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

    class CYX {
        typedef struct PTCConfig_s {
            u32 magic; // Checked for, it's SB3c
            u16 smileToolPath[0x40]; // Null-terminated UTF-16 string
            u16 activeProject[0x10]; // Null-terminated UTF-16 string
            u32 commentColor; // 0xAARRGGBB
            u32 commandColor; // 0xAARRGGBB
            u32 stringColor; // 0xAARRGGBB
            u32 labelColor; // 0xAARRGGBB
            u32 numericColor; // 0xAARRGGBB
            u32 textColor; // 0xAARRGGBB
            u32 keybDelay; // Key delay in frames
            u32 keybRepeat; // Key repeat in frames
            u32 wordWrap; // Editor word-wrap (boolean as u32)
            union {
                struct {
                    u32 something;
                    // I have to examine this area first
                    // Am unsure if this is used for something obscure
                } cyx;
                u8 pad[0x208]; // Otherwise, it's a 520-byte padding
            } extend;
            u32 blackListSize; // Number of blocked users
            u32 unk1; // Unknown
            struct {
                u32 userID; // User ID to block
                u32 unk; // Padding?
            } blockedUsers[100]; // List of blocked creators
            u32 ownUserID; // Own user ID
            u32 magic2; // Checked for, it's mCeD
            u32 functionColor; // 0xAARRGGBB
            u32 backColor; // 0xAARRGGBB
            u32 isGoldMember; // Local-only badge
            u32 spotPassEnabled; // Not directly saved but affects the tick in settings

        } PACKED PTCConfig;
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
        } PACKED ProjectSettings;
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

        static void LoadSettings(void);
        static void SaveSettings(void);
        static void ReplaceServerName(const std::string& saveURL, const std::string& loadURL);
        static void ChangeBootText(const char* text, const char* bytfre);
        static std::string GetProgramSlotFileName(u8 slot);
        static std::string PTCVersionString(u32 ver);
        static bool isPTCVersionValid(u32 ver);
        static u8 getSBVariableType(u32 rawType);
        static int scrShotStubFunc(void);
        static int controllerFuncHook(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv);
        static int stubBASICFunction(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv);

        static void MenuTick();
        static bool WouldOpenMenu();
        static void UpdateMirror();

        static s32 argGetInteger(BASICGenericVariable* arg);
        static void argGetString(string16& out, BASICGenericVariable* arg);
        static void argGetString(u16** ptr, u32* len, BASICGenericVariable* arg);
        static double argGetFloat(BASICGenericVariable* arg);

        static void CYXAPI_Out();
        static void CYXAPI_Out(s32 i);
        static void CYXAPI_Out(double f);
        static void CYXAPI_Out(const char* s);
        static void CYXAPI_Out(const std::string& s);

        static void CreateHomeFolder(const std::string& s);
        static void CreateHomeFolder();
        static std::string GetHomeFolder();
        static std::string GetHomeFolder(std::string project);

        static void LoadProjectSettings();
        static void SaveProjectSettings();

        static std::string ColorPTCVerValid(u32 ver, u32 ok, u32 ng);
        static void SetDarkMenuPalette();
        static void SetFontGetAddressStrictness(bool on);

        static std::tuple<u32, u32*, u32> soundThreadsInfo[];
	    static void playMusicAlongCTRPF(bool playMusic);
        static void SoundThreadHook();

        static u32 currentVersion;
        static BASICEditorData* editorInstance;
        static BASICActiveProject* activeProject;
        static BASICGRPStructs* GraphicPage;
        static BASICTextPalette* textPalette;
        static PTCConfig* ptcConfig;
        static std::string g_currentProject;
        static RT_HOOK clipboardFunc;
        static RT_HOOK basControllerFunc;
        static RT_HOOK scrShotStub;
        static Hook soundHook;
        static u32 cyxApiOutType;
        static string16 cyxApiTextOut;
        static double cyxApiFloatOut;
        static s32 cyxApiIntOut;
        static u16* basicFontMap;
        static u32 patch_FontGetOffset[];
        static u32 patch_FontGetOffsetNew[];
        static MirroredVars mirror;
        static FontOffFunc fontOff;
    private:
        static bool provideCYXAPI;
        static bool wasCYXAPIused;
        static char introText[];
        static char bytesFreeText[];
    };
}

#endif