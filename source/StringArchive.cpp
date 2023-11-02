#include "StringArchive.hpp"

namespace CTRPluginFramework {
    StringArchiveEntry*     StringArchive::entries = NULL;
    File                    StringArchive::arcFile;
    int                     StringArchive::entryCount = 0;
    int                     StringArchive::isArchiveReady = 0;
    
    int StringArchive::Load(u32 lang){
        StringArchiveHeader h;
        StringArchiveEntry e;
        char s[4] = {0};
        memcpy(s, STRINGARCHIVE_NAMES + lang*3, 3);

        isArchiveReady = 0;
        if (arcFile.IsOpen()) arcFile.Close();
        if (entries) free(entries);
        entries = NULL;

        if (File::Open(arcFile, Utils::Format(STRINGARCHIVE_PATH, s)))
            return STRINGARCHIVE_LOAD_FAIL_OPEN;
        if (arcFile.Read(&h, STRINGARCHIVEHEADER_SIZE))
            return STRINGARCHIVE_LOAD_FAIL_READ;
        if (h.magic != STRINGARCHIVE_MAGIC)
            return STRINGARCHIVE_LOAD_FAIL_SIGNATURE;
        if (h.version != STRINGARCHIVE_VERSION)
            return STRINGARCHIVE_LOAD_VERSION_MISMATCH;
        if (h.entries == 0)
            return STRINGARCHIVE_LOAD_NO_ENTRIES;
        if (h.entries > 1024)
            return STRINGARCHIVE_LOAD_TOO_MANY_ENTRIES;
        
        entryCount = h.entries;
        entries = (StringArchiveEntry*)malloc(h.entries * STRINGARCHIVEENTRY_SIZE);
        for (u32 i = 0; i < entryCount; i++){
            arcFile.Read(&entries[i], STRINGARCHIVEENTRY_SIZE);
            if (entries[i].fileOffset + entries[i].length > arcFile.GetSize())
                return STRINGARCHIVE_LOAD_OFFSET_OOB;
            if (entries[i].length > 32767)
                return STRINGARCHIVE_LOAD_STRING_TOO_LONG;
        }
        isArchiveReady = 1;
        return STRINGARCHIVE_LOAD_OKAY;
    }
    int StringArchive::Init(){
        int lang = Config::Get().language;
        if (Load(lang) != STRINGARCHIVE_LOAD_OKAY) {
            return Load(1);
        }
        return STRINGARCHIVE_LOAD_OKAY;
    }
    void StringArchive::Get(std::string& out, u32 id) {
        if (isArchiveReady) {
            for (u32 i = 0; i < entryCount; i++){
                if (entries[i].stringID == id) {
                    arcFile.Seek(entries[i].fileOffset, File::SeekPos::SET);
                    out.resize(entries[i].length);
                    arcFile.Read(&out[0], entries[i].length);
                    break;
                }
            }
        }
    }
    void StringArchive::Get(std::string& out, std::string& id) {
        if (isArchiveReady) {
            for (u32 i = 0; i < entryCount; i++){
                if (entries[i].stringName == id) {
                    arcFile.Seek(entries[i].fileOffset, File::SeekPos::SET);
                    out.resize(entries[i].length);
                    arcFile.Read(&out[0], entries[i].length);
                    break;
                }
            }
        }
    }
    void StringArchive::Get(std::string& out, const char* id) {
        if (isArchiveReady) {
            for (u32 i = 0; i < entryCount; i++){
                if (strncmp(entries[i].stringName, id, 20) == 0) {
                    arcFile.Seek(entries[i].fileOffset, File::SeekPos::SET);
                    out.resize(entries[i].length);
                    arcFile.Read(&out[0], entries[i].length);
                    break;
                }
            }
        }
    }

    std::string StringArchive::Get(u32 id) {
        #if DEBUG
            std::string s = Utils::Format("[ID: %08X]", id);
        #else
            std::string s = "";
        #endif
        if (isArchiveReady) {
            for (u32 i = 0; i < entryCount; i++){
                if (entries[i].stringID == id) {
                    arcFile.Seek(entries[i].fileOffset, File::SeekPos::SET);
                    s.resize(entries[i].length);
                    arcFile.Read(&s[0], entries[i].length);
                    break;
                }
            }
        }
        return s;
    }
    std::string StringArchive::Get(std::string& id) {
        #if DEBUG
            std::string s = Utils::Format("[ID: %s]", id.c_str());
        #else
            std::string s = "";
        #endif
        if (isArchiveReady) {
            for (u32 i = 0; i < entryCount; i++){
                if (entries[i].stringName == id) {
                    arcFile.Seek(entries[i].fileOffset, File::SeekPos::SET);
                    s.resize(entries[i].length);
                    arcFile.Read(&s[0], entries[i].length);
                    break;
                }
            }
        }
        return s;
    }
    std::string StringArchive::Get(const char* id) {
        #if DEBUG
            std::string s = Utils::Format("[ID: %s]", id);
        #else
            std::string s = "";
        #endif
        if (isArchiveReady) {
            for (u32 i = 0; i < entryCount; i++){
                if (strncmp(entries[i].stringName, id, 20) == 0) {
                    arcFile.Seek(entries[i].fileOffset, File::SeekPos::SET);
                    s.resize(entries[i].length);
                    arcFile.Read(&s[0], entries[i].length);
                    break;
                }
            }
        }
        return s;
    }

    std::string StringArchive::GetName(u32 id) {
        std::string s = "";
        if (isArchiveReady) {
            for (u32 i = 0; i < entryCount; i++){
                if (entries[i].stringID == id) {
                    s = entries[i].stringName;
                    break;
                }
            }
        }
        return s;
    }
    u32 StringArchive::GetID(std::string& id) {
        std::string s = "";
        if (isArchiveReady) {
            for (u32 i = 0; i < entryCount; i++){
                if (entries[i].stringName == id)
                    return entries[i].stringID;
            }
        }
        return 0;
    }
    u32 StringArchive::GetID(const char* id) {
        std::string s = "";
        if (isArchiveReady) {
            for (u32 i = 0; i < entryCount; i++){
                if (strncmp(entries[i].stringName, id, 20) == 0)
                    return entries[i].stringID;
            }
        }
        return 0;
    }

    void StringArchive::Exit(void) {
        isArchiveReady = 0;
        if (arcFile.IsOpen()) arcFile.Close();
        if (entries) free(entries);
        entries = NULL;
    }
}