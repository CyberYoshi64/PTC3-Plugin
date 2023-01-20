#ifndef EXCEPTIONHANDLER_HPP
#define EXCEPTIONHANDLER_HPP

#include "main.hpp"

namespace CTRPluginFramework {
    class Exception {
        public:
            static Process::ExceptionCallbackState Handler(ERRF_ExceptionInfo *excep, CpuRegisters *regs);
    };
}

#endif