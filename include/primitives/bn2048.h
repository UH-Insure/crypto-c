#ifndef PRIMITIVES_BN2048_H
#define PRIMITIVES_BN2048_H

#include <stdint.h>
#include <stddef.h>

/* Fixed-size 2048-bit big integer
 * - Limbs are 32-bit, little-endian (limb[0] = least significant 32 bits).
 * - Supports only the operations needed for RSA public-key verify:
 *   load/store, compare, subtract (borrow), left/right shifts,
 *   full multiply (4096-bit product), modulo (binary long-division style),
 *   modular multiplication, modular exponentiation with small public e (e.g., 65537).
 */

#define BN2048_LIMBS 64   /* 64 * 32 = 2048 bits */
#define BN4096_LIMBS 128  /* product size */

typedef struct { uint32_t v[BN2048_LIMBS]; } bn2048;
typedef struct { uint32_t v[BN4096_LIMBS]; } bn4096;

/* Load/store big-endian byte arrays (len must be 256 for 2048-bit) */
void bn2048_from_be(bn2048 *x, const uint8_t *be, size_t len);
void bn2048_to_be(uint8_t *be, size_t len, const bn2048 *x);

/* Basic ops / predicates */
int  bn2048_is_zero(const bn2048 *x);
int  bn2048_cmp(const bn2048 *a, const bn2048 *b);  /* -1,0,1 */
uint32_t bn2048_sub(bn2048 *r, const bn2048 *a, const bn2048 *b); /* r = a - b; returns borrow */

/* Shifts */
void bn2048_shl1(bn2048 *r, const bn2048 *a); /* r = a << 1 */
void bn2048_shr1(bn2048 *r, const bn2048 *a); /* r = a >> 1 */

/* Multiply: r = a * b (4096-bit) */
void bn4096_mul(bn4096 *r, const bn2048 *a, const bn2048 *b);

/* r = r % m  (reduce 4096-bit r modulo 2048-bit m) */
void bn4096_mod_bn2048(bn2048 *rem, const bn4096 *r, const bn2048 *m);

/* Modular multiplication: z = (x*y) mod m */
void bn2048_modmul(bn2048 *z, const bn2048 *x, const bn2048 *y, const bn2048 *m);

/* Modular exponentiation: out = base^e mod n; e is 32-bit public exponent (e.g., 65537) */
void bn2048_modexp_small(bn2048 *out, const bn2048 *base, uint32_t e, const bn2048 *n);

#endif /* PRIMITIVES_BN2048_H */
