#ifndef BASICAPI_HPP
#define BASICAPI_HPP

#include "main.hpp"

namespace CTRPluginFramework {
    using BasicAPIFunction = int(*)(std::string& data, StringVector& args);
    
    #define BASICAPI_HANDLE_START (u32)0xC6400000U

    enum BasicAPI_FileOpType {
        APIFOPTYP_DONE = 0,
        APIFOPTYP_LOAD,
        APIFOPTYP_SAVE
    };

    typedef struct BasicAPI_QueueEntry_s {
        bool wasProcessed;
        u32 handleID;
        u32 returnValue;
        u32 type;
        u32 arg1;
        u32 arg2;
        std::string arg3;
        std::string arg4;
    } BasicAPI_QueueEntry;
    
    typedef struct BasicAPI_Entry_s {
        std::string id;
        BasicAPIFunction func;
    } BasicAPI_Entry;
    
    class BasicAPI {
    public:
        static void Initialize(void);
        static void Finalize(void);
        static void MenuTick(void);
        static int ParseClipAPI(std::string& data);
    private:
        
        static int Func_FILES(std::string& data, StringVector& args);
        static int Func_LOAD(std::string& data, StringVector& args);
        static int Func_SAVE(std::string& data, StringVector& args);
        static int Func_CFGGET(std::string& data, StringVector& args);
        static int Func_OSD(std::string& data, StringVector& args);
        
        static void AddEntry(std::string id, BasicAPIFunction func);
        static std::vector<BasicAPI_Entry> Entries;
        static std::vector<BasicAPI_QueueEntry> Queue;
        static u32 queueOffset;
        static u32 handleIDCounter;
    };
    
}

#endif