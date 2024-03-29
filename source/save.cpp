#include "save.hpp"
#include "constants/main.h"
#include "commonFuncs.hpp"
#include <3ds/util/utf.h>

/*
    OnionFS — with modifications applied by CyberYoshi64 in 2022

    - Removed UI for configuring OnionFS; this should be handled by the plugin's code instead
    - Removed settings and related functions
    - Added hooking for extData, which is where the interesting data for SB3 is found.
*/

namespace CTRPluginFramework {
    archData OnionSave::save = {0};
    u16 OnionSave::romPath[50] = {0};
    u16 OnionSave::dataPath[50] = {0};
    u16 OnionSave::extPath[50] = {0};
    File* OnionSave::debugFile = nullptr;

    LightLock debugLock = {0};

    bool checkFolderExists(u16* name) {
        FS_Archive sdmcArchive;
        Handle dirHandle;
        Result ret = 0;
        ret = FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
        if (ret) return false;
        ret = FSUSER_OpenDirectory(&dirHandle, sdmcArchive, fsMakePath(PATH_UTF16, name));
        FSDIR_Close(dirHandle);
        FSUSER_CloseArchive(sdmcArchive);
        if (ret) return false;
        return true;
    }

    bool createDirectory(char* name) {
        return Directory::Create(name) == 0;
    }

    void OnionSave::initDebug() {
        if (ENABLE_DEBUG) {
            LightLock_Init(&debugLock);
            std::string debugfile = DEBUGLOG_FILE;
            File::Remove(debugfile);
            debugFile = new File(debugfile, File::RWC);
            if (!debugFile->IsOpen()) debugFile = nullptr;
            debugAppend("<Plugin start>\n---------\n\n");
        }
    }
    
    void OnionSave::debugAppend(std::string data) {
        if (!debugFile || !ENABLE_DEBUG) return;
        LightLock_Lock(&debugLock);
        debugFile->Write(data.c_str(), data.size());
        debugFile->Flush();
        LightLock_Unlock(&debugLock);
    }

    int OnionSave::existArchiveu16(u16 * arch) {
        for (int i = 0; i < save.numEntries; i++) {
            if (strcmpu8u16(save.entries[i].archName, arch) == 0) {
                if (save.entries[i].finished) return i;
                else return -1;
            }
        }
        return -1;
    }

    int OnionSave::existArchiveHnd(u64 hnd) {
        for (int i = 0; i < save.numEntries; i++) {
            if (save.entries[i].archHandle == hnd) {
                return i;
            }
        }
        return -1;
    }

    int OnionSave::existArchiveu8(u8 * arch) {
        for (int i = 0; i < save.numEntries; i++) {
            if (strcmpdot(save.entries[i].archName, (char*)arch) == 0) {
                if (save.entries[i].finished) return i;
                else return -1;
            }
        }
        return -1;
    }

    void OnionSave::addArchiveHnd(u64 handle, u32 archid) {
        if (save.numEntries >= MAX_SAVE_ENTRIES) {
            DEBUG("Archive buffer is full.\n");
            return;
        }
        if (archid == ARCHIVE_SAVEDATA) { // application save data
            if (existArchiveHnd(handle) != -1) return;
            DEBUG("Adding save archive: %016lX\n", handle);
            save.entries[save.numEntries].type = ARCH_SAVE;
            save.entries[save.numEntries].archHandle = handle;
            save.entries[save.numEntries++].finished = 0;
        } else if (archid == ARCHIVE_EXTDATA) { // Extra User Save Data
            if (existArchiveHnd(handle) != -1) return;
            DEBUG("Adding extData archive: %016lX\n", handle);
            save.entries[save.numEntries].type = ARCH_EXTDATA;
            save.entries[save.numEntries].archHandle = handle;
            save.entries[save.numEntries++].finished = 0;
        } else {
            DEBUG("Rejected %016llX, archiveID: 0x%08X\n", handle, archid);
        }
    }

    void OnionSave::addArchive(u8* arch, u64 handle) {
        char newbuf[10];
        if (ENABLE_DEBUG) strncpy(newbuf, (char*)arch, 10);
        /*
            Archives starting with $ are used by Nintendo libraries
            internally. In SmileBASIC, it's to mount the SD Card for
            screenshots (Nintendo 3DS Camera).
        */
        if (arch[0] == '$') {
            DEBUG("Rejected \"%s\" (%08X%08X)\n", newbuf, (u32)(handle >> 32), (u32)handle);
            return;
        }
        if (existArchiveu8(arch) == -1) {
            int hndpos = existArchiveHnd(handle);
            if (hndpos == -1) {
                if (true){ //if ((u32)(handle) > 0x100000 && (u32)(handle) < 0x20000000) { // rom archives doesn't have a fsarchive handle, it has a pointer in its place.
                    DEBUG("Added \"%s\" as romfs archive!\n", newbuf);
                    if (save.numEntries >= MAX_SAVE_ENTRIES) {
                        DEBUG("Archive buffer is full.\n");
                        return;
                    }
                    save.entries[save.numEntries].type = ARCH_ROMFS;
                    save.entries[save.numEntries].archHandle = ~0x0;
                    strcpydot((save.entries[save.numEntries].archName), (char*)arch, sizeof(save.entries[0].archName));
                    save.entries[save.numEntries++].finished = 1;
                    return;
                } else {
                    DEBUG("Rejected \"%s\" with handle: 0x%08X%08X\n", newbuf, (u32)(handle >> 32), (u32)handle);
                    return;
                }
            }
            DEBUG("Added \"%s\" with handle: 0x%08X%08X\n", newbuf, (u32)(handle >> 32), (u32)handle);
            if (save.entries[hndpos].finished == 1) return;
            strcpydot((save.entries[hndpos].archName), (char*)arch, sizeof(save.entries[0].archName));
            save.entries[hndpos].finished = 1;
        }
    }

    void OnionSave::setupPackPaths() {
        strcatu16(OnionSave::romPath, (char*)"ram:", (char*)ROMFS_PATH"/");
        strcatu16(OnionSave::dataPath, (char*)"ram:", (char*)SAVEDATA_PATH"/");
        strcatu16(OnionSave::extPath, (char*)"ram:", (char*)EXTDATA_PATH"/");
        if (!checkFolderExists(OnionSave::romPath + 4)) Directory::Create(ROMFS_PATH);
        if (!checkFolderExists(OnionSave::dataPath + 4)) Directory::Create(SAVEDATA_PATH);
        if (!checkFolderExists(OnionSave::extPath + 4)) Directory::Create(EXTDATA_PATH);
    }
    
    bool OnionSave::getArchive(u16 * arch, u8* mode, bool isReadOnly) {
        int entry = existArchiveu16(arch);
        if (entry == -1) return false;
        u8 flag = save.entries[entry].type;
        *mode = flag;
        u8 romode = !((flag & ARCH_ROMFS) && isReadOnly);
        return romode;
    }
}