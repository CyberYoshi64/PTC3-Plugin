#include "petitReimpl.hpp"

namespace CTRPluginFramework {
    void SHA1_HMAC::Init(CTX* ctx, u8* key, u32 key_len){
        memset(ctx, 0, sizeof(CTX));
        u8 innerPad[64];
        SHA1Init(&ctx->innerCtx);
        if (key_len <= 64) {
            memcpy(ctx->key, key, key_len);
        } else {
            SHA1_CTX keyctx;
            SHA1Init(&keyctx);
            SHA1Update(&keyctx, (u8*)key, key_len);
            SHA1Final((u8*)ctx->key, &keyctx);
        }
        memcpy(innerPad, ctx->key, 64);
        for (u32 i=0; i<64; i++) innerPad[i] ^= HMAC_INNERXOR;
        SHA1Update(&ctx->innerCtx, innerPad, 64);
    }
    void SHA1_HMAC::Update(CTX* ctx, u8* data, u32 data_len){
        SHA1Update(&ctx->innerCtx, data, data_len);
    }
    void SHA1_HMAC::Update(CTX* ctx, const char* data){
        SHA1Update(&ctx->innerCtx, (u8*)data, strlen(data));
    }
    void SHA1_HMAC::Final(u8* digest, CTX* ctx){
        u8 outerPad[64];
        u8 innerDigest[20];
        SHA1_CTX outerCtx;
        SHA1Init(&outerCtx);
        memcpy(outerPad, ctx->key, 64);
        for (u32 i=0; i<64; i++) outerPad[i] ^= HMAC_OUTERXOR;
        SHA1Update(&outerCtx, outerPad, 64);
        SHA1Final(innerDigest, &ctx->innerCtx);
        SHA1Update(&outerCtx, innerDigest, 20);
        SHA1Final(digest, &outerCtx);
    }

    int FileHMAC::Open(FileHMAC& ctx, const std::string& path, int mode){
        SHA1_HMAC::Init(&ctx.hmac, (u8*)Hooks::offsets.serverHMACKey, 64);
        return File::Open(ctx.f, path, mode);
    }
    int FileHMAC::Write(void* buf, u32 len){
        SHA1_HMAC::Update(&hmac, (u8*)buf, len);
        return f.Write(buf, len);
    }
    void FileHMAC::Close(){
        u8 digest[20];
        SHA1_HMAC::Final(digest, &hmac);
        f.Write(digest, sizeof(digest));
        f.Close();
    }
}
