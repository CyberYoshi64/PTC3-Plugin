#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "main.hpp"

#define CONFIG_HEADER       *(u64*)"CYX$CFG0"
#define CONFIG_VERSION      2
#define PLUGINDISCLAIMERVER 1
#define CONFIG_FILE_PATH    CONFIG_PATH"/sys.cyxcfg"

typedef struct {
    u64     magic;
    u32     version;
    u16     language;
    u16     pad1;
    struct  cyx {
        bool    enableAPI;
        bool    fontdefStrict;
    } cyx;
} PACKED Config_v1;

typedef struct {
    u64     magic;                          // File magic (see CONFIG_HEADER)
    u32     version;                        // File version (see CONFIG_VERSION)
    u16     language;                       // System language (TODO: Hook BASIC::Language field for in-app translations)
    bool    recoverFromException    : 1;    // Whether CYX is recovering from an exception or DANG()/ERROR()
    bool    wasExceptionFatal       : 1;    // Context (false=ERROR()/DANG(), true=actual exception)
    bool    clearCache              : 1;    // Whether to clear the cache folder
    bool    validateSaveData        : 1;    // Whether to check the savefs for errors
    u16     pluginDisclAgreed       : 12;   // Version of plugin disclaimer agreed to
    u64     lastExcepDumpID;                // Last exception dump ID (for Quick Dump Restore)
    struct  cyx {                           // CYX Settings
        bool    enableAPI;                  // CYX API activation status
        bool    fontdefStrict;              // Restrict/Loosen FONTDEF calls
    } cyx;
} PACKED Config_v2;

typedef Config_v2 ConfigStruct;

namespace CTRPluginFramework {
    class Config {
    public:
        static void New(void);
        static void Load(void);
        static void Save(void);
        static ConfigStruct& Get(void);
    private:
        static ConfigStruct data;
    };
}

#endif