CC := gcc
CFLAGS := -Wall -g

SRC_DIR := src
BUILD_DIR := build
INC_DIR := include
LIB_DIR := -L./libs
LIBS := -ljansson
LDFLAGS := $(LIB_DIR) $(LIBS) -Wl,-rpath=./libs -static

EXCLUDED_SRC := src/old_main.c

SRC := $(filter-out $(EXCLUDED_SRC), $(wildcard $(SRC_DIR)/*.c))

OBJ := $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -I../jansson-2.14/src -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

all: clean main

main: $(OBJ)
	@$(CC) $^ -o $@ $(LDFLAGS) -pthread

clean:
	rm -rf $(BUILD_DIR)/*.o main

.PHONY: clean all