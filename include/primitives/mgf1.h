#ifndef PRIMITIVES_MGF1_H
#define PRIMITIVES_MGF1_H

#include <stdint.h>
#include <stddef.h>

/* MGF1 with SHA-256: fills mask[mask_len] from seed[seed_len] */
void mgf1_sha256(uint8_t *mask, size_t mask_len, const uint8_t *seed, size_t seed_len);

#endif /* PRIMITIVES_MGF1_H */
