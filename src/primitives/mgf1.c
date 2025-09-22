#include "primitives/mgf1.h"
#include "primitives/sha256.h"
#include <string.h>

void mgf1_sha256(uint8_t *mask, size_t mask_len, const uint8_t *seed, size_t seed_len){
    uint8_t counter[4];
    uint8_t digest[32];
    size_t out = 0;
    uint32_t c = 0;
    while (out < mask_len){
        counter[0] = (uint8_t)((c >> 24) & 0xFF);
        counter[1] = (uint8_t)((c >> 16) & 0xFF);
        counter[2] = (uint8_t)((c >> 8) & 0xFF);
        counter[3] = (uint8_t)(c & 0xFF);
        sha256_ctx ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, seed, seed_len);
        sha256_update(&ctx, counter, 4);
        sha256_final(&ctx, digest);
        size_t take = (mask_len - out < 32) ? (mask_len - out) : 32;
        memcpy(mask + out, digest, take);
        out += take;
        c++;
    }
    memset(digest, 0, sizeof digest);
}
