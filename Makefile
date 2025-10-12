# -------- Toolchain --------
CC      ?= cc
CFLAGS  ?= -Wall -Wextra -Wpedantic -O2 -g -std=c11 -Iinclude
LDFLAGS ?=

# -------- Feature toggles --------
# Set these to 1 to include the DH and SIG sources/tests once they’re stable.
ENABLE_DH  ?= 0
ENABLE_SIG ?= 0

# -------- Outputs / Dirs --------
BUILD_DIR := build
LIB_NAME  := libcryptoar.a
LIB_PATH  := $(BUILD_DIR)/$(LIB_NAME)

SRC_DIRS := src
TEST_DIR  := tests
INC_DIR   := include

# -------- Source discovery --------
SRCS_ALL := $(shell find $(SRC_DIRS) -type f -name '*.c')

# Base exclusions (OpenSSL-era); conditionally re-include when toggled on.
EXCL_SRCS :=

# Protocols
ifeq ($(ENABLE_DH),0)
  EXCL_SRCS += src/protocols/dh.c
endif
ifeq ($(ENABLE_SIG),0)
  EXCL_SRCS += src/protocols/sig.c
endif

LIB_SRCS := $(filter-out $(EXCL_SRCS),$(SRCS_ALL))
LIB_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

# -------- Tests --------
TEST_SRCS_ALL := $(shell find $(TEST_DIR) -maxdepth 1 -type f -name 'test_*.c')

EXCL_TESTS :=
ifeq ($(ENABLE_DH),0)
  EXCL_TESTS += $(TEST_DIR)/test_dh.c
endif
ifeq ($(ENABLE_SIG),0)
  EXCL_TESTS += $(TEST_DIR)/test_sig.c
endif

TEST_SRCS := $(filter-out $(EXCL_TESTS),$(TEST_SRCS_ALL))
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%,$(TEST_SRCS))

# -------- Phonies --------
.PHONY: all lib test run-tests clean format full

# Default: build and run tests (with current toggles)
all: test

# Ensure build subdirs exist
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/src
	@mkdir -p $(BUILD_DIR)/src/protocols
	@mkdir -p $(BUILD_DIR)/src/primitives
	@mkdir -p $(BUILD_DIR)/tests

# Compile objects mirroring directory structure into build/
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# -------- Static library --------
lib: $(LIB_PATH)

$(LIB_PATH): $(LIB_OBJS) | $(BUILD_DIR)
	@rm -f $(LIB_PATH)
	ar rcs $(LIB_PATH) $(LIB_OBJS)
	@echo "Built $(LIB_PATH)"

# -------- Tests: each test links against the static lib --------
test: lib $(TEST_BINS)
	@echo "Running tests..."
	@set -e; for t in $(TEST_BINS); do echo "==> $$t"; "$$t"; done; echo "All tests passed."

$(BUILD_DIR)/%: $(TEST_DIR)/%.c $(LIB_PATH) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) $< $(LIB_PATH) -o $@ $(LDFLAGS)

# Convenience target: build + test with DH & SIG enabled
full:
	@$(MAKE) ENABLE_DH=1 ENABLE_SIG=1 clean
	@$(MAKE) ENABLE_DH=1 ENABLE_SIG=1 test

# -------- Utilities --------
# Prefer clang-format-16; fall back to clang-format if not found.
format:
	@if command -v clang-format-16 >/dev/null 2>&1; then \
		echo "Formatting with clang-format-16"; \
		clang-format-16 -i $(SRCS_ALL) $(TEST_SRCS_ALL) $(INC_DIR)/**/*.h 2>/dev/null || true; \
	elif command -v clang-format >/dev/null 2>&1; then \
		echo "Formatting with clang-format"; \
		clang-format -i $(SRCS_ALL) $(TEST_SRCS_ALL) $(INC_DIR)/**/*.h 2>/dev/null || true; \
	else \
		echo "clang-format not found; skipping format"; \
	fi

clean:
	rm -rf $(BUILD_DIR)