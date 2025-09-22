#ifndef PROTOCOLS_PSK_H
#define PROTOCOLS_PSK_H

#include <stddef.h>
#include <stdint.h>
#include "crypto_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSK_NONCE_LEN 32

/* Key material lengths for AEAD(CTR+HMAC) after HKDF */
#define PSK_KEY_ENC_LEN CU_AES_KEY_LEN   /* 16 bytes (AES-128) */
#define PSK_KEY_MAC_LEN CU_SHA256_LEN    /* 32 bytes (HMAC-SHA256) */

/* Server creates challenge nonce. */
int psk_challenge(uint8_t nonce[PSK_NONCE_LEN]);

/* Derive AEAD keys from PSK and transcript info. */
int psk_derive_aead_keys(const uint8_t *psk, size_t psk_len,
                         const uint8_t *client_id, size_t client_id_len,
                         const uint8_t srv_nonce[PSK_NONCE_LEN],
                         uint8_t out_k_enc[PSK_KEY_ENC_LEN],
                         uint8_t out_k_mac[PSK_KEY_MAC_LEN]);

/* Client proves PSK knowledge with HMAC over label||nonce||client_id. */
int psk_make_proof(const uint8_t *psk, size_t psk_len,
                   const uint8_t *client_id, size_t client_id_len,
                   const uint8_t srv_nonce[PSK_NONCE_LEN],
                   uint8_t out_mac[CU_SHA256_LEN]);

int psk_verify_proof(const uint8_t *psk, size_t psk_len,
                     const uint8_t *client_id, size_t client_id_len,
                     const uint8_t srv_nonce[PSK_NONCE_LEN],
                     const uint8_t mac[CU_SHA256_LEN]);

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOLS_PSK_H */
