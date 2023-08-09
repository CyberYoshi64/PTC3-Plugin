#ifndef BASICAPI_HPP
#define BASICAPI_HPP

#include "main.hpp"

namespace CTRPluginFramework {
    using BasicAPIFunction = int(*)(BASICGenericVariable* argv, u32 argc);
    
    #define BASICAPI_HANDLE_START (u32)0x10000000U
    #define BASICAPI_HANDLE_END (u32)0x3FFFFFFFU
    #define BASICAPI_FILESTACK_LIMIT    32

    enum BasicAPI_PathType {
        APIPATHTYPE_NONE = 0,
        APIPATHTYPE_SB_SELF,
        APIPATHTYPE_SB_OTHERS,
        APIPATHTYPE_SAFE_SELF,
        APIPATHTYPE_SAFE_SHARED,
        APIPATHTYPE_SAFE_OTHERS,
        APIPATHTYPE_SDMC
    };

    enum BasicAPI_FileStructFlag {
        APIFSTRUCT_UTF16    = BIT( 9), // Input data is UTF-16 (Data)
        APIFSTRUCT_ANSI     = BIT(10), // Input data is ANSI (truncated UTF-16)

        APIFSTRUCT_ENCODINGS = (APIFSTRUCT_ANSI|APIFSTRUCT_UTF16)
    };

    enum BasicAPI_Flags { // Project flags
        APIFLAG_READ_SYSINFO    = BIT( 0), APIFLAG_BIT_READ_SYSINFO    =  0, // Access basic info, such as system language and region
        APIFLAG_READ_FWINFO     = BIT( 1), APIFLAG_BIT_READ_FWINFO     =  1, // Access firmware version info
        APIFLAG_READ_HWINFO     = BIT( 2), APIFLAG_BIT_READ_HWINFO     =  2, // Access hardware info, such as 3D slider state, headset mode and Wi-Fi strength
        APIFLAG_FS_ACC_SAFE     = BIT(16), APIFLAG_BIT_FS_ACC_SAFE     = 16, // Allow use of a private folder (/homefs/[project name])
        APIFLAG_FS_ACC_XREF_RO  = BIT(17), APIFLAG_BIT_FS_ACC_XREF_RO  = 17, // Access files cross-project (read-only)
        APIFLAG_FS_ACC_XREF_RW  = BIT(18), APIFLAG_BIT_FS_ACC_XREF_RW  = 18, // Access files cross-project (with write permissions)
        APIFLAG_FS_ACC_SD_RO    = BIT(19), APIFLAG_BIT_FS_ACC_SD_RO    = 19, // Access the entire SD Card (read-only)
        APIFLAG_FS_ACC_SD_RW    = BIT(20), APIFLAG_BIT_FS_ACC_SD_RW    = 20, // Access the entire SD Card (with write permissions)
        
        // All flags set
        APIFLAG_ADMIN = BIT(23)-1,

        APIFLAG_FS_ACCESS_XREF = (APIFLAG_FS_ACC_XREF_RO|APIFLAG_FS_ACC_XREF_RW),
        APIFLAG_FS_ACCESS_SD = (APIFLAG_FS_ACC_SD_RO|APIFLAG_FS_ACC_SD_RW),

        // Default flag
        APIFLAG_DEFAULT     = (APIFLAG_READ_SYSINFO | APIFLAG_READ_HWINFO)
    };

    class BasicAPI {
    typedef struct FileStruct {
        u32 handle;
        u32 flags;
        File* f;
    } FileStruct;

    typedef struct QueueEntry_s {
        bool wasProcessed;
        u32* returnValue;
        u32 type;
        u32 arg1;
        u32 arg2;
        std::string arg3;
        std::string arg4;
    } QueueEntry;
    
    typedef struct Entry_s {
        char* id;
        BasicAPIFunction func;
    } Entry;
    public:
        static void Initialize(void);
        static void Finalize(void);
        static void Cleanup(void);
        static void MenuTick(void);
        static int Parse(BASICGenericVariable* argv, u32 argc);
        static int FileRead(BasicAPI::FileStruct f, u16* buf, u32* len, u32 size, u32 bytesLeft);
        static int FileWrite(BasicAPI::FileStruct f, u16* buf, u32 size);
        static u32 flags;
    private:
        static int Func_FILES(BASICGenericVariable* argv, u32 argc);
        static int Func_FOPEN(BASICGenericVariable* argv, u32 argc);
        static int Func_FMODE(BASICGenericVariable* argv, u32 argc);
        static int Func_FREAD(BASICGenericVariable* argv, u32 argc);
        static int Func_FWRITE(BASICGenericVariable* argv, u32 argc);
        static int Func_FCLOSE(BASICGenericVariable* argv, u32 argc);
        static int Func_FSEEK(BASICGenericVariable* argv, u32 argc);
        static int Func_MKFILE(BASICGenericVariable* argv, u32 argc);
        static int Func_RMFILE(BASICGenericVariable* argv, u32 argc);
        static int Func_MVFILE(BASICGenericVariable* argv, u32 argc);
        static int Func_MKDIR(BASICGenericVariable* argv, u32 argc);
        static int Func_RMDIR(BASICGenericVariable* argv, u32 argc);
        static int Func_MVDIR(BASICGenericVariable* argv, u32 argc);
        static int Func_CHKDIR(BASICGenericVariable* argv, u32 argc);
        static int Func_CHKFILE(BASICGenericVariable* argv, u32 argc);
        static int Func_INIT(BASICGenericVariable* argv, u32 argc);
        static int Func_EXIT(BASICGenericVariable* argv, u32 argc);
        static int Func_CRASH(BASICGenericVariable* argv, u32 argc);
        static int Func_CFGGET(BASICGenericVariable* argv, u32 argc);
        static int Func_CFGSET(BASICGenericVariable* argv, u32 argc);
        static int Func_OSD(BASICGenericVariable* argv, u32 argc);
        static int Func_SETUP_CLIP(BASICGenericVariable* argv, u32 argc);
        static int Func_FILLGRP(BASICGenericVariable* argv, u32 argc);
        static int Func_SYSGCOPY(BASICGenericVariable* argv, u32 argc);
        static int Func_UNIXTIME(BASICGenericVariable* argv, u32 argc);
        
        static void AddEntry(const char* id, BasicAPIFunction func);
        static std::vector<Entry> Entries;
        static std::vector<QueueEntry> Queue;
        static std::vector<FileStruct> Files;
        static u32 queueOffset;
        static u32 handleIDCounter;
    };
    
}

#endif