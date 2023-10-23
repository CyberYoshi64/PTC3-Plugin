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
        
        entries = (StringArchiveEntry*)malloc(h.entries * STRINGARCHIVEENTRY_SIZE);
        for (u32 i = 0; i < h.entries; i++){
            arcFile.Read(&entries[i], STRINGARCHIVEENTRY_SIZE);
            if (entries[i].fileOffset + entries[i].length > arcFile.GetSize())
                return STRINGARCHIVE_LOAD_OFFSET_OOB;
            if (entries[i].length > 32767)
                return STRINGARCHIVE_LOAD_STRING_TOO_LONG;
        }
        entryCount = h.entries;
        isArchiveReady = 1;
        return STRINGARCHIVE_LOAD_OKAY;
    }
    int StringArchive::Init(){
        int lang = (int)System::GetSystemLanguage();
        if (Load(lang) != STRINGARCHIVE_LOAD_OKAY) {
            return Load(1);
        }
        return STRINGARCHIVE_LOAD_OKAY;
    }
    std::string StringArchive::Get(u32 id) {
        std::string s = "";
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
        std::string s = "";
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
        std::string s = "";
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
    void StringArchive::Exit(void) {
        isArchiveReady = 0;
        if (arcFile.IsOpen()) arcFile.Close();
        if (entries) free(entries);
    }
}