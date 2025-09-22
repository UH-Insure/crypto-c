# Diffie–Hellman (X25519) Key Exchange Protocol

## Overview

This protocol establishes a shared secret between two parties using Curve25519
(X25519). Both generate ephemeral keypairs and derive the same session key.

Implemented in:

* `include/protocols/dh.h`
* `src/protocols/dh.c`
* Uses `cu_x25519_keypair` and `cu_x25519_shared` from `crypto_utils.c`
* Tested by `tests/test_dh.c`

## Steps

1. **Ephemeral Key Generation:**

   ```c
   int dh_ephemeral_keypair(uint8_t sk[32], uint8_t pk[32]);
   ```

   * Generates 32-byte secret, clamps it per RFC 7748.
   * Computes public key = scalar \* basepoint (9).

2. **Shared Secret Computation:**

   ```c
   int dh_derive_session_key(my_sk, their_pk, context, psk_opt, out_key);
   ```

   * Computes `shared = X25519(my_sk, their_pk)`.
   * Runs HKDF-SHA256:

     * Salt = PSK (optional) or zeros
     * Info = `"X25519-CTX" || context`
     * Output = 32-byte session key

3. **Session Key Use:**
   The derived key can be used with AEAD (see `psk.md`) or higher-level protocols.

## Security Notes

* Ephemeral DH ensures forward secrecy.
* HKDF context binding ties session key to transcript/application.
* Optional PSK mode gives hybrid forward secrecy + authentication.

## Example

See `tests/test_dh.c` for key agreement between two parties.

---