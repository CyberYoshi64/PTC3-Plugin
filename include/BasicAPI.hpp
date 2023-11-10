#ifndef BASICAPI_HPP
#define BASICAPI_HPP

#include "main.hpp"

namespace CTRPluginFramework {
    using BasicAPIFunction = int(*)(
        BASICGenericVariable* argv, u32 argc,
        BASICGenericVariable* outv, u32 outc
    );
    #define BASICAPI_FUNCVARS BASICGenericVariable* argv, u32 argc, BASICGenericVariable* outv, u32 outc
    
    #define BASICAPI_HANDLE_START (u32)0x20000000U
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
        static int Parse(BASICAPI_FUNCVARS);
        static int FileRead(BasicAPI::FileStruct f, u16* buf, u32* len, u32 size, u32 bytesLeft);
        static int FileWrite(BasicAPI::FileStruct f, u16* buf, u32 size);
        static u32 flags;
    private:
        static int Func_FILES(BASICAPI_FUNCVARS);
        static int Func_FOPEN(BASICAPI_FUNCVARS);
        static int Func_FMODE(BASICAPI_FUNCVARS);
        static int Func_FREAD(BASICAPI_FUNCVARS);
        static int Func_FWRITE(BASICAPI_FUNCVARS);
        static int Func_FCLOSE(BASICAPI_FUNCVARS);
        static int Func_FSEEK(BASICAPI_FUNCVARS);
        static int Func_MKFILE(BASICAPI_FUNCVARS);
        static int Func_RMFILE(BASICAPI_FUNCVARS);
        static int Func_MVFILE(BASICAPI_FUNCVARS);
        static int Func_MKDIR(BASICAPI_FUNCVARS);
        static int Func_RMDIR(BASICAPI_FUNCVARS);
        static int Func_MVDIR(BASICAPI_FUNCVARS);
        static int Func_CHKDIR(BASICAPI_FUNCVARS);
        static int Func_CHKFILE(BASICAPI_FUNCVARS);
        static int Func_INIT(BASICAPI_FUNCVARS);
        static int Func_EXIT(BASICAPI_FUNCVARS);
        static int Func_CRASH(BASICAPI_FUNCVARS);
        static int Func_CFGGET(BASICAPI_FUNCVARS);
        static int Func_CFGSET(BASICAPI_FUNCVARS);
        static int Func_OSD(BASICAPI_FUNCVARS);
        static int Func_SETUP_CLIP(BASICAPI_FUNCVARS);
        static int Func_FILLGRP(BASICAPI_FUNCVARS);
        static int Func_SYSGCOPY(BASICAPI_FUNCVARS);
        static int Func_UNIXTIME(BASICAPI_FUNCVARS);
        static int Func_VALIDATE(BASICAPI_FUNCVARS);
        
        static void AddEntry(const char* id, BasicAPIFunction func);
        static std::vector<Entry> Entries;
        static std::vector<QueueEntry> Queue;
        static std::vector<FileStruct> Files;
        static u32 queueOffset;
        static u32 handleIDCounter;
    };
    
}

#endif