# Compiler & flags
CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic -std=c11
LDFLAGS := -lncurses -lao -lmpg123

# Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Target executable
TARGET  := $(BIN_DIR)/app

# Auto-discover all .c files and map them to .o files
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# ── Rules ─────────────────────────────────────────────────────────────────────

.PHONY: all clean debug release

all: $(TARGET)

# Link
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile each .c → .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create dirs if missing
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Debug build — adds symbols, disables optimization
debug: CFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

# Release build — optimized, no debug info
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

# Remove build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
