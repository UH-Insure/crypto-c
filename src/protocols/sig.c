#include "protocols/sig.h"
#include "primitives/bn2048.h"
#include "primitives/mgf1.h"
#include "primitives/sha256.h"
#include <string.h>
#include <stdlib.h>

/* EMSA-PSS-VERIFY (PKCS#1 v2.2) with SHA-256, emBits = modBits-1 */
static int emsa_pss_verify_sha256(const uint8_t *mhash, size_t salt_len,
                                  const uint8_t *EM, size_t emLen){
    /* EM = 0x00 || maskedDB || H || 0xBC, where |H| = 32, |maskedDB| = emLen - hLen - 1 */
    if (emLen < 32 + 2) return -1;
    if (EM[emLen-1] != 0xBC) return -2;

    const size_t hLen = 32;
    const size_t dbLen = emLen - hLen - 1;
    const uint8_t *H = EM + dbLen;
    const uint8_t *maskedDB = EM + 1;

    /* The leftmost 8*emLen - emBits bits of the leftmost octet in maskedDB shall be zero.
       For typical RSA (emBits = modBits-1), this means the first (most significant) bit of EM[0] is zero.
       We already require EM[0] == 0x00 (as specified). So skip extra bit masking here. */
    if (EM[0] != 0x00) return -3;

    /* Construct DB = PS || 0x01 || salt, after unmasking: DB = maskedDB XOR MGF1(H, dbLen) */
    uint8_t *dbMask = (uint8_t*)malloc(dbLen);
    uint8_t *DB     = (uint8_t*)malloc(dbLen);
    if (!dbMask || !DB){ free(dbMask); free(DB); return -4; }
    mgf1_sha256(dbMask, dbLen, H, hLen);
    for (size_t i=0;i<dbLen;i++) DB[i] = maskedDB[i] ^ dbMask[i];

    /* Set leftmost 8*emLen - emBits bits of DB[0] to zero.
       With EM[0]==0x00 and typical emBits, this implies the high bit of DB[0] must be zero.
       Clear it just in case. */
    DB[0] &= 0x7F;

    /* Verify PS = 0x00...00 and separator 0x01 before salt */
    size_t ps_len = dbLen - salt_len - 1;
    for (size_t i=0;i<ps_len;i++) if (DB[i] != 0x00){ free(dbMask); free(DB); return -5; }
    if (DB[ps_len] != 0x01){ free(dbMask); free(DB); return -6; }
    const uint8_t *salt = DB + ps_len + 1;

    /* Compute H' = Hash(0x00..00(8 bytes) || mhash || salt) */
    uint8_t prefix8[8] = {0};
    uint8_t Hprime[32];
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, prefix8, 8);
    sha256_update(&ctx, mhash, 32);
    sha256_update(&ctx, salt, salt_len);
    sha256_final(&ctx, Hprime);

    int ok = 1;
    for (size_t i=0;i<32;i++) if (Hprime[i] != H[i]) { ok = 0; break; }

    memset(Hprime, 0, sizeof Hprime);
    memset(dbMask, 0, dbLen);
    memset(DB, 0, dbLen);
    free(dbMask); free(DB);
    return ok ? 0 : -7;
}

int sig_rsapss_sha256_verify(const uint8_t *n_be, size_t n_len, uint32_t e,
                             const uint8_t *msg, size_t msg_len, int msg_is_hash,
                             size_t salt_len,
                             const uint8_t *sig_be, size_t sig_len){
    if (n_len != 256 || sig_len != 256) return -10; /* this implementation is 2048-bit only */
    if (e == 0 || (e & 1) == 0) return -11; /* e must be odd and nonzero; typically 65537 */

    /* Compute message hash if needed */
    uint8_t mhash[32];
    if (msg_is_hash){
        if (msg_len != 32) return -12;
        memcpy(mhash, msg, 32);
    } else {
        sha256(msg, msg_len, mhash);
    }

    /* m = s^e mod n */
    bn2048 n, s, m;
    bn2048_from_be(&n, n_be, n_len);
    bn2048_from_be(&s, sig_be, sig_len);
    bn2048_modexp_small(&m, &s, e, &n);

    /* EM = I2OSP(m, k) of length k (here 256) */
    uint8_t EM[256];
    bn2048_to_be(EM, sizeof EM, &m);

    /* Verify EMSA-PSS */
    return emsa_pss_verify_sha256(mhash, salt_len, EM, sizeof EM);
}
