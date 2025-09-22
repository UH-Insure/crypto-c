#include "crypto_utils.h"
#include "primitives/sha256.h"
#include "primitives/aes.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>

/* ===== Random bytes (Linux: /dev/urandom) ===== */
#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
int cu_rand_bytes(uint8_t *dst, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t got = read(fd, dst + off, len - off);
        if (got < 0) { int e = errno; close(fd); errno = e; return -2; }
        if (got == 0) { close(fd); return -3; }
        off += (size_t)got;
    }
    close(fd);
    return 0;
}
#else
#include <time.h>
static uint32_t xorshift32(uint32_t *s){ uint32_t x=*s; x^=x<<13; x^=x>>17; x^=x<<5; return *s=x; }
int cu_rand_bytes(uint8_t *dst, size_t len) {
    uint32_t s = (uint32_t)time(NULL) ^ 0xA5A5A5A5u;
    for (size_t i=0;i<len;i++){ if ((i&3)==0) xorshift32(&s); dst[i] = ((uint8_t*)&s)[i&3]; }
    return 0;
}
#endif

int cu_ct_eq(const uint8_t *a, const uint8_t *b, size_t n) {
    uint8_t d = 0;
    for (size_t i=0;i<n;i++) d |= (uint8_t)(a[i] ^ b[i]);
    return d == 0;
}

/* ===== HMAC-SHA256 ===== */
int cu_hmac_sha256(const uint8_t *key, size_t keylen,
                   const uint8_t *msg, size_t msglen,
                   uint8_t out_mac[CU_SHA256_LEN]) {
    uint8_t k_ipad[64], k_opad[64], tk[CU_SHA256_LEN];
    if (keylen > 64) { sha256(key, keylen, tk); key = tk; keylen = CU_SHA256_LEN; }
    memset(k_ipad, 0x36, 64); memset(k_opad, 0x5c, 64);
    for (size_t i=0;i<keylen;i++){ k_ipad[i]^=key[i]; k_opad[i]^=key[i]; }

    sha256_ctx ctx;
    sha256_init(&ctx); sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, msg, msglen); sha256_final(&ctx, tk);

    sha256_init(&ctx); sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, tk, CU_SHA256_LEN); sha256_final(&ctx, out_mac);
    memset(tk, 0, sizeof tk);
    return 0;
}

/* ===== HKDF-SHA256 (RFC 5869) ===== */
static void hmac_once(const uint8_t *key, size_t keylen,
                      const uint8_t *msg, size_t msglen,
                      uint8_t out[CU_SHA256_LEN]) {
    cu_hmac_sha256(key, keylen, msg, msglen, out);
}

int cu_hkdf_sha256(const uint8_t *ikm, size_t ikm_len,
                   const uint8_t *salt, size_t salt_len,
                   const uint8_t *info, size_t info_len,
                   uint8_t *okm, size_t okm_len) {
    uint8_t prk[CU_SHA256_LEN];
    if (!salt) { uint8_t zero[CU_SHA256_LEN]={0}; salt=zero; salt_len=CU_SHA256_LEN; }
    hmac_once(salt, salt_len, ikm, ikm_len, prk);

    uint8_t T[CU_SHA256_LEN];
    size_t n = (okm_len + CU_SHA256_LEN - 1) / CU_SHA256_LEN;
    size_t pos = 0, Tlen = 0;
    for (size_t i=1; i<=n; i++) {
        uint8_t buf[CU_SHA256_LEN + 256 + 1];
        size_t off = 0;
        if (Tlen) { memcpy(buf, T, Tlen); off += Tlen; }
        if (info && info_len) { memcpy(buf+off, info, info_len); off += info_len; }
        buf[off++] = (uint8_t)i;
        hmac_once(prk, CU_SHA256_LEN, buf, off, T);
        Tlen = CU_SHA256_LEN;

        size_t take = (pos + CU_SHA256_LEN <= okm_len) ? CU_SHA256_LEN : (okm_len - pos);
        memcpy(okm + pos, T, take);
        pos += take;
    }
    memset(prk, 0, sizeof prk);
    memset(T, 0, sizeof T);
    return 0;
}

/* ===== AES-CTR (AES-128) ===== */
static void ctr_inc(uint8_t iv[CU_CTR_IV_LEN]){
    for (int i=CU_CTR_IV_LEN-1; i>=0; i--) { uint8_t v = (uint8_t)(iv[i] + 1); iv[i]=v; if (v) break; }
}

void cu_aes128ctr_crypt(const uint8_t key[CU_AES_KEY_LEN],
                        const uint8_t iv_in[CU_CTR_IV_LEN],
                        const uint8_t *in, size_t len,
                        uint8_t *out) {
    aes128_ctx a; aes128_set_key(&a, key);
    uint8_t iv[CU_CTR_IV_LEN]; memcpy(iv, iv_in, CU_CTR_IV_LEN);
    uint8_t stream[16];
    size_t off = 0;
    while (off < len) {
        aes128_encrypt_block(&a, iv, stream);
        size_t chunk = (len - off < 16) ? (len - off) : 16;
        for (size_t i=0;i<chunk;i++) out[off+i] = in[off+i] ^ stream[i];
        off += chunk; ctr_inc(iv);
    }
    memset(stream, 0, sizeof stream);
}

/* ===== AEAD: Encrypt-then-MAC (AES-128-CTR + HMAC-SHA256) ===== */
int cu_aead_etm_encrypt(const uint8_t k_enc[CU_AES_KEY_LEN], const uint8_t k_mac[CU_SHA256_LEN],
                        const uint8_t iv[CU_CTR_IV_LEN],
                        const uint8_t *aad, size_t aad_len,
                        const uint8_t *pt, size_t pt_len,
                        uint8_t *ct, uint8_t tag[CU_AEAD_TAG_LEN]) {
    cu_aes128ctr_crypt(k_enc, iv, pt, pt_len, ct);

    uint8_t mac1[CU_SHA256_LEN];
    cu_hmac_sha256(k_mac, CU_SHA256_LEN, aad ? aad : (const uint8_t*)"", aad ? aad_len : 0, mac1);

    size_t tmp_len = CU_SHA256_LEN + CU_CTR_IV_LEN + pt_len;
    uint8_t *tmp = (uint8_t*)malloc(tmp_len);
    if (!tmp) return -1;
    memcpy(tmp, mac1, CU_SHA256_LEN);
    memcpy(tmp + CU_SHA256_LEN, iv, CU_CTR_IV_LEN);
    memcpy(tmp + CU_SHA256_LEN + CU_CTR_IV_LEN, ct, pt_len);
    cu_hmac_sha256(k_mac, CU_SHA256_LEN, tmp, tmp_len, tag);
    memset(mac1, 0, sizeof mac1);
    memset(tmp, 0, tmp_len);
    free(tmp);
    return 0;
}

int cu_aead_etm_decrypt(const uint8_t k_enc[CU_AES_KEY_LEN], const uint8_t k_mac[CU_SHA256_LEN],
                        const uint8_t iv[CU_CTR_IV_LEN],
                        const uint8_t *aad, size_t aad_len,
                        const uint8_t *ct, size_t ct_len,
                        const uint8_t tag[CU_AEAD_TAG_LEN],
                        uint8_t *pt) {
    /* Recompute tag over ct */
    uint8_t mac1[CU_SHA256_LEN];
    cu_hmac_sha256(k_mac, CU_SHA256_LEN, aad ? aad : (const uint8_t*)"", aad ? aad_len : 0, mac1);

    size_t tmp_len = CU_SHA256_LEN + CU_CTR_IV_LEN + ct_len;
    uint8_t *tmp = (uint8_t*)malloc(tmp_len);
    if (!tmp) return -1;
    memcpy(tmp, mac1, CU_SHA256_LEN);
    memcpy(tmp + CU_SHA256_LEN, iv, CU_CTR_IV_LEN);
    memcpy(tmp + CU_SHA256_LEN + CU_CTR_IV_LEN, ct, ct_len);
    uint8_t recomputed[CU_AEAD_TAG_LEN];
    cu_hmac_sha256(k_mac, CU_SHA256_LEN, tmp, tmp_len, recomputed);
    memset(mac1, 0, sizeof mac1); memset(tmp, 0, tmp_len); free(tmp);

    if (!cu_ct_eq(recomputed, tag, CU_AEAD_TAG_LEN)) {
        memset(recomputed, 0, sizeof recomputed);
        return -3;
    }
    memset(recomputed, 0, sizeof recomputed);

    /* Auth OK: decrypt */
    cu_aes128ctr_crypt(k_enc, iv, ct, ct_len, pt);
    return 0;
}

/* ===================================================================== */
/* =========================== X25519 (pure C) ========================== */
/* ===================================================================== */
/* RFC 7748-mandated Montgomery ladder over Curve25519 in little-endian. */

static void fe25519_add(uint32_t *r, const uint32_t *a, const uint32_t *b){
    for (int i=0;i<8;i++) r[i]=a[i]+b[i];
}

static void fe25519_sub(uint32_t *r, const uint32_t *a, const uint32_t *b){
    /* r = a - b mod p where p = 2^255 - 19 */
    static const uint32_t two_p[8] = {0xFFFFFFEC,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x7FFFFFFF};
    uint64_t c=0;
    for (int i=0;i<8;i++){
        uint64_t t = (uint64_t)a[i] + two_p[i] - b[i] + c;
        r[i]=(uint32_t)t; c=t>>32;
    }
}

/* multiply (schoolbook) and reduce modulo 2^255-19; this is a compact, readable version,
   not constant-time tuned; good enough for reference impls and tests. */
static void fe25519_mul(uint32_t *r, const uint32_t *a, const uint32_t *b){
    uint64_t t[16]={0};
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            t[i+j]+= (uint64_t)a[i]*b[j];
        }
    }
    /* reduction: fold t[8..15] back with 19 */
    for(int i=8;i<16;i++){
        t[i-8] += t[i]*19;
    }
    /* propagate carries */
    uint64_t c=0;
    for(int i=0;i<8;i++){
        t[i]+=c; r[i]=(uint32_t)t[i]; c=t[i]>>32;
    }
    /* final reduction for top carry */
    uint64_t cc = (c*19);
    for(int i=0;i<8;i++){
        uint64_t s = (uint64_t)r[i] + (cc & 0xFFFFFFFFu);
        r[i]=(uint32_t)s; cc = (cc>>32) + (s>>32);
    }
    /* ensure < p */
    /* (r >= p) subtract p */
    uint32_t p[8]={0xFFFFFFED,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x7FFFFFFF};
    uint64_t borrow=0;
    for(int i=0;i<8;i++){
        uint64_t diff = (uint64_t)r[i] - p[i] - borrow;
        p[i]=(uint32_t)diff; borrow=(diff>>63)&1;
    }
    if (borrow==0){ /* r >= p */
        for(int i=0;i<8;i++) r[i]=p[i];
    }
}

static void fe25519_sq(uint32_t *r, const uint32_t *a){ fe25519_mul(r,a,a); }

/* inversion via Fermat little theorem: a^(p-2) */
static void fe25519_inv(uint32_t *r, const uint32_t *a){
    /* Exponentiate using square-and-multiply chain (compact, not fully optimized) */
    uint32_t t0[8], t1[8], t2[8], t3[8];
    fe25519_sq(t0,a);           /* 2 */
    fe25519_sq(t1,t0);          /* 4 */
    fe25519_sq(t1,t1);          /* 8 */
    fe25519_mul(t1,a,t1);       /* 9 */
    fe25519_mul(t0,t0,t1);      /* 11 */
    fe25519_sq(t2,t0);          /* 22 */
    fe25519_mul(t1,t1,t2);      /* 31 = 2^5 -1 */
    fe25519_sq(t2,t1);
    for (int i=0;i<4;i++) fe25519_sq(t2,t2); /* 2^10 - 2^5 */
    fe25519_mul(t1,t2,t1);      /* 2^10 -1 */
    fe25519_sq(t2,t1);
    for (int i=0;i<9;i++) fe25519_sq(t2,t2);
    fe25519_mul(t2,t2,t1);      /* 2^20 -1 */
    fe25519_sq(t3,t2);
    for (int i=0;i<19;i++) fe25519_sq(t3,t3);
    fe25519_mul(t2,t3,t2);      /* 2^40 -1 */
    for (int i=0;i<10;i++) fe25519_sq(t2,t2); /* 2^50 - 2^10 */
    fe25519_mul(t1,t2,t1);      /* 2^50 -1 */
    for (int i=0;i<50;i++) fe25519_sq(t1,t1); /* 2^100 - 2^50 */
    fe25519_mul(t1,t1,t2);      /* 2^100 -1 */
    for (int i=0;i<100;i++) fe25519_sq(t1,t1); /* 2^200 - 2^100 */
    fe25519_mul(t1,t1,t2);      /* 2^200 -1 */
    for (int i=0;i<50;i++) fe25519_sq(t1,t1);  /* 2^250 - 2^50 */
    fe25519_mul(t0,t1,t0);      /* 2^250 - 3 */
    for (int i=0;i<5;i++) fe25519_sq(t0,t0);   /* 2^255 - 96 */
    fe25519_mul(r,t0,a);        /* 2^255 - 19 -? matches a^(p-2) */
}

static void load_le32(uint32_t out[8], const uint8_t in[32]){
    for (int i=0;i<8;i++){
        out[i] = ((uint32_t)in[4*i]) | ((uint32_t)in[4*i+1]<<8) |
                 ((uint32_t)in[4*i+2]<<16) | ((uint32_t)in[4*i+3]<<24);
    }
}
static void store_le32(uint8_t out[32], const uint32_t in[8]){
    for (int i=0;i<8;i++){
        out[4*i+0]=(uint8_t)(in[i]      );
        out[4*i+1]=(uint8_t)(in[i] >> 8 );
        out[4*i+2]=(uint8_t)(in[i] >>16 );
        out[4*i+3]=(uint8_t)(in[i] >>24 );
    }
}

/* Montgomery ladder: scalar mult [k]BaseX for base point u=9 */
static void cswap(uint32_t cond, uint32_t a[8], uint32_t b[8]){
    uint32_t mask = -cond;
    for (int i=0;i<8;i++){ uint32_t t = mask & (a[i] ^ b[i]); a[i]^=t; b[i]^=t; }
}
/* Montgomery ladder: scalar mult [k]u for u on Curve25519 */
static void mont_ladder(uint8_t out[32], const uint8_t scalar[32], const uint8_t u_in[32]){
    /* decode inputs */
    uint32_t x1[8], x2[8], z2[8], x3[8], z3[8];
    uint32_t a[8], b[8], c[8], d[8], da[8], cb[8];
    uint32_t aa[8], bb[8], e[8], t0[8], t1[8];
    uint32_t b_saved[8];
    static const uint32_t A24[8] = {121665,0,0,0,0,0,0,0}; /* (A+2)/4 for Curve25519 */

    load_le32(x1, u_in);
    /* (x2,z2) = (1,0), (x3,z3) = (u,1) */
    memset(z2,0,sizeof z2); memset(z3,0,sizeof z3);
    memset(x2,0,sizeof x2); x2[0]=1;
    memcpy(x3, x1, sizeof x1); z3[0]=1;

    uint8_t ebytes[32]; memcpy(ebytes, scalar, 32);
    /* clamp k */
    ebytes[0] &= 248; ebytes[31] &= 127; ebytes[31] |= 64;

    int swap = 0;
    for (int t=254; t>=0; t--){
        int kt = (ebytes[t>>3] >> (t&7)) & 1;
        swap ^= kt;
        cswap(swap, x2, x3);
        cswap(swap, z2, z3);
        swap = kt;

        /* A = X2+Z2 ; B = X2-Z2 ; C = X3+Z3 ; D = X3-Z3 */
        fe25519_add(a, x2, z2);
        fe25519_sub(b, x2, z2);
        fe25519_add(c, x3, z3);
        fe25519_sub(d, x3, z3);

        /* Save B for later (BB = B^2) */
        memcpy(b_saved, b, sizeof b_saved);

        /* DA = D*A ; CB = C*B */
        fe25519_mul(da, d, a);
        fe25519_mul(cb, c, b);

        /* X3 = (DA+CB)^2 */
        fe25519_add(t0, da, cb);
        fe25519_sq(x3, t0);

        /* Z3 = (DA-CB)^2 * X1 */
        fe25519_sub(t1, da, cb);
        fe25519_sq(z3, t1);
        fe25519_mul(z3, z3, x1);

        /* AA = A^2 ; BB = B^2 */
        fe25519_sq(aa, a);
        fe25519_sq(bb, b_saved);

        /* E = AA - BB */
        fe25519_sub(e, aa, bb);

        /* X2 = AA * BB */
        fe25519_mul(x2, aa, bb);

        /* Z2 = E * (BB + a24*E) */
        fe25519_mul(t0, e, A24);      /* t0 = a24*E */
        fe25519_add(t0, t0, bb);      /* t0 = BB + a24*E */
        fe25519_mul(z2, e, t0);       /* Z2 = E * t0 */
    }
    cswap(swap, x2, x3);
    cswap(swap, z2, z3);

    /* Compute x/z */
    uint32_t z2inv[8];
    fe25519_inv(z2inv, z2);
    fe25519_mul(x2, x2, z2inv);
    store_le32(out, x2);
}


int cu_x25519_keypair(uint8_t sk[CU_X25519_SKLEN], uint8_t pk[CU_X25519_PKLEN]){
    if (cu_rand_bytes(sk, CU_X25519_SKLEN) != 0) return -1;
    /* clamp secret */
    sk[0] &= 248; sk[31] &= 127; sk[31] |= 64;
    static const uint8_t base[32] = {9,0};
    mont_ladder(pk, sk, base);
    return 0;
}

int cu_x25519_shared(const uint8_t sk[CU_X25519_SKLEN], const uint8_t peer_pk[CU_X25519_PKLEN],
                     uint8_t shared[CU_X25519_PKLEN]){
    mont_ladder(shared, sk, peer_pk);
    /* per RFC 7748, all-zero shared is invalid (small subgroup) — caller may check if needed */
    return 0;
}
