# ===== config =====
CC      ?= cc
LLVMBIN ?=                                
CFLAGS  ?= -O0 -g -std=c11 -I../include  
LDFLAGS ?=

# ===== sources =====
AES_SRC      := ../src/primitives/aes.c
SHA_SRC      := ../src/primitives/sha256.c
AES_WRAP_PTR := wrappers/aes_ptr_wrapper.c
SHA_WRAP_PTR := wrappers/sha_ptr_wrapper.c

# ===== build dir and artifacts =====
BUILD := build
AES_BC      := $(BUILD)/aes.bc
SHA_BC      := $(BUILD)/sha256.bc
AES_WRAP_BC := $(BUILD)/aes_ptr_wrapper.bc
SHA_WRAP_BC := $(BUILD)/sha_ptr_wrapper.bc

AES_COMB_PTR := $(BUILD)/aes_ptr_combined.bc
SHA_COMB_PTR := $(BUILD)/sha_ptr_combined.bc

# pick llvm-link
LLVM_LINK := $(if $(LLVMBIN),$(LLVMBIN)/llvm-link,llvm-link)

.PHONY: all build clean
all: build

build: $(AES_COMB_PTR) $(SHA_COMB_PTR)
	@echo "Built pointer-combined bitcode:"
	@ls -lh $(AES_COMB_PTR) $(SHA_COMB_PTR)

$(BUILD):
	@mkdir -p $(BUILD)

# bitcode compilation
$(AES_BC): $(AES_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -emit-llvm -c $< -o $@

$(SHA_BC): $(SHA_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -emit-llvm -c $< -o $@

$(AES_WRAP_BC): $(AES_WRAP_PTR) | $(BUILD)
	$(CC) $(CFLAGS) -emit-llvm -c $< -o $@

$(SHA_WRAP_BC): $(SHA_WRAP_PTR) | $(BUILD)
	$(CC) $(CFLAGS) -emit-llvm -c $< -o $@

# link the modules the way the SAW scripts expect
$(AES_COMB_PTR): $(AES_WRAP_BC) $(AES_BC) | $(BUILD)
	$(LLVM_LINK) $^ -o $@

$(SHA_COMB_PTR): $(SHA_WRAP_BC) $(SHA_BC) | $(BUILD)
	$(LLVM_LINK) $^ -o $@

clean:
	rm -rf $(BUILD)