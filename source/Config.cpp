#include "Config.hpp"

namespace CTRPluginFramework {
    ConfigStruct Config::data = {0};

    void Config::New(){
        memset(&data, 0, sizeof(data));
        data.magic = CONFIG_HEADER;
        data.version = CONFIG_VERSION;
        data.cyx.enableAPI = 0;
        data.cyx.fontdefStrict = 0;
    }
    void Config::Load(){
        File f; bool isOK = true;
        isOK = (File::Open(f, CONFIG_FILE_PATH, File::READ)>=0);
        if (isOK){
            f.Read(&data, sizeof(data));
        }
        f.Close();
        isOK &= (data.magic == CONFIG_HEADER);
        isOK &= (data.version <= CONFIG_VERSION);

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