#ifndef COMMIT_HASH
#define COMMIT_HASH 0x00000000
#endif
#ifndef BUILD_DATE
#define BUILD_DATE "(unknown)"
#endif

#define ENABLE_DEBUG        false
#define DBG_QKSET           true
#define TOP_DIR             "/PTC3PLG"              // Top directory (sdmc:/[TOP_DIR])
#define CONFIG_PATH         TOP_DIR "/config"       // General configurations
#define RESOURCES_PATH      TOP_DIR "/resources"    // Plugin resources
#define CACHE_PATH          TOP_DIR "/cache"        // Temporary data
#define DUMP_PATH           TOP_DIR "/dumps"        // Crash dumps
#define INCOMING_PATH       TOP_DIR "/in"           // Incoming data, such as packed project files
#define DEBUGLOG_FILE       "debug.log"             // Log file to use in debug mode

#define ROMFS_PATH          TOP_DIR "/datafs"       // Directory containing RomFS edits
#define HOMEFS_PATH         TOP_DIR "/homefs"       // Limited folder for BASIC projects using the CYX API

#define SAVEDATA_PATH       TOP_DIR "/savefs"       // In SB3, save:/config.dat
#define EXTDATA_PATH        TOP_DIR "/savefs"       // In SB3, data:/[projects]

#define PROJECTSET_PATH     CONFIG_PATH "/prjSet"   // Project-specific configurations
#define HOMEFS_SHARED_PATH  HOMEFS_PATH "/shared"   // Shared project home folder

#define CLIPBOARDCACHE_PATH CACHE_PATH "/clip.raw"

#define VERFAIL_PATH        TOP_DIR "/verificationFail.txt"  // Screenshots Path

#define SCRCAP_PATH         TOP_DIR "/screenshots"  // Screenshots Path
#define SCRCAP_FPREFIX      "CYX-"                  // Screenshot File Name Prefix

#define NUMBER_FILE_OP      9
#define VER_MAJOR           0 // Major version
#define VER_MINOR           0 // Minor version
#define VER_MICRO           7 // Micro version
#define VER_REVISION        0 // Revision
#define STRINGIFY(x)        #x
#define TOSTRING(x)         STRINGIFY(x)
#define STRING_VERSION      TOSTRING(VER_MAJOR) "." TOSTRING(VER_MINOR) "." TOSTRING(VER_MICRO) "-" TOSTRING(VER_REVISION)
#define VER_INTEGER         ((VER_MAJOR & 0xFF)<<24 | (VER_MINOR & 0xFF)<<16 | (VER_MICRO & 0xFF)<<8 | (VER_REVISION & 0xFF))

#define WRITEREMOTE32(addr, val) (*(u32 *)(PA_FROM_VA_PTR(addr)) = (val))

#define SWITCHEND32(a) ((a & 0xFF)<<24 | (a & 0xFF00)<<8 | (a & 0xFF0000)>>8 | (a & 0xFF000000)>>24)
#define SWITCHEND24(a) ((a & 0xFF)<<16 | (a & 0xFF00) | (a & 0xFF0000)>>16)
#define SWITCHEND16(a) ((a & 0xFF)<<8 | (a & 0xFF00)>>8)

#if ENABLE_DEBUG
#define	DEBUG(str, ...) {u8* cpybuf = new u8[0x300]; sprintf((char*)cpybuf, str, ##__VA_ARGS__); OnionSave::debugAppend((char*)cpybuf); delete[] cpybuf;}
#define DEBUGU16(str) {std::string out = "\""; Process::ReadString((u32)str, out, 0x200, StringFormat::Utf16); out += "\""; OnionSave::debugAppend(out);}
#else
#define	DEBUG(str, ...) {}
#define DEBUGU16(str) {}
#endif

// svcBreak() with r0-r3 set with defined arguments
extern "C" void customBreak(u32 r0, u32 r1, u32 r2, u32 r3);

#define __FILE      strrchr("/" __FILE__, '/')+1
#define __LINE      __LINE__

#define PLGFLAGS  ___pluginFlags
#define PLGSET(n) {PLGFLAGS|=(n);}
#define PLGGET(n) (PLGFLAGS&(n))
