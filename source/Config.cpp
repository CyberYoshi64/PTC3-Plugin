#include "Config.hpp"

namespace CTRPluginFramework {
    ConfigStruct Config::data = {0};

    void Config::New(){
        memset(&data, 0, sizeof(data));
        data.magic = CONFIG_HEADER;
        data.version = CONFIG_VERSION;
        data.language = (int)System::GetSystemLanguage();
        data.clearCache = true;
    }

    void config__convV1(ConfigStruct& data, File& f){
        Config_v1 o;
        f.Seek(0, File::SET);
        f.Read(&o, sizeof(o));
        data.language = o.language;
        data.cyx.enableAPI = o.cyx.enableAPI;
        data.cyx.fontdefStrict = o.cyx.fontdefStrict;
    }

    void Config::Load(){
        File f; bool isOK = true;
        isOK = (File::Open(f, CONFIG_FILE_PATH, File::READ)>=0);
        f.Read(&data, sizeof(data));
        isOK &= (data.magic == CONFIG_HEADER);
        isOK &= (data.version <= CONFIG_VERSION);
        if (isOK){
            if (data.version == 1) config__convV1(data, f);
        }
        f.Close();


        if (!isOK) New();
    }
    void Config::Save(){
        File f;
        data.magic = CONFIG_HEADER;
        data.version = CONFIG_VERSION;
        if (File::Open(f, CONFIG_FILE_PATH, File::RWC | File::TRUNCATE)>=0){
            f.Write(&data, sizeof(data));
        }
        f.Close();
    }
    ConfigStruct& Config::Get(){return data;}
}