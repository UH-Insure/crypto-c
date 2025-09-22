#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Sizes / constants ===== */
#define CU_SHA256_LEN   32

/* AES-CTR params (using AES-128 internally) */
#define CU_AES_KEY_LEN  16   /* AES-128 key bytes */
#define CU_CTR_IV_LEN   16   /* 128-bit CTR nonce/counter block */
#define CU_AEAD_TAG_LEN 32   /* HMAC-SHA256 tag length (full 32 bytes) */

/* X25519 sizes */
#define CU_X25519_PKLEN 32
#define CU_X25519_SKLEN 32

/* ===== Random & utils ===== */
int cu_rand_bytes(uint8_t *dst, size_t len);
int cu_ct_eq(const uint8_t *a, const uint8_t *b, size_t n);

/* ===== HMAC / HKDF (SHA-256) ===== */
int cu_hmac_sha256(const uint8_t *key, size_t keylen,
                   const uint8_t *msg, size_t msglen,
                   uint8_t out_mac[CU_SHA256_LEN]);

int cu_hkdf_sha256(const uint8_t *ikm, size_t ikm_len,
                   const uint8_t *salt, size_t salt_len,
                   const uint8_t *info, size_t info_len,
                   uint8_t *okm, size_t okm_len);

/* ===== AES-CTR (AES-128) ===== */
void cu_aes128ctr_crypt(const uint8_t key[CU_AES_KEY_LEN],
                        const uint8_t iv[CU_CTR_IV_LEN],
                        const uint8_t *in, size_t len,
                        uint8_t *out);

/* ===== AEAD (Encrypt-then-MAC): AES-128-CTR + HMAC-SHA256 ===== */
int cu_aead_etm_encrypt(const uint8_t k_enc[CU_AES_KEY_LEN], const uint8_t k_mac[CU_SHA256_LEN],
                        const uint8_t iv[CU_CTR_IV_LEN],
                        const uint8_t *aad, size_t aad_len,
                        const uint8_t *pt, size_t pt_len,
                        uint8_t *ct, uint8_t tag[CU_AEAD_TAG_LEN]);

int cu_aead_etm_decrypt(const uint8_t k_enc[CU_AES_KEY_LEN], const uint8_t k_mac[CU_SHA256_LEN],
                        const uint8_t iv[CU_CTR_IV_LEN],
                        const uint8_t *aad, size_t aad_len,
                        const uint8_t *ct, size_t ct_len,
                        const uint8_t tag[CU_AEAD_TAG_LEN],
                        uint8_t *pt);

/* ===== X25519 (pure C) ===== */
int cu_x25519_keypair(uint8_t sk[CU_X25519_SKLEN], uint8_t pk[CU_X25519_PKLEN]);
int cu_x25519_shared(const uint8_t sk[CU_X25519_SKLEN], const uint8_t peer_pk[CU_X25519_PKLEN],
                     uint8_t shared[CU_X25519_PKLEN]);

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_UTILS_H */
