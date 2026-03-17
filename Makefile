


CC 		:= gcc
CFLAGS 	:= -Wall -Wextra

LDFLAGS :=

SRC_DIR	:= ./src
OBJ_DIR	:= ./obj
BIN_DIR := ./bin

TARGET 	:= http-server


SRCS    := $(shell find $(SRC_DIR) -name '*.c')

OBJS 	:= $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all run clean

all: $(BIN_DIR)/$(TARGET)
	@echo BIN DIR: $(BIN_DIR)

$(BIN_DIR)/$(TARGET): $(OBJS) 
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run: all 
	$(BIN_DIR)/$(TARGET)
