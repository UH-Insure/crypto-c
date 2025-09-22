# -------- Toolchain --------
CC      ?= cc
CFLAGS  ?= -Wall -Wextra -O2 -g -std=c11 -Iinclude
LDFLAGS ?=

# -------- Outputs / Dirs --------
BUILD_DIR := build
LIB_NAME  := libcryptoar.a
LIB_PATH  := $(BUILD_DIR)/$(LIB_NAME)

SRC_DIRS := src
TEST_DIR  := tests
INC_DIR   := include

# -------- Source discovery --------
# All .c files under src/
SRCS_ALL := $(shell find $(SRC_DIRS) -type f -name '*.c')

# Exclude protocol files that depended on OpenSSL (keep them for later TODOs)
EXCLUDE_SRCS := src/protocols/dh.c src/protocols/sig.c
LIB_SRCS := $(filter-out $(EXCLUDE_SRCS),$(SRCS_ALL))

# Objects mirror tree into build/
LIB_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

# -------- Tests --------
TEST_SRCS_ALL := $(shell find $(TEST_DIR) -maxdepth 1 -type f -name 'test_*.c')

# Exclude tests that required OpenSSL-backed DH/SIG
EXCLUDE_TESTS := $(TEST_DIR)/test_dh.c $(TEST_DIR)/test_sig.c
TEST_SRCS := $(filter-out $(EXCLUDE_TESTS),$(TEST_SRCS_ALL))
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%,$(TEST_SRCS))

# -------- Phonies --------
.PHONY: all lib test run-tests clean format

# Default: build library and run tests
all: test

# Ensure build subdirs exist (mirror src tree)
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

# -------- Utilities --------
format:
	@command -v clang-format-14 >/dev/null 2>&1 || { echo "clang-format-14 not found"; exit 0; }
	@clang-format-14 -i $(SRCS_ALL) $(TEST_SRCS_ALL) $(INC_DIR)/**/*.h 2>/dev/null || true

clean:
	rm -rf $(BUILD_DIR)
