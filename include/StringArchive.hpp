#ifndef STRINGARCHIVE_HPP
#define STRINGARCHIVE_HPP

#include "main.hpp"

namespace CTRPluginFramework {
    #define STRINGARCHIVE_NAMES     "JPNENGFRADEUITASPAZHSKORNEDPORRUSZHTUNK"
    #define STRINGARCHIVE_PATH      RESOURCES_PATH"/lang/%s.bin"
    #define STRINGARCHIVE_VERSION   1
    #define STRINGARCHIVE_MAGIC     *(u32*)"CY5$"
    
    enum StringArchiveEntry_Encoding {
        STRINGARCHIVE_ENCODING_ANSI = 0,
        STRINGARCHIVE_ENCODING_UTF8,
        STRINGARCHIVE_ENCODING_UTF16
    };
    
    enum StringArchiveEntry_LoadStatus {
        STRINGARCHIVE_LOAD_OKAY = 0,
        STRINGARCHIVE_LOAD_FAIL_OPEN,
        STRINGARCHIVE_LOAD_FAIL_READ,
        STRINGARCHIVE_LOAD_FAIL_SIGNATURE,
        STRINGARCHIVE_LOAD_VERSION_MISMATCH,
        STRINGARCHIVE_LOAD_NO_ENTRIES,
        STRINGARCHIVE_LOAD_TOO_MANY_ENTRIES,
        STRINGARCHIVE_LOAD_OFFSET_OOB,
        STRINGARCHIVE_LOAD_STRING_TOO_LONG,
    };
    
    typedef struct StringArchiveEntry_s {
        u32 stringID;
        char stringName[20];
        u32 fileOffset;
        u16 length;
        u16 encoding;
    } PACKED StringArchiveEntry;
   #define STRINGARCHIVEENTRY_SIZE    sizeof(StringArchiveEntry)

    typedef struct StringArchiveHeader_s {
        u32 magic;
        u32 version;
        u32 entries;
    } PACKED StringArchiveHeader;
    #define STRINGARCHIVEHEADER_SIZE    sizeof(StringArchiveHeader)
    
    class StringArchive {
    public:
        static StringArchiveEntry* entries;
        static File arcFile;
        static int entryCount;
        static int isArchiveReady;
        static int Load(u32 lang);
        static int Init(void);
        static std::string Get(u32 id);
        static std::string Get(std::string& id);
        static std::string Get(const char* id);
        static void Exit(void);
    };
}

#endif