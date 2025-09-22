#include "primitives/bn2048.h"
#include <string.h>

void bn2048_from_be(bn2048 *x, const uint8_t *be, size_t len){
    memset(x, 0, sizeof *x);
    /* be[0] is MSB; map to little-endian limbs */
    for (size_t i=0;i<len;i++){
        size_t bitpos = (len-1 - i);
        size_t limb = bitpos / 4;
        size_t off  = bitpos % 4;
        if (limb < BN2048_LIMBS){
            x->v[limb] |= ((uint32_t)be[i]) << (off*8);
        }
    }
}

void bn2048_to_be(uint8_t *be, size_t len, const bn2048 *x){
    memset(be, 0, len);
    for (size_t i=0;i<len;i++){
        size_t bitpos = (len-1 - i);
        size_t limb = bitpos / 4;
        size_t off  = bitpos % 4;
        if (limb < BN2048_LIMBS){
            be[i] = (uint8_t)((x->v[limb] >> (off*8)) & 0xFF);
        }
    }
}

int bn2048_is_zero(const bn2048 *x){
    uint32_t acc = 0;
    for (int i=0;i<BN2048_LIMBS;i++) acc |= x->v[i];
    return acc == 0;
}

int bn2048_cmp(const bn2048 *a, const bn2048 *b){
    for (int i=BN2048_LIMBS-1;i>=0;i--){
        if (a->v[i] < b->v[i]) return -1;
        if (a->v[i] > b->v[i]) return 1;
    }
    return 0;
}

uint32_t bn2048_sub(bn2048 *r, const bn2048 *a, const bn2048 *b){
    uint64_t borrow = 0;
    for (int i=0;i<BN2048_LIMBS;i++){
        uint64_t ai = a->v[i];
        uint64_t bi = b->v[i];
        uint64_t t = ai - bi - borrow;
        r->v[i] = (uint32_t)t;
        borrow = (t >> 63) & 1; /* if underflow, high bit set after unsigned wrap */
    }
    return (uint32_t)borrow;
}

void bn2048_shl1(bn2048 *r, const bn2048 *a){
    uint32_t carry = 0;
    for (int i=0;i<BN2048_LIMBS;i++){
        uint32_t nxt = (a->v[i] >> 31) & 1;
        r->v[i] = (a->v[i] << 1) | carry;
        carry = nxt;
    }
}

void bn2048_shr1(bn2048 *r, const bn2048 *a){
    uint32_t carry = 0;
    for (int i=BN2048_LIMBS-1;i>=0;i--){
        uint32_t nxt = a->v[i] & 1;
        r->v[i] = (a->v[i] >> 1) | (carry << 31);
        carry = nxt;
    }
}

void bn4096_mul(bn4096 *r, const bn2048 *a, const bn2048 *b){
    uint64_t acc[BN4096_LIMBS] = {0};
    for (int i=0;i<BN2048_LIMBS;i++){
        uint64_t carry = 0;
        uint64_t ai = a->v[i];
        for (int j=0;j<BN2048_LIMBS;j++){
            uint64_t vij = acc[i+j] + ai * (uint64_t)b->v[j] + carry;
            acc[i+j] = (uint32_t)vij;
            carry = vij >> 32;
        }
        acc[i+BN2048_LIMBS] += carry;
    }
    for (int k=0;k<BN4096_LIMBS;k++) r->v[k] = (uint32_t)acc[k];
}

/* rem = r mod m, using a simple binary long-division style reduction.
 * Not optimized; adequate for occasional RSA verify.
 */
void bn4096_mod_bn2048(bn2048 *rem, const bn4096 *r, const bn2048 *m){
    /* Copy r into 4096-bit temp represented as 128 limbs; then do shift-subtract with aligned modulus. */
    uint32_t tmp[BN4096_LIMBS];
    for (int i=0;i<BN4096_LIMBS;i++) tmp[i] = r->v[i];

    /* Prepare modulus aligned buffer (shift left by k bits while k allows) */
    /* We operate per-limb alignment first (fast path). */
    /* While top half nonzero, subtract appropriately. */
    /* Convert tmp to bn-like operations: we’ll iteratively compare and subtract m shifted. */
    for (int pos = BN4096_LIMBS-1; pos >= BN2048_LIMBS; pos--){
        if (tmp[pos] == 0) continue;
        /* Determine how many limb-shifts we can do: align m's top limb to 'pos' */
        int shift_limbs = pos - (BN2048_LIMBS-1);
        /* Construct m << (shift_limbs*32) into a window and subtract as many times as needed (tmp[pos] gives an estimate). */
        uint64_t factor = tmp[pos]; /* crude estimate; subtract factor * (m << shift) */
        /* Build product = (m * factor) << (shift_limbs*32) */
        uint64_t carry = 0;
        for (int i=0;i<BN2048_LIMBS;i++){
            __uint128_t mult = (__uint128_t)m->v[i] * factor + carry;
            uint32_t lo = (uint32_t)mult;
            carry = (uint64_t)(mult >> 32);
            int idx = i + shift_limbs;
            __uint128_t sub = (__uint128_t)tmp[idx] - lo;
            tmp[idx] = (uint32_t)sub;
            /* borrow propagate with carry included next step; keep it simple: */
            /* Next limb: include borrow into subtraction by adding to product high part */
            __uint128_t nxt = (__uint128_t)tmp[idx+1] - ((uint32_t)carry) - ((sub >> 63) & 1);
            tmp[idx+1] = (uint32_t)nxt;
            carry = (uint64_t)((nxt >> 63) & 1); /* propagate borrow as carry for following high words */
            /* This simple approach is conservative; acceptable for small factor values. */
        }
        /* Zero out processed high limb */
        tmp[pos] = 0;
    }

    /* Now tmp fits in 2048 bits (lower 64 limbs), but may still be >= m. Do final reduction by repeated compare-subtract. */
    for (;;){
        int ge = 0;
        /* compare lower 64 limbs */
        for (int i=BN2048_LIMBS-1;i>=0;i--){
            if (tmp[i] > m->v[i]) { ge = 1; break; }
            if (tmp[i] < m->v[i]) { ge = 0; goto done_cmp; }
        }
        ge = 1;
    done_cmp:
        if (!ge) break;
        /* tmp = tmp - m */
        uint64_t borrow = 0;
        for (int i=0;i<BN2048_LIMBS;i++){
            uint64_t t = (uint64_t)tmp[i] - m->v[i] - borrow;
            tmp[i] = (uint32_t)t;
            borrow = (t >> 63) & 1;
        }
    }

    for (int i=0;i<BN2048_LIMBS;i++) rem->v[i] = tmp[i];
}

void bn2048_modmul(bn2048 *z, const bn2048 *x, const bn2048 *y, const bn2048 *m){
    bn4096 prod;
    bn4096_mul(&prod, x, y);
    bn4096_mod_bn2048(z, &prod, m);
}

void bn2048_modexp_small(bn2048 *out, const bn2048 *base, uint32_t e, const bn2048 *n){
    /* Square-and-multiply with modular reduction; e is small (<= 32 bits). */
    bn2048 result = {0};
    result.v[0] = 1;
    bn2048 pow = *base;

    while (e){
        if (e & 1){
            bn2048 t;
            bn2048_modmul(&t, &result, &pow, n);
            result = t;
        }
        e >>= 1;
        if (e){
            bn2048 t2;
            bn2048_modmul(&t2, &pow, &pow, n);
            pow = t2;
        }
    }
    *out = result;
}
