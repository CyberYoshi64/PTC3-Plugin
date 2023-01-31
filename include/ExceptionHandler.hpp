#ifndef EXCEPTIONHANDLER_HPP
#define EXCEPTIONHANDLER_HPP

#include "main.hpp"

namespace CTRPluginFramework {
    class Exception {
    public:
        static Color MemAreaColor(u32 ptr);
        static Process::ExceptionCallbackState Handler(ERRF_ExceptionInfo *excep, CpuRegisters *regs);
    private:
        static File* excepFile;
    };
}

#endif