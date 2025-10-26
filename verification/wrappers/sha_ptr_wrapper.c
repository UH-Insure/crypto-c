#include <stdint.h>

extern void sha256(const uint8_t *buf, uint64_t len, uint8_t *out);


void sha256_once(uint8_t *buf, uint64_t len, uint8_t *out) {
  sha256(buf, len, out);
}