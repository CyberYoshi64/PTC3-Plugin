#include "Config.hpp"

namespace CTRPluginFramework {
    ConfigStruct Config::data = {0};

    // Convert V1 to V2
    void config__convV1(ConfigStruct& data, File& f) {
        Config_v1 o;
        f.Seek(0, File::SET);
        f.Read(&o, sizeof(o));
        data.language = o.language;
        data.cyx.enableAPI = o.cyx.enableAPI;
        data.cyx.fontdefStrict = o.cyx.fontdefStrict;
    }

    void Config::New() {
        memset(&data, 0, sizeof(data));
        data.magic = CONFIG_HEADER;
        data.version = CONFIG_VERSION;
        data.language = (int)System::GetSystemLanguage();
        data.clearCache = true;
    }

    void Config::Load() {
        File f; bool isOK = true;
        isOK = (File::Open(f, CONFIG_FILE_PATH, File::READ)>=0);
        f.Read(&data, CONFIGS_SIZE);
        isOK &= (data.magic == CONFIG_HEADER);
        isOK &= (data.version <= CONFIG_VERSION);
        if (isOK){
            if (data.version == 1) config__convV1(data, f);
        }
        f.Close();


        if (!isOK) New();
    }
    void Config::Save() {
        File f;
        data.magic = CONFIG_HEADER;
        data.version = CONFIG_VERSION;
        data.cyx.zero = 0;
        data.cyx.set.server.zero = 0;
        if (File::Open(f, CONFIG_FILE_PATH, File::RWC | File::TRUNCATE)>=0) {
            f.Write(&data, CONFIGS_SIZE);
        }
        f.Close();
    }
    ConfigStruct& Config::Get(){return data;}
}