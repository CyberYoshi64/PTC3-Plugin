#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "main.hpp"

#define CONFIG_HEADER       *(u64*)"CYX$CFG0"
#define CONFIG_VERSION      1
#define CONFIG_FILE_PATH    CONFIG_PATH"/sys.cyxcfg"

typedef struct Config_v1 {
    u64 magic;      // File magic (see CONFIG_HEADER)
    u32 version;    // File version
    u16 language;   // System language
    u16 pad1;
    struct {    // CYX settings
        bool enableAPI;
        bool fontdefStrict;
    } cyx;
} PACKED ConfigStruct;

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