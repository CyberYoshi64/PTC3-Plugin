#ifndef BASICAPI_HPP
#define BASICAPI_HPP

#include "main.hpp"

namespace CTRPluginFramework {
    using BasicAPIFunction = int(*)(
        BASICGenericVariable* argv, u32 argc,
        BASICGenericVariable* outv, u32 outc
    );
    #define BASICAPI_FUNCVARS       BASICGenericVariable* argv, u32 argc, BASICGenericVariable* outv, u32 outc
    #define BASICAPI_FUNCVARSPASS   argv+1, argc-1, outv, outc
    
    #define BASICAPI_HANDLE_START (u32)0x20000000U
    #define BASICAPI_HANDLE_END (u32)0x3FFFFFFFU
    #define BASICAPI_FILESTACK_LIMIT    64

    enum BasicAPI_PathType {
        APIPATHTYPE_NONE = 0,
        APIPATHTYPE_SB_SELF,
        APIPATHTYPE_SB_OTHERS,
        APIPATHTYPE_SAFE_SELF,
        APIPATHTYPE_SAFE_SHARED,
        APIPATHTYPE_SAFE_OTHERS,
        APIPATHTYPE_SDMC,
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
    
    enum BasicAPI_Funcs {
        BAPIFUNC__GETMAP    = 1337,
        BAPIFUNC_INIT       = 1000000,
        BAPIFUNC_EXIT       = 1000001,
        BAPIFUNC_CFGGET     = 1000010,
        BAPIFUNC_CFGSET     = 1000011,
        BAPIFUNC_FILES      = 1000020,
        BAPIFUNC_FOPEN      = 1000021,
        BAPIFUNC_FMODE      = 1000022,
        BAPIFUNC_FREAD      = 1000023,
        BAPIFUNC_FWRITE     = 1000024,
        BAPIFUNC_FCLOSE     = 1000025,
        BAPIFUNC_FSEEK      = 1000026,
        BAPIFUNC_VALIDATE   = 1000027,
        BAPIFUNC_CHKFILE    = 1000030,
        BAPIFUNC_MKFILE     = 1000031,
        BAPIFUNC_MVFILE     = 1000032,
        BAPIFUNC_RMFILE     = 1000033,
        BAPIFUNC_CHKDIR     = 1000040,
        BAPIFUNC_MKDIR      = 1000041,
        BAPIFUNC_MVDIR      = 1000042,
        BAPIFUNC_RMDIR      = 1000043,
        BAPIFUNC_UNIXTIME   = 1100000,
        BAPIFUNC_OSD        = 1900000,
        BAPIFUNC_CRASH      = 1900001,
        BAPIFUNC_FILLGRP    = 1900002,
        BAPIFUNC_SYSGCOPY   = 1900003,
        BAPIFUNC_SETUP_CLIP = 1900004,
    };

    enum BasicAPI_CFGAPI_ID {
        BAPICFGID__GETMAP       = 1337,
        BAPICFGID_SAFEDIR       = 1000000,
        BAPICFGID_PRJ_ACCESS    = 1000001,
        BAPICFGID_SD_ACCESS     = 1000002,
        BAPICFGID_DIRECTMODE    = 2000000,
        BAPICFGID_PTC_VER       = 2000001,
        BAPICFGID_LANG          = 2000010,
        BAPICFGID_LANGSTR       = 2000011,
        BAPICFGID_REGION        = 2000012,
        BAPICFGID_REGIONSTR     = 2000013,
        BAPICFGID_SYS_VERSTR    = 2000020,
        BAPICFGID_SYS_REGION    = 2000021,
        BAPICFGID_SYS_REGIONSTR = 2000022,
        BAPICFGID_SYS_MODEL     = 2000023,
        BAPICFGID_ISCITRA       = 2000030,
        BAPICFGID_VOLSLIDER     = 2000040,
        BAPICFGID_HEADSET       = 2000041,
        BAPICFGID_3DSLIDER      = 2000050,
        BAPICFGID_WIFILEVEL     = 2000060,
        BAPICFGID_BAT_LEVEL_RAW = 2000070,
        BAPICFGID_BAT_LEVEL     = 2000071,
        BAPICFGID_BAT_CHARGING  = 2000072,
        BAPICFGID_BAT_CHARGER   = 2000073,
        BAPICFGID_CAN_SLEEP     = 2000080,
        BAPICFGID_NETWORKSTATE  = 2000090,
        BAPICFGID_PARENTALFLAGS = 2010000,
        BAPICFGID_FIRM_VER      = 2100000,
        BAPICFGID_KERNEL_VER    = 2100001,
        BAPICFGID_CYX_VER       = 3000000,
        BAPICFGID_CYX_VERSTR    = 3000001,
        BAPICFGID_CYX_COMMIT    = 3000002,
        BAPICFGID_CYX_BUILDSTR  = 3000003,
        BAPICFGID_SDMCCLUSTER   = 4000000,
        BAPICFGID_SDMCSECTOR    = 4000001,
        BAPICFGID_SDMCFREE      = 4000002,
        BAPICFGID_SDMCFREE_C    = 4000003,
        BAPICFGID_SDMCTOTAL     = 4000004,
        BAPICFGID_SDMCTOTAL_C   = 4000005
    };

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
        static int Func_INIT(BASICAPI_FUNCVARS);
        static int Func_EXIT(BASICAPI_FUNCVARS);
        static int Func_CFGGET(BASICAPI_FUNCVARS);
        static int Func_CFGSET(BASICAPI_FUNCVARS);
        static int Func_OSD(BASICAPI_FUNCVARS);
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
        static int Func_VALIDATE(BASICAPI_FUNCVARS);
        static int Func_FILLGRP(BASICAPI_FUNCVARS);
        static int Func_SYSGCOPY(BASICAPI_FUNCVARS);
        static int Func_UNIXTIME(BASICAPI_FUNCVARS);
        static int Func_SETUP_CLIP(BASICAPI_FUNCVARS);
        static int Func_CRASH(BASICAPI_FUNCVARS);
        
        static std::vector<QueueEntry> Queue;
        static std::vector<FileStruct> Files;
        static u32 queueOffset;
        static u32 handleIDCounter;
    };
    
}

#endif