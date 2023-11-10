#ifndef MAIN_HPP
#define MAIN_HPP

#include <CTRPluginFramework.hpp>
#include "rt.h"
#include "plgldr.h"
#include "commonFuncs.hpp"
#include <csvc.h>
#include <cmath>

#include "Config.hpp"
#include "Utils.hpp"
#include "Misc.hpp"
#include "MemDispOSD.hpp"
#include "save.hpp"
#include "PetitCYX.hpp"
#include "ExceptionHandler.hpp"
#include "CYXConfirmDlg.hpp"
#include "petitReimpl.hpp"
#include "StringArchive.hpp"

#ifndef COMMIT_HASH
#define COMMIT_HASH "00000000"
#endif
#ifndef BUILD_DATE
#define BUILD_DATE "(unknown)"
#endif

#define ENABLE_DEBUG        false
#define TOP_DIR             "/PTC3PLG" // Top directory (sdmc:/[TOP_DIR])
#define CONFIG_PATH         TOP_DIR "/config" // General configurations
#define RESOURCES_PATH      TOP_DIR "/resources" // Plugin resources
#define CACHE_PATH          TOP_DIR "/cache" // Temporary data
#define DUMP_PATH           TOP_DIR "/dumps" // Crash dumps
#define INCOMING_PATH       TOP_DIR "/in" // Incoming data, such as packed project files
#define DEBUGLOG_FILE       RESOURCES_PATH "/debug.log" // Log file to use in debug mode

#define ROMFS_PATH          TOP_DIR "/datafs" // Directory containing RomFS edits
#define HOMEFS_PATH         TOP_DIR "/homefs" // Limited folder for BASIC projects using the CYX API

// Both savedata and extdata use the same folder to be in-line with PetitCom BIG :)
#define SAVEDATA_PATH       TOP_DIR "/savefs" // In SB3, save:/config.dat
#define EXTDATA_PATH        TOP_DIR "/savefs" // In SB3, data:/[projects]

#ifdef EXTDATA_PATH_SEPERATE
#undef EXTDATA_PATH_SEPERATE
#endif

#define PROJECTSET_PATH     CONFIG_PATH "/prjSet" // Project-specific configurations
#define HOMEFS_SHARED_PATH  HOMEFS_PATH "/shared" // Shared project home folder

#define CLIPBOARDCACHE_PATH CACHE_PATH"/clip.raw"

#define NUMBER_FILE_OP      9
#define VER_MAJOR           0 // Major version
#define VER_MINOR           0 // Minor version
#define VER_MICRO           6 // Micro version
#define VER_REVISION        4 // Revision
#define STRINGIFY(x)        #x
#define TOSTRING(x)         STRINGIFY(x)
#define STRING_VERSION      TOSTRING(VER_MAJOR) "." TOSTRING(VER_MINOR) "." TOSTRING(VER_MICRO) "-" TOSTRING(VER_REVISION)
#define STRING_BUILD        BUILD_DATE "-" COMMIT_HASH
#define VER_INTEGER         ((VER_MAJOR&0xFF)<<24|(VER_MINOR&0xFF)<<16|(VER_MICRO&0xFF)<<8|(VER_REVISION&0xFF))

#define WRITEREMOTE32(addr, val) (*(u32 *)(PA_FROM_VA_PTR(addr)) = (val))

enum Region {
    REGION_NONE = 0,
    REGION_JPN,
    REGION_USA,
    REGION_EUR,
    REGION_MAX,
    REGION_MAX2
};
extern Region g_region;

enum fileSystemBits
{
    OPEN_FILE_OP,
    OPEN_DIRECTORY_OP,
    DELETE_FILE_OP,
    RENAME_FILE_OP,
    DELETE_DIRECTORY_OP,
    DELETE_DIRECTORY_RECURSIVE_OP,
    CREATE_FILE_OP,
    CREATE_DIRECTORY_OP,
    RENAME_DIRECTORY_OP
};

#define SWITCHEND32(a) ((a&0xFF)<<24|(a&0xFF00)<<8|(a&0xFF0000)>>8|(a&0xFF000000)>>24)

typedef u32(*fsRegArchiveTypeDef)(u8*, u32*, u32, u32);
typedef u32(*userFsTryOpenFileTypeDef)(u32, u16*, u32);
typedef u32(*fsMountArchiveTypeDef)(u32*, u32);

typedef u32(*fsu32u16u32)(u32, u16*, u32);
typedef u32(*fsu16)(u16*);
typedef u32(*fsu16u16)(u16*, u16*);
typedef u32(*fsu16u64)(u16*, u64);
typedef u32(*fsu32u16)(u32, u16*);

// svcBreak() with r0-r2 set with defined arguments
extern "C" void customBreak(u32 a1, u32 a2, u32 a3);

typedef struct miniHeap_s {
    u8 data[0x10][0x200];
    u8 entries[0x10];
} miniHeap;

namespace CTRPluginFramework {
    using StringVector = std::vector<std::string>;
    u32 fsRegArchiveCallback(u8* path, u32* arch, u32 isAddOnContent, u32 isAlias);
    int fsOpenArchiveFunc(u32* fsHandle, u64* out, u32 archiveID, u32 pathType, u32 pathData, u32 pathsize);
    int fsFormatSaveData(int *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, char a11);
    extern miniHeap strHeap;
    extern char g_ProcessTID[17];
    extern RT_HOOK fileOpHooks[NUMBER_FILE_OP];
    extern u32 fileOperationFuncs[NUMBER_FILE_OP];
    extern RT_HOOK regArchiveHook;
    extern u32 fsMountArchive;
    extern LightLock regLock;
    extern LightLock openLock;
    extern bool canSaveRedirect;
    extern Region g_region;
    extern vu32 ___confirmID;
    extern volatile int ___confirmRes;
    extern volatile bool ___confirmWaiting;
    extern u32 ___pluginFlags;
    extern char g_regionString[];
    extern FS_ArchiveResource g_sdmcArcRes;

    extern u32 g_osFirmVer;
    extern u32 g_osKernelVer;
    extern OS_VersionBin g_osNVer;
    extern OS_VersionBin g_osCVer;
    extern char g_osSysVer[];

    // Set the ID from SB3 to wait for a user response using the plugin
    void setCTRPFConfirm(u32 id, int defaultRes);

    // Stall the thread until a response was issued
    int waitCTRPFConfirm();

    // Enable/Disable sleep mode
    void mcuSetSleep(bool on);

    // Check, if sleep mode is enabled
    bool mcuIsSleepEnabled();
    int strlen16(u16* str);
    void OnProcessExit(void);
    int fsSetThisSaveDataSecureValue(u32 a1, u64 a2);
    int Obsoleted_5_0_fsSetSaveDataSecureValue(u64 a1, u32 a2, u32 a3, u8 a4);
    int fsSetSaveDataSecureValue(u64 a1, u32 a2, u64 a3, u8 a4);
}
#if ENABLE_DEBUG
#define	DEBUG(str, ...) {u8* cpybuf = new u8[0x300]; sprintf((char*)cpybuf, str, ##__VA_ARGS__); OnionSave::debugAppend((char*)cpybuf); delete[] cpybuf;}
#define DEBUGU16(str) {std::string out = "\""; Process::ReadString((u32)str, out, 0x200, StringFormat::Utf16); out += "\""; OnionSave::debugAppend(out);}
#else
#define	DEBUG(str, ...) {}
#define DEBUGU16(str) {}
#endif

enum PluginFlags {
    PLGFLG_EXPERIMENTS  = BIT(31), // An experiment was used
    PLGFLG_SPOOFED_VER  = BIT(30), // Version was spoofed
    PLGFLG_CYX_API      = BIT(24), // CYX API was used
    PLGFLG_SBSERVER     = BIT(23), // Server addresses were changed
    PLGFLG_PANIC        = BIT(0), // Panic
};

#define __FILE      strrchr("/" __FILE__, '/')+1
#define __LINE      __LINE__

#define PLGFLAGS  ___pluginFlags
#define PLGSET(n) {PLGFLAGS|=(n);}
#define PLGGET(n) (PLGFLAGS&(n))

#endif