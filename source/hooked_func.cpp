#include "hooked_func.hpp"

namespace CTRPluginFramework {

    LightLock regLock;
    LightLock openLock;
    int strlen16(u16* str) {
        int sz = 0;
        while (*str++) sz++;
        return sz;
    }

    static thread_local u16 *g_buffers[2] = { nullptr, nullptr };

    u16* GetBuffer(bool secondary) {
        if (g_buffers[secondary] == nullptr)
            g_buffers[secondary] = static_cast<u16 *>(::operator new(0x200));
        return g_buffers[secondary];
    }

    static void concatFileName(u16* dest, u16* s1, u16* s2) {
        while (*s1) *dest++ = *s1++; //Copy the default file path
        while (*s2++ != u'/'); //Skip the archive lowpath
        while (*s2 == u'/') ++s2; // Skip any remaining  /
        while (*s2) *dest++ = *s2++; //Copy the rest of the filename
        *dest = '\0';
    }

    void strcpy16(u16* dst, u16* src) {
        while (*src) *dst++ = *src++;
        *dst = '\0';
    }
    u16* calculateNewPath(u16* initialPath, bool isReadOnly, bool isSecondary, bool* shouldReopen) {
        u8 mode;
        if (shouldReopen) *shouldReopen = true;
        DEBUG("Checking: ");
        DEBUGU16(initialPath);
        if (OnionSave::getArchive(initialPath, &mode, isReadOnly)) {
            u16* basePath;
            u16* dst = GetBuffer(isSecondary);
            switch (mode) {
            case ARCH_SAVE:
                basePath = OnionSave::dataPath;
                if (shouldReopen) *shouldReopen = false;
                break;
            case ARCH_EXTDATA:
                basePath = OnionSave::extPath;
                if (shouldReopen) *shouldReopen = false;
                break;
            case ARCH_ROMFS:
            default:
                basePath = OnionSave::romPath;
                break;
            }
            concatFileName(dst, basePath, initialPath);
            DEBUG(" redirected: ");
            DEBUGU16(dst);
            DEBUG("\n");
            return dst;
        } else {
            DEBUG(" not redirected.\n");
            return initialPath;
        }
    }

    u32  fsOpenFileFunc(u32 a1, u16* path, u32 a2) {
        bool reopen;
        u16* newPath = calculateNewPath(path, 0, false,  &reopen);
        int ret = ((fsu32u16u32)fileOpHooks[OPEN_FILE_OP].callCode)(a1, newPath, a2);
        if (newPath != path) {
            if (!reopen) return ret;
            if (ret < 0) ret = ((fsu32u16u32)fileOpHooks[OPEN_FILE_OP].callCode)(a1, path, a2);
        }
        return ret;
    }

    int checkFileExistsWithDir(u16* path) { // Sometimes game devs choose to check if a file exists by doing open directory on it.
                                            // The problem is that SD archive doesn't behave the same way as other archives.
                                            // Doing openDir on a file in the SD returns "doesn't exist" while on the save file it returns "operation not supported".
        Handle file;
        int ret = FSUSER_OpenFileDirectly(&file, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""), fsMakePath(PATH_UTF16, skipArchive(path)), FS_OPEN_READ, 0);
        if (R_SUCCEEDED(ret)) { // If the file exists...
            FSFILE_Close(file);
            return 0xE0C04702; //.. return "not supported" error.
        }
        return 0;
    }

    u32  fsOpenDirectoryFunc(u32 a1, u16* path) {
        //PANIC("fsOpenDirectoryFunc Test", _FILENAME, __LINE__);
        u16* newPath = calculateNewPath(path, 1);
        if (newPath != path) {
            int res = checkFileExistsWithDir(newPath);
            if (res) return res;
        }
        int ret = ((fsu32u16)fileOpHooks[OPEN_DIRECTORY_OP].callCode)(a1, newPath);
        return ret;
    }

    u32  fsDeleteFileFunc(u16* path) {
        //PANIC("fsDeleteFileFunc Test", _FILENAME, __LINE__);
        u16* newPath = calculateNewPath(path, 1);
        int ret = ((fsu16)fileOpHooks[DELETE_FILE_OP].callCode)(newPath);
        return ret;
    }

    u32  fsRenameFileFunc(u16* path1, u16* path2) {
        //PANIC("fsRenameFileFunc Test", _FILENAME, __LINE__);
        u16* newPath1 = calculateNewPath(path1, 1);
        u16* newPath2 = calculateNewPath(path2, 1, true);
        int ret = ((fsu16u16)fileOpHooks[RENAME_FILE_OP].callCode)(newPath1, newPath2);
        return ret;
    }

    u32  fsDeleteDirectoryFunc(u16* path) {
        //PANIC("fsDeleteDirectoryFunc Test", _FILENAME, __LINE__);
        u16* newPath = calculateNewPath(path, 1);
        int ret = ((fsu16)fileOpHooks[DELETE_DIRECTORY_OP].callCode)(newPath);
        return ret;
    }
    u32  fsDeleteDirectoryRecFunc(u16* path) {
        //PANIC("fsDeleteDirectoryRecFunc Test", _FILENAME, __LINE__);
        u16* newPath = calculateNewPath(path, 1);
        int ret = ((fsu16)fileOpHooks[DELETE_DIRECTORY_RECURSIVE_OP].callCode)(newPath);
        return ret;
    }
    u32  fsCreateFileFunc(u16* path, u64 a2) {
        //PANIC("fsCreateFileFunc Test", _FILENAME, __LINE__);
        u16* newPath = calculateNewPath(path, 1);
        int ret = ((fsu16u64)fileOpHooks[CREATE_FILE_OP].callCode)(newPath, a2);
        return ret;
    }

    u32  fsCreateDirectoryFunc(u16* path) {
        //PANIC("fsCreateDirectoryFunc Test", _FILENAME, __LINE__);
        u16* newPath = calculateNewPath(path, 1);
        int ret = ((fsu16)fileOpHooks[CREATE_DIRECTORY_OP].callCode)(newPath);
        return ret;
    }

    u32  fsRenameDirectoryFunc(u16* path1, u16* path2) {
        //PANIC("fsRenameDirectoryFunc Test", _FILENAME, __LINE__);
        u16* newPath1 = calculateNewPath(path1, 1);
        u16* newPath2 = calculateNewPath(path2, 1, true);
        int ret = ((fsu16u16)fileOpHooks[RENAME_DIRECTORY_OP].callCode)(newPath1, newPath2);
        return ret;
    }
    
    u32 fsRegArchiveCallback(u8* path, u32* arch, u32 isAddOnContent, u32 isAlias) {
        u32 ret, ret2;
        static u32 isFisrt = 1;
        u32 sdmcArchive = 0;
        ret = ((fsRegArchiveTypeDef)regArchiveHook.callCode)(path, arch, isAddOnContent, isAlias);
        if (!ret) {
            LightLock_Lock(&regLock);
            OnionSave::addArchive(path, arch[2] | (u64)(arch[3]) << 32);
            LightLock_Unlock(&regLock);
        }
        if (isFisrt) {
            isFisrt = 0;
            ((fsMountArchiveTypeDef)fsMountArchive)(&sdmcArchive, 9);
            if (sdmcArchive) {
                ret2 = ((fsRegArchiveTypeDef)regArchiveHook.callCode)((u8*)"ram:", (u32*)sdmcArchive, 0, 0);
            }
        }
        return ret;
    }

    int  fsOpenArchiveFunc(u32* fsHandle, u64* out, u32 archiveID, u32 pathType, u32 pathData, u32 pathsize) {
        u32 *cmdbuf = getThreadCommandBuffer();

        cmdbuf[0] = IPC_MakeHeader(0x80C, 3, 2); // 0x80C00C2
        cmdbuf[1] = archiveID;
        cmdbuf[2] = pathType;
        cmdbuf[3] = pathsize;
        cmdbuf[4] = IPC_Desc_StaticBuffer(pathsize, 0);
        cmdbuf[5] = pathData;

        Result ret = 0;
        if (R_FAILED(ret = svcSendSyncRequest(*fsHandle))) customBreak(0xbad, ret, 0);

        *out = cmdbuf[2] | ((u64)(cmdbuf[3]) << 32);
        DEBUG("fsOpenArch return 0x%08X%08X\n", cmdbuf[3], cmdbuf[2]);
        if ((u32)(*out) < 0x100000) OnionSave::addArchiveHnd(*out, archiveID);
        return cmdbuf[1];
    }

    // Stubbed - SB3 doesn't need this besides the initial format.
    int fsFormatSaveData(int *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, char a11) {
        DEBUG("fsFormatSaveData called but the save data will not be removed.\n");
        // std::string dir;
        // Utils::ConvertUTF16ToUTF8(dir, OnionSave::dataPath);
        // Directory::Remove(dir);
        // Directory::Create(dir);
        return 0;
    }

    // Stubbed
    int fsSetThisSaveDataSecureValue(u32 a1, u64 a2) { //0x086E00C0
        DEBUG("fsSetThisSaveDataSecureValue called with secure value 0x%016llX, ignoring.\n", a2);
        return 0;
    }

    // Stubbed
    int Obsoleted_5_0_fsSetSaveDataSecureValue(u64 a1, u32 a2, u32 a3, u8 a4) { // 0x08650140
        DEBUG("fsSetThisSaveDataSecureValue called with secure value 0x%016llX, ignoring.\n", a1);
        return 0;
    }

    // Stubbed
    int fsSetSaveDataSecureValue(u64 a1, u32 a2, u64 a3, u8 a4 ) { // 0x08750180
        DEBUG("fsSetThisSaveDataSecureValue called with secure value 0x%016llX, ignoring.\n", a1);
        return 0;
    }
}