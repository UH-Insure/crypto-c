#include "primitives/sha256.h"
#include <string.h>
#include <stdint.h>

/* ===== SHA-256 per FIPS 180-4 ===== */

#define ROR32(x,n) ((uint32_t)(((x) >> (n)) | ((x) << (32 - (n)))))

static inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t BSIG0(uint32_t x){ return ROR32(x,2) ^ ROR32(x,13) ^ ROR32(x,22); }
static inline uint32_t BSIG1(uint32_t x){ return ROR32(x,6) ^ ROR32(x,11) ^ ROR32(x,25); }
static inline uint32_t SSIG0(uint32_t x){ return ROR32(x,7) ^ ROR32(x,18) ^ (x >> 3); }
static inline uint32_t SSIG1(uint32_t x){ return ROR32(x,17) ^ ROR32(x,19) ^ (x >> 10); }

static const uint32_t K[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xD807AA98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static inline void store_be32(uint8_t *dst, uint32_t w) {
    dst[0] = (uint8_t)(w >> 24);
    dst[1] = (uint8_t)(w >> 16);
    dst[2] = (uint8_t)(w >> 8);
    dst[3] = (uint8_t)(w);
}

static void process_block(sha256_ctx *ctx, const uint8_t block[64]){
    uint32_t W[64];

    /* message schedule */
    for (int i=0;i<16;i++) {
        W[i] = ((uint32_t)block[4*i] << 24) |
               ((uint32_t)block[4*i+1] << 16) |
               ((uint32_t)block[4*i+2] << 8) |
               ((uint32_t)block[4*i+3]);
    }
    for (int i=16;i<64;i++) {
        W[i] = SSIG1(W[i-2]) + W[i-7] + SSIG0(W[i-15]) + W[i-16];
    }

    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];

    for (int i=0;i<64;i++){
        uint32_t T1 = h + BSIG1(e) + Ch(e,f,g) + K[i] + W[i];
        uint32_t T2 = BSIG0(a) + Maj(a,b,c);
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }

    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

void sha256_init(sha256_ctx *ctx){
    ctx->state[0]=0x6a09e667u; ctx->state[1]=0xbb67ae85u;
    ctx->state[2]=0x3c6ef372u; ctx->state[3]=0xa54ff53au;
    ctx->state[4]=0x510e527fu; ctx->state[5]=0x9b05688cu;
    ctx->state[6]=0x1f83d9abu; ctx->state[7]=0x5be0cd19u;
    ctx->buffer_len = 0;
    ctx->total_len  = 0;
}

void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len) {
    ctx->total_len += len;

    size_t off = 0;
    if (ctx->buffer_len) {
        size_t take = 64 - ctx->buffer_len;
        if (take > len) take = len;
        memcpy(ctx->buffer + ctx->buffer_len, data, take);
        ctx->buffer_len += take;
        off += take;
        if (ctx->buffer_len == 64) {
            process_block(ctx, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }

    while (off + 64 <= len) {
        process_block(ctx, data + off);
        off += 64;
    }

    if (off < len) {
        size_t rem = len - off;
        memcpy(ctx->buffer, data + off, rem);
        ctx->buffer_len = rem;
    }
}

void sha256_final(sha256_ctx *ctx, uint8_t out[SHA256_DIGEST_LEN]) {
    /* append 0x80 */
    size_t i = ctx->buffer_len;
    ctx->buffer[i++] = 0x80;

    /* pad to 56 bytes, then append length (in bits) as 8-byte big-endian */
    if (i > 56) {
        while (i < 64) ctx->buffer[i++] = 0x00;
        process_block(ctx, ctx->buffer);
        i = 0;
    }
    while (i < 56) ctx->buffer[i++] = 0x00;

    uint64_t bitlen = ctx->total_len * 8ULL;
    ctx->buffer[56] = (uint8_t)(bitlen >> 56);
    ctx->buffer[57] = (uint8_t)(bitlen >> 48);
    ctx->buffer[58] = (uint8_t)(bitlen >> 40);
    ctx->buffer[59] = (uint8_t)(bitlen >> 32);
    ctx->buffer[60] = (uint8_t)(bitlen >> 24);
    ctx->buffer[61] = (uint8_t)(bitlen >> 16);
    ctx->buffer[62] = (uint8_t)(bitlen >> 8);
    ctx->buffer[63] = (uint8_t)(bitlen);

    process_block(ctx, ctx->buffer);

    /* output digest in big-endian */
    for (int w = 0; w < 8; w++) {
        store_be32(out + 4*w, ctx->state[w]);
    }

    /* wipe */
    memset(ctx->buffer, 0, sizeof ctx->buffer);
    ctx->buffer_len = 0;
    ctx->total_len  = 0;
}

void sha256(const uint8_t *data, size_t len, uint8_t out[SHA256_DIGEST_LEN]){
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, out);
}
