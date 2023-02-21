#ifndef EXCEPTIONHANDLER_HPP
#define EXCEPTIONHANDLER_HPP

#include "main.hpp"

namespace CTRPluginFramework {

    #define EXCEPTION_SYSDUMP_PATH  TOP_DIR"/ptcsysdump.bin"
    #define EXCEPTSET_VERSION   1
    
    #define EXCSYSDMPHDR_MAGIC "CY5%SYSD"
    typedef struct ExceptionSysDump_s {
        u64 magic;
        u16 version;
        ERRF_ExceptionInfo excep;
        CpuRegisters regs;
        u32 potential_stack_trace[16];
    } ExceptionSysDump;
    
    enum ExceptionRescueBitMask {
        EXCEPRESCUE_PROGRAM = 1,
        EXCEPRESCUE_CLIPBOARD = 2,
        EXCEPRESCUE_GRAPHICS = 4
    };
    
    #define CYXDMPHDR_MAGIC "CYX$DMP0"
    typedef struct CYXDumpHeader_s {
        u64 magic;
        u32 version;
        u16 contType;
        u16 blobCount;
        char blobName[16][24];
        u32 blobBufSize[16];
        u32 blobDataLen[16];
        u8 padding[112];
    } PACKED CYXDumpHeader;
    #define sdklfgj sizeof(ExceptionSysDump);

    typedef struct ExceptionSettings_s {
        u32 version;
        u8 rescue;
    } ExceptionSettings;
    
    class Exception {
    public:
        static Process::ExceptionCallbackState Handler(ERRF_ExceptionInfo *excep, CpuRegisters *regs);
        static void RescueIfRequired(void);
        static ExceptionSettings excepSet;
    private:
        static CYXDumpHeader mkHeader(u16 type, u16 cnt);
        static void BuildRescueScreen(u8 mode, u32 i, u32 j, std::string& s2);
        static void BuildScreen(Screen& top, Screen& bot, u64 timer);
    };
}

#endif