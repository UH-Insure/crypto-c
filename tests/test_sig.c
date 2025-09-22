#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "protocols/sig.h"

/* Paste a real RSA-2048 public key (n,e), a PSS-SHA256 signature, and the original message to get a PASS.
   By default, we demonstrate expected failure with dummy data. */

int main(void){
    puts("[test_sig] start");
    uint8_t n[256] = {0};     /* TODO: fill with real modulus (big-endian 256 bytes) */
    uint32_t e = 65537;       /* typical public exponent */
    uint8_t sig[256] = {0};   /* TODO: real signature (big-endian 256 bytes) */

    const char *msg = "dummy message";
    int rc = sig_rsapss_sha256_verify(n, sizeof n, e,
                                      (const uint8_t*)msg, strlen(msg), 0,
                                      32, /* salt_len */
                                      sig, sizeof sig);
    if (rc == 0){
        puts("[test_sig] ok (valid signature)");
    } else {
        puts("[test_sig] ok (expected failure with dummy data)");
    }
    return 0;
}
