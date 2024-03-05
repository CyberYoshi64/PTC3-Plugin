#ifndef MAIN_HPP
#define MAIN_HPP

#include <CTRPluginFramework.hpp>
#include "constants/main.h"
#include "commonFuncs.hpp"
#include "csvc.h"
#include "rt.h"
#include "plgldr.h"
#include "Utils.hpp"
#include "Misc.hpp"
#include "CYXConfirmDlg.hpp"
#include "StringArchive.hpp"
#include "nncy/ParentalControl.h"

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

typedef u32(*fsRegArchiveTypeDef)(u8*, u32*, u32, u32);
typedef u32(*userFsTryOpenFileTypeDef)(u32, u16*, u32);
typedef u32(*fsMountArchiveTypeDef)(u32*, u32);

typedef u32(*fsu32u16u32)(u32, u16*, u32);
typedef u32(*fsu16)(u16*);
typedef u32(*fsu16u16)(u16*, u16*);
typedef u32(*fsu16u64)(u16*, u64);
typedef u32(*fsu32u16)(u32, u16*);

typedef struct miniHeap_s {
    u8 data[0x10][0x200];
    u8 entries[0x10];
} miniHeap;

extern const char* pluginSerial;

namespace CTRPluginFramework {
    using StringVector = std::vector<std::string>;
    using u32Vector = std::vector<u32>;

    extern bool isCYXenabled;

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
    extern u8 g_systemModel;
    extern u8 g_systemRegion;
    extern u32 parentalControlFlag;
    extern char g_systemRegionString[];
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

enum PluginFlags {
    PLGFLG_EXPERIMENTS  = BIT(31), // An experiment was used
    PLGFLG_SPOOFED_VER  = BIT(30), // Version was spoofed
    PLGFLG_CYX_API      = BIT(24), // CYX API was used
    PLGFLG_SBSERVER     = BIT(23), // Server addresses were changed
    PLGFLG_PANIC        = BIT(0), // Panic
};

#endif