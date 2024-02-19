#include <3ds.h>
#include <malloc.h>

Result CFG_GetParentalControlMask(u32* flags) {
    Result res = 0;
    u8* buf;
    *flags = 0;

    if (!(buf = malloc(0xC0)))
        return MAKERESULT(RL_FATAL, RS_OUTOFRESOURCE, RM_CONFIG, RD_OUT_OF_MEMORY);

    if R_SUCCEEDED(res = CFGU_GetConfigInfoBlk2(0xC0, 0xC0000, buf))
        *flags = *((u32*)buf + 0);
    
    free(buf);

    return res;
}
