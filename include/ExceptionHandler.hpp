#ifndef EXCEPTIONHANDLER_HPP
#define EXCEPTIONHANDLER_HPP

#include "main.hpp"
#include "QrCode.hpp"

namespace CTRPluginFramework {

    #define EXCEPTION_SYSDUMP_PATH  TOP_DIR"/ptcsysdump.bin"
    #define EXCEPTSET_VERSION   1
    
    #define EXCSYSDMPHDR_MAGIC *(u32*)"CY5%"
    #define EXCSYSDMPTXT_VER    0x0001
    #define EXCSYSDMPDAT_VER    0x8001

    #define EXCEP_LANG      ((int)System::GetSystemLanguage()&0xF) | ((int)g_region << 4)

    typedef struct ExceptionSysDump_s {
        u32 magic;
        u16 version;
        u8 type;
        u8 lang;
        u32 ptcverOrig;
        u32 plgVer;
        u32 plgFlg;
        u32 pc;
        u32 lr;
        u32 sp;
        u32 far;
        u32 callStack[6];
        char prgn[4][15];
    } PACKED ExceptionSysDump;
    
    typedef struct ExceptionSysDumpTxt_s {
        u32 magic;
        u16 version;
        u8 type;
        u8 lang;
        u32 ptcverOrig;
        u32 plgVer;
        u32 plgFlg;
        char err[100];
    } PACKED ExceptionSysDumpTxt;

    #define sfdgkdh sizeof(ExceptionSysDump)
    
    enum ExceptionRescueBitMask {
        EXCEPRESCUE_PROGRAM = 1,
        EXCEPRESCUE_CLIPBOARD = 2,
        EXCEPRESCUE_GRAPHICS = 4
    };
    
    #define CYXDMPHDR_MAGIC "CYX$DMP0"
    typedef struct CYXDumpHeader_v1_s {
        u64 magic;
        u32 version;
        u16 contType;
        u16 blobCount;
        char blobName[16][24];
        u32 blobBufSize[16];
        u32 blobDataLen[16];
        u8 padding[112];
    } PACKED CYXDumpHeader;
    
    typedef struct ExceptionSettings_s {
        u32 version;
        u8 rescue;
    } ExceptionSettings;
    
    class Exception {
    public:
        static Process::ExceptionCallbackState Handler(ERRF_ExceptionInfo *excep, CpuRegisters *regs);
        static void RescueIfRequired(void);
        static void Panic(std::string s);
        static ExceptionSettings excepSet;
    private:
        static CYXDumpHeader mkHeader(u16 type, u16 cnt);
        static void BuildExceptionData(ERRF_ExceptionInfo *excep, CpuRegisters *regs);
        static void BuildRescueScreen(u8 mode, u32 i, u32 j, std::string& s2);
        static std::string SadMessageRnd(void);
        static void BuildScreen(Screen& top, Screen& bot, u64 timer);
        static std::string panicString;
        static qrcodegen::QrCode* qr;
        static u32 screenSadMessageIndex;
        static u32 renderState;
        static bool dumpAsText;
        static u8 dataBuffer[];
        static u16 dataLength;
    };
}

#define PANIC(s,fn,line)    Exception::Panic(Utils::Format("%s:%d: %s",fn,line,s))

#endif