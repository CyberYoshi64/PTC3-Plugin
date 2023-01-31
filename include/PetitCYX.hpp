#ifndef PETITCYX_HPP
#define PETITCYX_HPP

#include "main.hpp"
#include "Offsets.hpp"

namespace CTRPluginFramework {

    #define CYX__COLORVER_NOCOLOR   1

    typedef struct BASICActiveProject_s {
        u16 activeProject[15];
        u16 currentProject[15];
        u16 unk1;
        u16 unk2;
        u16 unk3;
    } BASICActiveProject;

    typedef struct BASICProgramSlot_s {
        u16 text[1048576]; // UTF-16 content of slot
        u32 text_len; // Length of slot content
        u32 text_og_len; // Used to test if file is clean
        u32 chars_left; // No. of characters left for this slot
        u16 file_name[14]; // File name (shown on OSK)
        u32 file_name_len; // Length of file name
        u32 unk1;
        u32 cursorPosition; // Used by editor to know where the cursor is.
    } BASICProgramSlot;

    typedef struct BASICGenericVariable_s {
        u32 type;
        u32 unk1;
        u32 data;
        void* data2;
    } BASICGenericVariable;

    typedef struct BASICEditorLine_s {
        u32 fileOffset;
        u32 lineLength;
        u32 lineNumber;
        u32 always_one; //?
    } BASICEditorLine;

    typedef struct BASICEditorData_s {
        u16 clipboardData[1048576];
        u32 clipboardLength;
        u32 unk1;
        void* ptr1;
        u32 cursorRaw;
        u32 cursorLineCount;
        u32 cursorLineOffset;
        BASICProgramSlot programSlot[4];
        u32 editorHeight;
        u32 editorWidth;
        BASICEditorLine lines[30];
    } BASICEditorData;

    enum ProgramSlotName {
        PRGSLOT_0 = 0,
        PRGSLOT_1,
        PRGSLOT_2,
        PRGSLOT_3,
        PRGSLOT_MAX,
    };

    #define SBVARRAW_INTEGER    0x305654
    #define SBVARRAW_DOUBLE     0x3056B8
    #define SBVARRAW_STRING     0x30571C
    #define SBVARRAW_INTARRAY   0x305780
    #define SBVARRAW_ARRAY      0x3057E4
    #define SBVARRAW_NULL       0x305910

    enum SBVariableTypes {
        VARTYPE_NONE = 0,
        VARTYPE_INT,
        VARTYPE_DOUBLE,
        VARTYPE_STRING,
        VARTYPE_INTARRAY,
        VARTYPE_DBLARRAY,
        VARTYPE_STRARRAY,
    };

    class CYX {
    public:
        static void Initialize(void);
        static void UTF16toUTF8(std::string& out, u16* str, u32 len);
        static void SetAPIClipboardAvailability(bool enabled);
        static bool GetAPIClipboardAvailability();
        static void DiscardAPIUse();
        static bool WasClipAPIUsed();
        static void LoadSettings(void);
        static void SaveSettings(void);
        static void ReplaceServerName(std::string& saveURL, std::string& loadURL);
        static void ChangeBootText(const char* text);
        static std::string PTCVersionString(u32 ver);
        static bool isPTCVersionValid(u32 ver);
        static int getSBVariableType(u32 rawType);
        static int scrShotStubFunc(void);
        static int clipboardFuncHook(void* ptr, u32 selfPtr, BASICGenericVariable* outv, u8 outc, void* a4, u8 argc, BASICGenericVariable* argv);

        static std::string ColorPTCVerValid(u32 ver, u32 ok, u32 ng);
        static void SetDarkMenuPalette();
        static int ParseClipAPI(std::string& data);

        static u32 currentVersion;
        static BASICEditorData* editorInstance;
        static RT_HOOK clipboardFunc;
        static RT_HOOK scrShotStub;
    private:
        static bool provideClipAPI;
        static bool wasClipAPIused;
    };
}

#endif