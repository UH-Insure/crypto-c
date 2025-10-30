# Crypto-C

C code repository scaffold for the **Automated Reasoning for Cryptography** project (University of Houston, 2025).

This repo provides:

* A reproducible Docker/Podman environment pinned to Clang 14.
* Makefile and CMakeLists.txt for building C code.
* Protocols: PSK handshake, X25519 Diffie–Hellman, RSA-PSS signatures.
* Primitives: SHA-256, AES-128 (ECB).
* A test suite with known-answer tests and protocol checks.
* Scripts for setup on Fedora/Docker/Podman and for Google Colab.

---

## Repository Layout

```
crypto-c/
├── .dockerignore
├── .gitignore
├── CMakeLists.txt        # Alternative build system (CMake)
├── Dockerfile            # Ubuntu 22.04 container with clang-14
├── Makefile              # Default build system
├── README.md             # This file
├── LICENSE               # License (MIT)
├── include/              # Public headers
│   ├── crypto_utils.h    # Shared crypto utilities (pure C)
│   ├── primitives/       # Pure C primitive APIs
│   │   ├── aes.h
│   │   ├── sha256.h
│   │   ├── bn2048.h
│   │   └── mgf1.h
│   └── protocols/        # Protocol APIs
│       ├── dh.h
│       ├── psk.h
│       └── sig.h
├── scripts/              # Environment setup scripts
│   ├── setup.sh          # Bash setup (Docker/Podman)
│   ├── setup.zsh         # Zsh setup (Docker/Podman)
│   └── colab_setup.sh    # Colab setup (clang-14, no Docker)
├── src/                  # Source code
│   ├── crypto_utils.c    # Randomness, HMAC, HKDF, AES-CTR, AEAD, X25519
│   ├── primitives/
│   │   ├── aes.c         # AES-128 ECB (FIPS-197)
│   │   ├── sha256.c      # SHA-256 (FIPS-180-4)
│   │   ├── bn2048.c      # 2048-bit bigint for RSA
│   │   └── mgf1.c        # MGF1 with SHA-256
│   └── protocols/
│       ├── dh.c          # Diffie–Hellman (X25519) session key derivation
│       ├── psk.c         # PSK handshake, key derivation, proof MAC
│       └── sig.c         # RSA-PSS verification
├── tests/                # Unit and known-answer tests
│   ├── test_aes.c
│   ├── test_dh.c
│   ├── test_psk.c
│   ├── test_sha256.c
│   └── test_sig.c
└── docs/                 # Documentation
    ├── README.md         # Documentation index
    ├── psk.md
    ├── dh.md
    ├── sig.md
    ├── sha256.md
    └── aes128.md
```

---

## 1. Setup Instructions

The setup process for Docker/Podman and Google Colab is in [**SETUP.md**](./SETUP.md).

Please refer to that file for detailed installation and usage instructions.

---

## 2. Protocols Implemented

The following cryptographic protocols are implemented in this repository:

1. **PSK Handshake** (`protocols/psk.c` / `psk.h`)

   * Server nonce challenge.
   * HKDF-based session key derivation from PSK + nonce + client\_id.
   * HMAC-SHA256 proof of key possession.

2. **Diffie–Hellman (X25519)** (`protocols/dh.c` / `dh.h`)

   * Ephemeral X25519 key exchange.
   * Session key derived with HKDF using optional PSK and transcript context.

3. **RSA-PSS Authentication** (`protocols/sig.c` / `sig.h`)

   * Server authentication using RSA-PSS signatures.

---

## 3. Cryptographic Primitives

The following primitives are implemented in pure C:

1. **SHA-256** (`primitives/sha256.c` / `sha256.h`)

   * Full streaming SHA-256 implementation.
   * Verified against known-answer test vectors.

2. **AES-128 (ECB)** (`primitives/aes.c` / `aes.h`)

   * Key expansion and single-block encrypt/decrypt.
   * Verified against FIPS-197 C.1 test vector.

3. **BN2048** (`primitives/bn2048.c` / `bn2048.h`)

   * Fixed-size 2048-bit integer arithmetic for RSA.

4. **MGF1** (`primitives/mgf1.c` / `mgf1.h`)

   * Mask Generation Function 1 using SHA-256.

---

## 4. Tests

The `tests/` folder contains:

* Protocol round-trip tests (`test_psk.c`, `test_dh.c`, `test_sig.c`).
* Known-answer tests for primitives (`test_sha256.c`, `test_aes.c`).

Run all with:

```bash
make test
```

---

## 5. Documentation

Each protocol and primitive has its own detailed documentation under the [`docs/`](./docs/) folder:

* [PSK Handshake](./docs/psk.md)
* [Diffie–Hellman (X25519)](./docs/dh.md)
* [RSA-PSS Signature Verification](./docs/sig.md)
* [SHA-256](./docs/sha256.md)
* [AES-128](./docs/aes128.md)

The [Documentation Index](./docs/README.md) links all of them together.

---


## TODO

python API to docker file for collab


## License

MIT (see `LICENSE`).




