#include <stdint.h>
#include <stddef.h>
#include "primitives/sha256.h"

void sha256_once(const uint8_t *data, size_t len, uint8_t out[32]) {
    sha256(data, len, out);
}