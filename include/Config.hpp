#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "main.hpp"

#define CONFIG_HEADER       *(u64*)"CYX$CFG0"
#define CONFIG_VERSION      1
#define CONFIG_FILE_PATH    CONFIG_PATH"/sys.cyxcfg"

typedef struct Config_v1 {
    u64 magic;      // File magic (see CONFIG_HEADER)
    u16 version;    // File version
    u8  language;   // System language
    u8 _padding;
    struct cyx {    // CYX settings
        struct experiments { // Experiments
            u8 clipboardHook;   // Enable Clipboard Hook
            u8 cyxApi;          // Enable CYX API
        };
    };
} PACKED ConfigStruct;


namespace CTRPluginFramework {
    class Config {
    public:
        static bool Load(void);
        static bool Save(void);
        static ConfigStruct& Get(void);
    private:
        static ConfigStruct data;
    };
}

#endif