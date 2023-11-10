#ifndef SHA1_H
#define SHA1_H

/*
   SHA-1 in C - edited
   By Steve Reid <steve@edmweb.com>
   100% Public Domain
 */

#include "stdint.h"
#include "types.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    u32 state[5];
    u32 count[2];
    u8 buffer[64];
} SHA1_CTX;

void SHA1Transform(u32 state[5], const u8 buffer[64]);
void SHA1Init(SHA1_CTX *context);
void SHA1Update(SHA1_CTX *context, const u8 *data, u32 len);
void SHA1Final(u8 digest[20], SHA1_CTX *context);
void SHA1FinalTemp(u8 digest[20], SHA1_CTX *context);
void SHA1(u8 *hash_out, const u8 *str, u32 len);

#if defined(__cplusplus)
}
#endif

#endif /* SHA1_H */