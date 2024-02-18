#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "main.hpp"

#define CONFIG_HEADER       *(u64*)"CYX$CFG0"
#define CONFIG_VERSION      2
#define PLUGINDISCLAIMERVER 1
#define CONFIG_FILE_PATH    CONFIG_PATH "/sys.cyxcfg"

//// Older config versions

typedef struct {
    u64     magic;
    u32     version;
    u16     language;
    u16     pad1;
    struct  cyx {
        bool    enableAPI;
        bool    fontdefStrict;
    } cyx;
} CTR_PACKED Config_v1;

//// Latest version

typedef struct Config_s {
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
        bool    enableAPI           : 1;    // CYX API activation status
        bool    fontdefStrict       : 1;    // Restrict/Loosen FONTDEF calls
        u32     zero                : 30;
        union {
            struct {
                struct server {
                    u8      serverType;         // Server Type (see Config::Enums::CYX::ServerType)
                    u64     zero   : 24;
                    char    serverName[28];     // Custom server domain
                } CTR_PACKED server;
            } set;
            u8 raw[284];
        };
    } CTR_PACKED cyx;
} CTR_PACKED Config_v2;

//// Config functions

typedef Config_v2 ConfigStruct;
#define CONFIGS_SIZE_CYX_SET    sizeof(ConfigStruct::cyx::set)
#define CONFIGS_SIZE_CYX        sizeof(ConfigStruct::cyx)
#define CONFIGS_SIZE            sizeof(ConfigStruct)

namespace CTRPluginFramework {
    class Config {
    public:
        class Enums {
        public:
            enum ServerType {
                VANILLA = 0,        // Original server (SmileBoom)
                GENERIC,            // Custom SBServer
                STUB_TOKEN = 16,   // Stub service token
            };
        };
        static void New(void);
        static void Load(void);
        static void Save(void);
        static ConfigStruct& Get(void);
    private:
        static ConfigStruct data;
    };
}

#endif