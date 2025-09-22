#ifndef PRIMITIVES_SHA256_H
#define PRIMITIVES_SHA256_H

#include <stddef.h>
#include <stdint.h>

#define SHA256_DIGEST_LEN 32

/* Streaming SHA-256 context */
typedef struct {
    uint32_t state[8];   /* hash state */
    uint8_t  buffer[64]; /* current 0..63 bytes */
    size_t   buffer_len; /* bytes currently in buffer */
    uint64_t total_len;  /* total message length in bytes */
} sha256_ctx;

/* API */
void sha256_init(sha256_ctx *ctx);
void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len);
void sha256_final(sha256_ctx *ctx, uint8_t out[SHA256_DIGEST_LEN]);

/* One-shot convenience */
void sha256(const uint8_t *data, size_t len, uint8_t out[SHA256_DIGEST_LEN]);

#endif /* PRIMITIVES_SHA256_H */
