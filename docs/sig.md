# RSA-PSS Signature Verification Protocol

## Overview

RSA-PSS provides probabilistic signature verification with strong security
guarantees (PKCS#1 v2.2, EMSA-PSS, SHA-256).

Implemented in:

* `include/protocols/sig.h`
* `src/protocols/sig.c`
* Depends on:

  * `primitives/bn2048.c` (fixed 2048-bit bigint)
  * `primitives/mgf1.c` (MGF1 with SHA-256)
  * `primitives/sha256.c`
* Tested by `tests/test_sig.c` (uses dummy values unless real key+sig are provided).

## Steps

1. **Inputs:**

   * RSA modulus `n` (2048-bit, 256 bytes big-endian)
   * Public exponent `e` (commonly 65537)
   * Message `M`
   * Signature `S` (256 bytes big-endian)

2. **Verification:**

   ```c
   int sig_rsapss_sha256_verify(n, 256, e,
                                msg, msg_len, msg_is_hash,
                                salt_len,
                                sig, 256);
   ```

   * Computes `m = S^e mod n` using bigint modular exponentiation.
   * Converts `m` to encoded message `EM` (256 bytes).
   * Runs EMSA-PSS-VERIFY:

     * Splits into `maskedDB || H || 0xBC`
     * Unmasks DB with `MGF1(H)`
     * Checks padding: PS (0x00…), 0x01 separator, salt
     * Recomputes `H' = Hash(0x00^8 || mHash || salt)`
     * Verifies `H' == H`.

3. **Return Codes:**

   * `0`: valid signature
   * `<0`: failure (invalid padding, mismatch, etc.)

## Security Notes

* Strict EMSA-PSS checks required for security.
* Constant-time bignum not yet implemented — safe for research, not production.
* Supports only RSA-2048 with SHA-256 + saltlen=32.

## Example

`tests/test_sig.c` shows how to call verify with dummy data. Replace `n` and
`sig` with real values to see a passing verification.
