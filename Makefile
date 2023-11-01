# Compilation Arguments
CC := gcc
CFLAGS := -Wall -g

# All needed directory
SRC_DIR := src
BUILD_DIR := build
INC_DIR := include
LIB_DIR := -L./libs

LIBS := -ljansson
LDFLAGS := $(LIB_DIR) $(LIBS) -Wl,-rpath=./libs -static

# List of source files to bo excluded from compilation
EXCLUDED_SRC := src/old_main.c

# Find all source files in the 'src' directory, except those listed in the 'EXCLUDED_SRC
SRC := $(filter-out $(EXCLUDED_SRC), $(wildcard $(SRC_DIR)/*.c))

# Generate a list of object files in the 'build' directory from the source files	
OBJ := $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Rule to compile a source file into an object file (mkdir if the directory doesn't exists)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -I../jansson-2.14/src -I$(INC_DIR) -c $< -o $@

# Rule to compile a source file from the 'libs' directory into an object file
$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Default target, builds the 'main' executable after call to clean
all: clean main

# Target to build the 'main' executable using the object files
main: $(OBJ)
	@$(CC) $^ -o $@ $(LDFLAGS) -pthread

# Target to clean up generated file from the compilation
clean:
	rm -rf $(BUILD_DIR)/*.o main

# Indicate that 'clean' and 'all' are not actual files
.PHONY: clean all