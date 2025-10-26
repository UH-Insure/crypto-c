#include <stdint.h>

extern void aes128_set_key(void *rk, const uint8_t *key);
extern void aes128_encrypt_block(void *rk, const uint8_t *in, uint8_t *out);


void aes128_ecb_encrypt_one(uint8_t *key, uint8_t *pt, uint8_t *out) {
  uint64_t rk[22];
  aes128_set_key((void *)rk, key);
  aes128_encrypt_block((void *)rk, pt, out);
}