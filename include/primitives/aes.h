#ifndef PRIMITIVES_AES_H
#define PRIMITIVES_AES_H

#include <stdint.h>
#include <stddef.h>

/* AES-128 ECB context: 11 round keys (16 bytes each) */
typedef struct {
    uint8_t rk[11][16];   /* round keys K0..K10 */
} aes128_ctx;

/* Key setup (128-bit key) */
void aes128_set_key(aes128_ctx *ctx, const uint8_t key[16]);

/* Single-block ECB encrypt/decrypt */
void aes128_encrypt_block(const aes128_ctx *ctx, const uint8_t in[16], uint8_t out[16]);
void aes128_decrypt_block(const aes128_ctx *ctx, const uint8_t in[16], uint8_t out[16]);

#endif /* PRIMITIVES_AES_H */
