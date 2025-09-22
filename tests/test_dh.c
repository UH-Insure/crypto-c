#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "crypto_utils.h"
#include "protocols/dh.h"

int main(void) {
    puts("[test_dh] start");
    uint8_t a_sk[CU_X25519_SKLEN], a_pk[CU_X25519_PKLEN];
    uint8_t b_sk[CU_X25519_SKLEN], b_pk[CU_X25519_PKLEN];
    assert(dh_ephemeral_keypair(a_sk, a_pk) == 0);
    assert(dh_ephemeral_keypair(b_sk, b_pk) == 0);

    const uint8_t ctx[] = "transcript-v1";
    uint8_t a_key[DH_SESSION_KEY_LEN], b_key[DH_SESSION_KEY_LEN];
    assert(dh_derive_session_key(a_sk, b_pk, ctx, sizeof ctx - 1, NULL, 0, a_key) == 0);
    assert(dh_derive_session_key(b_sk, a_pk, ctx, sizeof ctx - 1, NULL, 0, b_key) == 0);
    assert(cu_ct_eq(a_key, b_key, DH_SESSION_KEY_LEN));
    puts("[test_dh] ok");
    return 0;
}
