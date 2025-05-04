# Compiler and flags
CC = gcc
BASE_CFLAGS = -Wall -Wextra -Isrc/ -Wno-switch -Wno-sign-compare
LDFLAGS = -lm

# Debug flags
DEBUG_CFLAGS = -g -O0 -DDEBUG

# Release flags
RELEASE_CFLAGS = -O2 -DNDEBUG

# Binary name
BIN_NAME = interpreter

# Source files
SRCS = $(wildcard src/*.c)

# Object files
DEBUG_OBJS = $(patsubst src/%.c,build/debug/%.o,$(SRCS))
RELEASE_OBJS = $(patsubst src/%.c,build/release/%.o,$(SRCS))

# Default target
all: debug

# Debug build
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(BIN_NAME)

# Release build
release: CFLAGS += $(RELEASE_CFLAGS)
release: $(BIN_NAME)

# Executable rule
$(BIN_NAME): $(DEBUG_OBJS) $(RELEASE_OBJS)
	@# Link only the objects for current build type
	@if [ "$(MAKECMDGOALS)" = "debug" ] || [ "$(MAKECMDGOALS)" = "" ]; then \
		echo "Linking DEBUG objects"; \
		$(CC) $(CFLAGS) $(DEBUG_OBJS) -o $@ $(LDFLAGS); \
	else \
		echo "Linking RELEASE objects"; \
		$(CC) $(CFLAGS) $(RELEASE_OBJS) -o $@ $(LDFLAGS); \
	fi

# Pattern rules with proper separation
build/debug/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

build/release/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf build $(BIN_NAME)

.PHONY: all debug release clean