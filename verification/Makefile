# Toolchain (override CC/LLVMBIN from env if you prefer)
CC      ?= /opt/homebrew/opt/llvm@16/bin/clang
LLVMBIN ?= /opt/homebrew/opt/llvm@16/bin

CFLAGS  ?= -O0 -g -std=c11 -I../include -emit-llvm -Wall -Wextra -Wpedantic

BUILD   := build

# library sources we compile to bitcode
LIB_AES_SRC    := ../src/primitives/aes.c
LIB_SHA256_SRC := ../src/primitives/sha256.c

# pointer wrappers you keep in this repo
WRAP_AES_PTR   := wrappers/aes_ptr_wrapper.c
WRAP_SHA_PTR   := wrappers/sha_ptr_wrapper.c

AES_BC         := $(BUILD)/aes.bc
SHA_BC         := $(BUILD)/sha256.bc
AES_WRAP_BC    := $(BUILD)/aes_ptr_wrapper.bc
SHA_WRAP_BC    := $(BUILD)/sha_ptr_wrapper.bc

AES_COMBINED   := $(BUILD)/aes_ptr_combined.bc
SHA_COMBINED   := $(BUILD)/sha_ptr_combined.bc

SAW_SCRIPTS    := saw/aes128_ecb_kat.saw saw/sha256_empty_kat.saw

.PHONY: all build prove clean

all: prove

build: $(AES_COMBINED) $(SHA_COMBINED)

$(BUILD):
	@mkdir -p $(BUILD)

$(AES_BC): $(LIB_AES_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHA_BC): $(LIB_SHA256_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(AES_WRAP_BC): $(WRAP_AES_PTR) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHA_WRAP_BC): $(WRAP_SHA_PTR) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(AES_COMBINED): $(AES_WRAP_BC) $(AES_BC) | $(BUILD)
	$(LLVMBIN)/llvm-link $^ -o $@

$(SHA_COMBINED): $(SHA_WRAP_BC) $(SHA_BC) | $(BUILD)
	$(LLVMBIN)/llvm-link $^ -o $@

prove: build
	saw saw/aes128_ecb_kat.saw
	saw saw/sha256_empty_kat.saw

clean:
	rm -rf $(BUILD)
