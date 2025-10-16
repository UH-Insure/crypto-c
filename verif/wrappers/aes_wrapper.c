#include <stdint.h>
#include "primitives/aes.h"

// One-shot ECB encrypt of one 16-byte block with a 16-byte key.
void aes128_ecb_encrypt_one(const uint8_t key[16],
                            const uint8_t in[16],
                            uint8_t out[16]) {
    aes128_ctx ctx;
    aes128_set_key(&ctx, key);
    aes128_encrypt_block(&ctx, in, out);
}