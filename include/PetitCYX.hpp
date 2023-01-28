#ifndef PETITCYX_HPP
#define PETITCYX_HPP

#include "main.hpp"
#include "Offsets.hpp"

namespace CTRPluginFramework {

    #define CYX__COLORVER_NOCOLOR   1

    typedef struct BASICActiveProject_s {
        u16 activeProject[15];
        u16 currentProject[15];
        u16 unknown1;
        u16 unknown2;
        u16 unknown3;
    } BASICActiveProject;

    typedef struct BASICProgramSlot_s {
        u16 text[1048576]; // UTF-16 content of slot
        u32 text_len; // Length of slot content
        u32 text_og_len; // Used to test if file is clean
        u32 chars_left; // No. of characters left for this slot
        u16 file_name[14]; // File name (shown on OSK)
        u32 file_name_len; // Length of file name
        u32 unknown1;
        u32 cursorPosition; // Used by editor to know where the cursor is.
    } BASICProgramSlot;

    typedef struct BASICEditorLine_s {
        u32 fileOffset;
        u32 lineLength;
        u32 lineNumber;
        u32 always_one; //?
    } BASICEditorLine;

    typedef struct BASICEditorData_s {
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
        static BASICEditorData* editorInstance;
    };
}

#endif