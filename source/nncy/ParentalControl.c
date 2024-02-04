#include <3ds.h>

Result CFG_GetParentalControlMask(u32* flags) {
    Result res = 0;
    u8 buf[0x10] = {0};
    *flags = 0;

    if R_SUCCEEDED(res = CFGU_GetConfigInfoBlk2(0x10, 0xC0000, buf))
        *flags = *((u32*)buf + 0);

    return res;
}
