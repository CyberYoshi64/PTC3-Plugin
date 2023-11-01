#include "BasicAPI.hpp"

namespace CTRPluginFramework {
    std::vector<BasicAPI::Entry> BasicAPI::Entries = {};
    u32 BasicAPI::handleIDCounter = BASICAPI_HANDLE_START;
    std::vector<BasicAPI::QueueEntry> BasicAPI::Queue = {};
    std::vector<BasicAPI::FileStruct> BasicAPI::Files = {};
    u32 BasicAPI::queueOffset = 0;
    u32 BasicAPI::flags = APIFLAG_DEFAULT;
    u32 g_BasicAPI_PathType = 0;

    #define RGBA8_to_5551(r,g,b,a)  ((((b)>>3)&0x1f)<<1)|((((g)>>2)&0x1f)<<6)|((((r)>>3)&0x1f)<<11)|((a / 255)&1)

    #define APIOUT(o,s)       CYX::CYXAPI_Out(o,s)
    #define APIOUT_I(o,s)       CYX::CYXAPI_Out(o,(s32)(s))
    #define APIOUT_D(o,s)       CYX::CYXAPI_Out(o,(double)(s))
    #define APIOUT_C(o)       CYX::CYXAPI_Out(o)
    #define APIGETCLIP(s)   s.append(CYX::editorInstance->clipboardData, CYX::editorInstance->clipboardLength)

    bool basicapi__HavePermission(bool write){
        if (write){
            if (g_BasicAPI_PathType == APIPATHTYPE_SB_OTHERS && !(BasicAPI::flags & APIFLAG_FS_ACC_XREF_RW)) return false;
            if (g_BasicAPI_PathType == APIPATHTYPE_SAFE_OTHERS && !(BasicAPI::flags & APIFLAG_FS_ACC_XREF_RW)) return false;
            if (g_BasicAPI_PathType == APIPATHTYPE_SDMC && !(BasicAPI::flags & APIFLAG_FS_ACC_SD_RW)) return false;
        }
        return g_BasicAPI_PathType != APIPATHTYPE_NONE;
    }

    std::string basicapi__EvaluatePath(std::string path){
        s32 i, j;
        g_BasicAPI_PathType = APIPATHTYPE_NONE;
        while ((i=path.find("/../"))>=0){
            path.erase(path.begin()+i, path.begin()+i+3);
        }
        while ((i=path.find("/./"))>=0){
            path.erase(path.begin()+i, path.begin()+i+2);
        }
        while ((i=path.find("//"))>=0){
            path.erase(path.begin()+i);
        }
        if (path.ends_with("/.."))
            path.resize(path.size()-3);
        if (!path.size()) return "!EEMPTY_PATH";
        if (path.starts_with("/")){
            g_BasicAPI_PathType = APIPATHTYPE_SB_SELF;
            return EXTDATA_PATH"/" + CYX::GetExtDataFolderName() + path;
        }
        if (path.starts_with("ptc:/")) {
            g_BasicAPI_PathType = APIPATHTYPE_SB_OTHERS;
            return EXTDATA_PATH + path.substr(4);
        }
        if (BasicAPI::flags & APIFLAG_FS_ACC_SAFE){
            if (path.starts_with("home:/")){
                g_BasicAPI_PathType = APIPATHTYPE_SAFE_SELF;
                return CYX::GetHomeFolder() + path.substr(5);
            }
            if (path.starts_with("shared:/")){
                g_BasicAPI_PathType = APIPATHTYPE_SAFE_SHARED;
                return HOMEFS_SHARED_PATH + path.substr(7);
            }
        }
        if (BasicAPI::flags & APIFLAG_FS_ACCESS_XREF){
            if (path.starts_with("data:/")){
                g_BasicAPI_PathType = APIPATHTYPE_SAFE_OTHERS;
                return HOMEFS_PATH + path.substr(5);
            }
        }
        if (BasicAPI::flags & APIFLAG_FS_ACCESS_SD){
            if (path.starts_with("sdmc:/")){
                g_BasicAPI_PathType = APIPATHTYPE_SDMC;
                return path.substr(5);
            }
        }
        return "!EACCESS_DENIED";
    }
    std::string basicapi__EvaluatePath16(const string16& path){
        std::string s="";
        Utils::ConvertUTF16ToUTF8(s, path);
        return basicapi__EvaluatePath(s);
    }

    void basicapi__ClearClipboard(){
        memset(CYX::editorInstance->clipboardData, 0, sizeof(CYX::editorInstance->clipboardData));
        CYX::editorInstance->clipboardLength = 0;
    }
    void basicapi__AppendData_T(const std::string& data){
        u32 off = (u32)CYX::editorInstance->clipboardData + CYX::editorInstance->clipboardLength * sizeof(u16);
        Process::WriteString(off, data, StringFormat::Utf16);
        CYX::editorInstance->clipboardLength += strlen_utf8(data);
    }
    void basicapi__AppendData_U(const string16& data){
        u32 off = (u32)CYX::editorInstance->clipboardData + CYX::editorInstance->clipboardLength * sizeof(u16);
        memcpy((void*)off, data.data(), data.size()*sizeof(u16));
        CYX::editorInstance->clipboardLength += data.size();
    }
    void basicapi__AppendData_V(void* data, u32 size){
        u32 off = (u32)CYX::editorInstance->clipboardData + CYX::editorInstance->clipboardLength * sizeof(u16);
        memcpy((void*)off, data, size);
        CYX::editorInstance->clipboardLength += (size+1)/2;
    }

    int BasicAPI::Func_FILES(BASICAPI_FUNCVARS) {
        if (!argc) {
            APIOUT(outv, "EMISSING_ARGS");
            return 1;
        }
        string16 name; int mode = 0;
        if (argc>1) mode = CYX::argGetInteger(argv+1);
        if (argv->data){
            CYX::argGetString(name, argv);
        } else {
            APIGETCLIP(name);
        }
        std::string fpath = basicapi__EvaluatePath16(name);
        name.clear();
        basicapi__ClearClipboard();
        if (fpath==""){
            APIOUT(outv, "EEMPTY_PATH");
            return 1;
        }
        if (fpath[0]=='!') {
            APIOUT(outv, fpath.substr(1));
            return 1;
        }
        Directory dir(fpath);
        StringVector vec; u32 s;
        s = dir.ListDirectories(vec);
        if (mode == 0) basicapi__AppendData_T(Utils::Format("%d",s));
        if (mode < 2) {
            for (u32 i=0; i<vec.size(); i++)
                basicapi__AppendData_T("\t/"+vec[i]);
        }
        vec.clear();
        s = dir.ListFiles(vec);
        if (mode == 0)
            basicapi__AppendData_T(Utils::Format("\t%d",s));
        if (mode==0 || mode==2){
            for (u32 i=0; i<vec.size(); i++)
                basicapi__AppendData_T("\t*"+vec[i]);
        }
        vec.clear();
        dir.Close();
        APIOUT_C(outv /* Use clipboard */);
        return 1;
    }

    int BasicAPI::Func_SETUP_CLIP(BASICAPI_FUNCVARS) {
        if (argc) {
            s32 mode = CYX::argGetInteger(argv);
            switch (mode)
            {
            case 0: case 1: case 2: case 3:
                CYX::editorInstance->clipboardLength = CYX::editorInstance->programSlot[mode].text_len;
                memcpy(CYX::editorInstance->clipboardData, CYX::editorInstance->programSlot[mode].text, 2049152);
                break;
            case 4: case 5: case 6: case 7:
                CYX::editorInstance->programSlot[mode - 4].text_len = CYX::editorInstance->clipboardLength;
                memcpy(CYX::editorInstance->programSlot[mode - 4].text, CYX::editorInstance->clipboardData, 2049152);
                basicapi__ClearClipboard();
                break;
            case  8: case  9: case 10: case 11:
            case 12: case 13: case 14: case 15:
                CYX::editorInstance->clipboardLength = CYX::editorInstance->undo[mode-8].len;
                memcpy(CYX::editorInstance->clipboardData, CYX::editorInstance->undo[mode-8].data, 8204);
                break;
            default:
                basicapi__ClearClipboard();
                break;
            }
        } else {
            basicapi__ClearClipboard();
        }
        return 0;
    }

    int BasicAPI::FileRead(BasicAPI::FileStruct f, u16* buf, u32* len, u32 size, u32 bytesLeft){
        if (!f.f) return File::UNEXPECTED_ERROR;
        if (f.flags & APIFSTRUCT_UTF16) size *= 2;
        u32 l = (size > bytesLeft) ? bytesLeft : size;
        *len = l;
        if (f.flags & APIFSTRUCT_UTF16) *len = l/2;
        return f.f->Read(buf, l);
    }
    int BasicAPI::FileWrite(BasicAPI::FileStruct f, u16* buf, u32 size){
        if (!f.f) return File::UNEXPECTED_ERROR;
        if (f.flags & APIFSTRUCT_UTF16) size *= 2;
        return f.f->Write(buf, size);
    }

    int BasicAPI::Func_FOPEN(BASICAPI_FUNCVARS){
        if (argc < 2){
            APIOUT_I(outv, -511);
            return 1;
        }
        if (Files.size() >= BASICAPI_FILESTACK_LIMIT){
            APIOUT_I(outv, -128);
            return 1;
        }
        string16 argTmp;
        std::string path, mode$;
        u32 fflags = APIFSTRUCT_UTF16; int res;
        s32 mode = CYX::argGetInteger(argv+1);
        CYX::argGetString(argTmp, argv);
        if (!argTmp.size()) APIGETCLIP(argTmp);
        Utils::ConvertUTF16ToUTF8(path, argTmp);
        path = basicapi__EvaluatePath(path);
        if (path==""){
            APIOUT_I(outv, -256);
            APIOUT(outv+1, "EEMPTY_PATH");
            return 1;
        }
        if (path[0]=='!') {
            APIOUT_I(outv, -257);
            APIOUT(outv+1, path.substr(1));
            return 1;
        }
        argTmp.clear();
        if (mode == 0){
            CYX::argGetString(argTmp, argv+1);
            Utils::ConvertUTF16ToUTF8(mode$, argTmp); strupper(mode$);
            mode = File::SYNC;
            if (mode$.find('R')!=-1) mode |= (File::READ);
            if (mode$.find('W')!=-1) mode |= (File::WRITE | File::TRUNCATE | File::CREATE);
            if (mode$.find('A')!=-1) mode |= (File::WRITE | File::APPEND);
            if (mode$.find('C')!=-1) mode |= (File::WRITE | File::CREATE);
            if (mode$.find('X')!=-1) mode |= (File::WRITE);
            if (mode$.find('+')!=-1) mode |= (File::RWC);
            if (mode$.find('B')!=-1) fflags = APIFSTRUCT_ANSI;
        }
        if (!basicapi__HavePermission(mode & File::WRITE)){
            APIOUT_I(outv, File::UNEXPECTED_ERROR-1);
            APIOUT(outv+1, "EMISSING_PERM");
            return 1;
        }
        if (mode == 0){
            APIOUT_I(outv, File::INVALID_MODE);
            APIOUT(outv+1, "EINVALID_MODE");
            return 1;
        }
        FileStruct f;
        f.flags = fflags;
        f.handle = handleIDCounter++;
        if (handleIDCounter > BASICAPI_HANDLE_END) handleIDCounter = BASICAPI_HANDLE_START;
        f.f = new File(path, mode);
        APIOUT_I(outv, File::UNEXPECTED_ERROR);
        if (!f.f) return 1;
        if (!f.f->IsOpen()){
            APIOUT_I(outv, File::UNEXPECTED_ERROR-2);
            f.f->Close();
            delete f.f;
            return 1;
        }
        Files.push_back(f);
        APIOUT_I(outv, f.handle);
        return 0;
    }
    int BasicAPI::Func_FMODE(BASICAPI_FUNCVARS){
        if (argc < 2){
            APIOUT(outv, (s32)-1);
            return 1;
        }
        u32 handle = CYX::argGetInteger(argv);
        u32 fflags = APIFSTRUCT_UTF16;
        string16 flagstr;
        u32 mode = CYX::argGetInteger(argv+1);
        if (mode == 1) fflags = APIFSTRUCT_UTF16;
        if (mode == 2) fflags = APIFSTRUCT_ANSI;
        if (!handle){
            APIOUT(outv, (s32)-64);
            return 1;
        }
        for (int i=0; i<Files.size(); i++){
            if (Files[i].handle == handle){
                Files[i].flags = fflags;
                APIOUT(outv, (s32)0);
                return 0;
            }
        }
        APIOUT(outv, (s32)-64);
        return 1;
    }
    int BasicAPI::Func_FREAD(BASICAPI_FUNCVARS){
        if (!argc){
            APIOUT(outv, "EMISSING_ARGS");
            return 1;
        }
        s32 handle = CYX::argGetInteger(argv);
        s32 length = 272144;
        if (argc > 1) length = CYX::argGetInteger(argv+1);
        if (length > 272144) length = 272144;
        for (int i=0; i<Files.size(); i++){
            if (Files[i].handle == handle){
                basicapi__ClearClipboard();
                u32 t=(Files[i].f->GetSize() - Files[i].f->Tell());
                int res = FileRead(Files[i], CYX::editorInstance->clipboardData, &CYX::editorInstance->clipboardLength, length, t);
                if (Files[i].flags & APIFSTRUCT_ANSI){
                    strncpyu8u16((u8*)CYX::editorInstance->clipboardData, CYX::editorInstance->clipboardData, CYX::editorInstance->clipboardLength);
                }
                if (res)
                    APIOUT(outv, Utils::Format("%d",res));
                else
                    APIOUT_C(outv);
                return 0;
            }
        }
        APIOUT(outv, "EBAD_HANDLE");
        return 1;
    }
    int BasicAPI::Func_FWRITE(BASICAPI_FUNCVARS){
        if (!argc){
            APIOUT(outv, "EMISSING_ARGS");
            return 1;
        }
        u32 handle = CYX::argGetInteger(argv);
        bool allowANSI = false;
        u16* stringData = NULL; u32 stringLength;
        if (argc>1)
            CYX::argGetString(&stringData, &stringLength, argv+1);
        if (!stringData || !stringLength){
            allowANSI = true;
            stringData = CYX::editorInstance->clipboardData;
            stringLength = CYX::editorInstance->clipboardLength;
        }
        for (int i=0; i<Files.size(); i++){
            if (Files[i].handle == handle){
                if (allowANSI && Files[i].flags & APIFSTRUCT_ANSI){
                    strncpyu16u8(stringData, (u8*)stringData, stringLength);
                    if (stringData == CYX::editorInstance->clipboardData)
                        CYX::editorInstance->clipboardLength /= 2;
                } 
                int res = FileWrite(Files[i], stringData, stringLength);
                APIOUT(outv, Utils::Format("%d",res));
                return 0;
            }
        }
        APIOUT(outv, "EBAD_HANDLE");
        return 1;
    }
    int BasicAPI::Func_FSEEK(BASICAPI_FUNCVARS){
        if (!argc){
            APIOUT(outv, (s32)-511);
            return 1;
        }
        u32 handle = CYX::argGetInteger(argv);
        bool doSeek = false;
        s32 seek = 0, whence = 0;
        if (argc>1){
            doSeek = true;
            seek = CYX::argGetInteger(argv+1);
        }
        if (argc>2)
            whence = CYX::argGetInteger(argv+2);
        for (int i=0; i<Files.size(); i++){
            if (Files[i].handle == handle){
                if (doSeek) Files[i].f->Seek(seek, (File::SeekPos)whence);
                seek = Files[i].f->Tell();
                APIOUT(outv, seek);
                return 0;
            }
        }
        APIOUT(outv, (s32)-64);
        return 1;
    }
    int BasicAPI::Func_FCLOSE(BASICAPI_FUNCVARS){
        if (!argc){
            APIOUT(outv, (s32)-1);
            return 1;
        }
        u32 handle = CYX::argGetInteger(argv);
        for (int i=0; i<Files.size(); i++){
            if (Files[i].handle == handle){
                Files[i].f->Close();
                Files.erase(Files.begin()+i);
                APIOUT(outv, (s32)0);
                return 0;
            }
        }
        APIOUT(outv, (s32)-64);
        return 1;
    }
    int BasicAPI::Func_MKFILE(BASICAPI_FUNCVARS){
        if (!argc) {
            APIOUT(outv, (s32)-1);
            return 1;
        }
        string16 name;
        if (argv->data){
            CYX::argGetString(name, argv);
        } else {
            APIGETCLIP(name);
        }
        std::string fpath = basicapi__EvaluatePath16(name);
        name.clear();
        if (!basicapi__HavePermission(true)){
            APIOUT(outv, (s32)File::UNEXPECTED_ERROR-1);
            return 1;
        }
        File::Create(fpath);
        APIOUT(outv, (s32)0);
        return 0;
    }
    int BasicAPI::Func_RMFILE(BASICAPI_FUNCVARS){
        if (!argc) {
            APIOUT(outv, (s32)-1);
            return 1;
        }
        string16 name;
        if (argv->data){
            CYX::argGetString(name, argv);
        } else {
            APIGETCLIP(name);
        }
        std::string fpath = basicapi__EvaluatePath16(name);
        name.clear();
        if (!basicapi__HavePermission(true)){
            APIOUT(outv, (s32)File::UNEXPECTED_ERROR-1);
            return 1;
        }
        File::Remove(fpath);
        APIOUT(outv, (s32)0);
        return 0;
    }
    int BasicAPI::Func_MVFILE(BASICAPI_FUNCVARS){
        if (!argc) {
            APIOUT(outv, (s32)-1);
            return 1;
        }
        string16 name;
        std::string fpath1, fpath2;
        if (argv->data){
            CYX::argGetString(name, argv);
            Utils::ConvertUTF16ToUTF8(fpath1, name);
            name.clear();
            CYX::argGetString(name, argv+1);
            Utils::ConvertUTF16ToUTF8(fpath2, name);
            name.clear();
        } else {
            APIGETCLIP(name);
            Utils::ConvertUTF16ToUTF8(fpath1, name);
            name.clear();
            if (fpath1.find('\t')<0){
                APIOUT(outv, (s32)-2);
                return 1;
            }
            fpath2 = fpath1.substr(fpath1.find('\t')+1);
            fpath1 = fpath1.substr(0, fpath1.find('\t'));
        }
        fpath1 = basicapi__EvaluatePath(fpath1);
        if (!basicapi__HavePermission(true)){
            APIOUT(outv, (s32)File::UNEXPECTED_ERROR-1);
            return 1;
        }
        fpath2 = basicapi__EvaluatePath(fpath2);
        if (!basicapi__HavePermission(true)){
            APIOUT(outv, (s32)File::UNEXPECTED_ERROR-1);
            return 1;
        }
        if (fpath1=="" || fpath2==""){
            APIOUT(outv, (s32)-3);
            return 1;
        }
        if (fpath1.starts_with('!')) {
            APIOUT(outv, fpath1.substr(1));
            return 1;
        }
        if (fpath2.starts_with('!')) {
            APIOUT(outv, fpath2.substr(1));
            return 1;
        }
        File::Rename(fpath1, fpath2);
        APIOUT(outv, (s32)0);
        return 0;
    }
    int BasicAPI::Func_MKDIR(BASICAPI_FUNCVARS){
        if (!argc) {
            APIOUT(outv, (s32)-1);
            return 1;
        }
        string16 name;
        if (argv->data){
            CYX::argGetString(name, argv);
        } else {
            APIGETCLIP(name);
        }
        std::string fpath = basicapi__EvaluatePath16(name);
        name.clear();
        if (!basicapi__HavePermission(true)){
            APIOUT(outv, (s32)File::UNEXPECTED_ERROR-1);
            return 1;
        }
        Directory::Create(fpath);
        APIOUT(outv, (s32)0);
        return 0;
    }
    int BasicAPI::Func_RMDIR(BASICAPI_FUNCVARS){
        if (!argc) {
            APIOUT(outv, (s32)-1);
            return 1;
        }
        string16 name;
        if (argv->data){
            CYX::argGetString(name, argv);
        } else {
            APIGETCLIP(name);
        }
        std::string fpath = basicapi__EvaluatePath16(name);
        name.clear();
        if (!basicapi__HavePermission(true)){
            APIOUT(outv, (s32)File::UNEXPECTED_ERROR-1);
            return 1;
        }
        Directory::Remove(fpath);
        APIOUT(outv, (s32)0);
        return 0;
    }
    int BasicAPI::Func_MVDIR(BASICAPI_FUNCVARS){
        if (!argc) {
            APIOUT(outv, (s32)-1);
            return 1;
        }
        string16 name;
        std::string fpath1, fpath2;
        if (argv->data){
            CYX::argGetString(name, argv);
            Utils::ConvertUTF16ToUTF8(fpath1, name);
            name.clear();
            CYX::argGetString(name, argv+1);
            Utils::ConvertUTF16ToUTF8(fpath2, name);
            name.clear();
        } else {
            APIGETCLIP(name);
            Utils::ConvertUTF16ToUTF8(fpath1, name);
            name.clear();
            if (fpath1.find('\t')<0){
                APIOUT(outv, (s32)-2);
                return 1;
            }
            fpath2 = fpath1.substr(fpath1.find('\t')+1);
            fpath1 = fpath1.substr(0, fpath1.find('\t'));
        }
        fpath1 = basicapi__EvaluatePath(fpath1);
        if (!basicapi__HavePermission(true)){
            APIOUT(outv, (s32)File::UNEXPECTED_ERROR-1);
            return 1;
        }
        fpath2 = basicapi__EvaluatePath(fpath2);
        if (!basicapi__HavePermission(true)){
            APIOUT(outv, (s32)File::UNEXPECTED_ERROR-1);
            return 1;
        }
        if (fpath1=="" || fpath2==""){
            APIOUT(outv, (s32)-3);
            return 1;
        }
        if (fpath1.starts_with('!')) {
            APIOUT(outv, fpath1.substr(1));
            return 1;
        }
        if (fpath2.starts_with('!')) {
            APIOUT(outv, fpath2.substr(1));
            return 1;
        }
        Directory::Rename(fpath1, fpath2);
        APIOUT(outv, (s32)0);
        return 0;
    }
    int BasicAPI::Func_CHKDIR(BASICAPI_FUNCVARS){
        if (!argc) {
            APIOUT(outv, (s32)-1); return 1;
        }
        string16 name;
        if (argv->data){
            CYX::argGetString(name, argv);
        } else APIGETCLIP(name);
        std::string fpath = basicapi__EvaluatePath16(name);
        name.clear();
        if (!basicapi__HavePermission(false)){
            APIOUT(outv, (s32)-1); return 1;
        }
        APIOUT(outv, (s32)Directory::IsExists(fpath));
        return 0;
    }
    int BasicAPI::Func_CHKFILE(BASICAPI_FUNCVARS){
        if (!argc) {
            APIOUT(outv, (s32)-1); return 1;
        }
        string16 name;
        if (argv->data){
            CYX::argGetString(name, argv);
        } else APIGETCLIP(name);
        std::string fpath = basicapi__EvaluatePath16(name);
        name.clear();
        if (!basicapi__HavePermission(false)){
            APIOUT(outv, (s32)-1); return 1;
        }
        APIOUT(outv, (s32)File::Exists(fpath));
        return 0;
    }
    int BasicAPI::Func_INIT(BASICAPI_FUNCVARS){
        Cleanup();
        File f;
        if (File::Exists(CLIPBOARDCACHE_PATH))
            File::Remove(CLIPBOARDCACHE_PATH);
        if (File::Open(f, CLIPBOARDCACHE_PATH, File::RWC | File::TRUNCATE)==0){
            f.Write(&CYX::editorInstance->clipboardLength, sizeof(u32));
            f.Dump((u32)CYX::editorInstance->clipboardData, sizeof(CYX::editorInstance->clipboardData));
        }
        f.Close();
        if (!Directory::IsExists(HOMEFS_PATH)) Directory::Create(HOMEFS_PATH);
        if (!Directory::IsExists(HOMEFS_SHARED_PATH)) Directory::Create(HOMEFS_SHARED_PATH);
        return 0;
    }
    int BasicAPI::Func_EXIT(BASICAPI_FUNCVARS){
        Cleanup();
        File f;
        if (File::Open(f, CLIPBOARDCACHE_PATH, File::READ)==0){
            f.Read(&CYX::editorInstance->clipboardLength, sizeof(u32));
            f.Inject((u32)CYX::editorInstance->clipboardData, sizeof(CYX::editorInstance->clipboardData));
        }
        f.Close();
        CYX::SaveProjectSettings();
        return 0;
    }
    int BasicAPI::Func_CRASH(BASICAPI_FUNCVARS) {
        setCTRPFConfirm(CYXCONFIRM_BASICAPI_TOHOME, 0);
        if(!waitCTRPFConfirm()) return 3;
        OnProcessExit();
        Process::ReturnToHomeMenu();
        return 3;
    }
    int BasicAPI::Func_CFGSET(BASICAPI_FUNCVARS) {
        if (argc < 2) {
            APIOUT(outv, "EMISSING_ARGS");
            return 1;
        }
        basicapi__ClearClipboard();
        std::string arg1 = ""; s32 val = CYX::argGetInteger(argv+1);
        string16 arg1_16; CYX::argGetString(arg1_16, argv);
        Utils::ConvertUTF16ToUTF8(arg1, arg1_16); arg1_16.clear(); strupper(arg1);
        bool condition1 = false;

        if (arg1 == "SYSINFO"){
            flags &= ~APIFLAG_READ_SYSINFO;
            if (val) flags |= APIFLAG_READ_SYSINFO;
            return 0;
        }
        if (arg1 == "FWINFO"){
            flags &= ~APIFLAG_READ_FWINFO;
            if (val) flags |= APIFLAG_READ_FWINFO;
            return 0;
        }
        if (arg1 == "HWINFO"){
            flags &= ~APIFLAG_READ_HWINFO;
            if (val) flags |= APIFLAG_READ_HWINFO;
            return 0;
        }
        if (arg1 == "SAFEDIR"){
            flags &= ~APIFLAG_FS_ACC_SAFE;
            if (val) flags |= APIFLAG_FS_ACC_SAFE;
            return 0;
        }
        if (arg1 == "PRJ_ACCESS"){
            condition1 = flags & APIFLAG_FS_ACC_XREF_RW;
            flags &= ~APIFLAG_FS_ACCESS_XREF;
            if (val & 1) flags |= APIFLAG_FS_ACC_XREF_RO;
            if (val & 2) {
                flags |= APIFLAG_FS_ACC_XREF_RO;
                if (!condition1){
                    setCTRPFConfirm(CYXCONFIRM_BASICAPI_XREF_RW, 0);
                    condition1 = waitCTRPFConfirm();
                }
                if (condition1) flags |= APIFLAG_FS_ACC_XREF_RW;
            }
            return 0;
        }
        if (arg1 == "SD_ACCESS"){
            condition1 = flags & APIFLAG_FS_ACC_SD_RW;
            flags &= ~APIFLAG_FS_ACCESS_SD;
            if (val & 1) flags |= APIFLAG_FS_ACC_SD_RO;
            if (val & 2) {
                flags |= APIFLAG_FS_ACC_SD_RO;
                if (!condition1){
                    setCTRPFConfirm(CYXCONFIRM_BASICAPI_SD_RW, 0);
                    condition1 = waitCTRPFConfirm();
                }
                if (condition1) flags |= APIFLAG_FS_ACC_SD_RW;
            }
            return 0;
        }
        return 0;
    }

    int BasicAPI::Func_CFGGET(BASICAPI_FUNCVARS) {
        if (!argc) {
            APIOUT(outv, "EMISSING_ARGS");
            return 1;
        }
        basicapi__ClearClipboard();
        std::string arg1 = "";
        string16 arg1_16; CYX::argGetString(arg1_16, argv);
        Utils::ConvertUTF16ToUTF8(arg1, arg1_16); arg1_16.clear();
        std::string data = ""; strupper(arg1);

        if (arg1 == "SYSINFO"){
            APIOUT_I(outv, (bool)(flags & APIFLAG_READ_SYSINFO));
            return 0;
        }
        if (arg1 == "FWINFO"){
            APIOUT_I(outv, (bool)(flags & APIFLAG_READ_FWINFO));
            return 0;
        }
        if (arg1 == "HWINFO"){
            APIOUT_I(outv, (bool)(flags & APIFLAG_READ_HWINFO));
            return 0;
        }
        if (arg1 == "SAFEDIR"){
            APIOUT_I(outv, (bool)(flags & APIFLAG_FS_ACC_SAFE));
            return 0;
        }
        if (arg1 == "PRJ_ACCESS"){
            APIOUT_I(outv, ((flags & APIFLAG_FS_ACCESS_XREF)>>APIFLAG_BIT_FS_ACC_XREF_RO));
            return 0;
        }
        if (arg1 == "SD_ACCESS"){
            APIOUT_I(outv, ((flags & APIFLAG_FS_ACCESS_SD)>>APIFLAG_BIT_FS_ACC_SD_RO));
            return 0;
        }
        if (arg1 == "DIRECTMODE"){
            APIOUT_I(outv, CYX::mirror.isDirectMode);
            return 0;
        }
        if (arg1 == "SDMCFREE"){
            APIOUT(outv, (double)CYX::sdmcFreeSpace);
            return 0;
        }
        if (arg1 == "SDMCTOTAL"){
            APIOUT(outv, (double)CYX::sdmcTotalSpace);
            return 0;
        }
        if (arg1 == "SDMCFREE_C"){
            APIOUT_I(outv, g_sdmcArcRes.freeClusters);
            return 0;
        }
        if (arg1 == "SDMCCLUSTER"){
            APIOUT_I(outv, g_sdmcArcRes.clusterSize);
            return 0;
        }
        if (arg1 == "SDMCSECTOR"){
            APIOUT_I(outv, g_sdmcArcRes.sectorSize);
            return 0;
        }
        if (arg1 == "SDMCTOTAL_C"){
            APIOUT_I(outv, g_sdmcArcRes.totalClusters);
            return 0;
        }
        if (flags & APIFLAG_READ_HWINFO){
            if (arg1 == "3DSLIDER"){
                APIOUT_I(outv, osGet3DSliderState());
                return 0;
            }
            if (arg1 == "HEADSET"){
                APIOUT_I(outv, (s32)osIsHeadsetConnected());
                return 0;
            }
            if (arg1 == "VOLSLIDER"){
                if (System::IsCitra()) {
                    APIOUT_I(outv, (s32)63);
                    return 0;
                }
                mcuHwcInit();
                u8 out;
                MCUHWC_GetSoundSliderLevel(&out);
                mcuHwcExit();
                APIOUT_I(outv, out);
                return 0;
            }
            if (arg1 == "SIGNAL"){
                if (System::IsCitra()) {
                    APIOUT_I(outv, 0);
                    return 0;
                }
                APIOUT_I(outv, osGetWifiStrength());
                return 0;
            }
            if (arg1 == "BAT_LEVEL"){
                if (System::IsCitra()) {
                    APIOUT_I(outv, 100);
                    return 0;
                }
                mcuHwcInit();
                u8 out;
                MCUHWC_GetBatteryLevel(&out);
                mcuHwcExit();
                APIOUT_I(outv, out);
                return 0;
            }
            if (arg1 == "CHARGE"){
                if (System::IsCitra()) {
                    APIOUT_I(outv, 1);
                    return 0;
                }
                mcuHwcInit();
                u8 out;
                MCUHWC_ReadRegister(0xf,&out,1);
                mcuHwcExit();
                APIOUT_I(outv, ((out >> 4) & 1));
                return 0;
            }
            if (arg1 == "CANSLEEP"){
                if (System::IsCitra()) {
                    APIOUT_I(outv, 0);
                    return 0;
                }
                APIOUT_I(outv, mcuIsSleepEnabled());
                return 0;
            }
        }
        if (BasicAPI::flags & APIFLAG_READ_SYSINFO){
            if (arg1 == "LANG"){
                APIOUT_I(outv, System::GetSystemLanguage());
                return 0;
            } else if (arg1 == "LANG$"){
                switch ((int)System::GetSystemLanguage()){
                case  0: data="JPN"; break; case  1: data="ENG"; break;
                case  2: data="FRA"; break; case  3: data="DEU"; break;
                case  4: data="ITA"; break; case  5: data="SPA"; break;
                case  6: data="CHN"; break; case  7: data="KOR"; break;
                case  8: data="NED"; break; case  9: data="POR"; break;
                case 10: data="RUS"; break; case 11: data="TWN"; break;
                };
                APIOUT(outv, data);
                return 0;
            } else if (arg1 == "CITRA"){
                APIOUT_I(outv, System::IsCitra());
                return 0;
            }
            if (arg1 == "REGION$"){
                data.clear(); data = g_osNVer.region;
                APIOUT(outv, data);
                return 0;
            }
            if (arg1 == "REGION"){
                data = "JUE_CKT";
                APIOUT_I(outv, data.find(g_osNVer.region));
                return 0;
            }
        }
        if (arg1 == "VERSION"){
            APIOUT_I(outv, VER_INTEGER);
            return 0;
        }
        if (arg1 == "VERSION$"){
            APIOUT(outv, Utils::Format(STRING_VERSION "/" STRING_BUILD));
            return 0;
        }
        if (arg1 == "PTCVER"){
            APIOUT_I(outv, CYX::currentVersion);
            return 0;
        }
        if (arg1 == "PTCREG$"){
            APIOUT(outv, g_regionString);
            return 0;
        }
        if (arg1 == "PTCREG"){
            APIOUT_I(outv, g_region);
            return 0;
        }
        if (flags & APIFLAG_READ_FWINFO){
            if (arg1 == "SYSVER$"){
                char str[32]={0};
                APIOUT(outv, g_osSysVer);
                return 0;
            }
            if (arg1 == "FIRMVER"){
                APIOUT_I(outv, g_osFirmVer);
                return 0;
            }
            if (arg1 == "KERNELVER"){
                APIOUT_I(outv, g_osKernelVer);
                return 0;
            }
        }
        if (arg1 == "BUILD$"){
            APIOUT(outv, BUILD_DATE);
            return 0;
        }
        if (arg1 == "COMMIT$"){
            APIOUT(outv, COMMIT_HASH);
            return 0;
        }
        APIOUT_I(outv, 0);
        return 1;
    }

    int BasicAPI::Func_OSD(BASICAPI_FUNCVARS) {
        int type; std::string data="", chunk;
        u32 index = 0, index2;
        for (u32 i=0; i<argc; i++) {
            string16 st16; CYX::argGetString(st16, argv+i);
            std::string str = "";
            Utils::ConvertUTF16ToUTF8(str, st16); st16.clear();
            if (!i && str.size()>0){
                if (str[0] == 'W'){
                    type = 1; continue;
                } else if (str[0] == 'N'){
                    type = 2; continue;
                } else if (str[0] == 'E'){
                    type = 3; continue;
                } else if (str[0] == 'S'){
                    type = 4; continue;
                } else if (str[0] == 'M'){
                    type = 0; continue;
                }
            } else if (i) {
                data += str + "\n";
            }
        }
        if (!argc) return 0;
        if (argc==1) CYX::UTF16toUTF8(data, CYX::editorInstance->clipboardData, CYX::editorInstance->clipboardLength);
        while(true){
            if (index >= data.size()) break;
            index2 = data.find('\n', index);
            if (index2 < 0) index2 = 0xFFFFFFF;
            if (index2 > index+64) index2 = index + 64;
            chunk = data.substr(index, index2-index);
            switch (type){
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
    int BasicAPI::Func_FILLGRP(BASICAPI_FUNCVARS){
        if (argc < 2) {
            APIOUT(outv, "EMISSING_ARGS");
            return 1;
        }
        s32 num=CYX::argGetInteger(argv);
        u32 color=CYX::argGetInteger(argv+1);
        u8* colorv = (u8*)&color;

        u16* target = NULL;
        u16 color16 = RGBA8_to_5551(colorv[2],colorv[1],colorv[0],colorv[3]);

        if (num == -1){
            target = CYX::GraphicPage->font.dispBuf;
        } else if (num>=0&&num<6) {
            target = CYX::GraphicPage->grp[num].dispBuf;
        }
        if (!target) return 0;

        u32 i=0;
        while (i < 262144) target[i++] = color16;

        return 0;
    }
    int BasicAPI::Func_SYSGCOPY(BASICAPI_FUNCVARS){
        if (argc < 2) {
            APIOUT(outv, "EMISSING_ARGS");
            return 1;
        }
        s32 mode=CYX::argGetInteger(argv);
        s32 slot=CYX::argGetInteger(argv+1);
        BASICGraphicPage* target = NULL;

        if (slot == -1){
            target = &CYX::GraphicPage->font;
        } else if (slot>=0&&slot<6) {
            target = &CYX::GraphicPage->grp[slot];
        }
        if (!target) return 0;

        if (mode){
            memcpy(CYX::GraphicPage->system.dispBuf, target->workBuf, 524288);
        } else {
            memcpy(target->dispBuf, CYX::GraphicPage->system.dispBuf, 524288);
            memcpy(target->workBuf, CYX::GraphicPage->system.dispBuf, 524288);
        }

        return 0;
    }
    int BasicAPI::Func_UNIXTIME(BASICAPI_FUNCVARS){
        APIOUT_I(outv, osGetUnixTime());
        return 0;
    }
    int BasicAPI::Func_VALIDATE(BASICAPI_FUNCVARS){
        if (!argc) {
            APIOUT_I(outv, -1);
            return 1;
        }
        string16 name;
        if (argv->data){
            CYX::argGetString(name, argv);
        } else {
            APIGETCLIP(name);
        }
        u32 doFix = 0;
        if (argc > 1) doFix = CYX::argGetInteger(argv+1);
        std::string fpath = basicapi__EvaluatePath16(name);
        name.clear();
        if (!basicapi__HavePermission(true)){
            APIOUT_I(outv, File::UNEXPECTED_ERROR-1);
            return 1;
        }
        File f;
        if ((File::Open(f, fpath, File::RW))){
            APIOUT_I(outv, -1);
            return 1;
        }
        #define BUFFER_SIZE 262144
        u8 digest[20], oldDigest[20];
        void* buf = new u8[BUFFER_SIZE];
        u32 size, size2, chunk;
        size = size2 = f.GetSize()-20;
        f.Seek(-20, File::END);
        if (f.Read(oldDigest, 20)){
            APIOUT_I(outv, 0);
            f.Close();
            return 1;
        }
        f.Seek(0, File::SET);

        SHA1_HMAC::CTX hmacCtx;
        SHA1_HMAC::Init(&hmacCtx, (u8*)Hooks::offsets.serverHMACKey, 64);
        while (size){
            chunk = size > BUFFER_SIZE ? BUFFER_SIZE : size;
            if (f.Read(buf, chunk)){
                f.Close();
                APIOUT_I(outv, -1);
                return 1;
            }
            SHA1_HMAC::Update(&hmacCtx, (u8*)buf, chunk);
            size -= chunk;
        }
        ::operator delete(buf);
        SHA1_HMAC::Final(digest, &hmacCtx);
        if (memcmp(digest, oldDigest, 20)) {
            if (doFix) {
                f.Seek(-20, File::END);
                if (f.Write(digest, 20)){
                    APIOUT_I(outv, -1);
                } else {
                    APIOUT_I(outv, 1);
                }
            } else
                APIOUT_I(outv, 1);
        } else {
            APIOUT_I(outv, 0);
        }
        f.Close();
        #undef BUFFER_SIZE
        return 0;
    }
    u32 basicapi__getDataPtr(){
        return (u32)CYX::editorInstance->clipboardData;
    }
    int BasicAPI::Parse(BASICAPI_FUNCVARS) {
        if (!Entries.size()) {
            APIOUT(outv, "Error: Bad API state");
            return 0;
        }
        std::string str = ""; string16 st2;
        CYX::SetAPIUse(true);
        if (argc) {
            CYX::argGetString(st2, argv);
            Utils::ConvertUTF16ToUTF8(str, st2);
            strupper(str);
            APIOUT_I(outv, 0);
            if (str == "HELLO") {
                APIOUT(outv, "Hello!");
                return 0;
            }
            for (auto i : Entries) {
                if (str == i.id) return i.func(++argv, argc-1, outv, outc);
            }
            APIOUT_I(outv, -1);
            return 1;
        }
        return 0;
    }

    void BasicAPI::AddEntry(const char* id, BasicAPIFunction func) {
        Entries.push_back({(char*)id, func});
    }

    void BasicAPI::Initialize(){
        Entries.clear();
        AddEntry("CFGGET", Func_CFGGET);
        AddEntry("FILES", Func_FILES);
        AddEntry("FREAD", Func_FREAD);
        AddEntry("FWRITE", Func_FWRITE);
        AddEntry("OSD", Func_OSD);
        AddEntry("UNIXTIME", Func_UNIXTIME);
        AddEntry("FSEEK", Func_FSEEK);
        AddEntry("FMODE", Func_FMODE);
        AddEntry("FOPEN", Func_FOPEN);
        AddEntry("FCLOSE", Func_FCLOSE);
        AddEntry("CFGSET", Func_CFGSET);
        AddEntry("MKFILE", Func_MKFILE);
        AddEntry("MKDIR", Func_MKDIR);
        AddEntry("CHKFILE", Func_CHKFILE);
        AddEntry("CHKDIR", Func_CHKDIR);
        AddEntry("RMFILE", Func_RMFILE);
        AddEntry("RMDIR", Func_RMDIR);
        AddEntry("MVFILE", Func_MVFILE);
        AddEntry("MVDIR", Func_MVDIR);
        AddEntry("SETUP_CLIP", Func_SETUP_CLIP);
        AddEntry("FILLGRP", Func_FILLGRP);
        AddEntry("SYSGCOPY", Func_SYSGCOPY);
        AddEntry("INIT", Func_INIT);
        AddEntry("EXIT", Func_EXIT);
        AddEntry("CRASH", Func_CRASH);
        AddEntry("VALIDATE", Func_VALIDATE);
    }
    void BasicAPI::Finalize(){
        for (int i=0; i<Files.size(); i++){
            if (!Files[i].f) continue;
            Files[i].f->Close();
            delete Files[i].f;
        }
        Files.clear();
    }
    void BasicAPI::Cleanup(){
        for (int i=0; i<Files.size(); i++){
            if (!Files[i].f) continue;
            Files[i].f->Close();
            delete Files[i].f;
        }
        Files.clear();
    }
    void BasicAPI::MenuTick(){
        // IDK
    }
}
