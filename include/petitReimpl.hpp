#ifndef PETITREIMPL_HPP
#define PETITREIMPL_HPP

#include "main.hpp"
#include "sha1.h"

namespace CTRPluginFramework {

    #define HMAC_INNERXOR 0x36
    #define HMAC_OUTERXOR 0x5C
    
    // @brief HMAC-SHA1 functions
    class SHA1_HMAC {
    public:
        
        // @brief HMAC-SHA1 context
        typedef struct {
            u8 key[64];
            SHA1_CTX innerCtx;
        } CTX;
        
        /*
            @brief Initialize a HMAC-SHA1 context
            @param[in] ctx HMAC-SHA1 context to initialize
            @param[in] key HMAC key
            @param[in] key_len Length of the HMAC key
        */
        static void Init(CTX* ctx, u8* key, u32 key_len);
        
        /*
            @brief Update a HMAC-SHA1 context with binary data
            @param[in] ctx HMAC-SHA1 context to update
            @param[in] data Binary data to append to the hash
            @param[in] data_len Length of the data
        */
        static void Update(CTX* ctx, u8* data, u32 data_len);
        
        /*
            @brief Update a HMAC-SHA1 context with a C string
            @param[in] ctx HMAC-SHA1 context to update
            @param[in] data C string to append to the hash
        */
        static void Update(CTX* ctx, const char* data);
        
        /*
            @brief Finalize a HMAC-SHA1 context and return the digested hash
            @param[out] digest Buffer to output the digested hash to
            @param[in] ctx HMAC-SHA1 context to finalize
        */
        static void Final(u8* digest, CTX* ctx);
    };

    // @brief Write-only file with a HMAC-SHA1 hash appended to the end
    class FileHMAC {
    public:
        
        // @brief Write-only file to append HMAC hash to
        File f;

        // @brief HMAC-SHA1 context to update on each write
        SHA1_HMAC::CTX hmac;

        /*
            @brief Open a file in write-only mode to append a HMAC-SHA1 hash on close
            @param[out] ctx FileHMAC handle
            @param[in] path File path
            @param[in] mode Mode to open the file with (default: Write-only & Create)
        */
        int Open(FileHMAC &ctx, const std::string& path, int mode = (File::WRITE | File::CREATE));

        /*
            @brief Write to the file and update the HMAC context
            @param[in] buf Data to write to the file and update the HMAC context with
            @param[in] len Length of the data used
        */
        int Write(void* buf, u32 len);

        // @brief Append the digested HMAC hash and close the file
        void Close();
    };
}


#endif