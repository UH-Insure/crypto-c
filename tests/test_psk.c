#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "crypto_utils.h"
#include "protocols/psk.h"

static void hexdump(const char *label, const uint8_t *buf, size_t len) {
    printf("%s:", label);
    for (size_t i = 0; i < len; ++i) printf(" %02X", buf[i]);
    printf("\n");
}

int main(void) {
    puts("[test_psk] start");
    const uint8_t psk[16] = {0,1,2,3,4,5,6,7, 0xA0,0xB1,0xC2,0xD3,0xE4,0xF5,0x11,0x22};
    const uint8_t cid[] = "client-01";

    /* 1) Server generates challenge nonce */
    uint8_t nonce[PSK_NONCE_LEN];
    assert(psk_challenge(nonce) == 0);

    /* 2) Both sides derive AEAD keys */
    uint8_t k_enc1[PSK_KEY_ENC_LEN], k_mac1[PSK_KEY_MAC_LEN];
    uint8_t k_enc2[PSK_KEY_ENC_LEN], k_mac2[PSK_KEY_MAC_LEN];
    assert(psk_derive_aead_keys(psk, sizeof psk, cid, sizeof cid - 1, nonce, k_enc1, k_mac1) == 0);
    assert(psk_derive_aead_keys(psk, sizeof psk, cid, sizeof cid - 1, nonce, k_enc2, k_mac2) == 0);
    assert(cu_ct_eq(k_enc1, k_enc2, PSK_KEY_ENC_LEN));
    assert(cu_ct_eq(k_mac1, k_mac2, PSK_KEY_MAC_LEN));

    /* 3) Client proves PSK possession */
    uint8_t proof[CU_SHA256_LEN];
    assert(psk_make_proof(psk, sizeof psk, cid, sizeof cid - 1, nonce, proof) == 0);
    assert(psk_verify_proof(psk, sizeof psk, cid, sizeof cid - 1, nonce, proof) == 0);

    /* 4) AEAD EtM roundtrip */
    const uint8_t aad[] = "psk-aead-demo";
    uint8_t iv[CU_CTR_IV_LEN]; assert(cu_rand_bytes(iv, sizeof iv) == 0);

    const uint8_t msg[] = "hello etm";
    uint8_t ct[sizeof msg], tag[CU_AEAD_TAG_LEN], pt[sizeof msg];

    assert(cu_aead_etm_encrypt(k_enc1, k_mac1, iv, aad, sizeof aad - 1, msg, sizeof msg, ct, tag) == 0);
    assert(cu_aead_etm_decrypt(k_enc2, k_mac2, iv, aad, sizeof aad - 1, ct, sizeof ct, tag, pt) == 0);
    assert(memcmp(msg, pt, sizeof msg) == 0);

    hexdump("[iv]", iv, sizeof iv);
    puts("[test_psk] ok");
    return 0;
}
