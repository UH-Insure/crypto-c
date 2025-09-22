#ifndef PROTOCOLS_SIG_H
#define PROTOCOLS_SIG_H

#include <stdint.h>
#include <stddef.h>

/* Pure-C RSA-PSS (SHA-256) verification
 * - n: modulus (big-endian), len 256 (2048-bit)
 * - e: public exponent (commonly 65537)
 * - msg: original message buffer (or pre-hashed)
 * - If msg_is_hash = 0, we hash msg with SHA-256 internally.
 *   If msg_is_hash = 1, we treat msg as 32-byte SHA-256.
 * - salt_len: required PSS salt length (commonly 32); use same value signer used.
 * - sig: signature (big-endian), len 256
 * Returns 0 on success (valid signature), negative on failure.
 */
int sig_rsapss_sha256_verify(const uint8_t *n, size_t n_len, uint32_t e,
                             const uint8_t *msg, size_t msg_len, int msg_is_hash,
                             size_t salt_len,
                             const uint8_t *sig, size_t sig_len);

#endif /* PROTOCOLS_SIG_H */
