#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "primitives/sha256.h"

static void hex(const uint8_t *b, size_t n, char *out){
    static const char *H="0123456789abcdef";
    for (size_t i=0;i<n;i++){ out[2*i]=H[b[i]>>4]; out[2*i+1]=H[b[i]&0xF]; }
    out[2*n]='\0';
}

int main(void){
    puts("[test_sha256] start");
    uint8_t out[SHA256_DIGEST_LEN];
    char hexout[SHA256_DIGEST_LEN*2+1];

    /* Test vector: SHA256("") */
    sha256((const uint8_t *)"", 0, out);
    hex(out, 32, hexout);
    assert(strcmp(hexout,
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")==0);

    /* Test vector: SHA256(\"abc\") */
    sha256((const uint8_t *)"abc", 3, out);
    hex(out, 32, hexout);
    assert(strcmp(hexout,
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")==0);

    puts("[test_sha256] ok");
    return 0;
}
