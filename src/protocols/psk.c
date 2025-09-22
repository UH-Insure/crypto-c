#include "protocols/psk.h"
#include <string.h>

int psk_challenge(uint8_t nonce[PSK_NONCE_LEN]) {
    return cu_rand_bytes(nonce, PSK_NONCE_LEN);
}

int psk_derive_aead_keys(const uint8_t *psk, size_t psk_len,
                         const uint8_t *client_id, size_t client_id_len,
                         const uint8_t srv_nonce[PSK_NONCE_LEN],
                         uint8_t out_k_enc[PSK_KEY_ENC_LEN],
                         uint8_t out_k_mac[PSK_KEY_MAC_LEN]) {
    /* HKDF-Extract: salt = server nonce; IKM = PSK
       HKDF-Expand:  info = "PSK-AEAD" || client_id
       Output: 16 + 32 = 48 bytes total; split into k_enc || k_mac
    */
    const char *label = "PSK-AEAD";
    uint8_t info[64 + 256];
    size_t labellen = strlen(label);
    if (client_id_len > 256) return -1;
    memcpy(info, label, labellen);
    memcpy(info + labellen, client_id, client_id_len);

    uint8_t okm[PSK_KEY_ENC_LEN + PSK_KEY_MAC_LEN];
    if (cu_hkdf_sha256(psk, psk_len, srv_nonce, PSK_NONCE_LEN,
                       info, labellen + client_id_len,
                       okm, sizeof okm) != 0) return -2;

    memcpy(out_k_enc, okm, PSK_KEY_ENC_LEN);
    memcpy(out_k_mac, okm + PSK_KEY_ENC_LEN, PSK_KEY_MAC_LEN);
    memset(okm, 0, sizeof okm);
    return 0;
}

int psk_make_proof(const uint8_t *psk, size_t psk_len,
                   const uint8_t *client_id, size_t client_id_len,
                   const uint8_t srv_nonce[PSK_NONCE_LEN],
                   uint8_t out_mac[CU_SHA256_LEN]) {
    const char *label = "PSK-PROOF";
    uint8_t buf[64 + PSK_NONCE_LEN + 256];
    size_t labellen = strlen(label);
    if (client_id_len > 256) return -1;
    memcpy(buf, label, labellen);
    memcpy(buf + labellen, srv_nonce, PSK_NONCE_LEN);
    memcpy(buf + labellen + PSK_NONCE_LEN, client_id, client_id_len);
    return cu_hmac_sha256(psk, psk_len, buf, labellen + PSK_NONCE_LEN + client_id_len, out_mac);
}

int psk_verify_proof(const uint8_t *psk, size_t psk_len,
                     const uint8_t *client_id, size_t client_id_len,
                     const uint8_t srv_nonce[PSK_NONCE_LEN],
                     const uint8_t mac[CU_SHA256_LEN]) {
    uint8_t expect[CU_SHA256_LEN];
    if (psk_make_proof(psk, psk_len, client_id, client_id_len, srv_nonce, expect) != 0) return -1;
    return cu_ct_eq(expect, mac, CU_SHA256_LEN) ? 0 : -2;
}
