#pragma once
#include <stdlib.h>
#include <types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/ipc.h>
#ifdef __cplusplus
extern "C" {
#endif
Result ptmSysmInit(void);
Result PTMSYSM_RebootAsync(u64 timeout);
void ptmSysmExit(void);
#ifdef __cplusplus
}
#endif