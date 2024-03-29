#ifndef EXCEPTIONHANDLER_HPP
#define EXCEPTIONHANDLER_HPP

#include <CTRPluginFramework.hpp>
#include "Config.hpp"
#include "QrCode.hpp"
#include "base64.hpp"
#include <cmath>

namespace CTRPluginFramework {

    #define EXCEPTION_SYSDUMP_PATH  TOP_DIR"/ptcsysdump.bin"
    #define EXCEPTSET_VERSION   1
    
    #define EXCSYSDMPHDR_MAGIC *(u32*)"CY5%"
    #define EXCSYSDMPTXT_VER    0x0001
    #define EXCSYSDMPDAT_VER    0x8002

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
        u32 callStack[5];
        char prgn[4][15];
        char activeProject[16];
        char currentProject[16];
        u32 cyxApiFlags;
    } CTR_PACKED ExceptionSysDump;
    
    typedef struct ExceptionSysDumpTxt_s {
        u32 magic;
        u16 version;
        u8 type;
        u8 lang;
        u32 ptcverOrig;
        u32 plgVer;
        u32 plgFlg;
        char err[100];
    } CTR_PACKED ExceptionSysDumpTxt;

    enum ExceptionRescueBitMask {
        EXCEPRESCUE_PROGRAM = 1,
        EXCEPRESCUE_CLIPBOARD = 2,
        EXCEPRESCUE_GRAPHICS = 4
    };
    
    #define CYXDMPHDR_MAGIC "CYX$DMP0"
    #define CYXDMPHDR_VERSION 2
    typedef struct CYXDumpHeader_v1_s {
        u64 magic;
        u32 version;
        u16 contType;
        u16 blobCount;
        char blobName[16][24];
        u32 blobBufSize[16];
        u32 blobDataLen[16];
        u8 padding[112];
    } CTR_PACKED CYXDumpHeader;

    #define CYXDumpHeaderSize sizeof(CYXDumpHeader)
    
    typedef struct ExceptionSettings_s {
        u32 version;
        u8 rescue;
    } ExceptionSettings;
    
    class Exception {
    public:
        static Process::ExceptionCallbackState Handler(ERRF_ExceptionInfo *excep, CpuRegisters *regs);
        static void RescueIfRequired(void);
        static void Panic(std::string s);
        static void Error(std::string s, const char* f = NULL, s32 line = 0);
        static ExceptionSettings excepSet;
    private:
        static CYXDumpHeader mkHeader(u16 type, u16 cnt);
        static void BuildExceptionData(ERRF_ExceptionInfo *excep, CpuRegisters *regs);
        static void BuildRescueScreen(u8 mode, u32 i, u32 j, std::string& s2);
        static void BuildScreen(Screen& top, Screen& bot);
        static const char* SadMessage[];
        static std::string panicString;
        static std::vector<std::vector<bool>> qrModule;
        static u32 screenSadMessageIndex;
        static u32 state;
        static u32 timer;
        static bool dumpAsText;
        static u8 dataBuffer[];
        static u16 dataLength;
    };
}

#define PANIC(s,fn,line)    Exception::Panic(Utils::Format("%s:%d: %s",fn,line,s))
#define DANG(s,fn,line)     {if(!MessageBox("Dang...", (std::string)s+"\n\nProceeding may cause instability or corruption. Continue anyway?", DialogType::DialogYesNo)()){Exception::Error("User aborted execution\n\n"+(std::string)s,fn,line);}}
#define ERROR(s)            Exception::Error(s)
#define ERROR_F(s,fn,line)  Exception::Error(s,fn,line)

#endif