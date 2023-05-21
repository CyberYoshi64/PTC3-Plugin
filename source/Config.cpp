#include "Config.hpp"

namespace CTRPluginFramework {
    ConfigStruct Config::data = {0};

    bool Config::Load(){
        return false;
    }
    bool Config::Save(){
        return false;
    }
    ConfigStruct& Config::Get(){return data;}
}