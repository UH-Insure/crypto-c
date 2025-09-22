# Pre-Shared Key (PSK) Handshake Protocol

## Overview

The PSK handshake protocol allows two parties who share a secret key to derive
fresh session keys and prove possession of the PSK.

Implemented in:

* `include/protocols/psk.h`
* `src/protocols/psk.c`
* Tested by `tests/test_psk.c`

## Steps

1. **Challenge:**
   The server generates a 32-byte random nonce:

   ```c
   int psk_challenge(uint8_t nonce[32]);
   ```
2. **Key Derivation:**
   Both sides derive AEAD keys (`k_enc`, `k_mac`) using HKDF-SHA256:

   ```c
   int psk_derive_aead_keys(psk, client_id, srv_nonce, k_enc, k_mac);
   ```

   * Salt: server nonce
   * Info: "PSK-AEAD" || client\_id
   * Output: AES-128 encryption key + HMAC-SHA256 key
3. **Proof of Possession:**
   The client produces a MAC over `"PSK-PROOF" || nonce || client_id`:

   ```c
   int psk_make_proof(...);
   int psk_verify_proof(...);
   ```
4. **Authenticated Encryption:**
   The derived keys are used in Encrypt-then-MAC AEAD:

   ```c
   cu_aead_etm_encrypt(...);
   cu_aead_etm_decrypt(...);
   ```

## Security Notes

* Fresh nonce ensures unique session keys.
* Proof step prevents replay.
* Encrypt-then-MAC provides confidentiality + integrity.

## Example

See `tests/test_psk.c` for a full round-trip.

---

