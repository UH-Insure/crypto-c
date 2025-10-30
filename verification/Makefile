# === verification/Makefile (clean) ===

# Toolchain
CC       ?= clang-16
LLVMBIN  ?= llvm-link-16
CFLAGS   ?= -O0 -g -std=c11 -I../include -emit-llvm

# Paths
BUILD_DIR := build

# Sources
AES_LIB_SRC := ../src/primitives/aes.c
SHA_LIB_SRC := ../src/primitives/sha256.c
AES_WRAP    := wrappers/aes_ptr_wrapper.c
SHA_WRAP    := wrappers/sha_ptr_wrapper.c

# Outputs
AES_BC_LIB      := $(BUILD_DIR)/aes.bc
SHA_BC_LIB      := $(BUILD_DIR)/sha256.bc
AES_BC_WRAP     := $(BUILD_DIR)/aes_ptr_wrapper.bc
SHA_BC_WRAP     := $(BUILD_DIR)/sha_ptr_wrapper.bc
AES_BC_COMBINED := $(BUILD_DIR)/aes_ptr_combined.bc
SHA_BC_COMBINED := $(BUILD_DIR)/sha_ptr_combined.bc

# Default: build combined bitcode
all: $(AES_BC_COMBINED) $(SHA_BC_COMBINED)

# Ensure build/ exists once, use order-only prerequisite
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Compile to bitcode
$(AES_BC_LIB): $(AES_LIB_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHA_BC_LIB): $(SHA_LIB_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(AES_BC_WRAP): $(AES_WRAP) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHA_BC_WRAP): $(SHA_WRAP) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link wrappers+libs
$(AES_BC_COMBINED): $(AES_BC_WRAP) $(AES_BC_LIB) | $(BUILD_DIR)
	$(LLVMBIN) $^ -o $@

$(SHA_BC_COMBINED): $(SHA_BC_WRAP) $(SHA_BC_LIB) | $(BUILD_DIR)
	$(LLVMBIN) $^ -o $@

# Run the two SAW proofs that live here
run-saw: all
	@echo "[*] Running SAW in verification/saw"
	@saw saw/aes128_ecb_kat.saw
	@saw saw/sha256_empty_kat.saw

clean:
	rm -rf $(BUILD_DIR)
.PHONY: all run-saw clean