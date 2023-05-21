#ifndef BASICAPI_HPP
#define BASICAPI_HPP

#include "main.hpp"

namespace CTRPluginFramework {
    using BasicAPIFunction = int(*)(BASICGenericVariable* argv, u32 argc);
    
    #define BASICAPI_HANDLE_START (u32)0xC6400000U

    enum BasicAPI_FileOpType {
        APIFOPTYP_DONE = 0,
        APIFOPTYP_LOAD,
        APIFOPTYP_SAVE,
        APIFOPTYP_CFG,
    };

    enum BasicAPI_Flags { // Project flags
        APIFLAG_READ_SYSINFO    = BIT( 0), // Access basic info, such as system language and region
        APIFLAG_READ_FWINFO     = BIT( 1), // Access firmware version info
        APIFLAG_READ_HWINFO     = BIT( 2), // Access hardware info, such as 3D slider state, headset mode and Wi-Fi strength
        APIFLAG_FS_ACC_SAFE     = BIT(16), // Allow use of a private folder (/homefs/[project name])
        APIFLAG_FS_ACC_XREF_RO  = BIT(17), // Access files cross-project (read-only)
        APIFLAG_FS_ACC_TOP_RO   = BIT(18), // Access the entire mod folder (read-only)
        APIFLAG_FS_ACC_SD_RO    = BIT(19), // Access the entire SD Card (read-only)
        APIFLAG_FS_ACC_XREF_RW  = BIT(20), // Access files cross-project (with write permissions)
        APIFLAG_FS_ACC_TOP_RW   = BIT(21), // Access the entire mod folder (with write permissions)
        APIFLAG_FS_ACC_SD_RW    = BIT(22), // Access the entire SD Card (with write permissions)
        
        // All flags set
        APIFLAG_ADMIN = BIT(23)-1,

        APIFLAG_FS_ACCESS_XREF = (APIFLAG_FS_ACC_XREF_RO|APIFLAG_FS_ACC_XREF_RW),
        APIFLAG_FS_ACCESS_TOP = (APIFLAG_FS_ACC_TOP_RO|APIFLAG_FS_ACC_TOP_RW),
        APIFLAG_FS_ACCESS_SD = (APIFLAG_FS_ACC_SD_RO|APIFLAG_FS_ACC_SD_RW),

        // Default flag
        APIFLAG_DEFAULT     = (APIFLAG_READ_SYSINFO | APIFLAG_READ_HWINFO) 
    };

    typedef struct BasicAPI_QueueEntry_s {
        bool wasProcessed;
        u32 handleID;
        u32 returnValue;
        u32 type;
        u32 arg1;
        u32 arg2;
        std::string arg3;
        std::string arg4;
    } BasicAPI_QueueEntry;
    
    typedef struct BasicAPI_Entry_s {
        char* id;
        BasicAPIFunction func;
    } BasicAPI_Entry;
    
    class BasicAPI {
    public:
        static void Initialize(void);
        static void Finalize(void);
        static void MenuTick(void);
        static int Parse(BASICGenericVariable* argv, u32 argc);
        static u32 flags;
    private:
        
        static int Func_FILES(BASICGenericVariable* argv, u32 argc);
        static int Func_LOAD(BASICGenericVariable* argv, u32 argc);
        static int Func_SAVE(BASICGenericVariable* argv, u32 argc);
        static int Func_CFGGET(BASICGenericVariable* argv, u32 argc);
        static int Func_CFGSET(BASICGenericVariable* argv, u32 argc);
        static int Func_OSD(BASICGenericVariable* argv, u32 argc);
        static int Func_SETUP_CLIP(BASICGenericVariable* argv, u32 argc);
        
        static void AddEntry(const char* id, BasicAPIFunction func);
        static std::vector<BasicAPI_Entry> Entries;
        static std::vector<BasicAPI_QueueEntry> Queue;
        static u32 queueOffset;
        static u32 handleIDCounter;
    };
    
}

#endif