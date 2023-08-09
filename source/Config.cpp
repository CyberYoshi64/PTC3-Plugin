#include "Config.hpp"

namespace CTRPluginFramework {
    ConfigStruct Config::data = {0};

    void Config::New(){
        memset(&data, 0, sizeof(data));
        data.magic = CONFIG_HEADER;
        data.version = CONFIG_VERSION;
        data.cyx.enableAPI = 0;
    }
    void Config::Load(){
    }
    void Config::Save(){
    }
    ConfigStruct& Config::Get(){return data;}
}