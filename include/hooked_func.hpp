#include "main.hpp"
#include "save.hpp"
#include "rt.h"

namespace CTRPluginFramework {
    int strlen16(u16* str);
    u16* GetBuffer(bool secondary = false);
    static void concatFileName(u16* dest, u16* s1, u16* s2);
    static inline u16* skipArchive(u16* src) {
        while (*src++ != u'/'); //Skip the archive lowpath
        while (*src == u'/') ++src; // Skip any remaining  /
        return src - 1; //Return the position of the last /
    }
    void strcpy16(u16* dst, u16* src);
    u16* calculateNewPath(u16* initialPath, bool isReadOnly, bool isSecondary = false, bool* shouldReopen = nullptr);
    u32 fsOpenFileFunc(u32 a1, u16* path, u32 a2);
    int checkFileExistsWithDir(u16* path);
    u32 fsOpenDirectoryFunc(u32 a1, u16* path);
    u32 fsDeleteFileFunc(u16* path);
    u32 fsRenameFileFunc(u16* path1, u16* path2);
    u32 fsDeleteDirectoryFunc(u16* path);
    u32 fsDeleteDirectoryRecFunc(u16* path);
    u32 fsCreateFileFunc(u16* path, u64 a2);
    u32 fsCreateDirectoryFunc(u16* path);
    u32 fsRenameDirectoryFunc(u16* path1, u16* path2);
    u32 fsRegArchiveCallback(u8* path, u32* arch, u32 isAddOnContent, u32 isAlias);
    int fsOpenArchiveFunc(u32* fsHandle, u64* out, u32 archiveID, u32 pathType, u32 pathData, u32 pathsize);
    
    // Stubbed functions
    int fsFormatSaveData(int *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, char a11);
    int fsSetThisSaveDataSecureValue(u32 a1, u64 a2); //0x086E00C0
    int Obsoleted_5_0_fsSetSaveDataSecureValue(u64 a1, u32 a2, u32 a3, u8 a4); // 0x08650140
    int fsSetSaveDataSecureValue(u64 a1, u32 a2, u64 a3, u8 a4 ); // 0x08750180

    // Table containing pointers to replacement FS functions
    u32 fileOperationFuncs[NUMBER_FILE_OP] = { (u32)fsOpenFileFunc, (u32)fsOpenDirectoryFunc, (u32)fsDeleteFileFunc, (u32)fsRenameFileFunc, (u32)fsDeleteDirectoryFunc, (u32)fsDeleteDirectoryRecFunc, (u32)fsCreateFileFunc, (u32)fsCreateDirectoryFunc, (u32)fsRenameDirectoryFunc };
}