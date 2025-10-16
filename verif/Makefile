# verif/Makefile
CC ?= cc
CFLAGS ?= -O0 -g -std=c11 -I../include -emit-llvm -c
LDFLAGS ?=

BCDIR := build
WRAPDIR := wrappers

# If you add cryptol-specs as a submodule, set this path:
#   git submodule add https://github.com/GaloisInc/cryptol-specs.git external/cryptol-specs
CRYPTOL_SPECS_DIR ?= ../external/cryptol-specs

.PHONY: all clean sha256-bc aes128-bc prove-sha256 prove-aes128 kat-sha256 kat-aes128

all: prove-sha256 prove-aes128

# ---------------- LLVM bitcode ----------------
sha256-bc:
	mkdir -p $(BCDIR)
	$(CC) $(CFLAGS) ../src/primitives/sha256.c -o $(BCDIR)/sha256.bc
	$(CC) $(CFLAGS) $(WRAPDIR)/sha_wrapper.c          -o $(BCDIR)/sha_wrapper.bc

aes128-bc:
	mkdir -p $(BCDIR)
	$(CC) $(CFLAGS) ../src/primitives/aes.c    -o $(BCDIR)/aes.bc
	$(CC) $(CFLAGS) $(WRAPDIR)/aes_wrapper.c  -o $(BCDIR)/aes_wrapper.bc

# ---------------- SAW proofs (Cryptol equivalence) ----------------
prove-sha256: sha256-bc
	saw saw/sha256_equiv.saw

prove-aes128: aes128-bc
	saw saw/aes128_equiv.saw

# ---------------- Fallback Known-Answer-Tests (if cryptol-specs absent) ----------------
kat-sha256: sha256-bc
	SPEC_FALLBACK=1 saw saw/sha256_equiv.saw

kat-aes128: aes128-bc
	SPEC_FALLBACK=1 saw saw/aes128_equiv.saw

clean:
	rm -rf $(BCDIR)