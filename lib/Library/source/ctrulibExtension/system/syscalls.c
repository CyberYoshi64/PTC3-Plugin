#include <sys/iosupport.h>
#include <sys/time.h>
#include <sys/lock.h>
#include <sys/reent.h>
#include <string.h>
#include <3ds.h>
#include "ctrulibExtension.h"

void __ctru_exit(int rc);

static ThreadVars * g_mainThreadVars;

ThreadVars* __ctrpf_getThreadVars(ThreadVars * mainThreadVars);

struct _reent* __SYSCALL(getreent)()
{
	ThreadVars* tv = getThreadVars();

	if (tv->magic != THREADVARS_MAGIC)
	{
        // We're probably hooked from game so get main thread's reent
        return (__ctrpf_getThreadVars(g_mainThreadVars)->reent);
	}
	return tv->reent;
}

void*     __getThreadLocalStorage(void)
{
	ThreadVars* tv = getThreadVars();

	if (tv->magic != THREADVARS_MAGIC)
	{
        // We're probably hooked from game so get main thread's tls
        return __ctrpf_getThreadVars(g_mainThreadVars)->tls_tp;
	}

	return tv->tls_tp;
}

//---------------------------------------------------------------------------------
int __SYSCALL(gettod_r)(struct _reent *ptr, struct timeval *tp, struct timezone *tz) {
//---------------------------------------------------------------------------------
        if (tp != NULL) {
                // Retrieve current time, adjusting epoch from 1900 to 1970
                s64 now = osGetTime() - 2208988800000ULL;

                // Convert to struct timeval
                tp->tv_sec = now / 1000;
                tp->tv_usec = (now - 1000*tp->tv_sec) * 1000;
        }

        if (tz != NULL) {
                // Provide dummy information, as the 3DS does not have the concept of timezones
                tz->tz_minuteswest = 0;
                tz->tz_dsttime = 0;
        }

        return 0;
}

int __SYSCALL(nanosleep)(const struct timespec *req, struct timespec *rem)
{
	svcSleepThread(req->tv_sec * 1000000000ull + req->tv_nsec);
	return 0;
}

void __SYSCALL(lock_init) (_LOCK_T *lock)
{
	LightLock_Init(lock);
}

void __SYSCALL(lock_acquire) (_LOCK_T *lock)
{
	LightLock_Lock(lock);
}

int  __SYSCALL(lock_try_acquire) (_LOCK_T *lock)
{
	return LightLock_TryLock(lock);
}

void __SYSCALL(lock_release) (_LOCK_T *lock)
{
	LightLock_Unlock(lock);
}

void __SYSCALL(lock_init_recursive) (_LOCK_RECURSIVE_T *lock)
{
	RecursiveLock_Init(lock);
}

void __SYSCALL(lock_acquire_recursive) (_LOCK_RECURSIVE_T *lock)
{
	RecursiveLock_Lock(lock);
}

int  __SYSCALL(lock_try_acquire_recursive) (_LOCK_RECURSIVE_T *lock)
{
	return RecursiveLock_TryLock(lock);
}

void __SYSCALL(lock_release_recursive) (_LOCK_RECURSIVE_T *lock)
{
	RecursiveLock_Unlock(lock);
}

void __SYSCALL(exit)(int rc) {
	__ctru_exit(rc);
}

void initThreadVars(struct Thread_tag *thread)
{
	ThreadVars* tv = getThreadVars();
	tv->magic = THREADVARS_MAGIC;
	tv->reent = thread != NULL ? &thread->reent : _impure_ptr;
	tv->thread_ptr = thread;
	tv->tls_tp = (thread != NULL ? (u8*)thread->stacktop : __tls_start) - 8; // Arm ELF TLS ABI mandates an 8-byte header
	tv->srv_blocking_policy = false;

	// Kernel does not initialize fpscr at all, so we must do it ourselves
	// https://developer.arm.com/documentation/ddi0360/f/vfp-programmers-model/vfp11-system-registers/floating-point-status-and-control-register--fpscr

	// All flags clear, all interrupts disabled, all instruction scalar.
	// As for the 3 below fields: default NaN mode, flush-to-zero both enabled & round to nearest.
	__builtin_arm_set_fpscr(BIT(25) | BIT(24) | (0u << 22));
}

void __system_initSyscalls(void)
{
	// Initialize thread vars for the main thread
	initThreadVars(NULL);
	ThreadVars* tv = getThreadVars();

    g_mainThreadVars = tv;

	u32 tls_size = __tdata_lma_end - __tdata_lma;
	size_t tdata_start = alignTo((size_t)__tls_start, __tdata_align);
	if (tls_size)
		memcpy((void*)tdata_start, __tdata_lma, tls_size);
}
