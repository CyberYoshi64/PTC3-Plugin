#ifndef PETITREIMPL_HPP
#define PETITREIMPL_HPP

#include "main.hpp"
#include "sha1.h"

namespace CTRPluginFramework {

    #define HMAC_INNERXOR 0x36
    #define HMAC_OUTERXOR 0x5C
    
    class SHA1_HMAC {
    public:
        typedef struct {
            u8 key[64];
            SHA1_CTX innerCtx;
        } CTX;
        static void Init(CTX* ctx, u8* key, u32 key_len);
        static void Update(CTX* ctx, u8* data, u32 data_len);
        static void Update(CTX* ctx, const char* data);
        static void Final(u8* digest, CTX* ctx);
    };

    class FileHMAC {
    public:
        File f;
        SHA1_HMAC::CTX hmac;
        int Open(FileHMAC &ctx, const std::string& path, int mode = File::RWC);
        int Write(void* buf, u32 len);
        void Close();
    };
}


#endif