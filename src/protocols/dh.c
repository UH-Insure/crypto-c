#include "protocols/dh.h"
#include <string.h>

int dh_ephemeral_keypair(uint8_t sk[CU_X25519_SKLEN], uint8_t pk[CU_X25519_PKLEN]) {
    return cu_x25519_keypair(sk, pk);
}

int dh_derive_session_key(const uint8_t my_sk[CU_X25519_SKLEN],
                          const uint8_t their_pk[CU_X25519_PKLEN],
                          const uint8_t *context, size_t context_len,
                          const uint8_t *psk_opt, size_t psk_opt_len,
                          uint8_t out_key[DH_SESSION_KEY_LEN]) {
    uint8_t shared[CU_X25519_PKLEN];
    if (cu_x25519_shared(my_sk, their_pk, shared) != 0) return -1;

    /* HKDF-Extract:
       - salt = psk_opt (if provided) else zeros
       - ikm  = shared
       HKDF-Expand with info = "X25519-CTX" || context
     */
    uint8_t salt[CU_SHA256_LEN] = {0};
    const uint8_t *saltp = salt;
    size_t saltlen = sizeof(salt);
    if (psk_opt && psk_opt_len) { saltp = psk_opt; saltlen = psk_opt_len; }

    uint8_t info[64 + 256];
    const char *label = "X25519-CTX";
    size_t labellen = strlen(label);
    if (context_len > 256) return -2;
    memcpy(info, label, labellen);
    memcpy(info + labellen, context, context_len);

    return cu_hkdf_sha256(shared, sizeof(shared), saltp, saltlen,
                          info, labellen + context_len,
                          out_key, DH_SESSION_KEY_LEN);
}
