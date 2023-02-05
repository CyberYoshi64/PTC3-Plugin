#include "BasicAPI.hpp"

namespace CTRPluginFramework {
    std::vector<BasicAPI_Entry_s> BasicAPI::Entries = {};
    u32 BasicAPI::handleIDCounter = BASICAPI_HANDLE_START;
    std::vector<BasicAPI_QueueEntry> BasicAPI::Queue = {};
    u32 BasicAPI::queueOffset = 0;

    int BasicAPI::Func_FILES(std::string& data, StringVector& args) {
        if (args.size() < 2) {
            data = "Error: Missing arguments; need 2";
            return 1;
        }
        Directory dir(args[1]);
        StringVector vec;
        data = Utils::Format("%d",dir.ListDirectories(vec));
        for (u32 i=0; i<vec.size(); i++) data += ":/"+vec[i];
        vec.clear();
        data += Utils::Format(":%d",dir.ListFiles(vec));
        for (u32 i=0; i<vec.size(); i++) data += ":*"+vec[i];
        vec.clear();
        dir.Close();
        return 1;
    }

    int BasicAPI::Func_LOAD(std::string& data, StringVector& args) {
        if (args.size() < 3) {
            data = "Error: Missing arguments; need 3";
            return 1;
        }
        data = Utils::Format("#%08X", handleIDCounter++);
        return 1;
    }

    int BasicAPI::Func_SAVE(std::string& data, StringVector& args) {
        if (args.size() < 3) {
            data = "Error: Missing arguments; need 3";
            return 1;
        }
        data = Utils::Format("#%08X", handleIDCounter++);
        return 1;
    }

    int BasicAPI::Func_CFGGET(std::string& data, StringVector& args) {
        if (args.size() < 2) {
            data = "Error: Missing arguments; need 2";
            return 1;
        }
        data = "";
        if (args[1] == "LANG"){
            data = Utils::Format("%d",System::GetSystemLanguage());
        } else if (args[1] == "LANG$"){
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
        } else if (args[1] == "CITRA"){
            data=System::IsCitra()?"1":"0";
        } else if (args[1] == "COMMIT"){
            data=COMMIT_HASH;
        } else if (args[1] == "BUILD"){
            data=BUILD_DATE;
        } else if (args[1] == "VERSION"){
            data=Utils::Format(STRING_VERSION "/" STRING_BUILD);
        } else {
            data = "Error: \""+args[1]+"\" not found";
        }
        return 1;
    }

    int BasicAPI::Func_OSD(std::string& data, StringVector& args) {
        int type; data = "";
        for (u32 i=1; i<args.size(); i++) {
            if (i == 1 && args[i].length()==1){
                if (args[i] == "W"){
                    type = 1; continue;
                } else if (args[i] == "N"){
                    type = 2; continue;
                } else if (args[i] == "E"){
                    type = 3; continue;
                } else if (args[i] == "S"){
                    type = 4; continue;
                } else if (args[i] == "M"){
                    type = 0; continue;
                }
            }
            data += args.at(i);
            if (i+1 != args.size()) data += ":";
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
    
    int BasicAPI::ParseClipAPI(std::string& data) {
        if (data.compare(0, 4, "CYX:")!=0) return 1;
        if (!Entries.size()) {
            data = "Error: Bad API state";
            return 0;
        }
        CYX::SetAPIUse(true);
        StringVector args;
        u32 index = 4, newidx;
        while (index && index < data.length()){
            newidx = data.find(":", index);
            args.push_back(data.substr(index, newidx-index));
            index = ++newidx;
        }
        data = "";
        if (args.size()) {
            strupper(args[0]);
            for (auto i : Entries) {
                if (i.id == args[0]) return i.func(data, args);
            }
            data = "Error: Unknown API call \""+args[0]+"\"";
            return 1;
        }
        
        return 0;
    }

    void BasicAPI::AddEntry(std::string id, BasicAPIFunction func) {
        strupper(id); Entries.push_back({id, func});
    }

    void BasicAPI::Initialize(){
        Entries.clear();
        AddEntry("OSD", Func_OSD);
        AddEntry("CFGGET", Func_CFGGET);
        AddEntry("FILES", Func_FILES);
        AddEntry("LOAD", Func_LOAD);
        AddEntry("SAVE", Func_SAVE);
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