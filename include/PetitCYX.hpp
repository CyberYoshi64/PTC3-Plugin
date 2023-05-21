#ifndef PETITCYX_HPP
#define PETITCYX_HPP

#include "main.hpp"
#include "Offsets.hpp"

namespace CTRPluginFramework {

    #define CYX__COLORVER_NOCOLOR   1
    #define THREADVARS_MAGIC  0x21545624 // !TV$

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
    public:
        static void Initialize(void);
        static void Finalize(void);
        static void UTF16toUTF8(std::string& out, u16* str);
        static void UTF16toUTF8(std::string& out, u16* str, u32 len);
        static void SetAPIClipboardAvailability(bool enabled);
        static bool GetAPIClipboardAvailability();
        static void DiscardAPIUse();
        static void SetAPIUse(bool enabled);
        static bool WasClipAPIUsed();
        static void LoadSettings(void);
        static void SaveSettings(void);
        static void ReplaceServerName(std::string& saveURL, std::string& loadURL);
        static void ChangeBootText(const char* text, const char* bytfre);
        static std::string GetProgramSlotFileName(u8 slot);
        static std::string PTCVersionString(u32 ver);
        static bool isPTCVersionValid(u32 ver);
        static u8 getSBVariableType(u32 rawType);
        static int scrShotStubFunc(void);
        static int controllerFuncHook(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv);
        static int stubBASICFunction(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u32 outc, void* a4, u32 argc, BASICGenericVariable* argv);

        static s32 argGetInteger(BASICGenericVariable* arg);
        static void argGetString(string16& out, BASICGenericVariable* arg);
        static double argGetFloat(BASICGenericVariable* arg);

        static void CYXAPI_Out(s32 i);
        static void CYXAPI_Out(double f);
        static void CYXAPI_Out(const char* s);
        static void CYXAPI_Out(const std::string& s);

        static std::string ColorPTCVerValid(u32 ver, u32 ok, u32 ng);
        static void SetDarkMenuPalette();

        static std::tuple<u32, u32*, u32> soundThreadsInfo[];
	    static void playMusicAlongCTRPF(bool playMusic);
        static void SoundThreadHook();

        static u32 currentVersion;
        static BASICEditorData* editorInstance;
        static BASICActiveProject* activeProject;
        static BASICGRPStructs* GraphicPage;
        static BASICTextPalette* textPalette;
        static RT_HOOK clipboardFunc;
        static RT_HOOK basControllerFunc;
        static RT_HOOK scrShotStub;
        static Hook soundHook;
        static u32 cyxApiOutType;
        static string16 cyxApiTextOut;
        static double cyxApiFloatOut;
        static s32 cyxApiIntOut;
    private:
        static bool provideClipAPI;
        static bool wasClipAPIused;
        static char introText[];
        static char bytesFreeText[];
    };
}

#endif