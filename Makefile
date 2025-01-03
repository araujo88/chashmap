CC=gcc
CC_FLAGS=-g -Wall -Wextra -Wpedantic
CC_LIBS=

SRC_DIR=src
HDR_DIR=include
OBJ_DIR=obj

# source and header files
SRC_FILES=$(wildcard $(SRC_DIR)/*.c)
HDR_FILES=$(wildcard $(HDR_DIR)/*.h)
OBJ_FILES=$(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SRC_FILES)))

VPATH = $(sort $(dir $(SRC_FILES)))

BIN_FILE=chashmap_example

all: $(OBJ_DIR) $(BIN_FILE)

$(BIN_FILE): $(OBJ_FILES)
	$(CC) $(CC_FLAGS) $^ -o $@ $(CC_LIBS)

$(OBJ_DIR)/%.o: %.c $(HDR_FILES)
	$(CC) $(CC_FLAGS) -c $< -I$(HDR_DIR) -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(BIN_FILE) $(OBJ_DIR)
