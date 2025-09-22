#ifndef PROTOCOLS_DH_H
#define PROTOCOLS_DH_H

#include <stddef.h>
#include <stdint.h>
#include "crypto_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DH_SESSION_KEY_LEN 32 /* AES-256 key */

/* Generate ephemeral X25519 key pair. */
int dh_ephemeral_keypair(uint8_t sk[CU_X25519_SKLEN], uint8_t pk[CU_X25519_PKLEN]);

/* Derive session key from ECDH shared and context (optional PSK to bind). */
int dh_derive_session_key(const uint8_t my_sk[CU_X25519_SKLEN],
                          const uint8_t their_pk[CU_X25519_PKLEN],
                          const uint8_t *context, size_t context_len,
                          const uint8_t *psk_opt, size_t psk_opt_len,
                          uint8_t out_key[DH_SESSION_KEY_LEN]);

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOLS_DH_H */
