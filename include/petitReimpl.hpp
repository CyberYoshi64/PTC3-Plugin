#ifndef PETITREIMPL_HPP
#define PETITREIMPL_HPP

#include "main.hpp"
#include "sha1.h"

namespace CTRPluginFramework {

    #define HMAC_INNERXOR 0x36
    #define HMAC_OUTERXOR 0x5C
    typedef struct {
        u8 key[64];
        SHA1_CTX innerCtx;
    } SHA1_HMACCTX;
    
    void SHA1_HMACInit(SHA1_HMACCTX* ctx, u8* key, u32 key_len);
    void SHA1_HMACUpdate(SHA1_HMACCTX* ctx, u8* data, u32 data_len);
    void SHA1_HMACUpdate(SHA1_HMACCTX* ctx, const char* data);
    void SHA1_HMACFinal(u8* digest, SHA1_HMACCTX* ctx);
}


#endif