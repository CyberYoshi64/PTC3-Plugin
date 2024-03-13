#include "BasicAPI.hpp"

namespace CTRPluginFramework {
    u32 BasicAPI::handleIDCounter = BASICAPI_HANDLE_START;
    std::vector<FileStruct> BasicAPI::files = {};
    u32 BasicAPI::flags = APIFLAG_DEFAULT;
    u32 g_BasicAPI_PathType = 0;

    #define RGBA8_to_5551(r,g,b,a)  ((((b)>>3)&0x1f)<<1)|((((g)>>3)&0x1f)<<6)|((((r)>>3)&0x1f)<<11)|((a / 255)&1)

    bool basicapi__HavePermission(bool write) {
        if (write) {
            if (g_BasicAPI_PathType == APIPATHTYPE_SB_OTHERS && !(BasicAPI::flags & APIFLAG_FS_ACC_XREF_RW)) return false;
            if (g_BasicAPI_PathType == APIPATHTYPE_SAFE_OTHERS && !(BasicAPI::flags & APIFLAG_FS_ACC_XREF_RW)) return false;
            if (g_BasicAPI_PathType == APIPATHTYPE_SDMC && !(BasicAPI::flags & APIFLAG_FS_ACC_SD_RW)) return false;
        }
        return g_BasicAPI_PathType != APIPATHTYPE_NONE;
    }

    std::string basicapi__EvaluatePath(std::string path) {
        s32 i, j;
        g_BasicAPI_PathType = APIPATHTYPE_NONE;
        while ((i=path.find("/../"))>=0) {
            path.erase(path.begin()+i, path.begin()+i+3);
        }
        while ((i=path.find("/./"))>=0) {
            path.erase(path.begin()+i, path.begin()+i+2);
        }
        while ((i=path.find("//"))>=0) {
            path.erase(path.begin()+i);
        }
        if (path.ends_with("/.."))
            path.resize(path.size()-3);
        if (!path.size()) return "!EEMPTY_PATH";
        if (path.starts_with("/")) {
            g_BasicAPI_PathType = APIPATHTYPE_SB_SELF;
            return EXTDATA_PATH"/" + CYX::GetExtDataFolderName() + path;
        }
        if (path.starts_with("ptc:/")) {
            g_BasicAPI_PathType = APIPATHTYPE_SB_OTHERS;
            return EXTDATA_PATH + path.substr(4);
        }
        if (BasicAPI::flags & APIFLAG_FS_ACC_SAFE) {
            if (path.starts_with("home:/")) {
                g_BasicAPI_PathType = APIPATHTYPE_SAFE_SELF;
                return CYX::GetHomeFolder() + path.substr(5);
            }
            if (path.starts_with("shared:/")) {
                g_BasicAPI_PathType = APIPATHTYPE_SAFE_SHARED;
                return HOMEFS_SHARED_PATH + path.substr(7);
            }
        }
        if (BasicAPI::flags & APIFLAG_FS_ACCESS_XREF) {
            if (path.starts_with("data:/")) {
                g_BasicAPI_PathType = APIPATHTYPE_SAFE_OTHERS;
                return HOMEFS_PATH + path.substr(5);
            }
        }
        if (BasicAPI::flags & APIFLAG_FS_ACCESS_SD) {
            if (path.starts_with("sdmc:/")) {
                g_BasicAPI_PathType = APIPATHTYPE_SDMC;
                return path.substr(5);
            }
        }
        return "!EACCESS_DENIED";
    }
    std::string basicapi__EvaluatePath16(u16* path, int& len) {
        string16 ss(path, len);
        std::string s;
        Utils::ConvertUTF16ToUTF8(s, ss);
        return basicapi__EvaluatePath(s);
    }

    int BasicAPI::Func_FILES(BASICAPI_FUNCVARS) {
        if (argc != 1 || outc != 1) {
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);
        }
        u16* ptr; int mode = 0, len, res;
        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);
        
        std::string str = basicapi__EvaluatePath16(ptr, len);
        if (str=="")
            return CYX::bsaSetString(outv, "EEMPTY_PATH");
        if (str[0]=='!')
            return CYX::bsaSetString(outv, str.substr(1));
        Directory dir(str); str = "";
        StringVector vec; u32 s;
        s = dir.ListDirectories(vec);
        if (mode == 0) str += Utils::Format("%d",s);
        if (mode < 2) {
            for (u32 i=0; i<vec.size(); i++)
                str += "\t/"+vec[i];
        }
        vec.clear();
        s = dir.ListFiles(vec);
        if (mode == 0)
            str += Utils::Format("\t%d",s);
        if (mode==0 || mode==2) {
            for (u32 i=0; i<vec.size(); i++)
                str += "\t*"+vec[i];
        }
        vec.clear();
        dir.Close();
        return CYX::bsaSetString(outv, str);
    }

    int BasicAPI::FileRead(FileStruct f, u16* buf, u32* len, u32 size, u32 bytesLeft) {
        if (!f.f) return File::UNEXPECTED_ERROR;
        if (f.flags & APIFSTRUCT_UTF16) size *= 2;
        u32 l = (size > bytesLeft) ? bytesLeft : size;
        *len = l;
        if (f.flags & APIFSTRUCT_UTF16) *len = l/2;
        return f.f->Read(buf, l);
    }
    int BasicAPI::FileWrite(FileStruct f, u16* buf, u32 size) {
        if (!f.f) return File::UNEXPECTED_ERROR;
        if (f.flags & APIFSTRUCT_UTF16) size *= 2;
        return f.f->Write(buf, size);
    }

    int BasicAPI::Func_FOPEN(BASICAPI_FUNCVARS) {
        if (argc != 2 || outc != 1) {
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);
        }
        if (files.size() >= BASICAPI_FILESTACK_LIMIT) {
            return CYX::bsaSetInteger(outv, -128);
        }
        u16* ptr;
        std::string path;
        string16 mode_s;
        u32 fflags = APIFSTRUCT_UTF16; int res, len;
        int mode = 0; bool modeUseString;

        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, 0, 0);

        path = basicapi__EvaluatePath16(ptr, len);
        if (path=="")
            return CYX::bsaSetInteger(outv, -256);
        if (path[0]=='!')
            return CYX::bsaSetInteger(outv, -257);
        
        if (!(modeUseString = (argv+1)->type->type >= 2))
            CYX::bsaGetInteger(argv + 1, &mode);
        else if (!(ptr = CYX::bsaGetString(argv + 1, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);
        
        if (modeUseString) {
            mode_s = ptr; strupper16(mode_s);
            mode = File::SYNC;
            if (mode_s.find('R')!=-1) mode |= (File::READ);
            if (mode_s.find('W')!=-1) mode |= (File::WRITE | File::TRUNCATE | File::CREATE);
            if (mode_s.find('A')!=-1) mode |= (File::WRITE | File::APPEND);
            if (mode_s.find('C')!=-1) mode |= (File::WRITE | File::CREATE);
            if (mode_s.find('X')!=-1) mode |= (File::WRITE);
            if (mode_s.find('+')!=-1) mode |= (File::RWC);
            if (mode_s.find('B')!=-1) fflags = APIFSTRUCT_ANSI;
        }
        if (!basicapi__HavePermission(mode & File::WRITE))
            return CYX::bsaSetInteger(outv, -6);
        if ((mode & ~File::SYNC) == 0)
            return CYX::bsaSetInteger(outv, File::INVALID_MODE);

        FileStruct f;
        f.flags = fflags;
        f.handle = handleIDCounter++;
        if (handleIDCounter > BASICAPI_HANDLE_END) handleIDCounter = BASICAPI_HANDLE_START;
        f.f = new File(path, mode);
        if (!f.f) return CYX::bsaSetInteger(outv, File::UNEXPECTED_ERROR);
        if (!f.f->IsOpen()) {
            f.f->Close();
            delete f.f;
            return CYX::bsaSetInteger(outv, File::UNEXPECTED_ERROR-2);
        }
        files.push_back(f);
        return CYX::bsaSetInteger(outv, f.handle);
    }
    int BasicAPI::Func_FMODE(BASICAPI_FUNCVARS) {
        if (argc != 2)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u32 handle, mode; int res;
        if ((res = CYX::bsaGetInteger(argv, (int*)&handle)))
            return res;

        if ((res = CYX::bsaGetInteger(argv+1, (int*)&mode)))
            return res;

        u32 fflags = APIFSTRUCT_UTF16;
        string16 flagstr;
        if (mode == 1) fflags = APIFSTRUCT_UTF16;
        if (mode == 2) fflags = APIFSTRUCT_ANSI;
        if (!handle) {
            return CYX::bsaSetInteger(outv, -64);
        }
        for (int i=0; i<files.size(); i++) {
            if (files[i].handle == handle) {
                files[i].flags = fflags;
                return CYX::bsaSetInteger(outv, 0);
            }
        }
        return CYX::bsaSetInteger(outv, -64);
    }
    int BasicAPI::Func_FREAD(BASICAPI_FUNCVARS) {
        if (!argc || !outc)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u32 handle; int res;
        void* ptr;

        if ((res = CYX::bsaGetInteger(argv, (int*)&handle)))
            return res;

        int length = 272144;
        if (argc > 1) {
            if ((res = CYX::bsaGetInteger(argv+1, &length)))
                return res;
        }
        if (length > 272144) length = 272144;

        for (int i=0; i<files.size(); i++) {
            if (files[i].handle == handle) {
                u32 t=(files[i].f->GetSize() - files[i].f->Tell());
                res = FileRead(files[i], CYX::editorInstance->clipboardData, &CYX::editorInstance->clipboardLength, length, t);
                if (res)
                    return CYX::bsaSetInteger(outv, res);
                if (files[i].flags & APIFSTRUCT_ANSI)
                    strncpyu8u16((u8*)CYX::editorInstance->clipboardData, CYX::editorInstance->clipboardData, CYX::editorInstance->clipboardLength);
                if (!CYX::bsaAllocString(&ptr, CYX::editorInstance->clipboardLength, CYX::editorInstance->clipboardData))
                    return CYX::bsaError(PTCERR_OUT_OF_MEMORY, -1, 0);
                if ((res = CYX::bsaSetStringRaw(outv, ptr)));
                    return res;
                return 0;
            }
        }
        return CYX::bsaSetString(outv, "EBAD_HANDLE");
    }
    int BasicAPI::Func_FWRITE(BASICAPI_FUNCVARS) {
        if (argc < 2)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u32 handle; int res;
        if ((res = CYX::bsaGetInteger(argv, (int*)&handle)))
            return res;
        int stringLength;
        u16* stringData = CYX::bsaGetString(argv+1, &stringLength);
        if (!stringData) return CYX::bsaError(PTCERR_TYPE_MISMATCH, 1, 0);
        if (!stringLength) return CYX::bsaSetInteger(outv, 0);
            
        for (int i=0; i<files.size(); i++) {
            if (files[i].handle == handle) {
                CYX::editorInstance->clipboardLength = stringLength;
                if (files[i].flags & APIFSTRUCT_ANSI) {
                    strncpyu16u8(stringData, (u8*)CYX::editorInstance->clipboardData, stringLength);
                    CYX::editorInstance->clipboardLength /= 2;
                } 
                res = FileWrite(files[i], CYX::editorInstance->clipboardData, stringLength);
                return CYX::bsaSetInteger(outv, res);
            }
        }
        return CYX::bsaSetInteger(outv, 0);
    }
    int BasicAPI::Func_FSEEK(BASICAPI_FUNCVARS) {
        if (!argc)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u32 handle; int res;
        if ((res = CYX::bsaGetInteger(argv, (int*)&handle)))
            return res;
        bool doSeek = argc > 1;
        int seek = 0, whence = 0;
        if (argc>1) {
            if ((res = CYX::bsaGetInteger(argv+1, &seek)))
                return res;
        }
        if (argc>2) {
            if ((res = CYX::bsaGetInteger(argv+2, &whence)))
                return res;
        }
        whence = (whence & 3) % 3;
        for (int i=0; i<files.size(); i++) {
            if (files[i].handle == handle) {
                if (doSeek) files[i].f->Seek(seek, (File::SeekPos)whence);
                seek = files[i].f->Tell();
                return CYX::bsaSetInteger(outv, seek);
            }
        }
        return CYX::bsaSetInteger(outv, -1);
    }
    int BasicAPI::Func_FCLOSE(BASICAPI_FUNCVARS) {
        if (!argc)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u32 handle; int res;
        if ((res = CYX::bsaGetInteger(argv, (int*)&handle)))
            return res;

        for (int i=0; i<files.size(); i++) {
            if (files[i].handle == handle) {
                files[i].f->Close();
                files.erase(files.begin()+i);
                return CYX::bsaSetInteger(outv, 0);
            }
        }
        return CYX::bsaSetInteger(outv, -1);
    }
    int BasicAPI::Func_MKFILE(BASICAPI_FUNCVARS) {
        if (!argc)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u16* ptr; int len, res;
        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);
        
        std::string fpath = basicapi__EvaluatePath16(ptr, len);
        if (fpath=="")
            return CYX::bsaSetInteger(outv, -8);
        if (fpath.starts_with('!'))
            return CYX::bsaSetInteger(outv, -7);
        if (!basicapi__HavePermission(true))
            return CYX::bsaSetInteger(outv, -6);
        
        return CYX::bsaSetInteger(outv, File::Create(fpath));
    }
    int BasicAPI::Func_RMFILE(BASICAPI_FUNCVARS) {
        if (!argc)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u16* ptr; int len, res;
        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);
        
        std::string fpath = basicapi__EvaluatePath16(ptr, len);
        if (fpath=="")
            return CYX::bsaSetInteger(outv, -8);
        if (fpath.starts_with('!'))
            return CYX::bsaSetInteger(outv, -7);
        if (!basicapi__HavePermission(true))
            return CYX::bsaSetInteger(outv, -6);
        
        return CYX::bsaSetInteger(outv, File::Remove(fpath));
    }
    int BasicAPI::Func_MVFILE(BASICAPI_FUNCVARS) {
        if (argc < 1)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u16* ptr; int len, res;
        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, 0, 0);
        
        std::string fpath = basicapi__EvaluatePath16(ptr, len);
        if (fpath=="")
            return CYX::bsaSetInteger(outv, -8);
        if (fpath.starts_with('!'))
            return CYX::bsaSetInteger(outv, -7);
        if (!basicapi__HavePermission(true))
            return CYX::bsaSetInteger(outv, -6);
        
        if (!(ptr = CYX::bsaGetString(argv+1, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, 1, 0);
        
        std::string fpath2 = basicapi__EvaluatePath16(ptr, len);
        if (fpath=="")
            return CYX::bsaSetInteger(outv, -8);
        if (fpath.starts_with('!'))
            return CYX::bsaSetInteger(outv, -7);
        if (!basicapi__HavePermission(true))
            return CYX::bsaSetInteger(outv, -6);
        
        return CYX::bsaSetInteger(outv, File::Rename(fpath, fpath2));
    }
    int BasicAPI::Func_MKDIR(BASICAPI_FUNCVARS) {
        if (!argc)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u16* ptr; int len, res;
        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);
        
        std::string fpath = basicapi__EvaluatePath16(ptr, len);
        if (fpath=="")
            return CYX::bsaSetInteger(outv, -8);
        if (fpath.starts_with('!'))
            return CYX::bsaSetInteger(outv, -7);
        if (!basicapi__HavePermission(true))
            return CYX::bsaSetInteger(outv, -6);
        
        return CYX::bsaSetInteger(outv, Directory::Create(fpath));
    }
    int BasicAPI::Func_RMDIR(BASICAPI_FUNCVARS) {
        if (!argc)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u16* ptr; int len, res;
        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);
        
        std::string fpath = basicapi__EvaluatePath16(ptr, len);
        if (fpath=="")
            return CYX::bsaSetInteger(outv, -8);
        if (fpath.starts_with('!'))
            return CYX::bsaSetInteger(outv, -7);
        if (!basicapi__HavePermission(true))
            return CYX::bsaSetInteger(outv, -6);
        
        return CYX::bsaSetInteger(outv, Directory::Remove(fpath));
    }
    int BasicAPI::Func_MVDIR(BASICAPI_FUNCVARS) {
        if (argc < 1)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u16* ptr; int len, res;
        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, 0, 0);
        
        std::string fpath = basicapi__EvaluatePath16(ptr, len);
        if (fpath=="")
            return CYX::bsaSetInteger(outv, -8);
        if (fpath.starts_with('!'))
            return CYX::bsaSetInteger(outv, -7);
        if (!basicapi__HavePermission(true))
            return CYX::bsaSetInteger(outv, -6);
        
        if (!(ptr = CYX::bsaGetString(argv+1, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, 1, 0);
        
        std::string fpath2 = basicapi__EvaluatePath16(ptr, len);
        if (fpath=="")
            return CYX::bsaSetInteger(outv, -8);
        if (fpath.starts_with('!'))
            return CYX::bsaSetInteger(outv, -7);
        if (!basicapi__HavePermission(true))
            return CYX::bsaSetInteger(outv, -6);
        
        return CYX::bsaSetInteger(outv, Directory::Rename(fpath, fpath2));
    }
    int BasicAPI::Func_CHKDIR(BASICAPI_FUNCVARS) {
        if (!argc)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u16* ptr; int len, res;
        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);
        
        std::string fpath = basicapi__EvaluatePath16(ptr, len);
        if (fpath=="")
            return CYX::bsaSetInteger(outv, -8);
        if (fpath.starts_with('!'))
            return CYX::bsaSetInteger(outv, -7);
        if (!basicapi__HavePermission(false))
            return CYX::bsaSetInteger(outv, -6);
        
        return CYX::bsaSetInteger(outv, Directory::Exists(fpath));
    }
    int BasicAPI::Func_CHKFILE(BASICAPI_FUNCVARS) {
        if (!argc)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        u16* ptr; int len, res;
        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);
        
        std::string fpath = basicapi__EvaluatePath16(ptr, len);
        if (fpath=="")
            return CYX::bsaSetInteger(outv, -8);
        if (fpath.starts_with('!'))
            return CYX::bsaSetInteger(outv, -7);
        if (!basicapi__HavePermission(false))
            return CYX::bsaSetInteger(outv, -6);
        
        return CYX::bsaSetInteger(outv, File::Exists(fpath));
    }
    int BasicAPI::Func_INIT(BASICAPI_FUNCVARS) {
        Cleanup();
        File f;
        if (!Directory::Exists(CACHE_PATH))
            Directory::Create(CACHE_PATH);
        if (File::Exists(CLIPBOARDCACHE_PATH))
            File::Remove(CLIPBOARDCACHE_PATH);
        if (File::Open(f, CLIPBOARDCACHE_PATH, File::RWC | File::TRUNCATE)==0) {
            f.Dump((u32)CYX::editorInstance->clipboardLength, sizeof(u32));
            f.Dump((u32)CYX::editorInstance->clipboardData, sizeof(CYX::editorInstance->clipboardData));
        }
        f.Close();
        if (!Directory::Exists(HOMEFS_PATH)) Directory::Create(HOMEFS_PATH);
        if (!Directory::Exists(CYX::GetHomeFolder())) Directory::Create(CYX::GetHomeFolder());
        if (!Directory::Exists(HOMEFS_SHARED_PATH)) Directory::Create(HOMEFS_SHARED_PATH);
        return 0;
    }
    int BasicAPI::Func_EXIT(BASICAPI_FUNCVARS) {
        Cleanup();
        File f;
        if (File::Open(f, CLIPBOARDCACHE_PATH, File::READ)==0) {
            f.Inject((u32)CYX::editorInstance->clipboardLength, sizeof(u32));
            f.Inject((u32)CYX::editorInstance->clipboardData, sizeof(CYX::editorInstance->clipboardData));
        }
        f.Close();
        File::Remove(CLIPBOARDCACHE_PATH);
        CYX::SaveProjectSettings();
        return 0;
    }
    int BasicAPI::Func_CRASH(BASICAPI_FUNCVARS) {
        setCTRPFConfirm(CYXCONFIRM_BASICAPI_TOHOME, 0);
        if(!waitCTRPFConfirm()) return CYX::bsaError(PTCERR_END_WO_CALL, -1, 0);
        OnProcessExit();
        Process::ReturnToHomeMenu();
        return 3;
    }

    int basicapi__BSAConfig_StringToID(u16* ptr, int& len) {
        std::string s; Process::ReadString((u32)ptr, s, MIN(64, len * 2), StringFormat::Utf16);
        strupper(s);
        
        if (s == "SAFEDIR")         return BAPICFGID_SAFEDIR;
        if (s == "PRJ_ACCESS")      return BAPICFGID_PRJ_ACCESS;
        if (s == "SD_ACCESS")       return BAPICFGID_SD_ACCESS;
        if (s == "DIRECTMODE")      return BAPICFGID_DIRECTMODE;
        if (s == "PTC_VER")         return BAPICFGID_PTC_VER;
        if (s == "LANG")            return BAPICFGID_LANG;
        if (s == "LANGSTR")         return BAPICFGID_LANGSTR;
        if (s == "REGION")          return BAPICFGID_REGION;
        if (s == "REGIONSTR")       return BAPICFGID_REGIONSTR;
        if (s == "SYS_VERSTR")      return BAPICFGID_SYS_VERSTR;
        if (s == "SYS_REGION")      return BAPICFGID_SYS_REGION;
        if (s == "SYS_REGIONSTR")   return BAPICFGID_SYS_REGIONSTR;
        if (s == "SYS_MODEL")       return BAPICFGID_SYS_MODEL;
        if (s == "ISCITRA")         return BAPICFGID_ISCITRA;
        if (s == "VOLSLIDER")       return BAPICFGID_VOLSLIDER;
        if (s == "HEADSET")         return BAPICFGID_HEADSET;
        if (s == "3DSLIDER")        return BAPICFGID_3DSLIDER;
        if (s == "WIFILEVEL")       return BAPICFGID_WIFILEVEL;
        if (s == "BAT_LEVEL_RAW")   return BAPICFGID_BAT_LEVEL_RAW;
        if (s == "BAT_LEVEL")       return BAPICFGID_BAT_LEVEL;
        if (s == "BAT_CHARGING")    return BAPICFGID_BAT_CHARGING;
        if (s == "BAT_CHARGER")     return BAPICFGID_BAT_CHARGER;
        if (s == "CAN_SLEEP")       return BAPICFGID_CAN_SLEEP;
        if (s == "NETWORKSTATE")    return BAPICFGID_NETWORKSTATE;
        if (s == "PARENTALFLAGS")   return BAPICFGID_PARENTALFLAGS;
        if (s == "FIRM_VER")        return BAPICFGID_FIRM_VER;
        if (s == "KERNEL_VER")      return BAPICFGID_KERNEL_VER;
        if (s == "CYX_VER")         return BAPICFGID_CYX_VER;
        if (s == "CYX_VERSTR")      return BAPICFGID_CYX_VERSTR;
        if (s == "CYX_COMMIT")      return BAPICFGID_CYX_COMMIT;
        if (s == "CYX_BUILDSTR")    return BAPICFGID_CYX_BUILDSTR;
        if (s == "SDMCCLUSTER")     return BAPICFGID_SDMCCLUSTER;
        if (s == "SDMCSECTOR")      return BAPICFGID_SDMCSECTOR;
        if (s == "SDMCFREE")        return BAPICFGID_SDMCFREE;
        if (s == "SDMCFREE_C")      return BAPICFGID_SDMCFREE_C;
        if (s == "SDMCTOTAL")       return BAPICFGID_SDMCTOTAL;
        if (s == "SDMCTOTAL_C")     return BAPICFGID_SDMCTOTAL_C;
        return -1;
    }

    int BasicAPI::Func_CFGSET(BASICAPI_FUNCVARS) {
        if (argc < 2)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        int cfgType, len, val, res;
        u16* ptr; std::string s;
        if ((argv->type->type >= 4)) {
            if (!(ptr = CYX::bsaGetString(argv, &len)))
                return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);
            cfgType = basicapi__BSAConfig_StringToID(ptr, len);
        } else {
            if ((res = CYX::bsaGetInteger(argv, &cfgType)))
                return res;
        }
        if ((res = CYX::bsaGetInteger(argv+1, &val)))
            return res;

        bool condition1 = false;
        if (flags & APIFLAG_ALLOW_TOGGLE) {
            switch (cfgType) {
            case BAPICFGID_SAFEDIR:
                flags &= ~APIFLAG_FS_ACC_SAFE;
                if (val) flags |= APIFLAG_FS_ACC_SAFE;
                return 0;
            case BAPICFGID_PRJ_ACCESS:
                condition1 = flags & APIFLAG_FS_ACC_XREF_RW;
                flags &= ~APIFLAG_FS_ACCESS_XREF;
                if (val & 1) flags |= APIFLAG_FS_ACC_XREF_RO;
                if (val & 2) {
                    flags |= APIFLAG_FS_ACC_XREF_RO;
                    if (!condition1) {
                        setCTRPFConfirm(CYXCONFIRM_BASICAPI_XREF_RW, 0);
                        condition1 = waitCTRPFConfirm();
                    }
                    if (condition1) flags |= APIFLAG_FS_ACC_XREF_RW;
                }
                return 0;
            case BAPICFGID_SD_ACCESS:
                condition1 = flags & APIFLAG_FS_ACC_SD_RW;
                flags &= ~APIFLAG_FS_ACCESS_SD;
                if (val & 1) flags |= APIFLAG_FS_ACC_SD_RO;
                if (val & 2) {
                    flags |= APIFLAG_FS_ACC_SD_RO;
                    if (!condition1) {
                        setCTRPFConfirm(CYXCONFIRM_BASICAPI_SD_RW, 0);
                        condition1 = waitCTRPFConfirm();
                    }
                    if (condition1) flags |= APIFLAG_FS_ACC_SD_RW;
                }
                return 0;
            case BAPICFGID_CAN_SLEEP:
                mcuSetSleep(val);
                return 0;
            }
        }
        return 1;
    }

    int BasicAPI::Func_CFGGET(BASICAPI_FUNCVARS) {
        if (argc < 1 || !outv)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);
        
        int cfgType, len, res;
        u16* ptr; std::string s;
        if ((argv->type->type >= 4)) {
            if (!(ptr = CYX::bsaGetString(argv, &len)))
                return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);
            cfgType = basicapi__BSAConfig_StringToID(ptr, len);
        } else {
            if ((res = CYX::bsaGetInteger(argv, &cfgType)))
                return res;
        }
        switch (cfgType){
        case BAPICFGID_SAFEDIR:
            return CYX::bsaSetInteger(outv, (bool)(flags & APIFLAG_FS_ACC_SAFE));
        case BAPICFGID_PRJ_ACCESS:
            return CYX::bsaSetInteger(outv, ((flags & APIFLAG_FS_ACCESS_XREF)>>APIFLAG_BIT_FS_ACC_XREF_RO));
        case BAPICFGID_SD_ACCESS:
            return CYX::bsaSetInteger(outv, ((flags & APIFLAG_FS_ACCESS_SD)>>APIFLAG_BIT_FS_ACC_SD_RO));
        case BAPICFGID_DIRECTMODE:
            return CYX::bsaSetInteger(outv, CYX::mirror.isDirectMode);
        case BAPICFGID_PTC_VER:
            return CYX::bsaSetInteger(outv, CYX::currentVersion);
        case BAPICFGID_LANG:
            return CYX::bsaSetInteger(outv, (int)System::GetSystemLanguage());
        case BAPICFGID_LANGSTR:
            s = "JPNENGFRADEUITASPACHNKORNEDPORRUSTWNUNK";
            return CYX::bsaSetString(outv, s.substr(MIN((int)System::GetSystemLanguage(), 12), 3));
        case BAPICFGID_REGION:
            return CYX::bsaSetInteger(outv, g_region);
        case BAPICFGID_REGIONSTR:
            return CYX::bsaSetString(outv, g_regionString);
        case BAPICFGID_SYS_VERSTR:
            return CYX::bsaSetString(outv, g_osSysVer);
        case BAPICFGID_SYS_REGION:
            return CYX::bsaSetInteger(outv, g_systemRegion);
        case BAPICFGID_SYS_REGIONSTR:
            return CYX::bsaSetString(outv, g_systemRegionString);
        case BAPICFGID_SYS_MODEL:
            return CYX::bsaSetInteger(outv, g_systemModel);
        case BAPICFGID_ISCITRA:
            return CYX::bsaSetInteger(outv, System::IsCitra());
        case BAPICFGID_VOLSLIDER:
            return CYX::bsaSetInteger(outv, CYX::volumeSliderValue);
        case BAPICFGID_HEADSET:
            return CYX::bsaSetInteger(outv, OS_SharedConfig->headset_connected != 0);
        case BAPICFGID_3DSLIDER:
            return CYX::bsaSetInteger(outv, (double)OS_SharedConfig->slider_3d);
        case BAPICFGID_WIFILEVEL:
            return CYX::bsaSetInteger(outv, OS_SharedConfig->wifi_strength);
        case BAPICFGID_BAT_LEVEL_RAW:
            return CYX::bsaSetInteger(outv, CYX::rawBatteryLevel);
        case BAPICFGID_BAT_LEVEL:
            return CYX::bsaSetInteger(outv, OS_SharedConfig->led_battery>>2 & 7);
        case BAPICFGID_BAT_CHARGING:
            return CYX::bsaSetInteger(outv, (bool)(OS_SharedConfig->led_battery & 2));
        case BAPICFGID_BAT_CHARGER:
            return CYX::bsaSetInteger(outv, OS_SharedConfig->led_battery & 1);
        case BAPICFGID_CAN_SLEEP:
            return CYX::bsaSetInteger(outv, (bool)CYX::mcuSleep);
        case BAPICFGID_NETWORKSTATE:
            return CYX::bsaSetInteger(outv, OS_SharedConfig->network_state);
        case BAPICFGID_PARENTALFLAGS:
            return CYX::bsaSetInteger(outv, parentalControlFlag);
        case BAPICFGID_FIRM_VER:
            return CYX::bsaSetInteger(outv, g_osFirmVer);
        case BAPICFGID_KERNEL_VER:
            return CYX::bsaSetInteger(outv, g_osKernelVer);
        case BAPICFGID_CYX_VER:
            return CYX::bsaSetInteger(outv, VER_INTEGER);
        case BAPICFGID_CYX_VERSTR:
            return CYX::bsaSetString(outv, STRING_VERSION);
        case BAPICFGID_CYX_COMMIT:
            return CYX::bsaSetInteger(outv, COMMIT_HASH);
        case BAPICFGID_CYX_BUILDSTR:
            return CYX::bsaSetString(outv, BUILD_DATE);
        case BAPICFGID_SDMCCLUSTER:
            return CYX::bsaSetInteger(outv, g_sdmcArcRes.clusterSize);
        case BAPICFGID_SDMCSECTOR:
            return CYX::bsaSetInteger(outv, g_sdmcArcRes.sectorSize);
        case BAPICFGID_SDMCFREE:
            return CYX::bsaSetDouble(outv, (double)CYX::sdmcFreeSpace);
        case BAPICFGID_SDMCFREE_C:
            return CYX::bsaSetInteger(outv, g_sdmcArcRes.freeClusters);
        case BAPICFGID_SDMCTOTAL:
            return CYX::bsaSetDouble(outv, (double)CYX::sdmcTotalSpace);
        case BAPICFGID_SDMCTOTAL_C:
            return CYX::bsaSetInteger(outv, g_sdmcArcRes.totalClusters);
        }
        return CYX::bsaSetInteger(outv, 0);
    }

    int BasicAPI::Func_OSD(BASICAPI_FUNCVARS) {
        if (argc < 2)
            return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);

        int type; std::string data="", chunk;
        int len;
        u32 index = 0, index2;
        for (u32 i=0; i<argc; i++) {
            u16* ptr = CYX::bsaGetString(argv+i, &len);
            if (!ptr) continue;
            string16 ss(ptr, len);
            std::string str = "";
            Utils::ConvertUTF16ToUTF8(str, ss);
            if (!i && str.size()>0) {
                if (str[0] == 'W') {
                    type = 1; continue;
                } else if (str[0] == 'N') {
                    type = 2; continue;
                } else if (str[0] == 'E') {
                    type = 3; continue;
                } else if (str[0] == 'S') {
                    type = 4; continue;
                } else if (str[0] == 'M') {
                    type = 0; continue;
                }
            } else if (i) {
                data += str + "\n";
            }
        }
        while(true) {
            if (index >= data.size()) break;
            index2 = data.find('\n', index);
            if (index2 < 0) index2 = 0xFFFFFFF;
            if (index2 > index+64) index2 = index + 64;
            chunk = data.substr(index, index2-index);
            switch (type) {
            case 1:
                OSD::Notify(chunk, Color::White, Color::Olive);
                break;
            case 2:
                OSD::Notify(chunk, Color::Cyan, Color::Navy);
                break;
            case 3:
                OSD::Notify(chunk, Color::White, Color::Maroon);
                break;
            case 4:
                OSD::Notify(chunk, Color::White, Color::Green);
                break;
            default:
                OSD::Notify(chunk, Color::White, Color::Teal);
                break;
            }
            index = index2;
            while (index < data.size() && data[index]=='\n') index++;
        }
        return 0;
    }
    int BasicAPI::Func_SYSGCOPY(BASICAPI_FUNCVARS) {
        if (argc < 2) {
            CYX::bsaSetString(outv, "EMISSING_ARGS");
            return 1;
        }
        int mode, slot, res;
        if ((res = CYX::bsaGetInteger(argv, &mode)))
            return res;
        if ((res = CYX::bsaGetInteger(argv, &slot)))
            return res;

        BASICGraphicPage* target = NULL;

        if (slot == -1) {
            target = &CYX::GraphicPage->font;
        } else if (slot>=0&&slot<6) {
            target = &CYX::GraphicPage->grp[slot];
        }
        if (!target) return 0;

        if (mode) {
            memcpy(CYX::GraphicPage->system.dispBuf, target->workBuf, 524288);
        } else {
            memcpy(target->dispBuf, CYX::GraphicPage->system.dispBuf, 524288);
            memcpy(target->workBuf, CYX::GraphicPage->system.dispBuf, 524288);
        }

        return 0;
    }
    int BasicAPI::Func_VALIDATE(BASICAPI_FUNCVARS) {
        if (!argc) {
            CYX::bsaSetInteger(outv, -1);
            return 1;
        }
        u16* ptr; int len, res;
        if (!(ptr = CYX::bsaGetString(argv, &len)))
            return CYX::bsaError(PTCERR_TYPE_MISMATCH, -1, 0);

        int doFix = 0;
        if (argc > 1) {
            if ((res = CYX::bsaGetInteger(argv+1, &doFix)))
                return res;
        }
        std::string fpath = basicapi__EvaluatePath16(ptr, len);
        if (!basicapi__HavePermission(true)) {
            return CYX::bsaSetInteger(outv, -6);
        }
        File f;
        if ((File::Open(f, fpath, File::RW))) {
            CYX::bsaSetInteger(outv, -1);
            return 1;
        }
        #define BUFFER_SIZE 262144
        u8 digest[20], oldDigest[20];
        void* buf = new u8[BUFFER_SIZE];
        u32 size, size2, chunk;
        size = size2 = f.GetSize()-20;
        f.Seek(-20, File::END);
        if (f.Read(oldDigest, 20)) {
            f.Close();
            return CYX::bsaSetInteger(outv, 0);
        }
        f.Seek(0, File::SET);

        SHA1_HMAC::CTX hmacCtx;
        SHA1_HMAC::Init(&hmacCtx, (u8*)Hooks::offsets.serverHMACKey, 64);
        while (size) {
            chunk = size > BUFFER_SIZE ? BUFFER_SIZE : size;
            if (f.Read(buf, chunk)) {
                f.Close();
                return CYX::bsaSetInteger(outv, -1);
            }
            SHA1_HMAC::Update(&hmacCtx, (u8*)buf, chunk);
            size -= chunk;
        }
        ::operator delete(buf); res = 0;
        SHA1_HMAC::Final(digest, &hmacCtx);
        if (memcmp(digest, oldDigest, 20)) {
            if (doFix) {
                f.Seek(-20, File::END);
                if (f.Write(digest, 20)) {
                    res = CYX::bsaSetInteger(outv, -1);
                } else {
                    res = CYX::bsaSetInteger(outv, 1);
                }
            } else
                res = CYX::bsaSetInteger(outv, 1);
        } else {
            res = CYX::bsaSetInteger(outv, 0);
        }
        f.Close();
        #undef BUFFER_SIZE
        return res;
    }

    int basicapi__Parse_StringToID(u16* ptr, int len) {
        std::string s; Process::ReadString((u32)ptr, s, MIN(32, len * 2), StringFormat::Utf16);
        strupper(s);
        
        if (s == "INIT")        return BAPIFUNC_INIT;
        if (s == "EXIT")        return BAPIFUNC_EXIT;
        if (s == "CFGGET")      return BAPIFUNC_CFGGET;
        if (s == "CFGSET")      return BAPIFUNC_CFGSET;
        if (s == "FILES")       return BAPIFUNC_FILES;
        if (s == "FOPEN")       return BAPIFUNC_FOPEN;
        if (s == "FMODE")       return BAPIFUNC_FMODE;
        if (s == "FREAD")       return BAPIFUNC_FREAD;
        if (s == "FWRITE")      return BAPIFUNC_FWRITE;
        if (s == "FCLOSE")      return BAPIFUNC_FCLOSE;
        if (s == "FSEEK")       return BAPIFUNC_FSEEK;
        if (s == "VALIDATE")    return BAPIFUNC_VALIDATE;
        if (s == "CHKFILE")     return BAPIFUNC_CHKFILE;
        if (s == "MKFILE")      return BAPIFUNC_MKFILE;
        if (s == "MVFILE")      return BAPIFUNC_MVFILE;
        if (s == "RMFILE")      return BAPIFUNC_RMFILE;
        if (s == "CHKDIR")      return BAPIFUNC_CHKDIR;
        if (s == "MKDIR")       return BAPIFUNC_MKDIR;
        if (s == "MVDIR")       return BAPIFUNC_MVDIR;
        if (s == "RMDIR")       return BAPIFUNC_RMDIR;
        if (s == "UNIXTIME")    return BAPIFUNC_UNIXTIME;
        if (s == "OSD")         return BAPIFUNC_OSD;
        if (s == "CRASH")       return BAPIFUNC_CRASH;
        if (s == "SYSGCOPY")    return BAPIFUNC_SYSGCOPY;
        if (s == "TYPE")        return BAPIFUNC_TYPE;
        return -1;
    }

    int BasicAPI::Parse(BSAFuncStack* stk) {
        int funcType, res, len; u16* ptr;
        if (!stk->argc) return CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);
        
        if ((stk->argv->type->type >= 4)) {
            if (!(ptr = CYX::bsaGetString(stk->argv, &len)))
                return CYX::bsaError(PTCERR_TYPE_MISMATCH, 0, 0);
            funcType = basicapi__Parse_StringToID(ptr, len);

        } else {
            if ((res = CYX::bsaGetInteger(stk->argv, &funcType)))
                return res;
        }
        switch (funcType) {
            case 0:                     return CYX::bsaSetInteger(stk->outv, 0x10000001);
            case 1: case 2: case 3: case 4:
                                        return CYX__CONTROLLER_ACCURATE ? CYX::bsaError(PTCERR_INCOMPATIBLE_STATEMENT, 0, 0) : CYX::bsaSetInteger(stk->outv, 0);
            case BAPIFUNC_INIT:         return Func_INIT(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_EXIT:         return Func_EXIT(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_CFGGET:       return Func_CFGGET(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_CFGSET:       return Func_CFGSET(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_FILES:        return Func_FILES(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_FOPEN:        return Func_FOPEN(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_FMODE:        return Func_FMODE(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_FREAD:        return Func_FREAD(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_FWRITE:       return Func_FWRITE(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_FCLOSE:       return Func_FCLOSE(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_FSEEK:        return Func_FSEEK(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_VALIDATE:     return Func_VALIDATE(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_CHKFILE:      return Func_CHKFILE(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_MKFILE:       return Func_MKFILE(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_MVFILE:       return Func_MVFILE(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_RMFILE:       return Func_RMFILE(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_CHKDIR:       return Func_CHKDIR(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_MKDIR:        return Func_MKDIR(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_MVDIR:        return Func_MVDIR(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_RMDIR:        return Func_RMDIR(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_UNIXTIME:     return CYX::bsaSetInteger(stk->outv, osGetUnixTime());
            case BAPIFUNC_OSD:          return Func_OSD(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_CRASH:        return Func_CRASH(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_SYSGCOPY:     return Func_SYSGCOPY(BASICAPI_FUNCVARSPASS);
            case BAPIFUNC_TYPE:         return stk->argc > 1 ? CYX::bsaSetInteger(stk->outv, (stk->argv+1)->type->type) : CYX::bsaError(PTCERR_ILLEGAL_FUNCCALL, -1, 0);
        }
        return 0;
    }

    void BasicAPI::Initialize() {}

    void BasicAPI::Finalize() {
        for (int i=0; i<files.size(); i++) {
            if (!files[i].f) continue;
            files[i].f->Close();
            delete files[i].f;
        }
        files.clear();
    }
    void BasicAPI::Cleanup() {
        for (int i=0; i<files.size(); i++) {
            if (!files[i].f) continue;
            files[i].f->Close();
            delete files[i].f;
        }
        files.clear();
    }
    void BasicAPI::MenuTick() {}
}
