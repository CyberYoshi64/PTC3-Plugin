#ifndef EXCEPTIONHANDLER_HPP
#define EXCEPTIONHANDLER_HPP

#include "main.hpp"

namespace CTRPluginFramework {

    #define EXCEPTSET_VERSION   1
    
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
    } CYXDumpHeader;
    #define sdklfgj sizeof(CYXDumpHeader);

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
        static File* excepFile;
    };
}

#endif