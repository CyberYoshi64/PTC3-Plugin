#include "BasicAPI.hpp"

namespace CTRPluginFramework {
    std::vector<BasicAPI_Entry_s> BasicAPI::Entries = {};
    u32 BasicAPI::handleIDCounter = BASICAPI_HANDLE_START;
    std::vector<BasicAPI_QueueEntry> BasicAPI::Queue = {};
    u32 BasicAPI::queueOffset = 0;
    u32 BasicAPI::flags = APIFLAG_DEFAULT;

    std::string basicapi__EvaluatePath(const std::string& path){
        std::string actPrj = "";
        CYX::UTF16toUTF8(actPrj, CYX::activeProject->currentProject);
        if (!path.size()) return "";

        if (BasicAPI::flags & APIFLAG_FS_ACC_SAFE){
            if (path.starts_with("home:/")){
                return Utils::Format("%s/%s/", HOMEFS_PATH, actPrj.c_str()) + path.substr(6);
            }
        }
        if (BasicAPI::flags & APIFLAG_FS_ACCESS_XREF){
            if (path.starts_with("data:/")){
                return Utils::Format("%s/", HOMEFS_PATH) + path.substr(6);
            }
        }
        if (BasicAPI::flags & APIFLAG_FS_ACCESS_TOP){
            if (path.starts_with("plg:/")){
                return Utils::Format("%s/", TOP_DIR) + path.substr(5);
            }
        }
        if (BasicAPI::flags & APIFLAG_FS_ACCESS_SD){
            if (path.starts_with("sdmc:")){
                return path.substr(5);
            }
        }
        OSD::Notify("API::EvaluatePath: Error", Color::White, Color::Red);
        OSD::Notify("The following path is not allowed on this project", Color::White, Color::Maroon);
        OSD::Notify(path, Color::White, Color::Maroon);
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
        u32 off = (u32)&CYX::editorInstance->clipboardData + CYX::editorInstance->clipboardLength;
        Process::WriteString(off, data, StringFormat::Utf16);
        CYX::editorInstance->clipboardLength += strlen_utf8(data);
    }
    void basicapi__AppendData_U(const string16& data){
        u32 off = (u32)&CYX::editorInstance->clipboardData + CYX::editorInstance->clipboardLength;
        memcpy((void*)off, data.data(), data.size()*sizeof(u16));
        CYX::editorInstance->clipboardLength += data.size();
    }
    void basicapi__AppendData_V(void* data, u32 size){
        u32 off = (u32)&CYX::editorInstance->clipboardData + CYX::editorInstance->clipboardLength;
        memcpy((void*)off, data, size);
        CYX::editorInstance->clipboardLength += (size+1)/2;
    }

    int BasicAPI::Func_FILES(BASICGenericVariable* argv, u32 argc) {
        if (!argc) {
            OSD::Notify("API::FILES: Missing arguments; need 2", Color::White, Color::Maroon);
            return 1;
        }
        string16 name;
        if (argv->data){
            CYX::argGetString(name, argv);
        } else {
            name.append(CYX::editorInstance->clipboardData, CYX::editorInstance->clipboardLength);
        }
        std::string fpath = basicapi__EvaluatePath16(name);
        name.clear();
        basicapi__ClearClipboard();
        if (fpath=="") return 0;
        if (fpath[0]=='!') {
            CYX::CYXAPI_Out(fpath.substr(1).c_str());
            return 0;
        }
        Directory dir(fpath);
        StringVector vec;
        basicapi__AppendData_T(Utils::Format("%d",dir.ListDirectories(vec)));
        for (u32 i=0; i<vec.size(); i++)
            basicapi__AppendData_T("\t/"+vec[i]);
        vec.clear();
        basicapi__AppendData_T(Utils::Format("\t%d",dir.ListFiles(vec)));
        for (u32 i=0; i<vec.size(); i++)
            basicapi__AppendData_T("\t*"+vec[i]);
        vec.clear();
        dir.Close();
        return 1;
    }

    int BasicAPI::Func_SETUP_CLIP(BASICGenericVariable* argv, u32 argc) {
        basicapi__ClearClipboard();
        return 0;
    }

    int BasicAPI::Func_CFGSET(BASICGenericVariable* argv, u32 argc) {
        if (argc < 2) {
            OSD::Notify("API::CFGSET: Missing arguments; need 2", Color::White, Color::Maroon);
            return 1;
        }
        s32 num=CYX::argGetInteger(argv+1);
        switch (num){
        case 0:
            CYX::CYXAPI_Out((s32)1337);
            break;
        case 1:
            CYX::CYXAPI_Out((double)1337.42069);
            break;
        case 2:
            CYX::CYXAPI_Out("Test string");
            break;
        }
        return 1;
    }

    int BasicAPI::Func_LOAD(BASICGenericVariable* argv, u32 argc) {
        if (argc < 2) {
            OSD::Notify("API::LOAD: Missing arguments; need 2", Color::White, Color::Maroon);
            return 1;
        }
        CYX::CYXAPI_Out((s32)handleIDCounter++);
        return 1;
    }

    int BasicAPI::Func_SAVE(BASICGenericVariable* argv, u32 argc) {
        if (argc < 2) {
            OSD::Notify("API::SAVE: Missing arguments; need 2", Color::White, Color::Maroon);
            return 1;
        }
        CYX::CYXAPI_Out((s32)handleIDCounter++);
        return 1;
    }

    int BasicAPI::Func_CFGGET(BASICGenericVariable* argv, u32 argc) {
        if (argc < 1) {
            OSD::Notify("API::CFGGET: Missing arguments; need 1", Color::White, Color::Maroon);
            return 1;
        }
        basicapi__ClearClipboard();
        std::string arg1 = "";
        string16 arg1_16; CYX::argGetString(arg1_16, argv);
        Utils::ConvertUTF16ToUTF8(arg1, arg1_16); arg1_16.clear();
        std::string data = "";

        if (BasicAPI::flags & APIFLAG_READ_SYSINFO){
            if (arg1 == "LANG"){
                data = Utils::Format("%d",System::GetSystemLanguage());
            } else if (arg1 == "LANG$"){
                switch ((int)System::GetSystemLanguage()){
                case  0: data="JPN"; break;
                case  1: data="ENG"; break;
                case  2: data="FRA"; break;
                case  3: data="DEU"; break;
                case  4: data="ITA"; break;
                case  5: data="SPA"; break;
                case  6: data="CHN"; break;
                case  7: data="KOR"; break;
                case  8: data="NED"; break;
                case  9: data="POR"; break;
                case 10: data="RUS"; break;
                case 11: data="TWN"; break;
                };
            } else if (arg1 == "CITRA"){
                data=System::IsCitra()?"1":"0";
            }
        }

        if (arg1 == "COMMIT"){
            data=COMMIT_HASH;
        } else if (arg1 == "BUILD"){
            data=BUILD_DATE;
        } else if (arg1 == "VERSION"){
            data=Utils::Format(STRING_VERSION "/" STRING_BUILD);
        }
        if (data==""){
            basicapi__AppendData_T("Error: \""+arg1+"\" not found");
            return 1;
        }
        basicapi__AppendData_T(data);
        return 0;
    }

    int BasicAPI::Func_OSD(BASICGenericVariable* argv, u32 argc) {
        int type; std::string data="";
        for (u32 i=0; i<argc; i++) {
            string16 st16; CYX::argGetString(st16, argv+1);
            std::string str = "";
            Utils::ConvertUTF16ToUTF8(str, st16); st16.clear();
            if (!i && str.size()==1){
                if (str == "W"){
                    type = 1; continue;
                } else if (str == "N"){
                    type = 2; continue;
                } else if (str == "E"){
                    type = 3; continue;
                } else if (str == "S"){
                    type = 4; continue;
                } else if (str == "M"){
                    type = 0; continue;
                }
            } else if (i) {
                data += str;
            }
        }
        switch (type){
        case 1:
            OSD::Notify(data, Color::White, Color::Olive);
            break;
        case 2:
            OSD::Notify(data, Color::Cyan, Color::Navy);
            break;
        case 3:
            OSD::Notify(data, Color::White, Color::Maroon);
            break;
        case 4:
            OSD::Notify(data, Color::White, Color::Green);
            break;
        default:
            OSD::Notify(data, Color::White, Color::Teal);
            break;
        }
        return 0;
    }
    u32 basicapi__getDataPtr(){
        return (u32)CYX::editorInstance->clipboardData;
    }
    int BasicAPI::Parse(BASICGenericVariable* argv, u32 argc) {
        if (!Entries.size()) {
            CYX::CYXAPI_Out("Error: Bad API state");
            return 0;
        }
        std::string str = ""; string16 st2;
        CYX::SetAPIUse(true);
        if (argc) {
            CYX::argGetString(st2, argv);
            OSD::Notify(Utils::Format("%x %p", argv->data, argv->data2));
            OSD::Notify(Utils::Format("%d,", argc)+str);
            Utils::ConvertUTF16ToUTF8(str, st2);
            strupper(str);
            for (auto i : Entries) {
                if (str == i.id) return i.func(++argv, argc-1);
            }
            CYX::CYXAPI_Out("Error: Unknown API call '"+str+"'");
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
        AddEntry("SETUP_CLIP", Func_SETUP_CLIP);
        AddEntry("FILES", Func_FILES);
        AddEntry("LOAD", Func_LOAD);
        AddEntry("SAVE", Func_SAVE);
        AddEntry("OSD", Func_OSD);
        AddEntry("CFGSET", Func_CFGSET);
    }
    void BasicAPI::Finalize(){
        // TODO: Finish queue system to clean up potential
        // garbage that might result from it.
    }

    void BasicAPI::MenuTick(){
        u32 qsz = Queue.size();
        if (!qsz) return;
        if (queueOffset >= qsz) queueOffset = 0;
        if (!Queue[queueOffset].wasProcessed){
            Queue[queueOffset].wasProcessed = true;
        }
        queueOffset++;
        while (true) {
            qsz = Queue.size();
            if (!qsz || Queue[qsz-1].type != APIFOPTYP_DONE) break;
            Queue.pop_back();
        }
    }
}